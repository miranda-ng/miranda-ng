# CMP

[![Build Status](https://travis-ci.org/camgunz/cmp.svg?branch=master)](https://travis-ci.org/camgunz/cmp) [![Coverage Status](https://coveralls.io/repos/github/camgunz/cmp/badge.svg?branch=develop)](https://coveralls.io/github/camgunz/cmp?branch=develop)

CMP is a C implementation of the MessagePack serialization format.  It
currently implements version 5 of the [MessagePack
Spec](http://github.com/msgpack/msgpack/blob/master/spec.md).

CMP's goal is to be lightweight and straightforward, forcing nothing on the
programmer.

## License

While I'm a big believer in the GPL, I license CMP under the MIT license.

## Example Usage

The following examples use a file as the backend, and are modeled after the
examples included with the msgpack-c project.

```C
#include <stdio.h>
#include <stdlib.h>

#include "cmp.h"

static bool read_bytes(void *data, size_t sz, FILE *fh) {
    return fread(data, sizeof(uint8_t), sz, fh) == (sz * sizeof(uint8_t));
}

static bool file_reader(cmp_ctx_t *ctx, void *data, size_t limit) {
    return read_bytes(data, limit, (FILE *)ctx->buf);
}

static bool file_skipper(cmp_ctx_t *ctx, size_t count) {
    return fseek((FILE *)ctx->buf, count, SEEK_CUR);
}

static size_t file_writer(cmp_ctx_t *ctx, const void *data, size_t count) {
    return fwrite(data, sizeof(uint8_t), count, (FILE *)ctx->buf);
}

static void error_and_exit(const char *msg) {
    fprintf(stderr, "%s\n\n", msg);
    exit(EXIT_FAILURE);
}

int main(void) {
    FILE *fh = NULL;
    cmp_ctx_t cmp = {0};
    uint32_t array_size = 0;
    uint32_t str_size = 0;
    char hello[6] = {0};
    char message_pack[12] = {0};

    fh = fopen("cmp_data.dat", "w+b");

    if (fh == NULL) {
        error_and_exit("Error opening data.dat");
    }

    cmp_init(&cmp, fh, file_reader, file_skipper, file_writer);

    if (!cmp_write_array(&cmp, 2)) {
        error_and_exit(cmp_strerror(&cmp));
    }

    if (!cmp_write_str(&cmp, "Hello", 5)) {
        error_and_exit(cmp_strerror(&cmp));
    }

    if (!cmp_write_str(&cmp, "MessagePack", 11)) {
        error_and_exit(cmp_strerror(&cmp));
    }

    rewind(fh);

    if (!cmp_read_array(&cmp, &array_size)) {
        error_and_exit(cmp_strerror(&cmp));
    }

    /* You can read the str byte size and then read str bytes... */

    if (!cmp_read_str_size(&cmp, &str_size)) {
        error_and_exit(cmp_strerror(&cmp));
    }

    if (str_size > (sizeof(hello) - 1)) {
        error_and_exit("Packed 'hello' length too long\n");
    }

    if (!read_bytes(hello, str_size, fh)) {
        error_and_exit(cmp_strerror(&cmp));
    }

    /*
     * ...or you can set the maximum number of bytes to read and do it all in
     * one call
     */

    str_size = sizeof(message_pack);
    if (!cmp_read_str(&cmp, message_pack, &str_size)) {
        error_and_exit(cmp_strerror(&cmp));
    }

    printf("Array Length: %u.\n", array_size);
    printf("[\"%s\", \"%s\"]\n", hello, message_pack);

    fclose(fh);

    return EXIT_SUCCESS;
}
```

## Advanced Usage

See the `examples` folder.

## Fast, Lightweight, Flexible, and Robust

CMP uses no internal buffers; conversions, encoding and decoding are done on
the fly.

CMP's source and header file together are ~4k LOC.

CMP makes no heap allocations.

CMP uses standardized types rather than declaring its own, and it depends only
on `stdbool.h`, `stdint.h` and `string.h`.

CMP is written using C89 (ANSI C), aside, of course, from its use of
fixed-width integer types and `bool`.

On the other hand, CMP's test suite requires C99.

CMP only requires the programmer supply a read function, a write function, and
an optional skip function.  In this way, the programmer can use CMP on memory,
files, sockets, etc.

CMP is portable.  It uses fixed-width integer types, and checks the endianness
of the machine at runtime before swapping bytes (MessagePack is big-endian).

CMP provides a fairly comprehensive error reporting mechanism modeled after
`errno` and `strerror`.

CMP is thread aware; while contexts cannot be shared between threads, each
thread may use its own context freely.

CMP is tested using the MessagePack test suite as well as a large set of custom
test cases.  Its small test program is compiled with clang using `-Wall -Werror
-Wextra ...` along with several other flags, and generates no compilation
errors in either clang or GCC.

CMP's source is written as readably as possible, using explicit, descriptive
variable names and a consistent, clear style.

CMP's source is written to be as secure as possible.  Its testing suite checks
for invalid values, and data is always treated as suspect before it passes
validation.

CMP's API is designed to be clear, convenient and unsurprising.  Strings are
null-terminated, binary data is not, error codes are clear, and so on.

CMP provides optional backwards compatibility for use with other MessagePack
implementations that only implement version 4 of the spec.

## Building

There is no build system for CMP.  The programmer can drop `cmp.c` and `cmp.h`
in their source tree and modify as necessary.  No special compiler settings are
required to build it, and it generates no compilation errors in either clang or
gcc.

## Versioning

CMP's versions are single integers.  I don't use semantic versioning because
I don't guarantee that any version is completely compatible with any other.  In
general, semantic versioning provides a false sense of security.  You should be
evaluating compatibility yourself, not relying on some stranger's versioning
convention.

## Stability

I only guarantee stability for versions released on
[the releases page](../../releases).  While rare, both `master` and `develop`
branches may have errors or mismatched versions.

## Backwards Compatibility

Version 4 of the MessagePack spec has no `BIN` type, and provides no `STR8`
marker.  In order to remain backwards compatible with version 4 of MessagePack,
do the following:

Avoid these functions:

  - `cmp_write_bin`
  - `cmp_write_bin_marker`
  - `cmp_write_str8_marker`
  - `cmp_write_str8`
  - `cmp_write_bin8_marker`
  - `cmp_write_bin8`
  - `cmp_write_bin16_marker`
  - `cmp_write_bin16`
  - `cmp_write_bin32_marker`
  - `cmp_write_bin32`

Use these functions in lieu of their v5 counterparts:

  - `cmp_write_str_marker_v4` instead of `cmp_write_str_marker`
  - `cmp_write_str_v4` instead of `cmp_write_str`
  - `cmp_write_object_v4` instead of `cmp_write_object`

## Disabling Floating Point Operations

Thanks to [tdragon](https://github.com/tdragon) it's possible to disable
floating point operations in CMP by defining `CMP_NO_FLOAT`. No floating point
functionality will be included.  Fair warning: this changes the ABI.

## Setting Endianness at Compile Time

CMP will honor `WORDS_BIGENDIAN`. If defined to `0` it will convert data
to/from little-endian format when writing/reading. If defined to `1` it won't.
If not defined, CMP will check at runtime.
