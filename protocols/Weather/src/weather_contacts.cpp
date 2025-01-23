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

bool CWeatherProto::IsMyContact(MCONTACT hContact)
{
	return !mir_strcmp(m_szModuleName, Proto_GetBaseAccountName(hContact));
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
	wchar_t id[256], loc2[256];
	GetStationID(wParam, id, _countof(id));
	if (id[0] != 0) {
		// check if the complte forecast URL is set. If it is not, display warning and quit
		if (db_get_wstatic(wParam, m_szModuleName, "InfoURL", loc2, _countof(loc2)) || loc2[0] == 0) {
			MessageBox(nullptr, TranslateT("The URL for complete forecast has not been set. You can set it from the Edit Settings dialog."), TranslateT("Weather Protocol"), MB_ICONINFORMATION);
			return 1;
		}
		// set the url and open the webpage
		OpenUrl(loc2, id);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// load weather map
// wParam = current contact

INT_PTR CWeatherProto::WeatherMap(WPARAM wParam, LPARAM)
{
	wchar_t id[256], loc2[256];
	GetStationID(wParam, id, _countof(id));
	if (id[0] != 0) {
		// check if the weather map URL is set. If it is not, display warning and quit
		if (db_get_wstatic(wParam, m_szModuleName, "MapURL", loc2, _countof(loc2)) || loc2[0] == 0) {
			MessageBox(nullptr, TranslateT("The URL for weather map has not been set. You can set it from the Edit Settings dialog."), TranslateT("Weather Protocol"), MB_ICONINFORMATION);
			return 1;
		}

		// set the url and open the webpage
		OpenUrl(loc2, id);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// edit weather settings
// lParam = current contact

class CEditDlg : public CWeatherDlgBase
{
	MCONTACT hContact;
	HICON  hRename;
	HICON  hUserDetail;
	HICON  hFile;
	HICON  hSrchAll;

	CCtrlEdit edtID, edtName;
	CCtrlButton btnGetName, btnExternal, btnBrowse, btnView1, btnView2, btnReset1, btnReset2, btnSvcInfo, btnChange;

	wchar_t str[MAX_DATA_LEN], str2[256], city[256], filter[256], *chop;
	char loc[512];
	OPENFILENAME ofn;       // common dialog box structure
	WIDATA *sData;

public:
	CEditDlg(CWeatherProto *ppro, MCONTACT _1) :
		CWeatherDlgBase(ppro, IDD_EDIT),
		hContact(_1),
		edtID(this, IDC_ID),
		edtName(this, IDC_NAME),
		btnView1(this, IDC_VIEW1),
		btnView2(this, IDC_VIEW2),
		btnReset1(this, IDC_RESET1),
		btnReset2(this, IDC_RESET2),
		btnBrowse(this, IDC_BROWSE),
		btnChange(this, IDC_CHANGE),
		btnGetName(this, IDC_GETNAME),
		btnSvcInfo(this, IDC_SVCINFO),
		btnExternal(this, IDC_External)
	{
		edtID.OnChange = Callback(this, &CEditDlg::onChanged_ID);
		edtName.OnChange = Callback(this, &CEditDlg::onChanged_Name);

		btnView1.OnClick = Callback(this, &CEditDlg::onClick_View1);
		btnView2.OnClick = Callback(this, &CEditDlg::onClick_View2);
		btnReset1.OnClick = Callback(this, &CEditDlg::onClick_Reset1);
		btnReset2.OnClick = Callback(this, &CEditDlg::onClick_Reset2);
		btnBrowse.OnClick = Callback(this, &CEditDlg::onClick_Browse);
		btnChange.OnClick = Callback(this, &CEditDlg::onClick_Change);
		btnGetName.OnClick = Callback(this, &CEditDlg::onClick_GetName);
		btnSvcInfo.OnClick = Callback(this, &CEditDlg::onClick_SvcInfo);
		btnExternal.OnClick = Callback(this, &CEditDlg::onClick_External);
	}

	bool OnInitDialog() override
	{
		hRename = Skin_LoadIcon(SKINICON_OTHER_RENAME);
		hUserDetail = Skin_LoadIcon(SKINICON_OTHER_USERDETAILS);
		hFile = Skin_LoadIcon(SKINICON_EVENT_FILE);
		hSrchAll = Skin_LoadIcon(SKINICON_OTHER_SEARCHALL);

		// set button images
		SendDlgItemMessage(m_hwnd, IDC_GETNAME, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hRename);
		SendDlgItemMessage(m_hwnd, IDC_SVCINFO, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hUserDetail);
		SendDlgItemMessage(m_hwnd, IDC_BROWSE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hFile);
		SendDlgItemMessage(m_hwnd, IDC_VIEW1, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hSrchAll);
		SendDlgItemMessage(m_hwnd, IDC_RESET1, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hRename);
		SendDlgItemMessage(m_hwnd, IDC_VIEW2, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hSrchAll);
		SendDlgItemMessage(m_hwnd, IDC_RESET2, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hRename);

		// make all buttons flat
		SendDlgItemMessage(m_hwnd, IDC_GETNAME, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(m_hwnd, IDC_SVCINFO, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(m_hwnd, IDC_BROWSE, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(m_hwnd, IDC_VIEW1, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(m_hwnd, IDC_RESET1, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(m_hwnd, IDC_VIEW2, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(m_hwnd, IDC_RESET2, BUTTONSETASFLATBTN, TRUE, 0);

		// set tooltip for the buttons
		SendDlgItemMessage(m_hwnd, IDC_GETNAME, BUTTONADDTOOLTIP, (WPARAM)LPGENW("Get city name from ID"), BATF_UNICODE);
		SendDlgItemMessage(m_hwnd, IDC_SVCINFO, BUTTONADDTOOLTIP, (WPARAM)LPGENW("Weather INI information"), BATF_UNICODE);
		SendDlgItemMessage(m_hwnd, IDC_BROWSE, BUTTONADDTOOLTIP, (WPARAM)LPGENW("Browse"), BATF_UNICODE);
		SendDlgItemMessage(m_hwnd, IDC_VIEW1, BUTTONADDTOOLTIP, (WPARAM)LPGENW("View webpage"), BATF_UNICODE);
		SendDlgItemMessage(m_hwnd, IDC_RESET1, BUTTONADDTOOLTIP, (WPARAM)LPGENW("Reset to default"), BATF_UNICODE);
		SendDlgItemMessage(m_hwnd, IDC_VIEW2, BUTTONADDTOOLTIP, (WPARAM)LPGENW("View webpage"), BATF_UNICODE);
		SendDlgItemMessage(m_hwnd, IDC_RESET2, BUTTONADDTOOLTIP, (WPARAM)LPGENW("Reset to default"), BATF_UNICODE);

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

		if (!m_proto->getWString(hContact, "InfoURL", &dbv)) {
			SetDlgItemText(m_hwnd, IDC_IURL, dbv.pwszVal);
			db_free(&dbv);
		}
		if (!m_proto->getWString(hContact, "MapURL", &dbv)) {
			SetDlgItemText(m_hwnd, IDC_MURL, dbv.pwszVal);
			db_free(&dbv);
		}

		// display the dialog box and free memory
		Utils_RestoreWindowPositionNoMove(m_hwnd, NULL, MODULENAME, "EditSetting_");
		ShowWindow(m_hwnd, SW_SHOW);
		return true;
	}

	void OnDestroy() override
	{
		IcoLib_ReleaseIcon(hFile);
		IcoLib_ReleaseIcon(hRename);
		IcoLib_ReleaseIcon(hSrchAll);
		IcoLib_ReleaseIcon(hUserDetail);
		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);

		WindowList_Remove(hWindowList, m_hwnd);
		Utils_SaveWindowPosition(m_hwnd, NULL, MODULENAME, "EditSetting_");
	}

	void onChanged_ID(CCtrlEdit *)
	{
		// check if there are 2 parts in the ID (svc/id) seperated by "/"
		// if not, don't let user change the setting
		GetDlgItemText(m_hwnd, IDC_ID, str, _countof(str));
		chop = wcschr(str, '/');
		if (chop == nullptr)
			EnableWindow(GetDlgItem(m_hwnd, IDC_CHANGE), FALSE);
		else
			EnableWindow(GetDlgItem(m_hwnd, IDC_CHANGE), TRUE);
	}

	void onChanged_Name(CCtrlEdit *)
	{
		// check if station name is entered
		// if not, don't let user change the setting
		GetDlgItemText(m_hwnd, IDC_NAME, str, _countof(str));
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHANGE), str[0] != 0);
	}

	void onClick_GetName(CCtrlButton *)
	{
		// the button for getting station name from the internet
		// this function uses the ID search for add/find weather station
		if (!m_proto->CheckSearch())
			return;	// don't download if update is in progress

		// get the weather update data using the string in the ID field
		GetDlgItemText(m_hwnd, IDC_ID, str, _countof(str));
		GetSvc(str);
		sData = GetWIData(str);
		GetDlgItemText(m_hwnd, IDC_ID, str, _countof(str));
		GetID(str);

		// if ID search is available, do it
		if (sData->IDSearch.Available) {
			// load the page
			mir_snprintf(loc, sData->IDSearch.SearchURL, str);
			str[0] = 0;
			wchar_t *pData = nullptr;
			if (m_proto->InternetDownloadFile(loc, nullptr, sData->UserAgent, &pData) == 0) {
				wchar_t *szInfo = pData;
				wchar_t *search = wcsstr(szInfo, sData->IDSearch.NotFoundStr);

				// if the page is found (ie. valid ID), get the name of the city
				if (search == nullptr)
					m_proto->GetDataValue(&sData->IDSearch.Name, str, &szInfo);
			}
			// free memory
			mir_free(pData);
		}

		// give no station name but only ID if the search is unavailable
		if (str[0] != 0)
			SetDlgItemText(m_hwnd, IDC_NAME, str);
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
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = m_hwnd;
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
			SetDlgItemText(m_hwnd, IDC_LOG, ofn.lpstrFile);
	
		// if there is no log file specified, disable external logging
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHANGE), ofn.lpstrFile[0] != 0);
	}

	void onClick_View1(CCtrlButton *)
	{
		// view the page for more info
		GetDlgItemText(m_hwnd, IDC_IURL, str, _countof(str));
		if (str[0]) {
			GetDlgItemText(m_hwnd, IDC_ID, str2, _countof(str2));
			OpenUrl(str, str2);
		}
	}

	void onClick_View2(CCtrlButton *)
	{
		// view the page for weather map
		GetDlgItemText(m_hwnd, IDC_MURL, str, _countof(str));
		if (str[0]) {
			GetDlgItemText(m_hwnd, IDC_ID, str2, _countof(str2));
			OpenUrl(str, str2);
		}
	}

	void onClick_Reset1(CCtrlButton *)
	{
		// reset the more info url to service default
		GetDlgItemText(m_hwnd, IDC_ID, str, _countof(str));
		GetSvc(str);
		sData = GetWIData(str);
		SetDlgItemTextA(m_hwnd, IDC_IURL, sData->DefaultURL);
	}

	void onClick_Reset2(CCtrlButton *)
	{
		// reset the weathe map url to service default
		GetDlgItemText(m_hwnd, IDC_ID, str, _countof(str));
		GetSvc(str);
		sData = GetWIData(str);
		SetDlgItemText(m_hwnd, IDC_MURL, sData->DefaultMap);
	}

	void onClick_SvcInfo(CCtrlButton *)
	{
		// display the information of the ini file used by the weather station
		GetDlgItemText(m_hwnd, IDC_ID, str, _countof(str));
		GetSvc(str);
		GetINIInfo(str);
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
		GetDlgItemText(m_hwnd, IDC_NAME, city, _countof(city));
		m_proto->setWString(hContact, "Nick", city);
		mir_snwprintf(str2, TranslateT("Current weather information for %s."), city);
		if ((uint8_t)IsDlgButtonChecked(m_hwnd, IDC_External)) {
			GetDlgItemText(m_hwnd, IDC_LOG, str, _countof(str));
			m_proto->setWString(hContact, "Log", str);
		}
		else m_proto->delSetting(hContact, "Log");

		GetDlgItemText(m_hwnd, IDC_IURL, str, _countof(str));
		m_proto->setWString(hContact, "InfoURL", str);

		GetDlgItemText(m_hwnd, IDC_MURL, str, _countof(str));
		m_proto->setWString(hContact, "MapURL", str);
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
	if (tszID != NULL)
		if (mir_wstrcmp(tszID, opt.Default))
			return false;

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
