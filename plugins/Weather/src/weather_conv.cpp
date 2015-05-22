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
This file contain the source related unit conversion, icon assignment, 
string conversions, display text parsing, etc
*/

#include "weather.h"

//============  SOME HELPER FUNCTIONS  ============

// see if a string is a number
// s = the string to be determined
// return value = true if the string is a number, false if it isn't
BOOL is_number(TCHAR *s) 
{
	BOOL tag = FALSE;
	// looking character by character
	// for a number: numerous spaces in front, then optional +/-, then the number
	//               don't care anything that comes after it
	while(*s != '\0') 
	{
		if (*s >= '0' && *s <= '9') return TRUE;
		else if (*s == ' ');
		else if (*s != '+' && *s != '-') return FALSE;
		else if ((*s == '+' || *s == '-') && !tag) tag = TRUE;
		else return FALSE;
		s++;
	}
	return FALSE;
}

static void numToStr(double num, TCHAR *str, size_t strSize)
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
		mir_sntprintf(str, strSize, _T("%i.%i"), w, r);
	else
		mir_sntprintf(str, strSize, _T("%i"), w);
}

//============  UNIT CONVERSIONS  ============

// temperature conversion
// tempchar = the string containing the temperature value
// unit = the unit for temperature
// return value = the converted temperature with degree sign and unit; if fails, return N/A
void GetTemp(TCHAR *tempchar, TCHAR *unit, TCHAR* str) 
{
	// unit can be C, F
	double temp;
	TCHAR tstr[20];

	TrimString(tempchar);
	if (tempchar[0] == '-' && tempchar[1] == ' ')
		memmove(&tempchar[1], &tempchar[2], sizeof(TCHAR)*(mir_tstrlen(&tempchar[2])+1));

	// quit if the value obtained is N/A or not a number
	if ( !mir_tstrcmp(tempchar, NODATA) || !mir_tstrcmp(tempchar, _T("N/A"))) {
		_tcscpy(str, tempchar);
		return;
	}
	if ( !is_number(tempchar)) {
		_tcscpy(str, NODATA);
		return;
	}

	// convert the string to an integer
	temp = _ttof(tempchar);

	// convert all to F first
	if ( !mir_tstrcmpi(unit, _T("C")))		temp = (temp*9/5)+32;
	else if ( !mir_tstrcmpi(unit, _T("K")))	temp = ((temp-273.15)*9/5)+32;

	// convert to apporiate unit
	switch (opt.tUnit) {
	case 1:
		// rounding
		numToStr((temp-32)/9*5, tstr, SIZEOF(tstr));
		if (opt.DoNotAppendUnit)
			_tcsncpy_s(str, MAX_DATA_LEN, tstr, _TRUNCATE);
		else
			mir_sntprintf(str, MAX_DATA_LEN, _T("%s%sC"), tstr, opt.DegreeSign);
		break;

	case 2:
		numToStr(temp, tstr, SIZEOF(tstr));
		if (opt.DoNotAppendUnit)
			_tcsncpy_s(str, MAX_DATA_LEN, tstr, _TRUNCATE);
		else
			mir_sntprintf(str, MAX_DATA_LEN, _T("%s%sF"), tstr, opt.DegreeSign);
		break;
	}
}

