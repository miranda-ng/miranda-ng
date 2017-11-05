file(GLOB SOURCES "src/*.h" "src/*.cpp" "res/*.rc")
set(TARGET Dummy)
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)
add_subdirectory(proto_dummy)