/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson

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

#include "chat.h"

//globals
HINSTANCE   g_hInst;
PLUGINLINK  *pluginLink;
HANDLE      g_hWindowList;
HMENU       g_hMenu = NULL;
int         hLangpack;

struct MM_INTERFACE memoryManagerInterface;
struct UTF8_INTERFACE utfi;

FONTINFO    aFonts[OPTIONS_FONTCOUNT];
HICON       hIcons[30];
BOOL        IEviewInstalled = FALSE;
HBRUSH      hListBkgBrush = NULL;
BOOL        SmileyAddInstalled = FALSE;
BOOL        PopUpInstalled = FALSE;
HBRUSH      hEditBkgBrush = NULL;
HBRUSH      hListSelectedBkgBrush = NULL;

HIMAGELIST  hImageList = NULL;

HIMAGELIST  hIconsList = NULL;

TCHAR*      pszActiveWndID = 0;
char*       pszActiveWndModule = 0;

/* Missing MinGW GUIDs */
#ifdef __MINGW32__
const CLSID IID_IRichEditOle = { 0x00020D00, 0x00, 0x00, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
const CLSID IID_IRichEditOleCallback = { 0x00020D03, 0x00, 0x00, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
#endif

struct GlobalLogSettings_t g_Settings;

static void InitREOleCallback(void);

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"Chat",
	__VERSION_DWORD,
	"Provides chat rooms for protocols supporting it",
	"Miranda team",
	"project-info@miranda-im.org",
	"© 2003-2010 Miranda team",
	"http://miranda-im.org/",
	UNICODE_AWARE,
	0,
    #ifdef _UNICODE
    {0x9d6c3213, 0x2b4, 0x4fe1, { 0x92, 0xe6, 0x52, 0x6d, 0xe2, 0x4f, 0x8d, 0x65 }} //{9D6C3213-02B4-4fe1-92E6-526DE24F8D65}
    #else
    {0xd3d9f953, 0x85bc, 0x486c, { 0xa7, 0xbe, 0x31, 0x43, 0xa6, 0xfd, 0x29, 0xf }} //{D3D9F953-85BC-486c-A7BE-3143A6FD290F}
    #endif
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	g_hInst = hinstDLL;
	return TRUE;
}

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if (mirandaVersion < MIRANDA_VERSION_CORE) return NULL;
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_CHAT, MIID_LAST};
__declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}

int __declspec(dllexport) Load(PLUGINLINK *link)
{
	BOOL bFlag = FALSE;
	HINSTANCE hDll;

#ifndef NDEBUG //mem leak detector :-) Thanks Tornado!
	int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG); // Get current flag
	flag |= _CRTDBG_LEAK_CHECK_DF; // Turn on leak-checking bit
	_CrtSetDbgFlag(flag); // Set flag to the new value
#endif

	pluginLink = link;

	// set the memory & utf8 managers
	mir_getMMI( &memoryManagerInterface );
	mir_getUTFI( &utfi );
	mir_getLP( &pluginInfo );

	hDll = LoadLibraryA("riched20.dll");
	if ( hDll ) {
		char modulePath[MAX_PATH];
		if (GetModuleFileNameA(hDll, modulePath, MAX_PATH)) {
			DWORD dummy;
			VS_FIXEDFILEINFO* vsInfo;
			UINT vsInfoSize;
			DWORD size = GetFileVersionInfoSizeA(modulePath, &dummy);
			BYTE* buffer = (BYTE*) mir_alloc(size);

			if (GetFileVersionInfoA(modulePath, 0, size, buffer))
                if (VerQueryValueA(buffer, "\\", (LPVOID*) &vsInfo, &vsInfoSize))
                    if (LOWORD(vsInfo->dwFileVersionMS) != 0)
                        bFlag= TRUE;
			mir_free(buffer);
	}	}

	if ( !bFlag ) {
		if (IDYES == MessageBox(0, TranslateT("Miranda could not load the Chat plugin because Microsoft Rich Edit v 3 is missing.\nIf you are using Windows 95/98/NT or WINE please upgrade your Rich Edit control.\n\nDo you want to download an update now?."),TranslateT("Information"),MB_YESNO|MB_ICONINFORMATION))
			CallService(MS_UTILS_OPENURL, 1, (LPARAM) "http://members.chello.se/matrix/re3/richupd.exe");
		FreeLibrary(GetModuleHandleA("riched20.dll"));
		return 1;
	}

	UpgradeCheck();

	g_hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU));
	InitREOleCallback();
	HookEvents();
	CreateServiceFunctions();
	CreateHookableEvents();
	OptionsInit();
	TabsInit();
	return 0;
}

