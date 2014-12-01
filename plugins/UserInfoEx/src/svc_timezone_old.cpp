/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "commonheaders.h"

#define TZREG		"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones"
#define TZREG_9X	"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Time Zones"

/**************************************************************************************************
 * struct CTimeZone
 **************************************************************************************************/

/**
* This is the default constructure, which resets
* all attributes to NULL.
**/

CTimeZone::CTimeZone()
{
	memset(this, 0, sizeof(*this));
}

/**
* The default construcor's task ist to clear out
* all pieces of the used memory.
**/

CTimeZone::~CTimeZone()
{
	MIR_FREE(ptszName);
	MIR_FREE(ptszDisplay);
}

/**
* This method can be used to basically convert a Windows
* timezone to the format, known by miranda.
*
* @warning		This operation does not work vice versa in
*				all cases, as there are sometimes more then
*				one Windows timezones with the same Bias.
**/

BYTE CTimeZone::ToMirandaTimezone() const
{
	return (BYTE) (Bias / 30);
}

/**
* This operator translates the content of this object to
* a TIME_ZONE_INFORMATION structure as it is required by 
* several windows functions.
**/

CTimeZone::operator TIME_ZONE_INFORMATION() const
{
	TIME_ZONE_INFORMATION tzi;
	tzi.Bias = Bias;
	tzi.DaylightBias = DaylightBias;
	tzi.StandardBias = StandardBias;

	memcpy(&tzi.DaylightDate, &DaylightDate, sizeof(DaylightDate));
	memcpy(&tzi.StandardDate, &StandardDate, sizeof(DaylightDate));
	return tzi;
}		

/***********************************************************************************************************
 * class CTzBias
 ***********************************************************************************************************/

class CTzBias : public LIST<CTimeZone>
{
	static int sortFunc(const CTimeZone *tz1, const CTimeZone *tz2)
	{
		int result = tz2->Bias - tz1->Bias;
		// DO NOT USE mir_tcsicmp here as it does only return TRUE or FALSE!!!
		// mir_tstrcmpi takes care of umlauts e.g. Ä,Ö,....
		return (result || !tz1->ptszDisplay || !tz2->ptszDisplay) ? result : mir_tstrcmpi(tz1->ptszDisplay, tz2->ptszDisplay);
	}
public:
	CTzBias() : LIST<CTimeZone>(50, (FTSortFunc) CTzBias::sortFunc)
	{
	}

	~CTzBias()
	{
		// delete the list, items delete by CTzMgr
		this->destroy();
	}
};
/***********************************************************************************************************
 * class CTzMgr
 ***********************************************************************************************************/

/**
* This class is a deriviant of miranda's SortedList and holds all timezones
* known by Windows. By default there is no API to list timezones, so we
* need to get the information directly from the registry. In order to avoid
* heavy reading operations from registry, this class has the task to cache
* all required information for much faster access.
**/

class CTzMgr : public LIST<CTimeZone>
{
	CTzBias _bias;

	static int sortFunc(const CTimeZone *tz1, const CTimeZone *tz2)
	{
		// DO NOT USE mir_tcsicmp here as it does only return TRUE or FALSE!!!
		return _tcsicmp(tz1->ptszName, tz2->ptszName);
	}

	/**
	 * This method clears the TzTzMgr's data.
	 **/		
	void destroy()
	{
		// delete data
		for (int i = 0 ; i < count; i++)
			delete (*this)[i];

		// delete the list
		LIST<CTimeZone>::destroy();
	}

public:

	const CTzBias& Bias;

	CTzMgr() 
		:LIST<CTimeZone>(50, (FTSortFunc) CTzMgr::sortFunc),
		_bias(), Bias(_bias)
	{
	}

	/**
	* This is the default destructor of the class.
	*
	* @param	 none
	*
	* @return	nothing
	**/

	~CTzMgr()
	{
		destroy();
	}

	/**
	* This method loads all information about timezones from windows' registry.
	**/

