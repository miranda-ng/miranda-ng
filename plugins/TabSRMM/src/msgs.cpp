/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-17 Miranda NG project,
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

NEN_OPTIONS nen_options;
static HANDLE hUserPrefsWindowLis = 0;
HMODULE g_hIconDLL = 0, g_hMsftedit;

static void UnloadIcons();

void Chat_AddIcons();

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
	pci->SM_BroadcastMessage(nullptr, DM_SMILEYOPTIONSCHANGED, 0, 0, FALSE);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// basic window class

CTabBaseDlg::CTabBaseDlg(int iResource, SESSION_INFO *si)
	: CSrmmBaseDialog(g_hInst, iResource, si),
	m_pPanel(this),
	m_dwFlags(MWF_INITMODE),
	m_iInputAreaHeight(-1)
{
	m_autoClose = CLOSE_ON_CANCEL;
	m_forceResizable = true;
}

CTabBaseDlg::~CTabBaseDlg()
{
	delete m_pWnd;

	mir_free(m_sendBuffer);
	mir_free(m_hHistoryEvents);
	mir_free(m_hQueuedEvents);

	if (m_hClientIcon) DestroyIcon(m_hClientIcon);
	if (m_hSmileyIcon) DestroyIcon(m_hSmileyIcon);
	if (m_hXStatusIcon) DestroyIcon(m_hXStatusIcon);
	if (m_hTaskbarIcon) DestroyIcon(m_hTaskbarIcon);
}

void CTabBaseDlg::LoadSettings()
{
	m_clrInputBG = m_pContainer->theme.inputbg;
	LoadLogfont(FONTSECTION_IM, MSGFONTID_MESSAGEAREA, nullptr, &m_clrInputFG, FONTMODULE);
}

void CTabBaseDlg::OnInitDialog()
{
	CSrmmBaseDialog::OnInitDialog();

	// m_hwnd is valid, pass it to the tab control
	TCITEM tci;
	tci.mask = TCIF_PARAM;
	tci.lParam = (LPARAM)m_hwnd;
	TabCtrl_SetItem(m_hwndParent, m_iTabID, &tci);

	// update another tab ids
	m_pContainer->UpdateTabs();
	
	// add this window to window list & proxy
	if (PluginConfig.m_bIsWin7 && PluginConfig.m_useAeroPeek)
		m_pWnd = new CProxyWindow(this);
	else
		m_pWnd = nullptr;

	// set up Windows themes
	DM_ThemeChanged();

	// refresh cache data for this contact
	m_cache = CContactCache::getContactCache(m_hContact);
	m_cache->updateNick();
	m_cache->updateUIN();

	m_bIsAutosizingInput = IsAutoSplitEnabled();
}

