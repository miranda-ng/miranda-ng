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
This file contain the source related unit conversion, icon assignment,
string conversions, display text parsing, etc
*/

#include "stdafx.h"

//============  SOME HELPER FUNCTIONS  ============

// see if a string is a number
// s = the string to be determined
// return value = true if the string is a number, false if it isn't
BOOL is_number(wchar_t *s)
{
	BOOL tag = FALSE;
	// looking character by character
	// for a number: numerous spaces in front, then optional +/-, then the number
	//               don't care anything that comes after it
	while (*s != '\0') {
		if (*s >= '0' && *s <= '9') return TRUE;
		else if (*s == ' ');
		else if (*s != '+' && *s != '-') return FALSE;
		else if ((*s == '+' || *s == '-') && !tag) tag = TRUE;
		else return FALSE;
		s++;
	}
	return FALSE;
}

static void numToStr(double num, wchar_t *str, size_t strSize)
{
	int i = (int)(num * (opt.NoFrac ? 10 : 100));
	int u = abs(i);

	int r = u % 10;
	int w = u / 10 + (r >= 5);

	if (opt.NoFrac)
		r = 0;
	else {
		r = w % 10;
		w /= 10;
	}

	if (i < 0 && (w || r)) w = -w;
	if (r)
		mir_snwprintf(str, strSize, L"%i.%i", w, r);
	else
		mir_snwprintf(str, strSize, L"%i", w);
}

//============  UNIT CONVERSIONS  ============

// temperature conversion
// tempchar = the string containing the temperature value
// unit = the unit for temperature
// return value = the converted temperature with degree sign and unit; if fails, return N/A
void GetTemp(wchar_t *tempchar, wchar_t *unit, wchar_t *str)
{
	// unit can be C, F
	double temp;
	wchar_t tstr[20];

	TrimString(tempchar);
	if (tempchar[0] == '-' && tempchar[1] == ' ')
		memmove(&tempchar[1], &tempchar[2], sizeof(wchar_t) * (mir_wstrlen(&tempchar[2]) + 1));

	// quit if the value obtained is N/A or not a number
	if (!mir_wstrcmp(tempchar, NODATA) || !mir_wstrcmp(tempchar, L"N/A")) {
		mir_wstrcpy(str, tempchar);
		return;
	}
	if (!is_number(tempchar)) {
		mir_wstrcpy(str, NODATA);
		return;
	}

	// convert the string to an integer
	temp = _wtof(tempchar);

	// convert all to F first
	if (!mir_wstrcmpi(unit, L"C"))		temp = (temp * 9 / 5) + 32;
	else if (!mir_wstrcmpi(unit, L"K"))	temp = ((temp - 273.15) * 9 / 5) + 32;

	// convert to apporiate unit
	switch (opt.tUnit) {
	case 1:
		// rounding
		numToStr((temp - 32) / 9 * 5, tstr, _countof(tstr));
		if (opt.DoNotAppendUnit)
			wcsncpy_s(str, MAX_DATA_LEN, tstr, _TRUNCATE);
		else
			mir_snwprintf(str, MAX_DATA_LEN, L"%s%sC", tstr, opt.DegreeSign);
		break;

	case 2:
		numToStr(temp, tstr, _countof(tstr));
		if (opt.DoNotAppendUnit)
			wcsncpy_s(str, MAX_DATA_LEN, tstr, _TRUNCATE);
		else
			mir_snwprintf(str, MAX_DATA_LEN, L"%s%sF", tstr, opt.DegreeSign);
		break;
	}
}

