file(GLOB MIRANDA32_SOURCES "src/*.h" "src/*.cpp" "res/*.rc")
add_executable(miranda32 ${MIRANDA32_SOURCES})
include_directories(.)
set_target_properties(miranda32 PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")