/*
	New Away System - plugin for Miranda IM
	Copyright (C) 2005-2007 Chervov Dmitry
	Copyright (C) 2004-2005 Iksaif Entertainment
	Copyright (C) 2002-2003 Goblineye Entertainment

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "stdafx.h"

struct READAWAYMSGDATA
{
	MCONTACT hContact; // contact
	HANDLE hSeq; // sequence for stat msg request
	HANDLE hAwayMsgEvent; // hooked
};

#define RAMDLGSIZESETTING "ReadAwayMsgDlg"

MWindowList g_hReadWndList = nullptr;

static int ReadAwayMsgDlgResize(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_READAWAYMSG_MSG:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

	case IDC_READAWAYMSG_RETRIEVE:
		return RD_ANCHORX_CENTRE | RD_ANCHORY_CENTRE;

	case IDOK:
		return RD_ANCHORX_CENTRE | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

static INT_PTR CALLBACK ReadAwayMsgDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		Window_SetSkinIcon_IcoLib(hwndDlg, SKINICON_OTHER_MIRANDA);
		Utils_RestoreWindowPosition(hwndDlg, NULL, MODULENAME, RAMDLGSIZESETTING);
		{
			READAWAYMSGDATA *awayData = new READAWAYMSGDATA;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)awayData);
			awayData->hContact = lParam;
			awayData->hAwayMsgEvent = HookEventMessage(ME_PROTO_ACK, hwndDlg, UM_RAM_AWAYMSGACK);
			awayData->hSeq = (HANDLE)CallContactService(awayData->hContact, PS_GETAWAYMSG);
			WindowList_Add(g_hReadWndList, hwndDlg, awayData->hContact);

			wchar_t str[256], format[128];
			wchar_t *contactName = Clist_GetContactDisplayName(awayData->hContact);
			char *szProto = Proto_GetBaseAccountName(awayData->hContact);
			wchar_t *status = Clist_GetStatusModeDescription(db_get_w(awayData->hContact, szProto, "Status", ID_STATUS_OFFLINE), 0);
			GetWindowText(hwndDlg, format, _countof(format));
			mir_snwprintf(str, format, status, contactName);
			SetWindowText(hwndDlg, str);
			GetDlgItemText(hwndDlg, IDC_READAWAYMSG_RETRIEVE, format, _countof(format));
			mir_snwprintf(str, format, status);
			SetDlgItemText(hwndDlg, IDC_READAWAYMSG_RETRIEVE, str);
		}
		return true;

	case UM_RAM_AWAYMSGACK: // got away msg
		{
			ACKDATA *ack = (ACKDATA*)lParam;
			READAWAYMSGDATA *awayData = (READAWAYMSGDATA*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			// make sure everything is ok, and this is our ack
			if (ack->hContact != awayData->hContact || ack->type != ACKTYPE_AWAYMSG || ack->hProcess != awayData->hSeq || ack->result != ACKRESULT_SUCCESS)
				break;

			// done with the event
			if (awayData->hAwayMsgEvent) {
				UnhookEvent(awayData->hAwayMsgEvent);
				awayData->hAwayMsgEvent = nullptr;
			}
			const wchar_t *ptszStatusMsg = (const wchar_t*)ack->lParam;
			SetDlgItemText(hwndDlg, IDC_READAWAYMSG_MSG, ptszStatusMsg);
			ShowWindow(GetDlgItem(hwndDlg, IDC_READAWAYMSG_RETRIEVE), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_READAWAYMSG_MSG), SW_SHOW);
			SetDlgItemText(hwndDlg, IDOK, TranslateT("&Close"));
			db_set_ws(awayData->hContact, "CList", "StatusMsg", ptszStatusMsg);
		}
		break;

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case BN_CLICKED:
			switch (LOWORD(wParam)) {
			case IDCANCEL:
			case IDOK:
				DestroyWindow(hwndDlg);
				break;
			}
		}
		break;

	case WM_SIZE:
		Utils_ResizeDialog(hwndDlg, g_plugin.getInst(), MAKEINTRESOURCEA(IDD_READAWAYMSG), ReadAwayMsgDlgResize);
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		READAWAYMSGDATA *awayData = (READAWAYMSGDATA*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (awayData->hAwayMsgEvent) {
			UnhookEvent(awayData->hAwayMsgEvent);
			awayData->hAwayMsgEvent = nullptr;
		}
		delete awayData;
		Utils_SaveWindowPosition(hwndDlg, NULL, MODULENAME, RAMDLGSIZESETTING);
		WindowList_Remove(g_hReadWndList, hwndDlg);
	}
	return false;
}

INT_PTR GetContactStatMsg(WPARAM wParam, LPARAM)
{
	if (HWND hWnd = WindowList_Find(g_hReadWndList, wParam)) // already have it
		SetForegroundWindow(hWnd);
	else
		CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_READAWAYMSG), nullptr, ReadAwayMsgDlgProc, wParam);
	return 0;
}
