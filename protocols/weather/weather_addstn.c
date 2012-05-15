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

/* This file contain the source related to search and add a weather station
to the contact list.  Contain code for both name and ID search.
*/

#include "weather.h"

// variables used for weather_addstn.c
static int searchId=-1;
static char sID[32];
static char name1[256];

//============  ADDING NEW STATION  ============

// protocol service function for adding a new contact onto contact list
// lParam = PROTOSEARCHRESULT
INT_PTR WeatherAddToList(WPARAM wParam,LPARAM lParam) 
{
	PROTOSEARCHRESULT *psr=(PROTOSEARCHRESULT*)lParam;
	char str[256], svc[256];
	WIDATA *sData;

	// search for existing contact
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	while(hContact != NULL) 
	{
		// check if it is a weather contact
		if(IsMyContact(hContact)) 
		{
			DBVARIANT dbv;
			// check ID to see if the contact already exist in the database
			if(!DBGetContactSettingString(hContact,WEATHERPROTONAME,"ID",&dbv)) 
			{
				if(!_stricmp(psr->email,dbv.pszVal)) 
				{
					// remove the flag for not on list and hidden, thus make the contact visible
					// and add them on the list
					if (DBGetContactSettingByte(hContact,"CList","NotOnList",1)) 
					{
						DBDeleteContactSetting(hContact,"CList","NotOnList");
						DBDeleteContactSetting(hContact,"CList","Hidden");						
					}
					DBFreeVariant(&dbv);
					// contact is added, function quitting
					return (INT_PTR)hContact;
				}
				DBFreeVariant(&dbv);
			}
		}
		hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0);
	}

	// if contact with the same ID was not found, add it
	if (psr->cbSize < sizeof(PROTOSEARCHRESULT)) return 0;
	hContact = (HANDLE) CallService(MS_DB_CONTACT_ADD, 0, 0);
	CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)WEATHERPROTONAME);
	// suppress online notification for the new contact
	CallService(MS_IGNORE_IGNORE,(WPARAM)hContact, IGNOREEVENT_USERONLINE);
	// set contact info and settings
	strncpy(svc, psr->email, sizeof(svc)); svc[sizeof(svc)-1] = 0;
	GetSvc(svc);
	// set settings by obtaining the default for the service 
	if (psr->lastName[0] != 0) 
	{
		sData = GetWIData(svc);
		DBWriteContactSettingString(hContact, WEATHERPROTONAME, "MapURL", sData->DefaultMap);
		DBWriteContactSettingString(hContact, WEATHERPROTONAME, "InfoURL", sData->DefaultURL);
	}
	else // if no valid service is found, create empty strings for MapURL and InfoURL
	{
		DBWriteContactSettingString(hContact, WEATHERPROTONAME, "MapURL", "");
		DBWriteContactSettingString(hContact, WEATHERPROTONAME, "InfoURL", "");
	}
	// write the other info and settings to the database
	DBWriteContactSettingString(hContact, WEATHERPROTONAME, "ID", psr->email);
	DBWriteContactSettingString(hContact, WEATHERPROTONAME, "Nick", psr->nick);
	wsprintf(str, Translate("Current weather information for %s."), psr->nick);
	DBWriteContactSettingWord(hContact, WEATHERPROTONAME, "Status", ID_STATUS_OFFLINE);
	AvatarDownloaded(hContact);
	DBWriteContactSettingString(hContact, WEATHERPROTONAME, "About", str);
	// make the last update tags to something invalid
	DBWriteContactSettingString(hContact, WEATHERPROTONAME, "LastLog", "never");
	DBWriteContactSettingString(hContact, WEATHERPROTONAME, "LastCondition", "None");
	DBWriteContactSettingString(hContact, WEATHERPROTONAME, "LastTemperature", "None");

	// ignore status change
	DBWriteContactSettingDword(hContact, "Ignore", "Mask", 8);

	// if no default station is found, set the new contact as default station
	if (opt.Default[0] == 0) 
	{
		DBVARIANT dbv;
		GetStationID(hContact, opt.Default, sizeof(opt.Default));

		opt.DefStn = hContact;
		if (!DBGetContactSettingString(hContact,WEATHERPROTONAME,"Nick",&dbv))
		{
			// notification message box
			wsprintf(str, Translate("%s is now the default weather station"), dbv.pszVal);
			DBFreeVariant(&dbv);
			MessageBox(NULL, str, Translate("Weather Protocol"), MB_OK|MB_ICONINFORMATION);
		}
		DBWriteContactSettingString(NULL, WEATHERPROTONAME, "Default", opt.Default);
	}
	// display the Edit Settings dialog box
	EditSettings((WPARAM)hContact, 0);
	return (INT_PTR)hContact;
}

