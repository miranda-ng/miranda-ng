AC_PREREQ(2.57)
AC_INIT([liboauth], [-], [robin AT gareus DOT org], [], [http://liboauth.sourceforge.net/])
#AM_MAINTAINER_MODE

AC_PATH_PROG(SED, sed, "", $PATH:/bin:/usr/bin:/usr/local/bin)
if test -z "$SED"; then
  AC_MSG_WARN([sed was not found])
fi

AC_MSG_CHECKING([liboauth version])
VERSION=`$SED -ne 's/^#define LIBOAUTH_VERSION "\(.*\)"/\1/p' ${srcdir}/src/oauth.h 2>/dev/null`
AC_MSG_RESULT($VERSION)
if test -z "$VERSION"; then
  AC_MSG_ERROR([version number can not be retrieved from src/oauth.h])
fi

VERSION_CUR=`$SED -ne 's/^#define LIBOAUTH_CUR *\([0-9]*\)/\1/p' ${srcdir}/src/oauth.h 2>/dev/null`
VERSION_REV=`$SED -ne 's/^#define LIBOAUTH_REV *\([0-9]*\)/\1/p' ${srcdir}/src/oauth.h 2>/dev/null`
VERSION_AGE=`$SED -ne 's/^#define LIBOAUTH_AGE *\([0-9]*\)/\1/p' ${srcdir}/src/oauth.h 2>/dev/null`
VERSION_INFO=${VERSION_CUR}:${VERSION_REV}:${VERSION_AGE}

AC_CONFIG_SRCDIR([src/oauth.c])
AC_CONFIG_TESTDIR([tests])
AC_CANONICAL_TARGET([])
AC_COPYRIGHT([Copyright (C) Robin Gareus 2007-2011])
AM_INIT_AUTOMAKE(liboauth,$VERSION)

AM_CONFIG_HEADER(src/config.h)

AC_SUBST(VERSION)
AC_SUBST(VERSION_INFO)
ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_CANONICAL_HOST

AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_LN_S
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL
AM_PROG_CC_C_O
AC_LIBTOOL_WIN32_DLL
AC_CONFIG_MACRO_DIR([m4])

AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h time.h string.h alloca.h stdio.h stdarg.h math.h)

AC_HEADER_MAJOR
AC_FUNC_ALLOCA
AC_STRUCT_TM
AC_STRUCT_ST_BLOCKS
AC_FUNC_CLOSEDIR_VOID

AH_TEMPLATE([HAVE_CURL], [Define as 1 if you have libcurl])
AH_TEMPLATE([USE_BUILTIN_HASH], [Define to use neither NSS nor OpenSSL])
AH_TEMPLATE([USE_NSS], [Define to use NSS instead of OpenSSL])
AH_TEMPLATE([HAVE_SHELL_CURL], [Define if you can invoke curl via a shell command. This is only used if HAVE_CURL is not defined.])
AH_TEMPLATE([OAUTH_CURL_TIMEOUT], [Define the number of seconds for the HTTP request to timeout; if not defined no timeout (or libcurl default) is used.])

EXESUF=
TEST_UNICODE=-DTEST_UNICODE

dnl *** Target specific settings ***
case $target_os in
  *darwin*)
  ;;
  *mingw32*|*win*)
    EXESUF=.exe
    TEST_UNICODE=
  ;;
  *)    
  ;;
esac
AC_SUBST(TEST_UNICODE)
AC_SUBST(EXESUF)

dnl *** misc complier/linker flags ***
LIBOAUTH_CFLAGS="-Wall"
LIBOAUTH_LDFLAGS="${LIBOAUTH_CFLAGS} -export-symbols-regex '^oauth_.*'"
#LIBOAUTH_CFLAGS="${LIBOAUTH_CFLAGS} -g -posix -std=c99 -pedantic"

AC_MSG_CHECKING([if -Wl,--as-needed works])
LDFLAGS_save=$LDFLAGS
LDFLAGS="$LDFLAGS -Wl,--as-needed"
AC_TRY_LINK([], [],
            [
             AC_MSG_RESULT([yes])
             LIBOAUTH_LDFLAGS="$LIBOAUTH_LDFLAGS -Wl,--as-needed"
            ], 
            [AC_MSG_RESULT([no])])
LDFLAGS=$LDFLAGS_save

AC_SUBST(LIBOAUTH_CFLAGS)
AC_SUBST(LIBOAUTH_LDFLAGS)


dnl *** PKGconfig oauth.pc.in ***
PC_REQ=""
PC_LIB=""

