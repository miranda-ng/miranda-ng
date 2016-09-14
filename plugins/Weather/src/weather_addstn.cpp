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

/* This file contain the source related to search and add a weather station
to the contact list.  Contain code for both name and ID search.
*/

#include "stdafx.h"

// variables used for weather_addstn.c
static int sttSearchId = -1;
static wchar_t name1[256];

// ============ ADDING NEW STATION  ============

// protocol service function for adding a new contact onto contact list
// lParam = PROTOSEARCHRESULT
INT_PTR WeatherAddToList(WPARAM, LPARAM lParam)
{
	PROTOSEARCHRESULT *psr = (PROTOSEARCHRESULT*)lParam;
	if (!psr || !psr->email.w)
		return 0;

	// search for existing contact
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		// check if it is a weather contact
		if (IsMyContact(hContact)) {
			DBVARIANT dbv;
			// check ID to see if the contact already exist in the database
			if (!db_get_ws(hContact, WEATHERPROTONAME, "ID", &dbv)) {
				if (!mir_wstrcmpi(psr->email.w, dbv.ptszVal)) {
					// remove the flag for not on list and hidden, thus make the contact visible
					// and add them on the list
					if (db_get_b(hContact, "CList", "NotOnList", 1)) {
						db_unset(hContact, "CList", "NotOnList");
						db_unset(hContact, "CList", "Hidden");
					}
					db_free(&dbv);
					// contact is added, function quitting
					return (INT_PTR)hContact;
				}
				db_free(&dbv);
			}
		}
	}

	// if contact with the same ID was not found, add it
	if (psr->cbSize < sizeof(PROTOSEARCHRESULT)) return 0;
	MCONTACT hContact = db_add_contact();
	Proto_AddToContact(hContact, WEATHERPROTONAME);
	// suppress online notification for the new contact
	CallService(MS_IGNORE_IGNORE, hContact, IGNOREEVENT_USERONLINE);

	// set contact info and settings
	wchar_t svc[256];
	wcsncpy(svc, psr->email.w, _countof(svc)); svc[_countof(svc) - 1] = 0;
	GetSvc(svc);
	// set settings by obtaining the default for the service 
	if (psr->lastName.w[0] != 0) {
		WIDATA *sData = GetWIData(svc);
		db_set_ws(hContact, WEATHERPROTONAME, "MapURL", sData->DefaultMap);
		db_set_s(hContact, WEATHERPROTONAME, "InfoURL", sData->DefaultURL);
	}
	else { // if no valid service is found, create empty strings for MapURL and InfoURL
		db_set_s(hContact, WEATHERPROTONAME, "MapURL", "");
		db_set_s(hContact, WEATHERPROTONAME, "InfoURL", "");
	}
	// write the other info and settings to the database
	db_set_ws(hContact, WEATHERPROTONAME, "ID", psr->email.w);
	db_set_ws(hContact, WEATHERPROTONAME, "Nick", psr->nick.w);
	db_set_w(hContact, WEATHERPROTONAME, "Status", ID_STATUS_OFFLINE);

	AvatarDownloaded(hContact);

	wchar_t str[256];
	mir_snwprintf(str, TranslateT("Current weather information for %s."), psr->nick.w);
	db_set_ws(hContact, WEATHERPROTONAME, "About", str);

	// make the last update tags to something invalid
	db_set_s(hContact, WEATHERPROTONAME, "LastLog", "never");
	db_set_s(hContact, WEATHERPROTONAME, "LastCondition", "None");
	db_set_s(hContact, WEATHERPROTONAME, "LastTemperature", "None");

	// ignore status change
	db_set_dw(hContact, "Ignore", "Mask", 8);

	// if no default station is found, set the new contact as default station
	if (opt.Default[0] == 0) {
		DBVARIANT dbv;
		GetStationID(hContact, opt.Default, _countof(opt.Default));

		opt.DefStn = hContact;
		if (!db_get_ws(hContact, WEATHERPROTONAME, "Nick", &dbv)) {
			// notification message box
			mir_snwprintf(str, TranslateT("%s is now the default weather station"), dbv.ptszVal);
			db_free(&dbv);
			MessageBox(NULL, str, TranslateT("Weather Protocol"), MB_OK | MB_ICONINFORMATION);
		}
		db_set_ws(NULL, WEATHERPROTONAME, "Default", opt.Default);
	}
	// display the Edit Settings dialog box
	EditSettings(hContact, 0);
	return (INT_PTR)hContact;
}

// ============ WARNING DIALOG  ============

// show a message box and cancel search if update is in process
BOOL CheckSearch()
{
	if (UpdateListHead != NULL) {
		MessageBox(NULL, TranslateT("Please try again after weather update is completed."), TranslateT("Weather Protocol"), MB_OK | MB_ICONERROR);
		return FALSE;
	}
	return TRUE;
}

// ============ BASIC ID SEARCH  ============

static wchar_t sttSID[32];

