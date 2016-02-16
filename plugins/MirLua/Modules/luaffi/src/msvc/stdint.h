//  boost cstdint.hpp header file  ------------------------------------------//

//  (C) Copyright Beman Dawes 1999. 
//  (C) Copyright Jens Mauer 2001  
//  (C) Copyright John Maddock 2001 
//  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/integer for documentation.

//  Revision History
//   31 Oct 01  use BOOST_HAS_LONG_LONG to check for "long long" (Jens M.)
//   16 Apr 01  check LONGLONG_MAX when looking for "long long" (Jens Maurer)
//   23 Jan 01  prefer "long" over "int" for int32_t and intmax_t (Jens Maurer)
//   12 Nov 00  Merged <boost/stdint.h> (Jens Maurer)
//   23 Sep 00  Added INTXX_C macro support (John Maddock).
//   22 Sep 00  Better 64-bit support (John Maddock)
//   29 Jun 00  Reimplement to avoid including stdint.h within namespace boost
//    8 Aug 99  Initial version (Beman Dawes)


#ifndef STDINT_H
#define STDINT_H

#ifndef UNDER_CE
#include <crtdefs.h>
#endif

#include <limits.h>

//  These are fairly safe guesses for some 16-bit, and most 32-bit and 64-bit
//  platforms.  For other systems, they will have to be hand tailored.
//
//  Because the fast types are assumed to be the same as the undecorated types,
//  it may be possible to hand tailor a more efficient implementation.  Such
//  an optimization may be illusionary; on the Intel x86-family 386 on, for
//  example, byte arithmetic and load/stores are as fast as "int" sized ones.

//  8-bit types  ------------------------------------------------------------//

# if UCHAR_MAX == 0xff
     typedef signed char     int8_t;
     typedef signed char     int_least8_t;
     typedef signed char     int_fast8_t;
     typedef unsigned char   uint8_t;
     typedef unsigned char   uint_least8_t;
     typedef unsigned char   uint_fast8_t;
# else
#    error defaults not correct; you must hand modify boost/cstdint.hpp
# endif

//  16-bit types  -----------------------------------------------------------//

# if USHRT_MAX == 0xffff
#  if defined(__crayx1)
     // The Cray X1 has a 16-bit short, however it is not recommend
     // for use in performance critical code.
     typedef short           int16_t;
     typedef short           int_least16_t;
     typedef int             int_fast16_t;
     typedef unsigned short  uint16_t;
     typedef unsigned short  uint_least16_t;
     typedef unsigned int    uint_fast16_t;
#  else
     typedef short           int16_t;
     typedef short           int_least16_t;
     typedef short           int_fast16_t;
     typedef unsigned short  uint16_t;
     typedef unsigned short  uint_least16_t;
     typedef unsigned short  uint_fast16_t;
#  endif
# elif (USHRT_MAX == 0xffffffff) && defined(CRAY)
     // no 16-bit types on Cray:
     typedef short           int_least16_t;
     typedef short           int_fast16_t;
     typedef unsigned short  uint_least16_t;
     typedef unsigned short  uint_fast16_t;
# else
#    error defaults not correct; you must hand modify boost/cstdint.hpp
# endif

//  32-bit types  -----------------------------------------------------------//

# if ULONG_MAX == 0xffffffff
     typedef long            int32_t;
     typedef long            int_least32_t;
     typedef long            int_fast32_t;
     typedef unsigned long   uint32_t;
     typedef unsigned long   uint_least32_t;
     typedef unsigned long   uint_fast32_t;
# elif UINT_MAX == 0xffffffff
     typedef int             int32_t;
     typedef int             int_least32_t;
     typedef int             int_fast32_t;
     typedef unsigned int    uint32_t;
     typedef unsigned int    uint_least32_t;
     typedef unsigned int    uint_fast32_t;
# else
#    error defaults not correct; you must hand modify boost/cstdint.hpp
# endif

//  64-bit types + intmax_t and uintmax_t  ----------------------------------//

     //
     // we have Borland/Intel/Microsoft __int64:
     //
     typedef __int64             intmax_t;
     typedef unsigned __int64    uintmax_t;
     typedef __int64             int64_t;
     typedef __int64             int_least64_t;
     typedef __int64             int_fast64_t;
     typedef unsigned __int64    uint64_t;
     typedef unsigned __int64    uint_least64_t;
     typedef unsigned __int64    uint_fast64_t;



/****************************************************

Macro definition section:

Define various INTXX_C macros only if
__STDC_CONSTANT_MACROS is defined.

Undefine the macros if __STDC_CONSTANT_MACROS is
not defined and the macros are (cf <cassert>).

Added 23rd September 2000 (John Maddock).
Modified 11th September 2001 to be excluded when
BOOST_HAS_STDINT_H is defined (John Maddock).

******************************************************/

#if defined(__STDC_CONSTANT_MACROS) || !defined(__cplusplus)
//
// Borland/Intel/Microsoft compilers have width specific suffixes:
//
#  define INT8_C(value)     value##i8
#  define INT16_C(value)    value##i16
#  define INT32_C(value)    value##i32
#  define INT64_C(value)    value##i64
#  define UINT8_C(value)    value##ui8
#  define UINT16_C(value)   value##ui16
#  define UINT32_C(value)   value##ui32
#  define UINT64_C(value)   value##ui64
#  define INTMAX_C(value)   value##i64
#  define UINTMAX_C(value)  value##ui64

#endif // __STDC_CONSTANT_MACROS_DEFINED etc.

#if defined(__STDC_LIMIT_MACROS) || !defined(__cplusplus)
#  define INT8_MIN          INT8_C(-127)-1
#  define INT8_MAX          INT8_C(127)
#  define INT16_MIN         INT16_C(-32767)-1
#  define INT16_MAX         INT16_C(32767)
#  define INT32_MIN         INT32_C(-2147483647)-1
#  define INT32_MAX         INT32_C(2147483647)
#  define INT64_MAX         INT64_C(9223372036854775807)
#  define UINT8_MAX         UINT8_C(255)
#  define UINT16_MAX        UINT16_C(65535)
#  define UINT32_MAX        UINT32_C(4294967295)
#  define UINT64_MAX        UINT64_C(18446744073709551615)
#endif

#ifdef UNDER_CE
typedef unsigned long uintptr_t;
typedef long intptr_t;
#endif



#endif // BOOST_CSTDINT_HPP

