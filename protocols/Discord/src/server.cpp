/*
Copyright © 2016-17 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// removes a friend from the server

void CDiscordProto::RemoveFriend(SnowFlake id)
{
	Push(new AsyncHttpRequest(this, REQUEST_DELETE, CMStringA(FORMAT, "/users/@me/relationships/%lld", id), nullptr));
}

/////////////////////////////////////////////////////////////////////////////////////////
// retrieves server history 

void CDiscordProto::RetrieveHistory(MCONTACT hContact, CDiscordHistoryOp iOp, SnowFlake msgid, int iLimit)
{
	CDiscordUser *pUser = FindUser(getId(hContact, DB_KEY_ID));
	if (pUser == nullptr)
		return;

	CMStringA szUrl(FORMAT, "/channels/%lld/messages", pUser->channelId);
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_GET, szUrl, &CDiscordProto::OnReceiveHistory);
	pReq << INT_PARAM("limit", iLimit);
	switch (iOp) {
	case MSG_AFTER:
		pReq << INT64_PARAM("after", msgid); break;
	case MSG_BEFORE:
		pReq << INT64_PARAM("before", msgid); break;
	}
	pReq->pUserInfo = pUser;
	Push(pReq);
}

static int compareMsgHistory(const JSONNode *p1, const JSONNode *p2)
{
	return wcscmp((*p1)["id"].as_mstring(), (*p2)["id"].as_mstring());
}

void CDiscordProto::OnReceiveHistory(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	CDiscordUser *pUser = (CDiscordUser*)pReq->pUserInfo;

	if (pReply->resultCode != 200)
		return;

	JSONNode root = JSONNode::parse(pReply->pData);
	if (!root)
		return;

	SESSION_INFO *si = nullptr;
	if (!pUser->bIsPrivate) {
		si = pci->SM_FindSession(pUser->wszUsername, m_szModuleName);
		if (si == nullptr) {
			debugLogA("nessage to unknown channal %lld ignored", pUser->channelId);
			return;
		}
	}

	SnowFlake lastId = getId(pUser->hContact, DB_KEY_LASTMSGID); // as stored in a database

	LIST<JSONNode> arNodes(10, compareMsgHistory);
	int iNumMessages = 0;
	for (auto it = root.begin(); it != root.end(); ++it, ++iNumMessages) {
		JSONNode &p = *it;
		arNodes.insert(&p);
	}

	for (int i = 0; i < arNodes.getCount(); i++) {
		JSONNode &p = *arNodes[i];

		CMStringW wszText = PrepareMessageText(p);
		CMStringW wszUserId = p["author"]["id"].as_mstring();
		SnowFlake msgid = ::getId(p["id"]);
		SnowFlake authorid = _wtoi64(wszUserId);
		DWORD dwTimeStamp = StringToDate(p["timestamp"].as_mstring());

		if (pUser->bIsPrivate) {
			DBEVENTINFO dbei = {};
			dbei.szModule = m_szModuleName;
			dbei.flags = DBEF_UTF;
			dbei.eventType = EVENTTYPE_MESSAGE;

			if (authorid == m_ownId)
				dbei.flags |= DBEF_SENT;
			else
				dbei.flags &= ~DBEF_SENT;

			if (msgid <= pUser->lastReadId)
				dbei.flags |= DBEF_READ;
			else
				dbei.flags &= ~DBEF_READ;

			CMStringA szBody(ptrA(mir_utf8encodeW(wszText)));
			szBody.AppendFormat("%c%lld", 0, msgid);

			dbei.timestamp = dwTimeStamp;
			dbei.pBlob = (PBYTE)szBody.GetBuffer();
			dbei.cbBlob = szBody.GetLength();
			db_event_add(pUser->hContact, &dbei);
		}
		else {
			ParseSpecialChars(si, wszText);

			GCEVENT gce = { m_szModuleName, pUser->wszUsername, GC_EVENT_MESSAGE };
			gce.dwFlags = GCEF_ADDTOLOG;
			gce.ptszUID = wszUserId;
			gce.ptszText = wszText;
			gce.time = dwTimeStamp;
			gce.bIsMe = authorid == m_ownId;
			Chat_Event(&gce);
		}

		if (lastId < msgid)
			lastId = msgid;
	}

	setId(pUser->hContact, DB_KEY_LASTMSGID, lastId);

	// if we fetched 99 messages, but have smth more to go, continue fetching
	if (iNumMessages == 99 && lastId < pUser->lastMsg.id)
		RetrieveHistory(pUser->hContact, MSG_AFTER, lastId, 99);
}

/////////////////////////////////////////////////////////////////////////////////////////
// retrieves user info

void CDiscordProto::RetrieveMyInfo()
{
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/users/@me", &CDiscordProto::OnReceiveMyInfo));
}

void CDiscordProto::OnReceiveMyInfo(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	if (pReply->resultCode != 200) {
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		return;
	}

	JSONNode root = JSONNode::parse(pReply->pData);
	if (!root) {
		ConnectionFailed(LOGINERR_NOSERVER);
		return;
	}

	SnowFlake id = ::getId(root["id"]);
	setId(0, DB_KEY_ID, id);

	setByte(0, DB_KEY_MFA, root["mfa_enabled"].as_bool());
	setDword(0, DB_KEY_DISCR, _wtoi(root["discriminator"].as_mstring()));
	setWString(0, DB_KEY_NICK, root["username"].as_mstring());
	setWString(0, DB_KEY_EMAIL, root["email"].as_mstring());

	m_ownId = id;
	for (int i = 0; i < pReply->headersCount; i++) {
		if (!strcmp(pReply->headers[i].szName, "Set-Cookie")) {
			char *p = strchr(pReply->headers[i].szValue, ';');
			if (p) *p = 0;
			m_szAccessCookie = mir_strdup(pReply->headers[i].szValue);
		}
	}

	OnLoggedIn();

	CheckAvatarChange(0, root["avatar"].as_mstring());
}

/////////////////////////////////////////////////////////////////////////////////////////
// finds a gateway address

void CDiscordProto::OnReceiveGateway(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	if (pReply->resultCode != 200) {
		ShutdownSession();
		return;
	}

	JSONNode root = JSONNode::parse(pReply->pData);
	if (!root) {
		ShutdownSession();
		return;
	}

	m_szGateway = root["url"].as_mstring();
	ForkThread(&CDiscordProto::GatewayThread, nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::SetServerStatus(int iStatus)
{
	if (!m_bOnline)
		return;

	if (iStatus == ID_STATUS_OFFLINE)
		Push(new AsyncHttpRequest(this, REQUEST_POST, "/auth/logout", nullptr));
	else {
		const char *pszStatus;
		switch (iStatus) {
		case ID_STATUS_AWAY:
		case ID_STATUS_NA: 
			pszStatus = "idle"; break;
		case ID_STATUS_DND:
			pszStatus = "dnd"; break;
		case ID_STATUS_INVISIBLE:
			pszStatus = "invisible"; break;
		default:
			pszStatus = "online"; break;
		}
		JSONNode root; root << CHAR_PARAM("status", pszStatus);
		Push(new AsyncHttpRequest(this, REQUEST_PATCH, "/users/@me/settings", nullptr, &root));
	}

	int iOldStatus = m_iStatus; m_iStatus = iStatus;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
}

/////////////////////////////////////////////////////////////////////////////////////////
// channels

void CDiscordProto::OnReceiveCreateChannel(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	if (pReply->resultCode != 200)
		return;

	JSONNode root = JSONNode::parse(pReply->pData);
	if (root)
		OnCommandChannelCreated(root);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::OnReceiveMessage(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	MCONTACT hContact = (UINT_PTR)pReq->pUserInfo;

	bool bSucceeded = true;
	if (pReply->resultCode != 200 && pReply->resultCode != 204)
		bSucceeded = false;

	JSONNode root = JSONNode::parse(pReply->pData);
	if (root) {
		CDiscordUser *pUser = FindUserByChannel(::getId(root["channel_id"]));
		if (pUser != nullptr)
			pUser->lastMsg = CDiscordMessage(::getId(root["id"]), ::getId(root["author"]["id"]));
	}

	ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, bSucceeded ? ACKRESULT_SUCCESS : ACKRESULT_FAILED, (HANDLE)pReq->m_iReqNum, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::OnReceiveMessageAck(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	if (pReply->resultCode != 200)
		return;

	JSONNode root = JSONNode::parse(pReply->pData);
	if (!root)
		return;

	CMStringW wszToken(root["token"].as_mstring());
	if (!wszToken.IsEmpty()) {
		JSONNode props; props.set_name("properties");
		JSONNode reply; reply << props;
		reply << CHAR_PARAM("event", "ack_messages") << WCHAR_PARAM("token", root["token"].as_mstring());
		Push(new AsyncHttpRequest(this, REQUEST_POST, "/track", nullptr, &reply));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::OnReceiveToken(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	if (pReply->resultCode != 200) {
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		return;
	}

	JSONNode root = JSONNode::parse(pReply->pData);
	if (!root) {
LBL_Error:
		ConnectionFailed(LOGINERR_NOSERVER);
		return;
	}

	CMStringA szToken = root["token"].as_mstring();
	if (szToken.IsEmpty())
		goto LBL_Error;

	m_szAccessToken = szToken.Detach();
	setString("AccessToken", m_szAccessToken);

	RetrieveMyInfo();
}
