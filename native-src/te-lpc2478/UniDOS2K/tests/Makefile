all: hello disas dis6800 jump exit

TOOLSET := /opt/arm-2010.09/bin
TARGET  := arm-none-eabi-

CROSS := $(TOOLSET)/$(TARGET)

AS = $(CROSS)gcc -c
CC = $(CROSS)gcc

CFLAGS = -O2 -mcpu=arm7tdmi -Wall -I../include
LDFLAGS = $(CFLAGS) -Tarmelf.xn

hello: crt0.o hello.o
	$(CC) $(LDFLAGS) -o $@ $^ ../src/syscalls.c

disas: crt0.o disas.o
	$(CC) $(LDFLAGS) -o $@ $^ ../src/syscalls.c

dis6800: crt0.o dis6800.o
	$(CC) $(LDFLAGS) -o $@ $^ ../src/syscalls.c

jump: crt0.o jump.o
	$(CC) $(LDFLAGS) -o $@ $^ ../src/syscalls.c

exit: crt0.o exit.o
	$(CC) $(LDFLAGS) -o $@ $^ ../src/syscalls.c

clean:
	rm -f *.o disas hello dis6800 test jump exit
