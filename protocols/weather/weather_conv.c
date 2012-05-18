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
This file contain the source related unit conversion, icon assignment, 
string conversions, display text parsing, etc
*/

#include "weather.h"

//============  SOME HELPER FUNCTIONS  ============

// see if a string is a number
// s = the string to be determined
// return value = true if the string is a number, false if it isn't
BOOL is_number(char *s) 
{
	BOOL tag = FALSE;
	// looking character by character
	// for a number: numerous spaces in front, then optional +/-, then the number
	//               don't care anything that comes after it
	while(*s != '\0') 
	{
		if (*s >= '0' && *s <='9') return TRUE;
		else if (*s == ' ');
		else if (*s != '+' && *s != '-') return FALSE;
		else if ((*s == '+' || *s == '-') && !tag) tag = TRUE;
		else return FALSE;
		s++;
	}
	return FALSE;
}

static void numToStr(double num, char* str)
{
	int i = (int)(num * (opt.NoFrac ? 10 : 100));
	int u = abs(i);

	int r = u % 10;
	int w = u / 10 + (r >= 5);

	if (opt.NoFrac) 
	{
		r = 0;
	}
	else
	{
		r = w % 10;
		w /= 10;
	}

	if (i < 0 && (w || r)) *(str++) = '-';
	if (r)
		sprintf(str, "%i.%i", w, r);
	else
		sprintf(str, "%i", w);
}

//============  UNIT CONVERSIONS  ============

// temperature conversion
// tempchar = the string containing the temperature value
// unit = the unit for temperature
// return value = the converted temperature with degree sign and unit; if fails, return N/A
void GetTemp(char *tempchar, char *unit, char* str) 
{
	// unit can be C, F
	double temp;
	char tstr[20];

	TrimString(tempchar);
	if (tempchar[0] == '-' && tempchar[1] == ' ')
		memmove(&tempchar[1], &tempchar[2], strlen(&tempchar[2])+1);

	// quit if the value obtained is N/A or not a number
	if (!strcmp(tempchar, NODATA) || !strcmp(tempchar, "N/A")) 
	{
		strcpy(str, tempchar);
		return;
	}
	if (!is_number(tempchar))	
	{
		strcpy(str, NODATA);
		return;
	}

	// convert the string to an integer
	temp = atof(tempchar);

	// convert all to F first
	if (!_stricmp(unit, "C"))		temp = (temp*9/5)+32;
	else if (!_stricmp(unit, "K"))	temp = ((temp-273.15)*9/5)+32;

	// convert to apporiate unit
	switch (opt.tUnit) 
	{
	case 1:
		// rounding
		numToStr((temp-32)/9*5, tstr);
		if (opt.DoNotAppendUnit)
			sprintf(str, "%s", tstr);
		else
			sprintf(str, "%s%sC", tstr, opt.DegreeSign);
		break;

	case 2:
		numToStr(temp, tstr);
		if (opt.DoNotAppendUnit)
			sprintf(str, "%s", tstr);
		else
			sprintf(str, "%s%sF", tstr, opt.DegreeSign);
		break;
	}
}

// temperature conversion
// tempchar = the string containing the pressure value
// unit = the unit for pressure
// return value = the converted pressure with unit; if fail, return the original string
void GetPressure(char *tempchar, char *unit, char* str) 
{
	// unit can be kPa, hPa, mb, in, mm, torr
	double tempunit = 0, output;
	int intunit;

	// convert the string to a floating point number (always positive)
	// if it end up with 0, then it's not a number, return the original string and quit
	output = atof(tempchar);
	if (output == 0)
	{
		strcpy(str, tempchar); 
		return;
	}

	// convert all to mb first
	if (!_stricmp(unit, "KPA"))
		tempunit = (double)output * 10;
	else if (!_stricmp(unit, "HPA"))
		tempunit = (double)output;
	else if (!_stricmp(unit, "MB"))
		tempunit = (double)output;
	else if (!_stricmp(unit, "IN"))
		tempunit = (double)output * 33.86388;
	else if (!_stricmp(unit, "MM"))
		tempunit = (double)output * 1.33322;
	else if (!_stricmp(unit, "TORR"))
		tempunit = (double)output * 1.33322;

	// convert to apporiate unit
	switch (opt.pUnit)
	{
	case 1:
		intunit = (int)(tempunit + 0.5);
		wsprintf(str, "%i.%i %s", intunit/10, intunit%10, opt.DoNotAppendUnit ? "" : Translate("kPa"));
		break;
	case 2:
		intunit = (int)(tempunit + 0.5);
		wsprintf(str, "%i %s", intunit, opt.DoNotAppendUnit ? "" : Translate("mb"));
		break;
	case 3:
		intunit = (int)((tempunit*10 / 33.86388) + 0.5);
		wsprintf(str, "%i.%i %s", intunit/10, intunit%10, opt.DoNotAppendUnit ? "" : Translate("in"));
		break;
	case 4:
		intunit = (int)((tempunit*10 / 1.33322) + 0.5);
		wsprintf(str, "%i.%i %s", intunit/10, intunit%10, opt.DoNotAppendUnit ? "" : Translate("mm"));
		break;
	default:
		strcpy(str, tempchar); 
		break;

	}
}

