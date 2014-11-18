/*

Copyright 2000-12 Miranda IM, 2012-14 Miranda NG project,
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
#include "statusicon.h"

/* Missing MinGW GUIDs */
#ifdef __MINGW32__
const CLSID IID_IRichEditOle = { 0x00020D00, 0x00, 0x00, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
const CLSID IID_IRichEditOleCallback = { 0x00020D03, 0x00, 0x00, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
#endif

HCURSOR  hCurSplitNS, hCurSplitWE, hCurHyperlinkHand;
HANDLE   hHookWinEvt, hHookWinPopup, hHookWinWrite;
HGENMENU hMsgMenuItem;

static int SRMMStatusToPf2(int status)
{
	switch (status) {
		case ID_STATUS_ONLINE:     return PF2_ONLINE;
		case ID_STATUS_AWAY:       return PF2_SHORTAWAY;
		case ID_STATUS_DND:        return PF2_HEAVYDND;
		case ID_STATUS_NA:         return PF2_LONGAWAY;
		case ID_STATUS_OCCUPIED:   return PF2_LIGHTDND;
		case ID_STATUS_FREECHAT:   return PF2_FREECHAT;
		case ID_STATUS_INVISIBLE:  return PF2_INVISIBLE;
		case ID_STATUS_ONTHEPHONE: return PF2_ONTHEPHONE;
		case ID_STATUS_OUTTOLUNCH: return PF2_OUTTOLUNCH;
		case ID_STATUS_OFFLINE:    return MODEF_OFFLINE;
	}
	return 0;
}

static int MessageEventAdded(WPARAM hContact, LPARAM lParam)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	db_event_get((HANDLE)lParam, &dbei);

	if (dbei.flags & (DBEF_SENT | DBEF_READ) || !(dbei.eventType == EVENTTYPE_MESSAGE || DbEventIsForMsgWindow(&dbei)))
		return 0;

	CallServiceSync(MS_CLIST_REMOVEEVENT, hContact, 1);
	/* does a window for the contact exist? */
	HWND hwnd = WindowList_Find(g_dat.hMessageWindowList, hContact);
	if (hwnd) {
		if (!db_get_b(NULL, SRMMMOD, SRMSGSET_DONOTSTEALFOCUS, SRMSGDEFSET_DONOTSTEALFOCUS)) {
			ShowWindow(hwnd, SW_RESTORE);
			SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			SetForegroundWindow(hwnd);
			SkinPlaySound("RecvMsgActive");
		}
		else {
			if (GetForegroundWindow() == hwnd)
				SkinPlaySound("RecvMsgActive");
			else
				SkinPlaySound("RecvMsgInactive");
		}
		return 0;
	}
	/* new message */
	SkinPlaySound("AlertMsg");

	char *szProto = GetContactProto(hContact);
	if (szProto && (g_dat.openFlags & SRMMStatusToPf2(CallProtoService(szProto, PS_GETSTATUS, 0, 0)))) {
		NewMessageWindowLParam newData = { 0 };
		newData.hContact = hContact;
		newData.noActivate = db_get_b(NULL, SRMMMOD, SRMSGSET_DONOTSTEALFOCUS, SRMSGDEFSET_DONOTSTEALFOCUS);
		CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSG), NULL, DlgProcMessage, (LPARAM)&newData);
		return 0;
	}

	CLISTEVENT cle = { sizeof(cle) };
	cle.hContact = hContact;
	cle.hDbEvent = (HANDLE)lParam;
	cle.flags = CLEF_TCHAR;
	cle.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	cle.pszService = "SRMsg/ReadMessage";
	TCHAR toolTip[256];
	mir_sntprintf(toolTip, SIZEOF(toolTip), TranslateT("Message from %s"), pcli->pfnGetContactDisplayName(hContact, 0));
	cle.ptszTooltip = toolTip;
	CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
	return 0;
}

INT_PTR SendMessageCmd(MCONTACT hContact, char *msg, int isWchar)
{
	/* does the MCONTACT's protocol support IM messages? */
	char *szProto = GetContactProto(hContact);
	if (!szProto || (!CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND))
		return 1;

	hContact = db_mc_tryMeta(hContact);

	HWND hwnd = WindowList_Find(g_dat.hMessageWindowList, hContact);
	if (hwnd) {
		if (msg) {
			SendDlgItemMessage(hwnd, IDC_MESSAGE, EM_SETSEL, -1, SendDlgItemMessage(hwnd, IDC_MESSAGE, WM_GETTEXTLENGTH, 0, 0));
			if (isWchar)
				SendDlgItemMessageW(hwnd, IDC_MESSAGE, EM_REPLACESEL, FALSE, (LPARAM)msg);
			else
				SendDlgItemMessageA(hwnd, IDC_MESSAGE, EM_REPLACESEL, FALSE, (LPARAM)msg);
		}
		ShowWindow(hwnd, SW_RESTORE);
		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		SetForegroundWindow(hwnd);
	}
	else {
		NewMessageWindowLParam newData = { 0 };
		newData.hContact = hContact;
		newData.szInitialText = msg;
		newData.isWchar = isWchar;
		CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSG), NULL, DlgProcMessage, (LPARAM)&newData);
	}
	return 0;
}

