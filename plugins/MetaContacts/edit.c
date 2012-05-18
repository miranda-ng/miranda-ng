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

/** @file edit.c 
*
* Functions for the <b>'Edit'</b> Dialog.
* Contains all the functions and all the structures
* needed to display and control the <b>'Edit'</b> Dialog.
*/
#include "metacontacts.h"

//! Holds the differents changes that have to made
typedef struct tag_CHANGES {
	HANDLE hMeta;							//!< \c HANDLE of the MetaContact that is edited.
	HANDLE hDefaultContact;					//!< \c HANDLE of the new default contact
	HANDLE hOfflineContact;
	int num_deleted,						//!< \c DWORD number of deleted contacts
		num_contacts;						//!< \c DWORD number of contacts
	HANDLE hDeletedContacts[MAX_CONTACTS];	//!< \c HANDLEs of the subcontacts to be removed from this metacontact
	HANDLE hContact[MAX_CONTACTS];			//!< \c HANDLEs of the subcontacts, in the order they should be in
	int force_default;
} CHANGES;

CHANGES changes;							//!< \c global CHANGES structure

/** Fills the list of contacts
*
* @param chg : Structure holding all the change info (See CHANGES).
*/
void FillContactList(HWND hWndDlg, CHANGES *chg) {
	HWND hList = GetDlgItem(hWndDlg, IDC_LST_CONTACTS);
	char *proto, *field, buff[256];
	int i;
	LVITEM LvItem;
	DBVARIANT dbv;
	LVITEMW LvItemW; // for unicode nicks


	SendMessage(hList,LVM_DELETEALLITEMS,0,0);

	ZeroMemory(&LvItem, sizeof(LvItem));
	LvItem.mask=LVIF_TEXT;   // Text Style
	LvItem.cchTextMax = 256; // Max size of test

	ZeroMemory(&LvItemW, sizeof(LvItemW));
	LvItemW.mask=LVIF_TEXT;   // Text Style
	LvItemW.cchTextMax = 256; // Max size of test

	for(i = 0; i < chg->num_contacts; i++)  {
		LvItem.iItem = i;
		LvItemW.iItem = i;
		
		{

			char *szCDN = (char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)chg->hContact[i], 0);

			if(os_unicode_enabled) {
				wchar_t *swzCDN = (wchar_t *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)chg->hContact[i], GCDNF_UNICODE),
					*swzContactDisplayName;

				LvItemW.iSubItem = 0; // clist display name

				// detect if the clist provided unicode display name by comparing with non-unicode
				if(szCDN && swzCDN && strncmp(szCDN, (char *)swzCDN, strlen(szCDN)) != 0 && wcslen(swzCDN) >= strlen(szCDN)) { 
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

				LvItemW.pszText = swzContactDisplayName;
				SendMessageW(hList, LVM_INSERTITEMW, (WPARAM)0, (LPARAM)&LvItemW);
			} else {
				LvItem.iSubItem = 0; // clist display name
				LvItem.pszText = szCDN;
				SendMessage(hList, LVM_INSERTITEM, (WPARAM)0, (LPARAM)&LvItem);
			}
		}
		


		LvItem.iSubItem = 1; // id

		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)chg->hContact[i], 0);
		if(proto) {
			field = (char *)CallProtoService(proto,PS_GETCAPS,PFLAG_UNIQUEIDSETTING,0);

			DBGetContactSetting(chg->hContact[i],proto,field,&dbv);
			switch(dbv.type)
			{
				case DBVT_ASCIIZ:
					sprintf(buff,"%s",dbv.pszVal);
					break;
				case DBVT_BYTE:
					sprintf(buff,"%d",dbv.bVal);
					break;
				case DBVT_WORD:
					sprintf(buff,"%d",dbv.wVal);
					break;
				case DBVT_DWORD:
					sprintf(buff,"%d",(int)dbv.dVal);
					break;
				default:
					//sprintf(buff,"");
					buff[0] = 0;
			}
			DBFreeVariant(&dbv);

			LvItem.pszText = buff;
			SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

			LvItem.iSubItem = 2; // protocol
			LvItem.pszText = proto;
			SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems
		} else {
			LvItem.pszText = "Unknown";
			SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

			LvItem.iSubItem = 2; // protocol
			LvItem.pszText = "Unknown";
			SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems
		}
		LvItem.iSubItem = 3; // Default (Yes/No)
		LvItem.pszText = (chg->hContact[i] == chg->hDefaultContact ? Translate("Yes") : Translate("No"));
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iSubItem = 4; // Offline (Yes/No)
		LvItem.pszText = (chg->hContact[i] == chg->hOfflineContact ? Translate("Yes") : Translate("No"));
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems
	}
}


