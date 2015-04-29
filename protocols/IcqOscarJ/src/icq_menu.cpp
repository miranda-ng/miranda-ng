// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2008 Joe Kucera, Bio
// Copyright © 2012-2014 Miranda NG Team
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
// -----------------------------------------------------------------------------

#include "stdafx.h"

#include <m_skin.h>

HGENMENU g_hContactMenuItems[6];
HANDLE   g_hContactMenuSvc[6];

static int sttCompareProtocols(const CIcqProto *p1, const CIcqProto *p2)
{
	return strcmp(p1->m_szModuleName, p2->m_szModuleName);
}

LIST<CIcqProto> g_Instances(1, sttCompareProtocols);

static CIcqProto* IcqGetInstanceByHContact(MCONTACT hContact)
{
	char* szProto = GetContactProto(hContact);
	if (szProto == NULL)
		return NULL;

	for (int i = 0; i < g_Instances.getCount(); i++)
		if (!strcmp(szProto, g_Instances[i]->m_szModuleName))
			return g_Instances[i];

	return NULL;
}

static INT_PTR IcqMenuHandleRequestAuth(WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = IcqGetInstanceByHContact(wParam);
	return (ppro) ? ppro->RequestAuthorization(wParam, lParam) : 0;
}

static INT_PTR IcqMenuHandleGrantAuth(WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = IcqGetInstanceByHContact(wParam);
	return (ppro) ? ppro->GrantAuthorization(wParam, lParam) : 0;
}

static INT_PTR IcqMenuHandleRevokeAuth(WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = IcqGetInstanceByHContact(wParam);
	return (ppro) ? ppro->RevokeAuthorization(wParam, lParam) : 0;
}

static INT_PTR IcqMenuHandleAddServContact(WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = IcqGetInstanceByHContact(wParam);
	return (ppro) ? ppro->AddServerContact(wParam, lParam) : 0;
}

static INT_PTR IcqMenuHandleXStatusDetails(WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = IcqGetInstanceByHContact(wParam);
	return (ppro) ? ppro->ShowXStatusDetails(wParam, lParam) : 0;
}

static INT_PTR IcqMenuHandleOpenProfile(WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = IcqGetInstanceByHContact(wParam);
	return (ppro) ? ppro->OpenWebProfile(wParam, lParam) : 0;
}

static int IcqPrebuildContactMenu( WPARAM wParam, LPARAM lParam )
{
	Menu_ShowItem(g_hContactMenuItems[ICMI_AUTH_REQUEST], FALSE);
	Menu_ShowItem(g_hContactMenuItems[ICMI_AUTH_GRANT], FALSE);
	Menu_ShowItem(g_hContactMenuItems[ICMI_AUTH_REVOKE], FALSE);
	Menu_ShowItem(g_hContactMenuItems[ICMI_ADD_TO_SERVLIST], FALSE);
	Menu_ShowItem(g_hContactMenuItems[ICMI_XSTATUS_DETAILS], FALSE);
	Menu_ShowItem(g_hContactMenuItems[ICMI_OPEN_PROFILE], FALSE);

	CIcqProto* ppro = IcqGetInstanceByHContact(wParam);
	return (ppro) ? ppro->OnPreBuildContactMenu(wParam, lParam) : 0;
}

void g_MenuInit(void)
{
	///////////////
	// Contact menu

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, IcqPrebuildContactMenu);

	// Contact menu initialization

	char str[MAXMODULELABELLENGTH], *pszDest = str + 3;
	strcpy(str, "ICQ");

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszService = str;

	// "Request authorization"
	strcpy(pszDest, MS_REQ_AUTH); CreateServiceFunction(str, IcqMenuHandleRequestAuth );

	mi.pszName = LPGEN("Request authorization");
	mi.position = 1000030000;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REQUEST);
	g_hContactMenuItems[ICMI_AUTH_REQUEST] = Menu_AddContactMenuItem(&mi);
	
	// "Grant authorization"
	strcpy(pszDest, MS_GRANT_AUTH); CreateServiceFunction(str, IcqMenuHandleGrantAuth);

	mi.pszName = LPGEN("Grant authorization");
	mi.position = 1000029999;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_GRANT);
	g_hContactMenuItems[ICMI_AUTH_GRANT] = Menu_AddContactMenuItem(&mi);
	
	// "Revoke authorization"
	strcpy(pszDest, MS_REVOKE_AUTH); CreateServiceFunction(str, IcqMenuHandleRevokeAuth);

	mi.pszName = LPGEN("Revoke authorization");
	mi.position = 1000029998;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REVOKE);
	g_hContactMenuItems[ICMI_AUTH_REVOKE] = Menu_AddContactMenuItem(&mi);
	
	// "Add to server list"
	strcpy(pszDest, MS_ICQ_ADDSERVCONTACT); CreateServiceFunction(str, IcqMenuHandleAddServContact);

	mi.pszName = LPGEN("Add to server list");
	mi.position = -2049999999;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_ADD);
	g_hContactMenuItems[ICMI_ADD_TO_SERVLIST] = Menu_AddContactMenuItem(&mi);
	
	// "Show custom status details"
 	strcpy(pszDest, MS_XSTATUS_SHOWDETAILS); CreateServiceFunction(str, IcqMenuHandleXStatusDetails);

	mi.pszName = LPGEN("Show custom status details");
	mi.position = -2000004999;
	mi.icolibItem = 0;
	g_hContactMenuItems[ICMI_XSTATUS_DETAILS] = Menu_AddContactMenuItem(&mi);
	
	// "Open ICQ profile"
	strcpy(pszDest, MS_OPEN_PROFILE); CreateServiceFunction(str, IcqMenuHandleOpenProfile);

	mi.pszName = LPGEN("Open ICQ profile");
	mi.position = 1000029997;
	g_hContactMenuItems[ICMI_OPEN_PROFILE] = Menu_AddContactMenuItem(&mi);
}