// temperature conversion
// tempchar = the string containing the pressure value
// unit = the unit for pressure
// return value = the converted pressure with unit; if fail, return the original string
void GetPressure(wchar_t *tempchar, wchar_t *unit, wchar_t *str)
{
	// unit can be kPa, hPa, mb, in, mm, torr
	double tempunit = 0, output;
	int intunit;

	// convert the string to a floating point number (always positive)
	// if it end up with 0, then it's not a number, return the original string and quit
	output = _wtof(tempchar);
	if (output == 0) {
		mir_wstrcpy(str, tempchar);
		return;
	}

	// convert all to mb first
	if (!mir_wstrcmpi(unit, L"KPA"))
		tempunit = (double)output * 10;
	else if (!mir_wstrcmpi(unit, L"HPA"))
		tempunit = (double)output;
	else if (!mir_wstrcmpi(unit, L"MB"))
		tempunit = (double)output;
	else if (!mir_wstrcmpi(unit, L"IN"))
		tempunit = (double)output * 33.86388;
	else if (!mir_wstrcmpi(unit, L"MM"))
		tempunit = (double)output * 1.33322;
	else if (!mir_wstrcmpi(unit, L"TORR"))
		tempunit = (double)output * 1.33322;

	// convert to apporiate unit
	switch (opt.pUnit) {
	case 1:
		intunit = (int)(tempunit + 0.5);
		mir_snwprintf(str, MAX_DATA_LEN, L"%i.%i %s", intunit / 10, intunit % 10, opt.DoNotAppendUnit ? L"" : TranslateT("kPa"));
		break;
	case 2:
		intunit = (int)(tempunit + 0.5);
		mir_snwprintf(str, MAX_DATA_LEN, L"%i %s", intunit, opt.DoNotAppendUnit ? L"" : TranslateT("mb"));
		break;
	case 3:
		intunit = (int)((tempunit * 10 / 33.86388) + 0.5);
		mir_snwprintf(str, MAX_DATA_LEN, L"%i.%i %s", intunit / 10, intunit % 10, opt.DoNotAppendUnit ? L"" : TranslateT("in"));
		break;
	case 4:
		intunit = (int)((tempunit * 10 / 1.33322) + 0.5);
		mir_snwprintf(str, MAX_DATA_LEN, L"%i.%i %s", intunit / 10, intunit % 10, opt.DoNotAppendUnit ? L"" : TranslateT("mm"));
		break;
	default:
		mir_wstrcpy(str, tempchar);
		break;

	}
}

// speed conversion
// tempchar = the string containing the speed value
// unit = the unit for speed
// return value = the converted speed with unit; if fail, return _T(""
void GetSpeed(wchar_t *tempchar, wchar_t *unit, wchar_t *str)
{
	// unit can be km/h, mph, m/s, knots
	double tempunit;
	wchar_t tstr[20];

	str[0] = 0;

	// convert the string into an integer (always positive)
	// if the result is 0, then the string is not a number, return _T(""
	tempunit = _wtof(tempchar);
	if (tempunit == 0 && tempchar[0] != '0')
		return;

	// convert all to m/s first
	if (!mir_wstrcmpi(unit, L"KM/H"))
		tempunit /= 3.6;
	//	else if ( !mir_wstrcmpi(unit, L"M/S")
	//		tempunit = tempunit;
	else if (!mir_wstrcmpi(unit, L"MPH"))
		tempunit *= 0.44704;
	else if (!mir_wstrcmpi(unit, L"KNOTS"))
		tempunit *= 0.514444;

	// convert to apporiate unit
	switch (opt.wUnit) {
	case 1:
		numToStr(tempunit * 3.6, tstr, _countof(tstr));
		mir_snwprintf(str, MAX_DATA_LEN, L"%s %s", tstr, opt.DoNotAppendUnit ? L"" : TranslateT("km/h"));
		break;
	case 2:
		numToStr(tempunit, tstr, _countof(tstr));
		mir_snwprintf(str, MAX_DATA_LEN, L"%s %s", tstr, opt.DoNotAppendUnit ? L"" : TranslateT("m/s"));
		break;
	case 3:
		numToStr(tempunit / 0.44704, tstr, _countof(tstr));
		mir_snwprintf(str, MAX_DATA_LEN, L"%s %s", tstr, opt.DoNotAppendUnit ? L"" : TranslateT("mph"));
		break;
	case 4:
		numToStr(tempunit / 0.514444, tstr, _countof(tstr));
		mir_snwprintf(str, MAX_DATA_LEN, L"%s %s", tstr, opt.DoNotAppendUnit ? L"" : TranslateT("knots"));
		break;
	}
}

