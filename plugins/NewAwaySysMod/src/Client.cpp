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

#include "Common.h"
#include "MsgTree.h"
#include "Properties.h"

#define PARSE_INTERVAL 10000

HANDLE g_hTerminateUpdateMsgsThread = NULL;
HANDLE g_hUpdateMsgsThread = NULL;

void __cdecl UpdateMsgsThreadProc(void *)
{
	int ProtoCount;
	PROTOCOLDESCRIPTOR **proto;
	CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&ProtoCount, (LPARAM)&proto);
	int I;
	while (WaitForSingleObject(g_hTerminateUpdateMsgsThread, 0) == WAIT_TIMEOUT && !Miranda_Terminated())
	{
		DWORD MinUpdateTimeDifference = g_MoreOptPage.GetDBValueCopy(IDC_MOREOPTDLG_UPDATEMSGSPERIOD) * 1000; // in milliseconds
		for (I = 0; I < ProtoCount; I++)
		{
			if (proto[I]->type == PROTOTYPE_PROTOCOL && CallProtoService(proto[I]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND && !IsAnICQProto(proto[I]->szName))
			{
				int Status = CallProtoService(proto[I]->szName, PS_GETSTATUS, 0, 0);
				if (Status < ID_STATUS_OFFLINE || Status > ID_STATUS_OUTTOLUNCH)
				{
					Status = g_ProtoStates[proto[I]->szName].Status;
				}
				if (CallProtoService(proto[I]->szName, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(Status) && g_ProtoStates[proto[I]->szName].CurStatusMsg.GetUpdateTimeDifference() >= MinUpdateTimeDifference)
				{
					TCString CurMsg(GetDynamicStatMsg(INVALID_CONTACT_ID, proto[I]->szName));
					if ((TCString)g_ProtoStates[proto[I]->szName].CurStatusMsg != (const TCHAR*)CurMsg) // if the message has changed
					{
						g_ProtoStates[proto[I]->szName].CurStatusMsg = CurMsg;
						CallAllowedPS_SETAWAYMSG(proto[I]->szName, Status, (char*)_T2A(CurMsg));
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
	int UpdateMsgs = g_MoreOptPage.GetDBValueCopy(IDC_MOREOPTDLG_UPDATEMSGS);
	if (g_hUpdateMsgsThread && !UpdateMsgs)
	{
		_ASSERT(WaitForSingleObject(g_hUpdateMsgsThread, 0) == WAIT_TIMEOUT);
		SetEvent(g_hTerminateUpdateMsgsThread);
		QueueUserAPC(DummyAPCFunc, g_hUpdateMsgsThread, 0); // wake up the thread, as it's most probably in SleepEx() now
		WaitForSingleObject(g_hUpdateMsgsThread, INFINITE);
		g_hUpdateMsgsThread = NULL;
		CloseHandle(g_hTerminateUpdateMsgsThread);
	} else if (!g_hUpdateMsgsThread && UpdateMsgs)
	{
		g_hTerminateUpdateMsgsThread = CreateEvent(NULL, TRUE, FALSE, NULL);
		g_hUpdateMsgsThread = (HANDLE)mir_forkthread(UpdateMsgsThreadProc, NULL);
	}
}


void ChangeProtoMessages(char* szProto, int iMode, TCString &Msg)
{
	TCString CurMsg(Msg);
	if (szProto)
	{
		if (Msg == NULL)
		{
			CurMsg = GetDynamicStatMsg(INVALID_CONTACT_ID, szProto);
		}
		CallAllowedPS_SETAWAYMSG(szProto, iMode, (char*)_T2A(CurMsg));
		g_ProtoStates[szProto].CurStatusMsg = CurMsg;
	} else // change message of all protocols
	{
		int ProtoCount;
		PROTOCOLDESCRIPTOR **proto;
		CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&ProtoCount, (LPARAM)&proto);
		int I;
		for (I = 0; I < ProtoCount; I++)
		{
			if (proto[I]->type == PROTOTYPE_PROTOCOL && !db_get_b(NULL, proto[I]->szName, "LockMainStatus", 0))
			{
				if (Msg == NULL)
				{
					CurMsg = GetDynamicStatMsg(INVALID_CONTACT_ID, proto[I]->szName);
				}
				CallAllowedPS_SETAWAYMSG(proto[I]->szName, iMode, (char*)_T2A(CurMsg));
				g_ProtoStates[proto[I]->szName].CurStatusMsg = CurMsg;
			}
		}
	}
	static struct
	{
		int Status;
		char *Setting;
	} StatusSettings[] = {
		ID_STATUS_OFFLINE, "Off",
		ID_STATUS_ONLINE, "On",
		ID_STATUS_AWAY, "Away",
		ID_STATUS_NA, "Na",
		ID_STATUS_DND, "Dnd",
		ID_STATUS_OCCUPIED, "Occupied",
		ID_STATUS_FREECHAT, "FreeChat",
		ID_STATUS_INVISIBLE, "Inv",
		ID_STATUS_ONTHEPHONE, "Otp",
		ID_STATUS_OUTTOLUNCH, "Otl",
		ID_STATUS_IDLE, "Idl"
	};
	int I;
	for (I = 0; I < lengthof(StatusSettings); I++)
	{
		if (iMode == StatusSettings[I].Status)
		{
			db_set_ts(NULL, "SRAway", CString(StatusSettings[I].Setting) + "Msg", CurMsg);
			db_set_ts(NULL, "SRAway", CString(StatusSettings[I].Setting) + "Default", CurMsg); // TODO: make it more accurate, and change not only here, but when changing status messages through UpdateMsgsTimerFunc too; and when changing messages through AutoAway() ?
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
	TreeCtrl->DBToMem(CString(MOD_NAME));
	int Order;
	if (g_MoreOptPage.GetDBValueCopy(IDC_MOREOPTDLG_PERSTATUSMRM))
	{
		for (Order = 0; Order < TreeCtrl->Value.GetSize(); Order++) // find a group named accordingly to the current status
		{
			if (TreeCtrl->Value[Order].ParentID == g_Messages_RecentRootID && TreeCtrl->Value[Order].Flags & TIF_GROUP && !_tcsicmp(TreeCtrl->Value[Order].Title, iMode ? (TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, iMode, GSMDF_TCHAR) : MSGTREE_RECENT_OTHERGROUP))
			{
				return TreeCtrl->Value[Order].ID;
			}
		}
	} else // simply use Recent Messages category
	{
		return g_Messages_RecentRootID;
	}
	return -1;
}


int ICQStatusToGeneralStatus(int bICQStat)
{
	switch (bICQStat)
	{
		case MTYPE_AUTOONLINE: return ID_STATUS_ONLINE;
		case MTYPE_AUTOAWAY: return ID_STATUS_AWAY;
		case MTYPE_AUTONA: return ID_STATUS_NA;
		case MTYPE_AUTODND: return ID_STATUS_DND;
		case MTYPE_AUTOBUSY: return ID_STATUS_OCCUPIED;
		case MTYPE_AUTOFFC: return ID_STATUS_FREECHAT;
		default: return 0;
	}
}


TCString VariablesEscape(TCString Str)
{
	if (!Str.GetLen())
	{
		return _T("");
	}
	enum eState
	{
		ST_TEXT, ST_QUOTE
	};
	eState State = ST_QUOTE;
	TCString Result(_T("`"));
	const TCHAR *p = Str;
	while (*p)
	{
		if (*p == '`')
		{
			if (State == ST_TEXT)
			{
				Result += _T("````");
				State = ST_QUOTE;
			} else
			{
				Result += _T("``");
			}
		} else
		{
			Result += *p;
			State = ST_TEXT;
		}
		p++;
	}
	if (State == ST_QUOTE)
	{
		Result.GetBuffer()[Result.GetLen() - 1] = '\0';
		Result.ReleaseBuffer();
	} else
	{
		Result += '`';
	}
	return Result;
}
