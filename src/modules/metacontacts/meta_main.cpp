/*
former MetaContacts Plugin for Miranda IM.

Copyright © 2014 Miranda NG Team
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

#include "..\..\core\commonheaders.h"

#include "metacontacts.h"

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

HANDLE GetIconHandle(IconIndex i)
{
	return iconList[i].hIcolib;
}

HICON LoadIconEx(IconIndex i)
{
	return Skin_GetIcon(iconList[i].szName);
}

void UnloadMetacontacts(void)
{
	Meta_CloseHandles();
}
 
// Initializes the services provided and the link to those needed
// Called when the plugin is loaded into Miranda
int LoadMetacontacts(void)
{
	Icon_Register(hInst, LPGEN("MetaContacts"), iconList, SIZEOF(iconList), "mc");

	db_set_resident(META_PROTO, "Status");
	db_set_resident(META_PROTO, "IdleTS");

	//set all contacts to 'offline', and initialize subcontact counter for db consistency check
	for (MCONTACT hContact = db_find_first(META_PROTO); hContact; hContact = db_find_next(hContact, META_PROTO)) {
		db_set_w(hContact, META_PROTO, "Status", ID_STATUS_OFFLINE);
		db_set_dw(hContact, META_PROTO, "IdleTS", 0);
	}	

	Meta_ReadOptions();

	PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE };
	pd.szName = META_FILTER;
	pd.type = PROTOTYPE_FILTER;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	pd.szName = META_PROTO;
	pd.type = PROTOTYPE_VIRTUAL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	// further db setup done in modules loaded (nick [protocol string required] & clist display name)
	Meta_InitServices();
	return 0;
}
