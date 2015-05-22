/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "commonheaders.h"

INT_PTR CALLBACK DlgProcAdded(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MEVENT hDbEvent = (MEVENT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		Button_SetIcon_IcoLib(hwndDlg, IDC_DETAILS, SKINICON_OTHER_USERDETAILS, LPGEN("View user's details"));
		Button_SetIcon_IcoLib(hwndDlg, IDC_ADD, SKINICON_OTHER_ADDCONTACT, LPGEN("Add contact permanently to list"));

		hDbEvent = lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		{
			//blob is: uin(DWORD), hcontact(HANDLE), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ)
			DBEVENTINFO dbei = { sizeof(dbei) };
			dbei.cbBlob = db_event_getBlobSize(hDbEvent);
			dbei.pBlob = (PBYTE)alloca(dbei.cbBlob);
			db_event_get(hDbEvent, &dbei);

			DWORD uin = *(PDWORD)dbei.pBlob;
			MCONTACT hContact = DbGetAuthEventContact(&dbei);
			char* nick = (char*)dbei.pBlob + sizeof(DWORD) * 2;
			char* first = nick + mir_strlen(nick) + 1;
			char* last = first + mir_strlen(first) + 1;
			char* email = last + mir_strlen(last) + 1;

			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, CallProtoService(dbei.szModule, PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0));
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, CallProtoService(dbei.szModule, PS_LOADICON, PLI_PROTOCOL | PLIF_LARGE, 0));

			PROTOACCOUNT* acc = ProtoGetAccount(dbei.szModule);

			TCHAR* lastT = dbei.flags & DBEF_UTF ? Utf8DecodeT(last) : mir_a2t(last);
			TCHAR* firstT = dbei.flags & DBEF_UTF ? Utf8DecodeT(first) : mir_a2t(first);
			TCHAR* nickT = dbei.flags & DBEF_UTF ? Utf8DecodeT(nick) : mir_a2t(nick);
			TCHAR* emailT = dbei.flags & DBEF_UTF ? Utf8DecodeT(email) : mir_a2t(email);

			TCHAR name[128] = _T("");
			int off = 0;
			if (firstT[0] && lastT[0])
				off = mir_sntprintf(name, SIZEOF(name), _T("%s %s"), firstT, lastT);
			else if (firstT[0])
				off = mir_sntprintf(name, SIZEOF(name), _T("%s"), firstT);
			else if (lastT[0])
				off = mir_sntprintf(name, SIZEOF(name), _T("%s"), lastT);
			if (nickT[0]) {
				if (off)
					mir_sntprintf(name + off, SIZEOF(name) - off, _T(" (%s)"), nickT);
				else
					_tcsncpy_s(name, nickT, _TRUNCATE);
			}
			if (!name[0])
				_tcsncpy_s(name, TranslateT("<Unknown>"), _TRUNCATE);

			TCHAR hdr[256];
			if (uin && emailT[0])
				mir_sntprintf(hdr, SIZEOF(hdr), TranslateT("%s added you to the contact list\n%u (%s) on %s"), name, uin, emailT, acc->tszAccountName);
			else if (uin)
				mir_sntprintf(hdr, SIZEOF(hdr), TranslateT("%s added you to the contact list\n%u on %s"), name, uin, acc->tszAccountName);
			else
				mir_sntprintf(hdr, SIZEOF(hdr), TranslateT("%s added you to the contact list\n%s on %s"), name, emailT[0] ? emailT : TranslateT("(Unknown)"), acc->tszAccountName);

			SetDlgItemText(hwndDlg, IDC_HEADERBAR, hdr);

			mir_free(lastT);
			mir_free(firstT);
			mir_free(nickT);
			mir_free(emailT);

			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_DETAILS), GWLP_USERDATA, (LONG_PTR)hContact);

			if (hContact == INVALID_CONTACT_ID || !db_get_b(hContact, "CList", "NotOnList", 0))
				ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ADD:
			{
				ADDCONTACTSTRUCT acs = { 0 };
				acs.hDbEvent = hDbEvent;
				acs.handleType = HANDLE_EVENT;
				acs.szProto = "";
				CallService(MS_ADDCONTACT_SHOW, (WPARAM)hwndDlg, (LPARAM)&acs);

				MCONTACT hContact = (MCONTACT)GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_DETAILS), GWLP_USERDATA);
				if ((hContact == INVALID_CONTACT_ID) || !db_get_b(hContact, "CList", "NotOnList", 0))
					ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
			}
			break;

		case IDC_DETAILS:
			{
				MCONTACT hContact = (MCONTACT)GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_DETAILS), GWLP_USERDATA);
				CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
			}
			break;

		case IDOK:
			{
				ADDCONTACTSTRUCT acs = { 0 };
				acs.hDbEvent = hDbEvent;
				acs.handleType = HANDLE_EVENT;
				acs.szProto = "";
				CallService(MS_ADDCONTACT_SHOW, (WPARAM)hwndDlg, (LPARAM)&acs);
			}
			//fall through
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;
		}
		break;

	case WM_DESTROY:
		Button_FreeIcon_IcoLib(hwndDlg, IDC_ADD);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_DETAILS);
		DestroyIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
		DestroyIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, 0));
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK DlgProcAuthReq(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MEVENT hDbEvent = (MEVENT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		Button_SetIcon_IcoLib(hwndDlg, IDC_DETAILS, SKINICON_OTHER_USERDETAILS, LPGEN("View user's details"));
		Button_SetIcon_IcoLib(hwndDlg, IDC_ADD, SKINICON_OTHER_ADDCONTACT, LPGEN("Add contact permanently to list"));
		{
			hDbEvent = lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			//blob is: uin(DWORD), hcontact(HANDLE), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ), reason(ASCIIZ)
			DBEVENTINFO dbei = { sizeof(dbei) };
			dbei.cbBlob = db_event_getBlobSize(hDbEvent);
			dbei.pBlob = (PBYTE)alloca(dbei.cbBlob);
			db_event_get(hDbEvent, &dbei);

			DWORD uin = *(PDWORD)dbei.pBlob;
			MCONTACT hContact = DbGetAuthEventContact(&dbei);
			char *nick = (char*)dbei.pBlob + sizeof(DWORD) * 2;
			char *first = nick + mir_strlen(nick) + 1;
			char *last = first + mir_strlen(first) + 1;
			char *email = last + mir_strlen(last) + 1;
			char *reason = email + mir_strlen(email) + 1;

			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, CallProtoService(dbei.szModule, PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0));
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, CallProtoService(dbei.szModule, PS_LOADICON, PLI_PROTOCOL | PLIF_LARGE, 0));

			PROTOACCOUNT* acc = ProtoGetAccount(dbei.szModule);

			ptrT lastT(dbei.flags & DBEF_UTF ? Utf8DecodeT(last) : mir_a2t(last));
			ptrT firstT(dbei.flags & DBEF_UTF ? Utf8DecodeT(first) : mir_a2t(first));
			ptrT nickT(dbei.flags & DBEF_UTF ? Utf8DecodeT(nick) : mir_a2t(nick));
			ptrT emailT(dbei.flags & DBEF_UTF ? Utf8DecodeT(email) : mir_a2t(email));
			ptrT reasonT(dbei.flags & DBEF_UTF ? Utf8DecodeT(reason) : mir_a2t(reason));

			TCHAR name[128] = _T("");
			int off = 0;
			if (firstT[0] && lastT[0])
				off = mir_sntprintf(name, SIZEOF(name), _T("%s %s"), (TCHAR*)firstT, (TCHAR*)lastT);
			else if (firstT[0])
				off = mir_sntprintf(name, SIZEOF(name), _T("%s"), (TCHAR*)firstT);
			else if (lastT[0])
				off = mir_sntprintf(name, SIZEOF(name), _T("%s"), (TCHAR*)lastT);
			if (mir_tstrlen(nickT)) {
				if (off)
					mir_sntprintf(name + off, SIZEOF(name) - off, _T(" (%s)"), (TCHAR*)nickT);
				else
					_tcsncpy_s(name, nickT, _TRUNCATE);
			}
			if (!name[0])
				_tcsncpy_s(name, TranslateT("<Unknown>"), _TRUNCATE);

			TCHAR hdr[256];
			if (uin && emailT[0])
				mir_sntprintf(hdr, SIZEOF(hdr), TranslateT("%s requested authorization\n%u (%s) on %s"), name, uin, (TCHAR*)emailT, acc->tszAccountName);
			else if (uin)
				mir_sntprintf(hdr, SIZEOF(hdr), TranslateT("%s requested authorization\n%u on %s"), name, uin, acc->tszAccountName);
			else
				mir_sntprintf(hdr, SIZEOF(hdr), TranslateT("%s requested authorization\n%s on %s"), name, emailT[0] ? (TCHAR*)emailT : TranslateT("(Unknown)"), acc->tszAccountName);

			SetDlgItemText(hwndDlg, IDC_HEADERBAR, hdr);
			SetDlgItemText(hwndDlg, IDC_REASON, reasonT);

			if (hContact == INVALID_CONTACT_ID || !db_get_b(hContact, "CList", "NotOnList", 0))
				ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);

			SendDlgItemMessage(hwndDlg, IDC_DENYREASON, EM_LIMITTEXT, 255, 0);
			if (CallProtoService(dbei.szModule, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_NOAUTHDENYREASON) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_DENYREASON), FALSE);
				SetDlgItemText(hwndDlg, IDC_DENYREASON, TranslateT("Feature is not supported by protocol"));
			}
			if (!db_get_b(hContact, "CList", "NotOnList", 0)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADDCHECK), FALSE);
				CheckDlgButton(hwndDlg, IDC_ADDCHECK, BST_UNCHECKED);
			}
			else CheckDlgButton(hwndDlg, IDC_ADDCHECK, BST_CHECKED);

			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_DETAILS), GWLP_USERDATA, (LONG_PTR)hContact);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_DETAILS:
			CallService(MS_USERINFO_SHOWDIALOG, GetWindowLongPtr((HWND)lParam, GWLP_USERDATA), 0);
			break;

		case IDC_DECIDELATER:
			DestroyWindow(hwndDlg);
			break;

		case IDOK:
			{
				DBEVENTINFO dbei = { sizeof(dbei) };
				db_event_get(hDbEvent, &dbei);
				CallProtoService(dbei.szModule, PS_AUTHALLOW, (WPARAM)hDbEvent, 0);

				if (IsDlgButtonChecked(hwndDlg, IDC_ADDCHECK)) {
					ADDCONTACTSTRUCT acs = { 0 };
					acs.hDbEvent = hDbEvent;
					acs.handleType = HANDLE_EVENT;
					acs.szProto = "";
					CallService(MS_ADDCONTACT_SHOW, (WPARAM)hwndDlg, (LPARAM)&acs);
				}
			}
			DestroyWindow(hwndDlg);
			break;

		case IDCANCEL:
			{
				DBEVENTINFO dbei = { sizeof(dbei) };
				db_event_get(hDbEvent, &dbei);

				if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_DENYREASON))) {
					TCHAR szReason[256];
					GetDlgItemText(hwndDlg, IDC_DENYREASON, szReason, SIZEOF(szReason));
					CallProtoService(dbei.szModule, PS_AUTHDENYT, (WPARAM)hDbEvent, (LPARAM)szReason);
				}
				else CallProtoService(dbei.szModule, PS_AUTHDENYT, (WPARAM)hDbEvent, 0);
			}
			DestroyWindow(hwndDlg);
			break;
		}
		break;

	case WM_DESTROY:
		Button_FreeIcon_IcoLib(hwndDlg, IDC_ADD);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_DETAILS);
		DestroyIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
		DestroyIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, 0));
		break;
	}
	return FALSE;
}
