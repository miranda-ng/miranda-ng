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
// wraps some parts of Miranda API
// Also, OS dependent stuff (visual styles api etc.)

#include "stdafx.h"

PDTTE  CMimAPI::m_pfnDrawThemeTextEx = nullptr;
DEFICA CMimAPI::m_pfnDwmExtendFrameIntoClientArea = nullptr;
DICE   CMimAPI::m_pfnDwmIsCompositionEnabled = nullptr;
DRT    CMimAPI::m_pfnDwmRegisterThumbnail = nullptr;
BPI    CMimAPI::m_pfnBufferedPaintInit = nullptr;
BPU    CMimAPI::m_pfnBufferedPaintUninit = nullptr;
BBP    CMimAPI::m_pfnBeginBufferedPaint = nullptr;
EBP    CMimAPI::m_pfnEndBufferedPaint = nullptr;
BBW    CMimAPI::m_pfnDwmBlurBehindWindow = nullptr;
DGC    CMimAPI::m_pfnDwmGetColorizationColor = nullptr;
BPSA   CMimAPI::m_pfnBufferedPaintSetAlpha = nullptr;
DWMIIB CMimAPI::m_pfnDwmInvalidateIconicBitmaps = nullptr;
DWMSWA CMimAPI::m_pfnDwmSetWindowAttribute = nullptr;
DWMUT  CMimAPI::m_pfnDwmUpdateThumbnailProperties = nullptr;
DURT   CMimAPI::m_pfnDwmUnregisterThumbnail = nullptr;
DSIT   CMimAPI::m_pfnDwmSetIconicThumbnail = nullptr;
DSILP  CMimAPI::m_pfnDwmSetIconicLivePreviewBitmap = nullptr;
bool   CMimAPI::m_shutDown = 0;
wchar_t  CMimAPI::m_userDir[] = L"\0";

bool   CMimAPI::m_haveBufferedPaint = false;

/////////////////////////////////////////////////////////////////////////////////////////

int CMimAPI::FoldersPathChanged(WPARAM, LPARAM)
{
	return M.foldersPathChanged();
}

void CMimAPI::configureCustomFolders()
{
	m_hDataPath = FoldersRegisterCustomPathW(LPGEN("TabSRMM"), LPGEN("Data path"), const_cast<wchar_t *>(getDataPath()));
	m_hSkinsPath = FoldersRegisterCustomPathW(LPGEN("Skins"), LPGEN("TabSRMM"), const_cast<wchar_t *>(getSkinPath()));
	m_hAvatarsPath = FoldersRegisterCustomPathW(LPGEN("Avatars"), LPGEN("Saved TabSRMM avatars"), const_cast<wchar_t *>(getSavedAvatarPath()));
	m_hChatLogsPath = FoldersRegisterCustomPathW(LPGEN("TabSRMM"), LPGEN("Group chat logs root"), const_cast<wchar_t *>(getChatLogPath()));

	if (m_hDataPath)
		HookEvent(ME_FOLDERS_PATH_CHANGED, CMimAPI::FoldersPathChanged);

	foldersPathChanged();
}

INT_PTR CMimAPI::foldersPathChanged()
{
	wchar_t szTemp[MAX_PATH + 2];

	if (m_hDataPath) {
		szTemp[0] = 0;
		FoldersGetCustomPathW(m_hDataPath, szTemp, MAX_PATH, const_cast<wchar_t *>(getDataPath()));
		wcsncpy_s(m_szProfilePath, szTemp, _TRUNCATE);

		szTemp[0] = 0;
		FoldersGetCustomPathW(m_hSkinsPath, szTemp, MAX_PATH, const_cast<wchar_t *>(getSkinPath()));
		wcsncpy_s(m_szSkinsPath, (MAX_PATH - 1), szTemp, _TRUNCATE);
		Utils::ensureTralingBackslash(m_szSkinsPath);

		szTemp[0] = 0;
		FoldersGetCustomPathW(m_hAvatarsPath, szTemp, MAX_PATH, const_cast<wchar_t *>(getSavedAvatarPath()));
		wcsncpy_s(m_szSavedAvatarsPath, szTemp, _TRUNCATE);

		szTemp[0] = 0;
		FoldersGetCustomPathW(m_hChatLogsPath, szTemp, MAX_PATH, const_cast<wchar_t *>(getChatLogPath()));
		wcsncpy_s(m_szChatLogsPath, (MAX_PATH - 1), szTemp, _TRUNCATE);
		Utils::ensureTralingBackslash(m_szChatLogsPath);
	}

	CreateDirectoryTreeW(m_szProfilePath);
	CreateDirectoryTreeW(m_szSkinsPath);
	CreateDirectoryTreeW(m_szSavedAvatarsPath);

	Skin->extractSkinsAndLogo(true);
	Skin->setupAeroSkins();
	return 0;
}

