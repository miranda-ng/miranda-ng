include_directories(.)
file(GLOB SOURCES "src/*.h" "src/*.cpp" "res/*.rc")
add_library(${TARGET} SHARED ${SOURCES})
set_target_properties(${TARGET} PROPERTIES
	LINK_FLAGS "/SUBSYSTEM:WINDOWS"
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/$<CONFIG>/Core"
)
target_link_libraries(${TARGET} ${COMMON_LIBS})