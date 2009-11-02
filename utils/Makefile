all: splitdisk splitrom

PREFIX=/usr
BINDIR=$(PREFIX)/bin

CFLAGS = -O2

install:
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 755 splitdisk $(DESTDIR)$(BINDIR)
	install -m 755 splitrom  $(DESTDIR)$(BINDIR)

clean:
	rm -f splitdisk splitrom