//============  WARNING DIALOG  ============

// show a message box and cancel search if update is in process
BOOL CheckSearch() {
	if (UpdateListHead != NULL) {
		MessageBox(NULL, Translate("Please try again after weather update is completed."), Translate("Weather Protocol"), MB_OK|MB_ICONERROR);
		return FALSE;
	}
	return TRUE;
}

//============  BASIC ID SEARCH  ============

// A timer process for the ID search (threaded)
static void __cdecl BasicSearchTimerProc(LPVOID hWnd) 
{
	int result;
	// search only when it's not current updating weather.
	if (CheckSearch())	result = IDSearch(sID, searchId);
	// broadcast the search result
	ProtoBroadcastAck(WEATHERPROTONAME,NULL,ACKTYPE_SEARCH,ACKRESULT_SUCCESS,(HANDLE)searchId,0);

	// exit the search
	searchId=-1;
}

// the service function for ID search
// lParam = ID search string
INT_PTR WeatherBasicSearch(WPARAM wParam,LPARAM lParam) 
{
	if(searchId != -1) return 0;   //only one search at a time
	strncpy(sID, (char*)lParam, sizeof(sID));
	sID[sizeof(sID)-1] = 0;
	searchId=1;
	// create a thread for the ID search
	mir_forkthread(BasicSearchTimerProc, NULL);
	return searchId;
}

//============  NAME SEARCH  ============

// name search timer process (threaded)
static void __cdecl NameSearchTimerProc(LPVOID hWnd) 
{
	// search only when it's not current updating weather.
	if (CheckSearch())
	{
		if (name1[0] != 0)	NameSearch(name1, searchId);	// search nickname field
	}
	// broadcast the result
	ProtoBroadcastAck(WEATHERPROTONAME, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)searchId, 0);

	// exit the search
	searchId = -1;
}

static INT_PTR CALLBACK WeatherSearchAdvancedDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetFocus(GetDlgItem(hwndDlg, IDC_SEARCHCITY));
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == EN_SETFOCUS)
			PostMessage(GetParent(hwndDlg), WM_COMMAND, MAKEWPARAM(0, EN_SETFOCUS), (LPARAM)hwndDlg);
	}
	return FALSE;
}

INT_PTR WeatherCreateAdvancedSearchUI(WPARAM wParam, LPARAM lParam)
{
	HWND parent = (HWND)lParam;

	if (parent)
		return (INT_PTR)CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_SEARCHCITY), parent, WeatherSearchAdvancedDlgProc, 0);

	return 0;
}

// service function for name search
INT_PTR WeatherAdvancedSearch(WPARAM wParam, LPARAM lParam)
{
	if (searchId != -1) return 0;   //only one search at a time
	searchId = 1;

	GetDlgItemText((HWND)lParam, IDC_SEARCHCITY, name1, 256);

	// search for the weather station using a thread
	mir_forkthread(NameSearchTimerProc, NULL);
	return searchId;
}

//============  SEARCH FOR A WEATHER STATION USING ID  ============

