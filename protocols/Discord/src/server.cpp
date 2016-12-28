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

	JSONNode *root = json_parse(pReply->pData);
	if (root == NULL) {
		ConnectionFailed(LOGINERR_NOSERVER);
		return;
	}

	setWString("Username", root->at("username").as_mstring());
	setByte("MfaEnabled", root->at("mfa_enabled").as_bool());
	setWString("id", root->at("id").as_mstring());
	setWString("AvatarHash", root->at("avatar").as_mstring());
	setDword("Discriminator", root->at("discriminator").as_int());
	setWString("Email", root->at("email").as_mstring());

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

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::OnReceiveChannels(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	if (pReply->resultCode != 200)
		return;
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

	JSONNode *root = json_parse(pReply->pData);
	if (root == NULL) {
LBL_Error:
		ConnectionFailed(LOGINERR_NOSERVER);
		return;
	}

	CMStringA szToken = root->at("token").as_mstring();
	if (szToken.IsEmpty())
		goto LBL_Error;

	m_szAccessToken = szToken.Detach();
	setString("AccessToken", m_szAccessToken);

	RetrieveMyInfo();
}

/////////////////////////////////////////////////////////////////////////////////////////
