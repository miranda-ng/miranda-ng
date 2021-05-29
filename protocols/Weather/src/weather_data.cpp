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

//============  LOAD WEATHER INFO FROM A CONTACT  ============
// get station ID from DB
// hContact = the current contact handle
// return value = the string for station ID
//
void GetStationID(MCONTACT hContact, wchar_t *id, int idlen)
{
	// accessing the database
	if (db_get_wstatic(hContact, MODULENAME, "ID", id, idlen))
		id[0] = 0;
}

// initialize weather info by loading values from database
// hContact = current contact handle
// return value = the current weather information in WEATHERINFO struct
WEATHERINFO LoadWeatherInfo(MCONTACT hContact)
{
	// obtaining values from the DB
	// assuming station ID must exist at all time, but others does not have to
	// if the string is not found in database, a value of "N/A" is stored in the field
	WEATHERINFO winfo;
	winfo.hContact = hContact;
	GetStationID(hContact, winfo.id, _countof(winfo.id));

	if (db_get_wstatic(hContact, MODULENAME, "Nick", winfo.city, _countof(winfo.city)))
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

// getting weather setting from database
// return 0 on success
int DBGetData(MCONTACT hContact, char *setting, DBVARIANT *dbv)
{
	if (db_get_ws(hContact, WEATHERCONDITION, setting, dbv)) {
		size_t len = mir_strlen(setting) + 1;
		char *set = (char*)alloca(len + 1);
		*set = '#';
		memcpy(set + 1, setting, len);

		if (db_get_ws(hContact, WEATHERCONDITION, set, dbv))
			return 1;
	}
	return 0;
}


//============  ERASE OLD SETTINGS  ============
//
// erase all current weather information from database
// lastver = the last used version number in dword (using PLUGIN_MAKE_VERSION)
void EraseAllInfo()
{
	wchar_t str[255];
	int ContactCount = 0;
	MCONTACT LastContact = NULL;
	DBVARIANT dbv;
	// loop through all contacts
	for (auto &hContact : Contacts(MODULENAME)) {
		g_plugin.setWord(hContact, "Status", ID_STATUS_OFFLINE);
		g_plugin.setWord(hContact, "StatusIcon", -1);
		db_unset(hContact, "CList", "MyHandle");
		// clear all data
		if (g_plugin.getWString(hContact, "Nick", &dbv)) {
			g_plugin.setWString(hContact, "Nick", TranslateT("<Enter city name here>"));
			g_plugin.setString(hContact, "LastLog", "never");
			g_plugin.setString(hContact, "LastCondition", "None");
			g_plugin.setString(hContact, "LastTemperature", "None");
		}
		else db_free(&dbv);

		DBDataManage(hContact, WDBM_REMOVE, 0, 0);
		db_set_s(hContact, "UserInfo", "MyNotes", "");
		// reset update tag
		g_plugin.setByte(hContact, "IsUpdated", FALSE);
		// reset logging settings
		if (!g_plugin.getWString(hContact, "Log", &dbv)) {
			g_plugin.setByte(hContact, "File", (BYTE)(dbv.pwszVal[0] != 0));
			db_free(&dbv);
		}
		else g_plugin.setByte(hContact, "File", FALSE);

		// if no default station find, assign a new one
		if (opt.Default[0] == 0) {
			GetStationID(hContact, opt.Default, _countof(opt.Default));

			opt.DefStn = hContact;
			if (!g_plugin.getWString(hContact, "Nick", &dbv)) {
				mir_snwprintf(str, TranslateT("%s is now the default weather station"), dbv.pwszVal);
				db_free(&dbv);
				MessageBox(nullptr, str, TranslateT("Weather Protocol"), MB_OK | MB_ICONINFORMATION);
			}
		}
		// get the handle of the default station
		if (opt.DefStn == NULL) {
			if (!g_plugin.getWString(hContact, "ID", &dbv)) {
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
		if (!g_plugin.getWString(LastContact, "ID", &dbv)) {
			wcsncpy(opt.Default, dbv.pwszVal, _countof(opt.Default) - 1);
			db_free(&dbv);
		}
		opt.DefStn = LastContact;
		if (!g_plugin.getWString(LastContact, "Nick", &dbv)) {
			mir_snwprintf(str, TranslateT("%s is now the default weather station"), dbv.pwszVal);
			db_free(&dbv);
			MessageBox(nullptr, str, TranslateT("Weather Protocol"), MB_OK | MB_ICONINFORMATION);
		}
	}
	// save option in case of default station changed
	g_plugin.setWString("Default", opt.Default);
}

void ConvertDataValue(WIDATAITEM *UpdateData, wchar_t *Data)
{
	wchar_t str[MAX_DATA_LEN];

	// convert the unit
	if (mir_wstrcmp(Data, TranslateT("<Error>")) && mir_wstrcmp(Data, NODATA) && mir_wstrcmp(Data, TranslateW(NODATA))) {
		// temperature
		if (!mir_wstrcmp(UpdateData->Name, L"Temperature") || !mir_wstrcmp(UpdateData->Name, L"High") ||
			!mir_wstrcmp(UpdateData->Name, L"Low") || !mir_wstrcmp(UpdateData->Name, L"Feel") ||
			!mir_wstrcmp(UpdateData->Name, L"Dew point") ||
			!mir_wstrcmpi(UpdateData->Unit, L"C") || !mir_wstrcmpi(UpdateData->Unit, L"F") ||
			!mir_wstrcmpi(UpdateData->Unit, L"K")) {
			GetTemp(Data, UpdateData->Unit, str);
			mir_wstrcpy(Data, str);
		}
		// pressure
		else if (!mir_wstrcmp(UpdateData->Name, L"Pressure") || !mir_wstrcmpi(UpdateData->Unit, L"HPA") ||
			!mir_wstrcmpi(UpdateData->Unit, L"KPA") || !mir_wstrcmpi(UpdateData->Unit, L"MB") ||
			!mir_wstrcmpi(UpdateData->Unit, L"TORR") || !mir_wstrcmpi(UpdateData->Unit, L"IN") ||
			!mir_wstrcmpi(UpdateData->Unit, L"MM")) {
			GetPressure(Data, UpdateData->Unit, str);
			mir_wstrcpy(Data, str);
		}
		// speed
		else if (!mir_wstrcmp(UpdateData->Name, L"Wind Speed") || !mir_wstrcmpi(UpdateData->Unit, L"KM/H") ||
			!mir_wstrcmpi(UpdateData->Unit, L"M/S") || !mir_wstrcmpi(UpdateData->Unit, L"MPH") ||
			!mir_wstrcmpi(UpdateData->Unit, L"KNOTS")) {
			GetSpeed(Data, UpdateData->Unit, str);
			mir_wstrcpy(Data, str);
		}
		// visibility
		else if (!mir_wstrcmp(UpdateData->Name, L"Visibility") || !mir_wstrcmpi(UpdateData->Unit, L"KM") ||
			!mir_wstrcmpi(UpdateData->Unit, L"MILES")) {
			GetDist(Data, UpdateData->Unit, str);
			mir_wstrcpy(Data, str);
		}
		// elevation
		else if (!mir_wstrcmp(UpdateData->Name, L"Elevation") || !mir_wstrcmpi(UpdateData->Unit, L"FT") ||
			!mir_wstrcmpi(UpdateData->Unit, L"M")) {
			GetElev(Data, UpdateData->Unit, str);
			mir_wstrcpy(Data, str);
		}
		// converting case for condition to the upper+lower format
		else if (!mir_wstrcmpi(UpdateData->Unit, L"COND"))
			CaseConv(Data);
		// degree sign
		else if (!mir_wstrcmpi(UpdateData->Unit, L"DEG")) {
			if (!opt.DoNotAppendUnit) mir_wstrcat(Data, opt.DegreeSign);
		}
		// percent sign
		else if (!mir_wstrcmpi(UpdateData->Unit, L"%")) {
			if (!opt.DoNotAppendUnit) mir_wstrcat(Data, L"%");
		}
		// truncating strings for day/month to 2 or 3 characters
		else if (!mir_wstrcmpi(UpdateData->Unit, L"DAY") || !mir_wstrcmpi(UpdateData->Unit, L"MONTH"))
			if (opt.dUnit > 1 && mir_wstrlen(Data) > opt.dUnit)
				Data[opt.dUnit] = '\0';
	}
}

//============  GET THE VALUE OF A DATAITEM  ============
//
// get the value of the data using the start, end strings
// UpdateData = the WIDATAITEM struct containing start, end, unit
// Data = the string containing weather data obtained from UpdateData
// global var. used: szInfo = the downloaded string
//
void GetDataValue(WIDATAITEM *UpdateData, wchar_t *Data, wchar_t **szData)
{
	wchar_t last = 0, current, *start, *end;
	unsigned startloc = 0, endloc = 0, respos = 0;
	BOOL tag = FALSE, symb = FALSE;
	wchar_t *szInfo = *szData;

	Data[0] = 0;
	// parse the data if available
	if (UpdateData->Start[0] == 0 && UpdateData->End[0] == 0) return;
	start = szInfo;
	// the start string must be found
	if (UpdateData->Start[0] != 0) {
		start = wcsstr(szInfo, UpdateData->Start);
		if (start != nullptr) {
			// set the starting location for getting data
			start += mir_wstrlen(UpdateData->Start);
			szInfo = start;
		}
	}

	// the end string must be found too
	if (UpdateData->End[0] != 0)
		end = wcsstr(szInfo, UpdateData->End);
	else
		end = wcschr(szInfo, ' ');

	if (end != nullptr) {
		// set the ending location
		startloc = 0;
		endloc = end - szInfo;
		end += mir_wstrlen(UpdateData->End);
		last = '\n';
	}

	// ignore if not both of the string found - this prevent crashes
	if (start != nullptr && end != nullptr) {
		// begin reading the data from start location to end location
		// remove all HTML tag in between, as well as leading space, ending space,
		// multiple spaces, tabs, and return key
		while (startloc < endloc) {
			if (szInfo[startloc] == '<')	tag = TRUE;
			else if (szInfo[startloc] == '&' &&
				(szInfo[startloc + 1] == ';' || szInfo[startloc + 2] == ';' || szInfo[startloc + 3] == ';' ||
					szInfo[startloc + 4] == ';' || szInfo[startloc + 5] == ';' || szInfo[startloc + 6] == ';')) {
				// ...but do NOT strip &minus;
				if ((endloc - startloc) > 7 && wcsncmp(szInfo + startloc, L"&minus;", 7) == 0) {
					Data[respos++] = '-';
					startloc += 7;
					continue;
				}
				symb = TRUE;
			}
			else if (szInfo[startloc] == '>')	tag = FALSE;
			else if (szInfo[startloc] == ';')	symb = FALSE;
			else {
				if (!tag && !symb) {
					current = szInfo[startloc];
					if (current == '\n' || current == '\t' || current == '	' || current == '\r')
						current = ' ';
					if (current != ' ' || last != ' ') {
						if (last != '\n' && (respos != 0 || (respos == 0 && last != ' ')))
							Data[respos++] = last;
						last = current;
					}
				}
			}
			++startloc;
			// prevent crashes if the string go over maximun length -> generate an error
			if (respos >= MAX_DATA_LEN) {
				if (opt.ShowWarnings && UpdateData->Name[0] != 0 && mir_wstrcmp(UpdateData->Name, L"Ignore")) {
					mir_snwprintf(Data, MAX_DATA_LEN, TranslateT("Error when obtaining data: %s"), UpdateData->Name);
					WPShowMessage(Data, SM_WARNING);
				}
				wcsncpy(Data, TranslateT("<Error>"), MAX_DATA_LEN);
				last = ' ';
				respos = MAX_DATA_LEN - 1;
				break;
			}
		}

		// get the last character
		if (last != ' ')
			Data[respos++] = last;

		// null terminate the string
		Data[respos] = 0;

		// convert the unit
		ConvertDataValue(UpdateData, Data);

		// remove the string before the data from szInfo
		szInfo = end;
	}
	*szData = szInfo;
}

//============ ALLOCATE SPACE AND COPY STRING ============
//
// copy a string into a new memory location
// Data = the field the data is copied to
// Value = the original string, the string where data is copied from
void wSetData(char **Data, const char *Value)
{
	if (Value[0] != 0)
		*Data = mir_strdup(Value);
	else
		*Data = "";
}

void wSetData(WCHAR **Data, const char *Value)
{
	if (Value[0] != 0)
		*Data = mir_a2u(Value);
	else
		*Data = L"";
}

void wSetData(WCHAR **Data, const WCHAR *Value)
{
	if (Value[0] != 0)
		*Data = mir_wstrdup(Value);
	else
		*Data = L"";
}

// A safer free function that free memory for a string
// Data = the string occuping the data to be freed
void wfree(char **Data)
{
	if (*Data && mir_strlen(*Data) > 0)
		mir_free(*Data);
	*Data = nullptr;
}

void wfree(WCHAR **Data)
{
	if (*Data && mir_wstrlen(*Data) > 0)
		mir_free(*Data);
	*Data = nullptr;
}

//============ MANAGE THE ITEMS STORED IN DB ============
// get single setting that is found
// szSetting = the setting name
// lparam = the counter
int GetWeatherDataFromDB(const char *szSetting, void *lparam)
{
	LIST<char> *pList = (LIST<char>*)lparam;
	pList->insert(mir_strdup(szSetting));
	return 0;
}

// remove or display the weather information for a contact
// hContact - the contact in which the info is going to be removed
//
void DBDataManage(MCONTACT hContact, WORD Mode, WPARAM wParam, LPARAM)
{
	// get all the settings and store them in a temporary list
	LIST<char> arSettings(10);
	db_enum_settings(hContact, GetWeatherDataFromDB, WEATHERCONDITION, &arSettings);

	// begin deleting settings
	auto T = arSettings.rev_iter();
	for (auto &str : T) {
		ptrW wszText(db_get_wsa(hContact, WEATHERCONDITION, str));
		if (wszText == nullptr)
			continue;

		switch (Mode) {
		case WDBM_REMOVE:
			db_unset(hContact, WEATHERCONDITION, str);
			break;

		case WDBM_DETAILDISPLAY:
			// skip the "WeatherInfo" variable
			if (!mir_strcmp(str, "WeatherInfo") || !mir_strcmp(str, "Ignore") || str[0] == '#')
				continue;

			_A2T strW(str);
			HWND hList = GetDlgItem((HWND)wParam, IDC_DATALIST);
			LV_ITEM lvi = {};
			lvi.mask = LVIF_TEXT | LVIF_PARAM;
			lvi.lParam = T.indexOf(&str);
			lvi.pszText = TranslateW(strW);
			lvi.iItem = ListView_InsertItem(hList, &lvi);
			lvi.pszText = wszText;
			ListView_SetItemText(hList, lvi.iItem, 1, wszText);
			break;
		}
		mir_free(str);
	}
}
