#include "bytecode.h"

const char* OpcodeNames[256] = {
	"nop",
	"aconst_null",
	"iconst_m1", "iconst_0", "iconst_1", "iconst_2", "iconst_3", "iconst_4", "iconst_5",
	"lconst_0", "lconst_1",
	"fconst_0", "fconst_1", "fconst_2",
	"dconst_0", "dconst_1",
	"bipush", "sipush",
	"ldc", "ldc_w", "ldc2_w",
	"iload", "lload", "fload", "dload", "aload",
	"iload_0", "iload_1", "iload_2", "iload_3",
	"lload_0", "lload_1", "lload_2", "lload_3",
	"fload_0", "fload_1", "fload_2", "fload_3",
	"dload_0", "dload_1", "dload_2", "dload_3",
	"aload_0", "aload_1", "aload_2", "aload_3",
	"iaload", "laload", "faload", "daload", "aaload", "baload", "caload", "saload",
	"istore", "lstore", "fstore", "dstore", "astore",
	"istore_0", "istore_1", "istore_2", "istore_3",
	"lstore_0", "lstore_1", "lstore_2", "lstore_3",
	"fstore_0", "fstore_1", "fstore_2", "fstore_3",
	"dstore_0", "dstore_1", "dstore_2", "dstore_3",
	"astore_0", "astore_1", "astore_2", "astore_3",
	"iastore", "lastore", "fastore", "dastore", "aastore", "bastore", "castore", "sastore",
	"pop", "pop2",
	"dup", "dup_x1", "dup_x2",
	"dup2", "dup2_x1", "dup2_x2",
	"swap",
	"iadd", "ladd", "fadd", "dadd",
	"isub", "lsub", "fsub", "dsub",
	"imul", "lmul", "fmul", "dmul",
	"idiv", "ldiv", "fdiv", "ddiv",
	"irem", "lrem", "frem", "drem",
	"ineg", "lneg", "fneg", "dneg",
	"ishl", "lshl", "ishr", "lshr",
	"iushr", "lushr",
	"iand", "land",
	"ior", "lor",
	"ixor", "lxor",
	"iinc",
	"i2l", "i2f", "i2d",
	"l2i", "l2f", "l2d",
	"f2i", "f2l", "f2d",
	"d2i", "d2l", "d2f",
	"i2b", "i2c", "i2s",
	"lcmp", "fcmpl", "fcmpg", "dcmpl", "dcmpg",
	"ifeq", "ifne", "iflt", "ifge", "ifgt", "ifle",
	"if_icmpeq", "if_icmpne", "if_icmplt", "if_icmpge", "if_icmpgt", "if_icmple", "if_acmpeq", "if_acmpne",
	"goto", "jsr", "ret",
	"tableswitch", "lookupswitch",
	"ireturn", "lreturn", "freturn", "dreturn", "areturn", "return",
	"getstatic", "putstatic",
	"getfield", "putfield",
	"invokevirtual", "invokespecial", "invokestatic", "invokeinterface", "invokedynamic",
	"new", "newarray", "anewarray",
	"arraylength",
	"athrow",
	"checkcast",
	"instanceof",
	"monitorenter", "monitorexit",
	"wide",
	"multianewarray",
	"ifnull", "ifnonnull",
	"goto_w",
	"jsr_w",
	"breakpoint",
	"reserved_cb", "reserved_cc", "reserved_cd", "reserved_ce", "reserved_cf",
	"reserved_d0", "reserved_d1", "reserved_d2", "reserved_d3", "reserved_d4", "reserved_d5", "reserved_d6", "reserved_d7",
	"reserved_d8", "reserved_d9", "reserved_da", "reserved_db", "reserved_dc", "reserved_dd", "reserved_de", "reserved_df",
	"reserved_e0", "reserved_e1", "reserved_e2", "reserved_e3", "reserved_e4", "reserved_e5", "reserved_e6", "reserved_e7",
	"reserved_e8", "reserved_e9", "reserved_ea", "reserved_eb", "reserved_ec", "reserved_ed", "reserved_ee", "reserved_ef",
	"reserved_f0", "reserved_f1", "reserved_f2", "reserved_f3", "reserved_f4", "reserved_f5", "reserved_f6", "reserved_f7",
	"reserved_f8", "reserved_f9", "reserved_fa", "reserved_fb", "reserved_fc", "reserved_fd",
	"impdep1",
	"impdep2",
};

const char* ArrayTypeNames[] = {
	NULL, NULL, NULL, NULL,
	"boolean", "char", "float", "double",
	"byte", "short", "int", "long",
};

typedef struct
{
	uint32_t pc;
	uint32_t dest;
} Branch;

