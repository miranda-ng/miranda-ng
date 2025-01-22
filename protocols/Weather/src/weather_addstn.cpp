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

/* This file contain the source related to search and add a weather station
to the contact list.  Contain code for both name and ID search.
*/

#include "stdafx.h"

// variables used for weather_addstn.c
static int sttSearchId = -1;
static wchar_t name1[256];

/////////////////////////////////////////////////////////////////////////////////////////
// protocol service function for adding a new contact onto contact list

MCONTACT CWeatherProto::AddToList(int, PROTOSEARCHRESULT *psr)
{
	if (!psr || !psr->email.w)
		return 0;

	// search for existing contact
	for (auto &hContact : AccContacts()) {
		DBVARIANT dbv;
		// check ID to see if the contact already exist in the database
		if (!g_plugin.getWString(hContact, "ID", &dbv)) {
			if (!mir_wstrcmpi(psr->email.w, dbv.pwszVal)) {
				// remove the flag for not on list and hidden, thus make the contact visible
				// and add them on the list
				if (!Contact::OnList(hContact)) {
					Contact::PutOnList(hContact);
					Contact::Hide(hContact, false);
				}
				db_free(&dbv);
				// contact is added, function quitting
				return (INT_PTR)hContact;
			}
			db_free(&dbv);
		}
	}

	// if contact with the same ID was not found, add it
	if (psr->cbSize < sizeof(PROTOSEARCHRESULT))
		return 0;
	
	MCONTACT hContact = db_add_contact();
	Proto_AddToContact(hContact, MODULENAME);
	// suppress online notification for the new contact
	Ignore_Ignore(hContact, IGNOREEVENT_USERONLINE);

	// set contact info and settings
	wchar_t svc[256];
	wcsncpy(svc, psr->email.w, _countof(svc)); svc[_countof(svc) - 1] = 0;
	GetSvc(svc);
	// set settings by obtaining the default for the service 
	if (psr->lastName.w[0] != 0) {
		WIDATA *sData = GetWIData(svc);
		setWString(hContact, "MapURL", sData->DefaultMap);
		setString(hContact, "InfoURL", sData->DefaultURL);
	}
	else { // if no valid service is found, create empty strings for MapURL and InfoURL
		setString(hContact, "MapURL", "");
		setString(hContact, "InfoURL", "");
	}
	// write the other info and settings to the database
	setWString(hContact, "ID", psr->email.w);
	setWString(hContact, "Nick", psr->nick.w);
	setWord(hContact, "Status", ID_STATUS_OFFLINE);

	AvatarDownloaded(hContact);

	wchar_t str[256];
	mir_snwprintf(str, TranslateT("Current weather information for %s."), psr->nick.w);
	setWString(hContact, "About", str);

	// make the last update tags to something invalid
	setString(hContact, "LastLog", "never");
	setString(hContact, "LastCondition", "None");
	setString(hContact, "LastTemperature", "None");

	// ignore status change
	db_set_dw(hContact, "Ignore", "Mask", 8);

	// if no default station is found, set the new contact as default station
	if (opt.Default[0] == 0) {
		GetStationID(hContact, opt.Default, _countof(opt.Default));

		opt.DefStn = hContact;
		ptrW wszNick(g_plugin.getWStringA(hContact, "Nick"));
		if (mir_wstrlen(wszNick)) {
			// notification message box
			mir_snwprintf(str, TranslateT("%s is now the default weather station"), wszNick);
			MessageBox(nullptr, str, TranslateT("Weather Protocol"), MB_OK | MB_ICONINFORMATION);
		}
		
		g_plugin.setWString("Default", opt.Default);
	}
	
	// display the Edit Settings dialog box
	EditSettings(hContact, 0);
	return hContact;
}

/////////////////////////////////////////////////////////////////////////////////////////
// shows a message box and cancel search if update is in process

BOOL CheckSearch()
{
	if (UpdateListHead != nullptr) {
		MessageBox(nullptr, TranslateT("Please try again after weather update is completed."), TranslateT("Weather Protocol"), MB_OK | MB_ICONERROR);
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// A timer process for the ID search (threaded)

void __cdecl CWeatherProto::BasicSearchThread(void *pParam)
{
	ptrW sID((wchar_t *)pParam);

	// search only when it's not current updating weather.
	if (CheckSearch())
		IDSearch(sID, sttSearchId);

	// broadcast the search result
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)sttSearchId);

	// exit the search
	sttSearchId = -1;
}

HANDLE CWeatherProto::SearchBasic(const wchar_t *id)
{
	if (sttSearchId != -1)
		return 0;   // only one search at a time
	
	sttSearchId = 1;
	ForkThread(&CWeatherProto::BasicSearchThread, mir_wstrdup(id));
	return (HANDLE)sttSearchId;
}

