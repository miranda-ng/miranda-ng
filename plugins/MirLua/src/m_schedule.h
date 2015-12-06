#ifndef _LUA_M_SCHEDULE_H_
#define _LUA_M_SCHEDULE_H_

#define MLUA_SCHEDULE	"m_schedule"
LUAMOD_API int (luaopen_m_schedule)(lua_State *L);

void KillModuleScheduleTasks();

#endif //_LUA_M_SCHEDULE_H_