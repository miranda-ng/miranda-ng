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

/** @file meta_menu.c
*
* Functions needed to handle MetaContacts.
* Centralizes functions called when the user chooses
* an option integrated in the context-menu of the \c CList.
*/

#include "metacontacts.h"

/** Convert the contact chosen into a MetaContact.
*
* Create a new MetaContact, remove the selected contact from the \c CList
* and attach it to the MetaContact.
*
* @param wParam :	\c HANDLE to the contact that has been chosen.
* @param lParam :	Allways set to 0.
*/

INT_PTR Meta_Convert(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
	char *group = 0;

	// Get some information about the selected contact.
	if ( !db_get_utf(wParam, "CList", "Group", &dbv)) {
		group = _strdup(dbv.pszVal);
		db_free(&dbv);
	}

	// Create a new metacontact
	MCONTACT hMetaContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD,0,0);
	if (hMetaContact) {
		db_set_dw(hMetaContact, META_PROTO, META_ID,nextMetaID);
		db_set_dw(hMetaContact, META_PROTO, "NumContacts",0);
		db_set_dw(NULL, META_PROTO, "NextMetaID", ++nextMetaID);

		// Add the MetaContact protocol to the new meta contact
		CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hMetaContact, (LPARAM)META_PROTO);

		if (group)
			db_set_utf(hMetaContact, "CList", "Group", group);

		// Assign the contact to the MetaContact just created (and make default).
		if ( !Meta_Assign(wParam, hMetaContact, TRUE)) {
			MessageBox(0, TranslateT("There was a problem in assigning the contact to the MetaContact"), TranslateT("Error"), MB_ICONEXCLAMATION);
			CallService(MS_DB_CONTACT_DELETE, (WPARAM)hMetaContact, 0);
			return 0;
		}

		// hide the contact if clist groups disabled (shouldn't create one anyway since menus disabled)
		if ( !Meta_IsEnabled())
			db_set_b(hMetaContact, "CList", "Hidden", 1);
	}

	free(group);
	return (INT_PTR)hMetaContact;
}

/** Display the <b>'Add to'</b> Dialog
*
* Present a dialog in which the user can choose to which MetaContact this
* contact will be added
*
* @param wParam :	\c HANDLE to the contact that has been chosen.
* @param lParam :	Allways set to 0.
*/

INT_PTR Meta_AddTo(WPARAM wParam, LPARAM lParam)
{
	HWND clui = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_METASELECT), clui, &Meta_SelectDialogProc, (LPARAM)wParam);
	return 0;
}

/** Display the <b>'Edit'</b> Dialog
*
* Present a dialog in which the user can edit some properties of the MetaContact.
*
* @param wParam :	\c HANDLE to the MetaContact to be edited.
* @param lParam :	Allways set to 0.
*/
INT_PTR Meta_Edit(WPARAM wParam,LPARAM lParam)
{
	HWND clui = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_METAEDIT), clui, Meta_EditDialogProc, (LPARAM)wParam);
	return 0;
}

