#include "commonheaders.h"

static HANDLE hButtonsBarAddButton;
static HANDLE hButtonsBarRemoveButton;
static HANDLE hButtonsBarGetButtonState;
static HANDLE hButtonsBarSetButtonState;
static HANDLE hButtonsBarModifyButton;

HANDLE hHookButtonPressedEvt;
HANDLE hHookToolBarLoadedEvt;

static LIST<CustomButtonData> RButtonsList(1, NumericKeySortT), LButtonsList(1, NumericKeySortT);

DWORD LastCID = 4000;
DWORD dwSepCount = 0;

static mir_cs ToolBarCS;

static void wipeList(LIST<CustomButtonData> &list)
{
	for (int i = list.getCount()-1; i >= 0; i--) {
		delete list[i];
		list.remove(i);
	}
}

static int sstSortButtons(const void *p1, const void *p2)
{
	return (*(CustomButtonData**)p1)->dwPosition - (*(CustomButtonData**)p2)->dwPosition;
}

void CB_InitCustomButtons()
{
	dwSepCount = M.GetDword("TabSRMM_Toolbar", "SeparatorsCount", 0);

	hButtonsBarAddButton = CreateServiceFunction(MS_BB_ADDBUTTON, CB_AddButton);
	hButtonsBarRemoveButton = CreateServiceFunction(MS_BB_REMOVEBUTTON, CB_RemoveButton);
	hButtonsBarModifyButton = CreateServiceFunction(MS_BB_MODIFYBUTTON, CB_ModifyButton);
	hButtonsBarGetButtonState = CreateServiceFunction(MS_BB_GETBUTTONSTATE, CB_GetButtonState);
	hButtonsBarSetButtonState = CreateServiceFunction(MS_BB_SETBUTTONSTATE, CB_SetButtonState);

	hHookToolBarLoadedEvt = CreateHookableEvent(ME_MSG_TOOLBARLOADED);
	hHookButtonPressedEvt = CreateHookableEvent(ME_MSG_BUTTONPRESSED);
}

void CB_DeInitCustomButtons()
{
	wipeList(LButtonsList);
	wipeList(RButtonsList);

	DestroyHookableEvent(hHookToolBarLoadedEvt);
	DestroyHookableEvent(hHookButtonPressedEvt);
	DestroyServiceFunction(hButtonsBarAddButton);
	DestroyServiceFunction(hButtonsBarRemoveButton);
	DestroyServiceFunction(hButtonsBarModifyButton);
	DestroyServiceFunction(hButtonsBarGetButtonState);
	DestroyServiceFunction(hButtonsBarSetButtonState);
}

void CB_DestroyAllButtons(HWND hwndDlg)
{
	HWND hwndBtn = NULL;
	for (int i=0; i < LButtonsList.getCount(); i++) {
		CustomButtonData *cbd = LButtonsList[i];
		hwndBtn = GetDlgItem(hwndDlg, cbd->dwButtonCID);
		if (hwndBtn)
			DestroyWindow(hwndBtn);
	}

	for (int i=0; i < RButtonsList.getCount(); i++) {
		CustomButtonData *cbd = RButtonsList[i];
		hwndBtn = GetDlgItem(hwndDlg, cbd->dwButtonCID);
		if (hwndBtn)
			DestroyWindow(hwndBtn);
	}
}

void CB_DestroyButton(HWND hwndDlg, TWindowData *dat, DWORD dwButtonCID, DWORD dwFlags)
{
	HWND hwndBtn = GetDlgItem(hwndDlg, dwButtonCID);
	RECT rc = { 0 };
	if (hwndBtn) {
		GetClientRect(hwndBtn, &rc);
		if (dwFlags & BBBF_ISLSIDEBUTTON)
			dat->bbLSideWidth -= rc.right;
		else if (dwFlags & BBBF_ISRSIDEBUTTON)
			dat->bbRSideWidth -= rc.right;

		DestroyWindow(hwndBtn);
		BB_SetButtonsPos(dat);
	}
}

void CB_ChangeButton(HWND hwndDlg, TWindowData *dat, CustomButtonData *cbd)
{
	HWND hwndBtn = GetDlgItem(hwndDlg, cbd->dwButtonCID);
	if (hwndBtn) {
		if (cbd->hIcon)
			SendMessage(hwndBtn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_GetIconByHandle(cbd->hIcon));
		if (cbd->ptszTooltip)
			SendMessage(hwndBtn, BUTTONADDTOOLTIP, (WPARAM)cbd->ptszTooltip, BATF_TCHAR);
		SendMessage(hwndBtn, BUTTONSETCONTAINER, (LPARAM)dat->pContainer, 0);
		SetWindowTextA(hwndBtn, cbd->pszModuleName);
	}
}

void CB_ReInitCustomButtons()
{
	for (int i = 0; i < LButtonsList.getCount(); i++) {
		CustomButtonData *cbd = LButtonsList[i];

		if (cbd->opFlags & BBSF_NTBSWAPED || cbd->opFlags & BBSF_NTBDESTRUCT) {
			cbd->opFlags ^= BBSF_NTBSWAPED;

			if (!(cbd->opFlags & BBSF_NTBDESTRUCT))
				RButtonsList.insert(cbd);

			LButtonsList.remove(i);
			i--;
		}
	}

	for (int i = 0; i < RButtonsList.getCount(); i++) {
		CustomButtonData* cbd = RButtonsList[i];
		if (cbd->opFlags & BBSF_NTBSWAPED || cbd->opFlags & BBSF_NTBDESTRUCT) {
			cbd->opFlags ^= BBSF_NTBSWAPED;

			if (!(cbd->opFlags & BBSF_NTBDESTRUCT))
				LButtonsList.insert(cbd);

			RButtonsList.remove(i);
			i--;
		}
	}
	M.BroadcastMessage(DM_BBNEEDUPDATE, 0, 0);
	M.BroadcastMessage(DM_LOADBUTTONBARICONS, 0, 0);
}

void CB_HardReInit()
{
	M.BroadcastMessage(DM_CBDESTROY, 0, 0);
	{
		mir_cslock lck(ToolBarCS);
		wipeList(LButtonsList);
		wipeList(RButtonsList);
	}
	LastCID = 4000;
	dwSepCount = 0;

	CB_InitDefaultButtons();
	NotifyEventHooks(hHookToolBarLoadedEvt, 0, 0);
}

static INT_PTR CB_AddButton(WPARAM, LPARAM lParam)
{
	BBButton *bbdi = (BBButton *)lParam;
	if (!bbdi || bbdi->cbSize != sizeof(BBButton))
		return 1;

	CustomButtonData *cbd = new CustomButtonData();

	if (!bbdi->iButtonWidth && (bbdi->bbbFlags & BBBF_ISARROWBUTTON))
		cbd->iButtonWidth = DPISCALEX_S(34);
	else if (!bbdi->iButtonWidth)
		cbd->iButtonWidth = DPISCALEX_S(22);
	else
		cbd->iButtonWidth = DPISCALEX_S(bbdi->iButtonWidth);

	cbd->pszModuleName = mir_strdup(bbdi->pszModuleName);

	if (bbdi->ptszTooltip) {
		if (bbdi->bbbFlags & BBBF_ANSITOOLTIP)
			cbd->ptszTooltip = mir_a2u(bbdi->pszTooltip);
		else
			cbd->ptszTooltip = mir_tstrdup(bbdi->ptszTooltip);
	}
	else cbd->ptszTooltip = NULL;

	cbd->dwButtonOrigID = bbdi->dwButtonID;
	cbd->hIcon = bbdi->hIcon;
	cbd->dwPosition = bbdi->dwDefPos;
	cbd->dwButtonCID = (bbdi->bbbFlags & BBBF_CREATEBYID) ? bbdi->dwButtonID : LastCID;
	//ugly workaround for smileys plugins
	cbd->dwArrowCID = (bbdi->bbbFlags & BBBF_ISARROWBUTTON) ? (cbd->dwButtonCID == IDOK ? IDC_SENDMENU : (cbd->dwButtonCID + 1)) : 0;
	cbd->bHidden = (bbdi->bbbFlags & BBBF_HIDDEN) != 0;
	cbd->bLSided = (bbdi->bbbFlags & BBBF_ISLSIDEBUTTON) != 0;
	cbd->bRSided = (bbdi->bbbFlags & BBBF_ISRSIDEBUTTON) != 0;
	cbd->bCanBeHidden = (bbdi->bbbFlags & BBBF_CANBEHIDDEN) != 0;
	cbd->bSeparator = (bbdi->bbbFlags & BBBF_ISDUMMYBUTTON) != 0;
	cbd->bChatButton = (bbdi->bbbFlags & BBBF_ISCHATBUTTON) != 0;
	cbd->bIMButton = (bbdi->bbbFlags & BBBF_ISIMBUTTON) != 0;
	cbd->bDisabled = (bbdi->bbbFlags & BBBF_DISABLED) != 0;
	cbd->bPushButton = (bbdi->bbbFlags & BBBF_ISPUSHBUTTON) != 0;

	CB_GetButtonSettings(NULL, cbd);

	if (cbd->bLSided)
		LButtonsList.insert(cbd);
	else if (cbd->bRSided)
		RButtonsList.insert(cbd);
	else {
		delete cbd;
		return 1;
	}

	if (cbd->dwButtonCID != cbd->dwButtonOrigID)
		LastCID++;
	if (cbd->dwArrowCID == LastCID)
		LastCID++;

	M.BroadcastMessage(DM_BBNEEDUPDATE, 0, 0);
	return 0;
}

