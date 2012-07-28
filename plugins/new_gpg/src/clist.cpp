// Copyright © 2010-2012 SecureIM developers (baloo and others), sss
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "commonheaders.h"

extern HANDLE g_hCLIcon;
void RefreshContactListIcons(void);

int onExtraImageListRebuilding(WPARAM, LPARAM) 
{
	if(g_hCLIcon && ServiceExists(MS_CLIST_EXTRA_ADD_ICON) ) 
		RefreshContactListIcons();
	return 0;
}


int onExtraImageApplying(WPARAM wParam, LPARAM) 
{
	void setClistIcon(HANDLE);
	if(g_hCLIcon && ServiceExists(MS_CLIST_EXTRA_SET_ICON)) 
	{
//		IconExtraColumn iec = {0}; //need to init this
		if( g_hCLIcon ) 
			setClistIcon((HANDLE)wParam);
//			ExtraIcon_SetIcon(g_hCLIcon, (HANDLE)wParam, iec.hImage);
	}
	return 0;
}
