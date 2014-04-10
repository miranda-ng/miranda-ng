/*
   Splash Screen Plugin for Miranda NG (www.miranda-ng.org)
   (c) 2004-2007 nullbie, (c) 2005-2007 Thief

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "headers.h"

INT_PTR ShowSplashService(WPARAM wparam,LPARAM lparam)
{
	bserviceinvoked = true;
	TCHAR szOldfn [256];
	TCHAR* filename = (TCHAR*) wparam;
	int timetoshow = (int) lparam;

	_tcscpy_s(szOldfn, szSplashFile);
	options.showtime = timetoshow;

	TCHAR *pos = _tcsrchr(filename, _T(':'));
	if (pos == NULL)
		mir_sntprintf(szSplashFile, SIZEOF(szSplashFile), _T("%s\\%s"), szMirDir, filename);
	else
		_tcscpy_s(szSplashFile, filename);

	ShowSplash(false);

	_tcscpy_s(szSplashFile, szOldfn);

	return 0;
}
