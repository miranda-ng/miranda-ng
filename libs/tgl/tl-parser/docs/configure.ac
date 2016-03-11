#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([tl-parser], [1.0])

m4_include([m4-ax_gcc_builtin.m4])

AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# We only need the symbol 'crc32'. Stay on the safe side by checking a more specific symbol.
AC_CHECK_LIB([z], [crc32_combine])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h mach/mach.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([clock_gettime memset strdup])

AX_GCC_BUILTIN(__builtin_bswap32)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
