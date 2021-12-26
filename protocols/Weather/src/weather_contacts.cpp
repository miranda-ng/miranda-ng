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

/* This file contain the source that is related to weather contacts,
include the links, edit settings, and loading weather information for
the contact.
*/

#include "stdafx.h"

static void OpenUrl(wchar_t *format, wchar_t *id)
{
	wchar_t loc[512];
	GetID(id);
	mir_snwprintf(loc, format, id);
	Utils_OpenUrlW(loc);
}

//============ BASIC CONTACTS FUNCTIONS AND LINKS  ============

// view weather log for the contact
// wParam = current contact
INT_PTR ViewLog(WPARAM wParam, LPARAM lParam)
{
	// see if the log path is set
	DBVARIANT dbv;
	if (!g_plugin.getWString(wParam, "Log", &dbv)) {
		if (dbv.pszVal[0] != 0)
			ShellExecute((HWND)lParam, L"open", dbv.pwszVal, L"", L"", SW_SHOW);
		db_free(&dbv);
	}
	else	// display warning dialog if no path is specified
		MessageBox(nullptr, TranslateT("Weather condition was not logged."),
			TranslateT("Weather Protocol"), MB_OK | MB_ICONINFORMATION);
	return 0;
}

// read complete forecast
// wParam = current contact
INT_PTR LoadForecast(WPARAM wParam, LPARAM)
{
	wchar_t id[256], loc2[256];
	GetStationID(wParam, id, _countof(id));
	if (id[0] != 0) {
		// check if the complte forecast URL is set. If it is not, display warning and quit
		if (db_get_wstatic(wParam, MODULENAME, "InfoURL", loc2, _countof(loc2)) || loc2[0] == 0) {
			MessageBox(nullptr, TranslateT("The URL for complete forecast has not been set. You can set it from the Edit Settings dialog."), TranslateT("Weather Protocol"), MB_ICONINFORMATION);
			return 1;
		}
		// set the url and open the webpage
		OpenUrl(loc2, id);
	}
	return 0;
}

// load weather map
// wParam = current contact
INT_PTR WeatherMap(WPARAM wParam, LPARAM)
{
	wchar_t id[256], loc2[256];
	GetStationID(wParam, id, _countof(id));
	if (id[0] != 0) {
		// check if the weather map URL is set. If it is not, display warning and quit
		if (db_get_wstatic(wParam, MODULENAME, "MapURL", loc2, _countof(loc2)) || loc2[0] == 0) {
			MessageBox(nullptr, TranslateT("The URL for weather map has not been set. You can set it from the Edit Settings dialog."), TranslateT("Weather Protocol"), MB_ICONINFORMATION);
			return 1;
		}

		// set the url and open the webpage
		OpenUrl(loc2, id);
	}

	return 0;
}

//============ EDIT SETTINGS  ============

typedef struct
{
	MCONTACT hContact;
	HICON  hRename;
	HICON  hUserDetail;
	HICON  hFile;
	HICON  hSrchAll;
} CntSetWndDataType;

