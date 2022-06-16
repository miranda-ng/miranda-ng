/*
UserinfoEx plugin for Miranda IM

Copyright:
� 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

#define PSM_ENABLE_TABITEM	(WM_USER+106)

static MenuOptionsList ctrl_Menu[] = {
	{ SET_MI_MAIN,     CHECK_OPT_MI_MAIN,     RADIO_OPT_MI_MAIN_NONE,     RADIO_OPT_MI_MAIN_ALL,     RADIO_OPT_MI_MAIN_EXIMPORT     },
	{ SET_MI_CONTACT,  CHECK_OPT_MI_CONTACT,  RADIO_OPT_MI_CONTACT_NONE,  RADIO_OPT_MI_CONTACT_ALL,  RADIO_OPT_MI_CONTACT_EXIMPORT  },
	{ SET_MI_GROUP,    CHECK_OPT_MI_GROUP,    RADIO_OPT_MI_GROUP_NONE,    RADIO_OPT_MI_GROUP_ALL,    RADIO_OPT_MI_GROUP_EXIMPORT    },
	{ SET_MI_SUBGROUP, CHECK_OPT_MI_SUBGROUP, RADIO_OPT_MI_SUBGROUP_NONE, RADIO_OPT_MI_SUBGROUP_ALL, RADIO_OPT_MI_SUBGROUP_EXIMPORT },
	{ SET_MI_ACCOUNT,  CHECK_OPT_MI_ACCOUNT,  RADIO_OPT_MI_ACCOUNT_NONE,  RADIO_OPT_MI_ACCOUNT_ALL,  RADIO_OPT_MI_ACCOUNT_EXIMPORT  },
};

void __forceinline NotifyParentOfChange(HWND hDlg)
{
	SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
}

int __forceinline ComboBox_FindByItemDataPtr(HWND hCombo, LPARAM pData)
{
	int nItemIndex;
	for (nItemIndex = ComboBox_GetCount(hCombo); (nItemIndex >= 0) && (ComboBox_GetItemData(hCombo, nItemIndex) != pData); nItemIndex--);
	return nItemIndex;
}

void __forceinline ComboBox_SetCurSelByItemDataPtr(HWND hCombo, LPARAM pData)
{
	ComboBox_SetCurSel(hCombo, ComboBox_FindByItemDataPtr(hCombo, pData));
}

void __forceinline ComboBox_AddItemWithData(HWND hCombo, LPTSTR ptszText, LPARAM pData)
{
	ComboBox_SetItemData(hCombo, ComboBox_AddString(hCombo, TranslateW(ptszText)), pData);
}

/**
 * This function enables a dialog item
 *
 * @param	hWnd			- the dialog's window handle
 * @param	idCtrl			- the dialog item's identifier
 * @param	bEnabled		- TRUE if the item should be enabled, FALSE if disabled
 *
 * @retval	TRUE on success
 * @retval	FALSE on failure
 **/
static uint8_t EnableDlgItem(HWND hDlg, const int idCtrl, uint8_t bEnabled)
{
	return EnableWindow(GetDlgItem(hDlg, idCtrl), bEnabled);
}

/**
 * This function enables a list of dialog items, if they were enabled in the resource editor.
 *
 * @param	hWnd			- the dialog's window handle
 * @param	idCtrl			- the array of dialog items' identifiers
 * @param	countCtrl		- the number of items in the array of dialog items
 * @param	bEnabled		- TRUE if the item should be enabled, FALSE if disabled
 *
 * @return	bEnabled
 **/
static uint8_t InitialEnableControls(HWND hDlg, const int *idCtrl, int countCtrl, uint8_t bEnabled)
{
	HWND hCtrl;

	while (countCtrl-- > 0) {
		hCtrl = GetDlgItem(hDlg, idCtrl[countCtrl]);
		EnableWindow(hCtrl, IsWindowEnabled(hCtrl) && bEnabled);
	}
	return bEnabled;
}

/**
 * This function enables a list of dialog items.
 *
 * @param	hWnd			- the dialog's window handle
 * @param	idCtrl			- the array of dialog items' identifiers
 * @param	countCtrl		- the number of items in the array of dialog items
 * @param	bEnabled		- TRUE if the item should be enabled, FALSE if disabled
 *
 * @return	bEnabled
 **/
static uint8_t EnableControls(HWND hDlg, const int *idCtrl, int countCtrl, uint8_t bEnabled)
{
	while (countCtrl-- > 0)
		EnableDlgItem(hDlg, idCtrl[countCtrl], bEnabled);
	return bEnabled;
}

/**
 * This function checks an dialog button according to the value, read from the database
 *
 * @param	hWnd			- the dialog's window handle
 * @param	idCtrl			- the dialog item's identifier
 * @param	pszSetting		- the setting from the database to use
 * @param	bDefault		- the default value to use, if no database setting exists
 *
 * @return	This function returns the value from database or the default value.
 **/
static uint8_t DBGetCheckBtn(HWND hDlg, const int idCtrl, LPCSTR pszSetting, uint8_t bDefault)
{
	uint8_t val = (g_plugin.getByte(pszSetting, bDefault) & 1) == 1;
	CheckDlgButton(hDlg, idCtrl, val ? BST_CHECKED : BST_UNCHECKED);
	return val;
}

/**
 * This function writes a byte (flag = 1) to database according to the checkstate
 * of the dialog button identified by 'idCtrl'.
 *
 * @param	hWnd			- the dialog's window handle
 * @param	idCtrl			- the dialog item's identifier
 * @param	pszSetting		- the setting to write the button state to
 *
 * @return	checkstate
 **/
static uint8_t DBWriteCheckBtn(HWND hDlg, const int idCtrl, LPCSTR pszSetting)
{
	uint8_t val = IsDlgButtonChecked(hDlg, idCtrl);
	int Temp = g_plugin.getByte(pszSetting, 0);
	Temp &= ~1;
	g_plugin.setByte(pszSetting, Temp |= val);
	return val;
}

