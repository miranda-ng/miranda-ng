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
This file contain the source related loading, obtaining, and
saving individual weather data for a weather contact.
*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// get station ID from DB
// hContact = the current contact handle
// return value = the string for station ID

void CWeatherProto::GetStationID(MCONTACT hContact, wchar_t *id, int idlen)
{
	// accessing the database
	if (db_get_wstatic(hContact, m_szModuleName, "ID", id, idlen))
		id[0] = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// initialize weather info by loading values from database
// hContact = current contact handle
// return value = the current weather information in WEATHERINFO struct

WEATHERINFO CWeatherProto::LoadWeatherInfo(MCONTACT hContact)
{
	// obtaining values from the DB
	// assuming station ID must exist at all time, but others does not have to
	// if the string is not found in database, a value of "N/A" is stored in the field
	WEATHERINFO winfo;
	winfo.hContact = hContact;
	GetStationID(hContact, winfo.id, _countof(winfo.id));

	if (db_get_wstatic(hContact, m_szModuleName, "Nick", winfo.city, _countof(winfo.city)))
		wcsncpy(winfo.city, NODATA, _countof(winfo.city) - 1);
	if (db_get_wstatic(hContact, WEATHERCONDITION, "Update", winfo.update, _countof(winfo.update)))
		wcsncpy(winfo.update, NODATA, _countof(winfo.update) - 1);
	if (db_get_wstatic(hContact, WEATHERCONDITION, "Condition", winfo.cond, _countof(winfo.cond)))
		wcsncpy(winfo.cond, NODATA, _countof(winfo.cond) - 1);
	if (db_get_wstatic(hContact, WEATHERCONDITION, "Temperature", winfo.temp, _countof(winfo.temp)))
		wcsncpy(winfo.temp, NODATA, _countof(winfo.temp) - 1);
	if (db_get_wstatic(hContact, WEATHERCONDITION, "High", winfo.high, _countof(winfo.high)))
		wcsncpy(winfo.high, NODATA, _countof(winfo.high) - 1);
	if (db_get_wstatic(hContact, WEATHERCONDITION, "Low", winfo.low, _countof(winfo.low)))
		wcsncpy(winfo.low, NODATA, _countof(winfo.low) - 1);
	if (db_get_wstatic(hContact, WEATHERCONDITION, "Sunset", winfo.sunset, _countof(winfo.sunset)))
		wcsncpy(winfo.sunset, NODATA, _countof(winfo.sunset) - 1);
	if (db_get_wstatic(hContact, WEATHERCONDITION, "Sunrise", winfo.sunrise, _countof(winfo.sunrise)))
		wcsncpy(winfo.sunrise, NODATA, _countof(winfo.sunrise) - 1);
	if (db_get_wstatic(hContact, WEATHERCONDITION, "Wind speed", winfo.wind, _countof(winfo.wind)))
		wcsncpy(winfo.wind, NODATA, _countof(winfo.wind) - 1);
	if (db_get_wstatic(hContact, WEATHERCONDITION, "Wind direction", winfo.winddir, _countof(winfo.winddir)))
		wcsncpy(winfo.winddir, NODATA, _countof(winfo.winddir) - 1);
	if (db_get_wstatic(hContact, WEATHERCONDITION, "Dew point", winfo.dewpoint, _countof(winfo.dewpoint)))
		wcsncpy(winfo.dewpoint, NODATA, _countof(winfo.dewpoint) - 1);
	if (db_get_wstatic(hContact, WEATHERCONDITION, "Pressure", winfo.pressure, _countof(winfo.pressure)))
		wcsncpy(winfo.pressure, NODATA, _countof(winfo.pressure) - 1);
	if (db_get_wstatic(hContact, WEATHERCONDITION, "Visibility", winfo.vis, _countof(winfo.vis)))
		wcsncpy(winfo.vis, NODATA, _countof(winfo.vis) - 1);
	if (db_get_wstatic(hContact, WEATHERCONDITION, "Humidity", winfo.humid, _countof(winfo.humid)))
		wcsncpy(winfo.humid, NODATA, _countof(winfo.humid) - 1);
	if (db_get_wstatic(hContact, WEATHERCONDITION, "Feel", winfo.feel, _countof(winfo.feel)))
		wcsncpy(winfo.feel, NODATA, _countof(winfo.feel) - 1);
	return winfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
// erase all current weather information from database
// lastver = the last used version number in dword (using PLUGIN_MAKE_VERSION)

void CWeatherProto::EraseAllInfo()
{
	wchar_t str[255];
	int ContactCount = 0;
	MCONTACT LastContact = NULL;
	DBVARIANT dbv;
	// loop through all contacts
	for (auto &hContact : AccContacts()) {
		setWord(hContact, "Status", ID_STATUS_OFFLINE);
		setWord(hContact, "StatusIcon", -1);
		db_unset(hContact, "CList", "MyHandle");
		// clear all data
		if (getWString(hContact, "Nick", &dbv)) {
			setWString(hContact, "Nick", TranslateT("<Enter city name here>"));
			setString(hContact, "LastLog", "never");
			setString(hContact, "LastCondition", "None");
			setString(hContact, "LastTemperature", "None");
		}
		else db_free(&dbv);

		db_delete_module(hContact, WEATHERCONDITION);
		db_set_s(hContact, "UserInfo", "MyNotes", "");
		// reset update tag
		setByte(hContact, "IsUpdated", FALSE);
		// reset logging settings
		if (!getWString(hContact, "Log", &dbv)) {
			setByte(hContact, "File", (uint8_t)(dbv.pwszVal[0] != 0));
			db_free(&dbv);
		}
		else setByte(hContact, "File", FALSE);

		// if no default station find, assign a new one
		if (opt.Default[0] == 0) {
			GetStationID(hContact, opt.Default, _countof(opt.Default));

			opt.DefStn = hContact;
			if (!getWString(hContact, "Nick", &dbv)) {
				mir_snwprintf(str, TranslateT("%s is now the default weather station"), dbv.pwszVal);
				db_free(&dbv);
				MessageBox(nullptr, str, TranslateT("Weather Protocol"), MB_OK | MB_ICONINFORMATION);
			}
		}
		// get the handle of the default station
		if (opt.DefStn == NULL) {
			if (!getWString(hContact, "ID", &dbv)) {
				if (!mir_wstrcmp(dbv.pwszVal, opt.Default))
					opt.DefStn = hContact;
				db_free(&dbv);
			}
		}
		ContactCount++;		// increment counter
		LastContact = hContact;
	}

	// if weather contact exists, set the status to online so it is ready for update
	// if (ContactCount != 0) status = ONLINE;
	// in case where the default station is missing
	if (opt.DefStn == NULL && ContactCount != 0) {
		if (!getWString(LastContact, "ID", &dbv)) {
			wcsncpy(opt.Default, dbv.pwszVal, _countof(opt.Default) - 1);
			db_free(&dbv);
		}
		opt.DefStn = LastContact;
		if (!getWString(LastContact, "Nick", &dbv)) {
			mir_snwprintf(str, TranslateT("%s is now the default weather station"), dbv.pwszVal);
			db_free(&dbv);
			MessageBox(nullptr, str, TranslateT("Weather Protocol"), MB_OK | MB_ICONINFORMATION);
		}
	}
	// save option in case of default station changed
	setWString("Default", opt.Default);
}

/////////////////////////////////////////////////////////////////////////////////////////

static wchar_t rumbs[][16] = { 
	LPGENW("N"), LPGENW("NNE"), LPGENW("NE"), LPGENW("ENE"),
	LPGENW("E"), LPGENW("ESE"), LPGENW("ES"), LPGENW("SSE"),
	LPGENW("S"), LPGENW("SSW"), LPGENW("SW"), LPGENW("WSW"),
	LPGENW("W"), LPGENW("WNW"), LPGENW("WN"), LPGENW("NNW")
};

static wchar_t *degree2str(double angle)
{
	double a = 11.25;

	for (int i = 0; i < _countof(rumbs); i++, a += 22.5)
		if (angle < a)
			return TranslateW(rumbs[i]);

	// area between 348.75 & 360 degrees
	return TranslateT("N");
}

void CWeatherProto::ConvertDataValue(WIDATAITEM *p)
{
	wchar_t str[MAX_DATA_LEN];

	// temperature
	if (!mir_wstrcmp(p->Name, L"Temperature") || !mir_wstrcmp(p->Name, L"High") ||
		!mir_wstrcmp(p->Name, L"Low") || !mir_wstrcmp(p->Name, L"Feel") ||
		!mir_wstrcmp(p->Name, L"Dew point") ||
		!mir_wstrcmpi(p->Unit, L"C") || !mir_wstrcmpi(p->Unit, L"F") ||
		!mir_wstrcmpi(p->Unit, L"K")) {
		GetTemp(p->Value, p->Unit, str);
		p->Value = str;
	}
	// pressure
	else if (!mir_wstrcmp(p->Name, L"Pressure") || !mir_wstrcmpi(p->Unit, L"HPA") ||
		!mir_wstrcmpi(p->Unit, L"KPA") || !mir_wstrcmpi(p->Unit, L"MB") ||
		!mir_wstrcmpi(p->Unit, L"TORR") || !mir_wstrcmpi(p->Unit, L"IN") ||
		!mir_wstrcmpi(p->Unit, L"MM")) {
		GetPressure(p->Value, p->Unit, str);
		p->Value = str;
	}
	// speed
	else if (!mir_wstrcmp(p->Name, L"Wind Speed") || !mir_wstrcmpi(p->Unit, L"KM/H") ||
		!mir_wstrcmpi(p->Unit, L"M/S") || !mir_wstrcmpi(p->Unit, L"MPH") ||
		!mir_wstrcmpi(p->Unit, L"KNOTS")) {
		GetSpeed(p->Value, p->Unit, str);
		p->Value = str;
	}
	// visibility
	else if (!mir_wstrcmp(p->Name, L"Visibility") || !mir_wstrcmpi(p->Unit, L"KM") ||
		!mir_wstrcmpi(p->Unit, L"MILES")) {
		GetDist(p->Value, p->Unit, str);
		p->Value = str;
	}
	// elevation
	else if (!mir_wstrcmp(p->Name, L"Elevation") || !mir_wstrcmpi(p->Unit, L"FT") ||
		!mir_wstrcmpi(p->Unit, L"M")) {
		GetElev(p->Value, p->Unit, str);
		p->Value = str;
	}
	// convert degrees to compass
	else if (!mir_wstrcmpi(p->Unit, L"GRAD")) {
		p->Value = degree2str(_wtof(p->Value));
	}
	// degree sign
	else if (!mir_wstrcmpi(p->Unit, L"DEG")) {
		if (!opt.DoNotAppendUnit)
			p->Value.Append(opt.DegreeSign);
	}
	// percent sign
	else if (!mir_wstrcmpi(p->Unit, L"%")) {
		if (!opt.DoNotAppendUnit)
			p->Value.Append(L"%");
	}
	// truncating strings for day/month to 2 or 3 characters
	else if (!mir_wstrcmpi(p->Unit, L"DAY") || !mir_wstrcmpi(p->Unit, L"MONTH"))
		if (opt.dUnit > 1 && mir_wstrlen(p->Value) > opt.dUnit)
			p->Value.SetAt(opt.dUnit, '\0');
}

/////////////////////////////////////////////////////////////////////////////////////////
// data query

MHttpResponse* CWeatherProto::RunQuery(const wchar_t *id, int days)
{
	wchar_t *pKey = m_szApiKey;
	if (!mir_wstrlen(pKey)) {
		WPShowMessage(TranslateT("You need to obtain the personal key and enter it in the account's Options dialog"), SM_WARNING);
		return nullptr;
	}

	auto *pReq = new MHttpRequest(REQUEST_GET);
	pReq->flags = NLHRF_HTTP11 | NLHRF_DUMPASTEXT;
	pReq->m_szUrl = "https://weather.visualcrossing.com/VisualCrossingWebServices/rest/services/timeline/" + mir_urlEncode(T2Utf(id).get());

	if (days) {
		time_t today = time(0);
		struct tm *p = localtime(&today);
		pReq->m_szUrl.AppendFormat("/%04d-%02d-%02d", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday);

		today += 86400 * 7; // add one week
		p = localtime(&today);
		pReq->m_szUrl.AppendFormat("/%04d-%02d-%02d", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday);
	}

	pReq << CHAR_PARAM("unitGroup", "metric") << WCHAR_PARAM("key", pKey) << CHAR_PARAM("contentType", "json");
	if (days)
		pReq << CHAR_PARAM("elements", "+elevation");

	auto *ret = Netlib_HttpTransaction(m_hNetlibUser, pReq);
	delete pReq;
	return ret;
}
