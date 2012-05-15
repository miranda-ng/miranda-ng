// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2008 Joe Kucera, Bio
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
//
// -----------------------------------------------------------------------------
//
// File name      : $URL: icq_menu.cpp $
// Revision       : $Revision: 0 $
// Last change on : $Date: 2009-12-21 19:52:36 +0300 $
// Last change by : $Author: Unsane $
//
// DESCRIPTION:
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"

#include <m_skin.h>

static HANDLE hPrebuildMenuHook;

HANDLE g_hContactMenuItems[5];
HANDLE g_hContactMenuSvc[5];

static int sttCompareProtocols(const CIcqProto *p1, const CIcqProto *p2)
{
	return strcmp(p1->m_szModuleName, p2->m_szModuleName);
}

LIST<CIcqProto> g_Instances(1, sttCompareProtocols);

static CIcqProto* IcqGetInstanceByHContact(HANDLE hContact)
{
	char* szProto = ( char* )CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (szProto == NULL)
		return NULL;

	for (int i = 0; i < g_Instances.getCount(); i++)
		if (!strcmp(szProto, g_Instances[i]->m_szModuleName))
			return g_Instances[i];

	return NULL;
}

static INT_PTR IcqMenuHandleRequestAuth(WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = IcqGetInstanceByHContact((HANDLE)wParam);
	return (ppro) ? ppro->RequestAuthorization(wParam, lParam) : 0;
}

static INT_PTR IcqMenuHandleGrantAuth(WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = IcqGetInstanceByHContact((HANDLE)wParam);
	return (ppro) ? ppro->GrantAuthorization(wParam, lParam) : 0;
}

static INT_PTR IcqMenuHandleRevokeAuth(WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = IcqGetInstanceByHContact((HANDLE)wParam);
	return (ppro) ? ppro->RevokeAuthorization(wParam, lParam) : 0;
}

static INT_PTR IcqMenuHandleAddServContact(WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = IcqGetInstanceByHContact((HANDLE)wParam);
	return (ppro) ? ppro->AddServerContact(wParam, lParam) : 0;
}

static INT_PTR IcqMenuHandleXStatusDetails(WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = IcqGetInstanceByHContact((HANDLE)wParam);
	return (ppro) ? ppro->ShowXStatusDetails(wParam, lParam) : 0;
}

static void sttEnableMenuItem( HANDLE hMenuItem, bool bEnable )
{
	CLISTMENUITEM clmi = {0};
	clmi.cbSize = sizeof( CLISTMENUITEM );
	clmi.flags = CMIM_FLAGS;
	if ( !bEnable )
		clmi.flags |= CMIF_HIDDEN;

	CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuItem, ( LPARAM )&clmi );
}

static int IcqPrebuildContactMenu( WPARAM wParam, LPARAM lParam )
{
	sttEnableMenuItem(g_hContactMenuItems[ICMI_AUTH_REQUEST], FALSE);
	sttEnableMenuItem(g_hContactMenuItems[ICMI_AUTH_GRANT], FALSE);
	sttEnableMenuItem(g_hContactMenuItems[ICMI_AUTH_REVOKE], FALSE);
	sttEnableMenuItem(g_hContactMenuItems[ICMI_ADD_TO_SERVLIST], FALSE);
	sttEnableMenuItem(g_hContactMenuItems[ICMI_XSTATUS_DETAILS], FALSE);
	
	CIcqProto* ppro = IcqGetInstanceByHContact((HANDLE)wParam);
	return (ppro) ? ppro->OnPreBuildContactMenu(wParam, lParam) : 0;
}

void g_MenuInit(void)
{
	///////////////
	// Contact menu

	hPrebuildMenuHook = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, IcqPrebuildContactMenu);

	// Contact menu initialization

	char str[MAXMODULELABELLENGTH], *pszDest = str + 3;
	strcpy( str, "ICQ" );
	
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof( CLISTMENUITEM );
	mi.pszService = str;
	mi.flags = CMIF_ICONFROMICOLIB;

	// "Request authorization"
	mi.pszName = LPGEN("Request authorization");
	mi.position = 1000030000;
	mi.icolibItem = hStaticIcons[ISI_AUTH_REQUEST]->Handle();
	strcpy(pszDest, MS_REQ_AUTH);
	g_hContactMenuItems[ICMI_AUTH_REQUEST] = (HGENMENU)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
	g_hContactMenuSvc[ICMI_AUTH_REQUEST] = CreateServiceFunction( str, IcqMenuHandleRequestAuth );

	// "Grant authorization"
	mi.pszName = LPGEN("Grant authorization");
	mi.position = 1000029999;
	mi.icolibItem = hStaticIcons[ISI_AUTH_GRANT]->Handle();
	strcpy(pszDest, MS_GRANT_AUTH);
	g_hContactMenuItems[ICMI_AUTH_GRANT] = (HGENMENU)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
	g_hContactMenuSvc[ICMI_AUTH_GRANT] = CreateServiceFunction(mi.pszService, IcqMenuHandleGrantAuth);

	// "Revoke authorization"
	mi.pszName = LPGEN("Revoke authorization");
	mi.position = 1000029998;
	mi.icolibItem = hStaticIcons[ISI_AUTH_REVOKE]->Handle();
	strcpy(pszDest, MS_REVOKE_AUTH);
	g_hContactMenuItems[ICMI_AUTH_REVOKE] = (HGENMENU)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
	g_hContactMenuSvc[ICMI_AUTH_REVOKE] = CreateServiceFunction(mi.pszService, IcqMenuHandleRevokeAuth);

	// "Add to server list"
	mi.pszName = LPGEN("Add to server list");
	mi.position = -2049999999;
	mi.icolibItem = hStaticIcons[ISI_ADD_TO_SERVLIST]->Handle();
	strcpy(pszDest, MS_ICQ_ADDSERVCONTACT);
	g_hContactMenuItems[ICMI_ADD_TO_SERVLIST] = (HGENMENU)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
	g_hContactMenuSvc[ICMI_ADD_TO_SERVLIST] = CreateServiceFunction(mi.pszService, IcqMenuHandleAddServContact);

	// "Show custom status details"
 	mi.pszName = LPGEN("Show custom status details");
	mi.position = -2000004999;
	mi.flags = 0;
	strcpy(pszDest, MS_XSTATUS_SHOWDETAILS);
	g_hContactMenuItems[ICMI_XSTATUS_DETAILS] = (HGENMENU)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
	g_hContactMenuSvc[ICMI_XSTATUS_DETAILS] = CreateServiceFunction(mi.pszService, IcqMenuHandleXStatusDetails);
}

