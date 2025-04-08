set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "")

file(GLOB SOURCES "src/*.h" "res/*.rc"
"src/corp.cpp"
"src/event.cpp"
"src/options.cpp"
"src/packet.cpp"
"src/protocol.cpp"
"src/services.cpp"
"src/socket.cpp"
"src/transfer.cpp"
"src/user.cpp"
)
set(TARGET ICQCorp)
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)
add_subdirectory(proto_icq)