// edit weather settings
// lParam = current contact
static INT_PTR CALLBACK DlgProcChange(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
	wchar_t str[MAX_DATA_LEN], str2[256], city[256], filter[256], *chop;
	char loc[512];
	OPENFILENAME ofn;       // common dialog box structure
	MCONTACT hContact;
	WIDATA *sData;
	CntSetWndDataType *wndData = nullptr;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		wndData = (CntSetWndDataType*)mir_alloc(sizeof(CntSetWndDataType));
		wndData->hContact = hContact = lParam;
		wndData->hRename = Skin_LoadIcon(SKINICON_OTHER_RENAME);
		wndData->hUserDetail = Skin_LoadIcon(SKINICON_OTHER_USERDETAILS);
		wndData->hFile = Skin_LoadIcon(SKINICON_EVENT_FILE);
		wndData->hSrchAll = Skin_LoadIcon(SKINICON_OTHER_SEARCHALL);

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)wndData);

		// set button images
		SendDlgItemMessage(hwndDlg, IDC_GETNAME, BM_SETIMAGE, IMAGE_ICON, (LPARAM)wndData->hRename);
		SendDlgItemMessage(hwndDlg, IDC_SVCINFO, BM_SETIMAGE, IMAGE_ICON, (LPARAM)wndData->hUserDetail);
		SendDlgItemMessage(hwndDlg, IDC_BROWSE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)wndData->hFile);
		SendDlgItemMessage(hwndDlg, IDC_VIEW1, BM_SETIMAGE, IMAGE_ICON, (LPARAM)wndData->hSrchAll);
		SendDlgItemMessage(hwndDlg, IDC_RESET1, BM_SETIMAGE, IMAGE_ICON, (LPARAM)wndData->hRename);
		SendDlgItemMessage(hwndDlg, IDC_VIEW2, BM_SETIMAGE, IMAGE_ICON, (LPARAM)wndData->hSrchAll);
		SendDlgItemMessage(hwndDlg, IDC_RESET2, BM_SETIMAGE, IMAGE_ICON, (LPARAM)wndData->hRename);

		// make all buttons flat
		SendDlgItemMessage(hwndDlg, IDC_GETNAME, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_SVCINFO, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_BROWSE, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_VIEW1, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_RESET1, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_VIEW2, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_RESET2, BUTTONSETASFLATBTN, TRUE, 0);

		// set tooltip for the buttons
		SendDlgItemMessage(hwndDlg, IDC_GETNAME, BUTTONADDTOOLTIP, (WPARAM)LPGENW("Get city name from ID"), BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDC_SVCINFO, BUTTONADDTOOLTIP, (WPARAM)LPGENW("Weather INI information"), BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDC_BROWSE, BUTTONADDTOOLTIP, (WPARAM)LPGENW("Browse"), BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDC_VIEW1, BUTTONADDTOOLTIP, (WPARAM)LPGENW("View webpage"), BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDC_RESET1, BUTTONADDTOOLTIP, (WPARAM)LPGENW("Reset to default"), BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDC_VIEW2, BUTTONADDTOOLTIP, (WPARAM)LPGENW("View webpage"), BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDC_RESET2, BUTTONADDTOOLTIP, (WPARAM)LPGENW("Reset to default"), BATF_UNICODE);

		// save the handle for the contact
		WindowList_Add(hWindowList, hwndDlg, hContact);

		// start to get the settings
		// if the setting not exist, leave the dialog box blank
		if (!g_plugin.getWString(hContact, "ID", &dbv)) {
			SetDlgItemText(hwndDlg, IDC_ID, dbv.pwszVal);
			// check if the station is a default station
			CheckDlgButton(hwndDlg, IDC_DEFA, mir_wstrcmp(dbv.pwszVal, opt.Default) != 0 ? BST_CHECKED : BST_UNCHECKED);
			db_free(&dbv);
		}
		if (!g_plugin.getWString(hContact, "Nick", &dbv)) {
			SetDlgItemText(hwndDlg, IDC_NAME, dbv.pwszVal);
			db_free(&dbv);
		}
		if (!g_plugin.getWString(hContact, "Log", &dbv)) {
			SetDlgItemText(hwndDlg, IDC_LOG, dbv.pwszVal);
			// if the log path is not empty, check the checkbox for external log
			if (dbv.pwszVal[0]) CheckDlgButton(hwndDlg, IDC_External, BST_CHECKED);
			db_free(&dbv);
		}
		// enable/disable the browse button depending on the value of external log checkbox
		EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_External));

		// other checkbox options
		CheckDlgButton(hwndDlg, IDC_DPop, g_plugin.getByte(hContact, "DPopUp", FALSE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DAutoUpdate, g_plugin.getByte(hContact, "DAutoUpdate", FALSE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_Internal, g_plugin.getByte(hContact, "History", 0) ? BST_CHECKED : BST_UNCHECKED);

		if (!g_plugin.getWString(hContact, "InfoURL", &dbv)) {
			SetDlgItemText(hwndDlg, IDC_IURL, dbv.pwszVal);
			db_free(&dbv);
		}
		if (!g_plugin.getWString(hContact, "MapURL", &dbv)) {
			SetDlgItemText(hwndDlg, IDC_MURL, dbv.pwszVal);
			db_free(&dbv);
		}

		// display the dialog box and free memory
		Utils_RestoreWindowPositionNoMove(hwndDlg, NULL, MODULENAME, "EditSetting_");
		ShowWindow(hwndDlg, SW_SHOW);
		break;

	case WM_COMMAND:
		wndData = (CntSetWndDataType*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		hContact = wndData ? wndData->hContact : NULL;

		switch (LOWORD(wParam)) {
		case IDC_ID:
			// check if there are 2 parts in the ID (svc/id) seperated by "/"
			// if not, don't let user change the setting
			GetDlgItemText(hwndDlg, IDC_ID, str, _countof(str));
			chop = wcschr(str, '/');
			if (chop == nullptr)
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHANGE), FALSE);
			else
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHANGE), TRUE);
			break;

		case IDC_NAME:
			// check if station name is entered
			// if not, don't let user change the setting
			GetDlgItemText(hwndDlg, IDC_NAME, str, _countof(str));
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHANGE), str[0] != 0);
			break;

		case IDC_GETNAME:
			// the button for getting station name from the internet
			// this function uses the ID search for add/find weather station
			if (!CheckSearch())
				return TRUE;	// don't download if update is in progress

				// get the weather update data using the string in the ID field
			GetDlgItemText(hwndDlg, IDC_ID, str, _countof(str));
			GetSvc(str);
			sData = GetWIData(str);
			GetDlgItemText(hwndDlg, IDC_ID, str, _countof(str));
			GetID(str);
			// if ID search is available, do it
			if (sData->IDSearch.Available) {
				// load the page
				mir_snprintf(loc, sData->IDSearch.SearchURL, str);
				str[0] = 0;
				wchar_t *pData = nullptr;
				if (InternetDownloadFile(loc, nullptr, sData->UserAgent, &pData) == 0) {
					wchar_t *szInfo = pData;
					wchar_t *search = wcsstr(szInfo, sData->IDSearch.NotFoundStr);

					// if the page is found (ie. valid ID), get the name of the city
					if (search == nullptr)
						GetDataValue(&sData->IDSearch.Name, str, &szInfo);
				}
				// free memory
				mir_free(pData);
			}

			// give no station name but only ID if the search is unavailable
			if (str[0] != 0)
				SetDlgItemText(hwndDlg, IDC_NAME, str);
			break;

		case IDC_External:
			// enable/disable the borwse button depending if the external log is enabled
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_External));
			if (!(uint8_t)IsDlgButtonChecked(hwndDlg, IDC_External))
				return TRUE;
			__fallthrough;

		case IDC_BROWSE:
			// browse for the external log file
			GetDlgItemText(hwndDlg, IDC_LOG, str, _countof(str));
			// Initialize OPENFILENAME
			memset(&ofn, 0, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hwndDlg;
			ofn.lpstrFile = str;
			ofn.nMaxFile = _countof(str);

			// set filters
			mir_snwprintf(filter, L"%s (*.txt)%c*.txt%c%s (*.*)%c*.*%c%c", TranslateT("Text Files"), 0, 0, TranslateT("All Files"), 0, 0, 0);
			ofn.lpstrFilter = filter;
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = nullptr;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = nullptr;
			ofn.Flags = OFN_PATHMUSTEXIST;

			// Display a Open dialog box and put the file name on the dialog
			if (GetOpenFileName(&ofn))
				SetDlgItemText(hwndDlg, IDC_LOG, ofn.lpstrFile);
			// if there is no log file specified, disable external logging
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHANGE), ofn.lpstrFile[0] != 0);
			break;

		case IDC_VIEW1:
			// view the page for more info
			GetDlgItemText(hwndDlg, IDC_IURL, str, _countof(str));
			if (str[0] == 0)
				return TRUE;
			GetDlgItemText(hwndDlg, IDC_ID, str2, _countof(str2));
			OpenUrl(str, str2);
			break;

		case IDC_VIEW2:
			// view the page for weather map
			GetDlgItemText(hwndDlg, IDC_MURL, str, _countof(str));
			if (str[0] == 0)
				return TRUE;
			GetDlgItemText(hwndDlg, IDC_ID, str2, _countof(str2));
			OpenUrl(str, str2);
			break;

		case IDC_RESET1:
			// reset the more info url to service default
			GetDlgItemText(hwndDlg, IDC_ID, str, _countof(str));
			GetSvc(str);
			sData = GetWIData(str);
			SetDlgItemTextA(hwndDlg, IDC_IURL, sData->DefaultURL);
			break;

		case IDC_RESET2:
			// reset the weathe map url to service default
			GetDlgItemText(hwndDlg, IDC_ID, str, _countof(str));
			GetSvc(str);
			sData = GetWIData(str);
			SetDlgItemText(hwndDlg, IDC_MURL, sData->DefaultMap);
			break;

		case IDC_SVCINFO:
			// display the information of the ini file used by the weather station
			GetDlgItemText(hwndDlg, IDC_ID, str, _countof(str));
			GetSvc(str);
			GetINIInfo(str);
			break;

		case IDC_CHANGE:
			// temporary disable the protocol while applying the change
			// start writing the new settings to database
			GetDlgItemText(hwndDlg, IDC_ID, str, _countof(str));
			g_plugin.setWString(hContact, "ID", str);
			if ((uint8_t)IsDlgButtonChecked(hwndDlg, IDC_DEFA)) {	// if default station is set
				mir_wstrcpy(opt.Default, str);
				opt.DefStn = hContact;
				g_plugin.setWString("Default", opt.Default);
			}
			GetDlgItemText(hwndDlg, IDC_NAME, city, _countof(city));
			g_plugin.setWString(hContact, "Nick", city);
			mir_snwprintf(str2, TranslateT("Current weather information for %s."), city);
			if ((uint8_t)IsDlgButtonChecked(hwndDlg, IDC_External)) {
				GetDlgItemText(hwndDlg, IDC_LOG, str, _countof(str));
				g_plugin.setWString(hContact, "Log", str);
			}
			else g_plugin.delSetting(hContact, "Log");

			GetDlgItemText(hwndDlg, IDC_IURL, str, _countof(str));
			g_plugin.setWString(hContact, "InfoURL", str);

			GetDlgItemText(hwndDlg, IDC_MURL, str, _countof(str));
			g_plugin.setWString(hContact, "MapURL", str);
			g_plugin.setWord(hContact, "Status", ID_STATUS_OFFLINE);
			g_plugin.setWord(hContact, "StatusIcon", -1);
			AvatarDownloaded(hContact);
			g_plugin.setWString(hContact, "About", str2);
			g_plugin.setByte(hContact, "History", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_Internal));
			g_plugin.setByte(hContact, "Overwrite", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_Overwrite));
			g_plugin.setByte(hContact, "File", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_External));
			g_plugin.setByte(hContact, "DPopUp", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_DPop));
			g_plugin.setByte(hContact, "DAutoUpdate", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_DAutoUpdate));

			// re-enable the protocol and update the data for the station
			g_plugin.setString(hContact, "LastCondition", "None");
			UpdateSingleStation(hContact, 0);
			__fallthrough;

		case IDCANCEL:
			// remove the dialog from window list and close it
			DestroyWindow(hwndDlg);
			break;
		}
		break;

	case WM_CLOSE:
		// remove the dialog from window list and close it
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		wndData = (CntSetWndDataType*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		IcoLib_ReleaseIcon(wndData->hFile);
		IcoLib_ReleaseIcon(wndData->hRename);
		IcoLib_ReleaseIcon(wndData->hSrchAll);
		IcoLib_ReleaseIcon(wndData->hUserDetail);
		mir_free(wndData);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);

		WindowList_Remove(hWindowList, hwndDlg);
		Utils_SaveWindowPosition(hwndDlg, NULL, MODULENAME, "EditSetting_");
		break;
	}
	return FALSE;
}

