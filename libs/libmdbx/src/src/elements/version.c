/* This is CMake-template for libmdbx's version.c
 ******************************************************************************/

#include "internals.h"

#if MDBX_VERSION_MAJOR != 0 ||                             \
    MDBX_VERSION_MINOR != 4
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
    4,
    0,
    1692,
    {"2019-12-05T01:43:57+03:00", "456d64170249e6108e90da02648de8c7707e2ed0", "3dccbb25a7710d4eff5fb136182b7bb0abfc4914",
     "v0.4.0-12-g3dccbb2"},
    sourcery };

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
