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

/* This file contain the source that is related to weather contacts,
include the links, edit settings, and loading weather information for
the contact.
*/

#include "weather.h"

//============  BASIC CONTACTS FUNCTIONS AND LINKS  ============

// view weather log for the contact
// wParam = current contact
INT_PTR ViewLog(WPARAM wParam,LPARAM lParam) 
{
	DBVARIANT dbv;
	// see if the log path is set
	if (!DBGetContactSettingTString((HANDLE)wParam,WEATHERPROTONAME,"Log",&dbv))
	{
		if (dbv.pszVal[0] != 0)
			ShellExecute((HWND)lParam, _T("open"), dbv.ptszVal, _T(""), _T(""), SW_SHOW);
		DBFreeVariant(&dbv);
	}
	else	// display warning dialog if no path is specified
		MessageBox(NULL, TranslateT("Weather condition was not logged."), 
		TranslateT("Weather Protocol"), MB_OK | MB_ICONINFORMATION);
	return 0;
}

// read complete forecast
// wParam = current contact
INT_PTR LoadForecast(WPARAM wParam,LPARAM lParam) 
{
	char id[256], loc[256], loc2[256];

	// get station ID
	GetStationID((HANDLE)wParam, id, sizeof(id));

	if (id[0] != 0)
	{
		GetID(id);

		// check if the complte forecast URL is set. If it is not, display warning and quit
		if (DBGetStaticString((HANDLE)wParam, WEATHERPROTONAME, "InfoURL", loc2, sizeof(loc2)) || loc2[0] == 0)
		{
			MessageBox(NULL, TranslateT("The URL for complete forcast have not been set. You can set it from the Edit Settings dialog."), 
				TranslateT("Weather Protocol"), MB_ICONINFORMATION);
			return 1;
		}
		// set the url and open the webpage
		mir_snprintf(loc, sizeof(loc), loc2, id);
		CallService(MS_UTILS_OPENURL, opt.NewBrowserWin, (WPARAM)loc);
	}
	return 0;
}

// load weather map
// wParam = current contact
INT_PTR WeatherMap(WPARAM wParam,LPARAM lParam) 
{
	char id[256], loc[256], loc2[256];

	GetStationID((HANDLE)wParam, id, sizeof(id));

	if (id[0] != 0)
	{
		GetID(id);

		// check if the weather map URL is set. If it is not, display warning and quit
		if (DBGetStaticString((HANDLE)wParam, WEATHERPROTONAME, "MapURL", loc2, sizeof(loc2)) || loc2[0] == 0)
		{
			MessageBox(NULL, TranslateT("The URL for weather map have not been set.  You can set it from the Edit Settings dialog."), TranslateT("Weather Protocol"), MB_ICONINFORMATION);
			return 1;
		}
		// set the url and open the webpage
		mir_snprintf(loc, sizeof(loc), loc2, id);
		CallService(MS_UTILS_OPENURL, opt.NewBrowserWin, (WPARAM)loc);
	}

	return 0;
}

//============  EDIT SETTINGS  ============

// show edit settings dialog
// wParam = current contact
INT_PTR EditSettings(WPARAM wParam,LPARAM lParam) 
{
	HWND hEditDlg = WindowList_Find(hWindowList, (HANDLE)wParam);

	// search the dialog list to prevent multiple instance of dialog for the same contact
	if (hEditDlg != NULL) 
	{
		// if the dialog box already opened, bring it to the front
		SetForegroundWindow(hEditDlg);
		SetFocus(hEditDlg);
	}
	else
	{
		if(IsMyContact((HANDLE)wParam)) 
		{
			// if the dialog box is not opened, open a new one
			CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_EDIT), NULL, DlgProcChange, (LPARAM)wParam);
		}
	}

	return 0;
}

typedef struct
{
	HANDLE hContact;
	HICON  hRename;
	HICON  hUserDetail;
	HICON  hFile;
	HICON  hSrchAll;
} CntSetWndDataType;

