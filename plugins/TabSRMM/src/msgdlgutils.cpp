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
 * Helper functions for the message dialog.
 *
 */

#include "commonheaders.h"

#ifndef SHVIEW_THUMBNAIL
#define SHVIEW_THUMBNAIL 0x702D
#endif

#define EVENTTYPE_NICKNAME_CHANGE       9001
#define EVENTTYPE_STATUSMESSAGE_CHANGE  9002
#define EVENTTYPE_CONTACTLEFTCHANNEL    9004
#define EVENTTYPE_WAT_ANSWER            9602
#define EVENTTYPE_JABBER_CHATSTATES     2000
#define EVENTTYPE_JABBER_PRESENCE       2001

static int g_status_events[] = {
	EVENTTYPE_STATUSCHANGE,
	EVENTTYPE_CONTACTLEFTCHANNEL,
	EVENTTYPE_WAT_ANSWER,
	EVENTTYPE_JABBER_CHATSTATES,
	EVENTTYPE_JABBER_PRESENCE
};

static int g_status_events_size = 0;
#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]))

bool TSAPI IsStatusEvent(int eventType)
{
	if (g_status_events_size == 0)
		g_status_events_size = MAX_REGS(g_status_events);

	for (int i = 0; i < g_status_events_size; i++) {
		if (eventType == g_status_events[i])
			return true;
	}
	return false;
}

