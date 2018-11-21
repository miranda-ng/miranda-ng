file(GLOB SOURCES "src/*.h" "src/*.cpp" "res/*.rc")
set(TARGET Dbx_sqlite)

include_directories(${CMAKE_SOURCE_DIR}/libs/sqlite3/src)
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)
target_link_libraries(${TARGET} sqlite3)