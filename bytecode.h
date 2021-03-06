#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdint.h>
#include <stdio.h>

#include "byteorder.h"
#include "classfile.h"

#define OP_NOP             0x00
#define OP_ACONST_NULL     0x01
#define OP_ICONST_M1       0x02
#define OP_ICONST_0        0x03
#define OP_ICONST_1        0x04
#define OP_ICONST_2        0x05
#define OP_ICONST_3        0x06
#define OP_ICONST_4        0x07
#define OP_ICONST_5        0x08
#define OP_LCONST_0        0x09
#define OP_LCONST_1        0x0a
#define OP_FCONST_0        0x0b
#define OP_FCONST_1        0x0c
#define OP_FCONST_2        0x0d
#define OP_DCONST_0        0x0e
#define OP_DCONST_1        0x0f
#define OP_BIPUSH          0x10
#define OP_SIPUSH          0x11
#define OP_LDC             0x12
#define OP_LDC_W           0x13
#define OP_LDC2_W          0x14
#define OP_ILOAD           0x15
#define OP_LLOAD           0x16
#define OP_FLOAD           0x17
#define OP_DLOAD           0x18
#define OP_ALOAD           0x19
#define OP_ILOAD_0         0x1a
#define OP_ILOAD_1         0x1b
#define OP_ILOAD_2         0x1c
#define OP_ILOAD_3         0x1d
#define OP_LLOAD_0         0x1e
#define OP_LLOAD_1         0x1f
#define OP_LLOAD_2         0x20
#define OP_LLOAD_3         0x21
#define OP_FLOAD_0         0x22
#define OP_FLOAD_1         0x23
#define OP_FLOAD_2         0x24
#define OP_FLOAD_3         0x25
#define OP_DLOAD_0         0x26
#define OP_DLOAD_1         0x27
#define OP_DLOAD_2         0x28
#define OP_DLOAD_3         0x29
#define OP_ALOAD_0         0x2a
#define OP_ALOAD_1         0x2b
#define OP_ALOAD_2         0x2c
#define OP_ALOAD_3         0x2d
#define OP_IALOAD          0x2e
#define OP_LALOAD          0x2f
#define OP_FALOAD          0x30
#define OP_DALOAD          0x31
#define OP_AALOAD          0x32
#define OP_BALOAD          0x33
#define OP_CALOAD          0x34
#define OP_SALOAD          0x35
#define OP_ISTORE          0x36
#define OP_LSTORE          0x37
#define OP_FSTORE          0x38
#define OP_DSTORE          0x39
#define OP_ASTORE          0x3a
#define OP_ISTORE_0        0x3b
#define OP_ISTORE_1        0x3c
#define OP_ISTORE_2        0x3d
#define OP_ISTORE_3        0x3e
#define OP_LSTORE_0        0x3f
#define OP_LSTORE_1        0x40
#define OP_LSTORE_2        0x41
#define OP_LSTORE_3        0x42
#define OP_FSTORE_0        0x43
#define OP_FSTORE_1        0x44
#define OP_FSTORE_2        0x45
#define OP_FSTORE_3        0x46
#define OP_DSTORE_0        0x47
#define OP_DSTORE_1        0x48
#define OP_DSTORE_2        0x49
#define OP_DSTORE_3        0x4a
#define OP_ASTORE_0        0x4b
#define OP_ASTORE_1        0x4c
#define OP_ASTORE_2        0x4d
#define OP_ASTORE_3        0x4e
#define OP_IASTORE         0x4f
#define OP_LASTORE         0x50
#define OP_FASTORE         0x51
#define OP_DASTORE         0x52
#define OP_AASTORE         0x53
#define OP_BASTORE         0x54
#define OP_CASTORE         0x55
#define OP_SASTORE         0x56
#define OP_POP             0x57
#define OP_POP2            0x58
#define OP_DUP             0x59
#define OP_DUP_X1          0x5a
#define OP_DUP_X2          0x5b
#define OP_DUP2            0x5c
#define OP_DUP2_X1         0x5d
#define OP_DUP2_X2         0x5e
#define OP_SWAP            0x5f
#define OP_IADD            0x60
#define OP_LADD            0x61
#define OP_FADD            0x62
#define OP_DADD            0x63
#define OP_ISUB            0x64
#define OP_LSUB            0x65
#define OP_FSUB            0x66
#define OP_DSUB            0x67
#define OP_IMUL            0x68
#define OP_LMUL            0x69
#define OP_FMUL            0x6a
#define OP_DMUL            0x6b
#define OP_IDIV            0x6c
#define OP_LDIV            0x6d
#define OP_FDIV            0x6e
#define OP_DDIV            0x6f
#define OP_IREM            0x70
#define OP_LREM            0x71
#define OP_FREM            0x72
#define OP_DREM            0x73
#define OP_INEG            0x74
#define OP_LNEG            0x75
#define OP_FNEG            0x76
#define OP_DNEG            0x77
#define OP_ISHL            0x78
#define OP_LSHL            0x79
#define OP_ISHR            0x7a
#define OP_LSHR            0x7b
#define OP_IUSHR           0x7c
#define OP_LUSHR           0x7d
#define OP_IAND            0x7e
#define OP_LAND            0x7f
#define OP_IOR             0x80
#define OP_LOR             0x81
#define OP_IXOR            0x82
#define OP_LXOR            0x83
#define OP_IINC            0x84
#define OP_I2L             0x85
#define OP_I2F             0x86
#define OP_I2D             0x87
#define OP_L2I             0x88
#define OP_L2F             0x89
#define OP_L2D             0x8a
#define OP_F2I             0x8b
#define OP_F2L             0x8c
#define OP_F2D             0x8d
#define OP_D2I             0x8e
#define OP_D2L             0x8f
#define OP_D2F             0x90
#define OP_I2B             0x91
#define OP_I2C             0x92
#define OP_I2S             0x93
#define OP_LCMP            0x94
#define OP_FCMPL           0x95
#define OP_FCMPG           0x96
#define OP_DCMPL           0x97
#define OP_DCMPG           0x98
#define OP_IFEQ            0x99
#define OP_IFNE            0x9a
#define OP_IFLT            0x9b
#define OP_IFGE            0x9c
#define OP_IFGT            0x9d
#define OP_IFLE            0x9e
#define OP_IF_ICMPEQ       0x9f
#define OP_IF_ICMPNE       0xa0
#define OP_IF_ICMPLT       0xa1
#define OP_IF_ICMPGE       0xa2
#define OP_IF_ICMPGT       0xa3
#define OP_IF_ICMPLE       0xa4
#define OP_IF_ACMPEQ       0xa5
#define OP_IF_ACMPNE       0xa6
#define OP_GOTO            0xa7
#define OP_JSR             0xa8
#define OP_RET             0xa9
#define OP_TABLESWITCH     0xaa
#define OP_LOOKUPSWITCH    0xab
#define OP_IRETURN         0xac
#define OP_LRETURN         0xad
#define OP_FRETURN         0xae
#define OP_DRETURN         0xaf
#define OP_ARETURN         0xb0
#define OP_RETURN          0xb1
#define OP_GETSTATIC       0xb2
#define OP_PUTSTATIC       0xb3
#define OP_GETFIELD        0xb4
#define OP_PUTFIELD        0xb5
#define OP_INVOKEVIRTUAL   0xb6
#define OP_INVOKESPECIAL   0xb7
#define OP_INVOKESTATIC    0xb8
#define OP_INVOKEINTERFACE 0xb9
#define OP_INVOKEDYNAMIC   0xba
#define OP_NEW             0xbb
#define OP_NEWARRAY        0xbc
#define OP_ANEWARRAY       0xbd
#define OP_ARRAYLENGTH     0xbe
#define OP_ATHROW          0xbf
#define OP_CHECKCAST       0xc0
#define OP_INSTANCEOF      0xc1
#define OP_MONITORENTER    0xc2
#define OP_MONITOREXIT     0xc3
#define OP_WIDE            0xc4
#define OP_MULTIANEWARRAY  0xc5
#define OP_IFNULL          0xc6
#define OP_IFNONNULL       0xc7
#define OP_GOTO_W          0xc8
#define OP_JSR_W           0xc9
#define OP_BREAKPOINT      0xca
#define OP_RESERVED_CB     0xcb
#define OP_RESERVED_CC     0xcc
#define OP_RESERVED_CD     0xcd
#define OP_RESERVED_CE     0xce
#define OP_RESERVED_CF     0xcf
#define OP_RESERVED_D0     0xd0
#define OP_RESERVED_D1     0xd1
#define OP_RESERVED_D2     0xd2
#define OP_RESERVED_D3     0xd3
#define OP_RESERVED_D4     0xd4
#define OP_RESERVED_D5     0xd5
#define OP_RESERVED_D6     0xd6
#define OP_RESERVED_D7     0xd7
#define OP_RESERVED_D8     0xd8
#define OP_RESERVED_D9     0xd9
#define OP_RESERVED_DA     0xda
#define OP_RESERVED_DB     0xdb
#define OP_RESERVED_DC     0xdc
#define OP_RESERVED_DD     0xdd
#define OP_RESERVED_DE     0xde
#define OP_RESERVED_DF     0xdf
#define OP_RESERVED_E0     0xe0
#define OP_RESERVED_E1     0xe1
#define OP_RESERVED_E2     0xe2
#define OP_RESERVED_E3     0xe3
#define OP_RESERVED_E4     0xe4
#define OP_RESERVED_E5     0xe5
#define OP_RESERVED_E6     0xe6
#define OP_RESERVED_E7     0xe7
#define OP_RESERVED_E8     0xe8
#define OP_RESERVED_E9     0xe9
#define OP_RESERVED_EA     0xea
#define OP_RESERVED_EB     0xeb
#define OP_RESERVED_EC     0xec
#define OP_RESERVED_ED     0xed
#define OP_RESERVED_EE     0xee
#define OP_RESERVED_EF     0xef
#define OP_RESERVED_F0     0xf0
#define OP_RESERVED_F1     0xf1
#define OP_RESERVED_F2     0xf2
#define OP_RESERVED_F3     0xf3
#define OP_RESERVED_F4     0xf4
#define OP_RESERVED_F5     0xf5
#define OP_RESERVED_F6     0xf6
#define OP_RESERVED_F7     0xf7
#define OP_RESERVED_F8     0xf8
#define OP_RESERVED_F9     0xf9
#define OP_RESERVED_FA     0xfa
#define OP_RESERVED_FB     0xfb
#define OP_RESERVED_FC     0xfc
#define OP_RESERVED_FD     0xfd
#define OP_IMPDEP1         0xfe
#define OP_IMPDEP2         0xff