// edit weather settings
// lParam = current contact
INT_PTR CALLBACK DlgProcChange(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	DBVARIANT dbv;
	char str[256], str2[256], city[256], filter[256], *pfilter, loc[512], *chop;
	OPENFILENAME ofn;       // common dialog box structure
	HANDLE hContact;
	WIDATA *sData;
	CntSetWndDataType *wndData = NULL;

	switch (msg) 
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		wndData = mir_alloc(sizeof(CntSetWndDataType));
		wndData->hContact = hContact = (HANDLE)lParam;
		wndData->hRename = LoadSkinnedIcon(SKINICON_OTHER_RENAME);
		wndData->hUserDetail = LoadSkinnedIcon(SKINICON_OTHER_USERDETAILS);
		wndData->hFile = LoadSkinnedIcon(SKINICON_EVENT_FILE);
		wndData->hSrchAll = LoadSkinnedIcon(SKINICON_OTHER_SEARCHALL);

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)wndData);

		// set button images
		SendDlgItemMessage(hwndDlg, IDC_GETNAME, BM_SETIMAGE, IMAGE_ICON, (LPARAM)wndData->hRename);
		SendDlgItemMessage(hwndDlg, IDC_SVCINFO, BM_SETIMAGE, IMAGE_ICON, (LPARAM)wndData->hUserDetail);
		SendDlgItemMessage(hwndDlg, IDC_BROWSE,  BM_SETIMAGE, IMAGE_ICON, (LPARAM)wndData->hFile);
		SendDlgItemMessage(hwndDlg, IDC_VIEW1,   BM_SETIMAGE, IMAGE_ICON, (LPARAM)wndData->hSrchAll);
		SendDlgItemMessage(hwndDlg, IDC_RESET1,  BM_SETIMAGE, IMAGE_ICON, (LPARAM)wndData->hRename);
		SendDlgItemMessage(hwndDlg, IDC_VIEW2,   BM_SETIMAGE, IMAGE_ICON, (LPARAM)wndData->hSrchAll);
		SendDlgItemMessage(hwndDlg, IDC_RESET2,  BM_SETIMAGE, IMAGE_ICON, (LPARAM)wndData->hRename);

		// make all buttons flat
		SendDlgItemMessage(hwndDlg,IDC_GETNAME, BUTTONSETASFLATBTN, 0, 0);
		SendDlgItemMessage(hwndDlg,IDC_SVCINFO, BUTTONSETASFLATBTN, 0, 0);
		SendDlgItemMessage(hwndDlg,IDC_BROWSE, BUTTONSETASFLATBTN, 0, 0);
		SendDlgItemMessage(hwndDlg,IDC_VIEW1, BUTTONSETASFLATBTN, 0, 0);
		SendDlgItemMessage(hwndDlg,IDC_RESET1, BUTTONSETASFLATBTN, 0, 0);
		SendDlgItemMessage(hwndDlg,IDC_VIEW2, BUTTONSETASFLATBTN, 0, 0);
		SendDlgItemMessage(hwndDlg,IDC_RESET2, BUTTONSETASFLATBTN, 0, 0);

		// set tooltip for the buttons
		SendDlgItemMessage(hwndDlg,IDC_GETNAME, BUTTONADDTOOLTIP, (WPARAM)"Get city name from ID", 0);
		SendDlgItemMessage(hwndDlg,IDC_SVCINFO, BUTTONADDTOOLTIP, (WPARAM)"Weather INI information", 0);
		SendDlgItemMessage(hwndDlg,IDC_BROWSE, BUTTONADDTOOLTIP, (WPARAM)"Browse", 0);
		SendDlgItemMessage(hwndDlg,IDC_VIEW1, BUTTONADDTOOLTIP, (WPARAM)"View webpage", 0);
		SendDlgItemMessage(hwndDlg,IDC_RESET1, BUTTONADDTOOLTIP, (WPARAM)"Reset to default", 0);
		SendDlgItemMessage(hwndDlg,IDC_VIEW2, BUTTONADDTOOLTIP, (WPARAM)"View webpage", 0);
		SendDlgItemMessage(hwndDlg, IDC_RESET2, BUTTONADDTOOLTIP, (WPARAM)"Reset to default", 0);

		// save the handle for the contact
		WindowList_Add(hWindowList, hwndDlg, hContact);

		// start to get the settings
		// if the setting not exist, leave the dialog box blank
		if(!DBGetContactSettingString(hContact,WEATHERPROTONAME,"ID",&dbv)) 
		{
			SetDlgItemTextA(hwndDlg,IDC_ID,dbv.pszVal);
			// check if the station is a default station
			if (!strcmp(dbv.pszVal, opt.Default))
				CheckDlgButton(hwndDlg, IDC_DEFA, TRUE);
			else	CheckDlgButton(hwndDlg, IDC_DEFA, FALSE);
			DBFreeVariant(&dbv);
		}
		if(!DBGetContactSettingTString(hContact,WEATHERPROTONAME,"Nick",&dbv)) 
		{
			SetDlgItemText(hwndDlg,IDC_NAME,dbv.ptszVal);
			DBFreeVariant(&dbv);
		}
		if(!DBGetContactSettingTString(hContact,WEATHERPROTONAME,"Log",&dbv)) 
		{
			SetDlgItemText(hwndDlg, IDC_LOG, dbv.ptszVal);
			// if the log path is not empty, check the checkbox for external log
			if (dbv.ptszVal[0]) CheckDlgButton(hwndDlg, IDC_External, TRUE);
			DBFreeVariant(&dbv);
		}
		// enable/disable the browse button depending on the value of external log checkbox
		EnableWindow(GetDlgItem(hwndDlg,IDC_BROWSE),(BYTE)IsDlgButtonChecked(hwndDlg,IDC_External));

		// other checkbox options
		CheckDlgButton(hwndDlg, IDC_DPop, DBGetContactSettingByte(hContact,WEATHERPROTONAME,"DPopUp",FALSE));
		CheckDlgButton(hwndDlg, IDC_DAutoUpdate, DBGetContactSettingByte(hContact,WEATHERPROTONAME,
			"DAutoUpdate",FALSE));
		CheckDlgButton(hwndDlg, IDC_Internal, DBGetContactSettingByte(hContact,WEATHERPROTONAME,"History",0));
		if (!DBGetContactSettingString(hContact,WEATHERPROTONAME,"InfoURL",&dbv))
		{
			SetDlgItemText(hwndDlg,IDC_IURL,dbv.pszVal);
			DBFreeVariant(&dbv);
		}
		if (!DBGetContactSettingString(hContact,WEATHERPROTONAME,"MapURL",&dbv))
		{
			SetDlgItemText(hwndDlg,IDC_MURL,dbv.pszVal);
			DBFreeVariant(&dbv);
		}

		// display the dialog box and free memory
		Utils_RestoreWindowPositionNoMove(hwndDlg,NULL,WEATHERPROTONAME,"EditSetting_");
		ShowWindow(hwndDlg,SW_SHOW);
		break;

	case WM_COMMAND:
		wndData = (CntSetWndDataType*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		hContact = wndData ? wndData->hContact : NULL;

		switch(LOWORD(wParam)) 
		{
		case IDC_ID:
			// check if there are 2 parts in the ID (svc/id) seperated by "/"
			// if not, don't let user change the setting
			GetDlgItemText(hwndDlg,IDC_ID,str,sizeof(str));
			chop = strstr(str, "/");
			if (chop == NULL)
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHANGE),FALSE);
			else
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHANGE),TRUE);
			break;
		case IDC_NAME:
			// check if station name is entered
			// if not, don't let user change the setting
			GetDlgItemText(hwndDlg,IDC_NAME,str,sizeof(str));
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHANGE),str[0] != 0);
			break;
		case IDC_GETNAME: 
			{
				// the button for getting station name from the internet
				// this function uses the ID search for add/find weather station
				WIDATA *sData; 

				if (!CheckSearch()) return TRUE;	// don't download if update is in progress
				// get the weather update data using the string in the ID field
				GetDlgItemText(hwndDlg,IDC_ID,str,sizeof(str));
				GetSvc(str);
				sData = GetWIData(str);
				GetDlgItemText(hwndDlg,IDC_ID,str,sizeof(str));
				GetID(str);
				// if ID search is available, do it
				if (sData->IDSearch.Available) 
				{
					char *szData = NULL;

					// load the page
					wsprintf(loc, sData->IDSearch.SearchURL, str);
					str[0] = 0;
					if (InternetDownloadFile(loc, NULL, &szData) == 0) 
					{
						char *szInfo = szData;
						char* search = strstr(szInfo, sData->IDSearch.NotFoundStr);

						// if the page is found (ie. valid ID), get the name of the city
						if (search == NULL)
							GetDataValue(&sData->IDSearch.Name, str, &szInfo);

						// free memory
						mir_free(szData);
					}
					NetlibHttpDisconnect();
				}
				// give no station name but only ID if the search is unavailable
				if (str[0] != 0)	SetDlgItemText(hwndDlg,IDC_NAME,str);
				break;
			}
		case IDC_External:
			// enable/disable the borwse button depending if the external log is enabled
			EnableWindow(GetDlgItem(hwndDlg,IDC_BROWSE),(BYTE)IsDlgButtonChecked(hwndDlg,IDC_External));
			if (!(BYTE)IsDlgButtonChecked(hwndDlg,IDC_External)) return TRUE;

		case IDC_BROWSE:	// fall through
			// browse for the external log file
			GetDlgItemText(hwndDlg,IDC_LOG,str,sizeof(str));
			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hwndDlg;
			ofn.lpstrFile = str;
			ofn.nMaxFile = sizeof(str);
			// set filters
			strcpy(filter,Translate("Text Files"));
			strcat(filter," (*.txt)");
			pfilter=filter+strlen(filter)+1;
			strcpy(pfilter,"*.txt");
			pfilter=pfilter+strlen(pfilter)+1;
			strcpy(pfilter,Translate("All Files"));
			strcat(pfilter," (*.*)");
			pfilter=pfilter+strlen(pfilter)+1;
			strcpy(pfilter,"*.*");
			pfilter=pfilter+strlen(pfilter)+1;
			*pfilter='\0';
			ofn.lpstrFilter = filter;
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST;

			// Display a Open dialog box and put the file name on the dialog
			if(GetOpenFileName(&ofn))
				SetDlgItemText(hwndDlg,IDC_LOG,ofn.lpstrFile);
			// if there is no log file specified, disable external logging
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHANGE),ofn.lpstrFile[0] != 0);
			break;

		case IDC_VIEW1:
			// view the page for more info
			GetDlgItemText(hwndDlg,IDC_IURL,str,sizeof(str));
			if (str[0] == 0) return TRUE;
			GetDlgItemText(hwndDlg,IDC_ID,str2,sizeof(str2));
			GetID(str2);
			wsprintf(loc, str, str2);
			CallService(MS_UTILS_OPENURL, opt.NewBrowserWin, (WPARAM)loc);
			break;

		case IDC_VIEW2:
			// view the page for weather map
			GetDlgItemText(hwndDlg,IDC_MURL,str,sizeof(str));
			if (str[0] == 0) return TRUE;
			GetDlgItemText(hwndDlg,IDC_ID,str2,sizeof(str2));
			GetID(str2);
			wsprintf(loc, str, str2);
			CallService(MS_UTILS_OPENURL, opt.NewBrowserWin, (WPARAM)loc);
			break;

		case IDC_RESET1:
			// reset the more info url to service default
			GetDlgItemText(hwndDlg,IDC_ID,str,sizeof(str));
			GetSvc(str);
			sData = GetWIData(str);
			SetDlgItemText(hwndDlg,IDC_IURL,sData->DefaultURL);
			break;

		case IDC_RESET2:
			// reset the weathe map url to service default
			GetDlgItemText(hwndDlg,IDC_ID,str,sizeof(str));
			GetSvc(str);
			sData = GetWIData(str);
			SetDlgItemText(hwndDlg,IDC_MURL,sData->DefaultMap);
			break;

		case IDC_SVCINFO:
			// display the information of the ini file used by the weather station
			GetDlgItemText(hwndDlg,IDC_ID,str,sizeof(str));
			GetSvc(str);
			GetINIInfo(str);
			break;

		case IDC_CHANGE:
			// temporary disable the protocol while applying the change
			// start writing the new settings to database
			GetDlgItemText(hwndDlg,IDC_ID,str,sizeof(str));
			DBWriteContactSettingString(hContact, WEATHERPROTONAME, "ID", str);
			if ((BYTE)IsDlgButtonChecked(hwndDlg,IDC_DEFA)) {	// if default station is set
				strcpy(opt.Default, str);
				opt.DefStn = hContact;
				DBWriteContactSettingString(NULL, WEATHERPROTONAME, "Default", opt.Default);
			}
			GetDlgItemText(hwndDlg,IDC_NAME,city,sizeof(city));
			DBWriteContactSettingString(hContact, WEATHERPROTONAME, "Nick", city);
			wsprintf(str2, Translate("Current weather information for %s."), city);
			if ((BYTE)IsDlgButtonChecked(hwndDlg,IDC_External))
			{
				GetDlgItemText(hwndDlg,IDC_LOG,str,sizeof(str));
				DBWriteContactSettingString(hContact, WEATHERPROTONAME, "Log", str);
			}
			else	
				DBDeleteContactSetting(hContact, WEATHERPROTONAME, "Log");
			GetDlgItemTextA(hwndDlg,IDC_IURL,str,sizeof(str));
			DBWriteContactSettingString(hContact, WEATHERPROTONAME, "InfoURL", str);
			GetDlgItemTextA(hwndDlg,IDC_MURL,str,sizeof(str));
			DBWriteContactSettingString(hContact, WEATHERPROTONAME, "MapURL", str);
			DBWriteContactSettingWord(hContact, WEATHERPROTONAME, "Status", ID_STATUS_OFFLINE);
			DBWriteContactSettingWord(hContact, WEATHERPROTONAME, "StatusIcon", ID_STATUS_OFFLINE);
			AvatarDownloaded(hContact);
			DBWriteContactSettingString(hContact, WEATHERPROTONAME, "About", str2);
			DBWriteContactSettingByte(hContact,WEATHERPROTONAME,"History",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_Internal));
			DBWriteContactSettingByte(hContact,WEATHERPROTONAME,"Overwrite",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_Overwrite));
			DBWriteContactSettingByte(hContact,WEATHERPROTONAME,"File",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_External));
			DBWriteContactSettingByte(hContact,WEATHERPROTONAME,"DPopUp",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_DPop));
			DBWriteContactSettingByte(hContact,WEATHERPROTONAME,"DAutoUpdate",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_DAutoUpdate));

			// re-enable the protocol and update the data for the station
			DBWriteContactSettingString(hContact, WEATHERPROTONAME, "LastCondition", "None");
			UpdateSingleStation((WPARAM)hContact, 0);

		case IDCANCEL:		// fall through
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
		CallService(MS_SKIN2_RELEASEICON, (WPARAM)wndData->hFile, 0);
		CallService(MS_SKIN2_RELEASEICON, (WPARAM)wndData->hRename, 0);
		CallService(MS_SKIN2_RELEASEICON, (WPARAM)wndData->hSrchAll, 0);
		CallService(MS_SKIN2_RELEASEICON, (WPARAM)wndData->hUserDetail, 0);
		mir_free(wndData);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);

		WindowList_Remove(hWindowList, hwndDlg);
		Utils_SaveWindowPosition(hwndDlg,NULL,WEATHERPROTONAME,"EditSetting_");
		break;
	}
	return FALSE;
}

