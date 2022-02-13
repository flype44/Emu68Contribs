#################################################
#
# SAS/C Makefile for "Emu68Info"
# for AmigaOS M68K SAS/C 6.59 Compiler
# 
#################################################

PREFX := /opt/m68k-amigaos

CC			:= $(PREFX)/bin/m68k-amigaos-gcc
AS			:= $(PREFX)/bin/m68k-amigaos-as
AR			:= $(PREFX)/bin/m68k-amigaos-ar
LD			:= $(PREFX)/bin/m68k-amigaos-ld
RL			:= $(PREFX)/bin/m68k-amigaos-ranlib
VASM		:= $(PREFX)/bin/vasmm68k_mot

OUTFILE		= Emu68Info

OBJECTS		= Emu68Info.o AsmFuncs.o DeviceTree.o MailBox.o Modules.o VC4Msg.o VC4.o
DT_INCLUDES	:= ../../emu68-tools/devicetree.resource/include
CFLAGS		:= -noixemul -m68040 -mhard-float -I${DT_INCLUDES}
VFLAGS		= -devpac -I$(PREFX)/m68k-amigaos/ndk-include -Fhunk -m68040 -quiet

all: $(OUTFILE)

clean:
	rm -rfv $(OBJECTS)

%.o: %.a
	$(VASM) $(VFLAGS) -o $@ $*.a

$(OUTFILE): $(OBJECTS)
	$(CC) -o $(OUTFILE) $(OBJECTS)
