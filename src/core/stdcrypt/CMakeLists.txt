set(TARGET StdCrypt)
include(${CMAKE_SOURCE_DIR}/cmake/core.cmake)
target_link_libraries(${TARGET} Zlib)