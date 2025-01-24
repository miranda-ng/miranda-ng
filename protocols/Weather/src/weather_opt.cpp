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

CMStringW CWeatherProto::GetTextValue(int c)
{
	CMStringW ret;

	switch (c) {
	case 'C': ret = getMStringW("DisplayText"); break;
	case 'b': ret = getMStringW("BriefTextTitle"); break;
	case 'B': ret = getMStringW("BriefText"); break;
	case 'N': ret = getMStringW("NoteText"); break;
	case 'E': ret = getMStringW("ExtText"); break;
	case 'H': ret = getMStringW("HistoryText"); break;
	case 'X': ret = getMStringW("ExtraText"); break;
	case 'S': ret = getMStringW("StatusText"); break;
	case 'P': ret = getMStringW("PopupTitle"); break;
	case 'p': ret = getMStringW("PopupText"); break;
	}

	return (ret.IsEmpty()) ? GetDefaultText(c) : ret;
}

// load options from database + set default if the setting does not exist

void CWeatherProto::LoadOptions(void)
{
	memset(&opt, 0, sizeof(opt));

	// main options
	opt.StartupUpdate = getByte("StartupUpdate", true);
	opt.AutoUpdate = getByte("AutoUpdate", true);
	opt.UpdateTime = getWord("UpdateTime", 30);
	opt.UpdateOnlyConditionChanged = getByte("CondChangeAsUpdate", true);
	opt.RemoveOldData = getByte("RemoveOld", false);
	opt.MakeItalic = getByte("MakeItalic", true);
	opt.AvatarSize = getByte("AvatarSize", 128);

	// units
	opt.tUnit = getWord("tUnit", 1);
	opt.wUnit = getWord("wUnit", 2);
	opt.vUnit = getWord("vUnit", 1);
	opt.pUnit = getWord("pUnit", 4);
	opt.dUnit = getWord("dUnit", 1);
	opt.eUnit = getWord("eUnit", 2);

	ptrW szValue(getWStringA("DegreeSign"));
	wcsncpy_s(opt.DegreeSign, (szValue == NULL) ? L"" : szValue, _TRUNCATE);

	opt.DoNotAppendUnit = getByte("DoNotAppendUnit", 0);
	opt.NoFrac = getByte("NoFractions", 0);

	// advanced
	opt.DisCondIcon = getByte("DisableConditionIcon", false);

	// popup options
	opt.UpdatePopup = getByte("UpdatePopup", true);
	opt.AlertPopup = getByte("AlertPopup", true);
	opt.PopupOnChange = getByte("PopUpOnChange", true);
	opt.ShowWarnings = getByte("ShowWarnings", true);

	// popup colors
	opt.BGColour = getDword("BackgroundColour", GetSysColor(COLOR_BTNFACE));
	opt.TextColour = getDword("TextColour", GetSysColor(COLOR_WINDOWTEXT));
	opt.UseWinColors = getByte("UseWinColors", false);

	// popup actions
	opt.LeftClickAction = getDword("LeftClickAction", IDM_M2);
	opt.RightClickAction = getDword("RightClickAction", IDM_M1);

	// popup delay
	opt.pDelay = getDword("PopupDelay", 0);

	// misc
	if (szValue = getWStringA("Default"))
		wcsncpy_s(opt.Default, szValue, _TRUNCATE);
	else
		opt.Default[0] = 0;
}

// save the options to database
void CWeatherProto::SaveOptions(void)
{
	// main options
	setByte("StartupUpdate", (uint8_t)opt.StartupUpdate);
	setByte("AutoUpdate", (uint8_t)opt.AutoUpdate);
	setWord("UpdateTime", opt.UpdateTime);
	setByte("CondChangeAsUpdate", (uint8_t)opt.UpdateOnlyConditionChanged);
	setByte("RemoveOld", (uint8_t)opt.RemoveOldData);
	setByte("MakeItalic", (uint8_t)opt.MakeItalic);
	setByte("AvatarSize", (uint8_t)opt.AvatarSize);
	
	// units
	setWord("tUnit", opt.tUnit);
	setWord("wUnit", opt.wUnit);
	setWord("vUnit", opt.vUnit);
	setWord("pUnit", opt.pUnit);
	setWord("dUnit", opt.dUnit);
	setWord("eUnit", opt.eUnit);
	setWString("DegreeSign", opt.DegreeSign);
	setByte("DoNotAppendUnit", (uint8_t)opt.DoNotAppendUnit);
	setByte("NoFractions", (uint8_t)opt.NoFrac);
	
	// advanced
	setByte("DisableConditionIcon", (uint8_t)opt.DisCondIcon);
	
	// popup options
	setByte("UpdatePopup", (uint8_t)opt.UpdatePopup);
	setByte("AlertPopup", (uint8_t)opt.AlertPopup);
	setByte("PopUpOnChange", (uint8_t)opt.PopupOnChange);
	setByte("ShowWarnings", (uint8_t)opt.ShowWarnings);
	
	// popup colors
	setDword("BackgroundColour", opt.BGColour);
	setDword("TextColour", opt.TextColour);
	setByte("UseWinColors", (uint8_t)opt.UseWinColors);
	
	// popup actions
	setDword("LeftClickAction", opt.LeftClickAction);
	setDword("RightClickAction", opt.RightClickAction);
	
	// popup delay
	setDword("PopupDelay", opt.pDelay);
	
	// misc stuff
	setWString("Default", opt.Default);
}

