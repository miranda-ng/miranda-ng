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

#include "common.h"

HANDLE hService2[6];

VOID InitMenu()
{
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.pszContactOwner = MODULE;
	mi.flags = CMIF_TCHAR|CMIF_ICONFROMICOLIB|CMIF_NOTOFFLINE;

	// adding main menu items
	mi.ptszPopupName = LPGENT("News Aggregator");
	mi.popupPosition = 500099000;

	mi.position=10100001;
	mi.icolibItem = GetIconHandle("main");
	mi.ptszName = LPGENT("Check All Feeds");
	mi.pszService = MS_NEWSAGGR_CHECKALLFEEDS;
	hService2[0] = Menu_AddMainMenuItem(&mi);

	mi.position=10100002;
	mi.icolibItem = GetIconHandle("addfeed");
	mi.ptszName = LPGENT("Add Feed");
	mi.pszService = MS_NEWSAGGR_ADDFEED;
	hService2[1] = Menu_AddMainMenuItem(&mi);

	mi.position=10100003;
	mi.icolibItem = GetIconHandle("importfeeds");
	mi.ptszName = LPGENT("Import Feeds");
	mi.pszService = MS_NEWSAGGR_IMPORTFEEDS;
	hService2[2] = Menu_AddMainMenuItem(&mi);

	mi.position=10100004;
	mi.icolibItem = GetIconHandle("exportfeeds");
	mi.ptszName = LPGENT("Export Feeds");
	mi.pszService = MS_NEWSAGGR_EXPORTFEEDS;
	hService2[3] = Menu_AddMainMenuItem(&mi);

	// adding contact menu items
	mi.position=-0x7FFFFFFA;
	mi.icolibItem = GetIconHandle("checkfeed");
	mi.ptszName = LPGENT("Check feed");
	mi.pszService = MS_NEWSAGGR_CHECKFEED;
	hService2[4] = Menu_AddContactMenuItem(&mi);

	// adding contact menu items
	mi.position=-0x7FFFFFFA;
	//mi.icolibItem = GetIconHandle("checkfeed");
	mi.ptszName = LPGENT("Change feed");
	mi.pszService = MS_NEWSAGGR_CHANGEFEED;
	hService2[5] = Menu_AddContactMenuItem(&mi);

	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_ICON;
	mi.icolibItem = GetIconHandle("checkall");
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hService2[0], (LPARAM)&mi);
}