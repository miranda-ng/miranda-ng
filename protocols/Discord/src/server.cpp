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
// retrieves server history 

void CDiscordProto::RetrieveHistory(MCONTACT hContact, CDiscordHitoryOp iOp, SnowFlake msgid, int iLimit)
{
	CDiscordUser *pUser = FindUser(getId(hContact, DB_KEY_ID));
	if (pUser == NULL)
		return;

	CMStringA szUrl(FORMAT, "/channels/%lld/messages", pUser->channelId);
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_GET, szUrl, &CDiscordProto::OnReceiveHistory);
	pReq << INT_PARAM("limit", iLimit);
	switch (iOp) {
	case MSG_AFTER:
		pReq << CHAR_PARAM("after", CMStringA(FORMAT, "%lld", msgid)); break;
	case MSG_BEFORE:
		pReq << CHAR_PARAM("before", CMStringA(FORMAT, "%lld", msgid)); break;
	}
	pReq->pUserInfo = pUser;
	Push(pReq);
}

void CDiscordProto::OnReceiveHistory(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	CDiscordUser *pUser = (CDiscordUser*)pReq->pUserInfo;

	if (pReply->resultCode != 200)
		return;

	JSONNode root = JSONNode::parse(pReply->pData);
	if (!root)
		return;

	DBEVENTINFO dbei = {};
	dbei.cbSize = sizeof(dbei);
	dbei.szModule = m_szModuleName;
	dbei.flags = DBEF_READ | DBEF_UTF;
	dbei.eventType = EVENTTYPE_MESSAGE;

	SnowFlake lastId = getId(pUser->hContact, DB_KEY_LASTMSGID); // as stored in a database

	for (auto it = root.begin(); it != root.end(); ++it) {
		JSONNode &p = *it;

		SnowFlake authorid = _wtoi64(p["author"]["id"].as_mstring());
		if (authorid == m_ownId)
			dbei.flags |= DBEF_SENT;
		else
			dbei.flags &= ~DBEF_SENT;

		SnowFlake msgid = _wtoi64(p["id"].as_mstring());

		CMStringA szBody(ptrA(mir_utf8encodeW(p["content"].as_mstring())));
		szBody.AppendFormat("%c%lld", 0, msgid);

		dbei.timestamp = StringToDate(p["timestamp"].as_mstring());
		dbei.pBlob = (PBYTE)szBody.GetBuffer();
		dbei.cbBlob = szBody.GetLength();
		db_event_add(pUser->hContact, &dbei);

		if (lastId < msgid)
			lastId = msgid;
	}

	setId(pUser->hContact, DB_KEY_LASTMSGID, lastId);
}

/////////////////////////////////////////////////////////////////////////////////////////
// retrieves user info

void CDiscordProto::RetrieveUserInfo(MCONTACT hContact)
{
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_GET, "/users/@me", &CDiscordProto::OnReceiveUserInfo);
	pReq->pUserInfo = (void*)hContact;
	Push(pReq);
}

void CDiscordProto::OnReceiveUserInfo(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	MCONTACT hContact = (MCONTACT)pReq->pUserInfo;
	if (pReply->resultCode != 200) {
		if (hContact == NULL)
			ConnectionFailed(LOGINERR_WRONGPASSWORD);
		return;
	}

	JSONNode root = JSONNode::parse(pReply->pData);
	if (!root) {
		if (hContact == NULL)
			ConnectionFailed(LOGINERR_NOSERVER);
		return;
	}

	m_ownId = _wtoi64(root["id"].as_mstring());
	setId(hContact, DB_KEY_ID, m_ownId);

	setByte(hContact, DB_KEY_MFA, root["mfa_enabled"].as_bool());
	setDword(hContact, DB_KEY_DISCR, root["discriminator"].as_int());
	setWString(hContact, DB_KEY_NICK, root["username"].as_mstring());
	setWString(hContact, DB_KEY_AVHASH, root["avatar"].as_mstring());
	setWString(hContact, DB_KEY_EMAIL, root["email"].as_mstring());

	if (hContact == NULL)
		OnLoggedIn();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::SetServerStatus(int iStatus)
{
	if (!m_bOnline)
		return;

	if (iStatus == ID_STATUS_OFFLINE)
		Push(new AsyncHttpRequest(this, REQUEST_POST, "/auth/logout", NULL));

	int iOldStatus = m_iStatus; m_iStatus = iStatus;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
}

void CDiscordProto::OnReceiveAuth(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	MCONTACT hContact = (MCONTACT)pReq->pUserInfo;
	if (pReply->resultCode == 204)
		RetrieveUserInfo(hContact);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::OnReceiveChannels(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	if (pReply->resultCode != 200)
		return;

	JSONNode root = JSONNode::parse(pReply->pData);
	if (!root)
		return;

	for (auto it = root.begin(); it != root.end(); ++it) {
		JSONNode &p = *it;

		JSONNode &user = p["recipient"];
		if (!user)
			continue;

		CDiscordUser *pUser = PrepareUser(user);
		pUser->lastMessageId = _wtoi64(p["last_message_id"].as_mstring());
		pUser->channelId = _wtoi64(p["id"].as_mstring());
		pUser->bIsPrivate = p["is_private"].as_bool();

		setId(pUser->hContact, DB_KEY_CHANNELID, pUser->channelId);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::OnReceiveFriends(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	if (pReply->resultCode != 200)
		return;

	JSONNode root = JSONNode::parse(pReply->pData);
	if (!root)
		return;

	for (auto it = root.begin(); it != root.end(); ++it) {
		JSONNode &p = *it;

		JSONNode &user = p["user"];
		if (user)
			PrepareUser(user);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::OnReceiveGuilds(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	if (pReply->resultCode != 200)
		return;
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

	RetrieveUserInfo(NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////