/////////////////////////////////////////////////////////////////////////////////////////
// weather options

class COptionsDlg : public CWeatherDlgBase
{
public:
	COptionsDlg(CWeatherProto *ppro) :
		CWeatherDlgBase(ppro, IDD_OPTIONS)
	{}

	bool OnInitDialog() override
	{
		wchar_t str[512];
		auto &opt = m_proto->opt;

		_ltow(opt.UpdateTime, str, 10);
		SetDlgItemTextW(m_hwnd, IDC_UPDATETIME, str);
		SetDlgItemTextW(m_hwnd, IDC_DEGREE, opt.DegreeSign);

		SendDlgItemMessage(m_hwnd, IDC_AVATARSPIN, UDM_SETRANGE32, 0, 999);
		SendDlgItemMessage(m_hwnd, IDC_AVATARSPIN, UDM_SETPOS, 0, opt.AvatarSize);
		SendDlgItemMessage(m_hwnd, IDC_AVATARSIZE, EM_LIMITTEXT, 3, 0);

		CheckDlgButton(m_hwnd, IDC_STARTUPUPD, opt.StartupUpdate ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_UPDATE, opt.AutoUpdate ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_UPDCONDCHG, opt.UpdateOnlyConditionChanged ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_REMOVEOLD, opt.RemoveOldData ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_MAKEI, opt.MakeItalic ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_DISCONDICON, opt.DisCondIcon ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_DONOTAPPUNITS, opt.DoNotAppendUnit ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_NOFRAC, opt.NoFrac ? BST_CHECKED : BST_UNCHECKED);

		// load units
		switch (opt.tUnit) {	// temperature
		case 1: CheckRadioButton(m_hwnd, IDC_T1, IDC_T2, IDC_T1); break;
		case 2: CheckRadioButton(m_hwnd, IDC_T1, IDC_T2, IDC_T2); break;
		}
		switch (opt.wUnit) {	// wind
		case 1: CheckRadioButton(m_hwnd, IDC_W1, IDC_W4, IDC_W1); break;
		case 2: CheckRadioButton(m_hwnd, IDC_W1, IDC_W4, IDC_W2); break;
		case 3: CheckRadioButton(m_hwnd, IDC_W1, IDC_W4, IDC_W3); break;
		case 4: CheckRadioButton(m_hwnd, IDC_W1, IDC_W4, IDC_W4); break;
		}
		switch (opt.vUnit) {	// visibility
		case 1: CheckRadioButton(m_hwnd, IDC_V1, IDC_V2, IDC_V1); break;
		case 2: CheckRadioButton(m_hwnd, IDC_V1, IDC_V2, IDC_V2); break;
		}
		switch (opt.pUnit) {	// pressure
		case 1: CheckRadioButton(m_hwnd, IDC_P1, IDC_P4, IDC_P1); break;
		case 2: CheckRadioButton(m_hwnd, IDC_P1, IDC_P4, IDC_P2); break;
		case 3: CheckRadioButton(m_hwnd, IDC_P1, IDC_P4, IDC_P3); break;
		case 4: CheckRadioButton(m_hwnd, IDC_P1, IDC_P4, IDC_P4); break;
		}
		switch (opt.dUnit) {	// pressure
		case 1: CheckRadioButton(m_hwnd, IDC_D1, IDC_D3, IDC_D1); break;
		case 2: CheckRadioButton(m_hwnd, IDC_D1, IDC_D3, IDC_D2); break;
		case 3: CheckRadioButton(m_hwnd, IDC_D1, IDC_D3, IDC_D3); break;
		}

		switch (opt.eUnit) {	// elev
		case 1: CheckRadioButton(m_hwnd, IDC_E1, IDC_E2, IDC_E1); break;
		case 2: CheckRadioButton(m_hwnd, IDC_E1, IDC_E2, IDC_E2); break;
		}

		return true;
	}

