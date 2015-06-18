/*
Copyright (c) 2013-15 Miranda NG project (http://miranda-ng.org)

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

int CVkProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre)
{
	debugLogA("CVkProto::RecvMsg");
	Proto_RecvMessage(hContact, pre);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

void CVkProto::SendMsgAck(void *param)
{
	debugLogA("CVkProto::SendMsgAck");
	TFakeAckParams *ack = (TFakeAckParams*)param;
	Sleep(100);
	ProtoBroadcastAck(ack->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)ack->msgid, 0);
	delete ack;
}

int CVkProto::SendMsg(MCONTACT hContact, int, const char *szMsg)
{
	debugLogA("CVkProto::SendMsg");
	if (!IsOnline())
		return 0;
	LONG userID = getDword(hContact, "ID", -1);
	if (userID == -1 || userID == VK_FEED_USER) {
		ForkThread(&CVkProto::SendMsgAck, new TFakeAckParams(hContact, 0));
		return 0;
	}

	int StickerId = 0;
	ptrA retMsg(GetStickerId(szMsg, StickerId));

	ULONG msgId = ::InterlockedIncrement(&m_msgId);
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_POST, "/method/messages.send.json", true, &CVkProto::OnSendMessage, AsyncHttpRequest::rpHigh)
		<< INT_PARAM("user_id", userID)
		<< INT_PARAM("guid", ((LONG) time(NULL)) * 100 + msgId % 100)
		<< VER_API;

	if (StickerId != 0)
		pReq << INT_PARAM("sticker_id", StickerId);
	else
		pReq << CHAR_PARAM("message", szMsg);

	pReq->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	pReq->pUserInfo = new CVkSendMsgParam(hContact, msgId);
	Push(pReq);

	if (!m_bServerDelivery)
		ForkThread(&CVkProto::SendMsgAck, new TFakeAckParams(hContact, msgId));

	if (retMsg) {
		Sleep(330);
		SendMsg(hContact, 0, retMsg);
	}
	return msgId;
}

void CVkProto::OnSendMessage(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	int iResult = ACKRESULT_FAILED;
	if (pReq->pUserInfo == NULL) {
		debugLogA("CVkProto::OnSendMessage failed! (pUserInfo == NULL)");
		return;
	}
	CVkSendMsgParam *param = (CVkSendMsgParam*)pReq->pUserInfo;

	debugLogA("CVkProto::OnSendMessage %d", reply->resultCode);
	if (reply->resultCode == 200) {
		JSONNode jnRoot;
		const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
		if (!jnResponse.isnull()) {
			UINT mid;
			if (jnResponse.type() != JSON_STRING) 
				mid = jnResponse.as_int();
			else if (_stscanf(jnResponse.as_mstring(), _T("%d"), &mid) != 1)
				mid = 0;				

			if (param->iMsgID != -1)
				m_sendIds.insert((HANDLE)mid);
			if (mid > getDword(param->hContact, "lastmsgid", 0))
				setDword(param->hContact, "lastmsgid", mid);
			if (m_iMarkMessageReadOn >= markOnReply)
				MarkMessagesRead(param->hContact);
			iResult = ACKRESULT_SUCCESS;
		}
	}

	if (param->iMsgID == -1) {
		CVkFileUploadParam *fup = (CVkFileUploadParam *)param->iCount;
		ProtoBroadcastAck(fup->hContact, ACKTYPE_FILE, iResult, (HANDLE)fup, 0);
		if (!pReq->bNeedsRestart)
			delete fup;
		return;
	}
	else if (m_bServerDelivery)
		ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, iResult, HANDLE(param->iMsgID), 0);
	if (!pReq->bNeedsRestart) {
		delete param;
		pReq->pUserInfo = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

int CVkProto::OnDbEventRead(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::OnDbEventRead");
	if (m_iMarkMessageReadOn == markOnRead)
		MarkMessagesRead(hContact);
	return 0;
}

void CVkProto::MarkMessagesRead(const CMStringA &mids)
{
	debugLogA("CVkProto::MarkMessagesRead (mids)");
	if (!IsOnline())
		return;
	if (mids.IsEmpty())
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.markAsRead.json", true, &CVkProto::OnReceiveSmth, AsyncHttpRequest::rpLow)
		<< CHAR_PARAM("message_ids", mids)
		<< VER_API);
}

void CVkProto::MarkMessagesRead(const MCONTACT hContact)
{
	debugLogA("CVkProto::MarkMessagesRead (hContact)");
	if (!IsOnline())
		return;
	LONG userID = getDword(hContact, "ID", -1);
	if (userID == -1 || userID == VK_FEED_USER)
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.markAsRead.json", true, &CVkProto::OnReceiveSmth, AsyncHttpRequest::rpLow)
		<< INT_PARAM("peer_id", userID)
		<< VER_API);
}

void CVkProto::RetrieveMessagesByIds(const CMStringA &mids)
{
	debugLogA("CVkProto::RetrieveMessagesByIds");
	if (!IsOnline())
		return;
	if (mids.IsEmpty())
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.getById.json", true, &CVkProto::OnReceiveMessages, AsyncHttpRequest::rpHigh)
		<< CHAR_PARAM("message_ids", mids)
		<< VER_API);
}

void CVkProto::RetrieveUnreadMessages()
{
	debugLogA("CVkProto::RetrieveUnreadMessages");
	if (!IsOnline())
		return;
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.getDialogs.json", true, &CVkProto::OnReceiveDlgs)
		<< VER_API);
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

	CMStringA mids;
	int numMessages = jnResponse["count"].as_int();
	const JSONNode &jnMsgs = jnResponse["items"];

	debugLogA("CVkProto::OnReceiveMessages numMessages = %d", numMessages);

	for (auto it = jnMsgs.begin(); it != jnMsgs.end(); ++it) {
		const JSONNode &jnMsg = (*it);
		if (!jnMsg) {
			debugLogA("CVkProto::OnReceiveMessages pMsg == NULL");
			break;
		}

		UINT mid = jnMsg["id"].as_int();
		CMString tszBody(jnMsg["body"].as_mstring());
		int datetime = jnMsg["date"].as_int();
		int isOut = jnMsg["out"].as_int();
		int isRead = jnMsg["read_state"].as_int();
		int uid = jnMsg["user_id"].as_int();

		const JSONNode &jnFwdMessages = jnMsg["fwd_messages"];
		if (!jnFwdMessages.isnull()){
			CMString tszFwdMessages = GetFwdMessages(jnFwdMessages, m_iBBCForAttachments);
			if (!tszBody.IsEmpty())
				tszFwdMessages = _T("\n") + tszFwdMessages;
			tszBody +=  tszFwdMessages;
		}

		const JSONNode &jnAttachments = jnMsg["attachments"];
		if (!jnAttachments.isnull()){
			CMString tszAttachmentDescr = GetAttachmentDescr(jnAttachments, m_iBBCForAttachments);
			if (!tszBody.IsEmpty())
				tszAttachmentDescr = _T("\n") + tszAttachmentDescr;
			tszBody += tszAttachmentDescr;
		}

		MCONTACT hContact = NULL;
		int chat_id = jnMsg["chat_id"].as_int();
		if (chat_id == 0)
			hContact = FindUser(uid, true);

		char szMid[40];
		_itoa(mid, szMid, 10);
		if (m_iMarkMessageReadOn == markOnReceive || chat_id != 0) {
			if (!mids.IsEmpty())
				mids.AppendChar(',');
			mids.Append(szMid);
		}

		if (chat_id != 0) {
			debugLogA("CVkProto::OnReceiveMessages chat_id != 0");
			CMString action_chat = jnMsg["action"].as_mstring();
			int action_mid = _ttoi(jnMsg["action_mid"].as_mstring());
			if ((action_chat == "chat_kick_user") && (action_mid == m_myUserId))
				KickFromChat(chat_id, uid, jnMsg);
			else
				AppendChatMessage(chat_id, jnMsg, false);
			continue;
		}

		PROTORECVEVENT recv = { 0 };
		if (isRead && !m_bMesAsUnread)
			recv.flags |= PREF_CREATEREAD;
		if (isOut)
			recv.flags |= PREF_SENT;
		else if (m_bUserForceOnlineOnActivity)
			SetInvisible(hContact);

		T2Utf pszBody(tszBody);
		recv.timestamp = m_bUseLocalTime ? time(NULL) : datetime;
		recv.szMessage = pszBody;
		recv.lParam = isOut;
		recv.pCustomData = szMid;
		recv.cbCustomDataSize = (int)mir_strlen(szMid);
		Sleep(100);

		debugLogA("CVkProto::OnReceiveMessages mid = %d, datetime = %d, isOut = %d, isRead = %d, uid = %d", mid, datetime, isOut, isRead, uid);

		if (!CheckMid(m_sendIds, mid)) {
			debugLogA("CVkProto::OnReceiveMessages ProtoChainRecvMsg");
			ProtoChainRecvMsg(hContact, &recv);
			if (mid > getDword(hContact, "lastmsgid", -1))
				setDword(hContact, "lastmsgid", mid);
			if (!isOut)
				m_incIds.insert((HANDLE)mid);
		}
	}

	if (!mids.IsEmpty())
		MarkMessagesRead(mids);
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
		
	const JSONNode &jnDlgs = jnResponse["items"];
	if (!jnDlgs)
		return;

	for (auto it = jnDlgs.begin(); it != jnDlgs.end(); ++it) {
		if (!(*it))
			break;
		int numUnread = (*it)["unread"].as_int();
		const JSONNode &jnDlg = (*it)["message"];
		if (jnDlg == NULL)
			break;

		int chatid = jnDlg["chat_id"].as_int();
		if (chatid != 0) {
			debugLogA("CVkProto::OnReceiveDlgs chatid = %d", chatid);
			if (m_chats.find((CVkChatInfo*)&chatid) == NULL)
				AppendChat(chatid, jnDlg);
		}
		else if (m_iSyncHistoryMetod) {
			int mid = jnDlg["id"].as_int();
			int uid = jnDlg["user_id"].as_int();
			MCONTACT hContact = FindUser(uid, true);

			if (getDword(hContact, "lastmsgid", -1) == -1 && numUnread)
				GetServerHistory(hContact, 0, numUnread, 0, 0, true);
			else
				GetHistoryDlg(hContact, mid);

			if (m_iMarkMessageReadOn == markOnReceive && numUnread)
				MarkMessagesRead(hContact);
		}
		else if (numUnread) {
			int uid = jnDlg["user_id"].as_int();
			MCONTACT hContact = FindUser(uid, true);
			GetServerHistory(hContact, 0, numUnread, 0, 0, true);

			if (m_iMarkMessageReadOn == markOnReceive)
				MarkMessagesRead(hContact);
		}
	}
	RetrieveUsersInfo();
}


