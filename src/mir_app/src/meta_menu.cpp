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
#include "clc.h"

#include <m_nudge.h>
#include "metacontacts.h"

static HGENMENU hMenuContact[MAX_CONTACTS];

static HGENMENU
	hMenuRoot,         // root menu item of all subs
	hMenuConvert,      // HANDLE to the convert menu item.
	hMenuAdd,          // HANDLE to the add to menu item.
	hMenuEdit,         // HANDLE to the edit menu item.
	hMenuDelete,       // HANDLE to the delete menu item.
	hMenuDefault,      // HANDLE to the delete menu item.
	hMenuForceDefault, // HANDLE to the delete menu item.
	hMenuOnOff;	       // HANDLE to the enable/disable menu item.

/////////////////////////////////////////////////////////////////////////////////////////
// Convert the contact chosen into a MetaContact.
//
// Create a new MetaContact, remove the selected contact from the \c CList
// and attach it to the MetaContact.
//
// @param wParam : HANDLE to the contact that has been chosen.
// @param lParam : Allways set to 0.

INT_PTR Meta_Convert(WPARAM wParam, LPARAM)
{
	ptrW tszGroup(Clist_GetGroup(wParam));

	// Create a new metacontact
	MCONTACT hMetaContact = db_add_contact();
	if (hMetaContact == 0)
		return 0;

	DBCachedContact *cc = g_pCurrDb->getCache()->GetCachedContact(hMetaContact);
	if (cc == nullptr)
		return 0;

	db_set_dw(hMetaContact, META_PROTO, "NumContacts", 0);
	cc->nSubs = 0;
	g_pCurrDb->MetaSetDefault(cc); // explicitly write default sub to a db

	// Add the MetaContact protocol to the new meta contact
	Proto_AddToContact(hMetaContact, META_PROTO);

	if (tszGroup)
		Clist_SetGroup(hMetaContact, tszGroup);

	// Assign the contact to the MetaContact just created (and make default).
	if (!Meta_Assign(wParam, hMetaContact, true)) {
		MessageBoxW(nullptr, TranslateT("There was a problem in assigning the contact to the metacontact"), TranslateT("Error"), MB_ICONEXCLAMATION);
		db_delete_contact(hMetaContact);
		return 0;
	}

	// hide the contact if clist groups disabled (shouldn't create one anyway since menus disabled)
	if (!db_mc_isEnabled())
		Contact::Hide(hMetaContact);

	return hMetaContact;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Removes a sub from a metacontact

void Meta_RemoveContactNumber(DBCachedContact *ccMeta, int number, bool bUpdateInfo, bool bDeleteSub)
{
	if (ccMeta == nullptr)
		return;

	// make sure this contact thinks it's part of this metacontact
	DBCachedContact *ccSub = g_pCurrDb->getCache()->GetCachedContact(Meta_GetContactHandle(ccMeta, number));
	if (ccSub != nullptr)
		if (ccSub->parentID == ccMeta->contactID)
			Contact::Hide(ccSub->contactID, false);

	// each contact from 'number' upwards will be moved down one
	// and the last one will be deleted
	for (int i = number+1; i < ccMeta->nSubs; i++)
		Meta_SwapContacts(ccMeta, i, i - 1);

	// remove the last one
	int id = ccMeta->nSubs - 1;
	char buffer[512];
	mir_snprintf(buffer, "Handle%d", id);
	db_unset(ccMeta->contactID, META_PROTO, buffer);

	mir_snprintf(buffer, "Protocol%d", id);
	db_unset(ccMeta->contactID, META_PROTO, buffer);

	mir_snprintf(buffer, "Status%d", id);
	db_unset(ccMeta->contactID, META_PROTO, buffer);

	mir_snprintf(buffer, "StatusString%d", id);
	db_unset(ccMeta->contactID, META_PROTO, buffer);

	mir_snprintf(buffer, "Login%d", id);
	db_unset(ccMeta->contactID, META_PROTO, buffer);

	mir_snprintf(buffer, "Nick%d", id);
	db_unset(ccMeta->contactID, META_PROTO, buffer);

	mir_snprintf(buffer, "CListName%d", id);
	db_unset(ccMeta->contactID, META_PROTO, buffer);

	if (ccSub != nullptr) {
		g_pCurrDb->MetaDetouchSub(ccMeta, ccMeta->nSubs - 1);

		if (bDeleteSub)
			g_pCurrDb->MetaRemoveSubHistory(ccSub);
		else {
			g_pCurrDb->MetaSplitHistory(ccMeta, ccSub);
			ccSub->parentID = 0;
		}
	}

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
			PROTO_AVATAR_INFORMATION ai = { 0 };
			ai.hContact = ccMeta->contactID;
			ai.format = PA_FORMAT_UNKNOWN;
			wcsncpy_s(ai.filename, L"X", _TRUNCATE);

			if (CallProtoService(META_PROTO, PS_GETAVATARINFO, 0, (LPARAM)&ai) == GAIR_SUCCESS)
				db_set_ws(ccMeta->contactID, "ContactPhoto", "File", ai.filename);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Delete a MetaContact from the database
//
// Delete a MetaContact and remove all the information
// concerning this MetaContact in the contact linked to it.
//
// @param wParam : HANDLE to the MetaContact to be deleted, or to the subcontact to be removed from the MetaContact
// @param lParam : BOOL flag indicating whether to ask 'are you sure' when deleting a MetaContact

INT_PTR Meta_Delete(WPARAM hContact, LPARAM bSkipQuestion)
{
	DBCachedContact *cc = g_pCurrDb->getCache()->GetCachedContact(hContact);
	if (cc == nullptr)
		return 1;

	// The wParam is a metacontact
	if (cc->IsMeta()) {
		// check from recursion - see second half of this function
		if (!bSkipQuestion && IDYES != 
			MessageBoxW(g_clistApi.hwndContactList,
				TranslateT("This will remove the metacontact permanently.\n\nProceed anyway?"),
				TranslateT("Are you sure?"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
			return 0;

		for (int i = cc->nSubs-1; i >= 0; i--)
			Meta_RemoveContactNumber(cc, i, false);

		NotifyEventHooks(hSubcontactsChanged, hContact, 0);
		db_delete_contact(hContact);
	}
	else if (cc->IsSub()) {
		if ((cc = g_pCurrDb->getCache()->GetCachedContact(cc->parentID)) == nullptr)
			return 2;

		if (cc->nSubs == 1) {
			if (IDYES == MessageBoxW(nullptr, TranslateW(szDelMsg), TranslateT("Delete metacontact?"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1))
				Meta_Delete(cc->contactID, 1);

			return 0;
		}

		Meta_RemoveContactNumber(cc, Meta_GetContactNumber(cc, hContact), true);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Set contact as MetaContact default
//
// Set the given contact to be the default one for the metacontact to which it is linked.
//
// @param wParam : HANDLE to the MetaContact to be set as default
// @param lParam : HWND to the clist window
// (This means the function has been called via the contact menu).

INT_PTR Meta_Default(WPARAM hSub, LPARAM)
{
	DBCachedContact *cc = g_pCurrDb->getCache()->GetCachedContact(db_mc_getMeta(hSub));
	if (cc && cc->IsMeta())
		db_mc_setDefault(cc->contactID, hSub, true);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Called when the context-menu of a contact is about to be displayed
//
// This will test which of the 4 menu item should be displayed, depending
// on which contact triggered the event
//
// @param wParam : HANDLE to the contact that triggered the event
// @param lParam :	Always set to 0;

int Meta_ModifyMenu(WPARAM hMeta, LPARAM)
{
	DBCachedContact *cc = g_pCurrDb->getCache()->GetCachedContact(hMeta);
	if (cc == nullptr)
		return 0;
		
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
		Menu_ModifyItem(hMenuDelete, LPGENW("Remove from metacontact"));

		// show subcontact menu items
		CMStringW tszNick;
		for (int i = 0; i < MAX_CONTACTS; i++) {
			if (i >= cc->nSubs) {
				Menu_ShowItem(hMenuContact[i], false);
				continue;
			}

			MCONTACT hContact = Meta_GetContactHandle(cc, i);
			LPCTSTR ptszName;

			if (g_metaOptions.menu_contact_label == DNT_UID) {
				Meta_GetSubNick(hMeta, i, tszNick);
				ptszName = tszNick.GetBuffer();
			}
			else ptszName = Clist_GetContactDisplayName(hContact);

			HICON hIcon = ImageList_GetIcon(hCListImages, Clist_GetContactIcon(hContact), 0);
			Menu_ModifyItem(hMenuContact[i], ptszName, hIcon, 0);
			DestroyIcon(hIcon);
			
			Menu_ShowItem(hMenuRoot, true);
		}

		// show hide nudge menu item
		char serviceFunc[256];
		mir_snprintf(serviceFunc, "%s%s", Proto_GetBaseAccountName(Meta_GetMostOnline(cc)), PS_SEND_NUDGE);
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

		Menu_ModifyItem(hMenuDelete, LPGENW("Remove from metacontact"));
		Menu_ShowItem(hMenuDelete, true);

		Menu_ShowItem(hMenuAdd, false);
		Menu_ShowItem(hMenuConvert, false);
		Menu_ShowItem(hMenuEdit, false);
	}
	else {
		// The contact is neutral
		bool bHideChat = !Contact::IsGroupChat(hMeta, cc->szProto);
		Menu_ShowItem(hMenuAdd, bHideChat);
		Menu_ShowItem(hMenuConvert, bHideChat);
		Menu_ShowItem(hMenuEdit, false);
		Menu_ShowItem(hMenuDelete, false);
		Menu_ShowItem(hMenuDefault, false);
	}

	for (int i = 0; i < MAX_CONTACTS; i++)
		Menu_ShowItem(hMenuContact[i], false);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Toggle metacontacts on/off

INT_PTR Meta_OnOff(WPARAM, LPARAM)
{
	bool bToggled = !db_mc_isEnabled();
	db_set_b(0, META_PROTO, "Enabled", bToggled);
	if (bToggled)
		Menu_ModifyItem(hMenuOnOff, LPGENW("Toggle metacontacts off"), Meta_GetIconHandle(I_MENU));
	else
		Menu_ModifyItem(hMenuOnOff, LPGENW("Toggle metacontacts on"), Meta_GetIconHandle(I_MENUOFF));

	db_mc_enable(bToggled);
	Meta_HideMetaContacts(!bToggled);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Menu initialization

void InitMenus()
{
	CMenuItem mi(&g_plugin);

	// main menu item
	SET_UID(mi, 0x8999a6ca, 0x9c66, 0x49c1, 0xad, 0xe1, 0x48, 0x17, 0x28, 0xb, 0x94, 0x86);
	mi.hIcolibItem = Meta_GetIconHandle(I_MENU);
	mi.name.a = LPGEN("Toggle metacontacts off");
	mi.pszService = "MetaContacts/OnOff";
	mi.position = 500010000;
	hMenuOnOff = Menu_AddMainMenuItem(&mi);

	// contact menu items
	SET_UID(mi, 0x48cdb295, 0x858f, 0x4f4f, 0x80, 0xc7, 0x50, 0x49, 0x91, 0x75, 0xa6, 0x63);
	mi.hIcolibItem = Meta_GetIconHandle(I_CONVERT);
	mi.position = -200010;
	mi.name.a = LPGEN("Convert to metacontact");
	mi.pszService = "MetaContacts/Convert";
	hMenuConvert = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0xf1437693, 0x69f5, 0x48b0, 0x89, 0xed, 0x29, 0x2c, 0x20, 0x1f, 0xed, 0x3e);
	mi.hIcolibItem = Meta_GetIconHandle(I_ADD);
	mi.position = -200009;
	mi.name.a = LPGEN("Add to existing metacontact...");
	mi.pszService = "MetaContacts/AddTo";
	hMenuAdd = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x1673fd9e, 0x8d30, 0x4e07, 0x9f, 0x1b, 0xec, 0x92, 0xc0, 0x10, 0x90, 0x64);
	mi.hIcolibItem = Meta_GetIconHandle(I_EDIT);
	mi.position = -200010;
	mi.name.a = LPGEN("Edit metacontact...");
	mi.pszService = "MetaContacts/Edit";
	hMenuEdit = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x38b7400, 0x685a, 0x497d, 0xbc, 0x15, 0x99, 0x45, 0xcc, 0x1d, 0x9d, 0xaf);
	mi.hIcolibItem = Meta_GetIconHandle(I_SETDEFAULT);
	mi.position = -200009;
	mi.name.a = LPGEN("Set as metacontact default");
	mi.pszService = "MetaContacts/Default";
	hMenuDefault = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x102849ca, 0x9d2f, 0x4265, 0x81, 0xdc, 0x5d, 0xc8, 0x82, 0xb7, 0x70, 0xe5);
	mi.hIcolibItem = Meta_GetIconHandle(I_REMOVE);
	mi.position = -200008;
	mi.name.a = LPGEN("Delete metacontact");
	mi.pszService = "MetaContacts/Delete";
	hMenuDelete = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0xd31e772d, 0xf85a, 0x45e9, 0xb3, 0xa8, 0xf7, 0xa5, 0xf4, 0xfc, 0xf9, 0x6e);
	mi.position = -99000;
	mi.flags = CMIF_HIDDEN;
	mi.hIcolibItem = nullptr;
	mi.name.a = LPGEN("Subcontacts");
	hMenuRoot = Menu_AddContactMenuItem(&mi);

	UNSET_UID(mi);
	mi.flags |= CMIF_SYSTEM;
	mi.root = hMenuRoot;
	for (int i = 0; i < MAX_CONTACTS; i++) {
		mi.position--;
		mi.name.a = "";

		char buffer[512];
		mir_snprintf(buffer, "MetaContacts/MenuFunc%d", i);
		mi.pszService = buffer;
		mi.position++;
		hMenuContact[i] = Menu_AddContactMenuItem(&mi);
	}

	Meta_HideLinkedContacts();

	if (!db_mc_isEnabled()) {
		// modify main menu item
		Menu_ModifyItem(hMenuOnOff, LPGENW("Toggle metacontacts on"), Meta_GetIconHandle(I_MENUOFF));
		Meta_HideMetaContacts(true);
	}
	else Meta_HideMetaContacts(false);
}
