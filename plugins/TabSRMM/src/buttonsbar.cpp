#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Global functions

static int CB_InitDefaultButtons(WPARAM, LPARAM)
{
	BBButton bbd = {};
	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISARROWBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_PROTOCOL;
	bbd.dwDefPos = 10;
	bbd.hIcon = Skin_GetIconHandle(SKINICON_OTHER_CONNECTING);
	bbd.pszModuleName = "Tabsrmm";
	bbd.pwszTooltip = LPGENW("Protocol button");
	g_plugin.addButton(&bbd);

	bbd.dwButtonID = IDC_NAME;
	bbd.dwDefPos = 20;
	bbd.hIcon = Skin_GetIconHandle(SKINICON_OTHER_USERDETAILS);
	bbd.pwszTooltip = LPGENW("Info button");
	g_plugin.addButton(&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISPUSHBUTTON | BBBF_CANBEHIDDEN | BBBF_CREATEBYID | BBBF_NOREADONLY;
	bbd.dwButtonID = IDC_SRMM_BOLD;
	bbd.dwDefPos = 40;
	bbd.hIcon = g_plugin.getIconHandle(IDI_FONTBOLD);
	bbd.pwszTooltip = LPGENW("Bold");
	g_plugin.addButton(&bbd);

	bbd.dwButtonID = IDC_SRMM_ITALICS;
	bbd.dwDefPos = 50;
	bbd.hIcon = g_plugin.getIconHandle(IDI_FONTITALIC);
	bbd.pwszTooltip = LPGENW("Italics");
	g_plugin.addButton(&bbd);

	bbd.dwButtonID = IDC_SRMM_UNDERLINE;
	bbd.dwDefPos = 60;
	bbd.hIcon = g_plugin.getIconHandle(IDI_FONTUNDERLINE);
	bbd.pwszTooltip = LPGENW("Underlined");
	g_plugin.addButton(&bbd);

	bbd.dwButtonID = IDC_SRMM_STRIKEOUT;
	bbd.dwDefPos = 70;
	bbd.hIcon = g_plugin.getIconHandle(IDI_STRIKEOUT);
	bbd.pwszTooltip = LPGENW("Strikethrough");
	g_plugin.addButton(&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_ISARROWBUTTON | BBBF_CREATEBYID | BBBF_NOREADONLY;
	bbd.dwButtonID = IDOK;
	bbd.dwDefPos = 10;
	bbd.hIcon = g_plugin.getIconHandle(IDI_SEND);
	bbd.pwszTooltip = LPGENW("Send message\nClick dropdown arrow for sending options");
	g_plugin.addButton(&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_CLOSE;
	bbd.dwDefPos = 20;
	bbd.hIcon = g_plugin.getIconHandle(IDI_CLOSEMSGDLG);
	bbd.pwszTooltip = LPGENW("Close session");
	g_plugin.addButton(&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID | BBBF_NOREADONLY;
	bbd.dwButtonID = IDC_QUOTE;
	bbd.dwDefPos = 30;
	bbd.hIcon = g_plugin.getIconHandle(IDI_QUOTE);
	bbd.pwszTooltip = LPGENW("Quote last message OR selected text");
	g_plugin.addButton(&bbd);

	bbd.dwButtonID = IDC_TIME;
	bbd.dwDefPos = 40;
	bbd.hIcon = g_plugin.getIconHandle(IDI_MSGLOGOPT);
	bbd.pwszTooltip = LPGENW("Message log options");
	g_plugin.addButton(&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_SRMM_HISTORY;
	bbd.dwDefPos = 50;
	bbd.hIcon = Skin_GetIconHandle(SKINICON_OTHER_HISTORY);
	bbd.pwszTooltip = LPGENW("View user's history");
	g_plugin.addButton(&bbd);

	// chat buttons
	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISSEPARATOR;
	bbd.dwButtonID = 1;
	bbd.pszModuleName = "tb_splitter";
	bbd.dwDefPos = 35;
	bbd.hIcon = nullptr;
	bbd.pwszTooltip = nullptr;
	g_plugin.addButton(&bbd);

	bbd.dwButtonID = 2;
	bbd.dwDefPos = 22;
	g_plugin.addButton(&bbd);

	bbd.dwButtonID = 3;
	bbd.dwDefPos = 71;
	g_plugin.addButton(&bbd);

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISPUSHBUTTON | BBBF_CREATEBYID | BBBF_NOREADONLY;
	bbd.dwButtonID = IDC_SRMM_COLOR;
	bbd.pszModuleName = "Tabsrmm";
	bbd.dwDefPos = 80;
	bbd.hIcon = g_plugin.getIconHandle(IDI_FONTCOLOR);
	bbd.pwszTooltip = LPGENW("Select text color");
	g_plugin.addButton(&bbd);

	bbd.dwButtonID = IDC_SRMM_BKGCOLOR;
	bbd.dwDefPos = 81;
	bbd.hIcon = g_plugin.getIconHandle(IDI_BKGCOLOR);
	bbd.pwszTooltip = LPGENW("Select background color");
	g_plugin.addButton(&bbd);

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_SRMM_SHOWNICKLIST;
	bbd.dwDefPos = 22;
	bbd.hIcon = g_plugin.getIconHandle(IDI_SHOWNICKLIST);
	bbd.pwszTooltip = LPGENW("Show/hide the nick list");
	g_plugin.addButton(&bbd);

	bbd.dwButtonID = IDC_SRMM_FILTER;
	bbd.dwDefPos = 24;
	bbd.hIcon = g_plugin.getIconHandle(IDI_FILTER2);
	bbd.pwszTooltip = LPGENW("Enable/disable the event filter");
	g_plugin.addButton(&bbd);

	bbd.dwButtonID = IDC_SRMM_CHANMGR;
	bbd.dwDefPos = 33;
	bbd.hIcon = g_plugin.getIconHandle(IDI_TOPICBUT);
	bbd.pwszTooltip = LPGENW("Control this room");
	g_plugin.addButton(&bbd);
	return 0;
}

void CMsgDialog::BB_InitDlgButtons()
{
	uint8_t gap = DPISCALEX_S(Srmm_GetButtonGap());

	RECT rcSplitter;
	GetWindowRect(GetDlgItem(m_hwnd, IDC_SPLITTERY), &rcSplitter);
	POINT ptSplitter = { 0, rcSplitter.top };
	ScreenToClient(m_hwnd, &ptSplitter);

	RECT rect;
	GetClientRect(m_hwnd, &rect);

	m_bbLSideWidth = m_bbRSideWidth = 0;

	CustomButtonData *cbd;
	for (int i = 0; cbd = Srmm_GetNthButton(i); i++) {
		HWND hwndButton = GetDlgItem(m_hwnd, cbd->m_dwButtonCID);
		if (hwndButton == nullptr)
			continue;

		if (!cbd->m_bHidden) {
			if (cbd->m_bRSided)
				m_bbRSideWidth += cbd->m_iButtonWidth + gap;
			else
				m_bbLSideWidth += cbd->m_iButtonWidth + gap;
		}
		if (!cbd->m_bHidden && !cbd->m_bCanBeHidden)
			m_iButtonBarReallyNeeds += cbd->m_iButtonWidth + gap;

		if (cbd->m_bSeparator)
			continue;

		CustomizeButton(hwndButton);

		if (cbd->m_dwArrowCID)
			SendMessage(hwndButton, BUTTONSETARROW, (cbd->m_dwButtonCID == IDOK) ? IDC_SENDMENU : cbd->m_dwArrowCID, 0);

		SendMessage(hwndButton, BUTTONSETASTHEMEDBTN, CSkin::IsThemed(), 0);
		SendMessage(hwndButton, BUTTONSETCONTAINER, (LPARAM)m_pContainer, 0);
		SendMessage(hwndButton, BUTTONSETASTOOLBARBUTTON, TRUE, 0);
	}
}

void CMsgDialog::BB_RedrawButtons()
{
	Srmm_RedrawToolbarIcons(m_hwnd);

	HWND hwndToggleSideBar = GetDlgItem(m_hwnd, IDC_TOGGLESIDEBAR);
	if (hwndToggleSideBar && IsWindow(hwndToggleSideBar))
		InvalidateRect(hwndToggleSideBar, nullptr, TRUE);
}

void CMsgDialog::BB_RefreshTheme()
{
	CustomButtonData *cbd;
	for (int i = 0; cbd = Srmm_GetNthButton(i); i++)
		SendDlgItemMessage(m_hwnd, cbd->m_dwButtonCID, WM_THEMECHANGED, 0, 0);
}

BOOL CMsgDialog::BB_SetButtonsPos()
{
	if (!m_hwnd || !IsWindowVisible(m_hwnd))
		return 0;

	uint8_t gap = DPISCALEX_S(Srmm_GetButtonGap());
	bool showToolbar = !m_pContainer->cfg.flags.m_bHideToolbar;
	bool bBottomToolbar = m_pContainer->cfg.flags.m_bBottomToolbar;

	HWND hwndToggleSideBar = GetDlgItem(m_hwnd, IDC_TOGGLESIDEBAR);
	ShowWindow(hwndToggleSideBar, (showToolbar && m_pContainer->m_pSideBar->isActive()) ? SW_SHOW : SW_HIDE);

	HDWP hdwp = BeginDeferWindowPos(Srmm_GetButtonCount() + 1);

	RECT rcSplitter;
	GetWindowRect(GetDlgItem(m_hwnd, IDC_SPLITTERY), &rcSplitter);
	POINT ptSplitter = { 0, rcSplitter.top };
	ScreenToClient(m_hwnd, &ptSplitter);

	RECT rect;
	GetClientRect(m_hwnd, &rect);

	int splitterY = (!bBottomToolbar) ? ptSplitter.y - DPISCALEY_S(2) : rect.bottom;
	int tempL = m_bbLSideWidth, tempR = m_bbRSideWidth;
	int lwidth = 0, rwidth = 0;
	int iOff = DPISCALEY_S(22);

	int foravatar = 0;
	if ((rect.bottom - ptSplitter.y - (rcSplitter.bottom - rcSplitter.top) /*- DPISCALEY(2)*/ - (bBottomToolbar ? DPISCALEY_S(24) : 0) < m_pic.cy - DPISCALEY_S(2)) && m_bShowAvatar && !PluginConfig.m_bAlwaysFullToolbarWidth)
		foravatar = m_pic.cx + gap;

	if (m_pContainer->cfg.flags.m_bSideBar && (m_pContainer->m_pSideBar->getFlags() & CSideBar::SIDEBARORIENTATION_LEFT)) {
		if (nullptr != hwndToggleSideBar) /* Wine fix. */
			hdwp = DeferWindowPos(hdwp, hwndToggleSideBar, nullptr, 4, 2 + splitterY - iOff, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		lwidth += 10;
		tempL -= 10;
	}

	CustomButtonData *cbd;
	for (int i = 0; cbd = Srmm_GetNthButton(i); i++) {
		if (cbd->m_bRSided || !BB_IsDisplayed(cbd)) // filter only left & displayed buttons
			continue;

		HWND hwndButton = GetDlgItem(m_hwnd, cbd->m_dwButtonCID);
		if (hwndButton == nullptr)
			continue;

		bool bAutoHidden = mapHidden[cbd];

		int iCmd = SW_SHOW;
		if (showToolbar) {
			if (!cbd->m_bSeparator && !IsWindowEnabled(hwndButton) && !bAutoHidden)
				tempL -= cbd->m_iButtonWidth + gap;

			if (cbd->m_bCanBeHidden && !cbd->m_bHidden) {
				if (!bAutoHidden) {
					if (tempL + tempR > (rect.right - foravatar) && (cbd->m_bSeparator || IsWindowVisible(hwndButton))) {
						iCmd = SW_HIDE;
						bAutoHidden = true;
						tempL -= cbd->m_iButtonWidth + gap;
					}
				}
				else bAutoHidden = false;
			}
		}
		else iCmd = SW_HIDE;
		ShowWindow(hwndButton, iCmd);
		
		hdwp = DeferWindowPos(hdwp, hwndButton, nullptr, lwidth, splitterY - iOff, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		if (IsWindowVisible(hwndButton) || (cbd->m_bSeparator && !(bAutoHidden || cbd->m_bHidden)))
			lwidth += cbd->m_iButtonWidth + gap;

		mapHidden[cbd] = bAutoHidden;
	}

	if (m_pContainer->cfg.flags.m_bSideBar && (m_pContainer->m_pSideBar->getFlags() & CSideBar::SIDEBARORIENTATION_RIGHT)) {
		if (nullptr != hwndToggleSideBar) /* Wine fix. */
			hdwp = DeferWindowPos(hdwp, hwndToggleSideBar, nullptr, rect.right - foravatar - 10, 2 + splitterY - iOff, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		rwidth += 12;
		tempR -= 12;
	}

	for (int i = 0; cbd = Srmm_GetNthButton(i); i++) {
		if (!cbd->m_bRSided || !BB_IsDisplayed(cbd)) // filter only right & displayed buttons
			continue;
		
		HWND hwndButton = GetDlgItem(m_hwnd, cbd->m_dwButtonCID);
		if (hwndButton == nullptr)
			continue;

		bool bAutoHidden = mapHidden[cbd];

		int iCmd = SW_SHOW;
		if (showToolbar) {
			if (!cbd->m_bSeparator && !IsWindowVisible(hwndButton) && !IsWindowEnabled(hwndButton) && !bAutoHidden)
				tempR -= cbd->m_iButtonWidth + gap;

			if (cbd->m_bCanBeHidden && !cbd->m_bHidden) {
				if (!bAutoHidden) {
					if (tempL + tempR > (rect.right - foravatar) && (cbd->m_bSeparator || IsWindowVisible(hwndButton))) {
						iCmd = SW_HIDE;
						bAutoHidden = true;
						tempR -= cbd->m_iButtonWidth + gap;
					}
				}
				else bAutoHidden = false;
			}
		}
		else iCmd = SW_HIDE;
		ShowWindow(hwndButton, iCmd);

		if (IsWindowVisible(hwndButton) || (cbd->m_bSeparator && !(bAutoHidden || cbd->m_bHidden)))
			rwidth += cbd->m_iButtonWidth + gap;
		hdwp = DeferWindowPos(hdwp, hwndButton, nullptr, rect.right - foravatar - rwidth + gap, splitterY - iOff, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

		mapHidden[cbd] = bAutoHidden;
	}

	return EndDeferWindowPos(hdwp);
}

void CMsgDialog::CB_DestroyAllButtons()
{
	CustomButtonData *cbd;
	for (int i = 0; cbd = Srmm_GetNthButton(i); i++) {
		HWND hwndButton = GetDlgItem(m_hwnd, cbd->m_dwButtonCID);
		if (hwndButton)
			DestroyWindow(hwndButton);
	}
}

void CMsgDialog::CB_DestroyButton(uint32_t dwButtonCID, uint32_t dwFlags)
{
	HWND hwndButton = GetDlgItem(m_hwnd, dwButtonCID);
	if (hwndButton == nullptr)
		return;

	RECT rc = { 0 };
	GetClientRect(hwndButton, &rc);
	if (dwFlags & BBBF_ISRSIDEBUTTON)
		m_bbRSideWidth -= rc.right;
	else 
		m_bbLSideWidth -= rc.right;

	DestroyWindow(hwndButton);
	BB_SetButtonsPos();
}

void CMsgDialog::CB_ChangeButton(CustomButtonData *cbd)
{
	HWND hwndButton = GetDlgItem(m_hwnd, cbd->m_dwButtonCID);
	if (hwndButton == nullptr)
		return;

	if (cbd->m_hIcon)
		SendMessage(hwndButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIconByHandle(cbd->m_hIcon));
	if (cbd->m_pwszTooltip)
		SendMessage(hwndButton, BUTTONADDTOOLTIP, (WPARAM)LPARAM(TranslateW_LP(cbd->m_pwszTooltip, cbd->m_pPlugin)), BATF_UNICODE);
	SendMessage(hwndButton, BUTTONSETCONTAINER, (LPARAM)m_pContainer, 0);
	SetWindowTextA(hwndButton, cbd->m_pszModuleName);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Module init procedure

void CB_InitCustomButtons()
{
	HookTemporaryEvent(ME_MSG_TOOLBARLOADED, CB_InitDefaultButtons);
}