dnl *** configuration options ***
AC_ARG_ENABLE(curl, AC_HELP_STRING([--disable-curl],[do not use (command-line) curl]))
AC_ARG_ENABLE(libcurl, AC_HELP_STRING([--disable-libcurl],[do not use libcurl]))
AC_ARG_ENABLE(builtinhash, AC_HELP_STRING([--enable-builtinhash],[do use neither NSS nor OpenSSL: only HMAC/SHA1 signatures - no RSA/PK11]))
AC_ARG_ENABLE(nss, AC_HELP_STRING([--enable-nss],[use NSS instead of OpenSSL]))
AC_ARG_WITH([curltimeout], AC_HELP_STRING([--with-curltimeout@<:@=<int>@:>@],[use CURLOPT_TIMEOUT with libcurl HTTP requests. Timeout is given in seconds (default=60). Note: using this option also sets CURLOPT_NOSIGNAL. see http://curl.haxx.se/libcurl/c/curl_easy_setopt.html#CURLOPTTIMEOUT]))

report_curl="no"
dnl ** check for commandline executable curl 
if test "${enable_curl}" != "no"; then
  AC_PATH_PROG(CURLCMD, curl, no, $PATH:/bin:/usr/bin:/usr/local/bin)
  if test "$CURLCMD" != "no"; then
  AC_DEFINE(HAVE_SHELL_CURL, 1)
  report_curl="shell command"
  fi
fi

dnl ** check for libcurl
AS_IF([test "${enable_libcurl}" != "no"], [
  PKG_CHECK_MODULES(CURL, libcurl,
    [ AC_DEFINE(HAVE_CURL, 1) HAVE_CURL=1 PC_REQ="$PC_REQ libcurl" PC_LIB="$PC_LIB ${CURL_LIBS}" report_curl="libcurl" ] , 
    [
      AC_CHECK_HEADERS(curl/curl.h)
      AC_CHECK_LIB([curl], [curl_global_init], 
        [AC_DEFINE(HAVE_CURL, 1) HAVE_CURL=1 PC_REQ="$PC_REQ libcurl" PC_LIB="$PC_LIB -lcurl" report_curl="libcurl" ]
      )
    ]
  )
])

report_curltimeout="-"
if test -n "${with_curltimeout}"; then
  if test "${with_curltimeout}" == "yes"; then
    AC_DEFINE(OAUTH_CURL_TIMEOUT, 60)
    report_curltimeout="60"
  else 
    if test "${with_curltimeout}" -gt 0; then
    AC_DEFINE_UNQUOTED(OAUTH_CURL_TIMEOUT, [${with_curltimeout}])
    report_curltimeout=${with_curltimeout}
    fi
  fi
fi

AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)

dnl ** crypto/hash lib (OpenSSL or NSS)
AS_IF([test "${enable_builtinhash}" == "yes"], [
    AC_DEFINE(USE_BUILTIN_HASH, 1) USE_BUILTIN_HASH=1
    HASH_LIBS=""
    HASH_CFLAGS=""
    report_hash="built-in HMAC/SHA1 - no RSA"
    AC_MSG_NOTICE([
      using built-in HMAC/SHA1 hash algorithm without RSA/PK11 support.
      This option is not recommended for general use and should only
      be used on small devices (AVR, mircocontrollers) where neither
      NSS nor OpenSSL is available.
    ])
], [
  AS_IF([test "${enable_nss}" == "yes"], [
    PKG_CHECK_MODULES(NSS, nss, [ AC_DEFINE(USE_NSS, 1) USE_NSS=1 PC_REQ="$PC_REQ nss" ])
    HASH_LIBS=${NSS_LIBS}
    HASH_CFLAGS=${NSS_CFLAGS}
    report_hash="NSS"
  ], [
    AC_CHECK_HEADERS(openssl/hmac.h)
    if test -z "${HASH_LIBS}"; then
    HASH_LIBS="-lcrypto"
    fi
    if test -z "${HASH_CFLAGS}"; then
    HASH_CFLAGS=""
    fi
    report_hash="OpenSSL"
    PC_LIB="$PC_LIB ${HASH_LIBS}"
    PC_REQ="$PC_REQ libcrypto"
    AC_MSG_NOTICE([

    NOTE: OpenSSL is not compatible with GPL applications.
    Even if only linked with GPL code you are not allowed to distibute your app.
    However liboauth provides an exeption (to the GPL) to circumvent this issue
    (see README, src/hash.c). Nevertheless, double-check your licensing.
   
    liboauth itself is licensed under MIT license and comatible with the GPL. 

    Either way, you are probably better off using NSS (configure --enable-nss);
    future versions of liboauth will default to the Mozilla NSS.
     
    see http://people.gnome.org/~markmc/openssl-and-the-gpl.html
    ])
  ])
])

AC_SUBST(HASH_LIBS)
AC_SUBST(HASH_CFLAGS)

dnl *** doxygen ***
AC_ARG_VAR(DOXYGEN, Doxygen)
AC_PATH_PROG(DOXYGEN, doxygen, no)

if test "$DOXYGEN" != "no"; then
  DOXMAKE='run "make dox" to generate API html reference: doc/html/index.html'
fi

dnl *** graphviz ***
dnl (needed for Doxyfile.in)
AC_ARG_VAR(DOT, The 'dot' program from graphviz)
AC_PATH_PROG(DOT, dot, no)
if test "$DOT" != "no"; then
  HAVEDOT=YES
  DOTPATH=$( dirname "$DOT" )
else
  HAVEDOT=NO
fi
AC_SUBST(HAVEDOT)
AC_SUBST(DOTPATH)


dnl *** perl ***
dnl (needed for Doxyfile.in)
AC_ARG_VAR(PERL, Perl)
AC_PATH_PROG(PERL, perl, no)
if test "$PERL" == "no"; then
  AC_MSG_WARN([dude, where's your perl? doxygen will not like this!)])
fi

# PKGconfig oauth.pc.in
AC_SUBST(PC_REQ)
AC_SUBST(PC_LIB)


dnl *** output ***
subdirs="src doc tests"
AC_SUBST(subdirs)

AC_OUTPUT(Makefile src/Makefile doc/Makefile tests/Makefile liboauth.lsm oauth.pc Doxyfile doc/mainpage.dox)

AC_MSG_NOTICE([

 liboauth configured:
 -----------------------

  version:                $VERSION
  interface revision:     $VERSION_INFO
  hash/signature:         $report_hash
  http integration:       $report_curl
  libcurl-timeout:        $report_curltimeout
  generate documentation: $DOXYGEN
  installation prefix:    $prefix
  CFLAGS:                 $LIBOAUTH_CFLAGS $CFLAGS
  LDFLAGS:                $LIBOAUTH_LDFLAGS $LDFLAGS

 type "make" followed my "make install" as root.
 $DOXMAKE
])
