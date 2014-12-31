/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#include "hdr/modern_commonheaders.h"
#include "hdr/modern_commonprototypes.h"

INT_PTR CListTray_GetGlobalStatus(WPARAM wparam, LPARAM lparam);

int CLUIUnreadEmailCountChanged(WPARAM, LPARAM)
{
	CallService(MS_SKINENG_INVALIDATEFRAMEIMAGE, 0, 0);
	return 0;
}

void cliCluiProtocolStatusChanged(int, const char * proto)
{
	CallService(MS_SKINENG_INVALIDATEFRAMEIMAGE, (WPARAM)pcli->hwndStatus, 0);
	if (proto)
		pcli->pfnTrayIconUpdateBase(proto);
}

static INT_PTR MetaSupportCheck(WPARAM, LPARAM)
{
	return 1;
}

int CLUIServices_LoadModule(void)
{
	CreateServiceFunction(MS_CLUI_METASUPPORT, MetaSupportCheck);
	CreateServiceFunction(MS_CLIST_GETSTATUSMODE, CListTray_GetGlobalStatus);
	return 0;
}
