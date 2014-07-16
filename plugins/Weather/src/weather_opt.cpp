/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG Team
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

#include "weather.h"

static BOOL opt_startup;
int RedrawFrame(WPARAM wParam, LPARAM lParam);

//============  LOADING AND SAVING OPTIONS  ===========

// set a string to default
// in = string to determine which field to set default "CBNEHXPp"
void SetTextDefault(const char* in)
{
	TCHAR str[MAX_TEXT_SIZE];

	if (strchr(in, 'C') != NULL) {
		_tcscpy(str, C_DEFAULT);
		wSetData(&opt.cText, str);
	}
	if (strchr(in, 'b') != NULL) {
		_tcscpy(str, b_DEFAULT);
		wSetData(&opt.bTitle, str);
	}
	if (strchr(in, 'B') != NULL) {
		_tcscpy(str, B_DEFAULT);
		wSetData(&opt.bText, str);
	}
	if (strchr(in, 'N') != NULL) {
		_tcscpy(str, N_DEFAULT);
		wSetData(&opt.nText, str);
	}
	if (strchr(in, 'E') != NULL) {
		_tcscpy(str, E_DEFAULT);
		wSetData(&opt.eText, str);
	}
	if (strchr(in, 'H') != NULL) {
		_tcscpy(str, H_DEFAULT);
		wSetData(&opt.hText, str);
	}
	if (strchr(in, 'X') != NULL) {
		_tcscpy(str, X_DEFAULT);
		wSetData(&opt.xText, str);
	}
	if (strchr(in, 'P') != NULL) {
		_tcscpy(str, P_DEFAULT);
		wSetData(&opt.pTitle, str);
	}
	if (strchr(in, 'p') != NULL) {
		_tcscpy(str, p_DEFAULT);
		wSetData(&opt.pText, str);
	}
	if (strchr(in, 'S') != NULL) {
		_tcscpy(str, s_DEFAULT);
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
	ZeroMemory(&opt, sizeof(opt));

	// main options
	opt.StartupUpdate = db_get_b(NULL, WEATHERPROTONAME, "StartupUpdate", true);
	opt.AutoUpdate = db_get_b(NULL, WEATHERPROTONAME, "AutoUpdate", true);
	opt.UpdateTime = db_get_w(NULL, WEATHERPROTONAME, "UpdateTime", 30);
	opt.NoProtoCondition = db_get_b(NULL, WEATHERPROTONAME, "NoStatus", true);
	opt.UpdateOnlyConditionChanged = db_get_b(NULL, WEATHERPROTONAME, "CondChangeAsUpdate", true);
	opt.RemoveOldData = db_get_b(NULL, WEATHERPROTONAME, "RemoveOld", false);
	opt.MakeItalic = db_get_b(NULL, WEATHERPROTONAME, "MakeItalic", true);
	opt.AvatarSize = db_get_b(NULL, WEATHERPROTONAME, "AvatarSize", 128);

	// units
	opt.tUnit = db_get_w(NULL, WEATHERPROTONAME, "tUnit", 1);
	opt.wUnit = db_get_w(NULL, WEATHERPROTONAME, "wUnit", 2);
	opt.vUnit = db_get_w(NULL, WEATHERPROTONAME, "vUnit", 1);
	opt.pUnit = db_get_w(NULL, WEATHERPROTONAME, "pUnit", 4);
	opt.dUnit = db_get_w(NULL, WEATHERPROTONAME, "dUnit", 1);
	opt.eUnit = db_get_w(NULL, WEATHERPROTONAME, "eUnit", 2);

	ptrT szValue(db_get_tsa(NULL, WEATHERPROTONAME, "DegreeSign"));
	_tcsncpy_s(opt.DegreeSign, (szValue == NULL) ? _T("") : szValue, _TRUNCATE);

	opt.DoNotAppendUnit = db_get_b(NULL, WEATHERPROTONAME, "DoNotAppendUnit", 0);
	opt.NoFrac = db_get_b(NULL, WEATHERPROTONAME, "NoFractions", 0);
	
	// texts
	if (szValue = db_get_tsa(NULL, WEATHERPROTONAME, "DisplayText"))
		wSetData(&opt.cText, TranslateTS(szValue));
	else
		SetTextDefault("C");

	if (szValue = db_get_tsa(NULL, WEATHERPROTONAME, "BriefTextTitle"))
		wSetData(&opt.bTitle, TranslateTS(szValue));
	else
		SetTextDefault("b");

	if (szValue = db_get_tsa(NULL, WEATHERPROTONAME, "BriefText"))
		wSetData(&opt.bText, TranslateTS(szValue));
	else
		SetTextDefault("B");

	if (szValue = db_get_tsa(NULL, WEATHERPROTONAME, "NoteText"))
		wSetData(&opt.nText, TranslateTS(szValue));
	else
		SetTextDefault("N");

	if (szValue = db_get_tsa(NULL, WEATHERPROTONAME, "ExtText"))
		wSetData(&opt.eText, TranslateTS(szValue));
	else
		SetTextDefault("E");

	if (szValue = db_get_tsa(NULL, WEATHERPROTONAME, "HistoryText"))
		wSetData(&opt.hText, TranslateTS(szValue));
	else
		SetTextDefault("H");

	if (szValue = db_get_tsa(NULL, WEATHERPROTONAME, "ExtraText"))
		wSetData(&opt.xText, TranslateTS(szValue));
	else
		SetTextDefault("X");

	if (szValue = db_get_tsa(NULL, WEATHERPROTONAME, "StatusText"))
		wSetData(&opt.sText, TranslateTS(szValue));
	else
		SetTextDefault("S");

	// advanced
	opt.DisCondIcon = db_get_b(NULL, WEATHERPROTONAME, "DisableConditionIcon", false);
	// popup options
	opt.UsePopup = db_get_b(NULL, WEATHERPROTONAME, "UsePopUp", true);
	opt.UpdatePopup = db_get_b(NULL, WEATHERPROTONAME, "UpdatePopup", true);
	opt.AlertPopup = db_get_b(NULL, WEATHERPROTONAME, "AlertPopup", true);
	opt.PopupOnChange = db_get_b(NULL, WEATHERPROTONAME, "PopUpOnChange", true);
	opt.ShowWarnings = db_get_b(NULL, WEATHERPROTONAME, "ShowWarnings", true);
	// popup colors
	opt.BGColour = db_get_dw(NULL, WEATHERPROTONAME, "BackgroundColour", GetSysColor(COLOR_BTNFACE));
	opt.TextColour = db_get_dw(NULL, WEATHERPROTONAME, "TextColour", GetSysColor(COLOR_WINDOWTEXT));
	opt.UseWinColors = (BOOL)db_get_b(NULL, WEATHERPROTONAME, "UseWinColors", false);
	// popup actions
	opt.LeftClickAction = db_get_dw(NULL, WEATHERPROTONAME, "LeftClickAction", IDM_M2);
	opt.RightClickAction = db_get_dw(NULL, WEATHERPROTONAME, "RightClickAction", IDM_M1);
	// popup delay
	opt.pDelay = db_get_dw(NULL, WEATHERPROTONAME, "PopupDelay", 0);
	// popup texts
	if (szValue = db_get_tsa(NULL, WEATHERPROTONAME, "PopupTitle"))
		wSetData(&opt.pTitle, szValue);
	else
		SetTextDefault("P");

	if (szValue = db_get_tsa(NULL, WEATHERPROTONAME, "PopupText"))
		wSetData(&opt.pText, szValue);
	else
		SetTextDefault("p");

	// misc
	if (szValue = db_get_tsa(NULL, WEATHERPROTONAME, "Default"))
		_tcsncpy_s(opt.Default, szValue, _TRUNCATE);
	else
		opt.Default[0] = 0;
}

// save the options to database
void SaveOptions(void)
{
	// main options
	db_set_b(NULL, WEATHERPROTONAME, "StartupUpdate", (BYTE)opt.StartupUpdate);
	db_set_b(NULL, WEATHERPROTONAME, "AutoUpdate", (BYTE)opt.AutoUpdate);
	db_set_w(NULL, WEATHERPROTONAME, "UpdateTime", opt.UpdateTime);
	db_set_b(NULL, WEATHERPROTONAME, "NoStatus", (BYTE)opt.NoProtoCondition);
	db_set_b(NULL, WEATHERPROTONAME, "CondChangeAsUpdate", (BYTE)opt.UpdateOnlyConditionChanged);
	db_set_b(NULL, WEATHERPROTONAME, "RemoveOld", (BYTE)opt.RemoveOldData);
	db_set_b(NULL, WEATHERPROTONAME, "MakeItalic", (BYTE)opt.MakeItalic);
	db_set_b(NULL, WEATHERPROTONAME, "AvatarSize", (BYTE)opt.AvatarSize);
	// units
	db_set_w(NULL, WEATHERPROTONAME, "tUnit", opt.tUnit);
	db_set_w(NULL, WEATHERPROTONAME, "wUnit", opt.wUnit);
	db_set_w(NULL, WEATHERPROTONAME, "vUnit", opt.vUnit);
	db_set_w(NULL, WEATHERPROTONAME, "pUnit", opt.pUnit);
	db_set_w(NULL, WEATHERPROTONAME, "dUnit", opt.dUnit);
	db_set_w(NULL, WEATHERPROTONAME, "eUnit", opt.eUnit);
	db_set_ts(NULL, WEATHERPROTONAME, "DegreeSign", opt.DegreeSign);
	db_set_b(NULL, WEATHERPROTONAME, "DoNotAppendUnit", (BYTE)opt.DoNotAppendUnit);
	db_set_b(NULL, WEATHERPROTONAME, "NoFractions", (BYTE)opt.NoFrac);
	// texts
	db_set_ts(NULL, WEATHERPROTONAME, "DisplayText", opt.cText);
	db_set_ts(NULL, WEATHERPROTONAME, "BriefTextTitle", opt.bTitle);
	db_set_ts(NULL, WEATHERPROTONAME, "BriefText", opt.bText);
	db_set_ts(NULL, WEATHERPROTONAME, "NoteText", opt.nText);
	db_set_ts(NULL, WEATHERPROTONAME, "ExtText", opt.eText);
	db_set_ts(NULL, WEATHERPROTONAME, "HistoryText", opt.hText);
	db_set_ts(NULL, WEATHERPROTONAME, "ExtraText", opt.xText);
	db_set_ts(NULL, WEATHERPROTONAME, "StatusText", opt.sText);
	// advanced
	db_set_b(NULL, WEATHERPROTONAME, "DisableConditionIcon", (BYTE)opt.DisCondIcon);
	// popup options
	db_set_b(NULL, WEATHERPROTONAME, "UsePopUp", (BYTE)opt.UsePopup);
	db_set_b(NULL, WEATHERPROTONAME, "UpdatePopup", (BYTE)opt.UpdatePopup);
	db_set_b(NULL, WEATHERPROTONAME, "AlertPopup", (BYTE)opt.AlertPopup);
	db_set_b(NULL, WEATHERPROTONAME, "PopUpOnChange", (BYTE)opt.PopupOnChange);
	db_set_b(NULL, WEATHERPROTONAME, "ShowWarnings", (BYTE)opt.ShowWarnings);
	// popup colors
	db_set_dw(NULL, WEATHERPROTONAME, "BackgroundColour", opt.BGColour);
	db_set_dw(NULL, WEATHERPROTONAME, "TextColour", opt.TextColour);
	db_set_b(NULL, WEATHERPROTONAME, "UseWinColors", (BYTE)opt.UseWinColors);
	// popup actions
	db_set_dw(NULL, WEATHERPROTONAME, "LeftClickAction", opt.LeftClickAction);
	db_set_dw(NULL, WEATHERPROTONAME, "RightClickAction", opt.RightClickAction);
	// popup delay
	db_set_dw(NULL, WEATHERPROTONAME, "PopupDelay", opt.pDelay);
	// popup texts
	db_set_ts(NULL, WEATHERPROTONAME, "PopupTitle", opt.pTitle);
	db_set_ts(NULL, WEATHERPROTONAME, "PopupText", opt.pText);
	// misc stuff
	db_set_ts(NULL, WEATHERPROTONAME, "Default", opt.Default);
}

//============  OPTION INITIALIZATION  ============

// register the weather option pages
int OptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;

	// plugin options
	odp.position = 95600;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc = OptionsProc;
	odp.ptszGroup = LPGENT("Network");
	odp.ptszTitle = _T(WEATHERPROTOTEXT);
	odp.ptszTab = LPGENT("General");
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	Options_AddPage(wParam, &odp);

	// text options
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_TEXTOPT);
	odp.pfnDlgProc = DlgProcText;
	odp.ptszTab = LPGENT("Display");
	Options_AddPage(wParam, &odp);

	// if popup service exists, load the weather popup options
	if ((ServiceExists(MS_POPUP_ADDPOPUPT))) {
		odp.position = 100000000;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUP);
		odp.ptszGroup = LPGENT("Popups");
		odp.groupPosition = 910000000;
		odp.ptszTab = NULL;
		odp.pfnDlgProc = DlgPopupOpts;
		Options_AddPage(wParam, &odp);
	}

	return 0;
}

