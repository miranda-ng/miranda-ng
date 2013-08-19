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
HANDLE      g_hWindowList;
HMENU       g_hMenu = NULL;
int         hLangpack;

FONTINFO    aFonts[OPTIONS_FONTCOUNT];
HICON       hIcons[30];
BOOL        IEviewInstalled = FALSE;
HBRUSH      hListBkgBrush = NULL;
BOOL        SmileyAddInstalled = FALSE;
BOOL        PopupInstalled = FALSE;
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

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	MIRANDA_VERSION_DWORD,
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
    {0x9d6c3213, 0x2b4, 0x4fe1, { 0x92, 0xe6, 0x52, 0x6d, 0xe1, 0x4f, 0x8d, 0x65 }} //{9D6C3213-02B4-4fe1-92E6-526DE14F8D65}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	g_hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_CHAT, MIID_LAST};

extern "C" __declspec(dllexport) int Load(void)
{
	// set the memory & utf8 managers
	mir_getLP( &pluginInfo );

	UpgradeCheck();

	g_hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU));
	HookEvents();
	CreateServiceFunctions();
	CreateHookableEvents();
	OptionsInit();
	TabsInit();
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	db_set_w(NULL, "Chat", "SplitterX", (WORD)g_Settings.iSplitterX);
	db_set_w(NULL, "Chat", "SplitterY", (WORD)g_Settings.iSplitterY);
	db_set_dw(NULL, "Chat", "roomx", g_Settings.iX);
	db_set_dw(NULL, "Chat", "roomy", g_Settings.iY);
	db_set_dw(NULL, "Chat", "roomwidth" , g_Settings.iWidth);
	db_set_dw(NULL, "Chat", "roomheight", g_Settings.iHeight);

	CList_SetAllOffline(TRUE, NULL);

	mir_free(pszActiveWndID);
	mir_free(pszActiveWndModule);

	DestroyMenu(g_hMenu);
	DestroyHookableEvents();
	FreeIcons();
	OptionsUnInit();
	FreeLibrary(GetModuleHandle(_T("riched20.dll")));
	UnhookEvents();
	return 0;
}

void UpgradeCheck(void)
{
	DWORD dwVersion = db_get_dw(NULL, "Chat", "OldVersion", PLUGIN_MAKE_VERSION(0,2,9,9));
	if (pluginInfo.version > dwVersion && dwVersion < PLUGIN_MAKE_VERSION(0,3,0,0)) {
		db_unset(NULL, "ChatFonts",	"Font18");
		db_unset(NULL, "ChatFonts",	"Font18Col");
		db_unset(NULL, "ChatFonts",	"Font18Set");
		db_unset(NULL, "ChatFonts",	"Font18Size");
		db_unset(NULL, "ChatFonts",	"Font18Sty");
		db_unset(NULL, "ChatFonts",	"Font19");
		db_unset(NULL, "ChatFonts",	"Font19Col");
		db_unset(NULL, "ChatFonts",	"Font19Set");
		db_unset(NULL, "ChatFonts",	"Font19Size");
		db_unset(NULL, "ChatFonts",	"Font19Sty");
		db_unset(NULL, "Chat",		"ColorNicklistLines");
		db_unset(NULL, "Chat",		"NicklistIndent");
		db_unset(NULL, "Chat",		"NicklistRowDist");
		db_unset(NULL, "Chat",		"ShowFormatButtons");
		db_unset(NULL, "Chat",		"ShowLines");
		db_unset(NULL, "Chat",		"ShowName");
		db_unset(NULL, "Chat",		"ShowTopButtons");
		db_unset(NULL, "Chat",		"SplitterX");
		db_unset(NULL, "Chat",		"SplitterY");
		db_unset(NULL, "Chat",		"IconFlags");
		db_unset(NULL, "Chat",		"LogIndentEnabled");
	}

	db_set_dw(NULL, "Chat", "OldVersion", pluginInfo.version);
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
	memset(hIcons, 0, sizeof(hIcons));

	LoadLogIcons();
	LoadMsgLogBitmaps();

	hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),IsWinVerXPPlus()? ILC_COLOR32 | ILC_MASK : ILC_COLOR16 | ILC_MASK,0,3);
	hIconsList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),IsWinVerXPPlus()? ILC_COLOR32 | ILC_MASK : ILC_COLOR16 | ILC_MASK,0,100);
	ImageList_AddIcon(hIconsList,LoadSkinnedIcon( SKINICON_EVENT_MESSAGE));
	ImageList_AddIcon(hIconsList,LoadIconEx( "overlay", FALSE ));
	ImageList_SetOverlayImage(hIconsList, 1, 1);
	ImageList_AddIcon(hImageList, (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BLANK), IMAGE_ICON, 0, 0, 0));
	ImageList_AddIcon(hImageList, (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BLANK), IMAGE_ICON, 0, 0, 0));
}

