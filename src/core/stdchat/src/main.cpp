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
CLIST_INTERFACE *pcli;
CHAT_MANAGER *pci;

SESSION_INFO g_TabSession;

HINSTANCE   g_hInst;
HANDLE      g_hWindowList;
HMENU       g_hMenu = NULL;
int         hLangpack;

BOOL        IEviewInstalled = FALSE;
BOOL        SmileyAddInstalled = FALSE;
BOOL        PopupInstalled = FALSE;

HBRUSH      hListBkgBrush = NULL;
HBRUSH      hListSelectedBkgBrush = NULL;

HANDLE hBuildMenuEvent, hSendEvent;

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
	TabM_RemoveAll();
	return 0;
}

void TabsInit(void)
{
	ZeroMemory(&g_TabSession, sizeof(SESSION_INFO));
	g_TabSession.iType = GCW_TABROOM;
	g_TabSession.iSplitterX = g_Settings.iSplitterX;
	g_TabSession.iSplitterY = g_Settings.iSplitterY;
	g_TabSession.iLogFilterFlags = (int)db_get_dw(NULL, "Chat", "FilterFlags", 0x03E0);
	g_TabSession.bFilterEnabled = db_get_b(NULL, "Chat", "FilterEnabled", 0);
	g_TabSession.bNicklistEnabled = db_get_b(NULL, "Chat", "ShowNicklist", 1);
	g_TabSession.iFG = 4;
	g_TabSession.bFGSet = TRUE;
	g_TabSession.iBG = 2;
	g_TabSession.bBGSet = TRUE;

	HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);
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

static void OnSessionDblClick(SESSION_INFO *si)
{
	if (g_Settings.TabsEnable)
		SendMessage(si->hWnd, GC_REMOVETAB, 1, (LPARAM)si);
	else
		PostMessage(si->hWnd, GC_CLOSEWINDOW, 0, 0);
}

static void OnSessionRemove(SESSION_INFO *si)
{
	if (!g_Settings.TabsEnable) {
		if (si->hWnd)
			SendMessage(si->hWnd, GC_EVENT_CONTROL + WM_USER + 500, SESSION_TERMINATE, 0);
	}
	else if (g_TabSession.hWnd)
		SendMessage(g_TabSession.hWnd, GC_REMOVETAB, 1, (LPARAM)si);

	if (si->hWnd)
		g_TabSession.nUsersInNicklist = 0;
}

static void OnSessionRename(SESSION_INFO *si)
{
	if (g_TabSession.hWnd && g_Settings.TabsEnable) {
		g_TabSession.ptszName = si->ptszName;
		SendMessage(g_TabSession.hWnd, GC_SESSIONNAMECHANGE, 0, (LPARAM)si);
	}
}

static void OnSessionReplace(SESSION_INFO *si)
{
	if (si->hWnd)
		g_TabSession.nUsersInNicklist = 0;

	if (!g_Settings.TabsEnable) {
		if (si->hWnd)
			RedrawWindow(GetDlgItem(si->hWnd, IDC_LIST), NULL, NULL, RDW_INVALIDATE);
	}
	else if (g_TabSession.hWnd)
		RedrawWindow(GetDlgItem(g_TabSession.hWnd, IDC_LIST), NULL, NULL, RDW_INVALIDATE);
}

