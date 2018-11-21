file(GLOB SOURCES "src/*.h" "src/*.c")
set(TARGET sqlite3)
include(${CMAKE_SOURCE_DIR}/cmake/lib.cmake)
set_target_properties(${TARGET} PROPERTIES COMPILE_DEFINITIONS "_HAVE_SQLITE_CONFIG_H")