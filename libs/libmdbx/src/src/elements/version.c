/* This is CMake-template for libmdbx's version.c
 ******************************************************************************/

#include "internals.h"

#if MDBX_VERSION_MAJOR != 0 ||                             \
    MDBX_VERSION_MINOR != 5
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
        5,
        0,
        1879,
        {"2020-01-07T22:30:21+03:00", "f982437718fa106bde7985a976752c08e95599a8", "3ffcc1722de83ce4cd5a37772d3b41e8aea02042",
         "v0.5.0-10-g3ffcc17"},
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