void Meta_RemoveContactNumber(MCONTACT hMeta, int number)
{
	int num_contacts = db_get_dw(hMeta, META_PROTO, "NumContacts", 0);
	int default_contact = db_get_dw(hMeta, META_PROTO, "Default", -1);
	if (number < 0 && number >= num_contacts)
		return;

	// get the handle
	MCONTACT hContact = Meta_GetContactHandle(hMeta, number);

	// make sure this contact thinks it's part of this metacontact
	if ((MCONTACT)db_get_dw(hContact, META_PROTO, "Handle", 0) == hMeta) {
		// remove link to meta contact
		db_unset(hContact, META_PROTO, "IsSubcontact");
		db_unset(hContact, META_PROTO, META_LINK);
		db_unset(hContact, META_PROTO, "Handle");
		db_unset(hContact, META_PROTO, "ContactNumber");
		// unhide - must be done after removing link (see meta_services.c:Meta_ChangeStatus)
		Meta_RestoreGroup(hContact);
		db_unset(hContact, META_PROTO, "OldCListGroup");

		// stop ignoring, if we were
		if (options.suppress_status)
			CallService(MS_IGNORE_UNIGNORE, hContact, (WPARAM)IGNOREEVENT_USERONLINE);
	}

	// each contact from 'number' upwards will be moved down one
	// and the last one will be deleted
	for (int i = number + 1; i < num_contacts; i++)
		Meta_SwapContacts(hMeta, i, i-1);

	// remove the last one
	char buffer[512], idStr[20];
	_itoa(num_contacts-1, idStr, 10);
	strcpy(buffer, "Protocol"); strcat(buffer, idStr);
	db_unset(hMeta, META_PROTO, buffer);

	strcpy(buffer, "Status"); strcat(buffer, idStr);
	db_unset(hMeta, META_PROTO, buffer);

	strcpy(buffer, "Handle"); strcat(buffer, idStr);
	db_unset(hMeta, META_PROTO, buffer);

	strcpy(buffer, "StatusString"); strcat(buffer, idStr);
	db_unset(hMeta, META_PROTO, buffer);

	strcpy(buffer, "Login"); strcat(buffer, idStr);
	db_unset(hMeta, META_PROTO, buffer);

	strcpy(buffer, "Nick"); strcat(buffer, idStr);
	db_unset(hMeta, META_PROTO, buffer);

	strcpy(buffer, "CListName"); strcat(buffer, idStr);
	db_unset(hMeta, META_PROTO, buffer);

	// if the default contact was equal to or greater than 'number', decrement it (and deal with ends)
	if (default_contact >= number) {
		default_contact--;
		if (default_contact < 0)
			default_contact = 0;

		db_set_dw(hMeta, META_PROTO, "Default", (DWORD)default_contact);
		NotifyEventHooks(hEventDefaultChanged, (WPARAM)hMeta, (LPARAM)Meta_GetContactHandle(hMeta, default_contact));
	}
	num_contacts--;
	db_set_dw(hMeta, META_PROTO, "NumContacts", (DWORD)num_contacts);

	// fix nick
	hContact = Meta_GetMostOnline(hMeta);
	Meta_CopyContactNick(hMeta, hContact);

	// fix status
	Meta_FixStatus(hMeta);

	// fix avatar
	hContact = Meta_GetMostOnlineSupporting(hMeta, PFLAGNUM_4, PF4_AVATARS);
	if (hContact) {
		PROTO_AVATAR_INFORMATIONT AI = { sizeof(AI) };
		AI.hContact = hMeta;
		AI.format = PA_FORMAT_UNKNOWN;
		_tcscpy(AI.filename, _T("X"));

		if ((int)CallProtoService(META_PROTO, PS_GETAVATARINFOT, 0, (LPARAM)&AI) == GAIR_SUCCESS)
			db_set_ts(hMeta, "ContactPhoto", "File",AI.filename);
	}
}

/** Delete a MetaContact from the database
*
* Delete a MetaContact and remove all the information
* concerning this MetaContact in the contact linked to it.
*
* @param wParam :	\c HANDLE to the MetaContact to be deleted, or to the subcontact to be removed from the MetaContact
* @param lParam :	\c BOOL flag indicating whether to ask 'are you sure' when deleting a MetaContact
*/

