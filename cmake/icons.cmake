file(GLOB SOURCES "src/*.h" "res/*.rc")
add_library(${TARGET} SHARED ${SOURCES})
set_target_properties(${TARGET} PROPERTIES
	LINK_FLAGS "/SUBSYSTEM:WINDOWS /NOENTRY"
	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/$<CONFIG>/Icons"
)
target_link_libraries(${TARGET} ${COMMON_LIBS})