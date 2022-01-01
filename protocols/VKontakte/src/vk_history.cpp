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

//////////////////////////// History services ///////////////////////////////////////////

INT_PTR __cdecl CVkProto::SvcGetAllServerHistoryForContact(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcGetAllServerHistoryForContact");
	if (!IsOnline())
		return 0;

	if (getBool(hContact, "ActiveHistoryTask"))
		return 0;

	LPCWSTR str = TranslateT("Are you sure to reload all messages from vk.com?\nLocal contact history will be deleted and reloaded from the server.\nIt may take a long time.\nDo you want to continue?");
	if (IDNO == MessageBoxW(nullptr, str, TranslateT("Attention!"), MB_ICONWARNING | MB_YESNO))
		return 0;

	LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
	if (userID == VK_INVALID_USER || userID == VK_FEED_USER)
		return 0;

	setByte(hContact, "ActiveHistoryTask", 1);

	DB::ECPTR pCursor(DB::Events(hContact));
	while (pCursor.FetchNext())
		pCursor.DeleteEvent();

	m_bNotifyForEndLoadingHistory = true;

	db_unset(hContact, m_szModuleName, "lastmsgid");
	GetServerHistory(hContact, 0, MAXHISTORYMIDSPERONE, 0, 0);
	return 1;
}

