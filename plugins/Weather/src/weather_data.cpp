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

/*
This file contain the source related loading, obtaining, and
saving individual weather data for a weather contact.
*/

#include "weather.h"

//============  LOAD WEATHER INFO FROM A CONTACT  ============
// get station ID from DB
// hContact = the current contact handle
// return value = the string for station ID

void GetStationID(MCONTACT hContact, TCHAR* id, size_t idlen)
{
	// accessing the database
	if (DBGetStaticString(hContact, WEATHERPROTONAME, "ID", id, idlen))
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
	GetStationID(hContact, winfo.id, SIZEOF(winfo.id));

	if (DBGetStaticString(hContact, WEATHERPROTONAME, "Nick", winfo.city, SIZEOF(winfo.city)))
		_tcsncpy(winfo.city, NODATA, SIZEOF(winfo.city) - 1);
	if (DBGetStaticString(hContact, WEATHERCONDITION, "Update", winfo.update, SIZEOF(winfo.update)))
		_tcsncpy(winfo.update, NODATA, SIZEOF(winfo.update) - 1);
	if (DBGetStaticString(hContact, WEATHERCONDITION, "Condition", winfo.cond, SIZEOF(winfo.cond)))
		_tcsncpy(winfo.cond, NODATA, SIZEOF(winfo.cond) - 1);
	if (DBGetStaticString(hContact, WEATHERCONDITION, "Temperature", winfo.temp, SIZEOF(winfo.temp)))
		_tcsncpy(winfo.temp, NODATA, SIZEOF(winfo.temp) - 1);
	if (DBGetStaticString(hContact, WEATHERCONDITION, "High", winfo.high, SIZEOF(winfo.high)))
		_tcsncpy(winfo.high, NODATA, SIZEOF(winfo.high) - 1);
	if (DBGetStaticString(hContact, WEATHERCONDITION, "Low", winfo.low, SIZEOF(winfo.low)))
		_tcsncpy(winfo.low, NODATA, SIZEOF(winfo.low) - 1);
	if (DBGetStaticString(hContact, WEATHERCONDITION, "Sunset", winfo.sunset, SIZEOF(winfo.sunset)))
		_tcsncpy(winfo.sunset, NODATA, SIZEOF(winfo.sunset) - 1);
	if (DBGetStaticString(hContact, WEATHERCONDITION, "Sunrise", winfo.sunrise, SIZEOF(winfo.sunrise)))
		_tcsncpy(winfo.sunrise, NODATA, SIZEOF(winfo.sunrise) - 1);
	if (DBGetStaticString(hContact, WEATHERCONDITION, "Wind Speed", winfo.wind, SIZEOF(winfo.wind)))
		_tcsncpy(winfo.wind, NODATA, SIZEOF(winfo.wind) - 1);
	if (DBGetStaticString(hContact, WEATHERCONDITION, "Wind Direction", winfo.winddir, SIZEOF(winfo.winddir)))
		_tcsncpy(winfo.winddir, NODATA, SIZEOF(winfo.winddir) - 1);
	if (DBGetStaticString(hContact, WEATHERCONDITION, "Dewpoint", winfo.dewpoint, SIZEOF(winfo.dewpoint)))
		_tcsncpy(winfo.dewpoint, NODATA, SIZEOF(winfo.dewpoint) - 1);
	if (DBGetStaticString(hContact, WEATHERCONDITION, "Pressure", winfo.pressure, SIZEOF(winfo.pressure)))
		_tcsncpy(winfo.pressure, NODATA, SIZEOF(winfo.pressure) - 1);
	if (DBGetStaticString(hContact, WEATHERCONDITION, "Visibility", winfo.vis, SIZEOF(winfo.vis)))
		_tcsncpy(winfo.vis, NODATA, SIZEOF(winfo.vis) - 1);
	if (DBGetStaticString(hContact, WEATHERCONDITION, "Humidity", winfo.humid, SIZEOF(winfo.humid)))
		_tcsncpy(winfo.humid, NODATA, SIZEOF(winfo.humid) - 1);
	if (DBGetStaticString(hContact, WEATHERCONDITION, "Feel", winfo.feel, SIZEOF(winfo.feel)))
		_tcsncpy(winfo.feel, NODATA, SIZEOF(winfo.feel) - 1);

	winfo.status = (WORD)db_get_w(hContact, WEATHERPROTONAME, "StatusIcon", ID_STATUS_OFFLINE);
	return winfo;
}

