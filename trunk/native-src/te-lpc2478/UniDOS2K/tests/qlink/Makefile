all: qlink

TOOLSET := /opt/arm-2010.09/bin
TARGET  := arm-none-eabi-

CROSS := $(TOOLSET)/$(TARGET)

AS = $(CROSS)gcc -c
CC = $(CROSS)gcc

CFLAGS = -O2 -mcpu=arm7tdmi -Wall -I../../src -I../../startup
LDFLAGS = $(CFLAGS) -T../armelf.xn

OBJS = files.o hash.o join.o link.o mem.o output.o qlink.o riscos.o

qlink: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ ../crt0.s $^ ../../src/syscalls.c

clean:
	rm -f $(OBJS) qlink
