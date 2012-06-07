/*
Copyright © 2009 Jim Porter

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

#include "theme.h"
#include "proto.h"

extern OBJLIST<TwitterProto> g_Instances;

struct
{
	const char*  name;
	const char*  descr;
	int          defIconID;
	const char*  section;
}
static const icons[] =
{
	{ "twitter",  "Twitter Icon",   IDI_TWITTER },
	{ "tweet",    "Tweet",          IDI_TWITTER },
	{ "reply",    "Reply to Tweet", IDI_TWITTER },

	{ "homepage", "Visit Homepage", 0, "core_main_2" }, 
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
			mir_snprintf(setting_name,sizeof(setting_name),"%s_%s","Twitter",icons[i].name);

			if (icons[i].section)
			{
				mir_snprintf(section_name,sizeof(section_name),"%s/%s/%s",LPGEN("Protocols"),
					LPGEN("Twitter"), icons[i].section);
			}
			else
			{
				mir_snprintf(section_name,sizeof(section_name),"%s/%s",LPGEN("Protocols"),
					LPGEN("Twitter"));
			}

			sid.pszDescription = (char*)icons[i].descr;
			sid.iDefaultIndex = -icons[i].defIconID;
			hIconLibItem[i] = (HANDLE)CallService(MS_SKIN2_ADDICON,0,(LPARAM)&sid);
		}
		else // External icons
		{
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



// Contact List menu stuff
static HANDLE g_hMenuItems[2];
static HANDLE g_hMenuEvts[3];

// Helper functions
static TwitterProto * GetInstanceByHContact(HANDLE hContact)
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

template<int (__cdecl TwitterProto::*Fcn)(WPARAM,LPARAM)>
INT_PTR GlobalService(WPARAM wParam,LPARAM lParam)
{
	TwitterProto *proto = GetInstanceByHContact(reinterpret_cast<HANDLE>(wParam));
	return proto ? (proto->*Fcn)(wParam,lParam) : 0;
}

static int PrebuildContactMenu(WPARAM wParam,LPARAM lParam)
{
	ShowContactMenus(false);

	TwitterProto *proto = GetInstanceByHContact(reinterpret_cast<HANDLE>(wParam));
	return proto ? proto->OnPrebuildContactMenu(wParam,lParam) : 0;
}

void InitContactMenus()
{
	g_hMenuEvts[0] = HookEvent(ME_CLIST_PREBUILDCONTACTMENU,
		PrebuildContactMenu);

	CLISTMENUITEM mi = {sizeof(mi)};
	mi.flags = CMIF_NOTOFFLINE | CMIF_ICONFROMICOLIB;

	mi.position=-2000006000;
	mi.icolibItem = GetIconHandle("reply");
	mi.pszName = LPGEN("Reply...");
	mi.pszService = "Twitter/ReplyToTweet";
	g_hMenuEvts[1] = CreateServiceFunction(mi.pszService, GlobalService<&TwitterProto::ReplyToTweet>);
	g_hMenuItems[0] = reinterpret_cast<HANDLE>(CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi) );

	mi.position=-2000006000;
	mi.icolibItem = GetIconHandle("homepage");
	mi.pszName = LPGEN("Visit Homepage");
	mi.pszService = "Twitter/VisitHomepage";
	g_hMenuEvts[2] = CreateServiceFunction(mi.pszService,
		GlobalService<&TwitterProto::VisitHomepage>);
	g_hMenuItems[1] = reinterpret_cast<HANDLE>(
		CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi) );
}

void UninitContactMenus()
{
	for(size_t i=0; i<SIZEOF(g_hMenuItems); i++)
		CallService(MS_CLIST_REMOVECONTACTMENUITEM,(WPARAM)g_hMenuItems[i],0);

	UnhookEvent(g_hMenuEvts[0]);
	for(size_t i=1; i<SIZEOF(g_hMenuEvts); i++)
		DestroyServiceFunction(g_hMenuEvts[i]);
}

void ShowContactMenus(bool show)
{
	for(size_t i=0; i<SIZEOF(g_hMenuItems); i++)
	{
		CLISTMENUITEM item = { sizeof(item) };
		item.flags = CMIM_FLAGS | CMIF_NOTOFFLINE;
		if(!show)
			item.flags |= CMIF_HIDDEN;

		CallService(MS_CLIST_MODIFYMENUITEM,reinterpret_cast<WPARAM>(g_hMenuItems[i]),
			reinterpret_cast<LPARAM>(&item));
	}
}