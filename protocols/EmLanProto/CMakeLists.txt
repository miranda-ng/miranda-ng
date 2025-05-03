set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "")

file(GLOB SOURCES "src/*.h" "src/*.cpp" "res/*.rc")
set(TARGET EmLanProto)
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)
add_subdirectory(proto_EmLanProto)
