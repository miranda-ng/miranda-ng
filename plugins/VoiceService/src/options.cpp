/*
Copyright (C) 2006 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/


#include "stdafx.h"
#include "options.h"

#include "../utils/mir_options.h"

// Prototypes /////////////////////////////////////////////////////////////////////////////////////

HANDLE hOptHook = NULL;

Options opts;

/////////////////////////////////////////////////////////////////////////////////////////
// Popup options

static OptPageControl popupsControls[] = {
	{ &opts.popup_enable,             CONTROL_CHECKBOX,  IDC_POPUPS,         "PopupsEnable", FALSE },
	{ &opts.popup_bkg_color,          CONTROL_COLOR,     IDC_BGCOLOR,        "PopupsBgColor", RGB(255,255,255) },
	{ &opts.popup_text_color,         CONTROL_COLOR,     IDC_TEXTCOLOR,      "PopupsTextColor", RGB(0,0,0) },
	{ &opts.popup_use_win_colors,     CONTROL_CHECKBOX,  IDC_WINCOLORS,      "PopupsWinColors", FALSE },
	{ &opts.popup_use_default_colors, CONTROL_CHECKBOX,  IDC_DEFAULTCOLORS,  "PopupsDefaultColors", FALSE },
	{ &opts.popup_delay_type,         CONTROL_RADIO,     IDC_DELAYFROMPU,    "PopupsDelayType", POPUP_DELAY_DEFAULT, POPUP_DELAY_DEFAULT },
	{ NULL,                           CONTROL_RADIO,     IDC_DELAYCUSTOM,    "PopupsDelayType", POPUP_DELAY_DEFAULT, POPUP_DELAY_CUSTOM },
	{ NULL,                           CONTROL_RADIO,     IDC_DELAYPERMANENT, "PopupsDelayType", POPUP_DELAY_DEFAULT, POPUP_DELAY_PERMANENT },
	{ &opts.popup_timeout,            CONTROL_SPIN,      IDC_DELAY,          "PopupsTimeout", 10, IDC_DELAY_SPIN, (WORD)1, (WORD)255 },
	{ &opts.popup_right_click_action, CONTROL_COMBO,     IDC_RIGHT_ACTION,   "PopupsRightClick", POPUP_ACTION_CLOSEPOPUP },
	{ &opts.popup_left_click_action,  CONTROL_COMBO,     IDC_LEFT_ACTION,    "PopupsLeftClick", POPUP_ACTION_CLOSEPOPUP },
	
	{ &opts.opt_bImmDialog, CONTROL_CHECKBOX,     IDC_IMMDIALOG,   "PopupsRightClick", FALSE },
	{ &opts.opt_bImmDialogFocus,  CONTROL_CHECKBOX,     IDC_IMMDIALOG_FOCUS,    "PopupsLeftClick", FALSE }
};

static void PopupsEnableDisableCtrls(HWND hwndDlg)
{
	BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_POPUPS);

	EnableWindow(GetDlgItem(hwndDlg, IDC_COLOURS_G), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTCOLOR_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_G), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAYFROMPU), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAYCUSTOM), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAYPERMANENT), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ACTIONS_G), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHT_ACTION_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHT_ACTION), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LEFT_ACTION_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LEFT_ACTION), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_PREV), enabled);

	EnableWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR), enabled &&
		!IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS) &&
		!IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS));
	EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTCOLOR), enabled &&
		!IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS) &&
		!IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS));
	EnableWindow(GetDlgItem(hwndDlg, IDC_DEFAULTCOLORS), enabled && !IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS));
	EnableWindow(GetDlgItem(hwndDlg, IDC_WINCOLORS), enabled && !IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS));

	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY), enabled && IsDlgButtonChecked(hwndDlg, IDC_DELAYCUSTOM));
}

static INT_PTR CALLBACK PopupsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		SendDlgItemMessage(hwndDlg, IDC_RIGHT_ACTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("Do nothing"));
		SendDlgItemMessage(hwndDlg, IDC_RIGHT_ACTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("Close popup"));

		SendDlgItemMessage(hwndDlg, IDC_LEFT_ACTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("Do nothing"));
		SendDlgItemMessage(hwndDlg, IDC_LEFT_ACTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("Close popup"));

		// Needs to be called here in this case
		{
			BOOL ret = SaveOptsDlgProc(popupsControls, _countof(popupsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
			PopupsEnableDisableCtrls(hwndDlg);
			return ret;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_POPUPS:
		case IDC_WINCOLORS:
		case IDC_DEFAULTCOLORS:
		case IDC_DELAYFROMPU:
		case IDC_DELAYPERMANENT:
		case IDC_DELAYCUSTOM:
			if (HIWORD(wParam) == BN_CLICKED)
				PopupsEnableDisableCtrls(hwndDlg);
			break;

		case IDC_PREV:
			Options op = opts;

			if (IsDlgButtonChecked(hwndDlg, IDC_DELAYFROMPU))
				op.popup_delay_type = POPUP_DELAY_DEFAULT;
			else if (IsDlgButtonChecked(hwndDlg, IDC_DELAYCUSTOM))
				op.popup_delay_type = POPUP_DELAY_CUSTOM;
			else if (IsDlgButtonChecked(hwndDlg, IDC_DELAYPERMANENT))
				op.popup_delay_type = POPUP_DELAY_PERMANENT;

			op.popup_timeout = GetDlgItemInt(hwndDlg, IDC_DELAY, NULL, FALSE);
			op.popup_bkg_color = SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_GETCOLOUR, 0, 0);
			op.popup_text_color = SendDlgItemMessage(hwndDlg, IDC_TEXTCOLOR, CPM_GETCOLOUR, 0, 0);
			op.popup_use_win_colors = IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS) != 0;
			op.popup_use_default_colors = IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS) != 0;

			ShowTestPopup(TranslateT("Test Contact"), TranslateT("Test description"), &op);
			break;
		}
		break;
	}

	return SaveOptsDlgProc(popupsControls, _countof(popupsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

int ImageList_AddIcon_NotShared(HIMAGELIST hIml, int iconId)
{
	HICON hIcon = g_plugin.getIcon(iconId);
	int res = ImageList_AddIcon(hIml, hIcon);
	IcoLib_ReleaseIcon(hIcon);
	return res;
}

class CAutoOptsDlg : public CDlgBase
{
	void ResetListOptions(CCtrlClc* = 0)
	{
		m_clist.SetBkColor(GetSysColor(COLOR_WINDOW));
	}

	void SetAllContactIcons()
	{
		for (auto &cc : Contacts()) {
			HANDLE hItem = m_clist.FindContact(cc);
			if (hItem) {
				// Some Module can handle it?
				if (!CanCall(cc, FALSE))
					m_clist.DeleteItem(hItem);
				else
					m_clist.SetExtraImage(hItem, 0, g_plugin.getWord(cc, "AutoAccept"));
			}
		}
	}

	void SetListGroupIcons(HANDLE hFirstItem, HANDLE hParentItem, int *groupChildCount)
	{
		int iconOn[2] = { 1,1 };
		int childCount[2] = { 0,0 };

		int typeOfFirst = m_clist.GetItemType(hFirstItem);
		
		// check groups
		HANDLE hItem;
		if (typeOfFirst == CLCIT_GROUP) hItem = hFirstItem;
		else hItem = m_clist.GetNextItem(hFirstItem, CLGN_NEXTGROUP);
		while (hItem) {
			HANDLE hChildItem = m_clist.GetNextItem(hItem, CLGN_CHILD);
			if (hChildItem) SetListGroupIcons(hChildItem, hItem, childCount);
			for (int i = 0; i < _countof(iconOn); i++)
				if (iconOn[i] && m_clist.GetExtraImage(hItem, i) == 0)
					iconOn[i] = 0;
			hItem = m_clist.GetNextItem(hItem, CLGN_NEXTGROUP);
		}
		
		// check contacts
		if (typeOfFirst == CLCIT_CONTACT) hItem = hFirstItem;
		else hItem = m_clist.GetNextItem(hFirstItem, CLGN_NEXTCONTACT);
		while (hItem) {
			for (int i = 0; i < _countof(iconOn); i++) {
				int iImage = m_clist.GetExtraImage(hItem, i);
				if (iconOn[i] && iImage == 0)
					iconOn[i] = 0;
				if (iImage != 0xFF) childCount[i]++;
			}
			hItem = m_clist.GetNextItem(hItem, CLGN_NEXTCONTACT);
		}
		
		// set icons
		if (hParentItem != NULL) {
			for (int i = 0; i < _countof(iconOn); i++) {
				m_clist.GetExtraImage(hParentItem, MAKELPARAM(i, childCount[i] ? (iconOn[i] ? i + 1 : 0) : 0xFF));
				if (groupChildCount)
					groupChildCount[i] += childCount[i];
			}
		}
	}

	void SetAllChildIcons(HANDLE hFirstItem, int iColumn, int iImage)
	{
		HANDLE hItem;

		int typeOfFirst = m_clist.GetItemType(hFirstItem);

		// check groups
		if (typeOfFirst == CLCIT_GROUP) hItem = hFirstItem;
		else hItem = m_clist.GetNextItem(hFirstItem, CLGN_NEXTGROUP);
		while (hItem) {
			HANDLE hChildItem = m_clist.GetNextItem(hItem, CLGN_CHILD);
			if (hChildItem)
				SetAllChildIcons(hChildItem, iColumn, iImage);
			hItem = m_clist.GetNextItem(hItem, CLGN_NEXTGROUP);
		}
		
		// check contacts
		if (typeOfFirst == CLCIT_CONTACT) hItem = hFirstItem;
		else hItem = m_clist.GetNextItem(hFirstItem, CLGN_NEXTCONTACT);
		while (hItem) {
			int iOldIcon = m_clist.GetExtraImage(hItem, iColumn);
			if (iOldIcon != 0xFF && iOldIcon != iImage)
				m_clist.SetExtraImage(hItem, iColumn, iImage);
			hItem = m_clist.GetNextItem(hItem, CLGN_NEXTCONTACT);
		}
	}
	
	CCtrlClc m_clist;
	HICON hAnswerIcon, hDropIcon;

public:
	CAutoOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_AUTO),
		m_clist(this, IDC_LIST)
	{
		m_clist.OnClick = Callback(this, &CAutoOptsDlg::onClick_List);
		m_clist.OnNewContact = m_clist.OnListRebuilt = Callback(this, &CAutoOptsDlg::onListRebuilt);
		m_clist.OnOptionsChanged = Callback(this, &CAutoOptsDlg::ResetListOptions);
	}

	bool OnInitDialog() override
	{
		HIMAGELIST hIml = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 3, 3);
		ImageList_AddIcon(hIml, Skin_LoadIcon(SKINICON_OTHER_SMALLDOT));
		ImageList_AddIcon_NotShared(hIml, IDI_ACTION_ANSWER);
		ImageList_AddIcon_NotShared(hIml, IDI_ACTION_DROP);
		m_clist.SetExtraImageList(hIml);

		hAnswerIcon = ImageList_GetIcon(hIml, 1, ILD_NORMAL);
		SendDlgItemMessage(m_hwnd, IDC_ANSWER, STM_SETICON, (WPARAM)hAnswerIcon, 0);

		hDropIcon = ImageList_GetIcon(hIml, 2, ILD_NORMAL);
		SendDlgItemMessage(m_hwnd, IDC_DROP, STM_SETICON, (WPARAM)hDropIcon, 0);

		ResetListOptions();
		m_clist.SetExtraColumns(1);

		onListRebuilt(0);
		return true;
	}

	bool OnApply() override
	{
		for (auto &cc : Contacts()) {
			HANDLE hItem = m_clist.FindContact(cc);
			if (hItem)
				g_plugin.setWord(cc, "AutoAccept", m_clist.GetExtraImage(hItem, 0));
		}
		return true;
	}

	void OnDestroy() override
	{
		DestroyIcon(hAnswerIcon);
		DestroyIcon(hDropIcon);
		ImageList_Destroy((HIMAGELIST)SendDlgItemMessage(m_hwnd, IDC_LIST, CLM_GETEXTRAIMAGELIST, 0, 0));
	}

	void onListRebuilt(CCtrlClc *)
	{
		SetAllContactIcons();
		SetListGroupIcons(m_clist.GetNextItem(0, CLGN_ROOT), 0, 0);
	}

	void onClick_List(CCtrlClc::TEventInfo *pInfo)
	{
		// Make sure we have an extra column
		NMCLISTCONTROL *nm = pInfo->info;
		if (nm->iColumn == -1)
			return;

		// Find clicked item
		uint32_t hitFlags = 0;
		HANDLE hItem = m_clist.HitTest(nm->pt.x, nm->pt.y, &hitFlags);
		if (hItem == NULL || !(hitFlags & CLCHT_ONITEMEXTRA))
			return;

		// Get image in clicked column (0=none, 1=visible, 2=invisible)
		int iImage = m_clist.GetExtraImage(hItem, nm->iColumn);
		if (iImage == 2)
			iImage = 0;
		else
			iImage++;

		// Get item type (contact, group, etc...)
		int itemType = m_clist.GetItemType(hItem);

		// Update list, making sure that the options are mutually exclusive
		if (itemType == CLCIT_CONTACT) { // A contact
			m_clist.SetExtraImage(hItem, nm->iColumn, iImage);
		}
		else if (itemType == CLCIT_GROUP) { // A group
			hItem = m_clist.GetNextItem(hItem, CLGN_CHILD);
			if (hItem)
				SetAllChildIcons(hItem, nm->iColumn, iImage);
		}

		// Update the all/none icons
		SetListGroupIcons(m_clist.GetNextItem(0, CLGN_ROOT), 0, 0);
		NotifyChange();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

static bool IsModuleEnabled()
{
	for (auto& cc : Contacts())
		if (CanCall(cc, FALSE))
			return true;
	
	return false;
}

int InitOptionsCallback(WPARAM wParam, LPARAM)
{
	if (IsModuleEnabled()) {
		OPTIONSDIALOGPAGE odp;
		ZeroMemory(&odp, sizeof(odp));
		odp.pPlugin = &g_plugin;
		odp.flags = ODPF_BOLDGROUPS;

		odp.szGroup.a = LPGEN("Popups");
		odp.szTitle.a = LPGEN("Voice Calls");
		odp.pfnDlgProc = PopupsDlgProc;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUPS);
		g_plugin.addOptions(wParam, &odp);

		ZeroMemory(&odp, sizeof(odp));
		odp.szGroup.a = LPGEN("Voice Calls");
		odp.szTitle.a = LPGEN("Auto actions");
		odp.pDialog = new CAutoOptsDlg();
		g_plugin.addOptions(wParam, &odp);
	}
	return 0;
}

void InitOptions()
{
	LoadOptions();

	hOptHook = HookEvent(ME_OPT_INITIALISE, InitOptionsCallback);
}

void DeInitOptions()
{
	UnhookEvent(hOptHook);
}

void LoadOptions()
{
	LoadOpts(popupsControls, _countof(popupsControls), MODULE_NAME);
}