// Seaching station ID from a single weather service (Threaded)
// sID = search string for the station ID
// searchId = -1
// sData = the ID search data for that particular weather service
// svcname = the name of the weather service that is currently searching (ie. Yahoo Weather)
int IDSearchProc(char *sID, const int searchId, WIIDSEARCH *sData, char *svc, char *svcname) 
{
	PROTOSEARCHRESULT psr = {0};
	char str[MAX_DATA_LEN] = "", newID[MAX_DATA_LEN];

	if (sData->Available) 
	{
		char loc[255], *szData = NULL;

		// load the page
		mir_snprintf(loc, sizeof(loc), sData->SearchURL , sID);
		if (InternetDownloadFile(loc, NULL, &szData) == 0) 
		{
			char* szInfo = szData;

			// not found
			if (strstr(szInfo, sData->NotFoundStr) == NULL) 
				GetDataValue(&sData->Name, str, &szInfo);
		}
		mir_free(szData);
		// Station not found exit
		if (str[0] == 0) return 1;
	}

	// give no station name but only ID if the search is unavailable
	else	strcpy(str, Translate("<Enter station name here>"));
	mir_snprintf(newID, sizeof(newID), "%s/%s", svc, sID);

	// set the search result and broadcast it
	psr.cbSize=sizeof(psr);
	psr.nick=str;
	psr.firstName=" ";
	psr.lastName=svcname;
	psr.email=newID;
	ProtoBroadcastAck(WEATHERPROTONAME, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)searchId, (LPARAM)&psr);

	return 0;
}

// ID search	 (Threaded)
//  sID:		the ID to search for
//  searchId:	don't change
// return 0 if no error
int IDSearch(char *sID, const int searchId) 
{
	// for a normal ID search (ID != #)
	if (strcmp(sID, "#")) 
	{
		WIDATALIST *Item = WIHead;

		// search every weather service using the search station ID
		while (Item != NULL) 
		{
			IDSearchProc(sID, searchId, &Item->Data.IDSearch, Item->Data.InternalName, Item->Data.DisplayName);
			Item = Item->next;
		}
		NetlibHttpDisconnect();
	}
	// if the station ID is #, return a dummy result and quit the funciton
	else 
	{
		PROTOSEARCHRESULT psr = {0};
		// return an empty contact on "#"
		psr.cbSize=sizeof(psr);
		psr.nick=Translate("<Enter station name here>");	// to be entered
		psr.firstName=" ";
		psr.lastName="";
		psr.email=Translate("<Enter station ID here>");		// to be entered
		ProtoBroadcastAck(WEATHERPROTONAME,NULL,ACKTYPE_SEARCH,ACKRESULT_DATA,(HANDLE)searchId,(LPARAM)&psr);
	}

	return 0;
}

//============  SEARCH FOR A WEATHER STATION BY NAME  ============

