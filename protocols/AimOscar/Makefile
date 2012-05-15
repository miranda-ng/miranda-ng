####################################################################
# AIM OSCAR Protocol Plugin
# Makefile for Mingw
####################################################################
#
# TARGETS:
# all       Compiles dll into debug directory (create dir first)
# depend    Generate dependancy file (Makefile.dep)
# format    Formats the source files using GNU Indent
# clean     Cleans object files
#
#
# NOTES:
# To use this makefile you need to download the latest Mingw from
# http://mingw.org/.  I have only tested on MinGW Version 2.0.0
# (using the latest package updates).  Older versions make work as
# well.  You also need to checkout the SDK module alongside this
# module.  You will need a copy of rm.exe for the 'make clean' to
# work (http://unxutils.sourceforge.net/).  Make sure you create a
# 'Bin\debug\plugins' directory or 'Bin\release\plugins' in the parent 
# directory of this cvs module or the compile will fail.  To format
# the code (format target) you will need GNU Indent from:
# http://gnuwin32.sourceforge.net/.  Indent.exe will need to be in
# the path or in the same directory as the source.
####################################################################
SRC =\
	snac.cpp \
	flap.cpp \
	conv.cpp \
	aim.cpp \
	chat.cpp \
 	connection.cpp \
	packets.cpp \
	services.cpp \
	thread.cpp \
	theme.cpp \
	utility.cpp \
	popup.cpp \
	proxy.cpp \
	direct_connect.cpp \
	file.cpp \
	ui.cpp \
	links.cpp \
	server.cpp \
	client.cpp \
	error.cpp \
	tlv.cpp \
	proto.cpp \
	away.cpp \
	avatars.cpp
		
OBJ =	$(SRC:.cpp=.o)
RES = aim.res version.res
HDR = defines.h \
	tlv.h \
	snac.h \
	flap.h  \
	aim.h \
	chat.h \
	client.h \
 	connection.h \
	packets.h \
	services.h \
	client.h \
	server.h \
	thread.h \
	theme .h \
	utility.h \
	popup.h \
	proxy.h	\
	direct_connect.ch \
	file.h \
	resource.h \
	ui.h \
	m_cluiframes.h \
	links.h \
	error.h \
	conv.h \
	proto.h \
	avatars.h

LIB = -lgdi32 -lComdlg32 -lwsock32 -lstdc++ 
CC = gcc
RC = windres
RM = rm
ID = indent

# Install location
ifdef DEBUG
BIN = ../../bin/debug/plugins/Aim.dll
else
BIN = ../../bin/release/plugins/Aim.dll
endif

# Defines
DEFINES = -DWIN32 -D__SEH_NOOP -DUNICODE
ifdef DEBUG
DEFINES := $(DEFINES) -D_DEBUG
endif

# Flags
RCFLAGS = -O coff
ifdef DEBUG
CFLAGS   = -g -g3 $(DEFINES) -I../../include
LFLAGS   = -shared
else
CFLAGS  = -O1 $(DEFINES) -I../../include
LFLAGS  = -shared -s
endif

CPPFLAGS = $(CFLAGS)

# Targets
all : $(OBJ) $(RES)
	$(CC) $(LFLAGS) -o $(BIN) $(OBJ) $(RES) $(LIB) -Wl

%.res : %.rc resource.h Makefile
	$(RC) $(RCFLAGS) $< $@ 

depend : 
	$(CC) -MM $(CFLAGS) $(SRC)>Makefile.dep

clean :
	$(RM) -fr $(OBJ) $(RES) *.ncb *.suo *.aps Release Debug *.vcproj.* *.opt *.dat *.plg Ankh.Load

-include Makefile.dep
