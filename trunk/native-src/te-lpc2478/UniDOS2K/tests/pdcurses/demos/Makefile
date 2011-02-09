TOOLSET		:= /opt/arm-2010.09/bin
TOOLCHAIN	:= arm-none-eabi-

CROSS		:= $(TOOLSET)/$(TOOLCHAIN)

AS		:= $(CROSS)gcc -c
CC		:= $(CROSS)gcc
AR		:= $(CROSS)ar
RANLIB		:= $(CROSS)ranlib

all: firework ptest rain testcurs tuidemo worm xmas newdemo

CFLAGS		 = -I..

CFLAGS		+= -O2 -mcpu=arm7tdmi -Wall -I../../../include
LDFLAGS		 = -T../../armelf.xbn -pie -N

firework: ../../crt0.o
	$(CC) $(CFLAGS) $@.c -o $@ $^ ../../../src/syscalls.c ../libcurses.a $(LDFLAGS)

ptest: ../../crt0.o
	$(CC) $(CFLAGS) $@.c -o $@ $^ ../../../src/syscalls.c ../libcurses.a $(LDFLAGS)

rain: ../../crt0.o
	$(CC) $(CFLAGS) $@.c -o $@ $^ ../../../src/syscalls.c ../libcurses.a $(LDFLAGS)

testcurs: ../../crt0.o
	$(CC) $(CFLAGS) $@.c -o $@ $^ ../../../src/syscalls.c ../libcurses.a $(LDFLAGS)

tuidemo: ../../crt0.o tui.o
	$(CC) $(CFLAGS) $@.c -o $@ $^ ../../../src/syscalls.c ../libcurses.a $(LDFLAGS)

worm: ../../crt0.o
	$(CC) $(CFLAGS) $@.c -o $@ $^ ../../../src/syscalls.c ../libcurses.a $(LDFLAGS)

xmas: ../../crt0.o
	$(CC) $(CFLAGS) $@.c -o $@ $^ ../../../src/syscalls.c ../libcurses.a $(LDFLAGS)

newdemo: ../../crt0.o
	$(CC) $(CFLAGS) $@.c -o $@ $^ ../../../src/syscalls.c ../libcurses.a $(LDFLAGS)

clean:
	rm -f *.o firework newdemo ptest rain testcurs tui tuidemo worm xmas ../../crt0.o
