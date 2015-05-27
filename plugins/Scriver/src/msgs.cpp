/*
Scriver

Copyright (c) 2000-12 Miranda ICQ/IM project,

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

#include "commonheaders.h"

HCURSOR  hCurSplitNS, hCurSplitWE, hCurHyperlinkHand, hDragCursor;
HANDLE   hHookWinEvt, hHookWinPopup, hHookWinWrite;
HGENMENU hMsgMenuItem;
HMODULE hMsftEdit;

extern HWND GetParentWindow(MCONTACT hContact, BOOL bChat);

#define SCRIVER_DB_GETEVENTTEXT "Scriver/GetText"

static int SRMMStatusToPf2(int status)
{
	switch (status) {
	case ID_STATUS_ONLINE:
		return PF2_ONLINE;
	case ID_STATUS_AWAY:
		return PF2_SHORTAWAY;
	case ID_STATUS_DND:
		return PF2_HEAVYDND;
	case ID_STATUS_NA:
		return PF2_LONGAWAY;
	case ID_STATUS_OCCUPIED:
		return PF2_LIGHTDND;
	case ID_STATUS_FREECHAT:
		return PF2_FREECHAT;
	case ID_STATUS_INVISIBLE:
		return PF2_INVISIBLE;
	case ID_STATUS_ONTHEPHONE:
		return PF2_ONTHEPHONE;
	case ID_STATUS_OUTTOLUNCH:
		return PF2_OUTTOLUNCH;
	case ID_STATUS_OFFLINE:
		return MODEF_OFFLINE;
	}
	return 0;
}

int IsAutoPopup(MCONTACT hContact) {
	if (g_dat.flags & SMF_AUTOPOPUP) {
		char *szProto = GetContactProto(hContact);

		hContact = db_mc_getSrmmSub(hContact);
		if (hContact != NULL)
			szProto = GetContactProto(hContact);

		if (szProto && (g_dat.openFlags & SRMMStatusToPf2(CallProtoService(szProto, PS_GETSTATUS, 0, 0))))
			return 1;
	}
	return 0;
}

static INT_PTR ReadMessageCommand(WPARAM, LPARAM lParam)
{
	CLISTEVENT *pcle = (CLISTEVENT*)lParam;
	MCONTACT hContact = db_mc_tryMeta(pcle->hContact);

	HWND hwndExisting = WindowList_Find(g_dat.hMessageWindowList, hContact);
	if (hwndExisting == NULL) {
		NewMessageWindowLParam newData = { 0 };
		newData.hContact = hContact;
		CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSG), GetParentWindow(hContact, FALSE), DlgProcMessage, (LPARAM)&newData);
	}
	else SendMessage(GetParent(hwndExisting), CM_POPUPWINDOW, 0, (LPARAM)hwndExisting);
	return 0;
}

static int MessageEventAdded(WPARAM hContact, LPARAM lParam)
{
	MEVENT hDbEvent = (MEVENT)lParam;
	DBEVENTINFO dbei = { sizeof(dbei) };
	db_event_get(hDbEvent, &dbei);
	if (dbei.eventType == EVENTTYPE_MESSAGE && (dbei.flags & DBEF_READ))
		return 0;

	HWND hwnd = WindowList_Find(g_dat.hMessageWindowList, hContact);
	if (hwnd == NULL)
		hwnd = WindowList_Find(g_dat.hMessageWindowList, hContact = db_event_getContact(hDbEvent));
	if (hwnd)
		SendMessage(hwnd, HM_DBEVENTADDED, hContact, lParam);

	if (dbei.flags & DBEF_SENT || !DbEventIsMessageOrCustom(&dbei))
		return 0;

	CallServiceSync(MS_CLIST_REMOVEEVENT, hContact, 1);
	/* does a window for the contact exist? */
	if (hwnd == NULL) {
		/* new message */
		SkinPlaySound("AlertMsg");
		if (IsAutoPopup(hContact)) {
			NewMessageWindowLParam newData = { 0 };
			newData.hContact = hContact;
			HWND hParent = GetParentWindow(newData.hContact, FALSE);
			newData.flags = NMWLP_INCOMING;
			CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSG), hParent, DlgProcMessage, (LPARAM)&newData);
			return 0;
		}
	}
	if (hwnd == NULL || !IsWindowVisible(GetParent(hwnd))) {
		TCHAR *contactName = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR);
		TCHAR toolTip[256];

		CLISTEVENT cle = { sizeof(cle) };
		cle.flags = CLEF_TCHAR;
		cle.hContact = hContact;
		cle.hDbEvent = hDbEvent;
		cle.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
		cle.pszService = "SRMsg/ReadMessage";
		mir_sntprintf(toolTip, SIZEOF(toolTip), TranslateT("Message from %s"), contactName);
		cle.ptszTooltip = toolTip;
		CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR SendMessageCommandWorker(MCONTACT hContact, LPCSTR pszMsg, bool isWchar)
{
	hContact = db_mc_tryMeta(hContact);

	/* does the MCONTACT's protocol support IM messages? */
	char *szProto = GetContactProto(hContact);
	if (szProto == NULL)
		return 1; /* unknown contact */

	if (!CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND)
		return 1;

	HWND hwnd = WindowList_Find(g_dat.hMessageWindowList, hContact);
	if (hwnd != NULL) {
		if (pszMsg) {
			HWND hEdit = GetDlgItem(hwnd, IDC_MESSAGE);
			SendMessage(hEdit, EM_SETSEL, -1, GetWindowTextLength(hEdit));
			if (isWchar)
				SendMessageW(hEdit, EM_REPLACESEL, FALSE, (LPARAM)pszMsg);
			else
				SendMessageA(hEdit, EM_REPLACESEL, FALSE, (LPARAM)pszMsg);
		}
		SendMessage(GetParent(hwnd), CM_POPUPWINDOW, 0, (LPARAM)hwnd);
	}
	else {
		NewMessageWindowLParam newData = { 0 };
		newData.hContact = hContact;
		newData.szInitialText = pszMsg;
		newData.isWchar = isWchar;
		HWND hParent = GetParentWindow(newData.hContact, FALSE);
		CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSG), hParent, DlgProcMessage, (LPARAM)&newData);
	}
	return 0;
}

