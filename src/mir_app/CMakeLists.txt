file(GLOB MIR_APP_SOURCES "src/*.h" "src/*.cpp" "src/mir_app.def" "res/*.rc")
add_library(mir_app SHARED ${MIR_APP_SOURCES})
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/plugins/ExternalAPI)
target_compile_definitions(mir_app PRIVATE MIR_APP_EXPORTS)
set_target_properties(mir_app PROPERTIES
	LINK_FLAGS "/SUBSYSTEM:WINDOWS"
	SUFFIX ".mir"
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/$<CONFIG>/Libs"
)
target_link_libraries(mir_app winmm.lib Wtsapi32.lib netapi32.lib pdh.lib shlwapi.lib Strmiids.lib gdiplus.lib dbghelp.lib Setupapi.lib msimg32.lib comctl32.lib ws2_32.lib UxTheme.lib Iphlpapi.lib Zlib.lib)