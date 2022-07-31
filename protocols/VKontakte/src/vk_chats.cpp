/*
Copyright (c) 2013-22 Miranda NG team (https://miranda-ng.org)

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


CVkChatInfo* CVkProto::AppendConversationChat(int iChatId, const JSONNode& jnItem)
{
	debugLogW(L"CVkProto::AppendConversationChat %d", iChatId);
	if (iChatId == 0)
		return nullptr;


	const JSONNode& jnConversation = jnItem ? jnItem["conversation"] : nullNode;
	const JSONNode& jnLastMessage = jnItem ? jnItem["last_message"] : nullNode;
	const JSONNode& jnChatSettings = jnConversation ? jnConversation["chat_settings"] : nullNode;

	if (jnLastMessage) {
		const JSONNode& jnAction = jnLastMessage["action"];
		if (jnAction) {
			CMStringW wszActionType(jnAction["type"].as_mstring());
			if ((wszActionType == L"chat_kick_user") && (jnAction["member_id"].as_int() == m_myUserId))
				return nullptr;
		}
	}

	MCONTACT hChatContact = FindChat(iChatId);
	if (hChatContact && getBool(hChatContact, "kicked"))
		return nullptr;


	CVkChatInfo* vkChatInfo = m_chats.find((CVkChatInfo*)&iChatId);
	if (vkChatInfo != nullptr)
		return vkChatInfo;

	CMStringW wszTitle, wszState;
	vkChatInfo = new CVkChatInfo(iChatId);
	if (jnChatSettings) {
		wszTitle = jnChatSettings["title"].as_mstring();
		vkChatInfo->m_wszTopic = mir_wstrdup(!wszTitle.IsEmpty() ? wszTitle : L"");
		wszState = jnChatSettings["state"].as_mstring();
	}

	CMStringW sid;
	sid.Format(L"%S_%d", m_szModuleName, iChatId);
	vkChatInfo->m_wszId = mir_wstrdup(sid);


	SESSION_INFO* si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, sid, wszTitle);
	if (si == nullptr) {
		delete vkChatInfo;
		return nullptr;
	}

	vkChatInfo->m_hContact = si->hContact;
	setWString(si->hContact, "Nick", wszTitle);
	m_chats.insert(vkChatInfo);

	for (int i = _countof(sttStatuses) - 1; i >= 0; i--)
		Chat_AddGroup(si, TranslateW(sttStatuses[i]));

	setDword(si->hContact, "vk_chat_id", iChatId);

	CMStringW wszHomepage(FORMAT, L"https://vk.com/im?sel=c%d", iChatId);
	setWString(si->hContact, "Homepage", wszHomepage);

	db_unset(si->hContact, m_szModuleName, "off");

	if (jnChatSettings && wszState != L"in") {
		setByte(si->hContact, "off", 1);
		m_chats.remove(vkChatInfo);
		return nullptr;
	}

	Chat_Control(m_szModuleName, sid, (m_vkOptions.bHideChats) ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(m_szModuleName, sid, SESSION_ONLINE);

	RetrieveChatInfo(vkChatInfo);

	return vkChatInfo;

}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::RetrieveChatInfo(CVkChatInfo *cc)
{
	debugLogA("CVkProto::RetrieveChatInfo");

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.RetrieveChatInfo", true, &CVkProto::OnReceiveChatInfo)
		<< INT_PARAM("chatid", cc->m_iChatId)
		<< INT_PARAM("func_v", cc->m_bHistoryRead ? 1 : 3)
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
			LeaveChat(cc->m_iChatId);
			return;
		}
		cc->m_iAdminId = jnInfo["admin_id"].as_int();
	}


	if (!jnResponse["users"])
		return;

	const JSONNode &jnUsers = jnResponse["users"]["profiles"];

	if (jnUsers) {
		for (auto &it : cc->m_users)
			it->m_bDel = true;

		for (auto &jnUser : jnUsers) {
			if (!jnUser)
				break;

			LONG uid = jnUser["id"].as_int();
			bool bIsGroup = jnUser["type"].as_mstring() == L"group";
			if (bIsGroup)
				uid *= -1;

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
			if (wszNick.IsEmpty())
				wszNick = bIsGroup ?
					jnUser["name"].as_mstring() :
					jnUser["first_name"].as_mstring().Trim() + L" " + jnUser["last_name"].as_mstring().Trim();


			cu->m_wszNick = mir_wstrdup(wszNick);
			cu->m_bUnknown = false;

			if (bNew) {
				GCEVENT gce = { m_szModuleName, 0, GC_EVENT_JOIN };
				gce.pszID.w = cc->m_wszId;
				gce.bIsMe = uid == m_myUserId;
				gce.pszUID.w = wszId;
				gce.pszNick.w = wszNick;
				gce.pszStatus.w = TranslateW(sttStatuses[uid == cc->m_iAdminId]);
				gce.dwItemData = (INT_PTR)cu;
				Chat_Event(&gce);
			}
		}

		for (auto &cu : cc->m_users.rev_iter()) {
			if (!cu->m_bDel)
				continue;

			wchar_t wszId[20];
			_itow(cu->m_uid, wszId, 10);
			CMStringW wszNick(FORMAT, L"%s (%s)", cu->m_wszNick.get(), UserProfileUrl(cu->m_uid).c_str());

			GCEVENT gce = { m_szModuleName, 0, GC_EVENT_PART };
			gce.pszID.w = cc->m_wszId;
			gce.pszUID.w = wszId;
			gce.dwFlags = GCEF_NOTNOTIFY;
			gce.time = time(0);
			gce.pszNick.w = wszNick;
			Chat_Event(&gce);

			cc->m_users.removeItem(&cu);
		}
	}

	const JSONNode &jnMsgsUsers = jnResponse["msgs_users"];
	for (auto &jnUser : jnMsgsUsers) {
		LONG uid = jnUser["id"].as_int();
		CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&uid);
		if (cu)
			continue;

		MCONTACT hContact = FindUser(uid);
		if (hContact)
			continue;

		hContact = SetContactInfo(jnUser, true, VKContactType::vkContactMUCUser);

	}

	const JSONNode &jnMsgs = jnResponse["msgs"];
	const JSONNode &jnFUsers = jnResponse["fwd_users"];
	if (jnMsgs) {
		const JSONNode &jnItems = jnMsgs["items"];
		if (jnItems) {
			for (auto &jnMsg : jnItems) {
				if (!jnMsg)
					break;

				AppendChatConversationMessage(cc->m_iChatId, jnMsg, jnFUsers, true);
			}
			cc->m_bHistoryRead = true;
		}
	}

	for (auto &p : cc->m_msgs)
		AppendChatMessage(cc, p->m_uid, p->m_date, p->m_wszBody, p->m_bHistory, p->m_bIsAction);

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

void CVkProto::AppendChatConversationMessage(int id, const JSONNode& jnMsg, const JSONNode& jnFUsers, bool bIsHistory)
{
	debugLogA("CVkProto::AppendChatMessage");
	CVkChatInfo* cc = AppendConversationChat(id, nullNode);
	if (cc == nullptr)
		return;

	int mid = jnMsg["id"].as_int();
	int uid = jnMsg["from_id"].as_int();
	bool bIsAction = false;

	int msgTime = jnMsg["date"].as_int();
	time_t now = time(0);
	if (!msgTime || msgTime > now)
		msgTime = now;

	CMStringW wszBody(jnMsg["text"].as_mstring());

	const JSONNode& jnFwdMessages = jnMsg["fwd_messages"];
	if (jnFwdMessages && !jnFwdMessages.empty()) {
		CMStringW wszFwdMessages = GetFwdMessages(jnFwdMessages, jnFUsers, bbcNo);
		if (!wszBody.IsEmpty())
			wszFwdMessages = L"\n" + wszFwdMessages;
		wszBody += wszFwdMessages;
	}

	const JSONNode& jnReplyMessages = jnMsg["reply_message"];
	if (jnReplyMessages && !jnReplyMessages.empty()) {
		CMStringW wszReplyMessages = GetFwdMessages(jnReplyMessages, jnFUsers, bbcNo);
		if (!wszBody.IsEmpty())
			wszReplyMessages = L"\n" + wszReplyMessages;
		wszBody += wszReplyMessages;
	}

	const JSONNode& jnAttachments = jnMsg["attachments"];
	if (jnAttachments && !jnAttachments.empty()) {
		CMStringW wszAttachmentDescr = GetAttachmentDescr(jnAttachments, bbcNo);

		if (wszAttachmentDescr == L"== FilterAudioMessages ==")
			return;

		if (!wszBody.IsEmpty())
			wszAttachmentDescr = L"\n" + wszAttachmentDescr;
		wszBody += wszAttachmentDescr;
	}

	if (m_vkOptions.bAddMessageLinkToMesWAtt && ((jnAttachments && !jnAttachments.empty()) || (jnFwdMessages && !jnFwdMessages.empty()) || (jnReplyMessages && !jnReplyMessages.empty())))
		wszBody += SetBBCString(TranslateT("Message link"), bbcNo, vkbbcUrl,
			CMStringW(FORMAT, L"https://vk.com/im?sel=c%d&msgid=%d", cc->m_iChatId, mid));

	if (jnMsg["action"] && jnMsg["action"]["type"]) {
		bIsAction = true;
		CMStringW wszAction = jnMsg["action"]["type"].as_mstring();

		if (wszAction == L"chat_create") {
			CMStringW wszActionText = jnMsg["action"]["text"].as_mstring();
			wszBody.AppendFormat(L"%s \"%s\"", TranslateT("create chat"), wszActionText.IsEmpty() ? L" " : wszActionText.c_str());
		}
		else if (wszAction == L"chat_kick_user") {
			CMStringW wszActionMid = jnMsg["action"]["member_id"].as_mstring();
			if (wszActionMid.IsEmpty())
				wszBody = TranslateT("kick user");
			else {
				CMStringW wszUid(FORMAT, L"%d", uid);
				if (wszUid == wszActionMid) {
					if (cc->m_bHistoryRead)
						return;
					wszBody.AppendFormat(L" (%s) %s", UserProfileUrl(uid).c_str(), TranslateT("left chat"));
				}
				else {
					int a_uid = 0;
					int iReadCount = swscanf(wszActionMid, L"%d", &a_uid);
					if (iReadCount == 1) {
						CVkChatUser* cu = cc->m_users.find((CVkChatUser*)&a_uid);
						if (cu == nullptr)
							wszBody.AppendFormat(L"%s (%s)", TranslateT("kick user"), UserProfileUrl(a_uid).c_str());
						else
							wszBody.AppendFormat(L"%s %s (%s)", TranslateT("kick user"), cu->m_wszNick.get(), UserProfileUrl(a_uid).c_str());
					}
					else wszBody = TranslateT("kick user");
				}
			}
		}
		else if (wszAction == L"chat_invite_user" || wszAction == L"chat_invite_user_by_link") {
			CMStringW wszActionMid = jnMsg["action"]["member_id"].as_mstring();
			if (wszActionMid.IsEmpty())
				wszBody = TranslateT("invite user");
			else {
				CMStringW wszUid(FORMAT, L"%d", uid);
				if (wszUid == wszActionMid)
					wszBody.AppendFormat(L" (%s) %s", UserProfileUrl(uid).c_str(), TranslateT("returned to chat"));
				else {
					int a_uid = 0;
					int iReadCount = swscanf(wszActionMid, L"%d", &a_uid);
					if (iReadCount == 1) {
						CVkChatUser* cu = cc->m_users.find((CVkChatUser*)&a_uid);
						if (cu == nullptr)
							wszBody.AppendFormat(L"%s (%s)", TranslateT("invite user"), UserProfileUrl(a_uid).c_str());
						else
							wszBody.AppendFormat(L"%s %s (%s)", TranslateT("invite user"), cu->m_wszNick.get(), UserProfileUrl(a_uid).c_str());
					}
					else wszBody = TranslateT("invite user");
				}
			}
		}
		else if (wszAction == L"chat_title_update") {
			CMStringW wszTitle = jnMsg["action"]["text"].as_mstring();
			wszBody.AppendFormat(L"%s \"%s\"", TranslateT("change chat title to"), wszTitle.IsEmpty() ? L" " : wszTitle.c_str());

			if (!bIsHistory)
				SetChatTitle(cc, wszTitle);
		}
		else if (wszAction == L"chat_pin_message")
			wszBody = TranslateT("pin message");
		else if (wszAction == L"chat_unpin_message")
			wszBody = TranslateT("unpin message");
		else if (wszAction == L"chat_photo_update")
			wszBody.Replace(TranslateT("Attachments:"), TranslateT("changed chat cover:"));
		else if (wszAction == L"chat_photo_remove")
			wszBody = TranslateT("deleted chat cover");
		else
			wszBody.AppendFormat(L": %s (%s)", TranslateT("chat action not supported"), wszAction.c_str());
	}

	wszBody.Replace(L"%", L"%%");

	if (cc->m_bHistoryRead) {
		AppendChatMessage(cc, uid, msgTime, wszBody, bIsHistory, bIsAction);
	}
	else {
		CVkChatMessage* cm = cc->m_msgs.find((CVkChatMessage*)&mid);
		if (cm == nullptr)
			cc->m_msgs.insert(cm = new CVkChatMessage(mid));

		cm->m_uid = uid;
		cm->m_date = msgTime;
		cm->m_wszBody = mir_wstrdup(wszBody);
		cm->m_bHistory = bIsHistory;
		cm->m_bIsAction = bIsAction;
	}
}

void CVkProto::AppendChatMessage(CVkChatInfo *cc, LONG uid, int msgTime, LPCWSTR pwszBody, bool bIsHistory, bool bIsAction)
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

	GCEVENT gce = { m_szModuleName, 0, bIsAction ? GC_EVENT_ACTION : GC_EVENT_MESSAGE };
	gce.pszID.w = cc->m_wszId;
	gce.bIsMe = (uid == m_myUserId);
	gce.pszUID.w = wszId;
	gce.time = msgTime;
	gce.dwFlags = (bIsHistory) ? GCEF_NOTNOTIFY : GCEF_ADDTOLOG;
	gce.pszNick.w = cu->m_wszNick ? mir_wstrdup(cu->m_wszNick) : mir_wstrdup(hContact ? ptrW(db_get_wsa(hContact, m_szModuleName, "Nick")) : TranslateT("Unknown"));
	gce.pszText.w = IsEmpty((wchar_t *)pwszBody) ? mir_wstrdup(L"...") : mir_wstrdup(pwszBody);
	Chat_Event(&gce);
	StopChatContactTyping(cc->m_iChatId, uid);
}

/////////////////////////////////////////////////////////////////////////////////////////

CVkChatInfo* CVkProto::GetChatById(LPCWSTR pwszId)
{
	for (auto &it : m_chats)
		if (!mir_wstrcmp(it->m_wszId, pwszId))
			return it;

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

	if (mir_strcmpi(gch->si->pszModule, m_szModuleName))
		return 0;

	CVkChatInfo *cc = GetChatById(gch->si->ptszID);
	if (cc == nullptr)
		return 1;

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
				Contact::Hide(hContact);
				Contact::RemoveFromList(hContact);
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
	return 1;
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
	ENTER_STRING pForm = {};
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
				<< INT_PARAM("chat_id", cc->m_iChatId));
			mir_free(pwszNew);
		}
		break;

	case IDM_INVITE:
		{
			CVkInviteChatForm dlg(this);
			if (dlg.DoModal() && dlg.m_hContact != 0) {
				LONG uid = getDword(dlg.m_hContact, "ID", VK_INVALID_USER);

				if (uid < 0)
					MsgPopup(TranslateT("Adding bots to MUC is not supported"), TranslateT("Not supported"));
				else if (uid != VK_INVALID_USER)
					Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.addChatUser.json", true, &CVkProto::OnReceiveSmth)
						<< INT_PARAM("user_id", uid)
						<< INT_PARAM("chat_id", cc->m_iChatId));
			}
		}
		break;

	case IDM_DESTROY:
		if (IDYES == MessageBoxW(nullptr,
			TranslateT("This chat is going to be destroyed forever with all its contents. This action cannot be undone. Are you sure?"),
			TranslateT("Warning"), MB_YESNO | MB_ICONQUESTION)
			)
			Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.DestroyChat", true, &CVkProto::OnChatDestroy)
				<< INT_PARAM("chatid", cc->m_iChatId)
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

	AsyncHttpRequest* pReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/messages.addChatUser.json", true, &CVkProto::OnReceiveSmth, AsyncHttpRequest::rpHigh);
	pReq << INT_PARAM("user_id", m_myUserId);
	pReq<< INT_PARAM("chat_id", chat_id);
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
		<< INT_PARAM("chat_id", cc->m_iChatId)
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

void CVkProto::KickFromChat(int chat_id, LONG user_id, const JSONNode &jnMsg, const JSONNode &jnFUsers)
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

	CMStringW msg(jnMsg["text"].as_mstring());
	if (msg.IsEmpty()) {
		msg = TranslateT("You've been kicked by ");
		if (hContact != 0)
			msg += ptrW(db_get_wsa(hContact, m_szModuleName, "Nick"));
		else
			msg += TranslateT("(Unknown contact)");
	}
	else
		AppendChatConversationMessage(chat_id, jnMsg, jnFUsers, false);

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
	LeaveChat(cc->m_iChatId);
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
	LeaveChat(cc->m_iChatId, true, true);
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
	_itoa(cc->m_iChatId, szChatId, 10);

	switch (gch->dwData) {
	case IDM_INFO:
		hContact = FindUser(cu->m_uid);
		if (hContact == 0) {
			hContact = FindUser(cu->m_uid, true);
			Contact::Hide(hContact);
			Contact::RemoveFromList(hContact);
			db_set_dw(hContact, "Ignore", "Mask1", 0);
		}
		CallService(MS_USERINFO_SHOWDIALOG, hContact);
		break;

	case IDM_VISIT_PROFILE:
		hContact = FindUser(cu->m_uid);
		if (hContact == 0)
			Utils_OpenUrlW(UserProfileUrl(cu->m_uid));
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

			GCEVENT gce = { m_szModuleName, 0, GC_EVENT_NICK };
			gce.pszID.w = cc->m_wszId;
			gce.pszNick.w = mir_wstrdup(cu->m_wszNick);
			gce.bIsMe = (cu->m_uid == m_myUserId);
			gce.pszUID.w = wszId;
			gce.pszText.w = mir_wstrdup(wszNewNick);
			gce.dwFlags = GCEF_ADDTOLOG;
			gce.time = time(0);
			Chat_Event(&gce);

			cu->m_wszNick = mir_wstrdup(wszNewNick);
			setWString(cc->m_hContact, CMStringA(FORMAT, "nick%d", cu->m_uid), wszNewNick);
		}
		break;

	case IDM_KICK:
		if (!IsOnline())
			return;

		if (cu->m_uid < 0) {
			MsgPopup(TranslateT("Kick bots is not supported"), TranslateT("Not supported"));
			return;
		}

		Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.removeChatUser.json", true, &CVkProto::OnReceiveSmth)
			<< INT_PARAM("chat_id", cc->m_iChatId)
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
		Chat_AddMenuItems(gcmi->hMenu, _countof(sttLogListItems), sttLogListItems, &g_plugin);
	else if (gcmi->Type == MENU_ON_NICKLIST)
		Chat_AddMenuItems(gcmi->hMenu, _countof(sttListItems), sttListItems, &g_plugin);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::ChatContactTypingThread(void *p)
{
	CVKChatContactTypingParam *param = (CVKChatContactTypingParam *)p;
	if (!p)
		return;

	int iChatId = param->m_ChatId;
	LONG iUserId = param->m_UserId;

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

		Srmm_SetStatusText(hChatContact, CMStringW(FORMAT, TranslateT("%s is typing a message..."), cu->m_wszNick.get()));
	}

	Sleep(9500);
	StopChatContactTyping(iChatId, iUserId);
}

void CVkProto::StopChatContactTyping(int iChatId, LONG iUserId)
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
		AppendConversationChat(chat_id, nullNode);
}