// speed conversion
// tempchar = the string containing the speed value
// unit = the unit for speed
// return value = the converted speed with unit; if fail, return ""
void GetSpeed(char *tempchar, char *unit, char *str) 
{
	// unit can be km/h, mph, m/s, knots
	double tempunit;
	char tstr[20];

	str[0] = 0;

	// convert the string into an integer (always positive)
	// if the result is 0, then the string is not a number, return ""
	tempunit = atof(tempchar);
	if (tempunit == 0 && tempchar[0] != '0') return;

	// convert all to m/s first
	if (!_stricmp(unit, "KM/H"))
		tempunit /= 3.6;
//	else if (!_stricmp(unit, "M/S"))
//		tempunit = tempunit;
	else if (!_stricmp(unit, "MPH"))
		tempunit *= 0.44704;
	else if (!_stricmp(unit, "KNOTS"))
		tempunit *= 0.514444;

	// convert to apporiate unit
	switch (opt.wUnit)
	{
	case 1:
		numToStr(tempunit * 3.6, tstr);
		sprintf(str, "%s %s", tstr, opt.DoNotAppendUnit ? "" : Translate("km/h"));
		break;
	case 2:
		numToStr(tempunit, tstr);
		sprintf(str, "%s %s", tstr, opt.DoNotAppendUnit ? "" : Translate("m/s"));
		break;
	case 3:
		numToStr(tempunit / 0.44704, tstr);
		sprintf(str, "%s %s", tstr, opt.DoNotAppendUnit ? "" : Translate("mph"));
		break;
	case 4:
		numToStr(tempunit / 0.514444, tstr);
		sprintf(str, "%s %s", tstr, opt.DoNotAppendUnit ? "" : Translate("knots"));
		break;
	}
}

// distance conversion
// tempchar = the string containing the distance value
// unit = the unit for distance
// return value = the converted distance with unit; if fail, return original string
void GetDist(char *tempchar, char *unit, char *str) 
{
	// unit can be km, miles
	double tempunit = 0, output;
	int intunit;

	// convert the string to a floating point number (always positive)
	// if it end up with 0, then it's not a number, return the original string and quit
	output = atof(tempchar);
	if (output == 0)
	{
		strcpy(str, tempchar);
		return;
	}

	// convert all to km first
	if (!_stricmp(unit, "KM"))
		tempunit = (double)output;
	else if (!_stricmp(unit, "MILES"))
		tempunit = (double)output * 1.609;

	// convert to apporiate unit
	switch (opt.vUnit)
	{
	case 1:
		intunit = (int)((tempunit*10) + 0.5);
		wsprintf(str, "%i.%i %s", intunit/10, intunit%10, opt.DoNotAppendUnit ? "" : Translate("km"));
		break;
	case 2:
		intunit = (int)((tempunit*10 / 1.609) + 0.5);
		wsprintf(str, "%i.%i %s", intunit/10, intunit%10, opt.DoNotAppendUnit ? "" : Translate("miles"));
		break;
	default:
		strcpy(str, tempchar);
		break;
	}
}

