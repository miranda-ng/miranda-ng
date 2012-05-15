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
INT_PTR Meta_Convert(WPARAM wParam,LPARAM lParam)
{
	HANDLE hMetaContact;
	DBVARIANT dbv;
	char *group = 0;//, *proto;
		
	// Get some information about the selected contact.
//	proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0);
	if(!DBGetContactSettingStringUtf((HANDLE)wParam,"CList","Group",&dbv)) {
		group = _strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	// Create a new metacontact
	hMetaContact = (HANDLE)CallService(MS_DB_CONTACT_ADD,0,0);
			
	// Add the info for the metacontact
	if(hMetaContact)
	{

		DBWriteContactSettingDword(hMetaContact,META_PROTO,META_ID,nextMetaID);
		DBWriteContactSettingDword(hMetaContact,META_PROTO,"NumContacts",0);
		DBWriteContactSettingDword(NULL,META_PROTO,"NextMetaID",++nextMetaID);

		// Add the MetaContact protocol to the new meta contact
		CallService( MS_PROTO_ADDTOCONTACT, ( WPARAM )hMetaContact, ( LPARAM )META_PROTO );

		if(group) {
			if(ServiceExists(MS_DB_CONTACT_GETSETTING_STR))
				DBWriteContactSettingStringUtf(hMetaContact,"CList","Group",group);
			else
				DBWriteContactSettingString(hMetaContact,"CList","Group",group);
		}
		
		// Assign the contact to the MetaContact just created (and make default).
		if(!Meta_Assign((HANDLE)wParam,hMetaContact,TRUE)) {
			MessageBox(0,Translate("There was a problem in assigning the contact to the MetaContact"),Translate("Error"),MB_ICONEXCLAMATION);
			CallService(MS_DB_CONTACT_DELETE, (WPARAM)hMetaContact, 0);
			return 0;
		}

		// hide the contact if clist groups disabled (shouldn't create one anyway since menus disabled)
		if(!Meta_IsEnabled())
			DBWriteContactSettingByte(hMetaContact, "CList", "Hidden", 1);

	}

	//	Update the graphics
	CallService(MS_CLUI_SORTLIST,0,0);

	free(group);
	return (int)hMetaContact;
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
	HWND clui = (HWND)CallService(MS_CLUI_GETHWND,0,0);
	DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_METASELECT),clui,&Meta_SelectDialogProc,(LPARAM)wParam);
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
	HWND clui = (HWND)CallService(MS_CLUI_GETHWND,0,0);
	DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_METAEDIT),clui,Meta_EditDialogProc,(LPARAM)wParam);
	return 0;
}

/* DB/Contact/WriteSetting service
Change the value of, or create a new value with, a named setting for a specific
contact in the database to the given value
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)(DBCONTACTWRITESETTING*)&dbcws
hContact should have been returned by find*contact or addcontact
Returns 0 on success or nonzero if hContact was invalid
Note that DBCONTACTGETSETTING takes a pointer to a DBVARIANT, whereas
DBCONTACTWRITESETTING contains a DBVARIANT.
Because this is such a common function there are some short helper function at
the bottom of this header that use it.
Triggers a db/contact/settingchanged event just before it returns.
*/
//typedef struct {
//	const char *szModule;	// pointer to name of the module that wrote the
//	                        // setting to get
//	const char *szSetting;	// pointer to name of the setting to get
//	DBVARIANT value;		// variant containing the value to set
//} DBCONTACTWRITESETTING;
//#define MS_DB_CONTACT_WRITESETTING  "DB/Contact/WriteSetting"

