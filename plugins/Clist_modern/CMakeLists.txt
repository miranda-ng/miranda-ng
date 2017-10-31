file(GLOB SOURCES "src/*.h" "src/*.cpp" "res/*.rc")
set(TARGET Clist_modern)
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)
add_subdirectory(icons_pack)