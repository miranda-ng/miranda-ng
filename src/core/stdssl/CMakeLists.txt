set(TARGET StdSSL)
include(${CMAKE_SOURCE_DIR}/cmake/core.cmake)
target_link_libraries(${TARGET} secur32.lib crypt32.lib)