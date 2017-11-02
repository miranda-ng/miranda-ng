file(GLOB SOURCES "src/*.h" "src/api/*.h" "src/*.cpp" "res/*.rc")
set(TARGET Slack)
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)