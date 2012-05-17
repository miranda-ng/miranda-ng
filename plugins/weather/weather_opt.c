/*
Weather Protocol plugin for Miranda IM
Copyright (C) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2002-2005 Calvin Che

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
	char str[MAX_TEXT_SIZE];

	if (strchr(in, 'C') != NULL)
	{
		FixStr(C_DEFAULT, str);
		wSetData(&opt.cText, str);
	}
	if (strchr(in, 'b') != NULL)
	{
		FixStr(b_DEFAULT, str);
		wSetData(&opt.bTitle, str);
	}
	if (strchr(in, 'B') != NULL)
	{
		FixStr(B_DEFAULT, str);
		wSetData(&opt.bText, str);
	}
	if (strchr(in, 'N') != NULL)
	{
		FixStr(N_DEFAULT, str);
		wSetData(&opt.nText, str);
	}
	if (strchr(in, 'E') != NULL)
	{
		FixStr(E_DEFAULT, str);
		wSetData(&opt.eText, str);
	}
	if (strchr(in, 'H') != NULL)
	{
		FixStr(H_DEFAULT, str);
		wSetData(&opt.hText, str);
	}
	if (strchr(in, 'X') != NULL)
	{
		FixStr(X_DEFAULT, str);
		wSetData(&opt.xText, str);
	}
	if (strchr(in, 'P') != NULL)
	{
		FixStr(P_DEFAULT, str);
		wSetData(&opt.pTitle, str);
	}
	if (strchr(in, 'p') != NULL)
	{
		FixStr(p_DEFAULT, str);
		wSetData(&opt.pText, str);
	}
	if (strchr(in, 'S') != NULL)
		wSetData(&opt.sText, "");
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
	DBVARIANT dbv;
	ZeroMemory(&opt, sizeof(opt));

	// main options
	opt.StartupUpdate = DBGetContactSettingByte(NULL,WEATHERPROTONAME,"StartupUpdate",TRUE);
	opt.AutoUpdate = DBGetContactSettingByte(NULL,WEATHERPROTONAME,"AutoUpdate",TRUE);
	opt.UpdateTime = (WORD)DBGetContactSettingWord(NULL,WEATHERPROTONAME,"UpdateTime",20);
	opt.NewBrowserWin = DBGetContactSettingByte(NULL,WEATHERPROTONAME,"NewWindow",TRUE);
	opt.NoProtoCondition = DBGetContactSettingByte(NULL,WEATHERPROTONAME,"NoStatus",TRUE);
	opt.UpdateOnlyConditionChanged = DBGetContactSettingByte(NULL,WEATHERPROTONAME,"CondChangeAsUpdate",TRUE);
	opt.RemoveOldData = DBGetContactSettingByte(NULL,WEATHERPROTONAME,"RemoveOld",FALSE);
	opt.MakeItalic = DBGetContactSettingByte(NULL,WEATHERPROTONAME,"MakeItalic",TRUE);
	opt.AvatarSize = DBGetContactSettingByte(NULL,WEATHERPROTONAME,"AvatarSize", 32);
	// units
	opt.tUnit = (WORD)DBGetContactSettingWord(NULL, WEATHERPROTONAME, "tUnit", 1);
	opt.wUnit = (WORD)DBGetContactSettingWord(NULL, WEATHERPROTONAME, "wUnit", 1);
	opt.vUnit = (WORD)DBGetContactSettingWord(NULL, WEATHERPROTONAME, "vUnit", 1);
	opt.pUnit = (WORD)DBGetContactSettingWord(NULL, WEATHERPROTONAME, "pUnit", 1);
	opt.dUnit = (WORD)DBGetContactSettingWord(NULL, WEATHERPROTONAME, "dUnit", 1);
	if (DBGetContactSettingString(NULL,WEATHERPROTONAME,"DegreeSign",&dbv))
		FixStr(Translate(""), opt.DegreeSign);
	else
	{
		strcpy(opt.DegreeSign, dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	opt.DoNotAppendUnit = DBGetContactSettingByte(NULL, WEATHERPROTONAME, "DoNotAppendUnit", 0);
	opt.NoFrac = DBGetContactSettingByte(NULL, WEATHERPROTONAME, "NoFractions", 0);
	// texts
	if (DBGetContactSettingString(NULL,WEATHERPROTONAME,"DisplayText",&dbv))
		SetTextDefault("C");
	else
	{
		wSetData(&opt.cText, dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if (DBGetContactSettingString(NULL,WEATHERPROTONAME,"BriefTextTitle",&dbv))
		SetTextDefault("b");
	else
	{
		wSetData(&opt.bTitle, dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if (DBGetContactSettingString(NULL,WEATHERPROTONAME,"BriefText",&dbv))
		SetTextDefault("B");
	else	
	{
		wSetData(&opt.bText, dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if (DBGetContactSettingString(NULL,WEATHERPROTONAME,"NoteText",&dbv))
		SetTextDefault("N");
	else	
	{
		wSetData(&opt.nText, dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if (DBGetContactSettingString(NULL,WEATHERPROTONAME,"ExtText",&dbv))
		SetTextDefault("E");
	else
	{
		wSetData(&opt.eText, dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if (DBGetContactSettingString(NULL,WEATHERPROTONAME,"HistoryText",&dbv))
		SetTextDefault("H");
	else	
	{
		wSetData(&opt.hText, dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if (DBGetContactSettingString(NULL,WEATHERPROTONAME,"ExtraText",&dbv))
		SetTextDefault("X");
	else	
	{
		wSetData(&opt.xText, dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if (DBGetContactSettingString(NULL,WEATHERPROTONAME,"StatusText",&dbv))
		SetTextDefault("S");
	else
	{
		wSetData(&opt.sText, dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	// advanced
	opt.DisCondIcon = DBGetContactSettingByte(NULL,WEATHERPROTONAME,"DisableConditionIcon",FALSE);
	// popup options
	opt.UsePopup = DBGetContactSettingByte(NULL,WEATHERPROTONAME,"UsePopUp",TRUE);
	opt.UpdatePopup = DBGetContactSettingByte(NULL,WEATHERPROTONAME,"UpdatePopup",TRUE);
	opt.AlertPopup = DBGetContactSettingByte(NULL,WEATHERPROTONAME,"AlertPopup",TRUE);
	opt.PopupOnChange = DBGetContactSettingByte(NULL,WEATHERPROTONAME,"PopUpOnChange",TRUE);
	opt.ShowWarnings = DBGetContactSettingByte(NULL,WEATHERPROTONAME,"ShowWarnings",TRUE);
	// popup colors
	opt.BGColour = DBGetContactSettingDword(NULL,WEATHERPROTONAME,"BackgroundColour",GetSysColor(COLOR_BTNFACE));
	opt.TextColour = DBGetContactSettingDword(NULL,WEATHERPROTONAME,"TextColour",GetSysColor(COLOR_WINDOWTEXT));
	opt.UseWinColors = (BOOL)DBGetContactSettingByte(NULL,WEATHERPROTONAME, "UseWinColors", FALSE);
	// popup actions
	opt.LeftClickAction = DBGetContactSettingDword(NULL,WEATHERPROTONAME,"LeftClickAction",IDM_M1);
	opt.RightClickAction = DBGetContactSettingDword(NULL,WEATHERPROTONAME,"RightClickAction",IDM_M1);
	// popup delay
	opt.pDelay = DBGetContactSettingDword(NULL,WEATHERPROTONAME,"PopupDelay",0);
	// popup texts
	if (DBGetContactSettingString(NULL,WEATHERPROTONAME,"PopupTitle",&dbv))
		SetTextDefault("P");
	else	
	{
		wSetData(&opt.pTitle, dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if (DBGetContactSettingString(NULL,WEATHERPROTONAME,"PopupText",&dbv))
		SetTextDefault("p");
	else	
	{
		wSetData(&opt.pText, dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	// misc
	if (DBGetContactSettingString(NULL,WEATHERPROTONAME,"Default",&dbv))
		opt.Default[0] = 0;
	else	
	{
		strcpy(opt.Default, dbv.pszVal);
		DBFreeVariant(&dbv);
	}
}

// save the options to database
void SaveOptions(void) 
{
	// main options
	DBWriteContactSettingByte(NULL, WEATHERPROTONAME, "StartupUpdate", (BYTE)opt.StartupUpdate);
	DBWriteContactSettingByte(NULL, WEATHERPROTONAME, "AutoUpdate", (BYTE)opt.AutoUpdate);
	DBWriteContactSettingWord(NULL, WEATHERPROTONAME, "UpdateTime", opt.UpdateTime);
	DBWriteContactSettingByte(NULL, WEATHERPROTONAME, "NewWindow", (BYTE)opt.NewBrowserWin);
	DBWriteContactSettingByte(NULL, WEATHERPROTONAME, "NoStatus", (BYTE)opt.NoProtoCondition);
	DBWriteContactSettingByte(NULL, WEATHERPROTONAME, "CondChangeAsUpdate", (BYTE)opt.UpdateOnlyConditionChanged);
	DBWriteContactSettingByte(NULL, WEATHERPROTONAME, "RemoveOld", (BYTE)opt.RemoveOldData);
	DBWriteContactSettingByte(NULL, WEATHERPROTONAME, "MakeItalic", (BYTE)opt.MakeItalic);
	DBWriteContactSettingByte(NULL, WEATHERPROTONAME, "AvatarSize", (BYTE)opt.AvatarSize);
	// units
	DBWriteContactSettingWord(NULL, WEATHERPROTONAME, "tUnit", opt.tUnit);
	DBWriteContactSettingWord(NULL, WEATHERPROTONAME, "wUnit", opt.wUnit);
	DBWriteContactSettingWord(NULL, WEATHERPROTONAME, "vUnit", opt.vUnit);
	DBWriteContactSettingWord(NULL, WEATHERPROTONAME, "pUnit", opt.pUnit);
	DBWriteContactSettingWord(NULL, WEATHERPROTONAME, "dUnit", opt.dUnit);
	DBWriteContactSettingString(NULL, WEATHERPROTONAME, "DegreeSign", opt.DegreeSign);
	DBWriteContactSettingByte(NULL, WEATHERPROTONAME, "DoNotAppendUnit", (BYTE)opt.DoNotAppendUnit);
	DBWriteContactSettingByte(NULL, WEATHERPROTONAME, "NoFractions", (BYTE)opt.NoFrac);
	// texts
	DBWriteContactSettingString(NULL, WEATHERPROTONAME, "DisplayText", opt.cText);
	DBWriteContactSettingString(NULL, WEATHERPROTONAME, "BriefTextTitle", opt.bTitle);
	DBWriteContactSettingString(NULL, WEATHERPROTONAME, "BriefText", opt.bText);
	DBWriteContactSettingString(NULL, WEATHERPROTONAME, "NoteText", opt.nText);
	DBWriteContactSettingString(NULL, WEATHERPROTONAME, "ExtText", opt.eText);
	DBWriteContactSettingString(NULL, WEATHERPROTONAME, "HistoryText", opt.hText);
	DBWriteContactSettingString(NULL, WEATHERPROTONAME, "ExtraText", opt.xText);
	DBWriteContactSettingString(NULL, WEATHERPROTONAME, "StatusText", opt.sText);
	// advanced
	DBWriteContactSettingByte(NULL, WEATHERPROTONAME, "DisableConditionIcon", (BYTE)opt.DisCondIcon);
	// popup options
	DBWriteContactSettingByte(NULL, WEATHERPROTONAME, "UsePopUp", (BYTE)opt.UsePopup);
	DBWriteContactSettingByte(NULL, WEATHERPROTONAME, "UpdatePopup", (BYTE)opt.UpdatePopup);
	DBWriteContactSettingByte(NULL, WEATHERPROTONAME, "AlertPopup", (BYTE)opt.AlertPopup);
	DBWriteContactSettingByte(NULL, WEATHERPROTONAME, "PopUpOnChange", (BYTE)opt.PopupOnChange);
	DBWriteContactSettingByte(NULL, WEATHERPROTONAME, "ShowWarnings", (BYTE)opt.ShowWarnings);
	// popup colors
	DBWriteContactSettingDword(NULL, WEATHERPROTONAME, "BackgroundColour", opt.BGColour);
	DBWriteContactSettingDword(NULL, WEATHERPROTONAME, "TextColour", opt.TextColour);
	DBWriteContactSettingByte(NULL,WEATHERPROTONAME, "UseWinColors", (BYTE)opt.UseWinColors);
	// popup actions
	DBWriteContactSettingDword(NULL, WEATHERPROTONAME, "LeftClickAction", opt.LeftClickAction);
	DBWriteContactSettingDword(NULL, WEATHERPROTONAME, "RightClickAction", opt.RightClickAction);
	// popup delay
	DBWriteContactSettingDword(NULL, WEATHERPROTONAME, "PopupDelay", opt.pDelay);
	// popup texts
	DBWriteContactSettingString(NULL, WEATHERPROTONAME, "PopupTitle", opt.pTitle);
	DBWriteContactSettingString(NULL, WEATHERPROTONAME, "PopupText", opt.pText);
	// misc stuff
	DBWriteContactSettingString(NULL, WEATHERPROTONAME, "Default", opt.Default);
}

//============  OPTION INITIALIZATION  ============

// register the weather option pages
int OptInit(WPARAM wParam,LPARAM lParam) {
	OPTIONSDIALOGPAGE odp = {0};

	odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;

	// plugin options
	odp.position    = 95600;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS);
	odp.pfnDlgProc  = OptionsProc;
	odp.pszGroup    = LPGEN("Plugins");
	odp.pszTitle    = WEATHERPROTOTEXT;
	odp.pszTab      = LPGEN("General");
	odp.flags       = ODPF_BOLDGROUPS;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	// text options
	odp.pszTemplate = MAKEINTRESOURCE(IDD_TEXTOPT);
	odp.pfnDlgProc  = DlgProcText;
	odp.pszTab      = LPGEN("Display");
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	// if popup service exists, load the weather popup options
	if ((ServiceExists(MS_POPUP_ADDPOPUP))) 
	{
		odp.position      = 100000000;
		odp.pszTemplate   = MAKEINTRESOURCE(IDD_POPUP);
		odp.pszGroup      = LPGEN("PopUps");
		odp.groupPosition = 910000000;
		odp.pszTab        = NULL;
		odp.pfnDlgProc    = DlgPopUpOpts;
		CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	}

	return 0;
}

//============  MAIN OPTIONS  ============

// weather options
INT_PTR CALLBACK OptionsProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam) 
{
	char str[512];

	switch(msg) 
	{
	case WM_INITDIALOG:
		opt_startup = TRUE;
		TranslateDialogDefault(hdlg);
		// load settings
		_ltoa(opt.UpdateTime, str, 10);
		SetDlgItemTextWth(hdlg, IDC_UPDATETIME, str);
		SetDlgItemTextWth(hdlg, IDC_DEGREE, opt.DegreeSign);

		SendDlgItemMessage(hdlg, IDC_AVATARSPIN, UDM_SETRANGE32, 0, 999);
		SendDlgItemMessage(hdlg, IDC_AVATARSPIN, UDM_SETPOS, 0, opt.AvatarSize);
		SendDlgItemMessage(hdlg, IDC_AVATARSIZE, EM_LIMITTEXT, 3, 0);

		CheckDlgButton(hdlg, IDC_STARTUPUPD, opt.StartupUpdate);
		CheckDlgButton(hdlg, IDC_UPDATE, opt.AutoUpdate);
		CheckDlgButton(hdlg, IDC_PROTOCOND, !opt.NoProtoCondition);
		CheckDlgButton(hdlg, IDC_UPDCONDCHG, opt.UpdateOnlyConditionChanged);
		CheckDlgButton(hdlg, IDC_NEWWIN, opt.NewBrowserWin);
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

		opt_startup = FALSE;
		return 0;

	case WM_COMMAND:
		if (HIWORD(wparam)==BN_CLICKED && GetFocus()==(HWND)lparam)
			if (!opt_startup)	SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
		if (!((LOWORD(wparam) == IDC_UPDATE || LOWORD(wparam) == IDC_DEGREE) && 
			(HIWORD(wparam) != EN_CHANGE || (HWND)lparam != GetFocus())))
			if (!opt_startup)	SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
		return 0;

	case WM_NOTIFY:
		switch(((LPNMHDR)lparam)->code)
		{
		case PSN_APPLY:
			// change the status for weather protocol
			if (IsDlgButtonChecked(hdlg, IDC_PROTOCOND) && opt.DefStn != NULL)
			{
				old_status = status;
				status = DBGetContactSettingWord(opt.DefStn, WEATHERPROTONAME, "StatusIcon", NOSTATUSDATA);
				ProtoBroadcastAck(WEATHERPROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, status);
			}

			// get update time and remove the old timer
			GetDlgItemText(hdlg, IDC_UPDATETIME, str, sizeof(str));
			opt.UpdateTime = (WORD)atoi(str);
			if (opt.UpdateTime < 1)	opt.UpdateTime = 1;
			KillTimer(NULL, timerId);
			timerId = SetTimer(NULL, 0, opt.UpdateTime*60000, (TIMERPROC)timerProc);

			// other general options
			GetDlgItemText(hdlg, IDC_DEGREE, opt.DegreeSign, sizeof(opt.DegreeSign));
			opt.StartupUpdate = IsDlgButtonChecked(hdlg, IDC_STARTUPUPD);
			opt.AutoUpdate = IsDlgButtonChecked(hdlg, IDC_UPDATE);
			opt.NewBrowserWin = IsDlgButtonChecked(hdlg, IDC_NEWWIN);
			opt.NoProtoCondition = !IsDlgButtonChecked(hdlg, IDC_PROTOCOND);
			opt.DisCondIcon = IsDlgButtonChecked(hdlg, IDC_DISCONDICON);
			opt.UpdateOnlyConditionChanged = (BYTE)IsDlgButtonChecked(hdlg, IDC_UPDCONDCHG);
			opt.RemoveOldData = IsDlgButtonChecked(hdlg, IDC_REMOVEOLD);
			opt.MakeItalic = IsDlgButtonChecked(hdlg, IDC_MAKEI);
			opt.AvatarSize = GetDlgItemInt(hdlg, IDC_AVATARSIZE, NULL, FALSE);;
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
	SetDlgItemTextWth(hdlg, IDC_CTEXT, opt.cText);
	SetDlgItemTextWth(hdlg, IDC_BTITLE, opt.bTitle);
	SetDlgItemTextWth(hdlg, IDC_BTEXT, opt.bText);
	SetDlgItemTextWth(hdlg, IDC_ETEXT, opt.eText);
	SetDlgItemTextWth(hdlg, IDC_NTEXT, opt.nText);
	SetDlgItemTextWth(hdlg, IDC_HTEXT, opt.hText);
	SetDlgItemTextWth(hdlg, IDC_XTEXT, opt.xText);
	SetDlgItemTextWth(hdlg, IDC_BTITLE2, opt.sText);
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
INT_PTR CALLBACK DlgProcText(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	RECT rc, pos;
	HWND button;
	HMENU hMenu, hMenu1;
	char str[4096];
	switch (msg)
	{
	case WM_INITDIALOG:
		opt_startup = TRUE;
		// set windows position, make it top-most
		GetWindowRect(hdlg, &rc); 
		SetWindowPos(hdlg, HWND_TOPMOST, rc.left, rc.top, 0, 0, SWP_NOSIZE); 
		TranslateDialogDefault(hdlg);
		// generate the display text for variable list
		strcpy(str, Translate("%c\tcurrent condition\n%d\tcurrent date\n%e\tdewpoint\n%f\tfeel-like temp\n%h\ttoday's high\n%i\twind direction\n%l\ttoday's low\n%m\thumidity\n%n\tstation name\n%p\tpressure\n%r\tsunrise time\n%s\tstation ID\n%t\ttemperature\n%u\tupdate time\n%v\tvisibility\n%w\twind speed\n%y\tsun set"));
		SetDlgItemTextWth(hdlg, IDC_VARLIST, str);

		// make the more variable and other buttons flat
		SendMessage(GetDlgItem(hdlg,IDC_MORE), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hdlg,IDC_TM1), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hdlg,IDC_TM2), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hdlg,IDC_TM3), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hdlg,IDC_TM4), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hdlg,IDC_TM5), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hdlg,IDC_TM6), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hdlg,IDC_TM7), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hdlg,IDC_TM8), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hdlg,IDC_RESET), BUTTONSETASFLATBTN, 0, 0);
		// load the settings
		LoadTextSettings(hdlg);
		opt_startup = FALSE;
		return TRUE;

	case WM_COMMAND:
		if (opt_startup)	return TRUE;
		switch(LOWORD(wParam)) {
		case IDC_CTEXT:
		case IDC_BTITLE:
		case IDC_BTEXT:
		case IDC_NTEXT:
		case IDC_XTEXT:
		case IDC_ETEXT:
		case IDC_HTEXT:
		case IDC_BTITLE2:
			if (HIWORD(wParam) == EN_CHANGE)
				SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
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
			{
				static const char *varname[8] = {"C", "b", "B", "N", "X", "E", "H", "S"};
				static const int cname[8] = {IDC_CTEXT, IDC_BTITLE, IDC_BTEXT, IDC_NTEXT, IDC_XTEXT, 
					IDC_ETEXT, IDC_HTEXT, IDC_BTITLE2 };
				static char* const *var[8] = {&opt.cText, &opt.bTitle, &opt.bText, &opt.nText, 
					&opt.xText, &opt.eText, &opt.hText, &opt.sText };
				WEATHERINFO winfo;

				// display the menu
				button = GetDlgItem(hdlg, LOWORD(wParam));
				GetWindowRect(button, &pos); 
				hMenu  = LoadMenu(hInst, MAKEINTRESOURCE(IDR_TMMENU));
				hMenu1 = GetSubMenu(hMenu, 0);
				CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM)hMenu1, 0);
				switch(TrackPopupMenu(hMenu1, TPM_LEFTBUTTON|TPM_RETURNCMD, pos.left, pos.bottom, 0, hdlg, NULL)) 
				{
				case ID_MPREVIEW:
					// show the preview in a message box, using the weather data from the default station
					winfo = LoadWeatherInfo(opt.DefStn);
					GetDisplay(&winfo, *var[LOWORD(wParam)-IDC_TM1], str);
					MessageBox(NULL, str, Translate("Weather Protocol Text Preview"), MB_OK|MB_TOPMOST);
					break;

				case ID_MRESET:
					{
						unsigned varo = LOWORD(wParam) - IDC_TM1; 
						// remove the old setting from db and free memory
						char* vartmp = *var[varo]; 
						wfree(&vartmp);
						SetTextDefault(varname[varo]);
						SetDlgItemTextWth(hdlg, cname[varo], *var[varo]);
					}
					break;
				}
				DestroyMenu(hMenu);
				break;
			}

		case IDC_RESET:
			// left click action selection menu
			button = GetDlgItem(hdlg, IDC_RESET);
			GetWindowRect(button, &pos); 
			hMenu  = LoadMenu(hInst, MAKEINTRESOURCE(IDR_TMENU));
			hMenu1 = GetSubMenu(hMenu, 0);
			CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM)hMenu1, 0);
			switch(TrackPopupMenu(hMenu1, TPM_LEFTBUTTON|TPM_RETURNCMD, pos.left, pos.bottom, 0, hdlg, NULL)) 
			{
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
		switch(((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			{
				// save the option
				char textstr[MAX_TEXT_SIZE];
				// free memory for old settings
				FreeTextVar();
				// save new settings to memory
				GetDlgItemTextWth(hdlg, IDC_CTEXT, textstr, MAX_TEXT_SIZE);
				wSetData(&opt.cText, textstr);
				GetDlgItemTextWth(hdlg, IDC_BTEXT, textstr, MAX_TEXT_SIZE);
				wSetData(&opt.bText, textstr);
				GetDlgItemTextWth(hdlg, IDC_BTITLE, textstr, MAX_TEXT_SIZE);
				wSetData(&opt.bTitle, textstr);
				GetDlgItemTextWth(hdlg, IDC_ETEXT, textstr, MAX_TEXT_SIZE);
				wSetData(&opt.eText, textstr);
				GetDlgItemTextWth(hdlg, IDC_NTEXT, textstr, MAX_TEXT_SIZE);
				wSetData(&opt.nText, textstr);
				GetDlgItemTextWth(hdlg, IDC_HTEXT, textstr, MAX_TEXT_SIZE);
				wSetData(&opt.hText, textstr);
				GetDlgItemTextWth(hdlg, IDC_XTEXT, textstr, MAX_TEXT_SIZE);
				wSetData(&opt.xText, textstr);
				GetDlgItemTextWth(hdlg, IDC_BTITLE2, textstr, MAX_TEXT_SIZE);
				wSetData(&opt.sText, textstr);
				SaveOptions();
				UpdateAllInfo(0, 0);
				break;
			}
		}
		break;
	}
	return FALSE;
}

