TARGET		 = libcurses.a

TOOLSET		:= /opt/arm-2010.09/bin
TOOLCHAIN	:= arm-none-eabi-

CROSS		:= $(TOOLSET)/$(TOOLCHAIN)

AS		:= $(CROSS)gcc -c
CC		:= $(CROSS)gcc
AR		:= $(CROSS)ar
RANLIB		:= $(CROSS)ranlib

all: $(TARGET)

CFLAGS		 = -Ipdcurses -I.
CFLAGS		+= -Os -mcpu=arm7tdmi -Wall -I../../include

OBJ = pdcurses/addch.o pdcurses/addchstr.o pdcurses/addstr.o pdcurses/attr.o pdcurses/beep.o pdcurses/bkgd.o \
    pdcurses/border.o pdcurses/clear.o pdcurses/color.o pdcurses/debug.o pdcurses/delch.o pdcurses/deleteln.o \
    pdcurses/deprec.o pdcurses/getch.o pdcurses/getstr.o pdcurses/getyx.o pdcurses/inch.o pdcurses/inchstr.o \
    pdcurses/initscr.o pdcurses/inopts.o pdcurses/insch.o pdcurses/insstr.o pdcurses/instr.o pdcurses/kernel.o \
    pdcurses/keyname.o pdcurses/mouse.o pdcurses/move.o pdcurses/outopts.o pdcurses/overlay.o pdcurses/pad.o \
    pdcurses/panel.o pdcurses/printw.o pdcurses/refresh.o pdcurses/scanw.o pdcurses/scr_dump.o pdcurses/scroll.o \
    pdcurses/slk.o pdcurses/termattr.o pdcurses/terminfo.o pdcurses/touch.o pdcurses/util.o pdcurses/window.o

#DEST_OBJ = ansi/pdcclip.o ansi/pdcdisp.o ansi/pdcgetsc.o ansi/pdckbd.o ansi/pdcscrn.o ansi/pdcsetsc.o ansi/pdcutil.o
DEST_OBJ = serial/serial.o

$(TARGET): $(OBJ) $(DEST_OBJ)
	$(AR) rc $@ $^
	$(RANLIB) $@

clean:
	rm -f $(OBJ) $(DEST_OBJ) $(TARGET)
