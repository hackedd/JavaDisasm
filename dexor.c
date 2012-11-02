#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "classfile.h"
#include "bytecode.h"
#include "util.h"
#include "utf8.h"

void xorcrypt(uint32_t* buf, int length, unsigned char* key, int keylen);
uint8_t find_xor_byte(Attribute* codeAttribute, uint32_t start_pc);
int find_xor_key_in_method(ClassFile* classFile, Attribute* codeAttribute, unsigned char* key, int recursive);
int find_xor_method(ClassFile* classFile, Attribute* codeAttribute, uint32_t pc, unsigned char* key);
int find_xor_key(ClassFile* classFile, unsigned char* key);

static int verbose = 0;

void xorcrypt(uint32_t* buf, int length, unsigned char* key, int keylen)
{
	uint32_t *in, *out, *endptr = buf + length;
	int k;

	in = out = buf;
	k = 0;
	while (in < endptr)
	{
		if (in[0] == 0xFFFD && in[1] == 0xFFFD)
		{
			// Apparently, some obfuscators cause a NUL char to be output as
			// two Unicode Replacement chars.
			*out++ = key[k];
			in += 2;
		}
		else
		{
			*out++ = *in++ ^ key[k];
		}

		k = (k + 1) % keylen;
	}

	*out = L'\0';
}

uint8_t find_xor_byte(Attribute* codeAttribute, uint32_t start_pc)
{
	uint32_t pc, size;
	Instruction ins;

	for (pc = start_pc; pc < codeAttribute->code.code_length; )
	{
		size = get_single_instruction(codeAttribute->code.code + pc, &ins, pc);

		if (ins.opcode >= OP_ICONST_0 && ins.opcode <= OP_ICONST_5)
			return ins.opcode - OP_ICONST_0;
		if (ins.opcode == OP_BIPUSH)
			return ins.uint8;
		// does this happen?
		if (ins.opcode == OP_SIPUSH)
			return ins.uint16 & 0xFF;
		pc += size;
	}

	fprintf(stderr, "No bipush found at %d!", start_pc);
	return 0;
}

int find_xor_key_in_method(ClassFile* classFile, Attribute* codeAttribute, unsigned char* key, int recursive)
{
	uint32_t pc, size;
	Instruction ins;
	int i, is_load;

	for (pc = 0; pc < codeAttribute->code.code_length; )
	{
		size = get_single_instruction(codeAttribute->code.code + pc, &ins, pc);

		if (ins.opcode == OP_TABLESWITCH && ins.low == 0)
		{
			if (verbose > 1)
				fprintf(stderr, "Found tableswitch at %d, cases %d - %d\n", pc, ins.low, ins.high);

			if (ins.high > 10)
			{
				if (verbose > 1)
					fprintf(stderr, "  Discarding, too many cases\n");
			}
			else
			{
				for (i = 0; i <= ins.high - ins.low; i += 1)
					key[i] = find_xor_byte(codeAttribute, pc + ins.branchoffsets[i]);
				key[i] = find_xor_byte(codeAttribute, pc + ins.defaultoffset);
				return i + 1;
			}
		}

		if (recursive)
		{
			is_load  = ins.opcode == OP_ALOAD;
			is_load |= ins.opcode >= OP_ALOAD_0 && ins.opcode <= OP_ALOAD_3;
			is_load |= ins.opcode == OP_LDC || ins.opcode == OP_LDC_W;
			if (is_load && (i = find_xor_method(classFile, codeAttribute, pc + size, key)) > 0)
				return i;
		}

		pc += size;
	}

	return 0;
}

int find_xor_method(ClassFile* classFile, Attribute* codeAttribute, uint32_t pc, unsigned char* key)
{
	uint32_t size;
	Instruction ins;
	Constant *methodRef, *typedesc, *descriptor;
	Method* method;
	TypeDescriptor methodType;
	int i, keylen;

    // aload_0
    // { iconst_0-5 bipush 6-127 sipush 127-32767 }
    // * { ldc, ldc_w }
    // * invokestatic #175 // char[] com.whatsapp.App.z(java.lang.String param0)
    // * invokestatic #178 // java.lang.String com.whatsapp.App.z(char[] param0)
    // aastore

	size = get_single_instruction(codeAttribute->code.code + pc, &ins, pc);
	pc += size;

	if (ins.opcode != OP_INVOKESTATIC)
	{
		if ((ins.opcode < OP_ICONST_0 || ins.opcode > OP_ICONST_5) && ins.opcode != OP_BIPUSH && ins.opcode != OP_SIPUSH)
			return 0;

		size = get_single_instruction(codeAttribute->code.code + pc, &ins, pc);
		pc += size;

		if (ins.opcode != OP_LDC && ins.opcode != OP_LDC_W)
			return 0;

		size = get_single_instruction(codeAttribute->code.code + pc, &ins, pc);
		pc += size;

		if (ins.opcode != OP_INVOKESTATIC)
			return 0;
	}

	size = get_single_instruction(codeAttribute->code.code + pc, &ins, pc);
	pc += size;

	if (ins.opcode != OP_INVOKESTATIC)
		return 0;

	methodRef = find_constant(classFile, ins.constant);
	if (methodRef == NULL)
	{
		fprintf(stderr, "Unable to find method (#%d)\n", ins.constant);
		return 0;
	}

	typedesc = find_constant(classFile, methodRef->typedescref);
	if (typedesc == NULL)
	{
		fprintf(stderr, "Unable to find typedesc (#%d)\n", methodRef->typedescref);
		return 0;
	}

	descriptor = find_constant(classFile, typedesc->typeref);
	if (descriptor == NULL)
	{
		fprintf(stderr, "Unable to find descriptor (#%d)\n", typedesc->typeref);
		return 0;
	}

	if (parse_type_descriptor(descriptor->buffer, &methodType) == NULL)
	{
		fprintf(stderr, "Unable to parse method type descriptor (%s)\n", descriptor->buffer);
		return 0;
	}

	if (methodType.type == TYPE_METHOD && strcmp(methodType.returnType->name, "java.lang.String") == 0 &&
		methodType.param_count == 1 && methodType.params[0].type == TYPE_ARRAY &&
		strcmp(methodType.params[0].elementType->name, "char") == 0)
	{
		if (verbose > 1)
			fprintf(stderr, "Looking for method with name #%hd and descriptor #%hd\n", methodRef->nameref, methodRef->typedescref);

		for (i = 0, method = classFile->methods; i < classFile->method_count; i += 1, method += 1)
		{
			if (method->name_index == typedesc->nameref && method->descriptor_index == typedesc->typeref)
				break;
		}

		if (i >= classFile->method_count)
		{
			fprintf(stderr, "Method (name: #%hd, descriptor: #%hd) not found in class!\n", methodRef->nameref, methodRef->typedescref);
			return 0;
		}

		if (codeAttribute == NULL)
		{
			sprintf((char*)key, "Method has no " ATT_NAME_CODE " attribute (name: #%hd, descriptor: #%hd)", methodRef->nameref, methodRef->typedescref);
			return 0;
		}

		codeAttribute = find_attribute(classFile, ATT_NAME_CODE, method->attribute_count, method->attributes);
		keylen = find_xor_key_in_method(classFile, codeAttribute, key, 0);
	}
	else
	{
		keylen = 0;
	}

	free_type_descriptor(&methodType);
	return keylen;
}