static void OnSessionOffline(SESSION_INFO *si)
{
	if (si->hWnd) {
		g_TabSession.nUsersInNicklist = 0;
		if (g_Settings.TabsEnable)
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
	if (g_Settings.TabsEnable && si->hWnd)
		g_TabSession.pStatuses = si->pStatuses;
}

static void OnSetStatus(SESSION_INFO *si, int wStatus)
{
	if (g_Settings.TabsEnable) {
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

static void OnFlashWindow(SESSION_INFO *si, int bInactive)
{
	if (!bInactive)
		return;
	
	if (!g_Settings.TabsEnable && si->hWnd && db_get_b(NULL, "Chat", "FlashWindowHighlight", 0) != 0)
		SetTimer(si->hWnd, TIMERID_FLASHWND, 900, NULL);
	if (g_Settings.TabsEnable && g_TabSession.hWnd)
		SendMessage(g_TabSession.hWnd, GC_SETMESSAGEHIGHLIGHT, 0, (LPARAM)si);
}

static void OnLoadSettings()
{
	g_Settings.TabsEnable = db_get_b(NULL, "Chat", "Tabs", 1);
	g_Settings.TabRestore = db_get_b(NULL, "Chat", "TabRestore", 0);
	g_Settings.TabsAtBottom = db_get_b(NULL, "Chat", "TabBottom", 0);
	g_Settings.TabCloseOnDblClick = db_get_b(NULL, "Chat", "TabCloseOnDblClick", 0);

	if (hListBkgBrush != NULL)
		DeleteObject(hListBkgBrush);
	hListBkgBrush = CreateSolidBrush(db_get_dw(NULL, "Chat", "ColorNicklistBG", GetSysColor(COLOR_WINDOW)));

	if (hListSelectedBkgBrush != NULL)
		DeleteObject(hListSelectedBkgBrush);
	hListSelectedBkgBrush = CreateSolidBrush(db_get_dw(NULL, "Chat", "ColorNicklistSelectedBG", GetSysColor(COLOR_HIGHLIGHT)));
}

extern "C" __declspec(dllexport) int Load(void)
{
	// set the memory & utf8 managers
	mir_getLP(&pluginInfo);
	mir_getCLI();
	
	mir_getCI();
	pci->pSettings = &g_Settings;
	pci->OnAddUser = OnAddUser;
	pci->OnNewUser = OnNewUser;
	pci->OnRemoveUser = OnRemoveUser;
	
	pci->OnAddStatus = OnAddStatus;
	pci->OnSetStatus = OnSetStatus;
	pci->OnSetTopic = OnSetTopic;
	
	pci->OnAddLog = OnAddLog;
	pci->OnClearLog = OnClearLog;
	
	pci->OnSessionOffline = OnSessionOffline;
	pci->OnSessionRemove = OnSessionRemove;
	pci->OnSessionRename = OnSessionRename;
	pci->OnSessionReplace = OnSessionReplace;
	pci->OnSessionDblClick = OnSessionDblClick;
	
	pci->OnEventBroadcast = OnEventBroadcast;
	pci->OnLoadSettings = OnLoadSettings;
	pci->OnSetStatusBar = OnSetStatusBar;
	pci->OnFlashWindow = OnFlashWindow;
	pci->ShowRoom = ShowRoom;

	g_hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU));
	AddIcons();
	LoadIcons();
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

	DestroyMenu(g_hMenu);
	OptionsUnInit();
	return 0;
}

void LoadLogIcons(void)
{
	pci->hIcons[ICON_ACTION]     = LoadIconEx("log_action", FALSE);
	pci->hIcons[ICON_ADDSTATUS]  = LoadIconEx("log_addstatus", FALSE);
	pci->hIcons[ICON_HIGHLIGHT]  = LoadIconEx("log_highlight", FALSE);
	pci->hIcons[ICON_INFO]       = LoadIconEx("log_info", FALSE);
	pci->hIcons[ICON_JOIN]       = LoadIconEx("log_join", FALSE);
	pci->hIcons[ICON_KICK]       = LoadIconEx("log_kick", FALSE);
	pci->hIcons[ICON_MESSAGE]    = LoadIconEx("log_message_in", FALSE);
	pci->hIcons[ICON_MESSAGEOUT] = LoadIconEx("log_message_out", FALSE);
	pci->hIcons[ICON_NICK]       = LoadIconEx("log_nick", FALSE);
	pci->hIcons[ICON_NOTICE]     = LoadIconEx("log_notice", FALSE);
	pci->hIcons[ICON_PART]       = LoadIconEx("log_part", FALSE);
	pci->hIcons[ICON_QUIT]       = LoadIconEx("log_quit", FALSE);
	pci->hIcons[ICON_REMSTATUS]  = LoadIconEx("log_removestatus", FALSE);
	pci->hIcons[ICON_TOPIC]      = LoadIconEx("log_topic", FALSE);
	pci->hIcons[ICON_STATUS1]    = LoadIconEx("status1", FALSE);
	pci->hIcons[ICON_STATUS2]    = LoadIconEx("status2", FALSE);
	pci->hIcons[ICON_STATUS3]    = LoadIconEx("status3", FALSE);
	pci->hIcons[ICON_STATUS4]    = LoadIconEx("status4", FALSE);
	pci->hIcons[ICON_STATUS0]    = LoadIconEx("status0", FALSE);
	pci->hIcons[ICON_STATUS5]    = LoadIconEx("status5", FALSE);
}

void LoadIcons(void)
{
	memset(pci->hIcons, 0, sizeof(pci->hIcons));

	LoadLogIcons();

	pci->hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 0, 3);
	pci->hIconsList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 0, 100);
	ImageList_AddIcon(pci->hIconsList, LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
	ImageList_AddIcon(pci->hIconsList, LoadIconEx("overlay", FALSE));
	ImageList_SetOverlayImage(pci->hIconsList, 1, 1);
	ImageList_AddIcon(pci->hImageList, (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BLANK), IMAGE_ICON, 0, 0, 0));
	ImageList_AddIcon(pci->hImageList, (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BLANK), IMAGE_ICON, 0, 0, 0));
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