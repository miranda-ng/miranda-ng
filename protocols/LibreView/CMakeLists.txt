file(GLOB SOURCES "src/*.h" "src/*.cpp" "res/*.rc")
set(TARGET LibreView)
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)
target_link_libraries(${TARGET} libjson)
