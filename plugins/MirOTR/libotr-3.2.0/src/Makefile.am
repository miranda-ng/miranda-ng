INCLUDES = @LIBGCRYPT_CFLAGS@

lib_LTLIBRARIES = libotr.la

libotr_la_SOURCES = privkey.c context.c proto.c b64.c dh.c mem.c message.c \
		    userstate.c tlv.c auth.c sm.c

libotr_la_LDFLAGS = -version-info @LIBOTR_LIBTOOL_VERSION@ @LIBS@ @LIBGCRYPT_LIBS@

otrincdir = $(includedir)/libotr

otrinc_HEADERS = b64.h context.h dh.h mem.h message.h privkey.h proto.h \
		 version.h userstate.h tlv.h serial.h auth.h sm.h privkey-t.h
