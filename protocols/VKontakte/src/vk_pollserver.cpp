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

void CVkProto::RetrievePollingInfo()
{
	debugLogA("CVkProto::RetrievePollingInfo");
	if (!IsOnline())
		return;
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/messages.getLongPollServer.json", true, &CVkProto::OnReceivePollingInfo, AsyncHttpRequest::rpHigh)
		<< INT_PARAM("use_ssl", 1)
		<< VER_API);
}

void CVkProto::OnReceivePollingInfo(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceivePollingInfo %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	if (pResponse == NULL)
		return;

	m_pollingTs = mir_t2a(ptrT(json_as_string(json_get(pResponse, "ts"))));
	m_pollingKey = mir_t2a(ptrT(json_as_string(json_get(pResponse, "key"))));
	m_pollingServer = mir_t2a(ptrT(json_as_string(json_get(pResponse, "server"))));
	if (!m_hPollingThread) {
		debugLogA("CVkProto::OnReceivePollingInfo m_hPollingThread is NULL");
		debugLogA("CVkProto::OnReceivePollingInfo m_pollingTs = \'%s' m_pollingKey = \'%s\' m_pollingServer = \'%s\'",
			m_pollingTs ? m_pollingTs : "<NULL>",
			m_pollingKey ? m_pollingKey : "<NULL>",
			m_pollingServer ? m_pollingServer : "<NULL>");
		if (m_pollingTs != NULL && m_pollingKey != NULL && m_pollingServer != NULL) {
			debugLogA("CVkProto::OnReceivePollingInfo PollingThread starting...");
			m_hPollingThread = ForkThreadEx(&CVkProto::PollingThread, NULL, NULL);
		}
		else {
			debugLogA("CVkProto::OnReceivePollingInfo PollingThread not start");
			m_pollingConn = NULL;
			ShutdownSession();
			return;
		}
	}
	else
		debugLogA("CVkProto::OnReceivePollingInfo m_hPollingThread is not NULL");
}

void CVkProto::PollUpdates(JSONNODE *pUpdates)
{
	debugLogA("CVkProto::PollUpdates");
	CMStringA mids;
	int msgid, uid, flags, platform;
	MCONTACT hContact;

	JSONNODE *pChild;
	for (int i = 0; (pChild = json_at(pUpdates, i)) != NULL; i++) {
		switch (json_as_int(json_at(pChild, 0))) {
		case VKPOLL_MSG_DELFLAGS:
			msgid = json_as_int(json_at(pChild, 1));
			flags = json_as_int(json_at(pChild, 2));
			uid = json_as_int(json_at(pChild, 3));
			hContact = FindUser(uid);

			if (hContact != NULL && (flags & VKFLAG_MSGUNREAD) && !CheckMid(m_incIds, msgid)) {
				setDword(hContact, "LastMsgReadTime", time(NULL));
				SetSrmmReadStatus(hContact);
				if (m_bUserForceOnlineOnActivity)
					SetInvisible(hContact);
			}
			break;

		case VKPOLL_MSG_ADDED: // new message
			msgid = json_as_int(json_at(pChild, 1));

			// skip outgoing messages sent from a client
			flags = json_as_int(json_at(pChild, 2));
			if (flags & VKFLAG_MSGOUTBOX && !(flags & VKFLAG_MSGCHAT))
				if (CheckMid(m_sendIds, msgid))
					break;

			if (!mids.IsEmpty())
				mids.AppendChar(',');
			mids.AppendFormat("%d", msgid);
			break;

		case VKPOLL_READ_ALL_OUT:
			uid = json_as_int(json_at(pChild, 1));
			hContact = FindUser(uid);
			if (hContact != NULL) {
				setDword(hContact, "LastMsgReadTime", time(NULL));
				SetSrmmReadStatus(hContact);
				if (m_bUserForceOnlineOnActivity)
					SetInvisible(hContact);
			}
			break;

		case VKPOLL_USR_ONLINE:
			uid = -json_as_int(json_at(pChild, 1));
			if ((hContact = FindUser(uid)) != NULL) {
				setWord(hContact, "Status", ID_STATUS_ONLINE);
				platform = json_as_int(json_at(pChild, 2));
				SetMirVer(hContact, platform);
			}
			break;

		case VKPOLL_USR_OFFLINE:
			uid = -json_as_int(json_at(pChild, 1));
			if ((hContact = FindUser(uid)) != NULL) {
				setWord(hContact, "Status", ID_STATUS_OFFLINE);
				db_unset(hContact, m_szModuleName, "ListeningTo");
				SetMirVer(hContact, -1);
			}
			break;

		case VKPOLL_USR_UTN:
			uid = json_as_int(json_at(pChild, 1));
			hContact = FindUser(uid);
			if (hContact != NULL) {
				ForkThread(&CVkProto::ContactTypingThread, (void *)hContact);
				if (m_bUserForceOnlineOnActivity)
					SetInvisible(hContact);
			}
			break;

		case VKPOLL_CHAT_CHANGED:
			int chat_id = json_as_int(json_at(pChild, 1));
			CVkChatInfo *cc = m_chats.find((CVkChatInfo*)&chat_id);
			if (cc)
				RetrieveChatInfo(cc);
			break;
		}
	}

	RetrieveMessagesByIds(mids);
}

