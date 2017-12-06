set(TARGET StdFile)
include(${CMAKE_SOURCE_DIR}/cmake/core.cmake)
target_link_libraries(${TARGET} Shlwapi.lib)