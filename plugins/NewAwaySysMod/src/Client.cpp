/*
	New Away System - plugin for Miranda IM
	Copyright (c) 2005-2007 Chervov Dmitry
	Copyright (c) 2004-2005 Iksaif Entertainment
	Copyright (c) 2002-2003 Goblineye Entertainment

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
#include "MsgTree.h"
#include "Properties.h"

#define PARSE_INTERVAL 10000

HANDLE g_hTerminateUpdateMsgsThread = nullptr;
HANDLE g_hUpdateMsgsThread = nullptr;

void __cdecl UpdateMsgsThreadProc(void *)
{
	Thread_SetName("NewAwaySysMod: UpdateMsgsThreadProc");

	while (WaitForSingleObject(g_hTerminateUpdateMsgsThread, 0) == WAIT_TIMEOUT && !Miranda_IsTerminated()) {
		uint32_t MinUpdateTimeDifference = (uint32_t)g_MoreOptPage.GetDBValueCopy(IDC_MOREOPTDLG_UPDATEMSGSPERIOD) * 1000; // in milliseconds
		for (auto &p : Accounts()) {
			if (CallProtoService(p->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) {
				int Status = Proto_GetStatus(p->szModuleName);
				if (Status < ID_STATUS_OFFLINE || Status > ID_STATUS_MAX) {
					Status = g_ProtoStates[p->szModuleName].m_status;
				}
				if (CallProtoService(p->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(Status) && g_ProtoStates[p->szModuleName].CurStatusMsg.GetUpdateTimeDifference() >= MinUpdateTimeDifference) {
					TCString CurMsg(GetDynamicStatMsg(INVALID_CONTACT_ID, p->szModuleName));
					if ((TCString)g_ProtoStates[p->szModuleName].CurStatusMsg != (const wchar_t*)CurMsg) { // if the message has changed
						g_ProtoStates[p->szModuleName].CurStatusMsg = CurMsg;
						CallAllowedPS_SETAWAYMSG(p->szModuleName, Status, CurMsg);
					}
				}
			}
		}
		SleepEx(PARSE_INTERVAL, true);
	}
}

static void __stdcall DummyAPCFunc(ULONG_PTR)
{
	return;
}

void InitUpdateMsgs()
{
	int UpdateMsgs = (int)g_MoreOptPage.GetDBValueCopy(IDC_MOREOPTDLG_UPDATEMSGS);
	if (g_hUpdateMsgsThread && !UpdateMsgs) {
		_ASSERT(WaitForSingleObject(g_hUpdateMsgsThread, 0) == WAIT_TIMEOUT);
		SetEvent(g_hTerminateUpdateMsgsThread);
		QueueUserAPC(DummyAPCFunc, g_hUpdateMsgsThread, 0); // wake up the thread, as it's most probably in SleepEx() now
		WaitForSingleObject(g_hUpdateMsgsThread, INFINITE);
		g_hUpdateMsgsThread = nullptr;
		CloseHandle(g_hTerminateUpdateMsgsThread);
	}
	else if (!g_hUpdateMsgsThread && UpdateMsgs) {
		g_hTerminateUpdateMsgsThread = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		g_hUpdateMsgsThread = mir_forkthread(UpdateMsgsThreadProc);
	}
}


void ChangeProtoMessages(char* szProto, int iMode, const TCString &Msg)
{
	TCString CurMsg(Msg);
	if (szProto) {
		if (Msg == nullptr)
			CurMsg = GetDynamicStatMsg(INVALID_CONTACT_ID, szProto);

		CallAllowedPS_SETAWAYMSG(szProto, iMode, CurMsg);
		g_ProtoStates[szProto].CurStatusMsg = CurMsg;
	}
	else { // change message of all protocols
		for (auto &pa : Accounts()) {
			if (!db_get_b(0, pa->szModuleName, "LockMainStatus", 0)) {
				if (Msg == nullptr)
					CurMsg = GetDynamicStatMsg(INVALID_CONTACT_ID, pa->szModuleName);

				CallAllowedPS_SETAWAYMSG(pa->szModuleName, iMode, CurMsg);
				g_ProtoStates[pa->szModuleName].CurStatusMsg = CurMsg;
			}
		}
	}

	static struct
	{
		int Status;
		char *Setting;
	}
	StatusDbSettings[] = {
		ID_STATUS_OFFLINE, "Off",
		ID_STATUS_ONLINE, "On",
		ID_STATUS_AWAY, "Away",
		ID_STATUS_NA, "Na",
		ID_STATUS_DND, "Dnd",
		ID_STATUS_OCCUPIED, "Occupied",
		ID_STATUS_FREECHAT, "FreeChat",
		ID_STATUS_INVISIBLE, "Inv",
		ID_STATUS_IDLE, "Idl"
	};

	for (auto &it: StatusDbSettings) {
		if (iMode == it.Status) {
			db_set_ws(0, "SRAway", CString(it.Setting) + "Msg", CurMsg);
			db_set_ws(0, "SRAway", CString(it.Setting) + "Default", CurMsg);
			// TODO: make it more accurate, and change not only here, but when changing status messages through UpdateMsgsTimerFunc too; and when changing messages through AutoAway() ?
			break;
		}
	}
	//	InitUpdateMsgs();
}


int GetRecentGroupID(int iMode)
{ // returns an ID of a group where recent messages are stored, accordingly to current settings and status mode.
	// -1 if the group is not found
	//	COptPage MoreOptData(g_MoreOptPage);
	COptPage MsgTreeData(g_MsgTreePage);
	COptItem_TreeCtrl *TreeCtrl = (COptItem_TreeCtrl*)MsgTreeData.Find(IDV_MSGTREE);
	TreeCtrl->DBToMem(CString(MODULENAME));

	if (!g_MoreOptPage.GetDBValueCopy(IDC_MOREOPTDLG_PERSTATUSMRM))
		return g_Messages_RecentRootID;

	for (int Order = 0; Order < TreeCtrl->m_value.GetSize(); Order++) // find a group named accordingly to the current status
		if (TreeCtrl->m_value[Order].ParentID == g_Messages_RecentRootID && TreeCtrl->m_value[Order].Flags & TIF_GROUP && !mir_wstrcmpi(TreeCtrl->m_value[Order].Title, iMode ? Clist_GetStatusModeDescription(iMode, 0) : MSGTREE_RECENT_OTHERGROUP))
			return TreeCtrl->m_value[Order].ID;

	return -1;
}

TCString VariablesEscape(TCString Str)
{
	if (!Str.GetLen()) {
		return L"";
	}
	enum eState
	{
		ST_TEXT, ST_QUOTE
	};
	eState State = ST_QUOTE;
	TCString Result(L"`");
	const wchar_t *p = Str;
	while (*p) {
		if (*p == '`') {
			if (State == ST_TEXT) {
				Result += L"````";
				State = ST_QUOTE;
			}
			else {
				Result += L"``";
			}
		}
		else {
			Result += *p;
			State = ST_TEXT;
		}
		p++;
	}
	if (State == ST_QUOTE) {
		Result.GetBuffer()[Result.GetLen() - 1] = '\0';
		Result.ReleaseBuffer();
	}
	else {
		Result += '`';
	}
	return Result;
}
