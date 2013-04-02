# target type
#------------------
TARGET ?= quickchat
#TARGET ?= vypresschat
TARGET_DEBUG ?= 1
#TARGET_INSTALL_LOCATION = d:\\programs\\miranda-testing
TARGET_INSTALL_LOCATION = d:\\devel\\miranda\\miranda

# version stuff
#-----------------------
TARGET_VER_COPYRIGHT = (c) 2005 Saulius Menkevicius
TARGET_VER_MAJ0 = 0
TARGET_VER_MAJ1 = 9
TARGET_VER_MIN0 = 3
TARGET_VER_MIN1 = 0
#TARGET_CVS = 1

###########################################################
###########################################################

# target proto
#------------------------------
ifeq ($(TARGET), quickchat)
TARGET_PROTO=QUICKCHAT
endif

ifeq ($(TARGET), vypresschat)
TARGET_PROTO=VYPRESSCHAT
endif

# debug
#------------------------------
ifeq ($(TARGET_DEBUG), 1)
TARGET_DEBUG_OPT = -D_DEBUG -DMEMWATCH
TARGET_CC_OPTIMISE = -g -ggdb
else
TARGET_DEBUG_OPT = -DNDEBUG
TARGET_CC_OPTIMISE = -Os -pipe -march=pentium
endif

ifeq ($(TARGET_CVS), 1)
TARGET_VER_CVS	= -cvs$(shell date +%y%m%d)
endif

ifeq ($(TARGET_DEBUG), 1)
TARGET_VER_NAME = $(TARGET_VER_CVS)-dbg
else
TARGET_VER_NAME = $(TARGET_VER_CVS)
endif

# version stuff
#-------------------------------
TARGET_DIST_NAME = $(TARGET)-$(TARGET_VER_MAJ0)-$(TARGET_VER_MAJ1)-$(TARGET_VER_MIN0)-$(TARGET_VER_MIN1)$(TARGET_VER_NAME)

# compiler defines
#-------------------------------
CC_OPT = $(TARGET_CC_OPTIMISE) $(TARGET_DEBUG_OPT) \
	-Wall -mms-bitfields -mwindows -mno-cygwin 

TARGET_VER_OPT = -D$(TARGET_PROTO)
CC = gcc $(CC_OPT) $(TARGET_VER_OPT)
CC_OPT_DLL_LINK = $(CC_OPT) -shared -Wl,--enable-auto-image-base -lws2_32 -lrpcrt4 \
	-Lcontrib/ -lunicows -lcomctl32
WINDRES_OPT = $(TARGET_VER_OPT) $(TARGET_DEBUG_OPT)

# defines
#-------------------------------
OBJEXT = obj
TARGET_DIR = build/$(TARGET)

OBJFILES_MAIN = $(TARGET_DIR)/main.$(OBJEXT) $(TARGET_DIR)/service.$(OBJEXT) $(TARGET_DIR)/util.$(OBJEXT) \
	$(TARGET_DIR)/msghandler.$(OBJEXT) $(TARGET_DIR)/msgloop.$(OBJEXT) $(TARGET_DIR)/pthread.$(OBJEXT) \
	$(TARGET_DIR)/user.$(OBJEXT) $(TARGET_DIR)/contacts.$(OBJEXT) $(TARGET_DIR)/options.$(OBJEXT) \
	$(TARGET_DIR)/chanlist.$(OBJEXT) $(TARGET_DIR)/chatroom.$(OBJEXT) $(TARGET_DIR)/userlist.$(OBJEXT) \
	$(TARGET_DIR)/strhashfunc.$(OBJEXT) $(TARGET_DIR)/hashtable.$(OBJEXT) $(TARGET_DIR)/skin.$(OBJEXT) \
	$(TARGET_DIR)/resource.$(OBJEXT) \
	libvqproto/libvqproto.a

ifeq ($(TARGET_DEBUG), 1)
OBJFILES = $(OBJFILES_MAIN) $(TARGET_DIR)/memwatch.$(OBJEXT)
else
OBJFILES = $(OBJFILES_MAIN)
endif

HEADERS = main.h service.h resource.h miranda.h options.h \
	main.h msgloop.h msghandler.h user.h contacts.h userlist.h \
	chanlist.h chatroom.h util.h \
	libvqproto/vqproto.h contrib/memwatch.h \
	version.h	# this file is generated dynamically

ICONPACK_ICONS = icons/$(TARGET).ico \
	icons/$(TARGET)_online.ico \
	icons/$(TARGET)_offline.ico \
	icons/$(TARGET)_away.ico \
	icons/$(TARGET)_dnd.ico \
	icons/$(TARGET)_na.ico 

TARGET_ICONPACK = proto_$(TARGET_PROTO)

# dependencies
#----------------------------

