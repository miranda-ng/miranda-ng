/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG team
Copyright (c) 2005-2009 Boris Krasnovskiy All Rights Reserved
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

/* This file contain the source related to weather popups, including popup
	options, popup display, and the code for popup process.
*/

#include "stdafx.h"

// variables for weather_popup.c
static MCONTACT hPopupContact;

/////////////////////////////////////////////////////////////////////////////////////////
// wrapper function for displaying weather warning popup by triggering an event (threaded)
// lpzText = error text
// kind = display type (see m_popup.h)

int CWeatherProto::WPShowMessage(const wchar_t *lpzText, int kind)
{
	if (!m_bPopups)
		return 0;

	if (kind == SM_WARNING)
		PUShowMessageW(lpzText, SM_WARNING);
	else if (kind == SM_NOTIFY)
		PUShowMessageW(lpzText, SM_NOTIFY);
	else if (kind == SM_WEATHERALERT) {
		POPUPDATAW ppd;
		wchar_t str1[512], str2[512];

		// get the 2 strings
		wcsncpy(str1, lpzText, _countof(str1) - 1);
		wcsncpy(str2, lpzText, _countof(str2) - 1);
		wchar_t *chop = wcschr(str1, 255);
		if (chop != nullptr)
			*chop = '\0';
		else
			str1[0] = 0;

		chop = wcschr(str2, 255);
		if (chop != nullptr)
			wcsncpy(str2, chop + 1, _countof(str2) - 1);
		else
			str2[0] = 0;

		// setup the popup
		ppd.lchIcon = (HICON)LoadImage(nullptr, MAKEINTRESOURCE(OIC_BANG), IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);
		mir_wstrcpy(ppd.lpwzContactName, str1);
		mir_wstrcpy(ppd.lpwzText, str2);
		ppd.colorBack = (opt.UseWinColors) ? GetSysColor(COLOR_BTNFACE) : opt.BGColour;
		ppd.colorText = (opt.UseWinColors) ? GetSysColor(COLOR_WINDOWTEXT) : opt.TextColour;
		ppd.iSeconds = opt.pDelay;
		PUAddPopupW(&ppd);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// popup dialog pocess
// for selecting actions when click on the popup window
// use for displaying contact menu

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	uint32_t ID;
	MCONTACT hContact = PUGetContact(hWnd);
	auto *ppro = (CWeatherProto*)Proto_GetContactInstance(hContact);

	switch (message) {
	case WM_COMMAND:
		ID = (ppro) ? ppro->opt.LeftClickAction : 0;
		if (ID != IDM_M7)  PUDeletePopup(hWnd);
		SendMessage(hPopupWindow, ID, hContact, LPARAM(ppro));
		return TRUE;

	case WM_CONTEXTMENU:
		ID = (ppro) ? ppro->opt.RightClickAction : IDM_M7;
		if (ID != IDM_M7)  PUDeletePopup(hWnd);
		SendMessage(hPopupWindow, ID, hContact, LPARAM(ppro));
		return TRUE;

	case UM_FREEPLUGINDATA:
		DestroyIcon((HICON)PUGetPluginData(hWnd));
		return FALSE;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// display weather popups
// wParam = the contact to display popup
// lParam = whether the weather data is changed or not

int CWeatherProto::WeatherPopup(MCONTACT hContact, bool bAlways)
{
	// determine if the popup should display or not
	if (m_bPopups && opt.UpdatePopup && (!opt.PopupOnChange || bAlways) && !getByte(hContact, "DPopUp")) {
		WEATHERINFO winfo = LoadWeatherInfo(hContact);

		// setup the popup
		POPUPDATAW ppd;
		ppd.lchContact = hContact;
		ppd.PluginData = ppd.lchIcon = GetStatusIcon(winfo.hContact);
		GetDisplay(&winfo, GetTextValue('P'), ppd.lpwzContactName);
		GetDisplay(&winfo, GetTextValue('p'), ppd.lpwzText);
		ppd.PluginWindowProc = PopupDlgProc;
		ppd.colorBack = (opt.UseWinColors) ? GetSysColor(COLOR_BTNFACE) : opt.BGColour;
		ppd.colorText = (opt.UseWinColors) ? GetSysColor(COLOR_WINDOWTEXT) : opt.TextColour;
		ppd.iSeconds = opt.pDelay;
		PUAddPopupW(&ppd);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// process for the popup window
// containing the code for popup actions

LRESULT CALLBACK CWeatherProto::PopupWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	HMENU hMenu;
	auto *ppro = (CWeatherProto *)lParam;

	switch (uMsg) {
	case IDM_M2:	// brief info
		ppro->BriefInfo(wParam, 0);
		break;

	case IDM_M3:	// read complete forecast
		ppro->LoadForecast(wParam, 0);
		break;

	case IDM_M4:	// display weather map
		ppro->WeatherMap(wParam, 0);
		break;

	case IDM_M5:	// open history window
		CallService(MS_HISTORY_SHOWCONTACTHISTORY, wParam, 0);
		break;

	case IDM_M6:	// open external log
		ppro->ViewLog(wParam, 0);
		break;

	case IDM_M7:	// display contact menu
		hMenu = Menu_BuildContactMenu(wParam);
		GetCursorPos(&pt);
		hPopupContact = wParam;
		TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, nullptr);
		DestroyMenu(hMenu);
		break;

	case IDM_M8:	// display contact detail
		CallService(MS_USERINFO_SHOWDIALOG, wParam, 0);

	case WM_COMMAND:	 //Needed by the contact's context menu
		if (Clist_MenuProcessCommand(LOWORD(wParam), MPCF_CONTACTMENU, hPopupContact))
			break;
		return FALSE;

	case WM_MEASUREITEM: //Needed by the contact's context menu
		return Menu_MeasureItem(lParam);

	case WM_DRAWITEM: //Needed by the contact's context menu
		return Menu_DrawItem(lParam);
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);//FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct
{
	wchar_t c;
	int  id;
	char *setting;
}
static controls[] =
{
	{ 'p', IDC_PText, "PopupText" },
	{ 'P', IDC_PTitle, "PopupTitle" },
};

// used to select the menu item for popup action menu
static void SelectMenuItem(HMENU hMenu, int Check)
{
	for (int i = 0; i <= GetMenuItemCount(hMenu) - 1; i++)
		CheckMenuItem(hMenu, i, MF_BYPOSITION | ((int)GetMenuItemID(hMenu, i) == Check) * 8);
}

class CPopupOptsDlg : public CWeatherDlgBase
{
	CCtrlEdit edtDelay;
	CCtrlCheck chkUseWin;
	CCtrlButton btnLeft, btnRight, btnPD1, btnPD2, btnPdef, btnVars, btnPreview;

	// temporary read the current option to memory
	// but does not write to the database
	void ReadPopupOpt(HWND hdlg)
	{
		wchar_t str[512];
		auto &opt = m_proto->opt;

		// popup colour
		opt.TextColour = SendDlgItemMessage(hdlg, IDC_TEXTCOLOUR, CPM_GETCOLOUR, 0, 0);
		opt.BGColour = SendDlgItemMessage(hdlg, IDC_BGCOLOUR, CPM_GETCOLOUR, 0, 0);

		// get delay time
		GetDlgItemText(hdlg, IDC_DELAY, str, _countof(str));
		int num = _wtoi(str);
		opt.pDelay = num;

		// other options
		opt.UseWinColors = (uint8_t)IsDlgButtonChecked(hdlg, IDC_USEWINCOLORS);
		opt.UpdatePopup = (uint8_t)IsDlgButtonChecked(hdlg, IDC_POP1);
		opt.AlertPopup = (uint8_t)IsDlgButtonChecked(hdlg, IDC_POP2);
		opt.PopupOnChange = (uint8_t)IsDlgButtonChecked(hdlg, IDC_CH);
		opt.ShowWarnings = (uint8_t)IsDlgButtonChecked(hdlg, IDC_W);
	}

public:
	CPopupOptsDlg(CWeatherProto *ppro) :
		CWeatherDlgBase(ppro, IDD_POPUP),
		btnPD1(this, IDC_PD1),
		btnPD2(this, IDC_PD2),
		btnPdef(this, IDC_PDEF),
		btnVars(this, IDC_VAR3),
		btnLeft(this, IDC_LeftClick),
		btnRight(this, IDC_RightClick),
		btnPreview(this, IDC_PREVIEW),
		edtDelay(this, IDC_DELAY),
		chkUseWin(this, IDC_USEWINCOLORS)
	{
		edtDelay.OnChange = Callback(this, &CPopupOptsDlg::onChanged_Delay);
		chkUseWin.OnChange = Callback(this, &CPopupOptsDlg::onChanged_UseWin);

		btnPD1.OnClick = Callback(this, &CPopupOptsDlg::onClick_PD1);
		btnPD2.OnClick = Callback(this, &CPopupOptsDlg::onClick_PD2);
		btnPdef.OnClick = Callback(this, &CPopupOptsDlg::onClick_Pdef);
		btnVars.OnClick = Callback(this, &CPopupOptsDlg::onClick_Vars);
		btnLeft.OnClick = Callback(this, &CPopupOptsDlg::onClick_Left);
		btnRight.OnClick = Callback(this, &CPopupOptsDlg::onClick_Right);
		btnPreview.OnClick = Callback(this, &CPopupOptsDlg::onClick_Preview);
	}

	bool OnInitDialog() override
	{
		m_proto->SaveOptions();

		// click actions
		HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_PMENU));
		HMENU hMenu1 = GetSubMenu(hMenu, 0);

		wchar_t str[512];
		auto &opt = m_proto->opt;
		GetMenuStringW(hMenu1, opt.LeftClickAction, str, _countof(str), MF_BYCOMMAND);
		SetDlgItemTextW(m_hwnd, IDC_LeftClick, TranslateW(str));
		GetMenuStringW(hMenu1, opt.RightClickAction, str, _countof(str), MF_BYCOMMAND);
		SetDlgItemTextW(m_hwnd, IDC_RightClick, TranslateW(str));
		DestroyMenu(hMenu);

		// other options
		CheckDlgButton(m_hwnd, IDC_POP2, opt.AlertPopup ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_POP1, opt.UpdatePopup ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_CH, opt.PopupOnChange ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_W, opt.ShowWarnings ? BST_CHECKED : BST_UNCHECKED);

		for (auto &it : controls)
			SetDlgItemText(m_hwnd, it.id, m_proto->GetTextValue(it.c));

		// setting popup delay option
		_ltow(opt.pDelay, str, 10);
		SetDlgItemText(m_hwnd, IDC_DELAY, str);
		if (opt.pDelay == -1)
			CheckRadioButton(m_hwnd, IDC_PD1, IDC_PD3, IDC_PD2);
		else if (opt.pDelay == 0)
			CheckRadioButton(m_hwnd, IDC_PD1, IDC_PD3, IDC_PD1);
		else
			CheckRadioButton(m_hwnd, IDC_PD1, IDC_PD3, IDC_PD3);

		// Colours. First step is configuring the colours.
		SendDlgItemMessage(m_hwnd, IDC_BGCOLOUR, CPM_SETCOLOUR, 0, opt.BGColour);
		SendDlgItemMessage(m_hwnd, IDC_TEXTCOLOUR, CPM_SETCOLOUR, 0, opt.TextColour);

		// Second step is disabling them if we want to use default Windows ones.
		CheckDlgButton(m_hwnd, IDC_USEWINCOLORS, opt.UseWinColors ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(m_hwnd, IDC_BGCOLOUR), !opt.UseWinColors);
		EnableWindow(GetDlgItem(m_hwnd, IDC_TEXTCOLOUR), !opt.UseWinColors);

		// buttons
		SendDlgItemMessage(m_hwnd, IDC_PREVIEW, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(m_hwnd, IDC_PDEF, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(m_hwnd, IDC_LeftClick, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(m_hwnd, IDC_RightClick, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(m_hwnd, IDC_VAR3, BUTTONSETASFLATBTN, TRUE, 0);
		return true;
	}

	bool OnApply() override
	{
		ReadPopupOpt(m_hwnd);

		wchar_t textstr[MAX_TEXT_SIZE];
		for (auto &it : controls) {
			GetDlgItemText(m_hwnd, it.id, textstr, _countof(textstr));
			if (!mir_wstrcmpi(textstr, GetDefaultText(it.c)))
				m_proto->delSetting(it.setting);
			else
				m_proto->setWString(it.setting, textstr);
		}

		// save the options, and update main menu
		m_proto->SaveOptions();
		return true;
	}

	void onChanged_UseWin(CCtrlCheck *)
	{
		// use window color - enable/disable color selection controls
		EnableWindow(GetDlgItem(m_hwnd, IDC_BGCOLOUR), !(m_proto->opt.UseWinColors));
		EnableWindow(GetDlgItem(m_hwnd, IDC_TEXTCOLOUR), !(m_proto->opt.UseWinColors));
	}

	void onClick_Right(CCtrlButton *)
	{
		// right click action selection menu
		RECT pos;
		GetWindowRect(GetDlgItem(m_hwnd, IDC_RightClick), &pos);

		HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_PMENU));
		HMENU hMenu1 = GetSubMenu(hMenu, 0);
		TranslateMenu(hMenu1);
		SelectMenuItem(hMenu1, m_proto->opt.RightClickAction);
		int ID = TrackPopupMenu(hMenu1, TPM_LEFTBUTTON | TPM_RETURNCMD, pos.left, pos.bottom, 0, m_hwnd, nullptr);
		if (ID)
			m_proto->opt.RightClickAction = ID;
		DestroyMenu(hMenu);

		hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_PMENU));
		hMenu1 = GetSubMenu(hMenu, 0);

		wchar_t str[512];
		GetMenuString(hMenu1, m_proto->opt.RightClickAction, str, _countof(str), MF_BYCOMMAND);
		SetDlgItemText(m_hwnd, IDC_RightClick, TranslateW(str));
		DestroyMenu(hMenu);
	}

	void onClick_Left(CCtrlButton *)
	{
		// left click action selection menu
		RECT pos;
		GetWindowRect(GetDlgItem(m_hwnd, IDC_LeftClick), &pos);

		HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_PMENU));
		HMENU hMenu1 = GetSubMenu(hMenu, 0);
		TranslateMenu(hMenu1);
		SelectMenuItem(hMenu1, m_proto->opt.LeftClickAction);
		int ID = TrackPopupMenu(hMenu1, TPM_LEFTBUTTON | TPM_RETURNCMD, pos.left, pos.bottom, 0, m_hwnd, nullptr);
		if (ID)
			m_proto->opt.LeftClickAction = ID;
		DestroyMenu(hMenu);

		hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_PMENU));
		hMenu1 = GetSubMenu(hMenu, 0);

		wchar_t str[512];
		GetMenuString(hMenu1, m_proto->opt.LeftClickAction, str, _countof(str), MF_BYCOMMAND);
		SetDlgItemText(m_hwnd, IDC_LeftClick, TranslateW(str));
		DestroyMenu(hMenu);
	}

	void onClick_PD1(CCtrlButton *)
	{
		// Popup delay setting from popup plugin
		SetDlgItemText(m_hwnd, IDC_DELAY, L"0");
		CheckRadioButton(m_hwnd, IDC_PD1, IDC_PD3, IDC_PD1);
	}

	void onClick_PD2(CCtrlButton *)
	{
		// Popup delay = permanent
		SetDlgItemText(m_hwnd, IDC_DELAY, L"-1");
		CheckRadioButton(m_hwnd, IDC_PD1, IDC_PD3, IDC_PD2);
	}

	void onChanged_Delay(CCtrlEdit *)
	{
		// if text is edited
		CheckRadioButton(m_hwnd, IDC_PD1, IDC_PD3, IDC_PD3);
	}

	void onClick_Pdef(CCtrlButton *)
	{
		// set the default value for popup texts
		for (auto &it : controls)
			SetDlgItemText(m_hwnd, it.id, GetDefaultText(it.c));
	}

	void onClick_Vars(CCtrlButton *)
	{
		// display variable list
		CMStringW wszText;
		wszText += L"                                                            \n"; // to make the message box wider
		wszText += TranslateT("%c\tcurrent condition\n%d\tcurrent date\n%e\tdewpoint\n%f\tfeel-like temperature\n%h\ttoday's high\n%i\twind direction\n%l\ttoday's low\n%m\thumidity\n%n\tstation name\n%p\tpressure\n%r\tsunrise time\n%s\tstation ID\n%t\ttemperature\n%u\tupdate time\n%v\tvisibility\n%w\twind speed\n%y\tsun set");
		wszText += L"\n";
		wszText += TranslateT("%[..]\tcustom variables");
		MessageBox(nullptr, wszText, TranslateT("Variable List"), MB_OK | MB_ICONASTERISK | MB_TOPMOST);
	}
	
	void onClick_Preview(CCtrlButton *)
	{
		// popup preview
		MCONTACT hContact = m_proto->opt.DefStn;
		ReadPopupOpt(m_hwnd);	// read new options to memory
		m_proto->WeatherPopup(m_proto->opt.DefStn, true);	// display popup using new opt
		m_proto->LoadOptions();		// restore old option in memory
		m_proto->opt.DefStn = hContact;
	}
};

void CWeatherProto::InitPopupOptions(WPARAM wParam)
{
	// if popup service exists, load the weather popup options
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.position = 100000000;
	odp.szGroup.w = LPGENW("Popups");
	odp.szTitle.w = m_tszUserName;
	odp.pDialog = new CPopupOptsDlg(this);
	g_plugin.addOptions(wParam, &odp);
}
