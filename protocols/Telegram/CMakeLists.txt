file(GLOB SOURCES "src/*.h" "res/*.rc"
"src/tgl/auto/auto-autocomplete.c"
"src/tgl/auto/auto-fetch-ds.c"
"src/tgl/auto/auto-fetch.c"
"src/tgl/auto/auto-free-ds.c"
"src/tgl/auto/auto-print-ds.c"
"src/tgl/auto/auto-skip.c"
"src/tgl/auto/auto-store-ds.c"
"src/tgl/auto/auto-store.c"
"src/tgl/auto/auto-types.c"
"src/tgl/binlog.c"
"src/tgl/mime-types.c"
"src/tgl/mtproto-client.c"
"src/tgl/mtproto-common.c"
"src/tgl/mtproto-utils.c"
"src/tgl/queries.c"
"src/tgl/structures.c"
"src/tgl/tg-mime-types.c"
"src/tgl/tgl-net.c"
"src/tgl/tgl-timers.c"
"src/tgl/tgl.c"
"src/tgl/tools.c"
"src/tgl/updates.c"
)
include_directories(${CMAKE_SOURCE_DIR}/libs/libevent/include)
set(TARGET Telegram)
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)
set_target_properties(${TARGET} PROPERTIES COMPILE_DEFINITIONS "HAVE_CONFIG_H")
target_link_libraries(${TARGET} Zlib libevent libeay32.lib ssleay32.lib)