// distance conversion
// tempchar = the string containing the distance value
// unit = the unit for distance
// return value = the converted distance with unit; if fail, return original string
void GetDist(wchar_t *tempchar, wchar_t *unit, wchar_t *str)
{
	// unit can be km, miles
	double tempunit = 0, output;
	int intunit;

	// convert the string to a floating point number (always positive)
	// if it end up with 0, then it's not a number, return the original string and quit
	output = _wtof(tempchar);
	if (output == 0) {
		mir_wstrcpy(str, tempchar);
		return;
	}

	// convert all to km first
	if (!mir_wstrcmpi(unit, L"KM"))
		tempunit = (double)output;
	else if (!mir_wstrcmpi(unit, L"MILES"))
		tempunit = (double)output * 1.609;

	// convert to apporiate unit
	switch (opt.vUnit) {
	case 1:
		intunit = (int)((tempunit * 10) + 0.5);
		mir_snwprintf(str, MAX_DATA_LEN, L"%i.%i %s", intunit / 10, intunit % 10, opt.DoNotAppendUnit ? L"" : TranslateT("km"));
		break;
	case 2:
		intunit = (int)((tempunit * 10 / 1.609) + 0.5);
		mir_snwprintf(str, MAX_DATA_LEN, L"%i.%i %s", intunit / 10, intunit % 10, opt.DoNotAppendUnit ? L"" : TranslateT("miles"));
		break;
	default:
		mir_wstrcpy(str, tempchar);
		break;
	}
}

// elevation conversion
// tempchar = the string containing the elevation value
// unit = the unit for elevation
// return value = the converted elevation with unit; if fail, return original string
void GetElev(wchar_t *tempchar, wchar_t *unit, wchar_t *str)
{
	// unit can be ft, m
	double tempunit = 0, output;
	int intunit;

	// convert the string to a floating point number (always positive)
	// if it end up with 0, then it's not a number, return the original string and quit
	output = _wtof(tempchar);
	if (output == 0) {
		mir_wstrcpy(str, tempchar);
		return;
	}

	// convert all to m first
	if (!mir_wstrcmpi(unit, L"M"))
		tempunit = (double)output;
	else if (!mir_wstrcmpi(unit, L"FT"))
		tempunit = (double)output / 3.28;

	// convert to apporiate unit
	switch (opt.eUnit) {
	case 1:
		intunit = (int)((tempunit * 10 * 3.28) + 0.5);
		mir_snwprintf(str, MAX_DATA_LEN, L"%i.%i %s", intunit / 10, intunit % 10, opt.DoNotAppendUnit ? L"" : TranslateT("ft"));
		break;
	case 2:
		intunit = (int)((tempunit * 10) + 0.5);
		mir_snwprintf(str, MAX_DATA_LEN, L"%i.%i %s", intunit / 10, intunit % 10, opt.DoNotAppendUnit ? L"" : TranslateT("m"));
		break;
	default:
		mir_wstrcpy(str, tempchar);
		break;
	}
}

//============  CONDITION ICON ASSIGNMENT  ============

// assign the contact icon (status) from the condition string
// the description may be different between different sources
// cond = the string for weather condition
// return value = status for the icon (ONLINE, OFFLINE, etc)

static const wchar_t *statusStr[MAX_COND] = { L"Lightning", L"Fog", L"Snow", L"Rain", L"Partly Cloudy", L"Cloudy", L"Sunny", L"N/A", L"Rain Shower", L"Snow Shower"};
static const WORD statusValue[MAX_COND] = { LIGHT, FOG, SNOW, RAIN, PCLOUDY, CLOUDY, SUNNY, NA, RSHOWER, SSHOWER };

