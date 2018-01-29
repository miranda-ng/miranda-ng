file(GLOB SOURCES "src/*.h" "src/*.cpp")
set(TARGET AdvaImg)
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)

target_link_libraries(${TARGET} FreeImage)