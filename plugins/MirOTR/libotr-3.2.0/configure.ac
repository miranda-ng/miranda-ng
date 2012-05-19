dnl Process this file with autoconf to produce configure.

AC_INIT(toolkit/parse.c)

AM_CONFIG_HEADER(config.h)

dnl Notes on version numbering:
dnl   For an implementation-only change:
dnl     Change the libotr package version from a.b.c to a.b.(c+1)
dnl     Change the libotr libtool version from x:y:z to x:(y+1):z
dnl   For a backwards-compatible API change (e.g. adding functions):
dnl     Change the libotr package version from a.b.c to a.(b+1).0
dnl     Change the libotr libtool version from x:y:z to (x+1):0:(z+1)
dnl       [Note that this does *not* change the major number of the .so.]
dnl   For a backwards-incompatible API change (e.g. changing data structures):
dnl     Change the libotr package version from a.b.c to (a+1).0.0
dnl     Change the libotr libtool version from x:y:z to (x+1):0:0

AM_INIT_AUTOMAKE(libotr, 3.2.0)
LIBOTR_LIBTOOL_VERSION="4:0:2"

AC_SUBST(LIBOTR_LIBTOOL_VERSION)

AC_PROG_CC

AM_PROG_LIBTOOL

AM_PATH_LIBGCRYPT(1:1.2.0,,AC_MSG_ERROR(libgcrypt 1.2.0 or newer is required.))

AC_OUTPUT([Makefile src/Makefile toolkit/Makefile libotr.pc])

