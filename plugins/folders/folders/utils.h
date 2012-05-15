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

#ifndef M_FOLDERS_UTILS_H
#define M_FOLDERS_UTILS_H

#include <stdarg.h>
#include "commonheaders.h"

int Log(char *format, ...);

char *StrReplace(char *source, const char *what, const char *withWhat);
char *StrCopy(char *source, size_t index, const char *what, size_t count);
char *StrDelete(char *source, size_t index, size_t count);
char *StrInsert(char *source, size_t index, const char *what);
char *StrTrim(char *szText, const char *szTrimChars);

wchar_t *StrReplace(wchar_t *source, const wchar_t *what, const wchar_t *withWhat);
wchar_t *StrCopy(wchar_t *source, size_t index, const wchar_t *what, size_t count);
wchar_t *StrDelete(wchar_t *source, size_t index, size_t count);
wchar_t *StrInsert(wchar_t *source, size_t index, const wchar_t *what);
wchar_t *StrTrim(wchar_t *szText, const wchar_t *szTrimChars);

void CreateDirectories(char *szPath);
void RemoveDirectories(char *szPath);
int DirectoryExists(char *szPath);

void CreateDirectories(wchar_t *szPath);
void RemoveDirectories(wchar_t *szPath);
int DirectoryExists(wchar_t *szPath);

int GetStringFromDatabase(char *szSettingName, const char *szError, char *szResult, size_t size);
int WriteStringToDatabase(char *szSettingName, const char *szValue);

int GetStringFromDatabase(char *szSettingName, const wchar_t *szError, wchar_t *szResult, size_t size);
int WriteStringToDatabase(char *szSettingName, const wchar_t *szValue);

__inline static wchar_t *Utils_ReplaceVarsW(wchar_t *szData) {
	REPLACEVARSDATA dat = {0};
	dat.cbSize = sizeof(dat);
	dat.dwFlags = RVF_UNICODE;
	return (wchar_t *) CallService(MS_UTILS_REPLACEVARS, (WPARAM) szData, (LPARAM) &dat);
}

#endif