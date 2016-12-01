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
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISARROWBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_NAME;
	bbd.dwDefPos = 20;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[20];
	bbd.pwszTooltip = LPGENW("Info button");
	Srmm_AddButton(&bbd);

	if (PluginConfig.g_SmileyAddAvail) {
		bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_CREATEBYID;
		bbd.dwButtonID = IDC_SMILEYBTN;
		bbd.iButtonWidth = 0;
		bbd.dwDefPos = 30;
		bbd.hIcon = PluginConfig.g_buttonBarIconHandles[9];
		bbd.pwszTooltip = LPGENW("Insert emoticon");
		Srmm_AddButton(&bbd);
	}

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISPUSHBUTTON | BBBF_CANBEHIDDEN | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_FONTBOLD;
	bbd.dwDefPos = 40;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[10];
	bbd.pwszTooltip = LPGENW("Bold text");
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISPUSHBUTTON | BBBF_CANBEHIDDEN | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_FONTITALIC;
	bbd.dwDefPos = 50;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[11];
	bbd.pwszTooltip = LPGENW("Italic text");
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISPUSHBUTTON | BBBF_CANBEHIDDEN | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_FONTUNDERLINE;
	bbd.dwDefPos = 60;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[12];
	bbd.pwszTooltip = LPGENW("Underlined text");
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISPUSHBUTTON | BBBF_CANBEHIDDEN | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_FONTSTRIKEOUT;
	bbd.dwDefPos = 70;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[15];
	bbd.pwszTooltip = LPGENW("Strike-through text");
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_ISARROWBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDOK;
	bbd.dwDefPos = 10;
	bbd.iButtonWidth = 51;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[6];
	bbd.pwszTooltip = LPGENW("Send message\nClick dropdown arrow for sending options");
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_SAVE;
	bbd.dwDefPos = 20;
	bbd.iButtonWidth = 0;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[8];
	bbd.pwszTooltip = LPGENW("Close session");
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_QUOTE;
	bbd.dwDefPos = 30;
	bbd.iButtonWidth = 0;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[4];
	bbd.pwszTooltip = LPGENW("Quote last message OR selected text");
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_TIME;
	bbd.dwDefPos = 40;
	bbd.iButtonWidth = 0;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[2];
	bbd.pwszTooltip = LPGENW("Message log options");
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_HISTORY;
	bbd.dwDefPos = 50;
	bbd.iButtonWidth = 0;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[0];
	bbd.pwszTooltip = LPGENW("View user's history");
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_PIC;
	bbd.dwDefPos = 60;
	bbd.iButtonWidth = 0;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[7];
	bbd.pwszTooltip = LPGENW("Edit user notes");
	Srmm_AddButton(&bbd);

	// chat buttons
	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISDUMMYBUTTON;
	bbd.dwButtonID = 1;
	bbd.pszModuleName = "tb_splitter";
	bbd.dwDefPos = 31;
	bbd.iButtonWidth = 22;
	bbd.hIcon = 0;
	bbd.pwszTooltip = 0;
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISDUMMYBUTTON;
	bbd.dwButtonID = 2;
	bbd.dwDefPos = 22;
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISDUMMYBUTTON;
	bbd.dwButtonID = 3;
	bbd.dwDefPos = 71;
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISPUSHBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_COLOR;
	bbd.pszModuleName = "Tabsrmm";
	bbd.dwDefPos = 80;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[14];
	bbd.pwszTooltip = LPGENW("Select font color");
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISPUSHBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_BKGCOLOR;
	bbd.dwDefPos = 81;
	bbd.iButtonWidth = 22;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[16];
	bbd.pwszTooltip = LPGENW("Change background color");
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_SHOWNICKLIST;
	bbd.dwDefPos = 22;
	bbd.iButtonWidth = 22;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[19];
	bbd.pwszTooltip = LPGENW("Toggle nick list");
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_FILTER;
	bbd.dwDefPos = 24;
	bbd.iButtonWidth = 22;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[18];
	bbd.pwszTooltip = LPGENW("Event filter - right click to setup, left click to activate/deactivate");
	Srmm_AddButton(&bbd);

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_CHANMGR;
	bbd.dwDefPos = 33;
	bbd.iButtonWidth = 22;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[17];
	bbd.pwszTooltip = LPGENW("Channel manager");
	Srmm_AddButton(&bbd);
	
	return 0;
}

