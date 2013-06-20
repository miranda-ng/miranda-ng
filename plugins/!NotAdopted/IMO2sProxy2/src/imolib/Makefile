CFLAGS	= -I../common/ -lcurl -lpthread -lm -O0 -g
CC	= gcc
COMMON  = ../common/fifo.c ../common/cJSON.c
OBJS    = imo_request.c imo_skype.c skypetst.c io_layer.c

.PHONY: all skypetst clean

all	:	skypetst

skypetst:	$(COMMON) $(OBJS)
	        $(CC) -o skypetst $(COMMON) $(OBJS) $(CFLAGS)

clean	:
		rm skypetst
