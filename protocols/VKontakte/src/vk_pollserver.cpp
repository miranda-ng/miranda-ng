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

void CVkProto::RetrievePollingInfo()
{
	debugLogA("CVkProto::RetrievePollingInfo");
	if (!IsOnline())
		return;
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.getLongPollServer.json", true, &CVkProto::OnReceivePollingInfo, AsyncHttpRequest::rpHigh)
		<< INT_PARAM("use_ssl", 1)
		<< INT_PARAM("lp_version", 3)
	);
}

void CVkProto::OnReceivePollingInfo(MHttpResponse *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceivePollingInfo %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONNode jnRoot;
	JSONNode jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse) {
		if (!pReq->bNeedsRestart) {
			debugLogA("CVkProto::OnReceivePollingInfo PollingThread not start (getLongPollServer error)");
			ClosePollingConnection();
			ShutdownSession();
		}
		return;
	}

	char ts[32];
	itoa(jnResponse["ts"].as_int(), ts, 10);

	m_szPollingTs = mir_strdup(ts);
	m_szPollingKey = mir_u2a(jnResponse["key"].as_mstring());
	m_szPollingServer = mir_u2a(jnResponse["server"].as_mstring());

	if (!m_hPollingThread) {
		debugLogA("CVkProto::OnReceivePollingInfo m_hPollingThread is nullptr");
		debugLogA("CVkProto::OnReceivePollingInfo m_pollingTs = '%s' m_pollingKey = '%s' m_pollingServer = '%s'",
			m_szPollingTs ? m_szPollingTs.get() : "<nullptr>",
			m_szPollingKey ? m_szPollingKey.get() : "<nullptr>",
			m_szPollingServer ? m_szPollingServer.get() : "<nullptr>");

		if (m_szPollingTs != nullptr && m_szPollingKey != nullptr && m_szPollingServer != nullptr) {
			debugLogA("CVkProto::OnReceivePollingInfo PollingThread starting...");
			m_hPollingThread = ForkThreadEx(&CVkProto::PollingThread, nullptr, nullptr);
		}
		else {
			debugLogA("CVkProto::OnReceivePollingInfo PollingThread not start");
			ClosePollingConnection();
			ShutdownSession();
			return;
		}
	}
	else
		debugLogA("CVkProto::OnReceivePollingInfo m_hPollingThread is not nullptr");
}