//============  MAIN OPTIONS  ============

// weather options
INT_PTR CALLBACK OptionsProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	TCHAR str[512];

	switch (msg) {
	case WM_INITDIALOG:
		opt_startup = TRUE;
		TranslateDialogDefault(hdlg);
		// load settings
		_ltot(opt.UpdateTime, str, 10);
		SetDlgItemText(hdlg, IDC_UPDATETIME, str);
		SetDlgItemText(hdlg, IDC_DEGREE, opt.DegreeSign);

		SendDlgItemMessage(hdlg, IDC_AVATARSPIN, UDM_SETRANGE32, 0, 999);
		SendDlgItemMessage(hdlg, IDC_AVATARSPIN, UDM_SETPOS, 0, opt.AvatarSize);
		SendDlgItemMessage(hdlg, IDC_AVATARSIZE, EM_LIMITTEXT, 3, 0);

		CheckDlgButton(hdlg, IDC_STARTUPUPD, opt.StartupUpdate);
		CheckDlgButton(hdlg, IDC_UPDATE, opt.AutoUpdate);
		CheckDlgButton(hdlg, IDC_PROTOCOND, !opt.NoProtoCondition);
		CheckDlgButton(hdlg, IDC_UPDCONDCHG, opt.UpdateOnlyConditionChanged);
		CheckDlgButton(hdlg, IDC_REMOVEOLD, opt.RemoveOldData);
		CheckDlgButton(hdlg, IDC_MAKEI, opt.MakeItalic);
		CheckDlgButton(hdlg, IDC_DISCONDICON, opt.DisCondIcon);
		CheckDlgButton(hdlg, IDC_DONOTAPPUNITS, opt.DoNotAppendUnit);
		CheckDlgButton(hdlg, IDC_NOFRAC, opt.NoFrac);

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
				status = db_get_w(opt.DefStn, WEATHERPROTONAME, "StatusIcon", NOSTATUSDATA);
				ProtoBroadcastAck(WEATHERPROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, status);
			}

			// get update time and remove the old timer
			GetDlgItemText(hdlg, IDC_UPDATETIME, str, sizeof(str));
			opt.UpdateTime = (WORD)_ttoi(str);
			if (opt.UpdateTime < 1)	opt.UpdateTime = 1;
			KillTimer(NULL, timerId);
			timerId = SetTimer(NULL, 0, opt.UpdateTime * 60000, (TIMERPROC)timerProc);

			// other general options
			GetDlgItemText(hdlg, IDC_DEGREE, opt.DegreeSign, sizeof(opt.DegreeSign));
			opt.StartupUpdate = IsDlgButtonChecked(hdlg, IDC_STARTUPUPD);
			opt.AutoUpdate = IsDlgButtonChecked(hdlg, IDC_UPDATE);
			opt.NoProtoCondition = !IsDlgButtonChecked(hdlg, IDC_PROTOCOND);
			opt.DisCondIcon = IsDlgButtonChecked(hdlg, IDC_DISCONDICON);
			opt.UpdateOnlyConditionChanged = (BYTE)IsDlgButtonChecked(hdlg, IDC_UPDCONDCHG);
			opt.RemoveOldData = IsDlgButtonChecked(hdlg, IDC_REMOVEOLD);
			opt.MakeItalic = IsDlgButtonChecked(hdlg, IDC_MAKEI);
			opt.AvatarSize = GetDlgItemInt(hdlg, IDC_AVATARSIZE, NULL, FALSE);
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