void Meta_RemoveContactNumber(HANDLE hMeta, int number) {
	int i, num_contacts, default_contact;
	HANDLE hContact;//, handle;

	char buffer[512], buffer2[512];

	num_contacts = DBGetContactSettingDword(hMeta, META_PROTO, "NumContacts", 0);
	default_contact = DBGetContactSettingDword(hMeta, META_PROTO, "Default", -1);
	if(number >= 0 && number < num_contacts) {

		// get the handle
		hContact = Meta_GetContactHandle(hMeta, number);

		// make sure this contact thinks it's part of this metacontact
		if((HANDLE)DBGetContactSettingDword(hContact,META_PROTO,"Handle", 0) == hMeta) {

			// remove link to meta contact
			DBDeleteContactSetting(hContact,META_PROTO,"IsSubcontact");
			DBDeleteContactSetting(hContact,META_PROTO,META_LINK);
			DBDeleteContactSetting(hContact,META_PROTO,"Handle");
			DBDeleteContactSetting(hContact,META_PROTO,"ContactNumber");
			// unhide - must be done after removing link (see meta_services.c:Meta_ChangeStatus)
			Meta_RestoreGroup(hContact);
			DBDeleteContactSetting(hContact,META_PROTO,"OldCListGroup");
			//CallService(MS_PROTO_REMOVEFROMCONTACT,(WPARAM)hContact,(LPARAM)META_PROTO);
			CallService(MS_PROTO_REMOVEFROMCONTACT,(WPARAM)hContact,(LPARAM)META_FILTER);
			// stop ignoring, if we were
			if(options.suppress_status)
				CallService(MS_IGNORE_UNIGNORE, (WPARAM)hContact, (WPARAM)IGNOREEVENT_USERONLINE);
		}

		// remove history from metacontact
		//Meta_RemoveHistory(hMeta, hContact);

		// each contact from 'number' upwards will be moved down one
		// and the last one will be deleted
		for(i = number + 1; i < num_contacts; i++) {
			Meta_SwapContacts(hMeta, i, i-1);
		}

		// remove the last one
		strcpy(buffer, "Protocol");
		strcat(buffer, _itoa((num_contacts - 1), buffer2, 10));
		DBDeleteContactSetting(hMeta, META_PROTO, buffer);
		strcpy(buffer, "Status");
		strcat(buffer, _itoa((num_contacts - 1), buffer2, 10));
		DBDeleteContactSetting(hMeta, META_PROTO, buffer);
		strcpy(buffer, "Handle");
		strcat(buffer, _itoa((num_contacts - 1), buffer2, 10));
		DBDeleteContactSetting(hMeta, META_PROTO, buffer);
		strcpy(buffer, "StatusString");
		strcat(buffer, _itoa((num_contacts - 1), buffer2, 10));
		DBDeleteContactSetting(hMeta, META_PROTO, buffer);
		strcpy(buffer, "Login");
		strcat(buffer, _itoa((num_contacts - 1), buffer2, 10));
		DBDeleteContactSetting(hMeta, META_PROTO, buffer);
		strcpy(buffer, "Nick");
		strcat(buffer, _itoa((num_contacts - 1), buffer2, 10));
		DBDeleteContactSetting(hMeta, META_PROTO, buffer);
		strcpy(buffer, "CListName");
		strcat(buffer, _itoa((num_contacts - 1), buffer2, 10));
		DBDeleteContactSetting(hMeta, META_PROTO, buffer);

		// if the default contact was equal to or greater than 'number', decrement it (and deal with ends)
		if(default_contact >= number) {
			default_contact--;
			if(default_contact < 0) 
				default_contact = 0;

			DBWriteContactSettingDword(hMeta, META_PROTO, "Default", (DWORD)default_contact);
			NotifyEventHooks(hEventDefaultChanged, (WPARAM)hMeta, (LPARAM)Meta_GetContactHandle(hMeta, default_contact));
		}
		num_contacts--;
		DBWriteContactSettingDword(hMeta, META_PROTO, "NumContacts", (DWORD)num_contacts);

		// fix nick
		hContact = Meta_GetMostOnline(hMeta);
		Meta_CopyContactNick(hMeta, hContact);

		// fix status
		Meta_FixStatus(hMeta);

		// fix avatar
		hContact = Meta_GetMostOnlineSupporting(hMeta, PFLAGNUM_4, PF4_AVATARS);
		if(hContact) {
			PROTO_AVATAR_INFORMATION AI;

			AI.cbSize = sizeof(AI);
			AI.hContact = hMeta;
			AI.format = PA_FORMAT_UNKNOWN;
			strcpy(AI.filename, "X");

			if((int)CallProtoService(META_PROTO, PS_GETAVATARINFO, 0, (LPARAM)&AI) == GAIR_SUCCESS)
				DBWriteContactSettingString(hMeta, "ContactPhoto", "File",AI.filename);
		}
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
	HANDLE hContact;

	if((metaID=DBGetContactSettingDword((HANDLE)wParam,META_PROTO,META_ID,(DWORD)-1))!=(DWORD)-1)
	{// The wParam is a metacontact
		if(!lParam) { // check from recursion - see second half of this function
			if(MessageBox((HWND)CallService(MS_CLUI_GETHWND,0,0),Translate("This will remove the MetaContact permanently.\n\nProceed Anyway?"),
				Translate("Are you sure?"),MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2)!=IDYES)
			{
				return 0;
			}
		}

		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
		while(hContact)
		{	// Scans the database to get all the contacts that have been previously linked to this MetaContact
			if(DBGetContactSettingDword(hContact,META_PROTO,META_LINK,(DWORD)-1)==metaID)
			{ // This contact is assigned to the MetaContact that will be deleted, clear the "MetaContacts" information
				DBDeleteContactSetting(hContact,META_PROTO,"IsSubcontact");
				DBDeleteContactSetting(hContact,META_PROTO,META_LINK);
				DBDeleteContactSetting(hContact,META_PROTO,"Handle");
				DBDeleteContactSetting(hContact,META_PROTO,"ContactNumber");
				// unhide - must be done after removing link (see meta_services.c:Meta_ChangeStatus)
				Meta_RestoreGroup(hContact);
				DBDeleteContactSetting(hContact,META_PROTO,"OldCListGroup");

				CallService(MS_PROTO_REMOVEFROMCONTACT,(WPARAM)hContact,(LPARAM)META_FILTER);
				// stop ignoring, if we were
				if(options.suppress_status)
					CallService(MS_IGNORE_UNIGNORE, (WPARAM)hContact, (WPARAM)IGNOREEVENT_USERONLINE);
			}
			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0);
		}
		//DBDeleteContactSetting((HANDLE)wParam, META_PROTO, META_ID);
		//DBDeleteContactSetting((HANDLE)wParam, META_PROTO, "NumContacts");
		//CallService(MS_PROTO_REMOVEFROMCONTACT,wParam,(LPARAM)META_PROTO);
		NotifyEventHooks(hSubcontactsChanged, (WPARAM)wParam, 0);
		CallService(MS_DB_CONTACT_DELETE,wParam,0);
	}
	else
	{// The wParam is a simple contact
		//if(lParam == 0)
		//	return 1; // The function has been called by the menu of a simple contact. Should not happen.
		//else
		{// The function has been called by the edit dialog
			HANDLE hMeta = (HANDLE)DBGetContactSettingDword((HANDLE)wParam, META_PROTO, "Handle", 0);


			DWORD num_contacts = DBGetContactSettingDword(hMeta, META_PROTO, "NumContacts", -1);
			
			if(num_contacts == 1) {
				if(MessageBox(0,Translate("You are going to remove all the contacts associated with this MetaContact.\nThis will delete the MetaContact.\n\nProceed Anyway?"),
					Translate("Delete MetaContact?"),MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1)==IDYES)
				{
					// recurse - once
					Meta_Delete((WPARAM)hMeta,(LPARAM)1);
				}
				return 0;
			}

			Meta_RemoveContactNumber(hMeta, DBGetContactSettingDword((HANDLE)wParam,META_PROTO,"ContactNumber", -1));

			CallService(MS_PROTO_REMOVEFROMCONTACT,(WPARAM)wParam,(LPARAM)META_FILTER);
		}
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
	HANDLE hMeta;

	if((hMeta = (HANDLE)DBGetContactSettingDword((HANDLE)wParam,META_PROTO,"Handle",0)) != 0)
	{ // the wParam is a subcontact
		DBWriteContactSettingDword(hMeta, META_PROTO, "Default", (DWORD)Meta_GetContactNumber((HANDLE)wParam));
		NotifyEventHooks(hEventDefaultChanged, (WPARAM)hMeta, (LPARAM)(HANDLE)wParam);
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
	if(DBGetContactSettingDword((HANDLE)wParam,META_PROTO, META_ID, (DWORD)-1) != (DWORD)-1)
	{ // the wParam is a MetaContact

		BOOL current = DBGetContactSettingByte((HANDLE)wParam, META_PROTO, "ForceDefault", 0);
		current = !current;
		DBWriteContactSettingByte((HANDLE)wParam, META_PROTO, "ForceDefault", (BYTE)current);

		DBWriteContactSettingDword((HANDLE)wParam, META_PROTO, "ForceSend", 0);

		if(current) NotifyEventHooks(hEventForceSend, wParam, (LPARAM)Meta_GetContactHandle((HANDLE)wParam, DBGetContactSettingDword((HANDLE)wParam, META_PROTO, "Default", -1)));
		else NotifyEventHooks(hEventUnforceSend, wParam, 0);
	}
	return 0;
}

INT_PTR MenuFunc0(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 0);}
INT_PTR MenuFunc1(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 1);}
INT_PTR MenuFunc2(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 2);}
INT_PTR MenuFunc3(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 3);}
INT_PTR MenuFunc4(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 4);}
INT_PTR MenuFunc5(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 5);}
INT_PTR MenuFunc6(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 6);}
INT_PTR MenuFunc7(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 7);}
INT_PTR MenuFunc8(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 8);}
INT_PTR MenuFunc9(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 9);}
INT_PTR MenuFunc10(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 10);}
INT_PTR MenuFunc11(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 11);}
INT_PTR MenuFunc12(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 12);}
INT_PTR MenuFunc13(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 13);}
INT_PTR MenuFunc14(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 14);}
INT_PTR MenuFunc15(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 15);}
INT_PTR MenuFunc16(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 16);}
INT_PTR MenuFunc17(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 17);}
INT_PTR MenuFunc18(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 18);}
INT_PTR MenuFunc19(WPARAM wParam, LPARAM lParam) {return TranslateMenuFunc((HANDLE)wParam, 19);}

