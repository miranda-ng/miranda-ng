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
	{ "facebook",		LPGEN("Facebook Icon"),				IDI_FACEBOOK },
	{ "mind",			LPGEN("Mind"),						IDI_MIND },
	
	{ "authRevoke",		LPGEN("Cancel friendship"),			IDI_AUTH_REVOKE },
	//{ "authRevokeReq",	LPGEN("Cancel friendship request"),	IDI_AUTH_REVOKE },
	{ "authAsk",		LPGEN("Request friendship"),		IDI_AUTH_ASK },
	{ "authGrant",		LPGEN("Approve friendship"),		IDI_AUTH_GRANT },
	
	{ "homepage",		LPGEN("Visit Profile"),				0, "core_main_2" },
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
HANDLE hHookPreBuildMenu;
HANDLE g_hContactMenuItems[CMITEMS_COUNT];
HANDLE g_hContactMenuSvc[CMITEMS_COUNT];

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
	for (size_t i=0; i<SIZEOF(g_hContactMenuItems); i++)
	{
		EnableMenuItem(g_hContactMenuItems[i], false);
	}

	FacebookProto *proto = GetInstanceByHContact(reinterpret_cast<HANDLE>(wParam));
	return proto ? proto->OnPrebuildContactMenu(wParam,lParam) : 0;
}

void InitContactMenus()
{
	hHookPreBuildMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU,PrebuildContactMenu);

	CLISTMENUITEM mi = {sizeof(mi)};
	mi.flags = CMIF_ICONFROMICOLIB;

	mi.position=-2000006000;
	mi.icolibItem = GetIconHandle("homepage");
	mi.pszName = GetIconDescription("homepage");
	mi.pszService = "FacebookProto/VisitProfile";
	g_hContactMenuSvc[CMI_VISIT_PROFILE] = CreateServiceFunction(mi.pszService,GlobalService<&FacebookProto::VisitProfile>);
	g_hContactMenuItems[CMI_VISIT_PROFILE] = reinterpret_cast<HANDLE>(
		CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi) );

	mi.position=-2000006001;
	mi.icolibItem = GetIconHandle("authRevoke");
	mi.pszName = GetIconDescription("authRevoke");
	mi.pszService = "FacebookProto/CancelFriendship";
	g_hContactMenuSvc[CMI_AUTH_REVOKE] = CreateServiceFunction(mi.pszService,GlobalService<&FacebookProto::CancelFriendship>);
	g_hContactMenuItems[CMI_AUTH_REVOKE] = reinterpret_cast<HANDLE>(
		CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi) );

	/* mi.position=-2000006001;
	mi.icolibItem = GetIconHandle("authRevokeReq");
	mi.pszName = GetIconDescription("authRevokeReq");
	mi.pszService = "FacebookProto/CancelFriendshipRequest";
	g_hContactMenuSvc[CMI_AUTH_REVOKE_REQ] = CreateServiceFunction(mi.pszService,GlobalService<&FacebookProto::CancelFriendshipRequest>);
	g_hContactMenuItems[CMI_AUTH_REVOKE_REQ] = reinterpret_cast<HANDLE>(
		CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi) ); */

	mi.position=-2000006002;
	mi.icolibItem = GetIconHandle("authAsk");
	mi.pszName = GetIconDescription("authAsk");
	mi.pszService = "FacebookProto/RequestFriendship";
	g_hContactMenuSvc[CMI_AUTH_ASK] = CreateServiceFunction(mi.pszService,GlobalService<&FacebookProto::RequestFriendship>);
	g_hContactMenuItems[CMI_AUTH_ASK] = reinterpret_cast<HANDLE>(
		CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi) );

	mi.position=-2000006003;
	mi.icolibItem = GetIconHandle("authGrant");
	mi.pszName = GetIconDescription("authGrant");
	mi.pszService = "FacebookProto/ApproveFriendship";
	g_hContactMenuSvc[CMI_AUTH_GRANT] = CreateServiceFunction(mi.pszService,GlobalService<&FacebookProto::ApproveFriendship>);
	g_hContactMenuItems[CMI_AUTH_GRANT] = reinterpret_cast<HANDLE>(
		CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi) );
}

void UninitContactMenus()
{
	for(size_t i=0; i<SIZEOF(g_hContactMenuItems); i++)
		CallService(MS_CLIST_REMOVECONTACTMENUITEM,(WPARAM)g_hContactMenuItems[i],0);

	for(size_t i=0; i<SIZEOF(g_hContactMenuSvc); i++)
		DestroyServiceFunction(g_hContactMenuSvc[i]);
	
	UnhookEvent(hHookPreBuildMenu);
}

