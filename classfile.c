#include "classfile.h"

ConstantType ConstantTypes[] = {
	{ 0, "invalid_0" },

	/* 1: UTF-8 (Unicode) string: a character string prefixed by a 16-bit
		number (type u2) indicating the number of bytes in the encoded string
		which immediately follows (which may be different than the number of
		characters).
		Note that the encoding used is not actually UTF-8, but involves a
		slight modification of the Unicode standard encoding form. */
	{ 2, "string" },

	/* 2: Missing */
	{ 0, "invalid_2" },

	/* 3: Integer: a signed 32-bit two's complement number in big-endian format */
	{ 4, "integer" },

	/* 4: Float: a 32-bit single-precision IEEE 754 floating-point number */
	{ 4, "float" },

	/* 5: Long: a signed 64-bit two's complement number in big-endian format
		(takes two slots in the constant pool table) */
	{ 8, "long" },

	/* 6: Double: a 64-bit double-precision IEEE 754 floating-point number
		(takes two slots in the constant pool table) */
	{ 8, "double" },

	/* 7: Class reference: an index within the constant pool to a UTF-8 string
		containing the fully qualified classFile name (in internal format) */
	{ 2, "classref" },

	/* 8: String reference: an index within the constant pool to a UTF-8 string */
	{ 2, "stringref" },

	/* 9: Field reference: two indexes within the constant pool, the first
		pointing to a Class reference, the second to a Name and Type descriptor. */
	{ 4, "fieldref" },

	/* 10: Method reference: two indexes within the constant pool, the first
		pointing to a Class reference, the second to a Name and Type descriptor. */
	{ 4, "methodref" },

	/* 11: Interface method reference: two indexes within the constant pool,
		the first pointing to a Class reference, the second to a
		Name and Type descriptor. */
	{ 4, "ifaceref" },

	/* 12: Name and type descriptor: two indexes to UTF-8 strings within the
		constant pool, the first representing a name (identifier) and the
		second a specially encoded type descriptor. */
	{ 4, "typedesc" },
};

BaseType BaseTypes[] = {
	{ 'B', "byte"    },
	{ 'C', "char"    },
	{ 'D', "double"  },
	{ 'F', "float"   },
	{ 'I', "int"     },
	{ 'J', "long"    },
	{ 'S', "short"   },
	{ 'Z', "boolean" },
	{ 'V', "void"    },
	{  -1, NULL      }
};

#define read16(field) \
	fread(&uint16, sizeof(uint16_t), 1, fp); \
	field = be16toh(uint16)

#define read32(field) \
	fread(&uint32, sizeof(uint32_t), 1, fp); \
	field = be32toh(uint32)


int read_constants(FILE* fp, Constant** constants)
{
	uint16_t uint16;
	uint8_t tag;
	int max_index, i, length;
	char *buffer, shortbuf[8];
	Constant* p;

	// fseek(fp, sizeof(ClassFileHeader), SEEK_SET);
	read16(max_index);

	p = *constants = (Constant*)malloc((max_index - 1) * sizeof(Constant));

	for (i = 1; i < max_index; i += 1)
	{
		fread(&tag, sizeof(tag), 1, fp);
		if (tag == TAG_STRING)
		{
			fread(&uint16, sizeof(uint16), 1, fp);
			length = be16toh(uint16);

			buffer = malloc((length + 1) * sizeof(char));
			fread(buffer, sizeof(char), length, fp);
			buffer[length] = '\0';

			p->buffer = buffer;
			p->length = length;
		}
		else
		{
			length = ConstantTypes[tag].length;
			fread(shortbuf, sizeof(char), length, fp);
		}

		p->index = i;
		p->tag = tag;

		switch (tag)
		{
			case TAG_STRING:
				// Already handled above.
				break;

			case TAG_INTEGER:
				p->intval = be32toh(*(int32_t*)shortbuf);
				break;

			case TAG_FLOAT:
				p->floatval = *(float*)shortbuf;
				break;

			case TAG_LONG:
				p->longval = be64toh(*(int64_t*)shortbuf);
				break;

			case TAG_DOUBLE:
				p->doubleval = *(double*)shortbuf;
				break;

			case TAG_CLASSREF:
			case TAG_STRINGREF:
				p->ref = be16toh(*(uint16_t*)shortbuf);
				break;

			case TAG_FIELDREF:
			case TAG_METHODREF:
			case TAG_IFACEREF:
				p->classref = be16toh(*(uint16_t*)shortbuf);
				p->typedescref = be16toh(*(uint16_t*)(shortbuf + sizeof(uint16_t)));
				break;

			case TAG_TYPEDESC:
				p->nameref = be16toh(*(uint16_t*)shortbuf);
				p->typeref = be16toh(*(uint16_t*)(shortbuf + sizeof(uint16_t)));
				break;

			default:
				fprintf(stderr, "Warning: Invalid Constant Tag: %d\n", tag);
				break;
		}

		p += 1;

		if (tag == TAG_LONG || tag == TAG_DOUBLE)
			i += 1; /* Longs and doubles take up two slots in the table */
	}

	return p - *constants;
}

