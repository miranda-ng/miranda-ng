EKHTML_LIB = $(top_srcdir)/src/libekhtml.la

EXTRA_DIST = \
	ek_sgmllib.py \
	gen_html.py \
	plparser.pl \
	pyparser.py \
	test_basic.sh \
	test_bogus_feedsize.sh \
	test_crazypage.sh \
	tstpages

noinst_PROGRAMS = tester

tester_SOURCES      = tester.c
tester_LDADD        = $(EKHTML_LIB) 
tester_LDFLAGS      = -static
tester_DEPENDENCIES = $(EKHTML_LIB)

$(EKHTML_LIB):
	cd $(top_srcdir)/src && $(MAKE)

TESTS = test_basic.sh test_crazypage.sh test_bogus_feedsize.sh

dist-hook:
	rm -rf `find $(distdir)/tstpages -name CVS`
