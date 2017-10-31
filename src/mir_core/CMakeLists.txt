file(GLOB SOURCES "src/*.h" "src/*.cpp" "src/mir_core.def")
set(TARGET mir_core)
include(${CMAKE_SOURCE_DIR}/cmake/lib.cmake)

set_target_properties(${TARGET} PROPERTIES COMPILE_DEFINITIONS "MIR_CORE_EXPORTS")