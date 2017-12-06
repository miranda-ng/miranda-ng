set(TARGET StdClist)
include(${CMAKE_SOURCE_DIR}/cmake/core.cmake)
target_link_libraries(${TARGET} comctl32.lib UxTheme.lib Shlwapi.lib)