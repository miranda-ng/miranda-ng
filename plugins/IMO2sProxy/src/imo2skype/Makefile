CFLAGS	= -O0 -g -I../common/ -I../imolib/ -lcurl -lpthread -lm
CC	= gcc
COMMON  = ../common/fifo.c ../common/cJSON.c ../common/memlist.c
IMOLIB  = ../imolib/imo_request.c ../imolib/imo_skype.c ../imolib/io_layer.c
IMO2S   = buddylist.c queue.c msgqueue.c imo2skypeapi.c callqueue.c socksproxy.c avatarlist.c
.PHONY: all imo2stest imo2sproxy clean

all	:	imo2stest imo2sproxy

imo2stest:	$(COMMON) $(IMO2S) $(IMOLIB) imo2stest.c
	        $(CC) -o imo2stest $(COMMON) $(IMO2S) $(IMOLIB) imo2stest.c $(CFLAGS)
		strip imo2stest

imo2sproxy:	$(COMMON) $(IMO2S) $(IMOLIB) imo2sproxy.c main.c
	        $(CC) -o imo2sproxy $(COMMON) $(IMO2S) $(IMOLIB) imo2sproxy.c main.c $(CFLAGS)
#		strip imo2sproxy

clean	:
		rm imo2stest imo2sproxy
