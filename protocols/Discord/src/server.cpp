/*
Copyright © 2016 Miranda NG team

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
