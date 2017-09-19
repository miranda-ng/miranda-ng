#include "stdafx.h"
#include "proto.h"

static LPWSTR lpwszStatuses[] = { LPGENW("Owners"), LPGENW("Inviter"), LPGENW("Visitors") };
#define MRA_CHAT_STATUS_OWNER	0
#define MRA_CHAT_STATUS_INVITER	1
#define MRA_CHAT_STATUS_VISITOR	2


bool CMraProto::MraChatRegister()
{
	GCREGISTER gcr = {};
	gcr.iMaxText = MRA_MAXLENOFMESSAGE;
	gcr.nColors = 0;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);

	HookProtoEvent(ME_GC_EVENT, &CMraProto::MraChatGcEventHook);
	return TRUE;
}

INT_PTR CMraProto::MraChatSessionNew(MCONTACT hContact)
{
	if (bChatExists)
	if (!hContact)
		return 1;
		
	CMStringW wszEMail;
	mraGetStringW(hContact, "e-mail", wszEMail);

	if (Chat_NewSession(GCW_CHATROOM, m_szModuleName, wszEMail, pcli->pfnGetContactDisplayName(hContact, 0), (void*)hContact))
		return 1;

	for (int i = 0; i < _countof(lpwszStatuses); i++)
		Chat_AddGroup(m_szModuleName, wszEMail, TranslateW(lpwszStatuses[i]));

	Chat_Control(m_szModuleName, wszEMail, SESSION_INITDONE);
	Chat_Control(m_szModuleName, wszEMail, SESSION_ONLINE);

	DWORD opcode = MULTICHAT_GET_MEMBERS;
	CMStringA szEmail;
	if (mraGetStringA(hContact, "e-mail", szEmail))
		MraMessage(FALSE, NULL, 0, MESSAGE_FLAG_MULTICHAT, szEmail, L"", (LPBYTE)&opcode, sizeof(opcode));
	return 0;
}

void CMraProto::MraChatSessionDestroy(MCONTACT hContact)
{
	if (!bChatExists || hContact == NULL)
		return;

	CMStringW wszEMail;
	mraGetStringW(hContact, "e-mail", wszEMail);

	Chat_Terminate(m_szModuleName, wszEMail);
	Chat_Control(m_szModuleName, wszEMail, WINDOW_CLEARLOG);
}

INT_PTR CMraProto::MraChatSessionEventSendByHandle(MCONTACT hContactChatSession, int iType, DWORD dwFlags, const CMStringA &lpszUID, LPCWSTR lpwszStatus, LPCWSTR lpwszMessage, DWORD_PTR dwItemData, DWORD dwTime)
{
	if (!bChatExists)
		return 1;

	CMStringW wszID, wszUID, wszNick;

	GCEVENT gce = { m_szModuleName, 0, iType };
	if (hContactChatSession) {
		mraGetStringW(hContactChatSession, "e-mail", wszID);
		gce.ptszID = wszID.c_str();
	}

	gce.dwFlags = dwFlags;
	gce.ptszUID = wszUID;
	gce.ptszStatus = lpwszStatus;
	gce.ptszText = lpwszMessage;
	gce.dwItemData = dwItemData;
	gce.time = dwTime;

	if (!lpszUID.IsEmpty())
		gce.bIsMe = IsEMailMy(lpszUID);
	else
		gce.bIsMe = TRUE;

	if (gce.bIsMe) {
		mraGetStringW(NULL, "e-mail", wszUID);
		mraGetStringW(NULL, "Nick", wszNick);
		gce.ptszNick = wszNick;
	}
	else {
		MCONTACT hContactSender = MraHContactFromEmail(lpszUID, FALSE, TRUE, NULL);
		wszUID = lpszUID;
		if (hContactSender)
			gce.ptszNick = pcli->pfnGetContactDisplayName(hContactSender, 0);
		else
			gce.ptszNick = wszUID;
	}

	return Chat_Event(&gce);
}

INT_PTR CMraProto::MraChatSessionInvite(MCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime)
{
	if (!hContactChatSession)
		return 1;

	CMStringW wszBuff;
	wszBuff.Format(L"[%s]: %s", _A2T(lpszEMailInMultiChat.c_str()), TranslateT("invite sender"));
	return MraChatSessionEventSendByHandle(hContactChatSession, GC_EVENT_ACTION, GCEF_ADDTOLOG, lpszEMailInMultiChat, NULL, wszBuff, 0, dwTime);
}

INT_PTR CMraProto::MraChatSessionMembersAdd(MCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime)
{
	if (!hContactChatSession)
		return 1;

	CMStringW wszBuff;
	wszBuff.Format(L"[%s]: %s", _A2T(lpszEMailInMultiChat), TranslateT("invite new members"));
	return MraChatSessionEventSendByHandle(hContactChatSession, GC_EVENT_ACTION, GCEF_ADDTOLOG, lpszEMailInMultiChat, NULL, wszBuff, 0, dwTime);
}

