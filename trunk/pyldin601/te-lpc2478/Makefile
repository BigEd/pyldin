#*****************************************************************
#
#  Makefile 
#
#  Based on Makefile from scmRTOS GCC port
#  Modified by Alexander Chukov <sash@pdaXrom.org>
#
#*****************************************************************

# vim: sw=8:ts=8:si:noexpandtab

#TOOLSET =  /usr/local/arm/arm-elf/bin
TOOLSET = /opt/iphone/arm-elf/bin

#TARGET = arm-elf-
TARGET = arm-elf-

PATH=$(TOOLSET):/usr/bin:/bin:/usr/local/bin

CC   = $(TARGET)gcc
CXX  = $(TARGET)g++
AS   = $(TARGET)gcc -x assembler-with-cpp
LD   = $(TARGET)g++
GDB  = $(TARGET)gdb
OBJCOPY = $(TARGET)objcopy
OBJDUMP = $(TARGET)objdump 
SIZE = $(TARGET)size 

RM = rm -f

CHIP = LPC2478
#CHIP = LPC2368
#CHIP = LPC2103

FMCU  = -mcpu=arm7tdmi

MEM = ROM

PROJECT = $(CHIP)_pyldin601

OPTIMIZE = -O3 -fomit-frame-pointer -fno-exceptions

##############################################################################################
# Application sources directories
SRC_DIRS = ./
SRC_DIRS += ./startup
SRC_DIRS += ./core
SRC_DIRS += ../src
SRC_DIRS += ../src/core

# Directories to search includes
INC_DIRS = ./include


# Directory ld script
LD_DIRS = ./ldscripts
LIB_DIRS += $(LD_DIRS)

##############################################################################################
# ROM files
ROMFILES = bios.lzma.h video.lzma.h rom0.lzma.h rom1.lzma.h rom2.lzma.h rom3.lzma.h rom4.lzma.h

##############################################################################################
#  ARM .c sources
SRC_ARM_C += ./core/core.c

SRC_ARM_C += ../src/screen.c

SRC_ARM_C += main.c
SRC_ARM_C += unlzma.c
SRC_ARM_C += uart.c
SRC_ARM_C += fio.c
SRC_ARM_C += irq.c
SRC_ARM_C += syscalls.c
SRC_ARM_C += timer.c
SRC_ARM_C += kbd.c
SRC_ARM_C += mci.c
SRC_ARM_C += diskovod.c
#SRC_ARM_C += adc.c
#SRC_ARM_C += touchscreen.c
#SRC_ARM_C +=
SRC_ARM_C += interrupts.c
SRC_ARM_C += system.c

############################
#  ARM .cpp sources
SRC_ARM_CPP =

############################
#  ARM .s sources
SRC_ARM_ASM =
SRC_ARM_ASM += crt.s

############################
# libraries
LIBS =

##############################################################################################
# predefined symbols for all sources
#DEFS = -DUSE_FULL_ASSERT=1    
DEFS =

# predefined symbols for .c sources
C_DEFS = 

# predefined symbols for .cpp sources
CPP_DEFS = 

# predefined symbols for .s sources
ASM_DEFS = 

LPC21ISP = lpc21isp
LPC21ISP_PORT = /dev/ttyUSB0
LPC21ISP_BAUD = 115200
LPC21ISP_XTAL = 12000
# other options:
# * verbose output: -debug
# * enter bootloader via RS232 DTR/RTS (only if hardware supports this
#   feature - see Philips AppNote): -control
LPC21ISP_OPTIONS = -control -verify
#LPC21ISP_OPTIONS += -debug

##############################################################################################
# Output files
ELF = $(BINDIR)/$(PROJECT).elf
HEX = $(BINDIR)/$(PROJECT).hex
BIN = $(BINDIR)/$(PROJECT).bin
LSS = $(LSTDIR)/$(PROJECT).lss

##############################################################################################
##############################################################################################
##############################################################################################


INC_DIRS += $(SRC_DIRS)

##############################################################################################
LD_SCRIPT= $(LD_DIRS)/$(CHIP)_$(MEM).ld

OBJDIR := ./obj
LSTDIR := ./list
BINDIR := ./bin
DEPDIR := $(OBJDIR)/.dep
#-----------------------------------------------
#    add surce files directories to search path
vpath
vpath % $(scmRTOS_DIRS)
vpath % $(SRC_DIRS)
vpath % $(LIB_DIRS)/src

OBJS_ARM_ASM  	= $(addprefix $(OBJDIR)/,$(notdir $(SRC_ARM_ASM:.s=.o) ))
OBJS_ARM_C  	= $(addprefix $(OBJDIR)/,$(notdir $(SRC_ARM_C:.c=.o) ))
OBJS_ARM_CPP  	= $(addprefix $(OBJDIR)/,$(notdir $(SRC_ARM_CPP:.cpp=.o) ))