#define ATYPE_BOOLEAN  4
#define ATYPE_CHAR     5
#define ATYPE_FLOAT    6
#define ATYPE_DOUBLE   7
#define ATYPE_BYTE     8
#define ATYPE_SHORT    9
#define ATYPE_INT     10
#define ATYPE_LONG    11

extern const char* OpcodeNames[256];

typedef struct
{
	uint8_t opcode;
	union
	{
		uint8_t uint8;
		uint16_t uint16;

		struct
		{
			uint16_t constant; /* Constant Pool index */
			char dimensions;   /* for OP_MULTIANEWARRAY */
		};

		struct
		{
			char varIndex; /* local variable */
			char value;    /* for OP_IINC */
		};

		int16_t branchoffset;
		int32_t branchoffset32; /* for OP_GOTO_W and OP_JSR_W */

		struct /* for OP_WIDE */
		{
			uint8_t opcode2;
			uint16_t varIndex16;
			uint16_t value16;
		};

		struct /* for OP_TABLESWITCH and OP_LOOKUPSWITCH */
		{
			int32_t defaultoffset;
			union
			{
				int32_t low;     /* OP_TABLESWITCH */
				uint32_t npairs; /* OP_LOOKUPSWITCH */
			};
			int32_t high;
			int32_t* matches; /* OP_LOOKUPSWITCH */
			int32_t* branchoffsets;
		};
	};
} Instruction;

void dump_code_attribute(FILE* fp, ClassFile* classFile, Attribute* attribute);
int instruction_to_string(ClassFile* classFile, Instruction* ins, uint32_t pc, int bufsize, char* buf);
uint32_t instruction_to_bytecode(Instruction* ins, unsigned char* code, uint32_t pc);
uint32_t get_single_instruction(unsigned char* code, Instruction* ins, uint32_t pc);
void free_single_instruction(Instruction* ins);

#endif
