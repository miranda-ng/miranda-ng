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
		if (!getWString(hContact, "ID", &dbv)) {
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
	Proto_AddToContact(hContact, m_szModuleName);
	// suppress online notification for the new contact
	Ignore_Ignore(hContact, IGNOREEVENT_USERONLINE);

	// set contact info and settings
	wchar_t svc[256];
	wcsncpy(svc, psr->email.w, _countof(svc)); svc[_countof(svc) - 1] = 0;

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
		ptrW wszNick(getWStringA(hContact, "Nick"));
		if (mir_wstrlen(wszNick)) {
			// notification message box
			mir_snwprintf(str, TranslateT("%s is now the default weather station"), wszNick);
			MessageBox(nullptr, str, TranslateT("Weather Protocol"), MB_OK | MB_ICONINFORMATION);
		}
		
		setWString("Default", opt.Default);
	}
	
	// display the Edit Settings dialog box
	EditSettings(hContact, 0);
	return hContact;
}

/////////////////////////////////////////////////////////////////////////////////////////
// shows a message box and cancel search if update is in process

bool CWeatherProto::CheckSearch()
{
	if (UpdateListHead != nullptr) {
		MessageBox(nullptr, TranslateT("Please try again after weather update is completed."), TranslateT("Weather Protocol"), MB_OK | MB_ICONERROR);
		return false;
	}
	return true;
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

int CWeatherProto::IDSearch(wchar_t *sID, const int searchId)
{
	// return an empty contact on "#"
	PROTOSEARCHRESULT psr = { sizeof(psr) };
	psr.flags = PSR_UNICODE;
	psr.nick.w = TranslateT("<Enter station name here>");	// to be entered
	psr.firstName.w = L" ";
	psr.lastName.w = L"";
	psr.email.w = TranslateT("<Enter station ID here>");		// to be entered
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)searchId, (LPARAM)&psr);

	return 0;
}

int CWeatherProto::NameSearch(wchar_t *name, const int searchId)
{
	return 0;
}
