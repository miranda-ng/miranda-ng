/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project, 
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

#define OPTIONPAGE_OLD_SIZE offsetof(OPTIONSDIALOGPAGE,hLangpack)

typedef HMONITOR (WINAPI *pfnMyMonitorFromPoint)(POINT, DWORD);
extern pfnMyMonitorFromPoint MyMonitorFromPoint;

typedef HMONITOR (WINAPI *pfnMyMonitorFromRect)(LPCRECT, DWORD);
extern pfnMyMonitorFromRect MyMonitorFromRect;

typedef HMONITOR(WINAPI *pfnMyMonitorFromWindow) (HWND, DWORD);
extern pfnMyMonitorFromWindow MyMonitorFromWindow;

typedef BOOL(WINAPI *pfnMyGetMonitorInfo) (HMONITOR, LPMONITORINFO);
extern pfnMyGetMonitorInfo MyGetMonitorInfo;

typedef HRESULT (STDAPICALLTYPE *pfnSHAutoComplete)(HWND, DWORD);
extern pfnSHAutoComplete shAutoComplete;

typedef HRESULT (STDAPICALLTYPE *pfnSHGetSpecialFolderPathA)(HWND, LPSTR,  int, BOOL);
typedef HRESULT (STDAPICALLTYPE *pfnSHGetSpecialFolderPathW)(HWND, LPWSTR, int, BOOL);
extern pfnSHGetSpecialFolderPathA shGetSpecialFolderPathA;
extern pfnSHGetSpecialFolderPathW shGetSpecialFolderPathW;

#define shGetSpecialFolderPath shGetSpecialFolderPathW

typedef HDESK (WINAPI* pfnOpenInputDesktop)(DWORD, BOOL, DWORD);
extern pfnOpenInputDesktop openInputDesktop;

typedef HDESK (WINAPI* pfnCloseDesktop)(HDESK);
extern pfnCloseDesktop closeDesktop;

typedef BOOL (WINAPI* pfnAnimateWindow)(HWND, DWORD, DWORD);
extern pfnAnimateWindow animateWindow;

typedef BOOL (WINAPI * pfnSetLayeredWindowAttributes) (HWND, COLORREF, BYTE, DWORD);
extern pfnSetLayeredWindowAttributes setLayeredWindowAttributes;

typedef HTHEME  (STDAPICALLTYPE *pfnOpenThemeData)(HWND, LPCWSTR);
typedef HRESULT (STDAPICALLTYPE *pfnIsThemeBackgroundPartiallyTransparent)(HTHEME, int, int);
typedef HRESULT (STDAPICALLTYPE *pfnDrawThemeParentBackground)(HWND, HDC, const RECT *);
typedef HRESULT (STDAPICALLTYPE *pfnDrawThemeBackground)(HTHEME, HDC, int, int, const RECT *, const RECT *);
typedef HRESULT (STDAPICALLTYPE *pfnDrawThemeText)(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD, const RECT *);
typedef HRESULT (STDAPICALLTYPE *pfnDrawThemeTextEx)(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, LPRECT, const struct _DTTOPTS *);
typedef HRESULT (STDAPICALLTYPE *pfnGetThemeBackgroundContentRect)(HTHEME, HDC, int, int, LPCRECT, LPRECT);
typedef HRESULT (STDAPICALLTYPE *pfnGetThemeFont)(HTHEME, HDC, int, int, int, LOGFONT *);
typedef HRESULT (STDAPICALLTYPE *pfnCloseThemeData)(HTHEME);
typedef HRESULT (STDAPICALLTYPE *pfnEnableThemeDialogTexture)(HWND hwnd, DWORD dwFlags);
typedef HRESULT (STDAPICALLTYPE *pfnSetWindowTheme)(HWND, LPCWSTR, LPCWSTR);
typedef HRESULT (STDAPICALLTYPE *pfnSetWindowThemeAttribute)(HWND, enum WINDOWTHEMEATTRIBUTETYPE, PVOID, DWORD);
typedef BOOL    (STDAPICALLTYPE *pfnIsThemeActive)();
typedef HRESULT (STDAPICALLTYPE *pfnBufferedPaintInit)(void);
typedef HRESULT (STDAPICALLTYPE *pfnBufferedPaintUninit)(void);
typedef HANDLE  (STDAPICALLTYPE *pfnBeginBufferedPaint)(HDC, RECT *, BP_BUFFERFORMAT, BP_PAINTPARAMS *, HDC *);
typedef HRESULT (STDAPICALLTYPE *pfnEndBufferedPaint)(HANDLE, BOOL);
typedef HRESULT (STDAPICALLTYPE *pfnGetBufferedPaintBits)(HANDLE, RGBQUAD **, int *);

extern pfnOpenThemeData openThemeData;
extern pfnIsThemeBackgroundPartiallyTransparent isThemeBackgroundPartiallyTransparent;
extern pfnDrawThemeParentBackground drawThemeParentBackground;
extern pfnDrawThemeBackground drawThemeBackground;
extern pfnDrawThemeText drawThemeText;
extern pfnDrawThemeTextEx drawThemeTextEx;
extern pfnGetThemeBackgroundContentRect getThemeBackgroundContentRect;
extern pfnGetThemeFont getThemeFont;
extern pfnCloseThemeData closeThemeData;
extern pfnEnableThemeDialogTexture enableThemeDialogTexture;
extern pfnSetWindowTheme setWindowTheme;
extern pfnSetWindowThemeAttribute setWindowThemeAttribute;
extern pfnIsThemeActive isThemeActive;
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

