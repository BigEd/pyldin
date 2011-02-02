all: qas

TOOLSET := /opt/arm-2010.09/bin
TARGET  := arm-none-eabi-

CROSS := $(TOOLSET)/$(TARGET)

AS = $(CROSS)gcc -c
CC = $(CROSS)gcc

CFLAGS = -O2 -mcpu=arm7tdmi -Wall -I../../include
LDFLAGS = $(CFLAGS) -T../armelf.xn

OBJS   = a_cp.o a_norm.o bits.o eval.o instr.o operate.o \
	 parse.o riscos.o a_fp.o assm.o egettok.o input.o \
	 mem.o output.o qas.o

qas: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ ../crt0.s $^ ../../src/syscalls.c

clean:
	rm -f $(OBJS) qas