	bool OnApply() override
	{
		wchar_t str[512];
		auto &opt = m_proto->opt;

		// get update time and remove the old timer
		GetDlgItemText(m_hwnd, IDC_UPDATETIME, str, _countof(str));
		opt.UpdateTime = (uint16_t)_wtoi(str);
		if (opt.UpdateTime < 1)	opt.UpdateTime = 1;
		m_proto->RestartTimer();

		// other general options
		GetDlgItemText(m_hwnd, IDC_DEGREE, opt.DegreeSign, _countof(opt.DegreeSign));
		opt.StartupUpdate = IsDlgButtonChecked(m_hwnd, IDC_STARTUPUPD);
		opt.AutoUpdate = IsDlgButtonChecked(m_hwnd, IDC_UPDATE);
		opt.DisCondIcon = IsDlgButtonChecked(m_hwnd, IDC_DISCONDICON);
		opt.UpdateOnlyConditionChanged = (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_UPDCONDCHG);
		opt.RemoveOldData = IsDlgButtonChecked(m_hwnd, IDC_REMOVEOLD);
		opt.MakeItalic = IsDlgButtonChecked(m_hwnd, IDC_MAKEI);
		opt.AvatarSize = GetDlgItemInt(m_hwnd, IDC_AVATARSIZE, nullptr, FALSE);
		opt.DoNotAppendUnit = IsDlgButtonChecked(m_hwnd, IDC_DONOTAPPUNITS);
		opt.NoFrac = IsDlgButtonChecked(m_hwnd, IDC_NOFRAC);
		m_proto->UpdateMenu(opt.AutoUpdate);

		// save the units
		if (IsDlgButtonChecked(m_hwnd, IDC_T1)) opt.tUnit = 1;
		if (IsDlgButtonChecked(m_hwnd, IDC_T2)) opt.tUnit = 2;
		if (IsDlgButtonChecked(m_hwnd, IDC_W1)) opt.wUnit = 1;
		if (IsDlgButtonChecked(m_hwnd, IDC_W2)) opt.wUnit = 2;
		if (IsDlgButtonChecked(m_hwnd, IDC_W3)) opt.wUnit = 3;
		if (IsDlgButtonChecked(m_hwnd, IDC_W4)) opt.wUnit = 4;
		if (IsDlgButtonChecked(m_hwnd, IDC_V1)) opt.vUnit = 1;
		if (IsDlgButtonChecked(m_hwnd, IDC_V2)) opt.vUnit = 2;
		if (IsDlgButtonChecked(m_hwnd, IDC_P1)) opt.pUnit = 1;
		if (IsDlgButtonChecked(m_hwnd, IDC_P2)) opt.pUnit = 2;
		if (IsDlgButtonChecked(m_hwnd, IDC_P3)) opt.pUnit = 3;
		if (IsDlgButtonChecked(m_hwnd, IDC_P4)) opt.pUnit = 4;
		if (IsDlgButtonChecked(m_hwnd, IDC_D1)) opt.dUnit = 1;
		if (IsDlgButtonChecked(m_hwnd, IDC_D2)) opt.dUnit = 2;
		if (IsDlgButtonChecked(m_hwnd, IDC_D3)) opt.dUnit = 3;
		if (IsDlgButtonChecked(m_hwnd, IDC_E1)) opt.eUnit = 1;
		if (IsDlgButtonChecked(m_hwnd, IDC_E2)) opt.eUnit = 2;

		// save the new weather options
		m_proto->SaveOptions();

		RedrawFrame(0, 0);
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// text option dialog

#define VAR_LIST_OPT TranslateT("%c\tcurrent condition\n%d\tcurrent date\n%e\tdewpoint\n%f\tfeel-like temp\n%h\ttoday's high\n%i\twind direction\n%l\ttoday's low\n%m\thumidity\n%n\tstation name\n%p\tpressure\n%r\tsunrise time\n%s\tstation ID\n%t\ttemperature\n%u\tupdate time\n%v\tvisibility\n%w\twind speed\n%y\tsun set\n----------\n\\n\tnew line")

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

class COptionsTextDlg : public CWeatherDlgBase
{
	CCtrlMButton btnReset, tm1, tm2, tm3, tm4, tm5, tm6, tm7, tm8;

public:
	COptionsTextDlg(CWeatherProto *ppro) :
		CWeatherDlgBase(ppro, IDD_TEXTOPT),
		btnReset(this, IDC_RESET),
		tm1(this, IDC_TM1),
		tm2(this, IDC_TM2),
		tm3(this, IDC_TM3),
		tm4(this, IDC_TM4),
		tm5(this, IDC_TM5),
		tm6(this, IDC_TM6),
		tm7(this, IDC_TM7),
		tm8(this, IDC_TM8)
	{
		btnReset.OnClick = Callback(this, &COptionsTextDlg::onClick_Reset);

		tm1.OnClick = tm2.OnClick = tm3.OnClick = tm4.OnClick = tm5.OnClick = tm6.OnClick = tm7.OnClick = tm8.OnClick =
			Callback(this, &COptionsTextDlg::onClick_TM);
	}

	bool OnInitDialog() override
	{
		// set windows position, make it top-most
		RECT rc;
		GetWindowRect(m_hwnd, &rc);
		SetWindowPos(m_hwnd, HWND_TOPMOST, rc.left, rc.top, 0, 0, SWP_NOSIZE);

		// generate the display text for variable list
		SetDlgItemTextW(m_hwnd, IDC_VARLIST, VAR_LIST_OPT);

		for (auto &it : controls)
			SetDlgItemTextW(m_hwnd, it.id, m_proto->GetTextValue(it.c));

		// make the more variable and other buttons flat
		tm1.MakeFlat();
		tm2.MakeFlat();
		tm3.MakeFlat();
		tm4.MakeFlat();
		tm5.MakeFlat();
		tm6.MakeFlat();
		tm7.MakeFlat();
		tm8.MakeFlat();
		btnReset.MakeFlat();
		return true;
	}

	bool OnApply() override
	{
		// save the option
		wchar_t textstr[MAX_TEXT_SIZE];
		for (auto &it : controls) {
			GetDlgItemText(m_hwnd, it.id, textstr, _countof(textstr));
			if (!mir_wstrcmpi(textstr, GetDefaultText(it.c)))
				m_proto->delSetting(it.setting);
			else
				m_proto->setWString(it.setting, textstr);
		}

		m_proto->SaveOptions();
		m_proto->UpdateAllInfo(0, 0);
		return true;
	}

	void onClick_TM(CCtrlButton *pButton)
	{
		// display the menu
		RECT pos;
		GetWindowRect(pButton->GetHwnd(), &pos);
		HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_TMMENU));
		HMENU hMenu1 = GetSubMenu(hMenu, 0);
		TranslateMenu(hMenu1);
		{
			auto &var = controls[pButton->GetCtrlId() - IDC_TM1];

			switch (TrackPopupMenu(hMenu1, TPM_LEFTBUTTON | TPM_RETURNCMD, pos.left, pos.bottom, 0, m_hwnd, nullptr)) {
			case ID_MPREVIEW:
				{
					// show the preview in a message box, using the weather data from the default station
					WEATHERINFO winfo = m_proto->LoadWeatherInfo(m_proto->opt.DefStn);
					wchar_t buf[MAX_TEXT_SIZE], str[4096];
					GetDlgItemTextW(m_hwnd, var.id, buf, _countof(buf));
					GetDisplay(&winfo, buf, str);
					MessageBox(nullptr, str, TranslateT("Weather Protocol Text Preview"), MB_OK | MB_TOPMOST);
				}
				break;

			case ID_MRESET:
				SetDlgItemTextW(m_hwnd, var.id, GetDefaultText(var.c));
				break;
			}
			DestroyMenu(hMenu);
		}
	}

