file(GLOB SOURCES "src/*.h" "src/*.cpp" "src/dbtool/*.cpp" "res/*.rc"
"src/libmdbx/src/lck-windows.c" "src/libmdbx/src/mdbx.c" "src/libmdbx/src/osal.c"
"src/libmdbx/src/version.c" "src/libmdbx/mdbx.h")
set(TARGET Dbx_mdbx)

include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)