all: hello disas dis6800 jump exit hello2 ex2 ex1

TOOLSET := /opt/arm-2010.09/bin
TARGET  := arm-none-eabi-

CROSS := $(TOOLSET)/$(TARGET)

AS = $(CROSS)gcc -c
CC = $(CROSS)gcc
CXX= $(CROSS)g++

CFLAGS = -O2 -mcpu=arm7tdmi -Wall -I../include
LDFLAGS = $(CFLAGS) -Tarmelf.xbn -pie -N

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

hello2: crt0.o hello2.o
	$(CXX) $(LDFLAGS) -o $@ $^ ../src/syscalls.c

ex2: crt0.o ex2.o
	$(CXX) $(LDFLAGS) -o $@ $^ ../src/syscalls.c

ex1: crt0.o ex1.o
	$(CXX) $(LDFLAGS) -o $@ $^ ../src/syscalls.c

clean:
	rm -f *.o disas hello dis6800 test jump exit hello2 ex2 ex1