#define MAX_BRANCHES 500

uint32_t get_single_instruction(unsigned char* code, Instruction* ins, uint32_t pc)
{
	int offset, i;

	ins->opcode = code[0];
	switch (ins->opcode)
	{
		case OP_BIPUSH:
			ins->uint8 = code[1];
			return 2;

		case OP_SIPUSH:
			ins->uint16 = be16toh(*(uint16_t*)(code + 1));
			return 3;

		case OP_LDC:
			ins->constant = code[1];
			return 2;

		case OP_ILOAD:
		case OP_LLOAD:
		case OP_FLOAD:
		case OP_DLOAD:
		case OP_ALOAD:
		case OP_ISTORE:
		case OP_LSTORE:
		case OP_FSTORE:
		case OP_DSTORE:
		case OP_ASTORE:
		case OP_RET:
			ins->varIndex = code[1];
			return 2;

		case OP_IINC:
			ins->varIndex = code[1];
			ins->value = code[2];
			return 3;

		case OP_IFEQ:
		case OP_IFNE:
		case OP_IFLT:
		case OP_IFGE:
		case OP_IFGT:
		case OP_IFLE:
		case OP_IF_ICMPEQ:
		case OP_IF_ICMPNE:
		case OP_IF_ICMPLT:
		case OP_IF_ICMPGE:
		case OP_IF_ICMPGT:
		case OP_IF_ICMPLE:
		case OP_IF_ACMPEQ:
		case OP_IF_ACMPNE:
		case OP_GOTO:
		case OP_JSR:
		case OP_IFNULL:
		case OP_IFNONNULL:
			ins->branchoffset = be16toh(*(int16_t*)(code + 1));
			return 3;

		case OP_TABLESWITCH:
			offset = 4 - (pc % 4); // offset is 32bit aligned from pc
			ins->defaultoffset = be32toh(*(int32_t*)(code + offset));
			offset += 4;
			ins->low = be32toh(*(int32_t*)(code + offset));
			offset += 4;
			ins->high = be32toh(*(int32_t*)(code + offset));
			offset += 4;

			ins->branchoffsets = malloc((ins->high - ins->low + 1) * sizeof(int32_t));
			for (i = 0; i <= ins->high - ins->low; i += 1)
			{
				ins->branchoffsets[i] = be32toh(*(int32_t*)(code + offset));
				offset += 4;
			}

			return offset;

		case OP_LOOKUPSWITCH:
			offset = 4 - (pc % 4); // offset is 32bit aligned from pc
			ins->defaultoffset = be32toh(*(int32_t*)(code + offset));
			offset += 4;
			ins->npairs = be32toh(*(uint32_t*)(code + offset));
			offset += 4;

			ins->matches = malloc(ins->npairs * sizeof(int32_t));
			ins->branchoffsets = malloc(ins->npairs * sizeof(int32_t));

			for (i = 0; i < ins->npairs; i += 1)
			{
				ins->matches[i] = be32toh(*(int32_t*)(code + offset));
				offset += 4;
				ins->branchoffsets[i] = be32toh(*(int32_t*)(code + offset));
				offset += 4;
			}

			return offset;

		case OP_LDC_W:
		case OP_LDC2_W:
		case OP_GETSTATIC:
		case OP_PUTSTATIC:
		case OP_GETFIELD:
		case OP_PUTFIELD:
		case OP_INVOKEVIRTUAL:
		case OP_INVOKESPECIAL:
		case OP_INVOKESTATIC:
		case OP_NEW:
		case OP_ANEWARRAY:
		case OP_CHECKCAST:
		case OP_INSTANCEOF:
			ins->constant = be16toh(*(uint16_t*)(code + 1));
			return 3;

		case OP_INVOKEINTERFACE:
		case OP_INVOKEDYNAMIC:
			ins->constant = be16toh(*(uint16_t*)(code + 1));
			/* XXX: Skip two extra bytes */
			return 5;

		case OP_NEWARRAY:
			/* TODO: Should have it's own field? */
			ins->uint8 = code[1];
			return 2;

		case OP_WIDE:
			ins->opcode2 = code[1];
			ins->varIndex16 = be16toh(*(uint16_t*)(code + 2));

			if (ins->opcode2 != OP_IINC)
				return 4;

			ins->value16 = be16toh(*(uint16_t*)(code + 4));
			return 6;

		case OP_MULTIANEWARRAY:
			ins->constant = be16toh(*(uint16_t*)(code + 1));
			ins->dimensions = code[3];
			return 4;

		case OP_GOTO_W:
		case OP_JSR_W:
			ins->branchoffset32 = be32toh(*(int32_t*)(code + 1));
			return 5;

		default:
			return 1;
	}
}

