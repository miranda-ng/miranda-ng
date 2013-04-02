# $Id: makefile,v 1.5 2005/03/14 22:35:18 bobas Exp $

TARGET_STATIC = libvqproto.a

O_FILES = link.o message.o uuid.o
HEADERS	= vqproto.h link.h message.h uuid.h

$(TARGET_STATIC): $(O_FILES)
	$(AR) r $(TARGET_STATIC) $(O_FILES)

link.o:	link.c $(HEADERS)
	$(CC) -c link.c -o link.o

message.o: message.c $(HEADERS)
	$(CC) -c message.c -o message.o

uuid.o: uuid.c $(HEADERS)
	$(CC) -c uuid.c -o uuid.o

clean:
	rm -f *.o $(TARGET_STATIC)