INT_PTR Meta_Delete(WPARAM wParam,LPARAM lParam)
{
	DWORD metaID;

	// The wParam is a metacontact
	if ((metaID = db_get_dw(wParam, META_PROTO, META_ID, (DWORD)-1)) != (DWORD)-1) {
		if ( !lParam) { // check from recursion - see second half of this function
			if ( MessageBox((HWND)CallService(MS_CLUI_GETHWND,0,0),
					TranslateT("This will remove the MetaContact permanently.\n\nProceed Anyway?"),
					TranslateT("Are you sure?"),MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) != IDYES)
				return 0;
		}

		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
			 // This contact is assigned to the MetaContact that will be deleted, clear the "MetaContacts" information
			if ( db_get_dw(hContact, META_PROTO, META_LINK,(DWORD)-1) == metaID) {
				db_unset(hContact, META_PROTO, "IsSubcontact");
				db_unset(hContact, META_PROTO, META_LINK);
				db_unset(hContact, META_PROTO, "Handle");
				db_unset(hContact, META_PROTO, "ContactNumber");

				// unhide - must be done after removing link (see meta_services.c:Meta_ChangeStatus)
				Meta_RestoreGroup(hContact);
				db_unset(hContact, META_PROTO, "OldCListGroup");

				// stop ignoring, if we were
				if (options.suppress_status)
					CallService(MS_IGNORE_UNIGNORE, hContact, (WPARAM)IGNOREEVENT_USERONLINE);
			}
		}

		NotifyEventHooks(hSubcontactsChanged, (WPARAM)wParam, 0);
		CallService(MS_DB_CONTACT_DELETE,wParam,0);
	}
	else {
		MCONTACT hMeta = (MCONTACT)db_get_dw(wParam, META_PROTO, "Handle", 0);
		DWORD num_contacts = db_get_dw(hMeta, META_PROTO, "NumContacts", -1);

		if (num_contacts == 1) {
			if (IDYES == MessageBox(0, TranslateT(szDelMsg), TranslateT("Delete MetaContact?"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1))
				Meta_Delete((WPARAM)hMeta,(LPARAM)1);

			return 0;
		}

		Meta_RemoveContactNumber(hMeta, db_get_dw(wParam, META_PROTO, "ContactNumber", -1));
	}
	return 0;
}

/** Set contact as MetaContact default
*
* Set the given contact to be the default one for the metacontact to which it is linked.
*
* @param wParam :	\c HANDLE to the MetaContact to be set as default
* @param lParam :	\c HWND to the clist window
					(This means the function has been called via the contact menu).
*/
INT_PTR Meta_Default(WPARAM wParam,LPARAM lParam)
{
	MCONTACT hMeta;

	// the wParam is a subcontact
	if ((hMeta = (MCONTACT)db_get_dw(wParam, META_PROTO, "Handle",0)) != 0) {
		db_set_dw(hMeta, META_PROTO, "Default", (DWORD)Meta_GetContactNumber(wParam));
		NotifyEventHooks(hEventDefaultChanged, (WPARAM)hMeta, (LPARAM)wParam);
	}
	return 0;
}

/** Set/unset (i.e. toggle) contact to force use of default contact
*
* Set the given contact to be the default one for the metacontact to which it is linked.
*
* @param wParam :	\c HANDLE to the MetaContact to be set as default
* @param lParam :	\c HWND to the clist window
					(This means the function has been called via the contact menu).
*/

INT_PTR Meta_ForceDefault(WPARAM wParam,LPARAM lParam)
{
	// the wParam is a MetaContact
	if (db_get_dw(wParam, META_PROTO, META_ID, (DWORD)-1) != (DWORD)-1) {
		BOOL current = db_get_b(wParam, META_PROTO, "ForceDefault", 0);
		current = !current;
		db_set_b(wParam, META_PROTO, "ForceDefault", (BYTE)current);

		db_set_dw(wParam, META_PROTO, "ForceSend", 0);

		if (current)
			NotifyEventHooks(hEventForceSend, wParam, (LPARAM)Meta_GetContactHandle(wParam, db_get_dw(wParam, META_PROTO, "Default", -1)));
		else
			NotifyEventHooks(hEventUnforceSend, wParam, 0);
	}
	return 0;
}

HGENMENU hMenuContact[MAX_CONTACTS];

/** Called when the context-menu of a contact is about to be displayed
*
* This will test which of the 4 menu item should be displayed, depending
* on which contact triggered the event
*
* @param wParam :	\c HANDLE to the contact that triggered the event
* @param lParam :	Always set to 0;
*/