void SetListSelection(HWND hList, int sel) {
	LVITEM LvItem;

	ZeroMemory(&LvItem, sizeof(LvItem));
	LvItem.iItem = sel;
	LvItem.mask = LVIF_STATE;
	LvItem.stateMask = LVIS_SELECTED|LVIS_FOCUSED;
	LvItem.state = LVIS_SELECTED|LVIS_FOCUSED;

	SendMessage(hList, LVM_SETITEMSTATE, (WPARAM)sel, (LPARAM)&LvItem);

}

/** Scans the \c CHANGES and call the appropriate function for each change.
*
* @param chg : Structure holding all the change info (See CHANGES).
*/
void ApplyChanges(CHANGES *chg)
{
	HANDLE most_online;
	int i;

	// remove removed contacts
	for(i = 0; i < chg->num_deleted; i++) {
		Meta_Delete((WPARAM)chg->hDeletedContacts[i], 0);
		if(chg->hDeletedContacts[i] == chg->hDefaultContact)
			chg->hDefaultContact = 0;
		if(chg->hDeletedContacts[i] == chg->hOfflineContact)
			chg->hOfflineContact = 0;
	}

	// set contact positions
	for(i = 0; i < chg->num_contacts; i++) {
		if(Meta_GetContactNumber(chg->hContact[i]) != i)
			Meta_SwapContacts(chg->hMeta, Meta_GetContactNumber(chg->hContact[i]), i);
	}

	NotifyEventHooks(hSubcontactsChanged, (WPARAM)chg->hMeta, (LPARAM)chg->hDefaultContact);

	// set default
	if(chg->hDefaultContact)
		DBWriteContactSettingDword(chg->hMeta, META_PROTO, "Default", Meta_GetContactNumber(chg->hDefaultContact));
	else
		DBWriteContactSettingDword(chg->hMeta, META_PROTO, "Default", 0);
	NotifyEventHooks(hEventDefaultChanged, (WPARAM)chg->hMeta, (LPARAM)chg->hDefaultContact);

	// set offline
	if(chg->hOfflineContact)
		DBWriteContactSettingDword(chg->hMeta, META_PROTO, "OfflineSend", Meta_GetContactNumber(chg->hOfflineContact));
	else
		DBWriteContactSettingDword(chg->hMeta, META_PROTO, "OfflineSend", (DWORD)-1);

	// fix nick
	most_online = Meta_GetMostOnline(chg->hMeta);
	Meta_CopyContactNick(chg->hMeta, most_online);

	// fix status
	Meta_FixStatus(chg->hMeta);

	// fix avatar
	most_online = Meta_GetMostOnlineSupporting(chg->hMeta, PFLAGNUM_4, PF4_AVATARS);
	if(most_online) {
		PROTO_AVATAR_INFORMATION AI;

		AI.cbSize = sizeof(AI);
		AI.hContact = chg->hMeta;
		AI.format = PA_FORMAT_UNKNOWN;
		strcpy(AI.filename, "X");

		if((int)CallProtoService(META_PROTO, PS_GETAVATARINFO, 0, (LPARAM)&AI) == GAIR_SUCCESS)
	        DBWriteContactSettingString(chg->hMeta, "ContactPhoto", "File",AI.filename);
	}

	if(MetaAPI_GetForceState((WPARAM)chg->hMeta, 0) != chg->force_default)
		MetaAPI_ForceDefault((WPARAM)chg->hMeta, 0);
}