bool TSAPI IsCustomEvent(int eventType)
{
	if (eventType == EVENTTYPE_MESSAGE || eventType == EVENTTYPE_URL || eventType == EVENTTYPE_CONTACTS ||
		eventType == EVENTTYPE_ADDED || eventType == EVENTTYPE_AUTHREQUEST || eventType == EVENTTYPE_FILE)
		return false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// reorder tabs within a container. fSavePos indicates whether the new position should 
// be saved to the contacts db record (if so, the container will try to open the tab 
// at the saved position later)

void TSAPI RearrangeTab(HWND hwndDlg, const TWindowData *dat, int iMode, BOOL fSavePos)
{
	if (dat == NULL || !IsWindow(hwndDlg))
		return;

	HWND hwndTab = GetParent(hwndDlg);
	TCHAR oldText[512];

	TCITEM item = { 0 };
	item.mask = TCIF_IMAGE | TCIF_TEXT | TCIF_PARAM;
	item.pszText = oldText;
	item.cchTextMax = 500;
	TabCtrl_GetItem(hwndTab, dat->iTabID, &item);

	int newIndex = LOWORD(iMode);
	if (newIndex >= 0 && newIndex <= TabCtrl_GetItemCount(hwndTab)) {
		TabCtrl_DeleteItem(hwndTab, dat->iTabID);
		item.lParam = (LPARAM)hwndDlg;
		TabCtrl_InsertItem(hwndTab, newIndex, &item);
		BroadCastContainer(dat->pContainer, DM_REFRESHTABINDEX, 0, 0);
		ActivateTabFromHWND(hwndTab, hwndDlg);
		if (fSavePos)
			db_set_dw(dat->hContact, SRMSGMOD_T, "tabindex", newIndex * 100);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// subclassing for the save as file dialog (needed to set it to thumbnail view on Windows 2000
// or later

static UINT_PTR CALLBACK OpenFileSubclass(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);
		break;

	case WM_NOTIFY:
		OPENFILENAMEA *ofn = (OPENFILENAMEA *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		HWND hwndParent = GetParent(hwnd);
		HWND hwndLv = FindWindowEx(hwndParent, NULL, _T("SHELLDLL_DefView"), NULL);

		if (hwndLv != NULL && *((DWORD *)(ofn->lCustData))) {
			SendMessage(hwndLv, WM_COMMAND, SHVIEW_THUMBNAIL, 0);
			*((DWORD *)(ofn->lCustData)) = 0;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// saves a contact picture to disk
// takes hbm (bitmap handle) and bool isOwnPic (1 == save the picture as your own avatar)
// requires AVS and ADVAIMG services (Miranda 0.7+)

static void SaveAvatarToFile(TWindowData *dat, HBITMAP hbm, int isOwnPic)
{
	TCHAR szFinalFilename[MAX_PATH];
	time_t t = time(NULL);
	struct tm *lt = localtime(&t);
	DWORD setView = 1;

	TCHAR szTimestamp[100];
	mir_sntprintf(szTimestamp, SIZEOF(szTimestamp), _T("%04u %02u %02u_%02u%02u"), lt->tm_year + 1900, lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min);

	TCHAR *szProto = mir_a2t(dat->cache->getActiveProto());

	TCHAR szFinalPath[MAX_PATH];
	mir_sntprintf(szFinalPath, SIZEOF(szFinalPath), _T("%s\\%s"), M.getSavedAvatarPath(), szProto);
	mir_free(szProto);

	if (CreateDirectory(szFinalPath, 0) == 0) {
		if (GetLastError() != ERROR_ALREADY_EXISTS) {
			MessageBox(0, TranslateT("Error creating destination directory"),
				TranslateT("Save contact picture"), MB_OK | MB_ICONSTOP);
			return;
		}
	}

	TCHAR szBaseName[MAX_PATH];
	if (isOwnPic)
		mir_sntprintf(szBaseName, MAX_PATH, _T("My Avatar_%s"), szTimestamp);
	else
		mir_sntprintf(szBaseName, MAX_PATH, _T("%s_%s"), dat->cache->getNick(), szTimestamp);

	mir_sntprintf(szFinalFilename, MAX_PATH, _T("%s.png"), szBaseName);

	// do not allow / or \ or % in the filename
	Utils::sanitizeFilename(szFinalFilename);

	TCHAR filter[MAX_PATH];
	mir_sntprintf(filter, SIZEOF(filter), _T("%s%c*.bmp;*.png;*.jpg;*.gif%c%c"), TranslateT("Image files"), 0, 0, 0);

	OPENFILENAME ofn = { 0 };
	ofn.lpstrDefExt = _T("png");
	ofn.lpstrFilter = filter;
	ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLEHOOK;
	ofn.lpfnHook = OpenFileSubclass;
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = szFinalFilename;
	ofn.lpstrInitialDir = szFinalPath;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lCustData = (LPARAM)&setView;
	if (GetSaveFileName(&ofn)) {
		if (PathFileExists(szFinalFilename))
			if (MessageBox(0, TranslateT("The file exists. Do you want to overwrite it?"), TranslateT("Save contact picture"), MB_YESNO | MB_ICONQUESTION) == IDNO)
				return;

		IMGSRVC_INFO ii;
		ii.cbSize = sizeof(ii);
		ii.wszName = szFinalFilename;
		ii.hbm = hbm;
		ii.dwMask = IMGI_HBITMAP;
		ii.fif = FIF_UNKNOWN;			// get the format from the filename extension. png is default.
		CallService(MS_IMG_SAVE, (WPARAM)&ii, IMGL_TCHAR);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// flash a tab icon if mode = true, otherwise restore default icon
// store flashing state into bState

void TSAPI FlashTab(TWindowData *dat, HWND hwndTab, int iTabindex, BOOL *bState, BOOL mode, HICON origImage)
{
	if (mode)
		*bState = !(*bState);
	else
		dat->hTabIcon = origImage;

	TCITEM item = { 0 };
	item.mask = TCIF_IMAGE;
	TabCtrl_SetItem(hwndTab, iTabindex, &item);
	if (dat->pContainer->dwFlags & CNT_SIDEBAR)
		dat->pContainer->SideBar->updateSession(dat);
}

/////////////////////////////////////////////////////////////////////////////////////////
// calculates avatar layouting, based on splitter position to find the optimal size
// for the avatar w/o disturbing the toolbar too much.

void TSAPI CalcDynamicAvatarSize(TWindowData *dat, BITMAP *bminfo)
{
	RECT rc;
	double aspect = 0, newWidth = 0, picAspect = 0;
	double picProjectedWidth = 0;
	BOOL bBottomToolBar = dat->pContainer->dwFlags & CNT_BOTTOMTOOLBAR;
	BOOL bToolBar = dat->pContainer->dwFlags & CNT_HIDETOOLBAR ? 0 : 1;
	bool bInfoPanel = dat->Panel->isActive();
	int	 iSplitOffset = dat->bIsAutosizingInput ? 1 : 0;

	if (PluginConfig.g_FlashAvatarAvail) {
		FLASHAVATAR fa = { 0 };
		fa.cProto = dat->szProto;
		fa.id = 25367;
		fa.hContact = bInfoPanel ? NULL : dat->hContact;
		CallService(MS_FAVATAR_GETINFO, (WPARAM)&fa, 0);
		if (fa.hWindow) {
			bminfo->bmHeight = FAVATAR_HEIGHT;
			bminfo->bmWidth = FAVATAR_WIDTH;
		}
	}
	GetClientRect(dat->hwnd, &rc);

	if (dat->dwFlags & MWF_WASBACKGROUNDCREATE || dat->pContainer->dwFlags & CNT_DEFERREDCONFIGURE || dat->pContainer->dwFlags & CNT_CREATE_MINIMIZED || IsIconic(dat->pContainer->hwnd))
		return;  // at this stage, the layout is not yet ready...

	if (bminfo->bmWidth == 0 || bminfo->bmHeight == 0)
		picAspect = 1.0;
	else
		picAspect = (double)(bminfo->bmWidth / (double)bminfo->bmHeight);
	picProjectedWidth = (double)((dat->dynaSplitter - ((bBottomToolBar && bToolBar) ? DPISCALEX_S(24) : 0) + ((dat->showUIElements != 0) ? DPISCALEX_S(28) : DPISCALEX_S(2)))) * picAspect;

	if ((rc.right - (int)picProjectedWidth) > (dat->iButtonBarReallyNeeds) && !PluginConfig.m_AlwaysFullToolbarWidth && bToolBar)
		dat->iRealAvatarHeight = dat->dynaSplitter + 3 + (dat->showUIElements ? DPISCALEY_S(28) : DPISCALEY_S(2));
	else
		dat->iRealAvatarHeight = dat->dynaSplitter + DPISCALEY_S(6) + DPISCALEY_S(iSplitOffset);

	dat->iRealAvatarHeight -= ((bBottomToolBar&&bToolBar) ? DPISCALEY_S(22) : 0);

	if (PluginConfig.m_LimitStaticAvatarHeight > 0)
		dat->iRealAvatarHeight = min(dat->iRealAvatarHeight, PluginConfig.m_LimitStaticAvatarHeight);

	if (M.GetByte(dat->hContact, "dontscaleavatars", M.GetByte("dontscaleavatars", 0)))
		dat->iRealAvatarHeight = min(bminfo->bmHeight, dat->iRealAvatarHeight);

	if (bminfo->bmHeight != 0)
		aspect = (double)dat->iRealAvatarHeight / (double)bminfo->bmHeight;
	else
		aspect = 1;

	newWidth = (double)bminfo->bmWidth * aspect;
	if (newWidth > (double)(rc.right) * 0.8)
		newWidth = (double)(rc.right) * 0.8;
	dat->pic.cy = dat->iRealAvatarHeight + 2;
	dat->pic.cx = (int)newWidth + 2;
}

int TSAPI MsgWindowUpdateMenu(TWindowData *dat, HMENU submenu, int menuID)
{
	HWND	hwndDlg = dat->hwnd;
	bool 	bInfoPanel = dat->Panel->isActive();

	if (menuID == MENU_TABCONTEXT) {
		SESSION_INFO *si = dat->si;
		int iTabs = TabCtrl_GetItemCount(GetParent(hwndDlg));

		EnableMenuItem(submenu, ID_TABMENU_ATTACHTOCONTAINER, M.GetByte("useclistgroups", 0) || M.GetByte("singlewinmode", 0) ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(submenu, ID_TABMENU_CLEARSAVEDTABPOSITION, (M.GetDword(dat->hContact, "tabindex", -1) != -1) ? MF_ENABLED : MF_GRAYED);
	}
	else if (menuID == MENU_PICMENU) {
		MENUITEMINFO mii = { 0 };
		TCHAR *szText = NULL;
		char  avOverride = (char)M.GetByte(dat->hContact, "hideavatar", -1);
		HMENU visMenu = GetSubMenu(submenu, 0);
		BOOL picValid = bInfoPanel ? (dat->hOwnPic != 0) : (dat->ace && dat->ace->hbmPic && dat->ace->hbmPic != PluginConfig.g_hbmUnknown);
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_STRING;

		EnableMenuItem(submenu, ID_PICMENU_SAVETHISPICTUREAS, MF_BYCOMMAND | (picValid ? MF_ENABLED : MF_GRAYED));

		CheckMenuItem(visMenu, ID_VISIBILITY_DEFAULT, MF_BYCOMMAND | (avOverride == -1 ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(visMenu, ID_VISIBILITY_HIDDENFORTHISCONTACT, MF_BYCOMMAND | (avOverride == 0 ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(visMenu, ID_VISIBILITY_VISIBLEFORTHISCONTACT, MF_BYCOMMAND | (avOverride == 1 ? MF_CHECKED : MF_UNCHECKED));

		CheckMenuItem(submenu, ID_PICMENU_ALWAYSKEEPTHEBUTTONBARATFULLWIDTH, MF_BYCOMMAND | (PluginConfig.m_AlwaysFullToolbarWidth ? MF_CHECKED : MF_UNCHECKED));
		if (!bInfoPanel) {
			EnableMenuItem(submenu, ID_PICMENU_SETTINGS, MF_BYCOMMAND | (ServiceExists(MS_AV_GETAVATARBITMAP) ? MF_ENABLED : MF_GRAYED));
			szText = TranslateT("Contact Picture Settings...");
			EnableMenuItem(submenu, 0, MF_BYPOSITION | MF_ENABLED);
		}
		else {
			EnableMenuItem(submenu, 0, MF_BYPOSITION | MF_GRAYED);
			EnableMenuItem(submenu, ID_PICMENU_SETTINGS, MF_BYCOMMAND | ((ServiceExists(MS_AV_SETMYAVATAR) && CallService(MS_AV_CANSETMYAVATAR, (WPARAM)(dat->cache->getActiveProto()), 0)) ? MF_ENABLED : MF_GRAYED));
			szText = TranslateT("Set Your Avatar...");
		}
		mii.dwTypeData = szText;
		mii.cch = lstrlen(szText) + 1;
		SetMenuItemInfo(submenu, ID_PICMENU_SETTINGS, FALSE, &mii);
	}
	else if (menuID == MENU_PANELPICMENU) {
		HMENU visMenu = GetSubMenu(submenu, 0);
		char  avOverride = (char)M.GetByte(dat->hContact, "hideavatar", -1);

		CheckMenuItem(visMenu, ID_VISIBILITY_DEFAULT, MF_BYCOMMAND | (avOverride == -1 ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(visMenu, ID_VISIBILITY_HIDDENFORTHISCONTACT, MF_BYCOMMAND | (avOverride == 0 ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(visMenu, ID_VISIBILITY_VISIBLEFORTHISCONTACT, MF_BYCOMMAND | (avOverride == 1 ? MF_CHECKED : MF_UNCHECKED));

		EnableMenuItem(submenu, ID_PICMENU_SETTINGS, MF_BYCOMMAND | (ServiceExists(MS_AV_GETAVATARBITMAP) ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(submenu, ID_PANELPICMENU_SAVETHISPICTUREAS, MF_BYCOMMAND | ((ServiceExists(MS_AV_SAVEBITMAP) && dat->ace && dat->ace->hbmPic && dat->ace->hbmPic != PluginConfig.g_hbmUnknown) ? MF_ENABLED : MF_GRAYED));
	}
	return 0;
}

int TSAPI MsgWindowMenuHandler(TWindowData *dat, int selection, int menuId)
{
	if (dat == 0)
		return 0;

	HWND	hwndDlg = dat->hwnd;

	if (menuId == MENU_PICMENU || menuId == MENU_PANELPICMENU || menuId == MENU_TABCONTEXT) {
		switch (selection) {
		case ID_TABMENU_ATTACHTOCONTAINER:
			CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_SELECTCONTAINER), hwndDlg, SelectContainerDlgProc, (LPARAM)hwndDlg);
			return 1;
		case ID_TABMENU_CONTAINEROPTIONS:
			if (dat->pContainer->hWndOptions == 0)
				CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CONTAINEROPTIONS), hwndDlg, DlgProcContainerOptions, (LPARAM)dat->pContainer);
			return 1;
		case ID_TABMENU_CLOSECONTAINER:
			SendMessage(dat->pContainer->hwnd, WM_CLOSE, 0, 0);
			return 1;
		case ID_TABMENU_CLOSETAB:
			SendMessage(hwndDlg, WM_CLOSE, 1, 0);
			return 1;
		case ID_TABMENU_SAVETABPOSITION:
			db_set_dw(dat->hContact, SRMSGMOD_T, "tabindex", dat->iTabID * 100);
			break;
		case ID_TABMENU_CLEARSAVEDTABPOSITION:
			db_unset(dat->hContact, SRMSGMOD_T, "tabindex");
			break;
		case ID_TABMENU_LEAVECHATROOM:
			if (dat && dat->bType == SESSIONTYPE_CHAT) {
				SESSION_INFO *si = dat->si;
				if ((si != NULL) && (dat->hContact != NULL)) {
					char *szProto = GetContactProto(dat->hContact);
					if (szProto)
						CallProtoService(szProto, PS_LEAVECHAT, dat->hContact, 0);
				}
			}
			return 1;

		case ID_VISIBILITY_DEFAULT:
		case ID_VISIBILITY_HIDDENFORTHISCONTACT:
		case ID_VISIBILITY_VISIBLEFORTHISCONTACT:
			{
				BYTE avOverrideMode;
				if (selection == ID_VISIBILITY_DEFAULT)
					avOverrideMode = -1;
				else if (selection == ID_VISIBILITY_HIDDENFORTHISCONTACT)
					avOverrideMode = 0;
				else if (selection == ID_VISIBILITY_VISIBLEFORTHISCONTACT)
					avOverrideMode = 1;
				db_set_b(dat->hContact, SRMSGMOD_T, "hideavatar", avOverrideMode);
			}
			dat->panelWidth = -1;
			ShowPicture(dat, FALSE);
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			DM_ScrollToBottom(dat, 0, 1);
			return 1;

		case ID_PICMENU_ALWAYSKEEPTHEBUTTONBARATFULLWIDTH:
			PluginConfig.m_AlwaysFullToolbarWidth = !PluginConfig.m_AlwaysFullToolbarWidth;
			db_set_b(0, SRMSGMOD_T, "alwaysfulltoolbar", (BYTE)PluginConfig.m_AlwaysFullToolbarWidth);
			M.BroadcastMessage(DM_CONFIGURETOOLBAR, 0, 1);
			break;

		case ID_PICMENU_SAVETHISPICTUREAS:
			if (dat->Panel->isActive()) {
				if (dat)
					SaveAvatarToFile(dat, dat->hOwnPic, 1);
			}
			else {
				if (dat && dat->ace)
					SaveAvatarToFile(dat, dat->ace->hbmPic, 0);
			}
			break;

		case ID_PANELPICMENU_SAVETHISPICTUREAS:
			if (dat && dat->ace)
				SaveAvatarToFile(dat, dat->ace->hbmPic, 0);
			break;

		case ID_PICMENU_SETTINGS:
			if (menuId == MENU_PANELPICMENU)
				CallService(MS_AV_CONTACTOPTIONS, dat->hContact, 0);
			else if (menuId == MENU_PICMENU) {
				if (dat->Panel->isActive()) {
					if (ServiceExists(MS_AV_SETMYAVATAR) && CallService(MS_AV_CANSETMYAVATAR, (WPARAM)(dat->cache->getActiveProto()), 0))
						CallService(MS_AV_SETMYAVATAR, (WPARAM)(dat->cache->getActiveProto()), 0);
				}
				else
					CallService(MS_AV_CONTACTOPTIONS, dat->hContact, 0);
			}
			return 1;
		}
	}
	else if (menuId == MENU_LOGMENU) {
		int iLocalTime = 0;
		int iRtl = (PluginConfig.m_RTLDefault == 0 ? M.GetByte(dat->hContact, "RTL", 0) : M.GetByte(dat->hContact, "RTL", 1));
		int iLogStatus = (PluginConfig.m_LogStatusChanges != 0) && M.GetByte(dat->hContact, "logstatuschanges", 0);

		DWORD dwOldFlags = dat->dwFlags;

		switch (selection) {
		case ID_MESSAGELOGSETTINGS_GLOBAL:
			{
				OPENOPTIONSDIALOG	ood = { sizeof(ood) };
				ood.pszPage = "Message Sessions";
				db_set_b(0, SRMSGMOD_T, "opage", 3);			// force 3th tab to appear
				Options_Open(&ood);
			}
			return 1;

		case ID_MESSAGELOGSETTINGS_FORTHISCONTACT:
			CallService(MS_TABMSG_SETUSERPREFS, dat->hContact, 0);
			return 1;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// update the status bar field which displays the number of characters in the input area
// and various indicators (caps lock, num lock, insert mode).

void TSAPI UpdateReadChars(const TWindowData *dat)
{
	if (dat && (dat->pContainer->hwndStatus && dat->pContainer->hwndActive == dat->hwnd)) {
		TCHAR buf[128];
		int len;

		if (dat->bType == SESSIONTYPE_CHAT)
			len = GetWindowTextLength(GetDlgItem(dat->hwnd, IDC_CHAT_MESSAGE));
		else {
			// retrieve text length in UTF8 bytes, because this is the relevant length for most protocols
			GETTEXTLENGTHEX gtxl = { 0 };
			gtxl.codepage = CP_UTF8;
			gtxl.flags = GTL_DEFAULT | GTL_PRECISE | GTL_NUMBYTES;

			len = SendDlgItemMessage(dat->hwnd, IDC_MESSAGE, EM_GETTEXTLENGTHEX, (WPARAM)&gtxl, 0);
		}

		BOOL fCaps = (GetKeyState(VK_CAPITAL) & 1);
		BOOL fNum = (GetKeyState(VK_NUMLOCK) & 1);

		TCHAR szBuf[20]; szBuf[0] = 0;
		if (dat->fInsertMode)
			lstrcat(szBuf, _T("O"));
		if (fCaps)
			lstrcat(szBuf, _T("C"));
		if (fNum)
			lstrcat(szBuf, _T("N"));
		if (dat->fInsertMode || fCaps || fNum)
			lstrcat(szBuf, _T(" | "));

		mir_sntprintf(buf, SIZEOF(buf), _T("%s%s %d/%d"), szBuf, dat->lcID, dat->iOpenJobs, len);
		SendMessage(dat->pContainer->hwndStatus, SB_SETTEXT, 1, (LPARAM)buf);
		if (PluginConfig.m_visualMessageSizeIndicator)
			InvalidateRect(dat->pContainer->hwndStatus, NULL, FALSE);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// update all status bar fields and force a redraw of the status bar.

void TSAPI UpdateStatusBar(const TWindowData *dat)
{
	if (dat && dat->pContainer->hwndStatus && dat->pContainer->hwndActive == dat->hwnd) {
		if (dat->bType == SESSIONTYPE_IM) {
			if (dat->szStatusBar[0]) {
				SendMessage(dat->pContainer->hwndStatus, SB_SETICON, 0, (LPARAM)PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING]);
				SendMessage(dat->pContainer->hwndStatus, SB_SETTEXT, 0, (LPARAM)dat->szStatusBar);
			}
			else if (dat->sbCustom) {
				SendMessage(dat->pContainer->hwndStatus, SB_SETICON, 0, (LPARAM)dat->sbCustom->hIcon);
				SendMessage(dat->pContainer->hwndStatus, SB_SETTEXT, 0, (LPARAM)dat->sbCustom->tszText);
			}
			else {
				SendMessage(dat->pContainer->hwndStatus, SB_SETICON, 0, 0);
				DM_UpdateLastMessage(dat);
			}
		}
		else {
			if (dat->sbCustom) {
				SendMessage(dat->pContainer->hwndStatus, SB_SETICON, 0, (LPARAM)dat->sbCustom->hIcon);
				SendMessage(dat->pContainer->hwndStatus, SB_SETTEXT, 0, (LPARAM)dat->sbCustom->tszText);
			}
			else SendMessage(dat->pContainer->hwndStatus, SB_SETICON, 0, 0);
		}
		UpdateReadChars(dat);
		InvalidateRect(dat->pContainer->hwndStatus, NULL, TRUE);
		SendMessage(dat->pContainer->hwndStatus, WM_USER + 101, 0, (LPARAM)dat);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// provide user feedback via icons on tabs.Used to indicate "send in progress" or
// any error state.
//
// NOT used for typing notification feedback as this is handled directly from the
// MTN handler.

void TSAPI HandleIconFeedback(TWindowData *dat, HICON iIcon)
{
	TCITEM item = { 0 };
	HICON iOldIcon = dat->hTabIcon;

	if (iIcon == (HICON)-1) { // restore status image
		if (dat->dwFlags & MWF_ERRORSTATE)
			dat->hTabIcon = PluginConfig.g_iconErr;
		else
			dat->hTabIcon = dat->hTabStatusIcon;
	}
	else dat->hTabIcon = iIcon;

	item.iImage = 0;
	item.mask = TCIF_IMAGE;
	if (dat->pContainer->dwFlags & CNT_SIDEBAR)
		dat->pContainer->SideBar->updateSession(dat);
	else
		TabCtrl_SetItem(GetDlgItem(dat->pContainer->hwnd, IDC_MSGTABS), dat->iTabID, &item);
}

/////////////////////////////////////////////////////////////////////////////////////////
// retrieve the visiblity of the avatar window, depending on the global setting
// and local mode

int TSAPI GetAvatarVisibility(HWND hwndDlg, TWindowData *dat)
{
	BYTE bAvatarMode = dat->pContainer->avatarMode;
	BYTE bOwnAvatarMode = dat->pContainer->ownAvatarMode;
	char hideOverride = (char)M.GetByte(dat->hContact, "hideavatar", -1);

	// infopanel visible, consider own avatar display
	dat->showPic = 0;

	if (dat->Panel->isActive() && bAvatarMode != 3) {
		if (bOwnAvatarMode)
			dat->showPic = FALSE;
		else {
			FLASHAVATAR fa = { 0 };
			if (PluginConfig.g_FlashAvatarAvail) {
				fa.cProto = dat->szProto;
				fa.id = 25367;
				fa.hParentWindow = GetDlgItem(hwndDlg, IDC_CONTACTPIC);

				CallService(MS_FAVATAR_MAKE, (WPARAM)&fa, 0);
				if (fa.hWindow != NULL&&dat->hwndContactPic) {
					DestroyWindow(dat->hwndContactPic);
					dat->hwndContactPic = NULL;
				}
				dat->showPic = ((dat->hOwnPic && dat->hOwnPic != PluginConfig.g_hbmUnknown) || (fa.hWindow != NULL)) ? 1 : 0;
			}
			else dat->showPic = (dat->hOwnPic && dat->hOwnPic != PluginConfig.g_hbmUnknown) ? 1 : 0;

			if (!PluginConfig.g_bDisableAniAvatars && fa.hWindow == NULL && !dat->hwndContactPic)
				dat->hwndContactPic = CreateWindowEx(WS_EX_TOPMOST, AVATAR_CONTROL_CLASS, _T(""), WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, GetDlgItem(hwndDlg, IDC_CONTACTPIC), (HMENU)0, NULL, NULL);
		}

		switch (bAvatarMode) {
		case 2:
			dat->showInfoPic = 0;
			break;
		case 0:
			dat->showInfoPic = 1;
		case 1:
			FLASHAVATAR fa = { 0 };
			HBITMAP hbm = ((dat->ace && !(dat->ace->dwFlags & AVS_HIDEONCLIST)) ? dat->ace->hbmPic : 0);

			if (0 == hbm && 0 == bAvatarMode && !PluginConfig.g_bDisableAniAvatars) {
				dat->showInfoPic = 0;
				break;
			}

			if (PluginConfig.g_FlashAvatarAvail) {
				fa.cProto = dat->szProto;
				fa.id = 25367;
				fa.hContact = dat->hContact;
				fa.hParentWindow = dat->hwndPanelPicParent;

				CallService(MS_FAVATAR_MAKE, (WPARAM)&fa, 0);
				if (fa.hWindow != NULL && dat->hwndPanelPic) {
					DestroyWindow(dat->hwndPanelPic);
					dat->hwndPanelPic = NULL;
					ShowWindow(dat->hwndPanelPicParent, SW_SHOW);
					EnableWindow(dat->hwndPanelPicParent, TRUE);
				}
			}
			if (!PluginConfig.g_bDisableAniAvatars && fa.hWindow == NULL && !dat->hwndPanelPic) {
				dat->hwndPanelPic = CreateWindowEx(WS_EX_TOPMOST, AVATAR_CONTROL_CLASS, _T(""), WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, dat->hwndPanelPicParent, (HMENU)7000, NULL, NULL);
				if (dat->hwndPanelPic)
					SendMessage(dat->hwndPanelPic, AVATAR_SETAEROCOMPATDRAWING, 0, TRUE);
			}
			if (bAvatarMode != 0) {
				if ((hbm && hbm != PluginConfig.g_hbmUnknown) || (fa.hWindow != NULL))
					dat->showInfoPic = 1;
				else
					dat->showInfoPic = 0;
			}
			break;
		}

		if (dat->showInfoPic)
			dat->showInfoPic = hideOverride == 0 ? 0 : dat->showInfoPic;
		else
			dat->showInfoPic = hideOverride == 1 ? 1 : dat->showInfoPic;

		// reloads avatars
		if (dat->showInfoPic) {
			// panel and contact is shown, reloads contact's avatar -> panel
			// user avatar -> bottom picture
			Utils::setAvatarContact(dat->hwndPanelPic, dat->hContact);
			if (dat->hwndContactPic)
				SendMessage(dat->hwndContactPic, AVATAR_SETPROTOCOL, 0, (LPARAM)dat->szProto);
		}
		else {
			//show only user picture(contact is unaccessible)
			if (dat->hwndContactPic)
				SendMessage(dat->hwndContactPic, AVATAR_SETPROTOCOL, 0, (LPARAM)dat->szProto);
		}
	}
	else {
		dat->showInfoPic = 0;

		switch (bAvatarMode) {
		case 0: // globally on
			dat->showPic = 1;
			break;
		case 2: // globally OFF
			dat->showPic = 0;
			break;
		case 3: // on, if present
		case 1:
			HBITMAP hbm = (dat->ace && !(dat->ace->dwFlags & AVS_HIDEONCLIST)) ? dat->ace->hbmPic : 0;
			FLASHAVATAR fa = { 0 };

			if (PluginConfig.g_FlashAvatarAvail) {
				fa.cProto = dat->szProto;
				fa.id = 25367;
				fa.hContact = dat->hContact;
				fa.hParentWindow = GetDlgItem(hwndDlg, IDC_CONTACTPIC);
				CallService(MS_FAVATAR_MAKE, (WPARAM)&fa, 0);

				if (fa.hWindow != NULL&&dat->hwndContactPic) {
					DestroyWindow(dat->hwndContactPic);
					dat->hwndContactPic = NULL;
				}
			}
			if (!PluginConfig.g_bDisableAniAvatars && fa.hWindow == NULL && !dat->hwndContactPic)
				dat->hwndContactPic = CreateWindowEx(WS_EX_TOPMOST, AVATAR_CONTROL_CLASS, _T(""), WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, GetDlgItem(hwndDlg, IDC_CONTACTPIC), (HMENU)0, NULL, NULL);

			if ((hbm && hbm != PluginConfig.g_hbmUnknown) || (fa.hWindow != NULL))
				dat->showPic = 1;
			else
				dat->showPic = 0;
			break;
		}

		if (dat->showPic)
			dat->showPic = hideOverride == 0 ? 0 : dat->showPic;
		else
			dat->showPic = hideOverride == 1 ? 1 : dat->showPic;

		// reloads avatars
		if (dat->showPic)
			if (dat->hwndPanelPic) //shows contact or user picture, depending on panel visibility
				SendMessage(dat->hwndContactPic, AVATAR_SETPROTOCOL, 0, (LPARAM)dat->szProto);
		
		if (dat->hwndContactPic)
			Utils::setAvatarContact(dat->hwndContactPic, dat->hContact);
	}
	return dat->showPic;
}

/////////////////////////////////////////////////////////////////////////////////////////
// checks, if there is a valid smileypack installed for the given protocol

int TSAPI CheckValidSmileyPack(const char *szProto, MCONTACT hContact)
{
	if (!PluginConfig.g_SmileyAddAvail)
		return 0;

	SMADD_INFO2 smainfo = { 0 };
	smainfo.cbSize = sizeof(smainfo);
	smainfo.Protocolname = const_cast<char *>(szProto);
	smainfo.hContact = hContact;
	CallService(MS_SMILEYADD_GETINFO2, 0, (LPARAM)&smainfo);
	if (smainfo.ButtonIcon)
		DestroyIcon(smainfo.ButtonIcon);
	return smainfo.NumberOfVisibleSmileys;
}

/////////////////////////////////////////////////////////////////////////////////////////
// return value MUST be mir_free()'d by caller.

TCHAR* TSAPI QuoteText(const TCHAR *text, int charsPerLine, int removeExistingQuotes)
{
	int inChar, outChar, lineChar;
	int justDoneLineBreak, bufSize;
	TCHAR *strout;

	bufSize = lstrlenW(text) + 23;
	strout = (TCHAR*)mir_alloc(bufSize * sizeof(TCHAR));
	inChar = 0;
	justDoneLineBreak = 1;
	for (outChar = 0, lineChar = 0; text[inChar];) {
		if (outChar >= bufSize - 8) {
			bufSize += 20;
			strout = (TCHAR*)mir_realloc(strout, bufSize * sizeof(TCHAR));
		}
		if (justDoneLineBreak && text[inChar] != '\r' && text[inChar] != '\n') {
			if (removeExistingQuotes)
				if (text[inChar] == '>') {
				while (text[++inChar] != '\n');
				inChar++;
				continue;
				}
			strout[outChar++] = '>';
			strout[outChar++] = ' ';
			lineChar = 2;
		}
		if (lineChar == charsPerLine && text[inChar] != '\r' && text[inChar] != '\n') {
			int decreasedBy;
			for (decreasedBy = 0; lineChar > 10; lineChar--, inChar--, outChar--, decreasedBy++)
				if (strout[outChar] == ' ' || strout[outChar] == '\t' || strout[outChar] == '-') break;
			if (lineChar <= 10) {
				lineChar += decreasedBy;
				inChar += decreasedBy;
				outChar += decreasedBy;
			}
			else inChar++;
			strout[outChar++] = '\r';
			strout[outChar++] = '\n';
			justDoneLineBreak = 1;
			continue;
		}
		strout[outChar++] = text[inChar];
		lineChar++;
		if (text[inChar] == '\n' || text[inChar] == '\r') {
			if (text[inChar] == '\r' && text[inChar + 1] != '\n')
				strout[outChar++] = '\n';
			justDoneLineBreak = 1;
			lineChar = 0;
		}
		else justDoneLineBreak = 0;
		inChar++;
	}
	strout[outChar++] = '\r';
	strout[outChar++] = '\n';
	strout[outChar] = 0;
	return strout;
}

void TSAPI AdjustBottomAvatarDisplay(TWindowData *dat)
{
	if (dat) {
		bool bInfoPanel = dat->Panel->isActive();
		HBITMAP hbm = (bInfoPanel && dat->pContainer->avatarMode != 3) ? dat->hOwnPic : (dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown);
		HWND	hwndDlg = dat->hwnd;

		if (PluginConfig.g_FlashAvatarAvail) {
			FLASHAVATAR fa = { 0 };

			fa.hContact = dat->hContact;
			fa.hWindow = 0;
			fa.id = 25367;
			fa.cProto = dat->szProto;
			CallService(MS_FAVATAR_GETINFO, (WPARAM)&fa, 0);
			if (fa.hWindow) {
				dat->hwndFlash = fa.hWindow;
				SetParent(dat->hwndFlash, bInfoPanel ? dat->hwndPanelPicParent : GetDlgItem(dat->hwnd, IDC_CONTACTPIC));
			}
			fa.hContact = 0;
			fa.hWindow = 0;
			if (bInfoPanel) {
				fa.hParentWindow = GetDlgItem(hwndDlg, IDC_CONTACTPIC);
				CallService(MS_FAVATAR_MAKE, (WPARAM)&fa, 0);
				if (fa.hWindow) {
					SetParent(fa.hWindow, GetDlgItem(hwndDlg, IDC_CONTACTPIC));
					ShowWindow(fa.hWindow, SW_SHOW);
				}
			}
			else {
				CallService(MS_FAVATAR_GETINFO, (WPARAM)&fa, 0);
				if (fa.hWindow)
					ShowWindow(fa.hWindow, SW_HIDE);
			}
		}

		if (hbm) {
			dat->showPic = GetAvatarVisibility(hwndDlg, dat);
			if (dat->dynaSplitter == 0 || dat->splitterY == 0)
				LoadSplitter(dat);
			dat->dynaSplitter = dat->splitterY - DPISCALEY_S(34);
			DM_RecalcPictureSize(dat);
			Utils::showDlgControl(hwndDlg, IDC_CONTACTPIC, dat->showPic ? SW_SHOW : SW_HIDE);
			InvalidateRect(GetDlgItem(hwndDlg, IDC_CONTACTPIC), NULL, TRUE);
		}
		else {
			dat->showPic = GetAvatarVisibility(hwndDlg, dat);
			Utils::showDlgControl(hwndDlg, IDC_CONTACTPIC, dat->showPic ? SW_SHOW : SW_HIDE);
			dat->pic.cy = dat->pic.cx = DPISCALEY_S(60);
			InvalidateRect(GetDlgItem(hwndDlg, IDC_CONTACTPIC), NULL, TRUE);
		}
	}
}

void TSAPI ShowPicture(TWindowData *dat, BOOL showNewPic)
{
	DBVARIANT 	dbv = { 0 };
	RECT 		rc;
	HWND		hwndDlg = dat->hwnd;

	if (!dat->Panel->isActive())
		dat->pic.cy = dat->pic.cx = DPISCALEY_S(60);

	if (showNewPic) {
		if (dat->Panel->isActive() && dat->pContainer->avatarMode != 3) {
			if (!dat->hwndPanelPic) {
				dat->panelWidth = -1;
				InvalidateRect(dat->hwnd, NULL, TRUE);
				UpdateWindow(dat->hwnd);
				SendMessage(dat->hwnd, WM_SIZE, 0, 0);
			}
			return;
		}
		AdjustBottomAvatarDisplay(dat);
	}
	else {
		dat->showPic = dat->showPic ? 0 : 1;
		db_set_b(dat->hContact, SRMSGMOD_T, "MOD_ShowPic", (BYTE)dat->showPic);
	}
	GetWindowRect(GetDlgItem(hwndDlg, IDC_CONTACTPIC), &rc);
	if (dat->minEditBoxSize.cy + DPISCALEY_S(3) > dat->splitterY)
		SendMessage(hwndDlg, DM_SPLITTERMOVED, (WPARAM)rc.bottom - dat->minEditBoxSize.cy, (LPARAM)GetDlgItem(hwndDlg, IDC_SPLITTER));
	if (!showNewPic)
		SetDialogToType(hwndDlg);
	else
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
}

void TSAPI FlashOnClist(HWND hwndDlg, TWindowData *dat, HANDLE hEvent, DBEVENTINFO *dbei)
{
	dat->dwTickLastEvent = GetTickCount();

	if ((GetForegroundWindow() != dat->pContainer->hwnd || dat->pContainer->hwndActive != hwndDlg) && !(dbei->flags & DBEF_SENT) && dbei->eventType == EVENTTYPE_MESSAGE) {
		dat->dwUnread++;
		UpdateTrayMenu(dat, (WORD)(dat->cache->getActiveStatus()), dat->cache->getActiveProto(), dat->szStatus, dat->hContact, 0L);
		if (nen_options.bTraySupport)
			return;
	}
	if (hEvent == 0)
		return;

	if (!PluginConfig.m_FlashOnClist)
		return;
	if ((GetForegroundWindow() != dat->pContainer->hwnd || dat->pContainer->hwndActive != hwndDlg) && !(dbei->flags & DBEF_SENT) && dbei->eventType == EVENTTYPE_MESSAGE && !(dat->dwFlagsEx & MWF_SHOW_FLASHCLIST)) {
		CLISTEVENT cle = { 0 };
		cle.cbSize = sizeof(cle);
		cle.hContact = (MCONTACT)dat->hContact;
		cle.hDbEvent = hEvent;
		cle.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
		cle.pszService = "SRMsg/ReadMessage";
		CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
		dat->dwFlagsEx |= MWF_SHOW_FLASHCLIST;
		dat->hFlashingEvent = hEvent;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// callback function for text streaming

static DWORD CALLBACK Message_StreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	static DWORD dwRead;
	char **ppText = (char **)dwCookie;

	if (*ppText == NULL) {
		*ppText = (char *)mir_alloc(cb + 2);
		CopyMemory(*ppText, pbBuff, cb);
		*pcb = cb;
		dwRead = cb;
		*(*ppText + cb) = '\0';
	}
	else {
		char *p = (char *)mir_realloc(*ppText, dwRead + cb + 2);
		CopyMemory(p + dwRead, pbBuff, cb);
		*ppText = p;
		*pcb = cb;
		dwRead += cb;
		*(*ppText + dwRead) = '\0';
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// retrieve contents of the richedit control by streaming.Used to get the
// typed message before sending it.
// caller must mir_free the returned pointer.
// UNICODE version returns UTF-8 encoded string.

char* TSAPI Message_GetFromStream(HWND hwndRtf, const TWindowData *dat, DWORD dwPassedFlags)
{
	EDITSTREAM stream;
	char *pszText = NULL;
	DWORD dwFlags = 0;

	if (hwndRtf == 0 || dat == 0)
		return NULL;

	ZeroMemory(&stream, sizeof(stream));
	stream.pfnCallback = Message_StreamCallback;
	stream.dwCookie = (DWORD_PTR)& pszText; // pass pointer to pointer
	if (dwPassedFlags == 0)
		dwFlags = (CP_UTF8 << 16) | (SF_RTFNOOBJS | SFF_PLAINRTF | SF_USECODEPAGE);
	else
		dwFlags = (CP_UTF8 << 16) | dwPassedFlags;
	SendMessage(hwndRtf, EM_STREAMOUT, (WPARAM)dwFlags, (LPARAM)&stream);

	return pszText; // pszText contains the text
}

/////////////////////////////////////////////////////////////////////////////////////////
// convert rich edit code to bbcode (if wanted). Otherwise, strip all RTF formatting
// tags and return plain text

BOOL TSAPI DoRtfToTags(TCHAR * pszText, const TWindowData *dat)
{
	TCHAR * p1;
	BOOL bJustRemovedRTF = TRUE;
	BOOL bTextHasStarted = FALSE;
	static int inColor = 0;

	if (!pszText)
		return FALSE;

	// used to filter out attributes which are already set for the default message input area font
	LOGFONTA lf = dat->pContainer->theme.logFonts[MSGFONTID_MESSAGEAREA];
	COLORREF color = dat->pContainer->theme.fontColors[MSGFONTID_MESSAGEAREA];

	// create an index of colors in the module and map them to
	// corresponding colors in the RTF color table

	Utils::CreateColorMap(pszText);
	// scan the file for rtf commands and remove or parse them
	inColor = 0;
	p1 = _tcsstr(pszText, _T("\\pard"));
	if (!p1)
		return FALSE;

	size_t iRemoveChars;
	TCHAR InsertThis[50];
	p1 += 5;

	MoveMemory(pszText, p1, (lstrlen(p1) + 1) * sizeof(TCHAR));
	p1 = pszText;
	// iterate through all characters, if rtf control character found then take action
	while (*p1 != (TCHAR) '\0') {
		mir_sntprintf(InsertThis, SIZEOF(InsertThis), _T(""));
		iRemoveChars = 0;

		switch (*p1) {
		case (TCHAR) '\\':
			if (p1 == _tcsstr(p1, _T("\\cf"))) { // foreground color
				TCHAR szTemp[20];
				int iCol = _ttoi(p1 + 3);
				int iInd = Utils::RTFColorToIndex(iCol);
				bJustRemovedRTF = TRUE;

				mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%d"), iCol);
				iRemoveChars = 3 + lstrlen(szTemp);
				if (bTextHasStarted || iCol)
					mir_sntprintf(InsertThis, SIZEOF(InsertThis), (iInd > 0) ? (inColor ? _T("[/color][color=%s]") : _T("[color=%s]")) : (inColor ? _T("[/color]") : _T("")), Utils::rtf_ctable[iInd - 1].szName);
				inColor = iInd > 0 ? 1 : 0;
			}
			else if (p1 == _tcsstr(p1, _T("\\highlight"))) { //background color
				TCHAR szTemp[20];
				int iCol = _ttoi(p1 + 10);
				bJustRemovedRTF = TRUE;

				mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%d"), iCol);
				iRemoveChars = 10 + lstrlen(szTemp);
			}
			else if (p1 == _tcsstr(p1, _T("\\par"))) { // newline
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = 4;
			}
			else if (p1 == _tcsstr(p1, _T("\\line"))) { // soft line break;
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = 5;
				mir_sntprintf(InsertThis, SIZEOF(InsertThis), _T("\n"));
			}
			else if (p1 == _tcsstr(p1, _T("\\endash"))) {
				bTextHasStarted = bJustRemovedRTF = TRUE;
				iRemoveChars = 7;
				mir_sntprintf(InsertThis, SIZEOF(InsertThis), _T("%c"), 0x2013);
			}
			else if (p1 == _tcsstr(p1, _T("\\emdash"))) {
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = 7;
				mir_sntprintf(InsertThis, SIZEOF(InsertThis), _T("%c"), 0x2014);
			}
			else if (p1 == _tcsstr(p1, _T("\\bullet"))) {
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = 7;
				mir_sntprintf(InsertThis, SIZEOF(InsertThis), _T("%c"), 0x2022);
			}
			else if (p1 == _tcsstr(p1, _T("\\ldblquote"))) {
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = 10;
				mir_sntprintf(InsertThis, SIZEOF(InsertThis), _T("%c"), 0x201C);
			}
			else if (p1 == _tcsstr(p1, _T("\\rdblquote"))) {
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = 10;
				mir_sntprintf(InsertThis, SIZEOF(InsertThis), _T("%c"), 0x201D);
			}
			else if (p1 == _tcsstr(p1, _T("\\lquote"))) {
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = 7;
				mir_sntprintf(InsertThis, SIZEOF(InsertThis), _T("%c"), 0x2018);
			}
			else if (p1 == _tcsstr(p1, _T("\\rquote"))) {
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = 7;
				mir_sntprintf(InsertThis, SIZEOF(InsertThis), _T("%c"), 0x2019);
			}
			else if (p1 == _tcsstr(p1, _T("\\b"))) { //bold
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = (p1[2] != (TCHAR) '0') ? 2 : 3;
				if (!(lf.lfWeight == FW_BOLD)) { // only allow bold if the font itself isn't a bold one, otherwise just strip it..
					if (dat->SendFormat)
						mir_sntprintf(InsertThis, SIZEOF(InsertThis), (p1[2] != (TCHAR) '0') ? _T("[b]") : _T("[/b]"));
				}
			}
			else if (p1 == _tcsstr(p1, _T("\\i"))) { // italics
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = (p1[2] != (TCHAR) '0') ? 2 : 3;
				if (!lf.lfItalic) { // same as for bold
					if (dat->SendFormat)
						mir_sntprintf(InsertThis, SIZEOF(InsertThis), (p1[2] != (TCHAR) '0') ? _T("[i]") : _T("[/i]"));
				}
			}
			else if (p1 == _tcsstr(p1, _T("\\strike"))) { // strike-out
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = (p1[7] != (TCHAR) '0') ? 7 : 8;
				if (!lf.lfStrikeOut) { // same as for bold
					if (dat->SendFormat)
						mir_sntprintf(InsertThis, SIZEOF(InsertThis), (p1[7] != (TCHAR) '0') ? _T("[s]") : _T("[/s]"));
				}
			}
			else if (p1 == _tcsstr(p1, _T("\\ul"))) { // underlined
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				if (p1[3] == (TCHAR) 'n')
					iRemoveChars = 7;
				else if (p1[3] == (TCHAR) '0')
					iRemoveChars = 4;
				else
					iRemoveChars = 3;
				if (!lf.lfUnderline) { // same as for bold
					if (dat->SendFormat)
						mir_sntprintf(InsertThis, SIZEOF(InsertThis), (p1[3] != (TCHAR) '0' && p1[3] != (TCHAR) 'n') ? _T("[u]") : _T("[/u]"));
				}
			}
			else if (p1 == _tcsstr(p1, _T("\\tab"))) { // tab
				bTextHasStarted = TRUE;
				bJustRemovedRTF = TRUE;
				iRemoveChars = 4;
				mir_sntprintf(InsertThis, SIZEOF(InsertThis), _T("%c"), 0x09);
			}
			else if (p1[1] == (TCHAR) '\\' || p1[1] == (TCHAR) '{' || p1[1] == (TCHAR) '}') { // escaped characters
				bTextHasStarted = TRUE;
				//bJustRemovedRTF = TRUE;
				iRemoveChars = 2;
				mir_sntprintf(InsertThis, SIZEOF(InsertThis), _T("%c"), p1[1]);
			}
			else if (p1[1] == (TCHAR) '\'') { // special character
				bTextHasStarted = TRUE;
				bJustRemovedRTF = FALSE;
				if (p1[2] != (TCHAR) ' ' && p1[2] != (TCHAR) '\\') {
					int iLame = 0;
					TCHAR * p3;
					TCHAR *stoppedHere;

					if (p1[3] != (TCHAR) ' ' && p1[3] != (TCHAR) '\\') {
						_tcsncpy(InsertThis, p1 + 2, 3);
						iRemoveChars = 4;
						InsertThis[2] = 0;
					}
					else {
						_tcsncpy(InsertThis, p1 + 2, 2);
						iRemoveChars = 3;
						InsertThis[2] = 0;
					}
					// convert string containing char in hex format to int.
					p3 = InsertThis;
					iLame = _tcstol(p3, &stoppedHere, 16);
					mir_sntprintf(InsertThis, SIZEOF(InsertThis), _T("%c"), (TCHAR)iLame);

				}
				else
					iRemoveChars = 2;
			}
			else { // remove unknown RTF command
				int j = 1;
				bJustRemovedRTF = TRUE;
				while (!_tcschr(_T(" !$%()#*\"'"), p1[j]) && p1[j] != (TCHAR) '§' && p1[j] != (TCHAR) '\\' && p1[j] != (TCHAR) '\0')
					j++;
				iRemoveChars = j;
			}
			break;

		case (TCHAR) '{': // other RTF control characters
		case (TCHAR) '}':
			iRemoveChars = 1;
			break;

		case (TCHAR) ' ': // remove spaces following a RTF command
			if (bJustRemovedRTF)
				iRemoveChars = 1;
			bJustRemovedRTF = FALSE;
			bTextHasStarted = TRUE;
			break;

		default: // other text that should not be touched
			bTextHasStarted = TRUE;
			bJustRemovedRTF = FALSE;
			break;
		}

		// move the memory and paste in new commands instead of the old RTF
		if (lstrlen(InsertThis) || iRemoveChars) {
			MoveMemory(p1 + lstrlen(InsertThis), p1 + iRemoveChars, (lstrlen(p1) - iRemoveChars + 1) * sizeof(TCHAR));
			CopyMemory(p1, InsertThis, lstrlen(InsertThis) * sizeof(TCHAR));
			p1 += lstrlen(InsertThis);
		}
		else p1++;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// retrieve both buddys and my own UIN for a message session and store them in the message window *dat
// respects metacontacts and uses the current protocol if the contact is a MC

void TSAPI GetMYUIN(TWindowData *dat)
{
	CONTACTINFO ci = { sizeof(ci) };
	ci.szProto = const_cast<char *>(dat->cache->getActiveProto());
	ci.dwFlag = CNF_TCHAR | CNF_DISPLAYUID;

	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
		switch (ci.type) {
		case CNFT_ASCIIZ:
			mir_sntprintf(dat->myUin, SIZEOF(dat->myUin), _T("%s"), reinterpret_cast<TCHAR *>(ci.pszVal));
			mir_free((void*)ci.pszVal);
			break;
		case CNFT_DWORD:
			mir_sntprintf(dat->myUin, SIZEOF(dat->myUin), _T("%u"), ci.dVal);
			break;
		default:
			dat->myUin[0] = 0;
			break;
		}
	}
	else dat->myUin[0] = 0;
}

static int g_IEViewAvail = -1;
static int g_HPPAvail = -1;

UINT TSAPI GetIEViewMode(HWND hwndDlg, MCONTACT hContact)
{
	int iWantIEView = 0, iWantHPP = 0;

	if (g_IEViewAvail == -1)
		g_IEViewAvail = ServiceExists(MS_IEVIEW_WINDOW);

	if (g_HPPAvail == -1)
		g_HPPAvail = ServiceExists("History++/ExtGrid/NewWindow");

	PluginConfig.g_WantIEView = g_IEViewAvail && M.GetByte("default_ieview", 0);
	PluginConfig.g_WantHPP = g_HPPAvail && M.GetByte("default_hpp", 0);

	iWantIEView = (PluginConfig.g_WantIEView) || (M.GetByte(hContact, "ieview", 0) == 1 && g_IEViewAvail);
	iWantIEView = (M.GetByte(hContact, "ieview", 0) == (BYTE)-1) ? 0 : iWantIEView;

	iWantHPP = (PluginConfig.g_WantHPP) || (M.GetByte(hContact, "hpplog", 0) == 1 && g_HPPAvail);
	iWantHPP = (M.GetByte(hContact, "hpplog", 0) == (BYTE)-1) ? 0 : iWantHPP;

	return iWantHPP ? WANT_HPP_LOG : (iWantIEView ? WANT_IEVIEW_LOG : 0);
}

#if defined(__FEAT_DEPRECATED_DYNAMICSWITCHLOGVIEWER)
static void CheckAndDestroyHPP(TWindowData *dat)
{
	if (dat->hwndHPP) {
		IEVIEWWINDOW ieWindow;
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_DESTROY;
		ieWindow.hwnd = dat->hwndHPP;
		if (dat->oldIEViewProc) {
			SetWindowLongPtr(dat->hwndHPP, GWLP_WNDPROC, (LONG_PTR)dat->oldIEViewProc);
			dat->oldIEViewProc = 0;
		}
		CallService(MS_HPP_EG_WINDOW, 0, (LPARAM)&ieWindow);
		dat->hwndHPP = 0;
	}
}

void TSAPI CheckAndDestroyIEView(TWindowData *dat)
{
	if (dat->hwndIEView) {
		IEVIEWWINDOW ieWindow;
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_DESTROY;
		ieWindow.hwnd = dat->hwndIEView;
		if (dat->oldIEViewProc){
			SetWindowLongPtr(dat->hwndIEView, GWLP_WNDPROC, (LONG_PTR)dat->oldIEViewProc);
			dat->oldIEViewProc =0;
		}
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
		dat->oldIEViewProc = 0;
		dat->hwndIEView = 0;
	}
}
#endif

void TSAPI SetMessageLog(TWindowData *dat)
{
	HWND		 hwndDlg = dat->hwnd;
	unsigned int iLogMode = GetIEViewMode(hwndDlg, dat->hContact);

	if (iLogMode == WANT_IEVIEW_LOG && dat->hwndIEView == 0) {
		IEVIEWWINDOW ieWindow;

		ZeroMemory(&ieWindow, sizeof(ieWindow));
#if defined(__FEAT_DEPRECATED_DYNAMICSWITCHLOGVIEWER)
		CheckAndDestroyHPP(dat);
#endif
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_CREATE;
		ieWindow.dwFlags = 0;
		ieWindow.dwMode = IEWM_TABSRMM;
		ieWindow.parent = hwndDlg;
		ieWindow.x = 0;
		ieWindow.y = 0;
		ieWindow.cx = 200;
		ieWindow.cy = 200;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
		dat->hwndIEView = ieWindow.hwnd;
		Utils::showDlgControl(hwndDlg, IDC_LOG, SW_HIDE);
		Utils::enableDlgControl(hwndDlg, IDC_LOG, FALSE);
	}
	else if (iLogMode == WANT_HPP_LOG && dat->hwndHPP == 0) {
		IEVIEWWINDOW ieWindow;
#if defined(__FEAT_DEPRECATED_DYNAMICSWITCHLOGVIEWER)
		CheckAndDestroyIEView(dat);
#endif
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_CREATE;
		ieWindow.dwFlags = 0;
		ieWindow.dwMode = IEWM_TABSRMM;
		ieWindow.parent = hwndDlg;
		ieWindow.x = 0;
		ieWindow.y = 0;
		ieWindow.cx = 10;
		ieWindow.cy = 10;
		CallService(MS_HPP_EG_WINDOW, 0, (LPARAM)&ieWindow);
		dat->hwndHPP = ieWindow.hwnd;
		Utils::showDlgControl(hwndDlg, IDC_LOG, SW_HIDE);
		Utils::enableDlgControl(hwndDlg, IDC_LOG, FALSE);
	}
	else {
#if defined(__FEAT_DEPRECATED_DYNAMICSWITCHLOGVIEWER)
		if (iLogMode != WANT_IEVIEW_LOG)
			CheckAndDestroyIEView(dat);
		if (iLogMode != WANT_HPP_LOG)
			CheckAndDestroyHPP(dat);
		Utils::showDlgControl(hwndDlg, IDC_LOG, SW_SHOW);
		Utils::enableDlgControl(hwndDlg, IDC_LOG, TRUE);
		dat->hwndIEView = 0;
		dat->hwndIWebBrowserControl = 0;
		dat->hwndHPP = 0;
#endif
	}
}

#if defined(__FEAT_DEPRECATED_DYNAMICSWITCHLOGVIEWER)
void TSAPI SwitchMessageLog(TWindowData *dat, int iMode)
{
	HWND hwndDlg = dat->hwnd;

	if (iMode) {           // switch from rtf to IEview or hpp
		SetDlgItemText(hwndDlg, IDC_LOG, _T(""));
		Utils::enableDlgControl(hwndDlg, IDC_LOG, FALSE);
		Utils::showDlgControl(hwndDlg, IDC_LOG, SW_HIDE);
		SetMessageLog(dat);
	} else                    // switch from IEView or hpp to rtf
		SetMessageLog(dat);

	SetDialogToType(hwndDlg);
	SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
	SendMessage(hwndDlg, WM_SIZE, 0, 0);

	if (dat->hwndIEView) {
		if (M.GetByte("subclassIEView", 0)&&dat->oldIEViewProc == 0) {
			WNDPROC wndProc = (WNDPROC)SetWindowLongPtr(dat->hwndIEView, GWLP_WNDPROC, (LONG_PTR)IEViewSubclassProc);
			dat->oldIEViewProc = wndProc;
		}
	} else if (dat->hwndHPP) {
		if (dat->oldIEViewProc == 0) {
			WNDPROC wndProc = (WNDPROC)SetWindowLongPtr(dat->hwndHPP, GWLP_WNDPROC, (LONG_PTR)HPPKFSubclassProc);
			dat->oldIEViewProc = wndProc;
		}
	}
}
#endif

void TSAPI FindFirstEvent(TWindowData *dat)
{
	int historyMode = db_get_b(dat->hContact, SRMSGMOD, SRMSGSET_LOADHISTORY, -1);
	if (historyMode == -1)
		historyMode = (int)M.GetByte(SRMSGMOD, SRMSGSET_LOADHISTORY, SRMSGDEFSET_LOADHISTORY);

	dat->hDbEventFirst = db_event_firstUnread(dat->hContact);

	if (dat->bActualHistory)
		historyMode = LOADHISTORY_COUNT;

	switch (historyMode) {
	case LOADHISTORY_COUNT:
		{
			int i;
			HANDLE hPrevEvent;
			DBEVENTINFO dbei = { sizeof(dbei) };
			//MAD: ability to load only current session's history
			if (dat->bActualHistory)
				i = dat->cache->getSessionMsgCount();
			else
				i = db_get_w(NULL, SRMSGMOD, SRMSGSET_LOADCOUNT, SRMSGDEFSET_LOADCOUNT);
			//
			for (; i > 0; i--) {
				if (dat->hDbEventFirst == NULL)
					hPrevEvent = db_event_last(dat->hContact);
				else
					hPrevEvent = db_event_prev(dat->hContact, dat->hDbEventFirst);
				if (hPrevEvent == NULL)
					break;
				dbei.cbBlob = 0;
				dat->hDbEventFirst = hPrevEvent;
				db_event_get(dat->hDbEventFirst, &dbei);
				if (!DbEventIsShown(dat, &dbei))
					i++;
			}
		}
		break;

	case LOADHISTORY_TIME:
		HANDLE hPrevEvent;
		DWORD firstTime;

		DBEVENTINFO dbei = { sizeof(dbei) };
		if (dat->hDbEventFirst == NULL)
			dbei.timestamp = time(NULL);
		else
			db_event_get(dat->hDbEventFirst, &dbei);
		firstTime = dbei.timestamp - 60 * db_get_w(NULL, SRMSGMOD, SRMSGSET_LOADTIME, SRMSGDEFSET_LOADTIME);
		for (;;) {
			if (dat->hDbEventFirst == NULL)
				hPrevEvent = db_event_last(dat->hContact);
			else
				hPrevEvent = db_event_prev(dat->hContact, dat->hDbEventFirst);
			if (hPrevEvent == NULL)
				break;
			dbei.cbBlob = 0;
			db_event_get(hPrevEvent, &dbei);
			if (dbei.timestamp < firstTime)
				break;
			dat->hDbEventFirst = hPrevEvent;
		}
		break;
	}
}

void TSAPI SaveSplitter(TWindowData *dat)
{
	// group chats save their normal splitter position independently
	if (dat->bType == SESSIONTYPE_CHAT || dat->bIsAutosizingInput)
		return;

	if (dat->splitterY < DPISCALEY_S(MINSPLITTERY) || dat->splitterY < 0)
		dat->splitterY = DPISCALEY_S(MINSPLITTERY);

	if (dat->dwFlagsEx & MWF_SHOW_SPLITTEROVERRIDE)
		db_set_dw(dat->hContact, SRMSGMOD_T, "splitsplity", dat->splitterY);
	else {
		if (dat->pContainer->settings->fPrivate)
			dat->pContainer->settings->splitterPos = dat->splitterY;
		else
			db_set_dw(0, SRMSGMOD_T, "splitsplity", dat->splitterY);
	}
}

void TSAPI LoadSplitter(TWindowData *dat)
{
	if (dat->bIsAutosizingInput) {
		dat->splitterY = GetDefaultMinimumInputHeight(dat);
		return;
	}

	if (!(dat->dwFlagsEx & MWF_SHOW_SPLITTEROVERRIDE))
		if (!dat->pContainer->settings->fPrivate)
			dat->splitterY = (int)M.GetDword("splitsplity", (DWORD)60);
		else
			dat->splitterY = dat->pContainer->settings->splitterPos;
	else
		dat->splitterY = (int)M.GetDword(dat->hContact, "splitsplity", M.GetDword("splitsplity", (DWORD)70));

	if (dat->splitterY < MINSPLITTERY || dat->splitterY < 0)
		dat->splitterY = 150;
}

void TSAPI PlayIncomingSound(const TWindowData *dat)
{
	int iPlay = Utils::mustPlaySound(dat);

	if (iPlay) {
		if (GetForegroundWindow() == dat->pContainer->hwnd && dat->pContainer->hwndActive == dat->hwnd)
			SkinPlaySound("RecvMsgActive");
		else
			SkinPlaySound("RecvMsgInactive");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// reads send format and configures the toolbar buttons
// if mode == 0, int only configures the buttons and does not change send format

void TSAPI GetSendFormat(TWindowData *dat, int mode)
{
	UINT controls[5] = { IDC_FONTBOLD, IDC_FONTITALIC, IDC_FONTUNDERLINE, IDC_FONTSTRIKEOUT, IDC_FONTFACE };

	if (mode) {
		dat->SendFormat = M.GetDword(dat->hContact, "sendformat", PluginConfig.m_SendFormat);
		if (dat->SendFormat == -1)          // per contact override to disable it..
			dat->SendFormat = 0;
		else if (dat->SendFormat == 0)
			dat->SendFormat = PluginConfig.m_SendFormat ? 1 : 0;
	}
	for (int i = 0; i < 5; i++)
		Utils::enableDlgControl(dat->hwnd, controls[i], dat->SendFormat != 0 ? TRUE : FALSE);
	return;
}

/////////////////////////////////////////////////////////////////////////////////////////
// get user - readable locale information for the currently selected
// keyboard layout.
//
// GetLocaleInfo() should no longer be used on Vista and later

void TSAPI GetLocaleID(TWindowData *dat, const TCHAR *szKLName)
{
	TCHAR szLI[256], *stopped = NULL;
	USHORT langID;
	WORD   wCtype2[3];
	PARAFORMAT2 pf2;
	BOOL fLocaleNotSet;
	char szTest[4] = { (char)0xe4, (char)0xf6, (char)0xfc, 0 };

	szLI[0] = szLI[1] = 0;

	ZeroMemory(&pf2, sizeof(PARAFORMAT2));
	langID = (USHORT)_tcstol(szKLName, &stopped, 16);
	dat->lcid = MAKELCID(langID, 0);
	/*
	 * Vista+: read ISO locale names from the registry
	 */
	if (PluginConfig.m_bIsVista) {
		HKEY	hKey = 0;
		TCHAR	szKey[20];
		DWORD	dwLID = _tcstoul(szKLName, &stopped, 16);

		mir_sntprintf(szKey, 20, _T("%04.04x"), LOWORD(dwLID));
		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("MIME\\Database\\Rfc1766"), 0, KEY_READ, &hKey)) {
			DWORD dwLength = 255;
			if (ERROR_SUCCESS == RegQueryValueEx(hKey, szKey, 0, 0, (unsigned char *)szLI, &dwLength)) {
				TCHAR*	p;

				szLI[255] = 0;
				if ((p = _tcschr(szLI, ';')) != 0)
					*p = 0;
			}
			RegCloseKey(hKey);
		}
		szLI[0] = _totupper(szLI[0]);
		szLI[1] = _totupper(szLI[1]);
	}
	else {
		GetLocaleInfo(dat->lcid, LOCALE_SISO639LANGNAME, szLI, 10);
		_tcsupr(szLI);
	}
	fLocaleNotSet = (dat->lcID[0] == 0 && dat->lcID[1] == 0);
	mir_sntprintf(dat->lcID, SIZEOF(dat->lcID), szLI);
	GetStringTypeA(dat->lcid, CT_CTYPE2, szTest, 3, wCtype2);
	pf2.cbSize = sizeof(pf2);
	pf2.dwMask = PFM_RTLPARA;
	SendDlgItemMessage(dat->hwnd, IDC_MESSAGE, EM_GETPARAFORMAT, 0, (LPARAM)&pf2);
	if (Utils::FindRTLLocale(dat) && fLocaleNotSet) {
		if (wCtype2[0] == C2_RIGHTTOLEFT || wCtype2[1] == C2_RIGHTTOLEFT || wCtype2[2] == C2_RIGHTTOLEFT) {
			ZeroMemory(&pf2, sizeof(pf2));
			pf2.dwMask = PFM_RTLPARA;
			pf2.cbSize = sizeof(pf2);
			pf2.wEffects = PFE_RTLPARA;
			SendDlgItemMessage(dat->hwnd, IDC_MESSAGE, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
		}
		else {
			ZeroMemory(&pf2, sizeof(pf2));
			pf2.dwMask = PFM_RTLPARA;
			pf2.cbSize = sizeof(pf2);
			pf2.wEffects = 0;
			SendDlgItemMessage(dat->hwnd, IDC_MESSAGE, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
		}
		SendDlgItemMessage(dat->hwnd, IDC_MESSAGE, EM_SETLANGOPTIONS, 0, (LPARAM)SendDlgItemMessage(dat->hwnd, IDC_MESSAGE, EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);
	}
}

void TSAPI LoadContactAvatar(TWindowData *dat)
{
	if (dat == NULL) return;

	dat->ace = Utils::loadAvatarFromAVS(dat->bIsMeta ? db_mc_getSrmmSub(dat->hContact) : dat->hContact);

	if (!dat->Panel->isActive() || dat->pContainer->avatarMode == 3) {
		BITMAP bm;
		dat->iRealAvatarHeight = 0;

		if (dat->ace && dat->ace->hbmPic) {
			AdjustBottomAvatarDisplay(dat);
			GetObject(dat->ace->hbmPic, sizeof(bm), &bm);
			CalcDynamicAvatarSize(dat, &bm);
			PostMessage(dat->hwnd, WM_SIZE, 0, 0);
		}
		else if (dat->ace == NULL) {
			AdjustBottomAvatarDisplay(dat);
			GetObject(PluginConfig.g_hbmUnknown, sizeof(bm), &bm);
			CalcDynamicAvatarSize(dat, &bm);
			PostMessage(dat->hwnd, WM_SIZE, 0, 0);
		}
	}
}

void TSAPI LoadOwnAvatar(TWindowData *dat)
{
	AVATARCACHEENTRY *ace = NULL;

	if (ServiceExists(MS_AV_GETMYAVATAR))
		ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETMYAVATAR, 0, (LPARAM)(dat->cache->getActiveProto()));

	if (ace) {
		dat->hOwnPic = ace->hbmPic;
		dat->ownAce = ace;
	}
	else {
		dat->hOwnPic = PluginConfig.g_hbmUnknown;
		dat->ownAce = NULL;
	}
	if (dat->Panel->isActive() && dat->pContainer->avatarMode != 3) {
		BITMAP bm;

		dat->iRealAvatarHeight = 0;
		AdjustBottomAvatarDisplay(dat);
		GetObject(dat->hOwnPic, sizeof(bm), &bm);
		CalcDynamicAvatarSize(dat, &bm);
		SendMessage(dat->hwnd, WM_SIZE, 0, 0);
	}
}

void TSAPI LoadTimeZone(TWindowData *dat)
{
	if (dat)
		dat->hTimeZone = tmi.createByContact(dat->hContact, 0, TZF_KNOWNONLY);
}

// paste contents of the clipboard into the message input area and send it immediately

void TSAPI HandlePasteAndSend(const TWindowData *dat)
{
	UINT ctrlID = dat->bType == SESSIONTYPE_IM ? IDC_MESSAGE : IDC_CHAT_MESSAGE;

	if (!PluginConfig.m_PasteAndSend) {
		SendMessage(dat->hwnd, DM_ACTIVATETOOLTIP, ctrlID, (LPARAM)TranslateT("The 'paste and send' feature is disabled. You can enable it on the 'General' options page in the 'Sending Messages' section"));
		return;                                     // feature disabled
	}

	SendMessage(GetDlgItem(dat->hwnd, ctrlID), EM_PASTESPECIAL, CF_UNICODETEXT, 0);
	if (GetWindowTextLengthA(GetDlgItem(dat->hwnd, ctrlID)) > 0)
		SendMessage(dat->hwnd, WM_COMMAND, IDOK, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// draw various elements of the message window, like avatar(s), info panel fields
// and the color formatting menu

int TSAPI MsgWindowDrawHandler(WPARAM wParam, LPARAM lParam, TWindowData *dat)
{
	LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;

	if (!dat)
		return 0;

	bool	bAero = M.isAero();
	HWND	hwndDlg = dat->hwnd;

	if (dis->CtlType == ODT_MENU && dis->hwndItem == (HWND)GetSubMenu(PluginConfig.g_hMenuContext, 7)) {
		RECT rc = { 0 };
		HBRUSH old, col;
		COLORREF clr;
		switch (dis->itemID) {
		case ID_FONT_RED:
			clr = RGB(255, 0, 0);
			break;
		case ID_FONT_BLUE:
			clr = RGB(0, 0, 255);
			break;
		case ID_FONT_GREEN:
			clr = RGB(0, 255, 0);
			break;
		case ID_FONT_MAGENTA:
			clr = RGB(255, 0, 255);
			break;
		case ID_FONT_YELLOW:
			clr = RGB(255, 255, 0);
			break;
		case ID_FONT_WHITE:
			clr = RGB(255, 255, 255);
			break;
		case ID_FONT_DEFAULTCOLOR:
			clr = GetSysColor(COLOR_MENU);
			break;
		case ID_FONT_CYAN:
			clr = RGB(0, 255, 255);
			break;
		case ID_FONT_BLACK:
			clr = RGB(0, 0, 0);
			break;
		default:
			clr = 0;
		}
		col = (HBRUSH)CreateSolidBrush(clr);
		old = (HBRUSH)SelectObject(dis->hDC, col);
		rc.left = 2;
		rc.top = dis->rcItem.top - 5;
		rc.right = 15;
		rc.bottom = dis->rcItem.bottom + 4;
		Rectangle(dis->hDC, rc.left - 1, rc.top - 1, rc.right + 1, rc.bottom + 1);
		FillRect(dis->hDC, &rc, col);
		SelectObject(dis->hDC, old);
		DeleteObject(col);
		return TRUE;
	}
	
	if ((dis->hwndItem == GetDlgItem(hwndDlg, IDC_CONTACTPIC) && (dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown) && dat->showPic) || (dis->hwndItem == hwndDlg && dat->Panel->isActive() && (dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown))) {
		HBRUSH 		hOldBrush;
		BITMAP 		bminfo;
		double 		dAspect = 0, dNewWidth = 0, dNewHeight = 0;
		DWORD 		iMaxHeight = 0, top, cx, cy;
		RECT 		rc, rcClient, rcFrame;
		HDC 		hdcDraw;
		HBITMAP 	hbmDraw, hbmOld;
		BOOL 		bPanelPic = dis->hwndItem == hwndDlg;
		DWORD 		aceFlags = 0;
		HPEN 		hPenBorder = 0, hPenOld = 0;
		HRGN 		clipRgn = 0;
		int  		iRad = PluginConfig.m_WinVerMajor >= 5 ? 4 : 6;
		BOOL 		flashAvatar = FALSE;
		bool 		bInfoPanel = dat->Panel->isActive();

		if (PluginConfig.g_FlashAvatarAvail && (!bPanelPic || (bPanelPic && dat->showInfoPic == 1))) {
			FLASHAVATAR fa = { 0 };

			fa.id = 25367;
			fa.cProto = dat->szProto;
			if (!bPanelPic && bInfoPanel) {
				fa.hParentWindow = GetDlgItem(hwndDlg, IDC_CONTACTPIC);
				CallService(MS_FAVATAR_MAKE, (WPARAM)&fa, 0);
				Utils::enableDlgControl(hwndDlg, IDC_CONTACTPIC, fa.hWindow != 0);
			}
			else {
				fa.hContact = dat->hContact;
				fa.hParentWindow = bInfoPanel ? dat->hwndPanelPicParent : GetDlgItem(hwndDlg, IDC_CONTACTPIC);
				CallService(MS_FAVATAR_MAKE, (WPARAM)&fa, 0);
				if (bInfoPanel) {
					if (fa.hWindow != NULL&&dat->hwndPanelPic) {
						DestroyWindow(dat->hwndPanelPic);
						dat->hwndPanelPic = NULL;
					}
					if (!PluginConfig.g_bDisableAniAvatars && fa.hWindow == 0 && dat->hwndPanelPic == 0) {
						dat->hwndPanelPic = CreateWindowEx(WS_EX_TOPMOST, AVATAR_CONTROL_CLASS, _T(""), WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, dat->hwndPanelPicParent, (HMENU)7000, NULL, NULL);
						Utils::setAvatarContact(dat->hwndPanelPic, dat->hContact);
					}
				}
				else {
					if (fa.hWindow != NULL && dat->hwndContactPic) {
						DestroyWindow(dat->hwndContactPic);
						dat->hwndContactPic = NULL;
					}
					if (!PluginConfig.g_bDisableAniAvatars && fa.hWindow == 0 && dat->hwndContactPic == 0) {
						dat->hwndContactPic = CreateWindowEx(WS_EX_TOPMOST, AVATAR_CONTROL_CLASS, _T(""), WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, GetDlgItem(hwndDlg, IDC_CONTACTPIC), (HMENU)0, NULL, NULL);
						Utils::setAvatarContact(dat->hwndContactPic, dat->hContact);
					}
				}
				dat->hwndFlash = fa.hWindow;
			}
			if (fa.hWindow != 0) {
				bminfo.bmHeight = FAVATAR_HEIGHT;
				bminfo.bmWidth = FAVATAR_WIDTH;
				CallService(MS_FAVATAR_SETBKCOLOR, (WPARAM)&fa, (LPARAM)GetSysColor(COLOR_3DFACE));
				flashAvatar = TRUE;
			}
		}

		if (bPanelPic) {
			if (!flashAvatar)
				GetObject(dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown, sizeof(bminfo), &bminfo);

			if ((dat->ace && dat->showInfoPic && !(dat->ace->dwFlags & AVS_HIDEONCLIST)) || dat->showInfoPic)
				aceFlags = dat->ace ? dat->ace->dwFlags : 0;
			else {
				if (dat->panelWidth) {
					dat->panelWidth = -1;
					if (!CSkin::m_skinEnabled)
						SendMessage(hwndDlg, WM_SIZE, 0, 0);
				}
				return TRUE;
			}
		}
		else {
			if (!bInfoPanel || dat->pContainer->avatarMode == 3) {
				if (dat->ace)
					aceFlags = dat->ace->dwFlags;
			}
			else if (dat->ownAce)
				aceFlags = dat->ownAce->dwFlags;

			GetObject((bInfoPanel && dat->pContainer->avatarMode != 3) ? dat->hOwnPic : (dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown), sizeof(bminfo), &bminfo);
		}

		GetClientRect(hwndDlg, &rc);
		if (bPanelPic) {
			rcClient = dis->rcItem;
			cx = (rcClient.right - rcClient.left);
			cy = (rcClient.bottom - rcClient.top) + 1;
		}
		else {
			GetClientRect(dis->hwndItem, &rcClient);
			cx = rcClient.right;
			cy = rcClient.bottom;
		}

		if (cx < 5 || cy < 5)
			return TRUE;

		if (bPanelPic) {
			if (bminfo.bmHeight > bminfo.bmWidth) {
				if (bminfo.bmHeight > 0)
					dAspect = (double)(cy /*- 2*/) / (double)bminfo.bmHeight;
				else
					dAspect = 1.0;
				dNewWidth = (double)bminfo.bmWidth * dAspect;
				dNewHeight = cy;// - 2;
			}
			else {
				if (bminfo.bmWidth > 0)
					dAspect = (double)(cy /*- 2*/) / (double)bminfo.bmWidth;
				else
					dAspect = 1.0;
				dNewHeight = (double)bminfo.bmHeight * dAspect;
				dNewWidth = cy;// - 2;
			}
			if (dat->panelWidth == -1) {
				dat->panelWidth = (int)dNewWidth;
				return 0;
			}
		}
		else {

			if (bminfo.bmHeight > 0)
				dAspect = (double)dat->iRealAvatarHeight / (double)bminfo.bmHeight;
			else
				dAspect = 1.0;
			dNewWidth = (double)bminfo.bmWidth * dAspect;
			if (dNewWidth > (double)(rc.right) * 0.8)
				dNewWidth = (double)(rc.right) * 0.8;
			iMaxHeight = dat->iRealAvatarHeight;
		}

		if (flashAvatar) {
			SetWindowPos(dat->hwndPanelPicParent, HWND_TOP, rcClient.left, rcClient.top,
				(int)dNewWidth, (int)dNewHeight, SWP_SHOWWINDOW | SWP_NOCOPYBITS);
			return TRUE;
		}

		hdcDraw = CreateCompatibleDC(dis->hDC);
		hbmDraw = CreateCompatibleBitmap(dis->hDC, cx, cy);
		hbmOld = (HBITMAP)SelectObject(hdcDraw, hbmDraw);

		bool	bAero = M.isAero();

		hOldBrush = (HBRUSH)SelectObject(hdcDraw, bAero ? (HBRUSH)GetStockObject(HOLLOW_BRUSH) : GetSysColorBrush(COLOR_3DFACE));
		rcFrame = rcClient;

		if (!bPanelPic) {
			top = (cy - dat->pic.cy) / 2;
			RECT rcEdge = { 0, top, dat->pic.cx, top + dat->pic.cy };
			if (CSkin::m_skinEnabled)
				CSkin::SkinDrawBG(dis->hwndItem, dat->pContainer->hwnd, dat->pContainer, &dis->rcItem, hdcDraw);
			else if (PluginConfig.m_fillColor) {
				HBRUSH br = CreateSolidBrush(PluginConfig.m_fillColor);
				FillRect(hdcDraw, &rcFrame, br);
				DeleteObject(br);
			}
			else {
				if (bAero && CSkin::m_pCurrentAeroEffect) {
					COLORREF clr = PluginConfig.m_tbBackgroundHigh ? PluginConfig.m_tbBackgroundHigh :
						(CSkin::m_pCurrentAeroEffect ? CSkin::m_pCurrentAeroEffect->m_clrToolbar : 0xf0f0f0);

					HBRUSH br = CreateSolidBrush(clr);
					FillRect(hdcDraw, &rcFrame, br);
					DeleteObject(br);
				}
				else FillRect(hdcDraw, &rcFrame, GetSysColorBrush(COLOR_3DFACE));
			}

			hPenBorder = CreatePen(PS_SOLID, 1, CSkin::m_avatarBorderClr);
			hPenOld = (HPEN)SelectObject(hdcDraw, hPenBorder);

			if (CSkin::m_bAvatarBorderType == 1)
				Rectangle(hdcDraw, rcEdge.left, rcEdge.top, rcEdge.right, rcEdge.bottom);
			else if (CSkin::m_bAvatarBorderType == 2) {
				clipRgn = CreateRoundRectRgn(rcEdge.left, rcEdge.top, rcEdge.right + 1, rcEdge.bottom + 1, iRad, iRad);
				SelectClipRgn(hdcDraw, clipRgn);
			}
		}

		if ((((bInfoPanel && dat->pContainer->avatarMode != 3) ? dat->hOwnPic : (dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown)) && dat->showPic) || bPanelPic) {
			HDC hdcMem = CreateCompatibleDC(dis->hDC);
			HBITMAP hbmAvatar = bPanelPic ? (dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown) : ((bInfoPanel && dat->pContainer->avatarMode != 3) ? dat->hOwnPic : (dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown));
			HBITMAP hbmMem = 0;
			if (bPanelPic) {
				bool	bBorder = (CSkin::m_bAvatarBorderType ? true : false);

				LONG 	height_off = 0;
				LONG	border_off = bBorder ? 1 : 0;

				ResizeBitmap rb;
				rb.size = sizeof(rb);
				rb.fit = RESIZEBITMAP_STRETCH;
				rb.max_height = (int)(dNewHeight - (bBorder ? 2 : 0));
				rb.max_width = (int)(dNewWidth - (bBorder ? 2 : 0));
				rb.hBmp = hbmAvatar;

				HBITMAP hbmNew = (HBITMAP)CallService("IMG/ResizeBitmap", (WPARAM)&rb, 0);
				hbmMem = (HBITMAP)SelectObject(hdcMem, hbmNew);

				rcFrame.left = rcFrame.top = 0;
				rcFrame.right = (rcClient.right - rcClient.left);
				rcFrame.bottom = (rcClient.bottom - rcClient.top);

				rcFrame.left = rcFrame.right - (LONG)dNewWidth;
				rcFrame.bottom = (LONG)dNewHeight;

				height_off = ((cy)-(rb.max_height + (bBorder ? 2 : 0))) / 2;
				rcFrame.top += height_off;
				rcFrame.bottom += height_off;

				/*
				 * prepare border drawing (if avatar is rendered by ACC, the parent control will be responsible for
				 * the border, so skip it here)
				 */
				if (dat->hwndPanelPic == 0) {
					OffsetRect(&rcClient, -2, 0);
					if (CSkin::m_bAvatarBorderType == 1)
						clipRgn = CreateRectRgn(rcClient.left + rcFrame.left, rcClient.top + rcFrame.top, rcClient.left + rcFrame.right,
						rcClient.top + rcFrame.bottom);
					else if (CSkin::m_bAvatarBorderType == 2) {
						clipRgn = CreateRoundRectRgn(rcClient.left + rcFrame.left, rcClient.top + rcFrame.top, rcClient.left + rcFrame.right + 1,
							rcClient.top + rcFrame.bottom + 1, iRad, iRad);
						SelectClipRgn(dis->hDC, clipRgn);
					}
				}

				if (dat->hwndPanelPic) {
					/*
					 * paint avatar using ACC
					 */
					SendMessage(dat->hwndPanelPic, AVATAR_SETAEROCOMPATDRAWING, 0, bAero ? TRUE : FALSE);
					SetWindowPos(dat->hwndPanelPic, HWND_TOP, rcFrame.left + border_off, rcFrame.top + border_off,
						rb.max_width, rb.max_height, SWP_SHOWWINDOW | SWP_ASYNCWINDOWPOS | SWP_DEFERERASE | SWP_NOSENDCHANGING);
				}
				else
					GdiAlphaBlend(dis->hDC, rcClient.left + rcFrame.left + border_off, rcClient.top + rcFrame.top + border_off,
						rb.max_width, rb.max_height, hdcMem, 0, 0,
						rb.max_width, rb.max_height, CSkin::m_default_bf);

				SelectObject(hdcMem, hbmMem);
				DeleteDC(hdcMem);
				if (hbmNew != hbmAvatar)
					DeleteObject(hbmNew);
			}
			else {
				hbmMem = (HBITMAP)SelectObject(hdcMem, hbmAvatar);
				LONG xy_off = 1; //CSkin::m_bAvatarBorderType ? 1 : 0;
				LONG width_off = 0; //= CSkin::m_bAvatarBorderType ? 0 : 2;

				SetStretchBltMode(hdcDraw, HALFTONE);
				if (aceFlags & AVS_PREMULTIPLIED) {
					HDC hdcTemp = CreateCompatibleDC(hdcDraw);
					HBITMAP hbmTemp = CreateCompatibleBitmap(hdcMem, bminfo.bmWidth, bminfo.bmHeight);
					HBITMAP hbmOld = (HBITMAP)SelectObject(hdcTemp, hbmTemp);

					SetStretchBltMode(hdcTemp, HALFTONE);
					StretchBlt(hdcTemp, 0, 0, bminfo.bmWidth, bminfo.bmHeight, hdcDraw, xy_off, top + xy_off, (int)dNewWidth + width_off, iMaxHeight + width_off, SRCCOPY);
					GdiAlphaBlend(hdcTemp, 0, 0, bminfo.bmWidth, bminfo.bmHeight, hdcMem, 0, 0, bminfo.bmWidth, bminfo.bmHeight, CSkin::m_default_bf);
					StretchBlt(hdcDraw, xy_off, top + xy_off, (int)dNewWidth + width_off, iMaxHeight + width_off, hdcTemp, 0, 0, bminfo.bmWidth, bminfo.bmHeight, SRCCOPY);
					SelectObject(hdcTemp, hbmOld);
					DeleteObject(hbmTemp);
					DeleteDC(hdcTemp);
				}
				else StretchBlt(hdcDraw, xy_off, top + xy_off, (int)dNewWidth + width_off, iMaxHeight + width_off, hdcMem, 0, 0, bminfo.bmWidth, bminfo.bmHeight, SRCCOPY);

				SelectObject(hdcMem, hbmMem);
				DeleteObject(hbmMem);
				DeleteDC(hdcMem);
			}
			if (clipRgn) {
				HBRUSH hbr = CreateSolidBrush(CSkin::m_avatarBorderClr);
				if (bPanelPic)
					FrameRgn(dis->hDC, clipRgn, hbr, 1, 1);
				else
					FrameRgn(hdcDraw, clipRgn, hbr, 1, 1);
				DeleteObject(hbr);
				DeleteObject(clipRgn);
			}
		}
		SelectObject(hdcDraw, hPenOld);
		SelectObject(hdcDraw, hOldBrush);
		DeleteObject(hPenBorder);
		if (!bPanelPic)
			BitBlt(dis->hDC, 0, 0, cx, cy, hdcDraw, 0, 0, SRCCOPY);
		SelectObject(hdcDraw, hbmOld);
		DeleteObject(hbmDraw);
		DeleteDC(hdcDraw);
		return TRUE;
	}
	
	if (dis->hwndItem == GetDlgItem(hwndDlg, IDC_STATICTEXT) || dis->hwndItem == GetDlgItem(hwndDlg, IDC_LOGFROZENTEXT)) {
		TCHAR szWindowText[256];
		if (CSkin::m_skinEnabled) {
			SetTextColor(dis->hDC, CSkin::m_DefaultFontColor);
			CSkin::SkinDrawBG(dis->hwndItem, dat->pContainer->hwnd, dat->pContainer, &dis->rcItem, dis->hDC);
		}
		else {
			SetTextColor(dis->hDC, GetSysColor(COLOR_BTNTEXT));
			CSkin::FillBack(dis->hDC, &dis->rcItem);
		}
		GetWindowText(dis->hwndItem, szWindowText, 255);
		szWindowText[255] = 0;
		SetBkMode(dis->hDC, TRANSPARENT);
		DrawText(dis->hDC, szWindowText, -1, &dis->rcItem, DT_SINGLELINE | DT_VCENTER | DT_NOCLIP | DT_END_ELLIPSIS);
		return TRUE;
	}
	
	if (dis->hwndItem == GetDlgItem(hwndDlg, IDC_STATICERRORICON)) {
		if (CSkin::m_skinEnabled)
			CSkin::SkinDrawBG(dis->hwndItem, dat->pContainer->hwnd, dat->pContainer, &dis->rcItem, dis->hDC);
		else
			CSkin::FillBack(dis->hDC, &dis->rcItem);
		DrawIconEx(dis->hDC, (dis->rcItem.right - dis->rcItem.left) / 2 - 8, (dis->rcItem.bottom - dis->rcItem.top) / 2 - 8,
			PluginConfig.g_iconErr, 16, 16, 0, 0, DI_NORMAL);
		return TRUE;
	}
	
	if (dis->CtlType == ODT_MENU && dat->Panel->isHovered()) {
		DrawMenuItem(dis, (HICON)dis->itemData, 0);
		return TRUE;
	}
	
	return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
}

void TSAPI LoadThemeDefaults(TContainerData *pContainer)
{
	ZeroMemory(&pContainer->theme, sizeof(TLogTheme));
	pContainer->theme.bg = M.GetDword(FONTMODULE, SRMSGSET_BKGCOLOUR, GetSysColor(COLOR_WINDOW));
	pContainer->theme.statbg = PluginConfig.crStatus;
	pContainer->theme.oldinbg = PluginConfig.crOldIncoming;
	pContainer->theme.oldoutbg = PluginConfig.crOldOutgoing;
	pContainer->theme.inbg = PluginConfig.crIncoming;
	pContainer->theme.outbg = PluginConfig.crOutgoing;
	pContainer->theme.hgrid = M.GetDword(FONTMODULE, "hgrid", RGB(224, 224, 224));
	pContainer->theme.left_indent = M.GetDword("IndentAmount", 20) * 15;
	pContainer->theme.right_indent = M.GetDword("RightIndent", 20) * 15;
	pContainer->theme.inputbg = M.GetDword(FONTMODULE, "inputbg", SRMSGDEFSET_BKGCOLOUR);

	for (int i = 1; i <= 5; i++) {
		char szTemp[40];
		mir_snprintf(szTemp, SIZEOF(szTemp), "cc%d", i);
		COLORREF	colour = M.GetDword(szTemp, RGB(224, 224, 224));
		if (colour == 0)
			colour = RGB(1, 1, 1);
		pContainer->theme.custom_colors[i - 1] = colour;
	}
	pContainer->theme.logFonts = logfonts;
	pContainer->theme.fontColors = fontcolors;
	pContainer->theme.rtfFonts = NULL;
	pContainer->ltr_templates = &LTR_Active;
	pContainer->rtl_templates = &RTL_Active;
	pContainer->theme.dwFlags = (M.GetDword("mwflags", MWF_LOG_DEFAULT) & MWF_LOG_ALL);
	pContainer->theme.isPrivate = false;
}

void TSAPI LoadOverrideTheme(TContainerData *pContainer)
{
	BOOL bReadTemplates = TRUE;

	ZeroMemory(&pContainer->theme, sizeof(TLogTheme));
	if (lstrlen(pContainer->szAbsThemeFile) > 1) {
		if (PathFileExists(pContainer->szAbsThemeFile)) {
			if (CheckThemeVersion(pContainer->szAbsThemeFile) == 0) {
				LoadThemeDefaults(pContainer);
				return;
			}
			if (pContainer->ltr_templates == NULL) {
				pContainer->ltr_templates = (TTemplateSet *)mir_alloc(sizeof(TTemplateSet));
				CopyMemory(pContainer->ltr_templates, &LTR_Active, sizeof(TTemplateSet));
			}
			if (pContainer->rtl_templates == NULL) {
				pContainer->rtl_templates = (TTemplateSet *)mir_alloc(sizeof(TTemplateSet));
				CopyMemory(pContainer->rtl_templates, &RTL_Active, sizeof(TTemplateSet));
			}

			pContainer->theme.logFonts = (LOGFONTA *)mir_alloc(sizeof(LOGFONTA) * (MSGDLGFONTCOUNT + 2));
			pContainer->theme.fontColors = (COLORREF *)mir_alloc(sizeof(COLORREF) * (MSGDLGFONTCOUNT + 2));
			pContainer->theme.rtfFonts = (char *)mir_alloc((MSGDLGFONTCOUNT + 2) * RTFCACHELINESIZE);

			ReadThemeFromINI(pContainer->szAbsThemeFile, pContainer, bReadTemplates ? 0 : 1, THEME_READ_ALL);
			pContainer->theme.left_indent *= 15;
			pContainer->theme.right_indent *= 15;
			pContainer->theme.isPrivate = true;
			if (CSkin::m_skinEnabled)
				pContainer->theme.bg = SkinItems[ID_EXTBKCONTAINER].COLOR;
			else
				pContainer->theme.bg = PluginConfig.m_fillColor ? PluginConfig.m_fillColor : GetSysColor(COLOR_WINDOW);
			return;
		}
	}
	LoadThemeDefaults(pContainer);
}

void TSAPI ConfigureSmileyButton(TWindowData *dat)
{
	HWND  hwndDlg = dat->hwnd;
	int nrSmileys = 0;
	int showToolbar = dat->pContainer->dwFlags & CNT_HIDETOOLBAR ? 0 : 1;
	int iItemID = IDC_SMILEYBTN;

	if (PluginConfig.g_SmileyAddAvail) {
		nrSmileys = CheckValidSmileyPack(dat->cache->getActiveProto(), dat->cache->getActiveContact());
		dat->doSmileys = 1;
	}

	if (nrSmileys == 0 || dat->hContact == 0)
		dat->doSmileys = 0;

	Utils::showDlgControl(hwndDlg, iItemID, (dat->doSmileys && showToolbar) ? SW_SHOW : SW_HIDE);
	Utils::enableDlgControl(hwndDlg, iItemID, dat->doSmileys ? TRUE : FALSE);
}

HICON TSAPI GetXStatusIcon(const TWindowData *dat)
{
	BYTE xStatus = dat->cache->getXStatusId();
	if (xStatus == 0)
		return NULL;

	if (!ProtoServiceExists(dat->cache->getActiveProto(), PS_GETCUSTOMSTATUSICON))
		return NULL;

	return (HICON)(CallProtoService(dat->cache->getActiveProto(), PS_GETCUSTOMSTATUSICON, xStatus, 0));
}

LRESULT TSAPI GetSendButtonState(HWND hwnd)
{
	HWND hwndIDok = GetDlgItem(hwnd, IDOK);

	if (hwndIDok)
		return(SendMessage(hwndIDok, BUTTONGETSTATEID, TRUE, 0));
	else
		return 0;
}

void TSAPI EnableSendButton(const TWindowData *dat, int iMode)
{
	SendMessage(GetDlgItem(dat->hwnd, IDOK), BUTTONSETASNORMAL, iMode, 0);
	SendMessage(GetDlgItem(dat->hwnd, IDC_PIC), BUTTONSETASNORMAL, dat->fEditNotesActive ? TRUE : (!iMode && dat->iOpenJobs == 0) ? TRUE : FALSE, 0);

	HWND hwndOK = GetDlgItem(GetParent(GetParent(dat->hwnd)), IDOK);

	if (IsWindow(hwndOK))
		SendMessage(hwndOK, BUTTONSETASNORMAL, iMode, 0);
}

void TSAPI SendNudge(const TWindowData *dat)
{
	if (ProtoServiceExists(dat->cache->getActiveProto(), PS_SEND_NUDGE) && ServiceExists(MS_NUDGE_SEND))
		CallService(MS_NUDGE_SEND, (WPARAM)dat->cache->getActiveContact(), 0);
	else
		SendMessage(dat->hwnd, DM_ACTIVATETOOLTIP, IDC_MESSAGE,
		(LPARAM)TranslateT("Either the nudge plugin is not installed or the contact's protocol does not support sending a nudge event."));
}

void TSAPI GetClientIcon(TWindowData *dat)
{
	if (dat->hClientIcon)
		DestroyIcon(dat->hClientIcon);

	dat->hClientIcon = 0;
	if (ServiceExists(MS_FP_GETCLIENTICONT)) {
		ptrT tszMirver(db_get_tsa(dat->cache->getActiveContact(), dat->cache->getActiveProto(), "MirVer"));
		if (tszMirver)
			dat->hClientIcon = Finger_GetClientIcon(tszMirver, 1);
	}
}

void TSAPI GetMyNick(TWindowData *dat)
{
	CONTACTINFO ci;

	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = NULL;
	ci.szProto = const_cast<char *>(dat->cache->getActiveProto());
	ci.dwFlag = CNF_TCHAR | CNF_NICK;

	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
		if (ci.type == CNFT_ASCIIZ) {
			if (lstrlen(reinterpret_cast<TCHAR *>(ci.pszVal)) < 1 || !_tcscmp(reinterpret_cast<TCHAR *>(ci.pszVal),
				TranslateT("'(Unknown Contact)'"))) {
				mir_sntprintf(dat->szMyNickname, SIZEOF(dat->szMyNickname), _T("%s"), dat->myUin[0] ? dat->myUin : TranslateT("'(Unknown Contact)'"));
				if (ci.pszVal) {
					mir_free(ci.pszVal);
					ci.pszVal = NULL;
				}
			}
			else {
				_tcsncpy(dat->szMyNickname, reinterpret_cast<TCHAR *>(ci.pszVal), 110);
				dat->szMyNickname[109] = 0;
				if (ci.pszVal) {
					mir_free(ci.pszVal);
					ci.pszVal = NULL;
				}
			}
		}
		else if (ci.type == CNFT_DWORD)
			_ltot(ci.dVal, dat->szMyNickname, 10);
		else
			_tcsncpy(dat->szMyNickname, _T("<undef>"), 110);                // that really should *never* happen
	}
	else
		_tcsncpy(dat->szMyNickname, _T("<undef>"), 110);                    // same here
	if (ci.pszVal)
		mir_free(ci.pszVal);
}

HICON TSAPI MY_GetContactIcon(const TWindowData *dat, LPCSTR szSetting)
{
	int bUseMeta = (szSetting == NULL) ? false : M.GetByte(szSetting, strcmp(szSetting, "MetaiconTab") == 0);
	if (bUseMeta)
		return LoadSkinnedProtoIcon(dat->cache->getProto(), dat->cache->getStatus());
	return LoadSkinnedProtoIcon(dat->cache->getActiveProto(), dat->cache->getActiveStatus());
}

/////////////////////////////////////////////////////////////////////////////////////////
// read keyboard state and return the state of the modifier keys

void TSAPI KbdState(TWindowData *dat, BOOL& isShift, BOOL& isControl, BOOL& isAlt)
{
	GetKeyboardState(dat->kstate);
	isShift = (dat->kstate[VK_SHIFT] & 0x80);
	isControl = (dat->kstate[VK_CONTROL] & 0x80);
	isAlt = (dat->kstate[VK_MENU] & 0x80);
}

/////////////////////////////////////////////////////////////////////////////////////////
// clear the message log
// code needs to distuingish between IM and MUC sessions.

void TSAPI ClearLog(TWindowData *dat)
{
	if (dat && dat->bType == SESSIONTYPE_IM) {
		if (dat->hwndIEView || dat->hwndHPP) {
			IEVIEWEVENT event;
			event.cbSize = sizeof(IEVIEWEVENT);
			event.iType = IEE_CLEAR_LOG;
			event.dwFlags = (dat->dwFlags & MWF_LOG_RTL) ? IEEF_RTL : 0;
			event.hContact = dat->hContact;
			if (dat->hwndIEView) {
				event.hwnd = dat->hwndIEView;
				CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&event);
			}
			else {
				event.hwnd = dat->hwndHPP;
				CallService(MS_HPP_EG_EVENT, 0, (LPARAM)&event);
			}
		}
		SetDlgItemText(dat->hwnd, IDC_LOG, _T(""));
		dat->hDbEventFirst = NULL;
	}
	else if (dat && dat->bType == SESSIONTYPE_CHAT && dat->si) {
		SESSION_INFO *si = dat->si;
		SESSION_INFO* s = pci->SM_FindSession(si->ptszID, si->pszModule);
		if (s) {
			SetDlgItemText(dat->hwnd, IDC_CHAT_LOG, _T(""));
			pci->LM_RemoveAll(&s->pLog, &s->pLogEnd);
			s->iEventCount = 0;
			s->LastTime = 0;
			si->iEventCount = 0;
			si->LastTime = 0;
			si->pLog = s->pLog;
			si->pLogEnd = s->pLogEnd;
			PostMessage(dat->hwnd, WM_MOUSEACTIVATE, 0, 0);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// calculate the minimum required client height for the given message
// window layout
//
// the container will use this in its WM_GETMINMAXINFO handler to set
// minimum tracking height.

void TSAPI DetermineMinHeight(TWindowData *dat)
{
	if (!dat)
		return;

	RECT rc;
	LONG height = (dat->Panel->isActive() ? dat->Panel->getHeight() + 2 : 0);
	if (!(dat->pContainer->dwFlags & CNT_HIDETOOLBAR))
		height += DPISCALEY_S(24); // toolbar
	GetClientRect(GetDlgItem(dat->hwnd, dat->bType == SESSIONTYPE_IM ? IDC_MESSAGE : IDC_CHAT_MESSAGE), &rc);
	height += rc.bottom; // input area
	height += 40; // min space for log area and some padding

	dat->pContainer->uChildMinHeight = height;
}

bool TSAPI IsAutoSplitEnabled(const TWindowData *dat)
{
	return((dat && (dat->pContainer->dwFlags & CNT_AUTOSPLITTER) && !(dat->dwFlagsEx & MWF_SHOW_SPLITTEROVERRIDE)) ? true : false);
}

LONG TSAPI GetDefaultMinimumInputHeight(const TWindowData *dat)
{
	LONG height = MINSPLITTERY;

	if (dat) {
		if (dat->bType == SESSIONTYPE_IM)
			height = ((dat->pContainer->dwFlags & CNT_BOTTOMTOOLBAR) ? DPISCALEY_S(46 + 22) : DPISCALEY_S(46));
		else
			height = ((dat->pContainer->dwFlags & CNT_BOTTOMTOOLBAR) ? DPISCALEY_S(22 + 46) : DPISCALEY_S(46)) - DPISCALEY_S(23);

		if (CSkin::m_skinEnabled && !SkinItems[ID_EXTBKINPUTAREA].IGNORED)
			height += (SkinItems[ID_EXTBKINPUTAREA].MARGIN_BOTTOM + SkinItems[ID_EXTBKINPUTAREA].MARGIN_TOP - 2);
	}
	return(height);
}

static LIST<TCHAR> vTempFilenames(5);

// send a pasted bitmap by file transfer.

void TSAPI SendHBitmapAsFile(const TWindowData *dat, HBITMAP hbmp)
{
	const wchar_t* 	mirandatempdir = L"Miranda";
	const wchar_t* 	filenametemplate = L"\\clp-%Y%m%d-%H%M%S0.jpg";
	TCHAR 			filename[MAX_PATH];
	size_t 			tempdirlen = GetTempPath(MAX_PATH, filename);
	bool			fSend = true;

	const char *szProto = dat->cache->getActiveProto();
	WORD  wMyStatus = (WORD)CallProtoService(szProto, PS_GETSTATUS, 0, 0);

	DWORD protoCaps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0);
	DWORD typeCaps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0);

	// check protocol capabilities, status modes and visibility lists (privacy)
	// to determine whether the file can be sent. Throw a warning if any of
	// these checks fails.
	if (!(protoCaps & PF1_FILESEND))
		fSend = false;

	if ((ID_STATUS_OFFLINE == wMyStatus) || (ID_STATUS_OFFLINE == dat->cache->getActiveStatus() && !(typeCaps & PF4_OFFLINEFILES)))
		fSend = false;

	if (protoCaps & PF1_VISLIST && db_get_w(dat->cache->getActiveContact(), szProto, "ApparentMode", 0) == ID_STATUS_OFFLINE)
		fSend = false;

	if (protoCaps & PF1_INVISLIST && wMyStatus == ID_STATUS_INVISIBLE && db_get_w(dat->cache->getActiveContact(), szProto, "ApparentMode", 0) != ID_STATUS_ONLINE)
		fSend = false;

	if (!fSend) {
		CWarning::show(CWarning::WARN_SENDFILE, MB_OK | MB_ICONEXCLAMATION | CWarning::CWF_NOALLOWHIDE);
		return;
	}

	if (tempdirlen <= 0 || tempdirlen >= MAX_PATH - _tcslen(mirandatempdir) - _tcslen(filenametemplate) - 2) // -2 is because %Y takes 4 symbols
		filename[0] = 0;					// prompt for a new name
	else {
		_tcscpy(filename + tempdirlen, mirandatempdir);
		if ((GetFileAttributes(filename) == INVALID_FILE_ATTRIBUTES || ((GetFileAttributes(filename) & FILE_ATTRIBUTE_DIRECTORY) == 0)) && CreateDirectory(filename, NULL) == 0)
			filename[0] = 0;
		else {
			tempdirlen = _tcslen(filename);

			time_t rawtime;
			time(&rawtime);
			const tm* timeinfo;
			timeinfo = _localtime32((__time32_t *)&rawtime);
			_tcsftime(filename + tempdirlen, MAX_PATH - tempdirlen, filenametemplate, timeinfo);
			size_t firstnumberpos = tempdirlen + 14;
			size_t lastnumberpos = tempdirlen + 20;
			while (GetFileAttributes(filename) != INVALID_FILE_ATTRIBUTES) {	// while it exists
				for (size_t pos = lastnumberpos; pos >= firstnumberpos; pos--)
					if (filename[pos]++ != '9')
						break;
					else
						if (pos == firstnumberpos)
							filename[0] = 0;	// all filenames exist => prompt for a new name
						else
							filename[pos] = '0';
			}
		}
	}

	if (filename[0] == 0) {	// prompting to save
		TCHAR filter[MAX_PATH];
		mir_sntprintf(filter, SIZEOF(filter), _T("%s%c*.jpg%c%c"), TranslateT("JPEG-compressed images"), 0, 0, 0);

		OPENFILENAME dlg;
		dlg.lStructSize = sizeof(dlg);
		dlg.lpstrFilter = filter;
		dlg.nFilterIndex = 1;
		dlg.lpstrFile = filename;
		dlg.nMaxFile = MAX_PATH;
		dlg.Flags = OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
		dlg.lpstrDefExt = _T("jpg");
		if (!GetSaveFileName(&dlg))
			return;
	}

	IMGSRVC_INFO ii;
	ii.cbSize = sizeof(ii);
	ii.hbm = hbmp;
	ii.wszName = filename;
	ii.dwMask = IMGI_HBITMAP;
	ii.fif = FIF_JPEG;
	CallService(MS_IMG_SAVE, (WPARAM)&ii, IMGL_TCHAR);

	int fileCount = 1, totalCount = 0;
	TCHAR** ppFiles = NULL;
	Utils::AddToFileList(&ppFiles, &totalCount, filename);

	wchar_t* _t = mir_tstrdup(filename);
	vTempFilenames.insert(_t);

	CallService(MS_FILE_SENDSPECIFICFILEST, (WPARAM)dat->cache->getActiveContact(), (LPARAM)ppFiles);

	mir_free(ppFiles[0]);
	mir_free(ppFiles);
}

// remove all temporary files created by the "send clipboard as file" feature.

void TSAPI CleanTempFiles()
{
	for (int i = 0; i < vTempFilenames.getCount(); i++) {
		wchar_t* _t = vTempFilenames[i];
		DeleteFileW(_t);
		mir_free(_t);
	}
}
