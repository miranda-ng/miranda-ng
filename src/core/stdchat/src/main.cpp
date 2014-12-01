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

// globals
CLIST_INTERFACE *pcli;
CHAT_MANAGER *pci, saveCI;

SESSION_INFO g_TabSession;
HMENU g_hMenu = NULL;

HINSTANCE g_hInst;
int hLangpack;

BOOL SmileyAddInstalled = FALSE, PopupInstalled = FALSE;
HIMAGELIST hIconsList;

GlobalLogSettings g_Settings;

/* Missing MinGW GUIDs */
#ifdef __MINGW32__
const CLSID IID_IRichEditOle = { 0x00020D00, 0x00, 0x00, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
const CLSID IID_IRichEditOleCallback = { 0x00020D03, 0x00, 0x00, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
#endif

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

int OnShutdown(WPARAM, LPARAM)
{
	for (SESSION_INFO *si = pci->wndList; si; si = si->next)
		SendMessage(si->hWnd, WM_CLOSE, 0, 0);

	TabM_RemoveAll();
	ImageList_Destroy(hIconsList);
	return 0;
}

static void OnCreateModule(MODULEINFO *mi)
{
	mi->OnlineIconIndex = ImageList_AddIcon(hIconsList, LoadSkinnedProtoIcon(mi->pszModule, ID_STATUS_ONLINE));
	mi->hOnlineIcon = ImageList_GetIcon(hIconsList, mi->OnlineIconIndex, ILD_TRANSPARENT);
	mi->hOnlineTalkIcon = ImageList_GetIcon(hIconsList, mi->OnlineIconIndex, ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));
	ImageList_AddIcon(hIconsList, mi->hOnlineTalkIcon);

	mi->OfflineIconIndex = ImageList_AddIcon(hIconsList, LoadSkinnedProtoIcon(mi->pszModule, ID_STATUS_OFFLINE));
	mi->hOfflineIcon = ImageList_GetIcon(hIconsList, mi->OfflineIconIndex, ILD_TRANSPARENT);
	mi->hOfflineTalkIcon = ImageList_GetIcon(hIconsList, mi->OfflineIconIndex, ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));
	ImageList_AddIcon(hIconsList, mi->hOfflineTalkIcon);
}

static void OnAddLog(SESSION_INFO *si, int isOk)
{
	if (isOk && si->hWnd) {
		g_TabSession.pLog = si->pLog;
		g_TabSession.pLogEnd = si->pLogEnd;
		SendMessage(si->hWnd, GC_ADDLOG, 0, 0);
	}
	else if (si->hWnd) {
		g_TabSession.pLog = si->pLog;
		g_TabSession.pLogEnd = si->pLogEnd;
		SendMessage(si->hWnd, GC_REDRAWLOG2, 0, 0);
	}
}

static void OnClearLog(SESSION_INFO *si)
{
	if (si->hWnd) {
		g_TabSession.pLog = si->pLog;
		g_TabSession.pLogEnd = si->pLogEnd;
	}
}

static void OnDblClickSession(SESSION_INFO *si)
{
	if (g_Settings.bTabsEnable)
		SendMessage(si->hWnd, GC_REMOVETAB, 1, (LPARAM)si);
	else
		PostMessage(si->hWnd, GC_CLOSEWINDOW, 0, 0);
}

static void OnRemoveSession(SESSION_INFO *si)
{
	if (!g_Settings.bTabsEnable) {
		if (si->hWnd)
			SendMessage(si->hWnd, GC_EVENT_CONTROL + WM_USER + 500, SESSION_TERMINATE, 0);
	}
	else if (g_TabSession.hWnd)
		SendMessage(g_TabSession.hWnd, GC_REMOVETAB, 1, (LPARAM)si);

	if (si->hWnd)
		g_TabSession.nUsersInNicklist = 0;
}

static void OnRenameSession(SESSION_INFO *si)
{
	if (g_TabSession.hWnd && g_Settings.bTabsEnable) {
		g_TabSession.ptszName = si->ptszName;
		SendMessage(g_TabSession.hWnd, GC_SESSIONNAMECHANGE, 0, (LPARAM)si);
	}
}

static void OnReplaceSession(SESSION_INFO *si)
{
	if (si->hWnd)
		g_TabSession.nUsersInNicklist = 0;

	if (!g_Settings.bTabsEnable) {
		if (si->hWnd)
			RedrawWindow(GetDlgItem(si->hWnd, IDC_LIST), NULL, NULL, RDW_INVALIDATE);
	}
	else if (g_TabSession.hWnd)
		RedrawWindow(GetDlgItem(g_TabSession.hWnd, IDC_LIST), NULL, NULL, RDW_INVALIDATE);
}

static void OnOfflineSession(SESSION_INFO *si)
{
	if (si->hWnd) {
		g_TabSession.nUsersInNicklist = 0;
		if (g_Settings.bTabsEnable)
			g_TabSession.pUsers = 0;
	}
}

