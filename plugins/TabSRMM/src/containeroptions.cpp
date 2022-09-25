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
// The dialog to customize per container options

#include "stdafx.h"

static void MY_CheckDlgButton(HWND hWnd, UINT id, int iCheck)
{
	CheckDlgButton(hWnd, id, iCheck ? BST_CHECKED : BST_UNCHECKED);
}

static void ReloadGlobalContainerSettings(bool fForceReconfig)
{
	for (TContainerData *p = pFirstContainer; p; p = p->pNext) {
		if (!p->cfg.fPrivate) {
			if (fForceReconfig)
				p->Configure();
			else
				SendMessage(p->m_hwnd, WM_SIZE, 0, 1);
			p->BroadCastContainer(DM_SETINFOPANEL, 0, 0);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Apply a container setting

void TContainerData::ApplySetting(bool fForceResize)
{
	if (cfg.fPrivate)
		Configure();
	else
		ReloadGlobalContainerSettings(fForceResize);

	if (fForceResize)
		SendMessage(m_hwnd, WM_SIZE, 0, 1);

	BroadCastContainer(WM_CBD_UPDATED, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

struct
{
	const wchar_t *szTitle, *szDesc;
	UINT uIds[10];
}
static o_pages[] =
{
	{ LPGENW("General options"), nullptr, IDC_O_NOTABS, IDC_O_STICKY, IDC_VERTICALMAX, IDC_AUTOSPLITTER, IDC_O_AUTOHIDE, IDC_AUTOCLOSETABTIME, IDC_AUTOCLOSETABSPIN, IDC_O_AUTOHIDESECONDS, 0, 0 },
	{ LPGENW("Window layout"), nullptr, IDC_CNTNOSTATUSBAR, IDC_HIDEMENUBAR, IDC_UIDSTATUSBAR, IDC_HIDETOOLBAR, IDC_INFOPANEL, IDC_BOTTOMTOOLBAR, 0, 0, 0, 0 },
	{ LPGENW("Tabs and switch bar"), LPGENW("Choose your options for the tabbed user interface. Not all options can be applied to open windows. You may need to close and re-open them."), IDC_TABMODE, IDC_O_TABMODE, IDC_O_SBARLAYOUT, IDC_SBARLAYOUT, IDC_FLASHICON, IDC_FLASHLABEL, IDC_SINGLEROWTAB, IDC_BUTTONTABS, IDC_CLOSEBUTTONONTABS, 0 },
	{ LPGENW("Notifications"), LPGENW("Select, when you want to see event notifications (popups) for this window. The settings apply to all tabs within this window."), IDC_O_DONTREPORT, IDC_DONTREPORTUNFOCUSED2, IDC_DONTREPORTFOCUSED2, IDC_ALWAYSPOPUPSINACTIVE, IDC_O_EXPLAINGLOBALNOTIFY, 0, 0, 0, 0, 0 },
	{ LPGENW("Flashing"), nullptr, IDC_O_FLASHDEFAULT, IDC_O_FLASHALWAYS, IDC_O_FLASHNEVER, 0, 0, 0, 0, 0, 0, 0 },
	{ LPGENW("Title bar"), nullptr, IDC_O_HIDETITLE, IDC_TITLEFORMAT, IDC_O_TITLEBARFORMAT, IDC_O_HELP_TITLEFORMAT, 0, 0, 0, 0, 0, 0 },
	{ LPGENW("Window size and theme"), LPGENW("You can select a private theme (.tabsrmm file) for this container which will then override the default message log theme. You will have to close and re-open all message windows after changing this option."), IDC_THEME, IDC_SELECTTHEME, IDC_USEGLOBALSIZE, IDC_SAVESIZEASGLOBAL, IDC_LABEL_PRIVATETHEME, IDC_TSLABEL_EXPLAINTHEME, 0, 0, 0, 0 },
	{ LPGENW("Transparency"), LPGENW("This feature may be unavailable when using a container skin."), IDC_TRANSPARENCY, IDC_TRANSPARENCY_ACTIVE, IDC_TRANSPARENCY_INACTIVE, IDC_TSLABEL_ACTIVE, IDC_TSLABEL_INACTIVE, 0, 0, 0, 0, 0 },
	{ LPGENW("Contact avatars"), nullptr, IDC_O_STATIC_AVATAR, IDC_O_STATIC_OWNAVATAR, IDC_AVATARMODE, IDC_OWNAVATARMODE, IDC_AVATARSONTASKBAR, 0, 0, 0, 0, 0 },
	{ LPGENW("Sound notifications"), nullptr, IDC_O_ENABLESOUNDS, IDC_O_SOUNDSMINIMIZED, IDC_O_SOUNDSUNFOCUSED, IDC_O_SOUNDSINACTIVE, IDC_O_SOUNDSFOCUSED, 0, 0, 0, 0, 0 },
};

static void ShowPage(HWND hwndDlg, int iPage, BOOL fShow)
{
	if (iPage < 0 || iPage >= _countof(o_pages))
		return;

	for (int i = 0; i < _countof(o_pages[0].uIds) && o_pages[iPage].uIds[i] != 0; i++)
		Utils::showDlgControl(hwndDlg, o_pages[iPage].uIds[i], fShow ? SW_SHOW : SW_HIDE);
	
	if (fShow) {
		SetDlgItemText(hwndDlg, IDC_TITLEBOX, TranslateW(o_pages[iPage].szTitle));
		if (o_pages[iPage].szDesc != nullptr)
			SetDlgItemText(hwndDlg, IDC_DESC, TranslateW(o_pages[iPage].szDesc));
		else
			SetDlgItemText(hwndDlg, IDC_DESC, L"");
	}
	Utils::showDlgControl(hwndDlg, IDC_O_EXPLAINGLOBALNOTIFY, (iPage == 3 && NEN::bWindowCheck) ? SW_SHOW : SW_HIDE);

	if (iPage == 8 && !IsWinVer7Plus())
		Utils::showDlgControl(hwndDlg, IDC_AVATARSONTASKBAR, SW_HIDE);
}

static INT_PTR CALLBACK DlgProcContainerOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndTree = GetDlgItem(hwndDlg, IDC_SECTIONTREE);
	TContainerData *pContainer = (TContainerData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			TVINSERTSTRUCT tvis = {};
			int nr_layouts = 0;
			const TSideBarLayout *sblayouts = CSideBar::getLayouts(nr_layouts);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			pContainer = (TContainerData*)lParam;
			pContainer->m_hWndOptions = hwndDlg;
			SetWindowText(hwndDlg, TranslateT("Container options"));
			wchar_t szNewTitle[128];
			mir_snwprintf(szNewTitle, L"%s", !mir_wstrcmp(pContainer->m_wszName, L"default") ? TranslateT("Default container") : pContainer->m_wszName);
			SetDlgItemText(hwndDlg, IDC_HEADERBAR, szNewTitle);
			Utils::enableDlgControl(hwndDlg, IDC_O_HIDETITLE, !CSkin::m_frameSkins);
			CheckDlgButton(hwndDlg, IDC_CNTPRIVATE, pContainer->cfg.fPrivate ? BST_CHECKED : BST_UNCHECKED);

			SendDlgItemMessage(hwndDlg, IDC_TABMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Tabs at the top"));
			SendDlgItemMessage(hwndDlg, IDC_TABMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Tabs at the bottom"));
			SendDlgItemMessage(hwndDlg, IDC_TABMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Switch bar on the left side"));
			SendDlgItemMessage(hwndDlg, IDC_TABMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Switch bar on the right side"));

			SendDlgItemMessage(hwndDlg, IDC_AVATARMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Globally on"));
			SendDlgItemMessage(hwndDlg, IDC_AVATARMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("On, if present"));
			SendDlgItemMessage(hwndDlg, IDC_AVATARMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Globally OFF"));
			SendDlgItemMessage(hwndDlg, IDC_AVATARMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("On, if present, always in bottom display"));

			SendDlgItemMessage(hwndDlg, IDC_OWNAVATARMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("On, if present"));
			SendDlgItemMessage(hwndDlg, IDC_OWNAVATARMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Don't show them"));

			for (int i = 0; i < nr_layouts; i++)
				SendDlgItemMessage(hwndDlg, IDC_SBARLAYOUT, CB_INSERTSTRING, -1, (LPARAM)TranslateW(sblayouts[i].szName));

			/* bits 24 - 31 of dwFlagsEx hold the side bar layout id */
			SendDlgItemMessage(hwndDlg, IDC_SBARLAYOUT, CB_SETCURSEL, (WPARAM)((pContainer->cfg.flagsEx.dw & 0xff000000) >> 24), 0);

			SendMessage(hwndDlg, DM_SC_INITDIALOG, 0, (LPARAM)&pContainer->cfg);
			SendDlgItemMessage(hwndDlg, IDC_TITLEFORMAT, EM_LIMITTEXT, TITLE_FORMATLEN - 1, 0);
			SetDlgItemText(hwndDlg, IDC_TITLEFORMAT, pContainer->cfg.szTitleFormat);
			SetDlgItemText(hwndDlg, IDC_THEME, pContainer->m_szRelThemeFile);
			for (int i = 0; i < _countof(o_pages); i++) {
				tvis.hParent = nullptr;
				tvis.hInsertAfter = TVI_LAST;
				tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
				tvis.item.pszText = TranslateW(o_pages[i].szTitle);
				tvis.item.lParam = i;
				HTREEITEM hItem = TreeView_InsertItem(hwndTree, &tvis);
				if (i == 0)
					SendMessage(hwndTree, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem);
				ShowPage(hwndDlg, i, FALSE);
			}
			Window_SetSkinIcon_IcoLib(hwndDlg, SKINICON_EVENT_MESSAGE);
			ShowPage(hwndDlg, 0, TRUE);
			SetFocus(hwndTree);
			Utils::enableDlgControl(hwndDlg, IDC_APPLY, false);

			HFONT hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_DESC, WM_GETFONT, 0, 0);
			LOGFONT lf = { 0 };

			GetObject(hFont, sizeof(lf), &lf);
			lf.lfHeight = (int)(lf.lfHeight * 1.2);
			hFont = CreateFontIndirect(&lf);

			SendDlgItemMessage(hwndDlg, IDC_TITLEBOX, WM_SETFONT, (WPARAM)hFont, TRUE);

			if (pContainer->m_isCloned && pContainer->m_hContactFrom != 0) {
				Utils::showDlgControl(hwndDlg, IDC_CNTPRIVATE, SW_HIDE);
				Utils::showDlgControl(hwndDlg, IDC_O_CNTPRIVATE, SW_HIDE);
			}
		}
		return FALSE;

	case WM_NOTIFY:
		if (wParam == IDC_SECTIONTREE && ((LPNMHDR)lParam)->code == TVN_SELCHANGED) {
			NMTREEVIEW *pmtv = (NMTREEVIEW *)lParam;

			ShowPage(hwndDlg, pmtv->itemOld.lParam, 0);
			ShowPage(hwndDlg, pmtv->itemNew.lParam, 1);
		}
		break;

	case WM_HSCROLL:
		if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_TRANSPARENCY_ACTIVE) || (HWND)lParam == GetDlgItem(hwndDlg, IDC_TRANSPARENCY_INACTIVE))
			Utils::enableDlgControl(hwndDlg, IDC_APPLY, true);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CNTPRIVATE:
			if (IsDlgButtonChecked(hwndDlg, IDC_CNTPRIVATE)) {
				pContainer->ReadPrivateSettings(true);
				pContainer->cfg.fPrivate = true;
			}
			else {
				if (pContainer->cfg.fPrivate) {
					char szCname[40];
					mir_snprintf(szCname, "%s%d", CNT_BASEKEYNAME, pContainer->m_iContainerIndex);
					Utils::WriteContainerSettingsToDB(0, &pContainer->cfg, szCname);
				}
				pContainer->cfg = PluginConfig.globalContainerSettings;
				pContainer->cfg.fPrivate = false;
			}
			SendMessage(hwndDlg, DM_SC_INITDIALOG, 0, (LPARAM)&pContainer->cfg);
			goto do_apply;

		case IDC_TRANSPARENCY:
			{
				bool isTrans = IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENCY) != 0;
				Utils::enableDlgControl(hwndDlg, IDC_TRANSPARENCY_ACTIVE, isTrans);
				Utils::enableDlgControl(hwndDlg, IDC_TRANSPARENCY_INACTIVE, isTrans);
			}
			goto do_apply;

		case IDC_SECTIONTREE:
		case IDC_DESC:
			return 0;

		case IDC_SAVESIZEASGLOBAL:
			{
				WINDOWPLACEMENT wp = { 0 };

				wp.length = sizeof(wp);
				if (GetWindowPlacement(pContainer->m_hwnd, &wp)) {
					db_set_dw(0, SRMSGMOD_T, "splitx", wp.rcNormalPosition.left);
					db_set_dw(0, SRMSGMOD_T, "splity", wp.rcNormalPosition.top);
					db_set_dw(0, SRMSGMOD_T, "splitwidth", wp.rcNormalPosition.right - wp.rcNormalPosition.left);
					db_set_dw(0, SRMSGMOD_T, "splitheight", wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
				}
				break;
			}
		case IDC_O_ENABLESOUNDS:
			SendMessage(hwndDlg, DM_SC_CONFIG, 0, 0);
			goto do_apply;

		case IDC_TITLEFORMAT:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return TRUE;
			goto do_apply;

		case IDC_SELECTTHEME:
			{
				const wchar_t *szFileName = GetThemeFileName(0);
				if (PathFileExists(szFileName)) {
					SetDlgItemText(hwndDlg, IDC_THEME, szFileName);
					goto do_apply;
				}
				break;
			}

		case IDC_O_HELP_TITLEFORMAT:
			Utils_OpenUrl("https://wiki.miranda-ng.org/index.php?title=Plugin:TabSRMM/Title_bar_formatting");
			break;

		case IDOK:
		case IDC_APPLY:
			SendMessage(hwndDlg, DM_SC_BUILDLIST, 0, (LPARAM)&pContainer->cfg);

			pContainer->cfg.dwTransparency = MAKELONG((uint16_t)SendDlgItemMessage(hwndDlg, IDC_TRANSPARENCY_ACTIVE, TBM_GETPOS, 0, 0),
				(uint16_t)SendDlgItemMessage(hwndDlg, IDC_TRANSPARENCY_INACTIVE, TBM_GETPOS, 0, 0));

			pContainer->cfg.avatarMode = (uint16_t)SendDlgItemMessage(hwndDlg, IDC_AVATARMODE, CB_GETCURSEL, 0, 0);
			pContainer->cfg.ownAvatarMode = (uint16_t)SendDlgItemMessage(hwndDlg, IDC_OWNAVATARMODE, CB_GETCURSEL, 0, 0);

			GetDlgItemText(hwndDlg, IDC_TITLEFORMAT, pContainer->cfg.szTitleFormat, TITLE_FORMATLEN);
			pContainer->cfg.szTitleFormat[TITLE_FORMATLEN - 1] = 0;

			pContainer->m_szRelThemeFile[0] = pContainer->m_szAbsThemeFile[0] = 0;

			if (GetWindowTextLength(GetDlgItem(hwndDlg, IDC_THEME)) > 0) {
				wchar_t szFinalThemeFile[MAX_PATH], szFilename[MAX_PATH];

				GetDlgItemText(hwndDlg, IDC_THEME, szFilename, _countof(szFilename));
				szFilename[MAX_PATH - 1] = 0;
				PathToAbsoluteW(szFilename, szFinalThemeFile, M.getDataPath());

				if (mir_wstrcmp(szFilename, pContainer->m_szRelThemeFile))
					pContainer->m_fPrivateThemeChanged = TRUE;

				if (PathFileExists(szFinalThemeFile))
					wcsncpy_s(pContainer->m_szRelThemeFile, szFilename, _TRUNCATE);
				else
					pContainer->m_szRelThemeFile[0] = 0;
			}
			else {
				pContainer->m_szRelThemeFile[0] = 0;
				pContainer->m_fPrivateThemeChanged = TRUE;
			}

			if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_CNTPRIVATE)) {
				ReloadGlobalContainerSettings(true);
				Utils::WriteContainerSettingsToDB(0, &PluginConfig.globalContainerSettings);
			}
			else pContainer->SaveSettings(CONTAINER_PREFIX);

			pContainer->Configure();
			pContainer->BroadCastContainer(DM_SETINFOPANEL, 0, 0);

			ShowWindow(pContainer->m_hwnd, SW_HIDE);
			{
				RECT rc;
				GetWindowRect(pContainer->m_hwnd, &rc);
				SetWindowPos(pContainer->m_hwnd, nullptr, rc.left, rc.top, (rc.right - rc.left) - 1, (rc.bottom - rc.top) - 1, SWP_NOZORDER | SWP_DRAWFRAME | SWP_FRAMECHANGED);
				SetWindowPos(pContainer->m_hwnd, nullptr, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), SWP_NOZORDER | SWP_DRAWFRAME | SWP_SHOWWINDOW);
			}

			if (LOWORD(wParam) == IDOK)
				DestroyWindow(hwndDlg);
			else
				Utils::enableDlgControl(hwndDlg, IDC_APPLY, false);

			break;

		case IDCANCEL:
			DestroyWindow(hwndDlg);
			return TRUE;

		default:
do_apply: Utils::enableDlgControl(hwndDlg, IDC_APPLY, true);
			break;
		}
		break;

	case DM_SC_INITDIALOG:
		{
			TContainerSettings* cs = (TContainerSettings *)lParam;

			auto flags = cs->flags;
			auto flagsEx = cs->flagsEx;

			uint32_t dwTransparency = cs->dwTransparency;
			BOOL fAllowTrans = FALSE;

			if (IsWinVerVistaPlus())
				fAllowTrans = TRUE;
			else
				fAllowTrans = !CSkin::m_skinEnabled;

			MY_CheckDlgButton(hwndDlg, IDC_O_HIDETITLE, flags.m_bNoTitle);
			MY_CheckDlgButton(hwndDlg, IDC_O_DONTREPORT, flags.m_bDontReport);
			MY_CheckDlgButton(hwndDlg, IDC_O_NOTABS, flags.m_bHideTabs);
			MY_CheckDlgButton(hwndDlg, IDC_O_STICKY, flags.m_bSticky);
			MY_CheckDlgButton(hwndDlg, IDC_O_FLASHNEVER, flags.m_bNoFlash);
			MY_CheckDlgButton(hwndDlg, IDC_O_FLASHALWAYS, flags.m_bFlashAlways);
			MY_CheckDlgButton(hwndDlg, IDC_O_FLASHDEFAULT, !flags.m_bNoFlash && !flags.m_bFlashAlways);
			MY_CheckDlgButton(hwndDlg, IDC_TRANSPARENCY, flags.m_bTransparent);
			MY_CheckDlgButton(hwndDlg, IDC_DONTREPORTUNFOCUSED2, flags.m_bDontReportUnfocused);
			MY_CheckDlgButton(hwndDlg, IDC_DONTREPORTFOCUSED2, flags.m_bDontReportFocused);
			MY_CheckDlgButton(hwndDlg, IDC_ALWAYSPOPUPSINACTIVE, flags.m_bAlwaysReportInactive);
			MY_CheckDlgButton(hwndDlg, IDC_CNTNOSTATUSBAR, flags.m_bNoStatusBar);
			MY_CheckDlgButton(hwndDlg, IDC_HIDEMENUBAR, flags.m_bNoMenuBar);
			MY_CheckDlgButton(hwndDlg, IDC_HIDETOOLBAR, flags.m_bHideToolbar);
			MY_CheckDlgButton(hwndDlg, IDC_BOTTOMTOOLBAR, flags.m_bBottomToolbar);
			MY_CheckDlgButton(hwndDlg, IDC_UIDSTATUSBAR, flags.m_bUinStatusBar);
			MY_CheckDlgButton(hwndDlg, IDC_VERTICALMAX, flags.m_bVerticalMax);
			MY_CheckDlgButton(hwndDlg, IDC_AUTOSPLITTER, flags.m_bAutoSplitter);
			MY_CheckDlgButton(hwndDlg, IDC_AVATARSONTASKBAR, flags.m_bAvatarsOnTaskbar);
			MY_CheckDlgButton(hwndDlg, IDC_INFOPANEL, flags.m_bInfoPanel);
			MY_CheckDlgButton(hwndDlg, IDC_USEGLOBALSIZE, flags.m_bGlobalSize);

			MY_CheckDlgButton(hwndDlg, IDC_FLASHICON, flagsEx.m_bTabFlashIcon);
			MY_CheckDlgButton(hwndDlg, IDC_FLASHLABEL, flagsEx.m_bTabFlashLabel);
			MY_CheckDlgButton(hwndDlg, IDC_CLOSEBUTTONONTABS, flagsEx.m_bTabCloseButton);

			MY_CheckDlgButton(hwndDlg, IDC_SINGLEROWTAB, flagsEx.m_bTabSingleRow);
			MY_CheckDlgButton(hwndDlg, IDC_BUTTONTABS, flagsEx.m_bTabFlat);

			MY_CheckDlgButton(hwndDlg, IDC_O_ENABLESOUNDS, !flags.m_bNoSound);
			MY_CheckDlgButton(hwndDlg, IDC_O_SOUNDSMINIMIZED, flagsEx.m_bSoundMinimized);
			MY_CheckDlgButton(hwndDlg, IDC_O_SOUNDSUNFOCUSED, flagsEx.m_bSoundUnfocused);
			MY_CheckDlgButton(hwndDlg, IDC_O_SOUNDSINACTIVE, flagsEx.m_bSoundInactive);
			MY_CheckDlgButton(hwndDlg, IDC_O_SOUNDSFOCUSED, flagsEx.m_bSoundFocused);

			SendMessage(hwndDlg, DM_SC_CONFIG, 0, 0);

			if (!flagsEx.m_bTabSBarLeft && !flagsEx.m_bTabSBarRight)
				SendDlgItemMessage(hwndDlg, IDC_TABMODE, CB_SETCURSEL, flags.m_bTabsBottom ? 1 : 0, 0);
			else
				SendDlgItemMessage(hwndDlg, IDC_TABMODE, CB_SETCURSEL, flagsEx.m_bTabSBarLeft ? 2 : 3, 0);

			if (fAllowTrans)
				CheckDlgButton(hwndDlg, IDC_TRANSPARENCY, (flags.m_bTransparent) ? BST_CHECKED : BST_UNCHECKED);
			else
				CheckDlgButton(hwndDlg, IDC_TRANSPARENCY, BST_UNCHECKED);

			Utils::enableDlgControl(hwndDlg, IDC_TRANSPARENCY, fAllowTrans);

			bool isTrans = IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENCY) != 0;
			Utils::enableDlgControl(hwndDlg, IDC_TRANSPARENCY_ACTIVE, isTrans);
			Utils::enableDlgControl(hwndDlg, IDC_TRANSPARENCY_INACTIVE, isTrans);

			SendDlgItemMessage(hwndDlg, IDC_TRANSPARENCY_ACTIVE, TBM_SETRANGE, 0, (LPARAM)MAKELONG(50, 255));
			SendDlgItemMessage(hwndDlg, IDC_TRANSPARENCY_INACTIVE, TBM_SETRANGE, 0, (LPARAM)MAKELONG(50, 255));

			SendDlgItemMessage(hwndDlg, IDC_TRANSPARENCY_ACTIVE, TBM_SETPOS, TRUE, (LPARAM)LOWORD(dwTransparency));
			SendDlgItemMessage(hwndDlg, IDC_TRANSPARENCY_INACTIVE, TBM_SETPOS, TRUE, (LPARAM)HIWORD(dwTransparency));

			Utils::enableDlgControl(hwndDlg, IDC_O_DONTREPORT, !NEN::bWindowCheck);
			Utils::enableDlgControl(hwndDlg, IDC_DONTREPORTUNFOCUSED2, !NEN::bWindowCheck);
			Utils::enableDlgControl(hwndDlg, IDC_DONTREPORTFOCUSED2, !NEN::bWindowCheck);
			Utils::enableDlgControl(hwndDlg, IDC_ALWAYSPOPUPSINACTIVE, !NEN::bWindowCheck);

			SendDlgItemMessage(hwndDlg, IDC_AVATARMODE, CB_SETCURSEL, (WPARAM)cs->avatarMode, 0);
			SendDlgItemMessage(hwndDlg, IDC_OWNAVATARMODE, CB_SETCURSEL, (WPARAM)cs->ownAvatarMode, 0);

			Utils::showDlgControl(hwndDlg, IDC_O_EXPLAINGLOBALNOTIFY, NEN::bWindowCheck ? SW_SHOW : SW_HIDE);

			SendDlgItemMessage(hwndDlg, IDC_AUTOCLOSETABSPIN, UDM_SETRANGE, 0, MAKELONG(1000, 0));
			SendDlgItemMessage(hwndDlg, IDC_AUTOCLOSETABSPIN, UDM_SETPOS, 0, (LPARAM)cs->autoCloseSeconds);
		}
		break;

	case DM_SC_CONFIG:
		{
			bool bEnable = IsDlgButtonChecked(hwndDlg, IDC_O_ENABLESOUNDS) != 0;
			Utils::enableDlgControl(hwndDlg, IDC_O_SOUNDSINACTIVE, bEnable);
			Utils::enableDlgControl(hwndDlg, IDC_O_SOUNDSUNFOCUSED, bEnable);
			Utils::enableDlgControl(hwndDlg, IDC_O_SOUNDSMINIMIZED, bEnable);
			Utils::enableDlgControl(hwndDlg, IDC_O_SOUNDSFOCUSED, bEnable);
		}
		return 0;

	case DM_SC_BUILDLIST:
		{
			TContainerSettings* cs = (TContainerSettings *)lParam;

			TContainerFlags newFlags;
			newFlags.dw = 0;
			newFlags.m_bNoTitle = IsDlgButtonChecked(hwndDlg, IDC_O_HIDETITLE);
			newFlags.m_bDontReport = IsDlgButtonChecked(hwndDlg, IDC_O_DONTREPORT);
			newFlags.m_bHideTabs = IsDlgButtonChecked(hwndDlg, IDC_O_NOTABS);
			newFlags.m_bSticky = IsDlgButtonChecked(hwndDlg, IDC_O_STICKY);
			newFlags.m_bFlashAlways = IsDlgButtonChecked(hwndDlg, IDC_O_FLASHALWAYS);
			newFlags.m_bNoFlash = IsDlgButtonChecked(hwndDlg, IDC_O_FLASHNEVER);
			newFlags.m_bTransparent = IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENCY);
			newFlags.m_bDontReportUnfocused = IsDlgButtonChecked(hwndDlg, IDC_DONTREPORTUNFOCUSED2);
			newFlags.m_bDontReportFocused = IsDlgButtonChecked(hwndDlg, IDC_DONTREPORTFOCUSED2);
			newFlags.m_bAlwaysReportInactive = IsDlgButtonChecked(hwndDlg, IDC_ALWAYSPOPUPSINACTIVE);
			newFlags.m_bNoStatusBar = IsDlgButtonChecked(hwndDlg, IDC_CNTNOSTATUSBAR);
			newFlags.m_bNoMenuBar = IsDlgButtonChecked(hwndDlg, IDC_HIDEMENUBAR);
			newFlags.m_bHideToolbar = IsDlgButtonChecked(hwndDlg, IDC_HIDETOOLBAR);
			newFlags.m_bBottomToolbar = IsDlgButtonChecked(hwndDlg, IDC_BOTTOMTOOLBAR);
			newFlags.m_bUinStatusBar = IsDlgButtonChecked(hwndDlg, IDC_UIDSTATUSBAR);
			newFlags.m_bGlobalSize = IsDlgButtonChecked(hwndDlg, IDC_USEGLOBALSIZE);
			newFlags.m_bInfoPanel = IsDlgButtonChecked(hwndDlg, IDC_INFOPANEL);
			newFlags.m_bNoSound = !IsDlgButtonChecked(hwndDlg, IDC_O_ENABLESOUNDS);
			newFlags.m_bAvatarsOnTaskbar = IsDlgButtonChecked(hwndDlg, IDC_AVATARSONTASKBAR);
			newFlags.m_bVerticalMax = IsDlgButtonChecked(hwndDlg, IDC_VERTICALMAX);
			newFlags.m_bAutoSplitter = IsDlgButtonChecked(hwndDlg, IDC_AUTOSPLITTER);
			newFlags.m_bNewContainerFlags = true;

			LRESULT iTabMode = SendDlgItemMessage(hwndDlg, IDC_TABMODE, CB_GETCURSEL, 0, 0);
			LRESULT iTabLayout = SendDlgItemMessage(hwndDlg, IDC_SBARLAYOUT, CB_GETCURSEL, 0, 0);

			TContainerFlagsEx newFlagsEx;
			newFlagsEx.dw = 0;

			if (iTabMode < 2)
				newFlags.m_bTabsBottom = (iTabMode == 1);
			else {
				newFlags.m_bTabsBottom = false;
				if (iTabMode == 2)
					newFlagsEx.m_bTabSBarLeft = true;
				else
					newFlagsEx.m_bTabSBarRight = true;
			}

			newFlagsEx.m_bTabFlashIcon = IsDlgButtonChecked(hwndDlg, IDC_FLASHICON);
			newFlagsEx.m_bTabFlashLabel = IsDlgButtonChecked(hwndDlg, IDC_FLASHLABEL);
			newFlagsEx.m_bTabFlat = IsDlgButtonChecked(hwndDlg, IDC_BUTTONTABS);
			newFlagsEx.m_bTabCloseButton = IsDlgButtonChecked(hwndDlg, IDC_CLOSEBUTTONONTABS);
			newFlagsEx.m_bTabSingleRow = IsDlgButtonChecked(hwndDlg, IDC_SINGLEROWTAB);
			newFlagsEx.m_bSoundMinimized = IsDlgButtonChecked(hwndDlg, IDC_O_SOUNDSMINIMIZED);
			newFlagsEx.m_bSoundUnfocused = IsDlgButtonChecked(hwndDlg, IDC_O_SOUNDSUNFOCUSED);
			newFlagsEx.m_bSoundFocused = IsDlgButtonChecked(hwndDlg, IDC_O_SOUNDSFOCUSED);
			newFlagsEx.m_bSoundInactive = IsDlgButtonChecked(hwndDlg, IDC_O_SOUNDSINACTIVE);

			/* bits 24 - 31 of dwFlagsEx hold the sidebar layout id */
			newFlagsEx.dw |= ((int)((iTabLayout << 24) & 0xff000000));

			if (IsDlgButtonChecked(hwndDlg, IDC_O_FLASHDEFAULT))
				newFlags.m_bFlashAlways = newFlags.m_bNoFlash = false;

			cs->flags = newFlags;
			cs->flagsEx = newFlagsEx;
			cs->autoCloseSeconds = (uint16_t)SendDlgItemMessage(hwndDlg, IDC_AUTOCLOSETABSPIN, UDM_GETPOS, 0, 0);
		}
		break;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hwndDlg);
		pContainer->m_hWndOptions = nullptr;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);

		HFONT hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_TITLEBOX, WM_GETFONT, 0, 0);
		DeleteObject(hFont);
		break;
	}
	return FALSE;
}

void TContainerData::OptionsDialog()
{
	if (m_hWndOptions == nullptr)
		CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CONTAINEROPTIONS), m_hwnd, DlgProcContainerOptions, (LPARAM)this);
}
