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

/////////////////////////////////////////////////////////////////////////////////////////
// Common options dialog

struct
{
	LPCSTR pszKey;
	int idCheckbox;
	int idNONE;
	int idALL;
	int idEXIMPORT;
}
static ctrl_Menu[] = {
	{ SET_MI_MAIN,     CHECK_OPT_MI_MAIN,     RADIO_OPT_MI_MAIN_NONE,     RADIO_OPT_MI_MAIN_ALL,     RADIO_OPT_MI_MAIN_EXIMPORT     },
	{ SET_MI_CONTACT,  CHECK_OPT_MI_CONTACT,  RADIO_OPT_MI_CONTACT_NONE,  RADIO_OPT_MI_CONTACT_ALL,  RADIO_OPT_MI_CONTACT_EXIMPORT  },
	{ SET_MI_GROUP,    CHECK_OPT_MI_GROUP,    RADIO_OPT_MI_GROUP_NONE,    RADIO_OPT_MI_GROUP_ALL,    RADIO_OPT_MI_GROUP_EXIMPORT    },
	{ SET_MI_SUBGROUP, CHECK_OPT_MI_SUBGROUP, RADIO_OPT_MI_SUBGROUP_NONE, RADIO_OPT_MI_SUBGROUP_ALL, RADIO_OPT_MI_SUBGROUP_EXIMPORT },
	{ SET_MI_ACCOUNT,  CHECK_OPT_MI_ACCOUNT,  RADIO_OPT_MI_ACCOUNT_NONE,  RADIO_OPT_MI_ACCOUNT_ALL,  RADIO_OPT_MI_ACCOUNT_EXIMPORT  },
};

