file(GLOB SOURCES "src/*.h" "src/*.cpp" "src/mir_app.def" "res/*.rc")
set(TARGET mir_app)
include(${CMAKE_SOURCE_DIR}/cmake/lib.cmake)

set_target_properties(${TARGET} PROPERTIES COMPILE_DEFINITIONS "MIR_APP_EXPORTS")
target_link_libraries(${TARGET} Zlib)