#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <commctrl.h>
#include <malloc.h>
#include <time.h>

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
#include <m_hotkeys.h>
#include <m_icolib.h>
#include <m_message.h>
#include <m_chat.h>
#include <m_protocols.h>

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

class CMLuaScript;

#include "mlua.h"
#include "mlua_script.h"
#include "mlua_module_loader.h"
#include "mlua_script_loader.h"
#include "mlua_options.h"

#define MODULE "MirLua"

extern CMLua *g_mLua;

extern int hScriptsLangpack;

extern HINSTANCE g_hInstance;

extern HANDLE g_hCommonScriptFolder;

extern HANDLE hNetlib;

#ifdef _UNICODE
	#define COMMON_SCRIPTS_PATHT MIRANDA_PATHW L"\\Scripts"
#else
	#define COMMON_SCRIPTS_PATHT MIRANDA_PATH "\\Scripts"
#endif

#define MLUA_CORE	"m_core"
LUAMOD_API int (luaopen_m_core)(lua_State *L);

#define MLUA_CLIST	"m_clist"
LUAMOD_API int (luaopen_m_clist)(lua_State *L);

#define MLUA_DATABASE	"m_database"
LUAMOD_API int (luaopen_m_database)(lua_State *L);

#define MLUA_ICOLIB	"m_icolib"
LUAMOD_API int (luaopen_m_icolib)(lua_State *L);

#include "m_genmenu.h"

#define MLUA_HOTKEYS	"m_hotkeys"
LUAMOD_API int (luaopen_m_hotkeys)(lua_State *L);

#define MLUA_MESSAGE	"m_message"
LUAMOD_API int (luaopen_m_message)(lua_State *L);

#include "m_msg_buttonsbar.h"

#define MLUA_POPUP	"m_popup"
LUAMOD_API int (luaopen_m_popup)(lua_State *L);

#include "m_protocols.h"

#include "m_toptoolbar.h"

#define MLUA_VARIABLES	"m_variables"
LUAMOD_API int (luaopen_m_variables)(lua_State *L);

#define MLUA_WINDOWS	"m_windows"
LUAMOD_API int (luaopen_m_windows)(lua_State *L);

int luaM_print(lua_State *L);
int luaM_atpanic(lua_State *L);

int luaM_toansi(lua_State *L);
int luaM_toucs2(lua_State *L);

int luaM_totable(lua_State *L);

bool luaM_toboolean(lua_State *L, int idx);
WPARAM luaM_towparam(lua_State *L, int idx);
LPARAM luaM_tolparam(lua_State *L, int idx);

void ShowNotification(const char *caption, const char *message, int flags, MCONTACT hContact);

void ObsoleteMethod(lua_State *L, const char *message);

#endif //_COMMON_H_
