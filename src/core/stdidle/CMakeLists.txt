file(GLOB STDIDLE_SOURCES "src/*.h" "src/*.cpp" "res/*.rc")
add_library(StdIdle SHARED ${STDIDLE_SOURCES})
include_directories(. ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/plugins/ExternalAPI)
set_target_properties(StdIdle PROPERTIES
	LINK_FLAGS "/SUBSYSTEM:WINDOWS"
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/$<CONFIG>/Core"
)
target_link_libraries(StdIdle winmm.lib Wtsapi32.lib netapi32.lib pdh.lib shlwapi.lib Strmiids.lib gdiplus.lib dbghelp.lib Setupapi.lib msimg32.lib comctl32.lib ws2_32.lib UxTheme.lib Iphlpapi.lib)