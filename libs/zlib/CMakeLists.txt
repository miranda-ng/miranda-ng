file(GLOB ZLIB_SOURCES "src/*.h" "src/*.c" "src/zlib.def")
add_library(Zlib SHARED ${ZLIB_SOURCES})
set_target_properties(Zlib PROPERTIES SUFFIX ".mir")