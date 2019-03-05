/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG team
Copyright (c) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (c) 2002-2005 Calvin Che

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* This file contain the source related to weather option pages. It also
contain code for saving/loading options from the database.
*/

#include "stdafx.h"

static BOOL opt_startup;
int RedrawFrame(WPARAM wParam, LPARAM lParam);

//============  LOADING AND SAVING OPTIONS  ===========
//
// set a string to default
// in = string to determine which field to set default "CBNEHXPp"
void SetTextDefault(const char* in)
{
	wchar_t str[MAX_TEXT_SIZE];

	if (strchr(in, 'C') != nullptr) {
		wcsncpy(str, C_DEFAULT, MAX_TEXT_SIZE - 1);
		wSetData(&opt.cText, str);
	}
	if (strchr(in, 'b') != nullptr) {
		wcsncpy(str, b_DEFAULT, MAX_TEXT_SIZE - 1);
		wSetData(&opt.bTitle, str);
	}
	if (strchr(in, 'B') != nullptr) {
		wcsncpy(str, B_DEFAULT, MAX_TEXT_SIZE - 1);
		wSetData(&opt.bText, str);
	}
	if (strchr(in, 'N') != nullptr) {
		wcsncpy(str, N_DEFAULT, MAX_TEXT_SIZE - 1);
		wSetData(&opt.nText, str);
	}
	if (strchr(in, 'E') != nullptr) {
		wcsncpy(str, E_DEFAULT, MAX_TEXT_SIZE - 1);
		wSetData(&opt.eText, str);
	}
	if (strchr(in, 'H') != nullptr) {
		wcsncpy(str, H_DEFAULT, MAX_TEXT_SIZE - 1);
		wSetData(&opt.hText, str);
	}
	if (strchr(in, 'X') != nullptr) {
		wcsncpy(str, X_DEFAULT, MAX_TEXT_SIZE - 1);
		wSetData(&opt.xText, str);
	}
	if (strchr(in, 'P') != nullptr) {
		wcsncpy(str, P_DEFAULT, MAX_TEXT_SIZE - 1);
		wSetData(&opt.pTitle, str);
	}
	if (strchr(in, 'p') != nullptr) {
		wcsncpy(str, p_DEFAULT, MAX_TEXT_SIZE - 1);
		wSetData(&opt.pText, str);
	}
	if (strchr(in, 'S') != nullptr) {
		wcsncpy(str, s_DEFAULT, MAX_TEXT_SIZE - 1);
		wSetData(&opt.sText, str);
	}
}

void DestroyOptions(void)
{
	wfree(&opt.cText);
	wfree(&opt.bTitle);
	wfree(&opt.bText);
	wfree(&opt.nText);
	wfree(&opt.eText);
	wfree(&opt.hText);
	wfree(&opt.xText);
	wfree(&opt.pTitle);
	wfree(&opt.pText);
	wfree(&opt.sText);
}

