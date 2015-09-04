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

HGENMENU hService2[7];

void InitMenu()
{
	CMenuItem mi;
	mi.flags = CMIF_TCHAR | CMIF_NOTOFFLINE;
	mi.root = Menu_CreateRoot(MO_MAIN, LPGENT("News Aggregator"), 500099000);

	SET_UID(mi, 0x3ec91864, 0xefa7, 0x4994, 0xb7, 0x75, 0x6c, 0x96, 0xcb, 0x29, 0x2f, 0x93);
	mi.position = 10100001;
	if (db_get_b(NULL, MODULE, "AutoUpdate", 1))
		mi.name.t = LPGENT("Auto Update Enabled");
	else
		mi.name.t = LPGENT("Auto Update Disabled");
	mi.hIcolibItem = GetIconHandle("main");
	mi.pszService = MS_NEWSAGGREGATOR_ENABLED;
	hService2[0] = Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x8076bb4d, 0x1e44, 0x43af, 0x97, 0x1e, 0x31, 0xd8, 0xa4, 0xe9, 0xb8, 0x37);
	mi.position = 20100001;
	mi.name.t = LPGENT("Check All Feeds");
	mi.pszService = MS_NEWSAGGREGATOR_CHECKALLFEEDS;
	hService2[1] = Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0xb876484d, 0x28aa, 0x4e03, 0x9e, 0x98, 0xed, 0xbc, 0xd1, 0xcf, 0x31, 0x80);
	mi.position = 20100002;
	mi.hIcolibItem = GetIconHandle("addfeed");
	mi.name.t = LPGENT("Add Feed");
	mi.pszService = MS_NEWSAGGREGATOR_ADDFEED;
	hService2[2] = Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x600bf2c2, 0xa974, 0x44d3, 0x98, 0xf9, 0xe6, 0x65, 0x7c, 0x1f, 0x63, 0x37);
	mi.position = 20100003;
	mi.hIcolibItem = GetIconHandle("importfeeds");
	mi.name.t = LPGENT("Import Feeds");
	mi.pszService = MS_NEWSAGGREGATOR_IMPORTFEEDS;
	hService2[3] = Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0xc09c8119, 0x64c2, 0x49bd, 0x81, 0xf, 0x54, 0x20, 0x69, 0xd7, 0x30, 0xcf);
	mi.position = 20100004;
	mi.hIcolibItem = GetIconHandle("exportfeeds");
	mi.name.t = LPGENT("Export Feeds");
	mi.pszService = MS_NEWSAGGREGATOR_EXPORTFEEDS;
	hService2[4] = Menu_AddMainMenuItem(&mi);

	// adding contact menu items
	SET_UID(mi, 0x92be499c, 0x928c, 0x4789, 0x8f, 0x36, 0x28, 0xa2, 0x9f, 0xb7, 0x1a, 0x97);
	mi.root = NULL;
	mi.position = -0x7FFFFFFA;
	mi.hIcolibItem = GetIconHandle("checkfeed");
	mi.name.t = LPGENT("Check feed");
	mi.pszService = MS_NEWSAGGREGATOR_CHECKFEED;
	hService2[5] = Menu_AddContactMenuItem(&mi, MODULE);

	SET_UID(mi, 0x41a70fbc, 0x9241, 0x44c0, 0x90, 0x90, 0x87, 0xd2, 0xc5, 0x9f, 0xc9, 0xac);
	mi.name.t = LPGENT("Change feed");
	mi.pszService = MS_NEWSAGGREGATOR_CHANGEFEED;
	hService2[6] = Menu_AddContactMenuItem(&mi, MODULE);

	Menu_ModifyItem(hService2[0], NULL, GetIconHandle(db_get_b(NULL, MODULE, "AutoUpdate", 1) ? "enabled" : "disabled"));
}
