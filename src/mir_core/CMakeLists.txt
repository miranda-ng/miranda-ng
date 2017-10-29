file(GLOB MIR_CORE_SOURCES "src/*.h" "src/*.cpp" "src/mir_core.def")
add_library(mir_core SHARED ${MIR_CORE_SOURCES})
include_directories(${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(mir_core PRIVATE MIR_CORE_EXPORTS)
set_target_properties(mir_core PROPERTIES
	LINK_FLAGS "/SUBSYSTEM:WINDOWS"
	SUFFIX ".mir"
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/$<CONFIG>/Libs"
)
target_link_libraries(mir_core comctl32.lib UxTheme.lib Shlwapi.lib PowrProf.lib Rasapi32.lib)