void free_single_instruction(Instruction* ins)
{
	if (ins->opcode == OP_TABLESWITCH)
	{
		free(ins->branchoffsets);
	}
	else if (ins->opcode == OP_LOOKUPSWITCH)
	{
		free(ins->matches);
		free(ins->branchoffsets);
	}
}

int instruction_to_string(ClassFile* classFile, Instruction* ins, uint32_t pc, int bufsize, char* buf)
{
	int i, outsize;
	char casebuf[128];
	Constant* c;

	/* Longest opcode = 15 chars */
	if (bufsize < 16) return 16;

	strcpy(buf, OpcodeNames[ins->opcode]);
	outsize = strlen(buf);
	buf += outsize;

	switch (ins->opcode)
	{
		case OP_BIPUSH:
			outsize += snprintf(buf, bufsize - outsize, " %hhu", ins->uint8);
			break;

		case OP_SIPUSH:
			outsize += snprintf(buf, bufsize - outsize, " %hu", ins->uint16);
			break;

		case OP_LDC:
		case OP_LDC_W:
		case OP_LDC2_W:
		case OP_GETSTATIC:
		case OP_PUTSTATIC:
		case OP_GETFIELD:
		case OP_PUTFIELD:
		case OP_INVOKEVIRTUAL:
		case OP_INVOKESPECIAL:
		case OP_INVOKESTATIC:
		case OP_INVOKEINTERFACE:
		case OP_INVOKEDYNAMIC:
		case OP_NEW:
		case OP_ANEWARRAY:
		case OP_CHECKCAST:
		case OP_INSTANCEOF:
			c = find_constant(classFile, ins->constant);
			outsize += snprintf(buf, bufsize - outsize, " #%hu // %s", ins->constant, constant_to_string(classFile, c));
			break;

		case OP_ILOAD:
		case OP_LLOAD:
		case OP_FLOAD:
		case OP_DLOAD:
		case OP_ALOAD:
		case OP_ISTORE:
		case OP_LSTORE:
		case OP_FSTORE:
		case OP_DSTORE:
		case OP_ASTORE:
			outsize += snprintf(buf, bufsize - outsize, " %hhu", ins->varIndex);
			break;

		case OP_IINC:
			outsize += snprintf(buf, bufsize - outsize, " %hhu, %hhd", ins->varIndex, ins->value);
			break;

		case OP_IFEQ:
		case OP_IFNE:
		case OP_IFLT:
		case OP_IFGE:
		case OP_IFGT:
		case OP_IFLE:
		case OP_IF_ICMPEQ:
		case OP_IF_ICMPNE:
		case OP_IF_ICMPLT:
		case OP_IF_ICMPGE:
		case OP_IF_ICMPGT:
		case OP_IF_ICMPLE:
		case OP_IF_ACMPEQ:
		case OP_IF_ACMPNE:
		case OP_GOTO:
		case OP_JSR:
		case OP_IFNULL:
		case OP_IFNONNULL:
			outsize += snprintf(buf, bufsize - outsize, " %hd", pc + ins->branchoffset);
			break;

		case OP_GOTO_W:
		case OP_JSR_W:
			outsize += snprintf(buf, bufsize - outsize, " %d", pc + ins->branchoffset32);
			break;

		case OP_RET:
			outsize += snprintf(buf, bufsize - outsize, " #%hhu", ins->varIndex);
			break;

		case OP_NEWARRAY:
			if (ins->uint8 >= ATYPE_BOOLEAN && ins->uint8 <= ATYPE_LONG)
				outsize += snprintf(buf, bufsize - outsize, " %s", ArrayTypeNames[ins->uint8]);
			break;

		case OP_MULTIANEWARRAY:
			c = find_constant(classFile, ins->constant);
			outsize += snprintf(buf, bufsize - outsize, " #%hu, %hhu // %s", ins->constant, ins->dimensions, constant_to_string(classFile, c));
			break;

		case OP_WIDE:
			if (ins->opcode2 == OP_IINC)
				outsize += snprintf(buf, bufsize - outsize, " %s %hu %hd", OpcodeNames[ins->opcode2], ins->varIndex16, ins->value16);
			else
				outsize += snprintf(buf, bufsize - outsize, " %s %hu", OpcodeNames[ins->opcode2], ins->varIndex16);
			break;

#define _append(string)                     \
	if (outsize + strlen(string) < bufsize) \
		strcat(buf, string);                \
	outsize += strlen(string);

		case OP_TABLESWITCH:
			_append("\n        {\n");
			for (i = 0; i <= ins->high - ins->low; i += 1)
			{
				sprintf(casebuf, "%d: %d\n", ins->low + i, pc + ins->branchoffsets[i]);
				_append("            ");
				_append(casebuf);
			}
			sprintf(casebuf, "default: %d\n", pc + ins->defaultoffset);
			_append("            ");
			_append(casebuf);
			_append("        }");
			break;

		case OP_LOOKUPSWITCH:
			_append("\n        {\n");
			for (i = 0; i < ins->npairs; i += 1)
			{
				sprintf(casebuf, "%d: %d\n", ins->matches[i], pc + ins->branchoffsets[i]);
				_append("            ");
				_append(casebuf);
			}
			sprintf(casebuf, "default: %d\n", pc + ins->defaultoffset);
			_append("            ");
			_append(casebuf);
			_append("        }");
			break;

#undef _append

	}

	return outsize;
}

