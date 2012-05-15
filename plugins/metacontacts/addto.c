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

/** @file addto.c 
*
* Functions for the <b>'Add To'</b> Dialog.
* Contains all the functions and all the structures
* needed to display and control the <b>'Add To'</b> Dialog.
*/
#include "metacontacts.h"

//! Holds information about a contact.
typedef struct {
	char *name;		//!< Name of the contact 
	char *proto;	//!< Protocol under which the contact has been registered
	HANDLE hUser;	//!< Identifier of the contact in the DB.
}USERINFO;

//! Holds information for the callback function.
typedef struct {
	USERINFO uInfo;	//!< Information about the contact to add
}ADDTO_INFO;

/** Adds all the metacontacts desired in the listview.
*
* Adds all the metacontacts present in the database in the list,
*
* @param list :			\c HANDLE to the list which will contain the columns.
* @param nb_contacts :	Number of loaded contacts.
* @param contacts :		A list of the contacts' identifiers
*
* @param id :			Reference to a list of the MetaContacts IDs loaded in the listview.
*						Since this list is resized, its address must be passed.
*
* @return				An integer specifying the number of rows added in the list.
*/
int FillList(HWND list, BOOL sort)
{
	DWORD metaID;
	HANDLE hMetaUser = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	int i=0;
	int index;
	//BOOL mbs = FALSE;

	while(hMetaUser)	// The DB is searched through, to get all the metacontacts
	{
		if((metaID=DBGetContactSettingDword(hMetaUser,META_PROTO,META_ID,(DWORD)-1))==(DWORD)-1)
		{
			// This isn't a MetaContact, go to the next
			hMetaUser = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hMetaUser,0);
			continue;
		}


		{
			// get contact display name from clist
			char *szCDN = (char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hMetaUser, 0);

			if(os_unicode_enabled) {
				wchar_t *swzCDN = (wchar_t *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hMetaUser, GCDNF_UNICODE),
					*swzContactDisplayName;

				// detect if the clist provided unicode display name by comparing with non-unicode
				if(szCDN && swzCDN && strncmp(szCDN, (char *)swzCDN, strlen(szCDN)) != 0) { 
					swzContactDisplayName = swzCDN;
				} else {
					// no? convert to unicode
					if(szCDN) {
						swzContactDisplayName = (wchar_t *) _alloca(sizeof(wchar_t) * (strlen(szCDN) + 1));
						MultiByteToWideChar(CP_ACP, 0, (char *) szCDN, -1, swzContactDisplayName, (int)strlen((char *)szCDN) + 1);
					} else {
						swzContactDisplayName = TranslateW(L"(Unknown Contact)");
					}
				}				

				// don't insert huge strings that we have to compare with later
				if(wcslen(swzContactDisplayName) > 1023)
					swzContactDisplayName[1024] = 0;

				if(sort) {
					int j;
					wchar_t buff[1024];
					for(j = 0; j < i; j++) {
						SendMessageW(list, LB_GETTEXT, j, (LPARAM)buff);
						if(wcscmp(buff, swzContactDisplayName) > 0) break;
					}
					index = SendMessageW(list, LB_INSERTSTRING, (WPARAM)j, (LPARAM)swzContactDisplayName);
				} else {
					index = SendMessageW(list, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)swzContactDisplayName);
				}
			} else {
				// don't insert huge strings that we have to compare with later
				if(strlen(szCDN) > 1023)
					szCDN[1024] = 0;

				if(sort) {
					int j;
					char buff[1024];
					for(j = 0; j < i; j++) {
						SendMessage(list, LB_GETTEXT, j, (LPARAM)buff);
						if(strcmp(buff, szCDN) > 0) break;
					}
					index = SendMessage(list, LB_INSERTSTRING, (WPARAM)j, (LPARAM)szCDN);
				} else {
					index = SendMessage(list, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)szCDN);
				}
			}

			SendMessage(list, LB_SETITEMDATA, (WPARAM)index, (LPARAM)hMetaUser);
		}

		i++;

		hMetaUser = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hMetaUser,0);
	}
	return i;
}

/** Build or update the list.
*
* @param list :				\c HANDLE to the list which will contain the columns
* @param id :				Reference to a list that will contain all the MetaContacts IDs loaded in the listview
*							otherwise the list is only refilled \n (Warning : this value must be
*							set to \c TRUE only once per Dialog display, otherwise all columns will be doubled)
*
* @returns					An integer specifying the number of rows inserted or \c -1 if there was a problem
*/
int BuildList(HWND list, BOOL sort)
{
	int ret=-1;
	SendMessage(list, LB_RESETCONTENT, 0, 0);

	ret = FillList(list, sort);
	
    return ret;
}