// elevation conversion
// tempchar = the string containing the elevation value
// unit = the unit for elevation
// return value = the converted elevation with unit; if fail, return original string
void GetElev(char *tempchar, char *unit, char *str) 
{
	// unit can be ft, m
	double tempunit = 0, output;
	int intunit;

	// convert the string to a floating point number (always positive)
	// if it end up with 0, then it's not a number, return the original string and quit
	output = atof(tempchar);
	if (output == 0)
	{
		strcpy(str, tempchar);
		return;
	}

	// convert all to m first
	if (!_stricmp(unit, "M"))
		tempunit = (double)output;
	else if (!_stricmp(unit, "FT"))
		tempunit = (double)output / 3.28;

	// convert to apporiate unit
	switch (opt.eUnit) {
		case 1:
			intunit = (int)((tempunit*10 * 3.28) + 0.5);
			wsprintf(str, "%i.%i %s", intunit/10, intunit%10, opt.DoNotAppendUnit ? "" : Translate("ft"));
			break;
		case 2:
			intunit = (int)((tempunit*10) + 0.5);
			wsprintf(str, "%i.%i %s", intunit/10, intunit%10, opt.DoNotAppendUnit ? "" : Translate("m"));
			break;
		default:
			strcpy(str, tempchar);
			break;
	}
}

//============  CONDITION ICON ASSIGNMENT  ============

// assign the contact icon (status) from the condition string
// the description may be different between different sources
// cond = the string for weather condition
// return value = status for the icon (ONLINE, OFFLINE, etc)
WORD GetIcon(const char* cond, WIDATA *Data) 
{
	int i;

	static const 
		char *statusStr[10] = 
	{
		"Lightning",
		"Fog",
		"Snow Shower",
		"Snow",
		"Rain Shower", 
		"Rain",
		"Partly Cloudy",
		"Cloudy",
		"Sunny",
		"N/A"
	};

	static const 
		WORD statusValue[10] = 
	{
		LIGHT,
		FOG,
		SSHOWER,
		SNOW,
		RSHOWER,
		RAIN,
		PCLOUDY,
		CLOUDY,
		SUNNY,
		NA
	};

	// set the icon using ini
	for (i=0; i<10; i++) {
		if (IsContainedInCondList(cond, &Data->CondList[i]))
			return statusValue[i];
	}

	// internal detection
	if (
		strstr(cond, "mainy sunny") != NULL ||
		strstr(cond, "mainy clear") != NULL ||
		strstr(cond, "partly cloudy") != NULL ||
		strstr(cond, "mostly") != NULL ||
		strstr(cond, "clouds") != NULL) {
			return PCLOUDY;
	}
	else if (
		strstr(cond, "sunny") != NULL ||
		strstr(cond, "clear") != NULL ||
		strstr(cond, "fair") != NULL) {
			return SUNNY;
	}
	else if (
		strstr(cond, "thunder") != NULL || 
		strstr(cond, "t-storm") != NULL) {
			return LIGHT;
	}
	else if (
		strstr(cond, "cloud") != NULL ||
		strstr(cond, "overcast") != NULL) {
			return CLOUDY;
	}
	else if (
		strstr(cond, "fog") != NULL ||
		strstr(cond, "mist") != NULL ||
		strstr(cond, "smoke") != NULL ||
		strstr(cond, "sand") != NULL ||
		strstr(cond, "dust") != NULL ||
		strstr(cond, "haze") != NULL) {
			return FOG;
	}
	else if (
		(strstr(cond, "shower") != NULL && strstr(cond, "snow") != NULL) ||
		strstr(cond, "flurries") != NULL) {
			return SSHOWER;
	}
	else if (
		strstr(cond, "rain shower") != NULL ||
		strstr(cond, "shower") != NULL) 
	{
		return RSHOWER;
	}
	else if (
		strstr(cond, "snow") != NULL ||
		strstr(cond, "ice") != NULL ||
		strstr(cond, "freezing") != NULL ||
		strstr(cond, "wintry") != NULL) {
			return SNOW;
	}
	else if (
		strstr(cond, "drizzle") != NULL ||
		strstr(cond, "rain") != NULL) 
	{
		return RAIN;
	}

	// set the icon using langpack
	for (i=0; i<9; i++) 
	{
		char LangPackStr[64];
		char LangPackStr1[128];
		int j = 0;
		do 
		{
			j++;
			// using the format "# Weather <condition name> <counter> #"
			mir_snprintf(LangPackStr, sizeof(LangPackStr), "# Weather %s %i #", statusStr[i], j);
			mir_snprintf(LangPackStr1, sizeof(LangPackStr1), "%s", Translate(LangPackStr));
			CharLowerBuff(LangPackStr1, (DWORD)strlen(LangPackStr1));
			if (strstr(cond, LangPackStr1) != NULL)
				return statusValue[i];
			// loop until the translation string exists (ie, the translated string is differ from original)
		} 
		while (strcmp(Translate(LangPackStr), LangPackStr));
	}

	return NA;
}