// load options from database + set default if the setting does not exist
void LoadOptions(void)
{
	memset(&opt, 0, sizeof(opt));

	// main options
	opt.StartupUpdate = g_plugin.getByte("StartupUpdate", true);
	opt.AutoUpdate = g_plugin.getByte("AutoUpdate", true);
	opt.UpdateTime = g_plugin.getWord("UpdateTime", 30);
	opt.NoProtoCondition = g_plugin.getByte("NoStatus", true);
	opt.UpdateOnlyConditionChanged = g_plugin.getByte("CondChangeAsUpdate", true);
	opt.RemoveOldData = g_plugin.getByte("RemoveOld", false);
	opt.MakeItalic = g_plugin.getByte("MakeItalic", true);
	opt.AvatarSize = g_plugin.getByte("AvatarSize", 128);

	// units
	opt.tUnit = g_plugin.getWord("tUnit", 1);
	opt.wUnit = g_plugin.getWord("wUnit", 2);
	opt.vUnit = g_plugin.getWord("vUnit", 1);
	opt.pUnit = g_plugin.getWord("pUnit", 4);
	opt.dUnit = g_plugin.getWord("dUnit", 1);
	opt.eUnit = g_plugin.getWord("eUnit", 2);

	ptrW szValue(g_plugin.getWStringA("DegreeSign"));
	wcsncpy_s(opt.DegreeSign, (szValue == NULL) ? L"" : szValue, _TRUNCATE);

	opt.DoNotAppendUnit = g_plugin.getByte("DoNotAppendUnit", 0);
	opt.NoFrac = g_plugin.getByte("NoFractions", 0);

	// texts
	if (szValue = g_plugin.getWStringA("DisplayText"))
		wSetData(&opt.cText, TranslateW(szValue));
	else
		SetTextDefault("C");

	if (szValue = g_plugin.getWStringA("BriefTextTitle"))
		wSetData(&opt.bTitle, TranslateW(szValue));
	else
		SetTextDefault("b");

	if (szValue = g_plugin.getWStringA("BriefText"))
		wSetData(&opt.bText, TranslateW(szValue));
	else
		SetTextDefault("B");

	if (szValue = g_plugin.getWStringA("NoteText"))
		wSetData(&opt.nText, TranslateW(szValue));
	else
		SetTextDefault("N");

	if (szValue = g_plugin.getWStringA("ExtText"))
		wSetData(&opt.eText, TranslateW(szValue));
	else
		SetTextDefault("E");

	if (szValue = g_plugin.getWStringA("HistoryText"))
		wSetData(&opt.hText, TranslateW(szValue));
	else
		SetTextDefault("H");

	if (szValue = g_plugin.getWStringA("ExtraText"))
		wSetData(&opt.xText, TranslateW(szValue));
	else
		SetTextDefault("X");

	if (szValue = g_plugin.getWStringA("StatusText"))
		wSetData(&opt.sText, TranslateW(szValue));
	else
		SetTextDefault("S");

	// advanced
	opt.DisCondIcon = g_plugin.getByte("DisableConditionIcon", false);
	// popup options
	opt.UsePopup = g_plugin.getByte("UsePopUp", true);
	opt.UpdatePopup = g_plugin.getByte("UpdatePopup", true);
	opt.AlertPopup = g_plugin.getByte("AlertPopup", true);
	opt.PopupOnChange = g_plugin.getByte("PopUpOnChange", true);
	opt.ShowWarnings = g_plugin.getByte("ShowWarnings", true);
	// popup colors
	opt.BGColour = g_plugin.getDword("BackgroundColour", GetSysColor(COLOR_BTNFACE));
	opt.TextColour = g_plugin.getDword("TextColour", GetSysColor(COLOR_WINDOWTEXT));
	opt.UseWinColors = g_plugin.getByte("UseWinColors", false);
	// popup actions
	opt.LeftClickAction = g_plugin.getDword("LeftClickAction", IDM_M2);
	opt.RightClickAction = g_plugin.getDword("RightClickAction", IDM_M1);
	// popup delay
	opt.pDelay = g_plugin.getDword("PopupDelay", 0);
	// popup texts
	if (szValue = g_plugin.getWStringA("PopupTitle"))
		wSetData(&opt.pTitle, szValue);
	else
		SetTextDefault("P");

	if (szValue = g_plugin.getWStringA("PopupText"))
		wSetData(&opt.pText, szValue);
	else
		SetTextDefault("p");

	// misc
	if (szValue = g_plugin.getWStringA("Default"))
		wcsncpy_s(opt.Default, szValue, _TRUNCATE);
	else
		opt.Default[0] = 0;
}

