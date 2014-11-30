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

#include "commonheaders.h"

#define PSM_ENABLE_TABITEM	(WM_USER+106)

static MenuOptionsList ctrl_Menu[] = {
	{ SET_MI_MAIN,		CHECK_OPT_MI_MAIN,		RADIO_OPT_MI_MAIN_NONE,		RADIO_OPT_MI_MAIN_ALL,		RADIO_OPT_MI_MAIN_EXIMPORT },
	{ SET_MI_CONTACT,	CHECK_OPT_MI_CONTACT,	RADIO_OPT_MI_CONTACT_NONE,	RADIO_OPT_MI_CONTACT_ALL,	RADIO_OPT_MI_CONTACT_EXIMPORT },
	{ SET_MI_GROUP,		CHECK_OPT_MI_GROUP,		RADIO_OPT_MI_GROUP_NONE,	RADIO_OPT_MI_GROUP_ALL,		RADIO_OPT_MI_GROUP_EXIMPORT },
	{ SET_MI_SUBGROUP,	CHECK_OPT_MI_SUBGROUP,	RADIO_OPT_MI_SUBGROUP_NONE,	RADIO_OPT_MI_SUBGROUP_ALL,	RADIO_OPT_MI_SUBGROUP_EXIMPORT },
	{ SET_MI_ACCOUNT,	CHECK_OPT_MI_ACCOUNT,	RADIO_OPT_MI_ACCOUNT_NONE,	RADIO_OPT_MI_ACCOUNT_ALL,	RADIO_OPT_MI_ACCOUNT_EXIMPORT },
};

static FORCEINLINE void NotifyParentOfChange(HWND hDlg)
{
	SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
}

/**
 * Sends a PSN_INFOCHANGED notify to the handle.
 *
 * @param	hWnd					- the dialog's window handle
 *
 * @return	nothing
 **/
static void SendNotify_InfoChanged(HWND hDlg)
{
	PSHNOTIFY pshn;

	// send info changed message
	pshn.hdr.code = PSN_INFOCHANGED;
	SendMessage(hDlg, WM_NOTIFY, NULL, (LPARAM)&pshn);
}

static int FORCEINLINE ComboBox_FindByItemDataPtr(HWND hCombo, LPARAM pData)
{
	int nItemIndex;

	for (nItemIndex = ComboBox_GetCount(hCombo); (nItemIndex >= 0) && (ComboBox_GetItemData(hCombo, nItemIndex) != pData); nItemIndex--);
	return nItemIndex;
}

static void FORCEINLINE ComboBox_SetCurSelByItemDataPtr(HWND hCombo, LPARAM pData)
{
	ComboBox_SetCurSel(hCombo, ComboBox_FindByItemDataPtr(hCombo, pData));
}