void CVkProto::PollUpdates(const JSONNode &jnUpdates)
{
	debugLogA("CVkProto::PollUpdates");
	CMStringA szMids;
	
	VKMessageID_t iMessageId;
	int iFlags, iPlatform;
	VKUserID_t iUserId;
	MCONTACT hContact;
	time_t tDateTime = 0;
	CMStringW wszMsg;

	for (auto &it : jnUpdates) {
		const JSONNode &jnChild = it.as_array();
		switch (jnChild[json_index_t(0)].as_int()) {
		case VKPOLL_MSG_DELFLAGS:
			if (jnChild.size() < 4)
				break;
			iMessageId = jnChild[1].as_int();
			iFlags = jnChild[2].as_int();
			iUserId = jnChild[3].as_int();
			hContact = FindUser(iUserId);

			if (hContact != 0 && (iFlags & VKFLAG_MSGDELETED)) {
				if (!szMids.IsEmpty())
					szMids.AppendChar(',');
				szMids.AppendFormat("%d", iMessageId);
			}

			if (hContact != 0 && (iFlags & VKFLAG_MSGUNREAD) && !IsMessageExist(iMessageId, vkIN)) {
				setDword(hContact, "LastMsgReadTime", time(0));
				if (g_bMessageState)
					CallService(MS_MESSAGESTATE_UPDATE, hContact, MRD_TYPE_READ);
				else
					SetSrmmReadStatus(hContact);

				if (m_vkOptions.bUserForceInvisibleOnActivity)
					SetInvisible(hContact);
				if (m_vkOptions.bSyncReadMessageStatusFromServer)
					MarkDialogAsRead(hContact);
			}
			break;

		case VKPOLL_MSG_ADDFLAGS:
			if (jnChild.size() < 4)
				break;
			iMessageId = jnChild[1].as_int();
			iFlags = jnChild[2].as_int();
			iUserId = jnChild[3].as_int();
			hContact = FindUser(iUserId);

			if (hContact != 0 && (iFlags & VKFLAG_MSGDELETED) && IsMessageExist(iMessageId, vkALL) && GetMessageFromDb(iMessageId, tDateTime, wszMsg)) {
				wchar_t ttime[64];
				time_t tDeleteTime = time(0);
				_locale_t locale = _create_locale(LC_ALL, "");
				_wcsftime_l(ttime, _countof(ttime), TranslateT("%x at %X"), localtime(&tDeleteTime), locale);
				_free_locale(locale);

				wszMsg = SetBBCString(
					CMStringW(FORMAT, TranslateT("This message has been deleted by sender in %s:\n"), ttime),
						m_vkOptions.BBCForAttachments(), vkbbcB) +
					wszMsg;

				DB::EventInfo dbei;
				if (iUserId == m_iMyUserId)
					dbei.flags |= DBEF_SENT;
				else if (m_vkOptions.bUserForceInvisibleOnActivity && time(0) - tDateTime < 60 * m_vkOptions.iInvisibleInterval)
					SetInvisible(hContact);

				char szMid[40];
				_itoa(iMessageId, szMid, 10);

				T2Utf pszMsg(wszMsg);
				dbei.timestamp = tDateTime;
				dbei.pBlob = pszMsg;
				dbei.szId = szMid;
				ProtoChainRecvMsg(hContact, dbei);
			}
			break;

		case VKPOLL_MSG_EDITED:
			iMessageId = jnChild[1].as_int();
			if (!szMids.IsEmpty())
				szMids.AppendChar(',');
			szMids.AppendFormat("%d", iMessageId);
			break;

		case VKPOLL_MSG_ADDED: // new message
			iMessageId = jnChild[1].as_int();
				// skip outgoing messages sent from a client
			iFlags = jnChild[2].as_int();
			if (iFlags & VKFLAG_MSGOUTBOX && !(iFlags & VKFLAG_MSGCHAT) && !m_vkOptions.bSendVKLinksAsAttachments && IsMessageExist(iMessageId, vkOUT))
				break;

			if (!szMids.IsEmpty())
				szMids.AppendChar(',');
			szMids.AppendFormat("%d", iMessageId);
			break;

		case VKPOLL_READ_ALL_OUT:
			iUserId = jnChild[1].as_int();
			hContact = FindUser(iUserId);
			if (hContact != 0) {
				setDword(hContact, "LastMsgReadTime", time(0));
				if (g_bMessageState)
					CallService(MS_MESSAGESTATE_UPDATE, hContact, MRD_TYPE_READ);
				else
					SetSrmmReadStatus(hContact);

				if (m_vkOptions.bUserForceInvisibleOnActivity)
					SetInvisible(hContact);
			}
			break;
		case VKPOLL_READ_ALL_IN:
			iUserId = jnChild[1].as_int();
			hContact = FindUser(iUserId);
			if (hContact != 0 && m_vkOptions.bSyncReadMessageStatusFromServer)
				MarkDialogAsRead(hContact);
			break;

		case VKPOLL_USR_ONLINE:
			iUserId = -jnChild[1].as_int();
			if ((hContact = FindUser(iUserId)) != 0) {
				setWord(hContact, "Status", ID_STATUS_ONLINE);
				iPlatform = jnChild[2].as_int();
				SetMirVer(hContact, iPlatform);
			}
			break;

		case VKPOLL_USR_OFFLINE:
			iUserId = -jnChild[1].as_int();
			if ((hContact = FindUser(iUserId)) != 0) {
				setWord(hContact, "Status", ID_STATUS_OFFLINE);
				db_unset(hContact, m_szModuleName, "ListeningTo");
				SetMirVer(hContact, -1);
			}
			break;

		case VKPOLL_USR_UTN:
			iUserId = jnChild[1].as_int();
			hContact = FindUser(iUserId);
			if (hContact != 0) {
				ForkThread(&CVkProto::ContactTypingThread, (void *)hContact);
				if (m_vkOptions.bUserForceInvisibleOnActivity)
					SetInvisible(hContact);
			}
			break;

		case VKPOLL_CHAT_UTN:
			ForkThread(&CVkProto::ChatContactTypingThread, new CVKChatContactTypingParam(jnChild[2].as_int(), jnChild[1].as_int()));
			break;

		case VKPOLL_CHAT_CHANGED:
			VKUserID_t iChatId = jnChild[1].as_int();
			CVkChatInfo *cc = m_chats.find((CVkChatInfo*)&iChatId);
			if (cc)
				RetrieveChatInfo(cc);
			break;
		}
	}

	RetrieveMessagesByIds(szMids);
}