// save the options to database
void SaveOptions(void)
{
	// main options
	g_plugin.setByte("StartupUpdate", (BYTE)opt.StartupUpdate);
	g_plugin.setByte("AutoUpdate", (BYTE)opt.AutoUpdate);
	g_plugin.setWord("UpdateTime", opt.UpdateTime);
	g_plugin.setByte("NoStatus", (BYTE)opt.NoProtoCondition);
	g_plugin.setByte("CondChangeAsUpdate", (BYTE)opt.UpdateOnlyConditionChanged);
	g_plugin.setByte("RemoveOld", (BYTE)opt.RemoveOldData);
	g_plugin.setByte("MakeItalic", (BYTE)opt.MakeItalic);
	g_plugin.setByte("AvatarSize", (BYTE)opt.AvatarSize);
	// units
	g_plugin.setWord("tUnit", opt.tUnit);
	g_plugin.setWord("wUnit", opt.wUnit);
	g_plugin.setWord("vUnit", opt.vUnit);
	g_plugin.setWord("pUnit", opt.pUnit);
	g_plugin.setWord("dUnit", opt.dUnit);
	g_plugin.setWord("eUnit", opt.eUnit);
	g_plugin.setWString("DegreeSign", opt.DegreeSign);
	g_plugin.setByte("DoNotAppendUnit", (BYTE)opt.DoNotAppendUnit);
	g_plugin.setByte("NoFractions", (BYTE)opt.NoFrac);
	// texts
	g_plugin.setWString("DisplayText", opt.cText);
	g_plugin.setWString("BriefTextTitle", opt.bTitle);
	g_plugin.setWString("BriefText", opt.bText);
	g_plugin.setWString("NoteText", opt.nText);
	g_plugin.setWString("ExtText", opt.eText);
	g_plugin.setWString("HistoryText", opt.hText);
	g_plugin.setWString("ExtraText", opt.xText);
	g_plugin.setWString("StatusText", opt.sText);
	// advanced
	g_plugin.setByte("DisableConditionIcon", (BYTE)opt.DisCondIcon);
	// popup options
	g_plugin.setByte("UsePopUp", (BYTE)opt.UsePopup);
	g_plugin.setByte("UpdatePopup", (BYTE)opt.UpdatePopup);
	g_plugin.setByte("AlertPopup", (BYTE)opt.AlertPopup);
	g_plugin.setByte("PopUpOnChange", (BYTE)opt.PopupOnChange);
	g_plugin.setByte("ShowWarnings", (BYTE)opt.ShowWarnings);
	// popup colors
	g_plugin.setDword("BackgroundColour", opt.BGColour);
	g_plugin.setDword("TextColour", opt.TextColour);
	g_plugin.setByte("UseWinColors", (BYTE)opt.UseWinColors);
	// popup actions
	g_plugin.setDword("LeftClickAction", opt.LeftClickAction);
	g_plugin.setDword("RightClickAction", opt.RightClickAction);
	// popup delay
	g_plugin.setDword("PopupDelay", opt.pDelay);
	// popup texts
	g_plugin.setWString("PopupTitle", opt.pTitle);
	g_plugin.setWString("PopupText", opt.pText);
	// misc stuff
	g_plugin.setWString("Default", opt.Default);
}
//============  MAIN OPTIONS  ============