	int Init()
	{
		int			result;
		HKEY		hKeyRoot,
					hKeyTz;
		DWORD		i,
					cbData;
		TCHAR		szName[MAX_PATH],
					szDisplay[MAX_PATH];
		CTimeZone	*pTimeZone;

		result = RegOpenKey(HKEY_LOCAL_MACHINE, _T(TZREG), &hKeyRoot);
		if (result != ERROR_SUCCESS)
			result = RegOpenKey(HKEY_LOCAL_MACHINE, _T(TZREG_9X), &hKeyRoot);

		if (result == ERROR_SUCCESS) {
			// clear out old list
			this->destroy(); _bias.destroy();
			for (i = 0; ERROR_SUCCESS == RegEnumKey(hKeyRoot, i, szName, SIZEOF(szName)); i++) {
				result = RegOpenKey(hKeyRoot, szName, &hKeyTz);
				if (result == ERROR_SUCCESS) {
					pTimeZone = new CTimeZone();
					if (pTimeZone) {
						cbData = sizeof(szDisplay);
						result |= RegQueryValueEx(hKeyTz, _T("Display"), 0, 0, (LPBYTE)szDisplay, &cbData);

						cbData = sizeof(REG_TZI_FORMAT);
						result |= RegQueryValueEx(hKeyTz, _T("TZI"), 0, 0, (LPBYTE)pTimeZone, &cbData);

						cbData = sizeof(DWORD);
						if (RegQueryValueEx(hKeyTz, _T("Index"), 0, 0, (LPBYTE)(UINT_PTR)pTimeZone->dwIndex, &cbData) != ERROR_SUCCESS)
							pTimeZone->dwIndex = TZINDEX_UNSPECIFIED;

						if (result == ERROR_SUCCESS) {
							pTimeZone->ptszName = mir_tstrdup(szName);
							pTimeZone->ptszDisplay = mir_tstrdup(szDisplay);
							result = (insert(pTimeZone) == ERROR_SUCCESS);
						}

						if (result != ERROR_SUCCESS)
							delete pTimeZone;
						else
							_bias.insert(pTimeZone);
					}
					RegCloseKey(hKeyTz);
				}
			}
			RegCloseKey(hKeyRoot);
		}
		return result;
	}

	/**
	* This method is used to find a certain list entry according to
	* a key, providing information about the entry to look for.
	* 
	* @param	result		- Pointer to a pointer, retrieving the CTimeZone
	*						  object, matching the criteria provided by key
	* @param	key			- Pointer to a CTimeZone structure, providing
	*						  information about the item to look for.
	*						  The Bias member and/or pszDisplay member must
	*						  be valid.
	* @retval	-1			: item not found
	* @retval 0...count	: index of the found item
	**/

	int find(CTimeZone** pTimezone, CTimeZone* pKey) const
	{ 
		int nItemIndex = -1;
		
		if (pKey && pKey->ptszName) {
			nItemIndex = getIndex(pKey);
			if (pTimezone)
				*pTimezone = (nItemIndex == -1) ? NULL : items[nItemIndex];
		}
		return nItemIndex;
	}

	int find(CTimeZone** pTimezone, LPTSTR ptszName) const
	{ 
		CTimeZone key;
		int nItemIndex;

		key.ptszName = ptszName;
		nItemIndex = find(pTimezone, &key);
		key.ptszName = NULL; // prevents ptszName from being deleted by the destructor.
		return nItemIndex;
	}

	/**
	* This method is used to find a certain list entry according to
	* a given dwTzIndex, providing information about the entry to look for.
	* 
	* @param	result		- Pointer to a pointer, retrieving the CTimeZone
	*						  object, matching the criteria provided by key
	* @param	dwTzIndex	- Timezone index as read from Windows Registry
	* @retval	-1			: item not found
	* @retval	0...count	: index of the found item
	**/	 

	int find(CTimeZone** result, DWORD dwTzIndex) const
	{ 
		int nItemIndex = -1;
		CTimeZone *ptz = NULL;
		
		if (dwTzIndex != TZINDEX_UNSPECIFIED) {
			for (nItemIndex = 0; nItemIndex < count; nItemIndex++) {
				ptz = items[nItemIndex];
				if (ptz && (ptz->dwIndex == dwTzIndex))
					break;
			}
		}

		if (result)
			*result = ptz;
		return ((nItemIndex == count) ? -1 : nItemIndex);
	}
};

// global timezone TzMgr object
static CTzMgr TzMgr;

/***********************************************************************************************************
 * public Service functions
 ***********************************************************************************************************/

/**
* This method trys to find some default windows timezone idices for a given
* miranda timezone.
*
* @param	MirTz			- this is a miranda timezone with values between -24 and 24.
*
* @return	This method returns a @TZ_MAP struct of a windows timezone, which is maps
*			the @MirTz value,name or {-1,NULL} if no windows timezone index exists.
**/

