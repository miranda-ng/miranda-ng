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

const wchar_t* GetDefaultText(int c)
{
	switch (c) {
	case 'C':
		return L"%n  [%t, %c]";
	case 'b':
		return TranslateT("Weather Condition for %n as of %u");
	case 'B':
		return TranslateT("Feel-Like: %f\\nPressure: %p\\nWind: %i  %w\\nHumidity: %m\\nDew Point: %e\\nVisibility: %v\\n\\nSun Rise: %r\\nSun Set: %y\\n\\n5 Days Forecast:\\n%[Forecast Day 1]\\n%[Forecast Day 2]\\n%[Forecast Day 3]\\n%[Forecast Day 4]\\n%[Forecast Day 5]");
	case 'X': case 'N':
		return TranslateT("%c\\nTemperature: %t\\nFeel-Like: %f\\nPressure: %p\\nWind: %i  %w\\nHumidity: %m\\nDew Point: %e\\nVisibility: %v\\n\\nSun Rise: %r\\nSun Set: %y\\n\\n5 Days Forecast:\\n%[Forecast Day 1]\\n%[Forecast Day 2]\\n%[Forecast Day 3]\\n%[Forecast Day 4]\\n%[Forecast Day 5]");
	case 'E':
		return TranslateT("%n at %u:	%c, %t (feel-like %f)	Wind: %i %w	Humidity: %m");
	case 'H':
		return TranslateT("%c, %t (feel-like %f)	Wind: %i %w	Humidity: %m");
	case 'S':
		return TranslateT("Temperature: %[Temperature]");
	case 'P':
		return TranslateT("%n   (%u)");
	case 'p':
		return TranslateT("%c, %t\\nToday:  High %h, Low %l");
	}
	return L"";
}

CMStringW GetTextValue(int c)
{
	CMStringW ret;

	switch (c) {
	case 'C': ret = g_plugin.getMStringW("DisplayText"); break;
	case 'b': ret = g_plugin.getMStringW("BriefTextTitle"); break;
	case 'B': ret = g_plugin.getMStringW("BriefText"); break;
	case 'N': ret = g_plugin.getMStringW("NoteText"); break;
	case 'E': ret = g_plugin.getMStringW("ExtText"); break;
	case 'H': ret = g_plugin.getMStringW("HistoryText"); break;
	case 'X': ret = g_plugin.getMStringW("ExtraText"); break;
	case 'S': ret = g_plugin.getMStringW("StatusText"); break;
	case 'P': ret = g_plugin.getMStringW("PopupTitle"); break;
	case 'p': ret = g_plugin.getMStringW("PopupText"); break;
	}

	return (ret.IsEmpty()) ? GetDefaultText(c) : ret;
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
				status = MapCondToStatus(opt.DefStn);
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

// text option dialog

struct
{
	wchar_t c;
	int  id;
	char *setting;
}
static controls[] = 
{
	{ 'C', IDC_CTEXT,   "DisplayText"    },
	{ 'b', IDC_BTITLE,  "BriefTextTitle" },
	{ 'B', IDC_BTEXT,   "BriefText"      },
	{ 'N', IDC_NTEXT,   "NoteText"       },
	{ 'X', IDC_XTEXT,   "ExtText"        },
	{ 'E', IDC_ETEXT,   "HistoryText"    },
	{ 'H', IDC_HTEXT,   "ExtraText"      },
	{ 'S', IDC_BTITLE2, "StatusText"     },
};

static INT_PTR CALLBACK DlgProcText(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc, pos;
	HWND button;
	HMENU hMenu, hMenu1;
	switch (msg) {
	case WM_INITDIALOG:
		opt_startup = TRUE;
		// set windows position, make it top-most
		GetWindowRect(hdlg, &rc);
		SetWindowPos(hdlg, HWND_TOPMOST, rc.left, rc.top, 0, 0, SWP_NOSIZE);
		TranslateDialogDefault(hdlg);

		// generate the display text for variable list
		SetDlgItemText(hdlg, IDC_VARLIST, VAR_LIST_OPT);

		for (auto &it : controls)
			SetDlgItemText(hdlg, it.id, GetTextValue(it.c));

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
			{
				auto &var = controls[int(LOWORD(wParam)) - IDC_TM1];

				switch (TrackPopupMenu(hMenu1, TPM_LEFTBUTTON | TPM_RETURNCMD, pos.left, pos.bottom, 0, hdlg, nullptr)) {
				case ID_MPREVIEW:
					{
						// show the preview in a message box, using the weather data from the default station
						WEATHERINFO winfo = LoadWeatherInfo(opt.DefStn);
						wchar_t buf[2] = { var.c, 0 }, str[4096];
						GetDisplay(&winfo, buf, str);
						MessageBox(nullptr, str, TranslateT("Weather Protocol Text Preview"), MB_OK | MB_TOPMOST);
					}
					break;

				case ID_MRESET:
					SetDlgItemText(hdlg, var.id, GetDefaultText(var.c));
					break;
				}
				DestroyMenu(hMenu);
			}
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
				for (auto &it : controls)
					SetDlgItemText(hdlg, it.id, GetTextValue(it.c));
				break;

			case ID_T2:
				// reset to the default setting
				for (auto &it : controls)
					SetDlgItemText(hdlg, it.id, GetDefaultText(it.c));
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
			for (auto &it : controls) {
				GetDlgItemText(hdlg, it.id, textstr, _countof(textstr));
				if (!mir_wstrcmpi(textstr, GetDefaultText(it.c)))
					g_plugin.delSetting(it.setting);
				else
					g_plugin.setWString(it.setting, textstr);
			}

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
