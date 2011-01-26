TEST_TARGET = tffs
TSH_TARGET  = tsh

all: $(TEST_TARGET) $(TSH_TARGET)

CFLAGS = -I../inc -I../src -Wall -g -O3

LIBSRC  = ../src/initfs.c ../src/debug.c ../src/fat.c ../src/dir.c ../src/dirent.c ../src/common.c ../src/file.c ../src/crtdef.c ../src/cache.c
TESTSRC = main.c hai_file.c
TSHSRC  = tsh.c hai_file.c

LIBOBJ  =$(LIBSRC:.c=.o)
TESTOBJ =$(TESTSRC:.c=.o)
TSHOBJ  =$(TSHSRC:.c=.o)

$(TEST_TARGET): $(LIBOBJ) $(TESTOBJ)
	$(CC) -o $@ $^

$(TSH_TARGET): $(LIBOBJ) $(TSHOBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(LIBOBJ) $(TSHOBJ) $(TESTOBJ) $(TEST_TARGET) $(TSH_TARGET)
