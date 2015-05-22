/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include <commonheaders.h>

typedef enum {
	WPS_TILED=0,
	WPS_CENTERED,
	WPS_STRETCHED
} wallpaperstyle_t;

HBITMAP hPattern;
HBITMAP hAsSeenOnDesk;

/*
void GetWallpaperPattern()
{
	// \Control Panel\Desktop
	char wpbuf[MAX_PATH];
	if (hPattern) {DeleteObject(hPattern); hPattern=NULL;}
	SystemParametersInfo(SPI_GETDESKWALLPAPER,MAX_PATH,wpbuf,NULL);

	if (mir_strlen(wpbuf)>0)
	{
		hPattern = (HBITMAP)CallService(MS_UTILS_LOADBITMAP,0,(LPARAM)wpbuf);
	}
}

void GetWallpaperStyle()
{
	HKEY regk;
	char buff[256];
	RegOpenKeyEx(HKEY_CURRENT_USER,"\\Control Panel\\Desktop",0,KEY_READ,&regk);
	RegQueryValueEx(regk,"OriginalWallpaper",0,NULL,&buff,256);
	MessageBox(0,buff,"",0);
}*/