uint32_t instruction_to_bytecode(Instruction* ins, unsigned char* code, uint32_t pc)
{
	int offset, i;

	code[0] = ins->opcode;
	switch (ins->opcode)
	{
		case OP_BIPUSH:
			code[1] = ins->uint8;
			return 2;

		case OP_SIPUSH:
			*(uint16_t*)(code + 1) = htobe16(ins->uint16);
			return 3;

		case OP_LDC:
			code[1] = ins->constant;
			return 2;

		case OP_LDC_W:
		case OP_LDC2_W:
		case OP_GETSTATIC:
		case OP_PUTSTATIC:
		case OP_GETFIELD:
		case OP_PUTFIELD:
		case OP_INVOKEVIRTUAL:
		case OP_INVOKESPECIAL:
		case OP_INVOKESTATIC:
		case OP_NEW:
		case OP_ANEWARRAY:
		case OP_CHECKCAST:
		case OP_INSTANCEOF:
			*(uint16_t*)(code + 1) = htobe16(ins->constant);
			return 3;

		case OP_INVOKEINTERFACE:
		case OP_INVOKEDYNAMIC:
			*(uint16_t*)(code + 1) = htobe16(ins->constant);
			code[3] = '\0';
			code[4] = '\0';
			return 5;

		case OP_ILOAD:
		case OP_LLOAD:
		case OP_FLOAD:
		case OP_DLOAD:
		case OP_ALOAD:
		case OP_ISTORE:
		case OP_LSTORE:
		case OP_FSTORE:
		case OP_DSTORE:
		case OP_ASTORE:
		case OP_RET:
			code[1] = ins->varIndex;
			return 2;

		case OP_IINC:
			code[1] = ins->varIndex;
			code[2] = ins->value;
			return 3;

		case OP_IFEQ:
		case OP_IFNE:
		case OP_IFLT:
		case OP_IFGE:
		case OP_IFGT:
		case OP_IFLE:
		case OP_IF_ICMPEQ:
		case OP_IF_ICMPNE:
		case OP_IF_ICMPLT:
		case OP_IF_ICMPGE:
		case OP_IF_ICMPGT:
		case OP_IF_ICMPLE:
		case OP_IF_ACMPEQ:
		case OP_IF_ACMPNE:
		case OP_GOTO:
		case OP_JSR:
		case OP_IFNULL:
		case OP_IFNONNULL:
			*(uint16_t*)(code + 1) = htobe16(ins->branchoffset);
			return 3;

		case OP_NEWARRAY:
			/* TODO: Should have it's own field? */
			code[1] = ins->uint8;
			return 2;

		case OP_WIDE:
			code[1] = ins->opcode2;
			*(uint16_t*)(code + 2) = htobe16(ins->varIndex16);

			if (ins->opcode2 != OP_IINC)
				return 4;

			*(uint16_t*)(code + 4) = htobe16(ins->value16);
			return 6;

		case OP_MULTIANEWARRAY:
			*(uint16_t*)(code + 1) = htobe16(ins->constant);
			code[3] = ins->dimensions;
			return 4;

		case OP_GOTO_W:
		case OP_JSR_W:
			*(int32_t*)(code + 1) = htobe32(ins->branchoffset32);
			return 5;

		case OP_TABLESWITCH:
			offset = 4 - (pc % 4); // offset is 32bit aligned from pc
			for (i = 1; i < offset; i += 1)
				code[i] = '\0';

			*(int32_t*)(code + offset) = htobe32(ins->defaultoffset);
			offset += 4;
			*(int32_t*)(code + offset) = htobe32(ins->low);
			offset += 4;
			*(int32_t*)(code + offset) = htobe32(ins->high);
			offset += 4;

			for (i = 0; i <= ins->high - ins->low; i += 1)
			{
				*(int32_t*)(code + offset) = htobe32(ins->branchoffsets[i]);
				offset += 4;
			}

			return offset;

		case OP_LOOKUPSWITCH:
			offset = 4 - (pc % 4); // offset is 32bit aligned from pc
			for (i = 1; i < offset; i += 1)
				code[i] = '\0';

			*(int32_t*)(code + offset) = htobe32(ins->defaultoffset);
			offset += 4;
			*(uint32_t*)(code + offset) = htobe32(ins->npairs);
			offset += 4;

			for (i = 0; i < ins->npairs; i += 1)
			{
				*(int32_t*)(code + offset) = htobe32(ins->matches[i]);
				offset += 4;
				*(int32_t*)(code + offset) = htobe32(ins->branchoffsets[i]);
				offset += 4;
			}

			return offset;

		default:
			return 1;
	}
}