/////////////////////////////////////////////////////////////////////////////////////////
// name search timer process (threaded)

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

MWindow CWeatherProto::CreateExtendedSearchUI(MWindow hwndOwner)
{
	if (hwndOwner)
		return CreateDialogParamW(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SEARCHCITY), hwndOwner, WeatherSearchAdvancedDlgProc, 0);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// service function for name search

void __cdecl CWeatherProto::NameSearchThread(void *)
{
	// search only when it's not current updating weather.
	if (CheckSearch())
		if (name1[0] != 0)
			NameSearch(name1, sttSearchId);	// search nickname field

	// broadcast the result
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)sttSearchId);

	// exit the search
	sttSearchId = -1;
}

HANDLE CWeatherProto::SearchAdvanced(MWindow hwndOwner)
{
	if (sttSearchId != -1)
		return 0;   //only one search at a time

	sttSearchId = 1;
	GetDlgItemText(hwndOwner, IDC_SEARCHCITY, name1, _countof(name1));

	// search for the weather station using a thread
	ForkThread(&CWeatherProto::NameSearchThread);
	return (HANDLE)sttSearchId;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Seaching station ID from a single weather service
// sID = search string for the station ID
// searchId = -1
// sData = the ID search data for that particular weather service
// svcname = the name of the weather service that is currently searching (ie. Yahoo Weather)

int CWeatherProto::IDSearchProc(wchar_t *sID, const int searchId, WIIDSEARCH *sData, wchar_t *svc, wchar_t *svcname)
{
	wchar_t str[MAX_DATA_LEN], newID[MAX_DATA_LEN];

	if (sData->Available) {
		char loc[255];
		wchar_t *szData = nullptr;

		// load the page
		mir_snprintf(loc, sData->SearchURL, _T2A(sID).get());
		BOOL bFound = (InternetDownloadFile(loc, nullptr, nullptr, &szData) == 0);
		if (bFound) {
			wchar_t *szInfo = szData;

			// not found
			if (wcsstr(szInfo, sData->NotFoundStr) == nullptr)
				GetDataValue(&sData->Name, str, &szInfo);
		}

		mir_free(szData);
		// Station not found exit
		if (!bFound)
			return 1;
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
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)searchId, (LPARAM)&psr);

	return 0;
}

int CWeatherProto::IDSearch(wchar_t *sID, const int searchId)
{
	// for a normal ID search (ID != #)
	if (mir_wstrcmp(sID, L"#")) {
		WIDATALIST *Item = WIHead;

		// search every weather service using the search station ID
		while (Item != nullptr) {
			IDSearchProc(sID, searchId, &Item->Data.IDSearch, Item->Data.InternalName, Item->Data.DisplayName);
			Item = Item->next;
		}
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
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)searchId, (LPARAM)&psr);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Seaching station name from a single weather service (Threaded)
// name = the name of the weather station to be searched
// searchId = -1
// sData = the name search data for that particular weather service
// svcname = the name of the weather service that is currently searching (ie. Yahoo Weather)

int CWeatherProto::NameSearchProc(wchar_t *name, const int searchId, WINAMESEARCH *sData, wchar_t *svc, wchar_t *svcname)
{
	wchar_t Name[MAX_DATA_LEN], str[MAX_DATA_LEN], sID[MAX_DATA_LEN], *szData = nullptr, *search;

	// replace spaces with %20
	char loc[256];
	T2Utf szSearchName(name);
	mir_snprintf(loc, sData->SearchURL, mir_urlEncode(szSearchName).c_str());
	if (InternetDownloadFile(loc, nullptr, nullptr, &szData) == 0) {
		wchar_t *szInfo = szData;
		search = wcsstr(szInfo, sData->NotFoundStr);	// determine if data is available
		if (search == nullptr) { // if data is found
			// test if it is single result
			if (sData->Single.Available && sData->Multiple.Available)
				search = wcsstr(szInfo, sData->SingleStr);
			// for single result
			if (sData->Single.Available && (search != nullptr || !sData->Multiple.Available)) { // single result
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
				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)searchId, (LPARAM)&psr);
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
					ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)searchId, (LPARAM)&psr);
				}
			}
		}

		mir_free(szData);
		return 0;
	}

	mir_free(szData);
	return 1;
}

int CWeatherProto::NameSearch(wchar_t *name, const int searchId)
{
	// search every weather service using the search station name
	WIDATALIST *Item = WIHead;
	while (Item != nullptr) {
		if (Item->Data.NameSearch.Single.Available || Item->Data.NameSearch.Multiple.Available)
			NameSearchProc(name, searchId, &Item->Data.NameSearch, Item->Data.InternalName, Item->Data.DisplayName);
		Item = Item->next;
	}

	return 0;
}
