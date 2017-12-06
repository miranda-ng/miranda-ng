file(GLOB SOURCES "src/*.h" "res/*.rc"
"src/conference.cpp"
"src/files.cpp"
"src/messaging.cpp"
"src/options.cpp"
"src/places.cpp"
"src/sametime.cpp"
"src/sametime_proto.cpp"
"src/sametime_session.cpp"
"src/session_announce_win.cpp"
"src/userlist.cpp"
"src/utils.cpp"
"src/meanwhile/src/channel.c"
"src/meanwhile/src/cipher.c"
"src/meanwhile/src/common.c"
"src/meanwhile/src/error.c"
"src/meanwhile/src/message.c"
"src/meanwhile/src/mpi/mpi.c"
"src/meanwhile/src/mw_debug.c"
"src/meanwhile/src/mw_util.c"
"src/meanwhile/src/service.c"
"src/meanwhile/src/session.c"
"src/meanwhile/src/srvc_aware.c"
"src/meanwhile/src/srvc_conf.c"
"src/meanwhile/src/srvc_dir.c"
"src/meanwhile/src/srvc_ft.c"
"src/meanwhile/src/srvc_im.c"
"src/meanwhile/src/srvc_place.c"
"src/meanwhile/src/srvc_resolve.c"
"src/meanwhile/src/srvc_store.c"
"src/meanwhile/src/st_list.c"
)
set(TARGET Sametime)
include_directories("src" "src/glib" "src/meanwhile/src")
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)
target_link_libraries(${TARGET} glib)
add_subdirectory(proto_sametime)
add_subdirectory(src/glib)