LRESULT ProcessCustomDraw (LPARAM lParam)
{
    LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;

    switch(lplvcd->nmcd.dwDrawStage) 
    {
        case CDDS_PREPAINT : //Before the paint cycle begins
            //request notifications for individual listview items
            return CDRF_NOTIFYITEMDRAW;
            
        case CDDS_ITEMPREPAINT: //Before an item is drawn
			/*
            if (((int)lplvcd->nmcd.dwItemSpec%2)==0)
            {
                //customize item appearance
                //lplvcd->clrText   = RGB(255,0,0);
                lplvcd->clrTextBk = RGB(200,200,200);
            }
            else{
                //lplvcd->clrText   = RGB(0,0,255);
                lplvcd->clrTextBk = RGB(255,255,255);
            }
			*/
			if(changes.hContact[(int)lplvcd->nmcd.dwItemSpec] == changes.hDefaultContact) {
                lplvcd->clrText = RGB(255, 0, 0);
			}
            return CDRF_NEWFONT;
	}

	return 0;
}

/** Callback function for the <b>'Edit'</b> Dialog.
*
* All the UI is controlled here, from display to functionnalities.
*
* @param hwndDlg :	\c HANDLE to the <b>'Edit'</b> \c Dialog.
* @param uMsg :		Specifies the message received by this dialog.
* @param wParam :	Specifies additional message-specific information. 
* @param lParam :	Specifies additional message-specific information (handle of MetaContact to edit)
*
* @return			\c TRUE if the dialog processed the message, \c FALSE if it did not.
*/
#define WMU_SETTITLE		(WM_USER + 1)