// temperature conversion
// tempchar = the string containing the pressure value
// unit = the unit for pressure
// return value = the converted pressure with unit; if fail, return the original string
void GetPressure(TCHAR *tempchar, TCHAR *unit, TCHAR* str) 
{
	// unit can be kPa, hPa, mb, in, mm, torr
	double tempunit = 0, output;
	int intunit;

	// convert the string to a floating point number (always positive)
	// if it end up with 0, then it's not a number, return the original string and quit
	output = _ttof(tempchar);
	if (output == 0) {
		_tcscpy(str, tempchar); 
		return;
	}

	// convert all to mb first
	if ( !mir_tstrcmpi(unit, _T("KPA")))
		tempunit = (double)output * 10;
	else if ( !mir_tstrcmpi(unit, _T("HPA")))
		tempunit = (double)output;
	else if ( !mir_tstrcmpi(unit, _T("MB")))
		tempunit = (double)output;
	else if ( !mir_tstrcmpi(unit, _T("IN")))
		tempunit = (double)output * 33.86388;
	else if ( !mir_tstrcmpi(unit, _T("MM")))
		tempunit = (double)output * 1.33322;
	else if ( !mir_tstrcmpi(unit, _T("TORR")))
		tempunit = (double)output * 1.33322;

	// convert to apporiate unit
	switch (opt.pUnit) {
	case 1:
		intunit = (int)(tempunit + 0.5);
		mir_sntprintf(str, MAX_DATA_LEN, _T("%i.%i %s"), intunit/10, intunit%10, opt.DoNotAppendUnit ? _T("") : TranslateT("kPa"));
		break;
	case 2:
		intunit = (int)(tempunit + 0.5);
		mir_sntprintf(str, MAX_DATA_LEN, _T("%i %s"), intunit, opt.DoNotAppendUnit ? _T("") : TranslateT("mb"));
		break;
	case 3:
		intunit = (int)((tempunit*10 / 33.86388) + 0.5);
		mir_sntprintf(str, MAX_DATA_LEN, _T("%i.%i %s"), intunit/10, intunit%10, opt.DoNotAppendUnit ? _T("") : TranslateT("in"));
		break;
	case 4:
		intunit = (int)((tempunit*10 / 1.33322) + 0.5);
		mir_sntprintf(str, MAX_DATA_LEN, _T("%i.%i %s"), intunit/10, intunit%10, opt.DoNotAppendUnit ? _T("") : TranslateT("mm"));
		break;
	default:
		_tcscpy(str, tempchar); 
		break;

	}
}

// speed conversion
// tempchar = the string containing the speed value
// unit = the unit for speed
// return value = the converted speed with unit; if fail, return _T(""
void GetSpeed(TCHAR *tempchar, TCHAR *unit, TCHAR *str) 
{
	// unit can be km/h, mph, m/s, knots
	double tempunit;
	TCHAR tstr[20];

	str[0] = 0;

	// convert the string into an integer (always positive)
	// if the result is 0, then the string is not a number, return _T(""
	tempunit = _ttof(tempchar);
	if (tempunit == 0 && tempchar[0] != '0')
		return;

	// convert all to m/s first
	if ( !mir_tstrcmpi(unit, _T("KM/H")))
		tempunit /= 3.6;
//	else if ( !mir_tstrcmpi(unit, _T("M/S"))
//		tempunit = tempunit;
	else if ( !mir_tstrcmpi(unit, _T("MPH")))
		tempunit *= 0.44704;
	else if ( !mir_tstrcmpi(unit, _T("KNOTS")))
		tempunit *= 0.514444;

	// convert to apporiate unit
	switch (opt.wUnit) {
	case 1:
		numToStr(tempunit * 3.6, tstr, SIZEOF(tstr));
		mir_sntprintf(str, MAX_DATA_LEN, _T("%s %s"), tstr, opt.DoNotAppendUnit ? _T("") : TranslateT("km/h"));
		break;
	case 2:
		numToStr(tempunit, tstr, SIZEOF(tstr));
		mir_sntprintf(str, MAX_DATA_LEN, _T("%s %s"), tstr, opt.DoNotAppendUnit ? _T("") : TranslateT("m/s"));
		break;
	case 3:
		numToStr(tempunit / 0.44704, tstr, SIZEOF(tstr));
		mir_sntprintf(str, MAX_DATA_LEN, _T("%s %s"), tstr, opt.DoNotAppendUnit ? _T("") : TranslateT("mph"));
		break;
	case 4:
		numToStr(tempunit / 0.514444, tstr, SIZEOF(tstr));
		mir_sntprintf(str, MAX_DATA_LEN, _T("%s %s"), tstr, opt.DoNotAppendUnit ? _T("") : TranslateT("knots"));
		break;
	}
}

