/* This is CMake-template for libmdbx's config.h
 ******************************************************************************/

/* *INDENT-OFF* */
/* clang-format off */

#define LTO_ENABLED
/* #undef MDBX_USE_VALGRIND */
/* #undef ENABLE_GPROF */
/* #undef ENABLE_GCOV */
/* #undef ENABLE_ASAN */
/* #undef MDBX_FORCE_ASSERTIONS */

/* Common */
#define MDBX_HUGE_TRANSACTIONS 0
#define MDBX_TXN_CHECKOWNER 1
#define MDBX_TXN_CHECKPID_AUTO
#ifndef MDBX_TXN_CHECKPID_AUTO
#define MDBX_TXN_CHECKPID 0
#endif
#define MDBX_LOCKING_AUTO
#ifndef MDBX_LOCKING_AUTO
/* #undef MDBX_LOCKING */
#endif
#define MDBX_TRUST_RTC_AUTO
#ifndef MDBX_TRUST_RTC_AUTO
#define MDBX_TRUST_RTC 0
#endif

/* Windows */
#define MDBX_CONFIG_MANUAL_TLS_CALLBACK 1
#define MDBX_AVOID_CRT 0

/* MacOS & iOS */
#define MDBX_OSX_SPEED_INSTEADOF_DURABILITY 0

/* POSIX */
#define MDBX_DISABLE_GNU_SOURCE 0
/* #undef MDBX_USE_OFDLOCKS_AUTO */
#ifndef MDBX_USE_OFDLOCKS_AUTO
#define MDBX_USE_OFDLOCKS 0
#endif

/* Build Info */
#define MDBX_BUILD_TIMESTAMP "2020-06-14T10:50:19Z"
#define MDBX_BUILD_TARGET "x86_64-Windows"
/* #undef MDBX_BUILD_TYPE */
#define MDBX_BUILD_COMPILER "MSVC-19.26.28806.0"
#define MDBX_BUILD_FLAGS "/DWIN32 /D_WINDOWS /W3 /EHsc /Gy /W4 /utf-8 /WX /GL MDBX_BUILD_SHARED_LIBRARY=0"
#define MDBX_BUILD_SOURCERY d9a19ecb1e6bd3a67910c162b2e9c2dc476d665c134d041af742fa0196b19a3e_v0_8_1_1_g0c5496d

/* *INDENT-ON* */
/* clang-format on */