/**
 * This function reads a uint32_t from database and interprets it as an color value
 * to set to the color control.
 *
 * @param	hWnd			- the dialog's window handle
 * @param	idCtrl			- the dialog item's identifier
 * @param	pszSetting		- the setting from the database to use
 * @param	bDefault		- the default value to use, if no database setting exists
 *
 * @return	nothing
 **/
static void DBGetColor(HWND hDlg, const int idCtrl, LPCSTR pszSetting, uint32_t bDefault)
{
	SendDlgItemMessage(hDlg, idCtrl, CPM_SETCOLOUR, 0, g_plugin.getDword(pszSetting, bDefault));
}

/**
 * This function writes a uint32_t to database according to the value
 * of the color control identified by 'idCtrl'.
 *
 * @param	hWnd			- the dialog's window handle
 * @param	idCtrl			- the dialog item's identifier
 * @param	pszSetting		- the setting to write the button state to
 *
 * @return	nothing
 **/
static void DBWriteColor(HWND hDlg, const int idCtrl, LPCSTR pszSetting)
{
	g_plugin.setDword(pszSetting, (uint32_t)SendDlgItemMessage(hDlg, idCtrl, CPM_GETCOLOUR, 0, 0));
}

/**
 * This function writes a uint8_t to database according to the value
 * read from the edit control identified by 'idCtrl'.
 *
 * @param	hWnd			- the dialog's window handle
 * @param	idCtrl			- the dialog item's identifier
 * @param	pszSetting		- the setting to write the button state to
 * @param	defVal			- this is the default value used by the GetByte() function in order 
 *							  to check whether updating the value is required or not.
 *
 * @retval	TRUE			- the database value was updated
 * @retval	FALSE			- no database update needed
 **/
static uint8_t DBWriteEditByte(HWND hDlg, const int idCtrl, LPCSTR pszSetting, uint8_t defVal)
{
	uint8_t v;
	BOOL t;

	v = (uint8_t)GetDlgItemInt(hDlg, idCtrl, &t, FALSE);
	if (t && (v != g_plugin.getByte(pszSetting, defVal))) {
		g_plugin.setByte(pszSetting, v);
		return true;
	}
	return FALSE;
}

/**
 * This function writes a uint16_t to database according to the value
 * read from the edit control identified by 'idCtrl'.
 *
 * @param	hWnd			- the dialog's window handle
 * @param	idCtrl			- the dialog item's identifier
 * @param	pszSetting		- the setting to write the button state to
 * @param	defVal			- this is the default value used by the GetWord() function in order 
 *							  to check whether updating the value is required or not.
 *
 * @retval	TRUE			- the database value was updated
 * @retval	FALSE			- no database update needed
 **/
static uint8_t DBWriteEditWord(HWND hDlg, const int idCtrl, LPCSTR pszSetting, uint16_t defVal)
{
	uint16_t v;
	BOOL t;

	v = (uint16_t)GetDlgItemInt(hDlg, idCtrl, &t, FALSE);
	if (t && (v != g_plugin.getWord(pszSetting, defVal))) {
		g_plugin.setWord(pszSetting, v);
		return true;
	}
	return FALSE;
}

/**
 * This function writes a uint8_t to database according to the currently
 * selected item of a combobox identified by 'idCtrl'.
 *
 * @param	hWnd			- the dialog's window handle
 * @param	idCtrl			- the dialog item's identifier
 * @param	pszSetting		- the setting to write the button state to
 * @param	defVal			- this is the default value used by the GetByte() function in order 
 *							  to check whether updating the value is required or not.
 *
 * @retval	TRUE			- the database value was updated
 * @retval	FALSE			- no database update needed
 **/
