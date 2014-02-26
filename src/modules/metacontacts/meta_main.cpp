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

BOOL os_unicode_enabled = FALSE;

/////////////////////////////////////////////////////////////////////////////////////////
// icolib support

static IconItem iconList[] = {
	{ LPGEN("Toggle Off"),             "off",     IDI_MCMENUOFF     },
	{ LPGEN("Toggle On"),              "on",      IDI_MCMENU        },
	{ LPGEN("Convert to MetaContact"), "convert", IDI_MCCONVERT     },
	{ LPGEN("Add to Existing"),        "add",     IDI_MCADD         },
	{ LPGEN("Edit"),                   "edit",    IDI_MCEDIT        },
	{ LPGEN("Set to Default"),         "default", IDI_MCSETDEFAULT  },
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
	Icon_Register(hInst, META_PROTO, iconList, SIZEOF(iconList), "mc");

	db_set_resident(META_PROTO, "Status");
	db_set_resident(META_PROTO, "IdleTS");
	db_set_resident(META_PROTO, "ContactCountCheck");
	db_set_resident(META_PROTO, "Handle");
	db_set_resident(META_PROTO, "WindowOpen");

	//set all contacts to 'offline', and initialize subcontact counter for db consistency check
	for (MCONTACT hContact = db_find_first(META_PROTO); hContact; hContact = db_find_next(hContact, META_PROTO)) {
		db_set_w(hContact, META_PROTO, "Status", ID_STATUS_OFFLINE);
		db_set_dw(hContact, META_PROTO, "IdleTS", 0);
		db_set_b(hContact, META_PROTO, "ContactCountCheck", 0);

		// restore any saved defaults that might have remained if miranda was closed or crashed while a convo was happening
		if (db_get_dw(hContact, META_PROTO, "SavedDefault", (DWORD)-1) != (DWORD)-1) {
			db_set_dw(hContact, META_PROTO, "Default", db_get_dw(hContact, META_PROTO, "SavedDefault", 0));
			db_set_dw(hContact, META_PROTO, "SavedDefault", (DWORD)-1);
		}
	}	

	Meta_ReadOptions(&options);

	// sets subcontact handles to metacontacts, and metacontact handles to subcontacts
	// (since these handles are not necessarily the same from run to run of miranda)

	// also verifies that subcontacts: have metacontacts, and that contact numbers are reasonable, 
	// that metacontacts: have the correct number of subcontacts, and have reasonable defaults
	if (Meta_SetHandles()) {
		// error - db corruption
		if ( !db_get_b(0, META_PROTO, "DisabledMessageShown", 0)) {
			MessageBox(0, TranslateT("Error - Database corruption.\nPlugin disabled."), TranslateT("MetaContacts"), MB_OK | MB_ICONERROR);
			db_set_b(0, META_PROTO, "DisabledMessageShown", 1);
		}
		return 1;
	}

	db_unset(0, META_PROTO, "DisabledMessageShown");

	PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE };
	pd.szName = META_FILTER;
	pd.type = PROTOTYPE_FILTER;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	pd.szName = META_PROTO;
	pd.type = PROTOTYPE_VIRTUAL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	// further db setup done in modules loaded (nick [protocol string required] & clist display name)

	Meta_InitServices();

	// moved to 'modules loaded' event handler (in meta_services.c) because we need to 
	// check protocol for jabber hack, and the proto modules must be loaded
	//Meta_HideLinkedContactsAndSetHandles();

	if ( ServiceExists(MS_MSG_GETWINDOWAPI))
		message_window_api_enabled = TRUE;

	// for clist_meta_mw - write hidden group name to DB
	db_set_s(0, META_PROTO, "HiddenGroupName", META_HIDDEN_GROUP);
	return 0;
}
