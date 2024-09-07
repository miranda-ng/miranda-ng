file(GLOB SOURCES "src/*.h" "src/*.cpp" "src/dbtool/*.cpp" "res/*.rc")
set(TARGET Dbx_mmap)

# Just a workaround. Better to fix plugins that do not compile with this option.
# Even Microsoft recommends to use standard exception handling.
string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)