static uint8_t DBWriteComboByte(HWND hDlg, const int idCtrl, LPCSTR pszSetting, uint8_t defVal)
{
	uint8_t v;

	v = (uint8_t)SendDlgItemMessage(hDlg, idCtrl, CB_GETCURSEL, NULL, NULL);
	if (v != g_plugin.getByte(pszSetting, defVal)) {
		g_plugin.setByte(pszSetting, v);
		return true;
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProc_CommonOpts(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static uint8_t bInitialized = 0;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		bInitialized = 0;

		// menu item settings
		for (auto &it : ctrl_Menu) {
			int flag = g_plugin.getByte(it.pszKey, 2);
			// check button and enable / disable control
			int idEnable[] = { it.idCheckbox + 1, it.idNONE, it.idALL, it.idEXIMPORT };
			EnableControls(hDlg, idEnable, _countof(idEnable), DBGetCheckBtn(hDlg, it.idCheckbox, it.pszKey, 0));
			// set radio button state
			int id = it.idNONE;	//default
			if ((flag & 4) == 4)
				id = it.idALL;
			else if ((flag & 8) == 8)
				id = it.idEXIMPORT;
			CheckRadioButton(hDlg, it.idNONE, it.idEXIMPORT, id);
		}

		// extra icon settings
		CheckDlgButton(hDlg, CHECK_OPT_FLAGSUNKNOWN, g_bUseUnknownFlag ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, CHECK_OPT_FLAGSMSGSTATUS, g_bShowStatusIconFlag ? BST_CHECKED : BST_UNCHECKED);

		bInitialized = 1;
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			// menu item settings
			for (auto &it : ctrl_Menu) {
				int flag = IsDlgButtonChecked(hDlg, it.idCheckbox);
				flag |= IsDlgButtonChecked(hDlg, it.idNONE) ? 2 : 0;
				flag |= IsDlgButtonChecked(hDlg, it.idALL) ? 4 : 0;
				flag |= IsDlgButtonChecked(hDlg, it.idEXIMPORT) ? 8 : 0;
				g_plugin.setByte(it.pszKey, (uint8_t)flag);
			}

			RebuildMenu();

			// extra icon settings
			bool FlagsClistChange = false, FlagsMsgWndChange = false;

			bool valNew = IsDlgButtonChecked(hDlg, CHECK_OPT_FLAGSUNKNOWN) != 0;
			if (g_bUseUnknownFlag != valNew) {
				g_bUseUnknownFlag = valNew;
				db_set_b(0, MODNAMEFLAGS, "UseUnknownFlag", valNew);
				FlagsClistChange = true;
				FlagsMsgWndChange = true;
			}
			valNew = IsDlgButtonChecked(hDlg, CHECK_OPT_FLAGSMSGSTATUS) != 0;
			if (g_bShowStatusIconFlag != valNew) {
				g_bShowStatusIconFlag = valNew;
				db_set_b(0, MODNAMEFLAGS, "ShowStatusIconFlag", valNew);
				FlagsMsgWndChange = true;
			}

			if (FlagsClistChange)
				ExtraIcon_SetAll();
			if (FlagsMsgWndChange)
				UpdateStatusIcons();
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case CHECK_OPT_MI_MAIN:
		case CHECK_OPT_MI_CONTACT:
		case CHECK_OPT_MI_GROUP:
		case CHECK_OPT_MI_SUBGROUP:
		case CHECK_OPT_MI_ACCOUNT:
			for (auto &it : ctrl_Menu) {
				if (it.idCheckbox == LOWORD(wParam)) {
					const int idMenuItems[] = { it.idCheckbox + 1, it.idNONE, it.idALL, it.idEXIMPORT };
					EnableControls(hDlg, idMenuItems, _countof(idMenuItems), Button_GetCheck((HWND)lParam));
					break;
				}
			}
			if (bInitialized)
				NotifyParentOfChange(hDlg);
			break;

		case RADIO_OPT_MI_MAIN_ALL:
		case RADIO_OPT_MI_MAIN_NONE:
		case RADIO_OPT_MI_MAIN_EXIMPORT:
		case RADIO_OPT_MI_CONTACT_ALL:
		case RADIO_OPT_MI_CONTACT_NONE:
		case RADIO_OPT_MI_CONTACT_EXIMPORT:
		case RADIO_OPT_MI_GROUP_ALL:
		case RADIO_OPT_MI_GROUP_NONE:
		case RADIO_OPT_MI_GROUP_EXIMPORT:
		case RADIO_OPT_MI_SUBGROUP_ALL:
		case RADIO_OPT_MI_SUBGROUP_NONE:
		case RADIO_OPT_MI_SUBGROUP_EXIMPORT:
		case RADIO_OPT_MI_ACCOUNT_ALL:
		case RADIO_OPT_MI_ACCOUNT_NONE:
		case RADIO_OPT_MI_ACCOUNT_EXIMPORT:
		case CHECK_OPT_FLAGSUNKNOWN:
		case CHECK_OPT_FLAGSMSGSTATUS:
			if (bInitialized)
				NotifyParentOfChange(hDlg);
		}
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProc_AdvancedOpts(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static uint8_t bInitialized = 0;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		bInitialized = 0;

		DBGetCheckBtn(hDlg, CHECK_OPT_ICOVERSION, SET_ICONS_CHECKFILEVERSION, TRUE);
		DBGetCheckBtn(hDlg, CHECK_OPT_BUTTONICONS, SET_ICONS_BUTTONS, TRUE);
		DBGetCheckBtn(hDlg, CHECK_OPT_METASCAN, SET_META_SCAN, TRUE);
		DBGetCheckBtn(hDlg, CHECK_OPT_SREMAIL_ENABLED, SET_EXTENDED_EMAILSERVICE, TRUE);
		CheckDlgButton(hDlg, CHECK_OPT_AUTOTIMEZONE, BST_CHECKED);
		EnableWindow(GetDlgItem(hDlg, CHECK_OPT_AUTOTIMEZONE), FALSE);

		bInitialized = 1;
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			DBWriteCheckBtn(hDlg, CHECK_OPT_ICOVERSION, SET_ICONS_CHECKFILEVERSION);
			DBWriteCheckBtn(hDlg, CHECK_OPT_BUTTONICONS, SET_ICONS_BUTTONS);
			DBWriteCheckBtn(hDlg, CHECK_OPT_METASCAN, SET_META_SCAN);

			DBWriteCheckBtn(hDlg, CHECK_OPT_SREMAIL_ENABLED, SET_EXTENDED_EMAILSERVICE);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case CHECK_OPT_ICOVERSION:
		case CHECK_OPT_BUTTONICONS:
		case CHECK_OPT_METASCAN:
		case CHECK_OPT_SREMAIL_ENABLED:
		case CHECK_OPT_AUTOTIMEZONE:
			if (bInitialized)
				NotifyParentOfChange(hDlg);
			break;

		case BTN_OPT_RESET:
			uint8_t WantReset = MsgBox(hDlg,
				MB_ICON_WARNING | MB_YESNO,
				LPGENW("Question"),
				LPGENW("Reset factory defaults"),
				LPGENW("This will delete all settings, you've made!\nAll TreeView settings, window positions and any other settings!\n\nAre you sure to proceed?"));

			if (WantReset) {
				DB::CEnumList Settings;

				// delete all skin icons
				if (!Settings.EnumSettings(NULL, "SkinIcons"))
					for (auto &s : Settings)
						if (mir_strncmpi(s, "UserInfoEx", 10) == 0)
							db_unset(0, "SkinIcons", s);

				// delete global settings
				db_delete_module(NULL, USERINFO"Ex");
				db_delete_module(NULL, USERINFO"ExW");

				// delete old contactsettings
				for (auto &hContact : Contacts()) {
					db_unset(hContact, USERINFO, "PListColWidth0");
					db_unset(hContact, USERINFO, "PListColWidth1");
					db_unset(hContact, USERINFO, "PListColWidth2");
					db_unset(hContact, USERINFO, "EMListColWidth0");
					db_unset(hContact, USERINFO, "EMListColWidth1");
					db_unset(hContact, USERINFO, "BirthRemind");
					db_unset(hContact, USERINFO, "RemindBirthday");
					db_unset(hContact, USERINFO, "RemindDaysErlier");
					db_unset(hContact, USERINFO, "vCardPath");

					db_delete_module(hContact, USERINFO"Ex");
					db_delete_module(hContact, USERINFO"ExW");
				}

				SendMessage(GetParent(hDlg), PSM_FORCECHANGED, NULL, NULL);
				MsgBox(hDlg, MB_ICON_INFO,
					LPGENW("Ready"),
					LPGENW("Everything is done!"),
					LPGENW("All settings are reset to default values now!"));
			}
		}
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProc_DetailsDlgOpts(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static uint8_t bInitialized = 0;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		bInitialized = 0;

		// init colors
		DBGetCheckBtn(hDlg, CHECK_OPT_CLR, SET_PROPSHEET_SHOWCOLOURS, TRUE);
		SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(CHECK_OPT_CLR, BN_CLICKED), (LPARAM)GetDlgItem(hDlg, CHECK_OPT_CLR));
		DBGetColor(hDlg, CLR_NORMAL, SET_PROPSHEET_CLRNORMAL, RGB(90, 90, 90));
		DBGetColor(hDlg, CLR_USER, SET_PROPSHEET_CLRCUSTOM, RGB(0, 10, 130));
		DBGetColor(hDlg, CLR_BOTH, SET_PROPSHEET_CLRBOTH, RGB(0, 160, 10));
		DBGetColor(hDlg, CLR_CHANGED, SET_PROPSHEET_CLRCHANGED, RGB(190, 0, 0));
		DBGetColor(hDlg, CLR_META, SET_PROPSHEET_CLRMETA, RGB(120, 40, 130));

		// treeview options
		DBGetCheckBtn(hDlg, CHECK_OPT_GROUPS, SET_PROPSHEET_GROUPS, TRUE);
		DBGetCheckBtn(hDlg, CHECK_OPT_SORTTREE, SET_PROPSHEET_SORTITEMS, FALSE);
		DBGetCheckBtn(hDlg, CHECK_OPT_AEROADAPTION, SET_PROPSHEET_AEROADAPTION, TRUE);

		// common options
		DBGetCheckBtn(hDlg, CHECK_OPT_READONLY, SET_PROPSHEET_PCBIREADONLY, FALSE);
		DBGetCheckBtn(hDlg, CHECK_OPT_CHANGEMYDETAILS, SET_PROPSHEET_CHANGEMYDETAILS, FALSE);
		Button_Enable(GetDlgItem(hDlg, CHECK_OPT_CHANGEMYDETAILS), myGlobals.CanChangeDetails);

		bInitialized = 1;
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			DBWriteCheckBtn(hDlg, CHECK_OPT_CLR, SET_PROPSHEET_SHOWCOLOURS);
			DBWriteCheckBtn(hDlg, CHECK_OPT_GROUPS, SET_PROPSHEET_GROUPS);
			DBWriteCheckBtn(hDlg, CHECK_OPT_SORTTREE, SET_PROPSHEET_SORTITEMS);
			DBWriteCheckBtn(hDlg, CHECK_OPT_READONLY, SET_PROPSHEET_PCBIREADONLY);
			DBWriteCheckBtn(hDlg, CHECK_OPT_AEROADAPTION, SET_PROPSHEET_AEROADAPTION);
			DBWriteCheckBtn(hDlg, CHECK_OPT_CHANGEMYDETAILS, SET_PROPSHEET_CHANGEMYDETAILS);

			DBWriteColor(hDlg, CLR_NORMAL, SET_PROPSHEET_CLRNORMAL);
			DBWriteColor(hDlg, CLR_USER, SET_PROPSHEET_CLRCUSTOM);
			DBWriteColor(hDlg, CLR_BOTH, SET_PROPSHEET_CLRBOTH);
			DBWriteColor(hDlg, CLR_CHANGED, SET_PROPSHEET_CLRCHANGED);
			DBWriteColor(hDlg, CLR_META, SET_PROPSHEET_CLRMETA);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case CHECK_OPT_CLR:
			if (HIWORD(wParam) == BN_CLICKED) {
				BOOL bChecked = SendMessage((HWND)lParam, BM_GETCHECK, NULL, NULL);
				const int idCtrl[] = { CLR_NORMAL, CLR_USER, CLR_BOTH, CLR_CHANGED, CLR_META, TXT_OPT_CLR_NORMAL,
					TXT_OPT_CLR_USER, TXT_OPT_CLR_BOTH, TXT_OPT_CLR_CHANGED, TXT_OPT_CLR_META };

				EnableControls(hDlg, idCtrl, _countof(idCtrl), bChecked);
			}
			__fallthrough;

		case CHECK_OPT_GROUPS:
		case CHECK_OPT_SORTTREE:
		case CHECK_OPT_AEROADAPTION:
		case CHECK_OPT_READONLY:
		case CHECK_OPT_CHANGEMYDETAILS:
		case CHECK_OPT_MI_CONTACT:
			if (bInitialized && HIWORD(wParam) == BN_CLICKED)
				NotifyParentOfChange(hDlg);
			break;

		default:
			if (bInitialized && HIWORD(wParam) == CPN_COLOURCHANGED)
				NotifyParentOfChange(hDlg);
		}
	}
	return FALSE;
}