// distance conversion
// tempchar = the string containing the distance value
// unit = the unit for distance
// return value = the converted distance with unit; if fail, return original string
void GetDist(TCHAR *tempchar, TCHAR *unit, TCHAR *str) 
{
	// unit can be km, miles
	double tempunit = 0, output;
	int intunit;

	// convert the string to a floating point number (always positive)
	// if it end up with 0, then it's not a number, return the original string and quit
	output = _ttof(tempchar);
	if (output == 0) {
		_tcscpy(str, tempchar);
		return;
	}

	// convert all to km first
	if ( !mir_tstrcmpi(unit, _T("KM")))
		tempunit = (double)output;
	else if ( !mir_tstrcmpi(unit, _T("MILES")))
		tempunit = (double)output * 1.609;

	// convert to apporiate unit
	switch (opt.vUnit) {
	case 1:
		intunit = (int)((tempunit*10) + 0.5);
		mir_sntprintf(str, MAX_DATA_LEN, _T("%i.%i %s"), intunit/10, intunit%10, opt.DoNotAppendUnit ? _T("") : TranslateT("km"));
		break;
	case 2:
		intunit = (int)((tempunit*10 / 1.609) + 0.5);
		mir_sntprintf(str, MAX_DATA_LEN, _T("%i.%i %s"), intunit/10, intunit%10, opt.DoNotAppendUnit ? _T("") : TranslateT("miles"));
		break;
	default:
		_tcscpy(str, tempchar);
		break;
	}
}

// elevation conversion
// tempchar = the string containing the elevation value
// unit = the unit for elevation
// return value = the converted elevation with unit; if fail, return original string
void GetElev(TCHAR *tempchar, TCHAR *unit, TCHAR *str) 
{
	// unit can be ft, m
	double tempunit = 0, output;
	int intunit;

	// convert the string to a floating point number (always positive)
	// if it end up with 0, then it's not a number, return the original string and quit
	output = _ttof(tempchar);
	if (output == 0) {
		_tcscpy(str, tempchar);
		return;
	}

	// convert all to m first
	if ( !mir_tstrcmpi(unit, _T("M")))
		tempunit = (double)output;
	else if ( !mir_tstrcmpi(unit, _T("FT")))
		tempunit = (double)output / 3.28;

	// convert to apporiate unit
	switch (opt.eUnit) {
	case 1:
		intunit = (int)((tempunit*10 * 3.28) + 0.5);
		mir_sntprintf(str, MAX_DATA_LEN, _T("%i.%i %s"), intunit/10, intunit%10, opt.DoNotAppendUnit ? _T("") : TranslateT("ft"));
		break;
	case 2:
		intunit = (int)((tempunit*10) + 0.5);
		mir_sntprintf(str, MAX_DATA_LEN, _T("%i.%i %s"), intunit/10, intunit%10, opt.DoNotAppendUnit ? _T("") : TranslateT("m"));
		break;
	default:
		_tcscpy(str, tempchar);
		break;
	}
}

//============  CONDITION ICON ASSIGNMENT  ============

// assign the contact icon (status) from the condition string
// the description may be different between different sources
// cond = the string for weather condition
// return value = status for the icon (ONLINE, OFFLINE, etc)

static const TCHAR *statusStr[10] = { _T("Lightning"), _T("Fog"), _T("Snow Shower"), _T("Snow"), _T("Rain Shower"), _T("Rain"), _T("Partly Cloudy"), _T("Cloudy"), _T("Sunny"), _T("N/A") };
static const WORD statusValue[10] = { LIGHT, FOG, SSHOWER, SNOW, RSHOWER, RAIN, PCLOUDY, CLOUDY, SUNNY, NA };

