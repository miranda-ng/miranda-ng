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

//////////////////////////////////////////////////////////////////////////////

int CVkProto::SendMsg(MCONTACT hContact, int, const char *szMsg)
{
	debugLogA("CVkProto::SendMsg");
	if (!IsOnline())
		return 0;

	bool bIsChat = isChatRoom(hContact);
	LONG iUserID = getDword(hContact, bIsChat ? "vk_chat_id" : "ID", VK_INVALID_USER);

	if (iUserID == VK_INVALID_USER || iUserID == VK_FEED_USER) {
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, 0);
		return 0;
	}

	int StickerId = 0;
	ptrA pszRetMsg(GetStickerId(szMsg, StickerId));

	ULONG uMsgId = ::InterlockedIncrement(&m_msgId);
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/messages.send.json", true,
		bIsChat ? &CVkProto::OnSendChatMsg : &CVkProto::OnSendMessage, AsyncHttpRequest::rpHigh);
	pReq << INT_PARAM(bIsChat ? "chat_id" : "peer_id", iUserID) << INT_PARAM("random_id", ((LONG)time(0)) * 100 + uMsgId % 100);
	pReq->AddHeader("Content-Type", "application/x-www-form-urlencoded");

	if (StickerId)
		pReq << INT_PARAM("sticker_id", StickerId);
	else {
		pReq << CHAR_PARAM("message", szMsg);
		if (m_vkOptions.bSendVKLinksAsAttachments) {
			CMStringA szAttachments = GetAttachmentsFromMessage(szMsg);
			if (!szAttachments.IsEmpty()) {
				debugLogA("CVkProto::SendMsg Attachments = %s", szAttachments.c_str());
				pReq << CHAR_PARAM("attachment", szAttachments);
			}
		}
	}

	if (!bIsChat)
		pReq->pUserInfo = new CVkSendMsgParam(hContact, uMsgId);

	Push(pReq);

	if (!m_vkOptions.bServerDelivery && !bIsChat)
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)uMsgId);

	if (!IsEmpty(pszRetMsg))
		SendMsg(hContact, 0, pszRetMsg);
	else if (m_iStatus == ID_STATUS_INVISIBLE)
		Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/account.setOffline.json", true, &CVkProto::OnReceiveSmth));

	return uMsgId;
}

