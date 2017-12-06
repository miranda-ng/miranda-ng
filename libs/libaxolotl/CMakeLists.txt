file(GLOB SOURCES "src/*.h" "src/*.c"
"src/protobuf-c/*.c" "src/protobuf-c/*.h"
"src/curve25519/*.c" "src/curve25519/*.h"
"src/curve25519/ed25519/*.c" "src/curve25519/ed25519/*.h"
"src/curve25519/ed25519/nacl_sha512/*.c" "src/curve25519/ed25519/nacl_includes/*.h"
"src/curve25519/ed25519/additions/*.c" "src/curve25519/ed25519/additions/*.h"
"src/signal.def")
include_directories("src" "src/protobuf-c" "src/curve25519" "src/curve25519/ed25519"
"src/curve25519/ed25519/nacl_includes" "src/curve25519/ed25519/additions")
set(TARGET libaxolotl)
include(${CMAKE_SOURCE_DIR}/cmake/lib.cmake)