int Meta_ModifyMenu(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
	char buf[512], idStr[512];
	WORD status;
	CLISTMENUITEM mi = { sizeof(mi) };

	if (db_get_dw(wParam, META_PROTO, META_ID,-1) != (DWORD)-1) {
		// save the mouse pos in case they open a subcontact menu
		GetCursorPos(&menuMousePoint);

		// This is a MetaContact, show the edit, force default, and the delete menu, and hide the others
		Menu_ShowItem(hMenuEdit, true);
		Menu_ShowItem(hMenuAdd, false);
		Menu_ShowItem(hMenuConvert, false);
		Menu_ShowItem(hMenuDefault, false);
		Menu_ShowItem(hMenuDelete, false);

		mi.flags = CMIM_NAME | CMIF_TCHAR;
		mi.ptszName = (TCHAR*)TranslateT("Remove from MetaContact");
		Menu_ModifyItem(hMenuDelete, &mi);

		//show subcontact menu items
		int num_contacts = db_get_dw(wParam, META_PROTO, "NumContacts", 0);
		for (int i = 0; i < MAX_CONTACTS; i++) {
			if (i >= num_contacts) {
				Menu_ShowItem(hMenuContact[i], false);
				continue;
			}

			MCONTACT hContact = Meta_GetContactHandle(wParam, i);
			char *szProto = GetContactProto(hContact);
			if ( !szProto)
				status = ID_STATUS_OFFLINE;
			else
				status = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);

			if (options.menu_contact_label == DNT_UID) {
				strcpy(buf, "Login");
				strcat(buf, _itoa(i, idStr, 10));

				db_get(wParam, META_PROTO, buf, &dbv);
				switch(dbv.type) {
				case DBVT_ASCIIZ:
					mir_snprintf(buf,512,"%s",dbv.pszVal);
					break;
				case DBVT_BYTE:
					mir_snprintf(buf,512,"%d",dbv.bVal);
					break;
				case DBVT_WORD:
					mir_snprintf(buf,512,"%d",dbv.wVal);
					break;
				case DBVT_DWORD:
					mir_snprintf(buf,512,"%d",dbv.dVal);
					break;
				default:
					buf[0] = 0;
				}
				db_free(&dbv);
				mi.pszName = buf;
				mi.flags = 0;
			}
			else {
				mi.ptszName = pcli->pfnGetContactDisplayName(hContact, GCDNF_TCHAR);
				mi.flags = CMIF_TCHAR;
			}

			mi.flags |= CMIM_FLAGS | CMIM_NAME | CMIM_ICON;

			int iconIndex = CallService(MS_CLIST_GETCONTACTICON, hContact, 0);
			mi.hIcon = ImageList_GetIcon((HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0), iconIndex, 0);

			Menu_ModifyItem(hMenuContact[i], &mi);
			DestroyIcon(mi.hIcon);
		}

		// show hide nudge menu item
		#define MS_NUDGE_SHOWMENU	"NudgeShowMenu"
		// wParam = char *szProto
		// lParam = BOOL show
		char serviceFunc[256];
		mir_snprintf(serviceFunc, 256, "%s%s", GetContactProto( Meta_GetMostOnline(wParam)), PS_SEND_NUDGE);
		CallService(MS_NUDGE_SHOWMENU, (WPARAM)META_PROTO, (LPARAM)ServiceExists(serviceFunc));
	}
	else { // This is a simple contact
		if ( !Meta_IsEnabled()) {
			// groups disabled - all meta menu options hidden
			Menu_ShowItem(hMenuDefault, false);
			Menu_ShowItem(hMenuDelete, false);
			Menu_ShowItem(hMenuAdd, false);
			Menu_ShowItem(hMenuConvert, false);
			Menu_ShowItem(hMenuEdit, false);
		}
		else if (db_get_dw(wParam, META_PROTO, META_LINK,(DWORD)-1)!=(DWORD)-1) {
			// The contact is affected to a metacontact.
			Menu_ShowItem(hMenuDefault, true);

			mi.flags = CMIM_NAME | CMIF_TCHAR;
			mi.ptszName = (TCHAR*)TranslateT("Remove from MetaContact");
			Menu_ModifyItem(hMenuDelete, &mi);

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
	}
	return 0;
}