int CVkProto::PollServer()
{
	debugLogA("CVkProto::PollServer");
	if (!IsOnline()) {
		debugLogA("CVkProto::PollServer is dead (not online)");
		m_pollingConn = NULL;
		ShutdownSession();
		return 0;
	}

	debugLogA("CVkProto::PollServer (online)");
	int iPollConnRetry = MAX_RETRIES;
	NETLIBHTTPREQUEST *reply;
	CMStringA szReqUrl;
	szReqUrl.AppendFormat("https://%s?act=a_check&key=%s&ts=%s&wait=25&access_token=%s&mode=%d", m_pollingServer, m_pollingKey, m_pollingTs, m_szAccessToken, 106);
	// see mode parametr description on https://vk.com/dev/using_longpoll (Russian version)
	NETLIBHTTPREQUEST req = { sizeof(req) };
	req.requestType = REQUEST_GET;
	req.szUrl = mir_strdup(szReqUrl.GetBuffer());
	req.flags = VK_NODUMPHEADERS | NLHRF_PERSISTENT | NLHRF_HTTP11 | NLHRF_SSL;
	req.timeout = 30000;
	req.nlc = m_pollingConn;

	while ((reply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)&req)) == NULL) {
		debugLogA("CVkProto::PollServer is dead");
		m_pollingConn = NULL;
		if (iPollConnRetry && !m_bTerminated) {
			iPollConnRetry--;
			debugLogA("CVkProto::PollServer restarting %d", MAX_RETRIES - iPollConnRetry);
			Sleep(1000);
		}
		else {
			debugLogA("CVkProto::PollServer => ShutdownSession");
			mir_free(req.szUrl);
			ShutdownSession();
			return 0;
		}
	}

	mir_free(req.szUrl);

	int retVal = 0;
	if (reply->resultCode == 200) {
		JSONROOT pRoot(reply->pData);
		JSONNODE *pFailed = json_get(pRoot, "failed");
		if (pFailed != NULL && json_as_int(pFailed) == 2) {
			RetrievePollingInfo();
			retVal = -1;
			debugLogA("Polling key expired, restarting polling thread");
		}
		else if (CheckJsonResult(NULL, pRoot)) {
			m_pollingTs = mir_t2a(ptrT(json_as_string(json_get(pRoot, "ts"))));
			JSONNODE *pUpdates = json_get(pRoot, "updates");
			if (pUpdates != NULL)
				PollUpdates(pUpdates);
			retVal = 1;
		}
	}
	else if ((reply->resultCode >= 400 && reply->resultCode <= 417)
		|| (reply->resultCode >= 500 && reply->resultCode <= 509)) {
		debugLogA("CVkProto::PollServer is dead. Error code - %d", reply->resultCode);
		m_pollingConn = NULL;
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)reply);
		ShutdownSession();
		return 0;
	}

	m_pollingConn = reply->nlc;

	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)reply);
	debugLogA("CVkProto::PollServer return %d", retVal);
	return retVal;
}

void CVkProto::PollingThread(void*)
{
	debugLogA("CVkProto::PollingThread: entering");

	while (!m_bTerminated)
		if (PollServer() == -1)
			break;

	m_hPollingThread = NULL;
	m_pollingConn = NULL;
	debugLogA("CVkProto::PollingThread: leaving");
}