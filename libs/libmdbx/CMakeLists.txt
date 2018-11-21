file(GLOB SOURCES "src/src/lck-windows.c" "src/src/mdbx.c"
"src/src/osal.c" "src/src/version.c" "src/*.h")
set(TARGET libmdbx)
include(${CMAKE_SOURCE_DIR}/cmake/lib.cmake)

set_target_properties(${TARGET} PROPERTIES COMPILE_DEFINITIONS "MDBX_CONFIG_MANUAL_TLS_CALLBACK=1;MDBX_DEBUG=1;LIBMDBX_EXPORTS")