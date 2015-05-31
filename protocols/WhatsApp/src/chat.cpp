#include "common.h"

static const TCHAR *sttStatuses[] = { LPGENT("Members"), LPGENT("Owners") };

enum
{
	IDM_CANCEL,

	IDM_INVITE, IDM_LEAVE, IDM_TOPIC,

	IDM_AVATAR, IDM_KICK,
	IDM_CPY_NICK, IDM_CPY_TOPIC,
	IDM_ADD_RJID, IDM_CPY_RJID
};

/////////////////////////////////////////////////////////////////////////////////////////
// protocol menu handler - create a new group

INT_PTR __cdecl WhatsAppProto::OnCreateGroup(WPARAM wParam, LPARAM lParam)
{
	ENTER_STRING es = { 0 };
	es.cbSize = sizeof(es);
	es.type = ESF_MULTILINE;
	es.caption = _T("Enter a subject for new group");
	es.szModuleName = m_szModuleName;
	if (EnterString(&es)) {
		if (isOnline()) {
			std::string groupName(T2Utf(es.ptszResult));
			m_pConnection->sendCreateGroupChat(groupName);
		}
		mir_free(es.ptszResult);
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// handler to pass events from SRMM to WAConnection

int WhatsAppProto::onGroupChatEvent(WPARAM wParam, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK*)lParam;
	if (mir_strcmp(gch->pDest->pszModule, m_szModuleName))
		return 0;

	std::string chat_id(T2Utf(gch->pDest->ptszID));
	WAChatInfo *pInfo = SafeGetChat(chat_id);
	if (pInfo == NULL)
		return 0;

	switch (gch->pDest->iType) {
	case GC_USER_LOGMENU:
		ChatLogMenuHook(pInfo, gch);
		break;

	case GC_USER_NICKLISTMENU:
		NickListMenuHook(pInfo, gch);
		break;

	case GC_USER_MESSAGE:
		if (isOnline()) {
			std::string msg(T2Utf(gch->ptszText));
			
			try {
				int msgId = GetSerial();
				time_t now = time(NULL);
				std::string id = Utilities::intToStr(now) + "-" + Utilities::intToStr(msgId);

				FMessage fmsg(chat_id, true, id);
				fmsg.timestamp = now;
				fmsg.data = msg;
				m_pConnection->sendMessage(&fmsg);

				pInfo->m_unsentMsgs[id] = gch->ptszText;
			}
			CODE_BLOCK_CATCH_ALL
		}
		break;

	case GC_USER_PRIVMESS:
		string jid = string(_T2A(gch->ptszUID)) + "@s.whatsapp.net";
		MCONTACT hContact = ContactIDToHContact(jid);
		if (hContact == 0) {
			hContact = AddToContactList(jid, (char*)_T2A(gch->ptszUID));
			setWord(hContact, "Status", ID_STATUS_ONLINE);

			db_set_b(hContact, "CList", "Hidden", 1);
			setTString(hContact, "Nick", gch->ptszUID);
			db_set_dw(hContact, "Ignore", "Mask1", 0);
		}
		CallService(MS_MSG_SENDMESSAGE, hContact, 0);
		break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// chat log menu event handler

static gc_item sttLogListItems[] =
{
	{ LPGENT("&Invite a user"),      IDM_INVITE,    MENU_ITEM },
	{ NULL, 0, MENU_SEPARATOR },
	{ LPGENT("&Room options"),       0,             MENU_NEWPOPUP },
	{ LPGENT("View/change &topic"),  IDM_TOPIC,     MENU_POPUPITEM },
	{ LPGENT("&Quit chat session"),  IDM_LEAVE,     MENU_POPUPITEM },
#ifdef _DEBUG
	{ LPGENT("Set &avatar"),         IDM_AVATAR,    MENU_POPUPITEM }, // doesn't work, therefore commented out
#endif
	{ NULL, 0, MENU_SEPARATOR },
	{ LPGENT("Copy room &JID"),      IDM_CPY_RJID,  MENU_ITEM },
	{ LPGENT("Copy room topic"),     IDM_CPY_TOPIC, MENU_ITEM },
};

void WhatsAppProto::ChatLogMenuHook(WAChatInfo *pInfo, struct GCHOOK *gch)
{
	switch (gch->dwData) {
	case IDM_INVITE:
		InviteChatUser(pInfo);
		break;

	case IDM_TOPIC:
		EditChatSubject(pInfo);
		break;

	case IDM_CPY_RJID:
		utils::copyText(pcli->hwndContactList, pInfo->tszJid);
		break;

	case IDM_CPY_TOPIC:
		utils::copyText(pcli->hwndContactList, pInfo->tszNick);
		break;

	case IDM_LEAVE:
		if (isOnline())
			m_pConnection->sendJoinLeaveGroup(_T2A(pInfo->tszJid), false);
		break;

	case IDM_AVATAR:
		SetChatAvatar(pInfo);
		break;
	}
}

void WhatsAppProto::EditChatSubject(WAChatInfo *pInfo)
{
	CMString title(FORMAT, TranslateT("Set new subject for %s"), pInfo->tszNick);
	ptrT tszOldValue(getTStringA(pInfo->hContact, WHATSAPP_KEY_NICK));

	ENTER_STRING es = { 0 };
	es.cbSize = sizeof(es);
	es.type = ESF_RICHEDIT;
	es.szModuleName = m_szModuleName;
	es.ptszInitVal = tszOldValue;
	es.caption = title;
	es.szDataPrefix = "setSubject_";
	if (EnterString(&es)) {
		T2Utf gjid(pInfo->tszJid);
		T2Utf gsubject(es.ptszResult);
		m_pConnection->sendSetNewSubject(std::string(gjid), std::string(gsubject));
		mir_free(es.ptszResult);
	}
}

void WhatsAppProto::SetChatAvatar(WAChatInfo *pInfo)
{
	TCHAR tszFileName[MAX_PATH], filter[256];
	BmpFilterGetStrings(filter, SIZEOF(filter));

	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.lpstrFilter = filter;
	ofn.hwndOwner = 0;
	ofn.lpstrFile = tszFileName;
	ofn.nMaxFile = ofn.nMaxFileTitle = SIZEOF(tszFileName);
	ofn.Flags = OFN_HIDEREADONLY;
	ofn.lpstrInitialDir = _T(".");
	ofn.lpstrDefExt = _T("");
	if (GetOpenFileName(&ofn))
		if (_taccess(tszFileName, 4) != -1)
			InternalSetAvatar(pInfo->hContact, _T2A(pInfo->tszJid), tszFileName);
}

/////////////////////////////////////////////////////////////////////////////////////////
// nicklist menu event handler

static gc_item sttNickListItems[] =
{
	{ LPGENT("&Add to roster"), IDM_ADD_RJID, MENU_POPUPITEM },
	{ NULL, 0, MENU_SEPARATOR },
	{ LPGENT("&Kick"), IDM_KICK, MENU_ITEM },
	{ NULL, 0, MENU_SEPARATOR },
	{ LPGENT("Copy &nickname"), IDM_CPY_NICK, MENU_ITEM },
	{ LPGENT("Copy real &JID"), IDM_CPY_RJID, MENU_ITEM },
};

void WhatsAppProto::NickListMenuHook(WAChatInfo *pInfo, struct GCHOOK *gch)
{
	switch (gch->dwData) {
	case IDM_ADD_RJID:
		AddChatUser(pInfo, gch->ptszUID);
		break;

	case IDM_KICK:
		KickChatUser(pInfo, gch->ptszUID);
		break;

	case IDM_CPY_NICK:
		utils::copyText(pcli->hwndContactList, GetChatUserNick(std::string((char*)_T2A(gch->ptszUID))));
		break;

	case IDM_CPY_RJID:
		utils::copyText(pcli->hwndContactList, gch->ptszUID);
		break;
	}
}

void WhatsAppProto::AddChatUser(WAChatInfo *pInfo, const TCHAR *ptszJid)
{
	std::string jid((char*)_T2A(ptszJid));
	MCONTACT hContact = ContactIDToHContact(jid);
	if (hContact && !db_get_b(hContact, "CList", "NotInList", 0))
		return;

	PROTOSEARCHRESULT psr = { 0 };
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_TCHAR;
	psr.id.t = (TCHAR*)ptszJid;
	psr.nick.t = GetChatUserNick(jid);

	ADDCONTACTSTRUCT acs = { 0 };
	acs.handleType = HANDLE_SEARCHRESULT;
	acs.szProto = m_szModuleName;
	acs.psr = &psr;
	CallService(MS_ADDCONTACT_SHOW, (WPARAM)CallService(MS_CLUI_GETHWND, 0, 0), (LPARAM)&acs);
}

void WhatsAppProto::KickChatUser(WAChatInfo *pInfo, const TCHAR *ptszJid)
{
	if (!isOnline())
		return;

	std::string gjid((char*)_T2A(pInfo->tszJid));
	std::vector<std::string> jids(1);
	jids[0] = (char*)_T2A(ptszJid);
	m_pConnection->sendRemoveParticipants(gjid, jids);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Leave groupchat emulator for contact's deletion

int WhatsAppProto::OnDeleteChat(WPARAM hContact, LPARAM lParam)
{
	if (isChatRoom(hContact) && isOnline()) {
		ptrT tszID(getTStringA(hContact, WHATSAPP_KEY_ID));
		if (tszID)
			m_pConnection->sendJoinLeaveGroup(_T2A(tszID), false);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// handler to customize chat menus

int WhatsAppProto::OnChatMenu(WPARAM wParam, LPARAM lParam)
{
	GCMENUITEMS *gcmi = (GCMENUITEMS*)lParam;
	if (gcmi == NULL)
		return 0;

	if (mir_strcmpi(gcmi->pszModule, m_szModuleName))
		return 0;

	if (gcmi->Type == MENU_ON_LOG) {
		gcmi->nItems = SIZEOF(sttLogListItems);
		gcmi->Item = sttLogListItems;
	}
	else if (gcmi->Type == MENU_ON_NICKLIST) {
		gcmi->nItems = SIZEOF(sttNickListItems);
		gcmi->Item = sttNickListItems;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// chat helpers

WAChatInfo* WhatsAppProto::InitChat(const std::string &jid, const std::string &nick)
{
	TCHAR *ptszJid = str2t(jid), *ptszNick = str2t(nick);

	WAChatInfo *pInfo = new WAChatInfo(ptszJid, ptszNick);
	m_chats[jid] = pInfo;

	MCONTACT hOldContact = ContactIDToHContact(jid);
	if (hOldContact && !isChatRoom(hOldContact)) {
		delSetting(hOldContact, "ID");
		setByte(hOldContact, "ChatRoom", 1);
		setString(hOldContact, "ChatRoomID", jid.c_str());
	}

	GCSESSION gcw = { sizeof(GCSESSION) };
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = m_szModuleName;
	gcw.ptszName = ptszNick;
	gcw.ptszID = ptszJid;
	CallServiceSync(MS_GC_NEWSESSION, NULL, (LPARAM)&gcw);

	pInfo->hContact = (hOldContact != NULL) ? hOldContact : ContactIDToHContact(jid);

	GCDEST gcd = { m_szModuleName, ptszJid, GC_EVENT_ADDGROUP };
	GCEVENT gce = { sizeof(gce), &gcd };
	for (int i = SIZEOF(sttStatuses) - 1; i >= 0; i--) {
		gce.ptszStatus = TranslateTS(sttStatuses[i]);
		CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);
	}

	gcd.iType = GC_EVENT_CONTROL;
	CallServiceSync(MS_GC_EVENT, getBool(WHATSAPP_KEY_AUTORUNCHATS, true) ? SESSION_INITDONE : WINDOW_HIDDEN, (LPARAM)&gce);
	CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);

	if (m_pConnection)
		m_pConnection->sendGetParticipants(jid);

	return pInfo;
}

TCHAR* WhatsAppProto::GetChatUserNick(const std::string &jid)
{
	TCHAR* tszNick;
	if (m_szJid != jid) {
		MCONTACT hContact = ContactIDToHContact(jid);
		tszNick = (hContact == 0) ? utils::removeA(str2t(jid)) : mir_tstrdup(pcli->pfnGetContactDisplayName(hContact, 0));
	}
	else tszNick = str2t(m_szNick);

	if (tszNick == NULL)
		tszNick = mir_tstrdup(TranslateT("Unknown user"));
	return tszNick;
}

WAChatInfo* WhatsAppProto::SafeGetChat(const std::string &jid)
{
	mir_cslock lck(m_csChats);
	return m_chats[jid];
}

///////////////////////////////////////////////////////////////////////////////
// WAGroupListener members

void WhatsAppProto::onGroupInfo(const std::string &jid, const std::string &owner, const std::string &subject, const std::string &subject_owner, int time_subject, int time_created)
{
	WAChatInfo *pInfo = SafeGetChat(jid);
	if (pInfo == NULL) {
		pInfo = InitChat(jid, subject);
		pInfo->bActive = true;
		time_subject = 0;
	}
	else {
		GCDEST gcd = { m_szModuleName, pInfo->tszJid, GC_EVENT_CONTROL };
		GCEVENT gce = { sizeof(gce), &gcd };
		CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);
	}

	if (!subject.empty()) {
		pInfo->tszOwner = str2t(owner);

		onGroupNewSubject(jid, subject_owner, subject, time_subject);
	}
}

void WhatsAppProto::onGroupMessage(const FMessage &pMsg)
{
	// we need to add a contact, so there's no difference at all
	if (pMsg.media_wa_type == FMessage::WA_TYPE_CONTACT) {
		onMessageForMe(pMsg);
		return;
	}

	WAChatInfo *pInfo = SafeGetChat(pMsg.key.remote_jid);
	if (pInfo == NULL) {
		pInfo = InitChat(pMsg.key.remote_jid, "");
		pInfo->bActive = true;
	}

	std::string msg(pMsg.data);
	if (!pMsg.media_url.empty()) {
		if (!msg.empty())
			msg.append("\n");
		msg += pMsg.media_url;
	}

	ptrT tszText(str2t(msg));
	ptrT tszUID(str2t(pMsg.remote_resource));
	ptrT tszNick(GetChatUserNick(pMsg.remote_resource));

	GCDEST gcd = { m_szModuleName, pInfo->tszJid, GC_EVENT_MESSAGE };

	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszUID = tszUID;
	gce.ptszNick = tszNick;
	gce.time = pMsg.timestamp;
	gce.ptszText = tszText;
	gce.bIsMe = m_szJid == pMsg.remote_resource;
	CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);

	if (isOnline())
		m_pConnection->sendMessageReceived(pMsg);
}

void WhatsAppProto::onGroupNewSubject(const std::string &gjid, const std::string &author, const std::string &newSubject, int ts)
{
	WAChatInfo *pInfo = SafeGetChat(gjid);
	if (pInfo == NULL)
		return;

	ptrT tszText(str2t(newSubject));
	ptrT tszTextDb(getTStringA(pInfo->hContact, WHATSAPP_KEY_NICK));
	if (!mir_tstrcmp(tszText, tszTextDb)) // notify about subject change only if differs from the stored one
		return;

	ptrT tszUID(str2t(author));
	ptrT tszNick(GetChatUserNick(author));

	GCDEST gcd = { m_szModuleName, pInfo->tszJid, GC_EVENT_TOPIC };

	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = GCEF_ADDTOLOG + ((ts == 0) ? GCEF_NOTNOTIFY : 0);
	gce.ptszUID = tszUID;
	gce.ptszNick = tszNick;
	gce.time = ts;
	gce.ptszText = tszText;
	CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);

	setTString(pInfo->hContact, WHATSAPP_KEY_NICK, tszText);
}

void WhatsAppProto::onGroupAddUser(const std::string &gjid, const std::string &ujid, int ts)
{
	WAChatInfo *pInfo = SafeGetChat(gjid);
	if (pInfo == NULL || !pInfo->bActive)
		return;

	ptrT tszUID(str2t(ujid));
	ptrT tszNick(GetChatUserNick(ujid));

	GCDEST gcd = { m_szModuleName, pInfo->tszJid, GC_EVENT_JOIN };

	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszUID = tszUID;
	gce.ptszNick = tszNick;
	gce.time = ts;
	CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);
}

void WhatsAppProto::onGroupRemoveUser(const std::string &gjid, const std::string &ujid, int ts)
{
	WAChatInfo *pInfo = SafeGetChat(gjid);
	if (pInfo == NULL)
		return;

	ptrT tszUID(str2t(ujid));
	ptrT tszNick(GetChatUserNick(ujid));

	GCDEST gcd = { m_szModuleName, pInfo->tszJid, GC_EVENT_PART };

	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszUID = tszUID;
	gce.ptszNick = tszNick;
	gce.time = ts;
	CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);
}

void WhatsAppProto::onLeaveGroup(const std::string &gjid)
{
	WAChatInfo *pInfo = SafeGetChat(gjid);
	if (pInfo == NULL)
		return;

	GCDEST gcd = { m_szModuleName, pInfo->tszJid, GC_EVENT_CONTROL };
	
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.ptszUID = pInfo->tszJid;
	CallServiceSync(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);

	CallService(MS_DB_CONTACT_DELETE, pInfo->hContact, 0);
	m_chats.erase((char*)_T2A(pInfo->tszJid));
}

void WhatsAppProto::onGetParticipants(const std::string &gjid, const std::vector<string> &participants)
{
	mir_cslock lck(m_csChats);

	WAChatInfo *pInfo = m_chats[gjid];
	if (pInfo == NULL)
		return;

	pInfo->bActive = true;
	for (size_t i = 0; i < participants.size(); i++) {
		std::string curr = participants[i];

		ptrT ujid(str2t(curr)), nick(GetChatUserNick(curr));
		bool bIsOwner = !mir_tstrcmp(ujid, pInfo->tszOwner);

		GCDEST gcd = { m_szModuleName, pInfo->tszJid, GC_EVENT_JOIN };

		GCEVENT gce = { sizeof(gce), &gcd };
		gce.ptszNick = nick;
		gce.ptszUID = utils::removeA(ujid);
		gce.ptszStatus = (bIsOwner) ? _T("Owners") : _T("Members");
		CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);
	}
}

void WhatsAppProto::onGroupCreated(const std::string &gjid, const std::string &subject)
{
	WAChatInfo *pInfo = InitChat(gjid, subject);
	pInfo->tszOwner = str2t(m_szJid);

	// also set new subject if it's present
	if (!subject.empty())
		onGroupNewSubject(gjid, "Server", subject, 0);
}

void WhatsAppProto::onGroupMessageReceived(const FMessage &msg)
{
	WAChatInfo *pInfo = SafeGetChat(msg.key.remote_jid);
	if (pInfo == NULL)
		return;
	
	auto p = pInfo->m_unsentMsgs.find(msg.key.id);
	if (p == pInfo->m_unsentMsgs.end())
		return;

	ptrT tszUID(str2t(m_szJid));
	ptrT tszNick(str2t(m_szNick));

	GCDEST gcd = { m_szModuleName, pInfo->tszJid, GC_EVENT_MESSAGE };

	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszUID = tszUID;
	gce.ptszNick = tszNick;
	gce.time = time(NULL);
	gce.ptszText = p->second.c_str();
	gce.bIsMe = m_szJid == msg.remote_resource;
	CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);

	pInfo->m_unsentMsgs.erase(p);
}
