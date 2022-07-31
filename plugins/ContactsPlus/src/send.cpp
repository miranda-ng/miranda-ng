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


/* TSendProcessList */

TSendProcessList::TSendProcessList()
{
	Count = 0;
	Items = nullptr;
}

TSendProcessList::~TSendProcessList()
{
	mir_free(Items);
}

void TSendProcessList::Add(HANDLE hProcc)
{
	mir_cslock lck(lock);
	Items = (HANDLE*)mir_realloc(Items, (Count + 1)*sizeof(HANDLE));
	Items[Count] = hProcc;
	Count++;
}

void TSendProcessList::Remove(HANDLE hProcc)
{
	mir_cslock lck(lock);
	for (int i = 0; i < Count; i++)
		if (Items[i] == hProcc) {
			memmove(Items + i, Items + i + 1, (Count - i - 1)*sizeof(HANDLE));
			Count--;
			break;
		}
}

/* TSendContactsData */

TSendContactsData::TSendContactsData(MCONTACT contact) : uacklist()
{
	hContact = contact;
	hHook = nullptr;
	hError = nullptr;
	aContacts = nullptr;
	nContacts = 0;
}

TSendContactsData::~TSendContactsData()
{
	ClearContacts();
	UnhookProtoAck();
}


void TSendContactsData::HookProtoAck(HWND hwndDlg)
{
	if (!hHook)
		hHook = HookEventMessage(ME_PROTO_ACK, hwndDlg, HM_EVENTSENT);
}

void TSendContactsData::UnhookProtoAck()
{
	if (hHook) {
		UnhookEvent(hHook);
		hHook = nullptr;
	}
}

void TSendContactsData::ShowErrorDlg(HWND hwndDlg, char* szMsg, bool bAllowRetry)
{
	ShowWindow(hwndDlg, SW_SHOWNORMAL);
	EnableWindow(hwndDlg, FALSE);
	if (!hError) {
		hError = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_MSGSENDERROR), hwndDlg, ErrorDlgProc, _A2T(szMsg));
		if (!bAllowRetry)
			EnableDlgItem(hError, IDOK, FALSE); // do not allow again - fatal, could not be better
	}
}

void TSendContactsData::ClearContacts()
{
	mir_free(aContacts);
	nContacts = 0;
}

void TSendContactsData::AddContact(MCONTACT hContact1)
{
	aContacts = (MCONTACT*)mir_realloc(aContacts, (nContacts + 1)*sizeof(MCONTACT));
	aContacts[nContacts] = hContact1;
	nContacts++;
}

int TSendContactsData::SendContactsPacket(HWND hwndDlg, MCONTACT *phContacts, int nContacts1)
{
	HANDLE hProcc = (HANDLE)ProtoChainSend(hContact, PSS_CONTACTS, MAKEWPARAM(0, nContacts1), (LPARAM)phContacts);
	if (!hProcc) {
		// on trivial error - do not close dialog
		ShowErrorDlg(hwndDlg, "Contacts transfer failed!", FALSE);
		return FALSE; // Failure
	}
	
	TAckData *ackData = g_aAckData.Add(hProcc, new TAckData(hContact));
	uacklist.Add(hProcc);
	ackData->nContacts = nContacts1;
	ackData->aContacts = (MCONTACT*)mir_alloc(nContacts1*sizeof(MCONTACT));
	memmove(ackData->aContacts, phContacts, nContacts1*sizeof(MCONTACT)); // copy the array of hContact for ack array
	EnableDlgItem(hwndDlg, IDOK, FALSE);
	EnableDlgItem(hwndDlg, IDC_LIST, FALSE);
	return TRUE; // Success
}

int TSendContactsData::SendContacts(HWND hwndDlg)
{
	char *szProto = Proto_GetBaseAccountName(hContact);
	int nMaxContacts = CallProtoService(szProto, PS_GETCAPS, PFLAG_MAXCONTACTSPERPACKET, hContact);

	if (!nMaxContacts) {
		ShowErrorDlg(hwndDlg, "The selected contact does not support receiving contacts.", FALSE);
		return FALSE;
	}

	// hook event - we want to receive protocol acknowledgements
	HookProtoAck(hwndDlg);

	// send in packets, each of nMaxContacts contacts
	for (int j = 0; j < nContacts / nMaxContacts; j++)
		if (!SendContactsPacket(hwndDlg, aContacts + j*nMaxContacts, nMaxContacts))
			return FALSE;

	if (nContacts%nMaxContacts != 0)
		if (!SendContactsPacket(hwndDlg, aContacts + nContacts / nMaxContacts*nMaxContacts, nContacts%nMaxContacts))
			return FALSE;

	return TRUE;
}

