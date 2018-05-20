#pragma once

#define MLUA_CORE	"m_core"
LUAMOD_API int (luaopen_m_core)(lua_State *L);

#define MLUA_CHAT	"m_chat"
LUAMOD_API int (luaopen_m_chat)(lua_State *L);

#define MLUA_CLIST	"m_clist"
LUAMOD_API int (luaopen_m_clist)(lua_State *L);

#define MLUA_DATABASE	"m_database"
LUAMOD_API int (luaopen_m_database)(lua_State *L);

#define MLUA_ICOLIB	"m_icolib"
LUAMOD_API int (luaopen_m_icolib)(lua_State *L);

#define MLUA_GENMENU	"m_genmenu"
LUAMOD_API int (luaopen_m_genmenu)(lua_State *L);

#define MLUA_HTTP	"m_http"
LUAMOD_API int (luaopen_m_http)(lua_State *L);

#define MLUA_HOTKEYS	"m_hotkeys"
LUAMOD_API int (luaopen_m_hotkeys)(lua_State *L);

#define MLUA_JSON	"m_json"
LUAMOD_API int (luaopen_m_json)(lua_State *L);

#define MLUA_MESSAGE	"m_message"
LUAMOD_API int (luaopen_m_message)(lua_State *L);

#define MLUA_OPTIONS	"m_options"
LUAMOD_API int (luaopen_m_options)(lua_State *L);

#define MLUA_PROTOCOLS	"m_protocols"
#define MT_PROTOACCOUNT "PROTOACCOUNT"
LUAMOD_API int (luaopen_m_protocols)(lua_State *L);

#define MLUA_SOUNDS	"m_sounds"
LUAMOD_API int (luaopen_m_sounds)(lua_State *L);

#define MLUA_SRMM	"m_srmm"
LUAMOD_API int (luaopen_m_srmm)(lua_State *L);