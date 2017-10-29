file(GLOB SOURCES "src/*.h" "src/*.cpp" "src/dbtool/*.cpp" "res/*.rc")
set(TARGET Dbx_mmap)
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)