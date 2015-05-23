/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG Team
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

#include "weather.h"

// variables for weather_popup.c
static HANDLE hPopupContact;

//============  SHOW WEATHER POPUPS  ============

// display weather popups
// wParam = the contact to display popup
// lParam = whether the weather data is changed or not
int WeatherPopup(WPARAM hContact, LPARAM lParam) 
{
	// determine if the popup should display or not
	if (opt.UsePopup && opt.UpdatePopup && (!opt.PopupOnChange || (BOOL)lParam) &&
		!db_get_b(hContact, WEATHERPROTONAME, "DPopUp", 0))
	{
		WEATHERINFO winfo = LoadWeatherInfo(hContact);

		// setup the popup
		POPUPDATAT ppd = { 0 };
		ppd.lchContact = hContact;
		ppd.PluginData = ppd.lchIcon = LoadSkinnedProtoIcon(WEATHERPROTONAME, winfo.status);
		GetDisplay(&winfo, opt.pTitle, ppd.lptzContactName);
		GetDisplay(&winfo, opt.pText, ppd.lptzText);
		ppd.PluginWindowProc = PopupDlgProc;
		ppd.colorBack = (opt.UseWinColors)?GetSysColor(COLOR_BTNFACE):opt.BGColour;
		ppd.colorText = (opt.UseWinColors)?GetSysColor(COLOR_WINDOWTEXT):opt.TextColour;
		ppd.iSeconds = opt.pDelay;
		PUAddPopupT( &ppd );
	}
	return 0;
}

//============  WEATHER ERROR POPUPS  ============

// display weather error or notices (not threaded)
// wParam = error text
// lParam = display type
// Type can either be SM_WARNING, SM_NOTIFY, or SM_WEATHERALERT

int WeatherError(WPARAM wParam, LPARAM lParam) 
{
	if ( !opt.UsePopup)
		return 0;

	TCHAR* tszMsg = ( TCHAR* )wParam;

	if ((DWORD)lParam == SM_WARNING)
		PUShowMessageT( tszMsg, SM_WARNING );
	else if ((DWORD)lParam == SM_NOTIFY)
		PUShowMessageT( tszMsg, SM_NOTIFY);
	else if ((DWORD)lParam == SM_WEATHERALERT) 
	{
		POPUPDATAT ppd = {0};
		TCHAR str1[512], str2[512];

		// get the 2 strings
		_tcsncpy(str1, tszMsg, SIZEOF(str1) - 1);
		_tcsncpy(str2, tszMsg, SIZEOF(str2) - 1);
		TCHAR *chop = _tcschr(str1, 255);
		if (chop != NULL)
			*chop = '\0';
		else
			str1[0] = 0;
		chop = _tcschr(str2, 255);
		if (chop != NULL)
			_tcsncpy(str2, chop+1, SIZEOF(str2) - 1);
		else
			str2[0] = 0;

		// setup the popup
		ppd.lchIcon = (HICON)LoadImage(NULL, MAKEINTRESOURCE(OIC_BANG), IMAGE_ICON, 
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);
		mir_tstrcpy(ppd.lptzContactName, str1);
		mir_tstrcpy(ppd.lptzText, str2);
		ppd.colorBack = (opt.UseWinColors)?GetSysColor(COLOR_BTNFACE):opt.BGColour;
		ppd.colorText = (opt.UseWinColors)?GetSysColor(COLOR_WINDOWTEXT):opt.TextColour;
		ppd.iSeconds = opt.pDelay;
		PUAddPopupT( &ppd );
	}
	return 0;
}

// wrapper function for displaying weather warning popup by triggering an event
//  (threaded)
// lpzText = error text
// kind = display type (see m_popup.h)

int WPShowMessage(TCHAR* lpzText, WORD kind)
{
	NotifyEventHooks(hHookWeatherError, (WPARAM)lpzText, (LPARAM)kind);
	return 0;
}

//============  WEATHER POPUP PROCESSES  ============