Constant* find_constant(ClassFile* classFile, int index)
{
	/* We take advantage of two facts here:
		1) Constants are always sorted with increasing indexes
		2) The constant index is always at least one higher than the array offset
	*/

	Constant *p = classFile->constants;

	if (index - 1 < classFile->constant_count)
		p += index - 1;
	else
		p += classFile->constant_count - 1;

	for ( ; p >= classFile->constants; p -= 1)
		if (p->index == index)
			return p;
	return NULL;
}

void free_constants(int count, Constant* constants)
{
	Constant *p;
	int i;

	for (p = constants, i = 0; i < count; i += 1, p += 1)
		if (p->tag == TAG_STRING)
			free(p->buffer);
	free(constants);
}

const char* constant_to_string_r(ClassFile* classFile, Constant* constant, char* buffer)
{
	Constant *ref, *className, *name, *typedesc, *descriptor;

	char fullname[255];

	switch (constant->tag)
	{
		case TAG_STRING:
			strcpy(buffer, "\"");
			escape_string(buffer + 1, constant->buffer, constant->length);
			strcat(buffer, "\"");
			break;

		case TAG_INTEGER:
			sprintf(buffer, "%d", constant->intval);
			break;

		case TAG_FLOAT:
			sprintf(buffer, "%f", constant->floatval);
			break;

		case TAG_LONG:
			sprintf(buffer, "%ld", constant->longval);
			break;

		case TAG_DOUBLE:
			sprintf(buffer, "%f", constant->doubleval);
			break;

		case TAG_CLASSREF:
			ref = find_constant(classFile, constant->ref);
			sprintf(buffer, "class %s", class_name_from_internal(ref->buffer));
			break;

		case TAG_STRINGREF:
			ref = find_constant(classFile, constant->ref);
			constant_to_string_r(classFile, ref, buffer);
			break;

		case TAG_FIELDREF:
		case TAG_METHODREF:
		case TAG_IFACEREF:
			ref = find_constant(classFile, constant->classref);
			className = find_constant(classFile, ref->ref);
			typedesc = find_constant(classFile, constant->typedescref);
			name = find_constant(classFile, typedesc->nameref);
			descriptor = find_constant(classFile, typedesc->typeref);

			sprintf(fullname, "%s.%s", class_name_from_internal(className->buffer), name->buffer);
			descriptor_to_string(descriptor->buffer, fullname, buffer);
			break;

		case TAG_TYPEDESC:
			name = find_constant(classFile, constant->nameref);
			descriptor = find_constant(classFile, constant->typeref);

			descriptor_to_string(descriptor->buffer, name->buffer, buffer);
			break;

		default:
			sprintf(buffer, "???");
			break;
	}

	return buffer;
}

