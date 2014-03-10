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

void setClistIcon(MCONTACT hContact)
{
	bool enabled = isContactSecured(hContact);
	extern HANDLE g_hCLIcon;
	MCONTACT hMC = hContact;
	if(db_mc_isSub(hContact))
		hMC = db_mc_getMeta(hContact);
	else if(metaIsProtoMetaContacts(hContact))
		hMC = db_mc_getMeta(hContact);
	const char *szIconId = (enabled) ? "secured" : NULL;
	ExtraIcon_SetIcon(g_hCLIcon, hContact, szIconId);
	if(hMC)
		ExtraIcon_SetIcon(g_hCLIcon, hMC, szIconId);
}

void setSrmmIcon(MCONTACT h)
{
	MCONTACT hContact = metaIsProtoMetaContacts(h) ? metaGetMostOnline(h) : h;
	bool enabled = isContactSecured(hContact);	
	MCONTACT hMC = NULL;
	if(db_mc_isSub(hContact))
		hMC = db_mc_getMeta(hContact);
	else if(metaIsProtoMetaContacts(hContact))
		hMC = db_mc_getMeta(hContact);

	StatusIconData sid = { sizeof(sid) };
	sid.szModule = szGPGModuleName;
	sid.hIcon = IconLibGetIcon("secured");
	sid.dwId = 1;
	sid.flags = enabled ? 0 : MBF_HIDDEN;
	Srmm_ModifyIcon(hContact, &sid);
	if(hMC)
		Srmm_ModifyIcon(hMC, &sid);

	sid.hIcon = IconLibGetIcon("unsecured");
	sid.dwId = 2;
	sid.flags = enabled ? MBF_HIDDEN : 0;
	Srmm_ModifyIcon(hContact, &sid);
	if(hMC)
		Srmm_ModifyIcon(hMC, &sid);
}


void RefreshContactListIcons() 
{
	CallService(MS_CLUI_LISTBEGINREBUILD,0,0);

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		setClistIcon(hContact);

	CallService(MS_CLUI_LISTENDREBUILD,0,0);
}