// popup dialog pocess
// for selecting actions when click on the popup window
// use for displaying contact menu
LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
	DWORD ID = 0;
	MCONTACT hContact;
	hContact = PUGetContact(hWnd);

	switch(message) {
	case WM_COMMAND:
		ID = opt.LeftClickAction;
		if (ID != IDM_M7)  PUDeletePopup(hWnd);
		SendMessage(hPopupWindow, ID, hContact, 0);
		return TRUE;

	case WM_CONTEXTMENU:
		ID = opt.RightClickAction;
		if (ID != IDM_M7)  PUDeletePopup(hWnd);
		SendMessage(hPopupWindow, ID, hContact, 0);
		return TRUE;

	case UM_FREEPLUGINDATA:
		Skin_ReleaseIcon((HICON)PUGetPluginData(hWnd));
		return FALSE;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

// process for the popup window
// containing the code for popup actions
LRESULT CALLBACK PopupWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	POINT pt;
	HMENU hMenu;
	switch (uMsg) {
	case IDM_M2:	// brief info
		BriefInfo(wParam, 0);
		break;

	case IDM_M3:	// read complete forecast
		LoadForecast(wParam, 0);
		break;

	case IDM_M4:	// display weather map
		WeatherMap(wParam, 0);
		break;

	case IDM_M5:	// open history window
		CallService(MS_HISTORY_SHOWCONTACTHISTORY, wParam, 0);
		break;

	case IDM_M6:	// open external log
		ViewLog(wParam, 0);
		break;

	case IDM_M7:	// display contact menu
		hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT,wParam,0);
		GetCursorPos(&pt);
		hPopupContact = (HANDLE)wParam;
		TrackPopupMenu(hMenu,TPM_LEFTALIGN,pt.x,pt.y,0,hWnd,NULL);
		DestroyMenu(hMenu);
		break;

	case IDM_M8:	// display contact detail
		CallService(MS_USERINFO_SHOWDIALOG, wParam, 0);

	case WM_COMMAND:	 //Needed by the contact's context menu
		if ( CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam),MPCF_CONTACTMENU), (LPARAM)hPopupContact))
			break;
		return FALSE;

	case WM_MEASUREITEM: //Needed by the contact's context menu
		return CallService(MS_CLIST_MENUMEASUREITEM,wParam,lParam);

	case WM_DRAWITEM: //Needed by the contact's context menu
		return CallService(MS_CLIST_MENUDRAWITEM,wParam,lParam);
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);//FALSE;
}

//============  POPUP OPTIONS  ============

// used to select the menu item for popup action menu
static void SelectMenuItem(HMENU hMenu, int Check) 
{
	for (int i=0; i <= GetMenuItemCount(hMenu)-1; i++)
		CheckMenuItem(hMenu, i, MF_BYPOSITION|((int)GetMenuItemID(hMenu, i) == Check)*8);
}

// temporary read the current option to memory
// but does not write to the database
void ReadPopupOpt(HWND hdlg) 
{
	TCHAR text[MAX_TEXT_SIZE];
	TCHAR str[512];

	// popup colour
	opt.TextColour = SendDlgItemMessage(hdlg,IDC_TEXTCOLOUR,CPM_GETCOLOUR,0,0);
	opt.BGColour = SendDlgItemMessage(hdlg,IDC_BGCOLOUR,CPM_GETCOLOUR,0,0);

	// get delay time
	GetDlgItemText(hdlg, IDC_DELAY, str, SIZEOF(str));
	int num = _ttoi(str);
	opt.pDelay = num;

	// other options
	opt.UseWinColors = (BYTE)IsDlgButtonChecked(hdlg, IDC_USEWINCOLORS);
	opt.UsePopup = (BYTE)IsDlgButtonChecked(hdlg, IDC_E);
	opt.UpdatePopup = (BYTE)IsDlgButtonChecked(hdlg, IDC_POP1);
	opt.AlertPopup = (BYTE)IsDlgButtonChecked(hdlg, IDC_POP2);
	opt.PopupOnChange = (BYTE)IsDlgButtonChecked(hdlg, IDC_CH);
	opt.ShowWarnings = (BYTE)IsDlgButtonChecked(hdlg, IDC_W);

	// popup texts
	wfree(&opt.pText);
	wfree(&opt.pTitle);
	GetDlgItemText(hdlg, IDC_PText, text, SIZEOF(text));
	wSetData(&opt.pText, text);
	GetDlgItemText(hdlg, IDC_PTitle, text, SIZEOF(text));
	wSetData(&opt.pTitle, text);
}

