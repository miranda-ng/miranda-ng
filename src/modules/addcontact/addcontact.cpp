/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

INT_PTR CALLBACK AddContactDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	ADDCONTACTSTRUCT *acs;

	switch(msg) {
	case WM_INITDIALOG:
		{
			char szUin[10];
			acs = (ADDCONTACTSTRUCT *)lparam;
			SetWindowLongPtr(hdlg, GWLP_USERDATA, (LONG_PTR)acs);

			TranslateDialogDefault(hdlg);
			Window_SetIcon_IcoLib(hdlg, SKINICON_OTHER_ADDCONTACT);
			if (acs->handleType == HANDLE_EVENT) {
				DWORD dwUin;
				DBEVENTINFO dbei = { sizeof(dbei) };
				dbei.cbBlob = sizeof(DWORD);
				dbei.pBlob = (PBYTE)&dwUin;
				db_event_get(acs->hDbEvent, &dbei);
				_ltoa(dwUin, szUin, 10);
				acs->szProto = dbei.szModule;
			}
			{
				TCHAR *szName = NULL, *tmpStr = NULL;
				if (acs->handleType == HANDLE_CONTACT)
					szName = cli.pfnGetContactDisplayName(acs->hContact, 0);
				else {
					int isSet = 0;

					if (acs->handleType == HANDLE_EVENT) {
						DBEVENTINFO dbei = { sizeof(dbei) };
						dbei.cbBlob = db_event_getBlobSize(acs->hDbEvent);
						dbei.pBlob = (PBYTE)mir_alloc(dbei.cbBlob);
						db_event_get(acs->hDbEvent, &dbei);
						MCONTACT hcontact = *(MCONTACT*)(dbei.pBlob + sizeof(DWORD));
						mir_free(dbei.pBlob);
						if (hcontact != INVALID_CONTACT_ID) {
							szName = cli.pfnGetContactDisplayName(hcontact, 0);
							isSet = 1;
						}
					}
					if (!isSet) {
						szName = (acs->handleType == HANDLE_EVENT) ? (tmpStr = mir_a2t(szUin)) :
							(acs->psr->id ? acs->psr->id : acs->psr->nick);
					}
				}

				if (szName && szName[0]) {
					TCHAR  szTitle[128];
					mir_sntprintf(szTitle, SIZEOF(szTitle), TranslateT("Add %s"), szName);
					SetWindowText(hdlg, szTitle);
				}
				else SetWindowText(hdlg, TranslateT("Add contact"));
				mir_free(tmpStr);
			}
		}

		if (acs->handleType == HANDLE_CONTACT && acs->hContact)
			if (acs->szProto == NULL || (acs->szProto != NULL && *acs->szProto == 0))
				acs->szProto = GetContactProto(acs->hContact);

		{
			TCHAR *grpName;
			for (int groupId = 1; (grpName = cli.pfnGetGroupName(groupId, NULL)) != NULL; groupId++) {
				int id = SendDlgItemMessage(hdlg, IDC_GROUP, CB_ADDSTRING, 0, (LPARAM)grpName);
				SendDlgItemMessage(hdlg, IDC_GROUP, CB_SETITEMDATA, id, groupId);
			}
		}

		SendDlgItemMessage(hdlg, IDC_GROUP, CB_INSERTSTRING, 0, (LPARAM)TranslateT("None"));
		SendDlgItemMessage(hdlg, IDC_GROUP, CB_SETCURSEL, 0, 0);
		/* acs->szProto may be NULL don't expect it */
		{
			// By default check both checkboxes
			CheckDlgButton(hdlg, IDC_ADDED, BST_CHECKED);
			CheckDlgButton(hdlg, IDC_AUTH, BST_CHECKED);

			// Set last choice
			if (db_get_b(NULL, "Miranda", "AuthOpenWindow", 1))
				CheckDlgButton(hdlg, IDC_OPEN_WINDOW, BST_CHECKED);

			DWORD flags = (acs->szProto) ? CallProtoServiceInt(NULL,acs->szProto, PS_GETCAPS, PFLAGNUM_4, 0) : 0;
			if (flags&PF4_FORCEADDED) { // force you were added requests for this protocol
				EnableWindow( GetDlgItem(hdlg, IDC_ADDED), FALSE);
			}
			if (flags&PF4_FORCEAUTH) { // force auth requests for this protocol
				EnableWindow( GetDlgItem(hdlg, IDC_AUTH), FALSE);
			}
			if (flags&PF4_NOCUSTOMAUTH) {
				EnableWindow( GetDlgItem(hdlg, IDC_AUTHREQ), FALSE);
				EnableWindow( GetDlgItem(hdlg, IDC_AUTHGB), FALSE);
			}
			else {
				EnableWindow( GetDlgItem(hdlg, IDC_AUTHREQ), IsDlgButtonChecked(hdlg, IDC_AUTH));
				EnableWindow( GetDlgItem(hdlg, IDC_AUTHGB), IsDlgButtonChecked(hdlg, IDC_AUTH));
				SetDlgItemText(hdlg, IDC_AUTHREQ, TranslateT("Please authorize my request and add me to your contact list."));
			}
		}
		break;

	case WM_COMMAND:
		acs = (ADDCONTACTSTRUCT *)GetWindowLongPtr(hdlg, GWLP_USERDATA);

		switch (LOWORD(wparam)) {
		case IDC_AUTH:
			{
				DWORD flags = CallProtoServiceInt(NULL,acs->szProto, PS_GETCAPS, PFLAGNUM_4, 0);
				if (flags & PF4_NOCUSTOMAUTH) {
					EnableWindow( GetDlgItem(hdlg, IDC_AUTHREQ), FALSE);
					EnableWindow( GetDlgItem(hdlg, IDC_AUTHGB), FALSE);
				}
				else {
					EnableWindow( GetDlgItem(hdlg, IDC_AUTHREQ), IsDlgButtonChecked(hdlg, IDC_AUTH));
					EnableWindow( GetDlgItem(hdlg, IDC_AUTHGB), IsDlgButtonChecked(hdlg, IDC_AUTH));
				}
			}
			break;
		case IDC_OPEN_WINDOW:
			// Remember this choice
			db_set_b(NULL, "Miranda", "AuthOpenWindow", IsDlgButtonChecked(hdlg, IDC_OPEN_WINDOW));
			break;
		case IDOK:
			{
				MCONTACT hContact = INVALID_CONTACT_ID;
				switch (acs->handleType) {
				case HANDLE_EVENT:
					{
						DBEVENTINFO dbei = { sizeof(dbei) };
						db_event_get(acs->hDbEvent, &dbei);
						hContact = (MCONTACT)CallProtoServiceInt(NULL, dbei.szModule, PS_ADDTOLISTBYEVENT, 0, (LPARAM)acs->hDbEvent);
					}
					break;

				case HANDLE_SEARCHRESULT:
					hContact = (MCONTACT)CallProtoServiceInt(NULL, acs->szProto, PS_ADDTOLIST, 0, (LPARAM)acs->psr);
					break;

				case HANDLE_CONTACT:
					hContact = acs->hContact;
					break;
				}

				if (hContact == NULL)
					break;

				TCHAR szHandle[256];
				if ( GetDlgItemText(hdlg, IDC_MYHANDLE, szHandle, SIZEOF(szHandle)))
					db_set_ts(hContact, "CList", "MyHandle", szHandle);

				int item = SendDlgItemMessage(hdlg, IDC_GROUP, CB_GETCURSEL, 0, 0);
				if (item > 0) {
					item = SendDlgItemMessage(hdlg, IDC_GROUP, CB_GETITEMDATA, item, 0);
					CallService(MS_CLIST_CONTACTCHANGEGROUP, hContact, item);
				}

				db_unset(hContact, "CList", "NotOnList");

				if (IsDlgButtonChecked(hdlg, IDC_ADDED))
					CallContactService(hContact, PSS_ADDED, 0, 0);

				if (IsDlgButtonChecked(hdlg, IDC_AUTH)) {
					DWORD flags = CallProtoServiceInt(NULL,acs->szProto, PS_GETCAPS, PFLAGNUM_4, 0);
					if (flags & PF4_NOCUSTOMAUTH)
						CallContactService(hContact, PSS_AUTHREQUESTT, 0, 0);
					else {
						TCHAR szReason[512];
						GetDlgItemText(hdlg, IDC_AUTHREQ, szReason, SIZEOF(szReason));
						CallContactService(hContact, PSS_AUTHREQUESTT, 0, (LPARAM)szReason);
					}
				}

				if (IsDlgButtonChecked(hdlg, IDC_OPEN_WINDOW))
					CallService(MS_CLIST_CONTACTDOUBLECLICKED, hContact, 0);
			}
			// fall through
		case IDCANCEL:
			if (GetParent(hdlg) == NULL)
				DestroyWindow(hdlg);
			else
				EndDialog(hdlg, 0);
			break;
		}
		break;

	case WM_CLOSE:
		/* if there is no parent for the dialog, its a modeless dialog and can't be killed using EndDialog() */
		if (GetParent(hdlg) == NULL)
			DestroyWindow(hdlg);
		else
			EndDialog(hdlg, 0);
		break;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hdlg);
		acs = (ADDCONTACTSTRUCT*)GetWindowLongPtr(hdlg, GWLP_USERDATA);
		if (acs) {
			if (acs->psr) {
				mir_free(acs->psr->nick);
				mir_free(acs->psr->firstName);
				mir_free(acs->psr->lastName);
				mir_free(acs->psr->email);
				mir_free(acs->psr);
			}
			mir_free(acs);
		}
		break;
	}

	return FALSE;
}

