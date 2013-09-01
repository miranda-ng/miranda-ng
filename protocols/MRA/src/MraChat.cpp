#include "Mra.h"
#include "MraChat.h"
#include "proto.h"

static LPWSTR lpwszStatuses[] = { L"Owners", L"Inviter", L"Visitors" };
#define MRA_CHAT_STATUS_OWNER	0
#define MRA_CHAT_STATUS_INVITER	1
#define MRA_CHAT_STATUS_VISITOR	2

void CMraProto::MraChatDllError()
{
	MessageBoxW(NULL, TranslateW(L"CHAT plugin is required for conferences. Install it before chatting"), m_tszUserName, (MB_OK|MB_ICONWARNING));
}

bool CMraProto::MraChatRegister()
{
	if ( !ServiceExists(MS_GC_REGISTER))
		return FALSE;

	GCREGISTER gcr = {0};
	gcr.cbSize = sizeof(GCREGISTER);
	gcr.dwFlags = GC_UNICODE;
	gcr.iMaxText = MRA_MAXLENOFMESSAGE;
	gcr.nColors = 0;
	gcr.ptszModuleDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	CallServiceSync(MS_GC_REGISTER, NULL, (LPARAM)&gcr);

	HookProtoEvent(ME_GC_EVENT, &CMraProto::MraChatGcEventHook);
	return TRUE;
}

INT_PTR CMraProto::MraChatSessionNew(HANDLE hContact)
{
	if (bChatExists)
	if (hContact) {
		GCSESSION gcw = {0};
		CMStringW wszEMail;
		mraGetStringW(hContact, "e-mail", wszEMail);

		gcw.cbSize = sizeof(GCSESSION);
		gcw.iType = GCW_CHATROOM;
		gcw.pszModule = m_szModuleName;
		gcw.ptszName = GetContactNameW(hContact);
		gcw.ptszID = wszEMail;
		gcw.ptszStatusbarText = L"status bar";
		gcw.dwFlags = GC_UNICODE;
		gcw.dwItemData = (DWORD)hContact;
		if ( !CallServiceSync(MS_GC_NEWSESSION, NULL, (LPARAM)&gcw)) {
			GCDEST gcd = {0};
			GCEVENT gce = {0};

			gcd.pszModule = m_szModuleName;
			gcd.ptszID = (TCHAR*)wszEMail.c_str();
			gcd.iType = GC_EVENT_ADDGROUP;

			gce.cbSize = sizeof(GCEVENT);
			gce.pDest = &gcd;
			gce.dwFlags = GC_UNICODE;
			for (int i = 0; i < SIZEOF(lpwszStatuses); i++) {
				gce.ptszStatus = TranslateW(lpwszStatuses[i]);
				CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);
			}

			gce.cbSize = sizeof(GCEVENT);
			gce.pDest = &gcd;
			gcd.iType = GC_EVENT_CONTROL;

			CallServiceSync(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce);
			CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);

			DWORD opcode = MULTICHAT_GET_MEMBERS;
			CMStringA szEmail;
			mraGetStringA(hContact, "e-mail", szEmail);
			MraMessageW(FALSE, NULL, 0, MESSAGE_FLAG_MULTICHAT, szEmail, L"", (LPBYTE)&opcode, sizeof(opcode));
			return 0;
		}
	}
	return 1;
}

void CMraProto::MraChatSessionDestroy(HANDLE hContact)
{
	if ( !bChatExists)
		return;

	GCDEST gcd = {0};
	GCEVENT gce = {0};
	CMStringW wszEMail;

	gcd.pszModule = m_szModuleName;
	gcd.iType = GC_EVENT_CONTROL;
	if (hContact) {
		mraGetStringW(hContact, "e-mail", wszEMail);
		gcd.ptszID = (LPWSTR)wszEMail.c_str();
	}
	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	gce.dwFlags = GC_UNICODE;

	CallServiceSync(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);
	CallServiceSync(MS_GC_EVENT, WINDOW_CLEARLOG, (LPARAM)&gce);
}

INT_PTR CMraProto::MraChatSessionEventSendByHandle(HANDLE hContactChatSession, DWORD dwType, DWORD dwFlags, const CMStringA &lpszUID, LPCWSTR lpwszStatus, LPCWSTR lpwszMessage, DWORD_PTR dwItemData, DWORD dwTime)
{
	if (!bChatExists)
		return 1;

	CMStringW wszID, wszUID, wszNick;

	GCDEST gcd = {0};
	gcd.pszModule = m_szModuleName;
	if (hContactChatSession) {
		mraGetStringW(hContactChatSession, "e-mail", wszID);
		gcd.ptszID = (LPWSTR)wszID.c_str();
	}
	gcd.iType = dwType;

	GCEVENT gce = {0};
	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	gce.dwFlags = GC_UNICODE|dwFlags;
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
		HANDLE hContactSender = MraHContactFromEmail(lpszUID, FALSE, TRUE, NULL);
		wszUID = lpszUID;
		if (hContactSender)
			gce.ptszNick = GetContactNameW(hContactSender);
		else
			gce.ptszNick = wszUID;
	}

	return CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);
}