int __declspec(dllexport) Unload(void)
{
	DBWriteContactSettingWord(NULL, "Chat", "SplitterX", (WORD)g_Settings.iSplitterX);
	DBWriteContactSettingWord(NULL, "Chat", "SplitterY", (WORD)g_Settings.iSplitterY);
	DBWriteContactSettingDword(NULL, "Chat", "roomx", g_Settings.iX);
	DBWriteContactSettingDword(NULL, "Chat", "roomy", g_Settings.iY);
	DBWriteContactSettingDword(NULL, "Chat", "roomwidth" , g_Settings.iWidth);
	DBWriteContactSettingDword(NULL, "Chat", "roomheight", g_Settings.iHeight);

	CList_SetAllOffline(TRUE, NULL);

	mir_free( pszActiveWndID );
	mir_free( pszActiveWndModule );

	DestroyMenu(g_hMenu);
	DestroyServiceFunctions();
	DestroyHookableEvents();
	FreeIcons();
	OptionsUnInit();
	FreeLibrary(GetModuleHandleA("riched20.dll"));
	UnhookEvents();
	return 0;
}

void UpgradeCheck(void)
{
	DWORD dwVersion = DBGetContactSettingDword(NULL, "Chat", "OldVersion", PLUGIN_MAKE_VERSION(0,2,9,9));
	if (	pluginInfo.version > dwVersion)
	{
		if (dwVersion < PLUGIN_MAKE_VERSION(0,3,0,0))
		{
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font18");
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font18Col");
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font18Set");
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font18Size");
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font18Sty");
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font19");
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font19Col");
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font19Set");
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font19Size");
			DBDeleteContactSetting(NULL, "ChatFonts",	"Font19Sty");
			DBDeleteContactSetting(NULL, "Chat",		"ColorNicklistLines");
			DBDeleteContactSetting(NULL, "Chat",		"NicklistIndent");
			DBDeleteContactSetting(NULL, "Chat",		"NicklistRowDist");
			DBDeleteContactSetting(NULL, "Chat",		"ShowFormatButtons");
			DBDeleteContactSetting(NULL, "Chat",		"ShowLines");
			DBDeleteContactSetting(NULL, "Chat",		"ShowName");
			DBDeleteContactSetting(NULL, "Chat",		"ShowTopButtons");
			DBDeleteContactSetting(NULL, "Chat",		"SplitterX");
			DBDeleteContactSetting(NULL, "Chat",		"SplitterY");
			DBDeleteContactSetting(NULL, "Chat",		"IconFlags");
			DBDeleteContactSetting(NULL, "Chat",		"LogIndentEnabled");
	}	}

	DBWriteContactSettingDword(NULL, "Chat", "OldVersion", pluginInfo.version);
}