INT_PTR AddContactDialog(WPARAM wParam, LPARAM lParam)
{
	if (lParam == 0)
		return 1;
	
	ADDCONTACTSTRUCT *acs = (ADDCONTACTSTRUCT*)mir_alloc(sizeof(ADDCONTACTSTRUCT));
	memcpy(acs, (ADDCONTACTSTRUCT*)lParam, sizeof(ADDCONTACTSTRUCT));
	if (acs->psr) {
		// bad! structures that are bigger than psr will cause crashes if they define pointers within unreachable structural space
		PROTOSEARCHRESULT *psr = (PROTOSEARCHRESULT*)mir_alloc(acs->psr->cbSize);
		memcpy(psr, acs->psr, acs->psr->cbSize);
		psr->nick = psr->flags & PSR_UNICODE ? mir_u2t((wchar_t*)psr->nick) : mir_a2t((char*)psr->nick);
		psr->firstName = psr->flags & PSR_UNICODE ? mir_u2t((wchar_t*)psr->firstName) : mir_a2t((char*)psr->firstName);
		psr->lastName = psr->flags & PSR_UNICODE ? mir_u2t((wchar_t*)psr->lastName) : mir_a2t((char*)psr->lastName);
		psr->email = psr->flags & PSR_UNICODE ? mir_u2t((wchar_t*)psr->email) : mir_a2t((char*)psr->email);
		psr->flags = psr->flags & ~PSR_UNICODE | PSR_TCHAR;
		acs->psr = psr;
		/* copied the passed acs structure, the psr structure with, the pointers within that  */
	}

	if (wParam)
		DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ADDCONTACT), (HWND)wParam, AddContactDlgProc, (LPARAM)acs);
	else
		CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADDCONTACT), (HWND)wParam, AddContactDlgProc, (LPARAM)acs);
	return 0;
}

int LoadAddContactModule(void)
{
	CreateServiceFunction(MS_ADDCONTACT_SHOW, AddContactDialog);
	return 0;
}
