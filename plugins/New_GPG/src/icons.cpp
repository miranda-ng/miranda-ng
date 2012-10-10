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

HANDLE IconLibDefine(TCHAR* desc, TCHAR* section, char* ident, HICON icon, char* def_file, int def_idx, int size)
{
  SKINICONDESC sid = {0};
  HANDLE hIcon;

  if(!size)
	  size = 16;

  sid.cbSize = sizeof( SKINICONDESC );
  sid.ptszSection = section;
  sid.ptszDescription = desc;
  sid.flags = SIDF_TCHAR;

  sid.pszName = ident;
  sid.pszDefaultFile = def_file;
  sid.iDefaultIndex = def_idx;
  sid.hDefaultIcon = icon;
  sid.cx = sid.cy = size;
  
  hIcon =  Skin_AddIcon(&sid);

  return hIcon;
}


void InitIconLib()
{
	extern HINSTANCE hInst;
    char lib[MAX_PATH];
    GetModuleFileNameA(hInst, lib, MAX_PATH);
	TCHAR *module = mir_a2t(szGPGModuleName);

	IconLibDefine(_T("Secured"), module, "secured", NULL, lib, -IDI_SECURED,0);
	IconLibDefine(_T("Unsecured"), module, "unsecured", NULL, lib, -IDI_UNSECURED,0);
	mir_free(module);
}





HICON IconLibGetIcon(const char* ident)
{
  return (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)ident);
}



void IconLibReleaseIcon(const char* ident)
{
  CallService(MS_SKIN2_RELEASEICON, 0, (LPARAM)ident);
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
	if(g_hCLIcon && enabled)
	{
		HICON icon = IconLibGetIcon("secured");
		IconExtraColumn iec = {0};
		iec.cbSize = sizeof(iec);
		iec.hImage = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)icon, (LPARAM)0);
		ExtraIcon_SetIcon(g_hCLIcon, hContact, iec.hImage);
		if(hMC)
			ExtraIcon_SetIcon(g_hCLIcon, hMC, iec.hImage);
	}
	else
	{
		ExtraIcon_SetIcon(g_hCLIcon, hContact, (HANDLE)0); // is it right ? hmm.., at least working....
		if(hMC)
			ExtraIcon_SetIcon(g_hCLIcon, hMC, (HANDLE)0);
	}
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
	extern HANDLE g_hCLIcon;
	CallService(MS_CLUI_LISTBEGINREBUILD,0,0);
	HANDLE hContact = db_find_first();
	while (hContact) 
	{
		setClistIcon(hContact);
		hContact = db_find_next(hContact);
	}
	CallService(MS_CLUI_LISTENDREBUILD,0,0);
}