static INT_PTR CB_GetButtonState(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0 || lParam == 0)
		return 1;

	DWORD tempCID = 0;
	bool realbutton = false;
	BBButton *bbdi = (BBButton *)lParam;
	bbdi->bbbFlags = 0;
	for (int i = 0; i < LButtonsList.getCount(); i++) {
		CustomButtonData *cbd = LButtonsList[i];
		if (!strcmp(cbd->pszModuleName, bbdi->pszModuleName) && (cbd->dwButtonOrigID == bbdi->dwButtonID)) {
			realbutton = true;
			tempCID = cbd->dwButtonCID;
		}
	}
	if (!realbutton)
		for (int i = 0; i < RButtonsList.getCount(); i++) {
			CustomButtonData* cbd = RButtonsList[i];
			if (!strcmp(cbd->pszModuleName, bbdi->pszModuleName) && (cbd->dwButtonOrigID == bbdi->dwButtonID)) {
				realbutton = true;
				tempCID = cbd->dwButtonCID;
			}
		}

	if (!realbutton)
		return 1;

	HWND hwndDlg = M.FindWindow(wParam);
	if (!hwndDlg)
		return 1;

	HWND hwndBtn = GetDlgItem(hwndDlg, tempCID);
	bbdi->bbbFlags = (IsDlgButtonChecked(hwndDlg, tempCID) ? BBSF_PUSHED : BBSF_RELEASED) | (IsWindowVisible(hwndBtn) ? 0 : BBSF_HIDDEN) | (IsWindowEnabled(hwndBtn) ? 0 : BBSF_DISABLED);
	return 0;
}

static INT_PTR CB_SetButtonState(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0 || lParam == 0)
		return 1;

	bool realbutton = false;
	DWORD tempCID = 0;
	BBButton *bbdi = (BBButton *)lParam;
	for (int i = 0; i < LButtonsList.getCount(); i++) {
		CustomButtonData *cbd = LButtonsList[i];
		if (!strcmp(cbd->pszModuleName, bbdi->pszModuleName) && (cbd->dwButtonOrigID == bbdi->dwButtonID)) {
			realbutton = true;
			tempCID = cbd->dwButtonCID;
		}
	}
	if (!realbutton)
		for (int i = 0; i < RButtonsList.getCount(); i++) {
			CustomButtonData* cbd = RButtonsList[i];
			if (!strcmp(cbd->pszModuleName, bbdi->pszModuleName) && (cbd->dwButtonOrigID == bbdi->dwButtonID)) {
				realbutton = true;
				tempCID = cbd->dwButtonCID;
			}
		}

	if (!realbutton)
		return 1;

	HWND hwndDlg = M.FindWindow(wParam);
	if (!hwndDlg)
		return 1;

	SetDlgItemTextA(hwndDlg, tempCID, bbdi->pszModuleName);
	if (bbdi->hIcon)
		SendDlgItemMessage(hwndDlg, tempCID, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_GetIconByHandle(bbdi->hIcon));
	if (bbdi->pszTooltip)
		SendDlgItemMessage(hwndDlg, tempCID, BUTTONADDTOOLTIP, (WPARAM)bbdi->ptszTooltip, (bbdi->bbbFlags & BBBF_ANSITOOLTIP) ? 0 : BATF_TCHAR);
	if (bbdi->bbbFlags) {
		Utils::showDlgControl(hwndDlg, tempCID, (bbdi->bbbFlags&BBSF_HIDDEN) ? SW_HIDE : SW_SHOW);
		Utils::enableDlgControl(hwndDlg, tempCID, (bbdi->bbbFlags&BBSF_DISABLED) ? 0 : 1);
		CheckDlgButton(hwndDlg, tempCID, (bbdi->bbbFlags&BBSF_PUSHED)  ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, tempCID, (bbdi->bbbFlags&BBSF_RELEASED) ? BST_UNCHECKED : BST_CHECKED);
	}
	return 0;
}

static INT_PTR CB_RemoveButton(WPARAM, LPARAM lParam)
{
	BBButton *bbdi = (BBButton *)lParam;
	if (!bbdi)
		return 1;

	DWORD tempCID = 0;
	DWORD dwFlags = 0;
	{
		mir_cslock lck(ToolBarCS);

		for (int i = LButtonsList.getCount()-1; i >= 0; i--) {
			CustomButtonData *cbd = LButtonsList[i];
			if (!strcmp(cbd->pszModuleName, bbdi->pszModuleName) && cbd->dwButtonOrigID == bbdi->dwButtonID) {
				tempCID = cbd->dwButtonCID;
				dwFlags = cbd->bLSided ? BBBF_ISLSIDEBUTTON : BBBF_ISRSIDEBUTTON;
				LButtonsList.remove(i);
			}
		}

		if (!tempCID) {
			for (int i = RButtonsList.getCount()-1; i >= 0; i--) {
				CustomButtonData *cbd = RButtonsList[i];
				if (!strcmp(cbd->pszModuleName, bbdi->pszModuleName) && cbd->dwButtonOrigID == bbdi->dwButtonID) {
					tempCID = cbd->dwButtonCID;
					dwFlags = cbd->bLSided ? BBBF_ISLSIDEBUTTON : BBBF_ISRSIDEBUTTON;
					RButtonsList.remove(i);
				}
			}
		}
	}

	if (tempCID)
		M.BroadcastMessage(DM_CBDESTROY, (WPARAM)tempCID, (LPARAM)dwFlags);
	return 0;
}

static INT_PTR CB_ModifyButton(WPARAM, LPARAM lParam)
{
	BBButton *bbdi = (BBButton *)lParam;
	if (!bbdi)
		return 1;

	bool bFound = 1;
	CustomButtonData *cbd = NULL;
	{
		mir_cslock lck(ToolBarCS);

		for (int i = 0; i < LButtonsList.getCount(); i++) {
			cbd = LButtonsList[i];
			if (!strcmp(cbd->pszModuleName, bbdi->pszModuleName) && (cbd->dwButtonOrigID == bbdi->dwButtonID)) {
				bFound = true;
				break;
			}
		}

		if (!bFound) {
			for (int i = 0; i < RButtonsList.getCount(); i++) {
				cbd = RButtonsList[i];
				if (!strcmp(cbd->pszModuleName, bbdi->pszModuleName) && (cbd->dwButtonOrigID == bbdi->dwButtonID)) {
					bFound = true;
					break;
				}
			}
		}

		if (bFound) {
			if (bbdi->pszTooltip) {
				mir_free(cbd->ptszTooltip);
				if (bbdi->bbbFlags & BBBF_ANSITOOLTIP)
					cbd->ptszTooltip = mir_a2u(bbdi->pszTooltip);
				else
					cbd->ptszTooltip = mir_tstrdup(bbdi->ptszTooltip);
			}
			if (bbdi->hIcon)
				cbd->hIcon = bbdi->hIcon;
			if (bbdi->bbbFlags) {
				cbd->bHidden = (bbdi->bbbFlags & BBBF_HIDDEN) != 0;
				cbd->bLSided = (bbdi->bbbFlags & BBBF_ISLSIDEBUTTON) != 0;
				cbd->bRSided = (bbdi->bbbFlags & BBBF_ISRSIDEBUTTON) != 0;
				cbd->bCanBeHidden = (bbdi->bbbFlags & BBBF_CANBEHIDDEN) != 0;
				cbd->bChatButton = (bbdi->bbbFlags & BBBF_ISCHATBUTTON) != 0;
				cbd->bIMButton = (bbdi->bbbFlags & BBBF_ISIMBUTTON) != 0;
				cbd->bDisabled = (bbdi->bbbFlags & BBBF_DISABLED) != 0;
			}
		}
	}

	if (bFound)
		M.BroadcastMessage(DM_BBNEEDUPDATE, 0, (LPARAM)cbd);
	return 0;
}

