/*
Copyright (c) 2013-17 Miranda NG project (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

enum
{
	IDM_NONE,
	IDM_TOPIC, IDM_INVITE, IDM_DESTROY,
	IDM_KICK, IDM_INFO, IDM_CHANGENICK, IDM_VISIT_PROFILE
};

static LPCWSTR sttStatuses[] = { LPGENW("Participants"), LPGENW("Owners") };

extern JSONNode nullNode;

CVkChatInfo* CVkProto::AppendChat(int id, const JSONNode &jnDlg)
{
	debugLogW(L"CVkProto::AppendChat");
	if (id == 0)
		return nullptr;

	if (jnDlg) {
		CMStringW action_chat = jnDlg["action"].as_mstring();
		int action_mid = _wtoi(jnDlg["action_mid"].as_mstring());
		if ((action_chat == L"chat_kick_user") && (action_mid == m_myUserId))
			return nullptr;
	}

	MCONTACT chatContact = FindChat(id);
	if (chatContact && getBool(chatContact, "kicked"))
		return nullptr;

	CVkChatInfo *c = m_chats.find((CVkChatInfo*)&id);
	if (c != nullptr)
		return c;

	CMStringW wszTitle;
	c = new CVkChatInfo(id);
	if (jnDlg) {
		wszTitle = jnDlg["title"].as_mstring();
		c->m_wszTopic = mir_wstrdup(!wszTitle.IsEmpty() ? wszTitle : L"");
	}

	CMStringW sid;
	sid.Format(L"%S_%d", m_szModuleName, id);
	c->m_wszId = mir_wstrdup(sid);

	GCSessionInfoBase *si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, sid, wszTitle);
	c->m_hContact = si->hContact;

	setWString(si->hContact, "Nick", wszTitle);
	m_chats.insert(c);

	for (int i = _countof(sttStatuses) - 1; i >= 0; i--)
		Chat_AddGroup(m_szModuleName, sid, TranslateW(sttStatuses[i]));

	setDword(si->hContact, "vk_chat_id", id);

	CMStringW wszHomepage(FORMAT, L"https://vk.com/im?sel=c%d", id);
	setWString(si->hContact, "Homepage", wszHomepage);

	db_unset(si->hContact, m_szModuleName, "off");

	if (jnDlg && jnDlg["left"].as_bool()) {
		setByte(si->hContact, "off", 1);
		m_chats.remove(c);
		return nullptr;
	}

	Chat_Control(m_szModuleName, sid, (m_vkOptions.bHideChats) ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(m_szModuleName, sid, SESSION_ONLINE);

	RetrieveChatInfo(c);
	return c;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::RetrieveChatInfo(CVkChatInfo *cc)
{
	debugLogA("CVkProto::RetrieveChatInfo");

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.RetrieveChatInfo", true, &CVkProto::OnReceiveChatInfo)
		<< INT_PARAM("chatid", cc->m_chatid)
		<< INT_PARAM("func_v", cc->m_bHistoryRead ? 1 : 2)
	)->pUserInfo = cc;
}

void CVkProto::OnReceiveChatInfo(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveChatInfo %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse)
		return;

	CVkChatInfo *cc = (CVkChatInfo*)pReq->pUserInfo;
	if (m_chats.indexOf(cc) == -1)
		return;

	const JSONNode &jnInfo = jnResponse["info"];
	if (jnInfo) {
		if (jnInfo["title"])
			SetChatTitle(cc, jnInfo["title"].as_mstring());

		if (jnInfo["left"].as_bool() || jnInfo["kicked"].as_bool()) {
			setByte(cc->m_hContact, "kicked", jnInfo["kicked"].as_bool());
			LeaveChat(cc->m_chatid);
			return;
		}
		cc->m_admin_id = jnInfo["admin_id"].as_int();
	}

	const JSONNode &jnUsers = jnResponse["users"];
	if (jnUsers) {
		for (int i = 0; i < cc->m_users.getCount(); i++)
			cc->m_users[i].m_bDel = true;

		for (auto it = jnUsers.begin(); it != jnUsers.end(); ++it) {
			const JSONNode &jnUser = (*it);
			if (!jnUser)
				break;

			int uid = jnUser["id"].as_int();
			wchar_t wszId[20];
			_itow(uid, wszId, 10);

			bool bNew;
			CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&uid);
			if (cu == nullptr) {
				cc->m_users.insert(cu = new CVkChatUser(uid));
				bNew = true;
			}
			else
				bNew = cu->m_bUnknown;
			cu->m_bDel = false;

			CMStringW wszNick(ptrW(db_get_wsa(cc->m_hContact, m_szModuleName, CMStringA(FORMAT, "nick%d", cu->m_uid))));
			if (wszNick.IsEmpty()) {
				CMStringW fName(jnUser["first_name"].as_mstring());
				CMStringW lName(jnUser["last_name"].as_mstring());
				wszNick = fName.Trim() + L" " + lName.Trim();
			}
			cu->m_wszNick = mir_wstrdup(wszNick);
			cu->m_bUnknown = false;

			if (bNew) {
				GCEVENT gce = { m_szModuleName, cc->m_wszId, GC_EVENT_JOIN };
				gce.bIsMe = uid == m_myUserId;
				gce.ptszUID = wszId;
				gce.ptszNick = wszNick;
				gce.ptszStatus = TranslateW(sttStatuses[uid == cc->m_admin_id]);
				gce.dwItemData = (INT_PTR)cu;
				Chat_Event(&gce);
			}
		}

		for (int i = cc->m_users.getCount() - 1; i >= 0; i--) {
			CVkChatUser &cu = cc->m_users[i];
			if (!cu.m_bDel)
				continue;

			wchar_t wszId[20];
			_itow(cu.m_uid, wszId, 10);

			GCEVENT gce = { m_szModuleName, cc->m_wszId, GC_EVENT_PART };
			gce.ptszUID = wszId;
			gce.dwFlags = GCEF_NOTNOTIFY;
			gce.time = time(nullptr);
			gce.ptszNick = mir_wstrdup(CMStringW(FORMAT, L"%s (https://vk.com/id%s)", cu.m_wszNick, wszId));
			Chat_Event(&gce);

			cc->m_users.remove(i);
		}
	}

	const JSONNode &jnMsgsUsers = jnResponse["msgs_users"];
	for (auto it = jnMsgsUsers.begin(); it != jnMsgsUsers.end(); ++it) {
		const JSONNode &jnUser = (*it);
		LONG uid = jnUser["id"].as_int();
		CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&uid);
		if (cu)
			continue;

		MCONTACT hContact = FindUser(uid);
		if (hContact)
			continue;

		hContact = SetContactInfo(jnUser, true);
		db_set_b(hContact, "CList", "Hidden", 1);
		db_set_b(hContact, "CList", "NotOnList", 1);
		db_set_dw(hContact, "Ignore", "Mask1", 0);
	}

	const JSONNode &jnMsgs = jnResponse["msgs"];
	const JSONNode &jnFUsers = jnResponse["fwd_users"];
	if (jnMsgs) {
		const JSONNode &jnItems = jnMsgs["items"];
		if (jnItems) {
			for (auto it = jnItems.begin(); it != jnItems.end(); ++it) {
				const JSONNode &jnMsg = (*it);
				if (!jnMsg)
					break;

				AppendChatMessage(cc->m_chatid, jnMsg, jnFUsers, true);
			}
			cc->m_bHistoryRead = true;
		}
	}

	for (int j = 0; j < cc->m_msgs.getCount(); j++) {
		CVkChatMessage &p = cc->m_msgs[j];
		AppendChatMessage(cc, p.m_uid, p.m_date, p.m_wszBody, p.m_bHistory, p.m_bIsAction);
	}
	cc->m_msgs.destroy();
}

void CVkProto::SetChatTitle(CVkChatInfo *cc, LPCWSTR wszTopic)
{
	debugLogW(L"CVkProto::SetChatTitle");
	if (!cc)
		return;

	if (mir_wstrcmp(cc->m_wszTopic, wszTopic) == 0)
		return;

	cc->m_wszTopic = mir_wstrdup(wszTopic);
	setWString(cc->m_hContact, "Nick", wszTopic);

	Chat_ChangeSessionName(m_szModuleName, cc->m_wszId, wszTopic);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::AppendChatMessage(int id, const JSONNode &jnMsg, const JSONNode &jnFUsers, bool bIsHistory)
{
	debugLogA("CVkProto::AppendChatMessage");
	CVkChatInfo *cc = AppendChat(id, nullNode);
	if (cc == nullptr)
		return;

	int mid = jnMsg["id"].as_int();
	int uid = jnMsg["user_id"].as_int();
	bool bIsAction = false;

	int msgTime = jnMsg["date"].as_int();
	time_t now = time(nullptr);
	if (!msgTime || msgTime > now)
		msgTime = now;

	CMStringW wszBody(jnMsg["body"].as_mstring());

	const JSONNode &jnFwdMessages = jnMsg["fwd_messages"];
	if (jnFwdMessages) {
		CMStringW wszFwdMessages = GetFwdMessages(jnFwdMessages, jnFUsers, bbcNo);
		if (!wszBody.IsEmpty())
			wszFwdMessages = L"\n" + wszFwdMessages;
		wszBody += wszFwdMessages;
	}

	const JSONNode &jnAttachments = jnMsg["attachments"];
	if (jnAttachments) {
		CMStringW wszAttachmentDescr = GetAttachmentDescr(jnAttachments, bbcNo);
		if (!wszBody.IsEmpty())
			wszAttachmentDescr = L"\n" + wszAttachmentDescr;
		wszBody += wszAttachmentDescr;
	}

	if (m_vkOptions.bAddMessageLinkToMesWAtt && (jnAttachments || jnFwdMessages))
		wszBody += SetBBCString(TranslateT("Message link"), bbcNo, vkbbcUrl,
			CMStringW(FORMAT, L"https://vk.com/im?sel=c%d&msgid=%d", cc->m_chatid, mid));

	if (jnMsg["action"]) {
		bIsAction = true;
		CMStringW wszAction = jnMsg["action"].as_mstring();

		if (wszAction == L"chat_create") {
			CMStringW wszActionText = jnMsg["action_text"].as_mstring();
			wszBody.AppendFormat(L"%s \"%s\"", TranslateT("create chat"), wszActionText.IsEmpty() ? L" " : wszActionText);
		}
		else if (wszAction == L"chat_kick_user") {
			CMStringW wszActionMid = jnMsg["action_mid"].as_mstring();
			if (wszActionMid.IsEmpty())
				wszBody = TranslateT("kick user");
			else {
				CMStringW wszUid(FORMAT, L"%d", uid);
				if (wszUid == wszActionMid) {
					if (cc->m_bHistoryRead)
						return;
					wszBody.AppendFormat(L" (https://vk.com/id%s) %s", wszUid.c_str(), TranslateT("left chat"));
				}
				else {
					int a_uid = 0;
					int iReadCount = swscanf(wszActionMid, L"%d", &a_uid);
					if (iReadCount == 1) {
						CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&a_uid);
						if (cu == nullptr)
							wszBody.AppendFormat(L"%s (https://vk.com/id%d)", TranslateT("kick user"), a_uid);
						else
							wszBody.AppendFormat(L"%s %s (https://vk.com/id%d)", TranslateT("kick user"), cu->m_wszNick, a_uid);
					}
					else
						wszBody = TranslateT("kick user");
				}
			}
		}
		else if (wszAction == L"chat_invite_user") {
			CMStringW wszActionMid = jnMsg["action_mid"].as_mstring();
			if (wszActionMid.IsEmpty())
				wszBody = TranslateT("invite user");
			else {
				CMStringW wszUid(FORMAT, L"%d", uid);
				if (wszUid == wszActionMid)
					wszBody.AppendFormat(L" (https://vk.com/id%s) %s", wszUid.c_str(), TranslateT("returned to chat"));
				else {
					int a_uid = 0;
					int iReadCount = swscanf(wszActionMid, L"%d", &a_uid);
					if (iReadCount == 1) {
						CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&a_uid);
						if (cu == nullptr)
							wszBody.AppendFormat(L"%s (https://vk.com/id%d)", TranslateT("invite user"), a_uid);
						else
							wszBody.AppendFormat(L"%s %s (https://vk.com/id%d)", TranslateT("invite user"), cu->m_wszNick, a_uid);
					}
					else
						wszBody = TranslateT("invite user");
				}
			}
		}
		else if (wszAction == L"chat_title_update") {
			CMStringW wszTitle = jnMsg["action_text"].as_mstring();
			wszBody.AppendFormat(L"%s \"%s\"", TranslateT("change chat title to"), wszTitle.IsEmpty() ? L" " : wszTitle);

			if (!bIsHistory)
				SetChatTitle(cc, wszTitle);
		}
		else if (wszAction == L"chat_photo_update")
			wszBody.Replace(TranslateT("Attachments:"), TranslateT("changed chat cover:"));
		else if (wszAction == L"chat_photo_remove")
			wszBody = TranslateT("deleted chat cover");
		else
			wszBody.AppendFormat(L": %s (%s)", TranslateT("chat action not supported"), wszAction.c_str());
	}

	wszBody.Replace(L"%", L"%%");

	if (cc->m_bHistoryRead) {
		if (jnMsg["title"])
			SetChatTitle(cc, jnMsg["title"].as_mstring());
		AppendChatMessage(cc, uid, msgTime, wszBody, bIsHistory, bIsAction);
	}
	else {
		CVkChatMessage *cm = cc->m_msgs.find((CVkChatMessage *)&mid);
		if (cm == nullptr)
			cc->m_msgs.insert(cm = new CVkChatMessage(mid));

		cm->m_uid = uid;
		cm->m_date = msgTime;
		cm->m_wszBody = mir_wstrdup(wszBody);
		cm->m_bHistory = bIsHistory;
		cm->m_bIsAction = bIsAction;
	}
}

void CVkProto::AppendChatMessage(CVkChatInfo *cc, int uid, int msgTime, LPCWSTR pwszBody, bool bIsHistory, bool bIsAction)
{
	debugLogA("CVkProto::AppendChatMessage2");
	MCONTACT hContact = FindUser(uid);
	CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&uid);
	if (cu == nullptr) {
		cc->m_users.insert(cu = new CVkChatUser(uid));
		CMStringW wszNick(ptrW(db_get_wsa(cc->m_hContact, m_szModuleName, CMStringA(FORMAT, "nick%d", cu->m_uid))));
		cu->m_wszNick = mir_wstrdup(wszNick.IsEmpty() ? (hContact ? ptrW(db_get_wsa(hContact, m_szModuleName, "Nick")) : TranslateT("Unknown")) : wszNick);
		cu->m_bUnknown = true;
	}

	wchar_t wszId[20];
	_itow(uid, wszId, 10);

	GCEVENT gce = { m_szModuleName, cc->m_wszId, bIsAction ? GC_EVENT_ACTION : GC_EVENT_MESSAGE };
	gce.bIsMe = (uid == m_myUserId);
	gce.ptszUID = wszId;
	gce.time = msgTime;
	gce.dwFlags = (bIsHistory) ? GCEF_NOTNOTIFY : GCEF_ADDTOLOG;
	gce.ptszNick = cu->m_wszNick ? mir_wstrdup(cu->m_wszNick) : mir_wstrdup(hContact ? ptrW(db_get_wsa(hContact, m_szModuleName, "Nick")) : TranslateT("Unknown"));
	gce.ptszText = IsEmpty((wchar_t *)pwszBody) ? mir_wstrdup(L"...") : mir_wstrdup(pwszBody);
	Chat_Event(&gce);
	StopChatContactTyping(cc->m_chatid, uid);
}

/////////////////////////////////////////////////////////////////////////////////////////

CVkChatInfo* CVkProto::GetChatById(LPCWSTR pwszId)
{
	for (int i = 0; i < m_chats.getCount(); i++)
		if (!mir_wstrcmp(m_chats[i].m_wszId, pwszId))
			return &m_chats[i];

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::SetChatStatus(MCONTACT hContact, int iStatus)
{
	ptrW wszChatID(getWStringA(hContact, "ChatRoomID"));
	if (wszChatID == nullptr)
		return;

	CVkChatInfo *cc = GetChatById(wszChatID);
	if (cc != nullptr)
		Chat_Control(m_szModuleName, wszChatID, (iStatus == ID_STATUS_OFFLINE) ? SESSION_OFFLINE : SESSION_ONLINE);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CVkProto::OnChatEvent(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK*)lParam;
	if (gch == nullptr)
		return 0;

	if (mir_strcmpi(gch->pszModule, m_szModuleName))
		return 0;

	CVkChatInfo *cc = GetChatById(gch->ptszID);
	if (cc == nullptr)
		return 0;

	switch (gch->iType) {
	case GC_USER_MESSAGE:
		if (IsOnline() && mir_wstrlen(gch->ptszText) > 0) {
			ptrW pwszBuf(mir_wstrdup(gch->ptszText));
			rtrimw(pwszBuf);
			Chat_UnescapeTags(pwszBuf);
			SendMsg(cc->m_hContact, 0, T2Utf(pwszBuf));
		}
		break;

	case GC_USER_PRIVMESS:
		{
			MCONTACT hContact = FindUser(_wtoi(gch->ptszUID));
			if (hContact == 0) {
				hContact = FindUser(_wtoi(gch->ptszUID), true);
				db_set_b(hContact, "CList", "Hidden", 1);
				db_set_b(hContact, "CList", "NotOnList", 1);
				db_set_dw(hContact, "Ignore", "Mask1", 0);
				RetrieveUserInfo(_wtoi(gch->ptszUID));
			}
			CallService(MS_MSG_SENDMESSAGEW, hContact);
		}
		break;

	case GC_USER_LOGMENU:
		LogMenuHook(cc, gch);
		break;

	case GC_USER_NICKLISTMENU:
		NickMenuHook(cc, gch);
		break;
	}
	return 0;
}

void CVkProto::OnSendChatMsg(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnSendChatMsg %d", reply->resultCode);
	int iResult = ACKRESULT_FAILED;
	if (reply->resultCode == 200) {
		JSONNode jnRoot;
		CheckJsonResponse(pReq, reply, jnRoot);
		iResult = ACKRESULT_SUCCESS;
	}
	if (!pReq->pUserInfo)
		return;

	CVkFileUploadParam *fup = (CVkFileUploadParam *)pReq->pUserInfo;
	ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, iResult, (HANDLE)(fup));
	if (!pReq->bNeedsRestart || m_bTerminated) {
		delete fup;
		pReq->pUserInfo = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

LPTSTR CVkProto::ChangeChatTopic(CVkChatInfo *cc)
{
	ENTER_STRING pForm = { sizeof(pForm) };
	pForm.type = ESF_MULTILINE;
	pForm.caption = TranslateT("Enter new chat title");
	pForm.ptszInitVal = cc->m_wszTopic;
	pForm.szModuleName = m_szModuleName;
	pForm.szDataPrefix = "gctopic_";
	return (!EnterString(&pForm)) ? nullptr : pForm.ptszResult;
}

void CVkProto::LogMenuHook(CVkChatInfo *cc, GCHOOK *gch)
{
	if (!IsOnline())
		return;

	switch (gch->dwData) {
	case IDM_TOPIC:
		if (LPTSTR pwszNew = ChangeChatTopic(cc)) {
			Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.editChat.json", true, &CVkProto::OnReceiveSmth)
				<< WCHAR_PARAM("title", pwszNew)
				<< INT_PARAM("chat_id", cc->m_chatid));
			mir_free(pwszNew);
		}
		break;

	case IDM_INVITE:
		{
			CVkInviteChatForm dlg(this);
			if (dlg.DoModal() && dlg.m_hContact != 0) {
				int uid = getDword(dlg.m_hContact, "ID", VK_INVALID_USER);
				if (uid != VK_INVALID_USER)
					Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.addChatUser.json", true, &CVkProto::OnReceiveSmth)
						<< INT_PARAM("user_id", uid)
						<< INT_PARAM("chat_id", cc->m_chatid));
			}
		}
		break;

	case IDM_DESTROY:
		if (IDYES == MessageBoxW(nullptr,
			TranslateT("This chat is going to be destroyed forever with all its contents. This action cannot be undone. Are you sure?"),
			TranslateT("Warning"), MB_YESNO | MB_ICONQUESTION)
			)
			Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.DestroyChat", true, &CVkProto::OnChatDestroy)
				<< INT_PARAM("chatid", cc->m_chatid)
				<< INT_PARAM("userid", m_myUserId)
			)->pUserInfo = cc;

		break;
	}
}

INT_PTR __cdecl CVkProto::OnJoinChat(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::OnJoinChat");
	if (!IsOnline() || getBool(hContact, "kicked") || !getBool(hContact, "off"))
		return 1;

	int chat_id = getDword(hContact, "vk_chat_id", VK_INVALID_USER);
	if (chat_id == VK_INVALID_USER)
		return 1;

	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/messages.send.json", true, &CVkProto::OnSendChatMsg, AsyncHttpRequest::rpHigh)
		<< INT_PARAM("chat_id", chat_id)
		<< WCHAR_PARAM("message", m_vkOptions.pwszReturnChatMessage);
	pReq->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	Push(pReq);
	db_unset(hContact, m_szModuleName, "off");
	return 0;
}

INT_PTR __cdecl CVkProto::OnLeaveChat(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::OnLeaveChat");
	if (!IsOnline())
		return 1;

	ptrW wszChatID(getWStringA(hContact, "ChatRoomID"));
	if (wszChatID == nullptr)
		return 1;

	CVkChatInfo *cc = GetChatById(wszChatID);
	if (cc == nullptr)
		return 1;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.removeChatUser.json", true, &CVkProto::OnChatLeave)
		<< INT_PARAM("chat_id", cc->m_chatid)
		<< INT_PARAM("user_id", m_myUserId))->pUserInfo = cc;

	return 0;
}

void CVkProto::LeaveChat(int chat_id, bool close_window, bool delete_chat)
{
	debugLogA("CVkProto::LeaveChat");
	CVkChatInfo *cc = (CVkChatInfo*)m_chats.find((CVkChatInfo*)&chat_id);
	if (cc == nullptr)
		return;

	if (close_window)
		Chat_Terminate(m_szModuleName, cc->m_wszId);
	else
		Chat_Control(m_szModuleName, cc->m_wszId, SESSION_OFFLINE);

	if (delete_chat)
		DeleteContact(cc->m_hContact);
	else
		setByte(cc->m_hContact, "off", (int)true);
	m_chats.remove(cc);
}

void CVkProto::KickFromChat(int chat_id, int user_id, const JSONNode &jnMsg, const JSONNode &jnFUsers)
{
	debugLogA("CVkProto::KickFromChat (%d)", user_id);

	MCONTACT chatContact = FindChat(chat_id);
	if (chatContact && getBool(chatContact, "off"))
		return;

	if (user_id == m_myUserId)
		LeaveChat(chat_id);

	CVkChatInfo *cc = (CVkChatInfo*)m_chats.find((CVkChatInfo*)&chat_id);
	if (cc == nullptr)
		return;

	MCONTACT hContact = FindUser(user_id, false);
	CMStringW msg(jnMsg["body"].as_mstring());
	if (msg.IsEmpty()) {
		msg = TranslateT("You've been kicked by ");
		if (hContact != 0)
			msg += ptrW(db_get_wsa(hContact, m_szModuleName, "Nick"));
		else
			msg += TranslateT("(Unknown contact)");
	}
	else
		AppendChatMessage(chat_id, jnMsg, jnFUsers, false);

	MsgPopup(hContact, msg, TranslateT("Chat"));
	setByte(cc->m_hContact, "kicked", 1);
	LeaveChat(chat_id);
}

void CVkProto::OnChatLeave(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnChatLeave %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	CVkChatInfo *cc = (CVkChatInfo*)pReq->pUserInfo;
	LeaveChat(cc->m_chatid);
}

INT_PTR __cdecl CVkProto::SvcDestroyKickChat(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcDestroyKickChat");
	if (!IsOnline())
		return 1;

	if (!getBool(hContact, "off"))
		return 1;

	int chat_id = getDword(hContact, "vk_chat_id", VK_INVALID_USER);
	if (chat_id == VK_INVALID_USER)
		return 1;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.DestroyKickChat", true, &CVkProto::OnReceiveSmth)
		<< INT_PARAM("chatid", chat_id)
	);

	DeleteContact(hContact);

	return 0;
}

void CVkProto::OnChatDestroy(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnChatDestroy %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	CVkChatInfo *cc = (CVkChatInfo*)pReq->pUserInfo;
	LeaveChat(cc->m_chatid, true, true);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::NickMenuHook(CVkChatInfo *cc, GCHOOK *gch)
{
	CVkChatUser *cu = cc->GetUserById(gch->ptszUID);
	MCONTACT hContact;
	if (cu == nullptr)
		return;

	char szUid[20], szChatId[20];
	_itoa(cu->m_uid, szUid, 10);
	_itoa(cc->m_chatid, szChatId, 10);

	switch (gch->dwData) {
	case IDM_INFO:
		hContact = FindUser(cu->m_uid);
		if (hContact == 0) {
			hContact = FindUser(cu->m_uid, true);
			db_set_b(hContact, "CList", "Hidden", 1);
			db_set_b(hContact, "CList", "NotOnList", 1);
			db_set_dw(hContact, "Ignore", "Mask1", 0);
		}
		CallService(MS_USERINFO_SHOWDIALOG, hContact);
		break;

	case IDM_VISIT_PROFILE:
		hContact = FindUser(cu->m_uid);
		if (hContact == 0)
			Utils_OpenUrlW(CMStringW(FORMAT, L"https://vk.com/id%d", cu->m_uid));
		else
			SvcVisitProfile(hContact, 0);
		break;

	case IDM_CHANGENICK:
		{
			CMStringW wszNewNick = RunRenameNick(cu->m_wszNick);
			if (wszNewNick.IsEmpty() || wszNewNick == cu->m_wszNick)
				break;

			wchar_t wszId[20];
			_itow(cu->m_uid, wszId, 10);

			GCEVENT gce = { m_szModuleName, cc->m_wszId, GC_EVENT_NICK };
			gce.ptszNick = mir_wstrdup(cu->m_wszNick);
			gce.bIsMe = (cu->m_uid == m_myUserId);
			gce.ptszUID = wszId;
			gce.ptszText = mir_wstrdup(wszNewNick);
			gce.dwFlags = GCEF_ADDTOLOG;
			gce.time = time(nullptr);
			Chat_Event(&gce);

			cu->m_wszNick = mir_wstrdup(wszNewNick);
			setWString(cc->m_hContact, CMStringA(FORMAT, "nick%d", cu->m_uid), wszNewNick);
		}
		break;

	case IDM_KICK:
		if (!IsOnline())
			return;

		Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.removeChatUser.json", true, &CVkProto::OnReceiveSmth)
			<< INT_PARAM("chat_id", cc->m_chatid)
			<< INT_PARAM("user_id", cu->m_uid));
		cu->m_bUnknown = true;

		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static gc_item sttLogListItems[] =
{
	{ LPGENW("&Invite a user"), IDM_INVITE, MENU_ITEM },
	{ LPGENW("View/change &title"), IDM_TOPIC, MENU_ITEM },
	{ nullptr, 0, MENU_SEPARATOR },
	{ LPGENW("&Destroy room"), IDM_DESTROY, MENU_ITEM }
};

static gc_item sttListItems[] =
{
	{ LPGENW("&User details"), IDM_INFO, MENU_ITEM },
	{ LPGENW("Visit profile"), IDM_VISIT_PROFILE, MENU_ITEM },
	{ LPGENW("Change nick"), IDM_CHANGENICK, MENU_ITEM },
	{ LPGENW("&Kick"), IDM_KICK, MENU_ITEM }
};

int CVkProto::OnGcMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS *gcmi = (GCMENUITEMS*)lParam;
	if (gcmi == nullptr || mir_strcmpi(gcmi->pszModule, m_szModuleName))
		return 0;

	if (gcmi->Type == MENU_ON_LOG)
		Chat_AddMenuItems(gcmi->hMenu, _countof(sttLogListItems), sttLogListItems);
	else if (gcmi->Type == MENU_ON_NICKLIST)
		Chat_AddMenuItems(gcmi->hMenu, _countof(sttListItems), sttListItems);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::ChatContactTypingThread(void *p)
{
	CVKChatContactTypingParam *param = (CVKChatContactTypingParam *)p;
	if (!p)
		return;

	int iChatId = param->m_ChatId;
	int iUserId = param->m_UserId;

	debugLogA("CVkProto::ChatContactTypingThread %d %d", iChatId, iUserId);

	MCONTACT hChatContact = FindChat(iChatId);
	if (hChatContact && getBool(hChatContact, "off")) {
		delete param;
		return;
	}

	CVkChatInfo *cc = (CVkChatInfo*)m_chats.find((CVkChatInfo*)&iChatId);
	if (cc == nullptr) {
		delete param;
		return;
	}

	CVkChatUser *cu = cc->GetUserById(iUserId);
	if (cu == nullptr) {
		delete param;
		return;
	}

	{
		mir_cslock lck(m_csChatTyping);
		CVKChatContactTypingParam *cp = (CVKChatContactTypingParam *)m_ChatsTyping.find((CVKChatContactTypingParam *)&iChatId);
		if (cp != nullptr)
			m_ChatsTyping.remove(cp);
		m_ChatsTyping.insert(param);

		Srmm_SetStatusText(hChatContact, CMStringW(FORMAT, TranslateT("%s is typing a message..."), cu->m_wszNick));
	}

	Sleep(9500);
	StopChatContactTyping(iChatId, iUserId);
}

void CVkProto::StopChatContactTyping(int iChatId, int iUserId)
{
	debugLogA("CVkProto::StopChatContactTyping %d %d", iChatId, iUserId);
	MCONTACT hChatContact = FindChat(iChatId);
	if (hChatContact && getBool(hChatContact, "off"))
		return;

	CVkChatInfo *cc = (CVkChatInfo*)m_chats.find((CVkChatInfo*)&iChatId);
	if (cc == nullptr)
		return;

	CVkChatUser *cu = cc->GetUserById(iUserId);
	if (cu == nullptr)
		return;

	mir_cslock lck(m_csChatTyping);
	CVKChatContactTypingParam *cp = (CVKChatContactTypingParam *)m_ChatsTyping.find((CVKChatContactTypingParam *)&iChatId);

	if (cp != nullptr && cp->m_UserId == iUserId) {
		m_ChatsTyping.remove(cp);
		Srmm_SetStatusText(hChatContact, nullptr);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CVkProto::SvcCreateChat(WPARAM, LPARAM)
{
	if (!IsOnline())
		return (INT_PTR)1;

	CVkGCCreateForm dlg(this);
	return (INT_PTR)!dlg.DoModal();
}

void CVkProto::CreateNewChat(LPCSTR uids, LPCWSTR pwszTitle)
{
	if (!IsOnline())
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.createChat.json", true, &CVkProto::OnCreateNewChat)
		<< WCHAR_PARAM("title", pwszTitle ? pwszTitle : L"")
		<< CHAR_PARAM("user_ids", uids));
}

void CVkProto::OnCreateNewChat(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnCreateNewChat %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse)
		return;

	int chat_id = jnResponse.as_int();
	if (chat_id != 0)
		AppendChat(chat_id, nullNode);
}