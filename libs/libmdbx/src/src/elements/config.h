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
#define MDBX_BUILD_TIMESTAMP "2020-03-20T14:47:36Z"
#define MDBX_BUILD_TARGET "x86_64-Windows"
/* #undef MDBX_BUILD_CONFIG */
#define MDBX_BUILD_COMPILER "MSVC-19.25.28610.4"
#define MDBX_BUILD_FLAGS "LIBMDBX_EXPORTS"
#define MDBX_BUILD_SOURCERY b8a401ae08d0312d6e5365c29bb58adea2e44dbc7e3d808df4a0de2fefcc65ca_v0_6_0_45_gc20ba1e

/* *INDENT-ON* */
/* clang-format on */
