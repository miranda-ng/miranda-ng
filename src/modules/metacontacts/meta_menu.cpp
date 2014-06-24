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

#include <m_nudge.h>
#include "metacontacts.h"

static HGENMENU hMenuContact[MAX_CONTACTS];

static HGENMENU
	hMenuRoot,			 // root menu item of all subs
	hMenuConvert,      // HANDLE to the convert menu item.
	hMenuAdd,          // HANDLE to the add to menu item.
	hMenuEdit,         // HANDLE to the edit menu item.
	hMenuDelete,       // HANDLE to the delete menu item.
	hMenuDefault,      // HANDLE to the delete menu item.
	hMenuForceDefault, // HANDLE to the delete menu item.
	hMenuOnOff;	       // HANDLE to the enable/disable menu item.

/** Convert the contact chosen into a MetaContact.
*
* Create a new MetaContact, remove the selected contact from the \c CList
* and attach it to the MetaContact.
*
* @param wParam : HANDLE to the contact that has been chosen.
* @param lParam :	Allways set to 0.
*/

INT_PTR Meta_Convert(WPARAM wParam, LPARAM lParam)
{
	ptrT tszGroup(db_get_tsa(wParam, "CList", "Group"));

	// Create a new metacontact
	MCONTACT hMetaContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
	if (hMetaContact == NULL)
		return NULL;

	DBCachedContact *cc = currDb->m_cache->GetCachedContact(hMetaContact);
	if (cc == NULL)
		return 0;

	db_set_dw(hMetaContact, META_PROTO, "NumContacts", 0);
	cc->nSubs = 0;

	// Add the MetaContact protocol to the new meta contact
	CallService(MS_PROTO_ADDTOCONTACT, hMetaContact, (LPARAM)META_PROTO);

	if (tszGroup)
		db_set_ts(hMetaContact, "CList", "Group", tszGroup);

	// Assign the contact to the MetaContact just created (and make default).
	if (!Meta_Assign(wParam, hMetaContact, TRUE)) {
		MessageBox(0, TranslateT("There was a problem in assigning the contact to the MetaContact"), TranslateT("Error"), MB_ICONEXCLAMATION);
		CallService(MS_DB_CONTACT_DELETE, hMetaContact, 0);
		return 0;
	}

	// hide the contact if clist groups disabled (shouldn't create one anyway since menus disabled)
	if (!db_mc_isEnabled())
		db_set_b(hMetaContact, "CList", "Hidden", 1);

	return hMetaContact;
}

void Meta_RemoveContactNumber(DBCachedContact *ccMeta, int number, bool bUpdateInfo)
{
	if (ccMeta == NULL)
		return;

	// make sure this contact thinks it's part of this metacontact
	DBCachedContact *ccSub = currDb->m_cache->GetCachedContact(Meta_GetContactHandle(ccMeta, number));
	if (ccSub == NULL)
		return;

	if (ccSub->parentID == ccMeta->contactID) {
		db_unset(ccSub->contactID, "CList", "Hidden");

		// stop ignoring, if we were
		if (options.bSuppressStatus)
			CallService(MS_IGNORE_UNIGNORE, ccSub->contactID, IGNOREEVENT_USERONLINE);
	}

	// each contact from 'number' upwards will be moved down one
	// and the last one will be deleted
	for (int i = number+1; i < ccMeta->nSubs; i++)
		Meta_SwapContacts(ccMeta, i, i - 1);

	// remove the last one
	int id = ccMeta->nSubs - 1;
	char buffer[512];
	mir_snprintf(buffer, SIZEOF(buffer), "Protocol%d", id);
	db_unset(ccMeta->contactID, META_PROTO, buffer);

	mir_snprintf(buffer, SIZEOF(buffer), "Status%d", id);
	db_unset(ccMeta->contactID, META_PROTO, buffer);

	mir_snprintf(buffer, SIZEOF(buffer), "StatusString%d", id);
	db_unset(ccMeta->contactID, META_PROTO, buffer);

	mir_snprintf(buffer, SIZEOF(buffer), "Login%d", id);
	db_unset(ccMeta->contactID, META_PROTO, buffer);

	mir_snprintf(buffer, SIZEOF(buffer), "Nick%d", id);
	db_unset(ccMeta->contactID, META_PROTO, buffer);

	mir_snprintf(buffer, SIZEOF(buffer), "CListName%d", id);
	db_unset(ccMeta->contactID, META_PROTO, buffer);

	ccSub->parentID = 0;
	currDb->MetaDetouchSub(ccMeta, ccMeta->nSubs - 1);

	currDb->MetaSplitHistory(ccMeta, ccSub);

	// if the default contact was equal to or greater than 'number', decrement it (and deal with ends)
	if (ccMeta->nDefault >= number) {
		int iNumber = ccMeta->nDefault-1;
		if (iNumber < 0)
			iNumber = 0;
		db_mc_setDefaultNum(ccMeta->contactID, iNumber, true);
	}
	
	ccMeta->nSubs--;
	db_set_dw(ccMeta->contactID, META_PROTO, "NumContacts", ccMeta->nSubs);

	if (bUpdateInfo) {
		// fix nick
		Meta_CopyContactNick(ccMeta, Meta_GetMostOnline(ccMeta));

		// fix status
		Meta_FixStatus(ccMeta);

		// fix avatar
		MCONTACT hContact = Meta_GetMostOnlineSupporting(ccMeta, PFLAGNUM_4, PF4_AVATARS);
		if (hContact) {
			PROTO_AVATAR_INFORMATIONT AI = { sizeof(AI) };
			AI.hContact = ccMeta->contactID;
			AI.format = PA_FORMAT_UNKNOWN;
			_tcscpy(AI.filename, _T("X"));

			if (CallProtoService(META_PROTO, PS_GETAVATARINFOT, 0, (LPARAM)&AI) == GAIR_SUCCESS)
				db_set_ts(ccMeta->contactID, "ContactPhoto", "File", AI.filename);
		}
	}
}