static TZ_MAP MirTZ2WinTZ(const CHAR MirTz)
{
	/**
	* This is an item of an array of timezones, which are known by both Miranda-IM
	* and Windows. It is used to map an ICQ timezone against a Windows timezone
	* for retrieving information about daylight saving time and more.
	**/
	static const TZ_MAP TzMap[] = {
		{ 0,	_T("Dateline Standard Time")},			// GMT-12:00 Eniwetok; Kwajalein
		{-1,	_T("")},								// GMT-11:30
		{ 1,	_T("Samoa Standard Time")},				// GMT-11:00 Midway Island; Samoa
		{-1,	_T("")},								// GMT-10:30
		{ 2,	_T("Hawaiian Standard Time")},			// GMT-10:00 Hawaii
		{-1,	_T("")},								// GMT-9:30
		{ 3,	_T("Alaskan Standard Time")},			// GMT-9:00 Alaska
		{-1,	_T("")},								// GMT-8:30
		{ 4,	_T("Pacific Standard Time")},			// GMT-8:00 Pacific Time; Tijuana
		{-1,	_T("")},								// GMT-7:30
		{15,	_T("US Mountain Standard Time")},		// GMT-7:00 Arizona; Mountain Time
		{-1,	_T("")},								// GMT-6:30
		{33,	_T("Central America Standard Time")},	// GMT-6:00 Central Time; Central America; Saskatchewan
		{-1,	_T("")},								// GMT-5:30
		{45,	_T("SA Pacific Standard Time")},		// GMT-5:00 Eastern Time; Bogota; Lima; Quito
		{-1,	_T("")},								// GMT-4:30
		{56,	_T("Pacific SA Standard Time")},		// GMT-4:00 Atlantic Time; Santiago; Caracas; La Paz
		{60,	_T("Newfoundland Standard Time")},		// GMT-3:30 Newfoundland
		{70,	_T("SA Eastern Standard Time")},		// GMT-3:00 Greenland; Buenos Aires; Georgetown
		{-1,	_T("")},								// GMT-2:30
		{75,	_T("Mid-Atlantic Standard Time")},		// GMT-2:00 Mid-Atlantic
		{-1,	_T("")},								// GMT-1:30
		{80,	_T("Azores Standard Time")},			// GMT-1:00 Cape Verde Islands; Azores
		{-1,	_T("")},								// GMT-0:30
		{85,	_T("GMT Standard Time")},				// GMT+0:00 London; Dublin; Edinburgh; Lisbon; Casablanca
		{-1,	_T("")},								// GMT+0:30
		{105,	_T("Romance Standard Time")},			// GMT+1:00 Central European Time; West Central Africa; Warsaw
		{-1,	_T("")},								// GMT+1:30
		{140,	_T("South Africa Standard Time")},		// GMT+2:00 Jerusalem; Helsinki; Harare; Cairo; Bucharest; Athens
		{-1,	_T("")},								// GMT+2:30
		{145,	_T("Russian Standard Time")},			// GMT+3:00 Moscow; St. Petersburg; Nairobi; Kuwait; Baghdad
		{160,	_T("Iran Standard Time")},				// GMT+3:30 Tehran
		{165,	_T("Arabian Standard Time")},			// GMT+4:00 Baku; Tbilisi; Yerevan; Abu Dhabi; Muscat
		{175,	_T("Afghanistan Standard Time")},		// GMT+4:30 Kabul
		{185,	_T("West Asia Standard Time")},			// GMT+5:00 Calcutta; Chennai; Mumbai; New Delhi; Ekaterinburg
		{200,	_T("Sri Lanka Standard Time")},			// GMT+5:30 Sri Jayawardenepura
		{201,	_T("N. Central Asia Standard Time")},	// GMT+6:00 Astana; Dhaka; Almaty; Novosibirsk
		{203,	_T("Myanmar Standard Time")},			// GMT+6:30 Rangoon
		{207,	_T("North Asia Standard Time")},		// GMT+7:00 Bankok; Hanoi; Jakarta; Krasnoyarsk
		{-1,	_T("")},								// GMT+7:30
		{210,	_T("China Standard Time")},				// GMT+8:00 Perth; Taipei; Singapore; Hong Kong; Beijing
		{-1,	_T("")},								// GMT+8:30
		{235,	_T("Tokyo Standard Time")},				// GMT+9:00 Tokyo; Osaka; Seoul; Sapporo; Yakutsk
		{245,	_T("AUS Central Standard Time")},		// GMT+9:30 Darwin; Adelaide
		{270,	_T("Vladivostok Standard Time")},		// GMT+10:00 East Australia; Guam; Vladivostok
		{-1,	_T("")},								// GMT+10:30
		{280,	_T("Central Pacific Standard Time")},	// GMT+11:00 Magadan; Solomon Is.; New Caledonia
		{-1,	_T("")},								// GMT+11:30
		{290,	_T("New Zealand Standard Time")},		// GMT+12:00 Auckland; Wellington; Fiji; Kamchatka; Marshall Is.
		{-1,	_T("")}
	};
	return (MirTz >= -24 && MirTz <= 24) ? TzMap[24 - MirTz] : TzMap[49] ;
}

