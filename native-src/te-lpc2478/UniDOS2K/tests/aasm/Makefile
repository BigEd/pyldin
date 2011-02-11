all: aasm

TOOLSET := /opt/arm-2010.09/bin
TARGET  := arm-none-eabi-

CROSS := $(TOOLSET)/$(TARGET)

AS = $(CROSS)gcc -c
CC = $(CROSS)gcc

CFLAGS  = -O2
CFLAGS += -mcpu=arm7tdmi -mtune=arm7tdmi -I../../include
LDFLAGS = $(CFLAGS) -T../armelf.xbn -pie -N

OBJS = aasm.o

aasm: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ ../crt0.s $^ ../../src/syscalls.c

clean:
	rm -f $(OBJS) aasm
