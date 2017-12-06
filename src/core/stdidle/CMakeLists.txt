set(TARGET StdIdle)
include(${CMAKE_SOURCE_DIR}/cmake/core.cmake)
target_link_libraries(${TARGET} Wtsapi32.lib)