static INT_PTR SendMessageCommand_W(WPARAM wParam, LPARAM lParam)
{
	return SendMessageCmd(wParam, (char*)lParam, TRUE);
}

static INT_PTR SendMessageCommand(WPARAM wParam, LPARAM lParam)
{
	return SendMessageCmd(wParam, (char*)lParam, FALSE);
}

static INT_PTR ReadMessageCommand(WPARAM wParam, LPARAM lParam)
{
	CLISTEVENT *cle = (CLISTEVENT *)lParam;
	if (cle)
		SendMessageCmd(cle->hContact, NULL, 0);

	return 0;
}

static int TypingMessage(WPARAM hContact, LPARAM lParam)
{
	if (!(g_dat.flags & SMF_SHOWTYPING))
		return 0;

	hContact = db_mc_tryMeta(hContact);

	SkinPlaySound((lParam) ? "TNStart" : "TNStop");

	HWND hwnd = WindowList_Find(g_dat.hMessageWindowList, hContact);
	if (hwnd)
		SendMessage(hwnd, DM_TYPING, 0, lParam);
	else if (lParam && (g_dat.flags & SMF_SHOWTYPINGTRAY)) {
		TCHAR szTip[256];
		mir_sntprintf(szTip, SIZEOF(szTip), TranslateT("%s is typing a message"), pcli->pfnGetContactDisplayName(hContact, 0));

		if (ServiceExists(MS_CLIST_SYSTRAY_NOTIFY) && !(g_dat.flags & SMF_SHOWTYPINGCLIST)) {
			MIRANDASYSTRAYNOTIFY tn = { sizeof(tn) };
			tn.tszInfoTitle = TranslateT("Typing notification");
			tn.tszInfo = szTip;
			tn.dwInfoFlags = NIIF_INFO;
			tn.dwInfoFlags |= NIIF_INTERN_UNICODE;
			tn.uTimeout = 1000 * 4;
			CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM)& tn);
		}
		else {
			CLISTEVENT cle = { sizeof(cle) };
			cle.hContact = hContact;
			cle.hDbEvent = (HANDLE)1;
			cle.flags = CLEF_ONLYAFEW | CLEF_TCHAR;
			cle.hIcon = LoadSkinnedIcon(SKINICON_OTHER_TYPING);
			cle.pszService = "SRMsg/ReadMessage";
			cle.ptszTooltip = szTip;
			CallServiceSync(MS_CLIST_REMOVEEVENT, hContact, 1);
			CallServiceSync(MS_CLIST_ADDEVENT, hContact, (LPARAM)&cle);
			Skin_ReleaseIcon(cle.hIcon);
		}
	}
	return 0;
}

static int MessageSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (cws->szModule == NULL)
		return 0;

	if (!strcmp(cws->szModule, "CList"))
		WindowList_Broadcast(g_dat.hMessageWindowList, DM_UPDATETITLE, (WPARAM)cws, 0);
	else if (hContact) {
		if (cws->szSetting && !strcmp(cws->szSetting, "Timezone"))
			WindowList_Broadcast(g_dat.hMessageWindowList, DM_NEWTIMEZONE, (WPARAM)cws, 0);
		else {
			char *szProto = GetContactProto(hContact);
			if (szProto && !strcmp(cws->szModule, szProto))
				WindowList_Broadcast(g_dat.hMessageWindowList, DM_UPDATETITLE, (WPARAM)cws, 0);
		}
	}
	return 0;
}