void CVkProto::OnSendMessage(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	int iResult = ACKRESULT_FAILED;
	if (pReq->pUserInfo == nullptr) {
		debugLogA("CVkProto::OnSendMessage failed! (pUserInfo == nullptr)");
		return;
	}
	CVkSendMsgParam *param = (CVkSendMsgParam *)pReq->pUserInfo;

	debugLogA("CVkProto::OnSendMessage %d", reply->resultCode);
	UINT mid = 0;
	if (reply->resultCode == 200) {
		JSONNode jnRoot;
		const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
		if (jnResponse) {
			debugLogA("CVkProto::OnSendMessage jnResponse %d", jnResponse.as_int());
			switch (jnResponse.type()) {
			case JSON_NUMBER:
				mid = jnResponse.as_int();
				break;
			case JSON_STRING:
				if (swscanf(jnResponse.as_mstring(), L"%u", &mid) != 1)
					mid = 0;
				break;
			case JSON_ARRAY:
				mid = jnResponse.as_array()[json_index_t(0)].as_int();
				break;
			default:
				mid = 0;
			}

			if (mid > getDword(param->hContact, "lastmsgid"))
				setDword(param->hContact, "lastmsgid", mid);

			if (m_vkOptions.iMarkMessageReadOn >= MarkMsgReadOn::markOnReply)
				MarkMessagesRead(param->hContact);

			iResult = ACKRESULT_SUCCESS;
		}
	}

	char szMid[40];
	_itoa(mid, szMid, 10);

	if (param->pFUP) {
		ProtoBroadcastAck(param->hContact, ACKTYPE_FILE, iResult, (HANDLE)(param->pFUP));
		if (!pReq->bNeedsRestart || m_bTerminated)
			delete param->pFUP;
	}
	else if (m_vkOptions.bServerDelivery)
		ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, iResult, (HANDLE)(param->iMsgID), (LPARAM)szMid);

	if (!pReq->bNeedsRestart || m_bTerminated) {
		delete param;
		pReq->pUserInfo = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

int CVkProto::OnDbEventRead(WPARAM, LPARAM hDbEvent)
{
	debugLogA("CVkProto::OnDbEventRead");
	MCONTACT hContact = db_event_getContact(hDbEvent);
	if (!hContact)
		return 0;

	CMStringA szProto(Proto_GetBaseAccountName(hContact));
	if (szProto.IsEmpty() || szProto != m_szModuleName)
		return 0;

	if (m_vkOptions.iMarkMessageReadOn == MarkMsgReadOn::markOnRead)
		MarkMessagesRead(hContact);
	return 0;
}

INT_PTR CVkProto::SvcMarkMessagesAsRead(WPARAM hContact, LPARAM)
{
	MarkDialogAsRead(hContact);
	MarkMessagesRead(hContact);
	return 0;
}

void CVkProto::MarkMessagesRead(const MCONTACT hContact)
{
	debugLogA("CVkProto::MarkMessagesRead (hContact)");
	if (!IsOnline() || !hContact)
		return;

	if (!IsEmpty(ptrW(db_get_wsa(hContact, m_szModuleName, "Deactivated"))))
		return;

	LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
	if (userID == VK_INVALID_USER || userID == VK_FEED_USER)
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.markAsRead.json", true, &CVkProto::OnReceiveSmth, AsyncHttpRequest::rpLow)
		<< INT_PARAM("start_message_id", 0)
		<< INT_PARAM("peer_id", userID));
}

void CVkProto::RetrieveMessagesByIds(const CMStringA &mids)
{
	debugLogA("CVkProto::RetrieveMessagesByIds");
	if (!IsOnline() || mids.IsEmpty())
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.RetrieveMessagesConversationByIds", true, &CVkProto::OnReceiveMessages, AsyncHttpRequest::rpHigh)
		<< CHAR_PARAM("mids", mids)
	);
}

void CVkProto::RetrieveUnreadMessages()
{
	debugLogA("CVkProto::RetrieveUnreadMessages");
	if (!IsOnline())
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.RetrieveUnreadConversations", true, &CVkProto::OnReceiveDlgs, AsyncHttpRequest::rpHigh));
}

