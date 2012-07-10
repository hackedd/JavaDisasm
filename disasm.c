#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "classfile.h"
#include "bytecode.h"
#include "util.h"

void disassemble(const char* filename)
{
	ClassFile* classFile;
	int i;
	Constant *p, *ref, *className, *name, *descriptor;
	Method* method;
	Field* field;
	Attribute* codeAttribute;
	char buffer[1024], constantInfo[10], *dot, localClassName[128];

	classFile = read_class_file(filename);
	if (classFile == NULL)
	{
		fprintf(stderr, "Unable to read class file: '%s'\n", filename);
		return;
	}

	ref = find_constant(classFile, classFile->this_class);
	className = find_constant(classFile, ref->ref);

	fprintf(stdout, "/*\n    Filename: %s\n    Class %s\n*/\n", filename, class_name_from_internal(className->buffer));

	fprintf(stdout, "/*\n    Constant Pool\n\n");
	for (p = classFile->constants, i = 0; i < classFile->constant_count; i += 1, p += 1)
	{
		switch (p->tag)
		{
			case TAG_CLASSREF:
			case TAG_STRINGREF:
				sprintf(constantInfo, "#%hd", p->ref);
				break;

			case TAG_FIELDREF:
			case TAG_METHODREF:
			case TAG_IFACEREF:
				sprintf(constantInfo, "#%hd, #%hd", p->classref, p->typedescref);
				break;

			case TAG_TYPEDESC:
				sprintf(constantInfo, "#%hd, #%hd", p->nameref, p->typeref);
				break;

			default:
				strcpy(constantInfo, "");
		}

		fprintf(stdout, "    %3d: %-9s %-10s %s\n", p->index, ConstantTypes[p->tag].name, 
			constantInfo, constant_to_string_r(classFile, p, buffer));
	}
	fprintf(stdout, "*/\n");
	fprintf(stdout, "\n");

	fprintf(stdout, "%s class ", access_flags_to_string(classFile->access_flags));
	fprintf(stdout, "%s ", class_name_from_internal(className->buffer));

	dot = strrchr(class_name_from_internal(className->buffer), '.');
	strcpy(localClassName, dot ? dot + 1 : class_name_from_internal(className->buffer));

	if (classFile->super_class)
	{
		ref = find_constant(classFile, classFile->super_class);
		name = find_constant(classFile, ref->ref);
		fprintf(stdout, "extends %s", class_name_from_internal(name->buffer));
	}

	if (classFile->interface_count > 0)
	{
		fprintf(stdout, " implements");
		for (i = 0; i < classFile->interface_count; i += 1)
		{
			ref = find_constant(classFile, classFile->interfaces[i]);
			name = find_constant(classFile, ref->ref);
			fprintf(stdout, " %s", class_name_from_internal(name->buffer));
		}
	}

	fprintf(stdout, "\n");
	fprintf(stdout, "{\n");

	for (i = 0, field = classFile->fields; i < classFile->field_count; i += 1, field += 1)
	{
		name = find_constant(classFile, field->name_index);
		descriptor = find_constant(classFile, field->descriptor_index);
		descriptor_to_string(descriptor->buffer, name->buffer, buffer);
		fprintf(stdout, "    %s %s;\n", access_flags_to_string(field->access_flags), buffer);
	}

	for (i = 0, method = classFile->methods; i < classFile->method_count; i += 1, method += 1)
	{
		fprintf(stdout, "\n");

		name = find_constant(classFile, method->name_index);
		if (strcmp(name->buffer, "<clinit>") == 0)
		{
			// Static Class Initializer
			fprintf(stdout, "    static\n");
		}
		else
		{
			descriptor = find_constant(classFile, method->descriptor_index);

			if (strcmp(name->buffer, "<init>") == 0) // Constructor
				descriptor_to_string_ex(descriptor->buffer, localClassName, buffer, FLAG_OMIT_RETURN_TYPE);
			else
				descriptor_to_string(descriptor->buffer, name->buffer, buffer);

			fprintf(stdout, "    %s %s\n", access_flags_to_string(method->access_flags), buffer);
		}
		fprintf(stdout, "    {\n");

		codeAttribute = find_attribute(classFile, ATT_NAME_CODE, method->attribute_count, method->attributes);
		if (codeAttribute == NULL)
			fprintf(stdout, "        /* No Code */\n");
		else
			dump_code_attribute(stdout, classFile, codeAttribute);

		fprintf(stdout, "    }\n");
	}

	fprintf(stdout, "}\n");

	free_class(classFile);
}

int main(int argc, char** argv)
{
	int i;

	for (i = 1; i < argc; i += 1)
		disassemble(argv[i]);

	return 0;
}