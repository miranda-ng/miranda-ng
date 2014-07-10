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
 * wraps some parts of Miranda API
 * Also, OS dependent stuff (visual styles api etc.)
 *
 */

#include "commonheaders.h"

PDTTE  CMimAPI::m_pfnDrawThemeTextEx = 0;
DEFICA CMimAPI::m_pfnDwmExtendFrameIntoClientArea = 0;
DICE   CMimAPI::m_pfnDwmIsCompositionEnabled = 0;
DRT    CMimAPI::m_pfnDwmRegisterThumbnail = 0;
BPI    CMimAPI::m_pfnBufferedPaintInit = 0;
BPU    CMimAPI::m_pfnBufferedPaintUninit = 0;
BBP    CMimAPI::m_pfnBeginBufferedPaint = 0;
EBP    CMimAPI::m_pfnEndBufferedPaint = 0;
BBW    CMimAPI::m_pfnDwmBlurBehindWindow = 0;
DGC    CMimAPI::m_pfnDwmGetColorizationColor = 0;
BPSA   CMimAPI::m_pfnBufferedPaintSetAlpha = 0;
DWMIIB CMimAPI::m_pfnDwmInvalidateIconicBitmaps = 0;
DWMSWA CMimAPI::m_pfnDwmSetWindowAttribute = 0;
DWMUT  CMimAPI::m_pfnDwmUpdateThumbnailProperties = 0;
DURT   CMimAPI::m_pfnDwmUnregisterThumbnail = 0;
DSIT   CMimAPI::m_pfnDwmSetIconicThumbnail = 0;
DSILP  CMimAPI::m_pfnDwmSetIconicLivePreviewBitmap = 0;
bool   CMimAPI::m_shutDown = 0;
TCHAR  CMimAPI::m_userDir[] = _T("\0");

bool   CMimAPI::m_haveBufferedPaint = false;

/////////////////////////////////////////////////////////////////////////////////////////
// window list functions

void CMimAPI::BroadcastMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
	WindowList_Broadcast(m_hMessageWindowList, msg, wParam, lParam);
}

void CMimAPI::BroadcastMessageAsync(UINT msg, WPARAM wParam, LPARAM lParam)
{
	WindowList_BroadcastAsync(m_hMessageWindowList, msg, wParam, lParam);
}

HWND CMimAPI::FindWindow(MCONTACT h) const
{
	return WindowList_Find(m_hMessageWindowList, h);
}

INT_PTR CMimAPI::AddWindow(HWND hWnd, MCONTACT h)
{
	return WindowList_Add(m_hMessageWindowList, hWnd, h);
}

