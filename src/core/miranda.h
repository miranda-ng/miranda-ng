/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

typedef HRESULT (STDAPICALLTYPE *pfnDrawThemeTextEx)(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, LPRECT, const struct _DTTOPTS *);
typedef HRESULT (STDAPICALLTYPE *pfnSetWindowThemeAttribute)(HWND, enum WINDOWTHEMEATTRIBUTETYPE, PVOID, DWORD);
typedef HRESULT (STDAPICALLTYPE *pfnBufferedPaintInit)(void);
typedef HRESULT (STDAPICALLTYPE *pfnBufferedPaintUninit)(void);
typedef HANDLE  (STDAPICALLTYPE *pfnBeginBufferedPaint)(HDC, RECT *, BP_BUFFERFORMAT, BP_PAINTPARAMS *, HDC *);
typedef HRESULT (STDAPICALLTYPE *pfnEndBufferedPaint)(HANDLE, BOOL);
typedef HRESULT (STDAPICALLTYPE *pfnGetBufferedPaintBits)(HANDLE, RGBQUAD **, int *);

extern pfnDrawThemeTextEx drawThemeTextEx;
extern pfnSetWindowThemeAttribute setWindowThemeAttribute;
extern pfnBufferedPaintInit bufferedPaintInit;
extern pfnBufferedPaintUninit bufferedPaintUninit;
extern pfnBeginBufferedPaint beginBufferedPaint;
extern pfnEndBufferedPaint endBufferedPaint;
extern pfnGetBufferedPaintBits getBufferedPaintBits;

extern ITaskbarList3 * pTaskbarInterface;

typedef HRESULT (STDAPICALLTYPE *pfnDwmExtendFrameIntoClientArea)(HWND hwnd, const MARGINS *margins);
typedef HRESULT (STDAPICALLTYPE *pfnDwmIsCompositionEnabled)(BOOL *);

extern pfnDwmExtendFrameIntoClientArea dwmExtendFrameIntoClientArea;
extern pfnDwmIsCompositionEnabled dwmIsCompositionEnabled;

/**** database.cpp *********************************************************************/

extern MIDatabase* currDb;
extern DATABASELINK* currDblink;
extern LIST<DATABASELINK> arDbPlugins;

int  InitIni(void);
void UninitIni(void);

/**** extraicons.cpp *******************************************************************/

void KillModuleExtraIcons(int hLangpack);

/**** fontService.cpp ******************************************************************/

void KillModuleFonts(int hLangpack);
void KillModuleColours(int hLangpack);
void KillModuleEffects(int hLangpack);
void KillModuleHotkeys(int hLangpack);
void KillModuleSounds(int hLangpack);

/**** miranda.cpp **********************************************************************/

extern HINSTANCE hInst;
extern DWORD hMainThreadId;
extern HANDLE hOkToExitEvent, hModulesLoadedEvent, hevLoadModule, hevUnloadModule;
extern TCHAR mirandabootini[MAX_PATH];

/**** newplugins.cpp *******************************************************************/

char* GetPluginNameByInstance(HINSTANCE hInstance);
int   GetPluginFakeId(const MUUID &uuid, int hLangpack);
int   LoadStdPlugins(void);

/**** path.cpp *************************************************************************/

void InitPathVar(void);

/**** srmm.cpp *************************************************************************/

void KillModuleSrmmIcons(int hLangpack);

/**** utf.cpp **************************************************************************/

__forceinline char* Utf8DecodeA(const char* src)
{
	char* tmp = mir_strdup(src);
	Utf8Decode(tmp, NULL);
	return tmp;
}

#pragma optimize("", on)

/**** options.cpp **********************************************************************/

HTREEITEM FindNamedTreeItemAtRoot(HWND hwndTree, const TCHAR* name);

/**** skin2icons.cpp *******************************************************************/

void   KillModuleIcons(int hLangpack);

/**** skinicons.cpp ********************************************************************/