/** Delete a MetaContact from the database
*
* Delete a MetaContact and remove all the information
* concerning this MetaContact in the contact linked to it.
*
* @param wParam : HANDLE to the MetaContact to be deleted, or to the subcontact to be removed from the MetaContact
* @param lParam : BOOL flag indicating whether to ask 'are you sure' when deleting a MetaContact
*/

INT_PTR Meta_Delete(WPARAM hContact, LPARAM bSkipQuestion)
{
	DBCachedContact *cc = currDb->m_cache->GetCachedContact(hContact);
	if (cc == NULL)
		return 0;

	// The wParam is a metacontact
	if (cc->IsMeta()) {
		// check from recursion - see second half of this function
		if (!bSkipQuestion && IDYES != 
			 MessageBox((HWND)CallService(MS_CLUI_GETHWND, 0, 0),
				TranslateT("This will remove the MetaContact permanently.\n\nProceed Anyway?"),
				TranslateT("Are you sure?"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
			return 0;

		for (int i = cc->nSubs-1; i >= 0; i--)
			Meta_RemoveContactNumber(cc, i, false);

		NotifyEventHooks(hSubcontactsChanged, hContact, 0);
		CallService(MS_DB_CONTACT_DELETE, hContact, 0);
	}
	else if (cc->IsSub()) {
		if ((cc = currDb->m_cache->GetCachedContact(cc->parentID)) == NULL)
			return 0;

		if (cc->nSubs == 1) {
			if (IDYES == MessageBox(0, TranslateT(szDelMsg), TranslateT("Delete MetaContact?"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1))
				Meta_Delete(cc->contactID, 1);

			return 0;
		}

		Meta_RemoveContactNumber(cc, Meta_GetContactNumber(cc, hContact), true);
	}
	return 0;
}

/** Set contact as MetaContact default
*
* Set the given contact to be the default one for the metacontact to which it is linked.
*
* @param wParam : 	HANDLE to the MetaContact to be set as default
* @param lParam :	HWND to the clist window
(This means the function has been called via the contact menu).
*/

INT_PTR Meta_Default(WPARAM hSub, LPARAM wParam)
{
	DBCachedContact *cc = currDb->m_cache->GetCachedContact(db_mc_getMeta(hSub));
	if (cc && cc->IsMeta())
		db_mc_setDefault(cc->contactID, hSub, true);
	return 0;
}

/** Called when the context-menu of a contact is about to be displayed
*
* This will test which of the 4 menu item should be displayed, depending
* on which contact triggered the event
*
* @param wParam : HANDLE to the contact that triggered the event
* @param lParam :	Always set to 0;
*/

int Meta_ModifyMenu(WPARAM hMeta, LPARAM lParam)
{
	DBCachedContact *cc = currDb->m_cache->GetCachedContact(hMeta);
	if (cc == NULL)
		return 0;
		
	CLISTMENUITEM mi = { sizeof(mi) };
	Menu_ShowItem(hMenuRoot, false);

	if (cc->IsMeta()) {
		// save the mouse pos in case they open a subcontact menu
		GetCursorPos(&menuMousePoint);

		// This is a MetaContact, show the edit, force default, and the delete menu, and hide the others
		Menu_ShowItem(hMenuEdit, true);
		Menu_ShowItem(hMenuAdd, false);
		Menu_ShowItem(hMenuConvert, false);
		Menu_ShowItem(hMenuDefault, false);
		Menu_ShowItem(hMenuDelete, false);

		mi.flags = CMIM_NAME;
		mi.pszName = LPGEN("Remove from metacontact");
		Menu_ModifyItem(hMenuDelete, &mi);

		//show subcontact menu items
		for (int i = 0; i < MAX_CONTACTS; i++) {
			if (i >= cc->nSubs) {
				Menu_ShowItem(hMenuContact[i], false);
				continue;
			}

			MCONTACT hContact = Meta_GetContactHandle(cc, i);
			char *szProto = GetContactProto(hContact);

			if (options.menu_contact_label == DNT_UID) {
				char buf[512], idStr[512];
				strcpy(buf, "Login");
				strcat(buf, _itoa(i, idStr, 10));

				DBVARIANT dbv;
				db_get(hMeta, META_PROTO, buf, &dbv);
				switch (dbv.type) {
				case DBVT_ASCIIZ:
					mir_snprintf(buf, 512, "%s", dbv.pszVal);
					break;
				case DBVT_BYTE:
					mir_snprintf(buf, 512, "%d", dbv.bVal);
					break;
				case DBVT_WORD:
					mir_snprintf(buf, 512, "%d", dbv.wVal);
					break;
				case DBVT_DWORD:
					mir_snprintf(buf, 512, "%d", dbv.dVal);
					break;
				default:
					buf[0] = 0;
				}
				db_free(&dbv);
				mi.pszName = buf;
				mi.flags = 0;
			}
			else {
				mi.ptszName = cli.pfnGetContactDisplayName(hContact, 0);
				mi.flags = CMIF_TCHAR;
			}

			mi.flags |= CMIM_FLAGS | CMIM_NAME | CMIM_ICON;

			int iconIndex = CallService(MS_CLIST_GETCONTACTICON, hContact, 0);
			mi.hIcon = ImageList_GetIcon((HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0), iconIndex, 0);

			Menu_ModifyItem(hMenuContact[i], &mi);
			DestroyIcon(mi.hIcon);
			
			Menu_ShowItem(hMenuRoot, true);
		}

		// show hide nudge menu item
		char serviceFunc[256];
		mir_snprintf(serviceFunc, 256, "%s%s", GetContactProto(Meta_GetMostOnline(cc)), PS_SEND_NUDGE);
		CallService(MS_NUDGE_SHOWMENU, (WPARAM)META_PROTO, ServiceExists(serviceFunc));
		return 0;
	}

	PROTOACCOUNT *pa = Proto_GetAccount(cc->szProto);
	if (!db_mc_isEnabled() || !pa || pa->bIsVirtual) {
		// groups disabled - all meta menu options hidden
		Menu_ShowItem(hMenuDefault, false);
		Menu_ShowItem(hMenuDelete, false);
		Menu_ShowItem(hMenuAdd, false);
		Menu_ShowItem(hMenuConvert, false);
		Menu_ShowItem(hMenuEdit, false);
		return 0;
	}
	
	// the contact is affected to a metacontact
	if (cc->IsSub()) {
		Menu_ShowItem(hMenuDefault, true);

		mi.flags = CMIM_NAME;
		mi.pszName = LPGEN("Remove from metacontact");
		Menu_ModifyItem(hMenuDelete, &mi);
		Menu_ShowItem(hMenuDelete, true);

		Menu_ShowItem(hMenuAdd, false);
		Menu_ShowItem(hMenuConvert, false);
		Menu_ShowItem(hMenuEdit, false);
	}
	else {
		// The contact is neutral
		Menu_ShowItem(hMenuAdd, true);
		Menu_ShowItem(hMenuConvert, true);
		Menu_ShowItem(hMenuEdit, false);
		Menu_ShowItem(hMenuDelete, false);
		Menu_ShowItem(hMenuDefault, false);
	}

	for (int i = 0; i < MAX_CONTACTS; i++)
		Menu_ShowItem(hMenuContact[i], false);

	return 0;
}

INT_PTR Meta_OnOff(WPARAM wParam, LPARAM lParam)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_NAME | CMIM_ICON;

	bool bToggled = !db_mc_isEnabled();
	db_set_b(0, META_PROTO, "Enabled", bToggled);
	if (bToggled) {
		mi.icolibItem = GetIconHandle(I_MENUOFF);
		mi.pszName = LPGEN("Toggle MetaContacts Off");
	}
	else {
		mi.icolibItem = GetIconHandle(I_MENU);
		mi.pszName = LPGEN("Toggle MetaContacts On");
	}
	Menu_ModifyItem(hMenuOnOff, &mi);

	db_mc_enable(bToggled);
	Meta_HideMetaContacts(!bToggled);
	return 0;
}

void InitMenus()
{
	CLISTMENUITEM mi = { sizeof(mi) };

	// main menu item
	mi.icolibItem = GetIconHandle(I_MENUOFF);
	mi.pszName = LPGEN("Toggle MetaContacts Off");
	mi.pszService = "MetaContacts/OnOff";
	mi.position = 500010000;
	hMenuOnOff = Menu_AddMainMenuItem(&mi);

	// contact menu items
	mi.icolibItem = GetIconHandle(I_CONVERT);
	mi.position = -200010;
	mi.pszName = LPGEN("Convert to MetaContact");
	mi.pszService = "MetaContacts/Convert";
	hMenuConvert = Menu_AddContactMenuItem(&mi);

	mi.icolibItem = GetIconHandle(I_ADD);
	mi.position = -200009;
	mi.pszName = LPGEN("Add to existing MetaContact...");
	mi.pszService = "MetaContacts/AddTo";
	hMenuAdd = Menu_AddContactMenuItem(&mi);

	mi.icolibItem = GetIconHandle(I_EDIT);
	mi.position = -200010;
	mi.pszName = LPGEN("Edit MetaContact...");
	mi.pszService = "MetaContacts/Edit";
	hMenuEdit = Menu_AddContactMenuItem(&mi);

	mi.icolibItem = GetIconHandle(I_SETDEFAULT);
	mi.position = -200009;
	mi.pszName = LPGEN("Set as MetaContact default");
	mi.pszService = "MetaContacts/Default";
	hMenuDefault = Menu_AddContactMenuItem(&mi);

	mi.icolibItem = GetIconHandle(I_REMOVE);
	mi.position = -200008;
	mi.pszName = LPGEN("Delete MetaContact");
	mi.pszService = "MetaContacts/Delete";
	hMenuDelete = Menu_AddContactMenuItem(&mi);

	mi.position = -99000;
	mi.flags = CMIF_HIDDEN | CMIF_ROOTPOPUP;
	mi.icolibItem = 0;
	mi.pszName = LPGEN("Subcontacts");
	hMenuRoot = Menu_AddContactMenuItem(&mi);

	mi.flags = CMIF_HIDDEN | CMIF_CHILDPOPUP;
	mi.hParentMenu = hMenuRoot;
	for (int i = 0; i < MAX_CONTACTS; i++) {
		mi.position--;
		mi.pszName = "";

		char buffer[512];
		mir_snprintf(buffer, SIZEOF(buffer), "MetaContacts/MenuFunc%d", i);
		mi.pszService = buffer;

		hMenuContact[i] = Menu_AddContactMenuItem(&mi);
	}

	Meta_HideLinkedContacts();

	if (!db_mc_isEnabled()) {
		// modify main menu item
		mi.flags = CMIM_NAME | CMIM_ICON;
		mi.icolibItem = GetIconHandle(I_MENU);
		mi.pszName = LPGEN("Toggle MetaContacts On");
		Menu_ModifyItem(hMenuOnOff, &mi);

		Meta_HideMetaContacts(true);
	}
	else {
		Meta_SuppressStatus(options.bSuppressStatus);
		Meta_HideMetaContacts(false);
	}
}
