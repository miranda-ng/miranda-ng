/*

Copyright 2000-12 Miranda IM, 2012-17 Miranda NG project,
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
#include "statusicon.h"

/////////////////////////////////////////////////////////////////////////////////////////

CMsgDialog::CMsgDialog(int iDialogId, SESSION_INFO *si)
	: CSuper(g_hInst, iDialogId, si),
	m_btnOk(this, IDOK)
{
	m_autoClose = 0;
	m_forceResizable = true;
}

/////////////////////////////////////////////////////////////////////////////////////////

int OnCheckPlugins(WPARAM, LPARAM);

HGENMENU hMsgMenuItem;
HMODULE hMsftEdit;

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
	DBEVENTINFO dbei = {};
	db_event_get(lParam, &dbei);

	if (dbei.flags & (DBEF_SENT | DBEF_READ) || !(dbei.eventType == EVENTTYPE_MESSAGE || DbEventIsForMsgWindow(&dbei)))
		return 0;

	pcli->pfnRemoveEvent(hContact, 1);
	/* does a window for the contact exist? */
	HWND hwnd = Srmm_FindWindow(hContact);
	if (hwnd) {
		if (!g_dat.bDoNotStealFocus) {
			ShowWindow(hwnd, SW_RESTORE);
			SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			SetForegroundWindow(hwnd);
			Skin_PlaySound("RecvMsgActive");
		}
		else {
			if (GetForegroundWindow() == GetParent(hwnd))
				Skin_PlaySound("RecvMsgActive");
			else
				Skin_PlaySound("RecvMsgInactive");
		}
		return 0;
	}
	/* new message */
	Skin_PlaySound("AlertMsg");

	char *szProto = GetContactProto(hContact);
	if (szProto && (g_dat.openFlags & SRMMStatusToPf2(CallProtoService(szProto, PS_GETSTATUS, 0, 0)))) {
		GetContainer()->AddPage(hContact);
		return 0;
	}

	wchar_t toolTip[256];
	mir_snwprintf(toolTip, TranslateT("Message from %s"), pcli->pfnGetContactDisplayName(hContact, 0));

	CLISTEVENT cle = {};
	cle.hContact = hContact;
	cle.hDbEvent = lParam;
	cle.flags = CLEF_UNICODE;
	cle.hIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
	cle.pszService = MS_MSG_READMESSAGE;
	cle.szTooltip.w = toolTip;
	pcli->pfnAddEvent(&cle);
	return 0;
}

