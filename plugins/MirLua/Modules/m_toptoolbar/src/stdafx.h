#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>

#include <m_lua.h>
#include <m_core.h>
#include <m_utils.h>
#include <m_toptoolbar.h>

#define MLUA_TOPTOOLBAR	"m_toptoolbar"
LUAMOD_API int (luaopen_m_toptoolbar)(lua_State *L);

void KillModuleTTBButton();

#endif //_COMMON_H_