// weather options
static INT_PTR CALLBACK OptionsProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	wchar_t str[512];

	switch (msg) {
	case WM_INITDIALOG:
		opt_startup = TRUE;
		TranslateDialogDefault(hdlg);
		// load settings
		_ltow(opt.UpdateTime, str, 10);
		SetDlgItemText(hdlg, IDC_UPDATETIME, str);
		SetDlgItemText(hdlg, IDC_DEGREE, opt.DegreeSign);

		SendDlgItemMessage(hdlg, IDC_AVATARSPIN, UDM_SETRANGE32, 0, 999);
		SendDlgItemMessage(hdlg, IDC_AVATARSPIN, UDM_SETPOS, 0, opt.AvatarSize);
		SendDlgItemMessage(hdlg, IDC_AVATARSIZE, EM_LIMITTEXT, 3, 0);

		CheckDlgButton(hdlg, IDC_STARTUPUPD, opt.StartupUpdate ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_UPDATE, opt.AutoUpdate ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_PROTOCOND, !opt.NoProtoCondition ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_UPDCONDCHG, opt.UpdateOnlyConditionChanged ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_REMOVEOLD, opt.RemoveOldData ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_MAKEI, opt.MakeItalic ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_DISCONDICON, opt.DisCondIcon ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_DONOTAPPUNITS, opt.DoNotAppendUnit ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_NOFRAC, opt.NoFrac ? BST_CHECKED : BST_UNCHECKED);

		// load units
		switch (opt.tUnit) {	// temperature
		case 1: CheckRadioButton(hdlg, IDC_T1, IDC_T2, IDC_T1); break;
		case 2: CheckRadioButton(hdlg, IDC_T1, IDC_T2, IDC_T2); break;
		}
		switch (opt.wUnit) {	// wind
		case 1: CheckRadioButton(hdlg, IDC_W1, IDC_W4, IDC_W1); break;
		case 2: CheckRadioButton(hdlg, IDC_W1, IDC_W4, IDC_W2); break;
		case 3: CheckRadioButton(hdlg, IDC_W1, IDC_W4, IDC_W3); break;
		case 4: CheckRadioButton(hdlg, IDC_W1, IDC_W4, IDC_W4); break;
		}
		switch (opt.vUnit) {	// visibility
		case 1: CheckRadioButton(hdlg, IDC_V1, IDC_V2, IDC_V1); break;
		case 2: CheckRadioButton(hdlg, IDC_V1, IDC_V2, IDC_V2); break;
		}
		switch (opt.pUnit) {	// pressure
		case 1: CheckRadioButton(hdlg, IDC_P1, IDC_P4, IDC_P1); break;
		case 2: CheckRadioButton(hdlg, IDC_P1, IDC_P4, IDC_P2); break;
		case 3: CheckRadioButton(hdlg, IDC_P1, IDC_P4, IDC_P3); break;
		case 4: CheckRadioButton(hdlg, IDC_P1, IDC_P4, IDC_P4); break;
		}
		switch (opt.dUnit) {	// pressure
		case 1: CheckRadioButton(hdlg, IDC_D1, IDC_D3, IDC_D1); break;
		case 2: CheckRadioButton(hdlg, IDC_D1, IDC_D3, IDC_D2); break;
		case 3: CheckRadioButton(hdlg, IDC_D1, IDC_D3, IDC_D3); break;
		}

		switch (opt.eUnit) {	// elev
		case 1: CheckRadioButton(hdlg, IDC_E1, IDC_E2, IDC_E1); break;
		case 2: CheckRadioButton(hdlg, IDC_E1, IDC_E2, IDC_E2); break;
		}

		opt_startup = FALSE;
		return 0;

	case WM_COMMAND:
		if (HIWORD(wparam) == BN_CLICKED && GetFocus() == (HWND)lparam)
			if (!opt_startup)	SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
		if (!((LOWORD(wparam) == IDC_UPDATE || LOWORD(wparam) == IDC_DEGREE) &&
			(HIWORD(wparam) != EN_CHANGE || (HWND)lparam != GetFocus())))
			if (!opt_startup)	SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
		return 0;

	case WM_NOTIFY:
		switch (((LPNMHDR)lparam)->code) {
		case PSN_APPLY:
			// change the status for weather protocol
			if (IsDlgButtonChecked(hdlg, IDC_PROTOCOND) && opt.DefStn != NULL) {
				old_status = status;
				status = g_plugin.getWord(opt.DefStn, "StatusIcon", NOSTATUSDATA);
				ProtoBroadcastAck(MODULENAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, status);
			}

			// get update time and remove the old timer
			GetDlgItemText(hdlg, IDC_UPDATETIME, str, _countof(str));
			opt.UpdateTime = (WORD)_wtoi(str);
			if (opt.UpdateTime < 1)	opt.UpdateTime = 1;
			KillTimer(nullptr, timerId);
			timerId = SetTimer(nullptr, 0, opt.UpdateTime * 60000, timerProc);

			// other general options
			GetDlgItemText(hdlg, IDC_DEGREE, opt.DegreeSign, _countof(opt.DegreeSign));
			opt.StartupUpdate = IsDlgButtonChecked(hdlg, IDC_STARTUPUPD);
			opt.AutoUpdate = IsDlgButtonChecked(hdlg, IDC_UPDATE);
			opt.NoProtoCondition = BST_UNCHECKED == IsDlgButtonChecked(hdlg, IDC_PROTOCOND);
			opt.DisCondIcon = IsDlgButtonChecked(hdlg, IDC_DISCONDICON);
			opt.UpdateOnlyConditionChanged = (BYTE)IsDlgButtonChecked(hdlg, IDC_UPDCONDCHG);
			opt.RemoveOldData = IsDlgButtonChecked(hdlg, IDC_REMOVEOLD);
			opt.MakeItalic = IsDlgButtonChecked(hdlg, IDC_MAKEI);
			opt.AvatarSize = GetDlgItemInt(hdlg, IDC_AVATARSIZE, nullptr, FALSE);
			opt.DoNotAppendUnit = IsDlgButtonChecked(hdlg, IDC_DONOTAPPUNITS);
			opt.NoFrac = IsDlgButtonChecked(hdlg, IDC_NOFRAC);
			UpdateMenu(opt.AutoUpdate);

			// save the units
			if (IsDlgButtonChecked(hdlg, IDC_T1)) opt.tUnit = 1;
			if (IsDlgButtonChecked(hdlg, IDC_T2)) opt.tUnit = 2;
			if (IsDlgButtonChecked(hdlg, IDC_W1)) opt.wUnit = 1;
			if (IsDlgButtonChecked(hdlg, IDC_W2)) opt.wUnit = 2;
			if (IsDlgButtonChecked(hdlg, IDC_W3)) opt.wUnit = 3;
			if (IsDlgButtonChecked(hdlg, IDC_W4)) opt.wUnit = 4;
			if (IsDlgButtonChecked(hdlg, IDC_V1)) opt.vUnit = 1;
			if (IsDlgButtonChecked(hdlg, IDC_V2)) opt.vUnit = 2;
			if (IsDlgButtonChecked(hdlg, IDC_P1)) opt.pUnit = 1;
			if (IsDlgButtonChecked(hdlg, IDC_P2)) opt.pUnit = 2;
			if (IsDlgButtonChecked(hdlg, IDC_P3)) opt.pUnit = 3;
			if (IsDlgButtonChecked(hdlg, IDC_P4)) opt.pUnit = 4;
			if (IsDlgButtonChecked(hdlg, IDC_D1)) opt.dUnit = 1;
			if (IsDlgButtonChecked(hdlg, IDC_D2)) opt.dUnit = 2;
			if (IsDlgButtonChecked(hdlg, IDC_D3)) opt.dUnit = 3;
			if (IsDlgButtonChecked(hdlg, IDC_E1)) opt.eUnit = 1;
			if (IsDlgButtonChecked(hdlg, IDC_E2)) opt.eUnit = 2;

			// save the new weather options
			SaveOptions();

			RedrawFrame(0, 0);

			return 1;
		}
		break;
	}
	return 0;
}

