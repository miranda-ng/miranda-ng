/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-16 Miranda NG project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

#define MODULENAME "SRMM_Toolbar"

#define DPISCALEY_S(argY) ((int)((double)(argY) * g_DPIscaleY))
#define DPISCALEX_S(argX) ((int)((double)(argX) * g_DPIscaleX))

static double g_DPIscaleX, g_DPIscaleY;

static int SortButtons(const CustomButtonData *p1, const CustomButtonData *p2)
{
	if (p1->m_bRSided != p2->m_bRSided)
		return (p2->m_bRSided) ? -1 : 1;
	return p1->m_dwPosition - p2->m_dwPosition;
}

static LIST<CustomButtonData> arButtonsList(1, SortButtons);

DWORD LastCID = MIN_CBUTTONID;
DWORD dwSepCount = 0;

static mir_cs csToolBar;
static HANDLE hHookToolBarLoadedEvt, hHookButtonPressedEvt;

static void wipeList(LIST<CustomButtonData> &list)
{
	for (int i = list.getCount() - 1; i >= 0; i--) {
		delete list[i];
		list.remove(i);
	}
}

static int sstSortButtons(const void *p1, const void *p2)
{
	return SortButtons(*(CustomButtonData**)p1, *(CustomButtonData**)p2);
}

static void CB_RegisterSeparators()
{
	BBButton bbd = { 0 };
	bbd.pszModuleName = "Tabsrmm_sep";
	for (DWORD i = 0; dwSepCount > i; i++) {
		bbd.bbbFlags = BBBF_ISDUMMYBUTTON | BBBF_ISIMBUTTON;
		bbd.dwButtonID = i + 1;
		bbd.dwDefPos = 410 + i;
		Srmm_AddButton(&bbd);
	}
}

static void CB_GetButtonSettings(MCONTACT hContact, CustomButtonData *cbd)
{
	DBVARIANT  dbv = { 0 };
	char SettingName[1024];
	char* token = NULL;

	// modulename_buttonID, position_inIM_inCHAT_isLSide_isRSide_CanBeHidden

	mir_snprintf(SettingName, "%s_%d", cbd->m_pszModuleName, cbd->m_dwButtonOrigID);

	if (!db_get_s(hContact, MODULENAME, SettingName, &dbv)) {
		token = strtok(dbv.pszVal, "_");
		cbd->m_dwPosition = (DWORD)atoi(token);
		token = strtok(NULL, "_");
		cbd->m_bIMButton = atoi(token) != 0;
		token = strtok(NULL, "_");
		cbd->m_bChatButton = atoi(token) != 0;
		token = strtok(NULL, "_");
		token = strtok(NULL, "_");
		cbd->m_bRSided = atoi(token) != 0;
		token = strtok(NULL, "_");
		cbd->m_bCanBeHidden = atoi(token) != 0;

		db_free(&dbv);
	}
}

MIR_APP_DLL(CustomButtonData*) Srmm_GetNthButton(int i)
{
	return arButtonsList[i];
}

MIR_APP_DLL(int) Srmm_GetButtonCount(void)
{
	return arButtonsList.getCount();
}