INT_PTR SendMessageCmd(MCONTACT hContact, wchar_t *pwszInitialText)
{
	/* does the MCONTACT's protocol support IM messages? */
	char *szProto = GetContactProto(hContact);
	if (!szProto || (!CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND))
		return 1;

	hContact = db_mc_tryMeta(hContact);

	HWND hwnd = Srmm_FindWindow(hContact);
	if (hwnd) {
		if (pwszInitialText) {
			SendDlgItemMessage(hwnd, IDC_SRMM_MESSAGE, EM_SETSEL, -1, SendDlgItemMessage(hwnd, IDC_SRMM_MESSAGE, WM_GETTEXTLENGTH, 0, 0));
			SendDlgItemMessageW(hwnd, IDC_SRMM_MESSAGE, EM_REPLACESEL, FALSE, (LPARAM)pwszInitialText);
			mir_free(pwszInitialText);
		}
		
		if (!g_Settings.bTabsEnable) {
			HWND hwndContainer = GetParent(hwnd);
			ShowWindow(hwndContainer, SW_RESTORE);
			SetWindowPos(hwndContainer, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			SetForegroundWindow(hwndContainer);
		}
		else {
			CSrmmBaseDialog *pDlg = (CSrmmBaseDialog*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			pDialog->m_tab.ActivatePage(pDialog->m_tab.GetDlgIndex(pDlg));
		}
	}
	else GetContainer()->AddPage(hContact, pwszInitialText, false);

	return 0;
}

static INT_PTR SendMessageCommand_W(WPARAM wParam, LPARAM lParam)
{
	return SendMessageCmd(wParam, mir_wstrdup((wchar_t*)lParam));
}

static INT_PTR SendMessageCommand(WPARAM wParam, LPARAM lParam)
{
	return SendMessageCmd(wParam, mir_a2u((char*)lParam));
}

static INT_PTR ReadMessageCommand(WPARAM, LPARAM lParam)
{
	CLISTEVENT *cle = (CLISTEVENT*)lParam;
	if (cle)
		SendMessageCmd(cle->hContact, nullptr);

	return 0;
}

static int TypingMessage(WPARAM hContact, LPARAM lParam)
{
	if (!g_dat.bShowTyping)
		return 0;

	hContact = db_mc_tryMeta(hContact);

	Skin_PlaySound((lParam) ? "TNStart" : "TNStop");

	HWND hwnd = Srmm_FindWindow(hContact);
	if (hwnd)
		SendMessage(hwnd, DM_TYPING, 0, lParam);
	else if (lParam && g_dat.bShowTypingTray) {
		wchar_t szTip[256];
		mir_snwprintf(szTip, TranslateT("%s is typing a message"), pcli->pfnGetContactDisplayName(hContact, 0));

		if (g_dat.bShowTypingClist) {
			pcli->pfnRemoveEvent(hContact, 1);

			CLISTEVENT cle = {};
			cle.hContact = hContact;
			cle.hDbEvent = 1;
			cle.flags = CLEF_ONLYAFEW | CLEF_UNICODE;
			cle.hIcon = Skin_LoadIcon(SKINICON_OTHER_TYPING);
			cle.pszService = MS_MSG_READMESSAGE;
			cle.szTooltip.w = szTip;
			pcli->pfnAddEvent(&cle);

			IcoLib_ReleaseIcon(cle.hIcon);
		}
		else Clist_TrayNotifyW(nullptr, TranslateT("Typing notification"), szTip, NIIF_INFO, 1000 * 4);
	}
	return 0;
}

static int MessageSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (cws->szModule == nullptr)
		return 0;

	if (!strcmp(cws->szModule, "CList"))
		Srmm_Broadcast(DM_UPDATETITLE, (WPARAM)cws, 0);
	else if (hContact) {
		if (cws->szSetting && !strcmp(cws->szSetting, "Timezone"))
			Srmm_Broadcast(DM_NEWTIMEZONE, (WPARAM)cws, 0);
		else {
			char *szProto = GetContactProto(hContact);
			if (szProto && !strcmp(cws->szModule, szProto))
				Srmm_Broadcast(DM_UPDATETITLE, (WPARAM)cws, 0);
		}
	}
	return 0;
}

// If a contact gets deleted, close its message window if there is any
static int ContactDeleted(WPARAM wParam, LPARAM)
{
	HWND hwnd = Srmm_FindWindow(wParam);
	if (hwnd)
		SendMessage(hwnd, DM_CLOSETAB, 0, 0);

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
			bool autoPopup = false;

			DBEVENTINFO dbei = {};
			dbei.cbBlob = 0;
			db_event_get(hDbEvent, &dbei);
			if (!(dbei.flags & (DBEF_SENT | DBEF_READ)) && (dbei.eventType == EVENTTYPE_MESSAGE || DbEventIsForMsgWindow(&dbei))) {
				int windowAlreadyExists = Srmm_FindWindow(hContact) != nullptr;
				if (windowAlreadyExists)
					continue;

				char *szProto = GetContactProto(hContact);
				if (szProto && (g_dat.openFlags & SRMMStatusToPf2(CallProtoService(szProto, PS_GETSTATUS, 0, 0))))
					autoPopup = true;

				if (autoPopup && !windowAlreadyExists)
					GetContainer()->AddPage(hContact);
				else
					arEvents.insert(new MSavedEvent(hContact, hDbEvent));
			}
		}
	}

	wchar_t toolTip[256];

	CLISTEVENT cle = {};
	cle.hIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
	cle.pszService = MS_MSG_READMESSAGE;
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

void RegisterSRMMFonts(void);

/////////////////////////////////////////////////////////////////////////////////////////
// toolbar buttons support