//============  TEXT OPTION DIALOG  ============

static void LoadTextSettings(HWND hdlg)
{
	// load text option settings from memory
	SetDlgItemText(hdlg, IDC_CTEXT, opt.cText);
	SetDlgItemText(hdlg, IDC_BTITLE, opt.bTitle);
	SetDlgItemText(hdlg, IDC_BTEXT, opt.bText);
	SetDlgItemText(hdlg, IDC_ETEXT, opt.eText);
	SetDlgItemText(hdlg, IDC_NTEXT, opt.nText);
	SetDlgItemText(hdlg, IDC_HTEXT, opt.hText);
	SetDlgItemText(hdlg, IDC_XTEXT, opt.xText);
	SetDlgItemText(hdlg, IDC_BTITLE2, opt.sText);
}

// free the display text settings from memory
static void FreeTextVar(void)
{
	wfree(&opt.cText);
	wfree(&opt.bText);
	wfree(&opt.bTitle);
	wfree(&opt.eText);
	wfree(&opt.nText);
	wfree(&opt.hText);
	wfree(&opt.xText);
	wfree(&opt.sText);
}

// text option dialog

static const char *varname[8] = { "C", "b", "B", "N", "X", "E", "H", "S" };
static const int cname[8] = { IDC_CTEXT, IDC_BTITLE, IDC_BTEXT, IDC_NTEXT, IDC_XTEXT, IDC_ETEXT, IDC_HTEXT, IDC_BTITLE2 };
static wchar_t* const *var[8] = { &opt.cText, &opt.bTitle, &opt.bText, &opt.nText, &opt.xText, &opt.eText, &opt.hText, &opt.sText };