void BB_UpdateIcons(HWND hdlg)
{
	HWND hwndBtn = NULL;

	for (int i = 0; i < LButtonsList.getCount(); i++) {
		CustomButtonData *cbd = LButtonsList[i];
		if (cbd) {
			if (!cbd->bSeparator)
				hwndBtn = GetDlgItem(hdlg, cbd->dwButtonCID);

			if (hwndBtn && cbd->hIcon)
				SendMessage(hwndBtn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_GetIconByHandle(cbd->hIcon));
		}
	}

	hwndBtn = NULL;
	for (int i = 0; i < RButtonsList.getCount(); i++) {
		CustomButtonData *cbd = RButtonsList[i];
		if (cbd) {
			if (!cbd->bSeparator)
				hwndBtn = GetDlgItem(hdlg, cbd->dwButtonCID);

			if (hwndBtn && cbd->hIcon)
				SendMessage(hwndBtn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_GetIconByHandle(cbd->hIcon));
		}
	}
}

void TSAPI BB_InitDlgButtons(TWindowData *dat)
{
	if (dat == 0)
		return;
	HWND hdlg = dat->hwnd;
	if (hdlg == 0)
		return;
	RECT rect;
	int lwidth = 0, rwidth = 0;
	RECT rcSplitter;
	POINT ptSplitter;
	int splitterY;
	BYTE gap = DPISCALEX_S(PluginConfig.g_iButtonsBarGap);
	BOOL isThemed = TRUE;

	if (CSkin::m_skinEnabled && !SkinItems[ID_EXTBKBUTTONSNPRESSED].IGNORED &&
		 !SkinItems[ID_EXTBKBUTTONSPRESSED].IGNORED && !SkinItems[ID_EXTBKBUTTONSMOUSEOVER].IGNORED) {
		isThemed = FALSE;
	}

	GetWindowRect(GetDlgItem(hdlg, (dat->bType == SESSIONTYPE_IM) ? IDC_SPLITTER : IDC_SPLITTERY), &rcSplitter);
	ptSplitter.x = 0;
	ptSplitter.y = rcSplitter.top;
	ScreenToClient(hdlg, &ptSplitter);

	GetClientRect(hdlg, &rect);
	splitterY = ptSplitter.y - DPISCALEY_S(1);

	HWND hwndBtn = NULL;

	for (int i = 0; i < RButtonsList.getCount(); i++) {
		CustomButtonData *cbd = RButtonsList[i];
		if (((dat->bType == SESSIONTYPE_IM && cbd->bIMButton) || (dat->bType == SESSIONTYPE_CHAT && cbd->bChatButton))) {
			if (!cbd->bHidden)
				rwidth += cbd->iButtonWidth + gap;
			if (!cbd->bHidden && !cbd->bCanBeHidden)
				dat->iButtonBarReallyNeeds += cbd->iButtonWidth + gap;
			if (!cbd->bSeparator && !GetDlgItem(hdlg, cbd->dwButtonCID)) {
				hwndBtn = CreateWindowEx(0, _T("MButtonClass"), _T(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP, rect.right - rwidth + gap, splitterY, cbd->iButtonWidth, DPISCALEY_S(22), hdlg, (HMENU)cbd->dwButtonCID, g_hInst, NULL);
				CustomizeButton(hwndBtn);
			}
			if (!cbd->bSeparator && hwndBtn) {
				SendMessage(hwndBtn, BUTTONSETASFLATBTN, TRUE, 0);
				SendMessage(hwndBtn, BUTTONSETASTHEMEDBTN, isThemed != 0, 0);
				if (cbd->hIcon)
					SendMessage(hwndBtn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_GetIconByHandle(cbd->hIcon));
				if (cbd->ptszTooltip)
					SendMessage(hwndBtn, BUTTONADDTOOLTIP, (WPARAM)TranslateTS(cbd->ptszTooltip), BATF_TCHAR);
				SendMessage(hwndBtn, BUTTONSETCONTAINER, (LPARAM)dat->pContainer, 0);
				SendMessage(hwndBtn, BUTTONSETASTOOLBARBUTTON, TRUE, 0);

				if (hwndBtn && cbd->dwArrowCID)
					SendMessage(hwndBtn, BUTTONSETARROW, cbd->dwArrowCID, 0);
				if (hwndBtn && cbd->bPushButton)
					SendMessage(hwndBtn, BUTTONSETASPUSHBTN, TRUE, 0);
			}
		}
		else if (GetDlgItem(hdlg, cbd->dwButtonCID))
			DestroyWindow(GetDlgItem(hdlg, cbd->dwButtonCID));

		if (cbd->bDisabled)
			EnableWindow(hwndBtn, 0);
		if (cbd->bHidden)
			ShowWindow(hwndBtn, SW_HIDE);

	}

	hwndBtn = NULL;

	for (int i = 0; i < LButtonsList.getCount(); i++) {
		CustomButtonData *cbd = LButtonsList[i];
		if (((dat->bType == SESSIONTYPE_IM && cbd->bIMButton) || (dat->bType == SESSIONTYPE_CHAT && cbd->bChatButton))) {
			if (!cbd->bSeparator && !GetDlgItem(hdlg, cbd->dwButtonCID)) {
				hwndBtn = CreateWindowEx(0, _T("MButtonClass"), _T(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 2 + lwidth, splitterY,
												 cbd->iButtonWidth, DPISCALEY_S(22), hdlg, (HMENU)cbd->dwButtonCID, g_hInst, NULL);
				CustomizeButton(hwndBtn);
			}
			if (!cbd->bHidden)
				lwidth += cbd->iButtonWidth + gap;
			if (!cbd->bHidden && !cbd->bCanBeHidden)
				dat->iButtonBarReallyNeeds += cbd->iButtonWidth + gap;
			if (!cbd->bSeparator && hwndBtn) {
				SendMessage(hwndBtn, BUTTONSETASFLATBTN, TRUE, 0);
				SendMessage(hwndBtn, BUTTONSETASTHEMEDBTN, isThemed != 0, 0);
				if (cbd->hIcon)
					SendMessage(hwndBtn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_GetIconByHandle(cbd->hIcon));
				if (cbd->ptszTooltip)
					SendMessage(hwndBtn, BUTTONADDTOOLTIP, (WPARAM)TranslateTS(cbd->ptszTooltip), BATF_TCHAR);
				SendMessage(hwndBtn, BUTTONSETCONTAINER, (LPARAM)dat->pContainer, 0);
				SendMessage(hwndBtn, BUTTONSETASTOOLBARBUTTON, TRUE, 0);

				if (hwndBtn && cbd->dwArrowCID)
					SendMessage(hwndBtn, BUTTONSETARROW, cbd->dwArrowCID, 0);
				if (hwndBtn && cbd->bPushButton)
					SendMessage(hwndBtn, BUTTONSETASPUSHBTN, TRUE, 0);
			}
		}
		else if (GetDlgItem(hdlg, cbd->dwButtonCID))
			DestroyWindow(GetDlgItem(hdlg, cbd->dwButtonCID));

		if (cbd->bDisabled)
			EnableWindow(hwndBtn, 0);
		if (cbd->bHidden)
			ShowWindow(hwndBtn, SW_HIDE);
	}

	dat->bbLSideWidth = lwidth;
	dat->bbRSideWidth = rwidth;
}

void TSAPI BB_RedrawButtons(TWindowData *dat)
{
	for (int i = 0; i < LButtonsList.getCount(); i++) {
		CustomButtonData *cbd = LButtonsList[i];
		HWND hwnd = GetDlgItem(dat->hwnd, cbd->dwButtonCID);
		if (hwnd)
			InvalidateRect(hwnd, 0, TRUE);
	}

	for (int i = 0; i < RButtonsList.getCount(); i++) {
		CustomButtonData *cbd = RButtonsList[i];
		HWND hwnd = GetDlgItem(dat->hwnd, cbd->dwButtonCID);
		if (hwnd)
			InvalidateRect(hwnd, 0, TRUE);
	}

	HWND hwndToggleSideBar = GetDlgItem(dat->hwnd, dat->bType == SESSIONTYPE_IM ? IDC_TOGGLESIDEBAR : IDC_CHAT_TOGGLESIDEBAR);
	if (hwndToggleSideBar && IsWindow(hwndToggleSideBar))
		InvalidateRect(hwndToggleSideBar, 0, TRUE);
}

BOOL TSAPI BB_SetButtonsPos(TWindowData *dat)
{

	if (!dat || !IsWindowVisible(dat->hwnd))
		return 0;

	HWND hwnd = dat->hwnd;
	RECT rect;
	int  i;
	HWND hwndBtn = 0;

	BYTE gap = DPISCALEX_S(PluginConfig.g_iButtonsBarGap);
	bool showToolbar = !(dat->pContainer->dwFlags & CNT_HIDETOOLBAR);
	bool bBottomToolbar = (dat->pContainer->dwFlags & CNT_BOTTOMTOOLBAR) != 0;

	HDWP hdwp = BeginDeferWindowPos(LButtonsList.getCount() + RButtonsList.getCount() + 1);

	HWND hwndToggleSideBar = GetDlgItem(hwnd, dat->bType == SESSIONTYPE_IM ? IDC_TOGGLESIDEBAR : IDC_CHAT_TOGGLESIDEBAR);
	ShowWindow(hwndToggleSideBar, (showToolbar && dat->pContainer->SideBar->isActive()) ? SW_SHOW : SW_HIDE);

	mir_cslock lck(ToolBarCS);

	RECT rcSplitter;
	GetWindowRect(GetDlgItem(hwnd, (dat->bType == SESSIONTYPE_IM) ? IDC_SPLITTER : IDC_SPLITTERY), &rcSplitter);

	POINT ptSplitter = { 0, rcSplitter.top };
	ScreenToClient(hwnd, &ptSplitter);

	GetClientRect(hwnd, &rect);

	int splitterY = (!bBottomToolbar) ? ptSplitter.y - DPISCALEY_S(1) : rect.bottom;
	int tempL = dat->bbLSideWidth, tempR = dat->bbRSideWidth;
	int lwidth = 0, rwidth = 0;
	int iOff = DPISCALEY_S((PluginConfig.g_DPIscaleY > 1.0) ? (dat->bType == SESSIONTYPE_IM ? 22 : 23) : 22);

	int foravatar = 0;
	if ((rect.bottom - ptSplitter.y - (rcSplitter.bottom - rcSplitter.top) /*- DPISCALEY(2)*/ - (bBottomToolbar ? DPISCALEY_S(24) : 0) < dat->pic.cy - DPISCALEY_S(2)) && dat->bShowAvatar && !PluginConfig.m_bAlwaysFullToolbarWidth)
		foravatar = dat->pic.cx + gap;

	if ((dat->pContainer->dwFlags & CNT_SIDEBAR) && (dat->pContainer->SideBar->getFlags() & CSideBar::SIDEBARORIENTATION_LEFT)) {
		DeferWindowPos(hdwp, hwndToggleSideBar, NULL, 4, 2 + splitterY - iOff, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		lwidth += 10;
		tempL -= 10;
	}

	for (i = 0; i < LButtonsList.getCount(); i++) {
		CustomButtonData *cbd = LButtonsList[i];
		if (((dat->bType == SESSIONTYPE_IM) && cbd->bIMButton) || ((dat->bType == SESSIONTYPE_CHAT) && cbd->bChatButton)) {
			hwndBtn = GetDlgItem(hwnd, cbd->dwButtonCID);

			if (!showToolbar) {
				ShowWindow(hwndBtn, SW_HIDE);
				DeferWindowPos(hdwp, hwndBtn, NULL, lwidth, splitterY - iOff, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				if (IsWindowVisible(hwndBtn) || (cbd->bSeparator && !(cbd->bAutoHidden || cbd->bHidden)))
					lwidth += cbd->iButtonWidth + gap;
				if (!IsWindowEnabled(hwndBtn) && !IsWindowVisible(hwndBtn) && !cbd->bAutoHidden)
					cbd->bAutoHidden = 1;
				continue;
			}
			if (!cbd->bCanBeHidden && !cbd->bHidden && !(!IsWindowEnabled(hwndBtn) && !IsWindowVisible(hwndBtn) && !cbd->bAutoHidden)) {
				ShowWindow(hwndBtn, SW_SHOW);
				cbd->bAutoHidden = 0;
			}

			if (!cbd->bSeparator && !IsWindowVisible(hwndBtn) && !IsWindowEnabled(hwndBtn) && !cbd->bAutoHidden)
				tempL -= cbd->iButtonWidth + gap;

			if (cbd->bCanBeHidden && !cbd->bHidden && (cbd->bSeparator || !((!IsWindowEnabled(hwndBtn) && !IsWindowVisible(hwndBtn)) && !cbd->bAutoHidden))) {
				if (tempL + tempR > (rect.right - foravatar)) {
					ShowWindow(hwndBtn, SW_HIDE);
					cbd->bAutoHidden = 1;
					tempL -= cbd->iButtonWidth + gap;
				}
				else if (cbd->bAutoHidden) {
					ShowWindow(hwndBtn, SW_SHOW);
					cbd->bAutoHidden = 0;
				}
			}
			DeferWindowPos(hdwp, hwndBtn, NULL, lwidth, splitterY - iOff, 0, 0, SWP_NOZORDER | SWP_NOSIZE);// SWP_NOCOPYBITS);
			if (IsWindowVisible(hwndBtn) || (cbd->bSeparator && !(cbd->bAutoHidden || cbd->bHidden)))
				lwidth += cbd->iButtonWidth + gap;
		}
	}

	if ((dat->pContainer->dwFlags & CNT_SIDEBAR) && (dat->pContainer->SideBar->getFlags() & CSideBar::SIDEBARORIENTATION_RIGHT)) {
		DeferWindowPos(hdwp, hwndToggleSideBar, NULL, rect.right - foravatar - 10, 2 + splitterY - iOff, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		rwidth += 12;
		tempR -= 12;
	}

	for (i = 0; i < RButtonsList.getCount(); i++) {
		CustomButtonData *cbd = RButtonsList[i];
		if (((dat->bType == SESSIONTYPE_IM) && cbd->bIMButton) || ((dat->bType == SESSIONTYPE_CHAT) && cbd->bChatButton)) {
			hwndBtn = GetDlgItem(hwnd, cbd->dwButtonCID);

			if (!showToolbar) {
				ShowWindow(hwndBtn, SW_HIDE);
				if (IsWindowVisible(hwndBtn) || (cbd->bSeparator && !(cbd->bAutoHidden || cbd->bHidden)))
					rwidth += cbd->iButtonWidth + gap;
				DeferWindowPos(hdwp, hwndBtn, NULL, rect.right - foravatar - rwidth + gap, splitterY - iOff, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				if (!IsWindowEnabled(hwndBtn) && !IsWindowVisible(hwndBtn) && !cbd->bAutoHidden)
					cbd->bAutoHidden = 1;
				continue;
			}
			if (!cbd->bCanBeHidden && !cbd->bHidden && !((!IsWindowEnabled(hwndBtn) && !IsWindowVisible(hwndBtn)) && !cbd->bAutoHidden)) {
				ShowWindow(hwndBtn, SW_SHOW);
				cbd->bAutoHidden = 0;
			}

			if (!cbd->bSeparator && !IsWindowVisible(hwndBtn) && !IsWindowEnabled(hwndBtn) && !cbd->bAutoHidden)
				tempR -= cbd->iButtonWidth + gap;

			if (cbd->bCanBeHidden && !cbd->bHidden && (cbd->bSeparator || !((!IsWindowEnabled(hwndBtn) && !IsWindowVisible(hwndBtn)) && !cbd->bAutoHidden))) {
				if (tempL + tempR > (rect.right - foravatar)) {
					ShowWindow(hwndBtn, SW_HIDE);
					cbd->bAutoHidden = 1;
					tempR -= cbd->iButtonWidth + gap;
				}
				else if (cbd->bAutoHidden) {
					ShowWindow(hwndBtn, SW_SHOW);
					cbd->bAutoHidden = 0;
				}
			}

			if (IsWindowVisible(hwndBtn) || (cbd->bSeparator && !(cbd->bAutoHidden || cbd->bHidden)))
				rwidth += cbd->iButtonWidth + gap;
			DeferWindowPos(hdwp, hwndBtn, NULL, rect.right - foravatar - rwidth + gap, splitterY - iOff, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}
	}

	return EndDeferWindowPos(hdwp);
}

void TSAPI BB_CustomButtonClick(TWindowData *dat, DWORD idFrom, HWND hwndFrom, BOOL code)
{
	RECT rc;
	int i;
	BOOL bFromArrow = 0;
	CustomButtonClickData cbcd = { 0 };

	GetWindowRect(hwndFrom, &rc);
	cbcd.pt.x = rc.left;
	cbcd.pt.y = rc.bottom;

	for (i = 0; i < LButtonsList.getCount(); i++) {
		CustomButtonData* cbd = LButtonsList[i];
		if (cbd->dwButtonCID == idFrom) {
			cbcd.pszModule = cbd->pszModuleName;
			cbcd.dwButtonId = cbd->dwButtonOrigID;
		}
		else if (cbd->dwArrowCID == idFrom) {
			bFromArrow = 1;
			cbcd.pszModule = cbd->pszModuleName;
			cbcd.dwButtonId = cbd->dwButtonOrigID;
		}
	}

	if (!cbcd.pszModule)
		for (i = 0; i < RButtonsList.getCount(); i++) {
		CustomButtonData* cbd = RButtonsList[i];
		if (cbd->dwButtonCID == idFrom) {
			cbcd.pszModule = cbd->pszModuleName;
			cbcd.dwButtonId = cbd->dwButtonOrigID;
		}
		else if (cbd->dwArrowCID == idFrom) {
			bFromArrow = 1;
			cbcd.pszModule = cbd->pszModuleName;
			cbcd.dwButtonId = cbd->dwButtonOrigID;
		}
		}

	cbcd.cbSize = sizeof(CustomButtonClickData);
	cbcd.hwndFrom = dat->hwnd;
	cbcd.hContact = dat->hContact;
	cbcd.flags = (code ? BBCF_RIGHTBUTTON : 0) | (GetKeyState(VK_SHIFT) & 0x8000 ? BBCF_SHIFTPRESSED : 0) | (GetKeyState(VK_CONTROL) & 0x8000 ? BBCF_CONTROLPRESSED : 0) | (bFromArrow ? BBCF_ARROWCLICKED : 0);

	NotifyEventHooks(hHookButtonPressedEvt, dat->hContact, (LPARAM)&cbcd);
}


void CB_GetButtonSettings(MCONTACT hContact, CustomButtonData *cbd)
{
	DBVARIANT  dbv = { 0 };
	char SettingName[1024];
	char* token = NULL;

	//modulename_buttonID, position_inIM_inCHAT_isLSide_isRSide_CanBeHidden

	mir_snprintf(SettingName, SIZEOF(SettingName), "%s_%d", cbd->pszModuleName, cbd->dwButtonOrigID);

	if (!db_get_s(hContact, "TabSRMM_Toolbar", SettingName, &dbv)) {
		token = strtok(dbv.pszVal, "_");
		cbd->dwPosition = (DWORD)atoi(token);
		token = strtok(NULL, "_");
		cbd->bIMButton = atoi(token) != 0;
		token = strtok(NULL, "_");
		cbd->bChatButton = atoi(token) != 0;
		token = strtok(NULL, "_");
		cbd->bLSided = atoi(token) != 0;
		token = strtok(NULL, "_");
		cbd->bRSided = atoi(token) != 0;
		token = strtok(NULL, "_");
		cbd->bCanBeHidden = atoi(token) != 0;

		db_free(&dbv);
	}
}

void CB_WriteButtonSettings(MCONTACT hContact, CustomButtonData *cbd)
{
	char SettingName[1024];
	char SettingParameter[1024];

	//modulename_buttonID, position_inIM_inCHAT_isLSide_isRSide_CanBeHidden

	mir_snprintf(SettingName, SIZEOF(SettingName), "%s_%d", cbd->pszModuleName, cbd->dwButtonOrigID);
	mir_snprintf(SettingParameter, SIZEOF(SettingParameter), "%d_%u_%u_%u_%u_%u", cbd->dwPosition, cbd->bIMButton, cbd->bChatButton, cbd->bLSided, cbd->bRSided, cbd->bCanBeHidden);
	if (!(cbd->opFlags & BBSF_NTBDESTRUCT))
		db_set_s(hContact, "TabSRMM_Toolbar", SettingName, SettingParameter);
	else
		db_unset(hContact, "TabSRMM_Toolbar", SettingName);
}

void BB_RegisterSeparators()
{
	BBButton bbd = { 0 };
	DWORD i = 0;
	bbd.cbSize = sizeof(BBButton);
	bbd.pszModuleName = "Tabsrmm_sep";
	for (; dwSepCount > i; i++) {
		bbd.bbbFlags = BBBF_ISDUMMYBUTTON | BBBF_ISIMBUTTON | BBBF_ISLSIDEBUTTON;
		bbd.dwButtonID = i + 1;
		bbd.dwDefPos = 410 + i;
		CB_AddButton(0, (LPARAM)&bbd);
	}
}

void BB_RefreshTheme(const TWindowData *dat)
{
	for (int i = 0; i < RButtonsList.getCount(); i++) {
		CustomButtonData* cbd = RButtonsList[i];
		SendDlgItemMessage(dat->hwnd, cbd->dwButtonCID, WM_THEMECHANGED, 0, 0);
	}
	for (int i = 0; i < LButtonsList.getCount(); i++) {
		CustomButtonData* cbd = LButtonsList[i];
		SendDlgItemMessage(dat->hwnd, cbd->dwButtonCID, WM_THEMECHANGED, 0, 0);
	}
}

void CB_InitDefaultButtons()
{
	BBButton bbd = { 0 };
	bbd.cbSize = sizeof(BBButton);
	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISLSIDEBUTTON | BBBF_ISARROWBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_PROTOCOL;
	bbd.dwDefPos = 10;
	bbd.hIcon = LoadSkinnedIconHandle(SKINICON_OTHER_CONNECTING);
	bbd.pszModuleName = "Tabsrmm";
	bbd.ptszTooltip = LPGENT("Protocol button");
	CB_AddButton(0, (LPARAM)&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISLSIDEBUTTON | BBBF_ISARROWBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_NAME;
	bbd.dwDefPos = 20;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[20];
	bbd.ptszTooltip = LPGENT("Info button");
	CB_AddButton(0, (LPARAM)&bbd);

	if (PluginConfig.g_SmileyAddAvail) {
		bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISLSIDEBUTTON | BBBF_CREATEBYID;
		bbd.dwButtonID = IDC_SMILEYBTN;
		bbd.iButtonWidth = 0;
		bbd.dwDefPos = 30;
		bbd.hIcon = PluginConfig.g_buttonBarIconHandles[9];
		bbd.ptszTooltip = LPGENT("Insert emoticon");
		CB_AddButton(0, (LPARAM)&bbd);
	}

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISLSIDEBUTTON | BBBF_ISPUSHBUTTON | BBBF_CANBEHIDDEN | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_FONTBOLD;
	bbd.dwDefPos = 40;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[10];
	bbd.ptszTooltip = LPGENT("Bold text");
	CB_AddButton(0, (LPARAM)&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISLSIDEBUTTON | BBBF_ISPUSHBUTTON | BBBF_CANBEHIDDEN | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_FONTITALIC;
	bbd.dwDefPos = 50;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[11];
	bbd.ptszTooltip = LPGENT("Italic text");
	CB_AddButton(0, (LPARAM)&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISLSIDEBUTTON | BBBF_ISPUSHBUTTON | BBBF_CANBEHIDDEN | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_FONTUNDERLINE;
	bbd.dwDefPos = 60;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[12];
	bbd.ptszTooltip = LPGENT("Underlined text");
	CB_AddButton(0, (LPARAM)&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISLSIDEBUTTON | BBBF_ISPUSHBUTTON | BBBF_CANBEHIDDEN | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_FONTSTRIKEOUT;
	bbd.dwDefPos = 70;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[15];
	bbd.ptszTooltip = LPGENT("Strike-through text");
	CB_AddButton(0, (LPARAM)&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISLSIDEBUTTON | BBBF_CANBEHIDDEN | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_FONTFACE;
	bbd.dwDefPos = 80;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[14];
	bbd.ptszTooltip = LPGENT("Select font color");
	CB_AddButton(0, (LPARAM)&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_ISARROWBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDOK;
	bbd.dwDefPos = 10;
	bbd.iButtonWidth = 51;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[6];
	bbd.ptszTooltip = LPGENT("Send message\nClick dropdown arrow for sending options");
	CB_AddButton(0, (LPARAM)&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_SAVE;
	bbd.dwDefPos = 20;
	bbd.iButtonWidth = 0;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[8];
	bbd.ptszTooltip = LPGENT("Close session");
	CB_AddButton(0, (LPARAM)&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_QUOTE;
	bbd.dwDefPos = 30;
	bbd.iButtonWidth = 0;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[4];
	bbd.ptszTooltip = LPGENT("Quote last message OR selected text");
	CB_AddButton(0, (LPARAM)&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_TIME;
	bbd.dwDefPos = 40;
	bbd.iButtonWidth = 0;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[2];
	bbd.ptszTooltip = LPGENT("Message log options");
	CB_AddButton(0, (LPARAM)&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_HISTORY;
	bbd.dwDefPos = 50;
	bbd.iButtonWidth = 0;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[0];
	bbd.ptszTooltip = LPGENT("View user's history");
	CB_AddButton(0, (LPARAM)&bbd);

	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_PIC;
	bbd.dwDefPos = 60;
	bbd.iButtonWidth = 0;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[7];
	bbd.ptszTooltip = LPGENT("Edit user notes");
	CB_AddButton(0, (LPARAM)&bbd);

	//chat buttons

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISLSIDEBUTTON | BBBF_ISDUMMYBUTTON;
	bbd.dwButtonID = 1;
	bbd.pszModuleName = "tb_splitter";
	bbd.dwDefPos = 31;
	bbd.iButtonWidth = 22;
	bbd.hIcon = 0;
	bbd.pszTooltip = 0;
	CB_AddButton(0, (LPARAM)&bbd);

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISLSIDEBUTTON | BBBF_ISDUMMYBUTTON;
	bbd.dwButtonID = 2;
	bbd.dwDefPos = 22;
	bbd.iButtonWidth = 22;
	bbd.hIcon = 0;
	bbd.pszTooltip = 0;
	CB_AddButton(0, (LPARAM)&bbd);

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISLSIDEBUTTON | BBBF_ISDUMMYBUTTON;
	bbd.dwButtonID = 3;
	bbd.dwDefPos = 71;
	bbd.iButtonWidth = 22;
	bbd.hIcon = 0;
	bbd.pszTooltip = 0;
	CB_AddButton(0, (LPARAM)&bbd);

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISLSIDEBUTTON | BBBF_ISPUSHBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_BKGCOLOR;
	bbd.pszModuleName = "Tabsrmm";
	bbd.dwDefPos = 81;
	bbd.iButtonWidth = 22;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[16];
	bbd.ptszTooltip = LPGENT("Change background color");
	CB_AddButton(0, (LPARAM)&bbd);

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_SHOWNICKLIST;
	bbd.dwDefPos = 22;
	bbd.iButtonWidth = 22;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[19];
	bbd.ptszTooltip = LPGENT("Toggle nick list");
	CB_AddButton(0, (LPARAM)&bbd);

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_FILTER;
	bbd.dwDefPos = 24;
	bbd.iButtonWidth = 22;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[18];
	bbd.ptszTooltip = LPGENT("Event filter - right click to setup, left click to activate/deactivate");
	CB_AddButton(0, (LPARAM)&bbd);

	bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_CREATEBYID;
	bbd.dwButtonID = IDC_CHANMGR;
	bbd.dwDefPos = 33;
	bbd.iButtonWidth = 22;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[17];
	bbd.ptszTooltip = LPGENT("Channel manager");
	CB_AddButton(0, (LPARAM)&bbd);

	BB_RegisterSeparators();
}

#define MIDDLE_SEPARATOR _T(">-------M-------<")

static int SaveTree(HWND hToolBarTree)
{
	bool RSide = false;
	int count = 10;
	DWORD loc_sepcout = 0;
	TCHAR strbuf[128];

	TVITEM tvi;
	tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_HANDLE;
	tvi.hItem = TreeView_GetRoot(hToolBarTree);
	tvi.pszText = strbuf;
	tvi.cchTextMax = SIZEOF(strbuf);
	{
		mir_cslock lck(ToolBarCS);

		while (tvi.hItem != NULL) {
			TreeView_GetItem(hToolBarTree, &tvi);

			if (_tcscmp(tvi.pszText, MIDDLE_SEPARATOR) == 0) {
				RSide = true;
				count = TreeView_GetCount(hToolBarTree) * 10 - count;
				tvi.hItem = TreeView_GetNextSibling(hToolBarTree, tvi.hItem);
				continue;
			}
			CustomButtonData *cbd = (CustomButtonData*)tvi.lParam;
			if (cbd) {
				if (cbd->opFlags) {
					cbd->bIMButton = (cbd->opFlags & BBSF_IMBUTTON) != 0;
					cbd->bChatButton = (cbd->opFlags & BBSF_CHATBUTTON) != 0;
					cbd->bCanBeHidden = (cbd->opFlags & BBSF_CANBEHIDDEN) != 0;
				}
				if (RSide && cbd->bLSided) {
					cbd->bLSided = false;
					cbd->bRSided = true;
					cbd->opFlags |= BBSF_NTBSWAPED;
				}
				else if (!RSide && cbd->bRSided) {
					cbd->bLSided = true;
					cbd->bRSided = false;
					cbd->opFlags |= BBSF_NTBSWAPED;
				}
				if (!TreeView_GetCheckState(hToolBarTree, tvi.hItem)) {
					cbd->bIMButton = false;
					cbd->bChatButton = false;

					if (cbd->bSeparator && !strcmp(cbd->pszModuleName, "Tabsrmm_sep"))
						cbd->opFlags = BBSF_NTBDESTRUCT;
				}
				else {
					if (!cbd->bIMButton && !cbd->bChatButton)
						cbd->bIMButton = true;
					if (cbd->bSeparator && !strcmp(cbd->pszModuleName, "Tabsrmm_sep")) {
						cbd->bHidden = 0;
						cbd->opFlags &= ~BBSF_NTBDESTRUCT;
						++loc_sepcout;
					}
				}

				cbd->dwPosition = (DWORD)count;
				CB_WriteButtonSettings(NULL, cbd);

				if (!(cbd->opFlags & BBSF_NTBDESTRUCT))
					(RSide) ? (count -= 10) : (count += 10);
			}
			
			HTREEITEM hItem = TreeView_GetNextSibling(hToolBarTree, tvi.hItem);
			if (cbd->opFlags & BBSF_NTBDESTRUCT)
				TreeView_DeleteItem(hToolBarTree, tvi.hItem);
			tvi.hItem = hItem;
		}

		qsort(LButtonsList.getArray(), LButtonsList.getCount(), sizeof(void*), sstSortButtons);
		qsort(RButtonsList.getArray(), RButtonsList.getCount(), sizeof(void*), sstSortButtons);
	}
	db_set_dw(0, "TabSRMM_Toolbar", "SeparatorsCount", loc_sepcout);
	dwSepCount = loc_sepcout;
	return 1;
}

HIMAGELIST himgl = NULL;

static int BuildMenuObjectsTree(HWND hToolBarTree)
{
	HTREEITEM hti;
	int iImage = 0;

	TVINSERTSTRUCT tvis;
	tvis.hParent = NULL;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_IMAGE;

	TreeView_DeleteAllItems(hToolBarTree);

	himgl = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 2, 2);
	ImageList_AddIcon(himgl, LoadSkinnedIcon(SKINICON_OTHER_SMALLDOT));
	ImageList_Destroy(TreeView_GetImageList(hToolBarTree, TVSIL_NORMAL));
	TreeView_SetImageList(hToolBarTree, himgl, TVSIL_NORMAL);

	if ((RButtonsList.getCount() + LButtonsList.getCount()) == 0)
		return FALSE;

	mir_cslock lck(ToolBarCS);

	for (int i = 0; i < LButtonsList.getCount(); i++) {
		CustomButtonData *cbd = LButtonsList[i];
		tvis.item.lParam = (LPARAM)cbd;

		if (cbd->bSeparator) {
			tvis.item.pszText = TranslateT("<Separator>");
			tvis.item.iImage = tvis.item.iSelectedImage = 0;
		}
		else {
			tvis.item.pszText = TranslateTS(cbd->ptszTooltip);
			iImage = ImageList_AddIcon(himgl, Skin_GetIconByHandle(cbd->hIcon));
			tvis.item.iImage = tvis.item.iSelectedImage = iImage;
		}
		cbd->opFlags = 0;
		hti = TreeView_InsertItem(hToolBarTree, &tvis);

		TreeView_SetCheckState(hToolBarTree, hti, (cbd->bIMButton || cbd->bChatButton));
	}

	tvis.item.lParam = 0;
	tvis.item.mask |= TVIF_STATE;
	tvis.item.pszText = MIDDLE_SEPARATOR;
	tvis.item.stateMask = TVIS_BOLD;
	tvis.item.state = TVIS_BOLD;
	tvis.item.iImage = tvis.item.iSelectedImage = -1;
	hti = TreeView_InsertItem(hToolBarTree, &tvis);
	TreeView_SetCheckState(hToolBarTree, hti, 1);

	for (int i = RButtonsList.getCount()-1; i >= 0; i--) {
		CustomButtonData *cbd = RButtonsList[i];
		tvis.item.lParam = (LPARAM)cbd;

		if (cbd->bSeparator) {
			tvis.item.pszText = TranslateT("<Separator>");
			tvis.item.iImage = tvis.item.iSelectedImage = -1;
		}
		else {
			tvis.item.pszText = TranslateTS(cbd->ptszTooltip);
			iImage = ImageList_AddIcon(himgl, Skin_GetIconByHandle(cbd->hIcon));
			tvis.item.iImage = tvis.item.iSelectedImage = iImage;
		}
		tvis.item.state = 0;
		cbd->opFlags = 0;
		hti = TreeView_InsertItem(hToolBarTree, &tvis);
		TreeView_SetCheckState(hToolBarTree, hti, (cbd->bIMButton || cbd->bChatButton));
	}

	return 1;
}


BOOL drag = FALSE, bOptionsInit = TRUE;
HANDLE 	hDragItem = NULL;
HWND	 hToolBarTree = NULL;

INT_PTR CALLBACK DlgProcToolBar(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HTREEITEM hti;

	switch (uMsg) {
	case WM_INITDIALOG:
		hToolBarTree = GetDlgItem(hwndDlg, IDC_TOOLBARTREE);
		{
			LONG_PTR style = GetWindowLongPtr(hToolBarTree, GWL_STYLE);
			style ^= TVS_CHECKBOXES;
			SetWindowLongPtr(hToolBarTree, GWL_STYLE, style);
			style |= TVS_CHECKBOXES;
			style |= TVS_NOHSCROLL;
			SetWindowLongPtr(hToolBarTree, GWL_STYLE, style);
		}
		{
			mir_cslock lck(ToolBarCS);
			BuildMenuObjectsTree(hToolBarTree);
		}

		Utils::enableDlgControl(hwndDlg, IDC_IMCHECK, FALSE);
		Utils::enableDlgControl(hwndDlg, IDC_CHATCHECK, FALSE);
		Utils::enableDlgControl(hwndDlg, IDC_CANBEHIDDEN, FALSE);

		SendDlgItemMessage(hwndDlg, IDC_SPIN1, UDM_SETRANGE, 0, MAKELONG(10, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN1, UDM_SETPOS, 0, MAKELONG(PluginConfig.g_iButtonsBarGap, 0));
		TranslateDialogDefault(hwndDlg);
		bOptionsInit = FALSE;
		break;

	case WM_LBUTTONUP:
		if (drag) {
			TreeView_SetInsertMark(hToolBarTree, NULL, 0);
			drag = 0;
			ReleaseCapture();

			TVHITTESTINFO hti;
			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam);
			ClientToScreen(hwndDlg, &hti.pt);
			ScreenToClient(hToolBarTree, &hti.pt);
			hti.pt.y -= TreeView_GetItemHeight(hToolBarTree) / 2;
			TreeView_HitTest(hToolBarTree, &hti);
			if (hDragItem == hti.hItem) break;
			if (hti.flags & TVHT_ABOVE)
				hti.hItem = TVI_FIRST;

			TVITEM tvi;
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = (HTREEITEM)hDragItem;
			TreeView_GetItem(hToolBarTree, &tvi);
			if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT) || (hti.hItem == TVI_FIRST)) {
				TVINSERTSTRUCT tvis;
				TCHAR strbuf[128];
				tvis.item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
				tvis.item.stateMask = 0xFFFFFFFF;
				tvis.item.pszText = strbuf;
				tvis.item.cchTextMax = SIZEOF(strbuf);
				tvis.item.hItem = (HTREEITEM)hDragItem;
				TreeView_GetItem(hToolBarTree, &tvis.item);
				TreeView_DeleteItem(hToolBarTree, hDragItem);
				tvis.hParent = NULL;
				tvis.hInsertAfter = hti.hItem;
				TreeView_SelectItem(hToolBarTree, TreeView_InsertItem(hToolBarTree, &tvis));
				SendMessage((GetParent(hwndDlg)), PSM_CHANGED, 0, 0);
			}
		}
		break;

	case WM_MOUSEMOVE:
		if (drag) {
			TVHITTESTINFO hti;
			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam);
			ClientToScreen(hwndDlg, &hti.pt);
			ScreenToClient(hToolBarTree, &hti.pt);
			TreeView_HitTest(hToolBarTree, &hti);
			if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) {
				HTREEITEM it = hti.hItem;
				hti.pt.y -= TreeView_GetItemHeight(hToolBarTree) / 2;
				TreeView_HitTest(hToolBarTree, &hti);
				if (!(hti.flags & TVHT_ABOVE))
					TreeView_SetInsertMark(hToolBarTree, hti.hItem, 1);
				else
					TreeView_SetInsertMark(hToolBarTree, it, 0);
			}
			else {
				if (hti.flags & TVHT_ABOVE) SendMessage(hToolBarTree, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
				if (hti.flags & TVHT_BELOW) SendMessage(hToolBarTree, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
				TreeView_SetInsertMark(hToolBarTree, NULL, 0);
			}
		}
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED && GetFocus() == (HWND)lParam && (HWND)lParam != hToolBarTree)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		if ((HIWORD(wParam) == EN_CHANGE) && ((HWND)lParam == GetFocus()))
			if (!bOptionsInit)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		switch (LOWORD(wParam)) {
		case IDC_BBRESET:
			CallService(MS_DB_MODULE_DELETE, NULL, LPARAM("TabSRMM_Toolbar"));
			CB_HardReInit();
			BuildMenuObjectsTree(hToolBarTree);
			break;

		case IDC_SEPARATOR:
			hti = TreeView_GetSelection(hToolBarTree);
			if (!hti)
				hti = TVI_FIRST;

			CustomButtonData *cbd = new CustomButtonData();
			cbd->bSeparator = cbd->bHidden = cbd->bIMButton = cbd->bLSided = true;
			cbd->dwButtonOrigID = ++dwSepCount;
			cbd->pszModuleName = "Tabsrmm_sep";
			cbd->iButtonWidth = 22;
			cbd->opFlags = BBSF_NTBDESTRUCT;
			LButtonsList.insert(cbd);

			TVINSERTSTRUCT tvis;
			tvis.hParent = NULL;
			tvis.hInsertAfter = hti;
			tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

			tvis.item.pszText = TranslateT("<Separator>");
			tvis.item.iImage = tvis.item.iSelectedImage = -1;
			tvis.item.lParam = (LPARAM)cbd;
			hti = TreeView_InsertItem(hToolBarTree, &tvis);

			TreeView_SetCheckState(hToolBarTree, hti, (cbd->bIMButton || cbd->bChatButton));
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_RESET:
				CB_ReInitCustomButtons();
				dwSepCount = M.GetDword("TabSRMM_Toolbar", "SeparatorsCount", 0);
				return 1;

			case PSN_APPLY:
				hti = TreeView_GetSelection(hToolBarTree);
				if (hti) {
					TVITEM tvi;
					tvi.mask = TVIF_HANDLE | TVIF_PARAM;
					tvi.hItem = hti;
					TreeView_GetItem(hToolBarTree, &tvi);

					if (tvi.lParam) {
						CustomButtonData *cbd = (CustomButtonData*)tvi.lParam;
						if (cbd) {
							cbd->bIMButton = IsDlgButtonChecked(hwndDlg, IDC_IMCHECK) != 0;
							cbd->bChatButton = IsDlgButtonChecked(hwndDlg, IDC_CHATCHECK) != 0;
							cbd->bCanBeHidden = IsDlgButtonChecked(hwndDlg, IDC_CANBEHIDDEN) != 0;
						}
					}
				}

				SaveTree(hToolBarTree);
				CB_ReInitCustomButtons();
				PluginConfig.g_iButtonsBarGap = (BYTE)SendDlgItemMessage(hwndDlg, IDC_SPIN1, UDM_GETPOS, 0, 0);

				if (PluginConfig.g_iButtonsBarGap != M.GetByte("ButtonsBarGap", 1))
					M.BroadcastMessageAsync(WM_SIZE, 0, 0);

				db_set_b(0, SRMSGMOD_T, "ButtonsBarGap", PluginConfig.g_iButtonsBarGap);

				BuildMenuObjectsTree((HWND)hToolBarTree);
				Utils::enableDlgControl(hwndDlg, IDC_IMCHECK, FALSE);
				Utils::enableDlgControl(hwndDlg, IDC_CHATCHECK, FALSE);
				Utils::enableDlgControl(hwndDlg, IDC_CANBEHIDDEN, FALSE);
				return 1;
			}
			break;

		case IDC_TOOLBARTREE:
			switch (((LPNMHDR)lParam)->code) {
			case TVN_BEGINDRAGA:
			case TVN_BEGINDRAGW:
				SetCapture(hwndDlg);
				drag = 1;
				hDragItem = ((LPNMTREEVIEW)lParam)->itemNew.hItem;
				TreeView_SelectItem(hToolBarTree, hDragItem);
				break;

			case TVN_SELCHANGINGA:
			case TVN_SELCHANGINGW:
				hti = TreeView_GetSelection(hToolBarTree);
				if (hti != NULL) {
					TCHAR strbuf[128];

					TVITEM tvi;
					tvi.hItem = hti;
					tvi.pszText = strbuf;
					tvi.cchTextMax = SIZEOF(strbuf);
					tvi.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
					TreeView_GetItem(hToolBarTree, &tvi);

					if (tvi.lParam == 0 || !TreeView_GetCheckState(hToolBarTree, tvi.hItem) || !_tcscmp(tvi.pszText, MIDDLE_SEPARATOR))
						break;

					CustomButtonData *cbd = (CustomButtonData*)tvi.lParam;
					if (cbd) {
						cbd->opFlags = (IsDlgButtonChecked(hwndDlg, IDC_IMCHECK)) ? BBSF_IMBUTTON : 0;
						cbd->opFlags |= (IsDlgButtonChecked(hwndDlg, IDC_CHATCHECK)) ? BBSF_CHATBUTTON : 0;
						cbd->opFlags |= (IsDlgButtonChecked(hwndDlg, IDC_CANBEHIDDEN)) ? BBSF_CANBEHIDDEN : 0;

						cbd->bIMButton = (IsDlgButtonChecked(hwndDlg, IDC_IMCHECK) ? TRUE : FALSE);
						cbd->bChatButton = (IsDlgButtonChecked(hwndDlg, IDC_CHATCHECK) ? TRUE : FALSE);
						cbd->bCanBeHidden = (IsDlgButtonChecked(hwndDlg, IDC_CANBEHIDDEN) ? TRUE : FALSE);
					}
				}
				break;

			case TVN_SELCHANGEDW:
			case TVN_SELCHANGEDA:
				hti = TreeView_GetSelection(hToolBarTree);
				if (hti != NULL) {
					TCHAR strbuf[128];

					TVITEM tvi;
					tvi.pszText = strbuf;
					tvi.cchTextMax = SIZEOF(strbuf);
					tvi.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
					tvi.hItem = hti;
					TreeView_GetItem(hToolBarTree, &tvi);

					if (!TreeView_GetCheckState(hToolBarTree, tvi.hItem) || !_tcscmp(tvi.pszText, MIDDLE_SEPARATOR)) {
						Utils::enableDlgControl(hwndDlg, IDC_IMCHECK, FALSE);
						Utils::enableDlgControl(hwndDlg, IDC_CHATCHECK, FALSE);
						Utils::enableDlgControl(hwndDlg, IDC_CANBEHIDDEN, FALSE);
						break;
					}

					if (tvi.lParam == 0)
						break;

					CustomButtonData *cbd = (CustomButtonData*)tvi.lParam;
					if (cbd) {
						Utils::enableDlgControl(hwndDlg, IDC_IMCHECK, TRUE);
						Utils::enableDlgControl(hwndDlg, IDC_CHATCHECK, TRUE);
						Utils::enableDlgControl(hwndDlg, IDC_CANBEHIDDEN, TRUE);
						CheckDlgButton(hwndDlg, IDC_IMCHECK, (cbd->bIMButton) ? BST_CHECKED : BST_UNCHECKED);
						CheckDlgButton(hwndDlg, IDC_CHATCHECK, (cbd->bChatButton) ? BST_CHECKED : BST_UNCHECKED);
						CheckDlgButton(hwndDlg, IDC_CANBEHIDDEN, (cbd->bCanBeHidden) ? BST_CHECKED : BST_UNCHECKED);
					}
				}
				break;

			case NM_CLICK:
				TVHITTESTINFO hti = { 0 };
				GetCursorPos(&hti.pt);
				ScreenToClient(hToolBarTree, &hti.pt);
				if (TreeView_HitTest(hToolBarTree, &hti)) {
					if (hti.flags&TVHT_ONITEMSTATEICON) {
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
						if (TreeView_GetCheckState(hToolBarTree, hti.hItem)) {
							Utils::enableDlgControl(hwndDlg, IDC_IMCHECK, FALSE);
							Utils::enableDlgControl(hwndDlg, IDC_CHATCHECK, FALSE);
							Utils::enableDlgControl(hwndDlg, IDC_CANBEHIDDEN, FALSE);
							CheckDlgButton(hwndDlg, IDC_IMCHECK, BST_CHECKED);
						}
						else {
							Utils::enableDlgControl(hwndDlg, IDC_IMCHECK, TRUE);
							Utils::enableDlgControl(hwndDlg, IDC_CHATCHECK, TRUE);
							Utils::enableDlgControl(hwndDlg, IDC_CANBEHIDDEN, TRUE);
						}
						TreeView_SelectItem(hToolBarTree, hti.hItem);
					}
				}
			}
		}
		break;

	case WM_DESTROY:
		HIMAGELIST hIml = TreeView_GetImageList(GetDlgItem(hwndDlg, IDC_TOOLBARTREE), TVSIL_NORMAL);
		ImageList_Destroy(hIml);
		hIml = TreeView_GetImageList(GetDlgItem(hwndDlg, IDC_TOOLBARTREE), TVSIL_STATE);
		ImageList_Destroy(hIml);
		break;
	}

	return FALSE;
}
