file(GLOB SOURCES "cmstub.cpp")
set(TARGET cmstub)
add_library(${TARGET} ${SOURCES})
set_target_properties(${TARGET} PROPERTIES
	LINK_FLAGS "/SUBSYSTEM:WINDOWS"
)