void g_MenuUninit(void)
{
	CallService(MO_REMOVEMENUITEM, (WPARAM)g_hContactMenuItems[ICMI_AUTH_REQUEST], 0);
	CallService(MO_REMOVEMENUITEM, (WPARAM)g_hContactMenuItems[ICMI_AUTH_GRANT], 0);
	CallService(MO_REMOVEMENUITEM, (WPARAM)g_hContactMenuItems[ICMI_AUTH_REVOKE], 0);
	CallService(MO_REMOVEMENUITEM, (WPARAM)g_hContactMenuItems[ICMI_ADD_TO_SERVLIST], 0);
	CallService(MO_REMOVEMENUITEM, (WPARAM)g_hContactMenuItems[ICMI_XSTATUS_DETAILS], 0);
	CallService(MO_REMOVEMENUITEM, (WPARAM)g_hContactMenuItems[ICMI_OPEN_PROFILE], 0);
}


INT_PTR CIcqProto::OpenWebProfile(WPARAM hContact, LPARAM)
{
	DWORD dwUin = getContactUin(hContact);
	char url[256];
	mir_snprintf(url, SIZEOF(url), "http://www.icq.com/people/%d",dwUin);
	return CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, (LPARAM)url);
}


int CIcqProto::OnPreBuildContactMenu(WPARAM hContact, LPARAM)
{
	if (hContact == NULL)
		return 0;

	if (icqOnline()) {
		BOOL bCtrlPressed = (GetKeyState(VK_CONTROL)&0x8000 ) != 0;

		DWORD dwUin = getContactUin(hContact);

		Menu_ShowItem(g_hContactMenuItems[ICMI_AUTH_REQUEST],
			dwUin && (bCtrlPressed || (getByte(hContact, "Auth", 0) && getWord(hContact, DBSETTING_SERVLIST_ID, 0))));
		Menu_ShowItem(g_hContactMenuItems[ICMI_AUTH_GRANT], dwUin && (bCtrlPressed || getByte(hContact, "Grant", 0)));
		Menu_ShowItem(g_hContactMenuItems[ICMI_AUTH_REVOKE],
			dwUin && (bCtrlPressed || (getByte("PrivacyItems", 0) && !getByte(hContact, "Grant", 0))));
		Menu_ShowItem(g_hContactMenuItems[ICMI_ADD_TO_SERVLIST],
			m_bSsiEnabled && !getWord(hContact, DBSETTING_SERVLIST_ID, 0) &&
			!getWord(hContact, DBSETTING_SERVLIST_IGNORE, 0) &&
			!db_get_b(hContact, "CList", "NotOnList", 0));
	}

	Menu_ShowItem(g_hContactMenuItems[ICMI_OPEN_PROFILE],getContactUin(hContact) != 0);
	BYTE bXStatus = getContactXStatus(hContact);

	Menu_ShowItem(g_hContactMenuItems[ICMI_XSTATUS_DETAILS], m_bHideXStatusUI ? 0 : bXStatus != 0);
	if (bXStatus && !m_bHideXStatusUI) {
		CLISTMENUITEM clmi = { sizeof(clmi) };
		clmi.flags = CMIM_ICON;
		if (bXStatus > 0 && bXStatus <= XSTATUS_COUNT)
			clmi.hIcon = getXStatusIcon(bXStatus, LR_SHARED);
		else
			clmi.hIcon = LoadSkinnedIcon(SKINICON_OTHER_SMALLDOT);
		Menu_ModifyItem(g_hContactMenuItems[ICMI_XSTATUS_DETAILS], &clmi);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnPreBuildStatusMenu event

int CIcqProto::OnPreBuildStatusMenu(WPARAM, LPARAM)
{
	InitXStatusItems(TRUE);
	return 0;
}
