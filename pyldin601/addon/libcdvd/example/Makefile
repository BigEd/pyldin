EE_BIN = example.elf
EE_OBJS = main.o

EE_INCS += -I../common
EE_LDFLAGS += -L../lib
EE_LIBS += -lcdvdfs -ldebug

all: $(EE_BIN)

clean:
	rm -f *.elf *.o *.a

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
