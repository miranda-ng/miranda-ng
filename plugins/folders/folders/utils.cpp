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

#include "utils.h"

int Log(char *format, ...)
{
#ifdef _DEBUG
	char		str[4096];
	va_list	vararg;
	int tBytes;
	FILE *fout = fopen("folders.log", "at");
	
	va_start(vararg, format);
	
	tBytes = _vsnprintf(str, sizeof(str), format, vararg);
	if (tBytes > 0)
		{
			str[tBytes] = 0;
		}

	va_end(vararg);
	if (str[strlen(str) - 1] != '\n')
		{
			strcat(str, "\n");
		}
	fputs(str, fout);
	fclose(fout);
#endif
	return 0;
}

char *StrCopy(char *source, size_t index, const char *what, size_t count)
{
	size_t i;
	for (i = 0; i < count; i++)
		{
			source[index + i] = what[i];
		}
	return source;
}

wchar_t *StrCopy(wchar_t *source, size_t index, const wchar_t *what, size_t count)
{
	size_t i;
	for (i = 0; i < count; i++)
		{
			source[index + i] = what[i];
		}
	return source;
}

char *StrDelete(char *source, size_t index, size_t count)
{
	size_t len = strlen(source);
	size_t i;
	count = (count + index > len) ? len - index : count;
	for (i = index; i + count <= len; i++)
		{
			source[i] = source[i + count];
		}
	return source;
}

wchar_t *StrDelete(wchar_t *source, size_t index, size_t count)
{
	size_t len = wcslen(source);
	size_t i;
	count = (count + index > len) ? len - index : count;
	for (i = index; i + count <= len; i++)
		{
			source[i] = source[i + count];
		}
	return source;
}


char *StrInsert(char *source, size_t index, const char *what)
{
	size_t whatLen = strlen(what);
	size_t sourceLen = strlen(source);
	size_t i;
	for (i = sourceLen; i >= index; i--)
		{
			source[i + whatLen] = source[i];
		}
	for (i = 0; i < whatLen; i++)
		{
			source[index + i] = what[i];
		}
	return source;
}

wchar_t *StrInsert(wchar_t *source, size_t index, const wchar_t *what)
{
	size_t whatLen = wcslen(what);
	size_t sourceLen = wcslen(source);
	size_t i;
	for (i = sourceLen; i >= index; i--)
		{
			source[i + whatLen] = source[i];
		}
	for (i = 0; i < whatLen; i++)
		{
			source[index + i] = what[i];
		}
	return source;
}


char *StrReplace(char *source, const char *what, const char *withWhat)
{
	char *pos;
	size_t whatLen = strlen(what);
	size_t withWhatLen = strlen(withWhat);
	size_t minLen;
	size_t index;
	
	while ((pos = strstr(source, what)))
		{
			minLen = min(whatLen, withWhatLen);
			StrCopy(source, pos - source, withWhat, minLen); 
			index = pos - source + minLen;
			if (whatLen > withWhatLen)
				{
					StrDelete(source, index, whatLen - withWhatLen);
				}
				else{
					if (whatLen < withWhatLen)
						{
							StrInsert(source, index, withWhat + minLen);
						}
				}
		}
	return source;
}

wchar_t *StrReplace(wchar_t *source, const wchar_t *what, const wchar_t *withWhat)
{
	wchar_t *pos;
	size_t whatLen = wcslen(what);
	size_t withWhatLen = wcslen(withWhat);
	size_t minLen;
	size_t index;
	
	while ((pos = wcsstr(source, what)))
		{
			minLen = min(whatLen, withWhatLen);
			StrCopy(source, pos - source, withWhat, minLen); 
			index = pos - source + minLen;
			if (whatLen > withWhatLen)
				{
					StrDelete(source, index, whatLen - withWhatLen);
				}
				else{
					if (whatLen < withWhatLen)
						{
							StrInsert(source, index, withWhat + minLen);
						}
				}
		}
	return source;
}

char *StrTrim(char *szText, const char *szTrimChars)
{
	size_t i = strlen(szText) - 1;
	while ((i >= 0) && (strchr(szTrimChars, szText[i])))
		{
			szText[i--] = '\0';
		}
	i = 0;
	while ((i < strlen(szText)) && (strchr(szTrimChars, szText[i])))
		{
			i++;
		}
	if (i)
		{
			StrDelete(szText, 0, i);
		}
	return szText;
}

wchar_t *StrTrim(wchar_t *szText, const wchar_t *szTrimChars)
{
	size_t i = wcslen(szText) - 1;
	while ((i >= 0) && (wcschr(szTrimChars, szText[i])))
		{
			szText[i--] = '\0';
		}
	i = 0;
	while ((i < wcslen(szText)) && (wcschr(szTrimChars, szText[i])))
		{
			i++;
		}
	if (i)
		{
			StrDelete(szText, 0, i);
		}
	return szText;
}