//============  STRING CONVERSIONS  ============

// this function convert the string to the format with 1 upper case followed by lower case char
void CaseConv(char *str) 
{
	char *pstr;
	BOOL nextUp = TRUE;

	CharLowerBuff(str, (DWORD)strlen(str));
	for(pstr=str; *pstr; pstr++) 
	{
		if (*pstr==' ' || *pstr=='-')
			nextUp = TRUE;
		else 
		{
			unsigned ch = *(unsigned char*)pstr;
			if (nextUp)	*pstr = (char)(unsigned)CharUpper((LPSTR)ch);
			nextUp = FALSE;
		}
	}
}

// the next 2 functions are copied from miranda source
// str = the string to modify
void TrimString(char *str) 
{
	size_t len, start;

	len = strlen(str);
	while(len && (unsigned char)str[len-1] <= ' ') str[--len] = 0;
	for(start=0; (unsigned char)str[start]<=' ' && str[start]; start++);
	memmove(str, str+start, len-start+1);
}

// convert \t to tab and \n to linefeed
void ConvertBackslashes(char *str) 
{
	char *pstr;
	for(pstr=str;*pstr;pstr=CharNext(pstr)) 
	{
		if(*pstr=='\\') 
		{
			switch(pstr[1]) 
			{
			case 'n': *pstr='\n'; break;
			case 't': *pstr='\t'; break;
			default: *pstr=pstr[1]; break;
			}
			memmove(pstr+1,pstr+2,strlen(pstr+2)+1);
		}
	}
}

// replace spaces with "%20"
// dis = original string
// return value = the modified string with space -> "%20"
char *GetSearchStr(char *dis) 
{
	char *pstr = dis;
	size_t len = strlen(dis);
	while (*pstr != 0)
	{
		if (*pstr == ' ')
		{
			memmove(pstr+3, pstr+1, len);
			memcpy(pstr, "%20", 3);
			pstr += 2;
		}
		pstr++;
		len--;
	}
	return dis;
}

//============  ICON ASSIGNMENT  ============

// make display and history strings
// w = WEATHERINFO data to be parsed
// dis = the string to parse
// return value = the parsed string
char* GetDisplay(WEATHERINFO *w, const char *dis, char* str) 
{
	char lpzDate[32], chr, name[256], temp[2];
	DBVARIANT dbv;
	size_t i;

	// Clear the string
	str[0] = 0;

	// looking character by character
	for (i=0; i<strlen(dis); i++) 
	{
		// for the escape characters
		if (dis[i] == '\\') 
		{
			i++;
			chr = dis[i];
			switch (chr) 
			{
			case '%': strcat(str, "%"); break;
			case 't': strcat(str, "\t"); break;
			case 'n': strcat(str, "\r\n"); break;
			case '\\': strcat(str, "\\"); break;
			}
		}
		// for the % varaibles
		else if (dis[i] == '%') 
		{
			i++;
			chr = dis[i];
			// turn capitalized characters to small case
			if (chr < 'a' && chr != '[' && chr != '%') chr = (char)((int)chr + 32);
			switch (chr) {
			case 'c': strcat(str, w->cond); break;
			case 'd':	// get the current date
				GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, NULL, NULL, lpzDate, sizeof(lpzDate));
				strcat(str, lpzDate); break;
			case 'e': strcat(str, w->dewpoint); break;
			case 'f': strcat(str, w->feel); break;
			case 'h': strcat(str, w->high); break;
			case 'i': strcat(str, w->winddir); break;
			case 'l': strcat(str, w->low); break;
			case 'm': strcat(str, w->humid); break;
			case 'n': strcat(str, w->city); break;
			case 'p': strcat(str, w->pressure); break;
			case 'r': strcat(str, w->sunrise); break;
			case 's': strcat(str, w->id); break;
			case 't': strcat(str, w->temp); break;
			case 'u':
				if (strcmp(w->update, NODATA))	strcat(str, w->update);
				else	strcat(str, Translate("<unknown time>"));
				break;
			case 'v': strcat(str, w->vis); break;
			case 'w': strcat(str, w->wind); break;
			case 'y': strcat(str, w->sunset); break;
			case '%': strcat(str, "%"); break;
			case '[':	// custom variables 
				i++;
				name[0] = 0;
				// read the entire variable name
				while (dis[i] != ']' && i < strlen(dis)) {
					wsprintf(temp, "%c", dis[i++]);
					strcat(name, temp);
				}
				// access the database to get its value
				if (!DBGetContactSettingString(w->hContact, WEATHERCONDITION, name, &dbv))
				{
					if (dbv.pszVal != Translate(NODATA) && dbv.pszVal != Translate("<Error>"))	
						strcat(str, dbv.pszVal);
					DBFreeVariant(&dbv);
				}
				break;
			}
		}
		// if the character is not a variable, write the original character to the new string
		else 
		{
			wsprintf(temp, "%c", dis[i]);
			strcat(str, temp);
		}
	}
	return str;
}

