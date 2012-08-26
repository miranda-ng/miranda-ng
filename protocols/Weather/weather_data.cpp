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

/* 
This file contain the source related loading, obtaining, and
saving individual weather data for a weather contact.
*/

#include "weather.h"

// temporary counter and storage for removing weather data
typedef struct 
{
	char *value[1024];
	int current;
} WCOUNTER;

//============  LOAD WEATHER INFO FROM A CONTACT  ============

// get station ID from DB
// hContact = the current contact handle
// return value = the string for station ID
void GetStationID(HANDLE hContact, TCHAR* id, size_t idlen) 
{
	// accessing the database
	if (DBGetStaticString(hContact, WEATHERPROTONAME, "ID", id, idlen))
		id[0] = 0;
}

// initialize weather info by loading values from database
// Change = current contact handle
// return value = the current weather information in WEATHERINFO struct
WEATHERINFO LoadWeatherInfo(HANDLE Change) 
{
	WEATHERINFO winfo;

	winfo.hContact = Change;
	// obtaining values from the DB
	// assuming station ID must exist at all time, but others does not have to
	// if the string is not found in database, a value of "N/A" is stored in the field
	GetStationID(Change, winfo.id, SIZEOF(winfo.id));

	if (DBGetStaticString(Change, WEATHERPROTONAME, "Nick", winfo.city, SIZEOF(winfo.city)))
		_tcscpy(winfo.city, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Update", winfo.update, SIZEOF(winfo.update)))
		_tcscpy(winfo.update, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Condition", winfo.cond, SIZEOF(winfo.cond)))
		_tcscpy(winfo.cond, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Temperature", winfo.temp, SIZEOF(winfo.temp)))
		_tcscpy(winfo.temp, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "High", winfo.high, SIZEOF(winfo.high)))
		_tcscpy(winfo.high, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Low", winfo.low, SIZEOF(winfo.low)))
		_tcscpy(winfo.low, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Sunset", winfo.sunset, SIZEOF(winfo.sunset)))
		_tcscpy(winfo.sunset, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Sunrise", winfo.sunrise, SIZEOF(winfo.sunrise)))
		_tcscpy(winfo.sunrise, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Wind Speed", winfo.wind, SIZEOF(winfo.wind)))
		_tcscpy(winfo.wind, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Wind Direction", winfo.winddir, SIZEOF(winfo.winddir)))
		_tcscpy(winfo.winddir, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Dewpoint", winfo.dewpoint, SIZEOF(winfo.dewpoint)))
		_tcscpy(winfo.dewpoint, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Pressure", winfo.pressure, SIZEOF(winfo.pressure)))
		_tcscpy(winfo.pressure, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Visibility", winfo.vis, SIZEOF(winfo.vis)))
		_tcscpy(winfo.vis, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Humidity", winfo.humid, SIZEOF(winfo.humid)))
		_tcscpy(winfo.humid, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Feel", winfo.feel, SIZEOF(winfo.feel)))
		_tcscpy(winfo.feel, NODATA);

	winfo.status = (WORD)DBGetContactSettingWord(Change, WEATHERPROTONAME, "StatusIcon", ID_STATUS_OFFLINE);
	return winfo;
}

// getting weather setting from database
// return 0 on success
int DBGetData(HANDLE hContact, char *setting, DBVARIANT *dbv) 
{
	if (DBGetContactSettingTString(hContact, WEATHERCONDITION, setting, dbv)) {
		size_t len = strlen(setting) + 1;
		char *set = (char*)alloca(len + 1);
		*set = '#';
		memcpy(set + 1, setting, len);

		if ( DBGetContactSettingTString(hContact, WEATHERCONDITION, set, dbv))
			return 1;
	}
	return 0;
}

int DBGetStaticString(HANDLE hContact, const char *szModule, const char *valueName, TCHAR *dest, size_t dest_len)
{
	DBVARIANT dbv;
	if ( DBGetContactSettingTString( hContact, szModule, valueName, &dbv ))
		return 1;

	_tcsncpy( dest, dbv.ptszVal, dest_len );
	dest[ dest_len-1 ] = 0;
	DBFreeVariant( &dbv );
	return 0;
}


//============  ERASE OLD SETTINGS  ============

// erase all current weather information from database
// lastver = the last used version number in dword (using PLUGIN_MAKE_VERSION)
void EraseAllInfo(DWORD lastver) 
{
	TCHAR str[255];
	int ContactCount = 0;
	HANDLE hContact, LastContact = NULL;
	DBVARIANT dbv;
	// loop through all contacts
	hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while(hContact) 
	{
		// see if the contact is a weather contact
		if (IsMyContact(hContact)) 
		{
			// check for upgrade
			if (lastver < __VERSION_DWORD)	UpgradeContact(lastver, hContact);
			DBWriteContactSettingWord(hContact,WEATHERPROTONAME,"Status",ID_STATUS_OFFLINE);
			DBWriteContactSettingWord(hContact,WEATHERPROTONAME,"StatusIcon",ID_STATUS_OFFLINE);
			DBDeleteContactSetting(hContact,"CList","MyHandle");
			// clear all data
			if (DBGetContactSettingTString(hContact, WEATHERPROTONAME, "Nick", &dbv)) {
				DBWriteContactSettingTString(hContact, WEATHERPROTONAME, "Nick", TranslateT("<Enter city name here>"));
				DBWriteContactSettingString(hContact, WEATHERPROTONAME, "LastLog", "never");
				DBWriteContactSettingString(hContact, WEATHERPROTONAME, "LastCondition", "None");
				DBWriteContactSettingString(hContact, WEATHERPROTONAME, "LastTemperature", "None");
			}
			else DBFreeVariant(&dbv);

			DBDataManage(hContact, WDBM_REMOVE, 0, 0);
			DBWriteContactSettingString(hContact, "UserInfo", "MyNotes", "");
			// reset update tag
			DBWriteContactSettingByte(hContact,WEATHERPROTONAME,"IsUpdated",FALSE);
			// reset logging settings
			if ( !DBGetContactSettingTString(hContact,WEATHERPROTONAME,"Log", &dbv)) {
				DBWriteContactSettingByte(hContact,WEATHERPROTONAME,"File",(BYTE)(dbv.ptszVal[0] != 0));
				DBFreeVariant(&dbv);
			}
			else DBWriteContactSettingByte(hContact,WEATHERPROTONAME,"File",FALSE);

			// if no default station find, assign a new one
			if (opt.Default[0] == 0) {
				GetStationID(hContact, opt.Default, SIZEOF(opt.Default));

				opt.DefStn = hContact;
				if (!DBGetContactSettingTString(hContact,WEATHERPROTONAME,"Nick",&dbv)) {
					wsprintf(str, TranslateT("%s is now the default weather station"), dbv.ptszVal);
					DBFreeVariant(&dbv);
					MessageBox(NULL, str, TranslateT("Weather Protocol"), MB_OK|MB_ICONINFORMATION);
				}
			}
			// get the handle of the default station
			if (opt.DefStn == NULL) {
				if ( !DBGetContactSettingTString(hContact,WEATHERPROTONAME,"ID",&dbv)) {
					if ( !_tcscmp(dbv.ptszVal, opt.Default))	opt.DefStn = hContact;
					DBFreeVariant(&dbv);
				}
			}
			ContactCount++;		// increment counter
			LastContact = hContact;
		}
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
	// if weather contact exists, set the status to online so it is ready for update
	// if (ContactCount != 0) status = ONLINE;
	// in case where the default station is missing
	if (opt.DefStn == NULL && ContactCount != 0) {
		if ( !DBGetContactSettingTString(LastContact, WEATHERPROTONAME, "ID", &dbv)) {
			_tcscpy(opt.Default, dbv.ptszVal);
			DBFreeVariant(&dbv);
		}
		opt.DefStn = LastContact;
		if (!DBGetContactSettingTString(LastContact,WEATHERPROTONAME,"Nick",&dbv)) {
			wsprintf(str, TranslateT("%s is now the default weather station"), dbv.ptszVal);
			DBFreeVariant(&dbv);
			MessageBox(NULL, str, TranslateT("Weather Protocol"), MB_OK|MB_ICONINFORMATION);
		}
	}
	// save option in case of default station changed
	DBWriteContactSettingTString(NULL, WEATHERPROTONAME, "Default", opt.Default);
}

void ConvertDataValue(WIDATAITEM *UpdateData, TCHAR *Data) 
{
	TCHAR str[MAX_DATA_LEN];

	// convert the unit
	if ( _tcscmp(Data, TranslateT("<Error>")) && _tcscmp(Data, NODATA) && _tcscmp(Data, TranslateTS(NODATA))) {
		// temperature
		if (!_tcscmp(UpdateData->Name, _T("Temperature")) || !_tcscmp(UpdateData->Name, _T("High")) || 
			!_tcscmp(UpdateData->Name, _T("Low")) || !_tcscmp(UpdateData->Name, _T("Feel")) || 
			!_tcscmp(UpdateData->Name, _T("Dewpoint")) ||
			!_tcsicmp(UpdateData->Unit, _T("C")) || !_tcsicmp(UpdateData->Unit, _T("F")) || 
			!_tcsicmp(UpdateData->Unit, _T("K")))
		{
			GetTemp(Data, UpdateData->Unit, str);
			_tcscpy(Data, str);
		}
		// pressure
		else if (!_tcscmp(UpdateData->Name, _T("Pressure")) || !_tcsicmp(UpdateData->Unit, _T("HPA")) || 
			!_tcsicmp(UpdateData->Unit, _T("KPA")) || !_tcsicmp(UpdateData->Unit, _T("MB")) ||
			!_tcsicmp(UpdateData->Unit, _T("TORR")) || !_tcsicmp(UpdateData->Unit, _T("IN")) || 
			!_tcsicmp(UpdateData->Unit, _T("MM")))
		{
			GetPressure(Data, UpdateData->Unit, str);
			_tcscpy(Data, str);
		}
		// speed
		else if (!_tcscmp(UpdateData->Name, _T("Wind Speed")) || !_tcsicmp(UpdateData->Unit, _T("KM/H")) || 
			!_tcsicmp(UpdateData->Unit, _T("M/S")) || !_tcsicmp(UpdateData->Unit, _T("MPH")) || 
			!_tcsicmp(UpdateData->Unit, _T("KNOTS")))
		{
			GetSpeed(Data, UpdateData->Unit, str);
			_tcscpy(Data, str);
		}
		// visibility
		else if (!_tcscmp(UpdateData->Name, _T("Visibility")) || !_tcsicmp(UpdateData->Unit, _T("KM")) || 
			!_tcsicmp(UpdateData->Unit, _T("MILES")))
		{
			GetDist(Data, UpdateData->Unit, str);
			_tcscpy(Data, str);
		}
		// elevation
		else if (!_tcscmp(UpdateData->Name, _T("Elevation")) || !_tcsicmp(UpdateData->Unit, _T("FT")) || 
			!_tcsicmp(UpdateData->Unit, _T("M")))
		{
			GetElev(Data, UpdateData->Unit, str);
			_tcscpy(Data, str);
		}
		// converting case for condition to the upper+lower format
		else if (!_tcsicmp(UpdateData->Unit, _T("COND")))
			CaseConv(Data);
		// degree sign
		else if (!_tcsicmp(UpdateData->Unit, _T("DEG")))
		{
			if (!opt.DoNotAppendUnit) _tcscat(Data, opt.DegreeSign);
		}
		// percent sign
		else if (!_tcsicmp(UpdateData->Unit, _T("%")))
		{
			if (!opt.DoNotAppendUnit) _tcscat(Data, _T("%"));
		}
		// truncating strings for day/month to 2 or 3 characters
		else if (!_tcsicmp(UpdateData->Unit, _T("DAY")) || !_tcsicmp(UpdateData->Unit, _T("MONTH")))
			if (opt.dUnit > 1 && _tcslen(Data) > opt.dUnit)
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
			start += _tcslen(UpdateData->Start);
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
		end += _tcslen(UpdateData->End);
		last = '\n';
	}
	// ignore if not both of the string found - this prevent crashes
	if (start != NULL && end != NULL) 
	{
		// begin reading the data from start location to end location
		// remove all HTML tag in between, as well as leading space, ending space,
		// multiple spaces, tabs, and return key
		while (startloc < endloc) 
		{
			if (szInfo[startloc] == '<')	tag = TRUE;
			else if (szInfo[startloc] == '&' &&
				(szInfo[startloc+1] == ';' || szInfo[startloc+2] == ';' || szInfo[startloc+3] == ';' || 
				szInfo[startloc+4] == ';' || szInfo[startloc+5] == ';' || szInfo[startloc+6] == ';'))
				symb = TRUE;
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
				if (opt.ShowWarnings && UpdateData->Name[0] != 0 && _tcscmp(UpdateData->Name, _T("Ignore"))) {
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
	if (Value[0] != 0)
	{
		char *newData = (char*)mir_alloc(strlen(Value)+3);
		strcpy(newData, Value);
		*Data = newData;
	}
	else
		*Data = "";
}

void wSetData(WCHAR **Data, const char *Value) 
{
	if (Value[0] != 0)
		*Data = mir_a2u( Value );
	else
		*Data = L"";
}

void wSetData(WCHAR **Data, const WCHAR *Value) 
{
	if (Value[0] != 0) {
		WCHAR *newData = (WCHAR*)mir_alloc( sizeof(WCHAR)*(wcslen(Value)+3));
		wcscpy(newData, Value);
		*Data = newData;
	}
	else *Data = L"";
}

// A safer free function that free memory for a string
// Data = the string occuping the data to be freed
void wfree(char **Data) 
{
	if (*Data && strlen(*Data) > 0)	mir_free(*Data);
	*Data = NULL;
}

void wfree(WCHAR **Data) 
{
	if (*Data && wcslen(*Data) > 0)
		mir_free(*Data);
	*Data = NULL;
}

//============ MANAGE THE ITEMS STORED IN DB ============

// remove or display the weatehr information for a contact
// hContact - the contact in which the info is going to be removed
void DBDataManage(HANDLE hContact, WORD Mode, WPARAM wParam, LPARAM lParam) 
{
	DBCONTACTENUMSETTINGS dbces;
	DBVARIANT dbv;
	WCOUNTER wc;
	wc.current = 0;

	dbces.lParam = (LPARAM)&wc;
	dbces.pfnEnumProc = GetWeatherDataFromDB;
	dbces.szModule = WEATHERCONDITION;

	// get all the settings and stored them in a temporary list
	if (CallService(MS_DB_CONTACT_ENUMSETTINGS,(WPARAM)hContact,(LPARAM)&dbces) == -1)
		wc.current--;

	// begin deleting settings
	for (; --wc.current>-1;) {
		if (!DBGetContactSettingTString(hContact, WEATHERCONDITION, wc.value[wc.current], &dbv)) {
			switch (Mode) {
			case WDBM_REMOVE:
				DBDeleteContactSetting(hContact, WEATHERCONDITION, wc.value[wc.current]);
				break;

			case WDBM_DETAILDISPLAY: 
				{
					// skip the "WeatherInfo" variable
					if (strcmp(wc.value[wc.current], "WeatherInfo") == 0 ||
						strcmp(wc.value[wc.current], "Ignore") == 0 ||
						wc.value[wc.current][0] == '#')
					{
						wfree(&wc.value[wc.current]);
						DBFreeVariant(&dbv);
						continue;
					}

					HWND hList = GetDlgItem((HWND)wParam, IDC_DATALIST);
					LV_ITEM lvi = { 0 };
					lvi.mask = LVIF_TEXT | LVIF_PARAM;
					lvi.iItem = 0;
					lvi.iSubItem = 0;
					lvi.lParam = (LPARAM)wc.current;
					lvi.pszText = TranslateTS( _A2T(wc.value[wc.current] ));
					lvi.iItem = ListView_InsertItem(hList, &lvi);
					lvi.pszText = dbv.ptszVal;
					ListView_SetItemText(hList, lvi.iItem, 1, dbv.ptszVal );
					break;
				}
			}
			DBFreeVariant(&dbv);
		}
		wfree(&wc.value[wc.current]);
	}
}

// get single setting that is found
// szSetting = the setting name
// lparam = the counter
int GetWeatherDataFromDB(const char *szSetting, LPARAM lparam) 
{
	WCOUNTER *wc = (WCOUNTER*)lparam;
	wc->value[wc->current] = (char *)mir_alloc(strlen(szSetting) + 1);
	strcpy(wc->value[wc->current], szSetting);
	wc->current++;
	return 0;
}