HICON LoadIconEx(HINSTANCE hInstance, LPCTSTR lpIconName, BOOL bShared);
int ImageList_AddIcon_NotShared(HIMAGELIST hIml, LPCTSTR szResource);
int ImageList_ReplaceIcon_NotShared(HIMAGELIST hIml, int iIndex, HINSTANCE hInstance, LPCTSTR szResource);

int ImageList_AddIcon_IconLibLoaded(HIMAGELIST hIml, int iconId);
int ImageList_AddIcon_ProtoIconLibLoaded(HIMAGELIST hIml, const char *szProto, int iconId);
int ImageList_ReplaceIcon_IconLibLoaded(HIMAGELIST hIml, int nIndex, HICON hIcon);

#define Safe_DestroyIcon(hIcon) if (hIcon) DestroyIcon(hIcon)

/**** clistmenus.cpp ********************************************************************/

extern HANDLE hMainMenuObject, hContactMenuObject, hStatusMenuObject;
extern HANDLE hPreBuildMainMenuEvent, hPreBuildContactMenuEvent;
extern HANDLE hShutdownEvent, hPreShutdownEvent;

extern const int statusModeList[ MAX_STATUS_COUNT ];
extern const int skinIconStatusList[ MAX_STATUS_COUNT ];
extern const int skinIconStatusFlags[ MAX_STATUS_COUNT ];

int TryProcessDoubleClick(MCONTACT hContact);

void KillModuleMenus(int hLangpack);

/**** protocols.cpp *********************************************************************/

#define OFFSET_PROTOPOS 200
#define OFFSET_VISIBLE  400
#define OFFSET_ENABLED  600
#define OFFSET_NAME     800

extern LIST<PROTOACCOUNT> accounts;

INT_PTR ProtoCallService(LPCSTR szModule, const char *szService, WPARAM wParam, LPARAM lParam);

PROTOACCOUNT* Proto_CreateAccount(const char *szModuleName, const char *szBaseProto, const TCHAR *tszAccountName);

PROTOACCOUNT* __fastcall Proto_GetAccount(const char *accName);
PROTOACCOUNT* __fastcall Proto_GetAccount(MCONTACT hContact);

bool __fastcall Proto_IsAccountEnabled(PROTOACCOUNT *pa);
bool __fastcall Proto_IsAccountLocked(PROTOACCOUNT *pa);

PROTO_INTERFACE* AddDefaultAccount(const char *szProtoName);
int  FreeDefaultAccount(PROTO_INTERFACE* ppi);

BOOL ActivateAccount(PROTOACCOUNT *pa);
void EraseAccount(const char *pszProtoName);
void DeactivateAccount(PROTOACCOUNT *pa, bool bIsDynamic, bool bErase);
void UnloadAccount(PROTOACCOUNT *pa, bool bIsDynamic, bool bErase);
void OpenAccountOptions(PROTOACCOUNT *pa);

void LoadDbAccounts(void);
void WriteDbAccounts(void);

INT_PTR CallProtoServiceInt(MCONTACT hContact, const char* szModule, const char* szService, WPARAM wParam, LPARAM lParam);

/**** utils.cpp ************************************************************************/

void HotkeyToName(TCHAR *buf, int size, BYTE shift, BYTE key);
WORD GetHotkeyValue(INT_PTR idHotkey);
int  AssertInsideScreen(RECT &rc);

HBITMAP ConvertIconToBitmap(HICON hIcon, HIMAGELIST hIml, int iconId);

///////////////////////////////////////////////////////////////////////////////

extern "C"
{
	MIR_CORE_DLL(int)  Langpack_MarkPluginLoaded(PLUGININFOEX* pInfo);
	MIR_CORE_DLL(int)  GetSubscribersCount(HANDLE hHook);
	MIR_CORE_DLL(void) db_setCurrent(MIDatabase* _db);

	MIR_CORE_DLL(PROTOCOLDESCRIPTOR*) Proto_RegisterModule(PROTOCOLDESCRIPTOR *pd);
};