char svcReturnText[MAX_TEXT_SIZE];
INT_PTR GetDisplaySvcFunc(WPARAM wParam, LPARAM lParam) 
{
	WEATHERINFO winfo = LoadWeatherInfo((HANDLE)wParam);
	return (INT_PTR)GetDisplay(&winfo, (char *)lParam, svcReturnText);
}

//============  ID MANAGEMENT  ============

// get service data module internal name
//   mod/id  <- the mod part
// pszID = original 2-part id, return the service internal name
void GetSvc(char *pszID) 
{
	char *chop;

	chop = strstr(pszID, "/");
	if (chop != NULL)	*chop = '\0';
	else				pszID[0] = 0;
}

// get the id use for update without the service internal name
//   mod/id  <- the id part
// pszID = original 2-part id, return the single part id
void GetID(char *pszID) 
{
	char *chop;

	chop = strstr(pszID, "/");
	if (chop != NULL)	strcpy(pszID, chop+1);
	else				pszID[0] = 0;
}

//============  WEATHER ERROR CODE  ============

// Get the text when an error code is specified
// code = the error code obtained when updating weather
// str = the string for the error

char *GetError(int code) 
{
	char *str, str2[100];
	switch (code)
	{
	case 10:	str = E10; break;
	case 11:	str = E11; break;
	case 12:	str = E12; break;
	case 20:	str = E20; break;
	case 30:	str = E30; break;
	case 40:	str = E40; break;
	case 42:	str = E42; break;
	case 43:	str = E43; break;
	case 99:	str = E99; break;
	case 204:	str = E204; break;
	case 301:	str = E301; break;
	case 305:	str = E305; break;
	case 307:	str = E307; break;
	case 400:	str = E400; break;
	case 401:	str = E401; break;
	case 402:	str = E402; break;
	case 403:	str = E403; break;
	case 404:	str = E404; break;
	case 405:	str = E405; break;
	case 407:	str = E407; break;
	case 410:	str = E410; break;
	case 500:	str = E500; break;
	case 502:	str = E502; break;
	case 503:	str = E503; break;
	case 504:	str = E504; break;
	default:
		mir_snprintf(str2, sizeof(str2), Translate("HTTP Error %i"), code);
		str = str2;
		break;
	}
	return str;
}

LPWSTR ConvToUnicode(LPCSTR str2)
{
	const size_t nLength = MultiByteToWideChar(lpcp, 0, str2, -1, NULL, 0);
	LPWSTR res = mir_alloc(sizeof(WCHAR)*nLength);
	MultiByteToWideChar(lpcp, 0, str2, -1, res, (int)nLength);
	return res;
}

typedef BOOL (WINAPI *ft_SetDlgItemTextW) (
	HWND hDlg,
	int nIDDlgItem,
	LPCWSTR lpString
	);

typedef BOOL (WINAPI *ft_SetWindowTextW) (
	HWND hWnd,
	LPCWSTR lpString
	);

typedef UINT (WINAPI *ft_GetDlgItemTextW) (
	HWND hDlg,
	int nIDDlgItem,
	LPWSTR lpString,
	int nMaxCount
	);

static ft_GetDlgItemTextW f_GetDlgItemTextW = NULL;
static ft_SetDlgItemTextW f_SetDlgItemTextW = NULL;
static ft_SetWindowTextW  f_SetWindowTextW = NULL;

unsigned lpcp;

