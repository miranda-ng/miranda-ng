/*
Copyright © 2012-15 Miranda NG team
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
#include "proto.h"

extern OBJLIST<TwitterProto> g_Instances;

static IconItem icons[] =
{
	{ LPGEN("Twitter Icon"),   "twitter",  IDI_TWITTER },
	{ LPGEN("Tweet"),          "tweet",    IDI_TWITTER },
	{ LPGEN("Reply to Tweet"), "reply",    IDI_TWITTER },

	{ LPGEN("Visit Homepage"), "homepage", 0 }, 
};

static HANDLE hIconLibItem[SIZEOF(icons)];

// TODO: uninit
void InitIcons(void)
{
	Icon_Register(g_hInstance, "Protocols/Twitter", icons, SIZEOF(icons), "Twitter");
	icons[SIZEOF(icons) - 1].hIcolib = LoadSkinnedIconHandle(SKINICON_EVENT_URL);
}

HANDLE GetIconHandle(const char* name)
{
	for (size_t i = 0; i < SIZEOF(icons); i++)
		if (mir_strcmp(icons[i].szName, name) == 0)
			return hIconLibItem[i];

	return 0;
}

// Contact List menu stuff
static HGENMENU g_hMenuItems[2];
static HANDLE g_hMenuEvts[3];

// Helper functions
static TwitterProto * GetInstanceByHContact(MCONTACT hContact)
{
	char *proto = GetContactProto(hContact);
	if (!proto)
		return 0;

	for (int i = 0; i < g_Instances.getCount(); i++)
		if (!mir_strcmp(proto, g_Instances[i].m_szModuleName))
			return &g_Instances[i];

	return 0;
}

template<INT_PTR(__cdecl TwitterProto::*Fcn)(WPARAM, LPARAM)>
INT_PTR GlobalService(WPARAM wParam, LPARAM lParam)
{
	TwitterProto *proto = GetInstanceByHContact(MCONTACT(wParam));
	return proto ? (proto->*Fcn)(wParam, lParam) : 0;
}

static int PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	ShowContactMenus(false);

	TwitterProto *proto = GetInstanceByHContact(MCONTACT(wParam));
	return proto ? proto->OnPrebuildContactMenu(wParam, lParam) : 0;
}

void InitContactMenus()
{
	g_hMenuEvts[0] = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_NOTOFFLINE | CMIF_TCHAR;

	mi.position = -2000006000;
	mi.icolibItem = GetIconHandle("reply");
	mi.ptszName = LPGENT("Reply...");
	mi.pszService = "Twitter/ReplyToTweet";
	g_hMenuEvts[1] = CreateServiceFunction(mi.pszService, GlobalService<&TwitterProto::ReplyToTweet>);
	g_hMenuItems[0] = Menu_AddContactMenuItem(&mi);

	mi.position = -2000006000;
	mi.icolibItem = GetIconHandle("homepage");
	mi.ptszName = LPGENT("Visit Homepage");
	mi.pszService = "Twitter/VisitHomepage";
	g_hMenuEvts[2] = CreateServiceFunction(mi.pszService, GlobalService<&TwitterProto::VisitHomepage>);
	g_hMenuItems[1] = Menu_AddContactMenuItem(&mi);
}

void UninitContactMenus()
{
	for (size_t i = 0; i < SIZEOF(g_hMenuItems); i++)
		CallService(MO_REMOVEMENUITEM, (WPARAM)g_hMenuItems[i], 0);

	UnhookEvent(g_hMenuEvts[0]);
	for (size_t i = 1; i < SIZEOF(g_hMenuEvts); i++)
		DestroyServiceFunction(g_hMenuEvts[i]);
}

void ShowContactMenus(bool show)
{
	for (size_t i = 0; i < SIZEOF(g_hMenuItems); i++)
		Menu_ShowItem(g_hMenuItems[i], show);
}
