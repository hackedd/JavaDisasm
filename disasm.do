DEPS="classfile.o bytecode.o util.o disasm.o"
LDFLAGS=""

redo-ifchange $DEPS

g++ -g -Wall -o $3 $DEPS $LDFLAGS