static INT_PTR CALLBACK DlgProc_ReminderOpts(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static uint8_t bInitialized = 0;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		ShowWindow(GetDlgItem(hDlg, CHECK_REMIND_SECURED), SW_SHOW);

		SendDlgItemMessage(hDlg, ICO_BIRTHDAY, STM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(IDI_ANNIVERSARY, true));

		// set colours			
		SendDlgItemMessage(hDlg, EDIT_REMIND, EM_LIMITTEXT, 2, 0);
		SendDlgItemMessage(hDlg, SPIN_REMIND, UDM_SETRANGE32, 0, 50);
		SendDlgItemMessage(hDlg, EDIT_REMIND2, EM_LIMITTEXT, 4, 0);
		SendDlgItemMessage(hDlg, SPIN_REMIND2, UDM_SETRANGE32, 1, 8760);
		SendDlgItemMessage(hDlg, EDIT_REMIND_SOUNDOFFSET, EM_LIMITTEXT, 2, 0);
		SendDlgItemMessage(hDlg, SPIN_REMIND_SOUNDOFFSET, UDM_SETRANGE32, 0, 50);

		HWND hCtrl;
		if (hCtrl = GetDlgItem(hDlg, EDIT_REMIND_ENABLED)) {
			ComboBox_AddString(hCtrl, TranslateT("Reminder disabled"));
			ComboBox_AddString(hCtrl, TranslateT("Birthdays only"));
			ComboBox_AddString(hCtrl, TranslateT("Anniversaries only"));			
			ComboBox_AddString(hCtrl, TranslateT("Everything"));
		}

		bInitialized = 0;
		{
			// set reminder options
			uint8_t bEnabled = g_plugin.getByte(SET_REMIND_ENABLED, DEFVAL_REMIND_ENABLED);
			SendDlgItemMessage(hDlg, EDIT_REMIND_ENABLED, CB_SETCURSEL, bEnabled, NULL);
			DlgProc_ReminderOpts(hDlg, WM_COMMAND, MAKEWPARAM(EDIT_REMIND_ENABLED, CBN_SELCHANGE),
				(LPARAM)GetDlgItem(hDlg, EDIT_REMIND_ENABLED));

			DBGetCheckBtn(hDlg, CHECK_REMIND_MI, SET_REMIND_MENUENABLED, DEFVAL_REMIND_MENUENABLED);
			DBGetCheckBtn(hDlg, CHECK_REMIND_FLASHICON, SET_REMIND_FLASHICON, FALSE);
			DBGetCheckBtn(hDlg, CHECK_REMIND_VISIBLEONLY, SET_REMIND_CHECKVISIBLE, DEFVAL_REMIND_CHECKVISIBLE);
			DBGetCheckBtn(hDlg, CHECK_REMIND_STARTUP, SET_REMIND_CHECKON_STARTUP, FALSE);
			DBGetCheckBtn(hDlg, CHECK_REMIND_SECURED, SET_REMIND_SECUREBIRTHDAY, FALSE);

			SetDlgItemInt(hDlg, EDIT_REMIND, g_plugin.getWord(SET_REMIND_OFFSET, DEFVAL_REMIND_OFFSET), FALSE);
			SetDlgItemInt(hDlg, EDIT_REMIND_SOUNDOFFSET, g_plugin.getByte(SET_REMIND_SOUNDOFFSET, DEFVAL_REMIND_SOUNDOFFSET), FALSE);
			SetDlgItemInt(hDlg, EDIT_REMIND2, g_plugin.getWord(SET_REMIND_NOTIFYINTERVAL, DEFVAL_REMIND_NOTIFYINTERVAL), FALSE);

			MTime mtLast;
			wchar_t szTime[MAX_PATH];

			mtLast.DBGetStamp(0, MODULENAME, SET_REMIND_LASTCHECK);
			mtLast.UTCToLocal();
			mtLast.TimeFormat(szTime, _countof(szTime));

			SetDlgItemText(hDlg, TXT_REMIND_LASTCHECK, szTime);
		}
		bInitialized = 1;
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			{
				uint8_t bReminderCheck = FALSE;

				// save checkbox options
				DBWriteCheckBtn(hDlg, CHECK_REMIND_MI, SET_REMIND_MENUENABLED);
				DBWriteCheckBtn(hDlg, CHECK_REMIND_FLASHICON, SET_REMIND_FLASHICON);
				DBWriteCheckBtn(hDlg, CHECK_REMIND_VISIBLEONLY, SET_REMIND_CHECKVISIBLE);
				DBWriteCheckBtn(hDlg, CHECK_REMIND_STARTUP, SET_REMIND_CHECKON_STARTUP);
				DBWriteCheckBtn(hDlg, CHECK_REMIND_SECURED, SET_REMIND_SECUREBIRTHDAY);

				DBWriteEditByte(hDlg, EDIT_REMIND_SOUNDOFFSET, SET_REMIND_SOUNDOFFSET, DEFVAL_REMIND_SOUNDOFFSET);
				DBWriteEditWord(hDlg, EDIT_REMIND2, SET_REMIND_NOTIFYINTERVAL, DEFVAL_REMIND_NOTIFYINTERVAL);
				bReminderCheck = DBWriteEditWord(hDlg, EDIT_REMIND, SET_REMIND_OFFSET, DEFVAL_REMIND_OFFSET);

				// update current reminder state
				uint8_t bNewVal = (uint8_t)SendDlgItemMessage(hDlg, EDIT_REMIND_ENABLED, CB_GETCURSEL, NULL, NULL);
				if (g_plugin.getByte(SET_REMIND_ENABLED, 1) != bNewVal) {
					g_plugin.setByte(SET_REMIND_ENABLED, bNewVal);
					if (bNewVal == REMIND_OFF) {
						SvcReminderEnable(FALSE);
						bReminderCheck = FALSE;
					}
					else bReminderCheck = TRUE;
				}

				// update all contact list extra icons
				if (bReminderCheck) {
					SvcReminderEnable(TRUE); // reinit reminder options from db
					SvcReminderCheckAll(NOTIFY_CLIST); // notify
				}
				RebuildMain();
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case EDIT_REMIND_ENABLED:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				int bEnabled = ComboBox_GetCurSel((HWND)lParam) > 0;
				const int idCtrl[] = {
					CHECK_REMIND_MI, EDIT_REMIND, EDIT_REMIND2, SPIN_REMIND, SPIN_REMIND2, TXT_REMIND,
					TXT_REMIND2, TXT_REMIND3, TXT_REMIND4, TXT_REMIND6, TXT_REMIND8, TXT_REMIND9,
					TXT_REMIND_LASTCHECK, CHECK_REMIND_FLASHICON, CHECK_REMIND_VISIBLEONLY,
					CHECK_REMIND_SECURED, CHECK_REMIND_STARTUP, EDIT_REMIND_SOUNDOFFSET, SPIN_REMIND_SOUNDOFFSET
				};

				EnableControls(hDlg, idCtrl, _countof(idCtrl), bEnabled);
			}
			__fallthrough;

		case CHECK_REMIND_MI:
		case CHECK_REMIND_FLASHICON:
		case CHECK_REMIND_VISIBLEONLY:
		case CHECK_REMIND_STARTUP:
		case CHECK_REMIND_SECURED:
			if (bInitialized && HIWORD(wParam) == BN_CLICKED)
				NotifyParentOfChange(hDlg);
			break;

		// The user changes the number of days in advance of an anniversary to be notified by popups and clist extra icon.
		case EDIT_REMIND:
			if (bInitialized && HIWORD(wParam) == EN_UPDATE) {
				BOOL t;
				uint16_t v = (uint16_t)GetDlgItemInt(hDlg, LOWORD(wParam), &t, FALSE);
				if (t && (v != g_plugin.getWord(SET_REMIND_OFFSET, DEFVAL_REMIND_OFFSET)))
					NotifyParentOfChange(hDlg);
			}
			break;

		// The user changes the number of days in advance of an anniversary to be notified by sound.
		case EDIT_REMIND_SOUNDOFFSET:
			if (bInitialized && HIWORD(wParam) == EN_UPDATE) {
				BOOL t;
				uint8_t v = (uint8_t)GetDlgItemInt(hDlg, LOWORD(wParam), &t, FALSE);
				if (t && (v != g_plugin.getByte(SET_REMIND_SOUNDOFFSET, DEFVAL_REMIND_SOUNDOFFSET)))
					NotifyParentOfChange(hDlg);
			}
			break;

		// The user changes the notification interval
		case EDIT_REMIND2:
			if (bInitialized && HIWORD(wParam) == EN_UPDATE) {
				BOOL t;
				uint16_t v = (uint16_t)GetDlgItemInt(hDlg, LOWORD(wParam), &t, FALSE);
				if (t && (v != g_plugin.getWord(SET_REMIND_NOTIFYINTERVAL, DEFVAL_REMIND_NOTIFYINTERVAL)))
					NotifyParentOfChange(hDlg);
			}
		}
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProc_Popups(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static uint8_t bInitialized = 0;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		bInitialized = 0;
		{
			DBGetCheckBtn(hDlg, CHECK_OPT_POPUP_MSGBOX, SET_POPUPMSGBOX, DEFVAL_POPUPMSGBOX);
			DBGetCheckBtn(hDlg, CHECK_OPT_POPUP_PROGRESS, "PopupProgress", FALSE);
			// disable if popup plugin dos not sopport buttons inside popop
			if (!(db_get_dw(0, "Popup", "Actions", 0) & 1))
				EnableDlgItem(hDlg, CHECK_OPT_POPUP_MSGBOX, FALSE);

			// enable/disable popups
			uint8_t isEnabled = DBGetCheckBtn(hDlg, CHECK_OPT_POPUP_ENABLED, SET_POPUP_ENABLED, DEFVAL_POPUP_ENABLED);
			SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(CHECK_OPT_POPUP_ENABLED, BN_CLICKED), (LPARAM)GetDlgItem(hDlg, CHECK_OPT_POPUP_ENABLED));

			// set colortype checkboxes and color controls
			DBGetColor(hDlg, CLR_BBACK, SET_POPUP_BIRTHDAY_COLOR_BACK, RGB(192, 180, 30));
			DBGetColor(hDlg, CLR_BTEXT, SET_POPUP_BIRTHDAY_COLOR_TEXT, RGB(0, 0, 0));
			switch (g_plugin.getByte(SET_POPUP_BIRTHDAY_COLORTYPE, POPUP_COLOR_CUSTOM)) {
			case POPUP_COLOR_DEFAULT:
				CheckDlgButton(hDlg, CHECK_OPT_POPUP_DEFCLR, BST_CHECKED);
				break;

			case POPUP_COLOR_WINDOWS:
				CheckDlgButton(hDlg, CHECK_OPT_POPUP_WINCLR, BST_CHECKED);
			}

			DBGetColor(hDlg, CLR_ABACK, SET_POPUP_ANNIVERSARY_COLOR_BACK, RGB(90, 190, 130));
			DBGetColor(hDlg, CLR_ATEXT, SET_POPUP_ANNIVERSARY_COLOR_TEXT, RGB(0, 0, 0));
			switch (g_plugin.getByte(SET_POPUP_ANNIVERSARY_COLORTYPE, POPUP_COLOR_CUSTOM)) {
			case POPUP_COLOR_DEFAULT:
				CheckDlgButton(hDlg, CHECK_OPT_POPUP_ADEFCLR, BST_CHECKED);
				break;
			case POPUP_COLOR_WINDOWS:
				CheckDlgButton(hDlg, CHECK_OPT_POPUP_AWINCLR, BST_CHECKED);
			}

			if (isEnabled) {
				SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(CHECK_OPT_POPUP_DEFCLR, BN_CLICKED), NULL);
				SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(CHECK_OPT_POPUP_ADEFCLR, BN_CLICKED), NULL);
			}
			// set delay values
			uint8_t bDelay = g_plugin.getByte(SET_POPUP_DELAY, 0);
			switch (bDelay) {
			case 0:
				CheckDlgButton(hDlg, RADIO_OPT_POPUP_DEFAULT, BST_CHECKED);
				if (isEnabled)
					EnableDlgItem(hDlg, EDIT_DELAY, FALSE);
				break;

			case 255:
				CheckDlgButton(hDlg, RADIO_OPT_POPUP_PERMANENT, BST_CHECKED);
				if (isEnabled)
					EnableDlgItem(hDlg, EDIT_DELAY, FALSE);
				break;

			default:
				CheckDlgButton(hDlg, RADIO_OPT_POPUP_CUSTOM, BST_CHECKED);
				SetDlgItemInt(hDlg, EDIT_DELAY, bDelay, FALSE);
			}
		}
		bInitialized = TRUE;
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			DBWriteCheckBtn(hDlg, CHECK_OPT_POPUP_MSGBOX, SET_POPUPMSGBOX);
			DBWriteCheckBtn(hDlg, CHECK_OPT_POPUP_PROGRESS, "PopupProgress");
			DBWriteCheckBtn(hDlg, CHECK_OPT_POPUP_ENABLED, SET_POPUP_ENABLED);

			// save popup style for birthdays
			DBWriteColor(hDlg, CLR_BBACK, SET_POPUP_BIRTHDAY_COLOR_BACK);
			DBWriteColor(hDlg, CLR_BTEXT, SET_POPUP_BIRTHDAY_COLOR_TEXT);
			g_plugin.setByte(SET_POPUP_BIRTHDAY_COLORTYPE, 
				IsDlgButtonChecked(hDlg, CHECK_OPT_POPUP_DEFCLR)
				? POPUP_COLOR_DEFAULT
				: IsDlgButtonChecked(hDlg, CHECK_OPT_POPUP_WINCLR)
				? POPUP_COLOR_WINDOWS
				: POPUP_COLOR_CUSTOM);

			// save popup style for anniversaries
			DBWriteColor(hDlg, CLR_ABACK, SET_POPUP_ANNIVERSARY_COLOR_BACK);
			DBWriteColor(hDlg, CLR_ATEXT, SET_POPUP_ANNIVERSARY_COLOR_TEXT);
			g_plugin.setByte(SET_POPUP_ANNIVERSARY_COLORTYPE, 
				IsDlgButtonChecked(hDlg, CHECK_OPT_POPUP_ADEFCLR)
				? POPUP_COLOR_DEFAULT
				: IsDlgButtonChecked(hDlg, CHECK_OPT_POPUP_AWINCLR)
				? POPUP_COLOR_WINDOWS
				: POPUP_COLOR_CUSTOM);

			// save delay
			if (IsDlgButtonChecked(hDlg, RADIO_OPT_POPUP_PERMANENT))
				g_plugin.setByte(SET_POPUP_DELAY, 255);
			else if (IsDlgButtonChecked(hDlg, RADIO_OPT_POPUP_CUSTOM)) {
				wchar_t szDelay[4];
				GetDlgItemText(hDlg, EDIT_DELAY, szDelay, _countof(szDelay));
				g_plugin.setByte(SET_POPUP_DELAY, (uint8_t)wcstol(szDelay, nullptr, 10));
			}
			else
				g_plugin.delSetting(SET_POPUP_DELAY);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case BTN_PREVIEW:
			{
				POPUPDATAW ppd;
				ppd.iSeconds = (int)g_plugin.getByte(SET_POPUP_DELAY, 0);
				mir_wstrncpy(ppd.lpwzText, TranslateT("This is the reminder message"), MAX_SECONDLINE);

				// Birthday
				mir_wstrncpy(ppd.lpwzContactName, TranslateT("Birthday"), _countof(ppd.lpwzContactName));
				ppd.lchIcon = g_plugin.getIcon(IDI_RMD_DTB0);
				if (IsDlgButtonChecked(hDlg, CHECK_OPT_POPUP_WINCLR)) {
					ppd.colorBack = GetSysColor(COLOR_BTNFACE);
					ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
				}
				else if (BST_UNCHECKED == IsDlgButtonChecked(hDlg, CHECK_OPT_POPUP_DEFCLR)) {
					ppd.colorBack = SendDlgItemMessage(hDlg, CLR_BBACK, CPM_GETCOLOUR, 0, 0);
					ppd.colorText = SendDlgItemMessage(hDlg, CLR_BTEXT, CPM_GETCOLOUR, 0, 0);
				}
				PUAddPopupW(&ppd);

				// Anniversary
				mir_wstrncpy(ppd.lpwzContactName, TranslateT("Anniversary"), _countof(ppd.lpwzContactName));
				ppd.lchIcon = g_plugin.getIcon(IDI_RMD_DTAX);
				if (IsDlgButtonChecked(hDlg, CHECK_OPT_POPUP_WINCLR)) {
					ppd.colorBack = GetSysColor(COLOR_BTNFACE);
					ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
				}
				else if (IsDlgButtonChecked(hDlg, CHECK_OPT_POPUP_DEFCLR)) {
					ppd.colorBack = 0;
					ppd.colorText = 0;
				}
				else {
					ppd.colorBack = SendDlgItemMessage(hDlg, CLR_ABACK, CPM_GETCOLOUR, 0, 0);
					ppd.colorText = SendDlgItemMessage(hDlg, CLR_ATEXT, CPM_GETCOLOUR, 0, 0);
				}
				PUAddPopupW(&ppd);
			}
			break;

		case CHECK_OPT_POPUP_MSGBOX:
		case CHECK_OPT_POPUP_PROGRESS:
			if (bInitialized)
				NotifyParentOfChange(hDlg);
			break;

		case CHECK_OPT_POPUP_ENABLED:
			if (HIWORD(wParam) == BN_CLICKED) {
				const BOOL bEnabled = SendMessage((HWND)lParam, BM_GETCHECK, NULL, NULL);
				const int idCtrl[] = {
					CHECK_OPT_POPUP_DEFCLR, CHECK_OPT_POPUP_WINCLR,
					CLR_BBACK, TXT_OPT_POPUP_CLR_BACK,
					CLR_BTEXT, TXT_OPT_POPUP_CLR_TEXT,
					CHECK_OPT_POPUP_ADEFCLR, CHECK_OPT_POPUP_AWINCLR,
					CLR_ABACK, TXT_OPT_POPUP_CLR_ABACK,
					CLR_ATEXT, TXT_OPT_POPUP_CLR_ATEXT,
					RADIO_OPT_POPUP_DEFAULT, RADIO_OPT_POPUP_CUSTOM,
					RADIO_OPT_POPUP_PERMANENT, EDIT_DELAY
				};

				EnableControls(hDlg, idCtrl, _countof(idCtrl), bEnabled);

				if (bInitialized)
					NotifyParentOfChange(hDlg);
			}
			break;

		case CHECK_OPT_POPUP_DEFCLR:
		case CHECK_OPT_POPUP_WINCLR:
			if (HIWORD(wParam) == BN_CLICKED) {
				int bDefClr = IsDlgButtonChecked(hDlg, CHECK_OPT_POPUP_DEFCLR);
				int bWinClr = IsDlgButtonChecked(hDlg, CHECK_OPT_POPUP_WINCLR);

				EnableDlgItem(hDlg, CHECK_OPT_POPUP_DEFCLR, !bWinClr);
				EnableDlgItem(hDlg, CHECK_OPT_POPUP_WINCLR, !bDefClr);
				EnableDlgItem(hDlg, CLR_BBACK, !(bDefClr || bWinClr));
				EnableDlgItem(hDlg, TXT_OPT_POPUP_CLR_BACK, !(bDefClr || bWinClr));
				EnableDlgItem(hDlg, CLR_BTEXT, !(bDefClr || bWinClr));
				EnableDlgItem(hDlg, TXT_OPT_POPUP_CLR_TEXT, !(bDefClr || bWinClr));
				if (bInitialized)
					NotifyParentOfChange(hDlg);
			}
			break;

		case CHECK_OPT_POPUP_ADEFCLR:
		case CHECK_OPT_POPUP_AWINCLR:
			if (HIWORD(wParam) == BN_CLICKED) {
				int bDefClr = IsDlgButtonChecked(hDlg, CHECK_OPT_POPUP_ADEFCLR);
				int bWinClr = IsDlgButtonChecked(hDlg, CHECK_OPT_POPUP_AWINCLR);

				EnableDlgItem(hDlg, CHECK_OPT_POPUP_ADEFCLR, !bWinClr);
				EnableDlgItem(hDlg, CHECK_OPT_POPUP_AWINCLR, !bDefClr);
				EnableDlgItem(hDlg, CLR_ABACK, !(bDefClr || bWinClr));
				EnableDlgItem(hDlg, TXT_OPT_POPUP_CLR_ABACK, !(bDefClr || bWinClr));
				EnableDlgItem(hDlg, CLR_ATEXT, !(bDefClr || bWinClr));
				EnableDlgItem(hDlg, TXT_OPT_POPUP_CLR_ATEXT, !(bDefClr || bWinClr));
				if (bInitialized)
					NotifyParentOfChange(hDlg);
			}
			break;

		case RADIO_OPT_POPUP_DEFAULT:
			if (HIWORD(wParam) == BN_CLICKED) {
				EnableDlgItem(hDlg, EDIT_DELAY, FALSE);
				if (bInitialized)
					NotifyParentOfChange(hDlg);
			}
			break;

		case RADIO_OPT_POPUP_CUSTOM:
			if (HIWORD(wParam) == BN_CLICKED) {
				EnableDlgItem(hDlg, EDIT_DELAY, TRUE);
				if (bInitialized) 
					NotifyParentOfChange(hDlg);
			}
			break;

		case RADIO_OPT_POPUP_PERMANENT:
			if (HIWORD(wParam) == BN_CLICKED) {
				EnableDlgItem(hDlg, EDIT_DELAY, FALSE);
				if (bInitialized)
					NotifyParentOfChange(hDlg);
			}
			break;

		case EDIT_DELAY:
			if (bInitialized && HIWORD(wParam) == EN_UPDATE)
				NotifyParentOfChange(hDlg);
			break;

		default:
			if (bInitialized && HIWORD(wParam) == CPN_COLOURCHANGED)
				NotifyParentOfChange(hDlg);
		}
	}
	return FALSE;
}