INT_PTR CALLBACK Meta_EditDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwnd;
	char *str;
	int sel, i;

	switch(msg)
	{
		case WM_INITDIALOG:
		{	
			// Font necessary for all controls created with CreateWindowsEx
			//HFONT hfDefault = GetStockObject(DEFAULT_GUI_FONT);
			//HWND combo = GetDlgItem(hwndDlg,IDC_DEFAULT);
			int nb_contacts, default_contact_number, offline_contact_number;
			LVCOLUMN LvCol;

			TranslateDialogDefault( hwndDlg );

			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIconEx(I_EDIT));

			// Disable the 'Apply' button.
			EnableWindow(GetDlgItem(hwndDlg,IDC_VALIDATE),FALSE);

			// (from http://www.codeproject.com/listctrl/listview.asp)
			// initialize list
			hwnd = GetDlgItem(hwndDlg, IDC_LST_CONTACTS);
			SendMessage(hwnd,LVM_SETEXTENDEDLISTVIEWSTYLE, 0,LVS_EX_FULLROWSELECT); // Set style

			// Create list columns
			ZeroMemory(&LvCol, sizeof(LvCol));
			LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;    // Type of mask

			// Inserting Couloms as much as we want
			LvCol.pszText=Translate("Contact");              // First Header Text
			LvCol.cx=100;                                   // width of column
			SendMessage(hwnd,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol); // Insert/Show the coloum

			LvCol.pszText=Translate("Id");                            // Next coloum
			LvCol.cx=130;                                   // width of column
			SendMessage(hwnd,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol); // ...
			LvCol.pszText=Translate("Protocol");                            //
			LvCol.cx=100;                                   // width of column
			SendMessage(hwnd,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol); //
			LvCol.pszText=Translate("Default");                            //
			LvCol.cx=60;                                   // width of column
			SendMessage(hwnd,LVM_INSERTCOLUMN,3,(LPARAM)&LvCol); //
			LvCol.pszText=Translate("Send Offline");                            //
			LvCol.cx=85;                                   // width of column
			SendMessage(hwnd,LVM_INSERTCOLUMN,4,(LPARAM)&LvCol); //
	
			// disable buttons until a selection is made in the list
			hwnd = GetDlgItem(hwndDlg, IDC_BTN_REM);
			EnableWindow(hwnd, FALSE);
			hwnd = GetDlgItem(hwndDlg, IDC_BTN_SETDEFAULT);
			EnableWindow(hwnd, FALSE);
			hwnd = GetDlgItem(hwndDlg, IDC_BTN_SETOFFLINE);
			EnableWindow(hwnd, FALSE);
			hwnd = GetDlgItem(hwndDlg, IDC_BTN_UP);
			EnableWindow(hwnd, FALSE);
			hwnd = GetDlgItem(hwndDlg, IDC_BTN_DOWN);
			EnableWindow(hwnd, FALSE);

			nb_contacts = DBGetContactSettingDword((HANDLE)lParam, META_PROTO, "NumContacts", 0);
			default_contact_number = DBGetContactSettingDword((HANDLE)lParam, META_PROTO, "Default", (DWORD)-1);
			offline_contact_number = DBGetContactSettingDword((HANDLE)lParam, META_PROTO, "OfflineSend", (DWORD)-1);

			changes.hMeta = (HANDLE)lParam;
			changes.num_contacts = nb_contacts;
			changes.num_deleted = 0;
			changes.hDefaultContact = Meta_GetContactHandle((HANDLE)lParam, default_contact_number);
			changes.hOfflineContact = Meta_GetContactHandle((HANDLE)lParam, offline_contact_number);
			for(i = 0; i < nb_contacts; i++)
				changes.hContact[i] = Meta_GetContactHandle((HANDLE)lParam, i);
			changes.force_default = MetaAPI_GetForceState((WPARAM)lParam, 0);

			SendMessage(hwndDlg, WMU_SETTITLE, 0, lParam);

			CheckDlgButton(hwndDlg, IDC_CHK_FORCEDEFAULT, changes.force_default);

			FillContactList(hwndDlg, &changes);
			return TRUE;
		}
		case WMU_SETTITLE:
			{

				char *szCDN = (char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)lParam, 0);
				if(os_unicode_enabled) {
					wchar_t *swzCDN = (wchar_t *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)lParam, GCDNF_UNICODE),
						*swzContactDisplayName;

					// detect if the clist provided unicode display name by comparing with non-unicode
					if(szCDN && swzCDN && strncmp(szCDN, (char *)swzCDN, strlen(szCDN)) != 0 && wcslen(swzCDN) >= strlen(szCDN)) { 
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

					SetWindowTextW(GetDlgItem(hwndDlg,IDC_ED_NAME), swzContactDisplayName);
				} else {
					SetWindowText(GetDlgItem(hwndDlg,IDC_ED_NAME), szCDN);
				}
			}
			return TRUE;
		case WM_NOTIFY: // the message that is being sent always
			switch(LOWORD(wParam)) // hit control
			{
				case IDC_LST_CONTACTS:      // did we hit our ListView contorl?
					if(((LPNMHDR)lParam)->code == NM_CLICK) {
						hwnd = GetDlgItem(hwndDlg, IDC_LST_CONTACTS);
						sel=SendMessage(hwnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED|LVNI_SELECTED); // return item selected
						// enable buttons
						hwnd = GetDlgItem(hwndDlg, IDC_BTN_REM);
						EnableWindow(hwnd, sel!=-1);
						hwnd = GetDlgItem(hwndDlg, IDC_BTN_SETDEFAULT);
						EnableWindow(hwnd, sel!=-1 && changes.hContact[sel] != changes.hDefaultContact);
						hwnd = GetDlgItem(hwndDlg, IDC_BTN_SETOFFLINE);
						EnableWindow(hwnd, sel!=-1 && changes.hContact[sel] != changes.hOfflineContact);
						hwnd = GetDlgItem(hwndDlg, IDC_BTN_UP);
						EnableWindow(hwnd, (sel > 0));
						hwnd = GetDlgItem(hwndDlg, IDC_BTN_DOWN);
						EnableWindow(hwnd, (sel != -1) && (sel < changes.num_contacts - 1));
/*
					// custom draw stuff - change colour of listview things - doesn't affect selection :(
					} else if(((LPNMHDR)lParam)->code == NM_CUSTOMDRAW) {
						SetWindowLong(hwndDlg, DWL_MSGRESULT, (LONG)ProcessCustomDraw(lParam));
						return TRUE;					
*/
					}					
					break;
			}
			break;
		case WM_COMMAND:
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:	// A button ('Remove', 'Ok', 'Cancel' or 'Apply', normally) has been clicked
					switch(LOWORD(wParam))
					{
						case IDC_VALIDATE:			// Apply changes, if there is still one contact attached to the metacontact.
							if(changes.num_contacts == 0)	// Otherwise, delete the metacontact.
							{
								if(MessageBox(hwndDlg,Translate("You are going to remove all the contacts associated with this MetaContact.\nThis will delete the MetaContact.\n\nProceed Anyway?"),
									Translate("Delete MetaContact?"),MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1)!=IDYES)
									return TRUE;
								else
								{
									Meta_Delete((WPARAM)changes.hMeta,(LPARAM)NULL);
									DestroyWindow(hwndDlg);
									return TRUE;
								}
							}
							ApplyChanges(&changes);

							// Disable the 'Apply' button.
							EnableWindow(GetDlgItem(hwndDlg,IDC_VALIDATE),FALSE);
							break;
						case IDOK:
							if(IsWindowEnabled(GetDlgItem(hwndDlg,IDC_VALIDATE)))
							{							// If there are changes that could be made,
								if(changes.num_contacts == 0)	// do the work that would have be done if
								{						// the button 'Apply' has been clicked.
									if(MessageBox(hwndDlg,Translate("You are going to remove all the contacts associated with this MetaContact.\nThis will delete the MetaContact.\n\nProceed Anyway?"),
										Translate("Delete MetaContact?"),MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1)!=IDYES)
									{
										return TRUE;
									} else {
										Meta_Delete((WPARAM)changes.hMeta,(LPARAM)NULL);
										DestroyWindow(hwndDlg);
										return TRUE;
									}
								}
								ApplyChanges(&changes);
							}
							EndDialog(hwndDlg, IDOK);
							return TRUE;
						case IDCANCEL:	// Simply close the dialog
							EndDialog(hwndDlg, IDCANCEL);
							return TRUE;
						case IDC_BTN_SETDEFAULT:
							hwnd = GetDlgItem(hwndDlg, IDC_LST_CONTACTS);
							sel=SendMessage(hwnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED|LVNI_SELECTED); // return item selected
							InvalidateRect(hwnd, 0, TRUE);
							changes.hDefaultContact = changes.hContact[sel];
							SendMessage(hwndDlg, WMU_SETTITLE, 0, (LPARAM)changes.hContact[sel]);

							FillContactList(hwndDlg, &changes);
							SetListSelection(hwnd, sel);
							// Disable set default button
							EnableWindow(GetDlgItem(hwndDlg,IDC_BTN_SETDEFAULT),FALSE);
							// Enable the 'Apply' button.
							EnableWindow(GetDlgItem(hwndDlg,IDC_VALIDATE),TRUE);

							// repaint list
							return TRUE;
						case IDC_BTN_SETOFFLINE:
							hwnd = GetDlgItem(hwndDlg, IDC_LST_CONTACTS);
							sel=SendMessage(hwnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED|LVNI_SELECTED); // return item selected
							InvalidateRect(hwnd, 0, TRUE);
							changes.hOfflineContact = changes.hContact[sel];

							FillContactList(hwndDlg, &changes);
							SetListSelection(hwnd, sel);
							// Disable set offline button
							EnableWindow(GetDlgItem(hwndDlg,IDC_BTN_SETOFFLINE),FALSE);
							// Enable the 'Apply' button.
							EnableWindow(GetDlgItem(hwndDlg,IDC_VALIDATE),TRUE);

							// repaint list
							return TRUE;
						case IDC_BTN_REM:
							hwnd = GetDlgItem(hwndDlg, IDC_LST_CONTACTS);
							sel=SendMessage(hwnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED|LVNI_SELECTED); // return item selected
							changes.num_contacts--;
							changes.hDeletedContacts[changes.num_deleted++] = changes.hContact[sel];
							if(changes.hDefaultContact == changes.hContact[sel]) {
								if(changes.num_contacts > 0) {
									changes.hDefaultContact = changes.hContact[0];
									str = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)changes.hDefaultContact, 0);
									SetWindowText(GetDlgItem(hwndDlg,IDC_ED_DEFAULT),str);
								} else {
									changes.hDefaultContact = 0;
									SetWindowText(GetDlgItem(hwndDlg,IDC_ED_DEFAULT),"None");
								}

							}

							for(i = sel; i < changes.num_contacts; i++)
								changes.hContact[i] = changes.hContact[i + 1];
							FillContactList(hwndDlg, &changes);
							// disable buttons
							hwnd = GetDlgItem(hwndDlg, IDC_BTN_REM);
							EnableWindow(hwnd, FALSE);
							hwnd = GetDlgItem(hwndDlg, IDC_BTN_SETDEFAULT);
							EnableWindow(hwnd, FALSE);
							hwnd = GetDlgItem(hwndDlg, IDC_BTN_SETOFFLINE);
							EnableWindow(hwnd, FALSE);
							hwnd = GetDlgItem(hwndDlg, IDC_BTN_UP);
							EnableWindow(hwnd, FALSE);
							hwnd = GetDlgItem(hwndDlg, IDC_BTN_DOWN);
							EnableWindow(hwnd, FALSE);
							// Enable the 'Apply' button.
							EnableWindow(GetDlgItem(hwndDlg,IDC_VALIDATE),TRUE);
							return TRUE;
						case IDC_BTN_UP:
							hwnd = GetDlgItem(hwndDlg, IDC_LST_CONTACTS);
							sel=SendMessage(hwnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED|LVNI_SELECTED); // return item selected

							{
								HANDLE temp = changes.hContact[sel];
								changes.hContact[sel] = changes.hContact[sel - 1];
								changes.hContact[sel - 1] = temp;
							}
							FillContactList(hwndDlg, &changes);
							sel--;
							SetListSelection(hwnd, sel);

							hwnd = GetDlgItem(hwndDlg, IDC_BTN_UP);
							EnableWindow(hwnd, (sel > 0));
							hwnd = GetDlgItem(hwndDlg, IDC_BTN_DOWN);
							EnableWindow(hwnd, (sel < changes.num_contacts - 1));
							// Enable the 'Apply' button.
							EnableWindow(GetDlgItem(hwndDlg,IDC_VALIDATE),TRUE);
							return TRUE;
						case IDC_BTN_DOWN:
							hwnd = GetDlgItem(hwndDlg, IDC_LST_CONTACTS);
							sel=SendMessage(hwnd,LVM_GETNEXTITEM,-1,LVNI_FOCUSED|LVNI_SELECTED); // return item selected

							{
								HANDLE temp = changes.hContact[sel];
								changes.hContact[sel] = changes.hContact[sel + 1];
								changes.hContact[sel + 1] = temp;
							}
							FillContactList(hwndDlg, &changes);
							sel++;
							SetListSelection(hwnd, sel);

							hwnd = GetDlgItem(hwndDlg, IDC_BTN_UP);
							EnableWindow(hwnd, (sel > 0));
							hwnd = GetDlgItem(hwndDlg, IDC_BTN_DOWN);
							EnableWindow(hwnd, (sel < changes.num_contacts - 1));
							// Enable the 'Apply' button.
							EnableWindow(GetDlgItem(hwndDlg,IDC_VALIDATE),TRUE);
							return TRUE;
						case IDC_CHK_FORCEDEFAULT:
							changes.force_default = IsDlgButtonChecked(hwndDlg, IDC_CHK_FORCEDEFAULT);
							// Enable the 'Apply' button.
							EnableWindow(GetDlgItem(hwndDlg,IDC_VALIDATE),TRUE);
							return TRUE;
					}
			}
			break;
		case WM_CLOSE:
			DestroyWindow(hwndDlg);
			return TRUE;

		case WM_DESTROY:
			ReleaseIconEx((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
			EndDialog(hwndDlg, IDCANCEL);
			break;
	}

	return FALSE;
}
