/*

Copyright 2000-12 Miranda IM, 2012-22 Miranda NG team,
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

static HGENMENU hMsgMenuItem;
static HMODULE hMsftEdit;

int OnCheckPlugins(WPARAM, LPARAM);

/////////////////////////////////////////////////////////////////////////////////////////

int SendMessageDirect(const wchar_t *szMsg, MCONTACT hContact)
{
	if (hContact == 0)
		return 0;

	int flags = 0;
	if (Utils_IsRtl(szMsg))
		flags |= PREF_RTL;

	T2Utf sendBuffer(szMsg);
	if (!mir_strlen(sendBuffer))
		return 0;

	if (db_mc_isMeta(hContact))
		hContact = db_mc_getSrmmSub(hContact);

	int sendId = ProtoChainSend(hContact, PSS_MESSAGE, flags, (LPARAM)sendBuffer);
	msgQueue_add(hContact, sendId, sendBuffer.detach(), flags);
	return sendId;
}

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

	g_clistApi.pfnRemoveEvent(hContact, 1);
	/* does a window for the contact exist? */
	HWND hwnd = Srmm_FindWindow(hContact);
	if (hwnd) {
		if (!g_plugin.bDoNotStealFocus) {
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

	if (!g_plugin.bDoNotStealFocus) {
		char *szProto = Proto_GetBaseAccountName(hContact);
		if (szProto && (g_plugin.popupFlags & SRMMStatusToPf2(Proto_GetStatus(szProto)))) {
			GetContainer()->AddPage(hContact);
			return 0;
		}
	}

	wchar_t toolTip[256];
	mir_snwprintf(toolTip, TranslateT("Message from %s"), Clist_GetContactDisplayName(hContact));

	CLISTEVENT cle = {};
	cle.hContact = hContact;
	cle.hDbEvent = lParam;
	cle.flags = CLEF_UNICODE;
	cle.hIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
	cle.pszService = MS_MSG_READMESSAGE;
	cle.szTooltip.w = toolTip;
	g_clistApi.pfnAddEvent(&cle);
	return 0;
}

INT_PTR SendMessageCmd(MCONTACT hContact, wchar_t *pwszInitialText)
{
	/* does the MCONTACT's protocol support IM messages? */
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (!szProto || !(CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND))
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
			g_pTabDialog->m_tab.ActivatePage(g_pTabDialog->m_tab.GetDlgIndex(pDlg));
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
	if (!g_plugin.bShowTyping)
		return 0;

	hContact = db_mc_tryMeta(hContact);

	Skin_PlaySound((lParam) ? "TNStart" : "TNStop");

	auto *pDlg = Srmm_FindDialog(hContact);
	if (pDlg)
		pDlg->UserTyping(lParam);
	else if (lParam && g_plugin.bShowTypingTray) {
		wchar_t szTip[256];
		mir_snwprintf(szTip, TranslateT("%s is typing a message"), Clist_GetContactDisplayName(hContact));

		if (g_plugin.bShowTypingClist) {
			g_clistApi.pfnRemoveEvent(hContact, 1);

			CLISTEVENT cle = {};
			cle.hContact = hContact;
			cle.hDbEvent = 1;
			cle.flags = CLEF_ONLYAFEW | CLEF_UNICODE;
			cle.hIcon = Skin_LoadIcon(SKINICON_OTHER_TYPING);
			cle.pszService = MS_MSG_READMESSAGE;
			cle.szTooltip.w = szTip;
			g_clistApi.pfnAddEvent(&cle);

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
		Srmm_Broadcast(DM_UPDATETITLE, (WPARAM)cws, hContact);
	else if (hContact) {
		if (cws->szSetting && !strcmp(cws->szSetting, "Timezone"))
			Srmm_Broadcast(DM_NEWTIMEZONE, (WPARAM)cws, 0);
		else {
			char *szProto = Proto_GetBaseAccountName(hContact);
			if (szProto && !strcmp(cws->szModule, szProto))
				Srmm_Broadcast(DM_UPDATETITLE, (WPARAM)cws, hContact);
		}
	}
	return 0;
}

// If a contact gets deleted, close its message window if there is any
static int ContactDeleted(WPARAM wParam, LPARAM)
{
	auto *pDlg = Srmm_FindDialog(wParam);
	if (pDlg)
		pDlg->CloseTab();

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

	for (auto &hContact : Contacts()) {
		for (MEVENT hDbEvent = db_event_firstUnread(hContact); hDbEvent; hDbEvent = db_event_next(hContact, hDbEvent)) {
			bool autoPopup = false;

			DBEVENTINFO dbei = {};
			dbei.cbBlob = 0;
			if (db_event_get(hDbEvent, &dbei))
				continue;

			if (!dbei.markedRead() && (dbei.eventType == EVENTTYPE_MESSAGE || DbEventIsForMsgWindow(&dbei))) {
				int windowAlreadyExists = Srmm_FindWindow(hContact) != nullptr;
				if (windowAlreadyExists)
					continue;

				char *szProto = Proto_GetBaseAccountName(hContact);
				if (szProto == nullptr)
					continue;

				if (g_plugin.popupFlags & SRMMStatusToPf2(Proto_GetStatus(szProto)))
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

	for (auto &e : arEvents) {
		mir_snwprintf(toolTip, TranslateT("Message from %s"), Clist_GetContactDisplayName(e->hContact));
		cle.hContact = e->hContact;
		cle.hDbEvent = e->hEvent;
		g_clistApi.pfnAddEvent(&cle);
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
	Srmm_AddButton(&bbd, &g_plugin);

	bbd.dwButtonID = IDC_USERMENU;
	bbd.dwDefPos = 20;
	bbd.hIcon = Skin_GetIconHandle(SKINICON_OTHER_DOWNARROW);
	bbd.pwszText = LPGENW("&User menu");
	bbd.pwszTooltip = LPGENW("User menu");
	Srmm_AddButton(&bbd, &g_plugin);

	bbd.dwButtonID = IDC_DETAILS;
	bbd.dwDefPos = 30;
	bbd.hIcon = Skin_GetIconHandle(SKINICON_OTHER_USERDETAILS);
	bbd.pwszText = LPGENW("User &details");
	bbd.pwszTooltip = LPGENW("View user's details");
	Srmm_AddButton(&bbd, &g_plugin);

	bbd.bbbFlags |= BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON;
	bbd.dwButtonID = IDC_SRMM_HISTORY;
	bbd.dwDefPos = 40;
	bbd.hIcon = Skin_GetIconHandle(SKINICON_OTHER_HISTORY);
	bbd.pwszText = LPGENW("&History");
	bbd.pwszTooltip = LPGENW("View user's history (Ctrl+H)");
	Srmm_AddButton(&bbd, &g_plugin);

	// chat buttons
	bbd.bbbFlags = BBBF_ISPUSHBUTTON | BBBF_ISCHATBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_SRMM_BOLD;
	bbd.dwDefPos = 10;
	bbd.hIcon = g_plugin.getIconHandle(IDI_BBOLD);
	bbd.pwszText = LPGENW("&Bold");
	bbd.pwszTooltip = LPGENW("Make the text bold (Ctrl+B)");
	Srmm_AddButton(&bbd, &g_plugin);

	bbd.dwButtonID = IDC_SRMM_ITALICS;
	bbd.dwDefPos = 15;
	bbd.hIcon = g_plugin.getIconHandle(IDI_BITALICS);
	bbd.pwszText = LPGENW("&Italic");
	bbd.pwszTooltip = LPGENW("Make the text italicized (Ctrl+I)");
	Srmm_AddButton(&bbd, &g_plugin);

	bbd.dwButtonID = IDC_SRMM_UNDERLINE;
	bbd.dwDefPos = 20;
	bbd.hIcon = g_plugin.getIconHandle(IDI_BUNDERLINE);
	bbd.pwszText = LPGENW("&Underline");
	bbd.pwszTooltip = LPGENW("Make the text underlined (Ctrl+U)");
	Srmm_AddButton(&bbd, &g_plugin);

	bbd.dwButtonID = IDC_SRMM_COLOR;
	bbd.dwDefPos = 25;
	bbd.hIcon = g_plugin.getIconHandle(IDI_COLOR);
	bbd.pwszText = LPGENW("&Color");
	bbd.pwszTooltip = LPGENW("Select a foreground color for the text (Ctrl+K)");
	Srmm_AddButton(&bbd, &g_plugin);

	bbd.dwButtonID = IDC_SRMM_BKGCOLOR;
	bbd.dwDefPos = 30;
	bbd.hIcon = g_plugin.getIconHandle(IDI_BKGCOLOR);
	bbd.pwszText = LPGENW("&Background color");
	bbd.pwszTooltip = LPGENW("Select a background color for the text (Ctrl+L)");
	Srmm_AddButton(&bbd, &g_plugin);

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_SRMM_CHANMGR;
	bbd.dwDefPos = 30;
	bbd.hIcon = g_plugin.getIconHandle(IDI_TOPICBUT);
	bbd.pwszText = LPGENW("&Room settings");
	bbd.pwszTooltip = LPGENW("Control this room (Ctrl+O)");
	Srmm_AddButton(&bbd, &g_plugin);

	bbd.dwButtonID = IDC_SRMM_SHOWNICKLIST;
	bbd.dwDefPos = 20;
	bbd.hIcon = g_plugin.getIconHandle(IDI_NICKLIST);
	bbd.pwszText = LPGENW("&Show/hide nick list");
	bbd.pwszTooltip = LPGENW("Show/hide the nick list (Ctrl+N)");
	Srmm_AddButton(&bbd, &g_plugin);

	bbd.dwButtonID = IDC_SRMM_FILTER;
	bbd.dwDefPos = 10;
	bbd.hIcon = g_plugin.getIconHandle(IDI_FILTER);
	bbd.pwszText = LPGENW("&Filter");
	bbd.pwszTooltip = LPGENW("Enable/disable the event filter (Ctrl+F)");
	Srmm_AddButton(&bbd, &g_plugin);
	return 0;
}

void CMsgDialog::SetButtonsPos()
{
	HDWP hdwp = BeginDeferWindowPos(Srmm_GetButtonCount());

	int yPos;
	RECT rc;
	GetWindowRect(GetDlgItem(m_hwnd, IDC_SPLITTERY), &rc);
	POINT pt = { 0, rc.top };
	ScreenToClient(m_hwnd, &pt);
	yPos = pt.y;

	GetClientRect(m_hwnd, &rc);
	int iLeftX = 2, iRightX = rc.right - 2, iGap = Srmm_GetButtonGap();

	CustomButtonData *cbd;
	for (int i = 0; cbd = Srmm_GetNthButton(i); i++) {
		HWND hwndButton = GetDlgItem(m_hwnd, cbd->m_dwButtonCID);
		if (hwndButton == nullptr || cbd->m_bHidden)
			continue;

		if (cbd->m_bRSided) {
			iRightX -= iGap + cbd->m_iButtonWidth;
			hdwp = DeferWindowPos(hdwp, hwndButton, nullptr, iRightX, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}
		else {
			hdwp = DeferWindowPos(hdwp, hwndButton, nullptr, iLeftX, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			iLeftX += iGap + cbd->m_iButtonWidth;
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

	CMenuItem mi(&g_plugin);
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

static int Preshutdown(WPARAM, LPARAM)
{
	for (auto &it : g_arDialogs.rev_iter())
		it->CloseTab();
	return 0;
}

static int IconsChanged(WPARAM, LPARAM)
{
	FreeMsgLogIcons();
	LoadMsgLogIcons();

	// change all the icons
	for (auto &it : g_arDialogs) {
		it->RemakeLog();
		it->FixTabIcons();
	}
	return 0;
}

static int PrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (hContact) {
		bool bEnabled = false;
		char *szProto = Proto_GetBaseAccountName(hContact);
		if (szProto) {
			// leave this menu item hidden for chats
			if (!Contact::IsGroupChat(hContact, szProto))
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
		if (IDYES != MessageBox(nullptr, TranslateW(tszError), TranslateT("Information"), MB_YESNO | MB_ICONINFORMATION))
			return 1;
		return 0;
	}

	InitGlobals();

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);
	HookEvent(ME_DB_EVENT_ADDED, MessageEventAdded);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, MessageSettingChanged);
	HookEvent(ME_DB_CONTACT_DELETED, ContactDeleted);
	HookEvent(ME_OPT_INITIALISE, OptInitialise);
	HookEvent(ME_PROTO_CONTACTISTYPING, TypingMessage);
	HookEvent(ME_SKIN_ICONSCHANGED, IconsChanged);
	HookEvent(ME_SYSTEM_MODULESLOADED, SplitmsgModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, Preshutdown);

	CreateServiceFunction(MS_MSG_SENDMESSAGE, SendMessageCommand);
	CreateServiceFunction(MS_MSG_SENDMESSAGEW, SendMessageCommand_W);
	CreateServiceFunction(MS_MSG_READMESSAGE, ReadMessageCommand);

	g_plugin.addSound("RecvMsgActive",   LPGENW("Instant messages"), LPGENW("Incoming (focused window)"));
	g_plugin.addSound("RecvMsgInactive", LPGENW("Instant messages"), LPGENW("Incoming (unfocused window)"));
	g_plugin.addSound("AlertMsg",        LPGENW("Instant messages"), LPGENW("Incoming (new session)"));
	g_plugin.addSound("SendMsg",         LPGENW("Instant messages"), LPGENW("Outgoing"));
	g_plugin.addSound("SendError",       LPGENW("Instant messages"), LPGENW("Message send error"));
	g_plugin.addSound("TNStart",         LPGENW("Instant messages"), LPGENW("Contact started typing"));
	g_plugin.addSound("TNStop",          LPGENW("Instant messages"), LPGENW("Contact stopped typing"));

	InitStatusIcons();
	return 0;
}

void SplitmsgShutdown(void)
{
	FreeMsgLogIcons();
	FreeLibrary(hMsftEdit);
	msgQueue_destroy();
}