static INT_PTR SendMessageCommandW(WPARAM hContact, LPARAM lParam)
{
	return SendMessageCommandWorker(hContact, LPCSTR(lParam), true);
}

static INT_PTR SendMessageCommand(WPARAM hContact, LPARAM lParam)
{
	return SendMessageCommandWorker(hContact, LPCSTR(lParam), false);
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR TypingMessageCommand(WPARAM, LPARAM lParam)
{
	CLISTEVENT *cle = (CLISTEVENT*)lParam;
	if (cle)
		SendMessageCommand(cle->hContact, 0);
	return 0;
}

static int TypingMessage(WPARAM hContact, LPARAM lParam)
{
	if (!(g_dat.flags2 & SMF2_SHOWTYPING))
		return 0;

	hContact = db_mc_tryMeta(hContact);

	SkinPlaySound((lParam) ? "TNStart" : "TNStop");

	HWND hwnd = WindowList_Find(g_dat.hMessageWindowList, hContact);
	if (hwnd)
		SendMessage(hwnd, DM_TYPING, 0, lParam);
	else if (lParam && (g_dat.flags2 & SMF2_SHOWTYPINGTRAY)) {
		TCHAR szTip[256];

		mir_sntprintf(szTip, SIZEOF(szTip), TranslateT("%s is typing a message"), CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR));
		if ( ServiceExists(MS_CLIST_SYSTRAY_NOTIFY) && !(g_dat.flags2 & SMF2_SHOWTYPINGCLIST)) {
			MIRANDASYSTRAYNOTIFY tn;
			tn.szProto = NULL;
			tn.cbSize = sizeof(tn);
			tn.tszInfoTitle = TranslateT("Typing notification");
			tn.tszInfo = szTip;
			tn.dwInfoFlags = NIIF_INFO | NIIF_INTERN_UNICODE;
			tn.uTimeout = 1000 * 4;
			CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM)&tn);
		}
		else {
			CLISTEVENT cle = { sizeof(cle) };
			cle.hContact = hContact;
			cle.hDbEvent = 1;
			cle.flags = CLEF_ONLYAFEW | CLEF_TCHAR;
			cle.hIcon = GetCachedIcon("scriver_TYPING");
			cle.pszService = "SRMsg/TypingMessage";
			cle.ptszTooltip = szTip;
			CallServiceSync(MS_CLIST_REMOVEEVENT, hContact, 1);
			CallServiceSync(MS_CLIST_ADDEVENT, hContact, (LPARAM)&cle);
		}
	}
	return 0;
}