extern LPFN_GETADDRINFO MyGetaddrinfo;
extern LPFN_FREEADDRINFO MyFreeaddrinfo;
extern LPFN_WSASTRINGTOADDRESSA MyWSAStringToAddress;
extern LPFN_WSAADDRESSTOSTRINGA MyWSAAddressToString;

/**** database.cpp *********************************************************************/

extern MIDatabase* currDb;
extern DATABASELINK* currDblink;
extern LIST<DATABASELINK> arDbPlugins;

int  InitIni(void);
void UninitIni(void);

/**** fontService.cpp ******************************************************************/

void KillModuleFonts(int hLangpack);
void KillModuleColours(int hLangpack);
void KillModuleEffects(int hLangpack);
void KillModuleHotkeys(int hLangpack);
void KillModuleSounds(int hLangpack);

/**** miranda.cpp **********************************************************************/

extern HINSTANCE hInst;
extern HANDLE hOkToExitEvent, hModulesLoadedEvent, hevLoadModule, hevUnloadModule;

/**** newplugins.cpp *******************************************************************/

char* GetPluginNameByInstance(HINSTANCE hInstance);
int   GetPluginLangByInstance(HINSTANCE hInstance);
int   GetPluginFakeId(const MUUID &uuid, int hLangpack);

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
int ImageList_AddIcon_ProtoIconLibLoaded(HIMAGELIST hIml, const char* szProto, int iconId);
int ImageList_ReplaceIcon_IconLibLoaded(HIMAGELIST hIml, int nIndex, HICON hIcon);

#define Safe_DestroyIcon(hIcon) if (hIcon) DestroyIcon(hIcon)

/**** clistmenus.cpp ********************************************************************/

extern HANDLE hMainMenuObject, hContactMenuObject, hStatusMenuObject;
extern HANDLE hPreBuildMainMenuEvent, hPreBuildContactMenuEvent;
extern HANDLE hShutdownEvent, hPreShutdownEvent;

extern const int statusModeList[ MAX_STATUS_COUNT ];
extern const int skinIconStatusList[ MAX_STATUS_COUNT ];
extern const int skinIconStatusFlags[ MAX_STATUS_COUNT ];

int TryProcessDoubleClick(HANDLE hContact);

void KillModuleMenus(int hLangpack);

/**** protocols.cpp *********************************************************************/

#define OFFSET_PROTOPOS 200
#define OFFSET_VISIBLE  400
#define OFFSET_ENABLED  600
#define OFFSET_NAME     800

extern LIST<PROTOACCOUNT> accounts;

PROTOACCOUNT* __fastcall Proto_GetAccount(const char* accName);
PROTOACCOUNT* __fastcall Proto_GetAccount(HANDLE hContact);
PROTOCOLDESCRIPTOR* __fastcall Proto_IsProtocolLoaded(const char* szProtoName);

bool __fastcall Proto_IsAccountEnabled(PROTOACCOUNT* pa);
bool __fastcall Proto_IsAccountLocked(PROTOACCOUNT* pa);

PROTO_INTERFACE* AddDefaultAccount(const char* szProtoName);
int  FreeDefaultAccount(PROTO_INTERFACE* ppi);

BOOL ActivateAccount(PROTOACCOUNT* pa);
void EraseAccount(const char *pszProtoName);
void DeactivateAccount(PROTOACCOUNT* pa, bool bIsDynamic, bool bErase);
void UnloadAccount(PROTOACCOUNT* pa, bool bIsDynamic, bool bErase);
void OpenAccountOptions(PROTOACCOUNT* pa);
void KillProtoAccounts(const char *pszProtoName);

void LoadDbAccounts(void);
void WriteDbAccounts(void);

INT_PTR CallProtoServiceInt(HANDLE hContact, const char* szModule, const char* szService, WPARAM wParam, LPARAM lParam);

/**** utils.cpp ************************************************************************/

void HotkeyToName(TCHAR *buf, int size, BYTE shift, BYTE key);
WORD GetHotkeyValue(INT_PTR idHotkey);

HBITMAP ConvertIconToBitmap(HICON hIcon, HIMAGELIST hIml, int iconId);

class StrConvUT
{
private:
	wchar_t* m_body;

public:
	StrConvUT(const char* pSrc) :
		m_body(mir_a2u(pSrc)) {}

	~StrConvUT() {  mir_free(m_body); }
	operator const wchar_t* () const { return m_body; }
};

class StrConvAT
{
private:
	char* m_body;

public:
	StrConvAT(const wchar_t* pSrc) :
		m_body(mir_u2a(pSrc)) {}

	~StrConvAT() {  mir_free(m_body); }
	operator const char*  () const { return m_body; }
	operator const wchar_t* () const { return (wchar_t*)m_body; }  // type cast to fake the interface definition
	operator const LPARAM () const { return (LPARAM)m_body; }
};

#define StrConvT(x) StrConvUT(x)
#define StrConvTu(x) x
#define StrConvA(x) StrConvAT(x)
#define StrConvU(x) x

///////////////////////////////////////////////////////////////////////////////

extern "C"
{
	MIR_CORE_DLL(int) Langpack_MarkPluginLoaded(PLUGININFOEX* pInfo);
	MIR_CORE_DLL(int) GetSubscribersCount(HANDLE hHook);
	MIR_CORE_DLL(void) db_setCurrent(MIDatabase* _db);
};
