ACLOCAL_AMFLAGS= -I m4
lib_LTLIBRARIES = liboauth.la
include_HEADERS = oauth.h 

liboauth_la_SOURCES=oauth.c config.h hash.c xmalloc.c xmalloc.h oauth_http.c
liboauth_la_LDFLAGS=@LIBOAUTH_LDFLAGS@ -version-info @VERSION_INFO@
liboauth_la_LIBADD=@HASH_LIBS@ @CURL_LIBS@
liboauth_la_CFLAGS=@LIBOAUTH_CFLAGS@ @HASH_CFLAGS@ @CURL_CFLAGS@