const wchar_t* CMimAPI::getUserDir()
{
	if (m_userDir[0] == 0) {
		if (ServiceExists(MS_FOLDERS_REGISTER_PATH))
			wcsncpy_s(m_userDir, L"%miranda_userdata%", _TRUNCATE);
		else
			wcsncpy_s(m_userDir, VARSW(L"%miranda_userdata%"), _TRUNCATE);

		Utils::ensureTralingBackslash(m_userDir);
	}
	return m_userDir;
}

void CMimAPI::InitPaths()
{
	const wchar_t *szUserdataDir = getUserDir();

	mir_snwprintf(m_szProfilePath, L"%stabSRMM", szUserdataDir);
	if (ServiceExists(MS_FOLDERS_REGISTER_PATH)) {
		wcsncpy_s(m_szChatLogsPath, L"%miranda_logpath%", _TRUNCATE);
		wcsncpy_s(m_szSkinsPath, L"%miranda_path%\\Skins\\TabSRMM", _TRUNCATE);
	}
	else {
		wcsncpy_s(m_szChatLogsPath, VARSW(L"%miranda_logpath%"), _TRUNCATE);
		wcsncpy_s(m_szSkinsPath, VARSW(L"%miranda_path%\\Skins\\TabSRMM"), _TRUNCATE);
	}

	Utils::ensureTralingBackslash(m_szChatLogsPath);
	replaceStrW(g_Settings.pszLogDir, m_szChatLogsPath);

	Utils::ensureTralingBackslash(m_szSkinsPath);

	mir_snwprintf(m_szSavedAvatarsPath, L"%s\\Saved Contact Pictures", m_szProfilePath);
}