const char* constant_to_string(ClassFile* classFile, Constant* constant)
{
	static char buffer[1024];
	return constant_to_string_r(classFile, constant, buffer);
}

uint16_t read_attributes(FILE* fp, ClassFile* classFile, Attribute** attributes)
{
	int i, j;
	uint16_t uint16, count;
	uint32_t uint32;
	Attribute* a;
	Constant* name;
	ExceptionTableEntry* e;

	read16(count);
	a = *attributes = malloc(sizeof(Attribute) * count);

	for (i = 0; i < count; i += 1)
	{
		read16(a->name_index);
		read32(a->length);

		name = find_constant(classFile, a->name_index);
		if (strcmp(name->buffer, ATT_NAME_CODE) == 0)
		{
			a->type = ATT_CODE;

			read16(a->code.max_stack);
			read16(a->code.max_locals);
			read32(a->code.code_length);

			a->code.code = malloc(a->code.code_length);
			fread(a->code.code, sizeof(char), a->code.code_length, fp);

			read16(a->code.exception_table_length);
			e = a->code.exception_table = malloc(a->code.exception_table_length * sizeof(ExceptionTableEntry));
			for (j = 0; j < a->code.exception_table_length; j += 1)
			{
				read16(e->start_pc);
				read16(e->end_pc);
				read16(e->handler_pc);
				read16(e->catch_type);

				e += 1;
			}

			a->code.attribute_count = read_attributes(fp, classFile, &a->code.attributes);
		}
		else
		{
			a->type = ATT_UNKNOWN;
			a->buffer = malloc(a->length);
			fread(a->buffer, sizeof(char), a->length, fp);
		}

		a += 1;
	}

	return count;
}

Attribute* find_attribute(ClassFile* classFile, const char* name, int attribute_count, Attribute* attributes)
{
	Attribute* a;
	Constant* c;
	int i;

	for (a = attributes, i = 0; i < attribute_count; i += 1, a += 1)
	{
		c = find_constant(classFile, a->name_index);
		if (strcmp(c->buffer, name) == 0)
			return a;
	}

	return NULL;
}

ClassFile* read_class_file(const char* filename)
{
	ClassFile* classFile;
	FILE* fp;

	if ((fp = fopen(filename, "r")) == NULL)
	{
		perror("fopen");
		return NULL;
	}

	classFile = read_class(fp);
	fclose(fp);

	return classFile;
}

ClassFile* read_class(FILE* fp)
{
	ClassFile* classFile = malloc(sizeof(ClassFile));
	Field* field;
	Method* method;
	uint16_t uint16;
	int i;

	memset(classFile, 0, sizeof(ClassFile));

	fread(&classFile->header, sizeof(ClassFileHeader), 1, fp);
	if (be32toh(classFile->header.magic) != MAGIC)
	{
		free_class(classFile);
		return NULL;
	}

	swap32(classFile->header.magic);
	swap16(classFile->header.minor);
	swap16(classFile->header.major);

	classFile->constant_count = read_constants(fp, &classFile->constants);

	read16(classFile->access_flags);
	read16(classFile->this_class);
	read16(classFile->super_class);

	read16(classFile->interface_count);
	classFile->interfaces = malloc(sizeof(uint16_t) * classFile->interface_count);
	for (i = 0; i < classFile->interface_count; i += 1)
	{
		read16(classFile->interfaces[i]);
	}

	read16(classFile->field_count);
	field = classFile->fields = malloc(sizeof(Field) * classFile->field_count);
	for (i = 0; i < classFile->field_count; i += 1)
	{
		read16(field->access_flags);
		read16(field->name_index);
		read16(field->descriptor_index);

		field->attribute_count = read_attributes(fp, classFile, &field->attributes);
		field += 1;
	}

	read16(classFile->method_count);
	method = classFile->methods = malloc(sizeof(Method) * classFile->method_count);
	for (i = 0; i < classFile->method_count; i += 1)
	{
		read16(method->access_flags);
		read16(method->name_index);
		read16(method->descriptor_index);
		method->attribute_count = read_attributes(fp, classFile, &method->attributes);
		method += 1;
	}

	classFile->attribute_count = read_attributes(fp, classFile, &classFile->attributes);

	return classFile;
}

