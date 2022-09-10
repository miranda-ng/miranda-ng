/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team,
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
#include "chat.h"
#include "skin.h"

#define BB_MODULE_NAME "SRMM_Toolbar"

#define DPISCALEY_S(argY) ((int)((double)(argY) * g_DPIscaleY))
#define DPISCALEX_S(argX) ((int)((double)(argX) * g_DPIscaleX))

static double g_DPIscaleX, g_DPIscaleY;
static class CSrmmToolbarOptions *g_pDialog = nullptr;

static CMOption<uint8_t> g_iButtonGap(BB_MODULE_NAME, "ButtonsBarGap", 1);

static int SortButtons(const CustomButtonData *p1, const CustomButtonData *p2)
{
	if (p1->m_bRSided != p2->m_bRSided)
		return (p2->m_bRSided) ? -1 : 1;
	if (p1->m_dwPosition != p2->m_dwPosition)
		return p1->m_dwPosition - p2->m_dwPosition;
	int res = mir_strcmp(p1->m_pszModuleName, p2->m_pszModuleName);
	if (res != 0)
		return res;
	return p1->m_dwButtonID - p2->m_dwButtonID;
}

static LIST<CustomButtonData> arButtonsList(1, SortButtons);

int LastCID = MIN_CBUTTONID;
int dwSepCount = 0;

static mir_cs csToolBar;
static HANDLE hHookToolBarLoadedEvt, hHookButtonPressedEvt;

static int sstSortButtons(const void *p1, const void *p2)
{
	return SortButtons(*(CustomButtonData**)p1, *(CustomButtonData**)p2);
}