INT_PTR CMraProto::MraChatSessionJoinUser(MCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime)
{
	if (hContactChatSession)
		return MraChatSessionEventSendByHandle(hContactChatSession, GC_EVENT_JOIN, GCEF_ADDTOLOG, lpszEMailInMultiChat, lpwszStatuses[MRA_CHAT_STATUS_VISITOR], L"", 0, dwTime);

	return 1;
}

INT_PTR CMraProto::MraChatSessionLeftUser(MCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime)
{
	if (hContactChatSession)
		return MraChatSessionEventSendByHandle(hContactChatSession, GC_EVENT_PART, GCEF_ADDTOLOG, lpszEMailInMultiChat, NULL, NULL, 0, dwTime);

	return 1;
}

INT_PTR CMraProto::MraChatSessionSetIviter(MCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat)
{
	if (hContactChatSession && !lpszEMailInMultiChat.IsEmpty())
		return MraChatSessionEventSendByHandle(hContactChatSession, GC_EVENT_ADDSTATUS, 0, lpszEMailInMultiChat, lpwszStatuses[MRA_CHAT_STATUS_INVITER], NULL, 0, 0);

	return 1;
}

INT_PTR CMraProto::MraChatSessionSetOwner(MCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat)
{
	if (hContactChatSession && !lpszEMailInMultiChat.IsEmpty())
		return MraChatSessionEventSendByHandle(hContactChatSession, GC_EVENT_ADDSTATUS, 0, lpszEMailInMultiChat, lpwszStatuses[MRA_CHAT_STATUS_OWNER], NULL, 0, 0);

	return 1;
}


INT_PTR CMraProto::MraChatSessionMessageAdd(MCONTACT hContactChatSession, const CMStringA &lpszEMailInMultiChat, const CMStringW &lpwszMessage, DWORD dwTime)
{
	if (hContactChatSession)
		return MraChatSessionEventSendByHandle(hContactChatSession, GC_EVENT_MESSAGE, GCEF_ADDTOLOG, lpszEMailInMultiChat, NULL, lpwszMessage, 0, dwTime);

	return 1;
}

int CMraProto::MraChatGcEventHook(WPARAM, LPARAM lParam)
{
	if (lParam)
	if (bChatExists) {
		GCHOOK* gch = (GCHOOK*)lParam;

		if (!_stricmp(gch->pszModule, m_szModuleName)) {
			switch (gch->iType) {
			case GC_USER_MESSAGE:
				if (gch->ptszText && mir_wstrlen(gch->ptszText)) {
					CMStringA szEmail = gch->ptszID;
					MCONTACT hContact = MraHContactFromEmail(szEmail, FALSE, TRUE, NULL);
					BOOL bSlowSend = getByte("SlowSend", MRA_DEFAULT_SLOW_SEND);

					DWORD dwFlags = 0;
					if (getByte("RTFSendEnable", MRA_DEFAULT_RTF_SEND_ENABLE) && (MraContactCapabilitiesGet(hContact)&FEATURE_FLAG_RTF_MESSAGE))
						dwFlags |= MESSAGE_FLAG_RTF;

					INT_PTR iSendRet = MraMessage(bSlowSend, hContact, ACKTYPE_MESSAGE, dwFlags, szEmail, gch->ptszText, NULL, 0);
					if (bSlowSend == FALSE)
						ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)iSendRet, 0);

					MraChatSessionEventSendByHandle(hContact, GC_EVENT_MESSAGE, GCEF_ADDTOLOG, "", NULL, gch->ptszText, 0, (DWORD)_time32(NULL));
				}
				break;

			case GC_USER_PRIVMESS:
				MraChatSendPrivateMessage(_T2A(gch->ptszUID));
				break;

			case GC_USER_LOGMENU:
				//sttLogListHook( this, item, gch );
				break;

			case GC_USER_NICKLISTMENU:
				//sttNickListHook( this, item, gch );
				break;

			case GC_USER_CHANMGR:
				//int iqId = SerialNext();
				//IqAdd( iqId, IQ_PROC_NONE, &CJabberProto::OnIqResultGetMuc );
				//m_ThreadInfo->send( XmlNodeIq( L"get", iqId, item->jid ) << XQUERY( xmlnsOwner ));
				break;
			}
		}
	}
	return 0;
}

void CMraProto::MraChatSendPrivateMessage(LPSTR lpszEMail)
{
	BOOL bAdded;
	MCONTACT hContact = MraHContactFromEmail(lpszEMail, TRUE, TRUE, &bAdded);
	if (bAdded)
		db_set_b(hContact, "CList", "Hidden", 1);

	CallService(MS_IGNORE_UNIGNORE, hContact, IGNOREEVENT_ALL);
	CallService(MS_MSG_SENDMESSAGE, hContact, 0);
}
