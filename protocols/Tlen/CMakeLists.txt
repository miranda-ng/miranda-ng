file(GLOB SOURCES "src/*.h" "src/crypto/polarssl/*.h" "src/codec/*.h" "src/*.cpp" "src/crypto/*.c" "src/codec/*.c" "res/*.rc")
set(TARGET Tlen)
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)
add_subdirectory(proto_tlen)