file(GLOB DBX_MMAP_SOURCES "src/*.h" "src/*.cpp" "src/dbtool/*.cpp" "res/*.rc")
add_library(Dbx_mmap SHARED ${DBX_MMAP_SOURCES})
include_directories(. ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/plugins/ExternalAPI)
set_target_properties(Dbx_mmap PROPERTIES
	LINK_FLAGS "/SUBSYSTEM:WINDOWS"
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/$<CONFIG>/Plugins"
)
target_link_libraries(Dbx_mmap winmm.lib Wtsapi32.lib netapi32.lib pdh.lib shlwapi.lib Strmiids.lib gdiplus.lib dbghelp.lib Setupapi.lib msimg32.lib comctl32.lib ws2_32.lib UxTheme.lib Iphlpapi.lib Zlib.lib)