WORD GetIcon(const TCHAR* cond, WIDATA *Data) 
{
	// set the icon using ini
	for (int i=0; i<10; i++)
		if ( IsContainedInCondList(cond, &Data->CondList[i]))
			return statusValue[i];

	// internal detection
	if (
		_tcsstr(cond, _T("mainy sunny")) != NULL ||
		_tcsstr(cond, _T("mainy clear")) != NULL ||
		_tcsstr(cond, _T("partly cloudy")) != NULL ||
		_tcsstr(cond, _T("mostly")) != NULL ||
		_tcsstr(cond, _T("clouds")) != NULL) {
			return PCLOUDY;
	}
	else if (
		_tcsstr(cond, _T("sunny")) != NULL ||
		_tcsstr(cond, _T("clear")) != NULL ||
		_tcsstr(cond, _T("fair")) != NULL) {
			return SUNNY;
	}
	else if (
		_tcsstr(cond, _T("thunder")) != NULL || 
		_tcsstr(cond, _T("t-storm")) != NULL) {
			return LIGHT;
	}
	else if (
		_tcsstr(cond, _T("cloud")) != NULL ||
		_tcsstr(cond, _T("overcast")) != NULL) {
			return CLOUDY;
	}
	else if (
		_tcsstr(cond, _T("fog")) != NULL ||
		_tcsstr(cond, _T("mist")) != NULL ||
		_tcsstr(cond, _T("smoke")) != NULL ||
		_tcsstr(cond, _T("sand")) != NULL ||
		_tcsstr(cond, _T("dust")) != NULL ||
		_tcsstr(cond, _T("haze")) != NULL) {
			return FOG;
	}
	else if (
		(_tcsstr(cond, _T("shower")) != NULL && _tcsstr(cond, _T("snow")) != NULL) ||
		_tcsstr(cond, _T("flurries")) != NULL) {
			return SSHOWER;
	}
	else if (
		_tcsstr(cond, _T("rain shower")) != NULL ||
		_tcsstr(cond, _T("shower")) != NULL) 
	{
		return RSHOWER;
	}
	else if (
		_tcsstr(cond, _T("snow")) != NULL ||
		_tcsstr(cond, _T("ice")) != NULL ||
		_tcsstr(cond, _T("freezing")) != NULL ||
		_tcsstr(cond, _T("wintry")) != NULL) {
			return SNOW;
	}
	else if (
		_tcsstr(cond, _T("drizzle")) != NULL ||
		_tcsstr(cond, _T("rain")) != NULL) 
	{
		return RAIN;
	}

	// set the icon using langpack
	for (int i=0; i < 9; i++) {
		TCHAR LangPackStr[64], LangPackStr1[128];
		int j = 0;
		do {
			j++;
			// using the format _T("# Weather <condition name> <counter> #"
			mir_sntprintf(LangPackStr, SIZEOF(LangPackStr), _T("# Weather %s %i #"), statusStr[i], j);
			_tcsncpy_s(LangPackStr1, TranslateTS(LangPackStr), _TRUNCATE);
			CharLowerBuff(LangPackStr1, (DWORD)mir_tstrlen(LangPackStr1));
			if (_tcsstr(cond, LangPackStr1) != NULL)
				return statusValue[i];
			// loop until the translation string exists (ie, the translated string is differ from original)
		} 
			while (mir_tstrcmp(TranslateTS(LangPackStr), LangPackStr));
	}

	return NA;
}

//============  STRING CONVERSIONS  ============

// this function convert the string to the format with 1 upper case followed by lower case char
void CaseConv(TCHAR *str) 
{
	TCHAR *pstr;
	BOOL nextUp = TRUE;

	CharLowerBuff(str, (DWORD)mir_tstrlen(str));
	for(pstr = str; *pstr; pstr++) {
		if (*pstr == ' ' || *pstr == '-')
			nextUp = TRUE;
		else {
			TCHAR ch = *(TCHAR*)pstr;
			if (nextUp)
				*pstr = ( TCHAR )CharUpper((LPTSTR)ch);
			nextUp = FALSE;
}	}	}

// the next 2 functions are copied from miranda source
// str = the string to modify

void TrimString(char *str) 
{
	size_t len, start;

	len = mir_strlen(str);
	while(len && (unsigned char)str[len-1] <= ' ') str[--len] = 0;
	for(start=0; (unsigned char)str[start] <= ' ' && str[start]; start++);
	memmove(str, str+start, len-start+1);
}

void TrimString(WCHAR *str) 
{
	size_t len, start;

	len = wcslen(str);
	while(len && (unsigned char)str[len-1] <= ' ') str[--len] = 0;
	for(start=0; (unsigned char)str[start] <= ' ' && str[start]; start++);
	memmove(str, str+start, (len-start+1)*sizeof(WCHAR));
}

// convert \t to tab and \n to linefeed
void ConvertBackslashes(char *str) 
{
	for (char *pstr=str; *pstr; pstr = CharNextA(pstr)) {
		if (*pstr == '\\') {
			switch(pstr[1]) {
				case 'n': *pstr = '\n'; break;
				case 't': *pstr = '\t'; break;
				default: *pstr = pstr[1]; break;
			}
			memmove(pstr+1, pstr+2, mir_strlen(pstr+2)+1);
}	}	}