HANDLE hMenuContact[MAX_CONTACTS];

INT_PTR TranslateMenuFunc(HANDLE hMeta, int contact_number) {
	return Meta_ContactMenuFunc((WPARAM)hMeta, (LPARAM) contact_number);
}

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
	CLISTMENUITEM mi;
	DBVARIANT dbv;
	HANDLE hContact;
	char *proto;
	char buf[512], buffer2[512];
	int i, iconIndex;
	WORD status;

	mi.flags = CMIM_FLAGS;
	mi.cbSize = sizeof(CLISTMENUITEM);

	if(DBGetContactSettingDword((HANDLE)wParam,META_PROTO,META_ID,-1) != (DWORD)-1)
	{
		int num_contacts, i;

		// save the mouse pos in case they open a subcontact menu
		GetCursorPos(&menuMousePoint);
		
		// This is a MetaContact, show the edit, force default, and the delete menu, and hide the others
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuEdit, (LPARAM)&mi);
		//mi.flags |= CMIM_NAME;
		//if(DBGetContactSettingByte((HANDLE)wParam, META_PROTO, "ForceDefault", 0))
		//	mi.pszName = Translate("Unforce Default");
		//else
		//	mi.pszName = Translate("Force Default");
		//CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuForceDefault, (LPARAM)&mi);
		mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuAdd, (LPARAM)&mi);
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuConvert, (LPARAM)&mi);
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDefault, (LPARAM)&mi);
		mi.flags = CMIM_FLAGS | CMIM_NAME | CMIF_HIDDEN;	// we don't need delete - already in contact menu
		mi.pszName = Translate("Delete MetaContact");
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDelete, (LPARAM)&mi);


		//show subcontact menu items
		num_contacts = DBGetContactSettingDword((HANDLE)wParam, META_PROTO, "NumContacts", 0);
		for(i = 0; i < MAX_CONTACTS; i++) {
			if(i < num_contacts) {
				hContact = Meta_GetContactHandle((HANDLE)wParam, i);
				proto = _strdup((char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0));

				if(!proto)
					status = ID_STATUS_OFFLINE;
				else
					status = DBGetContactSettingWord(hContact, proto, "Status", ID_STATUS_OFFLINE);

				if(options.menu_contact_label == DNT_UID) {
					strcpy(buf, "Login");
					strcat(buf, _itoa(i, buffer2, 10));

					DBGetContactSetting((HANDLE)wParam,META_PROTO,buf,&dbv);
					switch(dbv.type)
					{
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
					DBFreeVariant(&dbv);
					mi.pszName = buf;
					mi.flags = 0;
				} else {
					char *name = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
					char *wname = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_UNICODE);

					if(wname && strncmp(name, wname, strlen(name)) != 0) {
						mi.pszName = wname;
						mi.flags = CMIF_UNICODE;
					}
					else {
						mi.pszName = name;
						mi.flags = 0;
					}
				}

				mi.flags |= CMIM_FLAGS | CMIM_NAME | CMIM_ICON;

				//mi.hIcon = LoadSkinnedProtoIcon(proto, status);
				iconIndex = (int)CallService(MS_CLIST_GETCONTACTICON, (WPARAM)hContact, 0);
				mi.hIcon = ImageList_GetIcon((HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0), iconIndex, 0);;

				free(proto);

				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuContact[i], (LPARAM)&mi);
				DestroyIcon(mi.hIcon);
				//CallService(MS_SKIN2_RELEASEICON, (WPARAM)mi.hIcon, 0);
			} else {
				mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuContact[i], (LPARAM)&mi);
			}
		}

		// show hide nudge menu item
