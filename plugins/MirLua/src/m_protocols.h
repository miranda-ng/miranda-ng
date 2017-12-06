#ifndef _LUA_M_PROTOCOLS_H_
#define _LUA_M_PROTOCOLS_H_

#define MLUA_PROTOCOLS	"m_protocols"
LUAMOD_API int (luaopen_m_protocols)(lua_State *L);

#define MT_PROTOCOLDESCRIPTOR "PROTOCOLDESCRIPTOR"
#define MT_PROTOACCOUNT "PROTOACCOUNT"

extern HANDLE hRecvMessage;
INT_PTR FilterRecvMessage(WPARAM wParam, LPARAM lParam);

#endif //_LUA_M_PROTOCOLS_H_