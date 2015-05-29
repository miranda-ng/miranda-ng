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

	JSONNode jnRoot;
	JSONNode jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse)
		return;

	char ts[32];
	itoa(jnResponse["ts"].as_int(), ts, 10);

	m_pollingTs = mir_strdup(ts);
	m_pollingKey = mir_t2a(jnResponse["key"].as_mstring());
	m_pollingServer = mir_t2a(jnResponse["server"].as_mstring());

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

void CVkProto::PollUpdates(const JSONNode &jnUpdates)
{
	debugLogA("CVkProto::PollUpdates");
	CMStringA mids;
	int msgid, uid, flags, platform;
	MCONTACT hContact;

	
	for (auto it = jnUpdates.begin(); it != jnUpdates.end(); ++it) {
		const JSONNode &jnChild = (*it).as_array();
		switch (jnChild[json_index_t(0)].as_int()) {
		case VKPOLL_MSG_DELFLAGS:
			msgid = jnChild[1].as_int();
			flags = jnChild[2].as_int();
			uid = jnChild[3].as_int();
			hContact = FindUser(uid);

			if (hContact != NULL && (flags & VKFLAG_MSGUNREAD) && !CheckMid(m_incIds, msgid)) {
				setDword(hContact, "LastMsgReadTime", time(NULL));
				SetSrmmReadStatus(hContact);
				if (m_bUserForceOnlineOnActivity)
					SetInvisible(hContact);
			}
			break;

		case VKPOLL_MSG_ADDED: // new message
			msgid = jnChild[1].as_int();

			// skip outgoing messages sent from a client
			flags = jnChild[2].as_int();
			if (flags & VKFLAG_MSGOUTBOX && !(flags & VKFLAG_MSGCHAT))
				if (CheckMid(m_sendIds, msgid))
					break;

			if (!mids.IsEmpty())
				mids.AppendChar(',');
			mids.AppendFormat("%d", msgid);
			break;

		case VKPOLL_READ_ALL_OUT:
			uid = jnChild[1].as_int();
			hContact = FindUser(uid);
			if (hContact != NULL) {
				setDword(hContact, "LastMsgReadTime", time(NULL));
				SetSrmmReadStatus(hContact);
				if (m_bUserForceOnlineOnActivity)
					SetInvisible(hContact);
			}
			break;

		case VKPOLL_USR_ONLINE:
			uid = -jnChild[1].as_int();
			if ((hContact = FindUser(uid)) != NULL) {
				setWord(hContact, "Status", ID_STATUS_ONLINE);
				platform = jnChild[2].as_int();
				SetMirVer(hContact, platform);
			}
			break;

		case VKPOLL_USR_OFFLINE:
			uid = -jnChild[1].as_int();
			if ((hContact = FindUser(uid)) != NULL) {
				setWord(hContact, "Status", ID_STATUS_OFFLINE);
				db_unset(hContact, m_szModuleName, "ListeningTo");
				SetMirVer(hContact, -1);
			}
			break;

		case VKPOLL_USR_UTN:
			uid = jnChild[1].as_int();
			hContact = FindUser(uid);
			if (hContact != NULL) {
				ForkThread(&CVkProto::ContactTypingThread, (void *)hContact);
				if (m_bUserForceOnlineOnActivity)
					SetInvisible(hContact);
			}
			break;

		case VKPOLL_CHAT_CHANGED:
			int chat_id = jnChild[1].as_int();
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
	req.szUrl = mir_strdup(szReqUrl);
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
		JSONNode jnRoot = JSONNode::parse(reply->pData);
		const JSONNode &jnFailed = jnRoot["failed"];
		if (!jnFailed.isnull() && jnFailed.as_int() > 1) {
			RetrievePollingInfo();
			retVal = -1;
			debugLogA("Polling key expired, restarting polling thread");
		}
		else if (CheckJsonResult(NULL, jnRoot)) {
			char ts[32];
			itoa(jnRoot["ts"].as_int(), ts, 10);
			m_pollingTs = mir_strdup(ts);
			const JSONNode &jnUpdates = jnRoot["updates"];
			if (!jnUpdates.isnull())
				PollUpdates(jnUpdates);
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