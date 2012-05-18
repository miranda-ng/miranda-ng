/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-12 Robert Pösel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "common.h"

extern OBJLIST<FacebookProto> g_Instances;

struct
{
	const char*  name;
	char*  descr;
	int          defIconID;
	const char*  section;
}
static const icons[] =
{
	{ "facebook",		LPGEN("Facebook Icon"),			IDI_FACEBOOK },
	{ "mind",			LPGEN("Mind"),					IDI_MIND },
	{ "removeFriend",	LPGEN("Remove from server"),	IDI_REMOVEFRIEND },
	{ "addFriend",		LPGEN("Request friendship"),	IDI_ADDFRIEND },

	{ "homepage",		LPGEN("Visit Profile"),	0, "core_main_2" },
};

static HANDLE hIconLibItem[SIZEOF(icons)];

// TODO: uninit
void InitIcons(void)
{
	TCHAR szFile[MAX_PATH];
	GetModuleFileName(g_hInstance, szFile, SIZEOF(szFile));

	char setting_name[100];
	char section_name[100];

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(SKINICONDESC);
	sid.ptszDefaultFile = szFile;
	sid.cx = sid.cy = 16;
	sid.pszName = setting_name;
	sid.pszSection = section_name;
    sid.flags = SIDF_PATH_TCHAR;

	for (int i=0; i<SIZEOF(icons); i++) 
	{
		if(icons[i].defIconID)
		{
			mir_snprintf(setting_name,sizeof(setting_name),"%s_%s","Facebook",icons[i].name);

			if (icons[i].section)
			{
				mir_snprintf(section_name,sizeof(section_name),"%s/%s/%s",LPGEN("Protocols"),
					LPGEN("Facebook"), icons[i].section);
			} else {
				mir_snprintf(section_name,sizeof(section_name),"%s/%s",LPGEN("Protocols"),
					LPGEN("Facebook"));
			}

			sid.pszDescription = (char*)icons[i].descr;
			sid.iDefaultIndex = -icons[i].defIconID;
			hIconLibItem[i] = (HANDLE)CallService(MS_SKIN2_ADDICON,0,(LPARAM)&sid);
		} else { // External icons
			hIconLibItem[i] = (HANDLE)CallService(MS_SKIN2_GETICONHANDLE,0,
				(LPARAM)icons[i].section);
		}
	}	
}

HANDLE GetIconHandle(const char* name)
{
	for(size_t i=0; i<SIZEOF(icons); i++)
	{
		if(strcmp(icons[i].name,name) == 0)
			return hIconLibItem[i];
	}
	return 0;
}

char *GetIconDescription(const char* name)
{
	for(size_t i=0; i<SIZEOF(icons); i++)
	{
		if(strcmp(icons[i].name,name) == 0)
			return icons[i].descr;
	}
	return "";
}

// Contact List menu stuff
HANDLE g_hMenuItems[4];

// Helper functions
static FacebookProto * GetInstanceByHContact(HANDLE hContact)
{
	char *proto = reinterpret_cast<char*>( CallService(MS_PROTO_GETCONTACTBASEPROTO,
		reinterpret_cast<WPARAM>(hContact),0) );
	if(!proto)
		return 0;

	for(int i=0; i<g_Instances.getCount(); i++)
		if(!strcmp(proto,g_Instances[i].m_szModuleName))
			return &g_Instances[i];

	return 0;
}

template<int (__cdecl FacebookProto::*Fcn)(WPARAM,LPARAM)>
INT_PTR GlobalService(WPARAM wParam,LPARAM lParam)
{
	FacebookProto *proto = GetInstanceByHContact(reinterpret_cast<HANDLE>(wParam));
	return proto ? (proto->*Fcn)(wParam,lParam) : 0;
}

static int PrebuildContactMenu(WPARAM wParam,LPARAM lParam)
{
	ShowContactMenus(false);

	FacebookProto *proto = GetInstanceByHContact(reinterpret_cast<HANDLE>(wParam));
	return proto ? proto->OnPrebuildContactMenu(wParam,lParam) : 0;
}

HANDLE hHookPreBuildMenu,sVisitProfile,sAddFriend,sRemoveFriend;
void InitContactMenus()
{
	hHookPreBuildMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU,PrebuildContactMenu);

	CLISTMENUITEM mi = {sizeof(mi)};
	mi.flags = CMIF_ICONFROMICOLIB;

	mi.position=-2000006000;
	mi.icolibItem = GetIconHandle("homepage");
	mi.pszName = GetIconDescription("homepage");
	mi.pszService = "FacebookProto/VisitProfile";
	sVisitProfile = CreateServiceFunction(mi.pszService,GlobalService<&FacebookProto::VisitProfile>);
	g_hMenuItems[1] = reinterpret_cast<HANDLE>(
		CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi) );

	mi.position=-2000006000;
	mi.icolibItem = GetIconHandle("removeFriend");
	mi.pszName = GetIconDescription("removeFriend");
	mi.pszService = "FacebookProto/RemoveFriend";
	sRemoveFriend = CreateServiceFunction(mi.pszService,GlobalService<&FacebookProto::RemoveFriend>);
	g_hMenuItems[2] = reinterpret_cast<HANDLE>(
		CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi) );

	mi.position=-2000006000;
	mi.icolibItem = GetIconHandle("addFriend");
	mi.pszName = GetIconDescription("addFriend");
	mi.pszService = "FacebookProto/AddFriend";
	sAddFriend = CreateServiceFunction(mi.pszService,GlobalService<&FacebookProto::AddFriend>);
	g_hMenuItems[3] = reinterpret_cast<HANDLE>(
		CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi) );
}

void UninitContactMenus()
{
	for(size_t i=0; i<SIZEOF(g_hMenuItems); i++)
		CallService(MS_CLIST_REMOVECONTACTMENUITEM,(WPARAM)g_hMenuItems[i],0);
	UnhookEvent(hHookPreBuildMenu);
	DestroyServiceFunction(sVisitProfile);
	DestroyServiceFunction(sRemoveFriend);
	DestroyServiceFunction(sAddFriend);
}

void ShowContactMenus(bool show, bool deleted)
{
	for(size_t i=0; i<SIZEOF(g_hMenuItems); i++)
	{
		CLISTMENUITEM item = { sizeof(item) };
		item.flags = CMIM_FLAGS;
		if(!show || (i == 3 && !deleted) || (i == 2 && deleted)) // 2 = REMOVE CONTACT; 3 = ADD CONTACT
			item.flags |= CMIF_HIDDEN;

		CallService(MS_CLIST_MODIFYMENUITEM,reinterpret_cast<WPARAM>(g_hMenuItems[i]),
			reinterpret_cast<LPARAM>(&item));
	}
}