#define MS_NUDGE_SHOWMENU	"NudgeShowMenu"
// wParam = char *szProto
// lParam = BOOL show
		{
			char serviceFunc[256];
			hContact = Meta_GetMostOnline((HANDLE)wParam);
			mir_snprintf(serviceFunc, 256, "%s/SendNudge", (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0));
			CallService(MS_NUDGE_SHOWMENU, (WPARAM)META_PROTO, (LPARAM)ServiceExists(serviceFunc));
		}
	}
	else
	{// This is a simple contact
		if(!Meta_IsEnabled())
		{
			// groups disabled - all meta menu options hidden
			mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDefault, (LPARAM)&mi);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDelete, (LPARAM)&mi);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuAdd, (LPARAM)&mi);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuConvert, (LPARAM)&mi);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuEdit, (LPARAM)&mi);
			for(i = 0; i < MAX_CONTACTS; i++) {
				mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuContact[i], (LPARAM)&mi);
			}

		} else if(DBGetContactSettingDword((HANDLE)wParam,META_PROTO,META_LINK,(DWORD)-1)!=(DWORD)-1) {
			// The contact is affected to a metacontact.
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDefault, (LPARAM)&mi);
			mi.flags |= CMIM_NAME;
			mi.pszName = (char *)Translate("Remove from MetaContact");
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDelete, (LPARAM)&mi);
			mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuAdd, (LPARAM)&mi);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuConvert, (LPARAM)&mi);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuEdit, (LPARAM)&mi);
			for(i = 0; i < MAX_CONTACTS; i++) {
				mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuContact[i], (LPARAM)&mi);
			}
		} else {
			// The contact is neutral
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuAdd, (LPARAM)&mi);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuConvert, (LPARAM)&mi);
			mi.flags |= CMIF_HIDDEN;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuEdit, (LPARAM)&mi);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDelete, (LPARAM)&mi);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDefault, (LPARAM)&mi);
			for(i = 0; i < MAX_CONTACTS; i++) {
				mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuContact[i], (LPARAM)&mi);
			}
		}
	}
	return 0;
}



