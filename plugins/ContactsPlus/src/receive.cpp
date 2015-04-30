// --------------------------------------------------------------------------
//                Contacts+ for Miranda Instant Messenger
//                _______________________________________
// 
// Copyright © 2002 Dominus Procellarum 
// Copyright © 2004-2008 Joe Kucera
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// -----------------------------------------------------------------------------

#include "stdafx.h"

/* TRecvContactsData */

TRecvContactsData::TRecvContactsData(MCONTACT contact)
{
	mhContact = contact;
	hHook = NULL;
	cbReceived = 0;
	maReceived = NULL;
	haUin = NULL;
}

TRecvContactsData::~TRecvContactsData()
{
	if (cbReceived) {
		for (int i = 0; i < cbReceived; i++)
			delete maReceived[i];
		mir_free(maReceived);
		mir_free(haUin);
	}
}

TReceivedItem* TRecvContactsData::AddReceivedItem()
{
	int iItem = cbReceived;

	cbReceived++;
	maReceived = (TReceivedItem**)mir_realloc(maReceived, cbReceived*sizeof(TReceivedItem*));
	maReceived[iItem] = new TReceivedItem();

	return maReceived[iItem];
}


static int RecvDlg_Resize(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_CONTACTS:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
		break;
	case IDOK:
	case IDDETAILS:
	case IDC_ENABLEGROUPS:
	case IDC_GROUPS:
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;      
		break;
	case IDC_ADD:
	case IDC_HISTORY:
	case IDC_USERMENU:
	case IDC_DETAILS:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_TOP;
		break;
	case IDCANCEL:
		return RD_ANCHORY_BOTTOM | RD_ANCHORX_RIGHT;
		break;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP; // default
}

static TCHAR* ListView_GetItemTextEx(HWND hLV, int iItem, int iSubItem)
{
	LVITEM lvi = {0};
	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = iSubItem;
	lvi.cchTextMax = 64;
	lvi.pszText = (TCHAR*)mir_alloc(lvi.cchTextMax * sizeof(TCHAR));
	// loop until the returned size is smaller than buffer size
	while (SendMessage(hLV, LVM_GETITEMTEXT, iItem, (LPARAM)&lvi) == lvi.cchTextMax - 1) { 
		lvi.cchTextMax += 64;
		lvi.pszText = (TCHAR*)mir_realloc(lvi.pszText, lvi.cchTextMax * sizeof(TCHAR));
	}
	return lvi.pszText;
}

static void EnableGroupCombo(HWND hwndDlg)
{
	EnableDlgItem(hwndDlg, IDC_GROUPS, IsDlgButtonChecked(hwndDlg, IDC_ENABLEGROUPS));
}

static void RebuildGroupCombo(HWND hwndDlg)
{
	int bHasGroups = pcli->pfnGetGroupName(0, NULL) != 0;
	HWND hGroupsCombo = GetDlgItem(hwndDlg, IDC_GROUPS);

	if (bHasGroups) {
		int curs = SendMessage(hGroupsCombo, CB_GETCURSEL, 0, 0);
		TCHAR *curst = NULL;

		EnableDlgItem(hwndDlg, IDC_ENABLEGROUPS, TRUE);
		EnableGroupCombo(hwndDlg);

		if (curs != CB_ERR) {
			curst = (TCHAR*)_alloca((SendMessage(hGroupsCombo, CB_GETLBTEXTLEN, curs, 0) + 1) * sizeof(TCHAR));
			SendMessage(hGroupsCombo, CB_GETLBTEXT, curs, (LPARAM)curst);
		}
		SendMessage(hGroupsCombo, CB_RESETCONTENT, 0, 0);

		TCHAR *szGroup;
		for (int i=1; (szGroup = pcli->pfnGetGroupName(i, NULL)) != NULL; i++) {
			int nIndex = SendMessage(hGroupsCombo, CB_ADDSTRING, 0, (LPARAM)szGroup);
			SendMessage(hGroupsCombo, CB_SETITEMDATA, nIndex, i);
		}
		if (curs != CB_ERR) 
			SendMessage(hGroupsCombo, CB_SELECTSTRING, -1, (LPARAM)curst);
		else
			SendMessage(hGroupsCombo, CB_SETCURSEL, 0, 0);
	}
	else {
		// no groups available
		EnableDlgItem(hwndDlg, IDC_ENABLEGROUPS, FALSE);
		EnableDlgItem(hwndDlg, IDC_GROUPS, FALSE);
	}
}

