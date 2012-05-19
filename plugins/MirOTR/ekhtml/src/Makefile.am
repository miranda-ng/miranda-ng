libekhtml_la_SOURCES = 		\
	ekhtml.c 		\
	ekhtml_comment.c 	\
	ekhtml_data.c		\
	ekhtml_endtag.c 	\
	ekhtml_special.c	\
	ekhtml_starttag.c	\
	ekhtml_util.c		\
	hash.c

libekhtml_la_DEPENDENCIES = ekhtml_tables.h
lib_LTLIBRARIES = libekhtml.la
noinst_PROGRAMS = ekhtml_mktables

ekhtml_mktables_SOURCES = ekhtml_mktables.c

ekhtml.c: ekhtml_tables.h

ekhtml_tables.h: ekhtml_mktables
	./ekhtml_mktables > $@

CLEANFILES = ekhtml_tables.h
