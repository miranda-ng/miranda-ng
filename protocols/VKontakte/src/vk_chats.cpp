/*
Copyright (c) 2013-24 Miranda NG team (https://miranda-ng.org)

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


INT_PTR __cdecl CVkProto::SvcChatChangeTopic(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcChatChangeTopic");
	if (!IsOnline())
		return 1;
	
	CVkChatInfo* cc = GetChatByContact(hContact);
	if (!cc)
		return 1;
		
	if (LPTSTR pwszNew = ChangeChatTopic(cc)) {
		Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.editChat.json", true, &CVkProto::OnReceiveSmth)
			<< WCHAR_PARAM("title", pwszNew)
			<< INT_PARAM("chat_id", cc->m_iChatId));
		mir_free(pwszNew);
	}

	return 0;
}

INT_PTR __cdecl CVkProto::SvcChatInviteUser(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcChatInviteUser");
	if (!IsOnline())
		return 1;

	CVkChatInfo* cc = GetChatByContact(hContact);
	if (!cc)
		return 1;

	CVkInviteChatForm dlg(this);
	if (dlg.DoModal() && dlg.m_hContact != 0) {
		VKUserID_t iUserId = ReadVKUserID(dlg.m_hContact);

		if (GetVKPeerType(iUserId) != VKPeerType::vkPeerUser)
			MsgPopup(TranslateT("Adding bots, MUC or groups to MUC is not supported"), TranslateT("Not supported"));
		else
			Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.addChatUser.json", true, &CVkProto::OnReceiveSmth)
				<< INT_PARAM("user_id", iUserId)
				<< INT_PARAM("chat_id", cc->m_iChatId));
	}

	return 0;
}

INT_PTR __cdecl CVkProto::SvcChatDestroy(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcChatDestroy");
	if (!IsOnline())
		return 1;

	CVkChatInfo* cc = GetChatByContact(hContact);
	if (!cc)
		return 1;
	if (IDYES == MessageBoxW(nullptr,
		TranslateT("This chat is going to be destroyed forever with all its contents. This action cannot be undone. Are you sure?"),
		TranslateT("Warning"), MB_YESNO | MB_ICONQUESTION)
		) {
		if (!getBool(hContact, "off"))
			Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.DestroyChat", true, &CVkProto::OnChatDestroy)
				<< INT_PARAM("chatid", cc->m_iChatId)
				<< INT_PARAM("userid", m_iMyUserId)
			)->pUserInfo = cc;
		else {
			Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.DestroyKickChat", true, &CVkProto::OnReceiveSmth)
				<< INT_PARAM("chatid", cc->m_iChatId)
			);
			DeleteContact(hContact);
		}
	}

	return 0;
}


CVkChatInfo* CVkProto::AppendConversationChat(VKUserID_t iChatId, const JSONNode& jnItem)
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
			if ((wszActionType == L"chat_kick_user") && (jnAction["member_id"].as_int() == m_iMyUserId))
				return nullptr;
		}
	}

	MCONTACT hChatContact = FindChat(iChatId);
	if (hChatContact && getBool(hChatContact, "kicked"))
		return nullptr;

	CVkChatInfo* vkChatInfo = m_chats.find((CVkChatInfo*)&iChatId);
	if (vkChatInfo != nullptr)
		return vkChatInfo;

	CMStringW wszTitle, wszState, wszAvatar;
	vkChatInfo = new CVkChatInfo(iChatId);
	if (jnChatSettings) {
		wszTitle = jnChatSettings["title"].as_mstring();
		vkChatInfo->m_wszTopic = mir_wstrdup(!wszTitle.IsEmpty() ? wszTitle : L"");
		wszState = jnChatSettings["state"].as_mstring();
		wszAvatar = jnChatSettings["photo"] ? jnChatSettings["photo"]["photo_100"].as_mstring() : L"";
	}

	CMStringW sid;
	sid.Format(L"%d", iChatId);

	SESSION_INFO* si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, sid, wszTitle);
	if (si == nullptr) {
		delete vkChatInfo;
		return nullptr;
	}

	vkChatInfo->m_si = si;
	setWString(si->hContact, "Nick", wszTitle);
	m_chats.insert(vkChatInfo);

	for (int i = _countof(sttStatuses) - 1; i >= 0; i--)
		Chat_AddGroup(si, TranslateW(sttStatuses[i]));

	WriteVKUserID(si->hContact, iChatId);

	CMStringW wszHomepage(FORMAT, L"https://vk.com/im?sel=c%d", iChatId);
	setWString(si->hContact, "Homepage", wszHomepage);

	if (!wszAvatar.IsEmpty()) {
		SetAvatarUrl(si->hContact, wszAvatar);
		ReloadAvatarInfo(si->hContact);
	}

	db_unset(si->hContact, m_szModuleName, "off");

	if (jnChatSettings && wszState != L"in") {
		setByte(si->hContact, "off", 1);
		m_chats.remove(vkChatInfo);
		return nullptr;
	}

	Chat_Control(si, (m_vkOptions.bHideChats) ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(si, SESSION_ONLINE);

	RetrieveChatInfo(vkChatInfo);

	return vkChatInfo;

}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::RetrieveChatInfo(CVkChatInfo *cc)
{
	debugLogA("CVkProto::RetrieveChatInfo");

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.RetrieveChatInfo", true, &CVkProto::OnReceiveChatInfo)
		<< INT_PARAM("chatid", cc->m_iChatId)
		<< INT_PARAM("func_v", (cc->m_bHistoryRead || m_vkOptions.iSyncHistoryMetod) ? 5 : 4)
	)->pUserInfo = cc;
}

void CVkProto::OnReceiveChatInfo(MHttpResponse *reply, AsyncHttpRequest *pReq)
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

		
		CMStringW wszValue = jnInfo["photo_100"].as_mstring();
		if (!wszValue.IsEmpty()) {
			SetAvatarUrl(cc->m_si->hContact, wszValue);
			ReloadAvatarInfo(cc->m_si->hContact);
		}

		if (jnInfo["left"].as_bool() || jnInfo["kicked"].as_bool()) {
			setByte(cc->m_si->hContact, "kicked", jnInfo["kicked"].as_bool());
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

			VKUserID_t iUserId = jnUser["id"].as_int();
			bool bIsGroup = jnUser["type"].as_mstring() == L"group";
			if (bIsGroup)
				iUserId *= -1;

			wchar_t wszId[20];
			_ltow(iUserId, wszId, 10);

			bool bNew;
			CVkChatUser *iChatUser = cc->m_users.find((CVkChatUser*)&iUserId);
			if (iChatUser == nullptr) {
				cc->m_users.insert(iChatUser = new CVkChatUser(iUserId));
				bNew = true;
			}
			else
				bNew = iChatUser->m_bUnknown;
			iChatUser->m_bDel = false;

			CMStringW wszNick(ptrW(db_get_wsa(cc->m_si->hContact, m_szModuleName, CMStringA(FORMAT, "nick%d", iChatUser->m_iUserId))));
			if (wszNick.IsEmpty())
				wszNick = bIsGroup ?
					jnUser["name"].as_mstring() :
					jnUser["first_name"].as_mstring().Trim() + L" " + jnUser["last_name"].as_mstring().Trim();


			iChatUser->m_wszNick = mir_wstrdup(wszNick);
			iChatUser->m_bUnknown = false;

			if (bNew) {
				GCEVENT gce = { cc->m_si, GC_EVENT_JOIN };
				gce.bIsMe = iUserId == m_iMyUserId;
				gce.pszUID.w = wszId;
				gce.pszNick.w = wszNick;
				gce.pszStatus.w = TranslateW(sttStatuses[iUserId == cc->m_iAdminId]);
				gce.dwItemData = (INT_PTR)iChatUser;
				Chat_Event(&gce);
			}
		}

		for (auto &cu : cc->m_users.rev_iter()) {
			if (!cu->m_bDel)
				continue;

			wchar_t wszId[20];
			_itow(cu->m_iUserId, wszId, 10);
			CMStringW wszNick(FORMAT, L"%s (%s)", cu->m_wszNick.get(), UserProfileUrl(cu->m_iUserId).c_str());

			GCEVENT gce = { cc->m_si, GC_EVENT_PART };
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
		VKUserID_t iUserId = jnUser["id"].as_int();
		CVkChatUser *cu = cc->m_users.find((CVkChatUser*)&iUserId);
		if (cu)
			continue;

		MCONTACT hContact = FindUser(iUserId);
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
		}
	}

	cc->m_bHistoryRead = true;

	for (auto &p : cc->m_msgs)
		AppendChatMessage(cc, p->m_iMessageId, p->m_iReplyMsgId, p->m_iUserId, p->m_tDate, p->m_wszBody, p->m_bHistory, p->m_bIsAction);

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
	setWString(cc->m_si->hContact, "Nick", wszTopic);

	Chat_ChangeSessionName(cc->m_si, wszTopic);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::AppendChatConversationMessage(VKUserID_t iChatId, const JSONNode& jnMsg, const JSONNode& jnFUsers, bool bIsHistory)
{
	debugLogA("CVkProto::AppendChatConversationMessage");
	CVkChatInfo* vkChatInfo = AppendConversationChat(iChatId, nullNode);
	if (vkChatInfo == nullptr)
		return;

	VKMessageID_t iMessageId = jnMsg["id"].as_int(), iReplyMsgId = 0;
	VKUserID_t iUserId = jnMsg["from_id"].as_int();
	bool bIsAction = false;

	time_t tMsgTime = jnMsg["date"].as_int();
	time_t tNow = time(0);
	if (!tMsgTime || tMsgTime > tNow)
		tMsgTime = tNow;

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
		if (m_vkOptions.bShowReplyInMessage) {
			CMStringW wszReplyMessages = GetFwdMessages(jnReplyMessages, jnFUsers, bbcNo);
			if (!wszBody.IsEmpty())
				wszReplyMessages = L"\n" + wszReplyMessages;
			wszBody += wszReplyMessages;
		}
		else if (jnReplyMessages["id"])
			iReplyMsgId = jnReplyMessages["id"].as_int();
	}

	const JSONNode& jnAttachments = jnMsg["attachments"];
	if (jnAttachments && !jnAttachments.empty()) {
		CMStringW wszAttachmentDescr = GetAttachmentDescr(jnAttachments, bbcNo, vkChatInfo->m_si->hContact, iMessageId);

		if (wszAttachmentDescr == L"== FilterAudioMessages ==")
			return;

		if (!wszBody.IsEmpty())
			wszAttachmentDescr = L"\n" + wszAttachmentDescr;
		wszBody += wszAttachmentDescr;
	}

	VKMessageID_t iReadMsg = ReadQSWord(vkChatInfo->m_si->hContact, "in_read", 0);
	bool bIsRead = (iMessageId <= iReadMsg);

	time_t tUpdateTime = (time_t)jnMsg["update_time"].as_int();
	bool bEdited = (tUpdateTime != 0);

	if (bEdited) {
		wchar_t ttime[64];
		_locale_t locale = _create_locale(LC_ALL, "");
		_wcsftime_l(ttime, _countof(ttime), TranslateT("%x at %X"), localtime(&tUpdateTime), locale);
		_free_locale(locale);

		wszBody = SetBBCString(
			CMStringW(FORMAT, TranslateT("Edited message (updated %s):\n"), ttime),
			m_vkOptions.BBCForAttachments(), vkbbcB) +
			wszBody;
		
		if (m_vkOptions.bShowBeforeEditedPostVersion) {
			CMStringW wszOldMsg;
			if (GetMessageFromDb(iMessageId, tMsgTime, wszOldMsg))
				wszBody += SetBBCString(TranslateT("\nOriginal message:\n"), m_vkOptions.BBCForAttachments(), vkbbcB) +
				wszOldMsg;
		}
	}

	if (m_vkOptions.bAddMessageLinkToMesWAtt && ((jnAttachments && !jnAttachments.empty()) || (jnFwdMessages && !jnFwdMessages.empty()) || (jnReplyMessages && !jnReplyMessages.empty() && m_vkOptions.bShowReplyInMessage)))
		wszBody += SetBBCString(TranslateT("Message link"), bbcNo, vkbbcUrl,
			CMStringW(FORMAT, L"https://vk.com/im?sel=c%d&msgid=%d", vkChatInfo->m_iChatId, iMessageId));
	

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
				CMStringW wszUid(FORMAT, L"%d", iUserId);
				if (wszUid == wszActionMid) {
					if (vkChatInfo->m_bHistoryRead)
						return;
					wszBody.AppendFormat(L" (%s) %s", UserProfileUrl(iUserId).c_str(), TranslateT("left chat"));
				}
				else {
					VKUserID_t iActionUserId = 0;
					int iReadCount = swscanf(wszActionMid, L"%d", &iActionUserId);
					if (iReadCount == 1) {
						CVkChatUser* cu = vkChatInfo->m_users.find((CVkChatUser*)&iActionUserId);
						if (cu == nullptr)
							wszBody.AppendFormat(L"%s (%s)", TranslateT("kick user"), UserProfileUrl(iActionUserId).c_str());
						else
							wszBody.AppendFormat(L"%s %s (%s)", TranslateT("kick user"), cu->m_wszNick.get(), UserProfileUrl(iActionUserId).c_str());
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
				CMStringW wszUid(FORMAT, L"%d", iUserId);
				if (wszUid == wszActionMid)
					wszBody.AppendFormat(L" (%s) %s", UserProfileUrl(iUserId).c_str(), TranslateT("returned to chat"));
				else {
					VKUserID_t iActionUserId = 0;
					int iReadCount = swscanf(wszActionMid, L"%d", &iActionUserId);
					if (iReadCount == 1) {
						CVkChatUser* cu = vkChatInfo->m_users.find((CVkChatUser*)&iActionUserId);
						if (cu == nullptr)
							wszBody.AppendFormat(L"%s (%s)", TranslateT("invite user"), UserProfileUrl(iActionUserId).c_str());
						else
							wszBody.AppendFormat(L"%s %s (%s)", TranslateT("invite user"), cu->m_wszNick.get(), UserProfileUrl(iActionUserId).c_str());
					}
					else wszBody = TranslateT("invite user");
				}
			}
		}
		else if (wszAction == L"chat_title_update") {
			CMStringW wszTitle = jnMsg["action"]["text"].as_mstring();
			wszBody.AppendFormat(L"%s \"%s\"", TranslateT("change chat title to"), wszTitle.IsEmpty() ? L" " : wszTitle.c_str());

			if (!bIsHistory)
				SetChatTitle(vkChatInfo, wszTitle);
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

	if (vkChatInfo->m_bHistoryRead) {
		AppendChatMessage(vkChatInfo, iMessageId, iReplyMsgId, iUserId, tMsgTime, wszBody, bIsHistory, bIsAction);
	}
	else {
		CVkChatMessage* vkChatMessage = vkChatInfo->m_msgs.find((CVkChatMessage*)&iMessageId);
		if (vkChatMessage == nullptr)
			vkChatInfo->m_msgs.insert(vkChatMessage = new CVkChatMessage(iMessageId));
		
		vkChatMessage->m_iReplyMsgId = iReplyMsgId;
		vkChatMessage->m_iUserId = iUserId;
		vkChatMessage->m_tDate = tMsgTime;
		vkChatMessage->m_wszBody = mir_wstrdup(wszBody);
		vkChatMessage->m_bHistory = bIsHistory;
		vkChatMessage->m_bIsRead = bIsRead;
		vkChatMessage->m_bIsAction = bIsAction;
	}
}


void CVkProto::AppendChatMessage(CVkChatInfo* vkChatInfo, VKMessageID_t iMessageId, VKMessageID_t iReplyMsgId, VKUserID_t iUserId, time_t tMsgTime, LPCWSTR pwszBody, bool bIsHistory, bool bIsRead, bool bIsAction)
{
	debugLogA("CVkProto::AppendChatMessage2");

	MCONTACT hChatContact = vkChatInfo->m_si->hContact;
	if (!hChatContact)
		return;

	MCONTACT hContact = FindTempUser(iUserId);
	
	CVkChatUser* cu = vkChatInfo->m_users.find((CVkChatUser*)&iUserId);
	if (cu == nullptr) {
		vkChatInfo->m_users.insert(cu = new CVkChatUser(iUserId));
		CMStringW wszNick(ptrW(db_get_wsa(vkChatInfo->m_si->hContact, m_szModuleName, CMStringA(FORMAT, "nick%d", cu->m_iUserId))));
		cu->m_wszNick = mir_wstrdup(wszNick.IsEmpty() ? (hContact ? ptrW(db_get_wsa(hContact, m_szModuleName, "Nick")) : TranslateT("Unknown")) : wszNick);
		cu->m_bUnknown = true;
	}

	if (bIsAction) {
		wchar_t wszId[20];
		_itow(iUserId, wszId, 10);

		GCEVENT gce = { vkChatInfo->m_si, GC_EVENT_ACTION };
		gce.bIsMe = (iUserId == m_iMyUserId);
		gce.pszUID.w = wszId;
		gce.time = tMsgTime;
		gce.dwFlags = (bIsHistory) ? GCEF_NOTNOTIFY : GCEF_ADDTOLOG;
		gce.pszNick.w = cu->m_wszNick ? mir_wstrdup(cu->m_wszNick) : mir_wstrdup(hContact ? ptrW(db_get_wsa(hContact, m_szModuleName, "Nick")) : TranslateT("Unknown"));
		gce.pszText.w = IsEmpty((wchar_t*)pwszBody) ? mir_wstrdup(L"...") : mir_wstrdup(pwszBody);
		Chat_Event(&gce);
	}
	else {
		char szReplyMsgId[40] = "";
		char szMid[40];

		_ltoa(iMessageId, szMid, 10);

		T2Utf pszBody(pwszBody);
		T2Utf pszNick(cu->m_wszNick);
		
		DB::EventInfo dbei;
		dbei.szId = szMid;
		dbei.timestamp = tMsgTime;
		dbei.pBlob = pszBody;
		if (iUserId == m_iMyUserId)
			dbei.flags |= DBEF_SENT;
		if (bIsHistory || bIsRead)
			dbei.flags |= DBEF_READ;
		dbei.szUserId = pszNick;
		
		if (iReplyMsgId) {
			_ltoa(iReplyMsgId, szReplyMsgId, 10);
			dbei.szReplyId = szReplyMsgId;
		}

		ProtoChainRecvMsg(hChatContact, dbei);
	}

	StopChatContactTyping(vkChatInfo->m_iChatId, iUserId);
}

/////////////////////////////////////////////////////////////////////////////////////////

CVkChatInfo* CVkProto::GetChatByContact(MCONTACT hContact)
{
	if (!isChatRoom(hContact))
		return nullptr;
	
	VKUserID_t iUserId = ReadVKUserID(hContact);
	if (iUserId == VK_INVALID_USER)
		return nullptr;

	wchar_t wszChatID[40];
	_itow(iUserId, wszChatID, 10);
	return GetChatById(wszChatID);
}

CVkChatInfo* CVkProto::GetChatById(LPCWSTR pwszId)
{
	for (auto &it : m_chats)
		if (!mir_wstrcmp(it->m_si->ptszID, pwszId))
			return it;

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::SetChatStatus(MCONTACT hContact, int iStatus)
{
	CVkChatInfo *cc = GetChatByContact(hContact);
	if (cc != nullptr)
		Chat_Control(cc->m_si, (iStatus == ID_STATUS_OFFLINE) ? SESSION_OFFLINE : SESSION_ONLINE);
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
			SendMsg(cc->m_si->hContact, gch->si->pDlg ? gch->si->pDlg->m_hQuoteEvent : 0, T2Utf(pwszBuf));
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

	case GC_USER_TYPNOTIFY:
		UserIsTyping(cc->m_si->hContact, (int)gch->dwData);
		break;
	}
	return 1;
}

void CVkProto::OnSendChatMsg(MHttpResponse *reply, AsyncHttpRequest *pReq)
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
		SvcChatChangeTopic(cc->m_si->hContact, 0);
		break;

	case IDM_INVITE:
		SvcChatInviteUser(cc->m_si->hContact, 0);
		break;

	case IDM_DESTROY:
		SvcChatDestroy(cc->m_si->hContact, 0);
		break;
	}
}

INT_PTR __cdecl CVkProto::OnJoinChat(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::OnJoinChat");
	if (!IsOnline() || getBool(hContact, "kicked") || !getBool(hContact, "off"))
		return 1;

	VKUserID_t iChatId = ReadVKUserID(hContact);
	if (iChatId == VK_INVALID_USER)
		return 1;

	AsyncHttpRequest* pReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/messages.addChatUser.json", true, &CVkProto::OnReceiveSmth, AsyncHttpRequest::rpHigh);
	pReq << INT_PARAM("user_id", m_iMyUserId);
	pReq<< INT_PARAM("chat_id", iChatId);
	Push(pReq);
	db_unset(hContact, m_szModuleName, "off");
	return 0;
}

INT_PTR __cdecl CVkProto::OnLeaveChat(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::OnLeaveChat");
	if (!IsOnline())
		return 1;

	CVkChatInfo *cc = GetChatByContact(hContact);
	if (cc == nullptr)
		return 1;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.removeChatUser.json", true, &CVkProto::OnChatLeave)
		<< INT_PARAM("chat_id", cc->m_iChatId)
		<< INT_PARAM("user_id", m_iMyUserId))->pUserInfo = cc;

	return 0;
}

void CVkProto::LeaveChat(VKUserID_t iChatId, bool bCloseWindow, bool bDeleteChat)
{
	debugLogA("CVkProto::LeaveChat");
	CVkChatInfo *cc = (CVkChatInfo*)m_chats.find((CVkChatInfo*)&iChatId);
	if (cc == nullptr)
		return;

	if (bCloseWindow)
		Chat_Terminate(cc->m_si);
	else
		Chat_Control(cc->m_si, SESSION_OFFLINE);

	if (bDeleteChat)
		DeleteContact(cc->m_si->hContact);
	else
		setByte(cc->m_si->hContact, "off", (int)true);
	m_chats.remove(cc);
}

void CVkProto::KickFromChat(VKUserID_t iChatId, VKUserID_t iUserId, const JSONNode &jnMsg, const JSONNode &jnFUsers)
{
	debugLogA("CVkProto::KickFromChat (%d)", iUserId);

	MCONTACT chatContact = FindChat(iChatId);
	if (chatContact && getBool(chatContact, "off"))
		return;

	if (iUserId == m_iMyUserId)
		LeaveChat(iChatId);

	CVkChatInfo *cc = (CVkChatInfo*)m_chats.find((CVkChatInfo*)&iChatId);
	if (cc == nullptr)
		return;

	MCONTACT hContact = FindUser(iUserId, false);

	CMStringW wszMsg(jnMsg["text"].as_mstring());
	if (wszMsg.IsEmpty()) {
		wszMsg = TranslateT("You've been kicked by ");
		if (hContact != 0)
			wszMsg += ptrW(db_get_wsa(hContact, m_szModuleName, "Nick"));
		else
			wszMsg += TranslateT("(Unknown contact)");
	}
	else
		AppendChatConversationMessage(iChatId, jnMsg, jnFUsers, false);

	MsgPopup(hContact, wszMsg, TranslateT("Chat"));
	setByte(cc->m_si->hContact, "kicked", 1);
	LeaveChat(iChatId);
}

void CVkProto::OnChatLeave(MHttpResponse *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnChatLeave %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	CVkChatInfo *cc = (CVkChatInfo*)pReq->pUserInfo;
	LeaveChat(cc->m_iChatId);
}


void CVkProto::OnChatDestroy(MHttpResponse *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnChatDestroy %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	CVkChatInfo *cc = (CVkChatInfo*)pReq->pUserInfo;
	LeaveChat(cc->m_iChatId, true, true);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::NickMenuHook(CVkChatInfo *vkChatInfo, GCHOOK *gch)
{
	CVkChatUser *vkChatUser = vkChatInfo->GetUserById(gch->ptszUID);
	MCONTACT hContact;
	if (vkChatUser == nullptr)
		return;

	char szUid[20], szChatId[20];
	_itoa(vkChatUser->m_iUserId, szUid, 10);
	_itoa(vkChatInfo->m_iChatId, szChatId, 10);

	switch (gch->dwData) {
	case IDM_INFO:
		hContact = FindTempUser(vkChatUser->m_iUserId, 1000);
		CallService(MS_USERINFO_SHOWDIALOG, hContact);
		break;

	case IDM_VISIT_PROFILE:
		hContact = FindUser(vkChatUser->m_iUserId);
		if (hContact == 0)
			Utils_OpenUrlW(UserProfileUrl(vkChatUser->m_iUserId));
		else
			SvcVisitProfile(hContact, 0);
		break;

	case IDM_CHANGENICK:
		{
			CMStringW wszNewNick = RunRenameNick(vkChatUser->m_wszNick);
			if (wszNewNick.IsEmpty() || wszNewNick == vkChatUser->m_wszNick)
				break;

			wchar_t wszId[20];
			_itow(vkChatUser->m_iUserId, wszId, 10);

			GCEVENT gce = { vkChatInfo->m_si, GC_EVENT_NICK };
			gce.pszNick.w = mir_wstrdup(vkChatUser->m_wszNick);
			gce.bIsMe = (vkChatUser->m_iUserId == m_iMyUserId);
			gce.pszUID.w = wszId;
			gce.pszText.w = mir_wstrdup(wszNewNick);
			gce.dwFlags = GCEF_ADDTOLOG;
			gce.time = time(0);
			Chat_Event(&gce);

			vkChatUser->m_wszNick = mir_wstrdup(wszNewNick);
			setWString(vkChatInfo->m_si->hContact, CMStringA(FORMAT, "nick%d", vkChatUser->m_iUserId), wszNewNick);
		}
		break;

	case IDM_KICK:
		if (!IsOnline())
			return;

		if (GetVKPeerType(vkChatUser->m_iUserId) != VKPeerType::vkPeerUser) {
			MsgPopup(TranslateT("Kick bots is not supported"), TranslateT("Not supported"));
			return;
		}

		Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.removeChatUser.json", true, &CVkProto::OnReceiveSmth)
			<< INT_PARAM("chat_id", vkChatInfo->m_iChatId)
			<< INT_PARAM("user_id", vkChatUser->m_iUserId));
		vkChatUser->m_bUnknown = true;

		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static gc_item sttLogListItems[] =
{
	{ LPGENW("&Invite a user"), IDM_INVITE, MENU_ITEM },
	{ LPGENW("View/change &topic"), IDM_TOPIC, MENU_ITEM },
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

	VKUserID_t iChatId = param->m_ChatId;
	VKUserID_t iUserId = param->m_UserId;

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

void CVkProto::StopChatContactTyping(VKUserID_t iChatId, VKUserID_t iUserId)
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

	
	CVkUserListForm dlg(
		this,
		"",
		TranslateT("Create group chat"),
		TranslateT("Mark users you want to invite to a new chat"),
		TranslateT("New chat's title:"),
		VKContactType::vkContactSelf | VKContactType::vkContactMUCUser | VKContactType::vkContactGroupUser
	);

	if (!dlg.DoModal())
		return 0;

	CMStringA szUIds;
	for (auto& hContact : dlg.lContacts) {
		if (isChatRoom((UINT_PTR)hContact))
			continue;

		VKUserID_t iUserId = ReadVKUserID((UINT_PTR)hContact);
		if (iUserId != 0) {
			if (!szUIds.IsEmpty())
				szUIds.AppendChar(',');
			szUIds.AppendFormat("%d", iUserId);
		}	
	}

	if (szUIds.IsEmpty())
		return 0;

	CreateNewChat(szUIds, dlg.wszMessage);
	
	return 1;
}

void CVkProto::CreateNewChat(LPCSTR uids, LPCWSTR pwszTitle)
{
	if (!IsOnline())
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.createChat.json", true, &CVkProto::OnCreateNewChat)
		<< WCHAR_PARAM("title", pwszTitle ? pwszTitle : L"")
		<< CHAR_PARAM("user_ids", uids));
}

void CVkProto::OnCreateNewChat(MHttpResponse *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnCreateNewChat %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse)
		return;

	VKUserID_t iChatId = jnResponse.as_int();
	if (iChatId != 0)
		AppendConversationChat(iChatId, nullNode);
}