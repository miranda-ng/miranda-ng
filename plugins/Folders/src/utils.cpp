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

#include "stdafx.h"

CMStringW ExpandPath(const wchar_t *format)
{
	CMStringW res;

	if (ServiceExists(MS_VARS_FORMATSTRING))
		res = VARSW(ptrW(variables_parse((wchar_t*)format, NULL, NULL)));
	else
		res = VARSW(format);

	res.Replace(PROFILE_PATHW, szCurrentProfilePath);
	res.Replace(CURRENT_PROFILEW, szCurrentProfile);
	res.Replace(MIRANDA_PATHW, szMirandaPath);
	res.Replace(MIRANDA_USERDATAW, szUserDataPath);
	res.Trim();

	// also remove the trailing slash
	if (!res.IsEmpty()) {
		int iNewSize = res.GetLength() - 1;
		switch (res[iNewSize]) {
		case '\\': case '/':
			res.Truncate(iNewSize);
		}
	}

	return res;
}

void RemoveDirectories(const wchar_t *path)
{
	wchar_t *pos;
	wchar_t *buffer = NEWWSTR_ALLOCA(path);
	if (!(GetFileAttributes(buffer) & FILE_ATTRIBUTE_REPARSE_POINT))
		RemoveDirectory(buffer);
	while (pos = wcsrchr(buffer, '\\')) {
		pos[0] = '\0';
		if (!(GetFileAttributes(buffer) & FILE_ATTRIBUTE_REPARSE_POINT))
			RemoveDirectory(buffer);
	}
}

bool DirectoryExists(const wchar_t *path)
{
	uint32_t dwAttributes = GetFileAttributes(path);
	if (dwAttributes == INVALID_FILE_ATTRIBUTES || !(dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return false;
	return true;
}
