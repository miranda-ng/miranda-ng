file(GLOB SOURCES "src/*.h" "src/*.cpp" "res/*.rc")
set(TARGET TopToolBar)
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)
target_link_libraries(${TARGET} comctl32.lib)