all: $(TARGET_DIR)/$(TARGET).dll $(TARGET_DIR)/$(TARGET_ICONPACK).dll

$(TARGET_DIR)/$(TARGET).dll: $(OBJFILES) $(TARGET_DIR)
	$(CC) $(OBJFILES) -o $(TARGET_DIR)/$(TARGET).dll $(CC_OPT_DLL_LINK)

$(TARGET_DIR)/$(TARGET_ICONPACK).dll: $(TARGET_DIR)/iconpack.$(OBJEXT)
	$(CC) $(TARGET_DIR)/iconpack.$(OBJEXT) -o $(TARGET_DIR)/$(TARGET_ICONPACK).dll $(CC_OPT_DLL_LINK) 
	
$(TARGET_DIR)/iconpack.$(OBJEXT): iconpack.rc $(ICONPACK_ICONS) $(TARGET_DIR)
	windres $(WINDRES_OPT) -i iconpack.rc -o $(TARGET_DIR)/iconpack.$(OBJEXT)
			      
$(TARGET_DIR)/main.obj: main.c $(HEADERS) $(TARGET_DIR)
	$(CC) -c main.c -o $(TARGET_DIR)/main.$(OBJEXT)

$(TARGET_DIR)/service.$(OBJEXT): service.c $(HEADERS) $(TARGET_DIR)
	$(CC) -c service.c -o $(TARGET_DIR)/service.$(OBJEXT)

$(TARGET_DIR)/msghandler.$(OBJEXT): msghandler.c $(HEADERS) $(TARGET_DIR)
	$(CC) -c msghandler.c -o $(TARGET_DIR)/msghandler.$(OBJEXT)

$(TARGET_DIR)/msgloop.$(OBJEXT): msgloop.c $(HEADERS) $(TARGET_DIR)
	$(CC) -c msgloop.c -o $(TARGET_DIR)/msgloop.$(OBJEXT)

$(TARGET_DIR)/pthread.$(OBJEXT): pthread.c $(HEADERS) $(TARGET_DIR)
	$(CC) -c pthread.c -o $(TARGET_DIR)/pthread.$(OBJEXT)

$(TARGET_DIR)/user.$(OBJEXT): user.c $(HEADERS) $(TARGET_DIR)
	$(CC) -c user.c -o $(TARGET_DIR)/user.$(OBJEXT)

$(TARGET_DIR)/contacts.$(OBJEXT): contacts.c $(HEADERS) $(TARGET_DIR)
	$(CC) -c contacts.c -o $(TARGET_DIR)/contacts.$(OBJEXT)

$(TARGET_DIR)/userlist.$(OBJEXT): userlist.c $(HEADERS) $(TARGET_DIR)
	$(CC) -c userlist.c -o $(TARGET_DIR)/userlist.$(OBJEXT)

$(TARGET_DIR)/chatroom.$(OBJEXT): chatroom.c $(HEADERS) $(TARGET_DIR)
	$(CC) -c chatroom.c -o $(TARGET_DIR)/chatroom.$(OBJEXT)

$(TARGET_DIR)/options.$(OBJEXT): options.c $(HEADERS) $(TARGET_DIR)
	$(CC) -c options.c -o $(TARGET_DIR)/options.$(OBJEXT)

$(TARGET_DIR)/chanlist.$(OBJEXT): chanlist.c $(HEADERS) $(TARGET_DIR)
	$(CC) -c chanlist.c -o $(TARGET_DIR)/chanlist.$(OBJEXT)

$(TARGET_DIR)/util.$(OBJEXT): util.c $(HEADERS) $(TARGET_DIR)
	$(CC) -c util.c -o $(TARGET_DIR)/util.$(OBJEXT)

$(TARGET_DIR)/skin.$(OBJEXT): skin.c $(HEADERS) $(TARGET_DIR)
	$(CC) -c skin.c -o $(TARGET_DIR)/skin.$(OBJEXT)

$(TARGET_DIR)/resource.$(OBJEXT): resource.rc $(HEADERS) $(TARGET_DIR)
	windres $(WINDRES_OPT) -i resource.rc -o $(TARGET_DIR)/resource.$(OBJEXT)

$(TARGET_DIR)/memwatch.$(OBJEXT): contrib/memwatch.c contrib/memwatch.h $(TARGET_DIR)
	$(CC) -c contrib/memwatch.c -o $(TARGET_DIR)/memwatch.$(OBJEXT)

$(TARGET_DIR)/strhashfunc.$(OBJEXT): contrib/strhashfunc.c contrib/strhashfunc.h $(TARGET_DIR)
	$(CC) -c contrib/strhashfunc.c -o $(TARGET_DIR)/strhashfunc.$(OBJEXT)

