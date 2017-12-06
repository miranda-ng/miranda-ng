set(TARGET StdSSL)
include(${CMAKE_SOURCE_DIR}/cmake/core.cmake)
target_link_libraries(${TARGET} ws2_32.lib secur32.lib crypt32.lib)