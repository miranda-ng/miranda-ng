////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2009-2012 Bartosz Bia³ek
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
////////////////////////////////////////////////////////////////////////////////

#include "gg.h"
#include "m_assocmgr.h"

//////////////////////////////////////////////////////////
// File Association Manager support

#define GGS_PARSELINK "%s/ParseLink"
#define GGS_MENUCHOOSE "%s/MenuChoose"

static HANDLE hInstanceMenu;
static HANDLE hServiceMenuChoose;
static HANDLE hServiceParseLink;

static INT_PTR gg_menuchoose(WPARAM wParam, LPARAM lParam)
{
	if (lParam)
		*(void**)lParam = (void*)wParam;
	return 0;
}

static INT_PTR gg_parselink(WPARAM wParam, LPARAM lParam)
{
	char *arg = (char*)lParam;
	list_t l = g_Instances;
	GGPROTO *gg = NULL;
	uin_t uin;
	CLISTMENUITEM mi = {0};
	int items = 0;

	if (list_count(l) == 0)
		return 0;

	if (arg == NULL)
		return 1;

	arg = strchr(arg, ':');

	if (arg == NULL)
		return 1;

	for (++arg; *arg == '/'; ++arg);
	uin = atoi(arg);

	if (!uin)
		return 1;

	for (mi.cbSize = sizeof(mi); l; l = l->next)
	{
		GGPROTO *gginst = l->data;

		mi.flags = CMIM_FLAGS;
		if (gginst->proto.m_iStatus > ID_STATUS_OFFLINE)
		{
			++items;
			gg = l->data;
			mi.flags |= CMIM_ICON;
			mi.hIcon = LoadSkinnedProtoIcon(GG_PROTO, gg->proto.m_iStatus);
		}
		else
		{
			mi.flags |= CMIF_HIDDEN;
			mi.hIcon = NULL;
		}

		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)gginst->hInstanceMenuItem, (LPARAM)&mi);
		if (mi.hIcon)
			CallService(MS_SKIN2_RELEASEICON, (WPARAM)mi.hIcon, 0);
	}

	if (items > 1)
	{
		ListParam param = {0};
		HMENU hMenu = CreatePopupMenu();
		POINT pt;
		int cmd = 0;

		param.MenuObjectHandle = hInstanceMenu;
		CallService(MO_BUILDMENU, (WPARAM)hMenu, (LPARAM)&param);

		GetCursorPos(&pt);
		cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, pcli->hwndContactList, NULL);
		DestroyMenu(hMenu);

		if (cmd)
			CallService(MO_PROCESSCOMMANDBYMENUIDENT, cmd, (LPARAM)&gg);
	}

	if (gg == NULL)
		return 0;

	if (ServiceExists(MS_MSG_SENDMESSAGE))
	{
		HANDLE hContact = gg_getcontact(gg, uin, 1, 0, NULL);
		if (hContact != NULL)
			CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact, 0);
	}

	return 0;
}

void gg_links_instancemenu_init()
{
	char service[MAXMODULELABELLENGTH];
	TMenuParam mnu = {0};
	TMO_MenuItem tmi = {0};

	mir_snprintf(service, sizeof(service), GGS_MENUCHOOSE, GGDEF_PROTO);
	hServiceMenuChoose = CreateServiceFunction(service, gg_menuchoose);
	mnu.cbSize = sizeof(mnu);
	mnu.name = "GGAccountChooser";
	mnu.ExecService = service;
	hInstanceMenu = (HANDLE)CallService(MO_CREATENEWMENUOBJECT, 0, (LPARAM)&mnu);

	tmi.cbSize = sizeof(tmi);
	tmi.flags = CMIF_ICONFROMICOLIB;
	tmi.pszName = "Cancel";
	tmi.position = 9999999;
	tmi.hIcolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_DELETE);
	CallService(MO_ADDNEWMENUITEM, (WPARAM)hInstanceMenu, (LPARAM)&tmi);
}

void gg_links_init()
{
	if (ServiceExists(MS_ASSOCMGR_ADDNEWURLTYPE))
	{
		char service[MAXMODULELABELLENGTH];

		mir_snprintf(service, sizeof(service), GGS_PARSELINK, GGDEF_PROTO);
		hServiceParseLink = CreateServiceFunction(service, gg_parselink);
		AssocMgr_AddNewUrlType("gg:", Translate("Gadu-Gadu Link Protocol"), hInstance, IDI_GG, service, 0);
	}
}

void gg_links_destroy()
{
	DestroyServiceFunction(hServiceMenuChoose);
	if (ServiceExists(MS_ASSOCMGR_ADDNEWURLTYPE))
		DestroyServiceFunction(hServiceParseLink);
}

void gg_links_instance_init(GGPROTO *gg)
{
	if (ServiceExists(MS_ASSOCMGR_ADDNEWURLTYPE))
	{
		TMO_MenuItem tmi = {0};
		tmi.cbSize = sizeof(tmi);
		tmi.flags = CMIF_TCHAR;
		tmi.ownerdata = gg;
		tmi.position = list_count(g_Instances);
		tmi.ptszName = GG_PROTONAME;
		gg->hInstanceMenuItem = (HANDLE)CallService(MO_ADDNEWMENUITEM, (WPARAM)hInstanceMenu, (LPARAM)&tmi);
	}
}