MIR_APP_DLL(int) Srmm_AddButton(const BBButton *bbdi, int _hLang)
{
	if (bbdi == NULL)
		return 1;

	CustomButtonData *cbd = new CustomButtonData();
	if (!bbdi->iButtonWidth && (bbdi->bbbFlags & BBBF_ISARROWBUTTON))
		cbd->m_iButtonWidth = DPISCALEX_S(34);
	else if (!bbdi->iButtonWidth)
		cbd->m_iButtonWidth = DPISCALEX_S(22);
	else
		cbd->m_iButtonWidth = DPISCALEX_S(bbdi->iButtonWidth);

	cbd->m_pszModuleName = mir_strdup(bbdi->pszModuleName);
	cbd->m_pwszTooltip = mir_wstrdup(bbdi->pwszTooltip);

	cbd->m_dwButtonOrigID = bbdi->dwButtonID;
	cbd->m_hIcon = bbdi->hIcon;
	cbd->m_dwPosition = bbdi->dwDefPos;
	cbd->m_dwButtonCID = (bbdi->bbbFlags & BBBF_CREATEBYID) ? bbdi->dwButtonID : LastCID;
	cbd->m_dwArrowCID = (bbdi->bbbFlags & BBBF_ISARROWBUTTON) ? cbd->m_dwButtonCID + 1 : 0;
	cbd->m_bHidden = (bbdi->bbbFlags & BBBF_HIDDEN) != 0;
	cbd->m_bRSided = (bbdi->bbbFlags & BBBF_ISRSIDEBUTTON) != 0;
	cbd->m_bCanBeHidden = (bbdi->bbbFlags & BBBF_CANBEHIDDEN) != 0;
	cbd->m_bSeparator = (bbdi->bbbFlags & BBBF_ISDUMMYBUTTON) != 0;
	cbd->m_bChatButton = (bbdi->bbbFlags & BBBF_ISCHATBUTTON) != 0;
	cbd->m_bIMButton = (bbdi->bbbFlags & BBBF_ISIMBUTTON) != 0;
	cbd->m_bDisabled = (bbdi->bbbFlags & BBBF_DISABLED) != 0;
	cbd->m_bPushButton = (bbdi->bbbFlags & BBBF_ISPUSHBUTTON) != 0;
	cbd->m_hLangpack = _hLang;

	// download database settings
	CB_GetButtonSettings(NULL, cbd);

	arButtonsList.insert(cbd);

	if (cbd->m_dwButtonCID != cbd->m_dwButtonOrigID)
		LastCID++;
	if (cbd->m_dwArrowCID == LastCID)
		LastCID++;

	WindowList_Broadcast(chatApi.hWindowList, WM_CBD_UPDATED, 0, 0);
	return 0;
}

MIR_APP_DLL(int) Srmm_GetButtonState(HWND hwndDlg, BBButton *bbdi)
{
	if (hwndDlg == NULL || bbdi == NULL)
		return 1;

	DWORD tempCID = 0;
	bool realbutton = false;
	bbdi->bbbFlags = 0;
	for (int i = 0; i < arButtonsList.getCount(); i++) {
		CustomButtonData *cbd = arButtonsList[i];
		if (!mir_strcmp(cbd->m_pszModuleName, bbdi->pszModuleName) && (cbd->m_dwButtonOrigID == bbdi->dwButtonID)) {
			realbutton = true;
			tempCID = cbd->m_dwButtonCID;
		}
	}
	if (!realbutton)
		return 1;

	HWND hwndBtn = GetDlgItem(hwndDlg, tempCID);
	bbdi->bbbFlags = (IsDlgButtonChecked(hwndDlg, tempCID) ? BBSF_PUSHED : BBSF_RELEASED) | (IsWindowVisible(hwndBtn) ? 0 : BBSF_HIDDEN) | (IsWindowEnabled(hwndBtn) ? 0 : BBSF_DISABLED);
	return 0;
}

MIR_APP_DLL(int) Srmm_SetButtonState(MCONTACT hContact, BBButton *bbdi)
{
	if (hContact == NULL || bbdi == NULL)
		return 1;

	bool realbutton = false;
	DWORD tempCID = 0;
	for (int i = 0; i < arButtonsList.getCount(); i++) {
		CustomButtonData *cbd = arButtonsList[i];
		if (!mir_strcmp(cbd->m_pszModuleName, bbdi->pszModuleName) && (cbd->m_dwButtonOrigID == bbdi->dwButtonID)) {
			realbutton = true;
			tempCID = cbd->m_dwButtonCID;
		}
	}
	if (!realbutton)
		return 1;

	HWND hwndDlg = WindowList_Find(NULL, hContact);
	if (hwndDlg == NULL)
		return 1;

	HWND hwndBtn = GetDlgItem(hwndDlg, tempCID);
	if (hwndBtn == NULL)
		return 1;

	SetWindowTextA(hwndBtn, bbdi->pszModuleName);
	if (bbdi->hIcon)
		SendMessage(hwndBtn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIconByHandle(bbdi->hIcon));
	if (bbdi->pwszTooltip)
		SendMessage(hwndBtn, BUTTONADDTOOLTIP, (WPARAM)bbdi->pwszTooltip, BATF_UNICODE);
	if (bbdi->bbbFlags) {
		ShowWindow(hwndBtn, (bbdi->bbbFlags & BBSF_HIDDEN) ? SW_HIDE : SW_SHOW);
		EnableWindow(hwndBtn, !(bbdi->bbbFlags & BBSF_DISABLED));
		Button_SetCheck(hwndBtn, (bbdi->bbbFlags & BBSF_PUSHED) != 0);
		Button_SetCheck(hwndBtn, (bbdi->bbbFlags & BBSF_RELEASED) != 0);
	}
	return 0;
}