// If a contact gets deleted, close its message window if there is any
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

	CLISTEVENT cle = { sizeof(cle) };
	cle.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	cle.pszService = "SRMsg/ReadMessage";
	cle.flags = CLEF_TCHAR;
	cle.ptszTooltip = toolTip;

	DBEVENTINFO dbei = { sizeof(dbei) };

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		for (HANDLE hDbEvent = db_event_firstUnread(hContact); hDbEvent; hDbEvent = db_event_next(hContact, hDbEvent)) {
			bool autoPopup = false;
			dbei.cbBlob = 0;
			db_event_get(hDbEvent, &dbei);
			if (!(dbei.flags & (DBEF_SENT | DBEF_READ)) && (dbei.eventType == EVENTTYPE_MESSAGE || DbEventIsForMsgWindow(&dbei))) {
				int windowAlreadyExists = WindowList_Find(g_dat.hMessageWindowList, hContact) != NULL;
				if (windowAlreadyExists)
					continue;

				char *szProto = GetContactProto(hContact);
				if (szProto && (g_dat.openFlags & SRMMStatusToPf2(CallProtoService(szProto, PS_GETSTATUS, 0, 0))))
					autoPopup = true;

				if (autoPopup && !windowAlreadyExists) {
					NewMessageWindowLParam newData = { 0 };
					newData.hContact = hContact;
					newData.noActivate = db_get_b(NULL, SRMMMOD, SRMSGSET_DONOTSTEALFOCUS, SRMSGDEFSET_DONOTSTEALFOCUS);
					CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSG), NULL, DlgProcMessage, (LPARAM)& newData);
				}
				else {
					cle.hContact = hContact;
					cle.hDbEvent = hDbEvent;
					mir_sntprintf(toolTip, SIZEOF(toolTip), TranslateT("Message from %s"), pcli->pfnGetContactDisplayName(hContact, 0));
					CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
				}
			}
		}
	}
}

void RegisterSRMMFonts(void);

static int FontsChanged(WPARAM, LPARAM)
{
	WindowList_Broadcast(g_dat.hMessageWindowList, DM_OPTIONSAPPLIED, 0, 0);
	return 0;
}

static int SplitmsgModulesLoaded(WPARAM, LPARAM)
{
	RegisterSRMMFonts();
	LoadMsgLogIcons();

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = -2000090000;
	mi.flags = CMIF_DEFAULT;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_EVENT_MESSAGE);
	mi.pszName = LPGEN("&Message");
	mi.pszService = MS_MSG_SENDMESSAGE;
	hMsgMenuItem = Menu_AddContactMenuItem(&mi);

	HookEvent(ME_FONT_RELOAD, FontsChanged);

	RestoreUnreadMessageAlerts();
	return 0;
}

int PreshutdownSendRecv(WPARAM, LPARAM)
{
	WindowList_Broadcast(g_dat.hMessageWindowList, WM_CLOSE, 0, 0);

	DeinitStatusIcons();
	return 0;
}

static int IconsChanged(WPARAM, LPARAM)
{
	FreeMsgLogIcons();
	LoadMsgLogIcons();
	WindowList_Broadcast(g_dat.hMessageWindowList, DM_REMAKELOG, 0, 0);
	// change all the icons
	WindowList_Broadcast(g_dat.hMessageWindowList, DM_UPDATEWINICON, 0, 0);
	return 0;
}

static int PrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	if (hContact) {
		bool bEnabled = false;
		char *szProto = GetContactProto(hContact);
		if (szProto) {
			// leave this menu item hidden for chats
			if (!db_get_b(hContact, szProto, "ChatRoom", 0))
				if (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND)
					bEnabled = true;
		}

		Menu_ShowItem(hMsgMenuItem, bEnabled);
	}
	return 0;
}

static INT_PTR GetWindowAPI(WPARAM, LPARAM)
{
	return PLUGIN_MAKE_VERSION(0, 0, 0, 4);
}

static INT_PTR GetWindowClass(WPARAM wParam, LPARAM lParam)
{
	char *szBuf = (char*)wParam;
	int size = (int)lParam;
	mir_snprintf(szBuf, size, SRMMMOD);
	return 0;
}

