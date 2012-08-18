/*

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

#include "Common.h"

char *fmtDBSettingName(const char *fmt, ...)
{
	va_list va;
	static char returnString[1024];

	va_start(va, fmt);
	mir_vsnprintf(returnString, sizeof(returnString), fmt, va);
	va_end(va);

	return returnString;
}


TCHAR *getAbsoluteProfileName(TCHAR *absoluteProfileName, size_t maxLen)
{
	TCHAR profilePath[MAX_PATH+1], profileName[MAX_PATH+1];

	profilePath[0] = profileName[0] = '\0';
	CallService(MS_DB_GETPROFILEPATHT, MAX_PATH, (LPARAM)profilePath);
	CallService(MS_DB_GETPROFILENAMET, MAX_PATH, (LPARAM)profileName);
	mir_sntprintf(absoluteProfileName, maxLen, _T("%s\\%s"), profilePath, profileName);

	return absoluteProfileName;
}
