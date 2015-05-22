/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "msn_global.h"

static TCHAR* a2tf(const TCHAR* str, bool unicode)
{
	if (str == NULL)
		return NULL;

	return unicode ? mir_tstrdup(str) : mir_a2t((char*)str);
}

void overrideStr(TCHAR*& dest, const TCHAR* src, bool unicode, const TCHAR* def)
{
	mir_free(dest);
	dest = NULL;

	if (src != NULL)
		dest = a2tf(src, unicode);
	else if (def != NULL)
		dest = mir_tstrdup(def);
}

char* arrayToHex(BYTE* data, size_t datasz)
{
	char *res = (char*)mir_alloc(2 * datasz + 1);
	bin2hex(data, datasz, res);
	return res;
}

bool txtParseParam(const char* szData, const char* presearch, const char* start, const char* finish, char* param, const int size)
{
	const char *cp, *cp1;
	int len;

	if (szData == NULL) return false;

	if (presearch != NULL) {
		cp1 = strstr(szData, presearch);
		if (cp1 == NULL) return false;
	}
	else cp1 = szData;

	cp = strstr(cp1, start);
	if (cp == NULL) return false;
	cp += mir_strlen(start);
	while (*cp == ' ') ++cp;

	if (finish) {
		cp1 = strstr(cp, finish);
		if (cp1 == NULL) return FALSE;
		while (*(cp1 - 1) == ' ' && cp1 > cp) --cp1;
	}
	else cp1 = strchr(cp, '\0');

	len = min(cp1 - cp, size - 1);
	memmove(param, cp, len);
	param[len] = 0;

	return true;
}

