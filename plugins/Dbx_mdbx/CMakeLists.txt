file(GLOB SOURCES "src/*.h" "src/*.cpp" "res/*.rc")
set(TARGET Dbx_mdbx)

include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)
target_link_libraries(${TARGET} libmdbx)