// A timer process for the ID search (threaded)
static void __cdecl BasicSearchTimerProc(LPVOID)
{
	int result;
	// search only when it's not current updating weather.
	if (CheckSearch())
		result = IDSearch(sttSID, sttSearchId);

	// broadcast the search result
	ProtoBroadcastAck(WEATHERPROTONAME, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)sttSearchId, 0);

	// exit the search
	sttSearchId = -1;
}

// the service function for ID search
// lParam = ID search string
INT_PTR WeatherBasicSearch(WPARAM, LPARAM lParam)
{
	if (sttSearchId != -1) return 0;   //only one search at a time
	wcsncpy(sttSID, (wchar_t*)lParam, _countof(sttSID));
	sttSID[_countof(sttSID) - 1] = 0;
	sttSearchId = 1;
	// create a thread for the ID search
	mir_forkthread(BasicSearchTimerProc, NULL);
	return sttSearchId;
}

// ============ NAME SEARCH  ============
//
// name search timer process (threaded)
static void __cdecl NameSearchTimerProc(LPVOID)
{
	// search only when it's not current updating weather.
	if (CheckSearch())
		if (name1[0] != 0)
			NameSearch(name1, sttSearchId);	// search nickname field

	// broadcast the result
	ProtoBroadcastAck(WEATHERPROTONAME, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)sttSearchId, 0);

	// exit the search
	sttSearchId = -1;
}

static INT_PTR CALLBACK WeatherSearchAdvancedDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
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

INT_PTR WeatherCreateAdvancedSearchUI(WPARAM, LPARAM lParam)
{
	HWND parent = (HWND)lParam;
	if (parent)
		return (INT_PTR)CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_SEARCHCITY), parent, WeatherSearchAdvancedDlgProc, 0);

	return 0;
}

// service function for name search
INT_PTR WeatherAdvancedSearch(WPARAM, LPARAM lParam)
{
	if (sttSearchId != -1) return 0;   //only one search at a time

	sttSearchId = 1;
	GetDlgItemText((HWND)lParam, IDC_SEARCHCITY, name1, _countof(name1));

	// search for the weather station using a thread
	mir_forkthread(NameSearchTimerProc, NULL);
	return sttSearchId;
}

// ============ SEARCH FOR A WEATHER STATION USING ID  ============

// Seaching station ID from a single weather service (Threaded)
// sID = search string for the station ID
// searchId = -1
// sData = the ID search data for that particular weather service
// svcname = the name of the weather service that is currently searching (ie. Yahoo Weather)
int IDSearchProc(wchar_t *sID, const int searchId, WIIDSEARCH *sData, wchar_t *svc, wchar_t *svcname)
{
	wchar_t str[MAX_DATA_LEN], newID[MAX_DATA_LEN];

	if (sData->Available) {
		char loc[255];
		wchar_t *szData = NULL;

		// load the page
		mir_snprintf(loc, sData->SearchURL, sID);
		BOOL bFound = (InternetDownloadFile(loc, NULL, NULL, &szData) == 0);
		if (bFound) {
			wchar_t* szInfo = szData;

			// not found
			if (wcsstr(szInfo, sData->NotFoundStr) == NULL)
				GetDataValue(&sData->Name, str, &szInfo);
		}

		mir_free(szData);
		// Station not found exit
		if (!bFound) return 1;
	}

	// give no station name but only ID if the search is unavailable
	else wcsncpy(str, TranslateT("<Enter station name here>"), MAX_DATA_LEN - 1);
	mir_snwprintf(newID, L"%s/%s", svc, sID);

	// set the search result and broadcast it
	PROTOSEARCHRESULT psr = { sizeof(psr) };
	psr.flags = PSR_UNICODE;
	psr.nick.w = str;
	psr.firstName.w = L" ";
	psr.lastName.w = svcname;
	psr.email.w = newID;
	ProtoBroadcastAck(WEATHERPROTONAME, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)searchId, (LPARAM)&psr);

	return 0;
}

// ID search	 (Threaded)
//  sID:		the ID to search for
//  searchId:	don't change
// return 0 if no error
int IDSearch(wchar_t *sID, const int searchId)
{
	// for a normal ID search (ID != #)
	if (mir_wstrcmp(sID, L"#")) {
		WIDATALIST *Item = WIHead;

		// search every weather service using the search station ID
		while (Item != NULL) {
			IDSearchProc(sID, searchId, &Item->Data.IDSearch, Item->Data.InternalName, Item->Data.DisplayName);
			Item = Item->next;
		}
		NetlibHttpDisconnect();
	}
	// if the station ID is #, return a dummy result and quit the funciton
	else {
		// return an empty contact on "#"
		PROTOSEARCHRESULT psr = { sizeof(psr) };
		psr.flags = PSR_UNICODE;
		psr.nick.w = TranslateT("<Enter station name here>");	// to be entered
		psr.firstName.w = L" ";
		psr.lastName.w = L"";
		psr.email.w = TranslateT("<Enter station ID here>");		// to be entered
		ProtoBroadcastAck(WEATHERPROTONAME, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)searchId, (LPARAM)&psr);
	}

	return 0;
}

