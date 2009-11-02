TARGET=as6800

PREFIX=/usr
BINDIR=$(PREFIX)/bin

CC=gcc
CFLAGS=-O2
LDFLAGS=

OBJS=main.o

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

install:
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)

