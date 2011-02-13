all: hello disas dis6800 jump exit hello2 ex2 ex1 conin fileio

TOOLSET := /opt/arm-2010.09/bin
TARGET  := arm-none-eabi-

CROSS := $(TOOLSET)/$(TARGET)

AS = $(CROSS)gcc -c
CC = $(CROSS)gcc
CXX= $(CROSS)g++

CFLAGS = -O2 -mcpu=arm7tdmi -Wall -I../include
LDFLAGS = $(CFLAGS) -Tarmelf.xbn -pie -N

../src/syscalls.o: ../src/syscalls.c

hello: crt0.o hello.o ../src/syscalls.o
	$(CC) $(LDFLAGS) -o $@ $^

disas: crt0.o disas.o ../src/syscalls.o
	$(CC) $(LDFLAGS) -o $@ $^

dis6800: crt0.o dis6800.o ../src/syscalls.o
	$(CC) $(LDFLAGS) -o $@ $^

jump: crt0.o jump.o ../src/syscalls.o
	$(CC) $(LDFLAGS) -o $@ $^

exit: crt0.o exit.o ../src/syscalls.o
	$(CC) $(LDFLAGS) -o $@ $^

hello2: crt0.o hello2.o ../src/syscalls.o
	$(CXX) $(LDFLAGS) -o $@ $^

ex2: crt0.o ex2.o ../src/syscalls.o
	$(CXX) $(LDFLAGS) -o $@ $^

ex1: crt0.o ex1.o ../src/syscalls.o
	$(CXX) $(LDFLAGS) -o $@ $^

conin: crt0.o conin.o ../src/syscalls.o
	$(CC) $(LDFLAGS) -o $@ $^

fileio: crt0.o fileio.o ../src/syscalls.o
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o disas hello dis6800 test jump exit hello2 ex2 ex1 conin fileio