MIR_APP_DLL(int) Srmm_RemoveButton(BBButton *bbdi)
{
	if (!bbdi)
		return 1;

	CustomButtonData *pFound = NULL;
	{
		mir_cslock lck(csToolBar);

		for (int i = arButtonsList.getCount() - 1; i >= 0; i--) {
			CustomButtonData *cbd = arButtonsList[i];
			if (!mir_strcmp(cbd->m_pszModuleName, bbdi->pszModuleName) && cbd->m_dwButtonOrigID == bbdi->dwButtonID) {
				pFound = cbd;
				arButtonsList.remove(i);
			}
		}
	}

	if (pFound) {
		WindowList_Broadcast(chatApi.hWindowList, WM_CBD_REMOVED, pFound->m_dwButtonCID, (LPARAM)pFound);
		delete pFound;
	}
	return 0;
}

MIR_APP_DLL(int) Srmm_ModifyButton(BBButton *bbdi)
{
	if (!bbdi)
		return 1;

	bool bFound = false;
	CustomButtonData *cbd = NULL;
	{
		mir_cslock lck(csToolBar);

		for (int i = 0; i < arButtonsList.getCount(); i++) {
			cbd = arButtonsList[i];
			if (!mir_strcmp(cbd->m_pszModuleName, bbdi->pszModuleName) && (cbd->m_dwButtonOrigID == bbdi->dwButtonID)) {
				bFound = true;
				break;
			}
		}

		if (bFound) {
			if (bbdi->pwszTooltip)
				cbd->m_pwszTooltip = mir_wstrdup(bbdi->pwszTooltip);
			if (bbdi->hIcon)
				cbd->m_hIcon = bbdi->hIcon;
			if (bbdi->bbbFlags) {
				cbd->m_bHidden = (bbdi->bbbFlags & BBBF_HIDDEN) != 0;
				cbd->m_bRSided = (bbdi->bbbFlags & BBBF_ISRSIDEBUTTON) != 0;
				cbd->m_bCanBeHidden = (bbdi->bbbFlags & BBBF_CANBEHIDDEN) != 0;
				cbd->m_bChatButton = (bbdi->bbbFlags & BBBF_ISCHATBUTTON) != 0;
				cbd->m_bIMButton = (bbdi->bbbFlags & BBBF_ISIMBUTTON) != 0;
				cbd->m_bDisabled = (bbdi->bbbFlags & BBBF_DISABLED) != 0;
			}
		}
	}

	if (bFound)
		WindowList_Broadcast(chatApi.hWindowList, WM_CBD_UPDATED, 0, (LPARAM)cbd);
	return 0;
}