static void OnEventBroadcast(SESSION_INFO *si, GCEVENT *gce)
{
	if (pci->SM_AddEvent(si->ptszID, si->pszModule, gce, FALSE) && si->hWnd && si->bInitDone) {
		g_TabSession.pLog = si->pLog;
		g_TabSession.pLogEnd = si->pLogEnd;
		SendMessage(si->hWnd, GC_ADDLOG, 0, 0);
	}
	else if (si->hWnd && si->bInitDone) {
		g_TabSession.pLog = si->pLog;
		g_TabSession.pLogEnd = si->pLogEnd;
		SendMessage(si->hWnd, GC_REDRAWLOG2, 0, 0);
	}
}

static void OnSetStatusBar(SESSION_INFO *si)
{
	if (si->hWnd) {
		g_TabSession.ptszStatusbarText = si->ptszStatusbarText;
		SendMessage(si->hWnd, GC_UPDATESTATUSBAR, 0, 0);
	}
}

static void OnAddUser(SESSION_INFO *si, USERINFO*)
{
	if (si->hWnd)
		g_TabSession.nUsersInNicklist++;
}

static void OnNewUser(SESSION_INFO *si, USERINFO*)
{
	if (si->hWnd) {
		g_TabSession.pUsers = si->pUsers;
		SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);
	}
}

static void OnRemoveUser(SESSION_INFO *si, USERINFO*)
{
	if (si->hWnd) {
		g_TabSession.pUsers = si->pUsers;
		g_TabSession.nUsersInNicklist--;
	}
}

static void OnAddStatus(SESSION_INFO *si, STATUSINFO*)
{
	if (g_Settings.bTabsEnable && si->hWnd)
		g_TabSession.pStatuses = si->pStatuses;
}

static void OnSetStatus(SESSION_INFO *si, int wStatus)
{
	if (g_Settings.bTabsEnable) {
		if (si->hWnd)
			g_TabSession.wStatus = wStatus;
		if (g_TabSession.hWnd)
			PostMessage(g_TabSession.hWnd, GC_FIXTABICONS, 0, (LPARAM)si);
	}
}

static void OnSetTopic(SESSION_INFO *si)
{
	if (si->hWnd)
		g_TabSession.ptszTopic = si->ptszTopic;
}	

static void OnFlashHighlight(SESSION_INFO *si, int bInactive)
{
	if (!bInactive)
		return;

	if (!g_Settings.bTabsEnable && si->hWnd && g_Settings.bFlashWindowHighlight)
		SetTimer(si->hWnd, TIMERID_FLASHWND, 900, NULL);
	if (g_Settings.bTabsEnable && g_TabSession.hWnd)
		SendMessage(g_TabSession.hWnd, GC_SETMESSAGEHIGHLIGHT, 0, (LPARAM)si);
}

static void OnFlashWindow(SESSION_INFO *si, int bInactive)
{
	if (!bInactive)
		return;

	if (!g_Settings.bTabsEnable && si->hWnd && g_Settings.bFlashWindow)
		SetTimer(si->hWnd, TIMERID_FLASHWND, 900, NULL);
	if (g_Settings.bTabsEnable && g_TabSession.hWnd)
		SendMessage(g_TabSession.hWnd, GC_SETTABHIGHLIGHT, 0, (LPARAM)si);
}

static BOOL DoTrayIcon(SESSION_INFO *si, GCEVENT *gce)
{
	if (gce->pDest->iType & g_Settings.dwTrayIconFlags)
		return saveCI.DoTrayIcon(si, gce);
	return TRUE;
}

static BOOL DoPopup(SESSION_INFO *si, GCEVENT *gce)
{
	if (gce->pDest->iType & g_Settings.dwPopupFlags)
		return saveCI.DoPopup(si, gce);
	return TRUE;
}

static void OnLoadSettings()
{
	if (g_Settings.MessageAreaFont)
		DeleteObject(g_Settings.MessageAreaFont);

	LOGFONT lf;
	LoadMessageFont(&lf, &g_Settings.MessageAreaColor);
	g_Settings.MessageAreaFont = CreateFontIndirect(&lf);

	g_Settings.iX = db_get_dw(NULL, CHAT_MODULE, "roomx", -1);
	g_Settings.iY = db_get_dw(NULL, CHAT_MODULE, "roomy", -1);

	g_Settings.bTabsEnable = db_get_b(NULL, CHAT_MODULE, "Tabs", 1) != 0;
	g_Settings.TabRestore = db_get_b(NULL, CHAT_MODULE, "TabRestore", 0) != 0;
	g_Settings.TabsAtBottom = db_get_b(NULL, CHAT_MODULE, "TabBottom", 0) != 0;
	g_Settings.TabCloseOnDblClick = db_get_b(NULL, CHAT_MODULE, "TabCloseOnDblClick", 0) != 0;
}

