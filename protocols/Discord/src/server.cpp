/*
Copyright © 2016-22 Miranda NG team

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

void CDiscordProto::RetrieveHistory(CDiscordUser *pUser, CDiscordHistoryOp iOp, SnowFlake msgid, int iLimit)
{
	if (!pUser->hContact || getByte(pUser->hContact, DB_KEY_DONT_FETCH))
		return;

	CMStringA szUrl(FORMAT, "/channels/%lld/messages", pUser->channelId);
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_GET, szUrl, &CDiscordProto::OnReceiveHistory);
	pReq << INT_PARAM("limit", iLimit);

	if (msgid) {
		switch (iOp) {
		case MSG_AFTER:
			pReq << INT64_PARAM("after", msgid); break;
		case MSG_BEFORE:
			pReq << INT64_PARAM("before", msgid); break;
		}
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

	JsonReply root(pReply);
	if (!root) {
		if (root.error() == 403) // forbidden, don't try to read it anymore
			setByte(pUser->hContact, DB_KEY_DONT_FETCH, true);
		return;
	}

	SESSION_INFO *si = nullptr;
	if (!pUser->bIsPrivate) {
		si = g_chatApi.SM_FindSession(pUser->wszUsername, m_szModuleName);
		if (si == nullptr) {
			debugLogA("message to unknown channel %lld ignored", pUser->channelId);
			return;
		}
	}

	SnowFlake lastId = getId(pUser->hContact, DB_KEY_LASTMSGID); // as stored in a database

	LIST<JSONNode> arNodes(10, compareMsgHistory);
	int iNumMessages = 0;
	for (auto &it : root.data()) {
		arNodes.insert(&it);
		iNumMessages++;
	}

	for (auto &it : arNodes) {
		auto &pNode = *it;
		CMStringW wszText = PrepareMessageText(pNode);
		CMStringW wszUserId = pNode["author"]["id"].as_mstring();
		SnowFlake msgid = ::getId(pNode["id"]);
		SnowFlake authorid = _wtoi64(wszUserId);
		uint32_t dwTimeStamp = StringToDate(pNode["timestamp"].as_mstring());

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

			ptrA szBody(mir_utf8encodeW(wszText));
			dbei.timestamp = dwTimeStamp;
			dbei.pBlob = (uint8_t*)szBody.get();
			dbei.cbBlob = (uint32_t)mir_strlen(szBody);

			bool bSucceeded = false;
			char szMsgId[100];
			_i64toa_s(msgid, szMsgId, _countof(szMsgId), 10);
			MEVENT hDbEvent = db_event_getById(m_szModuleName, szMsgId);
			if (hDbEvent != 0)
				bSucceeded = 0 == db_event_edit(pUser->hContact, hDbEvent, &dbei);
			
			if (!bSucceeded) {
				dbei.szId = szMsgId;
				db_event_add(pUser->hContact, &dbei);
			}
		}
		else {
			ProcessChatUser(pUser, wszUserId, pNode);

			ParseSpecialChars(si, wszText);

			GCEVENT gce = { m_szModuleName, 0, GC_EVENT_MESSAGE };
			gce.pszID.w = pUser->wszUsername;
			gce.dwFlags = GCEF_ADDTOLOG;
			gce.pszUID.w = wszUserId;
			gce.pszText.w = wszText;
			gce.time = dwTimeStamp;
			gce.bIsMe = authorid == m_ownId;
			Chat_Event(&gce);
		}

		if (lastId < msgid)
			lastId = msgid;
	}

	setId(pUser->hContact, DB_KEY_LASTMSGID, lastId);

	// if we fetched 99 messages, but have smth more to go, continue fetching
	if (iNumMessages == 99 && lastId < pUser->lastMsgId)
		RetrieveHistory(pUser, MSG_AFTER, lastId, 99);
}

/////////////////////////////////////////////////////////////////////////////////////////
// retrieves user info

void CDiscordProto::RetrieveMyInfo()
{
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/users/@me", &CDiscordProto::OnReceiveMyInfo));
}

void CDiscordProto::OnReceiveMyInfo(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	JsonReply root(pReply);
	if (!root) {
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		return;
	}

	auto &data = root.data();
	SnowFlake id = ::getId(data["id"]);
	setId(0, DB_KEY_ID, id);

	setByte(0, DB_KEY_MFA, data["mfa_enabled"].as_bool());
	setDword(0, DB_KEY_DISCR, _wtoi(data["discriminator"].as_mstring()));
	setWString(0, DB_KEY_NICK, data["username"].as_mstring());
	m_wszEmail = data["email"].as_mstring();

	m_ownId = id;
	
	m_szCookie.Empty();
	for (int i=0; i < pReply->headersCount; i++) {
		if (!mir_strcmpi(pReply->headers[i].szName, "Set-Cookie")) {
			char *p = strchr(pReply->headers[i].szValue, ';');
			if (p) *p = 0;
			if (!m_szCookie.IsEmpty())
				m_szCookie.Append("; ");

			m_szCookie.Append(pReply->headers[i].szValue);
		}
	}

	// launch gateway thread
	if (m_szGateway.IsEmpty())
		Push(new AsyncHttpRequest(this, REQUEST_GET, "/gateway", &CDiscordProto::OnReceiveGateway));
	else
		ForkThread(&CDiscordProto::GatewayThread, nullptr);

	CheckAvatarChange(0, data["avatar"].as_mstring());
}

/////////////////////////////////////////////////////////////////////////////////////////
// finds a gateway address

void CDiscordProto::OnReceiveGateway(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	JsonReply root(pReply);
	if (!root) {
		ShutdownSession();
		return;
	}

	auto &data = root.data();
	m_szGateway = data["url"].as_mstring();
	ForkThread(&CDiscordProto::GatewayThread, nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::SetServerStatus(int iStatus)
{
	if (GatewaySendStatus(iStatus, nullptr)) {
		int iOldStatus = m_iStatus; m_iStatus = iStatus;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// channels

void CDiscordProto::OnReceiveCreateChannel(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	JsonReply root(pReply);
	if (root)
		OnCommandChannelCreated(root.data());
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::OnReceiveMessageAck(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	JsonReply root(pReply);
	if (!root)
		return;

	auto &data = root.data();
	CMStringW wszToken(data["token"].as_mstring());
	if (!wszToken.IsEmpty()) {
		JSONNode props; props.set_name("properties");
		JSONNode reply; reply << props;
		reply << CHAR_PARAM("event", "ack_messages") << WCHAR_PARAM("token", data["token"].as_mstring());
		Push(new AsyncHttpRequest(this, REQUEST_POST, "/track", nullptr, &reply));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

#define RECAPTCHA_API_KEY "6Lef5iQTAAAAAKeIvIY-DeexoO3gj7ryl9rLMEnn"
#define RECAPTCHA_SITE_URL "https://discord.com"

void CDiscordProto::OnReceiveToken(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	if (pReply->resultCode != 200) {
		JSONNode root = JSONNode::parse(pReply->pData);
		if (root) {
			const JSONNode &captcha = root["captcha_key"].as_array();
			if (captcha) {
				for (auto &it : captcha) {
					if (it.as_mstring() == "captcha-required") {
						MessageBoxW(NULL, TranslateT("The server requires you to enter the captcha. Miranda will redirect you to a browser now"), L"Discord", MB_OK | MB_ICONINFORMATION);
						Utils_OpenUrl("https://discord.com/app");
					}
				}
			}
			
			for (auto &err: root["errors"]["email"]["_errors"]) {
				CMStringW code(err["code"].as_mstring());
				CMStringW message(err["message"].as_mstring());
				if (!code.IsEmpty() || !message.IsEmpty()) {
					POPUPDATAW popup;
					popup.lchIcon = IcoLib_GetIconByHandle(Skin_GetIconHandle(SKINICON_ERROR), true);
					wcscpy_s(popup.lpwzContactName, m_tszUserName);
					mir_snwprintf(popup.lpwzText, TranslateT("Connection failed.\n%s (%s)."), message.c_str(), code.c_str());
					PUAddPopupW(&popup);
				}
			}
		}
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		return;
	}

	JsonReply root(pReply);
	if (!root) {
		ConnectionFailed(LOGINERR_NOSERVER);
		return;
	}

	auto &data = root.data();
	CMStringA szToken = data["token"].as_mstring();
	if (szToken.IsEmpty()) {
		debugLogA("Strange empty token received, exiting");
		return;
	}

	m_szAccessToken = szToken.Detach();
	setString("AccessToken", m_szAccessToken);
	RetrieveMyInfo();
}
