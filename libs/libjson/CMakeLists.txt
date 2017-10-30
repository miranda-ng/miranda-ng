file(GLOB SOURCES "src/*.h" "src/*.cpp" "src/libjson.def")
set(TARGET libjson)
include(${CMAKE_SOURCE_DIR}/cmake/lib.cmake)

set_target_properties(${TARGET} PROPERTIES COMPILE_DEFINITIONS "LIBJSON_EXPORTS")