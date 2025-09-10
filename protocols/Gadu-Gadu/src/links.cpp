/////////////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2009-2012 Bartosz Białek
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

#include "gg.h"
#include "m_assocmgr.h"

/////////////////////////////////////////////////////////////////////////////////////////
// File Association Manager support

#define GGS_PARSELINK "GG/ParseLink"
#define GGS_MENUCHOOSE "GG/MenuChoose"

static int hInstanceMenu;

static INT_PTR gg_menuchoose(WPARAM wParam, LPARAM lParam)
{
	if (lParam)
		*(void**)lParam = (void*)wParam;

	return 0;
}

static INT_PTR gg_parselink(WPARAM, LPARAM lParam)
{
	char *arg = (char*)lParam;

	if (CMPlugin::g_arInstances.getCount() == 0)
		return 0;

	if (arg == nullptr)
		return 1;

	arg = strchr(arg, ':');

	if (arg == nullptr)
		return 1;

	for (++arg; *arg == '/'; ++arg);
	uin_t uin = atoi(arg);

	if (!uin)
		return 1;

	GaduProto *gg = nullptr;
	int items = 0;
	for (auto &it : CMPlugin::g_arInstances) {
		gg = it;
		if (it->m_iStatus > ID_STATUS_OFFLINE) {
			++items;
			Menu_ModifyItem(it->hInstanceMenuItem, nullptr, Skin_LoadProtoIcon(it->m_szModuleName, it->m_iStatus));
		}
		else Menu_ShowItem(it->hInstanceMenuItem, false);
	}

	if (items > 1) {
		POINT pt;
		GetCursorPos(&pt);

		HMENU hMenu = CreatePopupMenu();
		Menu_Build(hMenu, hInstanceMenu);
		int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, g_clistApi.hwndContactList, nullptr);
		DestroyMenu(hMenu);

		if (cmd)
			Menu_ProcessCommandById(cmd, (LPARAM)&gg);
	}

	if (gg == nullptr)
		return 0;

	MCONTACT hContact = gg->getcontact(uin, 1, 0, nullptr);
	if (hContact != NULL)
		CallService(MS_MSG_SENDMESSAGE, hContact, 0);

	return 0;
}

void gg_links_instancemenu_init()
{
	CreateServiceFunction(GGS_MENUCHOOSE, gg_menuchoose);
	hInstanceMenu = Menu_AddObject("GGAccountChooser", LPGEN("Gadu-Gadu account chooser"), nullptr, GGS_MENUCHOOSE);

	CMenuItem mi(&g_plugin);
	mi.name.a = "Cancel";
	mi.position = 9999999;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_DELETE);
	Menu_AddItem(hInstanceMenu, &mi, nullptr);
}

void gg_links_init()
{
	if (ServiceExists(MS_ASSOCMGR_ADDNEWURLTYPE)) {
		CreateServiceFunction(GGS_PARSELINK, gg_parselink);
		AssocMgr_AddNewUrlTypeW("gg:", TranslateT("Gadu-Gadu Link Protocol"), g_plugin.getInst(), IDI_GG, GGS_PARSELINK, 0);
	}
}

void GaduProto::links_instance_init()
{
	if (ServiceExists(MS_ASSOCMGR_ADDNEWURLTYPE)) {
		CMenuItem mi(&g_plugin);
		mi.flags = CMIF_UNICODE;
		mi.position = CMPlugin::g_arInstances.getCount();
		mi.name.w = m_tszUserName;
		hInstanceMenuItem = Menu_AddItem(hInstanceMenu, &mi, this);
	}
}
