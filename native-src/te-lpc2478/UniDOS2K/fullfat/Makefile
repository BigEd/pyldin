STATICLIB = lib/libfullfat.a
TESTAPP = test/testfat

all: $(STATICLIB) $(TESTAPP)

CFLAGS = -O2 -Wall -Isrc

SRC = src/ff_blk.c  src/ff_dir.c    src/ff_fat.c   src/ff_format.c  src/ff_ioman.c   src/ff_safety.c  src/ff_time.c \
    src/ff_crc.c  src/ff_error.c  src/ff_file.c  src/ff_hash.c    src/ff_memory.c  src/ff_string.c  src/ff_unicode.c

OBJS := $(SRC:.c=.o)

$(STATICLIB): $(OBJS)
	$(AR) rc $@ $^


TESTOBJS = test/main.o test/blkdev_mci.o

$(TESTAPP): $(TESTOBJS) $(OBJS)
	$(CC) -o $@ $^

clean:
	rm -f $(OBJS) $(STATICLIB) $(TESTAPP) $(TESTOBJS)