int find_xor_key(ClassFile* classFile, unsigned char* key)
{
	// 1. Find <clinit> method
	// 2. Find tableswitch, either in <clinit> itself or in static method String[] z(char[] param0)
	//    2a. Follow each case, look for bipush
	//    2b. Use bipush argument as key byte
	// 4. ???
	// 5. Profit!

	Method* method;
	Constant* name;
	Attribute* codeAttribute;
	int i, keylen;

	for (i = 0, method = classFile->methods; i < classFile->method_count; i += 1, method += 1)
	{
		name = find_constant(classFile, method->name_index);
		if (strcmp(name->buffer, "<clinit>") == 0)
			break;
	}

	if (i >= classFile->method_count)
	{
		strcpy((char*)key, "no static initializer");
		return 0;
	}

	codeAttribute = find_attribute(classFile, ATT_NAME_CODE, method->attribute_count, method->attributes);
	if (codeAttribute == NULL)
	{
		strcpy((char*)key, "no " ATT_NAME_CODE " attribute");
		return 0;
	}

	if ((keylen = find_xor_key_in_method(classFile, codeAttribute, key, 1)) > 0)
		return keylen;

	strcpy((char*)key, "no usable tableswitch found");
	return 0;
}

int main(int argc, char** argv)
{
	int i, j, k, length, keylen, opt;
	ClassFile *classFile;
	Constant *classRef, *className, *c, *string;
	unsigned char key[128];
	char classNameString[255];
	uint32_t wbuffer[1024];

	int output_as_java_array = 0;

	while ((opt = getopt(argc, argv, "vhj")) != -1)
	{
		switch (opt)
		{
			case 'v':
				verbose += 1;
				break;

			case 'j':
				output_as_java_array = 1;
				break;

			case 'h':
			case '?':
				printf("Usage: %s [options] CLASSFILE...\n"
					"options:\n"
					"  -v  increase verbosity (can be specified multiple times)\n"
					"  -j  output strings as Java array\n"
					"", argv[0]);
				return optopt ? 1 : 0;
		}
	}

	for (i = optind; i < argc; i += 1)
	{
		classFile = read_class_file(argv[i]);
		if (classFile == NULL)
		{
			fprintf(stderr, "%s: Unable to read class file\n", argv[i]);
			continue;
		}

		classRef = find_constant(classFile, classFile->this_class);
		className = find_constant(classFile, classRef->ref);

		strcpy(classNameString, class_name_from_internal(className->buffer));

		key[0] = '\0';
		if ((keylen = find_xor_key(classFile, key)) == 0)
		{
			fprintf(stderr, "%s: Unable to find XOR key (%s)\n", classNameString, key);
		}
		else
		{
			if (verbose)
			{
				if (output_as_java_array)
					printf("// ");

				printf("%s  key: ", classNameString);
				for (j = 0; j < keylen; j += 1)
					printf("%02x ", key[j]);
				printf("\n");
			}

			if (output_as_java_array)
				printf("private static final String[] z = new String[] {\n");

			for (c = classFile->constants, j = k = 0; j < classFile->constant_count; j += 1, c += 1)
			{
				if (c->tag != TAG_STRINGREF)
					continue;

				string = find_constant(classFile, c->ref);
				length = u8_toucs(wbuffer, sizeof(wbuffer) / sizeof(wbuffer[0]), string->buffer, string->length + 1);

				if (verbose > 1)
				{
					if (output_as_java_array)
						printf("// ");

					printf("%s  raw: ", classNameString);
					print_string(stdout, string->buffer);
					printf("\n");
				}

				xorcrypt(wbuffer, length - 1, key, keylen);

				if (output_as_java_array)
				{
					printf("\t/* %2d */ \"", k);
					print_string_w(stdout, wbuffer);
					printf("\",\n");
				}
				else
				{
					printf("%s %4d: ", classNameString, c->index);
					print_string_w(stdout, wbuffer);
					printf("\n");
				}

				k += 1;
			}

			if (output_as_java_array)
				printf("};\n\n");
		}

		free_class(classFile);
	}

	return 0;
}