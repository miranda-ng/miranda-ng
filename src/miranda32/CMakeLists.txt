file(GLOB SOURCES "src/*.h" "src/*.cpp" "res/*.rc")
add_executable(miranda32 ${SOURCES})
include_directories(.)
set_target_properties(miranda32 PROPERTIES
	LINK_FLAGS "/SUBSYSTEM:WINDOWS"
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/$<CONFIG>"
)