void LoadLogIcons(void)
{
	hIcons[ICON_ACTION]     = LoadIconEx( "log_action", FALSE );
	hIcons[ICON_ADDSTATUS]  = LoadIconEx( "log_addstatus", FALSE );
	hIcons[ICON_HIGHLIGHT]  = LoadIconEx( "log_highlight", FALSE );
	hIcons[ICON_INFO]       = LoadIconEx( "log_info", FALSE );
	hIcons[ICON_JOIN]       = LoadIconEx( "log_join", FALSE );
	hIcons[ICON_KICK]       = LoadIconEx( "log_kick", FALSE );
	hIcons[ICON_MESSAGE]    = LoadIconEx( "log_message_in", FALSE );
	hIcons[ICON_MESSAGEOUT] = LoadIconEx( "log_message_out", FALSE );
	hIcons[ICON_NICK]       = LoadIconEx( "log_nick", FALSE );
	hIcons[ICON_NOTICE]     = LoadIconEx( "log_notice", FALSE );
	hIcons[ICON_PART]       = LoadIconEx( "log_part", FALSE );
	hIcons[ICON_QUIT]       = LoadIconEx( "log_quit", FALSE );
	hIcons[ICON_REMSTATUS]  = LoadIconEx( "log_removestatus", FALSE );
	hIcons[ICON_TOPIC]      = LoadIconEx( "log_topic", FALSE );
	hIcons[ICON_STATUS1]    = LoadIconEx( "status1", FALSE );
	hIcons[ICON_STATUS2]    = LoadIconEx( "status2", FALSE );
	hIcons[ICON_STATUS3]    = LoadIconEx( "status3", FALSE );
	hIcons[ICON_STATUS4]    = LoadIconEx( "status4", FALSE );
	hIcons[ICON_STATUS0]    = LoadIconEx( "status0", FALSE );
	hIcons[ICON_STATUS5]    = LoadIconEx( "status5", FALSE );
}

void LoadIcons(void)
{
	int i;

	for(i = 0; i < 20; i++)
		hIcons[i] = NULL;

	LoadLogIcons();
	LoadMsgLogBitmaps();

	hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),IsWinVerXPPlus()? ILC_COLOR32 | ILC_MASK : ILC_COLOR16 | ILC_MASK,0,3);
	hIconsList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),IsWinVerXPPlus()? ILC_COLOR32 | ILC_MASK : ILC_COLOR16 | ILC_MASK,0,100);
	ImageList_AddIcon(hIconsList,LoadSkinnedIcon( SKINICON_EVENT_MESSAGE));
	ImageList_AddIcon(hIconsList,LoadIconEx( "overlay", FALSE ));
	ImageList_SetOverlayImage(hIconsList, 1, 1);
	ImageList_AddIcon(hImageList,LoadImage(g_hInst,MAKEINTRESOURCE(IDI_BLANK),IMAGE_ICON,0,0,0));
	ImageList_AddIcon(hImageList,LoadImage(g_hInst,MAKEINTRESOURCE(IDI_BLANK),IMAGE_ICON,0,0,0));
}

void FreeIcons(void)
{
	FreeMsgLogBitmaps();
	ImageList_Destroy(hImageList);
	ImageList_Destroy(hIconsList);
}

static IRichEditOleCallbackVtbl reOleCallbackVtbl;
struct CREOleCallback reOleCallback;

static STDMETHODIMP_(ULONG) CREOleCallback_QueryInterface(struct CREOleCallback *lpThis, REFIID riid, LPVOID * ppvObj)
{
	if (IsEqualIID(riid, &IID_IRichEditOleCallback)) {
		*ppvObj = lpThis;
		lpThis->lpVtbl->AddRef((IRichEditOleCallback *) lpThis);
		return S_OK;
	}
	*ppvObj = NULL;
	return E_NOINTERFACE;
}

static STDMETHODIMP_(ULONG) CREOleCallback_AddRef(struct CREOleCallback *lpThis)
{
	if (lpThis->refCount == 0) {
		if (S_OK != StgCreateDocfile(NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_DELETEONRELEASE, 0, &lpThis->pictStg))
			lpThis->pictStg = NULL;
		lpThis->nextStgId = 0;
	}
	return ++lpThis->refCount;
}