INT_PTR CTabBaseDlg::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case DM_SETINFOPANEL: // broadcasted when global info panel setting changes
		if (wParam == 0 && lParam == 0) {
			m_pPanel.getVisibility();
			m_pPanel.loadHeight();
			m_pPanel.showHide();
		}
		else {
			CTabBaseDlg *srcDat = (CTabBaseDlg*)wParam;
			if (lParam == 0)
				m_pPanel.loadHeight();
			else {
				if (srcDat && lParam && this != srcDat && !m_pPanel.isPrivateHeight()) {
					if (srcDat->isChat() != isChat() && M.GetByte("syncAllPanels", 0) == 0)
						return 0;

					if (m_pContainer->settings->fPrivate && srcDat->m_pContainer != m_pContainer)
						return 0;

					m_pPanel.setHeight((LONG)lParam);
				}
			}
			Resize();
		}
		return 0;

	case DM_ACTIVATEME: // the child window will activate itself
		ActivateExistingTab(m_pContainer, m_hwnd);
		return 0;

	case DM_SETLOCALE:
		if (m_dwFlags & MWF_WASBACKGROUNDCREATE)
			break;
		if (m_pContainer->m_hwndActive == m_hwnd && PluginConfig.m_bAutoLocaleSupport && m_pContainer->m_hwnd == GetForegroundWindow() && m_pContainer->m_hwnd == GetActiveWindow()) {
			if (lParam)
				m_hkl = (HKL)lParam;

			if (m_hkl)
				ActivateKeyboardLayout(m_hkl, 0);
		}
		return 0;
		
	case DM_QUERYCONTAINER: // container API support functions
		if (lParam)
			*(TContainerData**)lParam = m_pContainer;
		return 0;

	case DM_QUERYHCONTACT:
		if (lParam)
			*(MCONTACT*)lParam = m_hContact;
		return 0;

	case DM_CHECKSIZE:
		m_dwFlags |= MWF_NEEDCHECKSIZE;
		return 0;
		
	case DM_CONTAINERSELECTED:
		// sent by the select container dialog box when a container was selected...
		// lParam = (wchar_t*)selected name...
		{
			wchar_t *szNewName = (wchar_t*)lParam;
			if (!mir_wstrcmp(szNewName, TranslateT("Default container")))
				szNewName = CGlobals::m_default_container_name;

			int iOldItems = TabCtrl_GetItemCount(m_hwndParent);
			if (!wcsncmp(m_pContainer->m_wszName, szNewName, CONTAINER_NAMELEN))
				break;

			TContainerData *pNewContainer = FindContainerByName(szNewName);
			if (pNewContainer == nullptr)
				if ((pNewContainer = CreateContainer(szNewName, FALSE, m_hContact)) == nullptr)
					break;

			db_set_ws(m_hContact, SRMSGMOD_T, "containerW", szNewName);
			PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_DOCREATETAB, (WPARAM)pNewContainer, m_hContact);
			if (iOldItems > 1)                // there were more than 1 tab, container is still valid
				SendMessage(m_pContainer->m_hwndActive, WM_SIZE, 0, 0);
			SetForegroundWindow(pNewContainer->m_hwnd);
			SetActiveWindow(pNewContainer->m_hwnd);
		}
		return 0;

	case DM_ACTIVATETOOLTIP:
		// show the balloon tooltip control.
		// wParam == id of the "anchor" element, defaults to the panel status field (for away msg retrieval)
		// lParam == new text to show
		if (!IsIconic(m_pContainer->m_hwnd) && m_pContainer->m_hwndActive == m_hwnd)
			m_pPanel.showTip(wParam, lParam);
		return 0;

	case DM_STATUSBARCHANGED:
		tabUpdateStatusBar();
		break;

	case DM_CHECKAUTOHIDE:
		// This is broadcasted by the container to all child windows to check if the
		// container can be autohidden or -closed.
		//
		// wParam is the autohide timeout (in seconds)
		// lParam points to a BOOL and a session which wants to prevent auto-hiding
		// the container must set it to FALSE.
		//
		// If no session in the container disagrees, the container will be hidden.

		if (lParam) {
			BOOL *fResult = (BOOL*)lParam;
			// text entered in the input area -> prevent autohide/cose
			if (GetWindowTextLength(m_message.GetHwnd()) > 0)
				*fResult = FALSE;
			// unread events, do not hide or close the container
			else if (m_dwUnread)
				*fResult = FALSE;
			// time since last activity did not yet reach the threshold.
			else if (((GetTickCount() - m_dwLastActivity) / 1000) <= wParam)
				*fResult = FALSE;
		}
		return 0;

	case DM_SPLITTERGLOBALEVENT:
		DM_SplitterGlobalEvent(wParam, lParam);
		return 0;
	}

	return CSrmmBaseDialog::DlgProc(msg, wParam, lParam);
}