WORD GetIcon(const wchar_t *cond, WIDATA *Data)
{
	// set the icon using ini
	for (int i = 0; i < _countof(statusValue); i++)
		if (IsContainedInCondList(cond, &Data->CondList[i]))
			return statusValue[i];

	// internal detection
	if (wcsstr(cond, L"mainy sunny") || wcsstr(cond, L"mainy clear") || wcsstr(cond, L"partly sunny") || wcsstr(cond, L"partly cloudy") || wcsstr(cond, L"mostly") || wcsstr(cond, L"clouds"))
		return PCLOUDY;

	if (wcsstr(cond, L"sunny") || wcsstr(cond, L"clear") || wcsstr(cond, L"fair"))
		return SUNNY;

	if (wcsstr(cond, L"thunder") || wcsstr(cond, L"t-storm"))
		return LIGHT;

	if (wcsstr(cond, L"cloud") || wcsstr(cond, L"overcast"))
		return CLOUDY;

	if (wcsstr(cond, L"fog") || wcsstr(cond, L"mist") || wcsstr(cond, L"smoke") || wcsstr(cond, L"sand") || wcsstr(cond, L"dust") || wcsstr(cond, L"haze"))
		return FOG;

	if (wcsstr(cond, L"snow shower"))
		return SSHOWER;

	if (wcsstr(cond, L"snow") || wcsstr(cond, L"ice") || wcsstr(cond, L"freezing") || wcsstr(cond, L"wintry"))
		return SNOW;

	if (wcsstr(cond, L"rain shower"))
		return RSHOWER;

	if (wcsstr(cond, L"drizzle") || wcsstr(cond, L"rain"))
		return RAIN;

	// set the icon using langpack
	for (int i = 0; i < _countof(statusStr)-1; i++) {
		wchar_t LangPackStr[64], LangPackStr1[128];
		int j = 0;
		do {
			j++;
			// using the format _T("# Weather <condition name> <counter> #"
			mir_snwprintf(LangPackStr, L"# Weather %s %i #", statusStr[i], j);
			wcsncpy_s(LangPackStr1, TranslateW(LangPackStr), _TRUNCATE);
			CharLowerBuff(LangPackStr1, (DWORD)mir_wstrlen(LangPackStr1));
			if (wcsstr(cond, LangPackStr1) != nullptr)
				return statusValue[i];
			// loop until the translation string exists (ie, the translated string is differ from original)
		} while (mir_wstrcmp(TranslateW(LangPackStr), LangPackStr));
	}

	return NA;
}

//============  STRING CONVERSIONS  ============
//
// this function convert the string to the format with 1 upper case followed by lower case char
void CaseConv(wchar_t *str)
{
	bool nextUp = true;

	CharLowerBuffW(str, (DWORD)mir_wstrlen(str));
	for (wchar_t *pstr = str; *pstr; pstr++) {
		if (*pstr == ' ' || *pstr == '-')
			nextUp = true;
		else if (nextUp) {
			CharUpperBuffW(pstr, 1);
			nextUp = false;
		}
	}
}

// the next 2 functions are copied from miranda source
// str = the string to modify
//
void TrimString(char *str)
{
	size_t len, start;

	len = mir_strlen(str);
	while (len && (unsigned char)str[len - 1] <= ' ') str[--len] = 0;
	for (start = 0; (unsigned char)str[start] <= ' ' && str[start]; start++);
	memmove(str, str + start, len - start + 1);
}

void TrimString(WCHAR *str)
{
	size_t len, start;

	len = mir_wstrlen(str);
	while (len && (unsigned char)str[len - 1] <= ' ') str[--len] = 0;
	for (start = 0; (unsigned char)str[start] <= ' ' && str[start]; start++);
	memmove(str, str + start, (len - start + 1) * sizeof(WCHAR));
}

// convert \t to tab and \n to linefeed
void ConvertBackslashes(char *str)
{
	for (char *pstr = str; *pstr; pstr = CharNextA(pstr)) {
		if (*pstr == '\\') {
			switch (pstr[1]) {
			case 'n': *pstr = '\n'; break;
			case 't': *pstr = '\t'; break;
			default: *pstr = pstr[1]; break;
			}
			memmove(pstr + 1, pstr + 2, mir_strlen(pstr + 2) + 1);
		}
	}
}

// replace spaces with _T("%20"
// dis = original string
// return value = the modified string with space -> _T("%20"
char *GetSearchStr(char *dis)
{
	char *pstr = dis;
	size_t len = mir_strlen(dis);
	while (*pstr != 0) {
		if (*pstr == ' ') {
			memmove(pstr + 3, pstr + 1, len);
			memcpy(pstr, L"%20", 3);
			pstr += 2;
		}
		pstr++;
		len--;
	}
	return dis;
}