MIR_APP_DLL(void) Srmm_ClickToolbarIcon(MCONTACT hContact, DWORD idFrom, HWND hwndFrom, BOOL code)
{
	RECT rc;
	GetWindowRect(hwndFrom, &rc);

	bool bFromArrow = false;

	CustomButtonClickData cbcd = {};
	cbcd.pt.x = rc.left;
	cbcd.pt.y = rc.bottom;

	for (int i = 0; i < arButtonsList.getCount(); i++) {
		CustomButtonData *cbd = arButtonsList[i];
		if	(cbd->m_dwButtonCID == idFrom) {
			cbcd.pszModule = cbd->m_pszModuleName;
			cbcd.dwButtonId = cbd->m_dwButtonOrigID;
		}
		else if (cbd->m_dwArrowCID == idFrom) {
			bFromArrow = true;
			cbcd.pszModule = cbd->m_pszModuleName;
			cbcd.dwButtonId = cbd->m_dwButtonOrigID;
		}
	}

	cbcd.hwndFrom = GetParent(hwndFrom);
	cbcd.hContact = hContact;
	cbcd.flags = (code ? BBCF_RIGHTBUTTON : 0) | (GetKeyState(VK_SHIFT) & 0x8000 ? BBCF_SHIFTPRESSED : 0) | (GetKeyState(VK_CONTROL) & 0x8000 ? BBCF_CONTROLPRESSED : 0) | (bFromArrow ? BBCF_ARROWCLICKED : 0);

	NotifyEventHooks(hHookButtonPressedEvt, hContact, (LPARAM)&cbcd);
}

MIR_APP_DLL(void) Srmm_ResetToolbar()
{
	{	mir_cslock lck(csToolBar);
		wipeList(arButtonsList);
	}
	LastCID = MIN_CBUTTONID;
	dwSepCount = 0;
}

MIR_APP_DLL(void) Srmm_UpdateToolbarIcons(HWND hwndDlg)
{
	for (int i = 0; i < arButtonsList.getCount(); i++) {
		CustomButtonData *cbd = arButtonsList[i];
		if (cbd->m_bSeparator || cbd->m_hIcon == NULL)
			continue;

		HWND hwndBtn = GetDlgItem(hwndDlg, cbd->m_dwButtonCID);
		if (hwndBtn)
			SendMessage(hwndBtn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIconByHandle(cbd->m_hIcon));
	}
}

