/*
former MetaContacts Plugin for Miranda IM.

Copyright © 2014-22 Miranda NG team
Copyright © 2004-07 Scott Ellis
Copyright © 2004 Universite Louis PASTEUR, STRASBOURG.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

#include "metacontacts.h"

bool g_bMetaEnabled;

/////////////////////////////////////////////////////////////////////////////////////////
// icolib support

static IconItem iconList[] = {
	{ LPGEN("Toggle off"),             "off",     IDI_MCMENUOFF     },
	{ LPGEN("Toggle on"),              "on",      IDI_MCMENU        },
	{ LPGEN("Convert to metacontact"), "convert", IDI_MCCONVERT     },
	{ LPGEN("Add to existing"),        "add",     IDI_MCADD         },
	{ LPGEN("Edit"),                   "edit",    IDI_MCEDIT        },
	{ LPGEN("Set to default"),         "default", IDI_MCSETDEFAULT  },
	{ LPGEN("Remove"),                 "remove",  IDI_MCREMOVE      },
};

HANDLE Meta_GetIconHandle(IconIndex i)
{
	return iconList[i].hIcolib;
}

HICON Meta_LoadIconEx(IconIndex i, bool big)
{
	return IcoLib_GetIcon(iconList[i].szName, big);
}

void UnloadMetacontacts(void)
{
	Meta_CloseHandles();
}
 
// Initializes the services provided and the link to those needed
// Called when the plugin is loaded into Miranda

int LoadMetacontacts(void)
{
	g_plugin.registerIcon(LPGEN("MetaContacts"), iconList, "mc");

	db_set_resident(META_PROTO, "Status");
	db_set_resident(META_PROTO, "IdleTS");

	// set all contacts to 'offline', and initialize subcontact counter for db consistency check
	for (auto &hContact : Contacts(META_PROTO)) {
		db_set_w(hContact, META_PROTO, "Status", ID_STATUS_OFFLINE);
		db_set_dw(hContact, META_PROTO, "IdleTS", 0);
		db_unset(hContact, META_PROTO, "MirVer");
	}	

	Meta_ReadOptions();

	// further db setup done in modules loaded (nick [protocol string required] & clist display name)
	Meta_InitServices();
	return 0;
}