void CreateDirectories(char *path)
{
	char *pos = path;
	char tmp;
	while (pos = strchr(pos, '\\'))
		{
			tmp = pos[0];
			pos[0] = '\0';
			CreateDirectoryA(path, NULL);
			pos[0] = tmp;
			pos++;
		}
	CreateDirectoryA(path, NULL);	
	GetLastError();
}

void CreateDirectories(wchar_t *path)
{
	wchar_t *pos = path;
	wchar_t tmp;
	while (pos = wcschr(pos, '\\'))
		{
			tmp = pos[0];
			pos[0] = '\0';
			CreateDirectoryW(path, NULL);
			pos[0] = tmp;
			pos++;
		}
	CreateDirectoryW(path, NULL);	
	GetLastError();
}

void RemoveDirectories(char *path)
{
	char *pos;
	char *buffer = _strdup(path);
	if (!(GetFileAttributesA(buffer) & FILE_ATTRIBUTE_REPARSE_POINT)) { RemoveDirectoryA(buffer); }
	while (pos = strrchr(buffer, '\\'))
		{
			pos[0] = '\0';
			if (!(GetFileAttributesA(buffer) & FILE_ATTRIBUTE_REPARSE_POINT)) { RemoveDirectoryA(buffer); }
		}
	free(buffer);
}

void RemoveDirectories(wchar_t *path)
{
	wchar_t *pos;
	wchar_t *buffer = _wcsdup(path);
	if (!(GetFileAttributesW(buffer) & FILE_ATTRIBUTE_REPARSE_POINT)) { RemoveDirectoryW(buffer); }
	while (pos = wcsrchr(buffer, '\\'))
		{
			pos[0] = '\0';
			if (!(GetFileAttributesW(buffer) & FILE_ATTRIBUTE_REPARSE_POINT)) { RemoveDirectoryW(buffer); }
		}
	free(buffer);
}

int DirectoryExists(char *path)
{
	char buffer[4096];
	GetCurrentDirectoryA(sizeof(buffer), buffer);
	int res = SetCurrentDirectoryA(path);
	SetCurrentDirectoryA(buffer);
	return res;
}

int DirectoryExists(wchar_t *path)
{
	wchar_t buffer[4096];
	GetCurrentDirectoryW(sizeof(buffer), buffer);
	int res = SetCurrentDirectoryW(path);
	SetCurrentDirectoryW(buffer);
	return res;
}


int GetStringFromDatabase(char *szSettingName, const char *szError, char *szResult, size_t size)
{
	DBVARIANT dbv = {0};
	int res = 1;
	size_t len;
	dbv.type = DBVT_ASCIIZ;
	if (DBGetContactSetting(NULL, ModuleName, szSettingName, &dbv) == 0)
		{
			res = 0;
			size_t tmp = strlen(dbv.pszVal);
			len = (tmp < size - 1) ? tmp : size - 1;
			strncpy(szResult, dbv.pszVal, len);
			szResult[len] = '\0';
			mir_free(dbv.pszVal);
		}
		else{
			res = 1;
			size_t tmp = strlen(szError);
			len = (tmp < size - 1) ? tmp : size - 1;
			strncpy(szResult, szError, len);
			szResult[len] = '\0';
		}
	return res;
}

int GetStringFromDatabase(char *szSettingName, const wchar_t *szError, wchar_t *szResult, size_t size)
{
	DBVARIANT dbv = {0};
	int res = 1;
	size_t len;
	dbv.type = DBVT_WCHAR;
	if (DBGetContactSettingWString(NULL, ModuleName, szSettingName, &dbv) == 0)
	//if (DBGetContactSetting(NULL, ModuleName, szSettingName, &dbv) == 0)
		{
			res = 0;
			size_t tmp = wcslen(dbv.pwszVal);
			len = (tmp < size - 1) ? tmp : size - 1;
			wcsncpy(szResult, dbv.pwszVal, len);
			szResult[len] = '\0';
			mir_free(dbv.pwszVal);
		}
		else{
			res = 1;
			size_t tmp = wcslen(szError);
			len = (tmp < size - 1) ? tmp : size - 1;
			wcsncpy(szResult, szError, len);
			szResult[len] = '\0';
		}
	return res;
}


int WriteStringToDatabase(char *szSettingName, const char *szValue)
{
	return DBWriteContactSettingString(NULL, ModuleName, szSettingName, szValue);
}

int WriteStringToDatabase(char *szSettingName, const wchar_t *szValue)
{
	return DBWriteContactSettingWString(NULL, ModuleName, szSettingName, szValue);
}