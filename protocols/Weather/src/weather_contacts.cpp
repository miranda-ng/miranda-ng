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

bool CWeatherProto::IsMyContact(MCONTACT hContact)
{
	return hContact && !mir_strcmp(m_szModuleName, Proto_GetBaseAccountName(hContact));
}

/////////////////////////////////////////////////////////////////////////////////////////
// view weather log for the contact
// wParam = current contact

INT_PTR CWeatherProto::ViewLog(WPARAM wParam, LPARAM lParam)
{
	// see if the log path is set
	DBVARIANT dbv;
	if (!getWString(wParam, "Log", &dbv)) {
		if (dbv.pszVal[0] != 0)
			ShellExecute((HWND)lParam, L"open", dbv.pwszVal, L"", L"", SW_SHOW);
		db_free(&dbv);
	}
	else	// display warning dialog if no path is specified
		MessageBox(nullptr, TranslateT("Weather condition was not logged."),
			TranslateT("Weather Protocol"), MB_OK | MB_ICONINFORMATION);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// read complete forecast
// wParam = current contact

INT_PTR CWeatherProto::LoadForecast(WPARAM wParam, LPARAM)
{
	wchar_t id[256];
	GetStationID(wParam, id, _countof(id));
	if (id[0] != 0) {
		// set the url and open the webpage
		CMStringA szUrl("https://www.visualcrossing.com/weather-forecast/" + mir_urlEncode(T2Utf(id)) + "/metric");
		Utils_OpenUrl(szUrl);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// load weather map
// wParam = current contact

INT_PTR CWeatherProto::WeatherMap(WPARAM wParam, LPARAM)
{
	wchar_t id[256];
	GetStationID(wParam, id, _countof(id));
	if (id[0] != 0) {
		// set the url and open the webpage
		CMStringA szUrl("https://www.visualcrossing.com/weather-history/" + mir_urlEncode(T2Utf(id)) + "/metric");
		Utils_OpenUrl(szUrl);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// edit weather settings
// lParam = current contact

class CEditDlg : public CWeatherDlgBase
{
	MCONTACT hContact;

	CCtrlEdit edtName;
	CCtrlButton btnExternal, btnChange;
	CCtrlMButton btnBrowse;

	wchar_t str[MAX_DATA_LEN], str2[256];

public:
	CEditDlg(CWeatherProto *ppro, MCONTACT _1) :
		CWeatherDlgBase(ppro, IDD_EDIT),
		hContact(_1),
		edtName(this, IDC_NAME),
		btnBrowse(this, IDC_BROWSE, SKINICON_EVENT_FILE, LPGEN("Browse")),
		btnChange(this, IDC_CHANGE),
		btnExternal(this, IDC_External)
	{
		edtName.OnChange = Callback(this, &CEditDlg::onChanged_Name);

		btnBrowse.OnClick = Callback(this, &CEditDlg::onClick_Browse);
		btnChange.OnClick = Callback(this, &CEditDlg::onClick_Change);
		btnExternal.OnClick = Callback(this, &CEditDlg::onClick_External);
	}

	bool OnInitDialog() override
	{
		// make all buttons flat
		btnBrowse.MakeFlat();

		// save the handle for the contact
		WindowList_Add(hWindowList, m_hwnd, hContact);

		// start to get the settings
		// if the setting not exist, leave the dialog box blank
		DBVARIANT dbv;
		if (!m_proto->getWString(hContact, "ID", &dbv)) {
			SetDlgItemText(m_hwnd, IDC_ID, dbv.pwszVal);
			// check if the station is a default station
			CheckDlgButton(m_hwnd, IDC_DEFA, mir_wstrcmp(dbv.pwszVal, m_proto->opt.Default) != 0 ? BST_CHECKED : BST_UNCHECKED);
			db_free(&dbv);
		}
		if (!m_proto->getWString(hContact, "Nick", &dbv)) {
			SetDlgItemText(m_hwnd, IDC_NAME, dbv.pwszVal);
			db_free(&dbv);
		}
		if (!m_proto->getWString(hContact, "Log", &dbv)) {
			SetDlgItemText(m_hwnd, IDC_LOG, dbv.pwszVal);
			// if the log path is not empty, check the checkbox for external log
			if (dbv.pwszVal[0]) CheckDlgButton(m_hwnd, IDC_External, BST_CHECKED);
			db_free(&dbv);
		}
		// enable/disable the browse button depending on the value of external log checkbox
		EnableWindow(GetDlgItem(m_hwnd, IDC_BROWSE), (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_External));

		// other checkbox options
		CheckDlgButton(m_hwnd, IDC_DPop, m_proto->getByte(hContact, "DPopUp", FALSE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_DAutoUpdate, m_proto->getByte(hContact, "DAutoUpdate", FALSE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_Internal, m_proto->getByte(hContact, "History", 0) ? BST_CHECKED : BST_UNCHECKED);

		// display the dialog box and free memory
		Utils_RestoreWindowPositionNoSize(m_hwnd, NULL, MODULENAME, "EditSetting_");
		ShowWindow(m_hwnd, SW_SHOW);
		return true;
	}

	void OnDestroy() override
	{
		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);

		WindowList_Remove(hWindowList, m_hwnd);
		Utils_SaveWindowPosition(m_hwnd, NULL, MODULENAME, "EditSetting_");
	}

	void onChanged_Name(CCtrlEdit *)
	{
		// check if station name is entered
		// if not, don't let user change the setting
		GetDlgItemText(m_hwnd, IDC_NAME, str, _countof(str));
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHANGE), str[0] != 0);
	}

	void onClick_External(CCtrlButton *)
	{
		// enable/disable the borwse button depending if the external log is enabled
		EnableWindow(GetDlgItem(m_hwnd, IDC_BROWSE), (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_External));
		if (!(uint8_t)IsDlgButtonChecked(m_hwnd, IDC_External))
			return;

		onClick_Browse(0);
	}

	void onClick_Browse(CCtrlButton *)
	{
		// browse for the external log file
		GetDlgItemText(m_hwnd, IDC_LOG, str, _countof(str));

		// Initialize OPENFILENAME
		OPENFILENAME ofn = {};
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = m_hwnd;
		ofn.lpstrFile = str;
		ofn.nMaxFile = _countof(str);

		// set filters
		wchar_t filter[256];
		mir_snwprintf(filter, L"%s (*.txt)%c*.txt%c%s (*.*)%c*.*%c%c", TranslateT("Text Files"), 0, 0, TranslateT("All Files"), 0, 0, 0);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = nullptr;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = nullptr;
		ofn.Flags = OFN_PATHMUSTEXIST;

		// Display a Open dialog box and put the file name on the dialog
		if (GetOpenFileName(&ofn))
			SetDlgItemText(m_hwnd, IDC_LOG, ofn.lpstrFile);
	
		// if there is no log file specified, disable external logging
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHANGE), ofn.lpstrFile[0] != 0);
	}

	void onClick_Change(CCtrlButton *)
	{
		// temporary disable the protocol while applying the change
		// start writing the new settings to database
		GetDlgItemText(m_hwnd, IDC_ID, str, _countof(str));
		m_proto->setWString(hContact, "ID", str);
		if ((uint8_t)IsDlgButtonChecked(m_hwnd, IDC_DEFA)) {	// if default station is set
			mir_wstrcpy(m_proto->opt.Default, str);
			m_proto->opt.DefStn = hContact;
			m_proto->setWString("Default", m_proto->opt.Default);
		}
		
		wchar_t city[256];
		GetDlgItemText(m_hwnd, IDC_NAME, city, _countof(city));
		m_proto->setWString(hContact, "Nick", city);
		mir_snwprintf(str2, TranslateT("Current weather information for %s."), city);
		if ((uint8_t)IsDlgButtonChecked(m_hwnd, IDC_External)) {
			GetDlgItemText(m_hwnd, IDC_LOG, str, _countof(str));
			m_proto->setWString(hContact, "Log", str);
		}
		else m_proto->delSetting(hContact, "Log");

		m_proto->setWord(hContact, "Status", ID_STATUS_OFFLINE);
		m_proto->setWord(hContact, "StatusIcon", -1);
		m_proto->AvatarDownloaded(hContact);
		m_proto->setWString(hContact, "About", str2);
		m_proto->setByte(hContact, "History", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_Internal));
		m_proto->setByte(hContact, "Overwrite", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_Overwrite));
		m_proto->setByte(hContact, "File", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_External));
		m_proto->setByte(hContact, "DPopUp", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_DPop));
		m_proto->setByte(hContact, "DAutoUpdate", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_DAutoUpdate));

		// re-enable the protocol and update the data for the station
		m_proto->setString(hContact, "LastCondition", "None");
		m_proto->UpdateSingleStation(hContact, 0);
	}
};