void CTabBaseDlg::NotifyDeliveryFailure() const
{
	if (M.GetByte("adv_noErrorPopups", 0))
		return;

	if (CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0) != 1)
		return;

	POPUPDATAT ppd = { 0 };
	ppd.lchContact = m_hContact;
	wcsncpy_s(ppd.lptzContactName, m_cache->getNick(), _TRUNCATE);
	wcsncpy_s(ppd.lptzText, TranslateT("A message delivery has failed.\nClick to open the message window."), _TRUNCATE);

	if (!(BOOL)M.GetByte(MODULE, OPT_COLDEFAULT_ERR, TRUE)) {
		ppd.colorText = (COLORREF)M.GetDword(MODULE, OPT_COLTEXT_ERR, DEFAULT_COLTEXT);
		ppd.colorBack = (COLORREF)M.GetDword(MODULE, OPT_COLBACK_ERR, DEFAULT_COLBACK);
	}
	else ppd.colorText = ppd.colorBack = 0;

	ppd.PluginWindowProc = Utils::PopupDlgProcError;
	ppd.lchIcon = PluginConfig.g_iconErr;
	ppd.PluginData = 0;
	ppd.iSeconds = (int)M.GetDword(MODULE, OPT_DELAY_ERR, (DWORD)DEFAULT_DELAY);
	PUAddPopupT(&ppd);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Sets a status bar text for a contact

void CTabBaseDlg::SetStatusText(const wchar_t *wszText, HICON hIcon)
{
	if (wszText != nullptr) {
		m_bStatusSet = true;
		m_szStatusText = wszText;
		m_szStatusIcon = hIcon;
	}
	else {
		m_bStatusSet = false;
		m_szStatusText.Empty();
		m_szStatusIcon = nullptr;
	}

	tabUpdateStatusBar();
}

/////////////////////////////////////////////////////////////////////////////////////////
// service function. Invoke the user preferences dialog for the contact given (by handle) in wParam

static INT_PTR SetUserPrefs(WPARAM wParam, LPARAM)
{
	HWND hWnd = WindowList_Find(PluginConfig.hUserPrefsWindowList, wParam);
	if (hWnd) {
		SetForegroundWindow(hWnd);			// already open, bring it to front
		return 0;
	}
	CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_USERPREFS_FRAME), 0, DlgProcUserPrefsFrame, (LPARAM)wParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// service function - open the tray menu from the TTB button