/**
* This hook handler function is called on opening the options dialog
* to tell miranda, which pages userinfoex wants to add.
*
* @param	wParam			- options dialog's internal datastructure,
* @param	lParam			- not used
*
* @retval	MIR_OK
**/

int OnInitOptions(WPARAM wParam, LPARAM)
{
	DlgContactInfoInitTreeIcons();

	OPTIONSDIALOGPAGE odp = {};
	odp.position = 95400;
	odp.szTitle.a = MODULELONGNAME;
	odp.szGroup.a = LPGEN("Contacts");

	// Common page
	odp.szTab.a = LPGEN("Common");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_COMMON);
	odp.pfnDlgProc = DlgProc_CommonOpts;
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);

	// Advanced page
	odp.szTab.a = LPGEN("Advanced");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ADVANCED);
	odp.pfnDlgProc = DlgProc_AdvancedOpts;
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);

	// Details Dialog page
	odp.szTab.a = LPGEN("Details dialog");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_DETAILSDLG);
	odp.pfnDlgProc = DlgProc_DetailsDlgOpts;
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);

	// Reminder page
	odp.szTab.a = LPGEN("Reminder");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_REMINDER);
	odp.pfnDlgProc = DlgProc_ReminderOpts;
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);

	// Popups page
	odp.szTitle.a = MODULELONGNAME;
	odp.szGroup.a = LPGEN("Popups");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_POPUP);
	odp.pfnDlgProc = DlgProc_Popups;
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return MIR_OK;
}