void free_class(ClassFile* classFile)
{
	if (classFile == NULL)
		return;

	if (classFile->constants != NULL)
		free_constants(classFile->constant_count, classFile->constants);

	if (classFile->interfaces != NULL)
		free(classFile->interfaces);

	if (classFile->fields != NULL)
		free(classFile->fields);

	if (classFile->methods != NULL)
		free(classFile->methods);

	if (classFile->attributes != NULL)
		free(classFile->attributes);

	free(classFile);
}

const char* access_flags_to_string(uint16_t access_flags)
{
	static char buf[255];

	if (access_flags & ACC_PUBLIC)
		strcpy(buf, "public");
	else if (access_flags & ACC_PRIVATE)
		strcpy(buf, "private");
	else if (access_flags & ACC_PROTECTED)
		strcpy(buf, "protected");
	else
		strcpy(buf, "");

	if (access_flags & ACC_STATIC)
		strcat(buf, " static");

	if (access_flags & ACC_FINAL)
		strcat(buf, " final");

	if (access_flags & ACC_VOLATILE)
		strcat(buf, " volatile");

	if (access_flags & ACC_TRANSIENT)
		strcat(buf, " transient");

	if (access_flags & ACC_SYNTHETIC)
		strcat(buf, " /* synthetic */");

	if (access_flags & ACC_ENUM)
		strcat(buf, " enum");

	return buf;
}

void free_type_descriptor(TypeDescriptor* typeDescriptor)
{
	int i;

	if (typeDescriptor->name)
		free(typeDescriptor->name);

	if (typeDescriptor->type == TYPE_ARRAY)
	{
		free_type_descriptor(typeDescriptor->elementType);
		free(typeDescriptor->elementType);
	}
	else if (typeDescriptor->type == TYPE_METHOD)
	{
		free_type_descriptor(typeDescriptor->returnType);
		free(typeDescriptor->returnType);

		for (i = 0; i < typeDescriptor->param_count; i += 1)
			free_type_descriptor(&typeDescriptor->params[i]);
		free(typeDescriptor->params);
	}
}

const char* parse_type_descriptor(const char* descriptor, TypeDescriptor* typeDescriptor)
{
	const char* p, *ret;
	char *q;
	BaseType* bt;
	int i, dimensions;

	memset(typeDescriptor, 0, sizeof(TypeDescriptor));

	if (descriptor[0] == '(')
	{
		typeDescriptor->type = TYPE_METHOD;
		// Method Descriptor

		// First, skip params
		for (p = descriptor + 1; *p && *p != ')'; )
			p += 1;
		p += 1;

		// parse return type
		typeDescriptor->returnType = malloc(sizeof(TypeDescriptor));
		ret = parse_type_descriptor(p, typeDescriptor->returnType);

		// parse the parameter types
		typeDescriptor->params = NULL;
		typeDescriptor->param_count = 0;
		for (p = descriptor + 1, i = 0; *p != ')'; i += 1)
		{
			typeDescriptor->param_count += 1;
			typeDescriptor->params = realloc(typeDescriptor->params, sizeof(TypeDescriptor) * typeDescriptor->param_count);
			p = parse_type_descriptor(p, &typeDescriptor->params[i]);
		}

		return ret;
	}

	if (descriptor[0] == '[')
	{
		typeDescriptor->type = TYPE_ARRAY;

		for (dimensions = 0, p = descriptor; *p && *p == '['; p += 1)
			dimensions += 1;

		typeDescriptor->dimensions = dimensions;

		// parse element type
		typeDescriptor->elementType = malloc(sizeof(TypeDescriptor));
		return parse_type_descriptor(p, typeDescriptor->elementType);
	}

	if (descriptor[0] == 'L')
	{
		typeDescriptor->type = TYPE_CLASS;

		p = strchr(descriptor + 1, ';');
		typeDescriptor->name = malloc((p - descriptor) * sizeof(char));

		for (p = descriptor + 1, q = typeDescriptor->name; *p && *p != ';'; )
		{
			if (*p == '/') *q++ = '.', p++;
			else           *q++ = *p++;
		}
		*q++ = '\0';

		return p + 1;
	}

	for (bt = BaseTypes; bt->name; bt += 1)
	{
		if (bt->type == descriptor[0])
		{
			typeDescriptor->type = TYPE_BASE;
			typeDescriptor->name = strdup(bt->name);
			return descriptor + 1;
		}
	}

	typeDescriptor->type = TYPE_UNKNOWN;
	return NULL;
}

