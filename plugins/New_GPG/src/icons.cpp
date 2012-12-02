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

extern HINSTANCE hInst;

static IconItem iconList[] =
{
	{ "Secured", "secured", IDI_SECURED },
	{ "Unsecured", "unsecured", IDI_UNSECURED }
};

void InitIconLib()
{
	Icon_Register(hInst, szGPGModuleName, iconList, SIZEOF(iconList));
}

HICON IconLibGetIcon(const char* ident)
{
	return Skin_GetIcon(ident);
}

void IconLibReleaseIcon(const char* ident)
{
	Skin_ReleaseIcon(ident);
}

HANDLE IconLibHookIconsChanged(MIRANDAHOOK hook)
{
	return HookEvent(ME_SKIN2_ICONSCHANGED, hook);
}

void setClistIcon(HANDLE hContact)
{
	bool enabled = isContactSecured(hContact);
	extern HANDLE g_hCLIcon;
	HANDLE hMC = hContact;
	if(metaIsSubcontact(hContact))
		hMC = metaGetContact(hContact);
	else if(metaIsProtoMetaContacts(hContact))
		hMC = metaGetContact(hContact);
	const char *szIconId = (enabled) ? "secured" : NULL;
	ExtraIcon_SetIcon(g_hCLIcon, hContact, szIconId);
	if(hMC)
		ExtraIcon_SetIcon(g_hCLIcon, hMC, szIconId);
}

void setSrmmIcon(HANDLE h)
{
	HANDLE hContact = metaIsProtoMetaContacts(h)?metaGetMostOnline(h):h;
	bool enabled = isContactSecured(hContact);	
	HANDLE hMC = hContact;
	if(metaIsSubcontact(hContact))
		hMC = metaGetContact(hContact);
	else if(metaIsProtoMetaContacts(hContact))
		hMC = metaGetContact(hContact);
	if(ServiceExists(MS_MSG_MODIFYICON))
	{
		StatusIconData sid = {0};
		sid.cbSize = sizeof(sid);
		sid.szModule = szGPGModuleName;
		sid.hIcon = IconLibGetIcon("secured");
		sid.dwId = 0x00000001;
		sid.flags = enabled?0:MBF_HIDDEN;
		CallService(MS_MSG_MODIFYICON, (WPARAM)hContact, (LPARAM)&sid);
		if( hMC )
			CallService(MS_MSG_MODIFYICON, (WPARAM)hMC, (LPARAM)&sid);
		ZeroMemory(&sid, sizeof(sid));
		sid.cbSize = sizeof(sid);
		sid.szModule = szGPGModuleName;
		sid.hIcon = IconLibGetIcon("unsecured");
		sid.dwId = 0x00000002;
		sid.flags = enabled?MBF_HIDDEN:0;
		CallService(MS_MSG_MODIFYICON, (WPARAM)hContact, (LPARAM)&sid);
		if( hMC )
			CallService(MS_MSG_MODIFYICON, (WPARAM)hMC, (LPARAM)&sid);
	}
}


void RefreshContactListIcons() 
{
	CallService(MS_CLUI_LISTBEGINREBUILD,0,0);
	HANDLE hContact = db_find_first();
	while (hContact) 
	{
		setClistIcon(hContact);
		hContact = db_find_next(hContact);
	}
	CallService(MS_CLUI_LISTENDREBUILD,0,0);
}
