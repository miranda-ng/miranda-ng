/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#pragma once

#define MS_OPTIONS_OPEN "Options/OptionsCommand"

/**** database.cpp *********************************************************************/

extern MIR_CORE_EXPORT MDatabaseCommon *g_pCurrDb;
extern LIST<DATABASELINK> arDbPlugins;

int  InitIni(void);
void UninitIni(void);

/**** idle.cpp *************************************************************************/

int  LoadIdleModule(void);
void UnloadIdleModule(void);

/**** miranda.cpp **********************************************************************/

EXTERN_C MIR_CORE_DLL(void) BeginMessageLoop(void);
EXTERN_C MIR_CORE_DLL(void) EnterMessageLoop(void);
EXTERN_C MIR_CORE_DLL(void) LeaveMessageLoop(void);

extern uint32_t hMainThreadId;
extern HANDLE hOkToExitEvent, hModulesLoadedEvent;
extern HANDLE hAccListChanged;
extern wchar_t mirandabootini[MAX_PATH];
extern struct pluginEntry *plugin_checker, *plugin_crshdmp, *plugin_service, *plugin_clist;
extern bool g_bModulesLoadedFired;

/**** newplugins.cpp *******************************************************************/

const char* GetPluginNameByInstance(HINSTANCE hInstance);

int   LoadStdPlugins(void);
int   LaunchServicePlugin(pluginEntry *p);

/**** path.cpp *************************************************************************/

void InitPathVar(void);

/**** plugins.cpp **********************************************************************/

void KillModuleAccounts(HINSTANCE);

void KillModuleExtraIcons(CMPluginBase *pPlugin);
void KillModuleFonts(CMPluginBase *pPlugin);
void KillModuleHotkeys(CMPluginBase *pPlugin);
void KillModuleIcons(CMPluginBase *pPlugin);
void KillModuleMenus(CMPluginBase *pPlugin);
void KillModulePopups(CMPluginBase *pPlugin);
void KillModuleSounds(CMPluginBase *pPlugin);
void KillModuleSrmmIcons(CMPluginBase *pPlugin);
void KillModuleToolbarIcons(CMPluginBase *pPlugin);

/**** popupOptions.cpp *****************************************************************/

int   PopupOptionsInit(WPARAM);

/**** srmm.cpp *************************************************************************/

#define SRMM_MODULE "SRMsg"

/**** utf.cpp **************************************************************************/

__forceinline char* Utf8DecodeA(const char* src)
{
	char* tmp = mir_strdup(src);
	mir_utf8decode(tmp, nullptr);
	return tmp;
}

#pragma optimize("", on)

/**** skinicons.cpp ********************************************************************/

extern int g_iIconX, g_iIconY, g_iIconSX, g_iIconSY;

HICON LoadIconEx(HINSTANCE hInstance, LPCTSTR lpIconName, BOOL bShared);
int ImageList_AddIcon_NotShared(HIMAGELIST hIml, LPCTSTR szResource);

int ImageList_ReplaceIcon_IconLibLoaded(HIMAGELIST hIml, int nIndex, HICON hIcon);

#define Safe_DestroyIcon(hIcon) if (hIcon) DestroyIcon(hIcon)

/**** clistmenus.cpp ********************************************************************/

extern int hMainMenuObject, hContactMenuObject, hStatusMenuObject;
extern HANDLE hPreBuildMainMenuEvent, hPreBuildContactMenuEvent;
extern HMENU hMainMenu, hStatusMenu;

extern OBJLIST<CListEvent> g_cliEvents;

struct MStatus
{
	int iStatus;
	int iSkinIcon;
	int Pf2flag;

	INT_PTR iHotKey;
	HGENMENU hStatusMenu;
};

extern MStatus g_statuses[MAX_STATUS_COUNT];

/**** protocols.cpp *********************************************************************/

#define OFFSET_PROTOPOS 200
#define OFFSET_VISIBLE  400
#define OFFSET_ENABLED  600
#define OFFSET_NAME     800

extern LIST<PROTOACCOUNT> g_arAccounts;

struct MBaseProto : public PROTOCOLDESCRIPTOR, public MZeroedObject
{
	MBaseProto(const char *_proto)
	{
		this->szName = mir_strdup(_proto);
	}

	~MBaseProto()
	{
		mir_free(szName);
		mir_free(szUniqueId);
	}

	pfnInitProto fnInit;
	pfnUninitProto fnUninit;

	HINSTANCE hInst;
	char *szUniqueId;  // name of the unique setting that identifies a contact
};

extern OBJLIST<MBaseProto> g_arProtos;
extern LIST<MBaseProto> g_arFilters;

INT_PTR ProtoCallService(const char *szModule, const char *szService, WPARAM wParam, LPARAM lParam);

PROTO_INTERFACE* AddDefaultAccount(const char *szProtoName);
int  FreeDefaultAccount(PROTO_INTERFACE* ppi);

bool ActivateAccount(PROTOACCOUNT *pa, bool bIsDynamic);
void EraseAccount(const char *pszProtoName);
void OpenAccountOptions(PROTOACCOUNT *pa);

/////////////////////////////////////////////////////////////////////////////////////////

#define DAF_DYNAMIC 0x0001
#define DAF_ERASE   0x0002
#define DAF_FORK    0x0004

void DeactivateAccount(PROTOACCOUNT *pa, int flags);
void UnloadAccount(PROTOACCOUNT *pa, int flags);

/////////////////////////////////////////////////////////////////////////////////////////

void LoadDbAccounts(void);
void WriteDbAccounts(void);

void InitStaticAccounts();

INT_PTR CallProtoServiceInt(MCONTACT hContact, const char* szModule, const char* szService, WPARAM wParam, LPARAM lParam);

INT_PTR stubChainRecv(WPARAM, LPARAM);
#define MS_PROTO_HIDDENSTUB "Proto/stubChainRecv"

/**** utils.cpp ************************************************************************/

BOOL IsAeroMode();

#ifdef _WINDOWS
bool ProcessFileDrop(HDROP hDrop, MCONTACT hContact);
#endif

void RegisterModule(CMPluginBase*);

void HotkeyToName(wchar_t *buf, int size, uint8_t shift, uint8_t key);
uint16_t GetHotkeyValue(INT_PTR idHotkey);

HBITMAP ConvertIconToBitmap(HIMAGELIST hIml, int iconId);
MBaseProto* Proto_GetProto(const char *szProtoName);

///////////////////////////////////////////////////////////////////////////////

extern "C"
{
	MIR_CORE_DLL(int)  Langpack_MarkPluginLoaded(const MUUID &uuid);
	MIR_CORE_DLL(int)  GetSubscribersCount(struct THook *hHook);
};