static INT_PTR CALLBACK DlgProcText(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc, pos;
	HWND button;
	HMENU hMenu, hMenu1;
	wchar_t str[4096];
	switch (msg) {
	case WM_INITDIALOG:
		opt_startup = TRUE;
		// set windows position, make it top-most
		GetWindowRect(hdlg, &rc);
		SetWindowPos(hdlg, HWND_TOPMOST, rc.left, rc.top, 0, 0, SWP_NOSIZE);
		TranslateDialogDefault(hdlg);
		// generate the display text for variable list
		wcsncpy(str, VAR_LIST_OPT, _countof(str) - 1);
		SetDlgItemText(hdlg, IDC_VARLIST, str);

		// make the more variable and other buttons flat
		SendDlgItemMessage(hdlg, IDC_MORE, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hdlg, IDC_TM1, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hdlg, IDC_TM2, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hdlg, IDC_TM3, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hdlg, IDC_TM4, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hdlg, IDC_TM5, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hdlg, IDC_TM6, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hdlg, IDC_TM7, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hdlg, IDC_TM8, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hdlg, IDC_RESET, BUTTONSETASFLATBTN, TRUE, 0);
		// load the settings
		LoadTextSettings(hdlg);
		opt_startup = FALSE;
		return TRUE;

	case WM_COMMAND:
		if (opt_startup)	return TRUE;
		switch (LOWORD(wParam)) {
		case IDC_CTEXT:
		case IDC_BTITLE:
		case IDC_BTEXT:
		case IDC_NTEXT:
		case IDC_XTEXT:
		case IDC_ETEXT:
		case IDC_HTEXT:
		case IDC_BTITLE2:
			if (HIWORD(wParam) == EN_CHANGE)
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_MORE:
			// display custom variables list
			MoreVarList();
			break;

		case IDC_TM1:
		case IDC_TM2:
		case IDC_TM3:
		case IDC_TM4:
		case IDC_TM5:
		case IDC_TM6:
		case IDC_TM7:
		case IDC_TM8:
			// display the menu
			button = GetDlgItem(hdlg, LOWORD(wParam));
			GetWindowRect(button, &pos);
			hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_TMMENU));
			hMenu1 = GetSubMenu(hMenu, 0);
			TranslateMenu(hMenu1);
			switch (TrackPopupMenu(hMenu1, TPM_LEFTBUTTON | TPM_RETURNCMD, pos.left, pos.bottom, 0, hdlg, nullptr)) {
			case ID_MPREVIEW:
				{
					// show the preview in a message box, using the weather data from the default station
					WEATHERINFO winfo = LoadWeatherInfo(opt.DefStn);
					GetDisplay(&winfo, *var[LOWORD(wParam) - IDC_TM1], str);
					MessageBox(nullptr, str, TranslateT("Weather Protocol Text Preview"), MB_OK | MB_TOPMOST);
					break;
				}
			case ID_MRESET:
				unsigned varo = LOWORD(wParam) - IDC_TM1;
				// remove the old setting from db and free memory
				wchar_t* vartmp = *var[varo];
				wfree(&vartmp);
				SetTextDefault(varname[varo]);
				SetDlgItemText(hdlg, cname[varo], *var[varo]);
				break;
			}
			DestroyMenu(hMenu);
			break;

		case IDC_RESET:
			// left click action selection menu
			button = GetDlgItem(hdlg, IDC_RESET);
			GetWindowRect(button, &pos);
			hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_TMENU));
			hMenu1 = GetSubMenu(hMenu, 0);
			TranslateMenu(hMenu1);
			switch (TrackPopupMenu(hMenu1, TPM_LEFTBUTTON | TPM_RETURNCMD, pos.left, pos.bottom, 0, hdlg, nullptr)) {
			case ID_T1:
				// reset to the strings in memory, discard all changes
				LoadTextSettings(hdlg);
				break;

			case ID_T2:
				// reset to the default setting
				FreeTextVar();
				SetTextDefault("CbBENHX");
				LoadTextSettings(hdlg);
				break;
			}
			DestroyMenu(hMenu);
			break;
		}
		return TRUE;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			// save the option
			wchar_t textstr[MAX_TEXT_SIZE];
			// free memory for old settings
			FreeTextVar();
			// save new settings to memory
			GetDlgItemText(hdlg, IDC_CTEXT, textstr, _countof(textstr));
			wSetData(&opt.cText, textstr);
			GetDlgItemText(hdlg, IDC_BTEXT, textstr, _countof(textstr));
			wSetData(&opt.bText, textstr);
			GetDlgItemText(hdlg, IDC_BTITLE, textstr, _countof(textstr));
			wSetData(&opt.bTitle, textstr);
			GetDlgItemText(hdlg, IDC_ETEXT, textstr, _countof(textstr));
			wSetData(&opt.eText, textstr);
			GetDlgItemText(hdlg, IDC_NTEXT, textstr, _countof(textstr));
			wSetData(&opt.nText, textstr);
			GetDlgItemText(hdlg, IDC_HTEXT, textstr, _countof(textstr));
			wSetData(&opt.hText, textstr);
			GetDlgItemText(hdlg, IDC_XTEXT, textstr, _countof(textstr));
			wSetData(&opt.xText, textstr);
			GetDlgItemText(hdlg, IDC_BTITLE2, textstr, _countof(textstr));
			wSetData(&opt.sText, textstr);
			SaveOptions();
			UpdateAllInfo(0, 0);
			break;
		}
		break;
	}
	return FALSE;
}


//============  OPTION INITIALIZATION  ============

// register the weather option pages
int OptInit(WPARAM wParam, LPARAM)
{
	// plugin options
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 95600;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc = OptionsProc;
	odp.szGroup.a = LPGEN("Network");
	odp.szTitle.a = WEATHERPROTOTEXT;
	odp.szTab.a = LPGEN("General");
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);

	// text options
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_TEXTOPT);
	odp.pfnDlgProc = DlgProcText;
	odp.szTab.a = LPGEN("Display");
	g_plugin.addOptions(wParam, &odp);

	// if popup service exists, load the weather popup options
	odp.position = 100000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUP);
	odp.szGroup.a = LPGEN("Popups");
	odp.szTab.a = nullptr;
	odp.pfnDlgProc = DlgPopupOpts;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