void LoadTextSettings(HWND hdlg)
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
void FreeTextVar(void)
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
static TCHAR* const *var[8] = { &opt.cText, &opt.bTitle, &opt.bText, &opt.nText, &opt.xText, &opt.eText, &opt.hText, &opt.sText };

INT_PTR CALLBACK DlgProcText(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc, pos;
	HWND button;
	HMENU hMenu, hMenu1;
	TCHAR str[4096];
	switch (msg) {
	case WM_INITDIALOG:
		opt_startup = TRUE;
		// set windows position, make it top-most
		GetWindowRect(hdlg, &rc);
		SetWindowPos(hdlg, HWND_TOPMOST, rc.left, rc.top, 0, 0, SWP_NOSIZE);
		TranslateDialogDefault(hdlg);
		// generate the display text for variable list
		_tcscpy(str, TranslateT("%c\tcurrent condition\n%d\tcurrent date\n%e\tdewpoint\n%f\tfeel-like temp\n%h\ttoday's high\n%i\twind direction\n%l\ttoday's low\n%m\thumidity\n%n\tstation name\n%p\tpressure\n%r\tsunrise time\n%s\tstation ID\n%t\ttemperature\n%u\tupdate time\n%v\tvisibility\n%w\twind speed\n%y\tsun set\n----------\n\\n\tnew line"));
		SetDlgItemText(hdlg, IDC_VARLIST, str);

		// make the more variable and other buttons flat
		SendMessage(GetDlgItem(hdlg, IDC_MORE), BUTTONSETASFLATBTN, TRUE, 0);
		SendMessage(GetDlgItem(hdlg, IDC_TM1), BUTTONSETASFLATBTN, TRUE, 0);
		SendMessage(GetDlgItem(hdlg, IDC_TM2), BUTTONSETASFLATBTN, TRUE, 0);
		SendMessage(GetDlgItem(hdlg, IDC_TM3), BUTTONSETASFLATBTN, TRUE, 0);
		SendMessage(GetDlgItem(hdlg, IDC_TM4), BUTTONSETASFLATBTN, TRUE, 0);
		SendMessage(GetDlgItem(hdlg, IDC_TM5), BUTTONSETASFLATBTN, TRUE, 0);
		SendMessage(GetDlgItem(hdlg, IDC_TM6), BUTTONSETASFLATBTN, TRUE, 0);
		SendMessage(GetDlgItem(hdlg, IDC_TM7), BUTTONSETASFLATBTN, TRUE, 0);
		SendMessage(GetDlgItem(hdlg, IDC_TM8), BUTTONSETASFLATBTN, TRUE, 0);
		SendMessage(GetDlgItem(hdlg, IDC_RESET), BUTTONSETASFLATBTN, TRUE, 0);
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
			WEATHERINFO winfo;
			// display the menu
			button = GetDlgItem(hdlg, LOWORD(wParam));
			GetWindowRect(button, &pos);
			hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_TMMENU));
			hMenu1 = GetSubMenu(hMenu, 0);
			TranslateMenu(hMenu1);
			switch (TrackPopupMenu(hMenu1, TPM_LEFTBUTTON | TPM_RETURNCMD, pos.left, pos.bottom, 0, hdlg, NULL)) {
			case ID_MPREVIEW:
				// show the preview in a message box, using the weather data from the default station
				winfo = LoadWeatherInfo(opt.DefStn);
				GetDisplay(&winfo, *var[LOWORD(wParam) - IDC_TM1], str);
				MessageBox(NULL, str, TranslateT("Weather Protocol Text Preview"), MB_OK | MB_TOPMOST);
				break;

			case ID_MRESET:
				unsigned varo = LOWORD(wParam) - IDC_TM1;
				// remove the old setting from db and free memory
				TCHAR* vartmp = *var[varo];
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
			hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_TMENU));
			hMenu1 = GetSubMenu(hMenu, 0);
			TranslateMenu(hMenu1);
			switch (TrackPopupMenu(hMenu1, TPM_LEFTBUTTON | TPM_RETURNCMD, pos.left, pos.bottom, 0, hdlg, NULL)) {
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
			TCHAR textstr[MAX_TEXT_SIZE];
			// free memory for old settings
			FreeTextVar();
			// save new settings to memory
			GetDlgItemText(hdlg, IDC_CTEXT, textstr, MAX_TEXT_SIZE);
			wSetData(&opt.cText, textstr);
			GetDlgItemText(hdlg, IDC_BTEXT, textstr, MAX_TEXT_SIZE);
			wSetData(&opt.bText, textstr);
			GetDlgItemText(hdlg, IDC_BTITLE, textstr, MAX_TEXT_SIZE);
			wSetData(&opt.bTitle, textstr);
			GetDlgItemText(hdlg, IDC_ETEXT, textstr, MAX_TEXT_SIZE);
			wSetData(&opt.eText, textstr);
			GetDlgItemText(hdlg, IDC_NTEXT, textstr, MAX_TEXT_SIZE);
			wSetData(&opt.nText, textstr);
			GetDlgItemText(hdlg, IDC_HTEXT, textstr, MAX_TEXT_SIZE);
			wSetData(&opt.hText, textstr);
			GetDlgItemText(hdlg, IDC_XTEXT, textstr, MAX_TEXT_SIZE);
			wSetData(&opt.xText, textstr);
			GetDlgItemText(hdlg, IDC_BTITLE2, textstr, MAX_TEXT_SIZE);
			wSetData(&opt.sText, textstr);
			SaveOptions();
			UpdateAllInfo(0, 0);
			break;
		}
		break;
	}
	return FALSE;
}

