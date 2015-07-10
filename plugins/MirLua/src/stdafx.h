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
#include <m_netlib.h>

#include <m_genmenu.h>
#include <m_clist.h>
#include <m_icolib.h>
#include <m_message.h>

#include <m_folders.h>
#include <m_msg_buttonsbar.h>
#include <m_popup.h>
#include <m_toptoolbar.h>
#include <m_variables.h>

extern "C"
{
	#include "lua\lua.h"
	#include "lua\lualib.h"
	#include "lua\lauxlib.h"
}

#include "version.h"
#include "resource.h"

#include "mlua.h"
#include "mlua_module.h"
#include "mlua_m_clist.h"
#include "mlua_module_loader.h"
#include "mlua_script_loader.h"
#include "mlua_options.h"

#define MODULE "MirLua"

extern CMLua *g_mLua;

extern int hScriptsLangpack;

extern HINSTANCE g_hInstance;

extern HANDLE g_hCommonFolderPath;
extern HANDLE g_hCustomFolderPath;

extern HANDLE hNetlib;

#ifdef _UNICODE
	#define COMMON_SCRIPTS_PATHT MIRANDA_PATHW L"\\Scripts"
	#define CUSTOM_SCRIPTS_PATHT MIRANDA_USERDATAW L"\\Scripts"
#else
	#define COMMON_SCRIPTS_PATHT MIRANDA_PATH "\\Scripts"
	#define CUSTOM_SCRIPTS_PATHT MIRANDA_USERDATA "\\Scripts"
#endif

#define MLUA_CORE	"m"
LUAMOD_API int (luaopen_m)(lua_State *L);

#define MLUA_CLIST	"m_clist"
LUAMOD_API int (luaopen_m_clist)(lua_State *L);

#define MLUA_DATABASE	"m_database"
LUAMOD_API int (luaopen_m_database)(lua_State *L);

#define MLUA_ICOLIB	"m_icolib"
LUAMOD_API int (luaopen_m_icolib)(lua_State *L);

#include "m_genmenu.h"

#define MLUA_MESSAGE	"m_message"
LUAMOD_API int (luaopen_m_message)(lua_State *L);

#define MLUA_MSGBUTTONSBAR	"m_msg_buttonsbar"
LUAMOD_API int (luaopen_m_msg_buttonsbar)(lua_State *L);

#define MLUA_POPUP	"m_popup"
LUAMOD_API int (luaopen_m_popup)(lua_State *L);

#define MLUA_TOPTOOLBAR	"m_toptoolbar"
LUAMOD_API int (luaopen_m_toptoolbar)(lua_State *L);

#define MLUA_VARIABLES	"m_variables"
LUAMOD_API int (luaopen_m_variables)(lua_State *L);

#define MLUA_WINDOWS	"m_windows"
LUAMOD_API int (luaopen_m_windows)(lua_State *L);

int luaM_print(lua_State *L);
int luaM_atpanic(lua_State *L);

bool luaM_toboolean(lua_State *L, int idx);
WPARAM luaM_towparam(lua_State *L, int idx);
LPARAM luaM_tolparam(lua_State *L, int idx);

#endif //_COMMON_H_
