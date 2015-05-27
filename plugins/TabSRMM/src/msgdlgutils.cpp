/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-15 Miranda NG project,
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
// Helper functions for the message dialog.

#include "commonheaders.h"

#ifndef SHVIEW_THUMBNAIL
#define SHVIEW_THUMBNAIL 0x702D
#endif

#define EVENTTYPE_WAT_ANSWER            9602
#define EVENTTYPE_JABBER_CHATSTATES     2000
#define EVENTTYPE_JABBER_PRESENCE       2001

static int g_status_events[] = {
	EVENTTYPE_STATUSCHANGE,
	EVENTTYPE_WAT_ANSWER,
	EVENTTYPE_JABBER_CHATSTATES,
	EVENTTYPE_JABBER_PRESENCE
};

static int g_status_events_size = 0;

bool TSAPI IsStatusEvent(int eventType)
{
	if (g_status_events_size == 0)
		g_status_events_size = SIZEOF(g_status_events);

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
	item.cchTextMax = SIZEOF(oldText);
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

static UINT_PTR CALLBACK OpenFileSubclass(HWND hwnd, UINT msg, WPARAM, LPARAM lParam)
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
		mir_sntprintf(szBaseName, SIZEOF(szBaseName), _T("My Avatar_%s"), szTimestamp);
	else
		mir_sntprintf(szBaseName, SIZEOF(szBaseName), _T("%s_%s"), dat->cache->getNick(), szTimestamp);

	mir_sntprintf(szFinalFilename, SIZEOF(szFinalFilename), _T("%s.png"), szBaseName);

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
	if (dat->dwFlags & MWF_WASBACKGROUNDCREATE || dat->pContainer->dwFlags & CNT_DEFERREDCONFIGURE || dat->pContainer->dwFlags & CNT_CREATE_MINIMIZED || IsIconic(dat->pContainer->hwnd))
		return;  // at this stage, the layout is not yet ready...

	RECT rc;
	GetClientRect(dat->hwnd, &rc);

	BOOL bBottomToolBar = dat->pContainer->dwFlags & CNT_BOTTOMTOOLBAR;
	BOOL bToolBar = dat->pContainer->dwFlags & CNT_HIDETOOLBAR ? 0 : 1;
	int  iSplitOffset = dat->bIsAutosizingInput ? 1 : 0;

	double picAspect = (bminfo->bmWidth == 0 || bminfo->bmHeight == 0) ? 1.0 : (double)(bminfo->bmWidth / (double)bminfo->bmHeight);
	double picProjectedWidth = (double)((dat->dynaSplitter - ((bBottomToolBar && bToolBar) ? DPISCALEX_S(24) : 0) + ((dat->bShowUIElements) ? DPISCALEX_S(28) : DPISCALEX_S(2)))) * picAspect;

	if ((rc.right - (int)picProjectedWidth) > (dat->iButtonBarReallyNeeds) && !PluginConfig.m_bAlwaysFullToolbarWidth && bToolBar)
		dat->iRealAvatarHeight = dat->dynaSplitter + 3 + (dat->bShowUIElements ? DPISCALEY_S(28) : DPISCALEY_S(2));
	else
		dat->iRealAvatarHeight = dat->dynaSplitter + DPISCALEY_S(6) + DPISCALEY_S(iSplitOffset);

	dat->iRealAvatarHeight -= ((bBottomToolBar&&bToolBar) ? DPISCALEY_S(22) : 0);

	if (PluginConfig.m_LimitStaticAvatarHeight > 0)
		dat->iRealAvatarHeight = min(dat->iRealAvatarHeight, PluginConfig.m_LimitStaticAvatarHeight);

	if (M.GetByte(dat->hContact, "dontscaleavatars", M.GetByte("dontscaleavatars", 0)))
		dat->iRealAvatarHeight = min(bminfo->bmHeight, dat->iRealAvatarHeight);

	double aspect = (bminfo->bmHeight != 0) ? (double)dat->iRealAvatarHeight / (double)bminfo->bmHeight : 1.0;
	double newWidth = (double)bminfo->bmWidth * aspect;
	if (newWidth > (double)(rc.right) * 0.8)
		newWidth = (double)(rc.right) * 0.8;
	dat->pic.cy = dat->iRealAvatarHeight + 2;
	dat->pic.cx = (int)newWidth + 2;
}