INT_PTR CMimAPI::RemoveWindow(HWND hWnd)
{
	return WindowList_Remove(m_hMessageWindowList, hWnd);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMimAPI::FoldersPathChanged(WPARAM wParam, LPARAM lParam)
{
	return M.foldersPathChanged();
}

void CMimAPI::configureCustomFolders()
{
	m_hDataPath = FoldersRegisterCustomPathT(LPGEN("TabSRMM"), LPGEN("Data path"), const_cast<TCHAR *>(getDataPath()));
	m_hSkinsPath = FoldersRegisterCustomPathT(LPGEN("Skins"), LPGEN("TabSRMM"), const_cast<TCHAR *>(getSkinPath()));
	m_hAvatarsPath = FoldersRegisterCustomPathT(LPGEN("Avatars"), LPGEN("Saved TabSRMM avatars"), const_cast<TCHAR *>(getSavedAvatarPath()));
	m_hChatLogsPath = FoldersRegisterCustomPathT(LPGEN("TabSRMM"), LPGEN("Group chat logs root"), const_cast<TCHAR *>(getChatLogPath()));

	if (m_hDataPath)
		HookEvent(ME_FOLDERS_PATH_CHANGED, CMimAPI::FoldersPathChanged);

	foldersPathChanged();
}

INT_PTR CMimAPI::foldersPathChanged()
{
	TCHAR szTemp[MAX_PATH + 2] = {'\0'};

	if (m_hDataPath) {
		FoldersGetCustomPathT(m_hDataPath, szTemp, MAX_PATH, const_cast<TCHAR *>(getDataPath()));
		mir_sntprintf(m_szProfilePath, MAX_PATH, _T("%s"), szTemp);

		FoldersGetCustomPathT(m_hSkinsPath, szTemp, MAX_PATH, const_cast<TCHAR *>(getSkinPath()));
		mir_sntprintf(m_szSkinsPath, MAX_PATH - 1, _T("%s"), szTemp);
		Utils::ensureTralingBackslash(m_szSkinsPath);

		FoldersGetCustomPathT(m_hAvatarsPath, szTemp, MAX_PATH, const_cast<TCHAR *>(getSavedAvatarPath()));
		mir_sntprintf(m_szSavedAvatarsPath, MAX_PATH, _T("%s"), szTemp);

		FoldersGetCustomPathT(m_hChatLogsPath, szTemp, MAX_PATH, const_cast<TCHAR *>(getChatLogPath()));
		mir_sntprintf(m_szChatLogsPath, MAX_PATH, _T("%s"), szTemp);
		Utils::ensureTralingBackslash(m_szChatLogsPath);
	}

	CreateDirectoryTreeT(m_szProfilePath);
	CreateDirectoryTreeT(m_szSkinsPath);
	CreateDirectoryTreeT(m_szSavedAvatarsPath);

	Skin->extractSkinsAndLogo(true);
	Skin->setupAeroSkins();
	return 0;
}

const TCHAR* CMimAPI::getUserDir()
{
	if (m_userDir[0] == 0) {
		if (ServiceExists(MS_FOLDERS_REGISTER_PATH))
			lstrcpyn(m_userDir, L"%miranda_userdata%", SIZEOF(m_userDir));
		else
			lstrcpyn(m_userDir, VARST(_T("%miranda_userdata%")), SIZEOF(m_userDir));

		Utils::ensureTralingBackslash(m_userDir);
	}
	return m_userDir;
}

void CMimAPI::InitPaths()
{
	m_szProfilePath[0] = 0;
	m_szSkinsPath[0] = 0;
	m_szSavedAvatarsPath[0] = 0;

	const TCHAR *szUserdataDir = getUserDir();

	mir_sntprintf(m_szProfilePath, MAX_PATH, _T("%stabSRMM"), szUserdataDir);
	if (ServiceExists(MS_FOLDERS_REGISTER_PATH)) {
		lstrcpyn(m_szChatLogsPath, _T("%miranda_logpath%"), MAX_PATH);
		lstrcpyn(m_szSkinsPath, _T("%miranda_path%\\Skins\\TabSRMM"), MAX_PATH);
	}
	else {
		lstrcpyn(m_szChatLogsPath, VARST(_T("%miranda_logpath%")), MAX_PATH);
		lstrcpyn(m_szSkinsPath, VARST(_T("%miranda_path%\\Skins\\TabSRMM")), MAX_PATH);
	}

	Utils::ensureTralingBackslash(m_szChatLogsPath);
	replaceStrT(g_Settings.pszLogDir, m_szChatLogsPath);

	Utils::ensureTralingBackslash(m_szSkinsPath);

	mir_sntprintf(m_szSavedAvatarsPath, MAX_PATH, _T("%s\\Saved Contact Pictures"), m_szProfilePath);
}

bool CMimAPI::getAeroState()
{
	BOOL result = FALSE;
	m_isAero = m_DwmActive = false;
	if (IsWinVerVistaPlus()) {
		m_DwmActive = (m_pfnDwmIsCompositionEnabled && (m_pfnDwmIsCompositionEnabled(&result) == S_OK) && result) ? true : false;
		m_isAero = (CSkin::m_skinEnabled == false) && GetByte("useAero", 1) && CSkin::m_fAeroSkinsValid && m_DwmActive;

	}
	m_isVsThemed = IsThemeActive() != 0;
	return m_isAero;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Initialize various Win32 API functions which are not common to all versions of Windows.
// We have to work with functions pointers here.

void CMimAPI::InitAPI()
{
	m_hUxTheme = 0;

	// vista+ DWM API
	m_hDwmApi = 0;
	if (IsWinVerVistaPlus()) {
		m_hDwmApi = Utils::loadSystemLibrary(L"\\dwmapi.dll");
		if (m_hDwmApi) {
			m_pfnDwmExtendFrameIntoClientArea = (DEFICA)GetProcAddress(m_hDwmApi, "DwmExtendFrameIntoClientArea");
			m_pfnDwmIsCompositionEnabled = (DICE)GetProcAddress(m_hDwmApi, "DwmIsCompositionEnabled");
			m_pfnDwmRegisterThumbnail = (DRT)GetProcAddress(m_hDwmApi, "DwmRegisterThumbnail");
			m_pfnDwmBlurBehindWindow = (BBW)GetProcAddress(m_hDwmApi, "DwmEnableBlurBehindWindow");
			m_pfnDwmGetColorizationColor = (DGC)GetProcAddress(m_hDwmApi, "DwmGetColorizationColor");
			m_pfnDwmInvalidateIconicBitmaps = (DWMIIB)GetProcAddress(m_hDwmApi, "DwmInvalidateIconicBitmaps");
			m_pfnDwmSetWindowAttribute = (DWMSWA)GetProcAddress(m_hDwmApi, "DwmSetWindowAttribute");
			m_pfnDwmUpdateThumbnailProperties = (DWMUT)GetProcAddress(m_hDwmApi, "DwmUpdateThumbnailProperties");
			m_pfnDwmUnregisterThumbnail = (DURT)GetProcAddress(m_hDwmApi, "DwmUnregisterThumbnail");
			m_pfnDwmSetIconicThumbnail = (DSIT)GetProcAddress(m_hDwmApi, "DwmSetIconicThumbnail");
			m_pfnDwmSetIconicLivePreviewBitmap = (DSILP)GetProcAddress(m_hDwmApi, "DwmSetIconicLivePreviewBitmap");
		}

		// additional uxtheme APIs (Vista+)
		m_hUxTheme = Utils::loadSystemLibrary(L"\\uxtheme.dll");
		if (m_hUxTheme) {
			m_pfnDrawThemeTextEx = (PDTTE)GetProcAddress(m_hUxTheme, "DrawThemeTextEx");
			m_pfnBeginBufferedPaint = (BBP)GetProcAddress(m_hUxTheme, "BeginBufferedPaint");
			m_pfnEndBufferedPaint = (EBP)GetProcAddress(m_hUxTheme, "EndBufferedPaint");
			m_pfnBufferedPaintInit = (BPI)GetProcAddress(m_hUxTheme, "BufferedPaintInit");
			m_pfnBufferedPaintUninit = (BPU)GetProcAddress(m_hUxTheme, "BufferedPaintUnInit");
			m_pfnBufferedPaintSetAlpha = (BPSA)GetProcAddress(m_hUxTheme, "BufferedPaintSetAlpha");
			m_haveBufferedPaint = (m_pfnBeginBufferedPaint != 0 && m_pfnEndBufferedPaint != 0) ? true : false;
			if (m_haveBufferedPaint)
				m_pfnBufferedPaintInit();
		}
	}
	else m_haveBufferedPaint = false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// hook subscriber function for incoming message typing events

int CMimAPI::TypingMessage(WPARAM hContact, LPARAM mode)
{
	int issplit = 1, foundWin = 0, preTyping = 0;
	BOOL fShowOnClist = TRUE;

	HWND hwnd = M.FindWindow(hContact);
	MCONTACT hMeta = db_mc_getMeta(hContact);
	if (hMeta) {
		if (!hwnd)
			hwnd = M.FindWindow(hMeta);
		hContact = hMeta;
	}

	if (hwnd && M.GetByte(SRMSGMOD, SRMSGSET_SHOWTYPING, SRMSGDEFSET_SHOWTYPING))
		preTyping = SendMessage(hwnd, DM_TYPING, 0, mode);

	if (hwnd && IsWindowVisible(hwnd))
		foundWin = MessageWindowOpened(0, (LPARAM)hwnd);
	else
		foundWin = 0;

	TContainerData *pContainer = NULL;
	if (hwnd) {
		SendMessage(hwnd, DM_QUERYCONTAINER, 0, (LPARAM)&pContainer);
		if (pContainer == NULL) // should never happen
			return 0;
	}

	if (M.GetByte(SRMSGMOD, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST)) {
		if (!hwnd && !M.GetByte(SRMSGMOD, SRMSGSET_SHOWTYPINGNOWINOPEN, 1))
			fShowOnClist = false;
		if (hwnd && !M.GetByte(SRMSGMOD, SRMSGSET_SHOWTYPINGWINOPEN, 1))
			fShowOnClist = false;
	}
	else fShowOnClist = false;

	if ((!foundWin || !(pContainer->dwFlags & CNT_NOSOUND)) && preTyping != (mode != 0))
		SkinPlaySound(mode ? "TNStart" : "TNStop");

	if (M.GetByte(SRMSGMOD, "ShowTypingPopup", 0)) {
		BOOL fShow = false;
		int  iMode = M.GetByte("MTN_PopupMode", 0);

		switch (iMode) {
		case 0:
			fShow = true;
			break;
		case 1:
			if (!foundWin || !(pContainer && pContainer->hwndActive == hwnd && GetForegroundWindow() == pContainer->hwnd))
				fShow = true;
			break;
		case 2:
			if (hwnd == 0)
				fShow = true;
			else {
				if (PluginConfig.m_HideOnClose) {
					TContainerData *pContainer = 0;
					SendMessage(hwnd, DM_QUERYCONTAINER, 0, (LPARAM)&pContainer);
					if (pContainer && pContainer->fHidden)
						fShow = true;
				}
			}
			break;
		}
		if (fShow)
			TN_TypingMessage(hContact, mode);
	}

	if (mode) {
		TCHAR szTip[256];
		mir_sntprintf(szTip, SIZEOF(szTip), TranslateT("%s is typing a message"), pcli->pfnGetContactDisplayName(hContact, 0));
		if (fShowOnClist && ServiceExists(MS_CLIST_SYSTRAY_NOTIFY) && M.GetByte(SRMSGMOD, "ShowTypingBalloon", 0)) {
			MIRANDASYSTRAYNOTIFY tn;
			tn.szProto = NULL;
			tn.cbSize = sizeof(tn);
			tn.tszInfoTitle = TranslateT("Typing Notification");
			tn.tszInfo = szTip;
			tn.dwInfoFlags = NIIF_INFO | NIIF_INTERN_UNICODE;
			tn.uTimeout = 1000 * 4;
			CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM)&tn);
		}
		if (fShowOnClist) {
			CLISTEVENT cle = { sizeof(cle) };
			cle.hContact = hContact;
			cle.hDbEvent = (HANDLE)1;
			cle.flags = CLEF_ONLYAFEW | CLEF_TCHAR;
			cle.hIcon = PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING];
			cle.pszService = "SRMsg/TypingMessage";
			cle.ptszTooltip = szTip;
			CallServiceSync(MS_CLIST_REMOVEEVENT, hContact, 1);
			CallServiceSync(MS_CLIST_ADDEVENT, hContact, (LPARAM)&cle);
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// this is the global ack dispatcher.It handles both ACKTYPE_MESSAGE and ACKTYPE_AVATAR events
// for ACKTYPE_MESSAGE it searches the corresponding send job in the queue and, if found, dispatches
// it to the owners window
//
// ACKTYPE_AVATAR no longer handled here, because we have avs services now.

int CMimAPI::ProtoAck(WPARAM wParam, LPARAM lParam)
{
	ACKDATA *pAck = (ACKDATA*)lParam;
	if (lParam == 0)
		return 0;

	HWND hwndDlg = 0;
	int i = 0, iFound = SendQueue::NR_SENDJOBS;
	SendJob *jobs = sendQueue->getJobByIndex(0);

	if (pAck->type == ACKTYPE_MESSAGE) {
		MCONTACT hMeta = db_mc_getMeta(pAck->hContact);
		for (int j = 0; j < SendQueue::NR_SENDJOBS; j++) {
			SendJob &p = jobs[j];
			if (pAck->hProcess == p.hSendId && pAck->hContact == p.hContact) {
				TWindowData *dat = p.hOwnerWnd ? (TWindowData*)GetWindowLongPtr(p.hOwnerWnd, GWLP_USERDATA) : NULL;
				if (dat == NULL) {
					sendQueue->ackMessage(NULL, (WPARAM)MAKELONG(j, i), lParam);
					return 0;
				}
				if (dat->hContact == p.hContact || dat->hContact == hMeta) {
					iFound = j;
					break;
				}
			}
			if (iFound != SendQueue::NR_SENDJOBS)  // no mathing entry found in this queue entry.. continue
				break;
		}
		if (iFound == SendQueue::NR_SENDJOBS)     // no matching send info found in the queue
			sendLater->processAck(pAck);
		else                                      // try to find the process handle in the list of open send later jobs
			SendMessage(jobs[iFound].hOwnerWnd, HM_EVENTSENT, (WPARAM)MAKELONG(iFound, i), lParam);
	}
	return 0;
}

int CMimAPI::PrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	if (hContact == NULL)
		return NULL;

	bool bEnabled = false;
	char *szProto = GetContactProto(hContact);
	if (szProto) {
		// leave this menu item hidden for chats
		if (!db_get_b(hContact, szProto, "ChatRoom", 0))
			if (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND)
				bEnabled = true;
	}

	Menu_ShowItem(PluginConfig.m_hMenuItem, bEnabled);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// this handler is called first in the message window chain - it will handle events for which a message window
// is already open. if not, it will do nothing and the 2nd handler(MessageEventAdded) will perform all
// the needed actions.
//
// this handler POSTs the event to the message window procedure - so it is fast and can exit quickly which will
// improve the overall responsiveness when receiving messages.

int CMimAPI::DispatchNewEvent(WPARAM hContact, LPARAM lParam)
{
	if (hContact) {
		Utils::sendContactMessage(hContact, HM_DBEVENTADDED, hContact, lParam);

		// we're in meta and an event belongs to a sub
		MCONTACT hReal = db_event_getContact(HANDLE(lParam));
		if (hReal != hContact)
			Utils::sendContactMessage(hReal, HM_DBEVENTADDED, hContact, lParam);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Message event added is called when a new message is added to the database
// if no session is open for the contact, this function will determine if and how a new message
// session(tab) must be created.
//
// if a session is already created, it just does nothing and DispatchNewEvent() will take care.

int CMimAPI::MessageEventAdded(WPARAM hContact, LPARAM lParam)
{
	TCHAR szName[CONTAINER_NAMELEN + 1];

	HANDLE hDbEvent = (HANDLE)lParam;
	DBEVENTINFO dbei = { sizeof(dbei) };
	db_event_get(hDbEvent, &dbei);

	HWND hwnd = M.FindWindow(hContact);
	if (hwnd == NULL)
		hwnd = M.FindWindow(db_event_getContact(hDbEvent));

	BOOL isCustomEvent = IsCustomEvent(dbei.eventType);
	BOOL isShownCustomEvent = DbEventIsForMsgWindow(&dbei);
	if (dbei.markedRead() || (isCustomEvent && !isShownCustomEvent))
		return 0;

	CallServiceSync(MS_CLIST_REMOVEEVENT, hContact, 1);

	bool bAllowAutoCreate = false;
	bool bAutoPopup = M.GetByte(SRMSGSET_AUTOPOPUP, SRMSGDEFSET_AUTOPOPUP) != 0;
	bool bAutoCreate = M.GetByte("autotabs", 1) != 0;
	bool bAutoContainer = M.GetByte("autocontainer", 1) != 0;
	DWORD dwStatusMask = M.GetDword("autopopupmask", -1);

	if (hwnd) {
		TContainerData *pTargetContainer = 0;
		SendMessage(hwnd, DM_QUERYCONTAINER, 0, (LPARAM)&pTargetContainer);
		if (pTargetContainer == NULL || !PluginConfig.m_HideOnClose || IsWindowVisible(pTargetContainer->hwnd))
			return 0;

		WINDOWPLACEMENT wp = { 0 };
		wp.length = sizeof(wp);
		GetWindowPlacement(pTargetContainer->hwnd, &wp);
		GetContainerNameForContact(hContact, szName, CONTAINER_NAMELEN);

		if (bAutoPopup || bAutoCreate) {
			if (bAutoPopup) {
				if (wp.showCmd == SW_SHOWMAXIMIZED)
					ShowWindow(pTargetContainer->hwnd, SW_SHOWMAXIMIZED);
				else
					ShowWindow(pTargetContainer->hwnd, SW_SHOWNOACTIVATE);
				return 0;
			}

			TContainerData *pContainer = FindContainerByName(szName);
			if (pContainer != NULL) {
				if (bAutoContainer) {
					ShowWindow(pTargetContainer->hwnd, SW_SHOWMINNOACTIVE);
					return 0;
				}
				goto nowindowcreate;
			}
			else if (bAutoContainer) {
				ShowWindow(pTargetContainer->hwnd, SW_SHOWMINNOACTIVE);
				return 0;
			}
		}
	}
	else {
		switch (dbei.eventType) {
		case EVENTTYPE_AUTHREQUEST:
		case EVENTTYPE_ADDED:
			return 0;

		case EVENTTYPE_FILE:
			tabSRMM_ShowPopup(hContact, hDbEvent, dbei.eventType, 0, 0, 0, dbei.szModule, 0);
			return 0;
		}
	}

	// if no window is open, we are not interested in anything else but unread message events
	// new message 
	if (!nen_options.iNoSounds)
		SkinPlaySound("AlertMsg");

	if (nen_options.iNoAutoPopup)
		goto nowindowcreate;

	GetContainerNameForContact(hContact, szName, CONTAINER_NAMELEN);

	if (dwStatusMask == -1)
		bAllowAutoCreate = true;
	else {
		char *szProto = GetContactProto(hContact);
		if (szProto && !strcmp(szProto, META_PROTO))
			szProto = GetContactProto(db_mc_getSrmmSub(hContact));

		if (szProto) {
			DWORD dwStatus = (DWORD)CallProtoService(szProto, PS_GETSTATUS, 0, 0);
			if (dwStatus == 0 || dwStatus <= ID_STATUS_OFFLINE || ((1 << (dwStatus - ID_STATUS_ONLINE)) & dwStatusMask))           // should never happen, but...
				bAllowAutoCreate = true;
		}
	}

	if (bAllowAutoCreate && (bAutoPopup || bAutoCreate)) {
		if (bAutoPopup) {
			TContainerData *pContainer = FindContainerByName(szName);
			if (pContainer == NULL)
				pContainer = CreateContainer(szName, FALSE, hContact);
			if (pContainer)
				CreateNewTabForContact(pContainer, hContact, 0, NULL, TRUE, TRUE, FALSE, 0);
			return 0;
		}

		bool bActivate = false, bPopup = M.GetByte("cpopup", 0) != 0;
		TContainerData *pContainer = FindContainerByName(szName);
		if (pContainer != NULL) {
			if (M.GetByte("limittabs", 0) && !wcsncmp(pContainer->szName, L"default", 6)) {
				if ((pContainer = FindMatchingContainer(L"default", hContact)) != NULL) {
					CreateNewTabForContact(pContainer, hContact, 0, NULL, bActivate, bPopup, TRUE, hDbEvent);
					return 0;
				}
			}
			else {
				CreateNewTabForContact(pContainer, hContact, 0, NULL, bActivate, bPopup, TRUE, hDbEvent);
				return 0;
			}
		}
		if (bAutoContainer) {
			if ((pContainer = CreateContainer(szName, CNT_CREATEFLAG_MINIMIZED, hContact)) != NULL) { // 2 means create minimized, don't popup...
				CreateNewTabForContact(pContainer, hContact, 0, NULL, bActivate, bPopup, TRUE, hDbEvent);
				SendMessageW(pContainer->hwnd, WM_SIZE, 0, 0);
			}
			return 0;
		}
	}

	// for tray support, we add the event to the tray menu. otherwise we send it back to
	// the contact list for flashing
nowindowcreate:
	if (!(dbei.flags & DBEF_READ)) {
		UpdateTrayMenu(0, 0, dbei.szModule, NULL, hContact, 1);
		if (!nen_options.bTraySupport) {
			TCHAR toolTip[256], *contactName;

			CLISTEVENT cle = { sizeof(cle) };
			cle.hContact = hContact;
			cle.hDbEvent = hDbEvent;
			cle.flags = CLEF_TCHAR;
			cle.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
			cle.pszService = "SRMsg/ReadMessage";
			contactName = pcli->pfnGetContactDisplayName(hContact, 0);
			mir_sntprintf(toolTip, SIZEOF(toolTip), TranslateT("Message from %s"), contactName);
			cle.ptszTooltip = toolTip;
			CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
		}
		tabSRMM_ShowPopup(hContact, hDbEvent, dbei.eventType, 0, 0, 0, dbei.szModule, 0);
	}
	return 0;
}

CMimAPI M;
FI_INTERFACE *FIF = 0;