static INT_PTR Service_OpenTrayMenu(WPARAM, LPARAM lParam)
{
	SendMessage(PluginConfig.g_hwndHotkeyHandler, DM_TRAYICONNOTIFY, 101, lParam == 0 ? WM_LBUTTONUP : WM_RBUTTONUP);
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

int TSAPI MessageWindowOpened(MCONTACT hContact, HWND _hwnd)
{
	HWND hwnd = 0;
	TContainerData *pContainer = nullptr;

	if (hContact)
		hwnd = Srmm_FindWindow(hContact);
	else if (_hwnd)
		hwnd = _hwnd;
	else
		return 0;

	if (!hwnd)
		return 0;

	SendMessage(hwnd, DM_QUERYCONTAINER, 0, (LPARAM)&pContainer);
	if (pContainer) {
		if (pContainer->dwFlags & CNT_DONTREPORT) {
			if (IsIconic(pContainer->m_hwnd))
				return 0;
		}
		if (pContainer->dwFlags & CNT_DONTREPORTUNFOCUSED) {
			if (!IsIconic(pContainer->m_hwnd) && GetForegroundWindow() != pContainer->m_hwnd && GetActiveWindow() != pContainer->m_hwnd)
				return 0;
		}
		if (pContainer->dwFlags & CNT_ALWAYSREPORTINACTIVE) {
			if (pContainer->dwFlags & CNT_DONTREPORTFOCUSED)
				return 0;

			return pContainer->m_hwndActive == hwnd;
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

	HWND hwndExisting = Srmm_FindWindow(hContact);
	if (hwndExisting != 0)
		SendMessage(hwndExisting, DM_ACTIVATEME, 0, 0);
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
	char *szProto = GetContactProto(hContact);
	if (szProto == nullptr)
		return 0; // unknown contact
	if (!CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND)
		return 0;

	HWND hwnd = Srmm_FindWindow(hContact);
	if (hwnd) {
		if (pszMsg) {
			HWND hEdit = GetDlgItem(hwnd, IDC_SRMM_MESSAGE);
			SendMessage(hEdit, EM_SETSEL, -1, GetWindowTextLength(hEdit));
			if (isWchar)
				SendMessageW(hEdit, EM_REPLACESEL, FALSE, (LPARAM)pszMsg);
			else
				SendMessageA(hEdit, EM_REPLACESEL, FALSE, (LPARAM)pszMsg);
		}
		SendMessage(hwnd, DM_ACTIVATEME, 0, 0);
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

	if (g_hIconDLL) {
		FreeLibrary(g_hIconDLL);
		g_hIconDLL = nullptr;
	}

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

int MyAvatarChanged(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0 || IsBadReadPtr((void*)wParam, 4))
		return 0;

	for (TContainerData *p = pFirstContainer; p; p = p->pNext)
		BroadCastContainer(p, DM_MYAVATARCHANGED, wParam, lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// tabbed mode support functions...
// (C) by Nightwish
//
// this function searches and activates the tab belonging to the given hwnd (which is the
// hwnd of a message dialog window)

int TSAPI ActivateExistingTab(TContainerData *pContainer, HWND hwndChild)
{
	CSrmmWindow *dat = (CSrmmWindow*)GetWindowLongPtr(hwndChild, GWLP_USERDATA);	// needed to obtain the hContact for the message window
	if (!dat || !pContainer)
		return FALSE;

	NMHDR nmhdr = { 0 };
	nmhdr.code = TCN_SELCHANGE;
	if (TabCtrl_GetItemCount(GetDlgItem(pContainer->m_hwnd, IDC_MSGTABS)) > 1 && !(pContainer->dwFlags & CNT_DEFERREDTABSELECT)) {
		TabCtrl_SetCurSel(GetDlgItem(pContainer->m_hwnd, IDC_MSGTABS), GetTabIndexFromHWND(GetDlgItem(pContainer->m_hwnd, IDC_MSGTABS), hwndChild));
		SendMessage(pContainer->m_hwnd, WM_NOTIFY, 0, (LPARAM)&nmhdr);	// just select the tab and let WM_NOTIFY do the rest
	}
	if (!dat->isChat())
		pContainer->UpdateTitle(dat->m_hContact);
	if (IsIconic(pContainer->m_hwnd)) {
		SendMessage(pContainer->m_hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		SetForegroundWindow(pContainer->m_hwnd);
	}

	// hide on close feature
	if (!IsWindowVisible(pContainer->m_hwnd)) {
		WINDOWPLACEMENT wp = { 0 };
		wp.length = sizeof(wp);
		GetWindowPlacement(pContainer->m_hwnd, &wp);

		// all tabs must re-check the layout on activation because adding a tab while
		// the container was hidden can make this necessary
		BroadCastContainer(pContainer, DM_CHECKSIZE, 0, 0);
		if (wp.showCmd == SW_SHOWMAXIMIZED)
			ShowWindow(pContainer->m_hwnd, SW_SHOWMAXIMIZED);
		else {
			ShowWindow(pContainer->m_hwnd, SW_SHOWNA);
			SetForegroundWindow(pContainer->m_hwnd);
		}
		SendMessage(pContainer->m_hwndActive, WM_SIZE, 0, 0);			// make sure the active tab resizes its layout properly
	}
	else if (GetForegroundWindow() != pContainer->m_hwnd)
		SetForegroundWindow(pContainer->m_hwnd);

	if (!dat->isChat())
		SetFocus(GetDlgItem(hwndChild, IDC_SRMM_MESSAGE));
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// this function creates and activates a new tab within the given container.
// bActivateTab: make the new tab the active one
// bPopupContainer: restore container if it was minimized, otherwise flash it...

HWND TSAPI CreateNewTabForContact(TContainerData *pContainer, MCONTACT hContact, bool bActivateTab, bool bPopupContainer, bool bWantPopup, MEVENT hdbEvent, bool bIsUnicode, const char *pszInitialText)
{
	if (Srmm_FindWindow(hContact) != 0) {
		_DebugPopup(hContact, L"Warning: trying to create duplicate window");
		return 0;
	}

	// if we have a max # of tabs/container set and want to open something in the default container...
	if (hContact != 0 && M.GetByte("limittabs", 0) && !wcsncmp(pContainer->m_wszName, L"default", 6))
		if ((pContainer = FindMatchingContainer(L"default")) == nullptr)
			if ((pContainer = CreateContainer(L"default", CNT_CREATEFLAG_CLONED, hContact)) == nullptr)
				return 0;

	char *szProto = GetContactProto(hContact);

	// obtain various status information about the contact
	wchar_t *contactName = pcli->pfnGetContactDisplayName(hContact, 0);

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

	wchar_t *szStatus = pcli->pfnGetStatusModeDescription(szProto == nullptr ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), 0);

	if (PluginConfig.m_bStatusOnTabs)
		mir_snwprintf(tabtitle, L"%s (%s)", newcontactname, szStatus);
	else
		mir_snwprintf(tabtitle, L"%s", newcontactname);

	HWND hwndTab = GetDlgItem(pContainer->m_hwnd, IDC_MSGTABS);
	// hide the active tab
	if (pContainer->m_hwndActive && bActivateTab)
		ShowWindow(pContainer->m_hwndActive, SW_HIDE);

	int iTabIndex_wanted = M.GetDword(hContact, "tabindex", pContainer->iChilds * 100);
	int iCount = TabCtrl_GetItemCount(hwndTab);

	pContainer->iTabIndex = iCount;
	if (iCount > 0) {
		TCITEM item = {};
		for (int i = iCount - 1; i >= 0; i--) {
			item.mask = TCIF_PARAM;
			TabCtrl_GetItem(hwndTab, i, &item);
			HWND hwnd = (HWND)item.lParam;
			CSrmmWindow *dat = (CSrmmWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (dat) {
				int relPos = M.GetDword(dat->m_hContact, "tabindex", i * 100);
				if (iTabIndex_wanted <= relPos)
					pContainer->iTabIndex = i;
			}
		}
	}

	TCITEM item = {};
	item.pszText = tabtitle;
	item.mask = TCIF_TEXT | TCIF_IMAGE;
	item.iImage = 0;
	item.cchTextMax = _countof(tabtitle);
	int iTabId = TabCtrl_InsertItem(hwndTab, pContainer->iTabIndex, &item);

	SendMessage(hwndTab, EM_REFRESHWITHOUTCLIP, 0, 0);
	if (bActivateTab)
		TabCtrl_SetCurSel(hwndTab, iTabId);
	
	CSrmmWindow *pWindow = new CSrmmWindow();
	pWindow->m_hContact = hContact;
	pWindow->m_iTabID = iTabId;
	pWindow->m_pContainer = pContainer;
	pContainer->iChilds++;

	pWindow->m_bActivate = bActivateTab;
	pWindow->m_bWantPopup = bWantPopup;
	pWindow->m_hDbEventFirst = hdbEvent;
	if (pszInitialText)
		pWindow->wszInitialText = (bIsUnicode) ? mir_wstrdup((const wchar_t*)pszInitialText) : mir_a2u(pszInitialText);
	pWindow->SetParent(hwndTab);
	pWindow->Create();

	HWND hwndNew = pWindow->GetHwnd();

	// switchbar support
	if (pContainer->dwFlags & CNT_SIDEBAR)
		pContainer->SideBar->addSession(pWindow, pContainer->iTabIndex);

	SendMessage(pContainer->m_hwnd, WM_SIZE, 0, 0);

	// if the container is minimized, then pop it up...
	if (IsIconic(pContainer->m_hwnd)) {
		if (bPopupContainer) {
			SendMessage(pContainer->m_hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
			SetFocus(pContainer->m_hwndActive);
		}
		else {
			if (pContainer->dwFlags & CNT_NOFLASH)
				SendMessage(pContainer->m_hwnd, DM_SETICON, 0, (LPARAM)Skin_LoadIcon(SKINICON_EVENT_MESSAGE));
			else
				FlashContainer(pContainer, 1, 0);
		}
	}

	if (bActivateTab) {
		ActivateExistingTab(pContainer, hwndNew);
		SetFocus(hwndNew);
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

	if (PluginConfig.m_bHideOnClose&&!IsWindowVisible(pContainer->m_hwnd)) {
		WINDOWPLACEMENT wp = { 0 };
		wp.length = sizeof(wp);
		GetWindowPlacement(pContainer->m_hwnd, &wp);

		BroadCastContainer(pContainer, DM_CHECKSIZE, 0, 0); // make sure all tabs will re-check layout on activation
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

	if (PluginConfig.m_bIsWin7 && PluginConfig.m_useAeroPeek && CSkin::m_skinEnabled)
		CWarning::show(CWarning::WARN_AEROPEEK_SKIN, MB_ICONWARNING | MB_OK);

	if (ServiceExists(MS_HPP_EG_EVENT) && ServiceExists(MS_IEVIEW_EVENT) && db_get_b(0, "HistoryPlusPlus", "IEViewAPI", 0))
		if (IDYES == CWarning::show(CWarning::WARN_HPP_APICHECK, MB_ICONWARNING | MB_YESNO))
			db_set_b(0, "HistoryPlusPlus", "IEViewAPI", 0);

	return hwndNew;		// return handle of the new dialog
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
			if (!wcsncmp(p->m_wszName, L"default", 6) && p->iChilds < iMaxTabs)
				return p;

		return nullptr;
	}
	return FindContainerByName(szName);
}

/////////////////////////////////////////////////////////////////////////////////////////
// load some global icons.

void TSAPI CreateImageList(BOOL bInitial)
{
	// the imagelist is now a fake. It is still needed to provide the tab control with a
	// image list handle. This will make sure that the tab control will reserve space for
	// an icon on each tab. This is a blank and empty icon
	if (bInitial) {
		PluginConfig.g_hImageList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 0);
		HICON hIcon = CreateIcon(g_hInst, 16, 16, 1, 4, nullptr, nullptr);
		ImageList_AddIcon(PluginConfig.g_hImageList, hIcon);
		DestroyIcon(hIcon);
	}

	PluginConfig.g_IconFileEvent = Skin_LoadIcon(SKINICON_EVENT_FILE);
	PluginConfig.g_IconMsgEvent = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
	PluginConfig.g_IconMsgEventBig = Skin_LoadIcon(SKINICON_EVENT_MESSAGE, true);
	if ((HICON)CALLSERVICE_NOTFOUND == PluginConfig.g_IconMsgEventBig)
		PluginConfig.g_IconMsgEventBig = 0;
	PluginConfig.g_IconTypingEventBig = Skin_LoadIcon(SKINICON_OTHER_TYPING, true);
	if ((HICON)CALLSERVICE_NOTFOUND == PluginConfig.g_IconTypingEventBig)
		PluginConfig.g_IconTypingEventBig = 0;
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
	{ "tabSRMM_sb_slist", LPGEN("Session List"), &PluginConfig.g_sideBarIcons[0], -IDI_SESSIONLIST, 1 },
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
	{ LPGEN("Message Sessions") "/" LPGEN("Default"), _deficons, _countof(_deficons) },
	{ LPGEN("Message Sessions") "/" LPGEN("Toolbar"), _toolbaricons, _countof(_toolbaricons) },
	{ LPGEN("Message Sessions") "/" LPGEN("Toolbar"), _exttoolbaricons, _countof(_exttoolbaricons) },
	{ LPGEN("Message Sessions") "/" LPGEN("Toolbar"), _chattoolbaricons, _countof(_chattoolbaricons) },
	{ LPGEN("Message Sessions") "/" LPGEN("Message Log"), _logicons, _countof(_logicons) },
	{ LPGEN("Message Sessions") "/" LPGEN("Animated Tray"), _trayIcon, _countof(_trayIcon) }
};

static int GetIconPackVersion(HMODULE hDLL)
{
	char szIDString[256];
	int version = 0;

	if (LoadStringA(hDLL, IDS_IDENTIFY, szIDString, sizeof(szIDString)) == 0)
		version = 0;
	else if (!mir_strcmp(szIDString, "__tabSRMM_ICONPACK 1.0__"))
		version = 1;
	else if (!mir_strcmp(szIDString, "__tabSRMM_ICONPACK 2.0__"))
		version = 2;
	else if (!mir_strcmp(szIDString, "__tabSRMM_ICONPACK 3.0__"))
		version = 3;
	else if (!mir_strcmp(szIDString, "__tabSRMM_ICONPACK 3.5__"))
		version = 4;
	else if (!mir_strcmp(szIDString, "__tabSRMM_ICONPACK 5.0__"))
		version = 5;

	if (version < 5)
		CWarning::show(CWarning::WARN_ICONPACK_VERSION, MB_OK | MB_ICONERROR);
	return version;
}

/////////////////////////////////////////////////////////////////////////////////////////
// setup default icons for the IcoLib service. This needs to be done every time the
// plugin is loaded default icons are taken from the icon pack in either \icons or \plugins

static int TSAPI SetupIconLibConfig()
{
	int j = 2, version = 0;

	wchar_t szFilename[MAX_PATH];
	wcsncpy(szFilename, L"icons\\tabsrmm_icons.dll", MAX_PATH);
	g_hIconDLL = LoadLibrary(szFilename);
	if (g_hIconDLL == 0) {
		CWarning::show(CWarning::WARN_ICONPACKMISSING, CWarning::CWF_NOALLOWHIDE | MB_ICONERROR | MB_OK);
		return 0;
	}

	GetModuleFileName(g_hIconDLL, szFilename, MAX_PATH);
	Chat_AddIcons();
	version = GetIconPackVersion(g_hIconDLL);
	FreeLibrary(g_hIconDLL);
	g_hIconDLL = 0;

	SKINICONDESC sid = { 0 };
	sid.defaultFile.w = szFilename;
	sid.flags = SIDF_PATH_UNICODE;

	for (int n = 0; n < _countof(ICONBLOCKS); n++) {
		sid.section.a = ICONBLOCKS[n].szSection;
		for (int i = 0; i < ICONBLOCKS[n].nItems; i++) {
			sid.pszName = ICONBLOCKS[n].idesc[i].szName;
			sid.description.a = ICONBLOCKS[n].idesc[i].szDesc;
			sid.iDefaultIndex = ICONBLOCKS[n].idesc[i].uId == -IDI_HISTORY ? 0 : ICONBLOCKS[n].idesc[i].uId;        // workaround problem /w icoLib and a resource id of 1 (actually, a Windows problem)

			if (n > 0 && n < 4)
				PluginConfig.g_buttonBarIconHandles[j++] = IcoLib_AddIcon(&sid);
			else
				IcoLib_AddIcon(&sid);
		}
	}

	sid.section.a = LPGEN("Message Sessions") "/" LPGEN("Default");
	sid.pszName = "tabSRMM_clock_symbol";
	sid.description.a = LPGEN("Clock symbol (for the info panel clock)");
	sid.iDefaultIndex = -IDI_CLOCK;
	IcoLib_AddIcon(&sid);

	wcsncpy(szFilename, L"plugins\\tabsrmm.dll", MAX_PATH);

	sid.pszName = "tabSRMM_overlay_disabled";
	sid.description.a = LPGEN("Feature disabled (used as overlay)");
	sid.iDefaultIndex = -IDI_FEATURE_DISABLED;
	IcoLib_AddIcon(&sid);

	sid.pszName = "tabSRMM_overlay_enabled";
	sid.description.a = LPGEN("Feature enabled (used as overlay)");
	sid.iDefaultIndex = -IDI_FEATURE_ENABLED;
	IcoLib_AddIcon(&sid);

	sid.section.a = LPGEN("Message Sessions") "/" LPGEN("Popups");
	sid.pszName = "tabSRMM_popups_disabled";
	sid.description.a = LPGEN("Enable typing notification");
	sid.iDefaultIndex = -IDI_DISABLED;
	IcoLib_AddIcon(&sid);

	sid.pszName = "tabSRMM_popups_enabled";
	sid.description.a = LPGEN("Disable typing notification");
	sid.iDefaultIndex = -IDI_ENABLED;
	IcoLib_AddIcon(&sid);

	return 1;
}

// load the icon theme from IconLib - check if it exists...
static int TSAPI LoadFromIconLib()
{
	for (int n = 0; n < _countof(ICONBLOCKS); n++)
		for (int i = 0; i < ICONBLOCKS[n].nItems; i++)
			*(ICONBLOCKS[n].idesc[i].phIcon) = IcoLib_GetIcon(ICONBLOCKS[n].idesc[i].szName);

	PluginConfig.g_buttonBarIcons[0] = Skin_LoadIcon(SKINICON_OTHER_ADDCONTACT);
	PluginConfig.g_buttonBarIcons[1] = Skin_LoadIcon(SKINICON_OTHER_HISTORY);
	PluginConfig.g_buttonBarIconHandles[0] = Skin_GetIconHandle(SKINICON_OTHER_HISTORY);
	PluginConfig.g_buttonBarIconHandles[1] = Skin_GetIconHandle(SKINICON_OTHER_ADDCONTACT);
	PluginConfig.g_buttonBarIconHandles[20] = Skin_GetIconHandle(SKINICON_OTHER_USERDETAILS);

	PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING] =
		PluginConfig.g_buttonBarIcons[12] = Skin_LoadIcon(SKINICON_OTHER_TYPING);
	PluginConfig.g_IconChecked = Skin_LoadIcon(SKINICON_OTHER_TICK);
	PluginConfig.g_IconUnchecked = Skin_LoadIcon(SKINICON_OTHER_NOTICK);
	PluginConfig.g_IconGroupOpen = Skin_LoadIcon(SKINICON_OTHER_GROUPOPEN);
	PluginConfig.g_IconGroupClose = Skin_LoadIcon(SKINICON_OTHER_GROUPSHUT);

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
	for (int n = 0; n < _countof(ICONBLOCKS); n++)
		for (int i = 0; i < ICONBLOCKS[n].nItems; i++)
			if (*(ICONBLOCKS[n].idesc[i].phIcon) != 0) {
				DestroyIcon(*(ICONBLOCKS[n].idesc[i].phIcon));
				*(ICONBLOCKS[n].idesc[i].phIcon) = 0;
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
	CreateImageList(FALSE);
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
	CreateServiceFunction(MS_TABMSG_TRAYSUPPORT, Service_OpenTrayMenu);
	CreateServiceFunction(MS_TABMSG_SLQMGR, CSendLater::svcQMgr);

	SI_InitStatusIcons();
}

int LoadSendRecvMessageModule(void)
{
	if (FIF == 0) {
		MessageBox(0, TranslateT("The image service plugin (AdvaImg) is not properly installed.\n\nTabSRMM is disabled."), TranslateT("TabSRMM fatal error"), MB_OK | MB_ICONERROR);
		return 1;
	}

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	g_hMsftedit = Utils::loadSystemLibrary(L"\\Msftedit.dll");

	Win7Taskbar = new CTaskbarInteract;
	Win7Taskbar->updateMetrics();

	memset(&nen_options, 0, sizeof(nen_options));
	PluginConfig.hUserPrefsWindowList = WindowList_Create();
	sendQueue = new SendQueue;
	Skin = new CSkin;
	sendLater = new CSendLater;

	InitOptions();

	InitAPI();

	PluginConfig.reloadSystemStartup();
	ReloadTabConfig();
	NEN_ReadOptions(&nen_options);

	db_set_b(0, TEMPLATES_MODULE, "setup", 2);
	LoadDefaultTemplates();
	return 0;
}