static int MessageSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
	char *szProto = GetContactProto(hContact);
	if (!mir_strcmp(cws->szModule, "CList") || !mir_strcmp(cws->szModule, szProto))
		WindowList_Broadcast(g_dat.hMessageWindowList, DM_CLISTSETTINGSCHANGED, hContact, lParam);
	return 0;
}

static int ContactDeleted(WPARAM wParam, LPARAM)
{
	HWND hwnd = WindowList_Find(g_dat.hMessageWindowList, wParam);
	if (hwnd)
		SendMessage(hwnd, WM_CLOSE, 0, 0);
	return 0;
}

static void RestoreUnreadMessageAlerts(void)
{
	TCHAR toolTip[256];

	DBEVENTINFO dbei = { sizeof(dbei) };

	CLISTEVENT cle = { sizeof(cle) };
	cle.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	cle.pszService = "SRMsg/ReadMessage";
	cle.flags = CLEF_TCHAR;
	cle.ptszTooltip = toolTip;

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		for (MEVENT hDbEvent = db_event_firstUnread(hContact); hDbEvent; hDbEvent = db_event_next(hContact, hDbEvent)) {
			dbei.cbBlob = 0;
			db_event_get(hDbEvent, &dbei);
			if (!(dbei.flags & (DBEF_SENT | DBEF_READ)) && DbEventIsMessageOrCustom(&dbei)) {
				int windowAlreadyExists = WindowList_Find(g_dat.hMessageWindowList, hContact) != NULL;
				if (windowAlreadyExists)
					continue;

				if (IsAutoPopup(hContact) && !windowAlreadyExists) {
					NewMessageWindowLParam newData = { 0 };
					newData.hContact = hContact;
					newData.flags = NMWLP_INCOMING;
					HWND hParent = GetParentWindow(newData.hContact, FALSE);
					CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSG), hParent, DlgProcMessage, (LPARAM)&newData);
				}
				else {
					cle.hContact = hContact;
					cle.hDbEvent = hDbEvent;
					mir_sntprintf(toolTip, SIZEOF(toolTip), TranslateT("Message from %s"), CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR));
					CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
				}
			}
		}
	}
}

static INT_PTR GetWindowAPI(WPARAM, LPARAM)
{
	return PLUGIN_MAKE_VERSION(0,0,0,3);
}

static INT_PTR GetWindowClass(WPARAM wParam, LPARAM lParam)
{
	char *szBuf = (char*)wParam;
	size_t size = (size_t)lParam;
	mir_snprintf(szBuf, size, "Scriver");
	return 0;
}

static INT_PTR GetWindowData(WPARAM wParam, LPARAM lParam)
{
	MessageWindowInputData *mwid = (MessageWindowInputData*)wParam;
	if (mwid == NULL || mwid->cbSize != sizeof(MessageWindowInputData) || mwid->hContact == NULL || mwid->uFlags != MSG_WINDOW_UFLAG_MSG_BOTH)
		return 1;

	MessageWindowData *mwd = (MessageWindowData*)lParam;
	if (mwd == NULL || mwd->cbSize != sizeof(MessageWindowData))
		return 1;

	HWND hwnd = WindowList_Find(g_dat.hMessageWindowList, mwid->hContact);
	if (hwnd == NULL)
		hwnd = SM_FindWindowByContact(mwid->hContact);
	mwd->uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
	mwd->hwndWindow = hwnd;
	mwd->local = 0;
	mwd->uState = SendMessage(hwnd, DM_GETWINDOWSTATE, 0, 0);
	return 0;
}

