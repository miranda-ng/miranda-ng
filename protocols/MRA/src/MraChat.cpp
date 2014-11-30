#include "Mra.h"
#include "proto.h"

static LPWSTR lpwszStatuses[] = { L"Owners", L"Inviter", L"Visitors" };
#define MRA_CHAT_STATUS_OWNER	0
#define MRA_CHAT_STATUS_INVITER	1
#define MRA_CHAT_STATUS_VISITOR	2

void CMraProto::MraChatDllError()
{
	MessageBox(NULL, TranslateT("CHAT plugin is required for conferences. Install it before chatting"), m_tszUserName, (MB_OK | MB_ICONWARNING));
}

bool CMraProto::MraChatRegister()
{
	if (!ServiceExists(MS_GC_REGISTER))
		return FALSE;

	GCREGISTER gcr = { sizeof(gcr) };
	gcr.iMaxText = MRA_MAXLENOFMESSAGE;
	gcr.nColors = 0;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	CallServiceSync(MS_GC_REGISTER, NULL, (LPARAM)&gcr);

	HookProtoEvent(ME_GC_EVENT, &CMraProto::MraChatGcEventHook);
	return TRUE;
}

INT_PTR CMraProto::MraChatSessionNew(MCONTACT hContact)
{
	if (bChatExists)
	if (hContact) {
		CMStringW wszEMail;
		mraGetStringW(hContact, "e-mail", wszEMail);

		GCSESSION gcw = { sizeof(gcw) };
		gcw.iType = GCW_CHATROOM;
		gcw.pszModule = m_szModuleName;
		gcw.ptszName = GetContactNameW(hContact);
		gcw.ptszID = wszEMail;
		gcw.ptszStatusbarText = _T("status bar");
		gcw.dwItemData = (DWORD)hContact;
		if (!CallServiceSync(MS_GC_NEWSESSION, NULL, (LPARAM)&gcw)) {
			GCDEST gcd = { m_szModuleName, wszEMail.c_str(), GC_EVENT_ADDGROUP };
			GCEVENT gce = { sizeof(gce), &gcd };
			for (int i = 0; i < SIZEOF(lpwszStatuses); i++) {
				gce.ptszStatus = TranslateTS(lpwszStatuses[i]);
				CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);
			}

			gcd.iType = GC_EVENT_CONTROL;
			CallServiceSync(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce);
			CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);

			DWORD opcode = MULTICHAT_GET_MEMBERS;
			CMStringA szEmail;
			mraGetStringA(hContact, "e-mail", szEmail);
			MraMessage(FALSE, NULL, 0, MESSAGE_FLAG_MULTICHAT, szEmail, _T(""), (LPBYTE)&opcode, sizeof(opcode));
			return 0;
		}
	}
	return 1;
}

void CMraProto::MraChatSessionDestroy(MCONTACT hContact)
{
	if (!bChatExists || hContact == NULL)
		return;

	GCDEST gcd = { m_szModuleName, NULL, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };

	CMStringW wszEMail;
	mraGetStringW(hContact, "e-mail", wszEMail);
	gcd.ptszID = (LPWSTR)wszEMail.c_str();

	CallServiceSync(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);
	CallServiceSync(MS_GC_EVENT, WINDOW_CLEARLOG, (LPARAM)&gce);
}

INT_PTR CMraProto::MraChatSessionEventSendByHandle(MCONTACT hContactChatSession, DWORD dwType, DWORD dwFlags, const CMStringA &lpszUID, LPCWSTR lpwszStatus, LPCWSTR lpwszMessage, DWORD_PTR dwItemData, DWORD dwTime)
{
	if (!bChatExists)
		return 1;

	CMStringW wszID, wszUID, wszNick;

	GCDEST gcd = { m_szModuleName, 0, dwType };
	if (hContactChatSession) {
		mraGetStringW(hContactChatSession, "e-mail", wszID);
		gcd.ptszID = (LPWSTR)wszID.c_str();
	}

	GCEVENT gce = { sizeof(gce), &gcd };
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
			gce.ptszNick = GetContactNameW(hContactSender);
		else
			gce.ptszNick = wszUID;
	}

	return CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);
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
		return MraChatSessionEventSendByHandle(hContactChatSession, GC_EVENT_JOIN, GCEF_ADDTOLOG, lpszEMailInMultiChat, lpwszStatuses[MRA_CHAT_STATUS_VISITOR], _T(""), 0, dwTime);

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

		if (!_stricmp(gch->pDest->pszModule, m_szModuleName)) {
			switch (gch->pDest->iType) {
			case GC_USER_MESSAGE:
				if (gch->ptszText && mir_tstrlen(gch->ptszText)) {
					size_t dwMessageSize = mir_wstrlen(gch->ptszText);
					CMStringA szEmail = gch->pDest->ptszID;
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
	MCONTACT hContact = MraHContactFromEmail(lpszEMail, TRUE, TRUE, &bAdded);
	if (bAdded)
		db_set_b(hContact, "CList", "Hidden", 1);

	CallService(MS_IGNORE_UNIGNORE, hContact, IGNOREEVENT_ALL);
	CallService(MS_MSG_SENDMESSAGE, hContact, 0);
}
