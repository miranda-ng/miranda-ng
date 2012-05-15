###############################################################################
# MinGW/Cross-compiler build script for Miranda IM
# 
# Copyright 2008 Adam Strzelecki
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
###############################################################################
# SETTINGS

LIB := kernel32 user32 gdi32 comdlg32 advapi32 shell32 ole32 oleaut32 uuid ws2_32 version comctl32 winmm shlwapi msimg32 rpcrt4
INC := src include src/core include/mingw plugins/freeimage/Source plugins/zlib
DEF := FREEIMAGE_EXPORTS
BUILD := build
ZLIB := $(BUILD)/zlib.dll

CFLAGS := $(CFLAGS) $(if $(DEBUG),-g -D_DEBUG,-Os)
LDFLAGS := $(LDFLAGS) $(if $(DEBUG),-g,-Wl,-s)

CROSS_MINGW := i586-mingw32msvc i386-mingw32msvc i686-mingw32 i386-mingw32 mingw32 mingw
CROSS_CC := $(addsuffix -gcc, $(CROSS_MINGW))
CROSS_CXX := $(addsuffix -g++, $(CROSS_MINGW))
CROSS_RC := $(addsuffix -windres, $(CROSS_MINGW))

OBJDIR := $(BUILD)/.obj
UOBJDIR := $(BUILD)/.obj/unicode

###############################################################################
# COMPILER CHECKING
SPATH := $(subst ;, ,$(subst :, ,$(PATH)))
SYSTEM := $(shell uname -s)

$(if $(SYSTEM), $(info Cross compiling on $(SYSTEM).))

