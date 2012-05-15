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
void GetStationID(HANDLE hContact, char* id, size_t idlen) 
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
	GetStationID(Change, winfo.id, sizeof(winfo.id));

	if (DBGetStaticString(Change, WEATHERPROTONAME, "Nick", winfo.city, sizeof(winfo.city)))
		strcpy(winfo.city, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Update", winfo.update, sizeof(winfo.update)))
		strcpy(winfo.update, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Condition", winfo.cond, sizeof(winfo.cond)))
		strcpy(winfo.cond, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Temperature", winfo.temp, sizeof(winfo.temp)))
		strcpy(winfo.temp, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "High", winfo.high, sizeof(winfo.high)))
		strcpy(winfo.high, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Low", winfo.low, sizeof(winfo.low)))
		strcpy(winfo.low, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Sunset", winfo.sunset, sizeof(winfo.sunset)))
		strcpy(winfo.sunset, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Sunrise", winfo.sunrise, sizeof(winfo.sunrise)))
		strcpy(winfo.sunrise, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Wind Speed", winfo.wind, sizeof(winfo.wind)))
		strcpy(winfo.wind, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Wind Direction", winfo.winddir, sizeof(winfo.winddir)))
		strcpy(winfo.winddir, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Dewpoint", winfo.dewpoint, sizeof(winfo.dewpoint)))
		strcpy(winfo.dewpoint, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Pressure", winfo.pressure, sizeof(winfo.pressure)))
		strcpy(winfo.pressure, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Visibility", winfo.vis, sizeof(winfo.vis)))
		strcpy(winfo.vis, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Humidity", winfo.humid, sizeof(winfo.humid)))
		strcpy(winfo.humid, NODATA);
	if (DBGetStaticString(Change, WEATHERCONDITION, "Feel", winfo.feel, sizeof(winfo.feel)))
		strcpy(winfo.feel, NODATA);

	winfo.status = (WORD)DBGetContactSettingWord(Change, WEATHERPROTONAME, "StatusIcon", ID_STATUS_OFFLINE);
	return winfo;
}

// getting weather setting from database
// return 0 on success
int DBGetData(HANDLE hContact, char *setting, DBVARIANT *dbv) 
{
	if (DBGetContactSettingString(hContact, WEATHERCONDITION, setting, dbv)) 
	{
		size_t len = strlen(setting) + 1;
		char *set = (char*)alloca(len + 1);
		*set = '#';
		memcpy(set + 1, setting, len);

		if (DBGetContactSettingString(hContact, WEATHERCONDITION, set, dbv))
			return 1;
	}
	return 0;
}

int DBGetStaticString(HANDLE hContact, const char *szModule, const char *valueName, char *dest, size_t dest_len)
{
	DBVARIANT dbv;
	DBCONTACTGETSETTING sVal;

	dbv.pszVal = dest;
	dbv.cchVal = (WORD)dest_len;
	dbv.type = DBVT_ASCIIZ;

	sVal.pValue = &dbv;
	sVal.szModule = szModule;
	sVal.szSetting = valueName;

	if ( CallService( MS_DB_CONTACT_GETSETTINGSTATIC, ( WPARAM )hContact, ( LPARAM )&sVal ) != 0 )
		return 1;

	return ( dbv.type != DBVT_ASCIIZ );
}


//============  ERASE OLD SETTINGS  ============

// erase all current weather information from database
// lastver = the last used version number in dword (using PLUGIN_MAKE_VERSION)
void EraseAllInfo(DWORD lastver) 
{
	char str[255];
	int ContactCount = 0;
	HANDLE hContact, LastContact = NULL;
	DBVARIANT dbv;
	// loop through all contacts
	hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while(hContact) 
	{
		// see if the contact is a weather contact
		if(IsMyContact(hContact)) 
		{
			// check for upgrade
			if (lastver < __VERSION_DWORD)	UpgradeContact(lastver, hContact);
			DBWriteContactSettingWord(hContact,WEATHERPROTONAME,"Status",ID_STATUS_OFFLINE);
			DBWriteContactSettingWord(hContact,WEATHERPROTONAME,"StatusIcon",ID_STATUS_OFFLINE);
			DBDeleteContactSetting(hContact,"CList","MyHandle");
			// clear all data
			if (DBGetContactSettingString(hContact, WEATHERPROTONAME, "Nick", &dbv)) {
				DBWriteContactSettingString(hContact, WEATHERPROTONAME, "Nick", Translate("<Enter city name here>"));
				DBWriteContactSettingString(hContact, WEATHERPROTONAME, "LastLog", "never");
				DBWriteContactSettingString(hContact, WEATHERPROTONAME, "LastCondition", "None");
				DBWriteContactSettingString(hContact, WEATHERPROTONAME, "LastTemperature", "None");
			}
			else
				DBFreeVariant(&dbv);
/*
			DBWriteContactSettingString(hContact,WEATHERCONDITION,"Update", NODATA);
			DBWriteContactSettingString(hContact,WEATHERCONDITION,"Condition", NODATA);
			DBWriteContactSettingString(hContact,WEATHERCONDITION,"Temperature", NODATA);
			DBWriteContactSettingString(hContact,WEATHERCONDITION,"High", NODATA);
			DBWriteContactSettingString(hContact,WEATHERCONDITION,"Low", NODATA);
			DBWriteContactSettingString(hContact,WEATHERCONDITION,"Humidity", NODATA);
			DBWriteContactSettingString(hContact,WEATHERCONDITION,"Wind Speed", NODATA);
			DBWriteContactSettingString(hContact,WEATHERCONDITION,"Wind Direction", NODATA);
			DBWriteContactSettingString(hContact,WEATHERCONDITION,"Pressure", NODATA);
			DBWriteContactSettingString(hContact,WEATHERCONDITION,"Visibility", NODATA);
			DBWriteContactSettingString(hContact,WEATHERCONDITION,"Dewpoint", NODATA);
			DBWriteContactSettingString(hContact,WEATHERCONDITION,"Feel", NODATA);
			DBWriteContactSettingString(hContact,WEATHERCONDITION,"Heat Index", NODATA);
			DBWriteContactSettingString(hContact,WEATHERCONDITION,"Sunrise", NODATA);
			DBWriteContactSettingString(hContact,WEATHERCONDITION,"Sunset", NODATA);
*/
			DBDataManage(hContact, WDBM_REMOVE, 0, 0);
			DBWriteContactSettingString(hContact, "UserInfo", "MyNotes", "");
			// reset update tag
			DBWriteContactSettingByte(hContact,WEATHERPROTONAME,"IsUpdated",FALSE);
			// reset logging settings
			if (!DBGetContactSettingString(hContact,WEATHERPROTONAME,"Log",&dbv))
			{
				DBWriteContactSettingByte(hContact,WEATHERPROTONAME,"File",(BYTE)(dbv.pszVal[0] != 0));
				DBFreeVariant(&dbv);
			}
			else
				DBWriteContactSettingByte(hContact,WEATHERPROTONAME,"File",FALSE);
			// if no default station find, assign a new one
			if (opt.Default[0] == 0) 
			{
				GetStationID(hContact, opt.Default, sizeof(opt.Default));

				opt.DefStn = hContact;
				if (!DBGetContactSettingString(hContact,WEATHERPROTONAME,"Nick",&dbv))
				{
					wsprintf(str, Translate("%s is now the default weather station"), dbv.pszVal);
					DBFreeVariant(&dbv);
					MessageBox(NULL, str, Translate("Weather Protocol"), MB_OK|MB_ICONINFORMATION);
				}
			}
			// get the handle of the default station
			if (opt.DefStn == NULL) {
				if (!DBGetContactSettingString(hContact,WEATHERPROTONAME,"ID",&dbv))
				{
					if (!strcmp(dbv.pszVal, opt.Default))	opt.DefStn = hContact;
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
	if (opt.DefStn == NULL && ContactCount != 0) 
	{
		if (!DBGetContactSettingString(LastContact,WEATHERPROTONAME,"ID",&dbv))
		{
			strcpy(opt.Default, dbv.pszVal);
			DBFreeVariant(&dbv);
		}
		opt.DefStn = LastContact;
		if (!DBGetContactSettingString(LastContact,WEATHERPROTONAME,"Nick",&dbv))
		{
			wsprintf(str, Translate("%s is now the default weather station"), dbv.pszVal);
			DBFreeVariant(&dbv);
			MessageBox(NULL, str, Translate("Weather Protocol"), MB_OK|MB_ICONINFORMATION);
		}
	}
	// save option in case of default station changed
	DBWriteContactSettingString(NULL, WEATHERPROTONAME, "Default", opt.Default);
}

void ConvertDataValue(WIDATAITEM *UpdateData, char *Data) 
{
	char str[MAX_DATA_LEN];

	// convert the unit
	if (strcmp(Data, Translate("<Error>")) && strcmp(Data, NODATA) && strcmp(Data, Translate(NODATA))) 
	{
		// temperature
		if (!strcmp(UpdateData->Name, "Temperature") || !strcmp(UpdateData->Name, "High") || 
			!strcmp(UpdateData->Name, "Low") || !strcmp(UpdateData->Name, "Feel") || 
			!strcmp(UpdateData->Name, "Dewpoint") ||
			!_stricmp(UpdateData->Unit, "C") || !_stricmp(UpdateData->Unit, "F") || 
			!_stricmp(UpdateData->Unit, "K"))
		{
			GetTemp(Data, UpdateData->Unit, str);
			strcpy(Data, str);
		}
		// pressure
		else if (!strcmp(UpdateData->Name, "Pressure") || !_stricmp(UpdateData->Unit, "HPA") || 
			!_stricmp(UpdateData->Unit, "KPA") || !_stricmp(UpdateData->Unit, "MB") ||
			!_stricmp(UpdateData->Unit, "TORR") || !_stricmp(UpdateData->Unit, "IN") || 
			!_stricmp(UpdateData->Unit, "MM"))
		{
			GetPressure(Data, UpdateData->Unit, str);
			strcpy(Data, str);
		}
		// speed
		else if (!strcmp(UpdateData->Name, "Wind Speed") || !_stricmp(UpdateData->Unit, "KM/H") || 
			!_stricmp(UpdateData->Unit, "M/S") || !_stricmp(UpdateData->Unit, "MPH") || 
			!_stricmp(UpdateData->Unit, "KNOTS"))
		{
			GetSpeed(Data, UpdateData->Unit, str);
			strcpy(Data, str);
		}
		// visibility
		else if (!strcmp(UpdateData->Name, "Visibility") || !_stricmp(UpdateData->Unit, "KM") || 
			!_stricmp(UpdateData->Unit, "MILES"))
		{
			GetDist(Data, UpdateData->Unit, str);
			strcpy(Data, str);
		}
		// converting case for condition to the upper+lower format
		else if (!_stricmp(UpdateData->Unit, "COND"))
			CaseConv(Data);
		// degree sign
		else if (!_stricmp(UpdateData->Unit, "DEG"))
		{
			if (!opt.DoNotAppendUnit) strcat(Data, opt.DegreeSign);
		}
		// percent sign
		else if (!_stricmp(UpdateData->Unit, "%"))
		{
			if (!opt.DoNotAppendUnit) strcat(Data, "%");
		}
		// truncating strings for day/month to 2 or 3 characters
		else if (!_stricmp(UpdateData->Unit, "DAY") || !_stricmp(UpdateData->Unit, "MONTH"))
			if (opt.dUnit > 1 && strlen(Data) > opt.dUnit)		Data[opt.dUnit] = '\0';
	}
}

//============  GET THE VALUE OF A DATAITEM  ============

// get the value of the data using the start, end strings
// UpdateData = the WIDATAITEM struct containing start, end, unit
// Data = the string containing weather data obtained from UpdateData
// global var. used: szInfo = the downloaded string
void GetDataValue(WIDATAITEM *UpdateData, char *Data, char** szData) 
{
	char last = 0, current, *start, *end;
	unsigned startloc = 0, endloc = 0, respos = 0;
	BOOL tag = FALSE, symb = FALSE;
	char *szInfo = *szData;

	Data[0] = 0;
	// parse the data if available
	if (UpdateData->Start[0] == 0 && UpdateData->End[0] == 0) return;
	start = szInfo;
	// the start string must be found
	if (UpdateData->Start[0] != 0) {
		start = strstr(szInfo, UpdateData->Start);
		if (start != NULL) {
			// set the starting location for getting data
			start += strlen(UpdateData->Start);
			szInfo = start;
		}
	}
	// the end string must be found too
	if (UpdateData->End[0] != 0)
		end = strstr(szInfo, UpdateData->End);
	else end = strstr(szInfo, " ");
	if (end != NULL) {
		// set the ending location
		startloc = 0;
		endloc = end - szInfo;
		end += strlen(UpdateData->End);
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
				if (opt.ShowWarnings && UpdateData->Name[0] != 0 && strcmp(UpdateData->Name, "Ignore")) {
					mir_snprintf(Data, MAX_DATA_LEN, Translate("Error when obtaining data: %s"), UpdateData->Name);
					WPShowMessage(Data, SM_WARNING);
				}
				strncpy(Data, Translate("<Error>"), MAX_DATA_LEN);
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

		// write raw data for debug
		Netlib_Logf(hNetlibUser, "%s: %s", UpdateData->Name, Data);

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

// A safer free function that free memory for a string
// Data = the string occuping the data to be freed
void wfree(char **Data) 
{
	if (*Data && strlen(*Data) > 0)	mir_free(*Data);
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

	dbces.lParam=(LPARAM)&wc;
	dbces.pfnEnumProc=GetWeatherDataFromDB;
	dbces.szModule=WEATHERCONDITION;

	// get all the settings and stored them in a temporary list
	if(CallService(MS_DB_CONTACT_ENUMSETTINGS,(WPARAM)hContact,(LPARAM)&dbces)==-1)
		wc.current--;

	// begin deleting settings
	for (; --wc.current>-1;) 
	{
		if (!DBGetContactSettingString(hContact, WEATHERCONDITION, wc.value[wc.current], &dbv)) 
		{
			switch (Mode) 
			{
			case WDBM_REMOVE:
				DBDeleteContactSetting(hContact, WEATHERCONDITION, wc.value[wc.current]);
				break;

			case WDBM_DETAILDISPLAY: 
				{
					HWND hList = GetDlgItem((HWND)wParam, IDC_DATALIST);
					LV_ITEM lvi = { 0 };

					// skip the "WeatherInfo" variable
					if (strcmp(wc.value[wc.current], "WeatherInfo") == 0 ||
						strcmp(wc.value[wc.current], "Ignore") == 0 ||
						wc.value[wc.current][0] == '#')
					{
						wfree(&wc.value[wc.current]);
						DBFreeVariant(&dbv);
						continue;
					}

					lvi.mask = LVIF_TEXT | LVIF_PARAM;
					lvi.iItem = 0;
					lvi.iSubItem = 0;
					lvi.lParam = (LPARAM)wc.current;
					lvi.pszText = (LPSTR)Translate(wc.value[wc.current]);
					lvi.iItem = ListView_InsertItemWth(hList, &lvi);
					lvi.pszText = dbv.pszVal;
					ListView_SetItemTextWth(hList, lvi.iItem, 1, lvi.pszText);
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
