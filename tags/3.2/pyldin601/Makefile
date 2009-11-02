TARGET = pyldin
PREFIX = /usr
BINDIR = $(PREFIX)/bin
DATADIR= $(PREFIX)/share

UNIBIOSONLY=no

VERSION=3.2
EXTRAVERSION	:= $(shell \
			if [ -d .svn ]; then \
			    printf 'rev' ; \
			    unset LC_ALL LANG ; svn info | grep Revision | cut -f2 -d' ' ; \
			fi \
		    )

all:	$(TARGET)

CC := gcc
CXX := g++
OPT_FLAGS := -O3 -fomit-frame-pointer
#OPT_STRIP := -s
CFLAGS = -Wall $(OPT_FLAGS) -fno-exceptions -Isrc/core -Isrc `sdl-config --cflags` `pkg-config libpng --cflags` $(EXTRA_CFLAGS)
CFLAGS += -DDATADIR=\"$(DATADIR)/pyldin\" -DVERSION=\"$(VERSION)$(EXTRAVERSION)\"
CXXFLAGS = $(CFLAGS) -fno-rtti
LDFLAGS = $(OPT_LIBS) `sdl-config --libs` `pkg-config libpng --libs` -lz $(OPT_STRIP)

##########################
CFLAGS += -DPYLDIN_LOGO -DPYLDIN_ICON
CXXFLAGS += -DPYLDIN_LOGO -DPYLDIN_ICON
LDFLAGS += -lSDL_image -ljpeg -lpng
##########################

OBJS  = src/core/floppy.o src/core/virtkbd.o src/core/mc6800.o 
OBJS += src/wave.o src/screen.o src/discs.o src/sshot.o src/printer.o src/floppymanager.o src/pyldin.o

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(CXXFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

install:
	mkdir -p $(DESTDIR)$(DATADIR)/pyldin/Rom
	mkdir -p $(DESTDIR)$(DATADIR)/pyldin/Floppy
	mkdir -p $(DESTDIR)$(DATADIR)/pyldin/Bios	
	cp -f ../native/Bios/*.roz			$(DESTDIR)$(DATADIR)/pyldin/Bios/
	cp -f ../native/Floppy/*.imz			$(DESTDIR)$(DATADIR)/pyldin/Floppy/
ifeq (yes, $(UNIBIOSONLY))
	cp -f ../native/Rom/str*.roz			$(DESTDIR)$(DATADIR)/pyldin/Rom/
else
	cp -f ../native/RAMROMDiskPipnet/rom*.roz	$(DESTDIR)$(DATADIR)/pyldin/Rom/
endif
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET)      $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)$(DATADIR)/applications
	mkdir -p $(DESTDIR)$(DATADIR)/pixmaps
	install -m 644 pyldin.desktop $(DESTDIR)$(DATADIR)/applications
	install -m 644 pyldin.png     $(DESTDIR)$(DATADIR)/pixmaps
