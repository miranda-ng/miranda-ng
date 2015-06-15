#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <commctrl.h>

#include <newpluginapi.h>
#include <m_core.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_gui.h>

#include <m_clist.h>
#include <m_genmenu.h>
#include <m_icolib.h>

#include <m_folders.h>
#include <m_toptoolbar.h>
#include <m_msg_buttonsbar.h>

extern "C"
{
	#include "lua\lua.h"
	#include "lua\lualib.h"
	#include "lua\lauxlib.h"
}

#include "version.h"
#include "resource.h"

class CMLua;

#include "mlua.h"
#include "mlua_options.h"

#define MODULE "MirLua"

extern HINSTANCE g_hInstance;

extern HANDLE g_hCommonFolderPath;
extern HANDLE g_hCustomFolderPath;

#ifdef _UNICODE
	#define COMMON_SCRIPTS_PATHT MIRANDA_PATHW L"\\Scripts"
	#define CUSTOM_SCRIPTS_PATHT MIRANDA_USERDATAW L"\\Scripts"
#else
	#define COMMON_SCRIPTS_PATHT MIRANDA_PATH "\\Scripts"
	#define CUSTOM_SCRIPTS_PATHT MIRANDA_USERDATA "\\Scripts"
#endif

#define MLUA_DATABASE	"m_database"
LUAMOD_API int (luaopen_m_database)(lua_State *L);

#define MLUA_ICOLIB	"m_icolib"
LUAMOD_API int (luaopen_m_icolib)(lua_State *L);

#define MLUA_GENMENU	"m_genmenu"
LUAMOD_API int (luaopen_m_genmenu)(lua_State *L);

#define MLUA_MSGBUTTONSBAR	"m_msg_buttonsbar"
LUAMOD_API int (luaopen_m_msg_buttonsbar)(lua_State *L);

#define MLUA_TOPTOOLBAR	"m_toptoolbar"
LUAMOD_API int (luaopen_m_toptoolbar)(lua_State *L);

#endif //_COMMON_H_