void BB_InitDlgButtons(TWindowData *dat)
{
	if (dat == 0)
		return;
	HWND hdlg = dat->hwnd;
	if (hdlg == 0)
		return;
	RECT rect;
	RECT rcSplitter;
	POINT ptSplitter;
	int splitterY;
	BYTE gap = DPISCALEX_S(db_get_b(NULL, SRMSGMOD, "ButtonsBarGap", 1));

	GetWindowRect(GetDlgItem(hdlg, (dat->bType == SESSIONTYPE_IM) ? IDC_SPLITTER : IDC_SPLITTERY), &rcSplitter);
	ptSplitter.x = 0;
	ptSplitter.y = rcSplitter.top;
	ScreenToClient(hdlg, &ptSplitter);

	GetClientRect(hdlg, &rect);
	splitterY = ptSplitter.y - DPISCALEY_S(1);

	HWND hwndBtn = NULL;
	dat->bbLSideWidth = dat->bbRSideWidth = 0;

	CustomButtonData *cbd;
	for (int i = 0; cbd = Srmm_GetNthButton(i); i++) {
		if (((dat->bType == SESSIONTYPE_IM && cbd->m_bIMButton) || (dat->bType == SESSIONTYPE_CHAT && cbd->m_bChatButton))) {
			if (!cbd->m_bHidden) {
				if (cbd->m_bRSided)
					dat->bbRSideWidth += cbd->m_iButtonWidth + gap;
				else
					dat->bbLSideWidth += cbd->m_iButtonWidth + gap;
			}
			if (!cbd->m_bHidden && !cbd->m_bCanBeHidden)
				dat->iButtonBarReallyNeeds += cbd->m_iButtonWidth + gap;
			if (!cbd->m_bSeparator && !GetDlgItem(hdlg, cbd->m_dwButtonCID)) {
				int x = cbd->m_bRSided ? rect.right - dat->bbRSideWidth + gap : 2 + dat->bbLSideWidth;
				hwndBtn = CreateWindowEx(0, L"MButtonClass", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP, x, splitterY, cbd->m_iButtonWidth, DPISCALEY_S(22), hdlg, (HMENU)cbd->m_dwButtonCID, g_hInst, NULL);
				CustomizeButton(hwndBtn);
			}
			if (!cbd->m_bSeparator && hwndBtn) {
				SendMessage(hwndBtn, BUTTONSETASFLATBTN, TRUE, 0);
				SendMessage(hwndBtn, BUTTONSETASTHEMEDBTN, CSkin::IsThemed(), 0);
				if (cbd->m_hIcon)
					SendMessage(hwndBtn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIconByHandle(cbd->m_hIcon));
				if (cbd->m_pwszTooltip)
					SendMessage(hwndBtn, BUTTONADDTOOLTIP, (WPARAM)TranslateW(cbd->m_pwszTooltip), BATF_UNICODE);
				SendMessage(hwndBtn, BUTTONSETCONTAINER, (LPARAM)dat->pContainer, 0);
				SendMessage(hwndBtn, BUTTONSETASTOOLBARBUTTON, TRUE, 0);

				if (hwndBtn) {
					if (cbd->m_dwArrowCID)
						SendMessage(hwndBtn, BUTTONSETARROW, cbd->m_dwArrowCID, 0);
					if (cbd->m_bPushButton)
						SendMessage(hwndBtn, BUTTONSETASPUSHBTN, TRUE, 0);
				}
			}
		}
		else if (GetDlgItem(hdlg, cbd->m_dwButtonCID))
			DestroyWindow(GetDlgItem(hdlg, cbd->m_dwButtonCID));

		if (cbd->m_bDisabled)
			EnableWindow(hwndBtn, 0);
		if (cbd->m_bHidden)
			ShowWindow(hwndBtn, SW_HIDE);

	}
}