/* Send Dialog Implementation */

static MCONTACT FindNextClistContact(HWND hList, MCONTACT hContact, MCONTACT *phItem)
{
	MCONTACT hNextContact = db_find_next(hContact);
	MCONTACT hNextItem = NULL;

	while (hNextContact && !(hNextItem = (MCONTACT)SendMessage(hList, CLM_FINDCONTACT, (WPARAM)hNextContact, 0)))
		hNextContact = db_find_next(hNextContact);

	if (phItem)
		*phItem = hNextItem;

	return hNextContact;
}


static MCONTACT FindFirstClistContact(HWND hList, MCONTACT *phItem)
{
	MCONTACT hContact = db_find_first();
	MCONTACT hItem = (MCONTACT)SendMessage(hList, CLM_FINDCONTACT, hContact, 0);

	if (hContact && !hItem)
		return FindNextClistContact(hList, hContact, phItem);

	if (phItem)
		*phItem = hItem;

	return hContact;
}


bool binListEvent = FALSE;

static void SetAllContactChecks(HWND hwndList, MCONTACT hReceiver) // doubtful name
{
	if (binListEvent)
		return;

	binListEvent = TRUE;
	char *szProto = Proto_GetBaseAccountName(hReceiver);
	if (szProto == nullptr)
		return;

	SendMessage(hwndList, CLM_SETHIDEEMPTYGROUPS, Clist::HideEmptyGroups, 0);

	MCONTACT hItem, hContact = FindFirstClistContact(hwndList, &hItem);
	while (hContact) {
		char* szProto2 = Proto_GetBaseAccountName(hContact);

		// different protocols or protocol undefined, remove contact, useless anyway
		if (strcmpnull(szProto, szProto2))
			SendMessage(hwndList, CLM_DELETEITEM, (WPARAM)hItem, 0);
		else // otherwise uncheck
			SendMessage(hwndList, CLM_SETCHECKMARK, (WPARAM)hItem, 0);

		hContact = FindNextClistContact(hwndList, hContact, &hItem);
	}

	binListEvent = FALSE;
}