static INT_PTR SetStatusText(WPARAM hContact, LPARAM lParam)
{
	StatusTextData *st = (StatusTextData*)lParam;
	if (st != NULL && st->cbSize != sizeof(StatusTextData))
		return 1;

	ParentWindowData *pdat;
	HWND hwnd = WindowList_Find(g_dat.hMessageWindowList, hContact);
	if (hwnd == NULL) {
		hwnd = SM_FindWindowByContact(hContact);
		if (hwnd == NULL)
			return 1;

		SESSION_INFO *si = (SESSION_INFO*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (si == NULL || si->parent == NULL)
			return 1;

		pdat = si->parent;
	}
	else {
		SrmmWindowData *dat = (SrmmWindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (dat == NULL || dat->parent == NULL)
			return 1;

		pdat = dat->parent;
	}

	SendMessage(pdat->hwndStatus, SB_SETICON, 0, (LPARAM)(st == NULL ? 0 : st->hIcon));
	SendMessage(pdat->hwndStatus, SB_SETTEXT, 0, (LPARAM)(st == NULL ? _T("") : st->tszText));

	return 0;
}

static int PrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (hContact == 0)
		return 0;

	bool bEnabled = false;
	char *szProto = GetContactProto(hContact);
	if ( szProto ) {
		// leave this menu item hidden for chats
		if ( !db_get_b(hContact, szProto, "ChatRoom", 0))
			if ( CallProtoService( szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND)
				bEnabled = true;
	}

	Menu_ShowItem(hMsgMenuItem, bEnabled);
	return 0;
}

static int AvatarChanged(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0)          // protocol picture has changed...
		WindowList_Broadcast(g_dat.hMessageWindowList, DM_AVATARCHANGED, wParam, lParam);
	else {
		HWND hwnd = WindowList_Find(g_dat.hMessageWindowList, wParam);
		SendMessage(hwnd, DM_AVATARCHANGED, wParam, lParam);
	}
	return 0;
}

static void RegisterStatusIcons()
{
	StatusIconData sid = { sizeof(sid) };
	sid.szModule = SRMMMOD;
	sid.dwId = 1;
	sid.hIcon = GetCachedIcon("scriver_TYPING");
	sid.hIconDisabled = GetCachedIcon("scriver_TYPINGOFF");
	sid.flags = MBF_HIDDEN;
	Srmm_AddIcon(&sid);
}

void ChangeStatusIcons()
{
	StatusIconData sid = { sizeof(sid) };
	sid.szModule = SRMMMOD;
	sid.dwId = 1;
	sid.hIcon = GetCachedIcon("scriver_TYPING");
	sid.hIconDisabled = GetCachedIcon("scriver_TYPINGOFF");
	sid.flags = MBF_HIDDEN;
	Srmm_ModifyIcon(NULL, &sid);
}

int StatusIconPressed(WPARAM wParam, LPARAM lParam)
{
	StatusIconClickData *sicd = (StatusIconClickData *) lParam;
	if (mir_strcmp(SRMMMOD, sicd->szModule))
		return 0;

	HWND hwnd = WindowList_Find(g_dat.hMessageWindowList, wParam);
	if (hwnd == NULL)
		hwnd = SM_FindWindowByContact(wParam);

	if (hwnd != NULL)
		SendMessage(hwnd, DM_SWITCHTYPING, 0, 0);
	return 0;
}

static int ModuleLoad(WPARAM, LPARAM)
{
	g_dat.smileyAddInstalled = ServiceExists(MS_SMILEYADD_SHOWSELECTION) && ServiceExists(MS_SMILEYADD_REPLACESMILEYS);
	g_dat.popupInstalled = ServiceExists(MS_POPUP_ADDPOPUPT);
	g_dat.ieviewInstalled = ServiceExists(MS_IEVIEW_WINDOW);
	return 0;
}

static int MetaContactChanged(WPARAM hMeta, LPARAM)
{
	if (hMeta) {
		HWND hwnd = WindowList_Find(g_dat.hMessageWindowList, hMeta);
		if (hwnd != NULL)
			SendMessage(hwnd, DM_GETAVATAR, 0, 0);
	}
	return 0;
}

static int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	ReloadGlobals();
	LoadGlobalIcons();
	LoadMsgLogIcons();
	ModuleLoad(0, 0);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = -2000090000;
	mi.flags = CMIF_DEFAULT;
	mi.icolibItem = LoadSkinnedIconHandle( SKINICON_EVENT_MESSAGE );
	mi.pszName = LPGEN("&Message");
	mi.pszService = MS_MSG_SENDMESSAGE;
	hMsgMenuItem = Menu_AddContactMenuItem(&mi);
	Skin_ReleaseIcon(mi.hIcon);

	HookEvent(ME_SMILEYADD_OPTIONSCHANGED, SmileySettingsChanged);
	HookEvent(ME_IEVIEW_OPTIONSCHANGED, SmileySettingsChanged);
	HookEvent(ME_AV_AVATARCHANGED, AvatarChanged);
	HookEvent(ME_FONT_RELOAD, FontServiceFontsChanged);
	HookEvent(ME_MSG_ICONPRESSED, StatusIconPressed);
	HookEvent(ME_MC_DEFAULTTCHANGED, MetaContactChanged);

	RestoreUnreadMessageAlerts();
	Chat_ModulesLoaded(wParam, lParam);
	RegisterStatusIcons();
	return 0;
}