// Seaching station name from a single weather service (Threaded)
// name = the name of the weather station to be searched
// searchId = -1
// sData = the name search data for that particular weather service
// svcname = the name of the weather service that is currently searching (ie. Yahoo Weather)
int NameSearchProc(char *name, const int searchId, WINAMESEARCH *sData, char *svc, char *svcname)
{
	PROTOSEARCHRESULT psr;
	char *search, str[MAX_DATA_LEN], loc[256], sID[MAX_DATA_LEN], Name[MAX_DATA_LEN], *szData = NULL;

	// replace spaces with %20
	char *pstr = (char*)CallService(MS_NETLIB_URLENCODE, 0, (LPARAM) name);
	wsprintf(loc, sData->SearchURL, pstr);
	HeapFree(GetProcessHeap(), 0, pstr);

	if (InternetDownloadFile(loc, NULL, &szData) == 0) 
	{
		char* szInfo = szData;
		search = strstr(szInfo, sData->NotFoundStr);	// determine if data is available
		if (search == NULL)	// if data is found
		{
			// test if it is single result
			if (sData->Single.Available && sData->Multiple.Available)
				search = strstr(szInfo, sData->SingleStr);
			// for single result
			if (sData->Single.Available && (search != NULL || !sData->Multiple.Available))		// single result
			{
				// if station ID appears first in the downloaded data
				if (!_stricmp(sData->Single.First, "ID"))
				{
					GetDataValue(&sData->Single.ID, str, &szInfo);
					wsprintf(sID, "%s/%s", svc, str);
					GetDataValue(&sData->Single.Name, Name, &szInfo);
				}
				// if station name appears first in the downloaded data
				else if (!_stricmp(sData->Single.First, "NAME"))
				{
					GetDataValue(&sData->Single.Name, Name, &szInfo);
					GetDataValue(&sData->Single.ID, str, &szInfo);
					wsprintf(sID, "%s/%s", svc, str);
				}
				// if no station ID is obtained, quit the search
				if (str[0] == 0)
				{
					mir_free(szData);
					return 1;
				}
				// if can't get the name, use the search string as name
				if (Name[0] == 0)		strcpy(Name, name);

				// set the data and broadcast it
				memset(&psr,0,sizeof(psr));
				psr.cbSize=sizeof(psr);
				psr.nick=Name;
				psr.firstName=" ";
				psr.lastName=svcname;
				psr.email=sID;
				psr.id=sID;
				ProtoBroadcastAck(WEATHERPROTONAME,NULL,ACKTYPE_SEARCH,ACKRESULT_DATA,(HANDLE)searchId,(LPARAM)&psr);
				mir_free(szData);
				return 0;
			}
			// for multiple result
			else if (sData->Multiple.Available)		// multiple results
			{
				// search for the next occurrence of the string
				for (;;) 
				{
					// if station ID appears first in the downloaded data
					if (!_stricmp(sData->Multiple.First, "ID")) {
						GetDataValue(&sData->Multiple.ID, str, &szInfo);
						wsprintf(sID, "%s/%s", svc, str);
						GetDataValue(&sData->Multiple.Name, Name, &szInfo);
					}
					// if station name appears first in the downloaded data
					else if (!_stricmp(sData->Multiple.First, "NAME")) {
						GetDataValue(&sData->Multiple.Name, Name, &szInfo);
						GetDataValue(&sData->Multiple.ID, str, &szInfo);
						wsprintf(sID, "%s/%s", svc, str);
					}
					// if no station ID is obtained, search completed and quit the search
					if (str[0] == 0)	break;
					// if can't get the name, use the search string as name
					if (Name[0] == 0)	strcpy(Name, name);

					memset(&psr,0,sizeof(psr));
					psr.cbSize=sizeof(psr);
					psr.nick=Name;
					psr.firstName="";
					psr.lastName=svcname;
					psr.email=sID;
					psr.id=sID;
					ProtoBroadcastAck(WEATHERPROTONAME,NULL,ACKTYPE_SEARCH,ACKRESULT_DATA,
						(HANDLE)searchId,(LPARAM)&psr);
				}
			}
		}
		mir_free(szData);
		return 0;
	}
	mir_free(szData);
	return 1;  
}

// name search	(Threaded)
//  name:		the station name to search for
//  searchId:	don't change
// return 0 if no error
int NameSearch(char *name, const int searchId)
{
	WIDATALIST *Item = WIHead;

	// search every weather service using the search station name
	while (Item != NULL) 
	{
		if (Item->Data.NameSearch.Single.Available || Item->Data.NameSearch.Multiple.Available)
			NameSearchProc(name, searchId, &Item->Data.NameSearch, Item->Data.InternalName, Item->Data.DisplayName);
		Item = Item->next;
	}
	NetlibHttpDisconnect();

	return 0;
}

//====================== MENU ITEM FUNCTION ============

// add a new weather station via find/add dialog
int WeatherAdd(WPARAM wParam, LPARAM lParam) 
{
	DBWriteContactSettingString(NULL, "FindAdd", "LastSearched", "Weather");
	CallService(MS_FINDADD_FINDADD, 0, 0);
	return 0;
}
