/*

Omegle plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-12 Robert Pösel

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

extern OBJLIST<OmegleProto> g_Instances;

struct
{
	const char*  name;
	const char*  descr;
	int          defIconID;
	const char*  section;
}
static const icons[] =
{
	{ "omegle", LPGEN("Omegle Icon"), IDI_OMEGLE },
//	{ "homepage", LPGEN("Visit Profile"), 0, "core_main_2" }, 
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
			mir_snprintf(setting_name,sizeof(setting_name),"%s_%s","Omegle",icons[i].name);

			if (icons[i].section)
			{
				mir_snprintf(section_name,sizeof(section_name),"%s/%s/%s",LPGEN("Protocols"),
					LPGEN("Omegle"), icons[i].section);
			} else {
				mir_snprintf(section_name,sizeof(section_name),"%s/%s",LPGEN("Protocols"),
					LPGEN("Omegle"));
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

// Helper functions
static OmegleProto * GetInstanceByHContact(HANDLE hContact)
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

template<int (__cdecl OmegleProto::*Fcn)(WPARAM,LPARAM)>
INT_PTR GlobalService(WPARAM wParam,LPARAM lParam)
{
	OmegleProto *proto = GetInstanceByHContact(reinterpret_cast<HANDLE>(wParam));
	return proto ? (proto->*Fcn)(wParam,lParam) : 0;
}

