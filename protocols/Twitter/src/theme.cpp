/*
Copyright © 2012-22 Miranda NG team
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

#include "stdafx.h"
#include "theme.h"

static IconItem iconList[] =
{
	{ LPGEN("Twitter Icon"), "CTwitterProto", IDI_TWITTER },
	{ LPGEN("Tweet"), "tweet", IDI_TWITTER },
	{ LPGEN("Reply to Tweet"), "reply", IDI_TWITTER },

	{ LPGEN("Visit Homepage"), "homepage", 0 },
};

void TwitterInitSounds(void)
{
	g_plugin.addSound("TwitterNewContact", LPGENW("Twitter"), LPGENW("First tweet from new contact"));
	g_plugin.addSound("TwitterNew",        LPGENW("Twitter"), LPGENW("New tweet"));
}

// TODO: uninit
void InitIcons(void)
{
	g_plugin.registerIcon("Protocols/Twitter", iconList, "Twitter");
	iconList[_countof(iconList) - 1].hIcolib = Skin_GetIconHandle(SKINICON_EVENT_URL);
}

HANDLE GetIconHandle(const char *name)
{
	for (auto &it : iconList)
		if (mir_strcmp(it.szName, name) == 0)
			return it.hIcolib;

	return nullptr;
}

// Contact List menu stuff
static HGENMENU g_hMenuItems[2];

// Helper functions

template<INT_PTR(__cdecl CTwitterProto::*Fcn)(WPARAM, LPARAM)>
INT_PTR GlobalService(WPARAM hContact, LPARAM lParam)
{
	CTwitterProto *proto = CMPlugin::getInstance(MCONTACT(hContact));
	return proto ? (proto->*Fcn)(hContact, lParam) : 0;
}

static int PrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	ShowContactMenus(false);

	CTwitterProto *proto = CMPlugin::getInstance(MCONTACT(hContact));
	return proto ? proto->OnPrebuildContactMenu(hContact, lParam) : 0;
}

void InitContactMenus()
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_NOTOFFLINE | CMIF_UNICODE;

	SET_UID(mi, 0xfc4e1245, 0xc8e0, 0x4de2, 0x92, 0x15, 0xfc, 0xcf, 0x48, 0xf9, 0x41, 0x56);
	mi.position = -2000006000;
	mi.hIcolibItem = GetIconHandle("reply");
	mi.name.w = LPGENW("Reply...");
	mi.pszService = "Twitter/ReplyToTweet";
	g_hMenuItems[0] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CTwitterProto::ReplyToTweet>);

	SET_UID(mi, 0x7f7e4c24, 0x821c, 0x450f, 0x93, 0x76, 0xbe, 0x65, 0xe9, 0x2f, 0xb6, 0xc2);
	mi.position = -2000006000;
	mi.hIcolibItem = GetIconHandle("homepage");
	mi.name.w = LPGENW("Visit Homepage");
	mi.pszService = "Twitter/VisitHomepage";
	g_hMenuItems[1] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&CTwitterProto::VisitHomepage>);
}

void ShowContactMenus(bool show)
{
	for (auto &it : g_hMenuItems)
		Menu_ShowItem(it, show);
}
