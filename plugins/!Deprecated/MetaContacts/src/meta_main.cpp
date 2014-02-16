/*
MetaContacts Plugin for Miranda IM.

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

/** @file meta_main.c 
*
* Functions used by Miranda to launch the plugin.
* Centralizes functions needed by Miranda to get 
* information about the plugin and to initialize it
* and to dispose of it properly.
*/

/*! \mainpage MetaContacts plugin
 * \image html ulp.gif
 * \section desc Description
 * This library is a plugin for Miranda IM.\n
 * It allows the user to group several contacts from different
 * protocols (such as AIM or ICQ) into one unique metacontact.\n
 *
 * \section principle How does it work ?
 * Only one protocol will be used at a time (the default protocol).\n
 * This protocol is referenced in the proper field of the MetaContact
 * section in the Database.
 * 
 * \subsection send Emission of messages
 * The plugin will search through the Database to get the default protocol
 * (i.e. the protocol used to communicate with),\n and then call the Send function 
 * provided by this protocol.
 *
 * \subsection recv Reception of messages
 * When a contact is converted to a metacontact, or when it is added to an existing
 * metacontact, it gets a pseudo protocol \n (named "MetaContacts") in the protocol chain.\n
 * Usually, when a message is received, all the protocols in the chain get this message,
 * the real protocol (for example ICQ) at the end.\n But here, the message will be intercepted
 * by the MetaContact protocol, which will inhibit the further reception.\n The message will
 * then be redirected to the MetaContact, that will display it normally.
 *
 * \subsection handling Handling MetaContacts
 * There are four functionnality for handling MetaContacts :
 * \li Convert a contact to a MetaContact.
 * \li Add a contact to an existing MetaContact.
 * \li Edit a MetaContact.
 * \li Delete a MetaContact.
 *
 * They all are accessible via the context-menu displayed when a right click has occured,
 * but not at the same time : The 2 first will appear when the menu concerns a simple contact.\n
 * whereas the 2 last are only accessible from a MetaContact.\n
 * Those functions are self-explanatory, and a MessageBox is shown before any modification, so, for
 * further information, take a look at the Dialogs shown when they are called.\n
 *
 * \section cvats Caveats
 * Several functionnalities have not yet been developped :
 * \li Assigning contacts by Drag'n'Drop
 * \li Updating dynamically the status of the MetaContact
 * \li Merging history of all the contacts attached to MetaContact
 * \li Handling Files and URLs as well as Messages
 * \li and some other little functionnalities...
 *
 * Some of those functionnalities will not be developped due to the architecture
 * of Miranda (the 2 first, for example)
 * 
 * \section mail Contact
 * For any comment, suggestion or question, send a mail to shaalj@free.fr.\n
 * This code is provided as-is, and I cannot be held responsible for any harm
 * done to your database by this plugin.\n
 * Test it first on a fake database before using it normally.
 */

#include "metacontacts.h"

BOOL os_unicode_enabled = FALSE;
int hLangpack;
CLIST_INTERFACE *pcli = NULL;

//! Information gathered by Miranda, displayed in the plugin pane of the Option Dialog
PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {4C4A27CF-5E64-4242-A332-B98B08243E89}
	{0x4c4a27cf, 0x5e64, 0x4242, {0xa3, 0x32, 0xb9, 0x8b, 0x8, 0x24, 0x3e, 0x89}}
};

HINSTANCE hInstance;	//!< Global reference to the application

/** Called by Miranda to get the information associated to this plugin.
* It only returns the PLUGININFO structure, without any test on the version
* @param mirandaVersion The version of the application calling this function
*/
extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

/** DLL entry point
* Required to store the instance handle
*/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;
	return TRUE;
}

/** Prepare the plugin to stop
* Called by Miranda when it will exit or when the plugin gets deselected
*/
extern "C" __declspec(dllexport) int Unload(void)
{
	// see also meta_services.c, Meta_PreShutdown
	Meta_CloseHandles();
	//MessageBox(0, "Unload complete", "MC", MB_OK);
	return 0;
}
 
/** Initializes the services provided and the link to those needed
* Called when the plugin is loaded into Miranda
*/
extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);
	mir_getCLI();

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
		//Meta_HideMetaContacts(TRUE);
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