int RegisterToolbarIcons(WPARAM, LPARAM)
{
	BBButton bbd = {};
	bbd.pszModuleName = "SRMM";
	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_CREATEBYID;

	bbd.dwButtonID = IDC_ADD;
	bbd.dwDefPos = 10;
	bbd.hIcon = Skin_GetIconHandle(SKINICON_OTHER_ADDCONTACT);
	bbd.pwszText = LPGENW("&Add");
	bbd.pwszTooltip = LPGENW("Add contact permanently to list");
	Srmm_AddButton(&bbd);

	bbd.dwButtonID = IDC_USERMENU;
	bbd.dwDefPos = 20;
	bbd.hIcon = Skin_GetIconHandle(SKINICON_OTHER_DOWNARROW);
	bbd.pwszText = LPGENW("&User menu");
	bbd.pwszTooltip = LPGENW("User menu");
	Srmm_AddButton(&bbd);

	bbd.dwButtonID = IDC_DETAILS;
	bbd.dwDefPos = 30;
	bbd.hIcon = Skin_GetIconHandle(SKINICON_OTHER_USERDETAILS);
	bbd.pwszText = LPGENW("User &details");
	bbd.pwszTooltip = LPGENW("View user's details");
	Srmm_AddButton(&bbd);

	bbd.bbbFlags |= BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON;
	bbd.dwButtonID = IDC_SRMM_HISTORY;
	bbd.dwDefPos = 40;
	bbd.hIcon = Skin_GetIconHandle(SKINICON_OTHER_HISTORY);
	bbd.pwszText = LPGENW("&History");
	bbd.pwszTooltip = LPGENW("View user's history (CTRL+H)");
	Srmm_AddButton(&bbd);

	// chat buttons
	bbd.bbbFlags = BBBF_ISPUSHBUTTON | BBBF_ISCHATBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_SRMM_BOLD;
	bbd.dwDefPos = 10;
	bbd.hIcon = GetIconHandle("bold");
	bbd.pwszText = LPGENW("&Bold");
	bbd.pwszTooltip = LPGENW("Make the text bold (CTRL+B)");
	Srmm_AddButton(&bbd);

	bbd.dwButtonID = IDC_SRMM_ITALICS;
	bbd.dwDefPos = 15;
	bbd.hIcon = GetIconHandle("italics");
	bbd.pwszText = LPGENW("&Italic");
	bbd.pwszTooltip = LPGENW("Make the text italicized (CTRL+I)");
	Srmm_AddButton(&bbd);

	bbd.dwButtonID = IDC_SRMM_UNDERLINE;
	bbd.dwDefPos = 20;
	bbd.hIcon = GetIconHandle("underline");
	bbd.pwszText = LPGENW("&Underline");
	bbd.pwszTooltip = LPGENW("Make the text underlined (CTRL+U)");
	Srmm_AddButton(&bbd);

	bbd.dwButtonID = IDC_SRMM_COLOR;
	bbd.dwDefPos = 25;
	bbd.hIcon = GetIconHandle("fgcol");
	bbd.pwszText = LPGENW("&Color");
	bbd.pwszTooltip = LPGENW("Select a foreground color for the text (CTRL+K)");
	Srmm_AddButton(&bbd);

	bbd.dwButtonID = IDC_SRMM_BKGCOLOR;
	bbd.dwDefPos = 30;
	bbd.hIcon = GetIconHandle("bkgcol");
	bbd.pwszText = LPGENW("&Background color");
	bbd.pwszTooltip = LPGENW("Select a background color for the text (CTRL+L)");
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_SRMM_CHANMGR;
	bbd.dwDefPos = 30;
	bbd.hIcon = GetIconHandle("settings");
	bbd.pwszText = LPGENW("&Room settings");
	bbd.pwszTooltip = LPGENW("Control this room (CTRL+O)");
	Srmm_AddButton(&bbd);

	bbd.dwButtonID = IDC_SRMM_SHOWNICKLIST;
	bbd.dwDefPos = 20;
	bbd.hIcon = GetIconHandle("nicklist");
	bbd.pwszText = LPGENW("&Show/hide nick list");
	bbd.pwszTooltip = LPGENW("Show/hide the nick list (CTRL+N)");
	Srmm_AddButton(&bbd);

	bbd.dwButtonID = IDC_SRMM_FILTER;
	bbd.dwDefPos = 10;
	bbd.hIcon = GetIconHandle("filter");
	bbd.pwszText = LPGENW("&Filter");
	bbd.pwszTooltip = LPGENW("Enable/disable the event filter (CTRL+F)");
	Srmm_AddButton(&bbd);
	return 0;
}