//============  ICON ASSIGNMENT  ============
//
// make display and history strings
// w = WEATHERINFO data to be parsed
// dis = the string to parse
// return value = the parsed string
wchar_t *GetDisplay(WEATHERINFO *w, const wchar_t *dis, wchar_t *str)
{
	wchar_t lpzDate[32], chr;
	char name[256], temp[2];
	DBVARIANT dbv;
	size_t i;

	// Clear the string
	str[0] = 0;

	// looking character by character
	for (i = 0; i < mir_wstrlen(dis); i++) {
		// for the escape characters
		if (dis[i] == '\\') {
			i++;
			chr = dis[i];
			switch (chr) {
			case '%': mir_wstrcat(str, L"%"); break;
			case 't': mir_wstrcat(str, L"\t"); break;
			case 'n': mir_wstrcat(str, L"\r\n"); break;
			case '\\': mir_wstrcat(str, L"\\"); break;
			}
		}

		// for the % varaibles
		else if (dis[i] == '%') {
			i++;
			chr = dis[i];
			// turn capitalized characters to small case
			if (chr < 'a' && chr != '[' && chr != '%') chr = (char)((int)chr + 32);
			switch (chr) {
			case 'c': mir_wstrcat(str, w->cond); break;
			case 'd':	// get the current date
				GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, nullptr, nullptr, lpzDate, _countof(lpzDate));
				mir_wstrcat(str, lpzDate); break;
			case 'e': mir_wstrcat(str, w->dewpoint); break;
			case 'f': mir_wstrcat(str, w->feel); break;
			case 'h': mir_wstrcat(str, w->high); break;
			case 'i': mir_wstrcat(str, w->winddir); break;
			case 'l': mir_wstrcat(str, w->low); break;
			case 'm': mir_wstrcat(str, w->humid); break;
			case 'n': mir_wstrcat(str, w->city); break;
			case 'p': mir_wstrcat(str, w->pressure); break;
			case 'r': mir_wstrcat(str, w->sunrise); break;
			case 's': mir_wstrcat(str, w->id); break;
			case 't': mir_wstrcat(str, w->temp); break;
			case 'u':
				if (mir_wstrcmp(w->update, NODATA))	mir_wstrcat(str, w->update);
				else	mir_wstrcat(str, TranslateT("<unknown time>"));
				break;
			case 'v': mir_wstrcat(str, w->vis); break;
			case 'w': mir_wstrcat(str, w->wind); break;
			case 'y': mir_wstrcat(str, w->sunset); break;
			case '%': mir_wstrcat(str, L"%"); break;
			case '[':	// custom variables 
				i++;
				name[0] = 0;
				// read the entire variable name
				while (dis[i] != ']' && i < mir_wstrlen(dis)) {
					mir_snprintf(temp, "%c", dis[i++]);
					mir_strcat(name, temp);
				}
				// access the database to get its value
				if (!db_get_ws(w->hContact, WEATHERCONDITION, name, &dbv)) {
					if (dbv.pwszVal != TranslateW(NODATA) && dbv.pwszVal != TranslateT("<Error>"))
						mir_wstrcat(str, dbv.pwszVal);
					db_free(&dbv);
				}
				break;
			}
		}
		// if the character is not a variable, write the original character to the new string
		else {
			mir_snwprintf(lpzDate, L"%c", dis[i]);
			mir_wstrcat(str, lpzDate);
		}
	}

	return str;
}

wchar_t svcReturnText[MAX_TEXT_SIZE];
INT_PTR GetDisplaySvcFunc(WPARAM wParam, LPARAM lParam)
{
	WEATHERINFO winfo = LoadWeatherInfo(wParam);
	return (INT_PTR)GetDisplay(&winfo, (wchar_t*)lParam, svcReturnText);
}

//============  ID MANAGEMENT  ============
//
// get service data module internal name
//   mod/id  <- the mod part
// pszID = original 2-part id, return the service internal name
void GetSvc(wchar_t *pszID)
{
	wchar_t *chop = wcschr(pszID, '/');
	if (chop != nullptr)
		*chop = '\0';
	else
		pszID[0] = 0;
}

// get the id use for update without the service internal name
//   mod/id  <- the id part
// pszID = original 2-part id, return the single part id
void GetID(wchar_t *pszID)
{
	wchar_t *chop = wcschr(pszID, '/');
	if (chop != nullptr)
		mir_wstrcpy(pszID, chop + 1);
	else
		pszID[0] = 0;
}

//============  WEATHER ERROR CODE  ============
//
// Get the text when an error code is specified
// code = the error code obtained when updating weather
// str = the string for the error
//
wchar_t *GetError(int code)
{
	wchar_t *str, str2[100];
	switch (code) {
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
		mir_snwprintf(str2, TranslateT("HTTP Error %i"), code);
		str = str2;
		break;
	}
	return mir_wstrdup(str);
}
