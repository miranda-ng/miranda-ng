/*
Weather Protocol plugin for Miranda IM
Copyright (C) 2005-2009 Boris Krasnovskiy All Rights Reserved
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
int WeatherPopup(WPARAM wParam, LPARAM lParam) 
{
	// determine if the popup should display or not
	if (opt.UsePopup && opt.UpdatePopup && (!opt.PopupOnChange || (BOOL)lParam) &&
	      !DBGetContactSettingByte((HANDLE)wParam, WEATHERPROTONAME, "DPopUp", 0)) 
	{
		POPUPDATAEX ppd = {0};
		WEATHERINFO winfo;

	    // setup the popup
		ppd.lchContact = (HANDLE)wParam;
//		if ((HANDLE)wParam != NULL) {	// for actual contact
			winfo = LoadWeatherInfo((HANDLE)wParam);
			ppd.PluginData = ppd.lchIcon = LoadSkinnedProtoIcon(WEATHERPROTONAME, winfo.status);
			GetDisplay(&winfo, opt.pTitle, ppd.lpzContactName);
			GetDisplay(&winfo, opt.pText, ppd.lpzText);
			ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;
//		}
//		else {	// for preview
//			ppd.lchIcon = LoadSkinnedProtoIcon(WEATHERPROTONAME, ONLINE);
//			strcpy(ppd.lpzContactName, Translate("This is the name of the city"));
//			strcpy(ppd.lpzText, Translate("Here is a short weather description"));
//			ppd.PluginWindowProc = NULL;
//		}
		ppd.colorBack = (opt.UseWinColors)?GetSysColor(COLOR_BTNFACE):opt.BGColour;
		ppd.colorText = (opt.UseWinColors)?GetSysColor(COLOR_WINDOWTEXT):opt.TextColour;
		ppd.iSeconds = opt.pDelay;
		// display popups
		if (!ServiceExists(MS_POPUP_ADDPOPUPEX))	// old version
			CallService(MS_POPUP_ADDPOPUP, (WPARAM)&ppd, 0);
		else {	// new version with delay
			ppd.iSeconds = opt.pDelay;
			CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0);
		}
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
	if (!opt.UsePopup)	return 0;

	if ((DWORD)lParam == SM_WARNING)
		PUShowMessage((char*)wParam, SM_WARNING);
	else if ((DWORD)lParam == SM_NOTIFY)
		PUShowMessage((char*)wParam, SM_NOTIFY);
	else if ((DWORD)lParam == SM_WEATHERALERT) 
	{
		POPUPDATAEX ppd = {0};
		char *chop;
		char str1[512], str2[512];

		// get the 2 strings
		strcpy(str1, (char*)wParam);
		strcpy(str2, (char*)wParam);
		chop = strchr(str1, 255);
		if (chop != NULL)	*chop = '\0';
		else				str1[0] = 0;
		chop = strchr(str2, 255);
		if (chop != NULL)	strcpy(str2, chop+1);
		else				str2[0] = 0;

		// setup the popup
		ppd.lchIcon = (HICON)LoadImage(NULL, MAKEINTRESOURCE(OIC_BANG), IMAGE_ICON, 
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);
		strcpy(ppd.lpzContactName, str1);
		strcpy(ppd.lpzText, str2);
		ppd.colorBack = (opt.UseWinColors)?GetSysColor(COLOR_BTNFACE):opt.BGColour;
		ppd.colorText = (opt.UseWinColors)?GetSysColor(COLOR_WINDOWTEXT):opt.TextColour;
		ppd.iSeconds = opt.pDelay;

		// display popups
		if (!ServiceExists(MS_POPUP_ADDPOPUPEX))	// old version
			CallService(MS_POPUP_ADDPOPUP, (WPARAM)&ppd, 0);
		else {	// new version with delay
			ppd.iSeconds = opt.pDelay;
			CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0);
		}
	}
	return 0;
}

// wrapper function for displaying weather warning popup by triggering an event
//  (threaded)
// lpzText = error text
// kind = display type (see m_popup.h)
int WPShowMessage(char* lpzText, WORD kind) {
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
	HANDLE hContact;
	hContact = PUGetContact(hWnd);

	switch(message) 
	{
		case WM_COMMAND:
				ID = opt.LeftClickAction;
				if (ID != IDM_M7)  PUDeletePopUp(hWnd);
				SendMessage(hPopupWindow, ID, (WPARAM)hContact, 0);
				return TRUE;
		
		case WM_CONTEXTMENU:
				ID = opt.RightClickAction;
				if (ID != IDM_M7)  PUDeletePopUp(hWnd);
				SendMessage(hPopupWindow, ID, (WPARAM)hContact, 0);
				return TRUE;

		case UM_FREEPLUGINDATA:
			CallService(MS_SKIN2_RELEASEICON, (WPARAM)PUGetPluginData(hWnd), 0);
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
	switch (uMsg) 
	{
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
			hMenu=(HMENU)CallService(MS_CLIST_MENUBUILDCONTACT,wParam,0);
			GetCursorPos(&pt);
			hPopupContact = (HANDLE)wParam;
			TrackPopupMenu(hMenu,TPM_LEFTALIGN,pt.x,pt.y,0,hWnd,NULL);
			DestroyMenu(hMenu);
			break;

		case IDM_M8:	// display contact detail
			CallService(MS_USERINFO_SHOWDIALOG, wParam, 0);

		case WM_COMMAND:	 //Needed by the contact's context menu
			if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam),MPCF_CONTACTMENU),(LPARAM)hPopupContact))
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

// temporary read the current option to memory
// but does not write to the database
void ReadPopupOpt(HWND hdlg) 
{
	char text[MAX_TEXT_SIZE];
	int num;
	char str[512];

	// popup colour
	opt.TextColour = SendDlgItemMessage(hdlg,IDC_TEXTCOLOUR,CPM_GETCOLOUR,0,0);
	opt.BGColour = SendDlgItemMessage(hdlg,IDC_BGCOLOUR,CPM_GETCOLOUR,0,0);

	// get delay time
	GetDlgItemText(hdlg, IDC_DELAY, str, sizeof(str));
	num = atoi(str);
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
	GetDlgItemTextWth(hdlg, IDC_PText, text, MAX_TEXT_SIZE);
	wSetData(&opt.pText, text);
	GetDlgItemTextWth(hdlg, IDC_PTitle, text, MAX_TEXT_SIZE);
	wSetData(&opt.pTitle, text);
}

// copied and modified from NewStatusNotify
INT_PTR CALLBACK DlgPopUpOpts(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	int ID;
	char str[512];
	HMENU hMenu, hMenu1;
	RECT pos;
	HWND button;
	HANDLE hContact;

	switch (msg) 
	{
		case WM_INITDIALOG:
			TranslateDialogDefault(hdlg);
			SaveOptions();

			// click actions
			hMenu  = LoadMenu(hInst, MAKEINTRESOURCE(IDR_PMENU));
			hMenu1 = GetSubMenu(hMenu, 0);
			GetMenuString(hMenu1, opt.LeftClickAction, str, sizeof(str), MF_BYCOMMAND);
			SetDlgItemTextWth(hdlg, IDC_LeftClick, Translate(str));
			GetMenuString(hMenu1, opt.RightClickAction, str, sizeof(str), MF_BYCOMMAND);
			SetDlgItemTextWth(hdlg, IDC_RightClick, Translate(str));
			DestroyMenu(hMenu);

			// other options
			CheckDlgButton(hdlg, IDC_E, opt.UsePopup);
			CheckDlgButton(hdlg, IDC_POP2, opt.AlertPopup);
			CheckDlgButton(hdlg, IDC_POP1, opt.UpdatePopup);
			CheckDlgButton(hdlg, IDC_CH, opt.PopupOnChange);
			CheckDlgButton(hdlg, IDC_W, opt.ShowWarnings);
			SetDlgItemTextWth(hdlg,IDC_PText, opt.pText);
			SetDlgItemTextWth(hdlg,IDC_PTitle, opt.pTitle);
			// setting popup delay option
			_ltoa(opt.pDelay, str, 10);
			SetDlgItemTextWth(hdlg,IDC_DELAY, str);
			if (opt.pDelay == -1)
				CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD2);
			else if (opt.pDelay == 0)
				CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD1);
			else
				CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD3);
			//Colours. First step is configuring the colours.
			SendDlgItemMessage(hdlg,IDC_BGCOLOUR,CPM_SETCOLOUR,0,opt.BGColour);
			SendDlgItemMessage(hdlg,IDC_TEXTCOLOUR,CPM_SETCOLOUR,0,opt.TextColour);
			//Second step is disabling them if we want to use default Windows ones.
			CheckDlgButton(hdlg, IDC_USEWINCOLORS,opt.UseWinColors?BST_CHECKED:BST_UNCHECKED);
			EnableWindow(GetDlgItem(hdlg, IDC_BGCOLOUR), !opt.UseWinColors);
			EnableWindow(GetDlgItem(hdlg, IDC_TEXTCOLOUR), !opt.UseWinColors);

			// buttons
			SendMessage(GetDlgItem(hdlg,IDC_PREVIEW), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hdlg,IDC_PDEF), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hdlg,IDC_LeftClick), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hdlg,IDC_RightClick), BUTTONSETASFLATBTN, 0, 0);
			SendMessage(GetDlgItem(hdlg,IDC_VAR3), BUTTONSETASFLATBTN, 0, 0);

			return TRUE;

		case WM_COMMAND:
			// enable the "apply" button 
			if (HIWORD(wParam)==BN_CLICKED && GetFocus()==(HWND)lParam)
				SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
			if (!((LOWORD(wParam) == IDC_UPDATE || LOWORD(wParam) == IDC_DEGREE) && 
				(HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())))
				SendMessage(GetParent(hdlg),PSM_CHANGED,0,0);
			//These are simple clicks: we don't save, but we tell the Options Page to enable the "Apply" button.
			switch(LOWORD(wParam)) 
			{
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
					CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM)hMenu1, 0);
					SelectMenuItem(hMenu1, opt.RightClickAction);
					ID = TrackPopupMenu(hMenu1, TPM_LEFTBUTTON|TPM_RETURNCMD, pos.left, pos.bottom, 0, hdlg, NULL);
					if (ID)   opt.RightClickAction = ID;
					DestroyMenu(hMenu);

					hMenu  = LoadMenu(hInst, MAKEINTRESOURCE(IDR_PMENU));
					hMenu1 = GetSubMenu(hMenu, 0);
					GetMenuString(hMenu1, opt.RightClickAction, str, sizeof(str), MF_BYCOMMAND);
					SetDlgItemTextWth(hdlg, IDC_RightClick, Translate(str));
					DestroyMenu(hMenu);
					break;

				case IDC_LeftClick:
					// left click action selection menu
					button = GetDlgItem(hdlg, IDC_LeftClick);
					GetWindowRect(button, &pos); 

					hMenu  = LoadMenu(hInst, MAKEINTRESOURCE(IDR_PMENU));
					hMenu1 = GetSubMenu(hMenu, 0);
					CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM)hMenu1, 0);
					SelectMenuItem(hMenu1, opt.LeftClickAction);
					ID = TrackPopupMenu(hMenu1, TPM_LEFTBUTTON|TPM_RETURNCMD, pos.left, pos.bottom, 0, hdlg, NULL);
					if (ID)   opt.LeftClickAction = ID;
					DestroyMenu(hMenu);

					hMenu  = LoadMenu(hInst, MAKEINTRESOURCE(IDR_PMENU));
					hMenu1 = GetSubMenu(hMenu, 0);
					GetMenuString(hMenu1, opt.LeftClickAction, str, sizeof(str), MF_BYCOMMAND);
					SetDlgItemTextWth(hdlg, IDC_LeftClick, Translate(str));
					DestroyMenu(hMenu);
					break;

				case IDC_PD1:
					// Popup delay setting from PopUp plugin
					SetDlgItemText(hdlg, IDC_DELAY, "0");
					CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD1);
					break;

				case IDC_PD2:
					// Popup delay = permanent
					SetDlgItemText(hdlg, IDC_DELAY, "-1");
					CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD2);
					break;

				case IDC_DELAY:
					// if text is edited
					CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD3);
					break;

				case IDC_PDEF:
					// set the default value for popup texts
					SetTextDefault("Pp");
					SetDlgItemTextWth(hdlg,IDC_PText, opt.pText);
					SetDlgItemTextWth(hdlg,IDC_PTitle, opt.pTitle);
					wfree(&opt.pText);
					wfree(&opt.pTitle);
					break;

				case IDC_VAR3:
					// display variable list
					strcpy(str, "                                                            \n");		// to make the message box wider
					strcat(str, Translate("%c\tcurrent condition\n%d\tcurrent date\n%e\tdewpoint\n%f\tfeel-like temperature\n%h\ttoday's high\n%i\twind direction\n%l\ttoday's low\n%m\thumidity\n%n\tstation name\n%p\tpressure\n%r\tsunrise time\n%s\tstation ID\n%t\ttemperature\n%u\tupdate time\n%v\tvisibility\n%w\twind speed\n%y\tsun set"));
					strcat(str, "\n");
					strcat(str, Translate("%[..]\tcustom variables"));
					MessageBox(NULL, str, Translate("Variable List"), MB_OK|MB_ICONASTERISK|MB_TOPMOST);
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
		//End WM_COMMAND
		case WM_NOTIFY: //Here we have pressed either the OK or the APPLY button.
			switch (((LPNMHDR)lParam)->code) 
			{
				case PSN_APPLY: 
				{
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

// used to select the menu item for popup action menu
void SelectMenuItem(HMENU hMenu, int Check) 
{
	int i;
	for (i=0; i<=GetMenuItemCount(hMenu)-1; i++)
		CheckMenuItem(hMenu, i, MF_BYPOSITION|((int)GetMenuItemID(hMenu, i) == Check)*8);
}

