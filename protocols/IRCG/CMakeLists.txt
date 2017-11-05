file(GLOB SOURCES "src/*.h" "src/*.cpp" "res/*.rc")
set(TARGET IRC)
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)
add_subdirectory(proto_irc)