void g_MenuUninit(void)
{
	UnhookEvent(hPrebuildMenuHook);

	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)g_hContactMenuItems[ICMI_AUTH_REQUEST], 0);
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)g_hContactMenuItems[ICMI_AUTH_GRANT], 0);
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)g_hContactMenuItems[ICMI_AUTH_REVOKE], 0);
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)g_hContactMenuItems[ICMI_ADD_TO_SERVLIST], 0);
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)g_hContactMenuItems[ICMI_XSTATUS_DETAILS], 0);

	DestroyServiceFunction(g_hContactMenuSvc[ICMI_AUTH_REQUEST]);
	DestroyServiceFunction(g_hContactMenuSvc[ICMI_AUTH_GRANT]);
	DestroyServiceFunction(g_hContactMenuSvc[ICMI_AUTH_REVOKE]);
	DestroyServiceFunction(g_hContactMenuSvc[ICMI_ADD_TO_SERVLIST]);
	DestroyServiceFunction(g_hContactMenuSvc[ICMI_XSTATUS_DETAILS]);

}

int CIcqProto::OnPreBuildContactMenu(WPARAM wParam, LPARAM)
{
	HANDLE hContact = (HANDLE)wParam;
	if (hContact == NULL)
		return 0;

	if (icqOnline())
	{
		BOOL bCtrlPressed = (GetKeyState(VK_CONTROL)&0x8000 ) != 0;

		DWORD dwUin = getContactUin(hContact);


		sttEnableMenuItem(g_hContactMenuItems[ICMI_AUTH_REQUEST], 
			dwUin && (bCtrlPressed || (getSettingByte((HANDLE)wParam, "Auth", 0) && getSettingWord((HANDLE)wParam, DBSETTING_SERVLIST_ID, 0))));
		sttEnableMenuItem(g_hContactMenuItems[ICMI_AUTH_GRANT], dwUin && (bCtrlPressed || getSettingByte((HANDLE)wParam, "Grant", 0)));
		sttEnableMenuItem(g_hContactMenuItems[ICMI_AUTH_REVOKE], 
			dwUin && (bCtrlPressed || (getSettingByte(NULL, "PrivacyItems", 0) && !getSettingByte((HANDLE)wParam, "Grant", 0))));

		sttEnableMenuItem(g_hContactMenuItems[ICMI_ADD_TO_SERVLIST], 
			m_bSsiEnabled && !getSettingWord((HANDLE)wParam, DBSETTING_SERVLIST_ID, 0) && 
			!getSettingWord((HANDLE)wParam, DBSETTING_SERVLIST_IGNORE, 0) &&
			!DBGetContactSettingByte(hContact, "CList", "NotOnList", 0));
	}

	BYTE bXStatus = getContactXStatus((HANDLE)wParam);
	
	sttEnableMenuItem(g_hContactMenuItems[ICMI_XSTATUS_DETAILS], m_bHideXStatusUI ? 0 : bXStatus != 0);
	if (bXStatus && !m_bHideXStatusUI) {
		CLISTMENUITEM clmi = {0};

		clmi.cbSize = sizeof(clmi);
		clmi.flags = CMIM_ICON;

		if (bXStatus > 0 && bXStatus <= XSTATUS_COUNT)
			clmi.hIcon = getXStatusIcon(bXStatus, LR_SHARED);
		else
			clmi.hIcon = LoadSkinnedIcon(SKINICON_OTHER_SMALLDOT);
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)g_hContactMenuItems[ICMI_XSTATUS_DETAILS], (LPARAM)&clmi);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnPrebuildContactMenu event

int CIcqProto::OnPreBuildStatusMenu(WPARAM wParam, LPARAM lParam)
{
	InitXStatusItems(TRUE);
	return 0;
}
