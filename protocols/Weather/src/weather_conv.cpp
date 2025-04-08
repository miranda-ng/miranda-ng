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

/////////////////////////////////////////////////////////////////////////////////////////
// see if a string is a number
// s = the string to be determined
// return value = true if the string is a number, false if it isn't

BOOL is_number(const wchar_t *s)
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

void CWeatherProto::numToStr(double num, wchar_t *str, size_t strSize)
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

/////////////////////////////////////////////////////////////////////////////////////////
// temperature conversion
// tempchar = the string containing the temperature value
// unit = the unit for temperature
// return value = the converted temperature with degree sign and unit; if fails, return N/A

void CWeatherProto::GetTemp(const wchar_t *tempchar, const wchar_t *unit, wchar_t *str)
{
	// unit can be C, F
	double temp;
	wchar_t tstr[20];

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

/////////////////////////////////////////////////////////////////////////////////////////
// temperature conversion
// tempchar = the string containing the pressure value
// unit = the unit for pressure
// return value = the converted pressure with unit; if fail, return the original string

void CWeatherProto::GetPressure(const wchar_t *tempchar, const wchar_t *unit, wchar_t *str)
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

/////////////////////////////////////////////////////////////////////////////////////////
// speed conversion
// tempchar = the string containing the speed value
// unit = the unit for speed
// return value = the converted speed with unit; if fail, return _T(""

void CWeatherProto::GetSpeed(const wchar_t *tempchar, const wchar_t *unit, wchar_t *str)
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

/////////////////////////////////////////////////////////////////////////////////////////
// distance conversion
// tempchar = the string containing the distance value
// unit = the unit for distance
// return value = the converted distance with unit; if fail, return original string

void CWeatherProto::GetDist(const wchar_t *tempchar, const wchar_t *unit, wchar_t *str)
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

/////////////////////////////////////////////////////////////////////////////////////////
// elevation conversion
// tempchar = the string containing the elevation value
// unit = the unit for elevation
// return value = the converted elevation with unit; if fail, return original string

void CWeatherProto::GetElev(const wchar_t *tempchar, const wchar_t *unit, wchar_t *str)
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

/////////////////////////////////////////////////////////////////////////////////////////
// this function convert the string to the format with 1 upper case followed by lower case char

void CaseConv(wchar_t *str)
{
	bool nextUp = true;

	CharLowerBuffW(str, (uint32_t)mir_wstrlen(str));
	for (wchar_t *pstr = str; *pstr; pstr++) {
		if (*pstr == ' ' || *pstr == '-')
			nextUp = true;
		else if (nextUp) {
			CharUpperBuffW(pstr, 1);
			nextUp = false;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// the next 2 functions are copied from miranda source
// str = the string to modify

void TrimString(char *str)
{
	size_t len, start;

	len = mir_strlen(str);
	while (len && (unsigned char)str[len - 1] <= ' ') str[--len] = 0;
	for (start = 0; (unsigned char)str[start] <= ' ' && str[start]; start++);
	memmove(str, str + start, len - start + 1);
}

void TrimString(wchar_t *str)
{
	size_t len, start;

	len = mir_wstrlen(str);
	while (len && (unsigned char)str[len - 1] <= ' ') str[--len] = 0;
	for (start = 0; (unsigned char)str[start] <= ' ' && str[start]; start++);
	memmove(str, str + start, (len - start + 1) * sizeof(wchar_t));
}

/////////////////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////////////////
// replace spaces with _T("%20"
// dis = original string
// return value = the modified string with space -> _T("%20"

char* GetSearchStr(char *dis)
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

/////////////////////////////////////////////////////////////////////////////////////////
// make display and history strings
// w = WEATHERINFO data to be parsed
// dis = the string to parse
// return value = the parsed string

CMStringW GetDisplay(WEATHERINFO *w, const wchar_t *dis)
{
	wchar_t lpzDate[32], chr;
	char name[256], temp[2];
	DBVARIANT dbv;
	size_t i;

	// Clear the string
	CMStringW str;

	// looking character by character
	for (i = 0; i < mir_wstrlen(dis); i++) {
		// for the escape characters
		if (dis[i] == '\\') {
			i++;
			chr = dis[i];
			switch (chr) {
			case '%': str.Append(L"%"); break;
			case 't': str.Append(L"\t"); break;
			case 'n': str.Append(L"\r\n"); break;
			case '\\': str.Append(L"\\"); break;
			}
		}

		// for the % varaibles
		else if (dis[i] == '%') {
			i++;
			chr = dis[i];
			// turn capitalized characters to small case
			if (chr < 'a' && chr != '[' && chr != '%') chr = (char)((int)chr + 32);
			switch (chr) {
			case 'c': str.Append(w->cond); break;
			case 'd':	// get the current date
				GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, nullptr, nullptr, lpzDate, _countof(lpzDate));
				str.Append(lpzDate); break;
			case 'e': str.Append(w->dewpoint); break;
			case 'f': str.Append(w->feel); break;
			case 'h': str.Append(w->high); break;
			case 'i': str.Append(w->winddir); break;
			case 'l': str.Append(w->low); break;
			case 'm': str.Append(w->humid); break;
			case 'n': str.Append(w->city); break;
			case 'p': str.Append(w->pressure); break;
			case 'r': str.Append(w->sunrise); break;
			case 's': str.Append(w->id); break;
			case 't': str.Append(w->temp); break;
			case 'u':
				if (mir_wstrcmp(w->update, NODATA))
					str.Append(w->update);
				else
					str.Append(TranslateT("<unknown time>"));
				break;
			case 'v': str.Append(w->vis); break;
			case 'w': str.Append(w->wind); break;
			case 'y': str.Append(w->sunset); break;
			case '%': str.Append(L"%"); break;
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
						str.Append(dbv.pwszVal);
					db_free(&dbv);
				}
				break;
			}
		}
		// if the character is not a variable, write the original character to the new string
		else str.AppendChar(dis[i]);
	}

	return str;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Get the text when an error code is specified
// code = the error code obtained when updating weather
// str = the string for the error

wchar_t *GetError(int code)
{
	wchar_t *str, str2[100];
	switch (code) {
	case 10:	str = TranslateT("Invalid ID format, missing \"/\" (10)"); break;
	case 11:	str = TranslateT("Invalid service (11)"); break;
	case 12:	str = TranslateT("Invalid station (12)"); break;
	case 20:	str = TranslateT("Weather service ini for this station is not found (20)"); break;
	case 30:	str = TranslateT("Netlib error - check your internet connection (30)"); break;
	case 40:	str = TranslateT("Empty data is retrieved (40)"); break;
	case 42:	str = TranslateT("Document not found (42)"); break;
	case 43:	str = TranslateT("Document too short to contain any weather data (43)"); break;
	case 99:	str = TranslateT("Unknown error (99)"); break;
	// 100 Continue
	// 101 Switching Protocols
	// 200 OK
	// 201 Created
	// 202 Accepted
	// 203 Non-Authoritative Information
	case 204: str = TranslateT("HTTP Error: No content (204)"); break;
	// 205 Reset Content
	// 206 Partial Content
	// 300 Multiple Choices
	case 301: str = TranslateT("HTTP Error: Data moved (301)"); break;
	// 302 Found
	// 303 See Other
	// 304 Not Modified
	case 305: str = TranslateT("HTTP Error: Use proxy (305)"); break;
	case 307: str = TranslateT("HTTP Error: Temporary redirect (307)"); break;
	case 400: str = TranslateT("HTTP Error: Bad request (400)"); break;
	case 401: str = TranslateT("HTTP Error: Unauthorized (401)"); break;
	case 402: str = TranslateT("HTTP Error: Payment required (402)"); break;
	case 403: str = TranslateT("HTTP Error: Forbidden (403)"); break;
	case 404: str = TranslateT("HTTP Error: Not found (404)"); break;
	case 405: str = TranslateT("HTTP Error: Method not allowed (405)"); break;
	// 406 Not Acceptable
	case 407: str = TranslateT("HTTP Error: Proxy authentication required (407)"); break;
	// 408 Request Timeout
	// 409 Conflict
	case 410: str = TranslateT("HTTP Error: Gone (410)"); break;
	// 411 Length Required
	// 412 Precondition Failed
	// 413 Request Entity Too Large
	// 414 Request-URI Too Long
	// 415 Unsupported Media Type
	// 416 Requested Range Not Satisfiable
	// 417 Expectation Failed
	case 500: str = TranslateT("HTTP Error: Internal server error (500)"); break;
	// 501 Not Implemented
	case 502: str = TranslateT("HTTP Error: Bad gateway (502)"); break;
	case 503: str = TranslateT("HTTP Error: Service unavailable (503)"); break;
	case 504: str = TranslateT("HTTP Error: Gateway timeout (504)"); break;
	// 505 HTTP Version Not Supported
	default:
		mir_snwprintf(str2, TranslateT("HTTP Error %i"), code);
		str = str2;
		break;
	}
	return mir_wstrdup(str);
}