int OnSystemPreshutdown(WPARAM, LPARAM)
{
	WindowList_Broadcast(g_dat.hMessageWindowList, WM_CLOSE, 0, 0);
	WindowList_Broadcast(g_dat.hParentWindowList, WM_CLOSE, 0, 0);
	DeinitStatusIcons();
	return 0;
}

int OnUnloadModule(void)
{
	Chat_Unload();

	DestroyCursor(hCurSplitNS);
	DestroyCursor(hCurHyperlinkHand);
	DestroyCursor(hCurSplitWE);
	DestroyCursor(hDragCursor);

	DestroyHookableEvent(hHookWinEvt);
	DestroyHookableEvent(hHookWinPopup);
	DestroyHookableEvent(hHookWinWrite);

	ReleaseIcons();
	FreeMsgLogIcons();
	FreeLibrary(GetModuleHandleA("Msftedit.dll"));
	RichUtil_Unload();
	FreeGlobals();
	FreeLibrary(hMsftEdit);
	return 0;
}

int OnLoadModule(void)
{
	hMsftEdit = LoadLibrary(_T("Msftedit.dll"));
	if (hMsftEdit == NULL) {
		if (IDYES != MessageBox(0,
			TranslateT
			("Miranda could not load the built-in message module, Msftedit.dll is missing. If you are using WINE, please make sure you have Msftedit.dll installed. Press 'Yes' to continue loading Miranda."),
			TranslateT("Information"), MB_YESNO | MB_ICONINFORMATION))
			return 1;
		return 0;
	}

	InitGlobals();
	RichUtil_Load();
	InitStatusIcons();
	RegisterIcons();
	RegisterFontServiceFonts();
	RegisterKeyBindings();

	HookEvent(ME_OPT_INITIALISE, OptInitialise);
	HookEvent(ME_DB_EVENT_ADDED, MessageEventAdded);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, MessageSettingChanged);
	HookEvent(ME_DB_CONTACT_DELETED, ContactDeleted);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_SKIN_ICONSCHANGED, IconsChanged);
	HookEvent(ME_PROTO_CONTACTISTYPING, TypingMessage);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnSystemPreshutdown);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);
	HookEvent(ME_SYSTEM_MODULELOAD, ModuleLoad);
	HookEvent(ME_SYSTEM_MODULEUNLOAD, ModuleLoad);

	CreateServiceFunction(MS_MSG_SENDMESSAGE, SendMessageCommand);
	CreateServiceFunction(MS_MSG_SENDMESSAGEW, SendMessageCommandW);
	CreateServiceFunction(MS_MSG_GETWINDOWAPI, GetWindowAPI);
	CreateServiceFunction(MS_MSG_GETWINDOWCLASS, GetWindowClass);
	CreateServiceFunction(MS_MSG_GETWINDOWDATA, GetWindowData);
	CreateServiceFunction(MS_MSG_SETSTATUSTEXT, SetStatusText);
	CreateServiceFunction("SRMsg/ReadMessage", ReadMessageCommand);
	CreateServiceFunction("SRMsg/TypingMessage", TypingMessageCommand);

	hHookWinEvt = CreateHookableEvent(ME_MSG_WINDOWEVENT);
	hHookWinPopup = CreateHookableEvent(ME_MSG_WINDOWPOPUP);
	hHookWinWrite = CreateHookableEvent(ME_MSG_PRECREATEEVENT);

	SkinAddNewSoundEx("RecvMsgActive", LPGEN("Instant messages"), LPGEN("Incoming (focused window)"));
	SkinAddNewSoundEx("RecvMsgInactive", LPGEN("Instant messages"), LPGEN("Incoming (unfocused window)"));
	SkinAddNewSoundEx("AlertMsg", LPGEN("Instant messages"), LPGEN("Incoming (new session)"));
	SkinAddNewSoundEx("SendMsg", LPGEN("Instant messages"), LPGEN("Outgoing"));
	SkinAddNewSoundEx("TNStart", LPGEN("Instant messages"), LPGEN("Contact started typing"));
	SkinAddNewSoundEx("TNStop",  LPGEN("Instant messages"), LPGEN("Contact stopped typing"));

	hCurSplitNS = LoadCursor(NULL, IDC_SIZENS);
	hCurSplitWE = LoadCursor(NULL, IDC_SIZEWE);
	hCurHyperlinkHand = LoadCursor(NULL, IDC_HAND);
	if (hCurHyperlinkHand == NULL)
		hCurHyperlinkHand = LoadCursor(g_hInst, MAKEINTRESOURCE(IDC_HYPERLINKHAND));
	hDragCursor = LoadCursor(g_hInst, MAKEINTRESOURCE(IDC_DRAGCURSOR));

	Chat_Load();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////// 

