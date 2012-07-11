#ifndef CLASSFILE_H
#define CLASSFILE_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "byteorder.h"
#include "util.h"

#define MAGIC 0xCAFEBABE

typedef struct
{
	uint32_t magic; // 0xCAFEBABE
	uint16_t minor;
	uint16_t major;
} ClassFileHeader;

typedef struct
{
	int length;
	const char* name;
} ConstantType;

typedef struct
{
	char type;
	const char* name;
} BaseType;

extern ConstantType ConstantTypes[];
extern BaseType BaseTypes[];

#define TAG_STRING	   1
#define TAG_INTEGER	   3
#define TAG_FLOAT	   4
#define TAG_LONG	   5
#define TAG_DOUBLE	   6
#define TAG_CLASSREF   7
#define TAG_STRINGREF  8
#define TAG_FIELDREF   9
#define TAG_METHODREF 10
#define TAG_IFACEREF  11
#define TAG_TYPEDESC  12

/* Declared  public; may be accessed from outside its package. */
#define ACC_PUBLIC	   0x0001
/* Declared  private; usable only within the defining class. */
#define ACC_PRIVATE	0x0002
/* Declared  protected; may be accessed within subclasses. */
#define ACC_PROTECTED  0x0004
/* Declared static. */
#define ACC_STATIC	   0x0008
/* Declared final; never directly assigned to after object construction (JLS §17.5). */
#define ACC_FINAL	   0x0010
/* Declared volatile; cannot be cached. */
#define ACC_VOLATILE   0x0040
/* Declared transient; not written or read by a persistent object manager. */
#define ACC_TRANSIENT  0x0080
/*  Was an interface in source. */
#define ACC_INTERFACE  0x0200
/*  Marked or implicitly abstract in source. */
#define ACC_ABSTRACT   0x0400
/* Declared synthetic; not present in the source code. */
#define ACC_SYNTHETIC  0x1000
/* Declared as an annotation type. */
#define ACC_ANNOTATION 0x2000
/* Declared as an element of an enum. */
#define ACC_ENUM	   0x4000

typedef struct
{
	int index;
	int tag;

	union
	{
		struct			  /* TAG_STRING */
		{
			int length;
			char* buffer;
		};
		int32_t intval;   /* TAG_INTEGER */
		float floatval;   /* TAG_FLOAT */
		int64_t longval;  /* TAG_LONG */
		double doubleval; /* TAG_DOUBLE */
		uint16_t ref;	  /* TAG_CLASSREF, TAG_STRINGREF */
		struct			  /* TAG_FIELDREF, TAG_METHODREF, TAG_IFACEREF */
		{
			uint16_t classref;
			uint16_t typedescref;
		};
		struct			  /* TAG_TYPEDESC */
		{
			uint16_t nameref;
			uint16_t typeref;
		};
	};
} Constant;

typedef struct
{
	uint16_t start_pc;
	uint16_t end_pc;
	uint16_t handler_pc;
	uint16_t catch_type;
} ExceptionTableEntry;

#define ATT_NAME_CODE "Code"

#define ATT_UNKNOWN 0
#define ATT_CODE    1

typedef struct tagAttribute
{
	uint16_t name_index;
	uint32_t length;

	int type;
	char* buffer;

	union
	{
		uint16_t constantvalue_index;
		struct
		{
			uint16_t max_stack;
			uint16_t max_locals;
			uint32_t code_length;
			unsigned char* code;
			uint16_t exception_table_length;
			ExceptionTableEntry* exception_table;
			uint16_t attribute_count;
			struct tagAttribute* attributes;
		} code;
		struct
		{
			uint16_t number_of_exceptions;
			uint16_t exception_index_table;
		} exceptions;
	};
} Attribute;

typedef struct
{
	uint16_t access_flags;
	uint16_t name_index;
	uint16_t descriptor_index;
	uint16_t attribute_count;
	Attribute* attributes;
} Field;

typedef struct
{
	uint16_t access_flags;
	uint16_t name_index;
	uint16_t descriptor_index;
	uint16_t attribute_count;
	Attribute* attributes;
} Method;

typedef struct
{
	ClassFileHeader header;
	uint16_t constant_count;
	uint16_t access_flags;
	uint16_t this_class;
	uint16_t super_class;
	uint16_t interface_count;
	uint16_t field_count;
	uint16_t method_count;
	uint16_t attribute_count;

	Constant* constants;
	uint16_t* interfaces;
	Field* fields;
	Method* methods;
	Attribute* attributes;
} ClassFile;

#define TYPE_UNKNOWN 0
#define TYPE_BASE    1
#define TYPE_CLASS   2
#define TYPE_ARRAY   3
#define TYPE_METHOD  4

typedef struct tagTypeDescriptor
{
	int type;
	char* name;
	union
	{
		int dimensions;  // for arrays
		int param_count; // for methods
	};
	union
	{
		struct tagTypeDescriptor* elementType;
		struct tagTypeDescriptor* returnType;
	};
	struct tagTypeDescriptor* params;
} TypeDescriptor;

int read_constants(FILE* fp, Constant** constants);
void free_constants(int count, Constant* constants);

uint16_t read_attributes(FILE* fp, ClassFile* classFile, Attribute** attributes);
Attribute* find_attribute(ClassFile* classFile, const char* name, int attribute_count, Attribute* attributes);

ClassFile* read_class(FILE* fp);
ClassFile* read_class_file(const char* filename);
void free_class(ClassFile* classFile);
Constant* find_constant(ClassFile* classFile, int index);

const char* constant_to_string(ClassFile* classFile, Constant* constant);
const char* constant_to_string_r(ClassFile* classFile, Constant* constant, char* buffer);
const char* access_flags_to_string(uint16_t access_flags);

#define FLAG_NONE             0
#define FLAG_OMIT_NAME        1
#define FLAG_OMIT_RETURN_TYPE 2

const char* parse_type_descriptor(const char* descriptor, TypeDescriptor* typeDescriptor);
void free_type_descriptor(TypeDescriptor* typeDescriptor);

void descriptor_to_string(const char* descriptor, const char* name, char* buf);
const char* descriptor_to_string_ex(const char* descriptor, const char* name, char* buf, int flags);

const char* class_name_from_internal(const char* name);
const char* class_name_to_internal(const char* name);

#endif