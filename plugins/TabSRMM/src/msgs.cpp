/*
 * Miranda NG: the free IM client for Microsoft* Windows*
 *
 * Copyright (c) 2000-09 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * Load, setup and shutdown the plugin
 * core plugin messaging services (single IM chats only).
 *
 */

#include "commonheaders.h"

#define IDI_CORE_LOAD	132					// icon id for the "connecting" icon

REOLECallback *mREOLECallback;
NEN_OPTIONS    nen_options;
static HANDLE  hUserPrefsWindowLis = 0;
HMODULE        g_hIconDLL = 0;

static void UnloadIcons();

void Chat_AddIcons(void);

// fired event when user changes IEView plugin options. Apply them to all open tabs

int IEViewOptionsChanged(WPARAM, LPARAM)
{
	M.BroadcastMessage(DM_IEVIEWOPTIONSCHANGED, 0, 0);
	return 0;
}

// fired event when user changes smileyadd options. Notify all open tabs about the changes

int SmileyAddOptionsChanged(WPARAM,LPARAM)
{
	M.BroadcastMessage(DM_SMILEYOPTIONSCHANGED, 0, 0);
	pci->SM_BroadcastMessage(NULL, DM_SMILEYOPTIONSCHANGED, 0, 0, FALSE);
	return 0;
}

// Message API 0.0.0.3 services

static INT_PTR GetWindowClass(WPARAM wParam, LPARAM lParam)
{
	char *szBuf = (char*)wParam;
	int size = (int)lParam;
	mir_snprintf(szBuf, size, "tabSRMM");
	return 0;
}

//wparam = (MessageWindowInputData*)
//lparam = (MessageWindowData*)
//returns 0 on success and returns non-zero (1) on error or if no window data exists for that hcontact

static INT_PTR GetWindowData(WPARAM wParam, LPARAM lParam)
{
	MessageWindowInputData *mwid = (MessageWindowInputData*)wParam;
	if (mwid == NULL || (mwid->cbSize != sizeof(MessageWindowInputData)) || (mwid->hContact == NULL) || (mwid->uFlags != MSG_WINDOW_UFLAG_MSG_BOTH)) 
		return 1; 

	MessageWindowData *mwd = (MessageWindowData*)lParam;
	if(mwd == NULL || (mwd->cbSize != sizeof(MessageWindowData)))
		return 1;

	HWND hwnd = M.FindWindow(mwid->hContact);
	if (hwnd) {
		mwd->uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
		mwd->hwndWindow = hwnd;
		mwd->local = GetParent(GetParent(hwnd));
		SendMessage(hwnd, DM_GETWINDOWSTATE, 0, 0);
		mwd->uState = GetWindowLongPtr(hwnd, DWLP_MSGRESULT);
		return 0;
	}
	else
	{
		SESSION_INFO *si = SM_FindSessionByHCONTACT(mwid->hContact);
		if (si != NULL && si->hWnd != 0) {
			mwd->uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
			mwd->hwndWindow = si->hWnd;
			mwd->local = GetParent(GetParent(si->hWnd));
			SendMessage(si->hWnd, DM_GETWINDOWSTATE, 0, 0);
			mwd->uState = GetWindowLongPtr(si->hWnd, DWLP_MSGRESULT);
			return 0;
		}
		else {
			mwd->uState = 0;
			mwd->hContact = 0;
			mwd->hwndWindow = 0;
			mwd->uFlags = 0;
		}
	}
	return 1;
}

// service function. Sets a status bar text for a contact
static void SetStatusTextWorker(TWindowData *dat, StatusTextData *st)
{
	if (!dat)
		return;

	// delete old custom data
	if (dat->sbCustom) {
		delete dat->sbCustom;
		dat->sbCustom = NULL;
	}

	if (st != NULL && st->cbSize == sizeof(StatusTextData))
		dat->sbCustom = new StatusTextData(*st);

	UpdateStatusBar(dat);
}

static INT_PTR SetStatusText(WPARAM hContact, LPARAM lParam)
{
	SESSION_INFO *si = SM_FindSessionByHCONTACT(hContact);
	if (si == NULL) {
		HWND hwnd = M.FindWindow(hContact);
		if (hwnd != NULL)
			SetStatusTextWorker((TWindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA), (StatusTextData*)lParam);

		if (hContact = db_mc_getMeta(hContact))
			if (hwnd = M.FindWindow(hContact))
				SetStatusTextWorker((TWindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA), (StatusTextData*)lParam);
	}
	else SetStatusTextWorker(si->dat, (StatusTextData*)lParam);
	
	return 0;
}

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