int CVkProto::PollServer()
{
	debugLogA("CVkProto::PollServer");
	if (!IsOnline()) {
		debugLogA("CVkProto::PollServer is dead (not online)");
		ClosePollingConnection();
		ShutdownSession();
		return 0;
	}

	debugLogA("CVkProto::PollServer (online)");
	int iPollConnRetry = MAX_RETRIES;

	CMStringA szReqUrl(FORMAT, "https://%s?act=a_check&key=%s&ts=%s&wait=25&access_token=%s&mode=%d&version=%d", m_szPollingServer, m_szPollingKey, m_szPollingTs, m_szAccessToken, 106, 2);
	// see mode parametr description on https://vk.com/dev/using_longpoll (Russian version)
	MHttpRequest req(REQUEST_GET);
	req.m_szUrl = szReqUrl.GetBuffer();
	req.flags = VK_NODUMPHEADERS | NLHRF_PERSISTENT | NLHRF_HTTP11 | NLHRF_SSL;
	req.timeout = 30000;
	req.nlc = m_hPollingConn;
	time_t tLocalPoolThreadTimer;
	{
		mir_cslock lck(m_csPoolThreadTimer);
		tLocalPoolThreadTimer = m_tPoolThreadTimer = time(0);
	}

	NLHR_PTR reply(0);
	while ((reply = Netlib_HttpTransaction(m_hNetlibUser, &req)) == nullptr) {
		{
			mir_cslock lck(m_csPoolThreadTimer);
			if (m_tPoolThreadTimer != tLocalPoolThreadTimer) {
				debugLogA("CVkProto::PollServer is living dead => return");
				return -2;
			}
		}
		debugLogA("CVkProto::PollServer is dead");
		ClosePollingConnection();
		if (iPollConnRetry && !m_bTerminated) {
			iPollConnRetry--;
			debugLogA("CVkProto::PollServer restarting %d", MAX_RETRIES - iPollConnRetry);
			Sleep(1000);
		}
		else {
			debugLogA("CVkProto::PollServer => ShutdownSession");
			ShutdownSession();
			return 0;
		}
	}

	int retVal = 0;
	if (reply->resultCode == 200) {
		JSONNode jnRoot = JSONNode::parse(reply->body);
		const JSONNode &jnFailed = jnRoot["failed"];
		if (jnFailed && jnFailed.as_int() > 1) {
			RetrievePollingInfo();
			retVal = -1;
			debugLogA("Polling key expired, restarting polling thread");
		}
		else if (CheckJsonResult(nullptr, jnRoot)) {
			char ts[32];
			itoa(jnRoot["ts"].as_int(), ts, 10);
			m_szPollingTs = mir_strdup(ts);
			const JSONNode &jnUpdates = jnRoot["updates"];
			if (jnUpdates)
				PollUpdates(jnUpdates);
			retVal = 1;
		}
	}
	else if ((reply->resultCode >= 400 && reply->resultCode <= 417)
		|| (reply->resultCode >= 500 && reply->resultCode <= 509)) {
		debugLogA("CVkProto::PollServer is dead. Error code - %d", reply->resultCode);
		ClosePollingConnection();
		ShutdownSession();
		return 0;
	}

	m_hPollingConn = reply->nlc;

	debugLogA("CVkProto::PollServer return %d", retVal);
	return retVal;
}

void CVkProto::PollingThread(void*)
{
	debugLogA("CVkProto::PollingThread: entering");

	while (!m_bTerminated) {
		int iRetVal = PollServer();
		if (iRetVal == -2)
			return;

		if (iRetVal == -1 || !m_hPollingThread)
			break;
	}

	ClosePollingConnection();
	debugLogA("CVkProto::PollingThread: leaving");

	if (m_hPollingThread) {
		CloseHandle(m_hPollingThread);
		m_hPollingThread = nullptr;
	}
}