void parseWLID(char* wlid, char** net, char** email, char** inst)
{
	char *col = strchr(wlid, ':');
	if (col && strncmp(wlid, "tel:", 4)) {
		*col = 0;
		if (net) *net = wlid;
		if (email) *email = col + 1;
		++col;
		wlid=col;
	}
	else {
		if (net) *net = NULL;
		if (email) *email = wlid;
	}

	col = strchr(wlid, ';');
	if (col) {
		*col = 0;
		if (inst) *inst = col + 1;
	}
	else if (inst)
		*inst = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// UrlDecode - converts URL chars like %20 into printable characters

static int SingleHexToDecimal(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

template void UrlDecode(char* str);
template void UrlDecode(wchar_t* str);

template <class chartype> void UrlDecode(chartype* str)
{
	chartype* s = str, *d = str;

	while (*s) {
		if (*s == '%') {
			int digit1 = SingleHexToDecimal(s[1]);
			if (digit1 != -1) {
				int digit2 = SingleHexToDecimal(s[2]);
				if (digit2 != -1) {
					s += 3;
					*d++ = (char)((digit1 << 4) | digit2);
					continue;
				}
			}
		}
		*d++ = *s++;
	}

	*d = 0;
}

void HtmlDecode(char *str)
{
	if (str == NULL)
		return;

	char* p, *q;
	for (p = q = str; *p != '\0'; p++, q++) {
		if (*p == '&') {
			if (!strncmp(p, "&amp;", 5)) { *q = '&'; p += 4; }
			else if (!strncmp(p, "&apos;", 6)) { *q = '\''; p += 5; }
			else if (!strncmp(p, "&gt;", 4)) { *q = '>'; p += 3; }
			else if (!strncmp(p, "&lt;", 4)) { *q = '<'; p += 3; }
			else if (!strncmp(p, "&quot;", 6)) { *q = '"'; p += 5; }
			else { *q = *p; }
		}
		else *q = *p;
	}
	*q = '\0';
}

char* HtmlEncode(const char *str)
{
	char* s, *p, *q;
	int c;

	if (str == NULL)
		return NULL;

	for (c = 0, p = (char*)str; *p != '\0'; p++) {
		switch (*p) {
			case '&': c += 5; break;
			case '\'': c += 6; break;
			case '>': c += 4; break;
			case '<': c += 4; break;
			case '"': c += 6; break;
			default: c++; break;
		}
	}

	if ((s = (char*)mir_alloc(c + 1)) != NULL) {
		for (p = (char*)str, q = s; *p != '\0'; p++) {
			switch (*p) {
				case '&': strcpy(q, "&amp;"); q += 5; break;
				case '\'': strcpy(q, "&apos;"); q += 6; break;
				case '>': strcpy(q, "&gt;"); q += 4; break;
				case '<': strcpy(q, "&lt;"); q += 4; break;
				case '"': strcpy(q, "&quot;"); q += 6; break;
				default: *q = *p; q++; break;
			}
		}
		*q = '\0';
	}

	return s;
}

/////////////////////////////////////////////////////////////////////////////////////////

void stripBBCode(char* src)
{
	bool tag = false;
	char *ps = src;
	char *pd = src;

	while (*ps != 0) {
		if (!tag && *ps == '[') {
			char ch = ps[1];
			if (ch == '/') ch = ps[2];
			tag = ch == 'b' || ch == 'u' || ch == 'i' || ch == 'c' || ch == 'a' || ch == 's';
		}
		if (!tag) *(pd++) = *ps;
		else tag = *ps != ']';
		++ps;
	}
	*pd = 0;
}

void stripColorCode(char* src)
{
	unsigned char* ps = (unsigned char*)src;
	unsigned char* pd = (unsigned char*)src;

	while (*ps != 0) {
		if (ps[0] == 0xc2 && ps[1] == 0xb7) {
			char ch = ps[2];
			switch (ch) {
			case '#':
			case '&':
			case '\'':
			case '@':
			case '0':
				ps += 3;
				continue;

			case '$':
				if (isdigit(ps[3])) {
					ps += 3;
					if (isdigit(ps[1]))
						ps += 2;
					else
						++ps;

					if (ps[0] == ',' && isdigit(ps[1])) {
						ps += 2;
						if (isdigit(ps[1]))
							ps += 2;
						else
							++ps;
					}
					continue;
				}
				else if (ps[3] == '#') {
					ps += 4;
					for (int i = 0; i < 6; ++i) {
						if (isxdigit(*ps)) ++ps;
						else break;
					}
					continue;
				}
				break;
			}
		}
		*(pd++) = *(ps++);
	}
	*pd = 0;
}

void  stripHTML(char* str)
{
	char *p, *q;

	for ( p=q=str; *p!='\0'; p++,q++ ) 
	{
		if ( *p == '<' )
		{
			if      ( !strnicmp( p, "<p>",  3 )) { strcpy(q, "\r\n\r\n"); q += 3; p += 2; }
			else if ( !strnicmp( p, "</p>", 4 )) { strcpy(q, "\r\n\r\n"); q += 3; p += 3; }
			else if ( !strnicmp( p, "<br>", 4 )) { strcpy(q, "\r\n"); ++q; p += 3; }
			else if ( !strnicmp( p, "<br />", 6 )) { strcpy(q, "\r\n"); ++q; p += 5; }
			else if ( !strnicmp( p, "<hr>", 4 )) { strcpy(q, "\r\n"); ++q; p += 3; }
			else if ( !strnicmp( p, "<hr />", 6 )) { strcpy(q, "\r\n"); ++q; p += 5; }
			else { 
				char *l = strchr(p, '>');
				if (l) { p = l; --q; } else *q = *p; 
			}
		}
		else 
			*q = *p;
	}
	*q = '\0';
}


// Process a string, and double all % characters, according to chat.dll's restrictions
// Returns a pointer to the new string (old one is not freed)
TCHAR* EscapeChatTags(const TCHAR* pszText)
{
	int nChars = 0;
	for (const TCHAR* p = pszText; (p = _tcschr(p, '%')) != NULL; p++)
		nChars++;

	if (nChars == 0)
		return mir_tstrdup(pszText);

	TCHAR *pszNewText = (TCHAR*)mir_alloc(sizeof(TCHAR)*(mir_tstrlen(pszText) + 1 + nChars));
	if (pszNewText == NULL)
		return mir_tstrdup(pszText);

	const TCHAR *s = pszText;
	TCHAR *d = pszNewText;
	while (*s) {
		if (*s == '%')
			*d++ = '%';
		*d++ = *s++;
	}
	*d = 0;
	return pszNewText;
}

TCHAR* UnEscapeChatTags(TCHAR* str_in)
{
	TCHAR *s = str_in, *d = str_in;
	while (*s) {
		if ((*s == '%' && s[1] == '%') || (*s == '\n' && s[1] == '\n'))
			s++;
		*d++ = *s++;
	}
	*d = 0;
	return str_in;
}

char* getNewUuid(void)
{
	UUID id;
	UuidCreate(&id);

	BYTE *p;
	UuidToStringA(&id, &p);
	size_t len = mir_strlen((char*)p) + 3;
	char *result = (char*)mir_alloc(len);
	mir_snprintf(result, len, "{%s}", p);
	_strupr(result);
	RpcStringFreeA(&p);
	return result;
}

time_t IsoToUnixTime(const char *stamp)
{
	char date[9];
	int i, y;

	if (stamp == NULL)
		return 0;

	char *p = NEWSTR_ALLOCA(stamp);

	// skip '-' chars
	int si = 0, sj = 0;
	while (true) {
		if (p[si] == '-')
			si++;
		else if (!(p[sj++] = p[si++]))
			break;
	}

	// Get the date part
	for (i = 0; *p != '\0' && i < 8 && isdigit(*p); p++, i++)
		date[i] = *p;

	// Parse year
	if (i == 6) {
		// 2-digit year (1970-2069)
		y = (date[0] - '0') * 10 + (date[1] - '0');
		if (y < 70) y += 100;
	}
	else if (i == 8) {
		// 4-digit year
		y = (date[0] - '0') * 1000 + (date[1] - '0') * 100 + (date[2] - '0') * 10 + date[3] - '0';
		y -= 1900;
	}
	else return 0;

	struct tm timestamp;
	timestamp.tm_year = y;

	// Parse month
	timestamp.tm_mon = (date[i - 4] - '0') * 10 + date[i - 3] - '0' - 1;

	// Parse date
	timestamp.tm_mday = (date[i - 2] - '0') * 10 + date[i - 1] - '0';

	// Skip any date/time delimiter
	for (; *p != '\0' && !isdigit(*p); p++);

	// Parse time
	if (sscanf(p, "%d:%d:%d", &timestamp.tm_hour, &timestamp.tm_min, &timestamp.tm_sec) != 3)
		return (time_t)0;

	timestamp.tm_isdst = 0;	// DST is already present in _timezone below
	time_t t = mktime(&timestamp);

	_tzset();
	t -= _timezone;
	return (t >= 0) ? t : 0;
}

time_t MsnTSToUnixtime(const char *pszTS)
{
	char szTS[16];

	if (!*pszTS) return time(NULL);
	strncpy(szTS, pszTS, 10);
	return (time_t)atoi(szTS);
}
