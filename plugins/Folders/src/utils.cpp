/*
Custom profile folders plugin for Miranda IM

Copyright © 2005 Cristian Libotean

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"

char *StrCopy(char *source, size_t index, const char *what, size_t count)
{
	for (size_t i = 0; i < count; i++)
		source[index + i] = what[i];

	return source;
}

wchar_t *StrCopy(wchar_t *source, size_t index, const wchar_t *what, size_t count)
{
	for (size_t i = 0; i < count; i++)
		source[index + i] = what[i];

	return source;
}

char *StrDelete(char *source, size_t index, size_t count)
{
	size_t len = strlen(source);
	size_t i;
	count = (count + index > len) ? len - index : count;
	for (i = index; i + count <= len; i++)
		source[i] = source[i + count];

	return source;
}

wchar_t *StrDelete(wchar_t *source, size_t index, size_t count)
{
	size_t len = wcslen(source);
	count = (count + index > len) ? len - index : count;
	for (size_t i = index; i + count <= len; i++)
		source[i] = source[i + count];

	return source;
}

char *StrInsert(char *source, size_t index, const char *what)
{
	size_t whatLen = strlen(what);
	size_t sourceLen = strlen(source);
	size_t i;
	for (i = sourceLen; i >= index; i--)
		source[i + whatLen] = source[i];

	for (i = 0; i < whatLen; i++)
		source[index + i] = what[i];

	return source;
}

wchar_t *StrInsert(wchar_t *source, size_t index, const wchar_t *what)
{
	size_t whatLen = wcslen(what);
	size_t sourceLen = wcslen(source);
	size_t i;
	for (i = sourceLen; i >= index; i--)
		source[i + whatLen] = source[i];

	for (i = 0; i < whatLen; i++)
		source[index + i] = what[i];

	return source;
}

char *StrReplace(char *source, const char *what, const char *withWhat)
{
	size_t whatLen = strlen(what);
	size_t withWhatLen = strlen(withWhat);
	
	char *pos;
	while ((pos = strstr(source, what))) {
		size_t minLen = min(whatLen, withWhatLen);
		StrCopy(source, pos - source, withWhat, minLen); 
		size_t index = pos - source + minLen;
		if (whatLen > withWhatLen)
			StrDelete(source, index, whatLen - withWhatLen);
		else {
			if (whatLen < withWhatLen)
				StrInsert(source, index, withWhat + minLen);
		}
	}
	return source;
}

wchar_t *StrReplace(wchar_t *source, const wchar_t *what, const wchar_t *withWhat)
{
	size_t whatLen = wcslen(what);
	size_t withWhatLen = wcslen(withWhat);

	wchar_t *pos;
	while ((pos = wcsstr(source, what))) {
		size_t minLen = min(whatLen, withWhatLen);
		StrCopy(source, pos - source, withWhat, minLen); 
		size_t index = pos - source + minLen;
		if (whatLen > withWhatLen)
			StrDelete(source, index, whatLen - withWhatLen);
		else {
			if (whatLen < withWhatLen)
				StrInsert(source, index, withWhat + minLen);
		}
	}
	return source;
}

char *StrTrim(char *szText, const char *szTrimChars)
{
	size_t i = strlen(szText) - 1;
	while (i >= 0 && strchr(szTrimChars, szText[i]))
		szText[i--] = '\0';

	i = 0;
	while ((i < strlen(szText)) && (strchr(szTrimChars, szText[i])))
		i++;

	if (i)
		StrDelete(szText, 0, i);

	return szText;
}

wchar_t *StrTrim(wchar_t *szText, const wchar_t *szTrimChars)
{
	size_t i = wcslen(szText) - 1;
	while (i >= 0 && wcschr(szTrimChars, szText[i]))
		szText[i--] = '\0';

	i = 0;
	while ((i < wcslen(szText)) && (wcschr(szTrimChars, szText[i])))
		i++;

	if (i)
		StrDelete(szText, 0, i);

	return szText;
}

void RemoveDirectories(TCHAR *path)
{
	TCHAR *pos;
	TCHAR *buffer = NEWWSTR_ALLOCA(path);
	if (!(GetFileAttributes(buffer) & FILE_ATTRIBUTE_REPARSE_POINT))
		 RemoveDirectory(buffer);
	while (pos = _tcsrchr(buffer, '\\')) {
		pos[0] = '\0';
		if (!(GetFileAttributes(buffer) & FILE_ATTRIBUTE_REPARSE_POINT))
			RemoveDirectory(buffer);
	}
}

int DirectoryExists(TCHAR *path)
{
	TCHAR buffer[4096];
	GetCurrentDirectory( SIZEOF(buffer), buffer);
	int res = SetCurrentDirectory(path);
	SetCurrentDirectory(buffer);
	return res;
}

int GetStringFromDatabase(char *szSettingName, const wchar_t *szError, TCHAR *szResult, size_t size)
{
	size_t len;
	DBVARIANT dbv;
	if ( db_get_ws(NULL, ModuleName, szSettingName, &dbv) == 0) {
		size_t tmp = _tcslen(dbv.ptszVal);
		len = (tmp < size - 1) ? tmp : size - 1;
		_tcsncpy(szResult, dbv.ptszVal, len);
		szResult[len] = '\0';
		db_free(&dbv);
		return 0;
	}

	size_t tmp = _tcslen(szError);
	len = (tmp < size - 1) ? tmp : size - 1;
	_tcsncpy(szResult, szError, len);
	szResult[len] = '\0';
	return 1;
}
