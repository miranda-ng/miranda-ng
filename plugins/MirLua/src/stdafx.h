#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <time.h>
#include <wchar.h>

#include <newpluginapi.h>
#include <m_core.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_netlib.h>
#include <m_http.h>
#include <m_clist.h>
#include <m_hotkeys.h>
#include <m_icolib.h>
#include <m_skin.h>
#include <m_message.h>
#include <m_chat.h>
#include <m_protocols.h>
#include <m_contacts.h>
#include <m_folders.h>
#include <m_popup.h>
#include <m_toptoolbar.h>

#include <m_lua.h>
#include <mirlua.h>

#include "version.h"
#include "resource.h"

class CMLuaScript;

#include "mlua.h"
#include "mlua_enviroment.h"
#include "mlua_script.h"
#include "mlua_module_loader.h"
#include "mlua_script_loader.h"
#include "mlua_options.h"
#include "mlua_metatable.h"

#define MODULE "MirLua"

extern HINSTANCE g_hInstance;

extern CMLua *g_mLua;

extern int hMLuaLangpack;

extern HANDLE g_hCLibsFolder;
extern HANDLE g_hScriptsFolder;
#ifdef _UNICODE
	#define MIRLUA_PATHT MIRANDA_PATHW L"\\Scripts"
#else
	#define MIRLUA_PATHT MIRANDA_PATH "\\Scripts"
#endif

/* modules */

#define MLUA_CORE	"m_core"
LUAMOD_API int (luaopen_m_core)(lua_State *L);

#define MLUA_CHAT	"m_chat"
LUAMOD_API int (luaopen_m_chat)(lua_State *L);

#define MLUA_CLIST	"m_clist"
LUAMOD_API int (luaopen_m_clist)(lua_State *L);

#include "m_database.h"

#define MLUA_ICOLIB	"m_icolib"
LUAMOD_API int (luaopen_m_icolib)(lua_State *L);

#include "m_json.h"

#include "m_genmenu.h"

#define MLUA_HTTP	"m_http"
LUAMOD_API int (luaopen_m_http)(lua_State *L);

#define MLUA_HOTKEYS	"m_hotkeys"
LUAMOD_API int (luaopen_m_hotkeys)(lua_State *L);

#define MLUA_MESSAGE	"m_message"
LUAMOD_API int (luaopen_m_message)(lua_State *L);

#define MLUA_OPTIONS	"m_options"
LUAMOD_API int (luaopen_m_options)(lua_State *L);

#include "m_protocols.h"

#define MLUA_SOUNDS	"m_sounds"
LUAMOD_API int (luaopen_m_sounds)(lua_State *L);

#define MLUA_SRMM	"m_srmm"
LUAMOD_API int (luaopen_m_srmm)(lua_State *L);

/* services */

INT_PTR Call(WPARAM wParam, LPARAM lParam);

/* utils */

extern HNETLIBUSER hNetlib;
void Log(const char *format, ...);
void Log(const wchar_t *format, ...);

void ShowNotification(const char *caption, const char *message, int flags = 0, MCONTACT hContact = NULL);

void ObsoleteMethod(lua_State *L, const char *message);

void ReportError(lua_State *L);

int luaM_atpanic(lua_State *L);
int luaM_pcall(lua_State *L, int n = 0, int r = 0);

int luaM_getenv(lua_State *L);

bool luaM_toboolean(lua_State *L, int idx);

void InitIcons();
HICON GetIcon(int iconId);
HANDLE GetIconHandle(int iconId);

#endif //_COMMON_H_
