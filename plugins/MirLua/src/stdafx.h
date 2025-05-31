#pragma once

#include <windows.h>
#include <time.h>
#include <wchar.h>

#include <map>
#include <regex>
#include <string>

#include <newpluginapi.h>
#include <m_core.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_extraicons.h>
#include <m_options.h>
#include <m_netlib.h>
#include <m_genmenu.h>
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
#include <m_json.h>
#include <m_gui.h>
#include <m_variables.h>

#include <m_lua.h>
#include <mirlua.h>

#include "version.h"
#include "resource.h"

class CMLuaScript;

#include "plugin.h"
#include "modules.h"
#include "environment.h"
#include "script.h"
#include "function_loader.h"
#include "module_loader.h"
#include "script_loader.h"
#include "variables_loader.h"
#include "metatable.h"

#define MODULENAME LPGEN("MirLua")

extern PLUGININFOEX g_pluginInfoEx;

extern HANDLE g_hCLibsFolder;
extern HANDLE g_hScriptsFolder;
#ifdef _UNICODE
	#define MIRLUA_PATHT MIRANDA_PATHW L"\\Scripts"
#else
	#define MIRLUA_PATHT MIRANDA_PATH "\\Scripts"
#endif

#define LUACLIBSCRIPTEXT L"dll"
#define LUATEXTSCRIPTEXT L"lua"
#define LUAPRECSCRIPTEXT L"luac"

void LoadIcons();

/* utils */

void Log(const char *format, ...);
void Log(const wchar_t *format, ...);

void ShowNotification(const char *caption, const char *message, int flags = 0, MCONTACT hContact = NULL);

void ObsoleteMethod(lua_State *L, const char *message);

void ReportError(lua_State *L);

int luaM_atpanic(lua_State *L);
int luaM_pcall(lua_State *L, int n = 0, int r = 0);

int luaM_getenv(lua_State *L);

bool luaM_toboolean(lua_State *L, int idx);

bool luaM_isarray(lua_State *L, int idx);