// service function - open the tray menu from the TTB button

static INT_PTR Service_OpenTrayMenu(WPARAM, LPARAM lParam)
{
	SendMessage(PluginConfig.g_hwndHotkeyHandler, DM_TRAYICONNOTIFY, 101, lParam == 0 ? WM_LBUTTONUP : WM_RBUTTONUP);
	return 0;
}

// service function. retrieves the message window flags for a given hcontact or window
// wParam == hContact of the window to find
// lParam == window handle (set it to 0 if you want search for hcontact, otherwise it
// is directly used as the handle for the target window

static INT_PTR GetMessageWindowFlags(WPARAM wParam, LPARAM lParam)
{
	HWND hwndTarget = (HWND)lParam;
	if (hwndTarget == 0)
		hwndTarget = M.FindWindow(wParam);

	if (hwndTarget == 0)
		return 0;

	TWindowData *dat = (TWindowData*)GetWindowLongPtr(hwndTarget, GWLP_USERDATA);
	return (dat) ? dat->dwFlags : 0;
}

// return the version of the window api supported

static INT_PTR GetWindowAPI(WPARAM, LPARAM)
{
	return PLUGIN_MAKE_VERSION(0, 0, 0, 2);
}

// service function finds a message session
// wParam = contact handle for which we want the window handle
// thanks to bio for the suggestion of this service
// if wParam == 0, then lParam is considered to be a valid window handle and
// the function tests the popup mode of the target container

// returns the hwnd if there is an open window or tab for the given hcontact (wParam),
// or (if lParam was specified) the hwnd if the window exists.
// 0 if there is none (or the popup mode of the target container was configured to "hide"
// the window..

INT_PTR MessageWindowOpened(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd = 0;
	TContainerData *pContainer = NULL;

	if (wParam)
		hwnd = M.FindWindow(wParam);
	else if (lParam)
		hwnd = (HWND)lParam;
	else
		return NULL;

	if (!hwnd)
		return 0;

	SendMessage(hwnd, DM_QUERYCONTAINER, 0, (LPARAM)&pContainer);
	if (pContainer) {
		if (pContainer->dwFlags & CNT_DONTREPORT) {
			if (IsIconic(pContainer->hwnd))
				return 0;
		}
		if (pContainer->dwFlags & CNT_DONTREPORTUNFOCUSED) {
			if (!IsIconic(pContainer->hwnd) && GetForegroundWindow() != pContainer->hwnd && GetActiveWindow() != pContainer->hwnd)
				return 0;
		}
		if (pContainer->dwFlags & CNT_ALWAYSREPORTINACTIVE) {
			if (pContainer->dwFlags & CNT_DONTREPORTFOCUSED)
				return 0;

			return pContainer->hwndActive == hwnd;
		}
	}
	return 1;
}

// ReadMessageCommand is executed whenever the user wants to manually open a window.
// This can happen when double clicking a contact on the clist OR when opening a new
// message (clicking on a popup, clicking the flashing tray icon and so on).

static INT_PTR ReadMessageCommand(WPARAM, LPARAM lParam)
{
	MCONTACT hContact = ((CLISTEVENT *)lParam)->hContact;

	HWND hwndExisting = M.FindWindow(hContact);
	if (hwndExisting != 0)
		SendMessage(hwndExisting, DM_ACTIVATEME, 0, 0);
	else {
		TCHAR szName[CONTAINER_NAMELEN + 1];
		GetContainerNameForContact(hContact, szName, CONTAINER_NAMELEN);
		TContainerData *pContainer = FindContainerByName(szName);
		if (pContainer == NULL)
			pContainer = CreateContainer(szName, FALSE, hContact);
		if (pContainer)
			CreateNewTabForContact(pContainer, hContact, 0, NULL, TRUE, TRUE, FALSE, 0);
	}
	return 0;
}

// the SendMessageCommand() invokes a message session window for the given contact.
// e.g. it is called when user double clicks a contact on the contact list
// it is implemented as a service, so external plugins can use it to open a message window.
// contacts handle must be passed in wParam.