/**
* This function reads out the Timezone, associated with the given contact
*
* @param	hContact		- HANDLE of the contact to retrieve the timezone for.
* @param	pszProto		- contact's protocol 
*
* @retval	NULL			- No timezone exists.
* @retval	CTimeZone*		- Pointer to the timezone.
**/

CTimeZone* GetContactTimeZone(MCONTACT hContact, LPCSTR pszProto)
{
	LPTSTR ptszName;
	CTimeZone* pTimeZone = NULL;

	// read windows timezone from database (include meta subcontacts)
	ptszName = DB::Setting::GetTStringEx(hContact, USERINFO, pszProto, SET_CONTACT_TIMEZONENAME);
	if (!ptszName || FAILED(TzMgr.find(&pTimeZone, ptszName))) {
		DBVARIANT dbv;
		TZ_MAP MirTZ;

		// try to get miranda's timezone index value
		if (!myGlobals.TzIndexExist || DB::Setting::GetAsIsEx(hContact, USERINFO, pszProto, SET_CONTACT_TIMEZONEINDEX, &dbv) || FAILED(TzMgr.find(&pTimeZone,dbv.dVal))) {
			// maybe a failure lets us read a string, so clear it out
			db_free(&dbv);

			// try to get miranda's timezone value
			if (DB::Setting::GetAsIsEx(hContact, USERINFO, pszProto, SET_CONTACT_TIMEZONE, &dbv) || (dbv.type != DBVT_BYTE))
				db_free(&dbv); // maybe a failure lets us read a string, so clear it out
			else {
				MirTZ = MirTZ2WinTZ(dbv.cVal);
				if (*MirTZ.Name != 0)
					TzMgr.find(&pTimeZone, MirTZ.Name);
			}
		}
	}
	MIR_FREE(ptszName);
	return pTimeZone;
}

/**
*
*
**/

CTimeZone* GetContactTimeZone(MCONTACT hContact)
{
	return GetContactTimeZone(hContact, DB::Contact::Proto(hContact));
}

/**
* This method trys to find the contact's windows timezone.
*
* @warning	Make sure you convert @e dwIndex to CHAR if the function returns 1 in order to get
*			the correct miranda timezone!
*
* @param	hContact		- the HANDLE of the contact to read timezone information for
* @param	szProto			- contact's protocol
* @param	pTimeZone		- Pointer to the pointer of a CTimeZone structure, 
*							  which retrieves information about contact's timezone.
*
* @retval	CTRLF_... flag	- The index for a windows timezone was found for the contact.
* @retval	0				- There is no index, but if the contact's 'timezone' setting is valid,
*							  @e dwIndex retrieved its value. If not, dwIndex is -100 (unspecified).
**/

WORD GetContactTimeZoneCtrl(MCONTACT hContact, LPCSTR pszProto, CTimeZone** pTimeZone)
{
	WORD flags;
	DBVARIANT dbv;
	CTimeZone* pTz = NULL;

	// try to read windows' timezone name from database
	flags = DB::Setting::GetCtrl(hContact, USERINFO, USERINFO, pszProto, SET_CONTACT_TIMEZONENAME, &dbv, DBVT_TCHAR);
	if (flags == 0 || FAILED(TzMgr.find(&pTz, dbv.ptszVal))) {
		db_free(&dbv);

		// try to get miranda's timezone index value
		if (myGlobals.TzIndexExist) {
			flags = DB::Setting::GetAsIsCtrl(hContact, USERINFO, USERINFO, pszProto, SET_CONTACT_TIMEZONEINDEX, &dbv);
			if (flags && FAILED(TzMgr.find(&pTz, dbv.dVal)))
				flags = 0;
		}

		if (flags == 0) {
			// try to get miranda's timezone value
			flags = DB::Setting::GetAsIsCtrl(hContact, USERINFO, USERINFO, pszProto, SET_CONTACT_TIMEZONE, &dbv);
			if (flags != 0) {
				TZ_MAP MirTZ;
				MirTZ = MirTZ2WinTZ(dbv.cVal);
				if ((*MirTZ.Name == 0) || FAILED(TzMgr.find(&pTz, MirTZ.Name)))
					flags = 0;
			}
		}
	}

	if (pTimeZone && flags != 0)
		*pTimeZone = pTz;

	db_free(&dbv);
	return flags;
}

