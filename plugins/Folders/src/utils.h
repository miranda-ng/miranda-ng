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

TCHAR *StrReplace(TCHAR *source, const TCHAR *what, const TCHAR *withWhat);
TCHAR *StrCopy(TCHAR *source, size_t index, const TCHAR *what, size_t count);
TCHAR *StrDelete(TCHAR *source, size_t index, size_t count);
TCHAR *StrInsert(TCHAR *source, size_t index, const TCHAR *what);
TCHAR *StrTrim(TCHAR *szText, const TCHAR *szTrimChars);

void RemoveDirectories(TCHAR *szPath);
int DirectoryExists(TCHAR *szPath);

int GetStringFromDatabase(char *szSettingName, const TCHAR *szError, TCHAR *szResult, size_t size);

#endif