OBJS_THUMB_ASM  = $(addprefix $(OBJDIR)/,$(notdir $(SRC_THUMB_ASM:.s=.o) ))
OBJS_THUMB_C  	= $(addprefix $(OBJDIR)/,$(notdir $(SRC_THUMB_C:.c=.o) ))
OBJS_THUMB_CPP  = $(addprefix $(OBJDIR)/,$(notdir $(SRC_THUMB_CPP:.cpp=.o) ))

OBJS = $(OBJS_ARM_ASM) $(OBJS_THUMB_ASM) $(OBJS_ARM_C) $(OBJS_THUMB_C) $(OBJS_ARM_CPP) $(OBJS_THUMB_CPP)

# defines
DEFS += -D$(CHIP)=1
DEFS += -DRUN_FROM_$(MEM)=1
C_DEFS += $(DEFS)
CPP_DEFS += $(DEFS)
ASM_DEFS += $(DEFS)
# Flags

FLAGS = $(FMCU) -DROMOPTIMIZE

#FLAGS += -DENABLE_INT17_EMULATOR
#FLAGS += -DDEBUG

# generate debug info in dwarf-2 format
FLAGS += -g -gdwarf-2
# generate dependency info  
FLAGS += -MD -MP -MF $(DEPDIR)/$(@F).d
# include search path
FLAGS += $(addprefix -I,$(INC_DIRS))

#if any THUMB mode sources, generate interworking code
ifneq ($(strip $(SRC_THUMB_ASM) $(SRC_THUMB_CPP)),)
FLAGS += -mthumb-interwork
endif

ASM_FLAGS = $(FLAGS)
ASM_FLAGS += $(ASM_DEFS)
ASM_FLAGS += -Wa,-amhlds=$(LSTDIR)/$(notdir $(<:.s=.lst)) 

C_FLAGS = $(FLAGS)
C_FLAGS += $(OPTIMIZE)
C_FLAGS += $(CPP_DEFS)
C_FLAGS += -fomit-frame-pointer
#C_FLAGS += -fno-exceptions
C_FLAGS += -Wall -Wextra -Wundef -Wcast-align
C_FLAGS += -Wa,-ahlmsdc=$(LSTDIR)/$(notdir $(<:.c=.lst)) -fverbose-asm
#C_FLAGS += -ffunction-sections -fdata-sections    # to remove dead code, if any, at link time
#C_FLAGS += -Winline -finline-limit=10             # generate warning if inlinign fails
#C_FLAGS += -funsigned-bitfields -fshort-enums	  # actually not important for scmRTOS
#C_FLAGS += -mthumb-interwork -D THUMB_INTERWORK


CPP_FLAGS = $(FLAGS)
CPP_FLAGS += $(OPTIMIZE)
CPP_FLAGS += $(CPP_DEFS)
CPP_FLAGS += -fomit-frame-pointer
CPP_FLAGS += -fno-exceptions -fno-rtti
CPP_FLAGS += -Wall -Wextra -Wundef -Wcast-align
CPP_FLAGS += -Weffc++ -fno-elide-constructors -Wctor-dtor-privacy
CPP_FLAGS += -Wa,-ahlmsdc=$(LSTDIR)/$(notdir $(<:.cpp=.lst)) -fverbose-asm
#CPP_FLAGS += -ffunction-sections -fdata-sections    # to remove dead code, if any, at link time
#CPP_FLAGS += -Winline -finline-limit=10             # generate warning if inlinign fails
#CPP_FLAGS += -funsigned-bitfields -fshort-enums	    # actually not important for scmRTOS


LD_FLAGS = $(FMCU)
LD_FLAGS += -nostartfiles
#LD_FLAGS += -mthumb
LD_FLAGS += -Xlinker -M
LD_FLAGS += -T$(LD_SCRIPT)
LD_FLAGS += -Wl,-Map=$(LSTDIR)/$(PROJECT).map,--cref,--no-warn-mismatch
LD_FLAGS += $(addprefix -L,$(LIB_DIRS))
LD_FLAGS += $(LIBDIR)
#LD_FLAGS += -Wl,--gc-sections # итеративно выбрасывает из объектников секции, на которые нет ссылок
# LD_FLAGS += -Wl,--defsym,__dso_handle=0


#
# makefile rules
#

#.SILENT :
.PHONY : all clean load

all: directories $(ROMFILES) $(OBJS) $(ELF)  $(LSS) size $(HEX) $(BIN)
#all: directories $(OBJS) $(ELF) $(LSS) size