/**
* This function returns the display name for the contact's timezone
*
* @param	hContact		- handle of the contact
*
* @return	String containing display name.
**/

LPCTSTR GetContactTimeZoneDisplayName(MCONTACT hContact)
{
	CTimeZone *pTimeZone;
	
	pTimeZone = GetContactTimeZone(hContact);
	return (pTimeZone) ? pTimeZone->ptszDisplay : NULL;
}

/**
*
*
**/

INT_PTR EnumTimeZones(PEnumNamesProc enumProc, LPARAM lParam)
{
	INT_PTR i, c, r = 0;
	CTimeZone *pTz;

	for (i = 0, c = TzMgr.Bias.getCount(); i < c; i++) {
		pTz = TzMgr.Bias[i];
		if (pTz) {
			r = enumProc(pTz, i, lParam);
			if (r) break;
		}
	}
	return r;
}

/**
*
*
**/

static BOOL SvcTimezoneSyncWithWindowsProc(LPCSTR pszProto, int bias)
{
	int tz = (int) ((CHAR)db_get_b(NULL, pszProto, SET_CONTACT_TIMEZONE, (BYTE)-100));
	if (tz * 30 != bias) {
		db_set_b(NULL, pszProto, SET_CONTACT_TIMEZONE, (BYTE)(bias / 30));
		return TRUE;
	}
	return FALSE;
}

/**
*
*
**/

void SvcTimezoneSyncWithWindows()
{
	TIME_ZONE_INFORMATION tzi = { 0 };
	GetTimeZoneInformation(&tzi);

	PROTOACCOUNT **pAcc;
	int nAccCount;
	if (MIRSUCCEEDED(ProtoEnumAccounts(&nAccCount, &pAcc))) {
		for (int i = 0; i < nAccCount; i++) {
			// update local timezone as database setting
			if (IsProtoAccountEnabled(pAcc[i]) && SvcTimezoneSyncWithWindowsProc(pAcc[i]->szModuleName, tzi.Bias))
				// update my contact information on icq server
				CallProtoService(pAcc[i]->szModuleName, PS_CHANGEINFOEX, CIXT_LOCATION, NULL);
		}
	}
}

/***********************************************************************************************************
 * services use old UIEX timezone
 ***********************************************************************************************************/

/**
* This service function provides a TIME_ZONE_INFORMATION structure
* for the desired contact, in case the contact's timezone can be determined.
* 
* @param	wParam			- HANDLE of the contact, to retrieve timezone information from.
* @param	lParam			- pointer to a TIME_ZONE_INFORMATION to fill.
*
* @retval	0 - success
* @retval	1 - failure
**/

INT_PTR GetContactTimeZoneInformation_old(WPARAM wParam,LPARAM lParam)
{
	CTimeZone *pTimeZone;
	TIME_ZONE_INFORMATION* pTimeZoneInformation = (TIME_ZONE_INFORMATION*)lParam;
	
	pTimeZone = GetContactTimeZone(wParam);
	if (pTimeZone && pTimeZoneInformation)
		(*pTimeZoneInformation) = *pTimeZone;

	return (pTimeZone == NULL) || (pTimeZoneInformation == NULL);
}

/**
* This function returns the contact's local time.
*
* @param	wParam			- HANDLE of the contact, to retrieve timezone information from.
* @param	lParam			- pointer to a systemtime structure
*	
* @return	TRUE or FALSE
**/

INT_PTR GetContactLocalTime_old(WPARAM wParam, LPARAM lParam)
{
	MTime	now;
	now.GetLocalTime(wParam);

	LPSYSTEMTIME pSystemTime = (LPSYSTEMTIME)lParam;
	*pSystemTime = now.SystemTime();
	return 0;
}

/***********************************************************************************************************
 * initialization
 ***********************************************************************************************************/

/**
* This function initially loads the module upon startup.
**/

void SvcTimezoneLoadModule_old()
{
	TzMgr.Init();
	CreateServiceFunction(MS_USERINFO_TIMEZONEINFO, GetContactTimeZoneInformation);
	CreateServiceFunction(MS_USERINFO_LOCALTIME, GetContactLocalTime);
	if (db_get_b(NULL, MODNAME, SET_OPT_AUTOTIMEZONE, TRUE))
		SvcTimezoneSyncWithWindows();
}