int TSAPI MsgWindowUpdateMenu(TWindowData *dat, HMENU submenu, int menuID)
{
	bool bInfoPanel = dat->Panel->isActive();

	if (menuID == MENU_TABCONTEXT) {
		EnableMenuItem(submenu, ID_TABMENU_LEAVECHATROOM, (dat->bType == SESSIONTYPE_CHAT && ProtoServiceExists(dat->szProto, PS_LEAVECHAT)) ? MF_ENABLED : MF_DISABLED);
		EnableMenuItem(submenu, ID_TABMENU_ATTACHTOCONTAINER, (M.GetByte("useclistgroups", 0) || M.GetByte("singlewinmode", 0)) ? MF_GRAYED : MF_ENABLED);
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

		CheckMenuItem(submenu, ID_PICMENU_ALWAYSKEEPTHEBUTTONBARATFULLWIDTH, MF_BYCOMMAND | (PluginConfig.m_bAlwaysFullToolbarWidth ? MF_CHECKED : MF_UNCHECKED));
		if (!bInfoPanel) {
			EnableMenuItem(submenu, ID_PICMENU_SETTINGS, MF_BYCOMMAND | (ServiceExists(MS_AV_GETAVATARBITMAP) ? MF_ENABLED : MF_GRAYED));
			szText = TranslateT("Contact picture settings...");
			EnableMenuItem(submenu, 0, MF_BYPOSITION | MF_ENABLED);
		}
		else {
			EnableMenuItem(submenu, 0, MF_BYPOSITION | MF_GRAYED);
			EnableMenuItem(submenu, ID_PICMENU_SETTINGS, MF_BYCOMMAND | ((ServiceExists(MS_AV_SETMYAVATAR) && CallService(MS_AV_CANSETMYAVATAR, (WPARAM)(dat->cache->getActiveProto()), 0)) ? MF_ENABLED : MF_GRAYED));
			szText = TranslateT("Set your avatar...");
		}
		mii.dwTypeData = szText;
		mii.cch = (int)mir_tstrlen(szText) + 1;
		SetMenuItemInfo(submenu, ID_PICMENU_SETTINGS, FALSE, &mii);
	}
	else if (menuID == MENU_PANELPICMENU) {
		HMENU visMenu = GetSubMenu(submenu, 0);
		char  avOverride = (char)M.GetByte(dat->hContact, "hideavatar", -1);

		CheckMenuItem(visMenu, ID_VISIBILITY_DEFAULT, MF_BYCOMMAND | (avOverride == -1 ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(visMenu, ID_VISIBILITY_HIDDENFORTHISCONTACT, MF_BYCOMMAND | (avOverride == 0 ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(visMenu, ID_VISIBILITY_VISIBLEFORTHISCONTACT, MF_BYCOMMAND | (avOverride == 1 ? MF_CHECKED : MF_UNCHECKED));

		EnableMenuItem(submenu, ID_PICMENU_SETTINGS, MF_BYCOMMAND | (ServiceExists(MS_AV_GETAVATARBITMAP) ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(submenu, ID_PANELPICMENU_SAVETHISPICTUREAS, MF_BYCOMMAND | ((dat->ace && dat->ace->hbmPic && dat->ace->hbmPic != PluginConfig.g_hbmUnknown) ? MF_ENABLED : MF_GRAYED));
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
				if (si != NULL && dat->hContact != NULL) {
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
			else if (selection == ID_VISIBILITY_VISIBLEFORTHISCONTACT)
				avOverrideMode = 1;
			else
				avOverrideMode = 0;
			db_set_b(dat->hContact, SRMSGMOD_T, "hideavatar", avOverrideMode);
		}

		ShowPicture(dat, FALSE);
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		DM_ScrollToBottom(dat, 0, 1);
		return 1;

		case ID_PICMENU_ALWAYSKEEPTHEBUTTONBARATFULLWIDTH:
			PluginConfig.m_bAlwaysFullToolbarWidth = !PluginConfig.m_bAlwaysFullToolbarWidth;
			db_set_b(0, SRMSGMOD_T, "alwaysfulltoolbar", (BYTE)PluginConfig.m_bAlwaysFullToolbarWidth);
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
		switch (selection) {
		case ID_MESSAGELOGSETTINGS_GLOBAL:
		{
			OPENOPTIONSDIALOG ood = { sizeof(ood) };
			ood.pszPage = "Message sessions";
			ood.pszGroup = NULL;
			ood.pszTab = "Message log";
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
			mir_tstrcat(szBuf, _T("O"));
		if (fCaps)
			mir_tstrcat(szBuf, _T("C"));
		if (fNum)
			mir_tstrcat(szBuf, _T("N"));
		if (dat->fInsertMode || fCaps || fNum)
			mir_tstrcat(szBuf, _T(" | "));

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
// catches notifications from the AVS controls

void TSAPI ProcessAvatarChange(HWND hwnd, LPARAM lParam)
{
	if (((LPNMHDR)lParam)->code == NM_AVATAR_CHANGED) {
		HWND hwndDlg = GetParent(hwnd);
		TWindowData *dat = (TWindowData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (!dat)
			return;

		dat->ace = Utils::loadAvatarFromAVS(dat->cache->getActiveContact());

		GetAvatarVisibility(hwndDlg, dat);
		ShowPicture(dat, TRUE);
		if (dat->Panel->isActive())
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// retrieve the visiblity of the avatar window, depending on the global setting
// and local mode

bool TSAPI GetAvatarVisibility(HWND hwndDlg, TWindowData *dat)
{
	BYTE bAvatarMode = dat->pContainer->avatarMode;
	BYTE bOwnAvatarMode = dat->pContainer->ownAvatarMode;
	char hideOverride = (char)M.GetByte(dat->hContact, "hideavatar", -1);

	// infopanel visible, consider own avatar display
	dat->bShowAvatar = false;

	if (dat->Panel->isActive() && bAvatarMode != 3) {
		if (!bOwnAvatarMode) {
			dat->bShowAvatar = (dat->hOwnPic && dat->hOwnPic != PluginConfig.g_hbmUnknown);
			if (!dat->hwndContactPic)
				dat->hwndContactPic = CreateWindowEx(WS_EX_TOPMOST, AVATAR_CONTROL_CLASS, _T(""), WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, GetDlgItem(hwndDlg, IDC_CONTACTPIC), (HMENU)0, NULL, NULL);
		}

		switch (bAvatarMode) {
		case 2:
			dat->bShowInfoAvatar = false;
			break;
		case 0:
			dat->bShowInfoAvatar = true;
		case 1:
			HBITMAP hbm = ((dat->ace && !(dat->ace->dwFlags & AVS_HIDEONCLIST)) ? dat->ace->hbmPic : 0);
			if (hbm == NULL && !bAvatarMode) {
				dat->bShowInfoAvatar = false;
				break;
			}

			if (!dat->hwndPanelPic) {
				dat->hwndPanelPic = CreateWindowEx(WS_EX_TOPMOST, AVATAR_CONTROL_CLASS, _T(""), WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, dat->hwndPanelPicParent, (HMENU)7000, NULL, NULL);
				if (dat->hwndPanelPic)
					SendMessage(dat->hwndPanelPic, AVATAR_SETAEROCOMPATDRAWING, 0, TRUE);
			}

			if (bAvatarMode != 0)
				dat->bShowInfoAvatar = (hbm && hbm != PluginConfig.g_hbmUnknown);
			break;
		}

		if (dat->bShowInfoAvatar)
			dat->bShowInfoAvatar = hideOverride == 0 ? false : dat->bShowInfoAvatar;
		else
			dat->bShowInfoAvatar = hideOverride == 1 ? true : dat->bShowInfoAvatar;

		Utils::setAvatarContact(dat->hwndPanelPic, dat->hContact);
		SendMessage(dat->hwndContactPic, AVATAR_SETPROTOCOL, 0, (LPARAM)dat->cache->getActiveProto());
	}
	else {
		dat->bShowInfoAvatar = false;

		switch (bAvatarMode) {
		case 0: // globally on
			dat->bShowAvatar = true;
		LBL_Check:
			if (!dat->hwndContactPic)
				dat->hwndContactPic = CreateWindowEx(WS_EX_TOPMOST, AVATAR_CONTROL_CLASS, _T(""), WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, GetDlgItem(hwndDlg, IDC_CONTACTPIC), (HMENU)0, NULL, NULL);
			break;
		case 2: // globally OFF
			dat->bShowAvatar = false;
			break;
		case 3: // on, if present
		case 1:
			HBITMAP hbm = (dat->ace && !(dat->ace->dwFlags & AVS_HIDEONCLIST)) ? dat->ace->hbmPic : 0;
			dat->bShowAvatar = (hbm && hbm != PluginConfig.g_hbmUnknown);
			goto LBL_Check;
		}

		if (dat->bShowAvatar)
			dat->bShowAvatar = hideOverride == 0 ? 0 : dat->bShowAvatar;
		else
			dat->bShowAvatar = hideOverride == 1 ? 1 : dat->bShowAvatar;

		// reloads avatars
		if (dat->hwndPanelPic) { // shows contact or user picture, depending on panel visibility
			SendMessage(dat->hwndContactPic, AVATAR_SETPROTOCOL, 0, (LPARAM)dat->cache->getActiveProto());
			Utils::setAvatarContact(dat->hwndPanelPic, dat->hContact);
		}
		else Utils::setAvatarContact(dat->hwndContactPic, dat->hContact);
	}
	return dat->bShowAvatar;
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

TCHAR* TSAPI QuoteText(const TCHAR *text)
{
	int outChar, lineChar;
	int iCharsPerLine = M.GetDword("quoteLineLength", 64);

	size_t bufSize = mir_wstrlen(text) + 23;
	TCHAR *strout = (TCHAR*)mir_alloc(bufSize * sizeof(TCHAR));
	int inChar = 0;
	int justDoneLineBreak = 1;
	for (outChar = 0, lineChar = 0; text[inChar];) {
		if (outChar >= bufSize - 8) {
			bufSize += 20;
			strout = (TCHAR*)mir_realloc(strout, bufSize * sizeof(TCHAR));
		}
		if (justDoneLineBreak && text[inChar] != '\r' && text[inChar] != '\n') {
			strout[outChar++] = '>';
			strout[outChar++] = ' ';
			lineChar = 2;
		}
		if (lineChar == iCharsPerLine && text[inChar] != '\r' && text[inChar] != '\n') {
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
	if (!dat)
		return;

	HWND hwndDlg = dat->hwnd;
	dat->bShowAvatar = GetAvatarVisibility(hwndDlg, dat);

	bool bInfoPanel = dat->Panel->isActive();
	HBITMAP hbm = (bInfoPanel && dat->pContainer->avatarMode != 3) ? dat->hOwnPic : (dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown);
	if (hbm) {
		if (dat->dynaSplitter == 0 || dat->splitterY == 0)
			LoadSplitter(dat);
		dat->dynaSplitter = dat->splitterY - DPISCALEY_S(34);
		DM_RecalcPictureSize(dat);
		Utils::showDlgControl(hwndDlg, IDC_CONTACTPIC, dat->bShowAvatar ? SW_SHOW : SW_HIDE);
		InvalidateRect(GetDlgItem(hwndDlg, IDC_CONTACTPIC), NULL, TRUE);
	}
	else {
		Utils::showDlgControl(hwndDlg, IDC_CONTACTPIC, dat->bShowAvatar ? SW_SHOW : SW_HIDE);
		dat->pic.cy = dat->pic.cx = DPISCALEY_S(60);
		InvalidateRect(GetDlgItem(hwndDlg, IDC_CONTACTPIC), NULL, TRUE);
	}
}

void TSAPI ShowPicture(TWindowData *dat, BOOL showNewPic)
{
	HWND hwndDlg = dat->hwnd;

	if (!dat->Panel->isActive())
		dat->pic.cy = dat->pic.cx = DPISCALEY_S(60);

	if (showNewPic) {
		if (dat->Panel->isActive() && dat->pContainer->avatarMode != 3) {
			if (!dat->hwndPanelPic) {
				InvalidateRect(dat->hwnd, NULL, TRUE);
				UpdateWindow(dat->hwnd);
				SendMessage(dat->hwnd, WM_SIZE, 0, 0);
			}
			return;
		}
		AdjustBottomAvatarDisplay(dat);
	}
	else {
		dat->bShowAvatar = dat->bShowAvatar ? 0 : 1;
		db_set_b(dat->hContact, SRMSGMOD_T, "MOD_ShowPic", (BYTE)dat->bShowAvatar);
	}

	RECT rc;
	GetWindowRect(GetDlgItem(hwndDlg, IDC_CONTACTPIC), &rc);
	if (dat->minEditBoxSize.cy + DPISCALEY_S(3) > dat->splitterY)
		SendMessage(hwndDlg, DM_SPLITTERMOVED, (WPARAM)rc.bottom - dat->minEditBoxSize.cy, (LPARAM)GetDlgItem(hwndDlg, IDC_SPLITTER));
	if (!showNewPic)
		SetDialogToType(hwndDlg);
	else
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
}

void TSAPI FlashOnClist(HWND hwndDlg, TWindowData *dat, MEVENT hEvent, DBEVENTINFO *dbei)
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

	if (!PluginConfig.m_bFlashOnClist)
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
// retrieve contents of the richedit control by streaming.Used to get the
// typed message before sending it.
// caller must mir_free the returned pointer.
// UNICODE version returns UTF-8 encoded string.

static DWORD CALLBACK Message_StreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	static DWORD dwRead;
	char **ppText = (char **)dwCookie;

	if (*ppText == NULL) {
		*ppText = (char *)mir_alloc(cb + 2);
		memcpy(*ppText, pbBuff, cb);
		*pcb = cb;
		dwRead = cb;
		*(*ppText + cb) = '\0';
	}
	else {
		char *p = (char *)mir_realloc(*ppText, dwRead + cb + 2);
		memcpy(p + dwRead, pbBuff, cb);
		*ppText = p;
		*pcb = cb;
		dwRead += cb;
		*(*ppText + dwRead) = '\0';
	}
	return 0;
}

char* TSAPI Message_GetFromStream(HWND hwndRtf, DWORD dwPassedFlags)
{
	if (hwndRtf == 0)
		return NULL;

	DWORD dwFlags = (CP_UTF8 << 16) | SF_USECODEPAGE;
	if (dwPassedFlags == 0)
		dwFlags |= (SF_RTFNOOBJS | SFF_PLAINRTF);
	else
		dwFlags |= dwPassedFlags;

	char *pszText = NULL;
	EDITSTREAM stream = { 0 };
	stream.pfnCallback = Message_StreamCallback;
	stream.dwCookie = (DWORD_PTR)&pszText; // pass pointer to pointer
	SendMessage(hwndRtf, EM_STREAMOUT, dwFlags, (LPARAM)&stream);
	return pszText; // pszText contains the text
}

/////////////////////////////////////////////////////////////////////////////////////////
// convert rich edit code to bbcode (if wanted). Otherwise, strip all RTF formatting
// tags and return plain text

static TCHAR tszRtfBreaks[] = _T(" \\\n\r");

static void CreateColorMap(CMString &Text, int iCount, COLORREF *pSrc, int *pDst)
{
	const TCHAR *pszText = Text;
	int iIndex = 1, i = 0;

	static const TCHAR *lpszFmt = _T("\\red%[^ \x5b\\]\\green%[^ \x5b\\]\\blue%[^ \x5b;];");
	TCHAR szRed[10], szGreen[10], szBlue[10];

	const TCHAR *p1 = _tcsstr(pszText, _T("\\colortbl"));
	if (!p1)
		return;

	const TCHAR *pEnd = _tcschr(p1, '}');

	const TCHAR *p2 = _tcsstr(p1, _T("\\red"));

	for (i = 0; i < iCount; i++)
		pDst[i] = -1;

	while (p2 && p2 < pEnd) {
		if (_stscanf(p2, lpszFmt, &szRed, &szGreen, &szBlue) > 0) {
			for (int i = 0; i < iCount; i++) {
				if (pSrc[i] == RGB(_ttoi(szRed), _ttoi(szGreen), _ttoi(szBlue)))
					pDst[i] = iIndex;
			}
		}
		iIndex++;
		p1 = p2;
		p1++;

		p2 = _tcsstr(p1, _T("\\red"));
	}
}

static int GetRtfIndex(int iCol, int iCount, int *pIndex)
{
	for (int i = 0; i < iCount; i++)
		if (pIndex[i] == iCol)
			return i;

	return -1;
}

BOOL TSAPI DoRtfToTags(const TWindowData *dat, CMString &pszText, int iNumColors, COLORREF *pColors)
{
	if (pszText.IsEmpty())
		return FALSE;

	// used to filter out attributes which are already set for the default message input area font
	LOGFONTA lf = dat->pContainer->theme.logFonts[MSGFONTID_MESSAGEAREA];

	// create an index of colors in the module and map them to
	// corresponding colors in the RTF color table
	int *pIndex = (int*)_alloca(iNumColors * sizeof(int));
	CreateColorMap(pszText, iNumColors, pColors, pIndex);

	// scan the file for rtf commands and remove or parse them
	int idx = pszText.Find(_T("\\pard"));
	if (idx == -1) {
		if ((idx = pszText.Find(_T("\\ltrpar"))) == -1)
			return FALSE;
		idx += 7;
	}
	else idx += 5;

	bool bInsideColor = false, bInsideUl = false;
	CMString res;

	// iterate through all characters, if rtf control character found then take action
	for (const TCHAR *p = pszText.GetString() + idx; *p;) {
		switch (*p) {
		case '\\':
			if (p[1] == '\\' || p[1] == '{' || p[1] == '}') { // escaped characters
				res.AppendChar(p[1]);
				p += 2; break;
			}
			if (p[1] == '~') { // non-breaking space
				res.AppendChar(0xA0);
				p += 2; break;
			}
			
			if (!_tcsncmp(p, _T("\\cf"), 3)) { // foreground color
				int iCol = _ttoi(p + 3);
				int iInd = GetRtfIndex(iCol, iNumColors, pIndex);

				if (iCol && dat->bType != SESSIONTYPE_CHAT)
					res.AppendFormat((iInd > 0) ? (bInsideColor ? _T("[/color][color=%s]") : _T("[color=%s]")) : (bInsideColor ? _T("[/color]") : _T("")), Utils::rtf_ctable[iInd - 1].szName);

				bInsideColor = iInd > 0;
			}
			else if (!_tcsncmp(p, _T("\\highlight"), 10)) { //background color
				TCHAR szTemp[20];
				int iCol = _ttoi(p + 10);
				mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%d"), iCol);
			}
			else if (!_tcsncmp(p, _T("\\line"), 5)) { // soft line break;
				res.AppendChar('\n');
			}
			else if (!_tcsncmp(p, _T("\\endash"), 7)) {
				res.AppendChar(0x2013);
			}
			else if (!_tcsncmp(p, _T("\\emdash"), 7)) {
				res.AppendChar(0x2014);
			}
			else if (!_tcsncmp(p, _T("\\bullet"), 7)) {
				res.AppendChar(0x2022);
			}
			else if (!_tcsncmp(p, _T("\\ldblquote"), 10)) {
				res.AppendChar(0x201C);
			}
			else if (!_tcsncmp(p, _T("\\rdblquote"), 10)) {
				res.AppendChar(0x201D);
			}
			else if (!_tcsncmp(p, _T("\\lquote"), 7)) {
				res.AppendChar(0x2018);
			}
			else if (!_tcsncmp(p, _T("\\rquote"), 7)) {
				res.AppendChar(0x2019);
			}
			else if (!_tcsncmp(p, _T("\\b"), 2)) { //bold
				if (!(lf.lfWeight == FW_BOLD)) // only allow bold if the font itself isn't a bold one, otherwise just strip it..
					if (dat->SendFormat)
						res.Append((p[2] != '0') ? _T("[b]") : _T("[/b]"));
			}
			else if (!_tcsncmp(p, _T("\\i"), 2)) { // italics
				if (!lf.lfItalic && dat->SendFormat)
					res.Append((p[2] != '0') ? _T("[i]") : _T("[/i]"));
			}
			else if (!_tcsncmp(p, _T("\\strike"), 7)) { // strike-out
				if (!lf.lfStrikeOut && dat->SendFormat)
					res.Append((p[7] != '0') ? _T("[s]") : _T("[/s]"));
			}
			else if (!_tcsncmp(p, _T("\\ul"), 3)) { // underlined
				if (!lf.lfUnderline && dat->SendFormat) {
					if (p[3] == 0 || _tcschr(tszRtfBreaks, p[3])) {
						res.Append(_T("[u]"));
						bInsideUl = true;
					}
					else if (!_tcsnccmp(p + 3, _T("none"), 4)) {
						if (bInsideUl)
							res.Append(_T("[/u]"));
						bInsideUl = false;
					}
				}
			}
			else if (!_tcsncmp(p, _T("\\tab"), 4)) { // tab
				res.AppendChar('\t');
			}
			else if (p[1] == '\'') { // special character
				if (p[2] != ' ' && p[2] != '\\') {
					TCHAR tmp[10];

					if (p[3] != ' ' && p[3] != '\\') {
						_tcsncpy(tmp, p + 2, 3);
						tmp[3] = 0;
					}
					else {
						_tcsncpy(tmp, p + 2, 2);
						tmp[2] = 0;
					}

					// convert string containing char in hex format to int.
					TCHAR *stoppedHere;
					res.AppendChar(_tcstol(tmp, &stoppedHere, 16));
				}
			}

			p++; // skip initial slash
			p += _tcscspn(p, tszRtfBreaks);
			if (*p == ' ')
				p++;
			break;

		case '{': // other RTF control characters
		case '}':
			p++;
			break;

		default: // other text that should not be touched
			res.AppendChar(*p++);
			break;
		}
	}

	if (bInsideColor && dat->bType != SESSIONTYPE_CHAT)
		res.Append(_T("[/color]"));
	if (bInsideUl)
		res.Append(_T("[/u]"));

	pszText = res;
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
			_tcsncpy_s(dat->myUin, ci.pszVal, _TRUNCATE);
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

UINT TSAPI GetIEViewMode(MCONTACT hContact)
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

void TSAPI SetMessageLog(TWindowData *dat)
{
	HWND		 hwndDlg = dat->hwnd;
	unsigned int iLogMode = GetIEViewMode(dat->hContact);

	if (iLogMode == WANT_IEVIEW_LOG && dat->hwndIEView == 0) {
		IEVIEWWINDOW ieWindow;

		memset(&ieWindow, 0, sizeof(ieWindow));
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
		Utils::enableDlgControl(hwndDlg, IDC_LOG, false);
	}
	else if (iLogMode == WANT_HPP_LOG && dat->hwndHPP == 0) {
		IEVIEWWINDOW ieWindow;
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
		Utils::enableDlgControl(hwndDlg, IDC_LOG, false);
	}
}

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
		int i;
		MEVENT hPrevEvent;
		{
			DBEVENTINFO dbei = { sizeof(dbei) };
			// ability to load only current session's history
			if (dat->bActualHistory)
				i = dat->cache->getSessionMsgCount();
			else
				i = db_get_w(NULL, SRMSGMOD, SRMSGSET_LOADCOUNT, SRMSGDEFSET_LOADCOUNT);

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
				if (!DbEventIsShown(&dbei))
					i++;
			}
		}
		break;

	case LOADHISTORY_TIME:
		DBEVENTINFO dbei = { sizeof(dbei) };
		if (dat->hDbEventFirst == NULL)
			dbei.timestamp = time(NULL);
		else
			db_event_get(dat->hDbEventFirst, &dbei);

		DWORD firstTime = dbei.timestamp - 60 * db_get_w(NULL, SRMSGMOD, SRMSGSET_LOADTIME, SRMSGDEFSET_LOADTIME);
		for (;;) {
			MEVENT hPrevEvent;
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

void TSAPI GetSendFormat(TWindowData *dat)
{
	UINT controls[5] = { IDC_FONTBOLD, IDC_FONTITALIC, IDC_FONTUNDERLINE, IDC_FONTSTRIKEOUT, IDC_FONTFACE };

	dat->SendFormat = M.GetDword(dat->hContact, "sendformat", PluginConfig.m_SendFormat);
	if (dat->SendFormat == -1)          // per contact override to disable it..
		dat->SendFormat = 0;
	else if (dat->SendFormat == 0)
		dat->SendFormat = PluginConfig.m_SendFormat ? 1 : 0;

	for (int i = 0; i < 5; i++)
		Utils::enableDlgControl(dat->hwnd, controls[i], dat->SendFormat != 0);
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
	BYTE szTest[4] = { 0xe4, 0xf6, 0xfc, 0 };

	szLI[0] = szLI[1] = 0;

	memset(&pf2, 0, sizeof(PARAFORMAT2));
	langID = (USHORT)_tcstol(szKLName, &stopped, 16);
	dat->lcid = MAKELCID(langID, 0);
	/*
	 * Vista+: read ISO locale names from the registry
	 */
	if (PluginConfig.m_bIsVista) {
		HKEY	hKey = 0;
		TCHAR	szKey[20];
		DWORD	dwLID = _tcstoul(szKLName, &stopped, 16);

		mir_sntprintf(szKey, SIZEOF(szKey), _T("%04.04x"), LOWORD(dwLID));
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
	GetStringTypeA(dat->lcid, CT_CTYPE2, (char*)szTest, 3, wCtype2);
	pf2.cbSize = sizeof(pf2);
	pf2.dwMask = PFM_RTLPARA;
	SendDlgItemMessage(dat->hwnd, IDC_MESSAGE, EM_GETPARAFORMAT, 0, (LPARAM)&pf2);
	if (Utils::FindRTLLocale(dat) && fLocaleNotSet) {
		if (wCtype2[0] == C2_RIGHTTOLEFT || wCtype2[1] == C2_RIGHTTOLEFT || wCtype2[2] == C2_RIGHTTOLEFT) {
			memset(&pf2, 0, sizeof(pf2));
			pf2.dwMask = PFM_RTLPARA;
			pf2.cbSize = sizeof(pf2);
			pf2.wEffects = PFE_RTLPARA;
			SendDlgItemMessage(dat->hwnd, IDC_MESSAGE, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
		}
		else {
			memset(&pf2, 0, sizeof(pf2));
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

	BITMAP bm;
	if (dat->ace && dat->ace->hbmPic)
		GetObject(dat->ace->hbmPic, sizeof(bm), &bm);
	else if (dat->ace == NULL)
		GetObject(PluginConfig.g_hbmUnknown, sizeof(bm), &bm);
	else
		return;

	AdjustBottomAvatarDisplay(dat);
	CalcDynamicAvatarSize(dat, &bm);

	if (!dat->Panel->isActive() || dat->pContainer->avatarMode == 3) {
		dat->iRealAvatarHeight = 0;
		PostMessage(dat->hwnd, WM_SIZE, 0, 0);
	}
	else if (dat->Panel->isActive())
		GetAvatarVisibility(dat->hwnd, dat);
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
		SendMessage(dat->hwnd, DM_ACTIVATETOOLTIP, ctrlID, (LPARAM)TranslateT("The 'paste and send' feature is disabled. You can enable it on the 'General' options page in the 'Sending messages' section"));
		return;                                     // feature disabled
	}

	SendDlgItemMessage(dat->hwnd, ctrlID, EM_PASTESPECIAL, CF_UNICODETEXT, 0);
	if (GetWindowTextLength(GetDlgItem(dat->hwnd, ctrlID)) > 0)
		SendMessage(dat->hwnd, WM_COMMAND, IDOK, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// draw various elements of the message window, like avatar(s), info panel fields
// and the color formatting menu

int TSAPI MsgWindowDrawHandler(WPARAM wParam, LPARAM lParam, TWindowData *dat)
{
	if (!dat)
		return 0;

	HWND	hwndDlg = dat->hwnd;

	LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
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

	HBITMAP hbmAvatar = dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown;
	if ((dis->hwndItem == GetDlgItem(hwndDlg, IDC_CONTACTPIC) && dat->bShowAvatar) || (dis->hwndItem == hwndDlg && dat->Panel->isActive())) {
		if (hbmAvatar == NULL)
			return TRUE;

		int top, cx, cy;
		RECT rcClient, rcFrame;
		bool bPanelPic = dis->hwndItem == hwndDlg;
		if (bPanelPic && !dat->bShowInfoAvatar)
			return TRUE;

		RECT rc;
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

		HDC hdcDraw = CreateCompatibleDC(dis->hDC);
		HBITMAP hbmDraw = CreateCompatibleBitmap(dis->hDC, cx, cy);
		HBITMAP hbmOld = (HBITMAP)SelectObject(hdcDraw, hbmDraw);

		bool bAero = M.isAero();

		HRGN clipRgn = 0;
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcDraw, bAero ? (HBRUSH)GetStockObject(HOLLOW_BRUSH) : GetSysColorBrush(COLOR_3DFACE));
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
			else if (bAero && CSkin::m_pCurrentAeroEffect) {
				COLORREF clr = PluginConfig.m_tbBackgroundHigh ? PluginConfig.m_tbBackgroundHigh :
					(CSkin::m_pCurrentAeroEffect ? CSkin::m_pCurrentAeroEffect->m_clrToolbar : 0xf0f0f0);

				HBRUSH br = CreateSolidBrush(clr);
				FillRect(hdcDraw, &rcFrame, br);
				DeleteObject(br);
			}
			else FillRect(hdcDraw, &rcFrame, GetSysColorBrush(COLOR_3DFACE));

			HPEN hPenBorder = CreatePen(PS_SOLID, 1, CSkin::m_avatarBorderClr);
			HPEN hPenOld = (HPEN)SelectObject(hdcDraw, hPenBorder);

			if (CSkin::m_bAvatarBorderType == 1)
				Rectangle(hdcDraw, rcEdge.left, rcEdge.top, rcEdge.right, rcEdge.bottom);
			else if (CSkin::m_bAvatarBorderType == 2) {
				int iRad = PluginConfig.m_WinVerMajor >= 5 ? 4 : 6;
				clipRgn = CreateRoundRectRgn(rcEdge.left, rcEdge.top, rcEdge.right + 1, rcEdge.bottom + 1, iRad, iRad);
				SelectClipRgn(hdcDraw, clipRgn);

				HBRUSH hbr = CreateSolidBrush(CSkin::m_avatarBorderClr);
				FrameRgn(hdcDraw, clipRgn, hbr, 1, 1);
				DeleteObject(hbr);
				DeleteObject(clipRgn);
			}

			SelectObject(hdcDraw, hPenOld);
			DeleteObject(hPenBorder);
		}

		if (bPanelPic) {
			bool bBorder = (CSkin::m_bAvatarBorderType ? true : false);

			int border_off = bBorder ? 1 : 0;
			int iMaxHeight = dat->iPanelAvatarY - (bBorder ? 2 : 0);
			int iMaxWidth = dat->iPanelAvatarX - (bBorder ? 2 : 0);

			rcFrame.left = rcFrame.top = 0;
			rcFrame.right = (rcClient.right - rcClient.left);
			rcFrame.bottom = (rcClient.bottom - rcClient.top);

			rcFrame.left = rcFrame.right - (LONG)dat->iPanelAvatarX;
			rcFrame.bottom = (LONG)dat->iPanelAvatarY;

			int height_off = (cy - iMaxHeight - (bBorder ? 2 : 0)) / 2;
			rcFrame.top += height_off;
			rcFrame.bottom += height_off;

			SendMessage(dat->hwndPanelPic, AVATAR_SETAEROCOMPATDRAWING, 0, bAero ? TRUE : FALSE);
			SetWindowPos(dat->hwndPanelPic, HWND_TOP, rcFrame.left + border_off, rcFrame.top + border_off,
				iMaxWidth, iMaxHeight, SWP_SHOWWINDOW | SWP_ASYNCWINDOWPOS | SWP_DEFERERASE | SWP_NOSENDCHANGING);
		}

		SelectObject(hdcDraw, hOldBrush);
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
		GetWindowText(dis->hwndItem, szWindowText, SIZEOF(szWindowText));
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
	memset(&pContainer->theme, 0, sizeof(TLogTheme));
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
	memset(&pContainer->theme, 0, sizeof(TLogTheme));
	if (mir_tstrlen(pContainer->szAbsThemeFile) > 1) {
		if (PathFileExists(pContainer->szAbsThemeFile)) {
			if (CheckThemeVersion(pContainer->szAbsThemeFile) == 0) {
				LoadThemeDefaults(pContainer);
				return;
			}
			if (pContainer->ltr_templates == NULL) {
				pContainer->ltr_templates = (TTemplateSet *)mir_alloc(sizeof(TTemplateSet));
				memcpy(pContainer->ltr_templates, &LTR_Active, sizeof(TTemplateSet));
			}
			if (pContainer->rtl_templates == NULL) {
				pContainer->rtl_templates = (TTemplateSet *)mir_alloc(sizeof(TTemplateSet));
				memcpy(pContainer->rtl_templates, &RTL_Active, sizeof(TTemplateSet));
			}

			pContainer->theme.logFonts = (LOGFONTA *)mir_alloc(sizeof(LOGFONTA) * (MSGDLGFONTCOUNT + 2));
			pContainer->theme.fontColors = (COLORREF *)mir_alloc(sizeof(COLORREF) * (MSGDLGFONTCOUNT + 2));
			pContainer->theme.rtfFonts = (char *)mir_alloc((MSGDLGFONTCOUNT + 2) * RTFCACHELINESIZE);

			ReadThemeFromINI(pContainer->szAbsThemeFile, pContainer, 0, THEME_READ_ALL);
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
	HWND hwndDlg = dat->hwnd;
	int nrSmileys = 0;
	int showToolbar = dat->pContainer->dwFlags & CNT_HIDETOOLBAR ? 0 : 1;
	int iItemID = IDC_SMILEYBTN;

	if (PluginConfig.g_SmileyAddAvail) {
		nrSmileys = CheckValidSmileyPack(dat->cache->getActiveProto(), dat->cache->getActiveContact());
		dat->bShowSmileys = true;
	}

	if (nrSmileys == 0 || dat->hContact == 0)
		dat->bShowSmileys = false;

	Utils::showDlgControl(hwndDlg, iItemID, (dat->bShowSmileys && showToolbar) ? SW_SHOW : SW_HIDE);
	Utils::enableDlgControl(hwndDlg, iItemID, dat->bShowSmileys);
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
	SendDlgItemMessage(dat->hwnd, IDOK, BUTTONSETASNORMAL, iMode, 0);
	SendDlgItemMessage(dat->hwnd, IDC_PIC, BUTTONSETASNORMAL, dat->fEditNotesActive ? TRUE : (!iMode && dat->iOpenJobs == 0) ? TRUE : FALSE, 0);

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

	memset(&ci, 0, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = NULL;
	ci.szProto = const_cast<char *>(dat->cache->getActiveProto());
	ci.dwFlag = CNF_TCHAR | CNF_NICK;

	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
		switch (ci.type) {
		case CNFT_ASCIIZ:
			if (mir_tstrlen((TCHAR*)ci.pszVal) == 0 ||
				!mir_tstrcmp((TCHAR*)ci.pszVal, TranslateT("'(Unknown contact)'"))) {
				_tcsncpy_s(dat->szMyNickname, (dat->myUin[0] ? dat->myUin : TranslateT("'(Unknown contact)'")), _TRUNCATE);
			}
			else {
				_tcsncpy_s(dat->szMyNickname, (TCHAR*)ci.pszVal, _TRUNCATE);
			}
			break;
		case CNFT_DWORD:
			_ltot(ci.dVal, dat->szMyNickname, 10);
			break;
		default:
			_tcsncpy_s(dat->szMyNickname, _T("<undef>"), _TRUNCATE); // that really should *never* happen
			break;
		}
		mir_free(ci.pszVal);
	}
	else {
		_tcsncpy_s(dat->szMyNickname, _T("<undef>"), _TRUNCATE); // same here
	}
}

HICON TSAPI MY_GetContactIcon(const TWindowData *dat, LPCSTR szSetting)
{
	int bUseMeta = (szSetting == NULL) ? false : M.GetByte(szSetting, mir_strcmp(szSetting, "MetaiconTab") == 0);
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

	if (tempdirlen <= 0 || tempdirlen >= MAX_PATH - mir_tstrlen(mirandatempdir) - mir_tstrlen(filenametemplate) - 2) // -2 is because %Y takes 4 symbols
		filename[0] = 0;					// prompt for a new name
	else {
		mir_tstrcpy(filename + tempdirlen, mirandatempdir);
		if ((GetFileAttributes(filename) == INVALID_FILE_ATTRIBUTES || ((GetFileAttributes(filename) & FILE_ATTRIBUTE_DIRECTORY) == 0)) && CreateDirectory(filename, NULL) == 0)
			filename[0] = 0;
		else {
			tempdirlen = mir_tstrlen(filename);

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

	int totalCount = 0;
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