// copied and modified from NewStatusNotify
INT_PTR CALLBACK DlgPopupOpts(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	int ID;
	TCHAR str[512];
	HMENU hMenu, hMenu1;
	RECT pos;
	HWND button;
	MCONTACT hContact;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		SaveOptions();

		// click actions
		hMenu  = LoadMenu(hInst, MAKEINTRESOURCE(IDR_PMENU));
		hMenu1 = GetSubMenu(hMenu, 0);
		GetMenuString(hMenu1, opt.LeftClickAction, str, SIZEOF(str), MF_BYCOMMAND);
		SetDlgItemText(hdlg, IDC_LeftClick, TranslateTS(str));
		GetMenuString(hMenu1, opt.RightClickAction, str, SIZEOF(str), MF_BYCOMMAND);
		SetDlgItemText(hdlg, IDC_RightClick, TranslateTS(str));
		DestroyMenu(hMenu);

		// other options
		CheckDlgButton(hdlg, IDC_E, opt.UsePopup ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_POP2, opt.AlertPopup ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_POP1, opt.UpdatePopup ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_CH, opt.PopupOnChange ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_W, opt.ShowWarnings ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemText(hdlg,IDC_PText, opt.pText);
		SetDlgItemText(hdlg,IDC_PTitle, opt.pTitle);
		// setting popup delay option
		_ltot(opt.pDelay, str, 10);
		SetDlgItemText(hdlg,IDC_DELAY, str);
		if (opt.pDelay == -1)
			CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD2);
		else if (opt.pDelay == 0)
			CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD1);
		else
			CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD3);
		//Colours. First step is configuring the colours.
		SendDlgItemMessage(hdlg, IDC_BGCOLOUR, CPM_SETCOLOUR, 0, opt.BGColour);
		SendDlgItemMessage(hdlg, IDC_TEXTCOLOUR, CPM_SETCOLOUR, 0, opt.TextColour);
		//Second step is disabling them if we want to use default Windows ones.
		CheckDlgButton(hdlg, IDC_USEWINCOLORS, opt.UseWinColors ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hdlg, IDC_BGCOLOUR), !opt.UseWinColors);
		EnableWindow(GetDlgItem(hdlg, IDC_TEXTCOLOUR), !opt.UseWinColors);

		// buttons
		SendDlgItemMessage(hdlg, IDC_PREVIEW, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hdlg, IDC_PDEF, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hdlg, IDC_LeftClick, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hdlg, IDC_RightClick, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hdlg, IDC_VAR3, BUTTONSETASFLATBTN, TRUE, 0);
		return TRUE;

	case WM_COMMAND:
		// enable the "apply" button 
		if (HIWORD(wParam) == BN_CLICKED && GetFocus() == (HWND)lParam)
			SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
		if ( !((LOWORD(wParam) == IDC_UPDATE || LOWORD(wParam) == IDC_DEGREE) && 
			(HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())))
			SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
		//These are simple clicks: we don't save, but we tell the Options Page to enable the "Apply" button.
		switch(LOWORD(wParam)) {
		case IDC_BGCOLOUR: //Fall through
		case IDC_TEXTCOLOUR:
			// select new colors
			if (HIWORD(wParam) == CPN_COLOURCHANGED)
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_USEWINCOLORS:
			// use window color - enable/disable color selection controls
			EnableWindow(GetDlgItem(hdlg, IDC_BGCOLOUR), !(opt.UseWinColors));
			EnableWindow(GetDlgItem(hdlg, IDC_TEXTCOLOUR), !(opt.UseWinColors));
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_E:
		case IDC_CH:
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_RightClick:
			// right click action selection menu
			button = GetDlgItem(hdlg, IDC_RightClick);
			GetWindowRect(button, &pos); 

			hMenu  = LoadMenu(hInst, MAKEINTRESOURCE(IDR_PMENU));
			hMenu1 = GetSubMenu(hMenu, 0);
			TranslateMenu(hMenu1);
			SelectMenuItem(hMenu1, opt.RightClickAction);
			ID = TrackPopupMenu(hMenu1, TPM_LEFTBUTTON|TPM_RETURNCMD, pos.left, pos.bottom, 0, hdlg, NULL);
			if (ID)   opt.RightClickAction = ID;
			DestroyMenu(hMenu);

			hMenu  = LoadMenu(hInst, MAKEINTRESOURCE(IDR_PMENU));
			hMenu1 = GetSubMenu(hMenu, 0);
			GetMenuString(hMenu1, opt.RightClickAction, str, sizeof(str), MF_BYCOMMAND);
			SetDlgItemText(hdlg, IDC_RightClick, TranslateTS(str));
			DestroyMenu(hMenu);
			break;

		case IDC_LeftClick:
			// left click action selection menu
			button = GetDlgItem(hdlg, IDC_LeftClick);
			GetWindowRect(button, &pos); 

			hMenu  = LoadMenu(hInst, MAKEINTRESOURCE(IDR_PMENU));
			hMenu1 = GetSubMenu(hMenu, 0);
			TranslateMenu(hMenu1);
			SelectMenuItem(hMenu1, opt.LeftClickAction);
			ID = TrackPopupMenu(hMenu1, TPM_LEFTBUTTON|TPM_RETURNCMD, pos.left, pos.bottom, 0, hdlg, NULL);
			if (ID)   opt.LeftClickAction = ID;
			DestroyMenu(hMenu);

			hMenu  = LoadMenu(hInst, MAKEINTRESOURCE(IDR_PMENU));
			hMenu1 = GetSubMenu(hMenu, 0);
			GetMenuString(hMenu1, opt.LeftClickAction, str, sizeof(str), MF_BYCOMMAND);
			SetDlgItemText(hdlg, IDC_LeftClick, TranslateTS(str));
			DestroyMenu(hMenu);
			break;

		case IDC_PD1:
			// Popup delay setting from Popup plugin
			SetDlgItemText(hdlg, IDC_DELAY, _T("0"));
			CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD1);
			break;

		case IDC_PD2:
			// Popup delay = permanent
			SetDlgItemText(hdlg, IDC_DELAY, _T("-1"));
			CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD2);
			break;

		case IDC_DELAY:
			// if text is edited
			CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD3);
			break;

		case IDC_PDEF:
			// set the default value for popup texts
			SetTextDefault("Pp");
			SetDlgItemText(hdlg,IDC_PText, opt.pText);
			SetDlgItemText(hdlg,IDC_PTitle, opt.pTitle);
			wfree(&opt.pText);
			wfree(&opt.pTitle);
			break;

		case IDC_VAR3:
			// display variable list
			_tcsncpy(str, _T("                                                            \n"),SIZEOF(str) - 1);		// to make the message box wider
			mir_tstrncat(str, VAR_LIST_POPUP, SIZEOF(str) - mir_tstrlen(str));
			mir_tstrncat(str, _T("\n"), SIZEOF(str) - mir_tstrlen(str));
			mir_tstrncat(str, CUSTOM_VARS, SIZEOF(str) - mir_tstrlen(str));
			MessageBox(NULL, str, TranslateT("Variable List"), MB_OK|MB_ICONASTERISK|MB_TOPMOST);
			break;

		case IDC_PREVIEW:
			// popup preview
			hContact = opt.DefStn;
			ReadPopupOpt(hdlg);	// read new options to memory
			WeatherPopup((WPARAM)opt.DefStn, (BOOL)TRUE);	// display popup using new opt
			DestroyOptions();
			LoadOptions();		// restore old option in memory
			opt.DefStn = hContact;
			break;
		}
		break;
	
	case WM_NOTIFY: //Here we have pressed either the OK or the APPLY button.
		switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY: {
				ReadPopupOpt(hdlg);

				// save the options, and update main menu
				SaveOptions();
				UpdatePopupMenu(opt.UsePopup);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}