	void onClick_Reset(CCtrlButton *)
	{
		// left click action selection menu
		RECT pos;
		GetWindowRect(btnReset.GetHwnd(), &pos);
		HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_TMENU));
		HMENU hMenu1 = GetSubMenu(hMenu, 0);
		TranslateMenu(hMenu1);
		switch (TrackPopupMenu(hMenu1, TPM_LEFTBUTTON | TPM_RETURNCMD, pos.left, pos.bottom, 0, m_hwnd, nullptr)) {
		case ID_T1:
			// reset to the strings in memory, discard all changes
			for (auto &it : controls)
				SetDlgItemTextW(m_hwnd, it.id, m_proto->GetTextValue(it.c));
			break;

		case ID_T2:
			// reset to the default setting
			for (auto &it : controls)
				SetDlgItemTextW(m_hwnd, it.id, GetDefaultText(it.c));
			break;
		}
		DestroyMenu(hMenu);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// register the weather option pages

int CWeatherProto::OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.w = LPGENW("Network");
	odp.szTitle.w = m_tszUserName;
	odp.position = 95600;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;

	// plugin options
	odp.pDialog = new COptionsDlg(this);
	odp.szTab.w = LPGENW("General");
	g_plugin.addOptions(wParam, &odp);

	// text options
	odp.pDialog = new COptionsTextDlg(this);
	odp.szTab.w = LPGENW("Display");
	g_plugin.addOptions(wParam, &odp);

	if (m_bPopups)
		InitPopupOptions(wParam);
	return 0;
}
