cmake_minimum_required (VERSION 3.11)
 
project (Miranda)

if(CMAKE_CL_64)
	set(PREBUILT_DIR ${CMAKE_SOURCE_DIR}/libs/win64)
else()
	set(PREBUILT_DIR ${CMAKE_SOURCE_DIR}/libs/win32)
endif()

set(LIB_CRYPTO_DEBUG ${PREBUILT_DIR}/libcrypto.lib CACHE FILEPATH "libcrypto debug path")
set(LIB_CRYPTO_RELEASE ${PREBUILT_DIR}/libcrypto.lib CACHE FILEPATH "libcrypto release path")
set(LIB_SSL_DEBUG ${PREBUILT_DIR}/libssl.lib CACHE FILEPATH "libssl debug path")
set(LIB_SSL_RELEASE ${PREBUILT_DIR}/libssl.lib CACHE FILEPATH "libssl release path")

find_package(OpenSSL)

set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "_UNICODE;UNICODE;_CRT_SECURE_NO_WARNINGS;_CRT_NONSTDC_NO_WARNINGS")
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/msapi ${CMAKE_SOURCE_DIR}/plugins/ExternalAPI)

add_subdirectory(build)
add_subdirectory(libs)
add_subdirectory(src)
add_subdirectory(plugins)
add_subdirectory(protocols)