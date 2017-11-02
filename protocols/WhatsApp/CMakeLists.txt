file(GLOB SOURCES "src/*.h" "src/WhatsAPI++/*.h" "src/*.cpp" "src/OpenSSL/*.c" "src/WhatsAPI++/*.cpp" "res/*.rc")
set(TARGET WhatsApp)
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)
add_subdirectory(proto_whatsapp)