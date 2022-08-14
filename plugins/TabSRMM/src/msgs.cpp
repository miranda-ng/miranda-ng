/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// Load, setup and shutdown the plugin
// core plugin messaging services (single IM chats only).

#include "stdafx.h"

#define IDI_CORE_LOAD	132					// icon id for the "connecting" icon

static HANDLE hUserPrefsWindowLis = nullptr;
HMODULE g_hMsftedit;

static void UnloadIcons();

int OptInitialise(WPARAM wParam, LPARAM lParam);
void Chat_AddIcons(HINSTANCE);

/////////////////////////////////////////////////////////////////////////////////////////
// fired event when user changes IEView plugin options. Apply them to all open tabs

int IEViewOptionsChanged(WPARAM, LPARAM)
{
	Srmm_Broadcast(DM_IEVIEWOPTIONSCHANGED, 0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// fired event when user changes smileyadd options. Notify all open tabs about the changes

int SmileyAddOptionsChanged(WPARAM, LPARAM)
{
	Srmm_Broadcast(DM_SMILEYOPTIONSCHANGED, 0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// service function finds a message session
// wParam = contact handle for which we want the window handle
// thanks to bio for the suggestion of this service
// if wParam == 0, then lParam is considered to be a valid window handle and
// the function tests the popup mode of the target container
//
// returns the hwnd if there is an open window or tab for the given hcontact (wParam),
// or (if lParam was specified) the hwnd if the window exists.
// 0 if there is none (or the popup mode of the target container was configured to "hide"
// the window..

int TSAPI MessageWindowOpened(MCONTACT hContact, CMsgDialog *pDlg)
{
	if (hContact)
		pDlg = Srmm_FindDialog(hContact);
	else if (!pDlg)
		return 0;

	TContainerData *pContainer = pDlg->m_pContainer;
	if (pContainer) {
		if (pContainer->cfg.flags.m_bDontReport) {
			if (IsIconic(pContainer->m_hwnd))
				return 0;
		}
		if (pContainer->cfg.flags.m_bDontReportUnfocused) {
			if (!IsIconic(pContainer->m_hwnd) && !pContainer->IsActive())
				return 0;
		}
		if (pContainer->cfg.flags.m_bAlwaysReportInactive) {
			if (pContainer->cfg.flags.m_bDontReportFocused)
				return 0;

			return pContainer->m_hwndActive == pDlg->GetHwnd();
		}
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// ReadMessageCommand is executed whenever the user wants to manually open a window.
// This can happen when double clicking a contact on the clist OR when opening a new
// message (clicking on a popup, clicking the flashing tray icon and so on).

static INT_PTR ReadMessageCommand(WPARAM, LPARAM lParam)
{
	MCONTACT hContact = ((CLISTEVENT *)lParam)->hContact;

	auto *pDlg = Srmm_FindDialog(hContact);
	if (pDlg != nullptr)
		pDlg->ActivateTab();
	else {
		wchar_t szName[CONTAINER_NAMELEN + 1];
		GetContainerNameForContact(hContact, szName, CONTAINER_NAMELEN);
		TContainerData *pContainer = FindContainerByName(szName);
		if (pContainer == nullptr)
			pContainer = CreateContainer(szName, FALSE, hContact);
		if (pContainer)
			CreateNewTabForContact(pContainer, hContact, true, true, false, 0);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// the SendMessageCommand() invokes a message session window for the given contact.
// e.g. it is called when user double clicks a contact on the contact list
// it is implemented as a service, so external plugins can use it to open a message window.
// contacts handle must be passed in wParam.

INT_PTR SendMessageCommand_Worker(MCONTACT hContact, LPCSTR pszMsg, bool isWchar)
{
	// make sure that only the main UI thread will handle window creation
	if (GetCurrentThreadId() != PluginConfig.dwThreadID) {
		if (pszMsg) {
			wchar_t *tszText = (isWchar) ? mir_wstrdup((wchar_t*)pszMsg) : mir_a2u(pszMsg);
			PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_SENDMESSAGECOMMANDW, hContact, (LPARAM)tszText);
		}
		else PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_SENDMESSAGECOMMANDW, hContact, 0);
		return 0;
	}

	// does the MCONTACT's protocol support IM messages?
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto == nullptr)
		return 0; // unknown contact
	if (0 == (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND))
		return 0;

	auto *pDlg = Srmm_FindDialog(hContact);
	if (pDlg) {
		if (pszMsg) {
			HWND hEdit = GetDlgItem(pDlg->GetHwnd(), IDC_SRMM_MESSAGE);
			SendMessage(hEdit, EM_SETSEL, -1, GetWindowTextLength(hEdit));
			if (isWchar)
				SendMessageW(hEdit, EM_REPLACESEL, FALSE, (LPARAM)pszMsg);
			else
				SendMessageA(hEdit, EM_REPLACESEL, FALSE, (LPARAM)pszMsg);
		}
		pDlg->ActivateTab();
	}
	else {
		wchar_t szName[CONTAINER_NAMELEN + 1];
		GetContainerNameForContact(hContact, szName, CONTAINER_NAMELEN);

		TContainerData *pContainer = FindContainerByName(szName);
		if (pContainer == nullptr)
			pContainer = CreateContainer(szName, FALSE, hContact);
		if (pContainer)
			CreateNewTabForContact(pContainer, hContact, true, true, false, 0, isWchar, pszMsg);
	}
	return 0;
}

INT_PTR SendMessageCommand(WPARAM hContact, LPARAM lParam)
{
	return SendMessageCommand_Worker(hContact, LPCSTR(lParam), false);
}

INT_PTR SendMessageCommand_W(WPARAM hContact, LPARAM lParam)
{
	return SendMessageCommand_Worker(hContact, LPCSTR(lParam), true);
}

/////////////////////////////////////////////////////////////////////////////////////////
// open a window when user clicks on the flashing "typing message" tray icon.
// just calls SendMessageCommand() for the given contact.

static INT_PTR TypingMessageCommand(WPARAM, LPARAM lParam)
{
	CLISTEVENT *cle = (CLISTEVENT*)lParam;
	if (cle)
		SendMessageCommand((WPARAM)cle->hContact, 0);
	return 0;
}

int SplitmsgShutdown(void)
{
	WindowList_Destroy(PluginConfig.hUserPrefsWindowList);

	DestroyCursor(PluginConfig.hCurSplitNS);
	DestroyCursor(PluginConfig.hCurSplitWE);

	FreeLibrary(g_hMsftedit);

	ImageList_RemoveAll(PluginConfig.g_hImageList);
	ImageList_Destroy(PluginConfig.g_hImageList);

	delete Win7Taskbar;

	DestroyMenu(PluginConfig.g_hMenuContext);
	if (PluginConfig.g_hMenuContainer)
		DestroyMenu(PluginConfig.g_hMenuContainer);

	UnloadIcons();
	FreeTabConfig();

	UnloadTSButtonModule();
	return 0;
}

int AvatarChanged(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0)
		return 0;

	for (TContainerData *p = pFirstContainer; p; p = p->pNext)
		p->BroadCastContainer(DM_UPDATEPICLAYOUT, wParam, lParam);
	return 0;
}

int MyAvatarChanged(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0 || IsBadReadPtr((void*)wParam, 4))
		return 0;

	for (TContainerData *p = pFirstContainer; p; p = p->pNext)
		p->BroadCastContainer(DM_MYAVATARCHANGED, wParam, lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// this function creates and activates a new tab within the given container.
// bActivateTab: make the new tab the active one
// bPopupContainer: restore container if it was minimized, otherwise flash it...

void TSAPI CreateNewTabForContact(TContainerData *pContainer, MCONTACT hContact, bool bActivateTab, bool bPopupContainer, bool bWantPopup, MEVENT hdbEvent, bool bIsUnicode, const char *pszInitialText)
{
	if (hContact == 0) {
		_DebugPopup(hContact, L"Warning: trying to create a window for empty contact");
		return;
	}

	if (Srmm_FindWindow(hContact) != nullptr) {
		_DebugPopup(hContact, L"Warning: trying to create duplicate window");
		return ;
	}

	// if we have a max # of tabs/container set and want to open something in the default container...
	if (M.GetByte("limittabs", 0) && !wcsncmp(pContainer->m_wszName, L"default", 6))
		if ((pContainer = FindMatchingContainer(L"default")) == nullptr)
			if ((pContainer = CreateContainer(L"default", CNT_CREATEFLAG_CLONED, hContact)) == nullptr)
				return;

	char *szProto = Proto_GetBaseAccountName(hContact);

	// obtain various status information about the contact
	wchar_t *contactName = Clist_GetContactDisplayName(hContact);

	// cut nickname if larger than x chars...
	wchar_t newcontactname[128], tabtitle[128];
	if (contactName && mir_wstrlen(contactName) > 0) {
		if (M.GetByte("cuttitle", 0))
			CutContactName(contactName, newcontactname, _countof(newcontactname));
		else
			wcsncpy_s(newcontactname, contactName, _TRUNCATE);

		Utils::DoubleAmpersands(newcontactname, _countof(newcontactname));
	}
	else wcsncpy_s(newcontactname, L"_U_", _TRUNCATE);

	wchar_t *szStatus = Clist_GetStatusModeDescription(szProto == nullptr ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), 0);

	if (PluginConfig.m_bStatusOnTabs)
		mir_snwprintf(tabtitle, L"%s (%s)", newcontactname, szStatus);
	else
		mir_snwprintf(tabtitle, L"%s", newcontactname);

	// hide the active tab
	if (pContainer->m_hwndActive && bActivateTab)
		ShowWindow(pContainer->m_hwndActive, SW_HIDE);

	int iTabIndex_wanted = M.GetDword(hContact, "tabindex", pContainer->m_iChilds * 100);
	int iCount = TabCtrl_GetItemCount(pContainer->m_hwndTabs);

	pContainer->m_iTabIndex = iCount;
	if (iCount > 0) {
		for (int i = iCount - 1; i >= 0; i--) {
			HWND hwnd = GetTabWindow(pContainer->m_hwndTabs, i);
			CMsgDialog *dat = (CMsgDialog*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (dat) {
				int relPos = M.GetDword(dat->m_hContact, "tabindex", i * 100);
				if (iTabIndex_wanted <= relPos)
					pContainer->m_iTabIndex = i;
			}
		}
	}

	TCITEM item = {};
	item.pszText = tabtitle;
	item.mask = TCIF_TEXT | TCIF_IMAGE;
	item.iImage = 0;
	item.cchTextMax = _countof(tabtitle);
	int iTabId = TabCtrl_InsertItem(pContainer->m_hwndTabs, pContainer->m_iTabIndex, &item);

	SendMessage(pContainer->m_hwndTabs, EM_REFRESHWITHOUTCLIP, 0, 0);
	if (bActivateTab)
		TabCtrl_SetCurSel(pContainer->m_hwndTabs, iTabId);
	
	CMsgDialog *pWindow = new CMsgDialog(IDD_MSGSPLITNEW, hContact);
	pWindow->m_iTabID = iTabId;
	pWindow->m_pContainer = pContainer;
	pContainer->m_iChilds++;

	pWindow->m_bActivate = bActivateTab;
	pWindow->m_bWantPopup = bWantPopup;
	pWindow->m_hDbEventFirst = hdbEvent;
	if (pszInitialText)
		pWindow->wszInitialText = (bIsUnicode) ? mir_wstrdup((const wchar_t*)pszInitialText) : mir_a2u(pszInitialText);
	pWindow->SetParent(pContainer->m_hwndTabs);
	pWindow->Create();

	// switchbar support
	if (pContainer->cfg.flags.m_bSideBar)
		pContainer->m_pSideBar->addSession(pWindow, pContainer->m_iTabIndex);

	SendMessage(pContainer->m_hwnd, WM_SIZE, 0, 0);

	// if the container is minimized, then pop it up...
	if (IsIconic(pContainer->m_hwnd)) {
		if (bPopupContainer) {
			SendMessage(pContainer->m_hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
			SetFocus(pContainer->m_hwndActive);
		}
		else {
			if (pContainer->cfg.flags.m_bNoFlash)
				pContainer->SetIcon(0, Skin_LoadIcon(SKINICON_EVENT_MESSAGE));
			else
				pContainer->FlashContainer(1, 0);
		}
	}

	if (bActivateTab) {
		pWindow->ActivateTab();
		SetFocus(pWindow->GetHwnd());
		RedrawWindow(pContainer->m_hwnd, nullptr, nullptr, RDW_ERASENOW);
		UpdateWindow(pContainer->m_hwnd);
		if (GetForegroundWindow() != pContainer->m_hwnd && bPopupContainer == TRUE)
			SetForegroundWindow(pContainer->m_hwnd);
	}
	else if (!IsIconic(pContainer->m_hwnd) && IsWindowVisible(pContainer->m_hwnd)) {
		SendMessage(pContainer->m_hwndActive, WM_SIZE, 0, 0);
		RedrawWindow(pContainer->m_hwndActive, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
		RedrawWindow(pContainer->m_hwndActive, nullptr, nullptr, RDW_ERASENOW | RDW_UPDATENOW);
	}

	if (PluginConfig.m_bHideOnClose && !IsWindowVisible(pContainer->m_hwnd)) {
		WINDOWPLACEMENT wp = { 0 };
		wp.length = sizeof(wp);
		GetWindowPlacement(pContainer->m_hwnd, &wp);

		pContainer->BroadCastContainer(DM_CHECKSIZE, 0, 0); // make sure all tabs will re-check layout on activation
		if (wp.showCmd == SW_SHOWMAXIMIZED)
			ShowWindow(pContainer->m_hwnd, SW_SHOWMAXIMIZED);
		else {
			if (bPopupContainer)
				ShowWindow(pContainer->m_hwnd, SW_SHOWNORMAL);
			else
				ShowWindow(pContainer->m_hwnd, SW_SHOWMINNOACTIVE);
		}
		SendMessage(pContainer->m_hwndActive, WM_SIZE, 0, 0);
	}

	if (IsWinVer7Plus() && PluginConfig.m_useAeroPeek && CSkin::m_skinEnabled)
		CWarning::show(CWarning::WARN_AEROPEEK_SKIN, MB_ICONWARNING | MB_OK);

	if (ServiceExists(MS_HPP_EG_EVENT) && ServiceExists(MS_IEVIEW_EVENT) && db_get_b(0, "HistoryPlusPlus", "IEViewAPI", 0))
		if (IDYES == CWarning::show(CWarning::WARN_HPP_APICHECK, MB_ICONWARNING | MB_YESNO))
			db_set_b(0, "HistoryPlusPlus", "IEViewAPI", 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// this is used by the 2nd containermode (limit tabs on default containers).
// it searches a container with "room" for the new tabs or otherwise creates
// a new (cloned) one.

TContainerData* TSAPI FindMatchingContainer(const wchar_t *szName)
{
	int iMaxTabs = M.GetDword("maxtabs", 0);
	if (iMaxTabs > 0 && M.GetByte("limittabs", 0) && !wcsncmp(szName, L"default", 6)) {
		// search a "default" with less than iMaxTabs opened...
		for (TContainerData *p = pFirstContainer; p; p = p->pNext)
			if (!wcsncmp(p->m_wszName, L"default", 6) && p->m_iChilds < iMaxTabs)
				return p;

		return nullptr;
	}
	return FindContainerByName(szName);
}

/////////////////////////////////////////////////////////////////////////////////////////
// load some global icons.

void TSAPI CreateImageList(bool bInitial)
{
	// the imagelist is now a fake. It is still needed to provide the tab control with a
	// image list handle. This will make sure that the tab control will reserve space for
	// an icon on each tab. This is a blank and empty icon
	if (bInitial) {
		PluginConfig.g_hImageList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 0);
		HICON hIcon = CreateIcon(g_plugin.getInst(), 16, 16, 1, 4, nullptr, nullptr);
		ImageList_AddIcon(PluginConfig.g_hImageList, hIcon);
		DestroyIcon(hIcon);
	}

	PluginConfig.g_IconFileEvent = Skin_LoadIcon(SKINICON_EVENT_FILE);
	PluginConfig.g_IconMsgEvent = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
	PluginConfig.g_IconMsgEventBig = Skin_LoadIcon(SKINICON_EVENT_MESSAGE, true);
	PluginConfig.g_IconTypingEventBig = Skin_LoadIcon(SKINICON_OTHER_TYPING, true);
	PluginConfig.g_IconSend = PluginConfig.g_buttonBarIcons[9];
	PluginConfig.g_IconTypingEvent = PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING];
}

/////////////////////////////////////////////////////////////////////////////////////////
// standard icon definitions

static TIconDesc _toolbaricons[] =
{
	{ "tabSRMM_mlog", LPGEN("Message Log Options"), &PluginConfig.g_buttonBarIcons[2], -IDI_MSGLOGOPT, 1 },
	{ "tabSRMM_multi", LPGEN("Image tag"), &PluginConfig.g_buttonBarIcons[3], -IDI_IMAGETAG, 1 },
	{ "tabSRMM_quote", LPGEN("Quote text"), &PluginConfig.g_buttonBarIcons[8], -IDI_QUOTE, 1 },
	{ "tabSRMM_save", LPGEN("Save and close"), &PluginConfig.g_buttonBarIcons[7], -IDI_SAVE, 1 },
	{ "tabSRMM_send", LPGEN("Send message"), &PluginConfig.g_buttonBarIcons[9], -IDI_SEND, 1 },
	{ "tabSRMM_avatar", LPGEN("Edit user notes"), &PluginConfig.g_buttonBarIcons[10], -IDI_CONTACTPIC, 1 },
	{ "tabSRMM_close", LPGEN("Close"), &PluginConfig.g_buttonBarIcons[6], -IDI_CLOSEMSGDLG, 1 }
};

static TIconDesc _exttoolbaricons[] =
{
	{ "tabSRMM_emoticon", LPGEN("Smiley button"), &PluginConfig.g_buttonBarIcons[11], -IDI_SMILEYICON, 1 },
	{ "tabSRMM_bold", LPGEN("Format bold"), &PluginConfig.g_buttonBarIcons[17], -IDI_FONTBOLD, 1 },
	{ "tabSRMM_italic", LPGEN("Format italic"), &PluginConfig.g_buttonBarIcons[18], -IDI_FONTITALIC, 1 },
	{ "tabSRMM_underline", LPGEN("Format underline"), &PluginConfig.g_buttonBarIcons[19], -IDI_FONTUNDERLINE, 1 },
	{ "tabSRMM_face", LPGEN("Font face"), &PluginConfig.g_buttonBarIcons[20], -IDI_FONTFACE, 1 },
	{ "tabSRMM_color", LPGEN("Font color"), &PluginConfig.g_buttonBarIcons[21], -IDI_FONTCOLOR, 1 },
	{ "tabSRMM_strikeout", LPGEN("Format strike-through"), &PluginConfig.g_buttonBarIcons[30], -IDI_STRIKEOUT, 1 }
};

static TIconDesc _chattoolbaricons[] =
{
	{ "chat_bkgcol", LPGEN("Background color"), &PluginConfig.g_buttonBarIcons[31], -IDI_BKGCOLOR, 1 },
	{ "chat_settings", LPGEN("Room settings"), &PluginConfig.g_buttonBarIcons[32], -IDI_TOPICBUT, 1 },
	{ "chat_filter", LPGEN("Event filter"), &PluginConfig.g_buttonBarIcons[33], -IDI_FILTER2, 1 },
	{ "chat_shownicklist", LPGEN("Nick list"), &PluginConfig.g_buttonBarIcons[35], -IDI_SHOWNICKLIST, 1 }
};

static TIconDesc _logicons[] =
{
	{ "tabSRMM_error", LPGEN("Message delivery error"), &PluginConfig.g_iconErr, -IDI_MSGERROR, 1 },
	{ "tabSRMM_in", LPGEN("Incoming message"), &PluginConfig.g_iconIn, -IDI_ICONIN, 0 },
	{ "tabSRMM_out", LPGEN("Outgoing message"), &PluginConfig.g_iconOut, -IDI_ICONOUT, 0 },
	{ "tabSRMM_status", LPGEN("Status change"), &PluginConfig.g_iconStatus, -IDI_STATUSCHANGE, 0 }
};

static TIconDesc _deficons[] =
{
	{ "tabSRMM_container", LPGEN("Static container icon"), &PluginConfig.g_iconContainer, -IDI_CONTAINER, 1 },
	{ "tabSRMM_sounds_on", LPGEN("Sounds (status bar)"), &PluginConfig.g_buttonBarIcons[ICON_DEFAULT_SOUNDS], -IDI_SOUNDSON, 1 },
	{ "tabSRMM_pulldown", LPGEN("Pulldown Arrow"), &PluginConfig.g_buttonBarIcons[ICON_DEFAULT_PULLDOWN], -IDI_PULLDOWNARROW, 1 },
	{ "tabSRMM_Leftarrow", LPGEN("Left Arrow"), &PluginConfig.g_buttonBarIcons[ICON_DEFAULT_LEFT], -IDI_LEFTARROW, 1 },
	{ "tabSRMM_Rightarrow", LPGEN("Right Arrow"), &PluginConfig.g_buttonBarIcons[ICON_DEFAULT_RIGHT], -IDI_RIGHTARROW, 1 },
	{ "tabSRMM_Pulluparrow", LPGEN("Up Arrow"), &PluginConfig.g_buttonBarIcons[ICON_DEFAULT_UP], -IDI_PULLUPARROW, 1 },
};

static TIconDesc _trayIcon[] =
{
	{ "tabSRMM_frame1", LPGEN("Frame 1"), &PluginConfig.m_AnimTrayIcons[0], -IDI_TRAYANIM1, 1 },
	{ "tabSRMM_frame2", LPGEN("Frame 2"), &PluginConfig.m_AnimTrayIcons[1], -IDI_TRAYANIM2, 1 },
	{ "tabSRMM_frame3", LPGEN("Frame 3"), &PluginConfig.m_AnimTrayIcons[2], -IDI_TRAYANIM3, 1 },
	{ "tabSRMM_frame4", LPGEN("Frame 4"), &PluginConfig.m_AnimTrayIcons[3], -IDI_TRAYANIM4, 1 },
};

struct {
	char *szSection;
	TIconDesc *idesc;
	int nItems;
}
static ICONBLOCKS[] = {
	{ LPGEN("Messaging") "/" LPGEN("Default"), _deficons, _countof(_deficons) },
	{ LPGEN("Messaging") "/" LPGEN("Toolbar"), _toolbaricons, _countof(_toolbaricons) },
	{ LPGEN("Messaging") "/" LPGEN("Toolbar"), _exttoolbaricons, _countof(_exttoolbaricons) },
	{ LPGEN("Messaging") "/" LPGEN("Toolbar"), _chattoolbaricons, _countof(_chattoolbaricons) },
	{ LPGEN("Messaging") "/" LPGEN("Message Log"), _logicons, _countof(_logicons) },
	{ LPGEN("Messaging") "/" LPGEN("Animated Tray"), _trayIcon, _countof(_trayIcon) }
};

/////////////////////////////////////////////////////////////////////////////////////////
// setup default icons for the IcoLib service. This needs to be done every time the
// plugin is loaded default icons are taken from the icon pack in either \icons or \plugins

static int TSAPI SetupIconLibConfig()
{
	int j = 2;

	HINSTANCE hIconDll = LoadLibraryA("icons\\tabsrmm_icons.dll");
	if (hIconDll == nullptr) {
		CWarning::show(CWarning::WARN_ICONPACKMISSING, CWarning::CWF_NOALLOWHIDE | MB_ICONERROR | MB_OK);
		return 0;
	}

	wchar_t szFilename[MAX_PATH];
	GetModuleFileName(hIconDll, szFilename, MAX_PATH);
	Chat_AddIcons(hIconDll);
	FreeLibrary(hIconDll);

	SKINICONDESC sid = {};
	sid.defaultFile.w = szFilename;
	sid.flags = SIDF_PATH_UNICODE;

	for (int n = 0; n < _countof(ICONBLOCKS); n++) {
		auto &it = ICONBLOCKS[n];
		sid.section.a = it.szSection;
		for (int i = 0; i < it.nItems; i++) {
			sid.pszName = it.idesc[i].szName;
			sid.description.a = it.idesc[i].szDesc;
			sid.iDefaultIndex = it.idesc[i].uId == -IDI_HISTORY ? 0 : it.idesc[i].uId;        // workaround problem /w icoLib and a resource id of 1 (actually, a Windows problem)

			if (n > 0 && n < 4)
				PluginConfig.g_buttonBarIconHandles[j++] = g_plugin.addIcon(&sid);
			else
				g_plugin.addIcon(&sid);
		}
	}

	sid.section.a = LPGEN("Messaging") "/" LPGEN("Default");
	sid.pszName = "tabSRMM_clock_symbol";
	sid.description.a = LPGEN("Clock symbol (for the info panel clock)");
	sid.iDefaultIndex = -IDI_CLOCK;
	g_plugin.addIcon(&sid);

	wcsncpy_s(szFilename, L"plugins\\tabsrmm.dll", _TRUNCATE);

	sid.pszName = "tabSRMM_overlay_disabled";
	sid.description.a = LPGEN("Feature disabled (used as overlay)");
	sid.iDefaultIndex = -IDI_FEATURE_DISABLED;
	g_plugin.addIcon(&sid);

	sid.pszName = "tabSRMM_overlay_enabled";
	sid.description.a = LPGEN("Feature enabled (used as overlay)");
	sid.iDefaultIndex = -IDI_FEATURE_ENABLED;
	g_plugin.addIcon(&sid);
	return 1;
}

// load the icon theme from IconLib - check if it exists...
static int TSAPI LoadFromIconLib()
{
	for (auto &it : ICONBLOCKS)
		for (int i = 0; i < it.nItems; i++)
			*(it.idesc[i].phIcon) = IcoLib_GetIcon(it.idesc[i].szName);

	PluginConfig.g_buttonBarIcons[0] = Skin_LoadIcon(SKINICON_OTHER_ADDCONTACT);
	PluginConfig.g_buttonBarIcons[1] = Skin_LoadIcon(SKINICON_OTHER_HISTORY);
	PluginConfig.g_buttonBarIconHandles[0] = Skin_GetIconHandle(SKINICON_OTHER_HISTORY);
	PluginConfig.g_buttonBarIconHandles[1] = Skin_GetIconHandle(SKINICON_OTHER_ADDCONTACT);
	PluginConfig.g_buttonBarIconHandles[20] = Skin_GetIconHandle(SKINICON_OTHER_USERDETAILS);

	PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING] = PluginConfig.g_buttonBarIcons[12] = Skin_LoadIcon(SKINICON_OTHER_TYPING);

	PluginConfig.g_iconOverlayEnabled = IcoLib_GetIcon("tabSRMM_overlay_enabled");
	PluginConfig.g_iconOverlayDisabled = IcoLib_GetIcon("tabSRMM_overlay_disabled");

	PluginConfig.g_iconClock = IcoLib_GetIcon("tabSRMM_clock_symbol");

	CacheMsgLogIcons();
	Srmm_Broadcast(WM_CBD_LOADICONS, 0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// load icon theme from either icon pack or IcoLib

void TSAPI LoadIconTheme()
{
	if (SetupIconLibConfig() == 0)
		return;
	else
		LoadFromIconLib();

	Skin->setupTabCloseBitmap();
}

static void UnloadIcons()
{
	for (auto &it : ICONBLOCKS)
		for (int i = 0; i < it.nItems; i++)
			if (*(it.idesc[i].phIcon) != nullptr) {
				DestroyIcon(*(it.idesc[i].phIcon));
				*(it.idesc[i].phIcon) = nullptr;
			}

	if (PluginConfig.g_hbmUnknown)
		DeleteObject(PluginConfig.g_hbmUnknown);

	for (int i = 0; i < 4; i++)
		if (PluginConfig.m_AnimTrayIcons[i])
			DestroyIcon(PluginConfig.m_AnimTrayIcons[i]);
}

/////////////////////////////////////////////////////////////////////////////////////////

int IcoLibIconsChanged(WPARAM, LPARAM)
{
	LoadFromIconLib();
	CacheMsgLogIcons();
	return 0;
}

int IconsChanged(WPARAM, LPARAM)
{
	CreateImageList(false);
	CacheMsgLogIcons();
	Srmm_Broadcast(DM_OPTIONSAPPLIED, 0, 0);
	Srmm_Broadcast(DM_UPDATEWINICON, 0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// services needed for assembly management

static INT_PTR ReloadSkin(WPARAM, LPARAM)
{
	Skin->setFileName();
	Skin->Load();
	return 0;
}

static INT_PTR ReloadSettings(WPARAM, LPARAM lParam)
{
	PluginConfig.reloadSettings(lParam != 0);
	return 0;
}

static INT_PTR svcQMgr(WPARAM, LPARAM)
{
	SendLater::invokeQueueMgrDlg();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// initialises the internal API, services, events etc...

static void TSAPI InitAPI()
{
	CreateServiceFunction(MS_MSG_READMESSAGE, ReadMessageCommand);
	CreateServiceFunction(MS_MSG_SENDMESSAGE, SendMessageCommand);
	CreateServiceFunction(MS_MSG_SENDMESSAGEW, SendMessageCommand_W);
	CreateServiceFunction(MS_MSG_TYPINGMESSAGE, TypingMessageCommand);

	CreateServiceFunction("TabSRMsg/ReloadSkin", ReloadSkin);
	CreateServiceFunction("TabSRMsg/ReloadSettings", ReloadSettings);

	CreateServiceFunction(MS_TABMSG_SETUSERPREFS, SetUserPrefs);
	CreateServiceFunction(MS_TABMSG_SLQMGR, svcQMgr);

	SI_InitStatusIcons();
}

int LoadSendRecvMessageModule(void)
{
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	g_hMsftedit = Utils::loadSystemLibrary(L"\\Msftedit.dll");

	Win7Taskbar = new CTaskbarInteract;
	Win7Taskbar->updateMetrics();

	PluginConfig.hUserPrefsWindowList = WindowList_Create();
	sendQueue = new SendQueue;
	Skin = new CSkin;

	if (!db_is_module_empty(0, NEN_OLD_MODULE)) {
		db_copy_module(NEN_OLD_MODULE, NEN_MODULE);
		db_delete_module(0, NEN_OLD_MODULE);
	}

	if (M.GetDword("cWarningsV", 0) == 0)
		db_set_dw(0, SRMSGMOD_T, "cWarningsV", M.GetDword("cWarningsL", 0));

	HookEvent(ME_OPT_INITIALISE, OptInitialise);

	InitAPI();

	PluginConfig.reloadSystemStartup();
	ReloadTabConfig();

	LoadDefaultTemplates();
	return 0;
}