INT_PTR SendMessageCommand_Worker(MCONTACT hContact, LPCSTR pszMsg, bool isWchar)
{
	// make sure that only the main UI thread will handle window creation
	if (GetCurrentThreadId() != PluginConfig.dwThreadID) {
		if (pszMsg) {
			wchar_t *tszText = (isWchar) ? mir_wstrdup((WCHAR*)pszMsg) : mir_a2u(pszMsg);
			PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_SENDMESSAGECOMMANDW, hContact, (LPARAM)tszText);
		}
		else PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_SENDMESSAGECOMMANDW, hContact, 0);
		return 0;
	}

	// does the MCONTACT's protocol support IM messages?
	char *szProto = GetContactProto(hContact);
	if (szProto == NULL)
		return 0; // unknown contact
	if (!CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND)
		return 0;

	HWND hwnd = M.FindWindow(hContact);
	if (hwnd) {
		if (pszMsg) {
			HWND hEdit = GetDlgItem(hwnd, IDC_MESSAGE);
			SendMessage(hEdit, EM_SETSEL, -1, SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0));
			if (isWchar)
				SendMessageW(hEdit, EM_REPLACESEL, FALSE, (LPARAM)pszMsg);
			else
				SendMessageA(hEdit, EM_REPLACESEL, FALSE, (LPARAM)pszMsg);
		}
		SendMessage(hwnd, DM_ACTIVATEME, 0, 0);
	}
	else {
		TCHAR szName[CONTAINER_NAMELEN + 1];
		GetContainerNameForContact(hContact, szName, CONTAINER_NAMELEN);

		TContainerData *pContainer = FindContainerByName(szName);
		if (pContainer == NULL)
			pContainer = CreateContainer(szName, FALSE, hContact);
		if (pContainer)
			CreateNewTabForContact(pContainer, hContact, isWchar, pszMsg, TRUE, TRUE, FALSE, 0);
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
	WindowList_Destroy(M.m_hMessageWindowList);
	WindowList_Destroy(PluginConfig.hUserPrefsWindowList);

	DestroyCursor(PluginConfig.hCurSplitNS);
	DestroyCursor(PluginConfig.hCurHyperlinkHand);
	DestroyCursor(PluginConfig.hCurSplitWE);

	FreeLibrary(GetModuleHandleA("Msftedit.dll"));

	if (g_hIconDLL) {
		FreeLibrary(g_hIconDLL);
		g_hIconDLL = NULL;
	}

	ImageList_RemoveAll(PluginConfig.g_hImageList);
	ImageList_Destroy(PluginConfig.g_hImageList);

	delete Win7Taskbar;
	delete mREOLECallback;

	OleUninitialize();
	DestroyMenu(PluginConfig.g_hMenuContext);
	if (PluginConfig.g_hMenuContainer)
		DestroyMenu(PluginConfig.g_hMenuContainer);
	if (PluginConfig.g_hMenuEncoding)
		DestroyMenu(PluginConfig.g_hMenuEncoding);

	UnloadIcons();
	FreeTabConfig();

	if (Utils::rtf_ctable)
		mir_free(Utils::rtf_ctable);

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
	M.BroadcastMessage(DM_OPTIONSAPPLIED, 0, 0);
	M.BroadcastMessage(DM_UPDATEWINICON, 0, 0);
	return 0;
}

// initialises the internal API, services, events etc...

static void TSAPI InitAPI()
{
	CreateServiceFunction(MS_MSG_SENDMESSAGE, SendMessageCommand);
	CreateServiceFunction(MS_MSG_SENDMESSAGE "W", SendMessageCommand_W);
	CreateServiceFunction(MS_MSG_GETWINDOWAPI, GetWindowAPI);
	CreateServiceFunction(MS_MSG_GETWINDOWCLASS, GetWindowClass);
	CreateServiceFunction(MS_MSG_GETWINDOWDATA, GetWindowData);
	CreateServiceFunction(MS_MSG_SETSTATUSTEXT, SetStatusText);

	CreateServiceFunction("SRMsg/ReadMessage", ReadMessageCommand);
	CreateServiceFunction("SRMsg/TypingMessage", TypingMessageCommand);
	CreateServiceFunction(MS_TABMSG_SETUSERPREFS, SetUserPrefs);
	CreateServiceFunction(MS_TABMSG_TRAYSUPPORT, Service_OpenTrayMenu);
	CreateServiceFunction(MS_TABMSG_SLQMGR, CSendLater::svcQMgr);

	CreateServiceFunction(MS_MSG_MOD_GETWINDOWFLAGS, GetMessageWindowFlags);
	CreateServiceFunction(MS_MSG_MOD_MESSAGEDIALOGOPENED, MessageWindowOpened);

	SI_InitStatusIcons();
	CB_InitCustomButtons();

	// the event API
	PluginConfig.m_event_MsgWin = CreateHookableEvent(ME_MSG_WINDOWEVENT);
	PluginConfig.m_event_MsgPopup = CreateHookableEvent(ME_MSG_WINDOWPOPUP);
	PluginConfig.m_event_WriteEvent = CreateHookableEvent(ME_MSG_PRECREATEEVENT);
}

