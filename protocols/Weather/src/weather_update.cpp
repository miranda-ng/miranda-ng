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

/*
This file contain the source related to updating new weather
information, both automatic (by timer) and manually (by selecting
menu items).
*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// retrieve weather info and display / log them
// hContact = current contact

int CWeatherProto::UpdateWeather(MCONTACT hContact)
{
	DBVARIANT dbv;
	BOOL Ch = FALSE;

	if (hContact == NULL) // some error prevention
		return 1;

	dbv.pszVal = "";

	// log to netlib log for debug purpose
	Netlib_LogfW(m_hNetlibUser, L"************************************************************************");
	int dbres = getWString(hContact, "Nick", &dbv);

	Netlib_LogfW(m_hNetlibUser, L"<-- Start update for station -->");

	// download the info and parse it
	// result are stored in database
	int code = GetWeatherData(hContact);
	if (code != 0) {
		// error occurs if the return value is not equals to 0
		if (opt.ShowWarnings) {
			// show warnings by popup
			CMStringW str(FORMAT, TranslateT("Unable to retrieve weather information for %s"), dbv.pwszVal);
			str.AppendChar('\n');
			str.Append(ptrW(GetError(code)));
			WPShowMessage(str, SM_WARNING);
		}
		// log to netlib
		Netlib_LogfW(m_hNetlibUser, L"Error! Update cannot continue... Start to free memory");
		Netlib_LogfW(m_hNetlibUser, L"<-- Error occurs while updating station: %s -->", dbv.pwszVal);
		if (!dbres)
			db_free(&dbv);
		return 1;
	}
	
	if (!dbres)
		db_free(&dbv);

	// initialize, load new weather Data
	WEATHERINFO winfo = LoadWeatherInfo(hContact);

	// translate weather condition
	mir_wstrcpy(winfo.cond, TranslateW(winfo.cond));

	// compare the old condition and determine if the weather had changed
	if (opt.UpdateOnlyConditionChanged) {	// consider condition change
		if (!getWString(hContact, "LastCondition", &dbv)) {
			if (mir_wstrcmpi(winfo.cond, dbv.pwszVal))  Ch = TRUE;		// the weather condition is changed
			db_free(&dbv);
		}
		else Ch = TRUE;
		if (!getWString(hContact, "LastTemperature", &dbv)) {
			if (mir_wstrcmpi(winfo.temp, dbv.pwszVal))  Ch = TRUE;		// the temperature is changed
			db_free(&dbv);
		}
		else Ch = TRUE;
	}
	else {	// consider update time change
		if (!getWString(hContact, "LastUpdate", &dbv)) {
			if (mir_wstrcmpi(winfo.update, dbv.pwszVal))  Ch = TRUE;		// the update time is changed
			db_free(&dbv);
		}
		else Ch = TRUE;
	}

	// have weather alert issued?
	dbres = db_get_ws(hContact, WEATHERCONDITION, "Alert", &dbv);
	if (!dbres && dbv.pwszVal[0] != 0) {
		if (opt.AlertPopup && !getByte(hContact, "DPopUp") && Ch) {
			// display alert popup
			CMStringW str(FORMAT, L"Alert for %s%c%s", winfo.city, 255, dbv.pwszVal);
			WPShowMessage(str, SM_WEATHERALERT);
		}
		// alert issued, set display to italic
		if (opt.MakeItalic)
			setWord(hContact, "ApparentMode", ID_STATUS_OFFLINE);
		Skin_PlaySound("weatheralert");
	}
	// alert dropped, set the display back to normal
	else delSetting(hContact, "ApparentMode");
	if (!dbres) db_free(&dbv);

	// backup current condition for checking if the weather is changed or not
	setWString(hContact, "LastLog", winfo.update);
	setWString(hContact, "LastCondition", winfo.cond);
	setWString(hContact, "LastTemperature", winfo.temp);
	setWString(hContact, "LastUpdate", winfo.update);

	// display condition on contact list
	int iStatus = MapCondToStatus(winfo.hContact);
	if (opt.DisCondIcon && iStatus != ID_STATUS_OFFLINE)
		setWord(hContact, "Status", ID_STATUS_ONLINE);
	else
		setWord(hContact, "Status", iStatus);
	AvatarDownloaded(hContact);

	db_set_ws(hContact, "CList", "MyHandle", GetDisplay(&winfo, GetTextValue('C')));

	CMStringW str2(GetDisplay(&winfo, GetTextValue('S')));
	if (!str2.IsEmpty())
		db_set_ws(hContact, "CList", "StatusMsg", str2);
	else
		db_unset(hContact, "CList", "StatusMsg");
	ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, nullptr, (LPARAM)(str2.IsEmpty() ? nullptr : str2.c_str()));

	// save descriptions in MyNotes
	db_set_ws(hContact, "UserInfo", "MyNotes", GetDisplay(&winfo, GetTextValue('N')));
	db_set_ws(hContact, WEATHERCONDITION, "WeatherInfo", GetDisplay(&winfo, GetTextValue('X')));

	// set the update tag
	setByte(hContact, "IsUpdated", TRUE);

	// save current condition for default station to be displayed after the update
	int old_status = m_iStatus;
	m_iStatus = iStatus;

	// a workaround for a default station that currently have an n/a icon assigned
	if (m_iStatus == ID_STATUS_OFFLINE)
		m_iStatus = ID_STATUS_CONNECTING;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

	// logging
	if (Ch) {
		// play the sound event
		Skin_PlaySound("weatherupdated");

		if (getByte(hContact, "File")) {
			// external log
			if (!getWString(hContact, "Log", &dbv)) {
				// for the option for overwriting the file, delete old file first
				if (getByte(hContact, "Overwrite"))
					DeleteFile(dbv.pwszVal);

				// open the file and set point to the end of file
				FILE *file = _wfopen(dbv.pwszVal, L"a");
				db_free(&dbv);
				if (file != nullptr) {
					// write data to the file and close
					fputws(GetDisplay(&winfo, GetTextValue('E')), file);
					fclose(file);
				}
			}
		}

		if (getByte(hContact, "History")) {
			// internal log using history
			T2Utf szMessage(GetDisplay(&winfo, GetTextValue('H')));

			DBEVENTINFO dbei = {};
			dbei.szModule = m_szModuleName;
			dbei.iTimestamp = (uint32_t)time(0);
			dbei.flags = DBEF_READ | DBEF_UTF;
			dbei.eventType = EVENTTYPE_MESSAGE;
			dbei.pBlob = szMessage;
			dbei.cbBlob = (uint32_t)mir_strlen(szMessage) + 1;
			db_event_add(hContact, &dbei);
		}

		// show the popup
		WeatherPopup(hContact, Ch);
	}

	Netlib_LogfW(m_hNetlibUser, L"Update Completed - Start to free memory");
	Netlib_LogfW(m_hNetlibUser, L"<-- Update successful for station -->");

	// Update frame data
	UpdateMwinData(hContact);

	// update brief info if its opened
	HWND hMoreDataDlg = WindowList_Find(hDataWindowList, hContact);
	if (hMoreDataDlg != nullptr)
		PostMessage(hMoreDataDlg, WM_UPDATEDATA, 0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// a linked list queue for updating weather station
// this function add a weather contact to the end of queue for update
// hContact = current contact

void CWeatherProto::UpdateListAdd(MCONTACT hContact)
{
	mir_cslock lck(m_csUpdate);
	m_updateList.push_back(hContact);
}

// get the first item from the update queue and remove it from the queue
// return value = the contact for next update
MCONTACT CWeatherProto::UpdateGetFirst()
{
	mir_cslock lck(m_csUpdate);
	if (m_updateList.empty())
		return 0;

	auto it = m_updateList.begin();
	MCONTACT hContact = *it;
	m_updateList.erase(it);
	return hContact;
}

void CWeatherProto::DestroyUpdateList(void)
{
	mir_cslock lck(m_csUpdate);
	m_updateList.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////
// update all weather thread
// this thread update each weather station from the queue

void CWeatherProto::UpdateThread(void *)
{
	{	mir_cslock lck(m_csUpdate);
		if (m_bThreadRunning)
			return;

		m_bThreadRunning = true;	// prevent 2 instance of this thread running
	}

	// update weather by getting the first station from the queue until the queue is empty
	while (!m_updateList.empty() && !Miranda_IsTerminated())
		UpdateWeather(UpdateGetFirst());

	// exit the update thread
	m_bThreadRunning = false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// update all weather station
// AutoUpdate = true if it is from automatic update using timer
//				false if it is from update by clicking the main menu

void CWeatherProto::UpdateAll(BOOL AutoUpdate, BOOL RemoveData)
{
	// add all weather contact to the update queue list
	for (auto &hContact : AccContacts())
		if (!getByte(hContact, "AutoUpdate") || !AutoUpdate) {
			if (RemoveData)
				db_delete_module(hContact, WEATHERCONDITION);
			UpdateListAdd(hContact);
		}

	// if it is not updating, then start the update thread process
	// if it is updating, the stations just added to the queue will get updated by the already-running process
	if (!m_bThreadRunning)
		ForkThread(&CWeatherProto::UpdateThread);
}

/////////////////////////////////////////////////////////////////////////////////////////
// update a single station
// wParam = handle for the weather station that is going to be updated

INT_PTR CWeatherProto::UpdateSingleStation(WPARAM wParam, LPARAM)
{
	if (IsMyContact(wParam)) {
		// add the station to the end of the update queue	
		UpdateListAdd(wParam);

		// if it is not updating, then start the update thread process
		// if it is updating, the stations just added to the queue will get 
		// updated by the already-running process
		if (!m_bThreadRunning)
			ForkThread(&CWeatherProto::UpdateThread);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// update a single station with removing the old data
// wParam = handle for the weather station that is going to be updated

INT_PTR CWeatherProto::UpdateSingleRemove(WPARAM hContact, LPARAM)
{
	if (IsMyContact(hContact)) {
		// add the station to the end of the update queue, and also remove old data
		db_delete_module(hContact, WEATHERCONDITION);
		UpdateListAdd(hContact);

		// if it is not updating, then start the update thread process
		// if it is updating, the stations just added to the queue will get updated by the already-running process
		if (!m_bThreadRunning)
			ForkThread(&CWeatherProto::UpdateThread);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// the "Update All" menu item in main menu

INT_PTR CWeatherProto::UpdateAllInfo(WPARAM, LPARAM)
{
	if (!m_bThreadRunning)
		UpdateAll(FALSE, FALSE);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// the "Update All" menu item in main menu and remove the old data

INT_PTR CWeatherProto::UpdateAllRemove(WPARAM, LPARAM)
{
	if (!m_bThreadRunning)
		UpdateAll(FALSE, TRUE);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// getting weather data and save them into the database
// hContact = the contact to get the data

static wchar_t *moon2str(double phase)
{
	if (phase < 0.05) return TranslateT("New moon");
	if (phase < 0.26) return TranslateT("Waxing crescent");
	if (phase < 0.51) return TranslateT("Waxing gibbous");
	if (phase < 0.76) return TranslateT("Waning gibbous");
	return TranslateT("Waning crescent");
}

static CMStringW parseConditions(const CMStringW &str)
{
	CMStringW ret;
	int iStart = 0;
	while (true) {
		auto substr = str.Tokenize(L",", iStart);
		if (substr.IsEmpty())
			break;

		substr.Trim();
		if (!ret.IsEmpty())
			ret += ", ";
		ret += TranslateW(substr);
	}
	return ret;
}

static double g_elevation = 0;

static void getData(OBJLIST<WIDATAITEM> &arValues, const JSONNode &node)
{
	arValues.insert(new WIDATAITEM(LPGENW("Date"), L"", parseConditions(node["datetime"].as_mstring())));
	arValues.insert(new WIDATAITEM(LPGENW("Condition"), L"", parseConditions(node["conditions"].as_mstring())));
	arValues.insert(new WIDATAITEM(LPGENW("Temperature"), L"C", node["temp"].as_mstring()));
	arValues.insert(new WIDATAITEM(LPGENW("High"), L"C", node["tempmax"].as_mstring()));
	arValues.insert(new WIDATAITEM(LPGENW("Low"), L"C", node["tempmin"].as_mstring()));

	CMStringW wszPressure(FORMAT, L"%lf", node["pressure"].as_float() - g_elevation);
	arValues.insert(new WIDATAITEM(LPGENW("Pressure"), L"mb", wszPressure));

	arValues.insert(new WIDATAITEM(LPGENW("Sunset"), L"", node["sunset"].as_mstring()));
	arValues.insert(new WIDATAITEM(LPGENW("Sunrise"), L"", node["sunrise"].as_mstring()));
	arValues.insert(new WIDATAITEM(LPGENW("Moon phase"), L"", moon2str(node["moonphase"].as_float())));
	arValues.insert(new WIDATAITEM(LPGENW("Wind speed"), L"km/h", node["windspeed"].as_mstring()));
	arValues.insert(new WIDATAITEM(LPGENW("Wind direction"), L"grad", node["winddir"].as_mstring()));
	arValues.insert(new WIDATAITEM(LPGENW("Dew point"), L"C", node["dew"].as_mstring()));
	arValues.insert(new WIDATAITEM(LPGENW("Visibility"), L"km", node["visibility"].as_mstring()));
	arValues.insert(new WIDATAITEM(LPGENW("Humidity"), L"", node["humidity"].as_mstring()));
	arValues.insert(new WIDATAITEM(LPGENW("Feel"), L"C", node["feelslike"].as_mstring()));
}

int CWeatherProto::GetWeatherData(MCONTACT hContact)
{
	// get each part of the id's
	CMStringW wszID(getMStringW(hContact, "ID"));
	if (wszID.IsEmpty())
		return INVALID_ID;

	uint16_t cond = NA;

	// download the html file from the internet
	WeatherReply reply(RunQuery(wszID, 7));
	if (!reply)
		return reply.error();

	auto &root = reply.data();

	// writing current conditions
	auto &curr = root["currentConditions"];
	g_elevation = root["elevation"].as_float() / 7.877;

	OBJLIST<WIDATAITEM> arValues(20);
	getData(arValues, curr);

	auto szIcon = curr["icon"].as_string();
	if (szIcon == "snow")
		cond = SNOW;
	else if (szIcon == "snow-showers-day" || szIcon == "snow-showers-night")
		cond = SSHOWER;
	else if (szIcon == "thunder" || szIcon == "thunder-showers-day" || szIcon == "thunder-showers-night")
		cond = LIGHT;
	else if (szIcon == "partly-cloudy-day" || szIcon == "partly-cloudy-night" || szIcon == "wind")
		cond = PCLOUDY;
	else if (szIcon == "fog")
		cond = FOG;
	else if (szIcon == "rain")
		cond = RAIN;
	else if (szIcon == "showers-day" || szIcon == "showers-night")
		cond = RSHOWER;
	else if (szIcon == "clear-day" || szIcon == "clear-night")
		cond = SUNNY;
	else if (szIcon == "rain")
		cond = RAIN;
	else if (szIcon == "cloudy")
		cond = CLOUDY;

	// writing forecast
	int iFore = 0;
	db_set_ws(hContact, WEATHERCONDITION, "Update", curr["datetime"].as_mstring());

	for (auto &fore : root["days"]) {
		getData(arValues, fore);

		CMStringW result;
		for (auto &it : arValues) {
			ConvertDataValue(it);
			if (it->Value.IsEmpty())
				continue;

			if (iFore == 0)
				db_set_ws(hContact, WEATHERCONDITION, _T2A(it->Name), it->Value);

			if (!result.IsEmpty())
				result += L"; ";
			result.AppendFormat(L"%s: %s", TranslateW(it->Name), it->Value.c_str());
		}

		CMStringA szSetting(FORMAT, "Forecast Day %d", iFore++);
		db_set_ws(hContact, WEATHERCONDITION, szSetting, result);
		arValues.destroy();
	}

	// assign condition icon
	setWord(hContact, "StatusIcon", cond);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int enumSettings(const char *pszSetting, void *param)
{
	auto *pList = (OBJLIST<char>*)param;
	if (!pList->find((char*)pszSetting))
		pList->insert(newStr(pszSetting));
	return 0;
}

void CWeatherProto::GetVarsDescr(CMStringW &wszDescr)
{
	OBJLIST<char> vars(10, mir_strcmp);
	for (int i = 1; i <= 7; i++)
		vars.insert(newStr(CMStringA(FORMAT, "Forecast Day %d", i)));

	for (auto &cc : AccContacts())
		db_enum_settings(cc, &enumSettings, WEATHERCONDITION, &vars);

	CMStringW str;
	for (auto &it : vars) {
		if (!str.IsEmpty())
			str.Append(L", ");
		str.AppendFormat(L"%%[%S]", it);
	}
	wszDescr += str;
}

/////////////////////////////////////////////////////////////////////////////////////////
// main auto-update timer

void CWeatherProto::DoUpdate()
{
	// only run if it is not current updating and the auto update option is enabled
	if (!m_bThreadRunning && opt.CAutoUpdate && !Miranda_IsTerminated() && m_iStatus == ID_STATUS_ONLINE)
		UpdateAll(TRUE, FALSE);
}

// temporary timer for first run
// when this is run, it kill the old startup timer and create the permenant one above

void CWeatherProto::StartUpdate()
{
	m_bThreadRunning = false;

	if (!Miranda_IsTerminated())
		m_impl.m_update.Start(opt.UpdateTime * 60000);
}

void CWeatherProto::RestartTimer()
{
	m_impl.m_update.Stop();
	m_impl.m_update.Start(opt.UpdateTime * 60000);
}
