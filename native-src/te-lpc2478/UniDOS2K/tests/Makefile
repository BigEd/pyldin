all: hello

hello:
	/opt/arm-2010.09/bin/arm-none-eabi-gcc -O2 -mcpu=arm7tdmi -I../startup crt0.s hello.c ../src/syscalls.c -o hello -Tarmelf.xn
