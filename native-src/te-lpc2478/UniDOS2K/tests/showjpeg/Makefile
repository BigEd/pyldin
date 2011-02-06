all: showjpeg

TOOLSET := /opt/arm-2010.09/bin
TARGET  := arm-none-eabi-

CROSS := $(TOOLSET)/$(TARGET)

AS = $(CROSS)gcc -c
CC = $(CROSS)gcc

CFLAGS = -O3 -fomit-frame-pointer -ffast-math
CFLAGS += -DJPEG_FASTER_M211=0
CFLAGS += -mcpu=arm7tdmi -mtune=arm7tdmi -Wall -I../../include
LDFLAGS = $(CFLAGS) -T../armelf.xbn -pie -N

OBJS = main.o gba-jpeg-decode.o

showjpeg: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ ../crt0.s $^ ../../src/syscalls.c

clean:
	rm -f $(OBJS) showjpeg