void FreeIcons(void)
{
	FreeMsgLogBitmaps();
	ImageList_Destroy(hImageList);
	ImageList_Destroy(hIconsList);
}

CREOleCallback reOleCallback;

STDMETHODIMP CREOleCallback::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if (IsEqualIID(riid, IID_IRichEditOleCallback)) {
		*ppvObj = this;
		AddRef();
		return S_OK;
	}
	*ppvObj = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CREOleCallback::AddRef()
{
	if (refCount == 0) {
		if (S_OK != StgCreateDocfile(NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_DELETEONRELEASE, 0, &pictStg))
			pictStg = NULL;
		nextStgId = 0;
	}
	return ++refCount;
}

STDMETHODIMP_(ULONG) CREOleCallback::Release()
{
	if (--refCount == 0) {
		if (pictStg)
			pictStg->Release();
	}
	return refCount;
}

STDMETHODIMP CREOleCallback::ContextSensitiveHelp(BOOL fEnterMode)
{
	return S_OK;
}

STDMETHODIMP CREOleCallback::DeleteObject(LPOLEOBJECT lpoleobj)
{
	return S_OK;
}

STDMETHODIMP CREOleCallback::GetClipboardData(CHARRANGE * lpchrg, DWORD reco, LPDATAOBJECT * lplpdataobj)
{
	return E_NOTIMPL;
}

STDMETHODIMP CREOleCallback::GetContextMenu(WORD seltype, LPOLEOBJECT lpoleobj, CHARRANGE * lpchrg, HMENU * lphmenu)
{
	return E_INVALIDARG;
}

STDMETHODIMP CREOleCallback::GetDragDropEffect(BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect)
{
	return S_OK;
}

STDMETHODIMP CREOleCallback::GetInPlaceContext(LPOLEINPLACEFRAME * lplpFrame, LPOLEINPLACEUIWINDOW * lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	return E_INVALIDARG;
}

STDMETHODIMP CREOleCallback::GetNewStorage(LPSTORAGE * lplpstg)
{
	WCHAR szwName[64];
	char szName[64];
	mir_snprintf(szName, SIZEOF(szName), "s%u", nextStgId++);
	MultiByteToWideChar(CP_ACP, 0, szName, -1, szwName, SIZEOF(szwName));
	if (pictStg == NULL)
		return STG_E_MEDIUMFULL;
	return pictStg->CreateStorage(szwName, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, lplpstg);
}

STDMETHODIMP CREOleCallback::QueryAcceptData(LPDATAOBJECT lpdataobj, CLIPFORMAT * lpcfFormat, DWORD reco, BOOL fReally, HGLOBAL hMetaPict)
{
	return S_OK;
}

STDMETHODIMP CREOleCallback::QueryInsertObject(LPCLSID lpclsid, LPSTORAGE lpstg, LONG cp)
{
	return S_OK;
}

STDMETHODIMP CREOleCallback::ShowContainerUI(BOOL fShow)
{
	return S_OK;
}