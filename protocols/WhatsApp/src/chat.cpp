#include "stdafx.h"

static const wchar_t *sttStatuses[] = { LPGENW("Members"), LPGENW("Owners") };

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

INT_PTR __cdecl WhatsAppProto::OnCreateGroup(WPARAM, LPARAM)
{
	ENTER_STRING es = { 0 };
	es.cbSize = sizeof(es);
	es.type = ESF_MULTILINE;
	es.caption = L"Enter a subject for new group";
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

int WhatsAppProto::onGroupChatEvent(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK*)lParam;
	if (mir_strcmp(gch->pszModule, m_szModuleName))
		return 0;

	std::string chat_id(T2Utf(gch->ptszID));
	WAChatInfo *pInfo = SafeGetChat(chat_id);
	if (pInfo == NULL)
		return 0;

	switch (gch->iType) {
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
			setWString(hContact, "Nick", gch->ptszUID);
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
	{ LPGENW("&Invite a user"),      IDM_INVITE,    MENU_ITEM },
	{ NULL, 0, MENU_SEPARATOR },
	{ LPGENW("&Room options"),       0,             MENU_NEWPOPUP },
	{ LPGENW("View/change &topic"),  IDM_TOPIC,     MENU_POPUPITEM },
	{ LPGENW("&Quit chat session"),  IDM_LEAVE,     MENU_POPUPITEM },
#ifdef _DEBUG
	{ LPGENW("Set &avatar"),         IDM_AVATAR,    MENU_POPUPITEM }, // doesn't work, therefore commented out
#endif
	{ NULL, 0, MENU_SEPARATOR },
	{ LPGENW("Copy room &JID"),      IDM_CPY_RJID,  MENU_ITEM },
	{ LPGENW("Copy room topic"),     IDM_CPY_TOPIC, MENU_ITEM },
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
	CMStringW title(FORMAT, TranslateT("Set new subject for %s"), pInfo->tszNick);
	ptrW tszOldValue(getWStringA(pInfo->hContact, WHATSAPP_KEY_NICK));

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
	wchar_t tszFileName[MAX_PATH], filter[256];
	Bitmap_GetFilter(filter, _countof(filter));

	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.lpstrFilter = filter;
	ofn.hwndOwner = 0;
	ofn.lpstrFile = tszFileName;
	ofn.nMaxFile = ofn.nMaxFileTitle = _countof(tszFileName);
	ofn.Flags = OFN_HIDEREADONLY;
	ofn.lpstrInitialDir = L".";
	ofn.lpstrDefExt = L"";
	if (GetOpenFileName(&ofn))
		if (_waccess(tszFileName, 4) != -1)
			InternalSetAvatar(pInfo->hContact, _T2A(pInfo->tszJid), tszFileName);
}

/////////////////////////////////////////////////////////////////////////////////////////
// nicklist menu event handler

static gc_item sttNickListItems[] =
{
	{ LPGENW("&Add to roster"), IDM_ADD_RJID, MENU_POPUPITEM },
	{ NULL, 0, MENU_SEPARATOR },
	{ LPGENW("&Kick"), IDM_KICK, MENU_ITEM },
	{ NULL, 0, MENU_SEPARATOR },
	{ LPGENW("Copy &nickname"), IDM_CPY_NICK, MENU_ITEM },
	{ LPGENW("Copy real &JID"), IDM_CPY_RJID, MENU_ITEM },
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
		utils::copyText(pcli->hwndContactList, ptrW(GetChatUserNick(std::string((char*)_T2A(gch->ptszUID)))));
		break;

	case IDM_CPY_RJID:
		utils::copyText(pcli->hwndContactList, gch->ptszUID);
		break;
	}
}

void WhatsAppProto::AddChatUser(WAChatInfo*, const wchar_t *ptszJid)
{
	std::string jid((char*)_T2A(ptszJid));
	MCONTACT hContact = ContactIDToHContact(jid);
	if (hContact && !db_get_b(hContact, "CList", "NotInList", 0))
		return;

	PROTOSEARCHRESULT psr = { 0 };
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_UNICODE;
	psr.id.w = (wchar_t*)ptszJid;
	psr.nick.w = GetChatUserNick(jid);

	ADDCONTACTSTRUCT acs = { 0 };
	acs.handleType = HANDLE_SEARCHRESULT;
	acs.szProto = m_szModuleName;
	acs.psr = &psr;
	CallService(MS_ADDCONTACT_SHOW, (WPARAM)pcli->hwndContactList, (LPARAM)&acs);
}

