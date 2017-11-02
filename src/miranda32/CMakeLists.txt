file(GLOB SOURCES "src/*.h" "src/*.cpp" "res/*.rc")
if(CMAKE_CL_64)
	set(TARGET "Miranda64")
else()
	set(TARGET "Miranda32")
endif()
add_executable(${TARGET} ${SOURCES})
include_directories(.)
set_target_properties(${TARGET} PROPERTIES
	LINK_FLAGS "/SUBSYSTEM:WINDOWS"
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/$<CONFIG>"
)