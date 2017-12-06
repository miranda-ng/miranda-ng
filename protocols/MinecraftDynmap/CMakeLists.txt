file(GLOB SOURCES "src/*.h" "src/*.cpp" "res/*.rc" "${CMAKE_SOURCE_DIR}/utils/std_string_utils.cpp")
set(TARGET MinecraftDynmap)
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)
add_subdirectory(proto_minecraftdynmap)