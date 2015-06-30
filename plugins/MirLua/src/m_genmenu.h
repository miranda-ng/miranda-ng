#ifndef _LUA_M_GENMENU_H_
#define _LUA_M_GENMENU_H_

#define MLUA_GENMENU	"m_genmenu"
LUAMOD_API int (luaopen_m_genmenu)(lua_State *L);

void MakeMenuItem(lua_State *L, CMenuItem &mi);

#endif //_LUA_M_GENMENU_H_