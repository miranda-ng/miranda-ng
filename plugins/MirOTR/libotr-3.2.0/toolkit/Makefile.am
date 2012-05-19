INCLUDES = -I$(includedir) -I../src @LIBGCRYPT_CFLAGS@

noinst_HEADERS = aes.h ctrmode.h parse.h sesskeys.h readotr.h sha1hmac.h

bin_PROGRAMS = otr_parse otr_sesskeys otr_mackey otr_readforge \
	otr_modify otr_remac

COMMON_S = parse.c sha1hmac.c
COMMON_LD = ../src/libotr.la @LIBS@ @LIBGCRYPT_LIBS@

otr_parse_SOURCES = otr_parse.c readotr.c $(COMMON_S)
otr_parse_LDADD = $(COMMON_LD)

otr_sesskeys_SOURCES = otr_sesskeys.c sesskeys.c $(COMMON_S)
otr_sesskeys_LDADD = $(COMMON_LD)

otr_mackey_SOURCES = otr_mackey.c sesskeys.c $(COMMON_S)
otr_mackey_LDADD = $(COMMON_LD)

otr_readforge_SOURCES = otr_readforge.c readotr.c sesskeys.c \
	aes.c ctrmode.c $(COMMON_S)
otr_readforge_LDADD = $(COMMON_LD)

otr_modify_SOURCES = otr_modify.c readotr.c $(COMMON_S)
otr_modify_LDADD = $(COMMON_LD)

otr_remac_SOURCES = otr_remac.c $(COMMON_S)
otr_remac_LDADD = $(COMMON_LD)


man_MANS = otr_toolkit.1
EXTRA_DIST = otr_toolkit.1

MANLINKS = otr_parse.1 otr_sesskeys.1 otr_mackey.1 otr_readforge.1 \
	    otr_modify.1 otr_remac.1
	    
install-data-local:
	-mkdir -p $(DESTDIR)$(man1dir)
	(cd $(DESTDIR)$(man1dir) && \
	for f in $(MANLINKS); do ln -sf otr_toolkit.1 $$f; done)

uninstall-local:
	(cd $(DESTDIR)$(man1dir) && \
	for f in $(MANLINKS); do rm -f $$f; done)
