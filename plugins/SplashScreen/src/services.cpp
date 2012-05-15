/*
   Splash Screen Plugin for Miranda-IM (www.miranda-im.org)
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

   File name      : $URL: http://svn.miranda.im/mainrepo/splashscreen/trunk/src/services.cpp $
   Revision       : $Rev: 951 $
   Last change on : $Date: 2007-10-16 18:46:53 +0400 (Вт, 16 окт 2007) $
   Last change by : $Author: Thief $

*/

#include "headers.h"

INT_PTR ShowSplashService(WPARAM wparam,LPARAM lparam)
{
	bserviceinvoked = true;
	TCHAR szOldfn [256];
	TCHAR* pos;
	TCHAR* filename = (TCHAR*) wparam;
	int timetoshow = (int) lparam;

	lstrcpy(szOldfn, szSplashFile);
	options.showtime = timetoshow;

	pos = _tcsrchr(filename, _T(':'));
	if (pos == NULL)
		mir_sntprintf(szSplashFile, SIZEOF(szSplashFile), _T("%s\\%s"), szMirDir, filename);
	else
		lstrcpy(szSplashFile, filename);

	ShowSplash(false);

	lstrcpy(szSplashFile, szOldfn);

	return 0;
}

#ifdef _DEBUG
INT_PTR TestService(WPARAM wParam,LPARAM lParam)
{
	TCHAR szTempPath[MAX_PATH];

	OPENFILENAME ofn={0};
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.lpstrFilter = _T("PNG and BMP files\0*.png;*.bmp\0\0");
	ofn.hwndOwner=0;
	ofn.lpstrFile = szTempPath;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.Flags = OFN_HIDEREADONLY;
	ofn.lpstrInitialDir = szSplashFile;
	*szTempPath = '\0';
	ofn.lpstrDefExt = _T("");

	if (GetOpenFileName(&ofn))
		CallService(MS_SHOWSPLASH,(WPARAM)szTempPath,(LPARAM)0);

	return 0;
}
#endif