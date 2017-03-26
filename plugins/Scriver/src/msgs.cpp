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

#include "stdafx.h"

HCURSOR  hDragCursor;
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
		if (hContact != 0)
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

	HWND hwndExisting = WindowList_Find(pci->hWindowList, hContact);
	if (hwndExisting == nullptr)
		(new CSrmmWindow(hContact))->Show();
	else
		SendMessage(GetParent(hwndExisting), CM_POPUPWINDOW, 0, (LPARAM)hwndExisting);
	return 0;
}

static int MessageEventAdded(WPARAM hContact, LPARAM lParam)
{
	MEVENT hDbEvent = (MEVENT)lParam;
	DBEVENTINFO dbei = {};
	db_event_get(hDbEvent, &dbei);
	if (dbei.eventType == EVENTTYPE_MESSAGE && (dbei.flags & DBEF_READ))
		return 0;

	HWND hwnd = WindowList_Find(pci->hWindowList, hContact);
	if (hwnd == nullptr)
		hwnd = WindowList_Find(pci->hWindowList, hContact = db_event_getContact(hDbEvent));
	if (hwnd)
		SendMessage(hwnd, HM_DBEVENTADDED, hContact, lParam);

	if (dbei.flags & DBEF_SENT || !DbEventIsMessageOrCustom(&dbei))
		return 0;

	pcli->pfnRemoveEvent(hContact, 1);
	/* does a window for the contact exist? */
	if (hwnd == nullptr) {
		/* new message */
		SkinPlaySound("AlertMsg");
		if (IsAutoPopup(hContact)) {
			(new CSrmmWindow(hContact, true))->Show();
			return 0;
		}
	}
	if (hwnd == nullptr || !IsWindowVisible(GetParent(hwnd))) {
		wchar_t toolTip[256];
		mir_snwprintf(toolTip, TranslateT("Message from %s"), pcli->pfnGetContactDisplayName(hContact, 0));

		CLISTEVENT cle = {};
		cle.flags = CLEF_UNICODE;
		cle.hContact = hContact;
		cle.hDbEvent = hDbEvent;
		cle.hIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
		cle.pszService = "SRMsg/ReadMessage";
		cle.szTooltip.w = toolTip;
		pcli->pfnAddEvent(&cle);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR SendMessageCommandWorker(MCONTACT hContact, LPCSTR pszMsg, bool isWchar)
{
	hContact = db_mc_tryMeta(hContact);

	/* does the MCONTACT's protocol support IM messages? */
	char *szProto = GetContactProto(hContact);
	if (szProto == nullptr)
		return 1; /* unknown contact */

	if (!CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND)
		return 1;

	HWND hwnd = WindowList_Find(pci->hWindowList, hContact);
	if (hwnd != nullptr) {
		if (pszMsg) {
			HWND hEdit = GetDlgItem(hwnd, IDC_SRMM_MESSAGE);
			SendMessage(hEdit, EM_SETSEL, -1, GetWindowTextLength(hEdit));
			if (isWchar)
				SendMessageW(hEdit, EM_REPLACESEL, FALSE, (LPARAM)pszMsg);
			else
				SendMessageA(hEdit, EM_REPLACESEL, FALSE, (LPARAM)pszMsg);
		}
		SendMessage(GetParent(hwnd), CM_POPUPWINDOW, 0, (LPARAM)hwnd);
	}
	else (new CSrmmWindow(hContact, false, pszMsg, isWchar))->Show();

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

	HWND hwnd = WindowList_Find(pci->hWindowList, hContact);
	if (hwnd)
		SendMessage(hwnd, DM_TYPING, 0, lParam);
	else if (lParam && (g_dat.flags2 & SMF2_SHOWTYPINGTRAY)) {
		wchar_t szTip[256];
		mir_snwprintf(szTip, TranslateT("%s is typing a message"), pcli->pfnGetContactDisplayName(hContact, 0));
		if (g_dat.flags2 & SMF2_SHOWTYPINGCLIST) {
			pcli->pfnRemoveEvent(hContact, 1);

			CLISTEVENT cle = {};
			cle.hContact = hContact;
			cle.hDbEvent = 1;
			cle.flags = CLEF_ONLYAFEW | CLEF_UNICODE;
			cle.hIcon = GetCachedIcon("scriver_TYPING");
			cle.pszService = "SRMsg/TypingMessage";
			cle.szTooltip.w = szTip;
			pcli->pfnAddEvent(&cle);
		}
		else Clist_TrayNotifyW(nullptr, TranslateT("Typing notification"), szTip, NIIF_INFO, 1000 * 4);
	}
	return 0;
}

static int MessageSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
	char *szProto = GetContactProto(hContact); // szProto maybe nullptr
	if (!strcmp(cws->szModule, "CList") || !mir_strcmp(cws->szModule, szProto))
		WindowList_Broadcast(pci->hWindowList, DM_CLISTSETTINGSCHANGED, hContact, lParam);
	return 0;
}

static int ContactDeleted(WPARAM wParam, LPARAM)
{
	HWND hwnd = WindowList_Find(pci->hWindowList, wParam);
	if (hwnd)
		SendMessage(hwnd, WM_CLOSE, 0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct MSavedEvent
{
	MSavedEvent(MCONTACT _hContact, MEVENT _hEvent) :
		hContact(_hContact),
		hEvent(_hEvent)
	{
	}

	MEVENT   hEvent;
	MCONTACT hContact;
};

static void RestoreUnreadMessageAlerts(void)
{
	OBJLIST<MSavedEvent> arEvents(10, NumericKeySortT);

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		for (MEVENT hDbEvent = db_event_firstUnread(hContact); hDbEvent; hDbEvent = db_event_next(hContact, hDbEvent)) {
			DBEVENTINFO dbei = {};
			dbei.cbBlob = 0;
			db_event_get(hDbEvent, &dbei);
			if ((dbei.flags & (DBEF_SENT | DBEF_READ)) || !DbEventIsMessageOrCustom(&dbei))
				continue;

			int windowAlreadyExists = WindowList_Find(pci->hWindowList, hContact) != nullptr;
			if (windowAlreadyExists)
				continue;

			if (IsAutoPopup(hContact) && !windowAlreadyExists)
				(new CSrmmWindow(hContact, true))->Show();
			else
				arEvents.insert(new MSavedEvent(hContact, hDbEvent));
		}
	}

	wchar_t toolTip[256];

	CLISTEVENT cle = {};
	cle.hIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
	cle.pszService = "SRMsg/ReadMessage";
	cle.flags = CLEF_UNICODE;
	cle.szTooltip.w = toolTip;

	for (int i = 0; i < arEvents.getCount(); i++) {
		MSavedEvent &e = arEvents[i];
		mir_snwprintf(toolTip, TranslateT("Message from %s"), pcli->pfnGetContactDisplayName(e.hContact, 0));
		cle.hContact = e.hContact;
		cle.hDbEvent = e.hEvent;
		pcli->pfnAddEvent(&cle);
	}
}

static INT_PTR GetWindowData(WPARAM wParam, LPARAM lParam)
{
	MessageWindowInputData *mwid = (MessageWindowInputData*)wParam;
	if (mwid == nullptr || mwid->cbSize != sizeof(MessageWindowInputData) || mwid->hContact == 0 || mwid->uFlags != MSG_WINDOW_UFLAG_MSG_BOTH)
		return 1;

	MessageWindowData *mwd = (MessageWindowData*)lParam;
	if (mwd == nullptr || mwd->cbSize != sizeof(MessageWindowData))
		return 1;

	HWND hwnd = WindowList_Find(pci->hWindowList, mwid->hContact);
	if (hwnd == nullptr)
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
	if (st != nullptr && st->cbSize != sizeof(StatusTextData))
		return 1;

	HWND hwnd = WindowList_Find(pci->hWindowList, hContact);
	if (hwnd == nullptr) {
		hwnd = SM_FindWindowByContact(hContact);
		if (hwnd == nullptr)
			return 1;
	}

	CScriverWindow *dat = (CScriverWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (dat == nullptr || dat->m_pParent == nullptr)
		return 1;

	ParentWindowData *pdat = dat->m_pParent;

	SendMessage(pdat->hwndStatus, SB_SETICON, 0, (LPARAM)(st == nullptr ? 0 : st->hIcon));
	SendMessage(pdat->hwndStatus, SB_SETTEXT, 0, (LPARAM)(st == nullptr ? L"" : st->tszText));
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
		WindowList_Broadcast(pci->hWindowList, DM_AVATARCHANGED, wParam, lParam);
	else {
		HWND hwnd = WindowList_Find(pci->hWindowList, wParam);
		SendMessage(hwnd, DM_AVATARCHANGED, wParam, lParam);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CScriverWindow

CScriverWindow::CScriverWindow(int iDialog, SESSION_INFO *si)
	: CSrmmBaseDialog(g_hInst, iDialog, si)
{
	m_autoClose = CLOSE_ON_CANCEL;
}

void CScriverWindow::CloseTab()
{
	Close();
}

void CScriverWindow::LoadSettings()
{
	m_clrInputBG = db_get_dw(0, SRMM_MODULE, SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR);
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, nullptr, &m_clrInputFG);
}

/////////////////////////////////////////////////////////////////////////////////////////
// status icons processing

static void RegisterStatusIcons()
{
	StatusIconData sid = { sizeof(sid) };
	sid.szModule = SRMM_MODULE;
	sid.dwId = 1;
	sid.hIcon = GetCachedIcon("scriver_TYPING");
	sid.hIconDisabled = GetCachedIcon("scriver_TYPINGOFF");
	sid.flags = MBF_HIDDEN;
	Srmm_AddIcon(&sid);
}

void ChangeStatusIcons()
{
	StatusIconData sid = { sizeof(sid) };
	sid.szModule = SRMM_MODULE;
	sid.dwId = 1;
	sid.hIcon = GetCachedIcon("scriver_TYPING");
	sid.hIconDisabled = GetCachedIcon("scriver_TYPINGOFF");
	sid.flags = MBF_HIDDEN;
	Srmm_ModifyIcon(0, &sid);
}

int StatusIconPressed(WPARAM wParam, LPARAM lParam)
{
	StatusIconClickData *sicd = (StatusIconClickData *) lParam;
	if (mir_strcmp(SRMM_MODULE, sicd->szModule))
		return 0;

	HWND hwnd = WindowList_Find(pci->hWindowList, wParam);
	if (hwnd == nullptr)
		hwnd = SM_FindWindowByContact(wParam);

	if (hwnd != nullptr)
		SendMessage(hwnd, DM_SWITCHTYPING, 0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// toolbar icons processing

int RegisterToolbarIcons(WPARAM, LPARAM)
{
	BBButton bbd = {};
	bbd.pszModuleName = "SRMM";
	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISIMBUTTON | BBBF_CREATEBYID | BBBF_ISRSIDEBUTTON | BBBF_CANTBEHIDDEN;
	bbd.dwButtonID = IDOK;
	bbd.dwDefPos = 5;
	bbd.hIcon = IcoLib_GetIconHandle("scriver_SEND");
	bbd.pwszText = LPGENW("&OK");
	bbd.pwszTooltip = LPGENW("Send message");
	Srmm_AddButton(&bbd);
	
	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_QUOTE;
	bbd.dwDefPos = 10;
	bbd.hIcon = IcoLib_GetIconHandle("scriver_QUOTE");
	bbd.pwszText = LPGENW("&Quote");
	bbd.pwszTooltip = LPGENW("Quote");
	Srmm_AddButton(&bbd);

	bbd.bbbFlags |= BBBF_ISRSIDEBUTTON;
	bbd.dwButtonID = IDC_ADD;
	bbd.dwDefPos = 20;
	bbd.hIcon = IcoLib_GetIconHandle("scriver_ADD");
	bbd.pwszText = LPGENW("&Add");
	bbd.pwszTooltip = LPGENW("Add contact permanently to list");
	Srmm_AddButton(&bbd);

	bbd.dwButtonID = IDC_USERMENU;
	bbd.dwDefPos = 30;
	bbd.hIcon = Skin_GetIconHandle(SKINICON_OTHER_DOWNARROW);
	bbd.pwszText = LPGENW("&User menu");
	bbd.pwszTooltip = LPGENW("User menu");
	Srmm_AddButton(&bbd);

	bbd.dwButtonID = IDC_DETAILS;
	bbd.dwDefPos = 40;
	bbd.hIcon = IcoLib_GetIconHandle("scriver_USERDETAILS");
	bbd.pwszText = LPGENW("User &details");
	bbd.pwszTooltip = LPGENW("View user's details");
	Srmm_AddButton(&bbd);

	bbd.bbbFlags |= BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON;
	bbd.dwButtonID = IDC_SRMM_HISTORY;
	bbd.dwDefPos = 50;
	bbd.hIcon = IcoLib_GetIconHandle("scriver_HISTORY");
	bbd.pwszText = LPGENW("&History");
	bbd.pwszTooltip = LPGENW("View user's history (CTRL+H)");
	Srmm_AddButton(&bbd);

	// chat buttons
	bbd.bbbFlags = BBBF_ISPUSHBUTTON | BBBF_ISCHATBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_SRMM_BOLD;
	bbd.dwDefPos = 10;
	bbd.hIcon = IcoLib_GetIconHandle("chat_bold");
	bbd.pwszText = LPGENW("&Bold");
	bbd.pwszTooltip = LPGENW("Make the text bold (CTRL+B)");
	Srmm_AddButton(&bbd);

	bbd.dwButtonID = IDC_SRMM_ITALICS;
	bbd.dwDefPos = 15;
	bbd.hIcon = IcoLib_GetIconHandle("chat_italics");
	bbd.pwszText = LPGENW("&Italic");
	bbd.pwszTooltip = LPGENW("Make the text italicized (CTRL+I)");
	Srmm_AddButton(&bbd);

	bbd.dwButtonID = IDC_SRMM_UNDERLINE;
	bbd.dwDefPos = 20;
	bbd.hIcon = IcoLib_GetIconHandle("chat_underline");
	bbd.pwszText = LPGENW("&Underline");
	bbd.pwszTooltip = LPGENW("Make the text underlined (CTRL+U)");
	Srmm_AddButton(&bbd);

	bbd.dwButtonID = IDC_SRMM_COLOR;
	bbd.dwDefPos = 25;
	bbd.hIcon = IcoLib_GetIconHandle("chat_fgcol");
	bbd.pwszText = LPGENW("&Color");
	bbd.pwszTooltip = LPGENW("Select a foreground color for the text (CTRL+K)");
	Srmm_AddButton(&bbd);

	bbd.dwButtonID = IDC_SRMM_BKGCOLOR;
	bbd.dwDefPos = 30;
	bbd.hIcon = IcoLib_GetIconHandle("chat_bkgcol");
	bbd.pwszText = LPGENW("&Background color");
	bbd.pwszTooltip = LPGENW("Select a background color for the text (CTRL+L)");
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_SRMM_CHANMGR;
	bbd.dwDefPos = 30;
	bbd.hIcon = IcoLib_GetIconHandle("chat_settings");
	bbd.pwszText = LPGENW("&Room settings");
	bbd.pwszTooltip = LPGENW("Control this room (CTRL+O)");
	Srmm_AddButton(&bbd);

	bbd.dwButtonID = IDC_SRMM_SHOWNICKLIST;
	bbd.dwDefPos = 20;
	bbd.hIcon = IcoLib_GetIconHandle("chat_nicklist");
	bbd.pwszText = LPGENW("&Show/hide nick list");
	bbd.pwszTooltip = LPGENW("Show/hide the nick list (CTRL+N)");
	Srmm_AddButton(&bbd);

	bbd.dwButtonID = IDC_SRMM_FILTER;
	bbd.dwDefPos = 10;
	bbd.hIcon = IcoLib_GetIconHandle("chat_filter");
	bbd.pwszText = LPGENW("&Filter");
	bbd.pwszTooltip = LPGENW("Enable/disable the event filter (CTRL+F)");
	Srmm_AddButton(&bbd);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

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
		HWND hwnd = WindowList_Find(pci->hWindowList, hMeta);
		if (hwnd != nullptr)
			SendMessage(hwnd, DM_GETAVATAR, 0, 0);
	}
	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	ReloadGlobals();
	LoadGlobalIcons();
	LoadMsgLogIcons();
	ModuleLoad(0, 0);

	CMenuItem mi;
	SET_UID(mi, 0x58d8dc1, 0x1c25, 0x49c0, 0xb8, 0x7c, 0xa3, 0x22, 0x2b, 0x3d, 0xf1, 0xd8);
	mi.position = -2000090000;
	mi.flags = CMIF_DEFAULT;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_EVENT_MESSAGE);
	mi.name.a = LPGEN("&Message");
	mi.pszService = MS_MSG_SENDMESSAGE;
	hMsgMenuItem = Menu_AddContactMenuItem(&mi);
	IcoLib_ReleaseIcon((HICON)mi.hIcolibItem);

	HookEvent(ME_SMILEYADD_OPTIONSCHANGED, SmileySettingsChanged);
	HookEvent(ME_IEVIEW_OPTIONSCHANGED, SmileySettingsChanged);
	HookEvent(ME_AV_AVATARCHANGED, AvatarChanged);
	HookEvent(ME_FONT_RELOAD, FontServiceFontsChanged);
	HookEvent(ME_MSG_ICONPRESSED, StatusIconPressed);
	HookEvent(ME_MSG_TOOLBARLOADED, RegisterToolbarIcons);
	HookEvent(ME_MC_DEFAULTTCHANGED, MetaContactChanged);

	RestoreUnreadMessageAlerts();
	RegisterStatusIcons();
	return 0;
}

int OnSystemPreshutdown(WPARAM, LPARAM)
{
	WindowList_Broadcast(pci->hWindowList, WM_CLOSE, 0, 0);
	WindowList_Broadcast(g_dat.hParentWindowList, WM_CLOSE, 0, 0);
	DeinitStatusIcons();
	return 0;
}

int OnUnloadModule(void)
{
	DestroyCursor(hDragCursor);

	DestroyHookableEvent(hHookWinEvt);
	DestroyHookableEvent(hHookWinPopup);
	DestroyHookableEvent(hHookWinWrite);

	ReleaseIcons();
	FreeMsgLogIcons();
	FreeGlobals();
	FreeLibrary(hMsftEdit);
	return 0;
}

int OnLoadModule(void)
{
	hMsftEdit = LoadLibrary(L"Msftedit.dll");
	if (hMsftEdit == nullptr) {
		if (IDYES != MessageBox(0,
			TranslateT("Miranda could not load the built-in message module, Msftedit.dll is missing. If you are using WINE, please make sure you have Msftedit.dll installed. Press 'Yes' to continue loading Miranda."),
			TranslateT("Information"), MB_YESNO | MB_ICONINFORMATION))
			return 1;
		return 0;
	}

	InitGlobals();
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
	SkinAddNewSoundEx("TNStop", LPGEN("Instant messages"), LPGEN("Contact stopped typing"));

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
	*ppvObj = nullptr;
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CREOleCallback::AddRef()
{
	if (refCount == 0)
		StgCreateDocfile(nullptr, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_DELETEONRELEASE, 0, &pictStg);

	return ++refCount;
}

STDMETHODIMP_(ULONG) CREOleCallback::Release()
{
	if (--refCount == 0) {
		if (pictStg) {
			pictStg->Release();
			pictStg = nullptr;
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
	wchar_t sztName[64];
	mir_snwprintf(sztName, L"s%u", nextStgId++);
	if (pictStg == nullptr)
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