void SetButtonsPos(HWND hwndDlg, bool bIsChat)
{
	HDWP hdwp = BeginDeferWindowPos(Srmm_GetButtonCount());

	int yPos;
	RECT rc;
	if (bIsChat) {
		GetWindowRect(GetDlgItem(hwndDlg, IDC_SPLITTERY), &rc);
		POINT pt = { 0, rc.top };
		ScreenToClient(hwndDlg, &pt);
		yPos = pt.y;
	}
	else yPos = 2;

	GetClientRect(hwndDlg, &rc);
	int iLeftX = 2, iRightX = rc.right - 2;

	CustomButtonData *cbd;
	for (int i = 0; cbd = Srmm_GetNthButton(i); i++) {
		HWND hwndButton = GetDlgItem(hwndDlg, cbd->m_dwButtonCID);
		if (hwndButton == nullptr || cbd->m_bHidden)
			continue;

		if (cbd->m_bRSided) {
			iRightX -= g_dat.iGap + cbd->m_iButtonWidth;
			hdwp = DeferWindowPos(hdwp, hwndButton, nullptr, iRightX, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}
		else {
			hdwp = DeferWindowPos(hdwp, hwndButton, nullptr, iLeftX, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			iLeftX += g_dat.iGap + cbd->m_iButtonWidth;
		}
	}

	EndDeferWindowPos(hdwp);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int FontsChanged(WPARAM, LPARAM)
{
	Srmm_Broadcast(DM_OPTIONSAPPLIED, TRUE, 0);
	return 0;
}

static int SplitmsgModulesLoaded(WPARAM, LPARAM)
{
	RegisterSRMMFonts();
	LoadMsgLogIcons();
	OnCheckPlugins(0, 0);

	CMenuItem mi;
	SET_UID(mi, 0x58d8dc1, 0x1c25, 0x49c0, 0xb8, 0x7c, 0xa3, 0x22, 0x2b, 0x3d, 0xf1, 0xd8);
	mi.position = -2000090000;
	mi.flags = CMIF_DEFAULT;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_EVENT_MESSAGE);
	mi.name.a = LPGEN("&Message");
	mi.pszService = MS_MSG_SENDMESSAGE;
	hMsgMenuItem = Menu_AddContactMenuItem(&mi);

	HookEvent(ME_FONT_RELOAD, FontsChanged);
	
	HookTemporaryEvent(ME_MSG_TOOLBARLOADED, RegisterToolbarIcons);

	RestoreUnreadMessageAlerts();
	return 0;
}

int PreshutdownSendRecv(WPARAM, LPARAM)
{
	Srmm_Broadcast(DM_CLOSETAB, 0, 0);

	DeinitStatusIcons();
	return 0;
}

static int IconsChanged(WPARAM, LPARAM)
{
	FreeMsgLogIcons();
	LoadMsgLogIcons();

	// change all the icons
	Srmm_Broadcast(DM_REMAKELOG, 0, 0);
	Srmm_Broadcast(DM_UPDATEWINICON, 0, 0);
	return 0;
}

static int PrebuildContactMenu(WPARAM hContact, LPARAM)
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

static wchar_t tszError[] = LPGENW("Miranda could not load the built-in message module, msftedit.dll is missing. Press 'Yes' to continue loading Miranda.");

int LoadSendRecvMessageModule(void)
{
	if ((hMsftEdit = LoadLibrary(L"Msftedit.dll")) == nullptr) {
		if (IDYES != MessageBox(0, TranslateW(tszError), TranslateT("Information"), MB_YESNO | MB_ICONINFORMATION))
			return 1;
		return 0;
	}

	InitGlobals();
	InitOptions();

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
	CreateServiceFunction(MS_MSG_READMESSAGE, ReadMessageCommand);

	Skin_AddSound("RecvMsgActive",   LPGENW("Instant messages"), LPGENW("Incoming (focused window)"));
	Skin_AddSound("RecvMsgInactive", LPGENW("Instant messages"), LPGENW("Incoming (unfocused window)"));
	Skin_AddSound("AlertMsg",        LPGENW("Instant messages"), LPGENW("Incoming (new session)"));
	Skin_AddSound("SendMsg",         LPGENW("Instant messages"), LPGENW("Outgoing"));
	Skin_AddSound("SendError",       LPGENW("Instant messages"), LPGENW("Message send error"));
	Skin_AddSound("TNStart",         LPGENW("Instant messages"), LPGENW("Contact started typing"));
	Skin_AddSound("TNStop",          LPGENW("Instant messages"), LPGENW("Contact stopped typing"));

	InitStatusIcons();
	return 0;
}

void SplitmsgShutdown(void)
{
	FreeMsgLogIcons();
	FreeLibrary(hMsftEdit);
	msgQueue_destroy();
}
