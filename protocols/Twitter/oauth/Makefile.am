ACLOCAL_AMFLAGS= -I m4
SUBDIRS = @subdirs@

EXTRA_DIST=liboauth.lsm.in oauth.pc.in Doxyfile.in \
					 COPYING.GPL COPYING.MIT LICENSE.OpenSSL \
					 m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = oauth.pc

TESTS=tests/tcwiki@EXESUF@ tests/tceran@EXESUF@ tests/tcother@EXESUF@

CLEANFILES = stamp-doxygen stamp-doc

MAINTAINERCLEANFILES = \
	src/Makefile.in \
	tests/Makefile.in \
	tests/atconfig \
	doc/Makefile.in \
	Makefile.in \
	aclocal.m4 \
	compile \
	config.guess \
	config.h.in \
	config.sub \
	configure \
	depcomp \
	install-sh \
	missing

dox: stamp-doxygen

stamp-doxygen: src/oauth.h doc/mainpage.dox
	$(DOXYGEN) Doxyfile && touch stamp-doxygen
	cp doc/man/man3/oauth.h.3 doc/oauth.3
	sed 's/ -\([qsdU]\)/ \\-\1/g' doc/man/man3/oauth.h.3 \
	| sed 's/ -1 / \\-1 /' \
	| sed 's/--/\\-\\-/' \
	> doc/oauth.3
