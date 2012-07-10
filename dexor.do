DEPS="classfile.o bytecode.o util.o utf8.o dexor.o"
LDFLAGS=""

redo-ifchange $DEPS

g++ -g -Wall -o $3 $DEPS $LDFLAGS
