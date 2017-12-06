set(TARGET StdUserInfo)
include(${CMAKE_SOURCE_DIR}/cmake/core.cmake)
target_link_libraries(${TARGET} UxTheme.lib ws2_32.lib)