INT_PTR __cdecl CVkProto::SvcGetAllServerHistory(WPARAM, LPARAM)
{
	debugLogA("CVkProto::SvcGetAllServerHistory start");
	if (!IsOnline())
		return 0;
	LPCWSTR str = TranslateT("Are you sure you want to reload all messages for all contacts from vk.com?\nLocal contact history will be deleted and reloaded from the server.\nIt may take a very long time and/or corrupt Miranda database.\nDo you want to continue?");
	if (IDNO == MessageBoxW(nullptr, str, TranslateT("Attention!"), MB_ICONWARNING | MB_YESNO))
		return 0;

	for (auto &hContact : AccContacts()) {
		LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
		if (userID == VK_INVALID_USER || userID == VK_FEED_USER)
			continue;

		if (getBool(hContact, "ActiveHistoryTask"))
			break;
		setByte(hContact, "ActiveHistoryTask", 1);

		DB::ECPTR pCursor(DB::Events(hContact));
		while (pCursor.FetchNext())
			pCursor.DeleteEvent();

		{
			mir_cslock lck(m_csLoadHistoryTask);
			m_iLoadHistoryTask++;
			m_bNotifyForEndLoadingHistoryAllContact = m_bNotifyForEndLoadingHistory = true;
			debugLogA("CVkProto::SvcGetAllServerHistory for ID=%d m_iLoadHistoryTask=%d", userID, m_iLoadHistoryTask);
		}

		db_unset(hContact, m_szModuleName, "lastmsgid");
		GetServerHistory(hContact, 0, MAXHISTORYMIDSPERONE, 0, 0);
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::GetServerHistoryLastNDay(MCONTACT hContact, int NDay)
{
	debugLogA("CVkProto::SvcGetServerHistoryLastNDay %d", NDay);

	if (getBool(hContact, "ActiveHistoryTask"))
		return;
	setByte(hContact, "ActiveHistoryTask", 1);

	time_t tTime = time(0) - 60 * 60 * 24 * NDay;

	if (NDay > 3) {
		DB::ECPTR pCursor(DB::Events(hContact));
		while (MEVENT hDbEvent = pCursor.FetchNext()) {
			DBEVENTINFO dbei = {};
			db_event_get(hDbEvent, &dbei);
			if (dbei.timestamp > tTime && dbei.eventType != VK_USER_DEACTIVATE_ACTION)
				pCursor.DeleteEvent();
		}

		{
			mir_cslock lck(m_csLoadHistoryTask);
			m_iLoadHistoryTask++;
			m_bNotifyForEndLoadingHistory = true;
		}

	}

	setDword(hContact, "oldlastmsgid", getDword(hContact, "lastmsgid", -1));
	db_unset(hContact, m_szModuleName, "lastmsgid");
	GetServerHistory(hContact, 0, MAXHISTORYMIDSPERONE, tTime, 0);
}

void CVkProto::GetServerHistory(MCONTACT hContact, int iOffset, int iCount, int iTime, int iLastMsgId, bool once)
{
	debugLogA("CVkProto::GetServerHistory");
	if (!IsOnline() || iCount == 0)
		return;

	LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
	debugLogA("CVkProto::GetServerHistory %ld %d %d %d %d %d", userID, iOffset, iCount, iTime, iLastMsgId, (int)once);
	if (VK_INVALID_USER == userID || userID == VK_FEED_USER)
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.GetServerConversationHistory", true, &CVkProto::OnReceiveHistoryMessages, AsyncHttpRequest::rpLow)
		<< INT_PARAM("reqcount", iCount)
		<< INT_PARAM("offset", iOffset)
		<< INT_PARAM("userid", userID)
		<< INT_PARAM("time", iTime)
		<< INT_PARAM("lastmid", iLastMsgId)
		<< INT_PARAM("once", (int)once)
	)->pUserInfo = new CVkSendMsgParam(hContact, iLastMsgId, iOffset);
}

void CVkProto::GetHistoryDlg(MCONTACT hContact, int iLastMsg)
{
	debugLogA("CVkProto::GetHistoryDlg %d", iLastMsg);
	int lastmsgid = -1;
	switch (m_vkOptions.iSyncHistoryMetod) {
	case syncAuto:
		lastmsgid = getDword(hContact, "lastmsgid", -1);
		if (lastmsgid == -1 || !IsOnline()) {
			setDword(hContact, "lastmsgid", iLastMsg);
			return;
		}
		m_bNotifyForEndLoadingHistory = false;
		if (getBool(hContact, "ActiveHistoryTask"))
			return;
		setByte(hContact, "ActiveHistoryTask", 1);
		GetServerHistory(hContact, 0, MAXHISTORYMIDSPERONE, 0, lastmsgid);
		break;
	case sync1Days:
		GetServerHistoryLastNDay(hContact, 1);
		break;
	case sync3Days:
		GetServerHistoryLastNDay(hContact, 3);
		break;
	}
}

void CVkProto::OnReceiveHistoryMessages(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveHistoryMessages %d", reply->resultCode);
	if (reply->resultCode != 200 || !pReq->pUserInfo) {
		mir_cslock lck(m_csLoadHistoryTask);
		if (m_iLoadHistoryTask > 0)
			m_iLoadHistoryTask--;
		debugLogW(L"CVkProto::OnReceiveHistoryMessages error m_iLoadHistoryTask=%d", m_iLoadHistoryTask);
		MsgPopup(TranslateT("Error loading message history from server."), TranslateT("Error"), true);

		if (m_iLoadHistoryTask == 0 && m_bNotifyForEndLoadingHistoryAllContact) {
			MsgPopup(TranslateT("Loading messages for all contacts is completed."), TranslateT("Loading history"));
			m_bNotifyForEndLoadingHistoryAllContact = m_bNotifyForEndLoadingHistory = false;
		}
		if (pReq->pUserInfo)
			setByte(((CVkSendMsgParam*)pReq->pUserInfo)->hContact, "ActiveHistoryTask", 0);
		return;
	}

	JSONNode jnRoot;
	CVkSendMsgParam *param = (CVkSendMsgParam*)pReq->pUserInfo;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse) {
		setByte(param->hContact, "ActiveHistoryTask", 0);
		if (!pReq->bNeedsRestart || m_bTerminated) {
			mir_cslock lck(m_csLoadHistoryTask);
			if (m_iLoadHistoryTask > 0)
				m_iLoadHistoryTask--;

			ptrW pwszNick(db_get_wsa(param->hContact, m_szModuleName, "Nick"));
			CMStringW str(FORMAT, TranslateT("Error loading message history for %s from server."), pwszNick);

			MsgPopup(param->hContact, str, TranslateT("Error"), true);
			debugLogW(L"CVkProto::OnReceiveHistoryMessages error for %s m_iLoadHistoryTask=%d", pwszNick.get(), m_iLoadHistoryTask);

			if (m_iLoadHistoryTask == 0 && m_bNotifyForEndLoadingHistoryAllContact) {
				MsgPopup(TranslateT("Loading messages for all contacts is completed."), TranslateT("Loading history"));
				m_bNotifyForEndLoadingHistoryAllContact = m_bNotifyForEndLoadingHistory = false;
			}

			delete param;
			pReq->pUserInfo = nullptr;
		}
		return;
	}

	int iTime = jnResponse["datetime"].as_int();
	const JSONNode &jnMsgs = jnResponse["items"];
	const JSONNode &jnFUsers = jnResponse["fwd_users"];

	int iLastMsgId = getDword(param->hContact, "lastmsgid", -1);
	time_t tLastReadMessageTime = 0;
	int count = 0;

	for (auto it = jnMsgs.rbegin(); it != jnMsgs.rend(); ++it) {
		const JSONNode &jnMsg = (*it);
		int mid = jnMsg["id"].as_int();
		if (iLastMsgId < mid)
			iLastMsgId = mid;

		char szMid[40];
		_itoa(mid, szMid, 10);

		CMStringW wszBody(jnMsg["text"].as_mstring());
		int uid = jnMsg["peer_id"].as_int();
		int iReadMsg = getDword(param->hContact, "in_read", 0);
		int isRead = (mid <= iReadMsg);
		int datetime = jnMsg["date"].as_int();
		int isOut = jnMsg["out"].as_int();
		MCONTACT hContact = FindUser(uid, true);

		const JSONNode &jnFwdMessages = jnMsg["fwd_messages"];
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

		const JSONNode &jnAttachments = jnMsg["attachments"];
		if (jnAttachments && !jnAttachments.empty()) {
			CMStringW wszAttachmentDescr = GetAttachmentDescr(jnAttachments, m_vkOptions.BBCForAttachments());

			if (wszAttachmentDescr == L"== FilterAudioMessages ==") {
				count++;
				continue;
			}

			if (!wszBody.IsEmpty())
				wszAttachmentDescr = L"\n" + wszAttachmentDescr;
			wszBody += wszAttachmentDescr;
		}

		if (m_vkOptions.bAddMessageLinkToMesWAtt && ((jnAttachments && !jnAttachments.empty()) || (jnFwdMessages && !jnFwdMessages.empty()) || (jnReplyMessages && !jnReplyMessages.empty())))
			wszBody += SetBBCString(TranslateT("Message link"), m_vkOptions.BBCForAttachments(), vkbbcUrl,
				CMStringW(FORMAT, L"https://vk.com/im?sel=%d&msgid=%d", uid, mid));

		PROTORECVEVENT recv = { 0 };
		if (isRead)
			recv.flags |= PREF_CREATEREAD;
		if (isOut)
			recv.flags |= PREF_SENT;

		time_t update_time = (time_t)jnMsg["update_time"].as_int();
		if (update_time) {
			CMStringW wszEditTime;

			wchar_t ttime[64];
			_locale_t locale = _create_locale(LC_ALL, "");
			_wcsftime_l(ttime, _countof(ttime), TranslateT("%x at %X"), localtime(&update_time), locale);
			_free_locale(locale);

			wszEditTime.Format(TranslateT("Edited message (updated %s):\n"), ttime);

			wszBody = SetBBCString(
				wszEditTime,
				m_vkOptions.BBCForAttachments(), vkbbcB) +
				wszBody;
		}

		T2Utf pszBody(wszBody);

		recv.timestamp = datetime;
		recv.szMessage = pszBody;
		recv.szMsgId = szMid;
		ProtoChainRecvMsg(hContact, &recv);

		MEVENT hDbEvent = db_event_getById(m_szModuleName, strcat(szMid, "_"));
		if (hDbEvent)
			db_event_delete(hDbEvent);

		if (isRead && isOut && datetime > tLastReadMessageTime)
			tLastReadMessageTime = datetime;

		count++;
	}

	setDword(param->hContact, "lastmsgid", iLastMsgId);

	if (g_bMessageState)
		CallService(MS_MESSAGESTATE_UPDATE, param->hContact, MRD_TYPE_DELIVERED);

	int once = jnResponse["once"].as_int();
	int iRCount = jnResponse["rcount"].as_int();
	if (count == iRCount && once == 0 && iRCount > 0)
		GetServerHistory(param->hContact, param->iCount + count, iRCount, iTime, param->iMsgID);
	else {
		mir_cslock lck(m_csLoadHistoryTask);
		if (m_iLoadHistoryTask > 0)
			m_iLoadHistoryTask--;

		setDword(param->hContact, "lastmsgid", iLastMsgId == -1 ? getDword(param->hContact, "oldlastmsgid", -1) : iLastMsgId);
		db_unset(param->hContact, m_szModuleName, "oldlastmsgid");

		ptrW pwszNick(db_get_wsa(param->hContact, m_szModuleName, "Nick"));
		CMStringW str(FORMAT, TranslateT("Loading messages for %s is completed."), pwszNick);
		debugLogW(L"CVkProto::OnReceiveHistoryMessages for %s m_iLoadHistoryTask=%d", pwszNick.get(), m_iLoadHistoryTask);

		if (m_bNotifyForEndLoadingHistory)
			MsgPopup(param->hContact, str, TranslateT("Loading history"));

		if (m_iLoadHistoryTask == 0 && m_bNotifyForEndLoadingHistoryAllContact) {
			MsgPopup(TranslateT("Loading messages for all contacts is completed."), TranslateT("Loading history"));
			m_bNotifyForEndLoadingHistoryAllContact = m_bNotifyForEndLoadingHistory = false;
		}
		setByte(param->hContact, "ActiveHistoryTask", 0);
	}

	if (!pReq->bNeedsRestart || m_bTerminated) {
		delete param;
		pReq->pUserInfo = nullptr;
	}
}