void dump_code_attribute(FILE* fp, ClassFile* classFile, Attribute* attribute)
{
	uint32_t pc, size;
	Instruction ins;
	char insbuf[1024], label[128];
	Branch branches[MAX_BRANCHES], *branch = branches;
	int i, nins = 0, nbranch = 0, branchdest;

	for (pc = 0; pc < attribute->code.code_length; )
	{
		size = get_single_instruction(attribute->code.code + pc, &ins, pc);

		if (ins.opcode == OP_GOTO_W || ins.opcode == OP_JSR_W)
		{
			branch->pc = pc;
			branch->dest = pc + ins.branchoffset32;
			nbranch += 1;
			branch += 1;
		}
		else if ((ins.opcode >= OP_IFEQ && ins.opcode <= OP_JSR) || 
			ins.opcode == OP_IFNULL || ins.opcode == OP_IFNONNULL)
		{
			branch->pc = pc;
			branch->dest = pc + ins.branchoffset;
			nbranch += 1;
			branch += 1;
		}
		else if (ins.opcode == OP_TABLESWITCH)
		{
			for (i = 0; i <= ins.high - ins.low; i += 1)
			{
				branch->pc = pc;
				branch->dest = pc + ins.branchoffsets[i];
				nbranch += 1;
				branch += 1;
			}

			branch->pc = pc;
			branch->dest = pc + ins.defaultoffset;
			nbranch += 1;
			branch += 1;
		}
		else if (ins.opcode == OP_LOOKUPSWITCH)
		{
			for (i = 0; i < ins.npairs; i += 1)
			{
				branch->pc = pc;
				branch->dest = pc + ins.branchoffsets[i];
				nbranch += 1;
				branch += 1;
			}

			branch->pc = pc;
			branch->dest = pc + ins.defaultoffset;
			nbranch += 1;
			branch += 1;
		}

		if (nbranch >= MAX_BRANCHES)
		{
			fprintf(stderr, "Too many branches!\n");
			exit(1);
		}

		pc += size;
		nins += 1;
	}

	fprintf(fp, "        // Code Length: %d bytes / %d instructions\n", attribute->code.code_length, nins);
	fprintf(fp, "        // Max Stack: %hd, Max Locals: %hd, Attributes: %hd\n", attribute->code.max_stack, attribute->code.max_locals, attribute->code.attribute_count);
	fprintf(fp, "        // Branches: %d\n", nbranch);
	fprintf(fp, "\n");

	for (pc = 0; pc < attribute->code.code_length; )
	{
		size = get_single_instruction(attribute->code.code + pc, &ins, pc);

		if (instruction_to_string(classFile, &ins, pc, sizeof(insbuf), insbuf) >= sizeof(insbuf))
			strcpy(insbuf, "// Error: Unable to decode instruction");

		free_single_instruction(&ins);

		branchdest = 0;
		for (i = 0, branch = branches; i < nbranch; i += 1, branch += 1)
		{
			if (pc == branch->dest)
			{
				if (branchdest == 0)
				{
					sprintf(label, "%d:", pc);
					fprintf(fp, "%-7s // Branches from: %d", label, branch->pc);
				}
				else
					fprintf(fp, ", %d", branch->pc);
				branchdest += 1;
			}
		}
		if (branchdest)
			fprintf(fp, "\n");

		for (i = 0, branch = branches; i < nbranch; i += 1, branch += 1)
		{
			if (pc == branch->pc)
			{
				sprintf(label, "%d:", pc);
				fprintf(fp, "%-7s ", label);
				break;
			}
		}

		if (i >= nbranch)
			fprintf(fp, "        ");
		fprintf(fp, "%s\n", insbuf);

		pc += size;
	}
}