class CCommonOptsDlg : public CDlgBase
{
	CCtrlCheck chk1, chk2, chk3, chk4, chk5, chkFlagUnknown, chkStatusIcon;

public:
	CCommonOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_COMMON),
		chk1(this, CHECK_OPT_MI_MAIN),
		chk2(this, CHECK_OPT_MI_CONTACT),
		chk3(this, CHECK_OPT_MI_GROUP),
		chk4(this, CHECK_OPT_MI_SUBGROUP),
		chk5(this, CHECK_OPT_MI_ACCOUNT),
		chkStatusIcon(this, CHECK_OPT_FLAGSMSGSTATUS),
		chkFlagUnknown(this, CHECK_OPT_FLAGSUNKNOWN)
	{
		CreateLink(chkStatusIcon, g_plugin.bShowStatusIconFlag);
		CreateLink(chkFlagUnknown, g_plugin.bUseUnknownFlag);

		chk1.OnChange = chk2.OnChange = chk3.OnChange = chk4.OnChange = chk5.OnChange = Callback(this, &CCommonOptsDlg::onChange_Root);
	}

	bool OnInitDialog() override
	{
		// menu item settings
		for (auto &it : ctrl_Menu) {
			int flag = g_plugin.getByte(it.pszKey, 2);

			// check button and enable / disable control
			DBGetCheckBtn(m_hwnd, it.idCheckbox, it.pszKey, 0);

			// set radio button state
			int id = it.idNONE;	//default
			if ((flag & 4) == 4)
				id = it.idALL;
			else if ((flag & 8) == 8)
				id = it.idEXIMPORT;
			CheckRadioButton(m_hwnd, it.idNONE, it.idEXIMPORT, id);
		}
		return true;
	}

	bool OnApply() override
	{
		// menu item settings
		for (auto &it : ctrl_Menu) {
			int flag = IsDlgButtonChecked(m_hwnd, it.idCheckbox);
			flag |= IsDlgButtonChecked(m_hwnd, it.idNONE) ? 2 : 0;
			flag |= IsDlgButtonChecked(m_hwnd, it.idALL) ? 4 : 0;
			flag |= IsDlgButtonChecked(m_hwnd, it.idEXIMPORT) ? 8 : 0;
			g_plugin.setByte(it.pszKey, (uint8_t)flag);
		}

		RebuildMenu();

		// extra icon settings
		bool FlagsClistChange = false, FlagsMsgWndChange = false;

		if (chkFlagUnknown.IsChanged()) {
			FlagsClistChange = true;
			FlagsMsgWndChange = true;
		}

		if (chkStatusIcon.IsChanged())
			FlagsMsgWndChange = true;

		if (FlagsClistChange)
			ExtraIcon_SetAll();
		if (FlagsMsgWndChange)
			UpdateStatusIcons();
		return true;
	}

	void onChange_Root(CCtrlCheck *pCheck)
	{
		for (auto &it : ctrl_Menu) {
			if (it.idCheckbox == pCheck->GetCtrlId()) {
				const int idMenuItems[] = { it.idCheckbox + 1, it.idNONE, it.idALL, it.idEXIMPORT };
				EnableControls(m_hwnd, idMenuItems, _countof(idMenuItems), pCheck->IsChecked());
				break;
			}
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Advanced options dialog

class CAdvancedOptsDlg : public CDlgBase
{
	CCtrlCheck chkVersion, chkMetaScan, chkButtonIcons, chkEmail;
	CCtrlButton btnReset;

public:
	CAdvancedOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_ADVANCED),
		btnReset(this, BTN_OPT_RESET),
		chkEmail(this, CHECK_OPT_SREMAIL_ENABLED),
		chkVersion(this, CHECK_OPT_ICOVERSION),
		chkMetaScan(this, CHECK_OPT_METASCAN),
		chkButtonIcons(this, CHECK_OPT_BUTTONICONS)
	{
		CreateLink(chkEmail, g_plugin.bEmailService);
		CreateLink(chkVersion, g_plugin.bCheckVersion);
		CreateLink(chkMetaScan, g_plugin.bMetaScan);
		CreateLink(chkButtonIcons, g_plugin.bButtonIcons);

		btnReset.OnClick = Callback(this, &CAdvancedOptsDlg::onClick_Reset);
	}

	void onClick_Reset(CCtrlButton *)
	{
		uint8_t WantReset = MsgBox(m_hwnd,
			MB_ICON_WARNING | MB_YESNO,
			LPGENW("Question"),
			LPGENW("Reset factory defaults"),
			LPGENW("This will delete all settings, you've made!\nAll TreeView settings, window positions and any other settings!\n\nAre you sure to proceed?"));

		if (!WantReset)
			return;

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

		SendMessage(m_hwndParent, PSM_FORCECHANGED, NULL, NULL);
		MsgBox(m_hwnd, MB_ICON_INFO, LPGENW("Ready"), LPGENW("Everything is done!"), LPGENW("All settings are reset to default values now!"));
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Details options dialog

class CDetailsOptsDlg : public CDlgBase
{
	CCtrlCheck chkClr, chkGroups, chkSortTree, chkAero, chkReadonly, chkChange;
	CCtrlColor clrNormal, clrCustom, clrBoth, clrChanged, clrMeta;

public:
	CDetailsOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_DETAILSDLG),
		chkClr(this, CHECK_OPT_CLR),
		chkAero(this, CHECK_OPT_AEROADAPTION),
		chkGroups(this, CHECK_OPT_GROUPS),
		chkChange(this, CHECK_OPT_CHANGEMYDETAILS),
		chkReadonly(this, CHECK_OPT_READONLY),
		chkSortTree(this, CHECK_OPT_SORTTREE),
		
		clrBoth(this, CLR_BOTH),
		clrMeta(this, CLR_META),
		clrNormal(this, CLR_NORMAL),
		clrCustom(this, CLR_USER),
		clrChanged(this, CLR_CHANGED)
	{
		CreateLink(chkClr, g_plugin.bShowColours);
		CreateLink(chkAero, g_plugin.bAero);
		CreateLink(chkGroups, g_plugin.bTreeGroups);
		CreateLink(chkChange, g_plugin.bChangeDetails);
		CreateLink(chkReadonly, g_plugin.bReadOnly);
		CreateLink(chkSortTree, g_plugin.bSortTree);

		CreateLink(clrBoth, g_plugin.clrBoth);
		CreateLink(clrMeta, g_plugin.clrMeta);
		CreateLink(clrNormal, g_plugin.clrNormal);
		CreateLink(clrCustom, g_plugin.clrCustom);
		CreateLink(clrChanged, g_plugin.clrChanged);

		chkClr.OnChange = Callback(this, &CDetailsOptsDlg::onChange_Clr);
	}

	bool OnInitDialog() override
	{
		chkChange.Enable(myGlobals.CanChangeDetails);
		return true;
	}

	void onChange_Clr(CCtrlCheck *pCheck)
	{
		const int idCtrl[] = { CLR_NORMAL, CLR_USER, CLR_BOTH, CLR_CHANGED, CLR_META, TXT_OPT_CLR_NORMAL,
			TXT_OPT_CLR_USER, TXT_OPT_CLR_BOTH, TXT_OPT_CLR_CHANGED, TXT_OPT_CLR_META };

		EnableControls(m_hwnd, idCtrl, _countof(idCtrl), pCheck->IsChecked());
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Reminder options dialog

class CReminderOptsDlg : public CDlgBase
{
	CCtrlSpin spin1, spin2, spinOffset;
	CCtrlCheck chkVisible, chkFlash, chkStartup, chkMenu;
	CCtrlCombo cmbEnabled;

public:
	CReminderOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_REMINDER),
		cmbEnabled(this, EDIT_REMIND_ENABLED),
		chkMenu(this, CHECK_REMIND_MI),
		chkFlash(this, CHECK_REMIND_FLASHICON),
		chkStartup(this, CHECK_REMIND_STARTUP),
		chkVisible(this, CHECK_REMIND_VISIBLEONLY),
		spin1(this, SPIN_REMIND, 50),
		spin2(this, SPIN_REMIND2, 8760, 1),
		spinOffset(this, SPIN_REMIND_SOUNDOFFSET, 50)
	{
		CreateLink(chkMenu, g_plugin.bRemindMenuEnabled);
		CreateLink(chkFlash, g_plugin.bRemindFlashIcon);
		CreateLink(chkStartup, g_plugin.bRemindStartupCheck);
		CreateLink(chkVisible, g_plugin.bRemindCheckVisible);

		CreateLink(spin1, g_plugin.wRemindOffset);
		CreateLink(spin2, g_plugin.wRemindNotifyInterval);
		CreateLink(spinOffset, g_plugin.wRemindSoundOffset);

		cmbEnabled.OnSelChanged = Callback(this, &CReminderOptsDlg::onChange_Enabled);
	}

	bool OnInitDialog() override
	{
		SendDlgItemMessage(m_hwnd, ICO_BIRTHDAY, STM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(IDI_ANNIVERSARY, true));

		// set colours			
		cmbEnabled.AddString(TranslateT("Reminder disabled"));
		cmbEnabled.AddString(TranslateT("Birthdays only"));
		cmbEnabled.AddString(TranslateT("Anniversaries only"));
		cmbEnabled.AddString(TranslateT("Everything"));

		// set reminder options
		cmbEnabled.SetCurSel(g_plugin.iRemindState);
		
		MTime mtLast;
		wchar_t szTime[MAX_PATH];

		mtLast.DBGetStamp(0, MODULENAME, SET_REMIND_LASTCHECK);
		mtLast.UTCToLocal();
		mtLast.TimeFormat(szTime, _countof(szTime));

		SetDlgItemText(m_hwnd, TXT_REMIND_LASTCHECK, szTime);
		return true;
	}

	bool OnApply() override
	{
		// update current reminder state
		uint8_t bNewVal = (uint8_t)cmbEnabled.GetCurSel();
		if (g_plugin.iRemindState != bNewVal) {
			g_plugin.iRemindState = bNewVal;
			if (bNewVal == REMIND_OFF)
				SvcReminderEnable(false);
			else {
				// update all contact list extra icons
				SvcReminderEnable(true); // reinit reminder options from db
				SvcReminderCheckAll(NOTIFY_CLIST); // notify
			}
		}

		RebuildMain();
		return true;
	}

	void onChange_Enabled(CCtrlCombo *pCombo)
	{
		const int idCtrl[] = {
			CHECK_REMIND_MI, EDIT_REMIND, EDIT_REMIND2, SPIN_REMIND, SPIN_REMIND2, TXT_REMIND,
			TXT_REMIND2, TXT_REMIND3, TXT_REMIND4, TXT_REMIND6, TXT_REMIND8, TXT_REMIND9,
			TXT_REMIND_LASTCHECK, CHECK_REMIND_FLASHICON, CHECK_REMIND_VISIBLEONLY,
			CHECK_REMIND_STARTUP, EDIT_REMIND_SOUNDOFFSET, SPIN_REMIND_SOUNDOFFSET
		};

		EnableControls(m_hwnd, idCtrl, _countof(idCtrl), pCombo->GetCurSel() > 0);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Popup options dialog

class CPopupOptsDlg : public CDlgBase
{
	CCtrlCheck chkEnabled, chkMsgbox, chkDefClr, chkWinClr, chkADefClr, chkAWinClr, chkProgress;
	CCtrlCheck chkDefault, chkCustom, chkPermanent;
	CCtrlColor clrAback, clrAtext, clrBback, clrBtext;
	CCtrlButton btnPreview;

public:
	CPopupOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_POPUP),
		btnPreview(this, BTN_PREVIEW),
		chkDefClr(this, CHECK_OPT_POPUP_DEFCLR),
		chkWinClr(this, CHECK_OPT_POPUP_WINCLR),
		chkADefClr(this, CHECK_OPT_POPUP_ADEFCLR),
		chkAWinClr(this, CHECK_OPT_POPUP_AWINCLR),
		chkEnabled(this, CHECK_OPT_POPUP_ENABLED),
		chkMsgbox(this, CHECK_OPT_POPUP_MSGBOX),
		chkProgress(this, CHECK_OPT_POPUP_PROGRESS),
		chkCustom(this, RADIO_OPT_POPUP_CUSTOM),
		chkDefault(this, RADIO_OPT_POPUP_DEFAULT),
		chkPermanent(this, RADIO_OPT_POPUP_PERMANENT),
		clrAback(this, CLR_ABACK),
		clrAtext(this, CLR_ATEXT),
		clrBback(this, CLR_BBACK),
		clrBtext(this, CLR_BTEXT)
	{
		CreateLink(chkMsgbox, g_plugin.bPopupMsgbox);
		CreateLink(chkEnabled, g_plugin.bPopupEnabled);
		CreateLink(chkProgress, g_plugin.bPopupProgress);

		CreateLink(clrAback, g_plugin.clrAback);
		CreateLink(clrAtext, g_plugin.clrAtext);
		CreateLink(clrBback, g_plugin.clrBback);
		CreateLink(clrBtext, g_plugin.clrBtext);

		btnPreview.OnClick = Callback(this, &CPopupOptsDlg::onClick_Preview);

		chkEnabled.OnChange = Callback(this, &CPopupOptsDlg::onChange_Enabled);
		chkDefClr.OnChange = chkWinClr.OnChange = Callback(this, &CPopupOptsDlg::onChange_Clr);
		chkADefClr.OnChange = chkAWinClr.OnChange = Callback(this, &CPopupOptsDlg::onChange_AnnivClr);
		chkDefault.OnChange = chkCustom.OnChange = chkPermanent.OnChange = Callback(this, &CPopupOptsDlg::onChange_Delay);
	}

	bool OnInitDialog() override
	{
		// disable if popup plugin dos not sopport buttons inside popop
		if (!(db_get_dw(0, "Popup", "Actions", 0) & 1))
			chkMsgbox.Disable();
		
		// set colortype checkboxes and color controls
		switch (g_plugin.iBirthClrType) {
		case POPUP_COLOR_DEFAULT:
			chkDefClr.SetState(true);
			break;
		case POPUP_COLOR_WINDOWS:
			chkWinClr.SetState(true);
		}

		switch (g_plugin.iAnnivClrType) {
		case POPUP_COLOR_DEFAULT:
			chkADefClr.SetState(true);
			break;
		case POPUP_COLOR_WINDOWS:
			chkAWinClr.SetState(true);
		}

		// set delay values
		uint8_t bDelay = g_plugin.iPopupDelay;
		switch (bDelay) {
		case 0:
			chkDefault.SetState(true);
			break;

		case 255:
			chkPermanent.SetState(true);
			break;

		default:
			chkCustom.SetState(true);
			SetDlgItemInt(m_hwnd, EDIT_DELAY, bDelay, FALSE);
		}
		return true;
	}

	bool OnApply() override
	{
		// save popup style for birthdays
		g_plugin.iBirthClrType = chkDefClr.IsChecked() ? POPUP_COLOR_DEFAULT : (chkWinClr.IsChecked() ? POPUP_COLOR_WINDOWS : POPUP_COLOR_CUSTOM);

		// save popup style for anniversaries
		g_plugin.iAnnivClrType = chkADefClr.IsChecked() ? POPUP_COLOR_DEFAULT : (chkAWinClr.IsChecked() ? POPUP_COLOR_WINDOWS : POPUP_COLOR_CUSTOM);

		// save delay
		if (chkPermanent.IsChecked())
			g_plugin.iPopupDelay = 255;
		else if (chkCustom.IsChecked()) {
			wchar_t szDelay[4];
			GetDlgItemText(m_hwnd, EDIT_DELAY, szDelay, _countof(szDelay));
			g_plugin.iPopupDelay = (uint8_t)wcstol(szDelay, nullptr, 10);
		}
		else g_plugin.iPopupDelay.Delete();

		return true;
	}

	void onClick_Preview(CCtrlButton*)
	{
		POPUPDATAW ppd = {};
		ppd.iSeconds = g_plugin.iPopupDelay;
		mir_wstrncpy(ppd.lpwzText, TranslateT("This is the reminder message"), MAX_SECONDLINE);

		// Birthday
		mir_wstrncpy(ppd.lpwzContactName, TranslateT("Birthday"), _countof(ppd.lpwzContactName));
		ppd.lchIcon = g_plugin.getIcon(IDI_RMD_DTB0);
		if (chkWinClr.IsChecked()) {
			ppd.colorBack = GetSysColor(COLOR_BTNFACE);
			ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
		}
		else if (!chkDefClr.IsChecked()) {
			ppd.colorBack = clrBback.GetColor();
			ppd.colorText = clrBtext.GetColor();
		}
		PUAddPopupW(&ppd);

		// Anniversary
		mir_wstrncpy(ppd.lpwzContactName, TranslateT("Anniversary"), _countof(ppd.lpwzContactName));
		ppd.lchIcon = g_plugin.getIcon(IDI_RMD_DTAX);
		if (chkAWinClr.IsChecked()) {
			ppd.colorBack = GetSysColor(COLOR_BTNFACE);
			ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
		}
		else if (!chkDefClr.IsChecked()) {
			ppd.colorBack = clrAback.GetColor();
			ppd.colorText = clrAtext.GetColor();
		}
		PUAddPopupW(&ppd);
	}

	void onChange_Enabled(CCtrlCheck *pCheck)
	{
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

		EnableControls(m_hwnd, idCtrl, _countof(idCtrl), pCheck->GetState());
	}

	void onChange_Clr(CCtrlCheck *)
	{
		bool bDefClr = chkDefClr.IsChecked();
		bool bWinClr = chkWinClr.IsChecked();

		chkDefClr.Enable(!bWinClr);
		chkWinClr.Enable(!bDefClr);

		clrBback.Enable(!bDefClr && !bWinClr);
		clrBtext.Enable(!bDefClr && !bWinClr);

		EnableDlgItem(m_hwnd, TXT_OPT_POPUP_CLR_BACK, !(bDefClr || bWinClr));
		EnableDlgItem(m_hwnd, TXT_OPT_POPUP_CLR_TEXT, !(bDefClr || bWinClr));
	}

	void onChange_AnnivClr(CCtrlCheck *)
	{
		int bDefClr = chkADefClr.IsChecked();
		int bWinClr = chkAWinClr.IsChecked();

		chkADefClr.Enable(!bWinClr);
		chkAWinClr.Enable(!bDefClr);

		clrAback.Enable(!bDefClr && !bWinClr);
		clrAtext.Enable(!bDefClr && !bWinClr);

		EnableDlgItem(m_hwnd, TXT_OPT_POPUP_CLR_ABACK, !(bDefClr || bWinClr));
		EnableDlgItem(m_hwnd, TXT_OPT_POPUP_CLR_ATEXT, !(bDefClr || bWinClr));
	}

	void onChange_Delay(CCtrlCheck *)
	{
		EnableDlgItem(m_hwnd, EDIT_DELAY, chkCustom.IsChecked());
	}
};

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
	odp.flags = ODPF_BOLDGROUPS;
	odp.szTitle.a = MODULELONGNAME;
	odp.szGroup.a = LPGEN("Contacts");

	// Common page
	odp.szTab.a = LPGEN("Common");
	odp.pDialog = new CCommonOptsDlg();
	g_plugin.addOptions(wParam, &odp);

	// Advanced page
	odp.szTab.a = LPGEN("Advanced");
	odp.pDialog = new CAdvancedOptsDlg();
	g_plugin.addOptions(wParam, &odp);

	// Details Dialog page
	odp.szTab.a = LPGEN("Details dialog");
	odp.pDialog = new CDetailsOptsDlg();
	g_plugin.addOptions(wParam, &odp);

	// Reminder page
	odp.szTab.a = LPGEN("Reminder");
	odp.pDialog = new CReminderOptsDlg();
	g_plugin.addOptions(wParam, &odp);

	// Popups page
	odp.szTitle.a = MODULELONGNAME;
	odp.szGroup.a = LPGEN("Popups");
	odp.pDialog = new CPopupOptsDlg();
	g_plugin.addOptions(wParam, &odp);
	return MIR_OK;
}