INT_PTR CALLBACK SendDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TSendContactsData* wndData = (TSendContactsData*)GetWindowLongPtr(hwndDlg, DWLP_USER);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_CONTACTS)));
		SetAllContactChecks(GetDlgItem(hwndDlg, IDC_LIST), lParam);
		WindowList_Add(g_hSendWindowList, hwndDlg, lParam);
		wndData = new TSendContactsData(lParam);
		SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)wndData);
		// new dlg init 
		wndData->hIcons[0] = InitMButton(hwndDlg, IDC_ADD, SKINICON_OTHER_ADDCONTACT, LPGENW("Add Contact Permanently to List"));
		wndData->hIcons[1] = InitMButton(hwndDlg, IDC_DETAILS, SKINICON_OTHER_USERDETAILS, LPGENW("View User's Details"));
		wndData->hIcons[2] = InitMButton(hwndDlg, IDC_HISTORY, SKINICON_OTHER_HISTORY, LPGENW("View User's History"));
		wndData->hIcons[3] = InitMButton(hwndDlg, IDC_USERMENU, SKINICON_OTHER_DOWNARROW, LPGENW("User Menu"));

		SendMessage(hwndDlg, DM_UPDATETITLE, 0, 0);
		// new dialog init done
		return TRUE;

	case WM_SETFOCUS:
		SetFocus(GetDlgItem(hwndDlg, IDC_LIST));
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_LIST) {
			switch (((LPNMHDR)lParam)->code) {
			case CLN_NEWCONTACT:
			case CLN_LISTREBUILT: // rebuild list
				if (wndData) SetAllContactChecks(GetDlgItem(hwndDlg, IDC_LIST), wndData->hContact);
			}
		}
		break;

	case WM_TIMER:
		if (wParam == TIMERID_MSGSEND) {
			KillTimer(hwndDlg, wParam);
			wndData->ShowErrorDlg(hwndDlg, "The contacts send timed out.", TRUE);
		}
		break;

	case DM_ERRORDECIDED:
		EnableWindow(hwndDlg, TRUE);
		wndData->hError = nullptr;
		switch (wParam) {
		case MSGERROR_CANCEL:
			wndData->UnhookProtoAck();
			if (wndData->uacklist.Count) {
				for (int i = 0; i < wndData->uacklist.Count; i++)
					delete g_aAckData.Remove(wndData->uacklist.Items[i]); // remove our ackdata & release structure

				mir_free(wndData->uacklist.Items);
				wndData->uacklist.Items = nullptr;
				wndData->uacklist.Count = 0;
			}
			EnableDlgItem(hwndDlg, IDOK, TRUE);
			EnableDlgItem(hwndDlg, IDC_LIST, TRUE);
			ShowWindow(hwndDlg, SW_SHOWNORMAL);
			SetFocus(GetDlgItem(hwndDlg, IDC_LIST));
			break;

		case MSGERROR_DONE:
			// contacts were delivered succesfully after timeout
			SetFocus(GetDlgItem(hwndDlg, IDC_LIST));
			wndData->UnhookProtoAck();
			break;

		case MSGERROR_RETRY:// resend timeouted packets
			for (int i = 0; i < wndData->uacklist.Count; i++) {
				TAckData *lla = g_aAckData.Remove(wndData->uacklist.Items[i]);
				HANDLE hProcc = (HANDLE)ProtoChainSend(wndData->hContact, PSS_CONTACTS, MAKEWPARAM(0, lla->nContacts), (LPARAM)lla->aContacts);

				if (!hProcc) { // if fatal do not include
					wndData->uacklist.Remove(wndData->uacklist.Items[i]);
					delete lla; // release the structure
					continue;
				}
				else {
					// update process code
					wndData->uacklist.Items[i] = hProcc;
					g_aAckData.Add(hProcc, lla);
				}
			}// collect TAckData for our window, resend
			break;
		}
		break;

	case WM_COMMAND:
		if (!lParam && Clist_MenuProcessCommand(LOWORD(wParam), MPCF_CONTACTMENU, wndData->hContact))
			break;

		switch (LOWORD(wParam)) {
		case IDOK:
			if (IsWindowEnabled(GetDlgItem(hwndDlg, IDOK))) {
				MCONTACT hContact, hItem;
				wndData->ClearContacts(); // do not include contacts twice

				HWND hList = GetDlgItem(hwndDlg, IDC_LIST);
				hContact = FindFirstClistContact(hList, &hItem);
				while (hContact) {
					// build list of contacts to send
					if (SendMessage(hList, CLM_GETCHECKMARK, (WPARAM)hItem, 0))
						wndData->AddContact(hContact);

					hContact = FindNextClistContact(hList, hContact, &hItem);
				}
				/* send contacts */
				if (!wndData->SendContacts(hwndDlg))
					break;

				SetTimer(hwndDlg, TIMERID_MSGSEND, db_get_dw(0, "SRMsg", "MessageTimeout", TIMEOUT_MSGSEND), nullptr);
			}
			break;

		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;

		case ID_SELECTALL:
			{
				// select all contacts
				HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);
				MCONTACT hItem, hContact = FindFirstClistContact(hwndList, &hItem);
				while (hContact) {
					SendMessage(hwndList, CLM_SETCHECKMARK, (WPARAM)hItem, 1);
					hContact = FindNextClistContact(hwndList, hContact, &hItem);
				}
			}
			break;
	
		case IDC_USERMENU:
			{
				RECT rc;
				GetWindowRect(GetDlgItem(hwndDlg, IDC_USERMENU), &rc);
				HMENU hMenu = Menu_BuildContactMenu(wndData->hContact);
				TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, hwndDlg, nullptr);
				DestroyMenu(hMenu);
			}
			break;

		case IDC_HISTORY:
			CallService(MS_HISTORY_SHOWCONTACTHISTORY, (WPARAM)wndData->hContact, 0);
			break;

		case IDC_DETAILS:
			CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)wndData->hContact, 0);
			break;

		case IDC_ADD:
			Contact::Add(wndData->hContact, hwndDlg);
			break;
		}
		break;

	case HM_EVENTSENT:
		{
			ACKDATA *ack = (ACKDATA*)lParam;
			if (ack->type != ACKTYPE_CONTACTS)
				break;

			TAckData *ackData = g_aAckData.Get(ack->hProcess);
			if (ackData == nullptr)
				break;    // on unknown hprocc go away

			if (ackData->hContact != ack->hContact)
				break; // this is not ours, strange

			if (ack->result == ACKRESULT_FAILED) {
				// some process failed, show error dialog
				KillTimer(hwndDlg, TIMERID_MSGSEND);
				wndData->ShowErrorDlg(hwndDlg, (char *)ack->lParam, TRUE);
				// ackData get used in error handling, released there
				break;
			}

			DBEVENTINFO dbei = {};
			dbei.szModule = Proto_GetBaseAccountName(ackData->hContact);
			dbei.eventType = EVENTTYPE_CONTACTS;
			dbei.flags = DBEF_SENT | DBEF_UTF;
			dbei.timestamp = time(0);
			//make blob
			TCTSend* maSend = (TCTSend*)_alloca(ackData->nContacts*sizeof(TCTSend));
			memset(maSend, 0, (ackData->nContacts * sizeof(TCTSend)));
			dbei.cbBlob = 0;
			char* pBlob;
			int i;
			for (i = 0; i < ackData->nContacts; i++) {
				// prepare data & count size
				maSend[i].mcaNick = mir_utf8encodeW(Clist_GetContactDisplayName(ackData->aContacts[i]));
				maSend[i].mcaUIN = mir_utf8encodeW(ptrW(GetContactUID(ackData->aContacts[i])));
				dbei.cbBlob += (uint32_t)strlennull(maSend[i].mcaUIN) + (uint32_t)strlennull((char*)maSend[i].mcaNick) + 2;
			}
			dbei.pBlob = (uint8_t*)_alloca(dbei.cbBlob);
			for (i = 0, pBlob = (char*)dbei.pBlob; i < ackData->nContacts; i++) {
				strcpynull(pBlob, (char*)maSend[i].mcaNick);
				pBlob += strlennull(pBlob) + 1;
				strcpynull(pBlob, maSend[i].mcaUIN);
				pBlob += strlennull(pBlob) + 1;
			}
			db_event_add(ackData->hContact, &dbei);
			g_aAckData.Remove(ack->hProcess); // do not release here, still needed
			wndData->uacklist.Remove(ack->hProcess); // packet confirmed
			for (i = 0; i < ackData->nContacts; i++) {
				mir_free(maSend[i].mcaUIN);
				mir_free(maSend[i].mcaNick);
			}
			delete ackData; // all done, release structure
			if (!wndData->uacklist.Count) {
				Skin_PlaySound("SentContacts");
				KillTimer(hwndDlg, TIMERID_MSGSEND);

				if (wndData->hError)
					SendMessage(wndData->hError, DM_ERRORDECIDED, MSGERROR_DONE, 0);

				SendMessage(hwndDlg, WM_CLOSE, 0, 0); // all packets confirmed, close the dialog
			}
		}
		break;
	
	case WM_MEASUREITEM:
		return Menu_MeasureItem(lParam);

	case WM_DRAWITEM:
		DrawProtocolIcon(hwndDlg, lParam, wndData->hContact);
		return Menu_DrawItem(lParam);

	case DM_UPDATETITLE:
		UpdateDialogTitle(hwndDlg, wndData ? wndData->hContact : NULL, TranslateT("Send Contacts to"));
		if (wndData)
			UpdateDialogAddButton(hwndDlg, wndData->hContact);
		break;

	case WM_CLOSE:
		wndData->UnhookProtoAck();
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		WindowList_Remove(g_hSendWindowList, hwndDlg);
		delete wndData;
		break;
	}

	return FALSE;
}