const char* descriptor_to_string_ex(const char* descriptor, const char* name, char* buf, int flags)
{
	const char* p, *ret;
	char *q, paramName[10];
	BaseType* bt;
	int i, dimensions;

	if (descriptor[0] == '(')
	{
		// Method Descriptor

		// First, skip params
		for (p = descriptor + 1; *p && *p != ')'; )
			p += 1;
		p += 1;

		// Format return type and method name
		if (flags & FLAG_OMIT_RETURN_TYPE)
		{
			if (name != NULL)
				strcpy(buf, name);
		}
		else
			ret = descriptor_to_string_ex(p, name, buf, FLAG_NONE);

		// Add some parens
		strcat(buf, "(");

		// Add the parameters
		for (p = descriptor + 1, i = 0; *p != ')'; i += 1)
		{
			if (i > 0)
				strcat(q, ", ");

			q = buf + strlen(buf);
			sprintf(paramName, "param%d", i);
			p = descriptor_to_string_ex(p, paramName, q, FLAG_NONE);
		}

		strcat(buf, ")");
		return ret;
	}

	if (descriptor[0] == '[')
	{
		for (dimensions = 0, p = descriptor; *p && *p == '['; p += 1)
			dimensions += 1;

		// Format type without name
		ret = descriptor_to_string_ex(p, NULL, buf, FLAG_NONE);

		// Append array dimensions
		for (i = 0; i < dimensions; i += 1)
			strcat(buf, "[]");

		// Append name
		if (name != NULL)
		{
			strcat(buf, " ");
			strcat(buf, name);
		}

		return ret;
	}

	if (descriptor[0] == 'L')
	{
		for (p = descriptor + 1, q = buf; *p && *p != ';'; )
		{
			if (*p == '/') *q++ = '.', p++;
			else           *q++ = *p++;
		}
		*q++ = '\0';

		if (name != NULL)
		{
			strcat(buf, " ");
			strcat(buf, name);
		}

		return p + 1;
	}

	for (bt = BaseTypes; bt->name; bt += 1)
	{
		if (bt->type == descriptor[0])
		{
			if (name != NULL)
				sprintf(buf, "%s %s", bt->name, name);
			else
				strcpy(buf, bt->name);
			return descriptor + 1;
		}
	}

	return NULL;
}

void descriptor_to_string(const char* descriptor, const char* name, char* buf)
{
	descriptor_to_string_ex(descriptor, name, buf, FLAG_NONE);
}

const char* class_name_from_internal(const char* name)
{
	static char buf[255];
	const char* in;
	char* out;

	for (in = name, out = buf; *in; in++)
	{
		if (*in == '/') *out++ = '.';
		else            *out++ = *in;
	}

	*out++ = '\0';
	return buf;
}

const char* class_name_to_internal(const char* name)
{
	static char buf[255];
	const char* in;
	char* out;

	for (in = name, out = buf; *in; in++)
	{
		if (*in == '.') *out++ = '/';
		else            *out++ = *in;
	}

	*out++ = '\0';
	return buf;
}