static void RegisterFonts()
{
	ColourIDT colourid = { sizeof(colourid) };
	strncpy(colourid.dbSettingsGroup, CHAT_MODULE, sizeof(colourid.dbSettingsGroup));
	_tcsncpy(colourid.group, LPGENT("Chat module"), SIZEOF(colourid.group));

	strncpy(colourid.setting, "ColorLogBG", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Group chat log background"), SIZEOF(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	ColourRegisterT(&colourid);

	strncpy(colourid.setting, "ColorMessageBG", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Message background"), SIZEOF(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	ColourRegisterT(&colourid);

	strncpy(colourid.setting, "ColorNicklistBG", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Nick list background"), SIZEOF(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	ColourRegisterT(&colourid);

	strncpy(colourid.setting, "ColorNicklistLines", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Nick list lines"), SIZEOF(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_INACTIVEBORDER);
	ColourRegisterT(&colourid);

	strncpy(colourid.setting, "ColorNicklistSelectedBG", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Nick list background (selected)"), SIZEOF(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_HIGHLIGHT);
	ColourRegisterT(&colourid);
}

static void TabsInit()
{
	memset(&g_TabSession, 0, sizeof(SESSION_INFO));
	g_TabSession.iType = GCW_TABROOM;
	g_TabSession.iSplitterX = g_Settings.iSplitterX;
	g_TabSession.iSplitterY = g_Settings.iSplitterY;
	g_TabSession.iLogFilterFlags = (int)db_get_dw(NULL, CHAT_MODULE, "FilterFlags", 0x03E0);
	g_TabSession.bFilterEnabled = db_get_b(NULL, CHAT_MODULE, "FilterEnabled", 0);
	g_TabSession.bNicklistEnabled = db_get_b(NULL, CHAT_MODULE, "ShowNicklist", 1);
	g_TabSession.iFG = 4;
	g_TabSession.bFGSet = TRUE;
	g_TabSession.iBG = 2;
	g_TabSession.bBGSet = TRUE;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);
	mir_getCLI();

	AddIcons();
	RegisterFonts();

	CHAT_MANAGER_INITDATA data = { &g_Settings, sizeof(MODULEINFO), sizeof(SESSION_INFO), LPGENT("Chat module"), FONTMODE_SKIP };
	mir_getCI(&data);
	saveCI = *pci;

	pci->OnAddUser = OnAddUser;
	pci->OnNewUser = OnNewUser;
	pci->OnRemoveUser = OnRemoveUser;

	pci->OnAddStatus = OnAddStatus;
	pci->OnSetStatus = OnSetStatus;
	pci->OnSetTopic = OnSetTopic;

	pci->OnAddLog = OnAddLog;
	pci->OnClearLog = OnClearLog;

	pci->OnCreateModule = OnCreateModule;
	pci->OnOfflineSession = OnOfflineSession;
	pci->OnRemoveSession = OnRemoveSession;
	pci->OnRenameSession = OnRenameSession;
	pci->OnReplaceSession = OnReplaceSession;
	pci->OnDblClickSession = OnDblClickSession;

	pci->OnEventBroadcast = OnEventBroadcast;
	pci->OnLoadSettings = OnLoadSettings;
	pci->OnSetStatusBar = OnSetStatusBar;
	pci->OnFlashWindow = OnFlashWindow;
	pci->OnFlashHighlight = OnFlashHighlight;
	pci->ShowRoom = ShowRoom;

	pci->DoPopup = DoPopup;
	pci->DoTrayIcon = DoTrayIcon;
	pci->ReloadSettings();

	g_hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU));
	LoadIcons();
	TabsInit();

	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);
	HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	db_set_w(NULL, CHAT_MODULE, "SplitterX", (WORD)g_Settings.iSplitterX);
	db_set_w(NULL, CHAT_MODULE, "SplitterY", (WORD)g_Settings.iSplitterY);
	db_set_dw(NULL, CHAT_MODULE, "roomx", g_Settings.iX);
	db_set_dw(NULL, CHAT_MODULE, "roomy", g_Settings.iY);
	db_set_dw(NULL, CHAT_MODULE, "roomwidth", g_Settings.iWidth);
	db_set_dw(NULL, CHAT_MODULE, "roomheight", g_Settings.iHeight);

	if (g_Settings.MessageAreaFont)
		DeleteObject(g_Settings.MessageAreaFont);
	DestroyMenu(g_hMenu);
	return 0;
}

void LoadIcons(void)
{
	hIconsList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 0, 100);
	ImageList_AddIcon(hIconsList, LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
	ImageList_AddIcon(hIconsList, LoadIconEx("overlay", FALSE));
	ImageList_SetOverlayImage(hIconsList, 1, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////

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