static INT_PTR SetStatusText(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd = WindowList_Find(g_dat.hMessageWindowList, wParam);
	if (hwnd == NULL)
		return 1;

	SrmmWindowData *dat = (SrmmWindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (dat == NULL)
		return 1;

	StatusTextData *st = (StatusTextData*)lParam;
	if (st != NULL && st->cbSize != sizeof(StatusTextData))
		return 1;

	SendMessage(dat->hwndStatus, SB_SETICON, 0, (LPARAM)(st == NULL ? 0 : st->hIcon));
	SendMessage(dat->hwndStatus, SB_SETTEXT, 0, (LPARAM)(st == NULL ? _T("") : st->tszText));

	return 0;
}

static INT_PTR GetWindowData(WPARAM wParam, LPARAM lParam)
{
	MessageWindowInputData *mwid = (MessageWindowInputData*)wParam;
	if (mwid == NULL || (mwid->cbSize != sizeof(MessageWindowInputData)) || (mwid->hContact == NULL) || (mwid->uFlags != MSG_WINDOW_UFLAG_MSG_BOTH))
		return 1;

	MessageWindowData *mwd = (MessageWindowData*)lParam;
	if(mwd == NULL || (mwd->cbSize != sizeof(MessageWindowData)))
		return 1;

	HWND hwnd = WindowList_Find(g_dat.hMessageWindowList, mwid->hContact);
	mwd->uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
	mwd->hwndWindow = hwnd;
	mwd->local = 0;
	mwd->uState = SendMessage(hwnd, DM_GETWINDOWSTATE, 0, 0);
	return 0;
}

static TCHAR tszError[] = LPGENT("Miranda could not load the built-in message module, Msftedit.dll is missing. Press 'Yes' to continue loading Miranda.");

int LoadSendRecvMessageModule(void)
{
	if (LoadLibraryA("Msftedit.dll") == NULL) {
		if (IDYES != MessageBox(0, TranslateTS(tszError), TranslateT("Information"), MB_YESNO | MB_ICONINFORMATION))
			return 1;
		return 0;
	}

	InitGlobals();
	RichUtil_Load();
	OleInitialize(NULL);
	InitOptions();
	msgQueue_init();

	HookEvent(ME_DB_EVENT_ADDED, MessageEventAdded);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, MessageSettingChanged);
	HookEvent(ME_DB_CONTACT_DELETED, ContactDeleted);
	HookEvent(ME_SYSTEM_MODULESLOADED, SplitmsgModulesLoaded);
	HookEvent(ME_SKIN_ICONSCHANGED, IconsChanged);
	HookEvent(ME_PROTO_CONTACTISTYPING, TypingMessage);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreshutdownSendRecv);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

	CreateServiceFunction(MS_MSG_SENDMESSAGE, SendMessageCommand);
	CreateServiceFunction(MS_MSG_SENDMESSAGEW, SendMessageCommand_W);
	CreateServiceFunction(MS_MSG_GETWINDOWAPI, GetWindowAPI);
	CreateServiceFunction(MS_MSG_GETWINDOWCLASS, GetWindowClass);
	CreateServiceFunction(MS_MSG_GETWINDOWDATA, GetWindowData);
	CreateServiceFunction(MS_MSG_SETSTATUSTEXT, SetStatusText);
	CreateServiceFunction("SRMsg/ReadMessage", ReadMessageCommand);

	hHookWinEvt = CreateHookableEvent(ME_MSG_WINDOWEVENT);
	hHookWinPopup = CreateHookableEvent(ME_MSG_WINDOWPOPUP);
	hHookWinWrite = CreateHookableEvent(ME_MSG_PRECREATEEVENT);

	SkinAddNewSoundEx("RecvMsgActive", LPGEN("Instant messages"), LPGEN("Incoming (focused window)"));
	SkinAddNewSoundEx("RecvMsgInactive", LPGEN("Instant messages"), LPGEN("Incoming (unfocused window)"));
	SkinAddNewSoundEx("AlertMsg", LPGEN("Instant messages"), LPGEN("Incoming (new session)"));
	SkinAddNewSoundEx("SendMsg", LPGEN("Instant messages"), LPGEN("Outgoing"));
	SkinAddNewSoundEx("SendError", LPGEN("Instant messages"), LPGEN("Message send error"));
	SkinAddNewSoundEx("TNStart", LPGEN("Instant messages"), LPGEN("Contact started typing"));
	SkinAddNewSoundEx("TNStop", LPGEN("Instant messages"), LPGEN("Contact stopped typing"));

	hCurSplitNS = LoadCursor(NULL, IDC_SIZENS);
	hCurSplitWE = LoadCursor(NULL, IDC_SIZEWE);
	hCurHyperlinkHand = LoadCursor(NULL, IDC_HAND);
	if (hCurHyperlinkHand == NULL)
		hCurHyperlinkHand = LoadCursor(g_hInst, MAKEINTRESOURCE(IDC_HYPERLINKHAND));

	InitStatusIcons();
	return 0;
}

int SplitmsgShutdown(void)
{
	DestroyCursor(hCurSplitNS);
	DestroyCursor(hCurHyperlinkHand);
	DestroyCursor(hCurSplitWE);

	DestroyHookableEvent(hHookWinEvt);
	DestroyHookableEvent(hHookWinPopup);
	DestroyHookableEvent(hHookWinWrite);

	FreeMsgLogIcons();
	FreeLibrary(GetModuleHandleA("Msftedit"));
	OleUninitialize();
	RichUtil_Unload();
	msgQueue_destroy();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

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