// show edit settings dialog
// wParam = current contact
INT_PTR EditSettings(WPARAM wParam, LPARAM)
{
	HWND hEditDlg = WindowList_Find(hWindowList, wParam);

	// search the dialog list to prevent multiple instance of dialog for the same contact
	if (hEditDlg != nullptr) {
		// if the dialog box already opened, bring it to the front
		SetForegroundWindow(hEditDlg);
		SetFocus(hEditDlg);
	}
	else {
		// if the dialog box is not opened, open a new one
		if (IsMyContact(wParam))
			CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_EDIT), nullptr, DlgProcChange, (LPARAM)wParam);
	}

	return 0;
}

//============ CONTACT DELETION  ============
//
// when a contact is deleted, make sure some other contact take over the default station
// wParam = deleted contact
int ContactDeleted(WPARAM wParam, LPARAM)
{
	if (!IsMyContact(wParam))
		return 0;

	removeWindow(wParam);

	// exit this function if it is not default station
	ptrW tszID(g_plugin.getWStringA(wParam, "ID"));
	if (tszID != NULL)
		if (mir_wstrcmp(tszID, opt.Default))
			return 0;

	// now the default station is deleted, try to get a new one

	// start looking for other weather stations
	for (auto &hContact : Contacts(MODULENAME)) {
		tszID = g_plugin.getWStringA(hContact, "ID");
		if (tszID == NULL)
			continue;

		// if the station is not a default station, set it as the new default station
		// this is the first weather station encountered from the search
		if (mir_wstrcmp(opt.Default, tszID)) {
			wcsncpy_s(opt.Default, tszID, _TRUNCATE);
			opt.DefStn = hContact;
			ptrW tszNick(g_plugin.getWStringA(hContact, "Nick"));
			if (tszNick != NULL) {
				wchar_t str[255];
				mir_snwprintf(str, TranslateT("%s is now the default weather station"), (wchar_t*)tszNick);
				MessageBox(nullptr, str, TranslateT("Weather Protocol"), MB_OK | MB_ICONINFORMATION);
			}
			g_plugin.setWString("Default", opt.Default);
			return 0;		// exit this function quickly
		}
	}

	// got here if no more weather station left
	opt.Default[0] = 0;	// no default station
	opt.DefStn = NULL;
	g_plugin.setWString("Default", opt.Default);
	return 0;
}

BOOL IsMyContact(MCONTACT hContact)
{
	const char *szProto = Proto_GetBaseAccountName(hContact);
	return szProto != nullptr && mir_strcmp(MODULENAME, szProto) == 0;
}