// replace spaces with _T("%20"
// dis = original string
// return value = the modified string with space -> _T("%20"
char *GetSearchStr(char *dis) 
{
	char *pstr = dis;
	size_t len = mir_strlen(dis);
	while (*pstr != 0)
	{
		if (*pstr == ' ')
		{
			memmove(pstr+3, pstr+1, len);
			memcpy(pstr, _T("%20"), 3);
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
TCHAR* GetDisplay(WEATHERINFO *w, const TCHAR *dis, TCHAR* str) 
{
	TCHAR lpzDate[32], chr;
	char name[256], temp[2];
	DBVARIANT dbv;
	size_t i;

	// Clear the string
	str[0] = 0;

	// looking character by character
	for (i=0; i < mir_tstrlen(dis); i++) {
		// for the escape characters
		if (dis[i] == '\\') {
			i++;
			chr = dis[i];
			switch (chr) {
				case '%': _tcscat(str, _T("%")); break;
				case 't': _tcscat(str, _T("\t")); break;
				case 'n': _tcscat(str, _T("\r\n")); break;
				case '\\': _tcscat(str, _T("\\")); break;
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
			case 'c': _tcscat(str, w->cond); break;
			case 'd':	// get the current date
				GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, NULL, NULL, lpzDate, SIZEOF(lpzDate));
				_tcscat(str, lpzDate); break;
			case 'e': _tcscat(str, w->dewpoint); break;
			case 'f': _tcscat(str, w->feel); break;
			case 'h': _tcscat(str, w->high); break;
			case 'i': _tcscat(str, w->winddir); break;
			case 'l': _tcscat(str, w->low); break;
			case 'm': _tcscat(str, w->humid); break;
			case 'n': _tcscat(str, w->city); break;
			case 'p': _tcscat(str, w->pressure); break;
			case 'r': _tcscat(str, w->sunrise); break;
			case 's': _tcscat(str, w->id); break;
			case 't': _tcscat(str, w->temp); break;
			case 'u':
				if (mir_tstrcmp(w->update, NODATA))	_tcscat(str, w->update);
				else	_tcscat(str, TranslateT("<unknown time>"));
				break;
			case 'v': _tcscat(str, w->vis); break;
			case 'w': _tcscat(str, w->wind); break;
			case 'y': _tcscat(str, w->sunset); break;
			case '%': _tcscat(str, _T("%")); break;
			case '[':	// custom variables 
				i++;
				name[0] = 0;
				// read the entire variable name
				while (dis[i] != ']' && i < mir_tstrlen(dis)) {
					mir_snprintf(temp, SIZEOF(temp), "%c", dis[i++]);
					strcat(name, temp);
				}
				// access the database to get its value
				if ( !db_get_ts(w->hContact, WEATHERCONDITION, name, &dbv)) {
					if (dbv.ptszVal != TranslateTS(NODATA) && dbv.ptszVal != TranslateT("<Error>"))	
						_tcscat(str, dbv.ptszVal);
					db_free(&dbv);
				}
				break;
			}
		}
		// if the character is not a variable, write the original character to the new string
		else {
			mir_sntprintf(lpzDate, SIZEOF(lpzDate), _T("%c"), dis[i]);
			_tcscat(str, lpzDate);
	}	}

	return str;
}

TCHAR svcReturnText[MAX_TEXT_SIZE];
INT_PTR GetDisplaySvcFunc(WPARAM wParam, LPARAM lParam) 
{
	WEATHERINFO winfo = LoadWeatherInfo(wParam);
	return (INT_PTR)GetDisplay(&winfo, (TCHAR*)lParam, svcReturnText);
}

//============  ID MANAGEMENT  ============

// get service data module internal name
//   mod/id  <- the mod part
// pszID = original 2-part id, return the service internal name
void GetSvc(TCHAR *pszID) 
{
	TCHAR *chop = _tcsstr(pszID, _T("/"));
	if (chop != NULL)	*chop = '\0';
	else				pszID[0] = 0;
}

// get the id use for update without the service internal name
//   mod/id  <- the id part
// pszID = original 2-part id, return the single part id
void GetID(TCHAR *pszID) 
{
	TCHAR *chop = _tcsstr(pszID, _T("/"));
	if (chop != NULL)	_tcscpy(pszID, chop+1);
	else				pszID[0] = 0;
}

//============  WEATHER ERROR CODE  ============

// Get the text when an error code is specified
// code = the error code obtained when updating weather
// str = the string for the error

TCHAR *GetError(int code) 
{
	TCHAR *str, str2[100];
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
		mir_sntprintf(str2, SIZEOF(str2), TranslateT("HTTP Error %i"), code);
		str = str2;
		break;
	}
	return mir_tstrdup(str);
}