int LoadSendRecvMessageModule(void)
{
	if (FIF == 0) {
		MessageBox(0, TranslateT("The image service plugin (advaimg.dll) is not properly installed.\n\nTabSRMM is disabled."), TranslateT("TabSRMM fatal error"), MB_OK | MB_ICONERROR);
		return 1;
	}

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	Utils::loadSystemLibrary(L"\\Msftedit.dll");

	OleInitialize(NULL);
	mREOLECallback = new REOLECallback;
	Win7Taskbar = new CTaskbarInteract;
	Win7Taskbar->updateMetrics();

	memset(&nen_options, 0, sizeof(nen_options));
	M.m_hMessageWindowList = WindowList_Create();
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

	BuildCodePageList();
	return 0;
}

STDMETHODIMP REOLECallback::GetNewStorage(LPSTORAGE FAR *lplpstg)
{
	LPLOCKBYTES lpLockBytes = NULL;
	SCODE sc = ::CreateILockBytesOnHGlobal(NULL, TRUE, &lpLockBytes);
	if (sc != S_OK)
		return sc;
	sc = ::StgCreateDocfileOnILockBytes(lpLockBytes, STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_READWRITE, 0, lplpstg);
	if (sc != S_OK)
		lpLockBytes->Release();
	return sc;
}

// tabbed mode support functions...
// (C) by Nightwish
// 
// this function searches and activates the tab belonging to the given hwnd (which is the
// hwnd of a message dialog window)

