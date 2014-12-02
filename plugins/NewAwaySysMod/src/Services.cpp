/*
	New Away System - plugin for Miranda IM
	Copyright (c) 2005-2007 Chervov Dmitry

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


#include "Common.h"
#include "Properties.h"
#include "m_NewAwaySys.h"

struct NAS_PROTOINFOv1
{
	int cbSize;
	char *szProto;
	union {
		char *szMsg;
		WCHAR *wszMsg;
		TCHAR *tszMsg;
	};
	WORD status;
};

__inline void PSSetStatus(char *szProto, WORD Status, int bNoClistSetStatusMode = false) // just a helper function that sets the status and handles szProto==NULL correctly
{
	g_ProtoStates[szProto].Status = Status;
	if (szProto)
		CallProtoService(szProto, PS_SETSTATUS, Status, 0);
	else if (!bNoClistSetStatusMode) { // global status
		g_fNoProcessing = true;
		CallService(MS_CLIST_SETSTATUSMODE, Status, 0);
		_ASSERT(!g_fNoProcessing && g_ProtoStates[(char*)NULL].Status == Status);
		g_fNoProcessing = false;
	}
}

// called by GamerStatus and built-in miranda autoaway module
// wParam = (int)status, lParam = 0
// MS_AWAYMSG_GETSTATUSMSG "SRAway/GetStatusMessage"
INT_PTR GetStatusMsgW(WPARAM wParam, LPARAM)
{
	LogMessage("MS_AWAYMSG_GETSTATUSMSGW called. status=%d", wParam);
	WCHAR *szMsg = mir_t2u(GetDynamicStatMsg(INVALID_CONTACT_ID, NULL, 0, wParam));
	LogMessage("returned szMsgW:\n%S", szMsg ? szMsg : L"NULL");
	return (INT_PTR)szMsg;
}

// wParam = int iMode
// lParam = char* szMsg, may be null - then we need to use the default message
INT_PTR SetStatusMode(WPARAM wParam, LPARAM lParam) // called by GamerStatus and StatCtrl
{
	LogMessage("MS_AWAYSYS_SETSTATUSMODE called. status=%d, szMsg:\n%s", wParam, lParam ? (char*)lParam : "NULL");
	g_fNoProcessing = true;
	CallService(MS_CLIST_SETSTATUSMODE, wParam, 0);

	_ASSERT(!g_fNoProcessing && g_ProtoStates[(char*)NULL].Status == wParam);
	g_fNoProcessing = false;
	CProtoSettings(NULL, wParam).SetMsgFormat(SMF_TEMPORARY, lParam ? (TCHAR*)_A2T((char*)lParam) : CProtoSettings(NULL, wParam).GetMsgFormat(GMF_LASTORDEFAULT));
	ChangeProtoMessages(NULL, wParam, TCString());
	return 0;
}

// wParam = (WPARAM)(NAS_PROTOINFO*)pi - pointer to an array of NAS_PROTOINFO items to be filled.
// lParam = (LPARAM)(int)protoCount - number of items in pi.
// returns 0 on success
int GetState(WPARAM wParam, LPARAM lParam, int Widechar)
{
	NAS_PROTOINFO *pi = (NAS_PROTOINFO*)wParam;
	LogMessage("MS_NAS_GETSTATE called with %d items and Widechar=%d:", lParam, Widechar);
	for (int i = 0; i < lParam; i++) {
		if (pi->cbSize < sizeof(NAS_PROTOINFO) && pi->cbSize != sizeof(NAS_PROTOINFOv1))
			return 1;

		int Flags = (pi->cbSize > sizeof(NAS_PROTOINFOv1)) ? pi->Flags : 0;
		LogMessage("%d (received): cbSize=%d, status=%d, szProto=%s, Flags=0x%x", i + 1, pi->cbSize, pi->status, pi->szProto ? pi->szProto : "NULL", Flags);
		if ((pi->status >= ID_STATUS_ONLINE && pi->status <= ID_STATUS_OUTTOLUNCH) || !pi->status) {
			TCString Msg(pi->status ? CProtoSettings(pi->szProto, pi->status).GetMsgFormat(GMF_LASTORDEFAULT) : CProtoSettings(pi->szProto).GetMsgFormat(((Flags & PIF_NOTTEMPORARY) ? 0 : GMF_TEMPORARY) | GMF_PERSONAL));
			if (Msg != NULL) {
				if (Widechar)
					pi->tszMsg = mir_tstrdup(Msg);
				else
					pi->szMsg = mir_strdup(_T2A(Msg));
			}
			else pi->szMsg = NULL;

			if (!pi->status)
				pi->status = g_ProtoStates[pi->szProto].Status;
		}
		else pi->szMsg = NULL;

		LogMessage("%d (returned): status=%d, Flags=0x%x, szMsg:\n%s", i + 1, pi->status, (pi->cbSize > sizeof(NAS_PROTOINFOv1)) ? pi->Flags : 0, pi->szMsg ? (Widechar ? _T2A(pi->wszMsg) : pi->szMsg) : "NULL");
		*(char**)&pi += pi->cbSize;
	}
	return 0;
}

INT_PTR GetStateA(WPARAM wParam, LPARAM lParam)
{
	return GetState(wParam, lParam, false);
}

INT_PTR GetStateW(WPARAM wParam, LPARAM lParam)
{
	return GetState(wParam, lParam, true);
}

// wParam = (WPARAM)(NAS_PROTOINFO*)pi - pointer to an array of NAS_PROTOINFO items.
// lParam = (LPARAM)(int)protoCount - number of items in pi.
// returns 0 on success
int SetState(WPARAM wParam, LPARAM lParam, int Widechar)
{
	NAS_PROTOINFO *pi = (NAS_PROTOINFO*)wParam;
	LogMessage("MS_NAS_SETSTATE called with %d items and Widechar=%d:", lParam, Widechar);
	for (int i = 0; i < lParam; i++) {
		_ASSERT(pi->szMsg != (char*)(-1));
		if (pi->cbSize < sizeof(NAS_PROTOINFO) && pi->cbSize != sizeof(NAS_PROTOINFOv1))
			return 1;

		int Flags = (pi->cbSize > sizeof(NAS_PROTOINFOv1)) ? pi->Flags : 0;
		LogMessage("%d: cbSize=%d, status=%d, szProto=%s, Flags=0x%x, szMsg:\n%s", i + 1, pi->cbSize, pi->status, pi->szProto ? pi->szProto : "NULL", Flags, pi->szMsg ? (Widechar ? _T2A(pi->wszMsg) : pi->szMsg) : "NULL");
		if (pi->status)
			PSSetStatus(pi->szProto, pi->status, Flags & PIF_NO_CLIST_SETSTATUSMODE);
		else
			pi->status = g_ProtoStates[pi->szProto].Status;

		CProtoSettings(pi->szProto).SetMsgFormat((Flags & PIF_NOTTEMPORARY) ? SMF_PERSONAL : SMF_TEMPORARY, Widechar ? pi->wszMsg : _A2T(pi->szMsg));
		if (pi->szMsg || !(Flags & PIF_NO_CLIST_SETSTATUSMODE))
			ChangeProtoMessages(pi->szProto, pi->status, TCString());

		mir_free(pi->szMsg);
		pi->szMsg = (char*)(-1);
		*(char**)&pi += pi->cbSize;
	}
	return 0;
}

INT_PTR SetStateA(WPARAM wParam, LPARAM lParam)
{
	return SetState(wParam, lParam, false);
}

INT_PTR SetStateW(WPARAM wParam, LPARAM lParam)
{
	return SetState(wParam, lParam, true);
}

// wParam = (WPARAM)(NAS_ISWINFO*)iswi - pointer to a NAS_ISWINFO structure.
// lParam = 0
// returns HWND of the window on success, or NULL on failure.
INT_PTR InvokeStatusWindow(WPARAM wParam, LPARAM)
{
	NAS_ISWINFO *iswi = (NAS_ISWINFO*)wParam;
	if (iswi->cbSize != sizeof(NAS_ISWINFOv1) && iswi->cbSize < sizeof(NAS_ISWINFO))
		return NULL;

	LogMessage("MS_NAS_INVOKESTATUSWINDOW called. cbSize=%d, status=%d, szProto=%s, hContact=0x%08x, Flags=0x%x, szMsg:\n%s", iswi->cbSize, iswi->status, iswi->szProto ? iswi->szProto : "NULL", iswi->hContact, (iswi->cbSize < sizeof(NAS_ISWINFO)) ? 0 : iswi->Flags, iswi->szMsg ? ((iswi->Flags & ISWF_UNICODE) ? _T2A(iswi->wszMsg) : iswi->szMsg) : "NULL");
	if (iswi->status)
		PSSetStatus(iswi->szProto, iswi->status);

	if (g_SetAwayMsgPage.GetWnd()) { // already setting something
		SetForegroundWindow(g_SetAwayMsgPage.GetWnd());
		return (INT_PTR)g_SetAwayMsgPage.GetWnd();
	}
	SetAwayMsgData *dat = new SetAwayMsgData;
	memset(dat, 0, sizeof(SetAwayMsgData));
	dat->hInitContact = iswi->hContact;
	dat->szProtocol = iswi->szProto;
	dat->Message = (iswi->Flags & ISWF_UNICODE) ? iswi->wszMsg : _A2T(iswi->szMsg);
	dat->IsModeless = true;
	if (iswi->cbSize > sizeof(NAS_ISWINFOv1))
		dat->ISW_Flags = iswi->Flags;
	return (INT_PTR)CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_SETAWAYMSG), NULL, SetAwayMsgDlgProc, (LPARAM)dat);
}