bool CMimAPI::getAeroState()
{
	m_isAero = m_DwmActive = false;
	if (IsWinVerVistaPlus()) {
		BOOL result = FALSE;
		m_DwmActive = (m_pfnDwmIsCompositionEnabled && (m_pfnDwmIsCompositionEnabled(&result) == S_OK) && result);
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
	m_hUxTheme = nullptr;
	m_hDwmApi = nullptr;

	// vista+ DWM API
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
			m_haveBufferedPaint = (m_pfnBeginBufferedPaint != nullptr && m_pfnEndBufferedPaint != nullptr) ? true : false;
			if (m_haveBufferedPaint)
				m_pfnBufferedPaintInit();
		}
	}
	else m_haveBufferedPaint = false;

	switch (GetByte("default_ieview", -1)) {
	case 1:
		db_set_s(0, "SRMM", "Logger", "ieview");
		__fallthrough;

	case 0:
		db_unset(0, SRMSGMOD_T, "default_ieview");
	}

	switch (GetByte("default_hpp", -1)) {
	case 1:
		db_set_s(0, "SRMM", "Logger", "hpp");
		__fallthrough;

	case 0:
		db_unset(0, SRMSGMOD_T, "default_hpp");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// hook subscriber function for incoming message typing events

int CMimAPI::TypingMessage(WPARAM hContact, LPARAM nSecs)
{
	int foundWin = 0, preTyping = 0;
	BOOL fShowOnClist = TRUE;

	auto *pDlg = Srmm_FindDialog(hContact);
	MCONTACT hMeta = db_mc_getMeta(hContact);
	if (hMeta) {
		if (!pDlg)
			pDlg = Srmm_FindDialog(hMeta);
		hContact = hMeta;
	}

	if (pDlg && g_plugin.getByte(SRMSGSET_SHOWTYPING, SRMSGDEFSET_SHOWTYPING))
		preTyping = pDlg->Typing(nSecs);

	if (pDlg && IsWindowVisible(pDlg->GetHwnd()))
		foundWin = MessageWindowOpened(0, pDlg);
	else
		foundWin = 0;

	TContainerData *pContainer = nullptr;
	if (pDlg) {
		pContainer = pDlg->m_pContainer;
		if (pContainer == nullptr) // should never happen
			return 0;
	}

	if (g_plugin.getByte(SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST)) {
		if (!pDlg && !g_plugin.getByte(SRMSGSET_SHOWTYPINGNOWINOPEN, 1))
			fShowOnClist = false;
		if (pDlg && !g_plugin.getByte(SRMSGSET_SHOWTYPINGWINOPEN, 1))
			fShowOnClist = false;
	}
	else fShowOnClist = false;

	if ((!foundWin || !pContainer->cfg.flags.m_bNoSound) && preTyping != (nSecs != 0))
		Skin_PlaySound(nSecs ? "TNStart" : "TNStop");

	if (g_plugin.bPopups) {
		BOOL fShow = false;
		int  iMode = M.GetByte("MTN_PopupMode", 0);

		switch (iMode) {
		case 0:
			fShow = true;
			break;
		case 1:
			if (!foundWin || !(pContainer && pContainer->m_hwndActive == pDlg->GetHwnd() && GetForegroundWindow() == pContainer->m_hwnd))
				fShow = true;
			break;
		case 2:
			if (pDlg == nullptr)
				fShow = true;
			else {
				if (PluginConfig.m_bHideOnClose) {
					TContainerData *pCont = pDlg->m_pContainer;
					if (pCont && pCont->m_bHidden)
						fShow = true;
				}
			}
			break;
		}
		if (fShow)
			TN_TypingMessage(hContact, nSecs);
	}

	if (nSecs) {
		wchar_t szTip[256];
		mir_snwprintf(szTip, TranslateT("%s is typing a message"), Clist_GetContactDisplayName(hContact));
		if (fShowOnClist && g_plugin.getByte("ShowTypingBalloon", 0))
			Clist_TrayNotifyW(nullptr, TranslateT("Typing notification"), szTip, NIIF_INFO, 1000 * 4);

		if (fShowOnClist) {
			g_clistApi.pfnRemoveEvent(hContact, 1);

			CLISTEVENT cle = {};
			cle.hContact = hContact;
			cle.hDbEvent = 1;
			cle.flags = CLEF_ONLYAFEW | CLEF_UNICODE;
			cle.hIcon = PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING];
			cle.pszService = MS_MSG_TYPINGMESSAGE;
			cle.szTooltip.w = szTip;
			g_clistApi.pfnAddEvent(&cle);
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

int CMimAPI::ProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *pAck = (ACKDATA*)lParam;

	if ((pAck != nullptr) && (pAck->type == ACKTYPE_MESSAGE)) {
		int i = 0, iFound = SendQueue::NR_SENDJOBS;
		SendJob *jobs = sendQueue->getJobByIndex(0);
		MCONTACT hMeta = db_mc_getMeta(pAck->hContact);
		for (int j = 0; j < SendQueue::NR_SENDJOBS; j++) {
			SendJob &p = jobs[j];
			if (pAck->hProcess == (HANDLE)p.iSendId && pAck->hContact == p.hContact) {
				CMsgDialog *dat = p.hOwnerWnd ? (CMsgDialog*)GetWindowLongPtr(p.hOwnerWnd, GWLP_USERDATA) : nullptr;
				if (dat == nullptr) {
					sendQueue->ackMessage(nullptr, (WPARAM)MAKELONG(j, i), lParam);
					return 0;
				}
				if (dat->m_hContact == p.hContact || dat->m_hContact == hMeta) {
					iFound = j;
					break;
				}
			}
		}
		if (iFound == SendQueue::NR_SENDJOBS)     // no matching send info found in the queue
			SendLater::processAck(pAck);
		else                                      // try to find the process handle in the list of open send later jobs
			SendMessage(jobs[iFound].hOwnerWnd, HM_EVENTSENT, (WPARAM)MAKELONG(iFound, i), lParam);
	}
	return 0;
}

int CMimAPI::PrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (hContact == 0)
		return 0;

	bool bEnabled = false;
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto) {
		// leave this menu item hidden for chats
		if (!Contact::IsGroupChat(hContact, szProto))
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

int CMimAPI::DispatchNewEvent(WPARAM hContact, LPARAM hDbEvent)
{
	if (hContact) {
		Utils::sendContactMessage(hContact, HM_DBEVENTADDED, hContact, hDbEvent);

		// we're in meta and an event belongs to a sub
		MCONTACT hReal = db_event_getContact(hDbEvent);
		if (hReal != hContact)
			Utils::sendContactMessage(hReal, HM_DBEVENTADDED, hContact, hDbEvent);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Message event added is called when a new message is added to the database
// if no session is open for the contact, this function will determine if and how a new message
// session(tab) must be created.
//
// if a session is already created, it just does nothing and DispatchNewEvent() will take care.

int CMimAPI::MessageEventAdded(WPARAM hContact, LPARAM hDbEvent)
{
	if (hContact == 0)
		return 0;

	DBEVENTINFO dbei = {};
	db_event_get(hDbEvent, &dbei);

	auto *pDlg = Srmm_FindDialog(hContact);
	if (pDlg == nullptr)
		pDlg = Srmm_FindDialog(db_event_getContact(hDbEvent));

	BOOL isCustomEvent = IsCustomEvent(dbei.eventType);
	BOOL isShownCustomEvent = DbEventIsForMsgWindow(&dbei);
	if (dbei.markedRead() || (isCustomEvent && !isShownCustomEvent))
		return 0;

	g_clistApi.pfnRemoveEvent(hContact, 1);

	bool bAutoPopup = M.GetBool(SRMSGSET_AUTOPOPUP, SRMSGDEFSET_AUTOPOPUP);
	bool bAutoCreate = M.GetBool("autotabs", true);
	bool bAutoContainer = M.GetBool("autocontainer", true);

	if (pDlg) {
		TContainerData *pTargetContainer = pDlg->m_pContainer;
		if (pTargetContainer == nullptr || !PluginConfig.m_bHideOnClose || IsWindowVisible(pTargetContainer->m_hwnd))
			return 0;

		WINDOWPLACEMENT wp = { 0 };
		wp.length = sizeof(wp);
		GetWindowPlacement(pTargetContainer->m_hwnd, &wp);

		wchar_t szName[CONTAINER_NAMELEN + 1];
		GetContainerNameForContact(hContact, szName, CONTAINER_NAMELEN);

		if (bAutoPopup || bAutoCreate) {
			if (bAutoPopup) {
				if (wp.showCmd == SW_SHOWMAXIMIZED)
					ShowWindow(pTargetContainer->m_hwnd, SW_SHOWMAXIMIZED);
				else
					ShowWindow(pTargetContainer->m_hwnd, SW_SHOWNOACTIVATE);
				return 0;
			}

			TContainerData *pContainer = FindContainerByName(szName);
			if (pContainer != nullptr) {
				if (bAutoContainer) {
					ShowWindow(pTargetContainer->m_hwnd, SW_SHOWMINNOACTIVE);
					return 0;
				}
				goto nowindowcreate;
			}
			else if (bAutoContainer) {
				ShowWindow(pTargetContainer->m_hwnd, SW_SHOWMINNOACTIVE);
				return 0;
			}
		}
	}
	else {
		switch (dbei.eventType) {
		case EVENTTYPE_AUTHREQUEST:
		case EVENTTYPE_ADDED:
		case EVENTTYPE_FILE:
			return 0;
		}
	}

	if (!NEN::bNoSounds)
		Skin_PlaySound("AlertMsg");

	if (NEN::bNoAutoPopup)
		goto nowindowcreate;

	PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_CREATECONTAINER, hContact, hDbEvent);
	return 0;

nowindowcreate:
	// for tray support, we add the event to the tray menu. otherwise we send it back to
	// the contact list for flashing
	if (!(dbei.flags & DBEF_READ)) {
		AddUnreadContact(hContact);

		wchar_t toolTip[256];
		mir_snwprintf(toolTip, TranslateT("Message from %s"), Clist_GetContactDisplayName(hContact));

		CLISTEVENT cle = {};
		cle.hContact = hContact;
		cle.hDbEvent = hDbEvent;
		cle.flags = CLEF_UNICODE;
		cle.hIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
		cle.pszService = MS_MSG_READMESSAGE;
		cle.szTooltip.w = toolTip;
		g_clistApi.pfnAddEvent(&cle);
	}
	return 0;
}

CMimAPI M;