//============  CONTACT DELETION  ============

// when a contact is deleted, make sure some other contact take over the default station
// wParam = deleted contact
int ContactDeleted(WPARAM wParam,LPARAM lParam) 
{
	DBVARIANT dbv;

	if(!IsMyContact((HANDLE)wParam)) return 0; 

	removeWindow((HANDLE)wParam);

	// exit this function if it is not default station
	if (!DBGetContactSettingString((HANDLE)wParam, WEATHERPROTONAME, "ID", &dbv))
	{
		if (strcmp(dbv.pszVal, opt.Default)) 
		{
			DBFreeVariant(&dbv);
			return 0;
		}
		DBFreeVariant(&dbv);
	}

	// now the default station is deleted, try to get a new one
	{
		// start looking for other weather stations
		HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		while(hContact) 
		{
			if(IsMyContact(hContact)) 
			{
				if (!DBGetContactSettingString(hContact, WEATHERPROTONAME, "ID", &dbv))
				{
					// if the station is not a default station, set it as the new default station
					// this is the first weather station encountered from the search
					if (strcmp(opt.Default, dbv.pszVal)) 
					{
						strcpy(opt.Default, dbv.pszVal);
						opt.DefStn = hContact;
						DBFreeVariant(&dbv);
						if (!DBGetContactSettingTString(hContact,WEATHERPROTONAME,"Nick",&dbv))
						{
							TCHAR str[255];
							mir_sntprintf(str, SIZEOF(str), TranslateT("%s is now the default weather station"), dbv.ptszVal);
							DBFreeVariant(&dbv);
							MessageBox(NULL, str, TranslateT("Weather Protocol"), MB_OK | MB_ICONINFORMATION);
						}
						DBWriteContactSettingString(NULL, WEATHERPROTONAME, "Default", opt.Default);
						return 0;		// exit this function quickly
					}
					DBFreeVariant(&dbv);
				}
			}
			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
		}
		// got here if no more weather station left
		opt.Default[0] = 0;	// no default station
		opt.DefStn = NULL;
		DBWriteContactSettingString(NULL, WEATHERPROTONAME, "Default", opt.Default);
	}
	return 0;
}

BOOL IsMyContact(HANDLE hContact)
{
	const char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	return szProto != NULL && strcmp(WEATHERPROTONAME, szProto) == 0;
}