void CVkProto::OnReceiveMessages(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveMessages %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse)
		return;
	if (!jnResponse["Msgs"])
		return;

	CMStringA mids;
	int numMessages = jnResponse["Msgs"]["count"].as_int();
	const JSONNode &jnMsgs = jnResponse["Msgs"]["items"];
	const JSONNode &jnFUsers = jnResponse["fwd_users"];

	debugLogA("CVkProto::OnReceiveMessages numMessages = %d", numMessages);

	if (jnResponse["conv"]) {
		const JSONNode& jnConversation = jnResponse["conv"]["items"];
		for (auto& jnItem : jnConversation) {
			const JSONNode& jnPeer = jnItem["peer"];
			if (!jnPeer)
				break;

			CMStringW wszPeerType(jnPeer["type"].as_mstring());
			int iUserId = jnPeer["id"].as_int();

			if (wszPeerType == L"user" || wszPeerType == L"group") {
				MCONTACT hContact = FindUser(iUserId, true);
				setDword(hContact, "in_read", jnItem["in_read"].as_int());
				setDword(hContact, "out_read", jnItem["out_read"].as_int());
				if (m_vkOptions.iMarkMessageReadOn == MarkMsgReadOn::markOnReceive)
					MarkMessagesRead(hContact);
			}
			else {
				MCONTACT hContact = FindChat(iUserId % VK_CHAT_FLAG);
				MarkMessagesRead(hContact);
			}
		}
	}

	for (auto& jnMsg : jnMsgs) {
		if (!jnMsg) {
			debugLogA("CVkProto::OnReceiveMessages pMsg == nullptr");
			break;
		}

		UINT mid = jnMsg["id"].as_int();
		CMStringW wszBody(jnMsg["text"].as_mstring());
		UINT datetime = jnMsg["date"].as_int();
		int isOut = jnMsg["out"].as_int();
		int uid = jnMsg["peer_id"].as_int();

		MCONTACT hContact = 0;

		int chat_id = uid / VK_CHAT_FLAG ? uid % VK_CHAT_FLAG : 0;
		if (chat_id == 0)
			hContact = FindUser(uid, true);

		char szMid[40];
		_itoa(mid, szMid, 10);

		bool bUseServerReadFlag = m_vkOptions.bSyncReadMessageStatusFromServer ? true : !m_vkOptions.bMesAsUnread;

		if (chat_id != 0) {
			debugLogA("CVkProto::OnReceiveMessages chat_id != 0");
			CMStringW action_chat = jnMsg["action"]["type"].as_mstring();
			int action_mid = _wtoi(jnMsg["action"]["member_id"].as_mstring());
			if ((action_chat == L"chat_kick_user") && (action_mid == m_myUserId))
				KickFromChat(chat_id, uid, jnMsg, jnFUsers);
			else {
				MCONTACT chatContact = FindChat(chat_id);
				if (chatContact && getBool(chatContact, "kicked", true))
					db_unset(chatContact, m_szModuleName, "kicked");
				AppendChatConversationMessage(chat_id, jnMsg, jnFUsers, false);
			}
			continue;
		}

		const JSONNode& jnFwdMessages = jnMsg["fwd_messages"];
		if (jnFwdMessages && !jnFwdMessages.empty()) {
			CMStringW wszFwdMessages = GetFwdMessages(jnFwdMessages, jnFUsers, m_vkOptions.BBCForAttachments());
			if (!wszBody.IsEmpty())
				wszFwdMessages = L"\n" + wszFwdMessages;
			wszBody += wszFwdMessages;
		}

		const JSONNode& jnReplyMessages = jnMsg["reply_message"];
		if (jnReplyMessages && !jnReplyMessages.empty()) {
			CMStringW wszReplyMessages = GetFwdMessages(jnReplyMessages, jnFUsers, m_vkOptions.BBCForAttachments());
			if (!wszBody.IsEmpty())
				wszReplyMessages = L"\n" + wszReplyMessages;
			wszBody += wszReplyMessages;
		}

		CMStringW wszBodyNoAttachments = wszBody;

		CMStringW wszAttachmentDescr;
		const JSONNode& jnAttachments = jnMsg["attachments"];
		if (jnAttachments && !jnAttachments.empty()) {
			wszAttachmentDescr = GetAttachmentDescr(jnAttachments, m_vkOptions.BBCForAttachments());

			if (wszAttachmentDescr == L"== FilterAudioMessages ==") {
				if (hContact && (mid > getDword(hContact, "lastmsgid", -1)))
					setDword(hContact, "lastmsgid", mid);
				continue;
			}

			if (!wszBody.IsEmpty())
				wszBody += L"\n";
			wszBody += wszAttachmentDescr;
		}

		if (m_vkOptions.bAddMessageLinkToMesWAtt && ((jnAttachments && !jnAttachments.empty()) || (jnFwdMessages && !jnFwdMessages.empty()) || (jnReplyMessages && !jnReplyMessages.empty())))
			wszBody += SetBBCString(TranslateT("Message link"), m_vkOptions.BBCForAttachments(), vkbbcUrl,
				CMStringW(FORMAT, L"https://vk.com/im?sel=%d&msgid=%d", uid, mid));

		int iReadMsg = getDword(hContact, "in_read", 0);
		int isRead = (mid <= iReadMsg);

		time_t update_time = (time_t)jnMsg["update_time"].as_int();
		bool bEdited = (update_time != 0);

		if (bEdited) {
			wchar_t ttime[64];
			_locale_t locale = _create_locale(LC_ALL, "");
			_wcsftime_l(ttime, _countof(ttime), TranslateT("%x at %X"), localtime(&update_time), locale);
			_free_locale(locale);

			wszBody = SetBBCString(
				CMStringW(FORMAT, TranslateT("Edited message (updated %s):\n"), ttime),
				m_vkOptions.BBCForAttachments(), vkbbcB) +
				wszBody;

			CMStringW wszOldMsg;
			if (GetMessageFromDb(mid, datetime, wszOldMsg))
				wszBody += SetBBCString(TranslateT("\nOriginal message:\n"), m_vkOptions.BBCForAttachments(), vkbbcB) +
				wszOldMsg;
		}

		PROTORECVEVENT recv = {};

		if (isRead && bUseServerReadFlag)
			recv.flags |= PREF_CREATEREAD;

		if (isOut)
			recv.flags |= PREF_SENT;
		else if (m_vkOptions.bUserForceInvisibleOnActivity && time(0) - datetime < 60 * m_vkOptions.iInvisibleInterval)
			SetInvisible(hContact);

		T2Utf pszBody(wszBody);
		recv.timestamp = bEdited ? datetime : (m_vkOptions.bUseLocalTime ? time(0) : datetime);
		recv.szMessage = pszBody;

		debugLogA("CVkProto::OnReceiveMessages mid = %d, datetime = %d, isOut = %d, isRead = %d, uid = %d, Edited = %d", mid, datetime, isOut, isRead, uid, (int)bEdited);

		if (!IsMessageExist(mid, vkALL) || bEdited) {
			debugLogA("CVkProto::OnReceiveMessages new or edited message");
			recv.szMsgId = szMid;
			ProtoChainRecvMsg(hContact, &recv);
			if (mid > getDword(hContact, "lastmsgid", -1))
				setDword(hContact, "lastmsgid", mid);
		}
		else if (m_vkOptions.bLoadSentAttachments && !wszAttachmentDescr.IsEmpty()) {
			CMStringW wszOldMsg;

			if (GetMessageFromDb(mid, datetime, wszOldMsg) && (wszOldMsg == wszBody))
				continue;

			if (wszBodyNoAttachments != wszOldMsg)
				continue;

			debugLogA("CVkProto::OnReceiveMessages add attachments");

			T2Utf pszAttach(wszAttachmentDescr);
			recv.timestamp = isOut ? time(0) : datetime;
			recv.szMessage = pszAttach;
			recv.szMsgId = strcat(szMid, "_");
			ProtoChainRecvMsg(hContact, &recv);
		}
	}
}

