TARGET=dis6800

PREFIX=/usr
BINDIR=$(PREFIX)/bin

CC=gcc
CFLAGS=-O2 -Wall
LDFLAGS=

OBJS=dis6800.o

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

install:
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)

