/* This is CMake-template for libmdbx's version.c
 ******************************************************************************/

#include "src/internals.h"

#if MDBX_VERSION_MAJOR != 0 ||                             \
    MDBX_VERSION_MINOR != 8
#error "API version mismatch! Had `git fetch --tags` done?"
#endif

static const char sourcery[] = STRINGIFY(MDBX_BUILD_SOURCERY);

__dll_export
#ifdef __attribute_used__
    __attribute_used__
#elif defined(__GNUC__) || __has_attribute(__used__)
    __attribute__((__used__))
#endif
#ifdef __attribute_externally_visible__
        __attribute_externally_visible__
#elif (defined(__GNUC__) && !defined(__clang__)) ||                            \
    __has_attribute(__externally_visible__)
    __attribute__((__externally_visible__))
#endif
    const mdbx_version_info mdbx_version = {
        0,
        8,
        2,
        2,
        {"2020-07-08T16:46:29+03:00", "91ba90ae5964c49852b2fe8ec5c6733e10d0414c", "4fffd033615ec45fd2669edcf26abd4134f76b94",
         "v0.8.2-2-g4fffd03"},
        sourcery};

__dll_export
#ifdef __attribute_used__
    __attribute_used__
#elif defined(__GNUC__) || __has_attribute(__used__)
    __attribute__((__used__))
#endif
#ifdef __attribute_externally_visible__
        __attribute_externally_visible__
#elif (defined(__GNUC__) && !defined(__clang__)) ||                            \
    __has_attribute(__externally_visible__)
    __attribute__((__externally_visible__))
#endif
    const char *const mdbx_sourcery_anchor = sourcery;
