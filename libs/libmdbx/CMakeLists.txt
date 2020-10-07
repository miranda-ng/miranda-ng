file(GLOB SOURCES "src/mdbx.h" "src/mdbx.c")
set(TARGET libmdbx)
include(${CMAKE_SOURCE_DIR}/cmake/lib.cmake)

set_target_properties(${TARGET} PROPERTIES COMPILE_DEFINITIONS "MDBX_BUILD_SHARED_LIBRARY=1;LIBMDBX_EXPORTS=1")
