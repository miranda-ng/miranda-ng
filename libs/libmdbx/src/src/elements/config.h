/* This is CMake-template for libmdbx's config.h
 ******************************************************************************/

/* *INDENT-OFF* */
/* clang-format off */

/* #undef HAVE_VALGRIND_MEMCHECK_H */
/* #undef HAS_RELAXED_CONSTEXPR */

#define LTO_ENABLED
/* #undef MDBX_USE_VALGRIND */
/* #undef ENABLE_GPROF */
/* #undef ENABLE_GCOV */
/* #undef ENABLE_ASAN */
/* #undef MDBX_FORCE_ASSERTIONS */

/* Common */
#define MDBX_TXN_CHECKOWNER 1
#define MDBX_TXN_CHECKPID_AUTO
#ifndef MDBX_TXN_CHECKPID_AUTO
#define MDBX_TXN_CHECKPID 0
#endif
#define MDBX_BUILD_SHARED_LIBRARY 1
#define MDBX_LOCKING_AUTO
#ifndef MDBX_LOCKING_AUTO
/* #undef MDBX_LOCKING */
#endif
#define MDBX_TRUST_RTC_AUTO
#ifndef MDBX_TRUST_RTC_AUTO
#define MDBX_TRUST_RTC 0
#endif

/* Windows */
#define MDBX_CONFIG_MANUAL_TLS_CALLBACK 0
#define MDBX_AVOID_CRT 1

/* MacOS */
#define MDBX_OSX_SPEED_INSTEADOF_DURABILITY 0

/* POSIX */
#define MDBX_DISABLE_GNU_SOURCE 0
/* #undef MDBX_USE_OFDLOCKS_AUTO */
#ifndef MDBX_USE_OFDLOCKS_AUTO
#define MDBX_USE_OFDLOCKS 0
#endif

/* Build Info */
#define MDBX_BUILD_TIMESTAMP "2020-01-21T15:55:59Z"
#define MDBX_BUILD_TARGET "x86_64-Windows"
/* #undef MDBX_BUILD_CONFIG */
#define MDBX_BUILD_COMPILER "MSVC-19.24.28315.0"
#define MDBX_BUILD_FLAGS "LIBMDBX_EXPORTS"
#define MDBX_BUILD_SOURCERY 425d81103531494ef1989854f912bed8cbcb3acfa86c50ce374995c7edf1eb16_v0_6_0_1_g32db811

/* *INDENT-ON* */
/* clang-format on */
