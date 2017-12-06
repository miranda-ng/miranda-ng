file(GLOB SOURCES "src/*.h" "src/*.cpp" "res/*.rc")
set(TARGET AIM)
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)
add_subdirectory(proto_aim)