MIR_APP_DLL(void) Srmm_RedrawToolbarIcons(HWND hwndDlg)
{
	for (int i = 0; i < arButtonsList.getCount(); i++) {
		CustomButtonData *cbd = arButtonsList[i];
		HWND hwnd = GetDlgItem(hwndDlg, cbd->m_dwButtonCID);
		if (hwnd)
			InvalidateRect(hwnd, 0, TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static void CB_HardReInit()
{
	WindowList_Broadcast(chatApi.hWindowList, WM_CBD_REMOVED, 0, 0);

	Srmm_ResetToolbar();
	NotifyEventHooks(hHookToolBarLoadedEvt, 0, 0);
}

static void CB_ReInitCustomButtons()
{
	for (int i = arButtonsList.getCount() - 1; i >= 0; i--) {
		CustomButtonData *cbd = arButtonsList[i];

		if (cbd->m_opFlags & (BBSF_NTBSWAPED | BBSF_NTBDESTRUCT)) {
			cbd->m_opFlags ^= BBSF_NTBSWAPED;

			if (cbd->m_opFlags & BBSF_NTBDESTRUCT)
				arButtonsList.remove(i);
		}
	}
	qsort(arButtonsList.getArray(), arButtonsList.getCount(), sizeof(void*), sstSortButtons);

	WindowList_Broadcast(chatApi.hWindowList, WM_CBD_UPDATED, 0, 0);
	WindowList_Broadcast(chatApi.hWindowList, WM_CBD_LOADICONS, 0, 0);
}

static void CB_WriteButtonSettings(MCONTACT hContact, CustomButtonData *cbd)
{
	char SettingName[1024];
	char SettingParameter[1024];

	//modulename_buttonID, position_inIM_inCHAT_isLSide_isRSide_CanBeHidden

	mir_snprintf(SettingName, "%s_%d", cbd->m_pszModuleName, cbd->m_dwButtonOrigID);
	mir_snprintf(SettingParameter, "%d_%u_%u_%u_%u_%u", cbd->m_dwPosition, cbd->m_bIMButton, cbd->m_bChatButton, 0, cbd->m_bRSided, cbd->m_bCanBeHidden);
	if (!(cbd->m_opFlags & BBSF_NTBDESTRUCT))
		db_set_s(hContact, MODULENAME, SettingName, SettingParameter);
	else
		db_unset(hContact, MODULENAME, SettingName);
}

#define MIDDLE_SEPARATOR L">-------M-------<"

class CSrmmToolbarOptions : public CDlgBase
{
	CCtrlTreeView m_toolBar;
	CCtrlCheck m_btnIM, m_btnChat, m_btnHidden;
	CCtrlButton m_btnReset, m_btnSeparator;
	CCtrlSpin m_gap;

	HIMAGELIST m_hImgl;

	void SaveTree()
	{
		bool RSide = false;
		int count = 10;
		DWORD loc_sepcout = 0;
		wchar_t strbuf[128];

		TVITEMEX tvi;
		tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_HANDLE;
		tvi.hItem = m_toolBar.GetRoot();
		tvi.pszText = strbuf;
		tvi.cchTextMax = _countof(strbuf);
		{
			mir_cslock lck(csToolBar);

			while (tvi.hItem != NULL) {
				m_toolBar.GetItem(&tvi);

				if (mir_wstrcmp(tvi.pszText, MIDDLE_SEPARATOR) == 0) {
					RSide = true;
					count = m_toolBar.GetCount() * 10 - count;
					tvi.hItem = m_toolBar.GetNextSibling(tvi.hItem);
					continue;
				}
				CustomButtonData *cbd = (CustomButtonData*)tvi.lParam;
				if (cbd) {
					if (cbd->m_opFlags) {
						cbd->m_bIMButton = (cbd->m_opFlags & BBSF_IMBUTTON) != 0;
						cbd->m_bChatButton = (cbd->m_opFlags & BBSF_CHATBUTTON) != 0;
						cbd->m_bCanBeHidden = (cbd->m_opFlags & BBSF_CANBEHIDDEN) != 0;
					}
					if (RSide && !cbd->m_bRSided) {
						cbd->m_bRSided = true;
						cbd->m_opFlags |= BBSF_NTBSWAPED;
					}
					else if (!RSide && cbd->m_bRSided) {
						cbd->m_bRSided = false;
						cbd->m_opFlags |= BBSF_NTBSWAPED;
					}
					if (!m_toolBar.GetCheckState(tvi.hItem)) {
						cbd->m_bIMButton = false;
						cbd->m_bChatButton = false;

						if (cbd->m_bSeparator && !mir_strcmp(cbd->m_pszModuleName, "Tabsrmm_sep"))
							cbd->m_opFlags = BBSF_NTBDESTRUCT;
					}
					else {
						if (!cbd->m_bIMButton && !cbd->m_bChatButton)
							cbd->m_bIMButton = true;
						if (cbd->m_bSeparator && !mir_strcmp(cbd->m_pszModuleName, "Tabsrmm_sep")) {
							cbd->m_bHidden = 0;
							cbd->m_opFlags &= ~BBSF_NTBDESTRUCT;
							++loc_sepcout;
						}
					}

					cbd->m_dwPosition = (DWORD)count;
					CB_WriteButtonSettings(NULL, cbd);

					if (!(cbd->m_opFlags & BBSF_NTBDESTRUCT))
						(RSide) ? (count -= 10) : (count += 10);
				}

				HTREEITEM hItem = m_toolBar.GetNextSibling(tvi.hItem);
				if (cbd->m_opFlags & BBSF_NTBDESTRUCT)
					m_toolBar.DeleteItem(tvi.hItem);
				tvi.hItem = hItem;
			}

			qsort(arButtonsList.getArray(), arButtonsList.getCount(), sizeof(void*), sstSortButtons);
		}
		db_set_dw(0, MODULENAME, "SeparatorsCount", loc_sepcout);
		dwSepCount = loc_sepcout;
	}

	void BuildMenuObjectsTree()
	{
		HTREEITEM hti;
		int iImage = 0;

		TVINSERTSTRUCT tvis;
		tvis.hParent = NULL;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_IMAGE;

		m_toolBar.DeleteAllItems();

		m_hImgl = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 2, 2);
		ImageList_AddIcon(m_hImgl, Skin_LoadIcon(SKINICON_OTHER_SMALLDOT));
		ImageList_Destroy(m_toolBar.GetImageList(TVSIL_NORMAL));
		m_toolBar.SetImageList(m_hImgl, TVSIL_NORMAL);

		if (arButtonsList.getCount() == 0)
			return;

		bool bPrevSide = false;
		mir_cslock lck(csToolBar);

		for (int i = 0; i < arButtonsList.getCount(); i++) {
			CustomButtonData *cbd = arButtonsList[i];

			if (bPrevSide != cbd->m_bRSided) {
				bPrevSide = true;

				TVINSERTSTRUCT tvis2 = {};
				tvis.hInsertAfter = TVI_LAST;
				tvis2.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_IMAGE | TVIF_STATE;
				tvis2.item.pszText = MIDDLE_SEPARATOR;
				tvis2.item.stateMask = TVIS_BOLD;
				tvis2.item.state = TVIS_BOLD;
				tvis2.item.iImage = tvis.item.iSelectedImage = -1;
				tvis.hInsertAfter = hti = m_toolBar.InsertItem(&tvis2);
				m_toolBar.SetCheckState(hti, 1);
			}

			tvis.item.lParam = (LPARAM)cbd;

			if (cbd->m_bSeparator) {
				tvis.item.pszText = TranslateT("<Separator>");
				tvis.item.iImage = tvis.item.iSelectedImage = 0;
			}
			else {
				tvis.item.pszText = TranslateW(cbd->m_pwszTooltip);
				iImage = ImageList_AddIcon(m_hImgl, IcoLib_GetIconByHandle(cbd->m_hIcon));
				tvis.item.iImage = tvis.item.iSelectedImage = iImage;
			}
			cbd->m_opFlags = 0;
			hti = m_toolBar.InsertItem(&tvis);

			m_toolBar.SetCheckState(hti, (cbd->m_bIMButton || cbd->m_bChatButton));
		}
	}

public:
	CSrmmToolbarOptions() :
		CDlgBase(g_hInst, IDD_OPT_TOOLBAR),
		m_gap(this, IDC_SPIN1),
		m_btnIM(this, IDC_IMCHECK),
		m_btnChat(this, IDC_CHATCHECK),
		m_toolBar(this, IDC_TOOLBARTREE),
		m_btnReset(this, IDC_BBRESET),
		m_btnHidden(this, IDC_CANBEHIDDEN),
		m_btnSeparator(this, IDC_SEPARATOR),
		m_hImgl(NULL)
	{
		m_toolBar.SetFlags(MTREE_DND); // enable drag-n-drop
		m_toolBar.OnSelChanged = Callback(this, &CSrmmToolbarOptions::OnTreeSelChanged);
		m_toolBar.OnSelChanging = Callback(this, &CSrmmToolbarOptions::OnTreeSelChanging);

		m_btnReset.OnClick = Callback(this, &CSrmmToolbarOptions::btnResetClicked);
		m_btnSeparator.OnClick = Callback(this, &CSrmmToolbarOptions::btnSeparatorClicked);
	}
	
	virtual void OnInitDialog() override
	{
		BuildMenuObjectsTree();

		m_btnIM.Disable();
		m_btnChat.Disable();
		m_btnHidden.Disable();

		m_gap.SetRange(10);
		m_gap.SetPosition(db_get_b(NULL, MODULENAME, "ButtonsBarGap", 1));
	}

	virtual void OnDestroy() override
	{
		ImageList_Destroy(m_toolBar.GetImageList(TVSIL_NORMAL));
		ImageList_Destroy(m_toolBar.GetImageList(TVSIL_STATE));
	}

	virtual void OnApply() override
	{
		HTREEITEM hItem = m_toolBar.GetSelection();
		if (hItem) {
			TVITEMEX tvi;
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = hItem;
			m_toolBar.GetItem(&tvi);

			if (tvi.lParam) {
				CustomButtonData *cbd = (CustomButtonData*)tvi.lParam;
				if (cbd) {
					cbd->m_bIMButton = m_btnIM.Enabled() != 0;
					cbd->m_bChatButton = m_btnChat.Enabled() != 0;
					cbd->m_bCanBeHidden = m_btnHidden.Enabled() != 0;
				}
			}
		}

		SaveTree();
		CB_ReInitCustomButtons();

		BYTE newGap = (BYTE)SendDlgItemMessage(m_hwnd, IDC_SPIN1, UDM_GETPOS, 0, 0);
		if (newGap != db_get_b(NULL, MODULENAME, "ButtonsBarGap", 1))
			WindowList_BroadcastAsync(chatApi.hWindowList, WM_SIZE, 0, 0);

		db_set_b(0, MODULENAME, "ButtonsBarGap", newGap);

		BuildMenuObjectsTree();

		m_btnIM.Disable();
		m_btnChat.Disable();
		m_btnHidden.Disable();
	}

	virtual void OnReset() override
	{
		CB_ReInitCustomButtons();
		dwSepCount = db_get_dw(NULL, MODULENAME, "SeparatorsCount", 0);
	}

	void btnResetClicked(void*)
	{
		db_delete_module(NULL, MODULENAME);
		CB_HardReInit();
		BuildMenuObjectsTree();
	}

	void btnSeparatorClicked(void*)
	{
		HTREEITEM hItem = m_toolBar.GetSelection();
		if (!hItem)
			hItem = TVI_FIRST;

		CustomButtonData *cbd = new CustomButtonData();
		cbd->m_bSeparator = cbd->m_bHidden = cbd->m_bIMButton = true;
		cbd->m_dwButtonOrigID = ++dwSepCount;
		cbd->m_pszModuleName = "Tabsrmm_sep";
		cbd->m_iButtonWidth = 22;
		cbd->m_opFlags = BBSF_NTBDESTRUCT;
		arButtonsList.insert(cbd);

		TVINSERTSTRUCT tvis;
		tvis.hParent = NULL;
		tvis.hInsertAfter = hItem;
		tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

		tvis.item.pszText = TranslateT("<Separator>");
		tvis.item.iImage = tvis.item.iSelectedImage = -1;
		tvis.item.lParam = (LPARAM)cbd;
		hItem = m_toolBar.InsertItem(&tvis);

		m_toolBar.SetCheckState(hItem, (cbd->m_bIMButton || cbd->m_bChatButton));
	}

	void OnTreeSelChanging(void*)
	{
		HTREEITEM hItem = m_toolBar.GetSelection();
		if (hItem == NULL)
			return;

		wchar_t strbuf[128];
		TVITEMEX tvi;
		tvi.hItem = hItem;
		tvi.pszText = strbuf;
		tvi.cchTextMax = _countof(strbuf);
		tvi.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
		m_toolBar.GetItem(&tvi);

		if (tvi.lParam == 0 || !m_toolBar.GetCheckState(tvi.hItem) || !mir_wstrcmp(tvi.pszText, MIDDLE_SEPARATOR))
			return;

		CustomButtonData *cbd = (CustomButtonData*)tvi.lParam;
		cbd->m_bIMButton = m_btnIM.GetState() != 0;
		cbd->m_bChatButton = m_btnChat.GetState() != 0;
		cbd->m_bCanBeHidden = m_btnHidden.GetState() != 0;
		cbd->m_opFlags = (cbd->m_bIMButton ? BBSF_IMBUTTON : 0) + (cbd->m_bChatButton ? BBSF_CHATBUTTON : 0) + (cbd->m_bCanBeHidden ? BBSF_CANBEHIDDEN : 0);
	}

	void OnTreeSelChanged(void*)
	{
		HTREEITEM hItem = m_toolBar.GetSelection();
		if (hItem == NULL)
			return;

		wchar_t strbuf[128];
		TVITEMEX tvi;
		tvi.pszText = strbuf;
		tvi.cchTextMax = _countof(strbuf);
		tvi.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
		tvi.hItem = hItem;
		m_toolBar.GetItem(&tvi);

		if (!m_toolBar.GetCheckState(tvi.hItem) || !mir_wstrcmp(tvi.pszText, MIDDLE_SEPARATOR)) {
			m_btnIM.Disable();
			m_btnChat.Disable();
			m_btnHidden.Disable();
			return;
		}

		if (tvi.lParam == 0)
			return;

		CustomButtonData *cbd = (CustomButtonData*)tvi.lParam;
		m_btnIM.Enable(); m_btnIM.SetState(cbd->m_bIMButton);
		m_btnChat.Enable(); m_btnChat.SetState(cbd->m_bChatButton);
		m_btnHidden.Enable(); m_btnHidden.SetState(cbd->m_bCanBeHidden);
	}

	void OnTreeClicked(void*)
	{
		TVHITTESTINFO hti = { 0 };
		GetCursorPos(&hti.pt);
		ScreenToClient(m_toolBar.GetHwnd(), &hti.pt);
		if (m_toolBar.HitTest(&hti)) {
			if (hti.flags & TVHT_ONITEMSTATEICON) {
				NotifyChange();
				int iNewState = m_toolBar.GetCheckState(hti.hItem);
				m_btnIM.Enable(iNewState);
				m_btnChat.Enable(iNewState);
				m_btnHidden.Enable(iNewState);
				if (iNewState)
					m_btnIM.SetState(true);

				m_toolBar.SelectItem(hti.hItem);
			}
		}
	}
};

static int SrmmOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 910000000;
	odp.hInstance = g_hInst;
	odp.szGroup.a = LPGEN("Message sessions");
	odp.szTitle.a = LPGEN("Toolbar");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pDialog = new CSrmmToolbarOptions();
	Options_AddPage(wParam, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void KillModuleToolbarIcons(int _hLang)
{
	for (int i = arButtonsList.getCount() - 1; i >= 0; i--) {
		CustomButtonData *cbd = arButtonsList[i];
		if (cbd->m_hLangpack == _hLang) {
			arButtonsList.remove(i);
			delete cbd;
		}
	}
}

static int SrmmModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_OPT_INITIALISE, SrmmOptionsInit);

	NotifyEventHooks(hHookToolBarLoadedEvt, 0, 0);
	return 0;
}

static int ConvertToolbarData(const char *szSetting, LPARAM)
{
	DBVARIANT dbv;
	if (!db_get(NULL, "Tab" MODULENAME, szSetting, &dbv)) {
		db_set(NULL, MODULENAME, szSetting, &dbv);
		db_free(&dbv);
	}
	return 0;
}

void LoadSrmmToolbarModule()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, SrmmModulesLoaded);

	hHookButtonPressedEvt = CreateHookableEvent(ME_MSG_BUTTONPRESSED);
	hHookToolBarLoadedEvt = CreateHookableEvent(ME_MSG_TOOLBARLOADED);

	HDC hScrnDC = GetDC(0);
	g_DPIscaleX = GetDeviceCaps(hScrnDC, LOGPIXELSX) / 96.0;
	g_DPIscaleY = GetDeviceCaps(hScrnDC, LOGPIXELSY) / 96.0;
	ReleaseDC(0, hScrnDC);

	// old data? convert them
	if (db_get_dw(NULL, "Tab" MODULENAME, "SeparatorsCount", -1) != -1) {
		db_enum_settings(NULL, ConvertToolbarData, "Tab" MODULENAME, NULL);
		db_delete_module(NULL, "Tab" MODULENAME);
	}

	dwSepCount = db_get_dw(NULL, MODULENAME, "SeparatorsCount", 0);
	CB_RegisterSeparators();
}

void UnloadSrmmToolbarModule()
{
	DestroyHookableEvent(hHookButtonPressedEvt);
	DestroyHookableEvent(hHookToolBarLoadedEvt);

	wipeList(arButtonsList);
}