static MCONTACT CreateTemporaryContactForItem(HWND hwndDlg, TRecvContactsData *wndData, int iItem)
{
	TCHAR *caUIN = ListView_GetItemTextEx(GetDlgItem(hwndDlg, IDC_CONTACTS), iItem, 0);
	char *szProto = GetContactProto(wndData->mhContact);
	wndData->rhSearch = (HANDLE)CallProtoService(szProto, PS_BASICSEARCH, 0, (LPARAM)caUIN); // find it
	replaceStrT(wndData->haUin, caUIN);
	for (int j = 0; j < wndData->cbReceived; j++)
		if (!mir_tstrcmp(wndData->maReceived[j]->mcaUIN, caUIN))
			return (MCONTACT)CallProtoService(szProto, PS_ADDTOLISTBYEVENT, MAKEWPARAM(PALF_TEMPORARY, j), (LPARAM)wndData->mhDbEvent);
	return NULL;
}

void RecvListView_AddColumn(HWND hList, int nWidth, TCHAR *szTitle, int nItem)
{
	LVCOLUMN col;
	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;
	col.cx = nWidth;
	col.pszText = szTitle;
	col.iSubItem = nItem;
	ListView_InsertColumn(hList, nItem, &col);
}

INT_PTR CALLBACK RecvDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TRecvContactsData *wndData = (TRecvContactsData*)GetWindowLongPtr(hwndDlg, DWLP_USER);
	MCONTACT hContact;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			CLISTEVENT *pcle = (CLISTEVENT*)lParam;
			WindowList_Add(ghRecvWindowList, hwndDlg, pcle->hContact);
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_CONTACTS)));
			EnableDlgItem(hwndDlg, IDOK, FALSE);
			EnableDlgItem(hwndDlg, IDDETAILS, FALSE);
			wndData = new TRecvContactsData(pcle->hContact);
			SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)wndData);
			wndData->mhDbEvent = pcle->hDbEvent; /// initialized, pcle not needed anymore
			wndData->mhListIcon = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLORDDB|ILC_MASK, 0, 1);
			wndData->mhPopup = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTACTMENU));
			TranslateMenu(wndData->mhPopup);
			wndData->hHook = HookEventMessage(ME_PROTO_ACK, hwndDlg, HM_EVENTSENT);

			char *szProto =GetContactProto(wndData->mhContact);

			HWND hLV = GetDlgItem(hwndDlg, IDC_CONTACTS);
			ListView_SetExtendedListViewStyle(hLV, LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT);
			// add columns
			RecvListView_AddColumn(hLV, 120, _A2T((char*)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDTEXT, 0)), 0);
			RecvListView_AddColumn(hLV, 100, TranslateT("Nick"),       1);
			RecvListView_AddColumn(hLV, 100, TranslateT("First Name"), 2);
			RecvListView_AddColumn(hLV, 100, TranslateT("Last Name"),  3);

			// fill in groups
			CheckDlgButton(hwndDlg, IDC_ENABLEGROUPS, BST_UNCHECKED);
			RebuildGroupCombo(hwndDlg);

			{ // fill listview with received contacts
				DBEVENTINFO dbe = {0};

				dbe.cbSize = sizeof(DBEVENTINFO);
				dbe.cbBlob = db_event_getBlobSize(wndData->mhDbEvent);
				if (dbe.cbBlob != -1)  // this marks an invalid hDbEvent - all smashed anyway...
					dbe.pBlob = (PBYTE)_alloca(dbe.cbBlob);
				db_event_get(wndData->mhDbEvent, &dbe);
				char* pcBlob = (char*)dbe.pBlob;
				char* pcEnd = (char*)dbe.pBlob + dbe.cbBlob;

				HICON hiProto = LoadContactProtoIcon(wndData->mhContact);
				ImageList_AddIcon(wndData->mhListIcon, hiProto);
				DestroyIcon(hiProto); // imagelist copied the resource
				ListView_SetImageList(hLV, wndData->mhListIcon, LVSIL_SMALL);
				LVITEM lvi = {0};
				lvi.iImage = 0;
				lvi.mask = LVIF_TEXT | LVIF_IMAGE;

				for (int nItem = 0; ; nItem++)
				{ // Nick
					int strsize = (int)strlennull(pcBlob);
					TReceivedItem* pItem = wndData->AddReceivedItem();

					if (dbe.flags & DBEF_UTF)
						pItem->mcaNick = mir_utf8decodeT(pcBlob); 
					else
						pItem->mcaNick = mir_a2t(pcBlob);
					pcBlob += strsize + 1;
					// UIN
					strsize = (int)strlennull(pcBlob);
					pItem->mcaUIN = mir_a2t(pcBlob);
					pcBlob += strsize + 1;
					// add to listview
					lvi.iItem = nItem;
					lvi.pszText = pItem->mcaUIN;
					ListView_InsertItem(hLV, &lvi); // with image
					ListView_SetItemText(hLV, nItem, 1, pItem->mcaNick);
					// check for end of contacts
					if (pcBlob >= pcEnd)
						break;
				}
			}
			// new dlg init
			wndData->hIcons[0] = InitMButton(hwndDlg, IDC_ADD, MAKEINTRESOURCEA(IDI_ADDCONTACT), LPGEN("Add Contact Permanently to List"));
			wndData->hIcons[1] = InitMButton(hwndDlg, IDC_DETAILS, MAKEINTRESOURCEA(IDI_USERDETAILS), LPGEN("View User's Details"));
			wndData->hIcons[2] = InitMButton(hwndDlg, IDC_HISTORY, MAKEINTRESOURCEA(IDI_HISTORY), LPGEN("View User's History"));
			wndData->hIcons[3] = InitMButton(hwndDlg, IDC_USERMENU, MAKEINTRESOURCEA(IDI_DOWNARROW), LPGEN("User Menu"));

			SendMessage(hwndDlg,DM_UPDATETITLE,0,0);
			// new dialog init done
			Utils_RestoreWindowPosition(hwndDlg, NULL, MODULENAME, "");
			return TRUE;
		}
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_CONTACTS) {
			switch (((LPNMHDR)lParam)->code) {
			case NM_DBLCLK:
				{
					HWND hLV = GetDlgItem(hwndDlg, IDC_CONTACTS);
					if (ListView_GetSelectedCount(hLV) != 1) break; // if not clicking on item, bye
					wndData->iPopupItem = ListView_GetNextItem(hLV, -1, LVNI_ALL|LVNI_SELECTED);
					if (wndData->iPopupItem == -1) break; // if no item selected no user details available
					return SendMessage(hwndDlg, WM_COMMAND, ID_POPUP_USERDETAILS, 0); // show user details
				}
			case LVN_ITEMCHANGED:
				{
					LPNMLISTVIEW pNMLV = (LPNMLISTVIEW)lParam;
					HWND hLV = GetDlgItem(hwndDlg, IDC_CONTACTS); // optimisation, for FOR-Cycle
					bool bExistsCheckedItem = false;     // there must be no checked items to disable "Add" button

					if (ListView_GetCheckState(hLV, pNMLV->iItem))
					{ // the user has checked this item
						bExistsCheckedItem = true; // enable "Add" button
					}
					else
					{ // loop thru items and check if at least one is checked
						for (int i = 0; i < ListView_GetItemCount(hLV); i++)
							if (ListView_GetCheckState(hLV, i))
							{  // we found checked item, enable add, exit loop
								bExistsCheckedItem = true;
								break;
							}
					}
					EnableDlgItem(hwndDlg, IDOK, bExistsCheckedItem);
					EnableDlgItem(hwndDlg, IDDETAILS, ListView_GetSelectedCount(hLV) > 0);
					break;
				}
			}
		}
		break;

	case WM_COMMAND:
		{
			if (!lParam && CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)wndData->mhContact))
				break;

			switch(LOWORD(wParam))
			{
			case IDOK:  // "Add Selected" button click
				{ // for each selected item, find its index in the hDbEvent
					// and after that add this item to the DB permanently
					HWND hLV = GetDlgItem(hwndDlg, IDC_CONTACTS);
					HWND hGroupsCombo = GetDlgItem(hwndDlg, IDC_GROUPS);
					HWND hGroupsCheck = GetDlgItem(hwndDlg, IDC_ENABLEGROUPS);
					int curs = SendMessage(hGroupsCombo, CB_GETCURSEL, 0, 0);
					TCHAR* caGroup = NULL;
					int nGroupId = -1;
					if (curs != CB_ERR && IsWindowEnabled(hGroupsCheck) && SendMessage(hGroupsCheck, BM_GETCHECK, 0, 0))
					{ //got groups, get the one selected in combo
						TCHAR* caGroup = (TCHAR*)_alloca((SendMessage(hGroupsCombo, CB_GETLBTEXTLEN, curs, 0) + 1) * sizeof(TCHAR));
						SendMessage(hGroupsCombo, CB_GETLBTEXT, curs, (LPARAM)caGroup);
						nGroupId = SendMessage(hGroupsCombo, CB_GETITEMDATA, curs, 0);
					}

					for (int i = 0; i < ListView_GetItemCount(hLV); i++)
						if (ListView_GetCheckState(hLV, i)) {
							// found checked contact item, add it
							TCHAR *caUIN = ListView_GetItemTextEx(hLV, i, 0);
							for (int j = 0; j < wndData->cbReceived; j++)   // determine item index in packet
								if (!mir_tstrcmp(wndData->maReceived[j]->mcaUIN, caUIN)) {
									char *szProto =GetContactProto(wndData->mhContact);
									hContact = (MCONTACT)CallProtoService(szProto, PS_ADDTOLISTBYEVENT, MAKEWPARAM(0, j), (LPARAM)wndData->mhDbEvent);
									if (hContact && caGroup) {
										// use newest group API if available
										if (ServiceExists(MS_CLIST_CONTACTCHANGEGROUP))
											CallService(MS_CLIST_CONTACTCHANGEGROUP, hContact, (LPARAM)nGroupId);
										else
											db_set_ts(hContact, "CList", "Group", caGroup);
									}
									break;
								}
							mir_free(caUIN);
						} // move to next item
					break;
				}                    
			case IDDETAILS:
				{ // for each selected item, find its index in the hDbEvent
					// and after that add this item to the DB
					// finally, request Details window for this hContact
					HWND hLV = GetDlgItem(hwndDlg, IDC_CONTACTS);
					for (int i = 0; i < ListView_GetItemCount(hLV); i++)
						if (ListView_GetItemState(hLV, i, LVIS_SELECTED)) {
							hContact = CreateTemporaryContactForItem(hwndDlg, wndData, i);
							if (hContact)
								CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
						}
				}
				break;

			case IDCANCEL:
				SendMessage(hwndDlg, WM_CLOSE, 0, 0);
				break;

			case IDC_ENABLEGROUPS:
				EnableGroupCombo(hwndDlg);
				break;

			case IDC_GROUPS:
				// rebuild group list on popup
				if (HIWORD(wParam) == CBN_DROPDOWN)
					RebuildGroupCombo(hwndDlg);
				break;

			case ID_POPUP_ADDUSER:
				hContact = CreateTemporaryContactForItem(hwndDlg, wndData, wndData->iPopupItem);
				if (hContact)
					DialogAddContactExecute(hwndDlg, hContact);
				break;

			case ID_POPUP_USERDETAILS:
				hContact = CreateTemporaryContactForItem(hwndDlg, wndData, wndData->iPopupItem);
				if (hContact)
					CallService(MS_USERINFO_SHOWDIALOG, hContact, 0 );
				break;

			case ID_POPUP_SENDMESSAGE:
				hContact = CreateTemporaryContactForItem(hwndDlg, wndData, wndData->iPopupItem);
				if (hContact)
					CallService(MS_MSG_SENDMESSAGE, hContact, 0);
				break;   

			case IDC_USERMENU:
				{
					RECT rc;
					HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)wndData->mhContact, 0);
					GetWindowRect(GetDlgItem(hwndDlg,IDC_USERMENU), &rc);
					TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, hwndDlg, NULL);
					DestroyMenu(hMenu);
				}
				break;

			case IDC_HISTORY:
				CallService(MS_HISTORY_SHOWCONTACTHISTORY,(WPARAM)wndData->mhContact,0);
				break;

			case IDC_DETAILS:
				CallService(MS_USERINFO_SHOWDIALOG,(WPARAM)wndData->mhContact,0);
				break;

			case IDC_ADD:
				DialogAddContactExecute(hwndDlg, wndData->mhContact);
				break;
			}
		}
		break;

	case WM_CONTEXTMENU:
		{
			HWND hLV = GetDlgItem(hwndDlg, IDC_CONTACTS);
			LVHITTESTINFO lvh;
			RECT rt;

			wndData->iPopupItem = -1;
			if ((HWND)wParam != hLV) break;  // if not our ListView go away
			lvh.pt.x = LOWORD(lParam);
			lvh.pt.y = HIWORD(lParam);
			if (GetWindowRect(hLV, &rt)==0) return FALSE; // ?? why this, some check ??
			ScreenToClient(hLV, &lvh.pt); // convert to ListView local coordinates
			int ci = ListView_HitTest(hLV, &lvh);
			if (ci==-1) break; // mouse is not over any item
			wndData->iPopupItem = ci;
			TrackPopupMenu(GetSubMenu(wndData->mhPopup, 0), TPM_LEFTALIGN|TPM_TOPALIGN, LOWORD(lParam), HIWORD(lParam), 0, hwndDlg, NULL);
		}
		break;

	case HM_EVENTSENT:
		{
			ACKDATA *ack = (ACKDATA*)lParam;
			if (ack->type != ACKTYPE_SEARCH) break;      // not search ack, go away
			if (ack->hProcess != wndData->rhSearch) break; //not our search, go away
			if (ack->result == ACKRESULT_DATA) {
				HWND hLV = GetDlgItem(hwndDlg, IDC_CONTACTS);
				PROTOSEARCHRESULT *psr = (PROTOSEARCHRESULT*)ack->lParam;
				LVFINDINFO fi;
				fi.flags = LVFI_STRING;
				fi.psz = wndData->haUin;
				int iLPos = ListView_FindItem(hLV, -1, &fi);
				if (iLPos == -1) iLPos = 0;
				if (mir_tstrcmp(psr->nick, _T("")) && psr->nick)
					ListView_SetItemText(hLV, iLPos, 1, psr->nick);
				ListView_SetItemText(hLV, iLPos, 2, psr->firstName);
				ListView_SetItemText(hLV, iLPos, 3, psr->lastName);
				break;
			}
			mir_free(wndData->haUin);
		}
		break;
	
	case WM_CLOSE:  // user closed window, so destroy it
		WindowList_Remove(ghRecvWindowList, hwndDlg);
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY: // last message received by this dialog, cleanup
		db_event_markRead(wndData->mhContact, wndData->mhDbEvent);
		Utils_SaveWindowPosition(hwndDlg, NULL, MODULENAME, "");
		ImageList_Destroy(wndData->mhListIcon);
		UnhookEvent(wndData->hHook);
		DestroyMenu(wndData->mhPopup);
		{
			for (int i = 0; i < SIZEOF(wndData->hIcons); i++)
				DestroyIcon(wndData->hIcons[i]);
		}
		delete wndData; // automatically calls destructor
		break;

	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

	case WM_DRAWITEM:
		DrawProtocolIcon(hwndDlg, lParam, wndData->mhContact);
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

	case WM_SIZE:
		if (!IsIconic(hwndDlg)) {
			// make the dlg resizeable
			UTILRESIZEDIALOG urd = { sizeof(urd) };
			urd.hInstance = hInst;
			urd.hwndDlg = hwndDlg;
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_RECEIVE);
			urd.pfnResizer = RecvDlg_Resize;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM) & urd);
		}
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO* mmi=(MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = 480+2*GetSystemMetrics(SM_CXSIZEFRAME);
			mmi->ptMinTrackSize.y = 130+2*GetSystemMetrics(SM_CYSIZEFRAME);
		}
		break;

	case DM_UPDATETITLE:
		UpdateDialogTitle(hwndDlg, wndData ? wndData->mhContact : NULL, LPGENT("Contacts from"));
		if (wndData)
			UpdateDialogAddButton(hwndDlg, wndData->mhContact);
		break;        
	}
	return FALSE;
}