####
video.lzma.h:
	zcat ../../native/Bios/video.roz | lzma -cz9 - | hexdump -v -e '1/1 " 0x%02x," "\n"' > $@

bios.lzma.h:
	zcat ../../native/Bios/bios.roz | lzma -cz9 - | hexdump -v -e '1/1 " 0x%02x," "\n"' > $@

rom0.lzma.h:
	zcat ../../native/RAMROMDiskPipnet/rom0.roz | lzma -cz9 - | hexdump -v -e '1/1 " 0x%02x," "\n"' > $@
rom1.lzma.h:
	zcat ../../native/RAMROMDiskPipnet/rom1.roz | lzma -cz9 - | hexdump -v -e '1/1 " 0x%02x," "\n"' > $@
rom2.lzma.h:
	zcat ../../native/RAMROMDiskPipnet/rom2.roz | lzma -cz9 - | hexdump -v -e '1/1 " 0x%02x," "\n"' > $@
rom3.lzma.h:
	zcat ../../native/RAMROMDiskPipnet/rom3.roz | lzma -cz9 - | hexdump -v -e '1/1 " 0x%02x," "\n"' > $@
rom4.lzma.h:
	zcat ../../native/RAMROMDiskPipnet/rom4.roz | lzma -cz9 - | hexdump -v -e '1/1 " 0x%02x," "\n"' > $@

####


#all objects depends on makefile
$(OBJS): Makefile

$(OBJS_ARM_C) : $(OBJDIR)/%.o : %.c
	@echo Compiling: $<
	$(CC) -c $(C_FLAGS) $< -o $@

$(OBJS_ARM_CPP) : $(OBJDIR)/%.o : %.cpp
	@echo Compiling: $<
	$(CXX) -c $(CPP_FLAGS) $< -o $@

$(OBJS_ARM_ASM) : $(OBJDIR)/%.o : %.s
	@echo Assembling: $<
	$(AS) -c $(ASM_FLAGS) $< -o $@

# Thumb mode
$(OBJS_THUMB_C) : $(OBJDIR)/%.o : %.c
	@echo Compiling: $<
	$(CC) -c -mthumb $(C_FLAGS) $< -o $@

$(OBJS_THUMB_CPP) : $(OBJDIR)/%.o : %.cpp
	@echo Compiling: $<
	$(CC) -c -mthumb $(CPP_FLAGS) $< -o $@

$(OBJS_THUMB_ASM) : $(OBJDIR)/%.o : %.s
	@echo Assembling: $<
	$(AS) -c -mthumb $(ASM_FLAGS) $< -o $@

# Linking
$(ELF): $(OBJS) $(LD_SCRIPT)
	@echo Linking: $@
	$(LD) $(OBJS) $(LD_FLAGS) $(LIBS) -o $@

$(LSS): $(ELF)
	$(OBJDUMP) -h -S -C $< > $@

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

size:
	$(SIZE) -B -t $(ELF)
	$(SIZE) -A -t $(ELF)

directories:
	mkdir -p $(BINDIR) $(OBJDIR) $(DEPDIR) $(LSTDIR)

clean:
	$(RM) -r $(BINDIR) $(OBJDIR) $(DEPDIR) $(LSTDIR) $(ROMFILES)
	$(RM) utils/lpc21isp

utils/lpc21isp: utils/lpc21isp_148x.c
	@echo
	@echo Build lpc21isp programmer
	gcc $^ -o $@

# Program the device.  - lpc21isp will not work for SAM7
load program: $(HEX) utils/lpc21isp
	@echo
	@echo Upload over 
	utils/lpc21isp $(LPC21ISP_OPTIONS) $< $(LPC21ISP_PORT) $(LPC21ISP_BAUD) $(LPC21ISP_XTAL)
    
loadterm: $(HEX) utils/lpc21isp
	@echo
	@echo Upload over 
	utils/lpc21isp $(LPC21ISP_OPTIONS) -term $< $(LPC21ISP_PORT) $(LPC21ISP_BAUD) $(LPC21ISP_XTAL)

debug: $(ELF)
	$(GDB) -x debug.gdb $<

# set TOOLSET bin directory first in PATH    
ifeq (,$(findstring ;,$(PATH)))
  PATH := $(subst :,,/$(TOOLSET)/bin):$(PATH)
else
  PATH := $(subst /,\,$(TOOLSET)/bin);$(PATH)
endif
export PATH

# dependencies inclusion
# if make target list not contain this targets - include dependencies
ifeq (,$(findstring clean,$(MAKECMDGOALS)))
 ifeq (,$(findstring directories,$(MAKECMDGOALS)))
-include $(shell mkdir $(DEPDIR) 2>/dev/null) $(wildcard $(DEPDIR)/*.d)
 endif
endif