void EnableMenuItem(HANDLE hMenuItem, bool enable)
{
	CLISTMENUITEM clmi = {0};
	clmi.cbSize = sizeof(CLISTMENUITEM);
	clmi.flags = CMIM_FLAGS;
	if (!enable)
		clmi.flags |= CMIF_HIDDEN;

	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItem, (LPARAM)&clmi);
}

int FacebookProto::OnPrebuildContactMenu(WPARAM wParam,LPARAM lParam)
{	
	HANDLE hContact = reinterpret_cast<HANDLE>(wParam);

	EnableMenuItem(g_hContactMenuItems[CMI_VISIT_PROFILE], true);

	if (!isOffline() && !DBGetContactSettingByte(hContact, m_szModuleName, "ChatRoom", 0))
	{
		bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

		BYTE type = DBGetContactSettingDword(hContact, m_szModuleName, FACEBOOK_KEY_CONTACT_TYPE, 0);

		EnableMenuItem(g_hContactMenuItems[CMI_AUTH_ASK], ctrlPressed || type == FACEBOOK_CONTACT_NONE || !type);
		EnableMenuItem(g_hContactMenuItems[CMI_AUTH_GRANT], ctrlPressed || type == FACEBOOK_CONTACT_APPROVE);
		EnableMenuItem(g_hContactMenuItems[CMI_AUTH_REVOKE], ctrlPressed || type == FACEBOOK_CONTACT_FRIEND);
		//EnableMenuItem(g_hContactMenuItems[CMI_AUTH_CANCEL], ctrlPressed || type == FACEBOOK_CONTACT_REQUEST);
	}

	return 0;
}

int FacebookProto::OnBuildStatusMenu(WPARAM wParam,LPARAM lParam)
{
	char text[200];
	strcpy(text,m_szModuleName);
	char *tDest = text+strlen(text);

	HGENMENU hRoot;
	CLISTMENUITEM mi = {sizeof(mi)};
	mi.pszService = text;

	hRoot = MO_GetProtoRootMenu(m_szModuleName);
	if (hRoot == NULL)
	{
		mi.popupPosition = 500085000;
		mi.hParentMenu = HGENMENU_ROOT;
		mi.flags = CMIF_ICONFROMICOLIB | CMIF_ROOTPOPUP | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED | ( this->isOnline() ? 0 : CMIF_GRAYED );
		mi.icolibItem = GetIconHandle( "facebook" );
		mi.ptszName = m_tszUserName;
		hRoot = m_hMenuRoot = reinterpret_cast<HGENMENU>( CallService(
			MS_CLIST_ADDPROTOMENUITEM,0,reinterpret_cast<LPARAM>(&mi)) );
	} else {
		if ( m_hMenuRoot )
			CallService( MS_CLIST_REMOVEMAINMENUITEM, ( WPARAM )m_hMenuRoot, 0 );
		m_hMenuRoot = NULL;
	}

	mi.flags = CMIF_ICONFROMICOLIB | CMIF_CHILDPOPUP | ( this->isOnline() ? 0 : CMIF_GRAYED );
	mi.position = 201001;

	CreateProtoService(m_szModuleName,"/Mind",&FacebookProto::OnMind,this);
	strcpy(tDest,"/Mind");
	mi.hParentMenu = hRoot;
	mi.pszName = LPGEN("Mind...");
	mi.icolibItem = GetIconHandle("mind");
	m_hStatusMind = reinterpret_cast<HGENMENU>( CallService(
		MS_CLIST_ADDPROTOMENUITEM,0,reinterpret_cast<LPARAM>(&mi)) );

	CreateProtoService(m_szModuleName,"/VisitProfile",&FacebookProto::VisitProfile,this);
	strcpy(tDest,"/VisitProfile");
	mi.flags = CMIF_ICONFROMICOLIB | CMIF_CHILDPOPUP;
	mi.pszName = LPGEN("Visit Profile");
	mi.icolibItem = GetIconHandle("homepage");
	// TODO RM: remember and properly free in destructor?
	/*m_hStatusMind = */reinterpret_cast<HGENMENU>( CallService(
		MS_CLIST_ADDPROTOMENUITEM,0,reinterpret_cast<LPARAM>(&mi)) );

	return 0;
}

void FacebookProto::ToggleStatusMenuItems( BOOL bEnable )
{
	CLISTMENUITEM clmi = { 0 };
	clmi.cbSize = sizeof( CLISTMENUITEM );
	clmi.flags = CMIM_FLAGS | (( bEnable ) ? 0 : CMIF_GRAYED);

	CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )m_hMenuRoot,   ( LPARAM )&clmi );
	CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )m_hStatusMind, ( LPARAM )&clmi );
}