// getting weather setting from database
// return 0 on success
int DBGetData(MCONTACT hContact, char *setting, DBVARIANT *dbv) 
{
	if ( db_get_ts(hContact, WEATHERCONDITION, setting, dbv)) {
		size_t len = mir_strlen(setting) + 1;
		char *set = (char*)alloca(len + 1);
		*set = '#';
		memcpy(set + 1, setting, len);

		if ( db_get_ts(hContact, WEATHERCONDITION, set, dbv))
			return 1;
	}
	return 0;
}

int DBGetStaticString(MCONTACT hContact, const char *szModule, const char *valueName, TCHAR *dest, size_t dest_len)
{
	DBVARIANT dbv;
	if ( db_get_ts( hContact, szModule, valueName, &dbv ))
		return 1;

	_tcsncpy( dest, dbv.ptszVal, dest_len );
	dest[ dest_len-1 ] = 0;
	db_free( &dbv );
	return 0;
}


//============  ERASE OLD SETTINGS  ============

// erase all current weather information from database
// lastver = the last used version number in dword (using PLUGIN_MAKE_VERSION)
void EraseAllInfo() 
{
	TCHAR str[255];
	int ContactCount = 0;
	MCONTACT LastContact = NULL;
	DBVARIANT dbv;
	// loop through all contacts
	for (MCONTACT hContact = db_find_first(WEATHERPROTONAME); hContact; hContact = db_find_next(hContact, WEATHERPROTONAME)) {
		db_set_w(hContact,WEATHERPROTONAME, "Status",ID_STATUS_OFFLINE);
		db_set_w(hContact,WEATHERPROTONAME, "StatusIcon",ID_STATUS_OFFLINE);
		db_unset(hContact, "CList", "MyHandle");
		// clear all data
		if ( db_get_ts(hContact, WEATHERPROTONAME, "Nick", &dbv)) {
			db_set_ts(hContact, WEATHERPROTONAME, "Nick", TranslateT("<Enter city name here>"));
			db_set_s(hContact, WEATHERPROTONAME, "LastLog", "never");
			db_set_s(hContact, WEATHERPROTONAME, "LastCondition", "None");
			db_set_s(hContact, WEATHERPROTONAME, "LastTemperature", "None");
		}
		else db_free(&dbv);

		DBDataManage(hContact, WDBM_REMOVE, 0, 0);
		db_set_s(hContact, "UserInfo", "MyNotes", "");
		// reset update tag
		db_set_b(hContact,WEATHERPROTONAME, "IsUpdated",FALSE);
		// reset logging settings
		if ( !db_get_ts(hContact,WEATHERPROTONAME, "Log", &dbv)) {
			db_set_b(hContact,WEATHERPROTONAME, "File",(BYTE)(dbv.ptszVal[0] != 0));
			db_free(&dbv);
		}
		else db_set_b(hContact,WEATHERPROTONAME, "File",FALSE);

		// if no default station find, assign a new one
		if (opt.Default[0] == 0) {
			GetStationID(hContact, opt.Default, SIZEOF(opt.Default));

			opt.DefStn = hContact;
			if ( !db_get_ts(hContact,WEATHERPROTONAME, "Nick",&dbv)) {
				mir_sntprintf(str, SIZEOF(str), TranslateT("%s is now the default weather station"), dbv.ptszVal);
				db_free(&dbv);
				MessageBox(NULL, str, TranslateT("Weather Protocol"), MB_OK|MB_ICONINFORMATION);
			}
		}
		// get the handle of the default station
		if (opt.DefStn == NULL) {
			if ( !db_get_ts(hContact,WEATHERPROTONAME, "ID",&dbv)) {
				if ( !mir_tstrcmp(dbv.ptszVal, opt.Default))
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
		if ( !db_get_ts(LastContact, WEATHERPROTONAME, "ID", &dbv)) {
			_tcsncpy(opt.Default, dbv.ptszVal, SIZEOF(opt.Default) - 1);
			db_free(&dbv);
		}
		opt.DefStn = LastContact;
		if ( !db_get_ts(LastContact,WEATHERPROTONAME, "Nick",&dbv)) {
			mir_sntprintf(str, SIZEOF(str), TranslateT("%s is now the default weather station"), dbv.ptszVal);
			db_free(&dbv);
			MessageBox(NULL, str, TranslateT("Weather Protocol"), MB_OK|MB_ICONINFORMATION);
		}
	}
	// save option in case of default station changed
	db_set_ts(NULL, WEATHERPROTONAME, "Default", opt.Default);
}

void ConvertDataValue(WIDATAITEM *UpdateData, TCHAR *Data) 
{
	TCHAR str[MAX_DATA_LEN];

	// convert the unit
	if ( mir_tstrcmp(Data, TranslateT("<Error>")) && mir_tstrcmp(Data, NODATA) && mir_tstrcmp(Data, TranslateTS(NODATA))) {
		// temperature
		if ( !mir_tstrcmp(UpdateData->Name, _T("Temperature")) || !mir_tstrcmp(UpdateData->Name, _T("High")) || 
			!mir_tstrcmp(UpdateData->Name, _T("Low")) || !mir_tstrcmp(UpdateData->Name, _T("Feel")) || 
			!mir_tstrcmp(UpdateData->Name, _T("Dewpoint")) ||
			!_tcsicmp(UpdateData->Unit, _T("C")) || !_tcsicmp(UpdateData->Unit, _T("F")) || 
			!_tcsicmp(UpdateData->Unit, _T("K")))
		{
			GetTemp(Data, UpdateData->Unit, str);
			_tcscpy(Data, str);
		}
		// pressure
		else if ( !mir_tstrcmp(UpdateData->Name, _T("Pressure")) || !_tcsicmp(UpdateData->Unit, _T("HPA")) || 
			!_tcsicmp(UpdateData->Unit, _T("KPA")) || !_tcsicmp(UpdateData->Unit, _T("MB")) ||
			!_tcsicmp(UpdateData->Unit, _T("TORR")) || !_tcsicmp(UpdateData->Unit, _T("IN")) || 
			!_tcsicmp(UpdateData->Unit, _T("MM")))
		{
			GetPressure(Data, UpdateData->Unit, str);
			_tcscpy(Data, str);
		}
		// speed
		else if ( !mir_tstrcmp(UpdateData->Name, _T("Wind Speed")) || !_tcsicmp(UpdateData->Unit, _T("KM/H")) || 
			!_tcsicmp(UpdateData->Unit, _T("M/S")) || !_tcsicmp(UpdateData->Unit, _T("MPH")) || 
			!_tcsicmp(UpdateData->Unit, _T("KNOTS")))
		{
			GetSpeed(Data, UpdateData->Unit, str);
			_tcscpy(Data, str);
		}
		// visibility
		else if ( !mir_tstrcmp(UpdateData->Name, _T("Visibility")) || !_tcsicmp(UpdateData->Unit, _T("KM")) || 
			!_tcsicmp(UpdateData->Unit, _T("MILES")))
		{
			GetDist(Data, UpdateData->Unit, str);
			_tcscpy(Data, str);
		}
		// elevation
		else if ( !mir_tstrcmp(UpdateData->Name, _T("Elevation")) || !_tcsicmp(UpdateData->Unit, _T("FT")) || 
			!_tcsicmp(UpdateData->Unit, _T("M")))
		{
			GetElev(Data, UpdateData->Unit, str);
			_tcscpy(Data, str);
		}
		// converting case for condition to the upper+lower format
		else if ( !_tcsicmp(UpdateData->Unit, _T("COND")))
			CaseConv(Data);
		// degree sign
		else if ( !_tcsicmp(UpdateData->Unit, _T("DEG")))
		{
			if ( !opt.DoNotAppendUnit) _tcscat(Data, opt.DegreeSign);
		}
		// percent sign
		else if ( !_tcsicmp(UpdateData->Unit, _T("%")))
		{
			if ( !opt.DoNotAppendUnit) _tcscat(Data, _T("%"));
		}
		// truncating strings for day/month to 2 or 3 characters
		else if ( !_tcsicmp(UpdateData->Unit, _T("DAY")) || !_tcsicmp(UpdateData->Unit, _T("MONTH")))
			if (opt.dUnit > 1 && mir_tstrlen(Data) > opt.dUnit)
				Data[opt.dUnit] = '\0';
	}
}

//============  GET THE VALUE OF A DATAITEM  ============

// get the value of the data using the start, end strings
// UpdateData = the WIDATAITEM struct containing start, end, unit
// Data = the string containing weather data obtained from UpdateData
// global var. used: szInfo = the downloaded string

void GetDataValue(WIDATAITEM *UpdateData, TCHAR *Data, TCHAR** szData) 
{
	TCHAR last = 0, current, *start, *end;
	unsigned startloc = 0, endloc = 0, respos = 0;
	BOOL tag = FALSE, symb = FALSE;
	TCHAR *szInfo = *szData;

	Data[0] = 0;
	// parse the data if available
	if (UpdateData->Start[0] == 0 && UpdateData->End[0] == 0) return;
	start = szInfo;
	// the start string must be found
	if (UpdateData->Start[0] != 0) {
		start = _tcsstr(szInfo, UpdateData->Start);
		if (start != NULL) {
			// set the starting location for getting data
			start += mir_tstrlen(UpdateData->Start);
			szInfo = start;
		}
	}

	// the end string must be found too
	if (UpdateData->End[0] != 0)
		end = _tcsstr(szInfo, UpdateData->End);
	else 
		end = _tcsstr(szInfo, _T(" "));

	if (end != NULL) {
		// set the ending location
		startloc = 0;
		endloc = end - szInfo;
		end += mir_tstrlen(UpdateData->End);
		last = '\n';
	}
	
	// ignore if not both of the string found - this prevent crashes
	if (start != NULL && end != NULL) {
		// begin reading the data from start location to end location
		// remove all HTML tag in between, as well as leading space, ending space,
		// multiple spaces, tabs, and return key
		while (startloc < endloc) {
			if (szInfo[startloc] == '<')	tag = TRUE;
			else if (szInfo[startloc] == '&' &&
				(szInfo[startloc+1] == ';' || szInfo[startloc+2] == ';' || szInfo[startloc+3] == ';' || 
				szInfo[startloc+4] == ';' || szInfo[startloc+5] == ';' || szInfo[startloc+6] == ';'))
			{
				// ...but do NOT strip &minus;
				if ((endloc - startloc) > 7 &&_tcsncmp(szInfo + startloc, _T("&minus;"), 7) == 0)
				{
					Data[respos++] = '-';
					startloc += 7;
					continue;
				}
				symb = TRUE;
			}
			else if (szInfo[startloc] == '>')	tag = FALSE;
			else if (szInfo[startloc] == ';')	symb = FALSE;
			else {
				if ( !tag && !symb) {
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
				if (opt.ShowWarnings && UpdateData->Name[0] != 0 && mir_tstrcmp(UpdateData->Name, _T("Ignore"))) {
					mir_sntprintf(Data, MAX_DATA_LEN, TranslateT("Error when obtaining data: %s"), UpdateData->Name);
					WPShowMessage(Data, SM_WARNING);
				}
				_tcsncpy(Data, TranslateT("<Error>"), MAX_DATA_LEN);
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

// copy a string into a new memory location
// Data = the field the data is copied to
// Value = the original string, the string where data is copied from
void wSetData(char **Data, const char *Value)
{
	if (Value[0] != 0) {
		char *newData = (char*)mir_alloc(mir_strlen(Value) + 3);
		strcpy(newData, Value);
		*Data = newData;
	}
	else *Data = "";
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
	*Data = NULL;
}

void wfree(WCHAR **Data) 
{
	if (*Data && wcslen(*Data) > 0)
		mir_free(*Data);
	*Data = NULL;
}

//============ MANAGE THE ITEMS STORED IN DB ============
// get single setting that is found
// szSetting = the setting name
// lparam = the counter
int GetWeatherDataFromDB(const char *szSetting, LPARAM lparam)
{
	LIST<char> *pList = (LIST<char>*)lparam;
	pList->insert(mir_strdup(szSetting));
	return 0;
}

// remove or display the weather information for a contact
// hContact - the contact in which the info is going to be removed

void DBDataManage(MCONTACT hContact, WORD Mode, WPARAM wParam, LPARAM lParam)
{
	LIST<char> arSettings(10);

	// get all the settings and stored them in a temporary list
	DBCONTACTENUMSETTINGS dbces;
	dbces.lParam = (LPARAM)&arSettings;
	dbces.pfnEnumProc = GetWeatherDataFromDB;
	dbces.szModule = WEATHERCONDITION;
	CallService(MS_DB_CONTACT_ENUMSETTINGS, hContact, (LPARAM)&dbces);

	// begin deleting settings
	for (int i = arSettings.getCount() - 1; i >= 0; i--) {
		char *szSetting = arSettings[i];

		DBVARIANT dbv;
		if (!db_get_ts(hContact, WEATHERCONDITION, szSetting, &dbv)) {
			switch (Mode) {
			case WDBM_REMOVE:
				db_unset(hContact, WEATHERCONDITION, szSetting);
				break;

			case WDBM_DETAILDISPLAY:
				// skip the "WeatherInfo" variable
				if (!mir_strcmp(szSetting, "WeatherInfo") || !mir_strcmp(szSetting, "Ignore") || szSetting[0] == '#')
					continue;

				HWND hList = GetDlgItem((HWND)wParam, IDC_DATALIST);
				LV_ITEM lvi = { 0 };
				lvi.mask = LVIF_TEXT | LVIF_PARAM;
				lvi.lParam = i;
				lvi.pszText = TranslateTS(_A2T(szSetting));
				lvi.iItem = ListView_InsertItem(hList, &lvi);
				lvi.pszText = dbv.ptszVal;
				ListView_SetItemText(hList, lvi.iItem, 1, dbv.ptszVal);
				break;
			}
			db_free(&dbv);
		}
		mir_free(szSetting);
	}
}