/** Callback function for the <b>'Add To'</b> Dialog.
*
* All the UI is controlled here, from display to functionnalities.
*
* @param hwndDlg :	\c HANDLE to the <b>'Add To'</b> \c Dialog.
* @param uMsg :		Specifies the message received by this dialog.
* @param wParam :	Specifies additional message-specific information. 
* @param lParam :	Specifies additional message-specific information. 
*
* @return			\c TRUE if the dialog processed the message, \c FALSE if it did not.
*/
INT_PTR CALLBACK Meta_SelectDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{	
			TranslateDialogDefault( hwndDlg );


			if(DBGetContactSettingDword((HANDLE)lParam,META_PROTO,META_ID,(DWORD)-1)!=(DWORD)-1)
			{
				MessageBox(hwndDlg,Translate("This contact is a MetaContact.\nYou can't add a MetaContact to another MetaContact.\n\nPlease choose another."),
					Translate("MetaContact Conflict"),MB_ICONERROR);
				DestroyWindow(hwndDlg);
				return TRUE;
			}
			if(DBGetContactSettingDword((HANDLE)lParam,META_PROTO,META_LINK,(DWORD)-1)!=(DWORD)-1)
			{
				MessageBox(hwndDlg,Translate("This contact is already associated to a MetaContact.\nYou cannot add a contact to multiple MetaContacts."),
					Translate("Multiple MetaContacts"),MB_ICONERROR);
				DestroyWindow(hwndDlg);
				return TRUE;
			}

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam); // user data is contact handle

			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIconEx(I_ADD));
			//SendMessage(GetDlgItem(hwndDlg,IDC_METALIST),LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT);

			// Initialize the graphical part
			CheckDlgButton(hwndDlg,IDC_ONLYAVAIL,BST_CHECKED);	// Initially checked; display all metacontacts is only an option
																// Besides, we can check if there is at least one metacontact to add the contact to.
			if(BuildList(GetDlgItem(hwndDlg,IDC_METALIST), FALSE)<=0)
			{
				if(MessageBox(hwndDlg,Translate("Either there is no MetaContact in the database (in this case you should first convert a contact into one)\n"
					"or there is none that can host this contact.\n"
					"Another solution could be to convert this contact into a new MetaContact.\n\nConvert this contact into a new MetaContact?"),
					Translate("No suitable MetaContact found"),MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1)==IDYES)
					Meta_Convert((WPARAM)lParam,0);
				DestroyWindow(hwndDlg);
				return TRUE;
			}
			else
			{
				// Get the name displayed in the CList...

				// get contact display name from clist
				char *szCDN = (char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)lParam, 0);
				if(os_unicode_enabled) {
					wchar_t *swzCDN = (wchar_t *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)lParam, GCDNF_UNICODE),
						*swzContactDisplayName;
					wchar_t buf[256];

					// detect if the clist provided unicode display name by comparing with non-unicode
					if(szCDN && swzCDN && strncmp(szCDN, (char *)swzCDN, strlen(szCDN)) != 0) { 
						swzContactDisplayName = swzCDN;
					} else {
						// no? convert to unicode
						if(szCDN) {
							swzContactDisplayName = (wchar_t *) _alloca(sizeof(wchar_t) * (strlen(szCDN) + 1));
							MultiByteToWideChar(CP_ACP, 0, (char *) szCDN, -1, swzContactDisplayName, (int)strlen((char *)szCDN) + 1);
						} else {
							swzContactDisplayName = TranslateW(L"a contact");
						}
					}				

					// ... and set it to the Window title.
					//MessageBoxW(hwndDlg, swzContactDisplayName, L"Setting window title", MB_OK);

					// note - the swprintf function has changed (includes size_t for vc8+)
					//swprintf(buf, 256, TranslateW(L"Adding %s..."), swzContactDisplayName);
					// this *should* work for vc6, 7, and 8 (thx George)
					_snwprintf(buf, 256, TranslateW(L"Adding %s..."), swzContactDisplayName);

					SetWindowTextW(hwndDlg, buf);
				} else {
					char buf[256];
					sprintf(buf,Translate("Adding %s..."), szCDN);
					SetWindowText(hwndDlg, buf);
				}

				ShowWindow(hwndDlg,SW_SHOWNORMAL);
				return TRUE;
			}
		}
		case WM_COMMAND:
			if(HIWORD(wParam)!=BN_CLICKED)
				break;	// Only clicks of buttons are relevant, let other COMMANDs through
			switch(LOWORD(wParam))
			{
				case IDOK:
				{
					HANDLE hMeta, hContact = (HANDLE)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					int item;	// Get the index of the selected metacontact
					if((item = SendMessage(GetDlgItem(hwndDlg, IDC_METALIST),LB_GETCURSEL, 0, 0))==-1)
						return IDOK == MessageBox(hwndDlg,Translate("Please select a MetaContact"),Translate("No MetaContact selected"),MB_ICONHAND);
					
					hMeta = (HANDLE)SendMessage(GetDlgItem(hwndDlg, IDC_METALIST), LB_GETITEMDATA, (WPARAM)item, 0);

					{
						if(!Meta_Assign(hContact,hMeta, FALSE))
						{
							MessageBox(hwndDlg, Translate("Assignment to the MetaContact failed."), Translate("Assignment failure"),MB_ICONERROR);
						}
					}
				}
				case IDCANCEL:
					DestroyWindow(hwndDlg);
					break;

				case IDC_CHK_SRT:
					SetWindowLong(GetDlgItem(hwndDlg, IDC_METALIST), GWL_STYLE, GetWindowLong(GetDlgItem(hwndDlg, IDC_METALIST), GWL_STYLE) ^ LBS_SORT);
					if(BuildList(GetDlgItem(hwndDlg,IDC_METALIST), IsDlgButtonChecked(hwndDlg, IDC_CHK_SRT) ? TRUE : FALSE)<=0)
					{
						if(MessageBox(hwndDlg,Translate("Either there is no MetaContact in the database (in this case you should first convert a contact into one)\n"
							"or there is none that can host this contact.\n"
							"Another solution could be to convert this contact into a new MetaContact.\n\nConvert this contact into a new MetaContact?"),
							Translate("No suitable MetaContact found"),MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1)==IDYES)
							Meta_Convert((WPARAM)lParam,0);
						DestroyWindow(hwndDlg);
						return TRUE;
					}
					break;
			}
			break;
		case WM_DESTROY:
		{	// Free all allocated memory and return the focus to the CList
			HWND clist = GetParent(hwndDlg);
			ReleaseIconEx((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
			EndDialog(hwndDlg,TRUE);
			SetFocus(clist);
			return TRUE;
		}
	}
	return FALSE;	// All other Message are not handled
}