void BB_RedrawButtons(TWindowData *dat)
{
	Srmm_RedrawToolbarIcons(dat->hwnd);

	HWND hwndToggleSideBar = GetDlgItem(dat->hwnd, IDC_TOGGLESIDEBAR);
	if (hwndToggleSideBar && IsWindow(hwndToggleSideBar))
		InvalidateRect(hwndToggleSideBar, 0, TRUE);
}

void BB_RefreshTheme(const TWindowData *dat)
{
	CustomButtonData *cbd;
	for (int i = 0; cbd = Srmm_GetNthButton(i); i++)
		SendDlgItemMessage(dat->hwnd, cbd->m_dwButtonCID, WM_THEMECHANGED, 0, 0);
}

BOOL BB_SetButtonsPos(TWindowData *dat)
{
	if (!dat || !IsWindowVisible(dat->hwnd))
		return 0;

	HWND hwnd = dat->hwnd;
	RECT rect;
	HWND hwndBtn = 0;

	BYTE gap = DPISCALEX_S(db_get_b(NULL, SRMSGMOD, "ButtonsBarGap", 1));
	bool showToolbar = !(dat->pContainer->dwFlags & CNT_HIDETOOLBAR);
	bool bBottomToolbar = (dat->pContainer->dwFlags & CNT_BOTTOMTOOLBAR) != 0;

	HWND hwndToggleSideBar = GetDlgItem(hwnd, IDC_TOGGLESIDEBAR);
	ShowWindow(hwndToggleSideBar, (showToolbar && dat->pContainer->SideBar->isActive()) ? SW_SHOW : SW_HIDE);

	HDWP hdwp = BeginDeferWindowPos(Srmm_GetButtonCount() + 1);

	RECT rcSplitter;
	GetWindowRect(GetDlgItem(hwnd, (dat->bType == SESSIONTYPE_IM) ? IDC_SPLITTER : IDC_SPLITTERY), &rcSplitter);

	POINT ptSplitter = { 0, rcSplitter.top };
	ScreenToClient(hwnd, &ptSplitter);

	GetClientRect(hwnd, &rect);

	int splitterY = (!bBottomToolbar) ? ptSplitter.y - DPISCALEY_S(1) : rect.bottom;
	int tempL = dat->bbLSideWidth, tempR = dat->bbRSideWidth;
	int lwidth = 0, rwidth = 0;
	int iOff = DPISCALEY_S((PluginConfig.m_DPIscaleY > 1.0) ? (dat->bType == SESSIONTYPE_IM ? 22 : 23) : 22);

	int foravatar = 0;
	if ((rect.bottom - ptSplitter.y - (rcSplitter.bottom - rcSplitter.top) /*- DPISCALEY(2)*/ - (bBottomToolbar ? DPISCALEY_S(24) : 0) < dat->pic.cy - DPISCALEY_S(2)) && dat->bShowAvatar && !PluginConfig.m_bAlwaysFullToolbarWidth)
		foravatar = dat->pic.cx + gap;

	if ((dat->pContainer->dwFlags & CNT_SIDEBAR) && (dat->pContainer->SideBar->getFlags() & CSideBar::SIDEBARORIENTATION_LEFT)) {
		if (NULL != hwndToggleSideBar) /* Wine fix. */
			hdwp = DeferWindowPos(hdwp, hwndToggleSideBar, NULL, 4, 2 + splitterY - iOff, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		lwidth += 10;
		tempL -= 10;
	}

	CustomButtonData *cbd;
	for (int i = 0; cbd = Srmm_GetNthButton(i); i++) {
		if (cbd->m_bRSided) // filter only left buttons
			continue;

		if (((dat->bType == SESSIONTYPE_IM) && cbd->m_bIMButton) || ((dat->bType == SESSIONTYPE_CHAT) && cbd->m_bChatButton)) {
			hwndBtn = GetDlgItem(hwnd, cbd->m_dwButtonCID);

			if (!showToolbar) {
				ShowWindow(hwndBtn, SW_HIDE);
				if (NULL != hwndBtn) /* Wine fix. */
					hdwp = DeferWindowPos(hdwp, hwndBtn, NULL, lwidth, splitterY - iOff, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				if (IsWindowVisible(hwndBtn) || (cbd->m_bSeparator && !(cbd->m_bAutoHidden || cbd->m_bHidden)))
					lwidth += cbd->m_iButtonWidth + gap;
				if (!IsWindowEnabled(hwndBtn) && !IsWindowVisible(hwndBtn) && !cbd->m_bAutoHidden)
					cbd->m_bAutoHidden = 1;
				continue;
			}
			if (!cbd->m_bCanBeHidden && !cbd->m_bHidden && !(!IsWindowEnabled(hwndBtn) && !IsWindowVisible(hwndBtn) && !cbd->m_bAutoHidden)) {
				ShowWindow(hwndBtn, SW_SHOW);
				cbd->m_bAutoHidden = 0;
			}

			if (!cbd->m_bSeparator && !IsWindowVisible(hwndBtn) && !IsWindowEnabled(hwndBtn) && !cbd->m_bAutoHidden)
				tempL -= cbd->m_iButtonWidth + gap;

			if (cbd->m_bCanBeHidden && !cbd->m_bHidden && (cbd->m_bSeparator || !((!IsWindowEnabled(hwndBtn) && !IsWindowVisible(hwndBtn)) && !cbd->m_bAutoHidden))) {
				if (tempL + tempR > (rect.right - foravatar)) {
					ShowWindow(hwndBtn, SW_HIDE);
					cbd->m_bAutoHidden = 1;
					tempL -= cbd->m_iButtonWidth + gap;
				}
				else if (cbd->m_bAutoHidden) {
					ShowWindow(hwndBtn, SW_SHOW);
					cbd->m_bAutoHidden = 0;
				}
			}
			if (NULL != hwndBtn) /* Wine fix. */
				hdwp = DeferWindowPos(hdwp, hwndBtn, NULL, lwidth, splitterY - iOff, 0, 0, SWP_NOZORDER | SWP_NOSIZE);// SWP_NOCOPYBITS);
			if (IsWindowVisible(hwndBtn) || (cbd->m_bSeparator && !(cbd->m_bAutoHidden || cbd->m_bHidden)))
				lwidth += cbd->m_iButtonWidth + gap;
		}
	}

	if ((dat->pContainer->dwFlags & CNT_SIDEBAR) && (dat->pContainer->SideBar->getFlags() & CSideBar::SIDEBARORIENTATION_RIGHT)) {
		if (NULL != hwndToggleSideBar) /* Wine fix. */
			hdwp = DeferWindowPos(hdwp, hwndToggleSideBar, NULL, rect.right - foravatar - 10, 2 + splitterY - iOff, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		rwidth += 12;
		tempR -= 12;
	}

	for (int i = 0; cbd = Srmm_GetNthButton(i); i++) {
		if (!cbd->m_bRSided) // filter only right buttons
			continue;

		if (((dat->bType == SESSIONTYPE_IM) && cbd->m_bIMButton) || ((dat->bType == SESSIONTYPE_CHAT) && cbd->m_bChatButton)) {
			hwndBtn = GetDlgItem(hwnd, cbd->m_dwButtonCID);

			if (!showToolbar) {
				ShowWindow(hwndBtn, SW_HIDE);
				if (IsWindowVisible(hwndBtn) || (cbd->m_bSeparator && !(cbd->m_bAutoHidden || cbd->m_bHidden)))
					rwidth += cbd->m_iButtonWidth + gap;
				if (NULL != hwndBtn) /* Wine fix. */
					hdwp = DeferWindowPos(hdwp, hwndBtn, NULL, rect.right - foravatar - rwidth + gap, splitterY - iOff, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				if (!IsWindowEnabled(hwndBtn) && !IsWindowVisible(hwndBtn) && !cbd->m_bAutoHidden)
					cbd->m_bAutoHidden = 1;
				continue;
			}
			if (!cbd->m_bCanBeHidden && !cbd->m_bHidden && !((!IsWindowEnabled(hwndBtn) && !IsWindowVisible(hwndBtn)) && !cbd->m_bAutoHidden)) {
				ShowWindow(hwndBtn, SW_SHOW);
				cbd->m_bAutoHidden = 0;
			}

			if (!cbd->m_bSeparator && !IsWindowVisible(hwndBtn) && !IsWindowEnabled(hwndBtn) && !cbd->m_bAutoHidden)
				tempR -= cbd->m_iButtonWidth + gap;

			if (cbd->m_bCanBeHidden && !cbd->m_bHidden && (cbd->m_bSeparator || !((!IsWindowEnabled(hwndBtn) && !IsWindowVisible(hwndBtn)) && !cbd->m_bAutoHidden))) {
				if (tempL + tempR > (rect.right - foravatar)) {
					ShowWindow(hwndBtn, SW_HIDE);
					cbd->m_bAutoHidden = 1;
					tempR -= cbd->m_iButtonWidth + gap;
				}
				else if (cbd->m_bAutoHidden) {
					ShowWindow(hwndBtn, SW_SHOW);
					cbd->m_bAutoHidden = 0;
				}
			}

			if (IsWindowVisible(hwndBtn) || (cbd->m_bSeparator && !(cbd->m_bAutoHidden || cbd->m_bHidden)))
				rwidth += cbd->m_iButtonWidth + gap;
			if (NULL != hwndBtn) /* Wine fix. */
				hdwp = DeferWindowPos(hdwp, hwndBtn, NULL, rect.right - foravatar - rwidth + gap, splitterY - iOff, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}
	}
	return EndDeferWindowPos(hdwp);
}

void CB_DestroyAllButtons(HWND hwndDlg)
{
	CustomButtonData *cbd;
	for (int i = 0; cbd = Srmm_GetNthButton(i); i++) {
		HWND hwndBtn = GetDlgItem(hwndDlg, cbd->m_dwButtonCID);
		if (hwndBtn)
			DestroyWindow(hwndBtn);
	}
}

void CB_DestroyButton(HWND hwndDlg, TWindowData *dat, DWORD dwButtonCID, DWORD dwFlags)
{
	HWND hwndBtn = GetDlgItem(hwndDlg, dwButtonCID);
	if (hwndBtn == NULL)
		return;

	RECT rc = { 0 };
	GetClientRect(hwndBtn, &rc);
	if (dwFlags & BBBF_ISRSIDEBUTTON)
		dat->bbRSideWidth -= rc.right;
	else 
		dat->bbLSideWidth -= rc.right;

	DestroyWindow(hwndBtn);
	BB_SetButtonsPos(dat);
}

void CB_ChangeButton(HWND hwndDlg, TWindowData *dat, CustomButtonData *cbd)
{
	HWND hwndBtn = GetDlgItem(hwndDlg, cbd->m_dwButtonCID);
	if (hwndBtn == NULL)
		return;

	if (cbd->m_hIcon)
		SendMessage(hwndBtn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIconByHandle(cbd->m_hIcon));
	if (cbd->m_pwszTooltip)
		SendMessage(hwndBtn, BUTTONADDTOOLTIP, (WPARAM)cbd->m_pwszTooltip, BATF_UNICODE);
	SendMessage(hwndBtn, BUTTONSETCONTAINER, (LPARAM)dat->pContainer, 0);
	SetWindowTextA(hwndBtn, cbd->m_pszModuleName);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Module init procedure

void CB_InitCustomButtons()
{
	HookEvent(ME_MSG_TOOLBARLOADED, CB_InitDefaultButtons);
}
