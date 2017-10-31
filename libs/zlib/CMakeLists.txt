file(GLOB SOURCES "src/*.h" "src/*.c" "src/zlib.def")
set(TARGET Zlib)
include(${CMAKE_SOURCE_DIR}/cmake/lib.cmake)