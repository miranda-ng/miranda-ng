/* 
Copyright (C) 2012 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "stdafx.h"

HGENMENU hmiAutoUpdate;

void InitMenu()
{
	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_UNICODE | CMIF_NOTOFFLINE;
	mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("News Aggregator"), 500099000);
	Menu_ConfigureItem(mi.root, MCI_OPT_UID, "D9733E4F-1946-4390-8EB3-591E8687222E");

	SET_UID(mi, 0x3ec91864, 0xefa7, 0x4994, 0xb7, 0x75, 0x6c, 0x96, 0xcb, 0x29, 0x2f, 0x93);
	mi.position = 10100001;
	if (g_plugin.getByte("AutoUpdate", 1))
		mi.name.w = LPGENW("Auto Update Enabled");
	else
		mi.name.w = LPGENW("Auto Update Disabled");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_ICON);
	mi.pszService = MS_NEWSAGGREGATOR_ENABLED;
	hmiAutoUpdate = Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x8076bb4d, 0x1e44, 0x43af, 0x97, 0x1e, 0x31, 0xd8, 0xa4, 0xe9, 0xb8, 0x37);
	mi.position = 20100001;
	mi.name.w = LPGENW("Check All Feeds");
	mi.pszService = MS_NEWSAGGREGATOR_CHECKALLFEEDS;
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0xb876484d, 0x28aa, 0x4e03, 0x9e, 0x98, 0xed, 0xbc, 0xd1, 0xcf, 0x31, 0x80);
	mi.position = 20100002;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_ADDFEED);
	mi.name.w = LPGENW("Add Feed");
	mi.pszService = MS_NEWSAGGREGATOR_ADDFEED;
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x600bf2c2, 0xa974, 0x44d3, 0x98, 0xf9, 0xe6, 0x65, 0x7c, 0x1f, 0x63, 0x37);
	mi.position = 20100003;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_IMPORTFEEDS);
	mi.name.w = LPGENW("Import Feeds");
	mi.pszService = MS_NEWSAGGREGATOR_IMPORTFEEDS;
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0xc09c8119, 0x64c2, 0x49bd, 0x81, 0xf, 0x54, 0x20, 0x69, 0xd7, 0x30, 0xcf);
	mi.position = 20100004;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_EXPORTFEEDS);
	mi.name.w = LPGENW("Export Feeds");
	mi.pszService = MS_NEWSAGGREGATOR_EXPORTFEEDS;
	Menu_AddMainMenuItem(&mi);

	// adding contact menu items
	SET_UID(mi, 0x92be499c, 0x928c, 0x4789, 0x8f, 0x36, 0x28, 0xa2, 0x9f, 0xb7, 0x1a, 0x97);
	mi.root = nullptr;
	mi.position = -0x7FFFFFFA;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_CHECKFEED);
	mi.name.w = LPGENW("Check feed");
	mi.pszService = MS_NEWSAGGREGATOR_CHECKFEED;
	Menu_AddContactMenuItem(&mi, MODULENAME);

	SET_UID(mi, 0x41a70fbc, 0x9241, 0x44c0, 0x90, 0x90, 0x87, 0xd2, 0xc5, 0x9f, 0xc9, 0xac);
	mi.name.w = LPGENW("Change feed");
	mi.pszService = MS_NEWSAGGREGATOR_CHANGEFEED;
	Menu_AddContactMenuItem(&mi, MODULENAME);

	Menu_ModifyItem(hmiAutoUpdate, nullptr, g_plugin.getIconHandle(g_plugin.getByte("AutoUpdate", 1) ? IDI_ENABLED : IDI_DISABLED));
}

void UpdateMenu(bool State)
{
	if (!State) // to enable auto-update
		Menu_ModifyItem(hmiAutoUpdate, LPGENW("Auto Update Enabled"), g_plugin.getIconHandle(IDI_ENABLED));
	else  // to disable auto-update
		Menu_ModifyItem(hmiAutoUpdate, LPGENW("Auto Update Disabled"), g_plugin.getIconHandle(IDI_DISABLED));

	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTBButton, State ? TTBST_PUSHED : 0);
	g_plugin.setByte("AutoUpdate", !State);
}