void WhatsAppProto::KickChatUser(WAChatInfo *pInfo, const wchar_t *ptszJid)
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

int WhatsAppProto::OnDeleteChat(WPARAM hContact, LPARAM)
{
	if (isChatRoom(hContact) && isOnline()) {
		ptrW tszID(getWStringA(hContact, WHATSAPP_KEY_ID));
		if (tszID)
			m_pConnection->sendJoinLeaveGroup(_T2A(tszID), false);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// handler to customize chat menus

int WhatsAppProto::OnChatMenu(WPARAM, LPARAM lParam)
{
	GCMENUITEMS *gcmi = (GCMENUITEMS*)lParam;
	if (gcmi == NULL)
		return 0;

	if (mir_strcmpi(gcmi->pszModule, m_szModuleName))
		return 0;

	if (gcmi->Type == MENU_ON_LOG)
		Chat_AddMenuItems(gcmi->hMenu, _countof(sttLogListItems), sttLogListItems);
	else if (gcmi->Type == MENU_ON_NICKLIST)
		Chat_AddMenuItems(gcmi->hMenu, _countof(sttNickListItems), sttNickListItems);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// chat helpers

WAChatInfo* WhatsAppProto::InitChat(const std::string &jid, const std::string &nick)
{
	wchar_t *ptszJid = str2t(jid), *ptszNick = str2t(nick);

	WAChatInfo *pInfo = new WAChatInfo(ptszJid, ptszNick);
	m_chats[jid] = pInfo;

	MCONTACT hOldContact = ContactIDToHContact(jid);
	if (hOldContact && !isChatRoom(hOldContact)) {
		delSetting(hOldContact, "ID");
		setByte(hOldContact, "ChatRoom", 1);
		setString(hOldContact, "ChatRoomID", jid.c_str());
	}

	Chat_NewSession(GCW_CHATROOM, m_szModuleName, ptszJid, ptszNick);

	pInfo->hContact = (hOldContact != NULL) ? hOldContact : ContactIDToHContact(jid);

	for (int i = _countof(sttStatuses) - 1; i >= 0; i--)
		Chat_AddGroup(m_szModuleName, ptszJid, TranslateW(sttStatuses[i]));

	Chat_Control(m_szModuleName, ptszJid, getBool(WHATSAPP_KEY_AUTORUNCHATS, true) ? SESSION_INITDONE : WINDOW_HIDDEN);
	Chat_Control(m_szModuleName, ptszJid, SESSION_ONLINE);

	if (m_pConnection)
		m_pConnection->sendGetParticipants(jid);

	return pInfo;
}

wchar_t* WhatsAppProto::GetChatUserNick(const std::string &jid)
{
	wchar_t* tszNick;
	if (m_szJid != jid) {
		MCONTACT hContact = ContactIDToHContact(jid);
		tszNick = (hContact == 0) ? utils::removeA(str2t(jid)) : mir_wstrdup(pcli->pfnGetContactDisplayName(hContact, 0));
	}
	else tszNick = str2t(m_szNick);

	if (tszNick == NULL)
		tszNick = mir_wstrdup(TranslateT("Unknown user"));
	return tszNick;
}

WAChatInfo* WhatsAppProto::SafeGetChat(const std::string &jid)
{
	mir_cslock lck(m_csChats);
	return m_chats[jid];
}

///////////////////////////////////////////////////////////////////////////////
// WAGroupListener members

void WhatsAppProto::onGroupInfo(const std::string &jid, const std::string &owner, const std::string &subject, const std::string &subject_owner, int time_subject, int)
{
	WAChatInfo *pInfo = SafeGetChat(jid);
	if (pInfo == NULL) {
		pInfo = InitChat(jid, subject);
		pInfo->bActive = true;
		time_subject = 0;
	}
	else Chat_Control(m_szModuleName, pInfo->tszJid, SESSION_ONLINE);

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

	ptrW tszText(str2t(msg));
	ptrW tszUID(str2t(pMsg.remote_resource));
	ptrW tszNick(GetChatUserNick(pMsg.remote_resource));

	GCEVENT gce = { m_szModuleName, pInfo->tszJid, GC_EVENT_MESSAGE };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszUID = tszUID;
	gce.ptszNick = tszNick;
	gce.time = pMsg.timestamp;
	gce.ptszText = tszText;
	gce.bIsMe = m_szJid == pMsg.remote_resource;
	Chat_Event(&gce);

	if (isOnline())
		m_pConnection->sendMessageReceived(pMsg);
}

void WhatsAppProto::onGroupNewSubject(const std::string &gjid, const std::string &author, const std::string &newSubject, int ts)
{
	WAChatInfo *pInfo = SafeGetChat(gjid);
	if (pInfo == NULL)
		return;

	ptrW tszText(str2t(newSubject));
	ptrW tszTextDb(getWStringA(pInfo->hContact, WHATSAPP_KEY_NICK));
	if (!mir_wstrcmp(tszText, tszTextDb)) // notify about subject change only if differs from the stored one
		return;

	ptrW tszUID(str2t(author));
	ptrW tszNick(GetChatUserNick(author));

	GCEVENT gce = { m_szModuleName, pInfo->tszJid, GC_EVENT_TOPIC };
	gce.dwFlags = GCEF_ADDTOLOG + ((ts == 0) ? GCEF_NOTNOTIFY : 0);
	gce.ptszUID = tszUID;
	gce.ptszNick = tszNick;
	gce.time = ts;
	gce.ptszText = tszText;
	Chat_Event(&gce);

	setWString(pInfo->hContact, WHATSAPP_KEY_NICK, tszText);
}

void WhatsAppProto::onGroupAddUser(const std::string &gjid, const std::string &ujid, int ts)
{
	WAChatInfo *pInfo = SafeGetChat(gjid);
	if (pInfo == NULL || !pInfo->bActive)
		return;

	ptrW tszUID(str2t(ujid));
	ptrW tszNick(GetChatUserNick(ujid));

	GCEVENT gce = { m_szModuleName, pInfo->tszJid, GC_EVENT_JOIN };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszUID = tszUID;
	gce.ptszNick = tszNick;
	gce.time = ts;
	Chat_Event(&gce);
}

void WhatsAppProto::onGroupRemoveUser(const std::string &gjid, const std::string &ujid, int ts)
{
	WAChatInfo *pInfo = SafeGetChat(gjid);
	if (pInfo == NULL)
		return;

	ptrW tszUID(str2t(ujid));
	ptrW tszNick(GetChatUserNick(ujid));

	GCEVENT gce = { m_szModuleName, pInfo->tszJid, GC_EVENT_PART };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszUID = tszUID;
	gce.ptszNick = tszNick;
	gce.time = ts;
	Chat_Event(&gce);
}

void WhatsAppProto::onLeaveGroup(const std::string &gjid)
{
	WAChatInfo *pInfo = SafeGetChat(gjid);
	if (pInfo == NULL)
		return;

	Chat_Terminate(m_szModuleName, pInfo->tszJid);

	db_delete_contact(pInfo->hContact);
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

		ptrW ujid(str2t(curr)), nick(GetChatUserNick(curr));
		bool bIsOwner = !mir_wstrcmp(ujid, pInfo->tszOwner);

		GCEVENT gce = { m_szModuleName, pInfo->tszJid, GC_EVENT_JOIN };
		gce.ptszNick = nick;
		gce.ptszUID = utils::removeA(ujid);
		gce.ptszStatus = (bIsOwner) ? L"Owners" : L"Members";
		Chat_Event(&gce);
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

	ptrW tszUID(str2t(m_szJid));
	ptrW tszNick(str2t(m_szNick));

	GCEVENT gce = { m_szModuleName, pInfo->tszJid, GC_EVENT_MESSAGE };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszUID = tszUID;
	gce.ptszNick = tszNick;
	gce.time = time(NULL);
	gce.ptszText = p->second.c_str();
	gce.bIsMe = m_szJid == msg.remote_resource;
	Chat_Event(&gce);

	pInfo->m_unsentMsgs.erase(p);
}
