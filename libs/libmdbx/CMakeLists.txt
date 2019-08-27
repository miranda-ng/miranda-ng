file(GLOB SOURCES subtree/mdbx.h subtree/src/lck-windows.c subtree/src/mdbx.c subtree/src/osal.c subtree/src/version.c "subtree/src/*.h")
set(TARGET libmdbx)
include(${CMAKE_SOURCE_DIR}/cmake/lib.cmake)

set_target_properties(${TARGET} PROPERTIES COMPILE_DEFINITIONS "MDBX_CONFIG_MANUAL_TLS_CALLBACK=1;LIBMDBX_EXPORTS")