// ============ SEARCH FOR A WEATHER STATION BY NAME  ============

// Seaching station name from a single weather service (Threaded)
// name = the name of the weather station to be searched
// searchId = -1
// sData = the name search data for that particular weather service
// svcname = the name of the weather service that is currently searching (ie. Yahoo Weather)
int NameSearchProc(wchar_t *name, const int searchId, WINAMESEARCH *sData, wchar_t *svc, wchar_t *svcname)
{
	wchar_t Name[MAX_DATA_LEN], str[MAX_DATA_LEN], sID[MAX_DATA_LEN], *szData = NULL, *search;

	// replace spaces with %20
	char loc[256];
	T2Utf szSearchName(name);
	mir_snprintf(loc, sData->SearchURL, ptrA(mir_urlEncode(szSearchName)));
	if (InternetDownloadFile(loc, NULL, NULL, &szData) == 0) {
		wchar_t* szInfo = szData;
		search = wcsstr(szInfo, sData->NotFoundStr);	// determine if data is available
		if (search == NULL) { // if data is found
			// test if it is single result
			if (sData->Single.Available && sData->Multiple.Available)
				search = wcsstr(szInfo, sData->SingleStr);
			// for single result
			if (sData->Single.Available && (search != NULL || !sData->Multiple.Available)) { // single result
				// if station ID appears first in the downloaded data
				if (!mir_wstrcmpi(sData->Single.First, L"ID")) {
					GetDataValue(&sData->Single.ID, str, &szInfo);
					mir_snwprintf(sID, L"%s/%s", svc, str);
					GetDataValue(&sData->Single.Name, Name, &szInfo);
				}
				// if station name appears first in the downloaded data
				else if (!mir_wstrcmpi(sData->Single.First, L"NAME")) {
					GetDataValue(&sData->Single.Name, Name, &szInfo);
					GetDataValue(&sData->Single.ID, str, &szInfo);
					mir_snwprintf(sID, L"%s/%s", svc, str);
				}
				else
					str[0] = 0;

				// if no station ID is obtained, quit the search
				if (str[0] == 0) {
					mir_free(szData);
					return 1;
				}

				// if can't get the name, use the search string as name
				if (Name[0] == 0)
					wcsncpy(Name, name, _countof(Name));

				// set the data and broadcast it
				PROTOSEARCHRESULT psr = { sizeof(psr) };
				psr.flags = PSR_UNICODE;
				psr.nick.w = Name;
				psr.firstName.w = L" ";
				psr.lastName.w = svcname;
				psr.email.w = sID;
				psr.id.w = sID;
				ProtoBroadcastAck(WEATHERPROTONAME, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)searchId, (LPARAM)&psr);
				mir_free(szData);
				return 0;
			}
			// for multiple result
			else if (sData->Multiple.Available) { // multiple results
				// search for the next occurrence of the string
				while (true) {
					// if station ID appears first in the downloaded data
					if (!mir_wstrcmpi(sData->Multiple.First, L"ID")) {
						GetDataValue(&sData->Multiple.ID, str, &szInfo);
						mir_snwprintf(sID, L"%s/%s", svc, str);
						GetDataValue(&sData->Multiple.Name, Name, &szInfo);
					}
					// if station name appears first in the downloaded data
					else if (!mir_wstrcmpi(sData->Multiple.First, L"NAME")) {
						GetDataValue(&sData->Multiple.Name, Name, &szInfo);
						GetDataValue(&sData->Multiple.ID, str, &szInfo);
						mir_snwprintf(sID, L"%s/%s", svc, str);
					}
					else
						break;

					// if no station ID is obtained, search completed and quit the search
					if (str[0] == 0)
						break;

					// if can't get the name, use the search string as name
					if (Name[0] == 0)
						wcsncpy(Name, name, _countof(Name));

					PROTOSEARCHRESULT psr = { sizeof(psr) };
					psr.flags = PSR_UNICODE;
					psr.nick.w = Name;
					psr.firstName.w = L"";
					psr.lastName.w = svcname;
					psr.email.w = sID;
					psr.id.w = sID;
					ProtoBroadcastAck(WEATHERPROTONAME, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)searchId, (LPARAM)&psr);
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
int NameSearch(wchar_t *name, const int searchId)
{
	WIDATALIST *Item = WIHead;

	// search every weather service using the search station name
	while (Item != NULL) {
		if (Item->Data.NameSearch.Single.Available || Item->Data.NameSearch.Multiple.Available)
			NameSearchProc(name, searchId, &Item->Data.NameSearch, Item->Data.InternalName, Item->Data.DisplayName);
		Item = Item->next;
	}

	NetlibHttpDisconnect();
	return 0;
}

// ======================MENU ITEM FUNCTION ============

// add a new weather station via find/add dialog
int WeatherAdd(WPARAM, LPARAM)
{
	db_set_s(NULL, "FindAdd", "LastSearched", "Weather");
	CallService(MS_FINDADD_FINDADD, 0, 0);
	return 0;
}