INT_PTR CMraProto::MraChatSessionInvite(HANDLE hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime)
{
	if (!hContactChatSession)
		return 1;

	CMStringW wszBuff;
	wszBuff.Format(L"[%s]: %s", _A2T(lpszEMailInMultiChat.c_str()), TranslateW(L"invite sender"));
	return MraChatSessionEventSendByHandle(hContactChatSession, GC_EVENT_ACTION, GCEF_ADDTOLOG, lpszEMailInMultiChat, NULL, wszBuff, 0, dwTime);
}

INT_PTR CMraProto::MraChatSessionMembersAdd(HANDLE hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime)
{
	if (!hContactChatSession)
		return 1;

	CMStringW wszBuff;
	wszBuff.Format(L"[%s]: %s", _A2T(lpszEMailInMultiChat), TranslateW(L"invite new members"));
	return MraChatSessionEventSendByHandle(hContactChatSession, GC_EVENT_ACTION, GCEF_ADDTOLOG, lpszEMailInMultiChat, NULL, wszBuff, 0, dwTime);
}

INT_PTR CMraProto::MraChatSessionJoinUser(HANDLE hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime)
{
	if (hContactChatSession)
		return MraChatSessionEventSendByHandle(hContactChatSession, GC_EVENT_JOIN, GCEF_ADDTOLOG, lpszEMailInMultiChat, lpwszStatuses[MRA_CHAT_STATUS_VISITOR], L"", 0, dwTime);

	return 1;
}

INT_PTR CMraProto::MraChatSessionLeftUser(HANDLE hContactChatSession, const CMStringA &lpszEMailInMultiChat, DWORD dwTime)
{
	if (hContactChatSession)
		return MraChatSessionEventSendByHandle(hContactChatSession, GC_EVENT_PART, GCEF_ADDTOLOG, lpszEMailInMultiChat, NULL, NULL, 0, dwTime);

	return 1;
}

INT_PTR CMraProto::MraChatSessionSetIviter(HANDLE hContactChatSession, const CMStringA &lpszEMailInMultiChat)
{
	if (hContactChatSession && !lpszEMailInMultiChat.IsEmpty())
		return MraChatSessionEventSendByHandle(hContactChatSession, GC_EVENT_ADDSTATUS, 0, lpszEMailInMultiChat, lpwszStatuses[MRA_CHAT_STATUS_INVITER], NULL, 0, 0);

	return 1;
}

INT_PTR CMraProto::MraChatSessionSetOwner(HANDLE hContactChatSession, const CMStringA &lpszEMailInMultiChat)
{
	if (hContactChatSession && !lpszEMailInMultiChat.IsEmpty())
		return MraChatSessionEventSendByHandle(hContactChatSession, GC_EVENT_ADDSTATUS, 0, lpszEMailInMultiChat, lpwszStatuses[MRA_CHAT_STATUS_OWNER], NULL, 0, 0);

	return 1;
}


INT_PTR CMraProto::MraChatSessionMessageAdd(HANDLE hContactChatSession, const CMStringA &lpszEMailInMultiChat, const CMStringW &lpwszMessage, DWORD dwTime)
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

		if ( !_stricmp(gch->pDest->pszModule, m_szModuleName)) {
			switch (gch->pDest->iType) {
			case GC_USER_MESSAGE:
				if (gch->ptszText && lstrlen(gch->ptszText)) {
					size_t dwMessageSize = lstrlenW(gch->ptszText);
					CMStringA szEmail = gch->pDest->ptszID;
					HANDLE hContact = MraHContactFromEmail(szEmail, FALSE, TRUE, NULL);
					BOOL bSlowSend = getByte("SlowSend", MRA_DEFAULT_SLOW_SEND);

					DWORD dwFlags = 0;
					if (getByte("RTFSendEnable", MRA_DEFAULT_RTF_SEND_ENABLE) && (MraContactCapabilitiesGet(hContact)&FEATURE_FLAG_RTF_MESSAGE))
						dwFlags |= MESSAGE_FLAG_RTF;

					INT_PTR iSendRet = MraMessageW(bSlowSend, hContact, ACKTYPE_MESSAGE, dwFlags, szEmail, gch->ptszText, NULL, 0);
					if (bSlowSend == FALSE)
						ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)iSendRet, 0);

					MraChatSessionEventSendByHandle(hContact, GC_EVENT_MESSAGE, GCEF_ADDTOLOG, "", NULL, gch->ptszText, 0, (DWORD)_time32(NULL));
				}
				break;

			case GC_USER_PRIVMESS:
				MraChatSendPrivateMessage( _T2A(gch->ptszUID));
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
				//m_ThreadInfo->send( XmlNodeIq( _T("get"), iqId, item->jid ) << XQUERY( xmlnsOwner ));
				break;
			}
		}
	}
	return 0;
}

void CMraProto::MraChatSendPrivateMessage(LPSTR lpszEMail)
{
	BOOL bAdded;
	HANDLE hContact = MraHContactFromEmail(lpszEMail, TRUE, TRUE, &bAdded);
	if (bAdded)
		db_set_b(hContact, "CList", "Hidden", 1);

	CallService(MS_IGNORE_UNIGNORE, (WPARAM)hContact, IGNOREEVENT_ALL);
	CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact, 0);
}