find-path=$(wildcard $(addsuffix /$(1),$(SPATH)))
files = $(foreach dir,$(wildcard $(1)/*),$(call files,$(dir),$2,$3,$4)$(filter %$(2),$(dir))$(if $(3), $(filter %$(3),$(dir)))$(if $(4), $(filter %$(4),$(dir))))
objects = $(addprefix $(OBJDIR)/,$(patsubst %.rc,%.res,$(patsubst %.cpp,%.obj,$(1:.c=.o))))
uobjects = $(addprefix $(UOBJDIR)/,$(patsubst %.rc,%.res,$(patsubst %.cpp,%.obj,$(1:.c=.o))))

CFLAGS := $(CFLAGS) $(addprefix -I,$(INC)) $(addprefix -D,$(DEF))
RCFLAGS := $(RCFLAGS) $(addprefix -I,$(INC)) $(addprefix -D,$(DEF))
LDFLAGS := $(LDFLAGS) $(addprefix -l,$(LIB))
CXXFLAGS := $(CFLAGS)

CC := $(if $(SYSTEM),$(notdir $(firstword $(foreach prog,$(CROSS_CC),$(call find-path,$(prog))))),$(CC))
CXX := $(if $(SYSTEM),$(notdir $(firstword $(foreach prog,$(CROSS_CXX),$(call find-path,$(prog))))),$(CXX))
RC := $(if $(SYSTEM),$(notdir $(firstword $(foreach prog,$(CROSS_RC),$(call find-path,$(prog))))),$(RC))

$(if $(CC),,$(error MinGW C compiler not found ($(CROSS_CC))))
$(if $(CXX),,$(error MinGW C++ compiler not found ($(CROSS_CXX))))
$(if $(RC),,$(error MinGW RC compiler not found ($(CROSS_RC))))

###############################################################################
# RECIPES

define EXE
	@mkdir -p $(dir $@)
	$(info [L] $(patsubst $(BUILD)/%,%,$@))
	@$(CC) -mwindows -o $@ $^ $(LDFLAGS)
endef

define DLL
	@mkdir -p $(dir $@)
	$(info [L] $(patsubst $(BUILD)/%,%,$@))
	@$(CC) -mwindows -shared -o $@ $^ $(LDFLAGS)
endef

define DLLPP
	@mkdir -p $(dir $@)
	$(info [L] $(patsubst $(BUILD)/%,%,$@))
	@$(CXX) -mwindows -shared -o $@ $^ $(LDFLAGS)
endef

define ZDLL
	@mkdir -p $(dir $@)
	$(info [L] $(patsubst $(BUILD)/%,%,$@))
	@$(CC) -mwindows -shared -o $@ $^ $(LDFLAGS) $(ZLIB)
endef

define ZDLLPP
	@mkdir -p $(dir $@)
	$(info [L] $(patsubst $(BUILD)/%,%,$@))
	@$(CXX) -mwindows -shared -o $@ $^ $(LDFLAGS) $(ZLIB)
endef

define CORE
	@mkdir -p $(dir $@)
	$(info [C] $<)
	@$(CC) -c $(CFLAGS) -D_STATIC $< -o $@ -MMD
endef

define PLUGIN
	@mkdir -p $(dir $@)
	$(info [C] $<)
	@$(CC) -c $(CFLAGS) $< -o $@ -MMD
endef

define PLUGINPP
	@mkdir -p $(dir $@)
	$(info [C] $<)
	@$(CXX) -c $(CXXFLAGS) $< -o $@ -MMD
endef

define UCORE
	@mkdir -p $(dir $@)
	$(info [C] $<)
	@$(CC) -c $(CFLAGS) -DUNICODE -D_STATIC $< -o $@ -MMD
endef

define UPLUGIN
	@mkdir -p $(dir $@)
	$(info [C] $<)
	@$(CC) -c $(CFLAGS) -DUNICODE $< -o $@ -MMD
endef

define UPLUGINPP
	@mkdir -p $(dir $@)
	$(info [C] $<)
	@$(CXX) -c $(CXXFLAGS) -DUNICODE $< -o $@ -MMD
endef

define RES
	@mkdir -p $(dir $@)
	$(info [R] $<)
	@$(RC) --input-format rc --output-format coff $(RCFLAGS) -I$(dir $<) -i $< -o $@
endef

# ANSI
$(OBJDIR)/src/%.o: src/%.c; $(CORE)
$(OBJDIR)/plugins/%.o: plugins/%.c; $(PLUGIN)
$(OBJDIR)/plugins/%.obj: plugins/%.cpp; $(PLUGINPP)
$(OBJDIR)/protocols/%.o: protocols/%.c; $(PLUGIN)
$(OBJDIR)/protocols/%.obj: protocols/%.cpp; $(PLUGINPP)
$(OBJDIR)/%.res: %.rc; $(RES)
# UNICODE
$(UOBJDIR)/src/%.o: src/%.c; $(UCORE)
$(UOBJDIR)/plugins/%.o: plugins/%.c; $(UPLUGIN)
$(UOBJDIR)/plugins/%.obj: plugins/%.cpp; $(UPLUGINPP)
$(UOBJDIR)/protocols/%.o: protocols/%.c; $(UPLUGIN)
$(UOBJDIR)/protocols/%.obj: protocols/%.cpp; $(UPLUGINPP)
$(UOBJDIR)/%.res: %.rc; $(RES)

###############################################################################
# FINAL MAKE

BIN := $(BUILD)/miranda32.exe

all: $(BIN) \
	$(ZLIB) \
\
	$(BUILD)/plugins/advaimg.dll \
	$(BUILD)/plugins/avs.dll \
	$(BUILD)/plugins/chat.dll \
	$(BUILD)/plugins/dbx_3x.dll \
	$(BUILD)/plugins/dbx_mmap.dll \
	$(BUILD)/plugins/dbx_rw.dll \
	$(BUILD)/plugins/clist_classic.dll \
	$(BUILD)/plugins/clist_nicer.dll \
	$(BUILD)/plugins/clist_mw.dll \
	$(BUILD)/plugins/import.dll \
	$(BUILD)/plugins/scriver.dll \
	$(BUILD)/plugins/srmm.dll \
	$(BUILD)/plugins/tabsrmm.dll \
\
	$(BUILD)/plugins/AIM.dll \
	$(BUILD)/plugins/GG.dll \
	$(BUILD)/plugins/ICQ.dll \
	$(BUILD)/plugins/IRC.dll \
	$(BUILD)/plugins/Jabber.dll \
	$(BUILD)/plugins/MSN.dll \
	$(BUILD)/plugins/Yahoo.dll \

# FIXME: GdiPlus is not part of MinGW
#	$(BUILD)/plugins/clist_modern.dll \

clean:
	rm -rf build

-include $(call files,$(OBJDIR),.d)

deps:
	@echo $(call files,$(OBJDIR),.d)

###############################################################################
# IGNORES

NOFREEIMAGE := \
	%/example.c %/pngtest.c \
	%/jmemdos.c %/jmemmac.c %/jmemnobs.c %/jmemname.c \
	%/cjpeg.c %/djpeg.c %/jpegtran.c %/rdjpgcom.c %/wrjpgcom.c \
	%/ckconfig.c \
	%/ansi2knr.c \
	%/tif_acorn.c %/tif_apple.c %/tif_atari.c %/tif_unix.c %/tif_vms.c %/tif_msdos.c %/tif_win3.c %/tif_win32.c \

###############################################################################
# PLUGINS

$(BIN):								$(call uobjects,$(call files,src,.c) src/vc6.rc); $(EXE)
$(ZLIB): 							$(call objects,$(call files,plugins/zlib,.c,.rc)); $(DLL)

$(BUILD)/plugins/advaimg.dll:		$(call objects,$(filter-out $(NOFREEIMAGE),$(call files,plugins/freeimage,.c,.cpp,.rc))); $(ZDLLPP)
$(BUILD)/plugins/avs.dll:			$(call uobjects,$(call files,plugins/avs,.c,.rc)); $(DLL)
$(BUILD)/plugins/chat.dll:			$(call uobjects,$(call files,plugins/chat,.c,.rc)); $(DLL)
$(BUILD)/plugins/dbx_3x.dll:		$(call uobjects,$(call files,plugins/db3x,.c,.rc)); $(DLL)
$(BUILD)/plugins/dbx_mmap.dll:		$(call uobjects,$(call files,plugins/db3x_mmap,.c,.rc)); $(DLL)
$(BUILD)/plugins/dbx_rw.dll:		$(call objects,$(filter-out %/icu.c %/tclsqlite.c,$(call files,plugins/dbrw,.c,.rc))); $(DLL)
$(BUILD)/plugins/clist_classic.dll:	$(call uobjects,$(call files,plugins/clist,.c,.rc)); $(DLL)
$(BUILD)/plugins/clist_modern.dll:	$(call uobjects,$(filter-out plugins/modernb/_disabled_src/% %/modern_gdiplus.cpp,$(call files,plugins/modernb,.c,.cpp,.rc))); $(DLLPP)
$(BUILD)/plugins/clist_mw.dll:		$(call uobjects,$(call files,plugins/mwclist,.c,.cpp,.rc)); $(DLL)
$(BUILD)/plugins/clist_nicer.dll:	$(call uobjects,$(filter-out plugins/clist_nicer/skineditor/% %/CLUIFrames/statusbar.c,$(call files,plugins/clist_nicer,.c,.cpp,.rc))); $(DLL)
$(BUILD)/plugins/import.dll: 		$(call uobjects,$(filter-out %/mirandahistory.c,$(call files,plugins/import,.c,.rc))); $(DLL)
$(BUILD)/plugins/scriver.dll: 		$(call uobjects,$(filter-out %/icons.rc,$(call files,plugins/Scriver,.c,.rc))); $(DLL)
$(BUILD)/plugins/srmm.dll: 			$(call uobjects,$(filter-out %/icons.rc,$(call files,plugins/srmm,.c,.rc))); $(DLL)
$(BUILD)/plugins/tabsrmm.dll: 		$(call uobjects,$(call files,plugins/tabsrmm,.c,.cpp,.rc)); $(DLLPP)

$(BUILD)/plugins/AIM.dll:	 		$(call uobjects,$(call files,protocols/AimOscar,.cpp,.rc)); $(DLLPP)
$(BUILD)/plugins/GG.dll: 			$(call objects,$(call files,protocols/Gadu-Gadu,.c,.rc)); $(DLL)
$(BUILD)/plugins/ICQ.dll: 			$(call uobjects,$(call files,protocols/IcqOscarJ,.c,.cpp,.rc)); $(DLLPP)
$(BUILD)/plugins/IRC.dll: 			$(call uobjects,$(call files,protocols/IRCG,.cpp,.rc)); $(DLLPP)
$(BUILD)/plugins/Jabber.dll: 		$(call uobjects,$(call files,protocols/JabberG,.c,.cpp,.rc)); $(ZDLLPP)
$(BUILD)/plugins/MSN.dll: 			$(call uobjects,$(call files,protocols/MSN,.c,.cpp,.rc)); $(DLLPP)
$(BUILD)/plugins/Yahoo.dll: 		$(call objects,$(call files,protocols/Yahoo,.c,.rc)); $(DLL)