static void CB_RegisterSeparators()
{
	BBButton bbd = {};
	bbd.pszModuleName = "Tabsrmm_sep";
	for (int i = 0; dwSepCount > i; i++) {
		bbd.bbbFlags = BBBF_ISSEPARATOR | BBBF_ISIMBUTTON;
		bbd.dwButtonID = i + 1;
		bbd.dwDefPos = 410 + i;
		Srmm_AddButton(&bbd, &g_plugin);
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

MIR_APP_DLL(int) Srmm_GetButtonGap()
{
	return g_iButtonGap;
}

MIR_APP_DLL(int) Srmm_GetButtonState(HWND hwndDlg, BBButton *bbdi)
{
	if (hwndDlg == nullptr || bbdi == nullptr)
		return 1;

	uint32_t tempCID = 0;
	bbdi->bbbFlags = 0;
	for (auto &cbd : arButtonsList)
		if (!mir_strcmp(cbd->m_pszModuleName, bbdi->pszModuleName) && (cbd->m_dwButtonID == bbdi->dwButtonID)) {
			tempCID = cbd->m_dwButtonCID;
			break;
		}

	if (!tempCID)
		return 1;

	HWND hwndBtn = GetDlgItem(hwndDlg, tempCID);
	bbdi->bbbFlags = (IsDlgButtonChecked(hwndDlg, tempCID) ? BBSF_PUSHED : BBSF_RELEASED) | (IsWindowVisible(hwndBtn) ? 0 : BBSF_HIDDEN) | (IsWindowEnabled(hwndBtn) ? 0 : BBSF_DISABLED);
	return 0;
}

MIR_APP_DLL(int) Srmm_SetButtonState(MCONTACT hContact, BBButton *bbdi)
{
	if (hContact == 0 || bbdi == nullptr)
		return 1;

	uint32_t tempCID = 0;
	for (auto &cbd : arButtonsList)
		if (!mir_strcmp(cbd->m_pszModuleName, bbdi->pszModuleName) && (cbd->m_dwButtonID == bbdi->dwButtonID)) {
			tempCID = cbd->m_dwButtonCID;
			break;
		}

	if (!tempCID)
		return 1;

	HWND hwndDlg = WindowList_Find(g_hWindowList, hContact);
	if (hwndDlg == nullptr)
		return 1;

	HWND hwndBtn = GetDlgItem(hwndDlg, tempCID);
	if (hwndBtn == nullptr)
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
		Button_SetCheck(hwndBtn, (bbdi->bbbFlags & BBSF_RELEASED) == 0);
	}
	return 0;
}

MIR_APP_DLL(int) Srmm_ModifyButton(BBButton *bbdi)
{
	if (!bbdi)
		return 1;

	CustomButtonData *cbd = nullptr;
	{
		mir_cslock lck(csToolBar);

		for (auto &p : arButtonsList)
			if (!mir_strcmp(p->m_pszModuleName, bbdi->pszModuleName) && (p->m_dwButtonID == bbdi->dwButtonID)) {
				cbd = p;
				break;
			}

		if (cbd != nullptr) {
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

	if (cbd != nullptr)
		WindowList_Broadcast(g_hWindowList, WM_CBD_UPDATED, 0, (LPARAM)cbd);
	return 0;
}

MIR_APP_DLL(void) Srmm_ClickToolbarIcon(MCONTACT hContact, int idFrom, HWND hwndDlg, BOOL code)
{
	bool bFromArrow = false;
	HWND hwndFrom = nullptr;

	CustomButtonClickData cbcd = {};

	for (auto &cbd : arButtonsList) {
		if	(cbd->m_dwButtonCID == idFrom) {
			cbcd.pszModule = cbd->m_pszModuleName;
			cbcd.dwButtonId = cbd->m_dwButtonID;
			hwndFrom = GetDlgItem(hwndDlg, idFrom);
		}
		else if (cbd->m_dwArrowCID == idFrom) {
			bFromArrow = true;
			cbcd.pszModule = cbd->m_pszModuleName;
			cbcd.dwButtonId = cbd->m_dwButtonID;
			hwndFrom = GetDlgItem(hwndDlg, idFrom-1);
		}
	}

	if (hwndFrom == nullptr)
		return;

	RECT rc;
	GetWindowRect(hwndFrom, &rc);
	cbcd.pt.x = rc.left;
	cbcd.pt.y = rc.bottom;

	cbcd.hwndFrom = GetParent(hwndFrom);
	cbcd.hContact = hContact;
	cbcd.flags = (code ? BBCF_RIGHTBUTTON : 0) | (GetKeyState(VK_SHIFT) & 0x8000 ? BBCF_SHIFTPRESSED : 0) | (GetKeyState(VK_CONTROL) & 0x8000 ? BBCF_CONTROLPRESSED : 0) | (bFromArrow ? BBCF_ARROWCLICKED : 0);

	NotifyEventHooks(hHookButtonPressedEvt, hContact, (LPARAM)&cbcd);
}

void Srmm_ProcessToolbarHotkey(MCONTACT hContact, INT_PTR iButtonFrom, HWND hwndDlg)
{
	HWND hwndFrom = nullptr;

	CustomButtonClickData cbcd = {};

	for (auto &cbd : arButtonsList) {
		if (cbd->m_hotkey == nullptr || cbd->m_bDisabled)
			continue;

		if (cbd->m_hotkey->lParam == iButtonFrom) {
			cbcd.pszModule = cbd->m_pszModuleName;
			cbcd.dwButtonId = cbd->m_dwButtonID;
			hwndFrom = GetDlgItem(hwndDlg, cbd->m_dwButtonCID);
			break;
		}
	}

	if (hwndFrom == nullptr)
		return;

	RECT rc;
	GetWindowRect(hwndFrom, &rc);
	cbcd.pt.x = rc.left;
	cbcd.pt.y = rc.bottom;

	cbcd.hwndFrom = GetParent(hwndFrom);
	cbcd.hContact = hContact;
	cbcd.flags = (GetKeyState(VK_SHIFT) & 0x8000 ? BBCF_SHIFTPRESSED : 0) | (GetKeyState(VK_CONTROL) & 0x8000 ? BBCF_CONTROLPRESSED : 0);

	NotifyEventHooks(hHookButtonPressedEvt, hContact, (LPARAM)&cbcd);
}

MIR_APP_DLL(void) Srmm_ResetToolbar()
{
	for (auto &cbd : arButtonsList) {
		cbd->m_dwPosition = cbd->m_dwOrigPosition;
		cbd->m_bRSided = cbd->m_dwOrigFlags.bit1;
		cbd->m_bIMButton = cbd->m_dwOrigFlags.bit2;
		cbd->m_bChatButton = cbd->m_dwOrigFlags.bit3;
		cbd->m_bCanBeHidden = cbd->m_dwOrigFlags.bit4;
	}
}

void Srmm_CreateToolbarIcons(HWND hwndDlg, int flags)
{
	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwndDlg, GWLP_HINSTANCE);

	CDlgBase *pDlg = CDlgBase::Find(hwndDlg);

	for (auto &cbd : arButtonsList) {
		if (cbd->m_bSeparator)
			continue;

		HWND hwndButton = GetDlgItem(hwndDlg, cbd->m_dwButtonCID);
		if ((flags & BBBF_ISIMBUTTON) && cbd->m_bIMButton || (flags & BBBF_ISCHATBUTTON) && cbd->m_bChatButton) {
			if (hwndButton == nullptr) {
				hwndButton = CreateWindowEx(0, L"MButtonClass", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, cbd->m_iButtonWidth, DPISCALEX_S(22), hwndDlg, (HMENU)cbd->m_dwButtonCID, hInstance, nullptr);
				if (hwndButton == nullptr) // smth went wrong
					continue;

				// if there's a pre-created button control in a class, initialize it
				if (pDlg != nullptr) {
					CCtrlBase *pControl = (*pDlg)[cbd->m_dwButtonCID];
					if (pControl)
						pControl->OnInit();
				}
			}
			SendMessage(hwndButton, BUTTONSETASFLATBTN, TRUE, 0);
			if (cbd->m_pwszText)
				SetWindowTextW(hwndButton, cbd->m_pwszText);
			if (cbd->m_pwszTooltip)
				SendMessage(hwndButton, BUTTONADDTOOLTIP, LPARAM(cbd->m_pwszTooltip), BATF_UNICODE);
			if (cbd->m_hIcon)
				SendMessage(hwndButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIconByHandle(cbd->m_hIcon));

			if (cbd->m_dwArrowCID)
				SendMessage(hwndButton, BUTTONSETARROW, cbd->m_dwArrowCID, 0);
			if (cbd->m_bPushButton)
				SendMessage(hwndButton, BUTTONSETASPUSHBTN, TRUE, 0);

			if (cbd->m_bDisabled)
				EnableWindow(hwndButton, FALSE);
			if (cbd->m_bHidden)
				ShowWindow(hwndButton, SW_HIDE);
		}
		else if (hwndButton)
			DestroyWindow(hwndButton);
	}
}

MIR_APP_DLL(void) Srmm_UpdateToolbarIcons(HWND hwndDlg)
{
	for (auto &cbd : arButtonsList) {
		if (cbd->m_bSeparator || cbd->m_hIcon == nullptr)
			continue;

		HWND hwndBtn = GetDlgItem(hwndDlg, cbd->m_dwButtonCID);
		if (hwndBtn)
			SendMessage(hwndBtn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIconByHandle(cbd->m_hIcon));
	}
}

MIR_APP_DLL(void) Srmm_RedrawToolbarIcons(HWND hwndDlg)
{
	for (auto &cbd : arButtonsList) {
		HWND hwnd = GetDlgItem(hwndDlg, cbd->m_dwButtonCID);
		if (hwnd)
			InvalidateRect(hwnd, nullptr, TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static void CB_ReInitCustomButtons()
{
	for (auto &cbd : arButtonsList.rev_iter())
		if (cbd->m_opFlags & (BBSF_NTBSWAPED | BBSF_NTBDESTRUCT)) {
			cbd->m_opFlags ^= BBSF_NTBSWAPED;

			if (cbd->m_opFlags & BBSF_NTBDESTRUCT)
				arButtonsList.removeItem(&cbd);
		}

	qsort(arButtonsList.getArray(), arButtonsList.getCount(), sizeof(void*), sstSortButtons);

	WindowList_Broadcast(g_hWindowList, WM_CBD_RECREATE, 0, 0);
	WindowList_Broadcast(g_hWindowList, WM_CBD_UPDATED, 0, 0);
	WindowList_Broadcast(g_hWindowList, WM_CBD_LOADICONS, 0, 0);
}

static void CB_WriteButtonSettings(MCONTACT hContact, CustomButtonData *cbd)
{
	char SettingName[1024];
	char SettingParameter[1024];

	//modulename_buttonID, position_inIM_inCHAT_isLSide_isRSide_CanBeHidden

	mir_snprintf(SettingName, "%s_%d", cbd->m_pszModuleName.get(), cbd->m_dwButtonID);
	mir_snprintf(SettingParameter, "%d_%u_%u_%u_%u_%u", cbd->m_dwPosition, cbd->m_bIMButton, cbd->m_bChatButton, 0, cbd->m_bRSided, cbd->m_bCanBeHidden);
	if (!(cbd->m_opFlags & BBSF_NTBDESTRUCT))
		db_set_s(hContact, BB_MODULE_NAME, SettingName, SettingParameter);
	else
		db_unset(hContact, BB_MODULE_NAME, SettingName);
}

#define MIDDLE_SEPARATOR L">-------M-------<"

class CSrmmToolbarOptions : public CDlgBase
{
	CCtrlTreeView m_toolBar;
	CCtrlCheck m_btnIM, m_btnChat, m_btnHidden;
	CCtrlButton m_btnReset, m_btnSeparator;
	CCtrlSpin m_gap;
	CTimer timer;

	HIMAGELIST m_hImgl;

	void SaveTree()
	{
		bool RSide = false;
		int count = 10;
		uint32_t loc_sepcout = 0;
		wchar_t strbuf[128];

		TVITEMEX tvi;
		tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_HANDLE;
		tvi.hItem = m_toolBar.GetRoot();
		tvi.pszText = strbuf;
		tvi.cchTextMax = _countof(strbuf);
		{
			mir_cslock lck(csToolBar);

			while (tvi.hItem != nullptr) {
				m_toolBar.GetItem(&tvi);

				if (mir_wstrcmp(tvi.pszText, MIDDLE_SEPARATOR) == 0) {
					RSide = true;
					count = m_toolBar.GetCount() * 10 - count;
					tvi.hItem = m_toolBar.GetNextSibling(tvi.hItem);
					continue;
				}
				CustomButtonData *cbd = (CustomButtonData*)tvi.lParam;
				if (cbd && arButtonsList.indexOf(cbd) != -1) {
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
					
					if (!cbd->m_bCantBeHidden && !m_toolBar.GetCheckState(tvi.hItem)) {
						cbd->m_bIMButton = false;
						cbd->m_bChatButton = false;

						if (cbd->m_bSeparator && !mir_strcmp(cbd->m_pszModuleName, "Tabsrmm_sep"))
							cbd->m_opFlags = BBSF_NTBDESTRUCT;
					}
					else {
						if (!cbd->m_bIMButton && !cbd->m_bChatButton)
							cbd->m_bIMButton = true;
						if (cbd->m_bSeparator && !mir_strcmp(cbd->m_pszModuleName, "Tabsrmm_sep")) {
							cbd->m_bHidden = false;
							cbd->m_opFlags &= ~BBSF_NTBDESTRUCT;
							++loc_sepcout;
						}
					}

					cbd->m_dwPosition = (uint32_t)count;
					CB_WriteButtonSettings(0, cbd);

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
		db_set_dw(0, BB_MODULE_NAME, "SeparatorsCount", loc_sepcout);
		dwSepCount = loc_sepcout;
	}

	void BuildMenuObjectsTree()
	{
		m_toolBar.DeleteAllItems();

		m_hImgl = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 2, 2);
		ImageList_AddIcon(m_hImgl, Skin_LoadIcon(SKINICON_OTHER_SMALLDOT));
		ImageList_Destroy(m_toolBar.GetImageList(TVSIL_NORMAL));
		m_toolBar.SetImageList(m_hImgl, TVSIL_NORMAL);

		if (arButtonsList.getCount() == 0)
			return;

		bool bPrevSide = false;

		TVINSERTSTRUCT tvis;
		tvis.hParent = nullptr;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_IMAGE;

		mir_cslock lck(csToolBar);
		for (auto &cbd : arButtonsList) {
			if (bPrevSide != cbd->m_bRSided) {
				bPrevSide = true;

				TVINSERTSTRUCT tvis2 = {};
				tvis.hInsertAfter = TVI_LAST;
				tvis2.itemex.mask = TVIF_PARAM | TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_IMAGE | TVIF_STATE | TVIF_STATEEX;
				tvis2.itemex.pszText = MIDDLE_SEPARATOR;
				tvis2.itemex.stateMask = TVIS_BOLD;
				tvis2.itemex.state = TVIS_BOLD;
				tvis2.itemex.iImage = tvis.item.iSelectedImage = -1;
				tvis2.itemex.uStateEx = TVIS_EX_DISABLED;
				tvis.hInsertAfter = m_toolBar.InsertItem(&tvis2);
				m_toolBar.SetItemState(tvis.hInsertAfter, 0x3000, TVIS_STATEIMAGEMASK);
			}

			tvis.item.lParam = (LPARAM)cbd;

			if (cbd->m_bSeparator) {
				tvis.item.pszText = TranslateT("<Separator>");
				tvis.item.iImage = tvis.item.iSelectedImage = 0;
			}
			else {
				tvis.item.pszText = TranslateW(cbd->m_pwszTooltip);
				tvis.item.iImage = tvis.item.iSelectedImage = ImageList_AddIcon(m_hImgl, IcoLib_GetIconByHandle(cbd->m_hIcon));
			}
			cbd->m_opFlags = 0;
			HTREEITEM hti = m_toolBar.InsertItem(&tvis);

			m_toolBar.SetCheckState(hti, (cbd->m_bIMButton || cbd->m_bChatButton));
			if (cbd->m_bCantBeHidden)
				m_toolBar.SetItemState(hti, 0x3000, TVIS_STATEIMAGEMASK);
		}
	}

public:
	CSrmmToolbarOptions() :
		CDlgBase(g_plugin, IDD_OPT_TOOLBAR),
		m_gap(this, IDC_SPIN1, 10),
		m_btnIM(this, IDC_IMCHECK),
		m_btnChat(this, IDC_CHATCHECK),
		m_toolBar(this, IDC_TOOLBARTREE),
		m_btnReset(this, IDC_BBRESET),
		m_btnHidden(this, IDC_CANBEHIDDEN),
		m_btnSeparator(this, IDC_SEPARATOR),
		m_hImgl(nullptr),
		timer(this, 1)
	{
		timer.OnEvent = Callback(this, &CSrmmToolbarOptions::OnTimer);

		m_toolBar.SetFlags(MTREE_DND); // enable drag-n-drop
		m_toolBar.OnSelChanged = Callback(this, &CSrmmToolbarOptions::OnTreeSelChanged);
		m_toolBar.OnSelChanging = Callback(this, &CSrmmToolbarOptions::OnTreeSelChanging);
		m_toolBar.OnItemChanged = Callback(this, &CSrmmToolbarOptions::OnTreeItemChanged);

		m_btnReset.OnClick = Callback(this, &CSrmmToolbarOptions::btnResetClicked);
		m_btnSeparator.OnClick = Callback(this, &CSrmmToolbarOptions::btnSeparatorClicked);
	}
	
	bool OnInitDialog() override
	{
		g_pDialog = this;
		BuildMenuObjectsTree();

		m_btnIM.Disable();
		m_btnChat.Disable();
		m_btnHidden.Disable();

		m_gap.SetPosition(g_iButtonGap);
		return true;
	}

	void OnDestroy() override
	{
		g_pDialog = nullptr;
		ImageList_Destroy(m_toolBar.GetImageList(TVSIL_NORMAL));
		ImageList_Destroy(m_toolBar.GetImageList(TVSIL_STATE));
	}

	bool OnApply() override
	{
		OnTreeSelChanging(nullptr);  // save latest changes
		SaveTree();                  // save the whole tree then
		CB_ReInitCustomButtons();
		Chat_UpdateOptions();        // also restore chat windows

		uint16_t newGap = m_gap.GetPosition();
		if (newGap != g_iButtonGap) {
			g_iButtonGap = newGap;
			WindowList_BroadcastAsync(g_hWindowList, WM_SIZE, 0, 0);
		}

		BuildMenuObjectsTree();

		m_btnIM.Disable();
		m_btnChat.Disable();
		m_btnHidden.Disable();
		return true;
	}

	virtual void OnReset() override
	{
		CB_ReInitCustomButtons();
		dwSepCount = db_get_dw(0, BB_MODULE_NAME, "SeparatorsCount", 0);
	}

	void btnResetClicked(void*)
	{
		db_delete_module(0, BB_MODULE_NAME);

		Srmm_ResetToolbar();
		qsort(arButtonsList.getArray(), arButtonsList.getCount(), sizeof(void*), sstSortButtons);

		BuildMenuObjectsTree();
		NotifyChange();
	}

	void btnSeparatorClicked(void*)
	{
		HTREEITEM hItem = m_toolBar.GetSelection();
		if (!hItem)
			hItem = TVI_FIRST;

		BBButton bbd = {};
		bbd.pszModuleName = "Tabsrmm_sep";
		bbd.bbbFlags = BBBF_ISSEPARATOR | BBBF_ISIMBUTTON;
		bbd.dwButtonID = ++dwSepCount;

		CustomButtonData *cbd = (CustomButtonData*)Srmm_AddButton(&bbd, &g_plugin);

		TVINSERTSTRUCT tvis;
		tvis.hParent = nullptr;
		tvis.hInsertAfter = hItem;
		tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvis.item.pszText = TranslateT("<Separator>");
		tvis.item.iImage = tvis.item.iSelectedImage = -1;
		tvis.item.lParam = (LPARAM)cbd;
		hItem = m_toolBar.InsertItem(&tvis);

		m_toolBar.SetCheckState(hItem, (cbd->m_bIMButton || cbd->m_bChatButton));
		NotifyChange();
	}

	void OnTreeSelChanging(void*)
	{
		HTREEITEM hItem = m_toolBar.GetSelection();
		if (hItem == nullptr)
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
		cbd->m_bCanBeHidden = !cbd->m_bCantBeHidden && m_btnHidden.GetState() != 0;
		cbd->m_opFlags = (cbd->m_bIMButton ? BBSF_IMBUTTON : 0) + (cbd->m_bChatButton ? BBSF_CHATBUTTON : 0) + (cbd->m_bCanBeHidden ? BBSF_CANBEHIDDEN : 0);

		if (!cbd->m_bChatButton && !cbd->m_bIMButton)
			m_toolBar.SetCheckState(tvi.hItem, 0);
	}

	void OnTreeSelChanged(void*)
	{
		HTREEITEM hItem = m_toolBar.GetSelection();
		if (hItem == nullptr)
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

	void OnTreeItemChanged(CCtrlTreeView::TEventInfo *evt)
	{
		bool iNewState = !m_toolBar.GetCheckState(evt->hItem);
		m_btnIM.Enable(iNewState);
		m_btnChat.Enable(iNewState);
		m_btnHidden.Enable(iNewState);
		if (iNewState)
			m_btnIM.SetState(true);
	}

	void OnTimer(CTimer *pTimer)
	{
		pTimer->Stop();
		BuildMenuObjectsTree();
	}

	static void RereadButtons()
	{
		if (g_pDialog)
			g_pDialog->timer.Start(100);
	}
};

void SrmmLogOptionsInit(WPARAM wParam);

static int SrmmOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 910000000;
	odp.szGroup.a = LPGEN("Message sessions");
	odp.szTitle.a = LPGEN("Toolbar");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pDialog = new CSrmmToolbarOptions();
	g_plugin.addOptions(wParam, &odp);

	ChatOptionsInit(wParam);
	SrmmLogOptionsInit(wParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(HANDLE) Srmm_AddButton(const BBButton *bbdi, HPLUGIN _hLang)
{
	if (bbdi == nullptr)
		return nullptr;

	CustomButtonData *cbd = new CustomButtonData();
	cbd->m_pszModuleName = mir_strdup(bbdi->pszModuleName);
	cbd->m_pwszText = mir_wstrdup(bbdi->pwszText);
	cbd->m_pwszTooltip = mir_wstrdup(bbdi->pwszTooltip);

	cbd->m_dwButtonID = bbdi->dwButtonID;
	cbd->m_hIcon = bbdi->hIcon;
	cbd->m_dwPosition = cbd->m_dwOrigPosition = bbdi->dwDefPos;
	cbd->m_dwButtonCID = (bbdi->bbbFlags & BBBF_CREATEBYID) ? bbdi->dwButtonID : LastCID;
	cbd->m_dwArrowCID = (bbdi->bbbFlags & BBBF_ISARROWBUTTON) ? cbd->m_dwButtonCID + 1 : 0;
	cbd->m_bHidden = (bbdi->bbbFlags & BBBF_HIDDEN) != 0;
	cbd->m_bSeparator = (bbdi->bbbFlags & BBBF_ISSEPARATOR) != 0;

	cbd->m_bDisabled = (bbdi->bbbFlags & BBBF_DISABLED) != 0;
	cbd->m_bPushButton = (bbdi->bbbFlags & BBBF_ISPUSHBUTTON) != 0;
	cbd->m_pPlugin = _hLang;

	cbd->m_dwOrigFlags.bit1 = cbd->m_bRSided = (bbdi->bbbFlags & BBBF_ISRSIDEBUTTON) != 0;
	cbd->m_dwOrigFlags.bit2 = cbd->m_bIMButton = (bbdi->bbbFlags & BBBF_ISIMBUTTON) != 0;
	cbd->m_dwOrigFlags.bit3 = cbd->m_bChatButton = (bbdi->bbbFlags & BBBF_ISCHATBUTTON) != 0;
	cbd->m_dwOrigFlags.bit4 = cbd->m_bCanBeHidden = (bbdi->bbbFlags & BBBF_CANBEHIDDEN) != 0;

	if (cbd->m_bSeparator)
		cbd->m_iButtonWidth = DPISCALEX_S(10);
	else if (bbdi->bbbFlags & BBBF_ISARROWBUTTON)
		cbd->m_iButtonWidth = DPISCALEX_S(34);
	else
		cbd->m_iButtonWidth = DPISCALEX_S(22);

	if (bbdi->pszHotkey) {
		for (auto &p : hotkeys) {
			if (!mir_strcmp(p->getName(), bbdi->pszHotkey)) {
				cbd->m_hotkey = p;
				break;
			}
		}
	}

	// download database settings
	char SettingName[1024];
	mir_snprintf(SettingName, "%s_%d", cbd->m_pszModuleName.get(), cbd->m_dwButtonID);

	DBVARIANT dbv = { 0 };
	if (!db_get_s(0, BB_MODULE_NAME, SettingName, &dbv)) {
		// modulename_buttonID, position_inIM_inCHAT_isLSide_isRSide_CanBeHidden
		char *token = strtok(dbv.pszVal, "_");
		cbd->m_dwPosition = (uint32_t)atoi(token);
		token = strtok(nullptr, "_");
		cbd->m_bIMButton = atoi(token) != 0;
		token = strtok(nullptr, "_");
		cbd->m_bChatButton = atoi(token) != 0;
		token = strtok(nullptr, "_");
		token = strtok(nullptr, "_");
		cbd->m_bRSided = atoi(token) != 0;
		token = strtok(nullptr, "_");
		cbd->m_bCanBeHidden = atoi(token) != 0;

		db_free(&dbv);
	}

	arButtonsList.insert(cbd);

	if (cbd->m_dwButtonCID != cbd->m_dwButtonID)
		LastCID++;
	if (cbd->m_dwArrowCID == LastCID)
		LastCID++;

	WindowList_Broadcast(g_hWindowList, WM_CBD_UPDATED, 0, 0);
	CSrmmToolbarOptions::RereadButtons();
	return cbd;
}

MIR_APP_DLL(int) Srmm_RemoveButton(BBButton *bbdi)
{
	if (!bbdi)
		return 1;

	CustomButtonData *pFound = nullptr;
	{
		mir_cslock lck(csToolBar);

		for (auto &cbd : arButtonsList.rev_iter())
			if (!mir_strcmp(cbd->m_pszModuleName, bbdi->pszModuleName) && cbd->m_dwButtonID == bbdi->dwButtonID)
				pFound = arButtonsList.removeItem(&cbd);
	}

	if (pFound) {
		CSrmmToolbarOptions::RereadButtons();
		WindowList_Broadcast(g_hWindowList, WM_CBD_REMOVED, pFound->m_dwButtonCID, (LPARAM)pFound);
		delete pFound;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void KillModuleToolbarIcons(CMPluginBase *pPlugin)
{
	int oldCount = arButtonsList.getCount();

	auto T = arButtonsList.rev_iter();
	for (auto &cbd : T)
		if (cbd->m_pPlugin == pPlugin)
			delete arButtonsList.removeItem(&cbd);

	if (oldCount != arButtonsList.getCount())
		CSrmmToolbarOptions::RereadButtons();
}

static INT_PTR BroadcastMessage(WPARAM, LPARAM lParam)
{
	Srmm_Broadcast((UINT)lParam, 0, 0);
	return 0;
}

static void CALLBACK SrmmLoadToolbar()
{
	NotifyEventHooks(hHookToolBarLoadedEvt, 0, 0);
	DestroyHookableEvent(hHookToolBarLoadedEvt);

	HookEvent(ME_OPT_INITIALISE, SrmmOptionsInit);
}

static int ConvertToolbarData(const char *szSetting, void*)
{
	DBVARIANT dbv;
	if (!db_get(0, "Tab" BB_MODULE_NAME, szSetting, &dbv)) {
		db_set(0, BB_MODULE_NAME, szSetting, &dbv);
		db_free(&dbv);
	}
	return 0;
}

void LoadSrmmToolbarModule()
{
	CreateServiceFunction("SRMsg/BroadcastMessage", BroadcastMessage);

	Miranda_WaitOnHandle(SrmmLoadToolbar);

	hHookButtonPressedEvt = CreateHookableEvent(ME_MSG_BUTTONPRESSED);
	hHookToolBarLoadedEvt = CreateHookableEvent(ME_MSG_TOOLBARLOADED);

	HDC hScrnDC = GetDC(nullptr);
	g_DPIscaleX = GetDeviceCaps(hScrnDC, LOGPIXELSX) / 96.0;
	g_DPIscaleY = GetDeviceCaps(hScrnDC, LOGPIXELSY) / 96.0;
	ReleaseDC(nullptr, hScrnDC);

	// old data? convert them
	if (db_get_dw(0, "Tab" BB_MODULE_NAME, "SeparatorsCount", -1) != -1) {
		db_enum_settings(0, ConvertToolbarData, "Tab" BB_MODULE_NAME, nullptr);
		db_delete_module(0, "Tab" BB_MODULE_NAME);
	}

	dwSepCount = db_get_dw(0, BB_MODULE_NAME, "SeparatorsCount", 0);
	CB_RegisterSeparators();
}

void UnloadSrmmToolbarModule()
{
	DestroyHookableEvent(hHookButtonPressedEvt);

	for (auto &it : arButtonsList)
		delete it;
	arButtonsList.destroy();
}
