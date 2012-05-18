SRC = commonheaders.c \
      database.c \
	  dbcache.c \
	  dbcontacts.c \
	  dbevents.c \
	  dbheaders.c \
	  dbini.c \
	  dblists.c \
	  dbmodulechain.c \
	  dbsettings.c \
	  dbtime.c \
	  encrypt.c \
	  init.c
OBJ = $(SRC:.c=.o)
RES = resource.res
LIB = -lgdi32 -lversion -lcomctl32 -lcomdlg32 -lole32

CC = gcc
RC = windres
RM = rm

# Install location
ifdef DEBUG
BIN = ..\..\bin\debug\plugins\dbx_3x.dll
else
BIN = ..\..\bin\release\plugins\dbx_3x.dll
endif

# Defines
DEFINES = -DWIN32 -D__SEH_NOOP
ifdef DEBUG
DEFINES := $(DEFINES) -D_DEBUG
endif

# Flags
LFLAGS  = -shared 
RCFLAGS = --input-format rc --output-format coff
ifdef DEBUG
CFLAGS  = -g $(DEFINES) -I../../include
else
CFLAGS  = -O1 $(DEFINES) -I../../include
endif

# Targets
all : $(OBJ) $(RES)
	$(CC) $(LFLAGS) $(CFLAGS) -o $(BIN) $(OBJ) $(RES) $(LIB) -Wl

$(RES) : $(RES:.res=.rc) $(RES:.res=.h) Makefile
	$(RC) $(RCFLAGS) -o $(RES) -i $(RES:.res=.rc)

clean :
	$(RM) -f $(OBJ) $(RES)

