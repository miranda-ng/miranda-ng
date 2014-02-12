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

#define GGS_PARSELINK "GG/ParseLink"
#define GGS_MENUCHOOSE "GG/MenuChoose"

static HANDLE hInstanceMenu;

static INT_PTR gg_menuchoose(WPARAM wParam, LPARAM lParam)
{
	if (lParam)
		*(void**)lParam = (void*)wParam;
	return 0;
}

static INT_PTR gg_parselink(WPARAM wParam, LPARAM lParam)
{
	char *arg = (char*)lParam;
	uin_t uin;
	int items = 0;

	if (g_Instances.getCount() == 0)
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

	GGPROTO *gg = NULL;
	for (int i=0; i < g_Instances.getCount(); i++) {
		gg = g_Instances[i];

		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIM_FLAGS;
		if (gg->m_iStatus > ID_STATUS_OFFLINE) {
			++items;
			mi.flags |= CMIM_ICON;
			mi.hIcon = LoadSkinnedProtoIcon(gg->m_szModuleName, gg->m_iStatus);
		}
		else mi.flags |= CMIF_HIDDEN;

		Menu_ModifyItem(gg->hInstanceMenuItem, &mi);
		if (mi.hIcon)
			Skin_ReleaseIcon(mi.hIcon);
	}

	if (items > 1) {
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
		MCONTACT hContact = gg->getcontact(uin, 1, 0, NULL);
		if (hContact != NULL)
			CallService(MS_MSG_SENDMESSAGE, hContact, 0);
	}

	return 0;
}

void gg_links_instancemenu_init()
{
	CreateServiceFunction(GGS_MENUCHOOSE, gg_menuchoose);
	hInstanceMenu = MO_CreateMenuObject("GGAccountChooser", LPGEN("Gadu-Gadu account chooser"), 0, GGS_MENUCHOOSE);

	TMO_MenuItem tmi = {0};
	tmi.cbSize = sizeof(tmi);
	tmi.pszName = "Cancel";
	tmi.position = 9999999;
	tmi.hIcolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_DELETE);
	CallService(MO_ADDNEWMENUITEM, (WPARAM)hInstanceMenu, (LPARAM)&tmi);
}

void gg_links_init()
{
	if (ServiceExists(MS_ASSOCMGR_ADDNEWURLTYPE)) {
		CreateServiceFunction(GGS_PARSELINK, gg_parselink);
		AssocMgr_AddNewUrlType("gg:", Translate("Gadu-Gadu Link Protocol"), hInstance, IDI_GG, GGS_PARSELINK, 0);
	}
}

void GGPROTO::links_instance_init()
{
	if (ServiceExists(MS_ASSOCMGR_ADDNEWURLTYPE)) {
		TMO_MenuItem tmi = { sizeof(tmi) };
		tmi.flags = CMIF_TCHAR;
		tmi.ownerdata = this;
		tmi.position = g_Instances.getCount();
		tmi.ptszName = m_tszUserName;
		hInstanceMenuItem = (HGENMENU)CallService(MO_ADDNEWMENUITEM, (WPARAM)hInstanceMenu, (LPARAM)&tmi);
	}
}