CREOleCallback  reOleCallback;
CREOleCallback2 reOleCallback2;

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
	if (refCount == 0)
		StgCreateDocfile(NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_DELETEONRELEASE, 0, &pictStg);

	return ++refCount;
}

STDMETHODIMP_(ULONG) CREOleCallback::Release()
{
	if (--refCount == 0) {
		if (pictStg) {
			pictStg->Release();
			pictStg = NULL;
		}
	}
	return refCount;
}

STDMETHODIMP CREOleCallback::ContextSensitiveHelp(BOOL)
{
	return S_OK;
}

STDMETHODIMP CREOleCallback::DeleteObject(LPOLEOBJECT)
{
	return S_OK;
}

STDMETHODIMP CREOleCallback::GetClipboardData(CHARRANGE*, DWORD, LPDATAOBJECT*)
{
	return E_NOTIMPL;
}

STDMETHODIMP CREOleCallback::GetContextMenu(WORD, LPOLEOBJECT, CHARRANGE*, HMENU*)
{
	return E_INVALIDARG;
}

STDMETHODIMP CREOleCallback::GetDragDropEffect(BOOL, DWORD, LPDWORD)
{
	return S_OK;
}

STDMETHODIMP CREOleCallback::GetInPlaceContext(LPOLEINPLACEFRAME*, LPOLEINPLACEUIWINDOW*, LPOLEINPLACEFRAMEINFO)
{
	return E_INVALIDARG;
}

STDMETHODIMP CREOleCallback::GetNewStorage(LPSTORAGE *lplpstg)
{
	TCHAR sztName[64];
	mir_sntprintf(sztName, SIZEOF(sztName), _T("s%u"), nextStgId++);
	if (pictStg == NULL)
		return STG_E_MEDIUMFULL;
	return pictStg->CreateStorage(sztName, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, lplpstg);
}

STDMETHODIMP CREOleCallback::QueryAcceptData(LPDATAOBJECT, CLIPFORMAT*, DWORD, BOOL, HGLOBAL)
{
	return S_OK;
}

STDMETHODIMP CREOleCallback::QueryInsertObject(LPCLSID, LPSTORAGE, LONG)
{
	return S_OK;
}

STDMETHODIMP CREOleCallback::ShowContainerUI(BOOL)
{
	return S_OK;
}

STDMETHODIMP CREOleCallback2::QueryAcceptData(LPDATAOBJECT, CLIPFORMAT *lpcfFormat, DWORD, BOOL, HGLOBAL)
{
	*lpcfFormat = CF_UNICODETEXT;
	return S_OK;
}

