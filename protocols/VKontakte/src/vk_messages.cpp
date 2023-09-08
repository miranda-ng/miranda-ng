/*
Copyright (c) 2013-23 Miranda NG team (https://miranda-ng.org)

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
	if (!IsOnline()) {
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, 0);
		return 0;
	}

	bool bIsChat = isChatRoom(hContact);

	VKUserID_t iUserId = ReadVKUserID(hContact);
	if (iUserId == VK_INVALID_USER || iUserId == VK_FEED_USER) {
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, 0);
		return 0;
	}

	int StickerId = 0;
	ptrA pszRetMsg(GetStickerId(szMsg, StickerId));

	ULONG uMsgId = ::InterlockedIncrement(&m_iMsgId);
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/messages.send.json", true,
		bIsChat ? &CVkProto::OnSendChatMsg : &CVkProto::OnSendMessage, AsyncHttpRequest::rpHigh);
	pReq << INT_PARAM(bIsChat ? "chat_id" : "peer_id", iUserId) << INT_PARAM("random_id", ((long)time(0)) * 100 + uMsgId % 100);
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
	VKMessageID_t iMessageId = 0;
	if (reply->resultCode == 200) {
		JSONNode jnRoot;
		const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
		if (jnResponse) {
			debugLogA("CVkProto::OnSendMessage jnResponse %d", jnResponse.as_int());
			switch (jnResponse.type()) {
			case JSON_NUMBER:
				iMessageId = jnResponse.as_int();
				break;
			case JSON_STRING:
				if (swscanf(jnResponse.as_mstring(), L"%u", &iMessageId) != 1)
					iMessageId = 0;
				break;
			case JSON_ARRAY:
				iMessageId = jnResponse.as_array()[json_index_t(0)].as_int();
				break;
			default:
				iMessageId = 0;
			}

			if (iMessageId > ReadQSWord(param->hContact, "lastmsgid"))
				WriteQSWord(param->hContact, "lastmsgid", iMessageId);

			if (m_vkOptions.iMarkMessageReadOn >= MarkMsgReadOn::markOnReply)
				MarkMessagesRead(param->hContact);

			iResult = ACKRESULT_SUCCESS;
		}
	}

	char szMid[40];
	_ltoa(iMessageId, szMid, 10);

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

void CVkProto::OnMarkRead(MCONTACT hContact, MEVENT)
{
	debugLogA("CVkProto::OnDbEventRead");

	if (m_vkOptions.iMarkMessageReadOn == MarkMsgReadOn::markOnRead)
		MarkMessagesRead(hContact);
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

	VKUserID_t iUserId = ReadVKUserID(hContact);
	if (iUserId == VK_INVALID_USER || iUserId == VK_FEED_USER)
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.markAsRead.json", true, &CVkProto::OnReceiveSmth, AsyncHttpRequest::rpLow)
		<< INT_PARAM("mark_conversation_as_read", 1)
		<< INT_PARAM("peer_id", isChatRoom(hContact) ? VK_CHAT_MIN + iUserId : iUserId));
}

void CVkProto::RetrieveMessagesByIds(const CMStringA &szMids)
{
	debugLogA("CVkProto::RetrieveMessagesByIds");
	if (!IsOnline() || szMids.IsEmpty())
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.RetrieveMessagesConversationByIds", true, &CVkProto::OnReceiveMessages, AsyncHttpRequest::rpHigh)
		<< CHAR_PARAM("mids", szMids)
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

	CMStringA szMids;
	int iNumMessages = jnResponse["Msgs"]["count"].as_int();
	const JSONNode &jnMsgs = jnResponse["Msgs"]["items"];
	const JSONNode &jnFUsers = jnResponse["fwd_users"];

	debugLogA("CVkProto::OnReceiveMessages numMessages = %d", iNumMessages);

	if (jnResponse["conv"]) {
		const JSONNode& jnConversation = jnResponse["conv"]["items"];
		for (auto& jnItem : jnConversation) {
			const JSONNode& jnPeer = jnItem["peer"];
			if (!jnPeer)
				break;

			CMStringW wszPeerType(jnPeer["type"].as_mstring());
			VKUserID_t iUserId = jnPeer["id"].as_int();

			MCONTACT hContact = (wszPeerType == L"chat") ? FindChat(iUserId % VK_CHAT_MIN) : FindUser(iUserId, true);
			WriteQSWord(hContact, "in_read", jnItem["in_read"].as_int());
			WriteQSWord(hContact, "out_read", jnItem["out_read"].as_int());
			if (m_vkOptions.iMarkMessageReadOn == MarkMsgReadOn::markOnReceive)
				MarkMessagesRead(hContact);
		}
	}

	for (auto& jnMsg : jnMsgs) {
		if (!jnMsg) {
			debugLogA("CVkProto::OnReceiveMessages pMsg == nullptr");
			break;
		}

		VKMessageID_t iMessageId = jnMsg["id"].as_int();
		CMStringW wszBody(jnMsg["text"].as_mstring());
		time_t tDateTime = jnMsg["date"].as_int();
		int isOut = jnMsg["out"].as_int();
		VKUserID_t iUserId = jnMsg["peer_id"].as_int();

		MCONTACT hContact = 0;

		VKUserID_t iChatId = (GetVKPeerType(iUserId) == VKPeerType::vkPeerMUC) ? iUserId % VK_CHAT_MIN : 0;

		if (iChatId == 0)
			hContact = FindUser(iUserId, true);

		char szMid[40];
		_ltoa(iMessageId, szMid, 10);

		bool bUseServerReadFlag = m_vkOptions.bSyncReadMessageStatusFromServer ? true : !m_vkOptions.bMesAsUnread;

		if (iChatId != 0) {
			debugLogA("CVkProto::OnReceiveMessages chat_id != 0");
			CMStringW wszActionChat = jnMsg["action"]["type"].as_mstring();
			VKMessageID_t iActionMessageId = _wtol(jnMsg["action"]["member_id"].as_mstring());
			if ((wszActionChat == L"chat_kick_user") && (iActionMessageId == m_iMyUserId))
				KickFromChat(iChatId, iUserId, jnMsg, jnFUsers);
			else {
				MCONTACT chatContact = FindChat(iChatId);
				if (chatContact && getBool(chatContact, "kicked", true))
					db_unset(chatContact, m_szModuleName, "kicked");
				AppendChatConversationMessage(iChatId, jnMsg, jnFUsers, false);
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
				if (hContact && (iMessageId > ReadQSWord(hContact, "lastmsgid", -1)))
					WriteQSWord(hContact, "lastmsgid", iMessageId);
				continue;
			}

			if (!wszBody.IsEmpty())
				wszBody += L"\n";
			wszBody += wszAttachmentDescr;
		}

		if (m_vkOptions.bAddMessageLinkToMesWAtt && ((jnAttachments && !jnAttachments.empty()) || (jnFwdMessages && !jnFwdMessages.empty()) || (jnReplyMessages && !jnReplyMessages.empty())))
			wszBody += SetBBCString(TranslateT("Message link"), m_vkOptions.BBCForAttachments(), vkbbcUrl,
				CMStringW(FORMAT, L"https://vk.com/im?sel=%d&msgid=%d", iUserId, iMessageId));

		VKMessageID_t iReadMsg = ReadQSWord(hContact, "in_read", 0);
		int isRead = (iMessageId <= iReadMsg);

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

			CMStringW wszOldMsg;
			if (GetMessageFromDb(iMessageId, tDateTime, wszOldMsg))
				wszBody += SetBBCString(TranslateT("\nOriginal message:\n"), m_vkOptions.BBCForAttachments(), vkbbcB) +
				wszOldMsg;
		}

		PROTORECVEVENT recv = {};

		if (isRead && bUseServerReadFlag)
			recv.flags |= PREF_CREATEREAD;

		if (isOut)
			recv.flags |= PREF_SENT;
		else if (m_vkOptions.bUserForceInvisibleOnActivity && ((time(0) - tDateTime) < (60 * m_vkOptions.iInvisibleInterval)))
			SetInvisible(hContact);

		T2Utf pszBody(wszBody);
		recv.timestamp = bEdited ? tDateTime : (m_vkOptions.bUseLocalTime ? time(0) : tDateTime);
		recv.szMessage = pszBody;

		debugLogA("CVkProto::OnReceiveMessages mid = %d, datetime = %d, isOut = %d, isRead = %d, iUserId = %d, Edited = %d", iMessageId, tDateTime, isOut, isRead, iUserId, (int)bEdited);

		if (!IsMessageExist(iMessageId, vkALL) || bEdited) {
			debugLogA("CVkProto::OnReceiveMessages new or edited message");
			recv.szMsgId = szMid;
			ProtoChainRecvMsg(hContact, &recv);
			if (iMessageId > ReadQSWord(hContact, "lastmsgid", -1))
				WriteQSWord(hContact, "lastmsgid", iMessageId);
		}
		else if (m_vkOptions.bLoadSentAttachments && !wszAttachmentDescr.IsEmpty()) {
			CMStringW wszOldMsg;

			if (GetMessageFromDb(iMessageId, tDateTime, wszOldMsg) && (wszOldMsg == wszBody))
				continue;

			if (wszBodyNoAttachments != wszOldMsg)
				continue;

			debugLogA("CVkProto::OnReceiveMessages add attachments");

			T2Utf pszAttach(wszAttachmentDescr);
			recv.timestamp = isOut ? time(0) : tDateTime;
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

	OBJLIST<VKUserID_t> lufUsers(20, NumericKeySortT);
	const JSONNode &jnUsers = jnResponse["users"];
	if (jnUsers)
		for (auto &it : jnUsers) {
			VKUserID_t iUserId = it["user_id"].as_int();
			int iStatus = it["friend_status"].as_int();

			// iStatus == 3 - user is friend
			// iUserId < 0 - user is group

			if (GetVKPeerType(iUserId) != VKPeerType::vkPeerUser || iStatus != 3 || lufUsers.find((VKUserID_t *) &iUserId))
				continue;

			lufUsers.insert(new VKUserID_t(iUserId));
		}

	const JSONNode &jnGroups = jnResponse["groups"];
	if (jnGroups)
		for (auto &it : jnGroups) {
			VKUserID_t iUserId = it.as_int();
			if (lufUsers.find((VKUserID_t*) &iUserId))
				continue;

			lufUsers.insert(new VKUserID_t(iUserId));
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

		VKUserID_t iUserId = 0;
		MCONTACT hContact(0);
		CMStringW wszPeerType(jnPeer["type"].as_mstring());

		if (wszPeerType == L"user" || wszPeerType == L"group") {
			iUserId = jnPeer["id"].as_int();

			VKUserID_t *pUserID = lufUsers.find((VKUserID_t*) &iUserId);
			debugLogA("CVkProto::OnReceiveDlgs UserId = %d, iIndex = %p, numUnread = %d", iUserId, pUserID, iUnreadCount);

			if (m_vkOptions.bLoadOnlyFriends && iUnreadCount == 0 && !pUserID)
				continue;

			hContact = FindUser(iUserId, true);
			debugLogA("CVkProto::OnReceiveDlgs add UserId = %d", iUserId);

			if (IsGroupUser(hContact))
				szGroupIds.AppendFormat(szGroupIds.IsEmpty() ? "%d" : ",%d", -1 * iUserId);

			WriteQSWord(hContact, "in_read", jnConversation["in_read"].as_int());
			WriteQSWord(hContact, "out_read", jnConversation["out_read"].as_int());

			if (g_bMessageState) {
				bool bIsOut = jnLastMessage["out"].as_bool();
				bool bIsRead = (jnLastMessage["id"].as_int() <= jnConversation["in_read"].as_int());

				if (bIsRead && bIsOut)
					CallService(MS_MESSAGESTATE_UPDATE, hContact, MRD_TYPE_DELIVERED);
			}
		}

		if (wszPeerType == L"chat") {
			VKUserID_t iChatId = jnPeer["local_id"].as_int();
			debugLogA("CVkProto::OnReceiveDlgs chatid = %d", iChatId);
			if (m_chats.find((CVkChatInfo*)&iChatId) == nullptr)
				AppendConversationChat(iChatId, it);
		}
		else if (m_vkOptions.iSyncHistoryMetod) {
			VKMessageID_t iMessageId = jnLastMessage["id"].as_int();
			m_bNotifyForEndLoadingHistory = false;

			if (ReadQSWord(hContact, "lastmsgid", -1) == -1 && iUnreadCount && !getBool(hContact, "ActiveHistoryTask")) {
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