// Error Dialog

INT_PTR CALLBACK ErrorDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			if (lParam)
				SetDlgItemText(hwndDlg, IDC_ERRORTEXT, TranslateW((wchar_t*)lParam));

			RECT rc, rcParent;
			GetWindowRect(hwndDlg, &rc);
			GetWindowRect(GetParent(hwndDlg), &rcParent);
			SetWindowPos(hwndDlg, nullptr,
				(rcParent.left + rcParent.right - (rc.right - rc.left)) / 2,
				(rcParent.top + rcParent.bottom - (rc.bottom - rc.top)) / 2,
				0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			SendMessage(GetParent(hwndDlg), DM_ERRORDECIDED, MSGERROR_RETRY, 0);
			DestroyWindow(hwndDlg);
			break;

		case IDCANCEL:
			SendMessage(GetParent(hwndDlg), DM_ERRORDECIDED, MSGERROR_CANCEL, 0);
			DestroyWindow(hwndDlg);
			break;
		}
		break;

	case DM_ERRORDECIDED:
		if (wParam != MSGERROR_DONE) break;
		SendMessage(GetParent(hwndDlg), DM_ERRORDECIDED, MSGERROR_DONE, 0);
		DestroyWindow(hwndDlg);
		break;
	}

	return FALSE;
}
