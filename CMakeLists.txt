cmake_minimum_required (VERSION 2.8)
 
project (Miranda)

if(CMAKE_CL_64)
	set(PREBUILT_DIR ${CMAKE_SOURCE_DIR}/libs/win64)
else()
	set(PREBUILT_DIR ${CMAKE_SOURCE_DIR}/libs/win32)
endif()

set(LIB_EAY_DEBUG ${PREBUILT_DIR}/libeay32.lib CACHE FILEPATH "libeay debug path")
set(LIB_EAY_RELEASE ${PREBUILT_DIR}/libeay32.lib CACHE FILEPATH "libeay release path")
set(SSL_EAY_DEBUG ${PREBUILT_DIR}/ssleay32.lib CACHE FILEPATH "ssleay debug path")
set(SSL_EAY_RELEASE ${PREBUILT_DIR}/ssleay32.lib CACHE FILEPATH "ssleay release path")

find_package(OpenSSL)

set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "_UNICODE;UNICODE;_CRT_SECURE_NO_WARNINGS;_CRT_NONSTDC_NO_WARNINGS")
link_directories(${PREBUILT_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/msapi ${CMAKE_SOURCE_DIR}/plugins/ExternalAPI)

add_subdirectory(build)
add_subdirectory(libs)
add_subdirectory(src)
add_subdirectory(plugins)
add_subdirectory(protocols)