static STDMETHODIMP_(ULONG) CREOleCallback_Release(struct CREOleCallback *lpThis)
{
	if (--lpThis->refCount == 0) {
		if (lpThis->pictStg)
			lpThis->pictStg->lpVtbl->Release(lpThis->pictStg);
	}
	return lpThis->refCount;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_ContextSensitiveHelp(struct CREOleCallback *lpThis, BOOL fEnterMode)
{
	return S_OK;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_DeleteObject(struct CREOleCallback *lpThis, LPOLEOBJECT lpoleobj)
{
	return S_OK;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_GetClipboardData(struct CREOleCallback *lpThis, CHARRANGE * lpchrg, DWORD reco, LPDATAOBJECT * lplpdataobj)
{
	return E_NOTIMPL;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_GetContextMenu(struct CREOleCallback *lpThis, WORD seltype, LPOLEOBJECT lpoleobj, CHARRANGE * lpchrg, HMENU * lphmenu)
{
	return E_INVALIDARG;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_GetDragDropEffect(struct CREOleCallback *lpThis, BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect)
{
	return S_OK;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_GetInPlaceContext(struct CREOleCallback *lpThis, LPOLEINPLACEFRAME * lplpFrame, LPOLEINPLACEUIWINDOW * lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	return E_INVALIDARG;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_GetNewStorage(struct CREOleCallback *lpThis, LPSTORAGE * lplpstg)
{
	WCHAR szwName[64];
	char szName[64];
	wsprintfA(szName, "s%u", lpThis->nextStgId);
	MultiByteToWideChar(CP_ACP, 0, szName, -1, szwName, SIZEOF(szwName));
	if (lpThis->pictStg == NULL)
		return STG_E_MEDIUMFULL;
	return lpThis->pictStg->lpVtbl->CreateStorage(lpThis->pictStg, szwName, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, lplpstg);
}

static STDMETHODIMP_(HRESULT) CREOleCallback_QueryAcceptData(struct CREOleCallback *lpThis, LPDATAOBJECT lpdataobj, CLIPFORMAT * lpcfFormat, DWORD reco, BOOL fReally, HGLOBAL hMetaPict)
{
	return S_OK;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_QueryInsertObject(struct CREOleCallback *lpThis, LPCLSID lpclsid, LPSTORAGE lpstg, LONG cp)
{
	return S_OK;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_ShowContainerUI(struct CREOleCallback *lpThis, BOOL fShow)
{
	return S_OK;
}

static void InitREOleCallback(void)
{
	reOleCallback.lpVtbl = &reOleCallbackVtbl;
	reOleCallback.lpVtbl->AddRef = (ULONG(__stdcall *) (IRichEditOleCallback *)) CREOleCallback_AddRef;
	reOleCallback.lpVtbl->Release = (ULONG(__stdcall *) (IRichEditOleCallback *)) CREOleCallback_Release;
	reOleCallback.lpVtbl->QueryInterface = (ULONG(__stdcall *) (IRichEditOleCallback *, REFIID, PVOID *)) CREOleCallback_QueryInterface;
	reOleCallback.lpVtbl->ContextSensitiveHelp = (HRESULT(__stdcall *) (IRichEditOleCallback *, BOOL)) CREOleCallback_ContextSensitiveHelp;
	reOleCallback.lpVtbl->DeleteObject = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPOLEOBJECT)) CREOleCallback_DeleteObject;
	reOleCallback.lpVtbl->GetClipboardData = (HRESULT(__stdcall *) (IRichEditOleCallback *, CHARRANGE *, DWORD, LPDATAOBJECT *)) CREOleCallback_GetClipboardData;
	reOleCallback.lpVtbl->GetContextMenu = (HRESULT(__stdcall *) (IRichEditOleCallback *, WORD, LPOLEOBJECT, CHARRANGE *, HMENU *)) CREOleCallback_GetContextMenu;
	reOleCallback.lpVtbl->GetDragDropEffect = (HRESULT(__stdcall *) (IRichEditOleCallback *, BOOL, DWORD, LPDWORD)) CREOleCallback_GetDragDropEffect;
	reOleCallback.lpVtbl->GetInPlaceContext = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPOLEINPLACEFRAME *, LPOLEINPLACEUIWINDOW *, LPOLEINPLACEFRAMEINFO))CREOleCallback_GetInPlaceContext;
	reOleCallback.lpVtbl->GetNewStorage = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPSTORAGE *)) CREOleCallback_GetNewStorage;
	reOleCallback.lpVtbl->QueryAcceptData = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPDATAOBJECT, CLIPFORMAT *, DWORD, BOOL, HGLOBAL)) CREOleCallback_QueryAcceptData;
	reOleCallback.lpVtbl->QueryInsertObject = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPCLSID, LPSTORAGE, LONG)) CREOleCallback_QueryInsertObject;
	reOleCallback.lpVtbl->ShowContainerUI = (HRESULT(__stdcall *) (IRichEditOleCallback *, BOOL)) CREOleCallback_ShowContainerUI;
	reOleCallback.refCount = 0;
}