int TSAPI ActivateExistingTab(TContainerData *pContainer, HWND hwndChild)
{
	TWindowData *dat = (TWindowData*)GetWindowLongPtr(hwndChild, GWLP_USERDATA);	// needed to obtain the hContact for the message window
	if (!dat || !pContainer)
		return FALSE;

	NMHDR nmhdr = { 0 };
	nmhdr.code = TCN_SELCHANGE;
	if (TabCtrl_GetItemCount(GetDlgItem(pContainer->hwnd, IDC_MSGTABS)) > 1 && !(pContainer->dwFlags & CNT_DEFERREDTABSELECT)) {
		TabCtrl_SetCurSel(GetDlgItem(pContainer->hwnd, IDC_MSGTABS), GetTabIndexFromHWND(GetDlgItem(pContainer->hwnd, IDC_MSGTABS), hwndChild));
		SendMessage(pContainer->hwnd, WM_NOTIFY, 0, (LPARAM)&nmhdr);	// just select the tab and let WM_NOTIFY do the rest
	}
	if (dat->bType == SESSIONTYPE_IM)
		SendMessage(pContainer->hwnd, DM_UPDATETITLE, dat->hContact, 0);
	if (IsIconic(pContainer->hwnd)) {
		SendMessage(pContainer->hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		SetForegroundWindow(pContainer->hwnd);
	}
	
	// hide on close feature
	if (!IsWindowVisible(pContainer->hwnd)) {
		WINDOWPLACEMENT wp = { 0 };
		wp.length = sizeof(wp);
		GetWindowPlacement(pContainer->hwnd, &wp);

		// all tabs must re-check the layout on activation because adding a tab while
		// the container was hidden can make this necessary
		BroadCastContainer(pContainer, DM_CHECKSIZE, 0, 0);
		if (wp.showCmd == SW_SHOWMAXIMIZED)
			ShowWindow(pContainer->hwnd, SW_SHOWMAXIMIZED);
		else {
			ShowWindow(pContainer->hwnd, SW_SHOWNA);
			SetForegroundWindow(pContainer->hwnd);
		}
		SendMessage(pContainer->hwndActive, WM_SIZE, 0, 0);			// make sure the active tab resizes its layout properly
	}
	else if (GetForegroundWindow() != pContainer->hwnd)
		SetForegroundWindow(pContainer->hwnd);

	if (dat->bType == SESSIONTYPE_IM)
		SetFocus(GetDlgItem(hwndChild, IDC_MESSAGE));
	return TRUE;
}

// this function creates and activates a new tab within the given container.
// bActivateTab: make the new tab the active one
// bPopupContainer: restore container if it was minimized, otherwise flash it...

HWND TSAPI CreateNewTabForContact(TContainerData *pContainer, MCONTACT hContact, int isSend, const char *pszInitialText, BOOL bActivateTab, BOOL bPopupContainer, BOOL bWantPopup, HANDLE hdbEvent)
{
	DBVARIANT dbv = { 0 };

	if (M.FindWindow(hContact) != 0) {
		_DebugPopup(hContact, _T("Warning: trying to create duplicate window"));
		return 0;
	}

	// if we have a max # of tabs/container set and want to open something in the default container...
	if (hContact != 0 && M.GetByte("limittabs", 0) && !_tcsncmp(pContainer->szName, _T("default"), 6)) {
		if ((pContainer = FindMatchingContainer(_T("default"), hContact)) == NULL) {
			TCHAR szName[CONTAINER_NAMELEN + 1];
			mir_sntprintf(szName, CONTAINER_NAMELEN, _T("default"));
			if ((pContainer = CreateContainer(szName, CNT_CREATEFLAG_CLONED, hContact)) == NULL)
				return 0;
		}
	}

	TNewWindowData newData = { 0 };
	newData.hContact = hContact;
	newData.isWchar = isSend;
	newData.szInitialText = pszInitialText;
	char *szProto = GetContactProto(newData.hContact);

	memset(&newData.item, 0, sizeof(newData.item));

	// obtain various status information about the contact
	TCHAR *contactName = pcli->pfnGetContactDisplayName(newData.hContact, 0);

	// cut nickname if larger than x chars...
	TCHAR newcontactname[128], tabtitle[128];
	if (contactName && mir_tstrlen(contactName) > 0) {
		if (M.GetByte("cuttitle", 0))
			CutContactName(contactName, newcontactname, SIZEOF(newcontactname));
		else {
			mir_tstrncpy(newcontactname, contactName, SIZEOF(newcontactname));
			newcontactname[127] = 0;
		}
		Utils::DoubleAmpersands(newcontactname);
	}
	else mir_tstrncpy(newcontactname, _T("_U_"), SIZEOF(newcontactname));

	WORD wStatus = (szProto == NULL ? ID_STATUS_OFFLINE : db_get_w(newData.hContact, szProto, "Status", ID_STATUS_OFFLINE));
	TCHAR *szStatus = pcli->pfnGetStatusModeDescription(szProto == NULL ? ID_STATUS_OFFLINE : db_get_w(newData.hContact, szProto, "Status", ID_STATUS_OFFLINE), 0);

	if (M.GetByte("tabstatus", 1))
		mir_sntprintf(tabtitle, SIZEOF(tabtitle), _T("%s (%s)  "), newcontactname, szStatus);
	else
		mir_sntprintf(tabtitle, SIZEOF(tabtitle), _T("%s   "), newcontactname);

	newData.item.pszText = tabtitle;
	newData.item.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
	newData.item.iImage = 0;
	newData.item.cchTextMax = SIZEOF(tabtitle);

	HWND hwndTab = GetDlgItem(pContainer->hwnd, IDC_MSGTABS);
	// hide the active tab
	if (pContainer->hwndActive && bActivateTab)
		ShowWindow(pContainer->hwndActive, SW_HIDE);

	int iTabIndex_wanted = M.GetDword(hContact, "tabindex", pContainer->iChilds * 100);
	int iCount = TabCtrl_GetItemCount(hwndTab);
	TCITEM item = {0};

	pContainer->iTabIndex = iCount;
	if (iCount > 0) {
		for (int i = iCount - 1; i >= 0; i--) {
			item.mask = TCIF_PARAM;
			TabCtrl_GetItem(hwndTab, i, &item);
			HWND hwnd = (HWND)item.lParam;
			TWindowData *dat = (TWindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (dat) {
				int relPos = M.GetDword(dat->hContact, "tabindex", i * 100);
				if (iTabIndex_wanted <= relPos)
					pContainer->iTabIndex = i;
			}
		}
	}

	int newItem = TabCtrl_InsertItem(hwndTab, pContainer->iTabIndex, &newData.item);
	SendMessage(hwndTab, EM_REFRESHWITHOUTCLIP, 0, 0);
	if (bActivateTab)
		TabCtrl_SetCurSel(GetDlgItem(pContainer->hwnd, IDC_MSGTABS), newItem);
	newData.iTabID = newItem;
	newData.iTabImage = newData.item.iImage;
	newData.pContainer = pContainer;
	newData.iActivate = (int) bActivateTab;
	pContainer->iChilds++;
	newData.bWantPopup = bWantPopup;
	newData.hdbEvent = hdbEvent;
	HWND hwndNew = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSGSPLITNEW), GetDlgItem(pContainer->hwnd, IDC_MSGTABS), DlgProcMessage, (LPARAM)&newData);

	// switchbar support
	if (pContainer->dwFlags & CNT_SIDEBAR) {
		TWindowData *dat = (TWindowData*)GetWindowLongPtr(hwndNew, GWLP_USERDATA);
		if (dat)
			pContainer->SideBar->addSession(dat, pContainer->iTabIndex);
	}
	SendMessage(pContainer->hwnd, WM_SIZE, 0, 0);

	// if the container is minimized, then pop it up...
	if (IsIconic(pContainer->hwnd)) {
		if (bPopupContainer) {
			SendMessage(pContainer->hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
			SetFocus(pContainer->hwndActive);
		}
		else {
			if (pContainer->dwFlags & CNT_NOFLASH)
				SendMessage(pContainer->hwnd, DM_SETICON, 0, (LPARAM)LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
			else
				FlashContainer(pContainer, 1, 0);
		}
	}

	if (bActivateTab) {
		ActivateExistingTab(pContainer, hwndNew);
		SetFocus(hwndNew);
		RedrawWindow(pContainer->hwnd, NULL, NULL, RDW_ERASENOW);
		UpdateWindow(pContainer->hwnd);
		if (GetForegroundWindow() != pContainer->hwnd && bPopupContainer == TRUE)
			SetForegroundWindow(pContainer->hwnd);
	}
	else if (!IsIconic(pContainer->hwnd) && IsWindowVisible(pContainer->hwnd)) {
		SendMessage(pContainer->hwndActive, WM_SIZE, 0, 0);
		RedrawWindow(pContainer->hwndActive, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
		RedrawWindow(pContainer->hwndActive, NULL, NULL, RDW_ERASENOW | RDW_UPDATENOW);
	}

	if (PluginConfig.m_HideOnClose&&!IsWindowVisible(pContainer->hwnd)) {
		WINDOWPLACEMENT wp = { 0 };
		wp.length = sizeof(wp);
		GetWindowPlacement(pContainer->hwnd, &wp);

		BroadCastContainer(pContainer, DM_CHECKSIZE, 0, 0); // make sure all tabs will re-check layout on activation
		if (wp.showCmd == SW_SHOWMAXIMIZED)
			ShowWindow(pContainer->hwnd, SW_SHOWMAXIMIZED);
		else {
			if (bPopupContainer)
				ShowWindow(pContainer->hwnd, SW_SHOWNORMAL);
			else
				ShowWindow(pContainer->hwnd, SW_SHOWMINNOACTIVE);
		}
		SendMessage(pContainer->hwndActive, WM_SIZE, 0, 0);
	}

	if (PluginConfig.m_bIsWin7 && PluginConfig.m_useAeroPeek && CSkin::m_skinEnabled)
		CWarning::show(CWarning::WARN_AEROPEEK_SKIN, MB_ICONWARNING | MB_OK);

	if (ServiceExists(MS_HPP_EG_EVENT) && ServiceExists(MS_IEVIEW_EVENT) && db_get_b(0, "HistoryPlusPlus", "IEViewAPI", 0))
		if (IDYES == CWarning::show(CWarning::WARN_HPP_APICHECK, MB_ICONWARNING | MB_YESNO))
			db_set_b(0, "HistoryPlusPlus", "IEViewAPI", 0);

	return hwndNew;		// return handle of the new dialog
}

// this is used by the 2nd containermode (limit tabs on default containers).
// it searches a container with "room" for the new tabs or otherwise creates
// a new (cloned) one.

TContainerData* TSAPI FindMatchingContainer(const TCHAR *szName, MCONTACT hContact)
{
	int iMaxTabs = M.GetDword("maxtabs", 0);
	if (iMaxTabs > 0 && M.GetByte("limittabs", 0) && !_tcsncmp(szName, _T("default"), 6)) {
		// search a "default" with less than iMaxTabs opened...
		for (TContainerData *p = pFirstContainer; p; p = p->pNext)
			if (!_tcsncmp(p->szName, _T("default"), 6) && p->iChilds < iMaxTabs)
				return p;

		return NULL;
	}
	return FindContainerByName(szName);
}

// load some global icons.

void TSAPI CreateImageList(BOOL bInitial)
{
	int cxIcon = GetSystemMetrics(SM_CXSMICON);
	int cyIcon = GetSystemMetrics(SM_CYSMICON);

	// the imagelist is now a fake. It is still needed to provide the tab control with a
	// image list handle. This will make sure that the tab control will reserve space for
	// an icon on each tab. This is a blank and empty icon
	if (bInitial) {
		PluginConfig.g_hImageList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 0);
		HICON hIcon = CreateIcon(g_hInst, 16, 16, 1, 4, NULL, NULL);
		ImageList_AddIcon(PluginConfig.g_hImageList, hIcon);
		DestroyIcon(hIcon);
	}

	PluginConfig.g_IconFileEvent = LoadSkinnedIcon(SKINICON_EVENT_FILE);
	PluginConfig.g_IconMsgEvent = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	PluginConfig.g_IconMsgEventBig = LoadSkinnedIconBig(SKINICON_EVENT_MESSAGE);
	if ((HICON)CALLSERVICE_NOTFOUND == PluginConfig.g_IconMsgEventBig)
		PluginConfig.g_IconMsgEventBig = 0;
	PluginConfig.g_IconTypingEventBig = LoadSkinnedIconBig(SKINICON_OTHER_TYPING);
	if ((HICON)CALLSERVICE_NOTFOUND == PluginConfig.g_IconTypingEventBig)
		PluginConfig.g_IconTypingEventBig = 0;
	PluginConfig.g_IconSend = PluginConfig.g_buttonBarIcons[9];
	PluginConfig.g_IconTypingEvent = PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING];
}

int TABSRMM_FireEvent(MCONTACT hContact, HWND hwnd, unsigned int type, unsigned int subType)
{
	if (hContact == NULL || hwnd == NULL)
		return 0;

	TWindowData *dat = (TWindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	BYTE bType = dat ? dat->bType : SESSIONTYPE_IM;

	MessageWindowEventData mwe = { sizeof(mwe) };
	mwe.hContact = hContact;
	mwe.hwndWindow = hwnd;
	mwe.szModule = "tabSRMsgW";
	mwe.uType = type;
	mwe.hwndInput = GetDlgItem(hwnd, bType == SESSIONTYPE_IM ? IDC_MESSAGE : IDC_CHAT_MESSAGE);
	mwe.hwndLog = GetDlgItem(hwnd, bType == SESSIONTYPE_IM ? IDC_LOG : IDC_CHAT_LOG);

	if (type == MSG_WINDOW_EVT_CUSTOM) {
		TABSRMM_SessionInfo se = { sizeof(se) };
		se.evtCode = HIWORD(subType);
		se.hwnd = hwnd;
		se.extraFlags = (unsigned int)(LOWORD(subType));
		se.local = (void*)dat->sendBuffer;
		mwe.local = (void*)&se;
	}

	return NotifyEventHooks(PluginConfig.m_event_MsgWin, 0, (LPARAM)&mwe);
}

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
	{ LPGEN("Message Sessions")"/"LPGEN("Default"), _deficons, SIZEOF(_deficons) },
	{ LPGEN("Message Sessions")"/"LPGEN("Toolbar"), _toolbaricons, SIZEOF(_toolbaricons) },
	{ LPGEN("Message Sessions")"/"LPGEN("Toolbar"), _exttoolbaricons, SIZEOF(_exttoolbaricons) },
	{ LPGEN("Message Sessions")"/"LPGEN("Toolbar"), _chattoolbaricons, SIZEOF(_chattoolbaricons) },
	{ LPGEN("Message Sessions")"/"LPGEN("Message Log"), _logicons, SIZEOF(_logicons) },
	{ LPGEN("Message Sessions")"/"LPGEN("Animated Tray"), _trayIcon, SIZEOF(_trayIcon) }
};

static int GetIconPackVersion(HMODULE hDLL)
{
	char szIDString[256];
	int version = 0;

	if (LoadStringA(hDLL, IDS_IDENTIFY, szIDString, sizeof(szIDString)) == 0)
		version = 0;
	else if (!strcmp(szIDString, "__tabSRMM_ICONPACK 1.0__"))
		version = 1;
	else if (!strcmp(szIDString, "__tabSRMM_ICONPACK 2.0__"))
		version = 2;
	else if (!strcmp(szIDString, "__tabSRMM_ICONPACK 3.0__"))
		version = 3;
	else if (!strcmp(szIDString, "__tabSRMM_ICONPACK 3.5__"))
		version = 4;
	else if (!strcmp(szIDString, "__tabSRMM_ICONPACK 5.0__"))
		version = 5;

	if (version < 5)
		CWarning::show(CWarning::WARN_ICONPACK_VERSION, MB_OK | MB_ICONERROR);
	return version;
}

// setup default icons for the IcoLib service. This needs to be done every time the plugin is loaded
// default icons are taken from the icon pack in either \icons or \plugins

static int TSAPI SetupIconLibConfig()
{
	int j = 2, version = 0;

	TCHAR szFilename[MAX_PATH];
	_tcsncpy(szFilename, _T("icons\\tabsrmm_icons.dll"), MAX_PATH);
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

	SKINICONDESC sid = { sizeof(sid) };
	sid.ptszDefaultFile = szFilename;
	sid.flags = SIDF_PATH_TCHAR;

	for (int n = 0; n < SIZEOF(ICONBLOCKS); n++) {
		sid.pszSection = ICONBLOCKS[n].szSection;
		for (int i = 0; i < ICONBLOCKS[n].nItems; i++) {
			sid.pszName = ICONBLOCKS[n].idesc[i].szName;
			sid.pszDescription = ICONBLOCKS[n].idesc[i].szDesc;
			sid.iDefaultIndex = ICONBLOCKS[n].idesc[i].uId == -IDI_HISTORY ? 0 : ICONBLOCKS[n].idesc[i].uId;        // workaround problem /w icoLib and a resource id of 1 (actually, a Windows problem)

			if (n > 0 && n < 4)
				PluginConfig.g_buttonBarIconHandles[j++] = Skin_AddIcon(&sid);
			else
				Skin_AddIcon(&sid);
		}
	}

	sid.pszSection = LPGEN("Message Sessions")"/"LPGEN("Default");
	sid.pszName = "tabSRMM_clock_symbol";
	sid.pszDescription = LPGEN("Clock symbol (for the info panel clock)");
	sid.iDefaultIndex = -IDI_CLOCK;
	Skin_AddIcon(&sid);

	_tcsncpy(szFilename, _T("plugins\\tabsrmm.dll"), MAX_PATH);

	sid.pszName = "tabSRMM_overlay_disabled";
	sid.pszDescription = LPGEN("Feature disabled (used as overlay)");
	sid.iDefaultIndex = -IDI_FEATURE_DISABLED;
	Skin_AddIcon(&sid);

	sid.pszName = "tabSRMM_overlay_enabled";
	sid.pszDescription = LPGEN("Feature enabled (used as overlay)");
	sid.iDefaultIndex = -IDI_FEATURE_ENABLED;
	Skin_AddIcon(&sid);
	return 1;
}

// load the icon theme from IconLib - check if it exists...

static int TSAPI LoadFromIconLib()
{
	for (int n = 0; n < SIZEOF(ICONBLOCKS); n++)
		for (int i = 0; i < ICONBLOCKS[n].nItems; i++)
			*(ICONBLOCKS[n].idesc[i].phIcon) = Skin_GetIcon(ICONBLOCKS[n].idesc[i].szName);

	PluginConfig.g_buttonBarIcons[0] = LoadSkinnedIcon(SKINICON_OTHER_ADDCONTACT);
	PluginConfig.g_buttonBarIcons[1] = LoadSkinnedIcon(SKINICON_OTHER_HISTORY);
	PluginConfig.g_buttonBarIconHandles[0] = LoadSkinnedIconHandle(SKINICON_OTHER_HISTORY);
	PluginConfig.g_buttonBarIconHandles[1] = LoadSkinnedIconHandle(SKINICON_OTHER_ADDCONTACT);
	PluginConfig.g_buttonBarIconHandles[20] = LoadSkinnedIconHandle(SKINICON_OTHER_USERDETAILS);

	PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING] =
		PluginConfig.g_buttonBarIcons[12] = LoadSkinnedIcon(SKINICON_OTHER_TYPING);
	PluginConfig.g_IconChecked = LoadSkinnedIcon(SKINICON_OTHER_TICK);
	PluginConfig.g_IconUnchecked = LoadSkinnedIcon(SKINICON_OTHER_NOTICK);
	PluginConfig.g_IconFolder = LoadSkinnedIcon(SKINICON_OTHER_GROUPOPEN);

	PluginConfig.g_iconOverlayEnabled = Skin_GetIcon("tabSRMM_overlay_enabled");
	PluginConfig.g_iconOverlayDisabled = Skin_GetIcon("tabSRMM_overlay_disabled");

	PluginConfig.g_iconClock = Skin_GetIcon("tabSRMM_clock_symbol");

	CacheMsgLogIcons();
	M.BroadcastMessage(DM_LOADBUTTONBARICONS, 0, 0);
	return 0;
}

// load icon theme from either icon pack or IcoLib

void TSAPI LoadIconTheme()
{
	if (SetupIconLibConfig() == 0)
		return;
	else
		LoadFromIconLib();

	Skin->setupTabCloseBitmap();
	return;
}

static void UnloadIcons()
{
	for (int n = 0; n < SIZEOF(ICONBLOCKS); n++)
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
