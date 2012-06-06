check_PROGRAMS = oauthexample oauthdatapost tcwiki tceran tcother oauthtest oauthtest2 oauthsign oauthbodyhash
ACLOCAL_AMFLAGS= -I m4

OAUTHDIR =../src
INCLUDES = -I$(srcdir)/$(OAUTHDIR)
MYCFLAGS = @LIBOAUTH_CFLAGS@ @HASH_CFLAGS@ @CURL_CFLAGS@ 
MYLDADD  = $(OAUTHDIR)/liboauth.la
LIBS     = -lm @HASH_LIBS@ @CURL_LIBS@ @LIBS@

tcwiki_SOURCES = selftest_wiki.c commontest.c commontest.h
tcwiki_LDADD = $(MYLDADD)
tcwiki_CFLAGS = $(MYCFLAGS) @TEST_UNICODE@

tceran_SOURCES = selftest_eran.c commontest.c commontest.h
tceran_LDADD = $(MYLDADD)
tceran_CFLAGS = $(MYCFLAGS) @TEST_UNICODE@

tcother_SOURCES = selftest_other.c commontest.c commontest.h
tcother_LDADD = $(MYLDADD)
tcother_CFLAGS = $(MYCFLAGS)

oauthtest_SOURCES = oauthtest.c
oauthtest_LDADD = $(MYLDADD)
oauthtest_CFLAGS = $(MYCFLAGS)

oauthtest2_SOURCES = oauthtest2.c
oauthtest2_LDADD = $(MYLDADD)
oauthtest2_CFLAGS = $(MYCFLAGS)

oauthexample_SOURCES = oauthexample.c
oauthexample_LDADD = $(MYLDADD)
oauthexample_CFLAGS = $(MYCFLAGS)

oauthsign_SOURCES = oauthsign.c
oauthsign_LDADD = $(MYLDADD)
oauthsign_CFLAGS = $(MYCFLAGS)

oauthdatapost_SOURCES = oauthdatapost.c
oauthdatapost_LDADD = $(MYLDADD)
oauthdatapost_CFLAGS = $(MYCFLAGS)

oauthbodyhash_SOURCES = oauthbodyhash.c
oauthbodyhash_LDADD = $(MYLDADD)
oauthbodyhash_CFLAGS = $(MYCFLAGS)