$(TARGET_DIR)/hashtable.$(OBJEXT): contrib/hashtable.c contrib/hashtable.h $(TARGET_DIR)
	$(CC) -c contrib/hashtable.c -o $(TARGET_DIR)/hashtable.$(OBJEXT)
	
libvqproto/libvqproto.a:
	make -C libvqproto CC='gcc $(CC_OPT)'

version.h:
	echo "/*" > version.h
	echo " * This file is generated by 'make version.h'." >> version.h
	echo " * Any changes you make here will be lost, please" >> version.h
	echo " * modify the Makefile instead and run 'make version'." >> version.h
	echo " */" >> version.h
	echo "" >> version.h
	echo "#ifndef __VERSION_H" >> version.h
	echo "#define __VERSION_H" >> version.h
	echo "#define VQCHAT_VER_MAJ0	$(TARGET_VER_MAJ0)" >> version.h
	echo "#define VQCHAT_VER_MAJ1	$(TARGET_VER_MAJ1)" >> version.h
	echo "#define VQCHAT_VER_MIN0	$(TARGET_VER_MIN0)" >> version.h
	echo "#define VQCHAT_VER_MIN1	$(TARGET_VER_MIN1)" >> version.h
	echo -n "#define VQCHAT_VER_FILEVERSION " >> version.h
	echo -n "\"$(TARGET_VER_MAJ0), $(TARGET_VER_MAJ1), " >> version.h
	echo "$(TARGET_VER_MIN0), $(TARGET_VER_MIN1) $(TARGET_VER_NAME)\\0\"" >> version.h
	echo "#define VQCHAT_VER_COPYRIGHT \"$(TARGET_VER_COPYRIGHT)\"" >> version.h
	echo "#endif /* __VERSION_H */" >> version.h

version:
	rm -f version.h
	make version.h

clean: $(TARGET_DIR)
	make -C libvqproto clean
	rm -f version.h
	rm -f $(TARGET_DIR)/*.$(OBJEXT) $(TARGET_DIR)/*.dll
	rm -rf build/$(TARGET_DIST_NAME) build/$(TARGET_DIST_NAME).zip

install: all
	cp $(TARGET_DIR)/$(TARGET).dll $(TARGET_INSTALL_LOCATION)/Plugins/
	cp $(TARGET_DIR)/$(TARGET_ICONPACK).dll $(TARGET_INSTALL_LOCATION)/Icons/

deinstall:
	rm -f $(TARGET_INSTALL_LOCATION)/Plugins/$(TARGET).dll
	rm -f $(TARGET_INSTALL_LOCATION)/Icons/$(TARGET_ICONPACK).dll

debug:
	make install
	gdb $(TARGET_INSTALL_LOCATION)/miranda32.exe

dist: all
	rm -rf build/$(TARGET_DIST_NAME) build/$(TARGET_DIST_NAME).zip
	mkdir build/$(TARGET_DIST_NAME)
	mkdir build/$(TARGET_DIST_NAME)/Plugins build/$(TARGET_DIST_NAME)/Icons \
		build/$(TARGET_DIST_NAME)/docs build/$(TARGET_DIST_NAME)/for-win95-98-me-only
	cp contrib/unicows.dll build/$(TARGET_DIST_NAME)/for-win95-98-me-only
	echo "Win95/98/ME users: place unicows.dll in the same dir as miranda32.exe" \
		> build/$(TARGET_DIST_NAME)/for-win95-98-me-only/README
	cp $(TARGET_DIR)/$(TARGET).dll build/$(TARGET_DIST_NAME)/Plugins
	cp $(TARGET_DIR)/$(TARGET_ICONPACK).dll build/$(TARGET_DIST_NAME)/Icons
ifneq ($(TARGET_DEBUG), 1)
	strip build/$(TARGET_DIST_NAME)/Plugins/$(TARGET).dll
	strip build/$(TARGET_DIST_NAME)/Icons/$(TARGET_ICONPACK).dll
endif
	cp docs/TODO docs/README docs/ChangeLog docs/AUTHORS \
		docs/CREDITS docs/Copying docs/NEWS build/$(TARGET_DIST_NAME)\docs
	cd build; pwd; zip -9 -r $(TARGET_DIST_NAME).zip $(TARGET_DIST_NAME)
	rm -rf build/$(TARGET_DIST_NAME)

rebuild:
	make clean
	make

redist:
	make TARGET_DEBUG=0 TARGET=vypresschat clean
	make TARGET_DEBUG=0 TARGET=vypresschat dist
	make TARGET_DEBUG=0 TARGET=quickchat clean
	make TARGET_DEBUG=0 TARGET=quickchat dist
	make clean
	
$(TARGET_DIR): build
	mkdir $(TARGET_DIR)

build:
	mkdir build
