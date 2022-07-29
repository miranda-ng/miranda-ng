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
// implements the "Container" window which acts as a toplevel window
// for message sessions.

#include "stdafx.h"

#define CONTAINER_KEY "TAB_ContainersW"
#define CONTAINER_SUBKEY "containerW"

static bool fForceOverlayIcons = false;

/////////////////////////////////////////////////////////////////////////////////////////
// functions for handling the linked list of struct ContainerWindowData *foo

TContainerData *pFirstContainer = nullptr;        // the linked list of struct ContainerWindowData
TContainerData *pLastActiveContainer = nullptr;

static TContainerData *AppendToContainerList(TContainerData *pContainer)
{
	if (!pFirstContainer) {
		pFirstContainer = pContainer;
		pFirstContainer->pNext = nullptr;
		return pFirstContainer;
	}

	TContainerData *p = pFirstContainer;
	while (p->pNext != nullptr)
		p = p->pNext;
	p->pNext = pContainer;
	pContainer->pNext = nullptr;
	return p;
}

static TContainerData* RemoveContainerFromList(TContainerData *pContainer)
{
	if (pContainer == pFirstContainer) {
		if (pContainer->pNext != nullptr)
			pFirstContainer = pContainer->pNext;
		else
			pFirstContainer = nullptr;

		if (pLastActiveContainer == pContainer)     // make sure, we don't reference this container anymore
			pLastActiveContainer = pFirstContainer;

		return pFirstContainer;
	}

	for (TContainerData *p = pFirstContainer; p; p = p->pNext) {
		if (p->pNext == pContainer) {
			p->pNext = p->pNext->pNext;

			if (pLastActiveContainer == pContainer)     // make sure, we don't reference this container anymore
				pLastActiveContainer = pFirstContainer;

			return nullptr;
		}
	}
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

TContainerData::~TContainerData()
{
	delete m_pMenuBar;
	delete m_pSideBar;
}

/////////////////////////////////////////////////////////////////////////////////////////
// this function searches and activates the tab belonging to the given message dialog

void TContainerData::ActivateExistingTab(CMsgDialog *dat)
{
	if (dat == nullptr)
		return;

	NMHDR nmhdr = {};
	nmhdr.code = TCN_SELCHANGE;
	if (TabCtrl_GetItemCount(m_hwndTabs) > 1 && !cfg.flags.m_bDeferredTabSelect) {
		TabCtrl_SetCurSel(m_hwndTabs, GetTabIndexFromHWND(m_hwndTabs, dat->GetHwnd()));
		SendMessage(m_hwnd, WM_NOTIFY, 0, (LPARAM)&nmhdr);	// just select the tab and let WM_NOTIFY do the rest
		if (cfg.flags.m_bSideBar)
			m_pSideBar->setActiveItem(dat, true);
	}
	if (!dat->isChat())
		UpdateTitle(dat->m_hContact);
	if (IsIconic(m_hwnd)) {
		SendMessage(m_hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		SetForegroundWindow(m_hwnd);
	}

	// hide on close feature
	if (!IsWindowVisible(m_hwnd)) {
		WINDOWPLACEMENT wp = { 0 };
		wp.length = sizeof(wp);
		GetWindowPlacement(m_hwnd, &wp);

		// all tabs must re-check the layout on activation because adding a tab while
		// the container was hidden can make this necessary
		BroadCastContainer(DM_CHECKSIZE, 0, 0);
		if (wp.showCmd == SW_SHOWMAXIMIZED)
			ShowWindow(m_hwnd, SW_SHOWMAXIMIZED);
		else {
			ShowWindow(m_hwnd, SW_SHOWNA);
			SetForegroundWindow(m_hwnd);
		}
		SendMessage(m_hwndActive, WM_SIZE, 0, 0);			// make sure the active tab resizes its layout properly
	}
	else if (GetForegroundWindow() != m_hwnd)
		SetForegroundWindow(m_hwnd);

	if (!dat->isChat())
		SetFocus(dat->GetEntry().GetHwnd());
}

/////////////////////////////////////////////////////////////////////////////////////////
// calls the TabCtrl_AdjustRect to calculate the "real" client area of the tab.
// also checks for the option "hide tabs when only one tab open" and adjusts
// geometry if necessary
// rc is the RECT obtained by GetClientRect(hwndTab)

void TContainerData::AdjustTabClientRect(RECT &rc)
{
	uint32_t dwStyle = GetWindowLongPtr(m_hwndTabs, GWL_STYLE);

	RECT rcTab, rcTabOrig;
	GetClientRect(m_hwndTabs, &rcTab);
	if (!cfg.flags.m_bSideBar && (m_iChilds > 1 || !cfg.flags.m_bHideTabs)) {
		rcTabOrig = rcTab;
		TabCtrl_AdjustRect(m_hwndTabs, FALSE, &rcTab);
		uint32_t dwTopPad = rcTab.top - rcTabOrig.top;

		rc.left += m_tBorder;
		rc.right -= m_tBorder;

		if (dwStyle & TCS_BUTTONS) {
			if (cfg.flags.m_bTabsBottom) {
				int nCount = TabCtrl_GetItemCount(m_hwndTabs);
				if (nCount > 0) {
					RECT rcItem;
					TabCtrl_GetItemRect(m_hwndTabs, nCount - 1, &rcItem);
					rc.bottom = rcItem.top;
				}
			}
			else {
				rc.top += (dwTopPad - 2);
				rc.bottom = rcTabOrig.bottom;
			}
		}
		else {
			if (cfg.flags.m_bTabsBottom)
				rc.bottom = rcTab.bottom + 2;
			else {
				rc.top += (dwTopPad - 2);
				rc.bottom = rcTabOrig.bottom;
			}
		}

		rc.top += m_tBorder;
		rc.bottom -= m_tBorder;
	}
	else {
		rc.bottom = rcTab.bottom;
		rc.top = rcTab.top;
	}
	rc.right -= (m_tBorder_outer_left + m_tBorder_outer_right);
	if (m_pSideBar->isVisible())
		rc.right -= m_pSideBar->getWidth();
}

/////////////////////////////////////////////////////////////////////////////////////////
// broadcasts a message to all child windows (tabs/sessions)

void TContainerData::BroadCastContainer(UINT message, WPARAM wParam, LPARAM lParam) const
{
	int nCount = TabCtrl_GetItemCount(m_hwndTabs);
	for (int i = 0; i < nCount; i++) {
		HWND hDlg = GetTabWindow(m_hwndTabs, i);
		if (IsWindow(hDlg))
			SendMessage(hDlg, message, wParam, lParam);
	}
}

void TContainerData::CloseTabByMouse(POINT *pt)
{
	if (HWND hDlg = GetTabWindow(m_hwndTabs, GetTabItemFromMouse(m_hwndTabs, pt))) {
		if (hDlg != m_hwndActive) {
			m_bDontSmartClose = true;
			SendMessage(hDlg, WM_CLOSE, 0, 1);
			RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE);
			m_bDontSmartClose = false;
		}
		else SendMessage(hDlg, WM_CLOSE, 0, 1);
	}
}

void TContainerData::Configure()
{
	uint32_t wsold, ws = wsold = GetWindowLong(m_hwnd, GWL_STYLE);
	if (!CSkin::m_frameSkins) {
		ws = (cfg.flags.m_bNoTitle) ?
			((IsWindowVisible(m_hwnd) ? WS_VISIBLE : 0) | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN | WS_THICKFRAME | (CSkin::m_frameSkins ? WS_SYSMENU : WS_SYSMENU | WS_SIZEBOX)) :
			ws | WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
		SetWindowLong(m_hwnd, GWL_STYLE, ws);
	}

	m_tBorder = M.GetByte((CSkin::m_skinEnabled ? "S_tborder" : "tborder"), 2);
	m_tBorder_outer_left = g_ButtonSet.left + M.GetByte((CSkin::m_skinEnabled ? "S_tborder_outer_left" : "tborder_outer_left"), 2);
	m_tBorder_outer_right = g_ButtonSet.right + M.GetByte((CSkin::m_skinEnabled ? "S_tborder_outer_right" : "tborder_outer_right"), 2);
	m_tBorder_outer_top = g_ButtonSet.top + M.GetByte((CSkin::m_skinEnabled ? "S_tborder_outer_top" : "tborder_outer_top"), 2);
	m_tBorder_outer_bottom = g_ButtonSet.bottom + M.GetByte((CSkin::m_skinEnabled ? "S_tborder_outer_bottom" : "tborder_outer_bottom"), 2);
	UINT sBarHeight = (UINT)M.GetByte((CSkin::m_skinEnabled ? "S_sbarheight" : "sbarheight"), 0);

	BOOL fTransAllowed = !CSkin::m_skinEnabled || IsWinVerVistaPlus();

	uint32_t ex = GetWindowLong(m_hwnd, GWL_EXSTYLE);
	ex = (cfg.flags.m_bTransparent && (!CSkin::m_skinEnabled || fTransAllowed)) ? (ex | WS_EX_LAYERED) : (ex & ~WS_EX_LAYERED);
	SetWindowLong(m_hwnd, GWL_EXSTYLE, ex);

	if (cfg.flags.m_bTransparent && fTransAllowed) {
		uint32_t trans = LOWORD(cfg.dwTransparency);
		SetLayeredWindowAttributes(m_hwnd, Skin->getColorKey(), (uint8_t)trans, (/* m_bSkinned ? LWA_COLORKEY : */ 0) | (cfg.flags.m_bTransparent ? LWA_ALPHA : 0));
	}

	HMENU hSysmenu = GetSystemMenu(m_hwnd, FALSE);
	if (!CSkin::m_frameSkins)
		CheckMenuItem(hSysmenu, IDM_NOTITLE, (cfg.flags.m_bNoTitle) ? MF_CHECKED : MF_UNCHECKED);

	CheckMenuItem(hSysmenu, IDM_STAYONTOP, cfg.flags.m_bSticky ? MF_CHECKED : MF_UNCHECKED);
	SetWindowPos(m_hwnd, (cfg.flags.m_bSticky) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
	if (ws != wsold) {
		RECT rc;
		GetWindowRect(m_hwnd, &rc);
		if ((ws & WS_CAPTION) != (wsold & WS_CAPTION)) {
			SetWindowPos(m_hwnd, nullptr, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_NOCOPYBITS);
			RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);
			if (m_hwndActive != nullptr) {
				auto *dat = (CMsgDialog *)GetWindowLongPtr(m_hwndActive, GWLP_USERDATA);
				dat->DM_ScrollToBottom(0, 0);
			}
		}
	}

	cfg.flags.m_bSideBar = cfg.flagsEx.m_bTabSBarLeft || cfg.flagsEx.m_bTabSBarRight;
	m_pSideBar->Init();

	ws = wsold = GetWindowLong(m_hwndTabs, GWL_STYLE);
	if (cfg.flags.m_bTabsBottom)
		ws |= TCS_BOTTOM;
	else
		ws &= ~TCS_BOTTOM;
	if ((ws & (TCS_BOTTOM | TCS_MULTILINE)) != (wsold & (TCS_BOTTOM | TCS_MULTILINE))) {
		SetWindowLong(m_hwndTabs, GWL_STYLE, ws);
		RedrawWindow(m_hwndTabs, nullptr, nullptr, RDW_INVALIDATE);
	}

	if (cfg.flags.m_bNoStatusBar) {
		if (m_hwndStatus) {
			DestroyWindow(m_hwndStatus);
			m_hwndStatus = nullptr;
			m_statusBarHeight = 0;
			SendMessage(m_hwnd, DM_STATUSBARCHANGED, 0, 0);
		}
	}
	else if (m_hwndStatus == nullptr) {
		m_hwndStatus = CreateWindowEx(0, L"TSStatusBarClass", nullptr, SBT_TOOLTIPS | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, m_hwnd, nullptr, g_plugin.getInst(), nullptr);

		if (sBarHeight && CSkin::m_skinEnabled)
			SendMessage(m_hwndStatus, SB_SETMINHEIGHT, sBarHeight, 0);
	}
	if (m_hwndActive != nullptr) {
		MCONTACT hContact = 0;
		SendMessage(m_hwndActive, DM_QUERYHCONTACT, 0, (LPARAM)&hContact);
		if (hContact)
			UpdateTitle(hContact);
	}
	SendMessage(m_hwnd, WM_SIZE, 0, 1);
	BroadCastContainer(DM_CONFIGURETOOLBAR, 0, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////
// flashes the container
// iMode != 0: turn on flashing
// iMode == 0: turn off flashing

void TContainerData::FlashContainer(int iMode, int iCount)
{
	if (cfg.flags.m_bNoFlash)                  // container should never flash
		return;

	FLASHWINFO fwi;
	fwi.cbSize = sizeof(fwi);
	fwi.uCount = 0;

	if (iMode) {
		fwi.dwFlags = FLASHW_ALL;
		if (cfg.flags.m_bFlashAlways)
			fwi.dwFlags |= FLASHW_TIMER;
		else
			fwi.uCount = (iCount == 0) ? M.GetByte("nrflash", 4) : iCount;
		fwi.dwTimeout = M.GetDword("flashinterval", 1000);

	}
	else fwi.dwFlags = FLASHW_STOP;

	fwi.hwnd = m_hwnd;
	m_dwFlashingStarted = GetTickCount();
	FlashWindowEx(&fwi);
}

void TContainerData::InitDialog(HWND hwndDlg)
{
	m_hwnd = hwndDlg;
	m_hwndTabs = ::GetDlgItem(hwndDlg, IDC_MSGTABS);
	{
		uint32_t dwCreateFlags = cfg.flags.dw;
		m_isCloned = cfg.flags.m_bCreateCloned;
		m_fPrivateThemeChanged = FALSE;

		::SendMessage(hwndDlg, DM_OPTIONSAPPLIED, 0, 0); // set options...
		cfg.flags.dw |= dwCreateFlags;

		LoadOverrideTheme();
		uint32_t ws = ::GetWindowLong(m_hwndTabs, GWL_STYLE);
		if (cfg.flagsEx.m_bTabFlat)
			ws |= TCS_BUTTONS;

		ClearMargins();

		if (cfg.flagsEx.m_bTabSingleRow) {
			ws &= ~TCS_MULTILINE;
			ws |= TCS_SINGLELINE;
			ws |= TCS_FIXEDWIDTH;
		}
		else {
			ws &= ~TCS_SINGLELINE;
			ws |= TCS_MULTILINE;
			if (ws & TCS_BUTTONS)
				ws |= TCS_FIXEDWIDTH;
		}
		::SetWindowLong(m_hwndTabs, GWL_STYLE, ws);

		m_buttonItems = g_ButtonSet.items;

		cfg.flags.m_bSideBar = cfg.flagsEx.m_bTabSBarLeft || cfg.flagsEx.m_bTabSBarRight;
		m_pSideBar = new CSideBar(this);
		m_pMenuBar = new CMenuBar(this);

		::SetClassLong(hwndDlg, GCL_STYLE, ::GetClassLong(hwndDlg, GCL_STYLE) & ~(CS_VREDRAW | CS_HREDRAW | CS_DROPSHADOW));
		::SetClassLong(m_hwndTabs, GCL_STYLE, ::GetClassLong(m_hwndTabs, GCL_STYLE) & ~(CS_VREDRAW | CS_HREDRAW));

		// additional system menu items...
		HMENU hSysmenu = GetSystemMenu(hwndDlg, FALSE);
		int iMenuItems = GetMenuItemCount(hSysmenu);

		::InsertMenu(hSysmenu, iMenuItems++ - 2, MF_BYPOSITION | MF_SEPARATOR, 0, L"");
		::InsertMenu(hSysmenu, iMenuItems++ - 2, MF_BYPOSITION | MF_STRING, IDM_STAYONTOP, TranslateT("Stay on top"));
		if (!CSkin::m_frameSkins)
			::InsertMenu(hSysmenu, iMenuItems++ - 2, MF_BYPOSITION | MF_STRING, IDM_NOTITLE, TranslateT("Hide title bar"));
		::InsertMenu(hSysmenu, iMenuItems++ - 2, MF_BYPOSITION | MF_SEPARATOR, 0, L"");
		::InsertMenu(hSysmenu, iMenuItems++ - 2, MF_BYPOSITION | MF_STRING, IDM_MOREOPTIONS, TranslateT("Container options..."));
		::SetWindowText(hwndDlg, TranslateT("Message session..."));
		::SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)PluginConfig.g_iconContainer);

		// make the tab control the controlling parent window for all message dialogs

		ws = ::GetWindowLong(m_hwndTabs, GWL_EXSTYLE);
		::SetWindowLong(m_hwndTabs, GWL_EXSTYLE, ws | WS_EX_CONTROLPARENT);

		LONG x_pad = M.GetByte("x-pad", 3) + (cfg.flagsEx.m_bTabCloseButton ? 7 : 0);
		LONG y_pad = M.GetByte("y-pad", 3) + ((cfg.flags.m_bTabsBottom) ? 1 : 0);

		if (cfg.flagsEx.m_bTabFlat)
			y_pad++; //(cfg.flags.m_bTabsBottom ? 1 : 2);

		TabCtrl_SetPadding(m_hwndTabs, x_pad, y_pad);

		TabCtrl_SetImageList(m_hwndTabs, PluginConfig.g_hImageList);

		Configure();

		// tab tooltips...
		if (!::ServiceExists(MS_TIPPER_SHOWTIPW) || M.GetByte("d_tooltips", 0) == 0) {
			m_hwndTip = ::CreateWindowEx(0, TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT,
				CW_USEDEFAULT, CW_USEDEFAULT, hwndDlg, nullptr, g_plugin.getInst(), (LPVOID)nullptr);

			if (m_hwndTip) {
				::SetWindowPos(m_hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				TabCtrl_SetToolTips(m_hwndTabs, m_hwndTip);
			}
		}
		else m_hwndTip = nullptr;

		if (cfg.flags.m_bCreateMinimized) {
			SetWindowLongPtr(hwndDlg, GWL_STYLE, GetWindowLongPtr(hwndDlg, GWL_STYLE) & ~WS_VISIBLE);
			::ShowWindow(hwndDlg, SW_SHOWMINNOACTIVE);
			RestoreWindowPos();
			::ShowWindow(hwndDlg, SW_SHOWMINNOACTIVE);

			WINDOWPLACEMENT wp = {};
			wp.length = sizeof(wp);
			::GetWindowPlacement(hwndDlg, &wp);
			m_rcSaved.left = m_rcSaved.top = 0;
			m_rcSaved.right = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
			m_rcSaved.bottom = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
		}
		else {
			RestoreWindowPos();
			::ShowWindow(hwndDlg, SW_SHOWNORMAL);
		}
	}
}

void TContainerData::InitRedraw()
{
	::KillTimer(m_hwnd, (UINT_PTR)this);
	::SetTimer(m_hwnd, (UINT_PTR)this, 100, nullptr);
}

void TContainerData::LoadOverrideTheme()
{
	memset(&m_theme, 0, sizeof(TLogTheme));
	if (mir_wstrlen(m_szAbsThemeFile) > 1) {
		if (PathFileExists(m_szAbsThemeFile)) {
			if (CheckThemeVersion(m_szAbsThemeFile) == 0) {
				LoadThemeDefaults();
				return;
			}
			if (m_ltr_templates == nullptr) {
				m_ltr_templates = (TTemplateSet *)mir_alloc(sizeof(TTemplateSet));
				memcpy(m_ltr_templates, &LTR_Active, sizeof(TTemplateSet));
			}
			if (m_rtl_templates == nullptr) {
				m_rtl_templates = (TTemplateSet *)mir_alloc(sizeof(TTemplateSet));
				memcpy(m_rtl_templates, &RTL_Active, sizeof(TTemplateSet));
			}

			m_theme.logFonts = (LOGFONTW *)mir_alloc(sizeof(LOGFONTW) * (MSGDLGFONTCOUNT + 2));
			m_theme.fontColors = (COLORREF *)mir_alloc(sizeof(COLORREF) * (MSGDLGFONTCOUNT + 2));
			m_theme.rtfFonts = (char *)mir_alloc((MSGDLGFONTCOUNT + 2) * RTFCACHELINESIZE);

			ReadThemeFromINI(m_szAbsThemeFile, this, 0, THEME_READ_ALL);
			m_theme.left_indent *= 15;
			m_theme.right_indent *= 15;
			m_theme.isPrivate = true;
			if (CSkin::m_skinEnabled)
				m_theme.bg = SkinItems[ID_EXTBKCONTAINER].COLOR;
			else
				m_theme.bg = PluginConfig.m_fillColor ? PluginConfig.m_fillColor : GetSysColor(COLOR_WINDOW);
			return;
		}
	}
	LoadThemeDefaults();
}

void TContainerData::LoadThemeDefaults()
{
	memset(&m_theme, 0, sizeof(TLogTheme));
	m_theme.bg = db_get_dw(0, FONTMODULE, SRMSGSET_BKGCOLOUR, GetSysColor(COLOR_WINDOW));
	m_theme.statbg = PluginConfig.crStatus;
	m_theme.oldinbg = PluginConfig.crOldIncoming;
	m_theme.oldoutbg = PluginConfig.crOldOutgoing;
	m_theme.inbg = PluginConfig.crIncoming;
	m_theme.outbg = PluginConfig.crOutgoing;
	m_theme.hgrid = db_get_dw(0, FONTMODULE, "hgrid", RGB(224, 224, 224));
	m_theme.left_indent = M.GetDword("IndentAmount", 20) * 15;
	m_theme.right_indent = M.GetDword("RightIndent", 20) * 15;
	m_theme.inputbg = db_get_dw(0, FONTMODULE, "inputbg", SRMSGDEFSET_BKGCOLOUR);

	for (int i = 1; i <= 5; i++) {
		char szTemp[40];
		mir_snprintf(szTemp, "cc%d", i);
		COLORREF	colour = M.GetDword(szTemp, RGB(224, 224, 224));
		if (colour == 0)
			colour = RGB(1, 1, 1);
		m_theme.custom_colors[i - 1] = colour;
	}
	m_theme.logFonts = logfonts;
	m_theme.fontColors = fontcolors;
	m_theme.rtfFonts = nullptr;
	m_ltr_templates = &LTR_Active;
	m_rtl_templates = &RTL_Active;
	m_theme.dwFlags = (M.GetDword("mwflags", MWF_LOG_DEFAULT) & MWF_LOG_ALL);
	m_theme.isPrivate = false;
}

void TContainerData::QueryClientArea(RECT &rc)
{
	if (!IsIconic(m_hwnd))
		GetClientRect(m_hwnd, &rc);
	else
		CopyRect(&rc, &m_rcSaved);
	AdjustTabClientRect(rc);
}

// search tab with either next or most recent unread message and select it
void TContainerData::QueryPending()
{
	int   iMostRecent = -1;
	uint32_t dwMostRecent = 0;
	HWND  hwndMostRecent = nullptr;

	int iItems = TabCtrl_GetItemCount(m_hwndTabs);
	for (int i = 0; i < iItems; i++) {
		HWND hDlg = GetTabWindow(m_hwndTabs, i);
		uint32_t dwTimestamp;
		SendMessage(hDlg, DM_QUERYLASTUNREAD, 0, (LPARAM)&dwTimestamp);
		if (dwTimestamp > dwMostRecent) {
			dwMostRecent = dwTimestamp;
			iMostRecent = i;
			hwndMostRecent = hDlg;
		}
	}

	if (iMostRecent != -1) {
		TabCtrl_SetCurSel(m_hwndTabs, iMostRecent);

		NMHDR nmhdr;
		nmhdr.code = TCN_SELCHANGE;
		SendMessage(m_hwnd, WM_NOTIFY, 0, (LPARAM)&nmhdr);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// read settings for a container with private settings enabled.
//
// @param pContainer	container window info struct
// @param fForce		true -> force them private, even if they were not marked as private in the db

void TContainerData::ReadPrivateSettings(bool fForce)
{
	char szCname[50];
	TContainerSettings csTemp;
	memcpy(&csTemp, &PluginConfig.globalContainerSettings, sizeof(csTemp));

	mir_snprintf(szCname, "%s%d", CNT_BASEKEYNAME, m_iContainerIndex);
	Utils::ReadContainerSettingsFromDB(0, &csTemp, szCname);
	if (csTemp.fPrivate || fForce) {
		cfg = csTemp;
		cfg.fPrivate = true;
	}
	else cfg = PluginConfig.globalContainerSettings;
}

void TContainerData::SaveSettings(const char *szSetting)
{
	char szCName[50];

	auto &f = cfg.flags;
	f.m_bDeferredConfigure = f.m_bCreateMinimized = f.m_bDeferredResize = f.m_bCreateCloned = false;

	if (cfg.fPrivate) {
		mir_snprintf(szCName, "%s%d", szSetting, m_iContainerIndex);
		Utils::WriteContainerSettingsToDB(0, &cfg, szCName);
	}
	else {
		PluginConfig.globalContainerSettings = cfg;
		Utils::WriteContainerSettingsToDB(0, &cfg, nullptr);
	}

	mir_snprintf(szCName, "%s%d_theme", szSetting, m_iContainerIndex);
	if (mir_wstrlen(m_szRelThemeFile) > 1) {
		if (m_fPrivateThemeChanged == TRUE) {
			PathToRelativeW(m_szRelThemeFile, m_szAbsThemeFile, M.getDataPath());
			db_set_ws(0, SRMSGMOD_T, szCName, m_szAbsThemeFile);
			m_fPrivateThemeChanged = FALSE;
		}
	}
	else {
		::db_unset(0, SRMSGMOD_T, szCName);
		m_fPrivateThemeChanged = FALSE;
	}
}

void TContainerData::ReflashContainer()
{
	if (IsActive()) // dont care about active windows
		return;

	if (cfg.flags.m_bNoFlash || m_dwFlashingStarted == 0)
		return;                                                                                 // dont care about containers which should never flash

	uint32_t dwStartTime = m_dwFlashingStarted;

	if (cfg.flags.m_bFlashAlways)
		FlashContainer(1, 0);
	else {
		// recalc the remaining flashes
		uint32_t dwInterval = M.GetDword("flashinterval", 1000);
		int iFlashesElapsed = (GetTickCount() - dwStartTime) / dwInterval;
		uint32_t dwFlashesDesired = M.GetByte("nrflash", 4);
		if (iFlashesElapsed < (int)dwFlashesDesired)
			FlashContainer(1, dwFlashesDesired - iFlashesElapsed);
		else {
			BOOL isFlashed = FlashWindow(m_hwnd, TRUE);
			if (!isFlashed)
				FlashWindow(m_hwnd, TRUE);
		}
	}
	m_dwFlashingStarted = dwStartTime;
}

// retrieve the container window geometry information from the database.
void TContainerData::RestoreWindowPos()
{
	if (m_isCloned && m_hContactFrom != 0 && !cfg.flags.m_bGlobalSize) {
		if (Utils_RestoreWindowPosition(m_hwnd, m_hContactFrom, SRMSGMOD_T, "split")) {
			if (Utils_RestoreWindowPositionNoMove(m_hwnd, m_hContactFrom, SRMSGMOD_T, "split"))
				if (Utils_RestoreWindowPosition(m_hwnd, 0, SRMSGMOD_T, "split"))
					if (Utils_RestoreWindowPositionNoMove(m_hwnd, 0, SRMSGMOD_T, "split"))
						SetWindowPos(m_hwnd, nullptr, 50, 50, 450, 300, SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
	else {
		if (cfg.flags.m_bGlobalSize) {
			if (Utils_RestoreWindowPosition(m_hwnd, 0, SRMSGMOD_T, "split"))
				if (Utils_RestoreWindowPositionNoMove(m_hwnd, 0, SRMSGMOD_T, "split"))
					SetWindowPos(m_hwnd, nullptr, 50, 50, 450, 300, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		else {
			char szCName[CONTAINER_NAMELEN + 20];
			mir_snprintf(szCName, "%s%d", CONTAINER_PREFIX, m_iContainerIndex);
			if (Utils_RestoreWindowPosition(m_hwnd, 0, SRMSGMOD_T, szCName)) {
				if (Utils_RestoreWindowPositionNoMove(m_hwnd, 0, SRMSGMOD_T, szCName))
					if (Utils_RestoreWindowPosition(m_hwnd, 0, SRMSGMOD_T, "split"))
						if (Utils_RestoreWindowPositionNoMove(m_hwnd, 0, SRMSGMOD_T, "split"))
							SetWindowPos(m_hwnd, nullptr, 50, 50, 450, 300, SWP_NOZORDER | SWP_NOACTIVATE);
			}
		}
	}
}

void TContainerData::Resize(bool bRestored, int newWidth)
{
	RECT rcClient, rcUnadjusted;

	GetClientRect(m_hwnd, &rcClient);
	m_pMenuBar->getClientRect();

	if (m_hwndStatus) {
		auto *dat = (CMsgDialog *)GetWindowLongPtr(m_hwndActive, GWLP_USERDATA);
		SendMessage(m_hwndStatus, WM_USER + 101, 0, (LPARAM)dat);

		RECT rcs;
		GetWindowRect(m_hwndStatus, &rcs);
		m_statusBarHeight = (rcs.bottom - rcs.top) + 1;
		SendMessage(m_hwndStatus, SB_SETTEXT, (WPARAM)(SBT_OWNERDRAW) | 2, 0);
	}
	else m_statusBarHeight = 0;

	CopyRect(&m_rcSaved, &rcClient);
	rcUnadjusted = rcClient;

	m_pMenuBar->Resize(newWidth);
	LONG rebarHeight = m_pMenuBar->getHeight();
	m_pMenuBar->Show((cfg.flags.m_bNoMenuBar) ? SW_HIDE : SW_SHOW);

	LONG sbarWidth = m_pSideBar->getWidth();
	LONG sbarWidth_left = m_pSideBar->getFlags() & CSideBar::SIDEBARORIENTATION_LEFT ? sbarWidth : 0;

	if (newWidth) {
		SetWindowPos(m_hwndTabs, nullptr, m_tBorder_outer_left + sbarWidth_left, m_tBorder_outer_top + rebarHeight,
			(rcClient.right - rcClient.left) - (m_tBorder_outer_left + m_tBorder_outer_right + sbarWidth),
			(rcClient.bottom - rcClient.top) - m_statusBarHeight - (m_tBorder_outer_top + m_tBorder_outer_bottom) - rebarHeight, 
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_DEFERERASE | SWP_NOCOPYBITS);
	}

	m_pSideBar->resizeScrollWnd(sbarWidth_left ? m_tBorder_outer_left : rcClient.right - m_tBorder_outer_right - (sbarWidth - 2),
		m_tBorder_outer_top + rebarHeight, 0,
		(rcClient.bottom - rcClient.top) - m_statusBarHeight - (m_tBorder_outer_top + m_tBorder_outer_bottom) - rebarHeight);

	AdjustTabClientRect(rcClient);

	BOOL sizeChanged = (((rcClient.right - rcClient.left) != m_preSIZE.cx) || ((rcClient.bottom - rcClient.top) != m_preSIZE.cy));
	if (sizeChanged) {
		m_preSIZE.cx = rcClient.right - rcClient.left;
		m_preSIZE.cy = rcClient.bottom - rcClient.top;
	}

	// we care about all client sessions, but we really resize only the active tab (hwndActive)
	// we tell inactive tabs to resize theirselves later when they get activated (DM_CHECKSIZE
	// just queues a resize request)
	int nCount = TabCtrl_GetItemCount(m_hwndTabs);

	for (int i = 0; i < nCount; i++) {
		HWND hDlg = GetTabWindow(m_hwndTabs, i);
		if (hDlg == m_hwndActive) {
			SetWindowPos(hDlg, nullptr, rcClient.left, rcClient.top, (rcClient.right - rcClient.left), (rcClient.bottom - rcClient.top), SWP_NOSENDCHANGING | SWP_NOACTIVATE);
			if (!m_bSizingLoop && sizeChanged) {
				auto *dat = (CMsgDialog *)GetWindowLongPtr(m_hwndActive, GWLP_USERDATA);
				if (dat)
					dat->DM_ScrollToBottom(0, 1);
			}
		}
		else if (sizeChanged)
			SendMessage(hDlg, DM_CHECKSIZE, 0, 0);
	}
	m_pSideBar->scrollIntoView();

	if (!M.isAero()) {					// aero mode uses buffered paint, no forced redraw needed
		RedrawWindow(m_hwndTabs, nullptr, nullptr, RDW_INVALIDATE | (m_bSizingLoop ? RDW_ERASE : 0));
		RedrawWindow(m_hwnd, nullptr, nullptr, (CSkin::m_skinEnabled ? RDW_FRAME : 0) | RDW_INVALIDATE | ((m_bSizingLoop || bRestored) ? RDW_ERASE : 0));
	}

	if (m_hwndStatus)
		InvalidateRect(m_hwndStatus, nullptr, FALSE);

	if ((CSkin::m_bClipBorder != 0 || CSkin::m_bRoundedCorner) && CSkin::m_frameSkins) {
		HRGN rgn;
		int clip = CSkin::m_bClipBorder;

		RECT rcWindow;
		GetWindowRect(m_hwnd, &rcWindow);

		if (CSkin::m_bRoundedCorner)
			rgn = CreateRoundRectRgn(clip, clip, (rcWindow.right - rcWindow.left) - clip + 1,
			(rcWindow.bottom - rcWindow.top) - clip + 1, CSkin::m_bRoundedCorner + clip, CSkin::m_bRoundedCorner + clip);
		else
			rgn = CreateRectRgn(clip, clip, (rcWindow.right - rcWindow.left) - clip, (rcWindow.bottom - rcWindow.top) - clip);
		SetWindowRgn(m_hwnd, rgn, TRUE);
	}
	else if (CSkin::m_frameSkins)
		SetWindowRgn(m_hwnd, nullptr, TRUE);
}

void TContainerData::SelectTab(int iCommand, int idx)
{
	switch (iCommand) {
	case DM_SELECT_BY_HWND:
		ActivateTabFromHWND(m_hwndTabs, (HWND)idx);
		break;

	case DM_SELECT_NEXT:
	case DM_SELECT_PREV:
	case DM_SELECT_BY_INDEX:
		int iItems = TabCtrl_GetItemCount(m_hwndTabs);
		if (iItems == 1)
			break;

		int iCurrent = TabCtrl_GetCurSel(m_hwndTabs), iNewTab;

		if (iCommand == DM_SELECT_PREV)
			iNewTab = iCurrent ? iCurrent - 1 : iItems - 1;     // cycle if current is already the leftmost tab..
		else if (iCommand == DM_SELECT_NEXT)
			iNewTab = (iCurrent == (iItems - 1)) ? 0 : iCurrent + 1;
		else {
			if (idx > iItems)
				break;
			iNewTab = idx - 1;
		}

		if (iNewTab != iCurrent) {
			if (HWND hDlg = GetTabWindow(m_hwndTabs, iNewTab)) {
				TabCtrl_SetCurSel(m_hwndTabs, iNewTab);
				ShowWindow(m_hwndActive, SW_HIDE);
				m_hwndActive = hDlg;
				ShowWindow(hDlg, SW_SHOW);
				SetFocus(m_hwndActive);
			}
		}
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Windows Vista+
// extend the glassy area to get aero look for the status bar, tab bar, info panel
// and outer margins.

void TContainerData::SetAeroMargins()
{
	if (!M.isAero() || CSkin::m_skinEnabled) {
		m_pMenuBar->setAero(false);
		return;
	}

	CMsgDialog *dat = (CMsgDialog *)::GetWindowLongPtr(m_hwndActive, GWLP_USERDATA);
	if (!dat)
		return;

	RECT rcWnd;
	if (dat->m_pPanel.isActive())
		::GetWindowRect(::GetDlgItem(dat->GetHwnd(), IDC_SRMM_LOG), &rcWnd);
	else
		::GetWindowRect(dat->GetHwnd(), &rcWnd);

	POINT	pt = { rcWnd.left, rcWnd.top };
	::ScreenToClient(m_hwnd, &pt);

	MARGINS m;
	m.cyTopHeight = pt.y;
	m_pMenuBar->setAero(true);

	// bottom part
	::GetWindowRect(dat->GetHwnd(), &rcWnd);
	pt.x = rcWnd.left;

	LONG sbar_left, sbar_right;
	if (!m_pSideBar->isActive()) {
		pt.y = rcWnd.bottom + ((m_iChilds > 1 || !cfg.flags.m_bHideTabs) ? m_tBorder : 0);
		sbar_left = 0, sbar_right = 0;
	}
	else {
		pt.y = rcWnd.bottom;
		sbar_left = (m_pSideBar->getFlags() & CSideBar::SIDEBARORIENTATION_LEFT ? m_pSideBar->getWidth() : 0);
		sbar_right = (m_pSideBar->getFlags() & CSideBar::SIDEBARORIENTATION_RIGHT ? m_pSideBar->getWidth() : 0);
	}
	::ScreenToClient(m_hwnd, &pt);
	::GetClientRect(m_hwnd, &rcWnd);
	m.cyBottomHeight = (rcWnd.bottom - pt.y);

	if (m.cyBottomHeight < 0 || m.cyBottomHeight >= rcWnd.bottom)
		m.cyBottomHeight = 0;

	m.cxLeftWidth = m_tBorder_outer_left;
	m.cxRightWidth = m_tBorder_outer_right;
	m.cxLeftWidth += sbar_left;
	m.cxRightWidth += sbar_right;

	if (memcmp(&m, &m_mOld, sizeof(MARGINS)) != 0) {
		m_mOld = m;
		CMimAPI::m_pfnDwmExtendFrameIntoClientArea(m_hwnd, &m);
	}
}

void TContainerData::SetIcon(CMsgDialog *pDlg, HICON hIcon)
{
	HICON hIconMsg = PluginConfig.g_IconMsgEvent;
	HICON hIconBig = (pDlg && pDlg->m_cache) ? Skin_LoadProtoIcon(pDlg->m_cache->getProto(), pDlg->m_cache->getStatus(), true) : nullptr;

	if (Win7Taskbar->haveLargeIcons()) {
		if (hIcon == PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING] || hIcon == hIconMsg) {
			Win7Taskbar->setOverlayIcon(m_hwnd, (LPARAM)hIcon);
			if (GetForegroundWindow() != m_hwnd)
				SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
			if (hIcon == hIconMsg)
				m_hIconTaskbarOverlay = hIconMsg;
			return;
		}

		if (pDlg) {
			if (pDlg->m_hTaskbarIcon != nullptr) {
				DestroyIcon(pDlg->m_hTaskbarIcon);
				pDlg->m_hTaskbarIcon = nullptr;
			}

			if (pDlg->m_pContainer->cfg.flags.m_bAvatarsOnTaskbar)
				pDlg->m_hTaskbarIcon = pDlg->IconFromAvatar();

			if (pDlg->m_hTaskbarIcon) {
				SendMessage(m_hwnd, WM_SETICON, ICON_BIG, (LPARAM)pDlg->m_hTaskbarIcon);
				SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
				Win7Taskbar->setOverlayIcon(m_hwnd, (LPARAM)(pDlg->m_hTabIcon ? (LPARAM)pDlg->m_hTabIcon : (LPARAM)hIcon));
			}
			else {
				SendMessage(m_hwnd, WM_SETICON, ICON_BIG, hIconBig ? (LPARAM)hIconBig : (LPARAM)hIcon);
				SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
				
				if (pDlg->m_pContainer->m_hIconTaskbarOverlay)
					Win7Taskbar->setOverlayIcon(m_hwnd, (LPARAM)pDlg->m_pContainer->m_hIconTaskbarOverlay);
				else if (Win7Taskbar->haveAlwaysGroupingMode() && fForceOverlayIcons)
					Win7Taskbar->setOverlayIcon(m_hwnd, (LPARAM)hIcon);
				else
					Win7Taskbar->clearOverlayIcon(m_hwnd);
			}
			return;
		}
	}
	
	// default handling (no win7 taskbar)
	if (hIcon == PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING]) {              // always set typing icon, but don't save it...
		SendMessage(m_hwnd, WM_SETICON, ICON_BIG, (LPARAM)PluginConfig.g_IconTypingEventBig);
		SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		return;
	}

	if (hIcon == hIconMsg)
		hIconBig = Skin_LoadIcon(SKINICON_EVENT_MESSAGE, true);

	if (m_hIcon == STICK_ICON_MSG && hIcon != hIconMsg && cfg.flags.m_bNeedsUpdateTitle) {
		hIcon = hIconMsg;
		hIconBig = Skin_LoadIcon(SKINICON_EVENT_MESSAGE, true);
	}
	SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	if (nullptr != hIconBig && reinterpret_cast<HICON>(CALLSERVICE_NOTFOUND) != hIconBig)
		SendMessage(m_hwnd, WM_SETICON, ICON_BIG, LPARAM(hIconBig));
	m_hIcon = (hIcon == hIconMsg) ? STICK_ICON_MSG : 0;
}

void TContainerData::UpdateTabs()
{
	int nTabs = TabCtrl_GetItemCount(m_hwndTabs);
	for (int i = 0; i < nTabs; i++) {
		HWND hDlg = GetTabWindow(m_hwndTabs, i);
		if (!hDlg)
			continue;

		CMsgDialog *dat = (CMsgDialog*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
		if (dat)
			dat->m_iTabID = i;
	}
}

void TContainerData::UpdateTitle(MCONTACT hContact, CMsgDialog *pDlg)
{
	// pDlg != 0 means sent by a chat window
	if (pDlg) {
		wchar_t szText[512];
		GetWindowText(pDlg->GetHwnd(), szText, _countof(szText));
		szText[_countof(szText) - 1] = 0;
		SetWindowText(m_hwnd, szText);
		SetIcon(pDlg, (pDlg->m_hTabIcon != pDlg->m_hTabStatusIcon) ? pDlg->m_hTabIcon : pDlg->m_hTabStatusIcon);
		return;
	}

	// no hContact given - obtain the hContact for the active tab
	if (hContact == 0) {
		if (m_hwndActive && IsWindow(m_hwndActive))
			pDlg = (CMsgDialog*)GetWindowLongPtr(m_hwndActive, GWLP_USERDATA);
	}
	else {
		HWND hwnd = Srmm_FindWindow(hContact);
		if (hwnd != nullptr)
			pDlg = (CMsgDialog*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	}

	if (pDlg) {
		SetIcon(pDlg, pDlg->m_hXStatusIcon ? pDlg->m_hXStatusIcon : pDlg->m_hTabStatusIcon);
		CMStringW szTitle;
		if (pDlg->FormatTitleBar(cfg.szTitleFormat, szTitle))
			SetWindowText(m_hwnd, szTitle);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Window procedure (subclassed)

static LRESULT CALLBACK ContainerWndProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TContainerData *pContainer = (TContainerData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_NCPAINT:
		if (pContainer && CSkin::m_skinEnabled) {
			if (CSkin::m_frameSkins) {
				HDC dcFrame = GetDCEx(hwndDlg, nullptr, DCX_WINDOW |/*DCX_INTERSECTRGN|*/0x10000); // GetWindowDC(hwndDlg);
				LONG clip_top, clip_left;
				RECT rcText;
				HDC dcMem = CreateCompatibleDC(pContainer->m_cachedDC ? pContainer->m_cachedDC : dcFrame);

				RECT rcWindow, rcClient;
				POINT pt, pt1;
				GetWindowRect(hwndDlg, &rcWindow);
				GetClientRect(hwndDlg, &rcClient);
				pt.y = 0;
				pt.x = 0;
				ClientToScreen(hwndDlg, &pt);
				pt1.x = rcClient.right;
				pt1.y = rcClient.bottom;
				ClientToScreen(hwndDlg, &pt1);
				clip_top = pt.y - rcWindow.top;
				clip_left = pt.x - rcWindow.left;

				rcWindow.right = rcWindow.right - rcWindow.left;
				rcWindow.bottom = rcWindow.bottom - rcWindow.top;
				rcWindow.left = rcWindow.top = 0;

				HBITMAP hbmMem = CreateCompatibleBitmap(dcFrame, rcWindow.right, rcWindow.bottom);
				HBITMAP hbmOld = (HBITMAP)SelectObject(dcMem, hbmMem);

				ExcludeClipRect(dcFrame, clip_left, clip_top, clip_left + (pt1.x - pt.x), clip_top + (pt1.y - pt.y));
				ExcludeClipRect(dcMem, clip_left, clip_top, clip_left + (pt1.x - pt.x), clip_top + (pt1.y - pt.y));

				CSkin::DrawItem(dcMem, &rcWindow, &SkinItems[pContainer->m_ncActive ? ID_EXTBKFRAME : ID_EXTBKFRAMEINACTIVE]);

				wchar_t szWindowText[512];
				GetWindowText(hwndDlg, szWindowText, _countof(szWindowText));
				szWindowText[511] = 0;

				HFONT hOldFont = (HFONT)SelectObject(dcMem, PluginConfig.hFontCaption);

				TEXTMETRIC tm;
				GetTextMetrics(dcMem, &tm);
				SetTextColor(dcMem, CInfoPanel::m_ipConfig.clrs[IPFONTCOUNT - 1]);
				SetBkMode(dcMem, TRANSPARENT);
				rcText.left = 20 + CSkin::m_SkinnedFrame_left + CSkin::m_bClipBorder + CSkin::m_titleBarLeftOff;//26;
				rcText.right = rcWindow.right - 3 * CSkin::m_titleBarButtonSize.cx - 11 - CSkin::m_titleBarRightOff;
				rcText.top = CSkin::m_captionOffset + CSkin::m_bClipBorder;
				rcText.bottom = rcText.top + tm.tmHeight;
				rcText.left += CSkin::m_captionPadding;
				DrawText(dcMem, szWindowText, -1, &rcText, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
				SelectObject(dcMem, hOldFont);

				// icon
				HICON hIcon = (HICON)SendMessage(hwndDlg, WM_GETICON, ICON_SMALL, 0);
				DrawIconEx(dcMem, 4 + CSkin::m_SkinnedFrame_left + CSkin::m_bClipBorder + CSkin::m_titleBarLeftOff, rcText.top + (rcText.bottom - rcText.top) / 2 - 8, hIcon, 16, 16, 0, nullptr, DI_NORMAL);

				// title buttons
				pContainer->m_rcClose.top = pContainer->m_rcMin.top = pContainer->m_rcMax.top = CSkin::m_titleButtonTopOff;
				pContainer->m_rcClose.bottom = pContainer->m_rcMin.bottom = pContainer->m_rcMax.bottom = CSkin::m_titleButtonTopOff + CSkin::m_titleBarButtonSize.cy;

				pContainer->m_rcClose.right = rcWindow.right - 10 - CSkin::m_titleBarRightOff;
				pContainer->m_rcClose.left = pContainer->m_rcClose.right - CSkin::m_titleBarButtonSize.cx;

				pContainer->m_rcMax.right = pContainer->m_rcClose.left - 2;
				pContainer->m_rcMax.left = pContainer->m_rcMax.right - CSkin::m_titleBarButtonSize.cx;

				pContainer->m_rcMin.right = pContainer->m_rcMax.left - 2;
				pContainer->m_rcMin.left = pContainer->m_rcMin.right - CSkin::m_titleBarButtonSize.cx;

				CSkinItem *item_normal = &SkinItems[ID_EXTBKTITLEBUTTON];
				CSkinItem *item_hot = &SkinItems[ID_EXTBKTITLEBUTTONMOUSEOVER];
				CSkinItem *item_pressed = &SkinItems[ID_EXTBKTITLEBUTTONPRESSED];

				for (int i = 0; i < 3; i++) {
					RECT *pRect = nullptr;

					switch (i) {
					case 0:
						pRect = &pContainer->m_rcMin;
						hIcon = CSkin::m_minIcon;
						break;
					case 1:
						pRect = &pContainer->m_rcMax;
						hIcon = CSkin::m_maxIcon;
						break;
					case 2:
						pRect = &pContainer->m_rcClose;
						hIcon = CSkin::m_closeIcon;
						break;
					}
					if (pRect) {
						CSkinItem *item = pContainer->m_buttons[i].isPressed ? item_pressed : (pContainer->m_buttons[i].isHot ? item_hot : item_normal);
						CSkin::DrawItem(dcMem, pRect, item);
						DrawIconEx(dcMem, pRect->left + ((pRect->right - pRect->left) / 2 - 8), pRect->top + ((pRect->bottom - pRect->top) / 2 - 8), hIcon, 16, 16, 0, nullptr, DI_NORMAL);
					}
				}
				SetBkMode(dcMem, TRANSPARENT);
				BitBlt(dcFrame, 0, 0, rcWindow.right, rcWindow.bottom, dcMem, 0, 0, SRCCOPY);
				SelectObject(dcMem, hbmOld);
				DeleteObject(hbmMem);
				DeleteDC(dcMem);
				ReleaseDC(hwndDlg, dcFrame);
			}
			else mir_callNextSubclass(hwndDlg, ContainerWndProc, msg, wParam, lParam);

			PAINTSTRUCT ps;
			HDC hdcReal = BeginPaint(hwndDlg, &ps);

			RECT rcClient;
			GetClientRect(hwndDlg, &rcClient);
			int width = rcClient.right - rcClient.left;
			int height = rcClient.bottom - rcClient.top;
			if (width != pContainer->m_oldDCSize.cx || height != pContainer->m_oldDCSize.cy) {
				CSkinItem *sbaritem = &SkinItems[ID_EXTBKSTATUSBAR];
				BOOL statusBarSkinnd = !pContainer->cfg.flags.m_bNoStatusBar && !sbaritem->IGNORED;
				LONG sbarDelta = statusBarSkinnd ? pContainer->m_statusBarHeight : 0;

				pContainer->m_oldDCSize.cx = width;
				pContainer->m_oldDCSize.cy = height;

				if (pContainer->m_cachedDC) {
					SelectObject(pContainer->m_cachedDC, pContainer->m_oldHBM);
					DeleteObject(pContainer->m_cachedHBM);
					DeleteDC(pContainer->m_cachedDC);
				}
				pContainer->m_cachedDC = CreateCompatibleDC(hdcReal);
				pContainer->m_cachedHBM = CreateCompatibleBitmap(hdcReal, width, height);
				pContainer->m_oldHBM = (HBITMAP)SelectObject(pContainer->m_cachedDC, pContainer->m_cachedHBM);

				HDC hdc = pContainer->m_cachedDC;
				if (!CSkin::DrawItem(hdc, &rcClient, &SkinItems[0]))
					FillRect(hdc, &rcClient, GetSysColorBrush(COLOR_3DFACE));

				if (sbarDelta) {
					rcClient.top = rcClient.bottom - sbarDelta;
					CSkin::DrawItem(hdc, &rcClient, sbaritem);
				}
			}
			BitBlt(hdcReal, 0, 0, width, height, pContainer->m_cachedDC, 0, 0, SRCCOPY);
			EndPaint(hwndDlg, &ps);
			return 0;
		}
		break;

	case WM_NCLBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCMOUSEHOVER:
	case WM_NCMOUSEMOVE:
		if (pContainer && CSkin::m_frameSkins) {
			POINT pt;
			GetCursorPos(&pt);

			RECT rcWindow;
			GetWindowRect(hwndDlg, &rcWindow);

			memcpy(&pContainer->m_oldbuttons[0], &pContainer->m_buttons[0], sizeof(TitleBtn) * 3);
			memset(&pContainer->m_buttons[0], 0, (sizeof(TitleBtn) * 3));

			if (pt.x >= (rcWindow.left + pContainer->m_rcMin.left) && pt.x <= (rcWindow.left + pContainer->m_rcClose.right) && pt.y < rcWindow.top + 24 && wParam != HTTOPRIGHT) {
				LRESULT result = 0; //DefWindowProc(hwndDlg, msg, wParam, lParam);
				HDC hdc = GetWindowDC(hwndDlg);
				LONG left = rcWindow.left;

				pt.y = 10;
				bool isMin = pt.x >= left + pContainer->m_rcMin.left && pt.x <= left + pContainer->m_rcMin.right;
				bool isMax = pt.x >= left + pContainer->m_rcMax.left && pt.x <= left + pContainer->m_rcMax.right;
				bool isClose = pt.x >= left + pContainer->m_rcClose.left && pt.x <= left + pContainer->m_rcClose.right;

				if (msg == WM_NCMOUSEMOVE) {
					if (isMax)
						pContainer->m_buttons[BTN_MAX].isHot = TRUE;
					else if (isMin)
						pContainer->m_buttons[BTN_MIN].isHot = TRUE;
					else if (isClose)
						pContainer->m_buttons[BTN_CLOSE].isHot = TRUE;
				}
				else if (msg == WM_NCLBUTTONDOWN) {
					if (isMax)
						pContainer->m_buttons[BTN_MAX].isPressed = TRUE;
					else if (isMin)
						pContainer->m_buttons[BTN_MIN].isPressed = TRUE;
					else if (isClose)
						pContainer->m_buttons[BTN_CLOSE].isPressed = TRUE;
				}
				else if (msg == WM_NCLBUTTONUP) {
					if (isMin)
						SendMessage(hwndDlg, WM_SYSCOMMAND, SC_MINIMIZE, 0);
					else if (isMax) {
						if (IsZoomed(hwndDlg))
							PostMessage(hwndDlg, WM_SYSCOMMAND, SC_RESTORE, 0);
						else
							PostMessage(hwndDlg, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
					}
					else if (isClose)
						PostMessage(hwndDlg, WM_SYSCOMMAND, SC_CLOSE, 0);
				}
				for (int i = 0; i < 3; i++) {
					if (pContainer->m_buttons[i].isHot != pContainer->m_oldbuttons[i].isHot) {
						RECT *rc;
						HICON hIcon;

						switch (i) {
						case 0:
							rc = &pContainer->m_rcMin;
							hIcon = CSkin::m_minIcon;
							break;
						case 1:
							rc = &pContainer->m_rcMax;
							hIcon = CSkin::m_maxIcon;
							break;
						case 2:
							rc = &pContainer->m_rcClose;
							hIcon = CSkin::m_closeIcon;
							break;
						default:
							continue; // shall never happen
						}
						if (rc) {
							CSkinItem *item = &SkinItems[pContainer->m_buttons[i].isPressed ? ID_EXTBKTITLEBUTTONPRESSED : (pContainer->m_buttons[i].isHot ? ID_EXTBKTITLEBUTTONMOUSEOVER : ID_EXTBKTITLEBUTTON)];
							CSkin::DrawItem(hdc, rc, item);
							DrawIconEx(hdc, rc->left + ((rc->right - rc->left) / 2 - 8), rc->top + ((rc->bottom - rc->top) / 2 - 8), hIcon, 16, 16, 0, nullptr, DI_NORMAL);
						}
					}
				}
				ReleaseDC(hwndDlg, hdc);
				return result;
			}
			else {
				LRESULT result = DefWindowProc(hwndDlg, msg, wParam, lParam);
				RedrawWindow(hwndDlg, nullptr, nullptr, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_NOCHILDREN);
				return result;
			}
		}
		break;

	case WM_SETCURSOR:
		if (CSkin::m_frameSkins && (HWND)wParam == hwndDlg) {
			DefWindowProc(hwndDlg, msg, wParam, lParam);
			RedrawWindow(hwndDlg, nullptr, nullptr, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_NOCHILDREN);
			return 1;
		}
		break;

	case WM_NCCALCSIZE:
		if (!CSkin::m_frameSkins)
			break;

		if (wParam) {
			NCCALCSIZE_PARAMS *ncsp = (NCCALCSIZE_PARAMS *)lParam;
			DefWindowProc(hwndDlg, msg, wParam, lParam);

			RECT *rc = &ncsp->rgrc[0];
			rc->left += CSkin::m_realSkinnedFrame_left;
			rc->right -= CSkin::m_realSkinnedFrame_right;
			rc->bottom -= CSkin::m_realSkinnedFrame_bottom;
			rc->top += CSkin::m_realSkinnedFrame_caption;
			return TRUE;
		}

		return DefWindowProc(hwndDlg, msg, wParam, lParam);

	case WM_NCACTIVATE:
		if (pContainer) {
			pContainer->m_ncActive = wParam;
			if (CSkin::m_skinEnabled && CSkin::m_frameSkins) {
				SendMessage(hwndDlg, WM_NCPAINT, 0, 0);
				return 1;
			}
		}
		break;

	case WM_SETTEXT:
	case WM_SETICON:
		if (CSkin::m_frameSkins) {
			DefWindowProc(hwndDlg, msg, wParam, lParam);
			RedrawWindow(hwndDlg, nullptr, nullptr, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOCHILDREN);
			return 0;
		}
		break;

	case WM_NCHITTEST:
		if (pContainer && (pContainer->cfg.flags.m_bNoTitle)) {
			RECT r;
			GetWindowRect(hwndDlg, &r);

			POINT pt;
			GetCursorPos(&pt);
			int clip = CSkin::m_bClipBorder;
			if (pt.y <= r.bottom && pt.y >= r.bottom - clip - 6) {
				if (pt.x > r.left + clip + 10 && pt.x < r.right - clip - 10)
					return HTBOTTOM;
				if (pt.x < r.left + clip + 10)
					return HTBOTTOMLEFT;
				if (pt.x > r.right - clip - 10)
					return HTBOTTOMRIGHT;

			}
			else if (pt.y >= r.top && pt.y <= r.top + 6) {
				if (pt.x > r.left + clip + 10 && pt.x < r.right - clip - 10)
					return HTTOP;
				if (pt.x < r.left + clip + 10)
					return HTTOPLEFT;
				if (pt.x > r.right - clip - 10)
					return HTTOPRIGHT;
			}
			else if (pt.x >= r.left && pt.x <= r.left + clip + 6)
				return HTLEFT;
			else if (pt.x >= r.right - clip - 6 && pt.x <= r.right)
				return HTRIGHT;
		}
		break;

	case WM_TIMER:
		if (wParam == (WPARAM)pContainer && pContainer->m_hwndStatus) {
			SendMessage(pContainer->m_hwnd, WM_SIZE, 0, 0);
			SendMessage(pContainer->m_hwndStatus, SB_SETTEXT, (WPARAM)(SBT_OWNERDRAW) | 2, 0);
			InvalidateRect(pContainer->m_hwndStatus, nullptr, TRUE);
			KillTimer(hwndDlg, wParam);
		}
		break;

	case 0xae: // must be some undocumented message - seems it messes with the title bar...
		if (CSkin::m_frameSkins)
			return 0;
	}
	return mir_callNextSubclass(hwndDlg, ContainerWndProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// container window procedure...

static INT_PTR CALLBACK DlgProcContainer(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	POINT pt;
	MCONTACT hContact;
	CMsgDialog *dat;

	TContainerData *pContainer = (TContainerData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		fForceOverlayIcons = M.GetByte("forceTaskBarStatusOverlays", 0) ? true : false;

		pContainer = (TContainerData*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)pContainer);
		mir_subclassWindow(hwndDlg, ContainerWndProc);
		pContainer->InitDialog(hwndDlg);

		// prevent ugly back background being visible while tabbed clients are created
		if (M.isAero()) {
			MARGINS m = { -1 };
			CMimAPI::m_pfnDwmExtendFrameIntoClientArea(hwndDlg, &m);
		}
		return TRUE;

	case WM_SIZE:
		if (IsIconic(hwndDlg))
			pContainer->cfg.flags.m_bDeferredResize = true;
		else
			pContainer->Resize(wParam == SIZE_RESTORED, LOWORD(lParam));
		break;

	case WM_NOTIFY:
		if (pContainer == nullptr)
			break;
		if (pContainer->m_pMenuBar) {
			LRESULT processed = pContainer->m_pMenuBar->processMsg(msg, wParam, lParam);
			if (processed != -1) {
				SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, processed);
				return(processed);
			}
		}

		if (pContainer->m_hwndStatus != nullptr && ((LPNMHDR)lParam)->hwndFrom == pContainer->m_hwndStatus) {
			switch (((LPNMHDR)lParam)->code) {
			case NM_CLICK:
			case NM_RCLICK:
				NMMOUSE *nm = (NMMOUSE*)lParam;
				int nPanel;
				if (nm->dwItemSpec == 0xFFFFFFFE) {
					nPanel = 2;
					SendMessage(pContainer->m_hwndStatus, SB_GETRECT, nPanel, (LPARAM)&rc);
					if (nm->pt.x > rc.left && nm->pt.x < rc.right)
						goto panel_found;
					else
						return FALSE;
				}
				else nPanel = nm->dwItemSpec;
panel_found:
				if (nPanel == 2) {
					dat = (CMsgDialog*)GetWindowLongPtr(pContainer->m_hwndActive, GWLP_USERDATA);
					SendMessage(pContainer->m_hwndStatus, SB_GETRECT, nPanel, (LPARAM)&rc);
					if (dat)
						dat->CheckStatusIconClick(nm->pt, rc, 2, ((LPNMHDR)lParam)->code);
				}
				else if (((LPNMHDR)lParam)->code == NM_RCLICK) {
					GetCursorPos(&pt);
					hContact = 0;
					SendMessage(pContainer->m_hwndActive, DM_QUERYHCONTACT, 0, (LPARAM)&hContact);
					if (hContact) {
						int iSel = 0;
						HMENU hMenu = Menu_BuildContactMenu(hContact);
						iSel = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, nullptr);
						if (iSel)
							Clist_MenuProcessCommand(LOWORD(iSel), MPCF_CONTACTMENU, hContact);
						DestroyMenu(hMenu);
					}
				}
				return TRUE;
			}
			break;
		}

		switch (((LPNMHDR)lParam)->code) {
		case TCN_SELCHANGE:
			if (HWND hDlg = GetTabWindow(pContainer->m_hwndTabs, TabCtrl_GetCurSel(pContainer->m_hwndTabs))) {
				if (hDlg != pContainer->m_hwndActive)
					if (pContainer->m_hwndActive && IsWindow(pContainer->m_hwndActive))
						ShowWindow(pContainer->m_hwndActive, SW_HIDE);

				pContainer->m_hwndActive = hDlg;
				SendMessage(hDlg, DM_SAVESIZE, 0, 1);
				ShowWindow(hDlg, SW_SHOW);
				if (!IsIconic(hwndDlg))
					SetFocus(pContainer->m_hwndActive);
			}
			SendMessage(pContainer->m_hwndTabs, EM_VALIDATEBOTTOM, 0, 0);
			return 0;

		// tooltips
		case NM_RCLICK:
			bool fFromSidebar = false;

			GetCursorPos(&pt);
			HMENU subMenu = GetSubMenu(PluginConfig.g_hMenuContext, 0);

			HWND hDlg = nullptr;
			dat = nullptr;
			if (((LPNMHDR)lParam)->idFrom == IDC_MSGTABS) {
				hDlg = GetTabWindow(pContainer->m_hwndTabs, GetTabItemFromMouse(pContainer->m_hwndTabs, &pt));
				if (hDlg && IsWindow(hDlg))
					dat = (CMsgDialog*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
			}
			// sent from a sidebar button (RMB click) instead of the tab control
			else if (((LPNMHDR)lParam)->idFrom == 5000) {
				TSideBarNotify* n = reinterpret_cast<TSideBarNotify *>(lParam);
				dat = n->dat;
				fFromSidebar = true;
			}

			if (dat)
				dat->MsgWindowUpdateMenu(subMenu, MENU_TABCONTEXT);

			int iSelection = TrackPopupMenu(subMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, nullptr);
			if (iSelection >= IDM_CONTAINERMENU) {
				char szIndex[10];
				itoa(iSelection - IDM_CONTAINERMENU, szIndex, 10);
				if (iSelection - IDM_CONTAINERMENU >= 0) {
					ptrW tszName(db_get_wsa(0, CONTAINER_KEY, szIndex));
					if (hDlg && tszName != nullptr)
						dat->SwitchToContainer(tszName);
				}
				return 1;
			}
			
			switch (iSelection) {
			case ID_TABMENU_CLOSETAB:
				if (fFromSidebar && dat)
					SendMessage(dat->GetHwnd(), WM_CLOSE, 1, 0);
				else
					pContainer->CloseTabByMouse(&pt);
				break;
			case ID_TABMENU_CLOSEOTHERTABS:
				if (dat)
					CloseOtherTabs(pContainer->m_hwndTabs, *dat);
				break;
			case ID_TABMENU_SAVETABPOSITION:
				if (dat)
					db_set_dw(dat->m_hContact, SRMSGMOD_T, "tabindex", dat->m_iTabID * 100);
				break;
			case ID_TABMENU_CLEARSAVEDTABPOSITION:
				if (dat)
					db_unset(dat->m_hContact, SRMSGMOD_T, "tabindex");
				break;
			case ID_TABMENU_LEAVECHATROOM:
				if (dat && dat->isChat()) {
					char *szProto = Proto_GetBaseAccountName(dat->m_hContact);
					if (szProto)
						CallProtoService(szProto, PS_LEAVECHAT, dat->m_hContact, 0);
				}
				break;
			case ID_TABMENU_ATTACHTOCONTAINER:
				hDlg = GetTabWindow(pContainer->m_hwndTabs, GetTabItemFromMouse(pContainer->m_hwndTabs, &pt));
				if (hDlg)
					((CMsgDialog *)GetWindowLongPtr(hDlg, GWLP_USERDATA))->SelectContainer();
				break;
			case ID_TABMENU_CONTAINEROPTIONS:
				pContainer->OptionsDialog();
				break;
			case ID_TABMENU_CLOSECONTAINER:
				SendMessage(hwndDlg, WM_CLOSE, 0, 0);
				break;
			}
			InvalidateRect(pContainer->m_hwndTabs, nullptr, FALSE);
			return 1;
		}
		break;

	case WM_COMMAND:
		{
			bool fProcessContactMenu = pContainer->m_pMenuBar->isContactMenu();
			bool fProcessMainMenu = pContainer->m_pMenuBar->isMainMenu();
			pContainer->m_pMenuBar->Cancel();

			dat = (CMsgDialog*)GetWindowLongPtr(pContainer->m_hwndActive, GWLP_USERDATA);
			uint32_t dwOldFlags = pContainer->cfg.flags.dw;

			auto &f = pContainer->cfg.flags;

			if (dat) {
				if (fProcessContactMenu)
					return Clist_MenuProcessCommand(LOWORD(wParam), MPCF_CONTACTMENU, dat->m_hContact);
				if (fProcessMainMenu)
					return Clist_MenuProcessCommand(LOWORD(wParam), MPCF_MAINMENU, 0);
				if (dat->MsgWindowMenuHandler(LOWORD(wParam), MENU_PICMENU) == 1)
					break;
			}
			SendMessage(pContainer->m_hwndActive, DM_QUERYHCONTACT, 0, (LPARAM)&hContact);
			if (LOWORD(wParam) == IDC_TBFIRSTUID - 1)
				break;

			switch (LOWORD(wParam)) {
			case IDC_TOGGLESIDEBAR:
				GetWindowRect(hwndDlg, &rc);
				{
					LONG dwNewLeft;
					bool fVisible = pContainer->m_pSideBar->isVisible();
					if (fVisible) {
						dwNewLeft = pContainer->m_pSideBar->getWidth();
						pContainer->m_pSideBar->setVisible(false);
					}
					else {
						pContainer->m_pSideBar->setVisible(true);
						dwNewLeft = -(pContainer->m_pSideBar->getWidth());
					}

					pContainer->m_preSIZE.cx = pContainer->m_preSIZE.cy = 0;
					pContainer->m_oldDCSize.cx = pContainer->m_oldDCSize.cy = 0;
				}

				PostMessage(hwndDlg, WM_SIZE, 0, 1);
				break;

			case IDC_SIDEBARDOWN:
			case IDC_SIDEBARUP:
				{
					HWND hwnd = GetFocus();
					pContainer->m_pSideBar->processScrollerButtons(LOWORD(wParam));
					SetFocus(hwnd);
				}
				break;

			case IDC_CLOSE:
				SendMessage(hwndDlg, WM_SYSCOMMAND, SC_CLOSE, 0);
				break;
			
			case IDC_MINIMIZE:
				PostMessage(hwndDlg, WM_SYSCOMMAND, SC_MINIMIZE, 0);
				break;
			
			case IDC_MAXIMIZE:
				SendMessage(hwndDlg, WM_SYSCOMMAND, IsZoomed(hwndDlg) ? SC_RESTORE : SC_MAXIMIZE, 0);
				break;
			
			case IDOK:
				SendMessage(pContainer->m_hwndActive, WM_COMMAND, wParam, lParam);      // pass the IDOK command to the active child - fixes the "enter not working
				break;
			
			case ID_FILE_CLOSEMESSAGESESSION:
				PostMessage(pContainer->m_hwndActive, WM_CLOSE, 0, 1);
				break;
			
			case ID_FILE_CLOSE:
				PostMessage(hwndDlg, WM_CLOSE, 0, 1);
				break;
			
			case ID_VIEW_SHOWSTATUSBAR:
				f.m_bNoStatusBar = !f.m_bNoStatusBar;
				pContainer->ApplySetting(true);
				break;
			
			case ID_VIEW_VERTICALMAXIMIZE:
				f.m_bVerticalMax = !f.m_bVerticalMax;
				pContainer->ApplySetting();
				break;
			
			case ID_VIEW_BOTTOMTOOLBAR:
				f.m_bBottomToolbar = !f.m_bBottomToolbar;
				pContainer->ApplySetting();
				Srmm_Broadcast(DM_CONFIGURETOOLBAR, 0, 1);
				return 0;
			
			case ID_VIEW_SHOWTOOLBAR:
				f.m_bHideToolbar = !f.m_bHideToolbar;
				pContainer->ApplySetting();
				Srmm_Broadcast(DM_CONFIGURETOOLBAR, 0, 1);
				return 0;
			
			case ID_VIEW_SHOWMENUBAR:
				f.m_bNoMenuBar = !f.m_bNoMenuBar;
				pContainer->ApplySetting(true);
				break;
			
			case ID_VIEW_SHOWTITLEBAR:
				f.m_bNoTitle = !f.m_bNoTitle;
				pContainer->ApplySetting(true);
				break;
			
			case ID_VIEW_TABSATBOTTOM:
				f.m_bTabsBottom = !f.m_bTabsBottom;
				pContainer->ApplySetting();
				break;
			
			case ID_VIEW_SHOWMULTISENDCONTACTLIST:
				SendMessage(pContainer->m_hwndActive, WM_COMMAND, MAKEWPARAM(IDC_SENDMENU, ID_SENDMENU_SENDTOMULTIPLEUSERS), 0);
				break;
			
			case ID_VIEW_STAYONTOP:
				SendMessage(hwndDlg, WM_SYSCOMMAND, IDM_STAYONTOP, 0);
				break;
			
			case ID_CONTAINER_CONTAINEROPTIONS:
				SendMessage(hwndDlg, WM_SYSCOMMAND, IDM_MOREOPTIONS, 0);
				break;
			
			case ID_EVENTPOPUPS_DISABLEALLEVENTPOPUPS:
				f.m_bDontReport = f.m_bDontReportUnfocused = f.m_bDontReportFocused = f.m_bAlwaysReportInactive = false;
				pContainer->ApplySetting();
				return 0;
			
			case ID_EVENTPOPUPS_SHOWPOPUPSIFWINDOWISMINIMIZED:
				f.m_bDontReport = !f.m_bDontReport;
				pContainer->ApplySetting();
				return 0;
			
			case ID_EVENTPOPUPS_SHOWPOPUPSIFWINDOWISUNFOCUSED:
				f.m_bDontReportUnfocused = !f.m_bDontReportUnfocused;
				pContainer->ApplySetting();
				return 0;
			
			case ID_EVENTPOPUPS_SHOWPOPUPSIFWINDOWISFOCUSED:
				f.m_bDontReportFocused = !f.m_bDontReportFocused;
				pContainer->ApplySetting();
				return 0;
			
			case ID_EVENTPOPUPS_SHOWPOPUPSFORALLINACTIVESESSIONS:
				f.m_bAlwaysReportInactive = !f.m_bAlwaysReportInactive;
				pContainer->ApplySetting();
				return 0;
			
			case ID_WINDOWFLASHING_DISABLEFLASHING:
				f.m_bNoFlash = true;
				f.m_bFlashAlways = false;
				pContainer->ApplySetting();
				return 0;
			
			case ID_WINDOWFLASHING_FLASHUNTILFOCUSED:
				f.m_bNoFlash = false;
				f.m_bFlashAlways = true;
				pContainer->ApplySetting();
				return 0;
			
			case ID_WINDOWFLASHING_USEDEFAULTVALUES:
				f.m_bNoFlash = f.m_bFlashAlways = false;
				pContainer->ApplySetting();
				return 0;
			
			case ID_OPTIONS_SAVECURRENTWINDOWPOSITIONASDEFAULT:
				{
					WINDOWPLACEMENT wp = { 0 };
					wp.length = sizeof(wp);
					if (GetWindowPlacement(hwndDlg, &wp)) {
						db_set_dw(0, SRMSGMOD_T, "splitx", wp.rcNormalPosition.left);
						db_set_dw(0, SRMSGMOD_T, "splity", wp.rcNormalPosition.top);
						db_set_dw(0, SRMSGMOD_T, "splitwidth", wp.rcNormalPosition.right - wp.rcNormalPosition.left);
						db_set_dw(0, SRMSGMOD_T, "splitheight", wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
					}
				}
				return 0;

			case ID_VIEW_INFOPANEL:
				if (dat) {
					GetWindowRect(pContainer->m_hwndActive, &rc);
					pt.x = rc.left + 10;
					pt.y = rc.top + dat->m_pPanel.getHeight() - 10;
					dat->m_pPanel.invokeConfigDialog(pt);
				}
				return 0;

				// commands from the message log popup will be routed to the
				// message log menu handler
			case ID_MESSAGELOGSETTINGS_FORTHISCONTACT:
			case ID_MESSAGELOGSETTINGS_GLOBAL:
				if (dat) {
					dat->MsgWindowMenuHandler((int)LOWORD(wParam), MENU_LOGMENU);
					return 1;
				}
				break;
			}

			if (f.dw != dwOldFlags)
				pContainer->Configure();
		}
		break;

	case WM_ENTERSIZEMOVE:
		GetClientRect(pContainer->m_hwndTabs, &rc);
		{
			SIZE sz;
			sz.cx = rc.right - rc.left;
			sz.cy = rc.bottom - rc.top;
			pContainer->m_oldSize = sz;
			pContainer->m_bSizingLoop = TRUE;
		}
		break;

	case WM_EXITSIZEMOVE:
		GetClientRect(pContainer->m_hwndTabs, &rc);
		if (!((rc.right - rc.left) == pContainer->m_oldSize.cx && (rc.bottom - rc.top) == pContainer->m_oldSize.cy)) {
			dat = (CMsgDialog*)GetWindowLongPtr(pContainer->m_hwndActive, GWLP_USERDATA);
			if (dat)
				dat->DM_ScrollToBottom(0, 0);
			SendMessage(pContainer->m_hwndActive, WM_SIZE, 0, 0);
		}
		pContainer->m_bSizingLoop = FALSE;
		break;

		// determine minimum and maximum size limits
		// 1) for maximizing the window when the "vertical maximize" option is set
		// 2) to limit the minimum height when manually resizing the window
		// (this avoids overlapping of controls inside the window and ensures
		// that at least 2 lines of the message log are always visible).
	case WM_GETMINMAXINFO:
		RECT rcWindow;
		{
			RECT rcClient = { 0 };

			MINMAXINFO *mmi = (MINMAXINFO *)lParam;
			mmi->ptMinTrackSize.x = 275;
			mmi->ptMinTrackSize.y = 130;
			GetClientRect(pContainer->m_hwndTabs, &rc);
			if (pContainer->m_hwndActive)								// at container creation time, there is no hwndActive yet..
				GetClientRect(pContainer->m_hwndActive, &rcClient);
			GetWindowRect(hwndDlg, &rcWindow);
			pt.y = rc.top;
			TabCtrl_AdjustRect(pContainer->m_hwndTabs, FALSE, &rc);
			// uChildMinHeight holds the min height for the client window only
			// so let's add the container's vertical padding (title bar, tab bar,
			// window border, status bar) to this value
			if (pContainer->m_hwndActive)
				mmi->ptMinTrackSize.y = pContainer->m_uChildMinHeight + (pContainer->m_hwndActive ? ((rcWindow.bottom - rcWindow.top) - rcClient.bottom) : 0);

			if (pContainer->cfg.flags.m_bVerticalMax || (GetKeyState(VK_CONTROL) & 0x8000)) {
				RECT rcDesktop = { 0 };
				BOOL fDesktopValid = FALSE;
				int monitorXOffset = 0;
				WINDOWPLACEMENT wp = { 0 };

				HMONITOR hMonitor = MonitorFromWindow(hwndDlg, 2);
				if (hMonitor) {
					MONITORINFO mi = { 0 };
					mi.cbSize = sizeof(mi);
					GetMonitorInfoA(hMonitor, &mi);
					rcDesktop = mi.rcWork;
					OffsetRect(&rcDesktop, -mi.rcMonitor.left, -mi.rcMonitor.top);
					monitorXOffset = mi.rcMonitor.left;
					fDesktopValid = TRUE;
				}
				if (!fDesktopValid)
					SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);

				wp.length = sizeof(wp);
				GetWindowPlacement(hwndDlg, &wp);
				mmi->ptMaxSize.y = rcDesktop.bottom - rcDesktop.top;
				mmi->ptMaxSize.x = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
				mmi->ptMaxPosition.x = wp.rcNormalPosition.left - monitorXOffset;
				mmi->ptMaxPosition.y = 0;
				if (IsIconic(hwndDlg)) {
					mmi->ptMaxPosition.x += rcDesktop.left;
					mmi->ptMaxPosition.y += rcDesktop.top;
				}

				// protect against invalid values...
				if (mmi->ptMinTrackSize.y < 50 || mmi->ptMinTrackSize.y > rcDesktop.bottom)
					mmi->ptMinTrackSize.y = 130;
			}
		}
		return 0;

	case WM_TIMER:
		if (wParam == TIMERID_HEARTBEAT) {
			if (GetForegroundWindow() != hwndDlg && (pContainer->cfg.autoCloseSeconds > 0) && !pContainer->m_bHidden) {
				BOOL fResult = TRUE;
				pContainer->BroadCastContainer(DM_CHECKAUTOHIDE, (WPARAM)pContainer->cfg.autoCloseSeconds, (LPARAM)&fResult);

				if (fResult && nullptr == pContainer->m_hWndOptions)
					PostMessage(hwndDlg, WM_CLOSE, 1, 0);
			}

			dat = (CMsgDialog*)GetWindowLongPtr(pContainer->m_hwndActive, GWLP_USERDATA);
			if (dat && !dat->isChat()) {
				if (dat->m_idle && pContainer->m_hwndActive && IsWindow(pContainer->m_hwndActive))
					dat->m_pPanel.Invalidate(TRUE);
			}
			else if (dat)
				dat->UpdateStatusBar();
		}
		break;

	case WM_SYSCOMMAND:
		switch (wParam) {
		case IDM_STAYONTOP:
			SetWindowPos(hwndDlg, (pContainer->cfg.flags.m_bSticky) ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			CheckMenuItem(GetSystemMenu(hwndDlg, FALSE), IDM_STAYONTOP, (pContainer->cfg.flags.m_bSticky) ? MF_UNCHECKED : MF_CHECKED);

			pContainer->cfg.flags.m_bSticky = !pContainer->cfg.flags.m_bSticky;
			pContainer->ApplySetting();
			break;
		case IDM_NOTITLE:
			pContainer->m_oldSize.cx = 0;
			pContainer->m_oldSize.cy = 0;

			CheckMenuItem(GetSystemMenu(hwndDlg, FALSE), IDM_NOTITLE, (pContainer->cfg.flags.m_bNoTitle) ? MF_UNCHECKED : MF_CHECKED);

			pContainer->cfg.flags.m_bNoTitle = !pContainer->cfg.flags.m_bNoTitle;
			pContainer->ApplySetting(true);
			break;
		case IDM_MOREOPTIONS:
			if (IsIconic(hwndDlg))
				SendMessage(hwndDlg, WM_SYSCOMMAND, SC_RESTORE, 0);
			pContainer->OptionsDialog();
			break;
		case SC_MAXIMIZE:
			pContainer->m_oldSize.cx = pContainer->m_oldSize.cy = 0;
			break;
		case SC_RESTORE:
			pContainer->m_oldSize.cx = pContainer->m_oldSize.cy = 0;
			pContainer->ClearMargins();
			break;
		case SC_MINIMIZE:
			dat = (CMsgDialog*)(GetWindowLongPtr(pContainer->m_hwndActive, GWLP_USERDATA));
			if (dat) {
				GetWindowRect(pContainer->m_hwndActive, &pContainer->m_rcLogSaved);
				pContainer->m_ptLogSaved.x = pContainer->m_rcLogSaved.left;
				pContainer->m_ptLogSaved.y = pContainer->m_rcLogSaved.top;
				ScreenToClient(hwndDlg, &pContainer->m_ptLogSaved);
			}
		}
		break;

	case WM_INITMENUPOPUP:
		pContainer->m_pMenuBar->setActive(reinterpret_cast<HMENU>(wParam));
		break;

	case WM_LBUTTONDOWN:
		if (pContainer->cfg.flags.m_bNoTitle) {
			GetCursorPos(&pt);
			return SendMessage(hwndDlg, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
		}
		break;

		// pass the WM_ACTIVATE msg to the active message dialog child
	case WM_NCACTIVATE:
		if (IsWindowVisible(hwndDlg))
			pContainer->m_bHidden = false;
		break;

	case WM_ACTIVATE:
		if (pContainer == nullptr)
			break;

		if (LOWORD(wParam == WA_INACTIVE))
			pContainer->BroadCastContainer(DM_CHECKINFOTIP, wParam, lParam);

		if (LOWORD(wParam == WA_INACTIVE) && (HWND)lParam != PluginConfig.g_hwndHotkeyHandler && GetParent((HWND)lParam) != hwndDlg) {
			BOOL fTransAllowed = !CSkin::m_skinEnabled || IsWinVerVistaPlus();

			if (pContainer->cfg.flags.m_bTransparent && fTransAllowed) {
				SetLayeredWindowAttributes(hwndDlg, Skin->getColorKey(), (uint8_t)HIWORD(pContainer->cfg.dwTransparency), (pContainer->cfg.flags.m_bTransparent ? LWA_ALPHA : 0));
			}
		}
		pContainer->m_hwndSaved = nullptr;

		if (LOWORD(wParam) != WA_ACTIVE) {
			pContainer->m_pMenuBar->Cancel();

			if (HWND hDlg = GetTabWindow(pContainer->m_hwndTabs, TabCtrl_GetCurSel(pContainer->m_hwndTabs)))
				SendMessage(hDlg, WM_ACTIVATE, WA_INACTIVE, 0);
			break;
		}

	case WM_MOUSEACTIVATE:
		if (pContainer != nullptr) {
			BOOL fTransAllowed = !CSkin::m_skinEnabled || IsWinVerVistaPlus();

			pContainer->FlashContainer(0, 0);
			pContainer->m_dwFlashingStarted = 0;
			pLastActiveContainer = pContainer;
			if (pContainer->cfg.flags.m_bDeferredTabSelect) {
				pContainer->cfg.flags.m_bDeferredTabSelect = false;
				SendMessage(hwndDlg, WM_SYSCOMMAND, SC_RESTORE, 0);

				NMHDR nmhdr = { pContainer->m_hwndTabs, IDC_MSGTABS, TCN_SELCHANGE };
				SendMessage(hwndDlg, WM_NOTIFY, 0, (LPARAM)&nmhdr);     // do it via a WM_NOTIFY / TCN_SELCHANGE to simulate user-activation
			}
			if (pContainer->cfg.flags.m_bDeferredResize) {
				pContainer->cfg.flags.m_bDeferredResize = false;
				SendMessage(hwndDlg, WM_SIZE, 0, 0);
			}

			if (pContainer->cfg.flags.m_bTransparent && fTransAllowed) {
				uint32_t trans = LOWORD(pContainer->cfg.dwTransparency);
				SetLayeredWindowAttributes(hwndDlg, Skin->getColorKey(), (uint8_t)trans, (pContainer->cfg.flags.m_bTransparent ? LWA_ALPHA : 0));
			}
			if (pContainer->cfg.flags.m_bNeedsUpdateTitle) {
				pContainer->cfg.flags.m_bNeedsUpdateTitle = false;
				if (pContainer->m_hwndActive) {
					hContact = 0;
					SendMessage(pContainer->m_hwndActive, DM_QUERYHCONTACT, 0, (LPARAM)&hContact);
					if (hContact)
						pContainer->UpdateTitle(hContact);
				}
			}
			
			HWND hDlg = GetTabWindow(pContainer->m_hwndTabs, TabCtrl_GetCurSel(pContainer->m_hwndTabs));
			if (pContainer->cfg.flags.m_bDeferredConfigure && hDlg) {
				pContainer->cfg.flags.m_bDeferredConfigure = false;
				pContainer->m_hwndActive = hDlg;
				SendMessage(hwndDlg, WM_SYSCOMMAND, SC_RESTORE, 0);
				if (pContainer->m_hwndActive != nullptr && IsWindow(pContainer->m_hwndActive)) {
					ShowWindow(pContainer->m_hwndActive, SW_SHOW);
					SetFocus(pContainer->m_hwndActive);
					SendMessage(pContainer->m_hwndActive, WM_ACTIVATE, WA_ACTIVE, 0);
					RedrawWindow(pContainer->m_hwndActive, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);
				}
			}
			else if (hDlg)
				SendMessage(hDlg, WM_ACTIVATE, WA_ACTIVE, 0);
		}
		break;

	case WM_MOUSEMOVE:
		// wine: fix for erase/paint tab on mouse enter/leave tab.
		GetCursorPos(&pt);
		ScreenToClient(pContainer->m_hwndTabs, &pt);
		SendMessage(pContainer->m_hwndTabs, WM_MOUSEMOVE, wParam, (LPARAM)&pt);
		break;

	case WM_PAINT:
		if (CSkin::m_skinEnabled || M.isAero()) {
			PAINTSTRUCT ps;
			BeginPaint(hwndDlg, &ps);
			EndPaint(hwndDlg, &ps);
			return 0;
		}
		break;

	case WM_ERASEBKGND:
		// avoid flickering of the menu bar when aero is active
		if (pContainer) {
			HDC hdc = (HDC)wParam;
			GetClientRect(hwndDlg, &rc);

			if (M.isAero()) {
				HDC hdcMem;
				HANDLE  hbp = CMimAPI::m_pfnBeginBufferedPaint(hdc, &rc, BPBF_TOPDOWNDIB, nullptr, &hdcMem);
				FillRect(hdcMem, &rc, CSkin::m_BrushBack);
				CSkin::FinalizeBufferedPaint(hbp, &rc);
			}
			else {
				if (CSkin::m_skinEnabled)
					CSkin::DrawItem(hdc, &rc, &SkinItems[ID_EXTBKCONTAINER]);
				else {
					CSkin::FillBack(hdc, &rc);
					if (pContainer->m_pSideBar->isActive() && pContainer->m_pSideBar->isVisible()) {

						HPEN hPen = ::CreatePen(PS_SOLID, 1, PluginConfig.m_cRichBorders ? PluginConfig.m_cRichBorders : ::GetSysColor(COLOR_3DSHADOW));
						HPEN hOldPen = reinterpret_cast<HPEN>(::SelectObject(hdc, hPen));
						LONG x = (pContainer->m_pSideBar->getFlags() & CSideBar::SIDEBARORIENTATION_LEFT ? pContainer->m_pSideBar->getWidth() - 2 + pContainer->m_tBorder_outer_left :
							rc.right - pContainer->m_pSideBar->getWidth() + 1 - pContainer->m_tBorder_outer_right);
						::MoveToEx(hdc, x, rc.top, nullptr);
						::LineTo(hdc, x, rc.bottom);
						::SelectObject(hdc, hOldPen);
						::DeleteObject(hPen);
					}
				}
			}
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 1);
			return TRUE;
		}
		break;

	case DM_OPTIONSAPPLIED:
		char szCname[40];
		wchar_t szTitleFormat[200];
		{
			wchar_t *szThemeName = nullptr;
			DBVARIANT dbv = { 0 };

			szTitleFormat[0] = 0;

			if (pContainer->m_isCloned && pContainer->m_hContactFrom != 0) {
				pContainer->cfg = PluginConfig.globalContainerSettings;

				pContainer->m_szRelThemeFile[0] = pContainer->m_szAbsThemeFile[0] = 0;
				mir_snprintf(szCname, "%s_theme", CONTAINER_PREFIX);
				if (!db_get_ws(pContainer->m_hContactFrom, SRMSGMOD_T, szCname, &dbv))
					szThemeName = dbv.pwszVal;
			}
			else {
				pContainer->ReadPrivateSettings(false);
				if (szThemeName == nullptr) {
					mir_snprintf(szCname, "%s%d_theme", CONTAINER_PREFIX, pContainer->m_iContainerIndex);
					if (!db_get_ws(0, SRMSGMOD_T, szCname, &dbv))
						szThemeName = dbv.pwszVal;
				}
			}

			if (szThemeName != nullptr) {
				PathToAbsoluteW(szThemeName, pContainer->m_szAbsThemeFile, M.getDataPath());
				wcsncpy_s(pContainer->m_szRelThemeFile, szThemeName, _TRUNCATE);
				db_free(&dbv);
			}
			else pContainer->m_szAbsThemeFile[0] = pContainer->m_szRelThemeFile[0] = 0;

			pContainer->m_ltr_templates = pContainer->m_rtl_templates = nullptr;
		}
		break;

	case DM_STATUSBARCHANGED:
		SendMessage(hwndDlg, WM_SIZE, 0, 0);

		GetWindowRect(hwndDlg, &rc);
		SetWindowPos(hwndDlg, nullptr, rc.left, rc.top, rc.right - rc.left, (rc.bottom - rc.top) + 1, SWP_NOZORDER | SWP_NOACTIVATE);
		SetWindowPos(hwndDlg, nullptr, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);
		RedrawWindow(hwndDlg, nullptr, nullptr, RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);

		if (pContainer->m_hwndStatus != nullptr && pContainer->m_hwndActive != nullptr)
			PostMessage(pContainer->m_hwndActive, DM_STATUSBARCHANGED, 0, 0);
		return 0;

	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
			if (dis->hwndItem == pContainer->m_hwndStatus && !pContainer->cfg.flags.m_bNoStatusBar) {
				dat = (CMsgDialog*)GetWindowLongPtr(pContainer->m_hwndActive, GWLP_USERDATA);
				if (dat)
					dat->DrawStatusIcons(dis->hDC, dis->rcItem, 2);
				return TRUE;
			}
		}
		return Menu_DrawItem(lParam);

	case WM_MEASUREITEM:
		return Menu_MeasureItem(lParam);

	case WM_MOUSEWHEEL:
		GetCursorPos(&pt);

		if (pContainer->cfg.flags.m_bSideBar) {
			RECT rc1;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_SIDEBARUP), &rc);
			GetWindowRect(GetDlgItem(hwndDlg, IDC_SIDEBARDOWN), &rc1);
			rc.bottom = rc1.bottom;
			if (PtInRect(&rc, pt)) {
				short amount = (short)(HIWORD(wParam));
				SendMessage(hwndDlg, WM_COMMAND, MAKELONG(amount > 0 ? IDC_SIDEBARUP : IDC_SIDEBARDOWN, 0), IDC_SRMM_MESSAGE);
				return 0;
			}
		}
		break;

	case WM_DESTROY:
		pContainer->m_hwnd = nullptr;
		pContainer->m_hwndActive = nullptr;
		if (pContainer->m_hwndStatus)
			DestroyWindow(pContainer->m_hwndStatus);

		// mir_free private theme...
		if (pContainer->m_theme.isPrivate) {
			mir_free(pContainer->m_ltr_templates);
			mir_free(pContainer->m_rtl_templates);
			mir_free(pContainer->m_theme.logFonts);
			mir_free(pContainer->m_theme.fontColors);
			mir_free(pContainer->m_theme.rtfFonts);
		}

		if (pContainer->m_hwndTip)
			DestroyWindow(pContainer->m_hwndTip);
		RemoveContainerFromList(pContainer);
		if (pContainer->m_cachedDC) {
			SelectObject(pContainer->m_cachedDC, pContainer->m_oldHBM);
			DeleteObject(pContainer->m_cachedHBM);
			DeleteDC(pContainer->m_cachedDC);
		}
		if (pContainer->m_cachedToolbarDC) {
			SelectObject(pContainer->m_cachedToolbarDC, pContainer->m_oldhbmToolbarBG);
			DeleteObject(pContainer->m_hbmToolbarBG);
			DeleteDC(pContainer->m_cachedToolbarDC);
		}
		return 0;

	case WM_NCDESTROY:
		delete pContainer;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		break;

	case WM_CLOSE:
		if (PluginConfig.m_bHideOnClose && !lParam) {
			ShowWindow(hwndDlg, SW_HIDE);
			pContainer->m_bHidden = true;
		}
		else {
			if (TabCtrl_GetItemCount(pContainer->m_hwndTabs) > 1) {
				LRESULT res = CWarning::show(CWarning::WARN_CLOSEWINDOW, MB_YESNOCANCEL | MB_ICONQUESTION);
				if (IDNO == res || IDCANCEL == res)
					break;
			}

			// dont ask if container is empty (no tabs)
			if (lParam == 0 && TabCtrl_GetItemCount(pContainer->m_hwndTabs) > 0) {
				int clients = TabCtrl_GetItemCount(pContainer->m_hwndTabs), iOpenJobs = 0;

				for (int i = 0; i < clients; i++) {
					HWND hDlg = GetTabWindow(pContainer->m_hwndTabs, i);
					if (hDlg && IsWindow(hDlg))
						SendMessage(hDlg, DM_CHECKQUEUEFORCLOSE, 0, (LPARAM)&iOpenJobs);
				}
				if (iOpenJobs && pContainer) {
					if (pContainer->m_exFlags & CNT_EX_CLOSEWARN)
						return TRUE;

					pContainer->m_exFlags |= CNT_EX_CLOSEWARN;
					LRESULT result = SendQueue::WarnPendingJobs(iOpenJobs);
					pContainer->m_exFlags &= ~CNT_EX_CLOSEWARN;
					if (result == IDNO)
						return TRUE;
				}
			}

			// save geometry information to the database...
			if (!pContainer->cfg.flags.m_bGlobalSize) {
				WINDOWPLACEMENT wp = { 0 };
				wp.length = sizeof(wp);
				if (GetWindowPlacement(hwndDlg, &wp) != 0) {
					if (pContainer->m_isCloned && pContainer->m_hContactFrom != 0) {
						HWND hDlg = GetTabWindow(pContainer->m_hwndTabs, TabCtrl_GetCurSel(pContainer->m_hwndTabs));
						SendMessage(hDlg, DM_QUERYHCONTACT, 0, (LPARAM)&hContact);
						db_set_b(hContact, SRMSGMOD_T, "splitmax", (uint8_t)((wp.showCmd == SW_SHOWMAXIMIZED) ? 1 : 0));

						for (int i = 0; i < TabCtrl_GetItemCount(pContainer->m_hwndTabs); i++) {
							if (hDlg = GetTabWindow(pContainer->m_hwndTabs, i)) {
								SendMessage(hDlg, DM_QUERYHCONTACT, 0, (LPARAM)&hContact);
								db_set_dw(hContact, SRMSGMOD_T, "splitx", wp.rcNormalPosition.left);
								db_set_dw(hContact, SRMSGMOD_T, "splity", wp.rcNormalPosition.top);
								db_set_dw(hContact, SRMSGMOD_T, "splitwidth", wp.rcNormalPosition.right - wp.rcNormalPosition.left);
								db_set_dw(hContact, SRMSGMOD_T, "splitheight", wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
							}
						}
					}
					else {
						char szCName[40];
						mir_snprintf(szCName, "%s%dx", CONTAINER_PREFIX, pContainer->m_iContainerIndex);
						db_set_dw(0, SRMSGMOD_T, szCName, wp.rcNormalPosition.left);
						mir_snprintf(szCName, "%s%dy", CONTAINER_PREFIX, pContainer->m_iContainerIndex);
						db_set_dw(0, SRMSGMOD_T, szCName, wp.rcNormalPosition.top);
						mir_snprintf(szCName, "%s%dwidth", CONTAINER_PREFIX, pContainer->m_iContainerIndex);
						db_set_dw(0, SRMSGMOD_T, szCName, wp.rcNormalPosition.right - wp.rcNormalPosition.left);
						mir_snprintf(szCName, "%s%dheight", CONTAINER_PREFIX, pContainer->m_iContainerIndex);
						db_set_dw(0, SRMSGMOD_T, szCName, wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);

						db_set_b(0, SRMSGMOD_T, "splitmax", (uint8_t)((wp.showCmd == SW_SHOWMAXIMIZED) ? 1 : 0));
					}
				}
			}

			// clear temp flags which should NEVER be saved...
			if (pContainer->m_isCloned && pContainer->m_hContactFrom != 0) {
				pContainer->cfg.flags.m_bDeferredConfigure = pContainer->cfg.flags.m_bCreateMinimized = pContainer->cfg.flags.m_bDeferredResize = pContainer->cfg.flags.m_bCreateCloned = false;
				for (int i = 0; i < TabCtrl_GetItemCount(pContainer->m_hwndTabs); i++) {
					if (HWND hDlg = GetTabWindow(pContainer->m_hwndTabs, i)) {
						SendMessage(hDlg, DM_QUERYHCONTACT, 0, (LPARAM)&hContact);

						char szCName[40];
						mir_snprintf(szCName, "%s_theme", CONTAINER_PREFIX);
						if (mir_wstrlen(pContainer->m_szRelThemeFile) > 1) {
							if (pContainer->m_fPrivateThemeChanged == TRUE) {
								PathToRelativeW(pContainer->m_szRelThemeFile, pContainer->m_szAbsThemeFile, M.getDataPath());
								db_set_ws(hContact, SRMSGMOD_T, szCName, pContainer->m_szRelThemeFile);
								pContainer->m_fPrivateThemeChanged = FALSE;
							}
						}
						else {
							db_unset(hContact, SRMSGMOD_T, szCName);
							pContainer->m_fPrivateThemeChanged = FALSE;
						}
					}
				}
			}
			else pContainer->SaveSettings(CONTAINER_PREFIX);
			DestroyWindow(hwndDlg);
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CreateContainer MUST allocate a ContainerWindowData and pass its address
// to CreateDialogParam() via the LPARAM. It also adds the struct to the linked list
// of containers.
//
// The WM_DESTROY handler of the container DlgProc is responsible for mir_free()'ing the
// pointer and for removing the struct from the linked list.

TContainerData* TSAPI CreateContainer(const wchar_t *name, int iTemp, MCONTACT hContactFrom)
{
	if (CMimAPI::m_shutDown)
		return nullptr;

	TContainerData *pContainer = new TContainerData();
	wcsncpy_s(pContainer->m_wszName, name, _TRUNCATE);
	AppendToContainerList(pContainer);

	if (M.GetByte("limittabs", 0) && !mir_wstrcmp(name, L"default"))
		iTemp |= CNT_CREATEFLAG_CLONED;

	// save container name to the db
	if (!M.GetByte("singlewinmode", 0)) {
		int iFirstFree = -1, iFound = FALSE, i = 0;
		do {
			char szCounter[10];
			itoa(i, szCounter, 10);
			ptrW tszName(db_get_wsa(0, CONTAINER_KEY, szCounter));
			if (tszName == nullptr) {
				if (iFirstFree != -1) {
					pContainer->m_iContainerIndex = iFirstFree;
					itoa(iFirstFree, szCounter, 10);
				}
				else pContainer->m_iContainerIndex = i;

				db_set_ws(0, CONTAINER_KEY, szCounter, name);
				BuildContainerMenu();
				break;
			}

			if (!wcsncmp(tszName, name, CONTAINER_NAMELEN)) {
				pContainer->m_iContainerIndex = i;
				iFound = TRUE;
			}
			else if (!wcsncmp(tszName, L"**free**", CONTAINER_NAMELEN))
				iFirstFree = i;
		} while (++i && iFound == FALSE);
	}
	else {
		iTemp |= CNT_CREATEFLAG_CLONED;
		pContainer->m_iContainerIndex = 1;
	}

	if (iTemp & CNT_CREATEFLAG_MINIMIZED)
		pContainer->cfg.flags.m_bCreateMinimized = true;

	if (iTemp & CNT_CREATEFLAG_CLONED) {
		pContainer->cfg.flags.m_bCreateCloned = true;
		pContainer->m_hContactFrom = hContactFrom;
	}

	pContainer->m_hwnd = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_MSGCONTAINER), nullptr, DlgProcContainer, (LPARAM)pContainer);
	return pContainer;
}

/////////////////////////////////////////////////////////////////////////////////////////
// activates the tab belonging to the given client HWND (handle of the actual
// message window.

int TSAPI ActivateTabFromHWND(HWND hwndTab, HWND hwnd)
{
	int iItem = GetTabIndexFromHWND(hwndTab, hwnd);
	if (iItem >= 0) {
		TabCtrl_SetCurSel(hwndTab, iItem);

		NMHDR nmhdr = {};
		nmhdr.code = TCN_SELCHANGE;
		SendMessage(GetParent(hwndTab), WM_NOTIFY, 0, (LPARAM)&nmhdr);     // do it via a WM_NOTIFY / TCN_SELCHANGE to simulate user-activation
		return iItem;
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////

void TSAPI AutoCreateWindow(MCONTACT hContact, MEVENT hDbEvent)
{
	wchar_t szName[CONTAINER_NAMELEN + 1];
	GetContainerNameForContact(hContact, szName, CONTAINER_NAMELEN);

	bool bAllowAutoCreate = false;
	bool bAutoCreate = M.GetBool("autotabs", true);
	bool bAutoPopup = M.GetBool(SRMSGSET_AUTOPOPUP, SRMSGDEFSET_AUTOPOPUP);
	bool bAutoContainer = M.GetBool("autocontainer", true);

	uint32_t dwStatusMask = M.GetDword("autopopupmask", -1);
	if (dwStatusMask == -1)
		bAllowAutoCreate = true;
	else {
		char *szProto = Proto_GetBaseAccountName(hContact);
		if (szProto && !mir_strcmp(szProto, META_PROTO))
			szProto = Proto_GetBaseAccountName(db_mc_getSrmmSub(hContact));

		if (szProto) {
			int dwStatus = Proto_GetStatus(szProto);
			if (dwStatus == 0 || dwStatus <= ID_STATUS_OFFLINE)
				bAllowAutoCreate = true;
		}
	}

	if (bAllowAutoCreate && (bAutoPopup || bAutoCreate)) {
		if (bAutoPopup) {
			TContainerData *pContainer = FindContainerByName(szName);
			if (pContainer == nullptr)
				pContainer = CreateContainer(szName, 0, hContact);
			if (pContainer)
				CreateNewTabForContact(pContainer, hContact, true, true, false);
			return;
		}

		bool bPopup = M.GetByte("cpopup", 0) != 0;
		TContainerData *pContainer = FindContainerByName(szName);
		if (pContainer != nullptr)
			if (M.GetByte("limittabs", 0) && !wcsncmp(pContainer->m_wszName, L"default", 6))
				pContainer = FindMatchingContainer(L"default");

		if (pContainer == nullptr && bAutoContainer)
			pContainer = CreateContainer(szName, CNT_CREATEFLAG_MINIMIZED, hContact);

		if (pContainer != nullptr) {
			CreateNewTabForContact(pContainer, hContact, false, bPopup, true, hDbEvent);
			return;
		}
	}

	// no window created, simply add an unread event to contact list
	DBEVENTINFO dbei = {};
	db_event_get(hDbEvent, &dbei);

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
}

/////////////////////////////////////////////////////////////////////////////////////////

HMENU TSAPI BuildContainerMenu()
{
	if (PluginConfig.g_hMenuContainer != nullptr) {
		HMENU submenu = GetSubMenu(PluginConfig.g_hMenuContext, 0);
		RemoveMenu(submenu, 6, MF_BYPOSITION);
		DestroyMenu(PluginConfig.g_hMenuContainer);
		PluginConfig.g_hMenuContainer = nullptr;
	}

	// no container attach menu, if we are using the "clist group mode"
	if (M.GetByte("useclistgroups", 0) || M.GetByte("singlewinmode", 0))
		return nullptr;

	HMENU hMenu = CreateMenu();
	int i = 0;
	while (true) {
		char szCounter[10];
		itoa(i, szCounter, 10);
		ptrW tszName(db_get_wsa(0, CONTAINER_KEY, szCounter));
		if (tszName == nullptr)
			break;

		if (wcsncmp(tszName, L"**free**", CONTAINER_NAMELEN))
			AppendMenu(hMenu, MF_STRING, IDM_CONTAINERMENU + i, !mir_wstrcmp(tszName, L"default") ? TranslateT("Default container") : tszName);
		i++;
	}

	InsertMenu(PluginConfig.g_hMenuContext, ID_TABMENU_ATTACHTOCONTAINER, MF_POPUP, (UINT_PTR)hMenu, TranslateT("Attach to"));
	PluginConfig.g_hMenuContainer = hMenu;
	return hMenu;
}

/////////////////////////////////////////////////////////////////////////////////////////

void TSAPI CloseAllContainers()
{
	bool fOldHideSetting = PluginConfig.m_bHideOnClose;

	while (pFirstContainer != nullptr) {
		if (!IsWindow(pFirstContainer->m_hwnd))
			pFirstContainer = pFirstContainer->pNext;
		else {
			PluginConfig.m_bHideOnClose = false;
			::SendMessage(pFirstContainer->m_hwnd, WM_CLOSE, 0, 1);
		}
	}

	PluginConfig.m_bHideOnClose = fOldHideSetting;
}

/////////////////////////////////////////////////////////////////////////////////////////
// enumerates tabs and closes all of them, but the one in dat

void TSAPI CloseOtherTabs(HWND hwndTab, CMsgDialog &dat)
{
	for (int idxt = 0; idxt < dat.m_pContainer->m_iChilds;) {
		HWND otherTab = GetTabWindow(hwndTab, idxt);
		if (otherTab != nullptr && otherTab != dat.GetHwnd())
			SendMessage(otherTab, WM_CLOSE, 1, 0);
		else
			++idxt;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// cut off contact name to the option value set via Options->Tabbed messaging
// some people were requesting this, because really long contact list names
// are causing extraordinary wide tabs and these are looking ugly and wasting
// screen space.
//
// size = max length of target string

void TSAPI CutContactName(const wchar_t *oldname, wchar_t *newname, size_t size)
{
	if ((int)mir_wstrlen(oldname) <= PluginConfig.m_iTabNameLimit)
		wcsncpy_s(newname, size, oldname, _TRUNCATE);
	else {
		wchar_t fmt[30];
		mir_snwprintf(fmt, L"%%%d.%ds...", PluginConfig.m_iTabNameLimit, PluginConfig.m_iTabNameLimit);
		mir_snwprintf(newname, size, fmt, oldname);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void TSAPI DeleteContainer(int iIndex)
{
	char szIndex[10];
	itoa(iIndex, szIndex, 10);
	ptrW tszContainerName(db_get_wsa(0, CONTAINER_KEY, szIndex));
	if (tszContainerName == nullptr)
		return;

	db_set_ws(0, CONTAINER_KEY, szIndex, L"**free**");

	for (auto &hContact : Contacts()) {
		ptrW tszValue(db_get_wsa(hContact, SRMSGMOD_T, CONTAINER_SUBKEY));
		if (!mir_wstrcmp(tszValue, tszContainerName))
			db_unset(hContact, SRMSGMOD_T, CONTAINER_SUBKEY);
	}

	char szSetting[CONTAINER_NAMELEN + 30];
	mir_snprintf(szSetting, "%s%d_Flags", CONTAINER_PREFIX, iIndex);
	db_unset(0, SRMSGMOD_T, szSetting);
	mir_snprintf(szSetting, "%s%d_Trans", CONTAINER_PREFIX, iIndex);
	db_unset(0, SRMSGMOD_T, szSetting);
	mir_snprintf(szSetting, "%s%dwidth", CONTAINER_PREFIX, iIndex);
	db_unset(0, SRMSGMOD_T, szSetting);
	mir_snprintf(szSetting, "%s%dheight", CONTAINER_PREFIX, iIndex);
	db_unset(0, SRMSGMOD_T, szSetting);
	mir_snprintf(szSetting, "%s%dx", CONTAINER_PREFIX, iIndex);
	db_unset(0, SRMSGMOD_T, szSetting);
	mir_snprintf(szSetting, "%s%dy", CONTAINER_PREFIX, iIndex);
	db_unset(0, SRMSGMOD_T, szSetting);
}

/////////////////////////////////////////////////////////////////////////////////////////
// retrieve the container name for the given contact handle.
// if none is assigned, return the name of the default container

void TSAPI GetContainerNameForContact(MCONTACT hContact, wchar_t *szName, int iNameLen)
{
	// single window mode using cloned (temporary) containers
	if (M.GetByte("singlewinmode", 0)) {
		wcsncpy_s(szName, iNameLen, L"Message Session", _TRUNCATE);
		return;
	}

	// use clist group names for containers...
	if (M.GetByte("useclistgroups", 0)) {
		wcsncpy_s(szName, iNameLen, ptrW(db_get_wsa(hContact, "CList", "Group", L"default")), _TRUNCATE);
		return;
	}

	wcsncpy_s(szName, iNameLen, ptrW(db_get_wsa(hContact, SRMSGMOD_T, CONTAINER_SUBKEY, L"default")), _TRUNCATE);
}

/////////////////////////////////////////////////////////////////////////////////////////
// search the list of tabs and return the tab (by index) which "belongs" to the given
// hwnd. The hwnd is the handle of a message dialog childwindow. At creation,
// the dialog handle is stored in the TCITEM.lParam field, because we need
// to know the owner of the tab.
//
// hwndTab: handle of the tab control itself.
// hwnd: handle of a message dialog.
//
// returns the tab index (zero based), -1 if no tab is found (which SHOULD not
// really happen, but who knows... ;))

int TSAPI GetTabIndexFromHWND(HWND hwndTab, HWND hwnd)
{
	int iItems = TabCtrl_GetItemCount(hwndTab);

	for (int i = 0; i < iItems; i++) {
		HWND pDlg = GetTabWindow(hwndTab, i);
		if (pDlg == hwnd)
			return i;
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////

TContainerData* TSAPI FindContainerByName(const wchar_t *name)
{
	if (name == nullptr || mir_wstrlen(name) == 0)
		return nullptr;

	if (M.GetByte("singlewinmode", 0)) // single window mode - always return 0 and force a new container
		return nullptr;

	for (TContainerData *p = pFirstContainer; p; p = p->pNext)
		if (!wcsncmp(p->m_wszName, name, CONTAINER_NAMELEN))
			return p;

	// error, didn't find it.
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

void TSAPI RenameContainer(int iIndex, const wchar_t *szNew)
{
	if (mir_wstrlen(szNew) == 0)
		return;

	char szIndex[10];
	itoa(iIndex, szIndex, 10);
	ptrW tszContainerName(db_get_wsa(0, CONTAINER_KEY, szIndex));
	if (tszContainerName == nullptr)
		return;

	db_set_ws(0, CONTAINER_KEY, szIndex, szNew);

	for (auto &hContact : Contacts()) {
		ptrW tszValue(db_get_wsa(hContact, SRMSGMOD_T, CONTAINER_SUBKEY));
		if (!mir_wstrcmp(tszValue, tszContainerName))
			db_set_ws(hContact, SRMSGMOD_T, CONTAINER_SUBKEY, szNew);
	}
}
