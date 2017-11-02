file(GLOB SOURCES "src/*.h" "src/api/*.h" "src/*.cpp" "res/*.rc")
set(TARGET Steam)
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)
add_subdirectory(proto_steam)