void InitUniConv(void)
{
	HMODULE hUser = GetModuleHandle("user32.dll");
	f_GetDlgItemTextW = (ft_GetDlgItemTextW)GetProcAddress(hUser, "GetDlgItemTextW");
	f_SetDlgItemTextW = (ft_SetDlgItemTextW)GetProcAddress(hUser, "SetDlgItemTextW");
	f_SetWindowTextW  = (ft_SetWindowTextW) GetProcAddress(hUser, "SetWindowTextW");

	lpcp = (unsigned)CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
	if (lpcp == CALLSERVICE_NOTFOUND || lpcp == GetUserDefaultLangID()) 
		lpcp = CP_ACP;
}

UINT GetDlgItemTextWth(HWND hDlg, int nIDDlgItem, LPSTR lpString, int nMaxCount)
{
	UINT res;

	if (lpcp != CP_ACP && f_GetDlgItemTextW != NULL)
	{
		LPWSTR m_psz = mir_alloc(sizeof(WCHAR) * nMaxCount);
		res = f_GetDlgItemTextW(hDlg, nIDDlgItem, m_psz, nMaxCount);
		WideCharToMultiByte( lpcp, 0, m_psz, -1, lpString, nMaxCount, NULL, NULL );
		mir_free(m_psz);
	}
	else
		res = GetDlgItemText(hDlg, nIDDlgItem, lpString, nMaxCount);

	return res;
}

BOOL SetDlgItemTextWth(HWND hDlg, int nIDDlgItem, LPCSTR lpString)
{
	BOOL res;

	if (lpcp != CP_ACP && f_SetDlgItemTextW != NULL)
	{
		LPWSTR m_psz = ConvToUnicode(lpString);
		res = f_SetDlgItemTextW(hDlg, nIDDlgItem, m_psz);
		mir_free(m_psz);
	}
	else
		res = SetDlgItemText(hDlg, nIDDlgItem, lpString);

	return res;
}

BOOL SetWindowTextWth(HWND hWnd, LPCSTR lpString)
{
	BOOL res;

	if (lpcp != CP_ACP && f_SetWindowTextW != NULL)
	{
		LPWSTR m_psz = ConvToUnicode(lpString);
		res = f_SetWindowTextW(hWnd, m_psz);
		mir_free(m_psz);
	}
	else
		res = SetWindowText(hWnd, lpString);

	return res;
}

void ListView_SetItemTextWth(HWND hwndLV, int i, int iSubItem_, LPSTR pszText_)
{
	LV_ITEM _ms_lvi;
	_ms_lvi.iSubItem = iSubItem_;

	if (lpcp != CP_ACP)
	{
		LPWSTR m_psz = ConvToUnicode(pszText_);
		_ms_lvi.pszText = (LPSTR)m_psz;
		SendMessage(hwndLV, LVM_SETITEMTEXTW, (WPARAM)(i), (LPARAM)&_ms_lvi);
		mir_free(m_psz);
	}
	else
	{
		_ms_lvi.pszText = pszText_;
		SendMessage(hwndLV, LVM_SETITEMTEXTA, (WPARAM)(i), (LPARAM)&_ms_lvi);
	}
}

int ListView_InsertItemWth(HWND hwnd, LV_ITEM *pitem)
{
	int res;
	if (lpcp != CP_ACP)
	{
		LPSTR otxt = pitem->pszText;
		LPWSTR m_psz = ConvToUnicode(otxt);
		pitem->pszText = (LPSTR)m_psz;
		res = SendMessage(hwnd, LVM_INSERTITEMW, 0, (LPARAM)pitem);
		mir_free(m_psz);
		pitem->pszText = otxt;
	}
	else
		res = SendMessage(hwnd, LVM_INSERTITEMA, 0, (LPARAM)pitem);

	return res;
}

int ListView_InsertColumnWth(HWND hwnd, int iCol, LV_COLUMN *pitem)
{
	int res;
	if (lpcp != CP_ACP)
	{
		LPSTR otxt = pitem->pszText;
		LPWSTR m_psz = ConvToUnicode(otxt);
		pitem->pszText = (LPSTR)m_psz;
		res = SendMessage(hwnd, LVM_INSERTCOLUMNW, iCol, (LPARAM)pitem);
		mir_free(m_psz);
		pitem->pszText = otxt;
	}
	else
		res = SendMessage(hwnd, LVM_INSERTCOLUMNA, iCol, (LPARAM)pitem);

	return res;
}