static void FORCEINLINE ComboBox_AddItemWithData(HWND hCombo, LPTSTR ptszText, LPARAM pData)
{
	ComboBox_SetItemData(hCombo, ComboBox_AddString(hCombo, TranslateTS(ptszText)), pData);
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
static BYTE EnableDlgItem(HWND hDlg, const int idCtrl, BYTE bEnabled)
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
static BYTE InitialEnableControls(HWND hDlg, const int *idCtrl, int countCtrl, BYTE bEnabled)
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
static BYTE EnableControls(HWND hDlg, const int *idCtrl, int countCtrl, BYTE bEnabled)
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
static BYTE DBGetCheckBtn(HWND hDlg, const int idCtrl, LPCSTR pszSetting, BYTE bDefault)
{
	BYTE val = (db_get_b(NULL, MODNAME, pszSetting, bDefault) & 1) == 1;
	CheckDlgButton(hDlg, idCtrl, val);
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
static BYTE DBWriteCheckBtn(HWND hDlg, const int idCtrl, LPCSTR pszSetting)
{
	BYTE val = IsDlgButtonChecked(hDlg, idCtrl);
	int Temp = db_get_b(NULL, MODNAME, pszSetting, 0);
	Temp &= ~1;
	db_set_b(NULL, MODNAME, pszSetting, Temp |= val);
	return val;
}

/**
 * This function reads a DWORD from database and interprets it as an color value
 * to set to the color control.
 *
 * @param	hWnd			- the dialog's window handle
 * @param	idCtrl			- the dialog item's identifier
 * @param	pszSetting		- the setting from the database to use
 * @param	bDefault		- the default value to use, if no database setting exists
 *
 * @return	nothing
 **/
static void DBGetColor(HWND hDlg, const int idCtrl, LPCSTR pszSetting, DWORD bDefault)
{
	SendDlgItemMessage(hDlg, idCtrl, CPM_SETCOLOUR, 0, db_get_dw(NULL, MODNAME, pszSetting, bDefault));
}

/**
 * This function writes a DWORD to database according to the value
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
	db_set_dw(NULL, MODNAME, pszSetting, (DWORD)SendDlgItemMessage(hDlg, idCtrl, CPM_GETCOLOUR, 0, 0));
}

/**
 * This function writes a BYTE to database according to the value
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
static BYTE DBWriteEditByte(HWND hDlg, const int idCtrl, LPCSTR pszSetting, BYTE defVal)
{
	BYTE v;
	BOOL t;

	v = (BYTE)GetDlgItemInt(hDlg, idCtrl, &t, FALSE);
	if (t && (v != db_get_b(NULL, MODNAME, pszSetting, defVal)))
		return db_set_b(NULL, MODNAME, pszSetting, v) == 0;
	return FALSE;
}

/**
 * This function writes a WORD to database according to the value
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
static BYTE DBWriteEditWord(HWND hDlg, const int idCtrl, LPCSTR pszSetting, WORD defVal)
{
	WORD v;
	BOOL t;

	v = (WORD)GetDlgItemInt(hDlg, idCtrl, &t, FALSE);
	if (t && (v != db_get_w(NULL, MODNAME, pszSetting, defVal)))
		return db_set_w(NULL, MODNAME, pszSetting, v) == 0;
	return FALSE;
}

/**
 * This function writes a BYTE to database according to the currently
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
static BYTE DBWriteComboByte(HWND hDlg, const int idCtrl, LPCSTR pszSetting, BYTE defVal)
{
	BYTE v;

	v = (BYTE)SendDlgItemMessage(hDlg, idCtrl, CB_GETCURSEL, NULL, NULL);
	if (v != db_get_b(NULL, MODNAME, pszSetting, defVal))
		return db_set_b(NULL, MODNAME, pszSetting, v) == 0;
	return FALSE;
}

static INT_PTR CALLBACK DlgProc_CommonOpts(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static BYTE bInitialized = 0;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		ShowWindow(GetDlgItem(hDlg, CHECK_OPT_ZODIACAVATAR), SW_HIDE);
		SendNotify_InfoChanged(hDlg);
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_INFOCHANGED:
			bInitialized = 0;
			{
				// menu item settings
				for (int i = 0; i < SIZEOF(ctrl_Menu); i++) {
					int flag = db_get_b(NULL, MODNAME, ctrl_Menu[i].pszKey, 2);
					// check button and enable / disable control
					int idEnable[] = { ctrl_Menu[i].idCheckbox + 1, ctrl_Menu[i].idNONE, ctrl_Menu[i].idALL, ctrl_Menu[i].idEXIMPORT };
					EnableControls(hDlg, idEnable, SIZEOF(idEnable), DBGetCheckBtn(hDlg, ctrl_Menu[i].idCheckbox, ctrl_Menu[i].pszKey, 0));
					// set radio button state
					int id = ctrl_Menu[i].idNONE;	//default
					if ((flag & 4) == 4)
						id = ctrl_Menu[i].idALL;
					else if ((flag & 8) == 8)
						id = ctrl_Menu[i].idEXIMPORT;
					CheckRadioButton(hDlg, ctrl_Menu[i].idNONE, ctrl_Menu[i].idEXIMPORT, id);
				}
			}
			// extra icon settings
			CheckDlgButton(hDlg, CHECK_OPT_GENDER, g_eiGender);
			CheckDlgButton(hDlg, CHECK_OPT_EMAILICON, g_eiEmail);
			CheckDlgButton(hDlg, CHECK_OPT_PHONEICON, g_eiPhone);
			CheckDlgButton(hDlg, CHECK_OPT_HOMEPAGEICON, g_eiHome);
			CheckDlgButton(hDlg, CHECK_OPT_FLAGSUNKNOWN, g_bUseUnknownFlag);
			CheckDlgButton(hDlg, CHECK_OPT_FLAGSMSGSTATUS, g_bShowStatusIconFlag);

			// misc
			DBGetCheckBtn(hDlg, CHECK_OPT_ZODIACAVATAR, SET_ZODIAC_AVATARS, FALSE);

			bInitialized = 1;
			break;

		case PSN_APPLY:
			// menu item settings
			for (int i = 0; i < SIZEOF(ctrl_Menu); i++) {
				int flag = IsDlgButtonChecked(hDlg, ctrl_Menu[i].idCheckbox);
				flag |= IsDlgButtonChecked(hDlg, ctrl_Menu[i].idNONE) ? 2 : 0;
				flag |= IsDlgButtonChecked(hDlg, ctrl_Menu[i].idALL) ? 4 : 0;
				flag |= IsDlgButtonChecked(hDlg, ctrl_Menu[i].idEXIMPORT) ? 8 : 0;
				db_set_b(NULL, MODNAME, ctrl_Menu[i].pszKey, (BYTE)flag);
			}

			RebuildMenu();

			// extra icon settings
			bool FlagsClistChange = false, FlagsMsgWndChange = false;

			bool valNew = IsDlgButtonChecked(hDlg, CHECK_OPT_FLAGSUNKNOWN) != 0;
			if (g_bUseUnknownFlag != valNew) {
				g_bUseUnknownFlag = valNew;
				db_set_b(NULL, MODNAMEFLAGS, "UseUnknownFlag", valNew);
				FlagsClistChange = true;
				FlagsMsgWndChange = true;
			}
			valNew = IsDlgButtonChecked(hDlg, CHECK_OPT_FLAGSMSGSTATUS) != 0;
			if (g_bShowStatusIconFlag != valNew) {
				g_bShowStatusIconFlag = valNew;
				db_set_b(NULL, MODNAMEFLAGS, "ShowStatusIconFlag", valNew);
				FlagsMsgWndChange = true;
			}

			FlagsClistChange |= SvcHomepageEnableExtraIcons(0 != IsDlgButtonChecked(hDlg, CHECK_OPT_HOMEPAGEICON), true);
			FlagsClistChange |= SvcEMailEnableExtraIcons(0 != IsDlgButtonChecked(hDlg, CHECK_OPT_EMAILICON), true);
			FlagsClistChange |= SvcPhoneEnableExtraIcons(0 != IsDlgButtonChecked(hDlg, CHECK_OPT_PHONEICON), true);
			FlagsClistChange |= SvcGenderEnableExtraIcons(0 != IsDlgButtonChecked(hDlg, CHECK_OPT_GENDER), true);

			if (FlagsClistChange)
				pcli->pfnSetAllExtraIcons(NULL);
			if (FlagsMsgWndChange)
				UpdateStatusIcons();

			// misc
			BYTE bEnabled = IsDlgButtonChecked(hDlg, CHECK_OPT_ZODIACAVATAR);
			db_set_b(NULL, MODNAME, SET_ZODIAC_AVATARS, bEnabled);
			NServices::NAvatar::Enable(bEnabled);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case CHECK_OPT_MI_MAIN:
		case CHECK_OPT_MI_CONTACT:
		case CHECK_OPT_MI_GROUP:
		case CHECK_OPT_MI_SUBGROUP:
		case CHECK_OPT_MI_ACCOUNT:
			for (int i = 0; i < SIZEOF(ctrl_Menu); i++) {
				if (ctrl_Menu[i].idCheckbox == LOWORD(wParam)) {
					const int idMenuItems[] = { ctrl_Menu[i].idCheckbox + 1, ctrl_Menu[i].idNONE, ctrl_Menu[i].idALL, ctrl_Menu[i].idEXIMPORT };
					EnableControls(hDlg, idMenuItems, SIZEOF(idMenuItems), Button_GetCheck((HWND)lParam));
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
		case CHECK_OPT_HOMEPAGEICON:
		case CHECK_OPT_EMAILICON:
		case CHECK_OPT_PHONEICON:
		case CHECK_OPT_GENDER:
		case CHECK_OPT_FLAGSUNKNOWN:
		case CHECK_OPT_FLAGSMSGSTATUS:
		case CHECK_OPT_ZODIACAVATAR:
			if (bInitialized)
				NotifyParentOfChange(hDlg);
		}
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProc_AdvancedOpts(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static BYTE bInitialized = 0;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		SendNotify_InfoChanged(hDlg);
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_INFOCHANGED:
			bInitialized = 0;

			DBGetCheckBtn(hDlg, CHECK_OPT_ICOVERSION, SET_ICONS_CHECKFILEVERSION, TRUE);
			DBGetCheckBtn(hDlg, CHECK_OPT_BUTTONICONS, SET_ICONS_BUTTONS, TRUE);
			DBGetCheckBtn(hDlg, CHECK_OPT_METASCAN, SET_META_SCAN, TRUE);
			DBGetCheckBtn(hDlg, CHECK_OPT_SREMAIL_ENABLED, SET_EXTENDED_EMAILSERVICE, TRUE);
			if (tmi.getTimeZoneTime) {
				CheckDlgButton(hDlg, CHECK_OPT_AUTOTIMEZONE, TRUE);
				EnableWindow(GetDlgItem(hDlg, CHECK_OPT_AUTOTIMEZONE), FALSE);
			}
			else {
				DBGetCheckBtn(hDlg, CHECK_OPT_AUTOTIMEZONE, SET_OPT_AUTOTIMEZONE, TRUE);
			}

			bInitialized = 1;
			break;

		case PSN_APPLY:
			DBWriteCheckBtn(hDlg, CHECK_OPT_ICOVERSION, SET_ICONS_CHECKFILEVERSION);
			DBWriteCheckBtn(hDlg, CHECK_OPT_BUTTONICONS, SET_ICONS_BUTTONS);
			DBWriteCheckBtn(hDlg, CHECK_OPT_METASCAN, SET_META_SCAN);

			DBWriteCheckBtn(hDlg, CHECK_OPT_SREMAIL_ENABLED, SET_EXTENDED_EMAILSERVICE);
			if (!tmi.getTimeZoneTime) {
				DBWriteCheckBtn(hDlg, CHECK_OPT_AUTOTIMEZONE, SET_OPT_AUTOTIMEZONE);
				if (IsDlgButtonChecked(hDlg, CHECK_OPT_AUTOTIMEZONE))
					SvcTimezoneSyncWithWindows();
			}
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
			BYTE WantReset = MsgBox(hDlg,
				MB_ICON_WARNING | MB_YESNO,
				LPGENT("Question"),
				LPGENT("Reset factory defaults"),
				LPGENT("This will delete all settings, you've made!\nAll TreeView settings, window positions and any other settings!\n\nAre you sure to proceed?"));

			if (WantReset) {
				MCONTACT hContact;
				DB::CEnumList	Settings;

				// delete all skin icons
				if (!Settings.EnumSettings(NULL, "SkinIcons"))
					for (int i = 0; i < Settings.getCount(); i++) {
						LPSTR s = Settings[i];
						if (!mir_strnicmp(s, "UserInfoEx", 10))
							db_unset(NULL, "SkinIcons", s);
					}

				// delete global settings
				DB::Module::Delete(NULL, USERINFO"Ex");
				DB::Module::Delete(NULL, USERINFO"ExW");

				// delete old contactsettings
				for (hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact)) {
					db_unset(hContact, USERINFO, "PListColWidth0");
					db_unset(hContact, USERINFO, "PListColWidth1");
					db_unset(hContact, USERINFO, "PListColWidth2");
					db_unset(hContact, USERINFO, "EMListColWidth0");
					db_unset(hContact, USERINFO, "EMListColWidth1");
					db_unset(hContact, USERINFO, "BirthRemind");
					db_unset(hContact, USERINFO, "RemindBirthday");
					db_unset(hContact, USERINFO, "RemindDaysErlier");
					db_unset(hContact, USERINFO, "vCardPath");

					DB::Module::Delete(hContact, USERINFO"Ex");
					DB::Module::Delete(hContact, USERINFO"ExW");
				}

				SendMessage(GetParent(hDlg), PSM_FORCECHANGED, NULL, NULL);
				MsgBox(hDlg, MB_ICON_INFO,
					LPGENT("Ready"),
					LPGENT("Everything is done!"),
					LPGENT("All settings are reset to default values now!"));
			}
		}
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProc_DetailsDlgOpts(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static BYTE bInitialized = 0;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		SendNotify_InfoChanged(hDlg);
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_INFOCHANGED:
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
			break;

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

				EnableControls(hDlg, idCtrl, SIZEOF(idCtrl), bChecked);
			}

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
	static BYTE bInitialized = 0;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		ShowWindow(GetDlgItem(hDlg, CHECK_REMIND_SECURED), myGlobals.UseDbxTree ? SW_HIDE : SW_SHOW);

		SendDlgItemMessage(hDlg, ICO_BIRTHDAY, STM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_GetIcon(ICO_DLG_ANNIVERSARY, TRUE));

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
			ComboBox_AddString(hCtrl, TranslateT("Anniversaries only"));
			ComboBox_AddString(hCtrl, TranslateT("Birthdays only"));
			ComboBox_AddString(hCtrl, TranslateT("Everything"));
		}
		if (hCtrl = GetDlgItem(hDlg, EDIT_BIRTHMODULE)) {
			ComboBox_AddString(hCtrl, TranslateT("mBirthday"));
			ComboBox_AddString(hCtrl, TranslateT("UserInfo (default)"));
		}
		SendNotify_InfoChanged(hDlg);
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_INFOCHANGED:
			{
				bInitialized = 0;

				// set reminder options
				BYTE bEnabled = db_get_b(NULL, MODNAME, SET_REMIND_ENABLED, DEFVAL_REMIND_ENABLED);
				SendDlgItemMessage(hDlg, EDIT_REMIND_ENABLED, CB_SETCURSEL, bEnabled, NULL);
				DlgProc_ReminderOpts(hDlg, WM_COMMAND, MAKEWPARAM(EDIT_REMIND_ENABLED, CBN_SELCHANGE),
					(LPARAM)GetDlgItem(hDlg, EDIT_REMIND_ENABLED));

				DBGetCheckBtn(hDlg, CHECK_REMIND_MI, SET_REMIND_MENUENABLED, DEFVAL_REMIND_MENUENABLED);
				DBGetCheckBtn(hDlg, CHECK_REMIND_FLASHICON, SET_REMIND_FLASHICON, FALSE);
				DBGetCheckBtn(hDlg, CHECK_REMIND_VISIBLEONLY, SET_REMIND_CHECKVISIBLE, DEFVAL_REMIND_CHECKVISIBLE);
				DBGetCheckBtn(hDlg, CHECK_REMIND_STARTUP, SET_REMIND_CHECKON_STARTUP, FALSE);
				DBGetCheckBtn(hDlg, CHECK_REMIND_SECURED, SET_REMIND_SECUREBIRTHDAY, FALSE);

				SetDlgItemInt(hDlg, EDIT_REMIND, db_get_w(NULL, MODNAME, SET_REMIND_OFFSET, DEFVAL_REMIND_OFFSET), FALSE);
				SetDlgItemInt(hDlg, EDIT_REMIND_SOUNDOFFSET, db_get_b(NULL, MODNAME, SET_REMIND_SOUNDOFFSET, DEFVAL_REMIND_SOUNDOFFSET), FALSE);
				SetDlgItemInt(hDlg, EDIT_REMIND2, db_get_w(NULL, MODNAME, SET_REMIND_NOTIFYINTERVAL, DEFVAL_REMIND_NOTIFYINTERVAL), FALSE);

				SendDlgItemMessage(hDlg, EDIT_BIRTHMODULE, CB_SETCURSEL, db_get_b(NULL, MODNAME, SET_REMIND_BIRTHMODULE, DEFVAL_REMIND_BIRTHMODULE), NULL);

				MTime mtLast;
				TCHAR szTime[MAX_PATH];

				mtLast.DBGetStamp(NULL, MODNAME, SET_REMIND_LASTCHECK);
				mtLast.UTCToLocal();
				mtLast.TimeFormat(szTime, SIZEOF(szTime));

				SetDlgItemText(hDlg, TXT_REMIND_LASTCHECK, szTime);

				bInitialized = 1;
			}
			break;

		case PSN_APPLY:
			{
				BYTE bReminderCheck = FALSE;

				// save checkbox options
				DBWriteCheckBtn(hDlg, CHECK_REMIND_MI, SET_REMIND_MENUENABLED);
				DBWriteCheckBtn(hDlg, CHECK_REMIND_FLASHICON, SET_REMIND_FLASHICON);
				DBWriteCheckBtn(hDlg, CHECK_REMIND_VISIBLEONLY, SET_REMIND_CHECKVISIBLE);
				DBWriteCheckBtn(hDlg, CHECK_REMIND_STARTUP, SET_REMIND_CHECKON_STARTUP);
				DBWriteCheckBtn(hDlg, CHECK_REMIND_SECURED, SET_REMIND_SECUREBIRTHDAY);

				DBWriteEditByte(hDlg, EDIT_REMIND_SOUNDOFFSET, SET_REMIND_SOUNDOFFSET, DEFVAL_REMIND_SOUNDOFFSET);
				DBWriteEditWord(hDlg, EDIT_REMIND2, SET_REMIND_NOTIFYINTERVAL, DEFVAL_REMIND_NOTIFYINTERVAL);
				bReminderCheck = DBWriteEditWord(hDlg, EDIT_REMIND, SET_REMIND_OFFSET, DEFVAL_REMIND_OFFSET);

				// save primary birthday module
				BYTE bOld = db_get_b(NULL, MODNAME, SET_REMIND_BIRTHMODULE, DEFVAL_REMIND_BIRTHMODULE);  //    = 1
				BYTE bNew = (BYTE)ComboBox_GetCurSel(GetDlgItem(hDlg, EDIT_BIRTHMODULE));
				if (bOld != bNew) {
					// keep the database clean
					DBWriteComboByte(hDlg, EDIT_BIRTHMODULE, SET_REMIND_BIRTHMODULE, DEFVAL_REMIND_BIRTHMODULE);
					
					// walk through all the contacts stored in the DB
					MAnnivDate mdb;
					for (MCONTACT hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact))
						mdb.DBMoveBirthDate(hContact, bOld, bNew);
				}

				// update current reminder state
				BYTE bNewVal = (BYTE)SendDlgItemMessage(hDlg, EDIT_REMIND_ENABLED, CB_GETCURSEL, NULL, NULL);
				if (db_get_b(NULL, MODNAME, SET_REMIND_ENABLED, 1) != bNewVal) {
					db_set_b(NULL, MODNAME, SET_REMIND_ENABLED, bNewVal);
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
					TXT_REMIND2, TXT_REMIND3, TXT_REMIND4, TXT_REMIND6, TXT_REMIND7, TXT_REMIND8, TXT_REMIND9,
					TXT_REMIND_LASTCHECK, CHECK_REMIND_FLASHICON, EDIT_BIRTHMODULE, CHECK_REMIND_VISIBLEONLY,
					CHECK_REMIND_SECURED, CHECK_REMIND_STARTUP, EDIT_REMIND_SOUNDOFFSET, SPIN_REMIND_SOUNDOFFSET
				};

				EnableControls(hDlg, idCtrl, SIZEOF(idCtrl), bEnabled);
			}

		case EDIT_BIRTHMODULE:
			if (bInitialized && HIWORD(wParam) == CBN_SELCHANGE)
				NotifyParentOfChange(hDlg);
			break;

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
				WORD v = (WORD)GetDlgItemInt(hDlg, LOWORD(wParam), &t, FALSE);
				if (t && (v != db_get_w(NULL, MODNAME, SET_REMIND_OFFSET, DEFVAL_REMIND_OFFSET)))
					NotifyParentOfChange(hDlg);
			}
			break;

		// The user changes the number of days in advance of an anniversary to be notified by sound.
		case EDIT_REMIND_SOUNDOFFSET:
			if (bInitialized && HIWORD(wParam) == EN_UPDATE) {
				BOOL t;
				BYTE v = (BYTE)GetDlgItemInt(hDlg, LOWORD(wParam), &t, FALSE);
				if (t && (v != db_get_b(NULL, MODNAME, SET_REMIND_SOUNDOFFSET, DEFVAL_REMIND_SOUNDOFFSET)))
					NotifyParentOfChange(hDlg);
			}
			break;

		// The user changes the notification interval
		case EDIT_REMIND2:
			if (bInitialized && HIWORD(wParam) == EN_UPDATE) {
				BOOL t;
				WORD v = (WORD)GetDlgItemInt(hDlg, LOWORD(wParam), &t, FALSE);
				if (t && (v != db_get_w(NULL, MODNAME, SET_REMIND_NOTIFYINTERVAL, DEFVAL_REMIND_NOTIFYINTERVAL)))
					NotifyParentOfChange(hDlg);
			}
		}
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProc_Popups(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static BYTE bInitialized = 0;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		SendNotify_InfoChanged(hDlg);
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_INFOCHANGED:
			{
				BYTE bDelay, isEnabled;

				bInitialized = 0;

				DBGetCheckBtn(hDlg, CHECK_OPT_POPUP_MSGBOX, SET_POPUPMSGBOX, DEFVAL_POPUPMSGBOX);
				DBGetCheckBtn(hDlg, CHECK_OPT_POPUP_PROGRESS, "PopupProgress", FALSE);
				// disable if popup plugin dos not sopport buttons inside popop
				if (!myGlobals.PopupActionsExist) {
					EnableDlgItem(hDlg, CHECK_OPT_POPUP_MSGBOX, FALSE);
					EnableDlgItem(hDlg, CHECK_OPT_POPUP_PROGRESS, FALSE);
				}
				else if (!(db_get_dw(0, "Popup","Actions", 0) & 1))
					EnableDlgItem(hDlg, CHECK_OPT_POPUP_MSGBOX, FALSE);

				// enable/disable popups
				isEnabled = DBGetCheckBtn(hDlg, CHECK_OPT_POPUP_ENABLED, SET_POPUP_ENABLED, DEFVAL_POPUP_ENABLED);
				SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(CHECK_OPT_POPUP_ENABLED, BN_CLICKED), (LPARAM)GetDlgItem(hDlg, CHECK_OPT_POPUP_ENABLED));

				// set colortype checkboxes and color controls
				DBGetColor(hDlg, CLR_BBACK, SET_POPUP_BIRTHDAY_COLOR_BACK, RGB(192, 180, 30));
				DBGetColor(hDlg, CLR_BTEXT, SET_POPUP_BIRTHDAY_COLOR_TEXT, RGB(0, 0, 0));
				switch (db_get_b(NULL, MODNAME, SET_POPUP_BIRTHDAY_COLORTYPE, POPUP_COLOR_CUSTOM)) {
				case POPUP_COLOR_DEFAULT:
					CheckDlgButton(hDlg, CHECK_OPT_POPUP_DEFCLR, TRUE);
					break;

				case POPUP_COLOR_WINDOWS:
					CheckDlgButton(hDlg, CHECK_OPT_POPUP_WINCLR, TRUE);
				}

				DBGetColor(hDlg, CLR_ABACK, SET_POPUP_ANNIVERSARY_COLOR_BACK, RGB(90, 190, 130));
				DBGetColor(hDlg, CLR_ATEXT, SET_POPUP_ANNIVERSARY_COLOR_TEXT, RGB(0, 0, 0));
				switch (db_get_b(NULL, MODNAME, SET_POPUP_ANNIVERSARY_COLORTYPE, POPUP_COLOR_CUSTOM)) {
				case POPUP_COLOR_DEFAULT:
					CheckDlgButton(hDlg, CHECK_OPT_POPUP_ADEFCLR, TRUE);
					break;
				case POPUP_COLOR_WINDOWS:
					CheckDlgButton(hDlg, CHECK_OPT_POPUP_AWINCLR, TRUE);
				}

				if (isEnabled) {
					SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(CHECK_OPT_POPUP_DEFCLR, BN_CLICKED), NULL);
					SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(CHECK_OPT_POPUP_ADEFCLR, BN_CLICKED), NULL);
				}
				// set delay values
				bDelay = db_get_b(NULL, MODNAME, SET_POPUP_DELAY, 0);
				switch (bDelay) {
				case 0:
					CheckDlgButton(hDlg, RADIO_OPT_POPUP_DEFAULT, TRUE);
					if (isEnabled)
						EnableDlgItem(hDlg, EDIT_DELAY, FALSE);
					break;

				case 255:
					CheckDlgButton(hDlg, RADIO_OPT_POPUP_PERMANENT, TRUE);
					if (isEnabled)
						EnableDlgItem(hDlg, EDIT_DELAY, FALSE);
					break;

				default:
					CheckDlgButton(hDlg, RADIO_OPT_POPUP_CUSTOM, TRUE);
					SetDlgItemInt(hDlg, EDIT_DELAY, bDelay, FALSE);
				}
				bInitialized = TRUE;
			}
			break;

		case PSN_APPLY:
			DBWriteCheckBtn(hDlg, CHECK_OPT_POPUP_MSGBOX, SET_POPUPMSGBOX);
			DBWriteCheckBtn(hDlg, CHECK_OPT_POPUP_PROGRESS, "PopupProgress");
			DBWriteCheckBtn(hDlg, CHECK_OPT_POPUP_ENABLED, SET_POPUP_ENABLED);

			// save popup style for birthdays
			DBWriteColor(hDlg, CLR_BBACK, SET_POPUP_BIRTHDAY_COLOR_BACK);
			DBWriteColor(hDlg, CLR_BTEXT, SET_POPUP_BIRTHDAY_COLOR_TEXT);
			db_set_b(NULL, MODNAME, SET_POPUP_BIRTHDAY_COLORTYPE, 
				SendDlgItemMessage(hDlg, CHECK_OPT_POPUP_DEFCLR, BM_GETCHECK, NULL, NULL)
				? POPUP_COLOR_DEFAULT
				: SendDlgItemMessage(hDlg, CHECK_OPT_POPUP_WINCLR, BM_GETCHECK, NULL, NULL)
				? POPUP_COLOR_WINDOWS
				: POPUP_COLOR_CUSTOM);

			// save popup style for anniversaries
			DBWriteColor(hDlg, CLR_ABACK, SET_POPUP_ANNIVERSARY_COLOR_BACK);
			DBWriteColor(hDlg, CLR_ATEXT, SET_POPUP_ANNIVERSARY_COLOR_TEXT);
			db_set_b(NULL, MODNAME, SET_POPUP_ANNIVERSARY_COLORTYPE, 
				SendDlgItemMessage(hDlg, CHECK_OPT_POPUP_ADEFCLR, BM_GETCHECK, NULL, NULL)
				? POPUP_COLOR_DEFAULT
				: SendDlgItemMessage(hDlg, CHECK_OPT_POPUP_AWINCLR, BM_GETCHECK, NULL, NULL)
				? POPUP_COLOR_WINDOWS
				: POPUP_COLOR_CUSTOM);

			// save delay
			if (SendDlgItemMessage(hDlg, RADIO_OPT_POPUP_PERMANENT, BM_GETCHECK, NULL, NULL))
				db_set_b(NULL, MODNAME, SET_POPUP_DELAY, 255);
			else if (SendDlgItemMessage(hDlg, RADIO_OPT_POPUP_CUSTOM, BM_GETCHECK, NULL, NULL)) {
				TCHAR szDelay[4];
				GetDlgItemText(hDlg, EDIT_DELAY, szDelay, SIZEOF(szDelay));
				db_set_b(NULL, MODNAME, SET_POPUP_DELAY, (BYTE)_tcstol(szDelay, NULL, 10));
			}
			else
				db_unset(NULL, MODNAME, SET_POPUP_DELAY);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case BTN_PREVIEW:
			{
				POPUPDATAT ppd = { 0 };
				ppd.iSeconds = (int)db_get_b(NULL, MODNAME, SET_POPUP_DELAY, 0);
				mir_tstrncpy(ppd.lptzText, TranslateT("This is the reminder message"), MAX_SECONDLINE);

				// Birthday
				mir_tstrncpy(ppd.lptzContactName, TranslateT("Birthday"), SIZEOF(ppd.lptzContactName));
				ppd.lchIcon = Skin_GetIcon(ICO_RMD_DTB0);
				if (IsDlgButtonChecked(hDlg, CHECK_OPT_POPUP_WINCLR)) {
					ppd.colorBack = GetSysColor(COLOR_BTNFACE);
					ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
				}
				else if (!IsDlgButtonChecked(hDlg, CHECK_OPT_POPUP_DEFCLR)) {
					ppd.colorBack = SendDlgItemMessage(hDlg, CLR_BBACK, CPM_GETCOLOUR, 0, 0);
					ppd.colorText = SendDlgItemMessage(hDlg, CLR_BTEXT, CPM_GETCOLOUR, 0, 0);
				}
				PUAddPopupT(&ppd);

				// Anniversary
				mir_tstrncpy(ppd.lptzContactName, TranslateT("Anniversary"), SIZEOF(ppd.lptzContactName));
				ppd.lchIcon = Skin_GetIcon(ICO_RMD_DTAX);
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
				PUAddPopupT(&ppd);
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

				EnableControls(hDlg, idCtrl, SIZEOF(idCtrl), bEnabled);

				if (bInitialized)
					NotifyParentOfChange(hDlg);
			}
			break;

		case CHECK_OPT_POPUP_DEFCLR:
		case CHECK_OPT_POPUP_WINCLR:
			if (HIWORD(wParam) == BN_CLICKED) {
				int bDefClr = SendDlgItemMessage(hDlg, CHECK_OPT_POPUP_DEFCLR, BM_GETCHECK, NULL, NULL);
				int bWinClr = SendDlgItemMessage(hDlg, CHECK_OPT_POPUP_WINCLR, BM_GETCHECK, NULL, NULL);

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
				int bDefClr = SendDlgItemMessage(hDlg, CHECK_OPT_POPUP_ADEFCLR, BM_GETCHECK, NULL, NULL);
				int bWinClr = SendDlgItemMessage(hDlg, CHECK_OPT_POPUP_AWINCLR, BM_GETCHECK, NULL, NULL);

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

static int OnInitOptions(WPARAM wParam, LPARAM lParam)
{
	DlgContactInfoInitTreeIcons();

	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 95400;
	odp.hInstance = ghInst;
	odp.pszTitle = LPGEN(MODULELONGNAME);
	odp.pszGroup = LPGEN("Contacts");
	odp.cbSize = sizeof(odp);

	// Common page
	odp.pszTab = LPGEN("Common");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_COMMON);
	odp.pfnDlgProc = DlgProc_CommonOpts;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);

	// Advanced page
	odp.pszTab = LPGEN("Advanced");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ADVANCED);
	odp.pfnDlgProc = DlgProc_AdvancedOpts;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);

	// Details Dialog page
	odp.pszTab = LPGEN("Details dialog");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_DETAILSDLG);
	odp.pfnDlgProc = DlgProc_DetailsDlgOpts;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);

	// Reminder page
	odp.pszTab = LPGEN("Reminder");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_REMINDER);
	odp.pfnDlgProc = DlgProc_ReminderOpts;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);

	// Popups page
	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		odp.pszTitle = LPGEN(MODULELONGNAME);
		odp.pszGroup = LPGEN("Popups");
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_POPUP);
		odp.pfnDlgProc = DlgProc_Popups;
		odp.flags = ODPF_BOLDGROUPS;
		Options_AddPage(wParam, &odp);
	}
	return MIR_OK;
}

/**
* This function loads the options module.
*
* @param	none
*
* @retval	nothing
**/

void OptionsLoadModule()
{
	HookEvent(ME_OPT_INITIALISE, OnInitOptions);
}
