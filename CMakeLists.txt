cmake_minimum_required (VERSION 2.6)
 
project (Miranda)

add_definitions(-D_UNICODE -DUNICODE -D_CRT_SECURE_NO_WARNINGS)

link_directories("${CMAKE_SOURCE_DIR}/libs/win32")
link_directories("${PROJECT_BINARY_DIR}/libs/zlib")

add_subdirectory(libs)
add_subdirectory(src)