void CVkProto::OnReceiveDlgs(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveDlgs %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse)
		return;

	const JSONNode &jnDialogs = jnResponse["dialogs"];
	if (!jnDialogs)
		return;

	const JSONNode &jnDlgs = jnDialogs["items"];
	if (!jnDlgs)
		return;

	LIST<void> lufUsers(20, PtrKeySortT);
	const JSONNode &jnUsers = jnResponse["users"];
	if (jnUsers)
		for (auto &it : jnUsers) {
			int iUserId = it["user_id"].as_int();
			int iStatus = it["friend_status"].as_int();

			// iStatus == 3 - user is friend
			// uid < 0 - user is group
			if (iUserId < 0 || iStatus != 3 || lufUsers.indexOf((HANDLE)iUserId) != -1)
				continue;

			lufUsers.insert((HANDLE)iUserId);
		}

	const JSONNode &jnGroups = jnResponse["groups"];
	if (jnGroups)
		for (auto &it : jnGroups) {
			int iUserId = 1000000000 + it.as_int();

			if (lufUsers.indexOf((HANDLE)iUserId) != -1)
				continue;

			lufUsers.insert((HANDLE)iUserId);
		}

	CMStringA szGroupIds;

	for (auto& it : jnDlgs) {
		if (!it)
			break;

		const JSONNode& jnConversation = it["conversation"];
		const JSONNode& jnLastMessage = it["last_message"];

		if (!jnConversation)
			break;

		int iUnreadCount = jnConversation["unread_count"].as_int();

		const JSONNode& jnPeer = jnConversation["peer"];
		if (!jnPeer)
			break;

		int iUserId = 0;
		MCONTACT hContact(0);
		CMStringW wszPeerType(jnPeer["type"].as_mstring());

		if (wszPeerType == L"user" || wszPeerType == L"group") {
			iUserId = jnPeer["id"].as_int();
			int iSearchId = (wszPeerType == L"group") ?  (1000000000 - iUserId) : iUserId;
			int iIndex = lufUsers.indexOf((HANDLE)iSearchId);

			debugLogA("CVkProto::OnReceiveDlgs UserId = %d, iIndex = %d, numUnread = %d", iUserId, iIndex, iUnreadCount);

			if (m_vkOptions.bLoadOnlyFriends && iUnreadCount == 0 && iIndex == -1)
				continue;

			hContact = FindUser(iUserId, true);
			debugLogA("CVkProto::OnReceiveDlgs add UserId = %d", iUserId);

			if (IsGroupUser(hContact))
				szGroupIds.AppendFormat(szGroupIds.IsEmpty() ? "%d" : ",%d", -1 * iUserId);

			setDword(hContact, "in_read", jnConversation["in_read"].as_int());
			setDword(hContact, "out_read", jnConversation["out_read"].as_int());

			if (g_bMessageState) {
				bool bIsOut = jnLastMessage["out"].as_bool();
				bool bIsRead = (jnLastMessage["id"].as_int() <= jnConversation["in_read"].as_int());

				if (bIsRead && bIsOut)
					CallService(MS_MESSAGESTATE_UPDATE, hContact, MRD_TYPE_DELIVERED);
			}
		}

		if (wszPeerType == L"chat") {
			int iChatId = jnPeer["local_id"].as_int();
			debugLogA("CVkProto::OnReceiveDlgs chatid = %d", iChatId);
			if (m_chats.find((CVkChatInfo*)&iChatId) == nullptr)
				AppendConversationChat(iChatId, it);
		}
		else if (m_vkOptions.iSyncHistoryMetod) {
			int iMessageId = jnLastMessage["id"].as_int();
			m_bNotifyForEndLoadingHistory = false;

			if (getDword(hContact, "lastmsgid", -1) == -1 && iUnreadCount && !getBool(hContact, "ActiveHistoryTask")) {
				setByte(hContact, "ActiveHistoryTask", 1);
				GetServerHistory(hContact, 0, iUnreadCount, 0, 0, true);
			}
			else
				GetHistoryDlg(hContact, iMessageId);

			if (m_vkOptions.iMarkMessageReadOn == MarkMsgReadOn::markOnReceive && iUnreadCount)
				MarkMessagesRead(hContact);
		}
		else if (iUnreadCount && !getBool(hContact, "ActiveHistoryTask")) {

			m_bNotifyForEndLoadingHistory = false;
			setByte(hContact, "ActiveHistoryTask", 1);
			GetServerHistory(hContact, 0, iUnreadCount, 0, 0, true);

			if (m_vkOptions.iMarkMessageReadOn == MarkMsgReadOn::markOnReceive)
				MarkMessagesRead(hContact);
		}
	}
	lufUsers.destroy();
	RetrieveUsersInfo();
	RetrieveGroupInfo(szGroupIds);
}