INT_PTR CWeatherProto::EditSettings(WPARAM wParam, LPARAM)
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
			(new CEditDlg(this, wParam))->Create();
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// when a contact is deleted, make sure some other contact take over the default station

bool CWeatherProto::OnContactDeleted(MCONTACT hContact, uint32_t)
{
	RemoveFrameWindow(hContact);

	// exit this function if it is not default station
	ptrW tszID(getWStringA(hContact, "ID"));
	if (mir_wstrcmp(tszID, opt.Default))
		return true;

	// now the default station is deleted, try to get a new one

	// start looking for other weather stations
	for (auto &cc: AccContacts()) {
		tszID = getWStringA(cc, "ID");
		if (tszID == NULL)
			continue;

		// if the station is not a default station, set it as the new default station
		// this is the first weather station encountered from the search
		if (mir_wstrcmp(opt.Default, tszID)) {
			wcsncpy_s(opt.Default, tszID, _TRUNCATE);
			opt.DefStn = cc;
			ptrW tszNick(getWStringA(cc, "Nick"));
			if (tszNick != NULL) {
				wchar_t str[255];
				mir_snwprintf(str, TranslateT("%s is now the default weather station"), (wchar_t*)tszNick);
				MessageBox(nullptr, str, TranslateT("Weather Protocol"), MB_OK | MB_ICONINFORMATION);
			}
			setWString("Default", opt.Default);
			return true;
		}
	}

	// got here if no more weather station left
	opt.Default[0] = 0;	// no default station
	opt.DefStn = NULL;
	setWString("Default", opt.Default);
	return true;
}
