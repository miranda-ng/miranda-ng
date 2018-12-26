// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018 Miranda NG team
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------

#include "stdafx.h"

#pragma comment(lib, "libeay32.lib")

void CIcqProto::ConnectionFailed(int iReason)
{
	debugLogA("ConnectionFailed -> reason %d", iReason);

	ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, iReason);
	ShutdownSession();
}

void CIcqProto::OnLoggedIn()
{
	debugLogA("CIcqProto::OnLoggedIn");
	m_bOnline = true;
	SetServerStatus(m_iDesiredStatus);
}

void CIcqProto::OnLoggedOut()
{
	debugLogA("CIcqProto::OnLoggedOut");
	m_bOnline = false;
	m_bTerminated = true;

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

	setAllContactStatuses(ID_STATUS_OFFLINE, false);
}

void CIcqProto::SetServerStatus(int iStatus)
{
	int iOldStatus = m_iStatus; m_iStatus = iStatus;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
}

void CIcqProto::ShutdownSession()
{
	if (m_bTerminated)
		return;

	debugLogA("CIcqProto::ShutdownSession");

	// shutdown all resources
	if (m_hWorkerThread)
		SetEvent(m_evRequestsQueue);

	for (auto &it : m_ConnPool)
		if (it)
			Netlib_Shutdown(it);

	OnLoggedOut();
}

/////////////////////////////////////////////////////////////////////////////////////////

#define CAPS "094613504c7f11d18222444553540000,094613514c7f11d18222444553540000,094613534c7f11d18222444553540000,094613544c7f11d18222444553540000,094613594c7f11d18222444553540000,0946135b4c7f11d18222444553540000,0946135a4c7f11d18222444553540000"
#define EVENTS "myInfo,presence,buddylist,typing,dataIM,userAddedToBuddyList,webrtcMsg,mchat,hist,hiddenChat,diff,permitDeny,imState,notification,apps"
#define FIELDS "aimId,buddyIcon,bigBuddyIcon,iconId,bigIconId,largeIconId,displayId,friendly,offlineMsg,state,statusMsg,userType,phoneNumber,cellNumber,smsNumber,workNumber,otherNumber,capabilities,ssl,abPhoneNumber,moodIcon,lastName,abPhones,abContactName,lastseen,mute,livechat,official"

void CIcqProto::StartSession()
{
	ptrA szDeviceId(getStringA("DeviceId"));
	if (szDeviceId == nullptr) {
		UUID deviceId;
		UuidCreate(&deviceId);
		RPC_CSTR szId;
		UuidToStringA(&deviceId, &szId);
		szDeviceId = mir_strdup((char*)szId);
		setString("DeviceId", szDeviceId);
		RpcStringFreeA(&szId);
	}

	int ts = time(0);
	CMStringA nonce(FORMAT, "%d-2", ts);

	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_POST, ICQ_API_SERVER "/aim/startSession", &CIcqProto::OnStartSession);

	pReq << CHAR_PARAM("a", m_szAToken) << INT_PARAM("activeTimeout", 180) << CHAR_PARAM("assertCaps", CAPS)
		<< INT_PARAM("buildNumber", __BUILD_NUM) << CHAR_PARAM("deviceId", szDeviceId) << CHAR_PARAM("events", EVENTS) 
		<< CHAR_PARAM("f", "json") << CHAR_PARAM("imf", "plain") << CHAR_PARAM("inactiveView", "offline") 
		<< CHAR_PARAM("includePresenceFields", FIELDS) << CHAR_PARAM("invisible", "false")
		<< CHAR_PARAM("k", ICQ_APP_ID) << INT_PARAM("mobile", 0) << CHAR_PARAM("nonce", nonce) << CHAR_PARAM("r", pReq->m_reqId) 
		<< INT_PARAM("rawMsg", 0) << INT_PARAM("sessionTimeout", 7776000) << INT_PARAM("ts", ts) << CHAR_PARAM("view", "online");

	CMStringA hashData(FORMAT, "POST&%s&%s", ptrA(mir_urlEncode(pReq->m_szUrl)), ptrA(mir_urlEncode(pReq->m_szParam)));
	unsigned int len;
	BYTE hashOut[MIR_SHA256_HASH_SIZE];
	HMAC(EVP_sha256(), m_szSessionKey, m_szSessionKey.GetLength(), (BYTE*)hashData.c_str(), hashData.GetLength(), hashOut, &len);
	pReq << CHAR_PARAM("sig_sha256", ptrA(mir_base64_encode(hashOut, sizeof(hashOut))));

	Push(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OnCheckPassword(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	JsonReply root(pReply);
	switch (root.error()) {
	case 200:
		break;

	case 440:
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		return;

	default:
		ConnectionFailed(LOGINERR_WRONGPROTOCOL);
		return;
	}

	JSONNode &data = root.data();
	m_szAToken = data["token"]["a"].as_mstring();
	m_szAToken = ptrA(mir_urlDecode(m_szAToken));
	setString("AToken", m_szAToken);

	CMStringA szSessionSecret = data["sessionSecret"].as_mstring();
	CMStringA szPassTemp = m_szPassword;

	unsigned int len;
	BYTE hashOut[MIR_SHA256_HASH_SIZE];
	HMAC(EVP_sha256(), szPassTemp, szPassTemp.GetLength(), (BYTE*)szSessionSecret.c_str(), szSessionSecret.GetLength(), hashOut, &len);
	m_szSessionKey = ptrA(mir_base64_encode(hashOut, sizeof(hashOut)));
	setString("SessionKey", m_szSessionKey);

	CMStringA szUin = data["loginId"].as_mstring();
	if (szUin)
		m_dwUin = atoi(szUin);

	StartSession();
}

void CIcqProto::OnStartSession(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	JsonReply root(pReply);
	switch (root.error()) {
	case 200:
		break;

	case 401:
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		return;

	default:
		ConnectionFailed(LOGINERR_WRONGPROTOCOL);
		return;
	}

	JSONNode &data = root.data();
	m_fetchBaseURL = data["fetchBaseURL"].as_mstring();
	m_aimsid = data["aimsid"].as_mstring();

	OnLoggedIn();

	for (auto &it : data["events"])
		ProcessEvent(it);

	m_hPollThread = ForkThreadEx(&CIcqProto::PollThread, 0, 0);
}

void CIcqProto::OnReceiveAvatar(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	MCONTACT hContact = (MCONTACT)pReq->pUserInfo;

	const wchar_t *pwszExtension;
	PROTO_AVATAR_INFORMATION ai;
	ai.hContact = hContact;
	ai.format = ProtoGetBufferFormat(pReply->pData, &pwszExtension);
	setByte(hContact, "AvatarType", ai.format);
	GetAvatarFileName(hContact, ai.filename, _countof(ai.filename));

	FILE *out = _wfopen(ai.filename, L"wb");
	if (out != nullptr) {
		fwrite(pReply->pData, pReply->dataLength, 1, out);
		fclose(out);

		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, HANDLE(&ai), 0);
		debugLogW(L"Broadcast new avatar: %s", ai.filename);
	}
	else ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, HANDLE(&ai), 0);
}

void CIcqProto::OnSendMessage(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	IcqOwnMessage *ownMsg = (IcqOwnMessage*)pReq->pUserInfo;

	JsonReply root(pReply);
	if (root.error() != 200) {
		ProtoBroadcastAck(ownMsg->m_hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)ownMsg->m_msgid, 0);
		for (auto &it : m_arOwnIds) {
			if (it == ownMsg) {
				m_arOwnIds.remove(m_arOwnIds.indexOf(&it));
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::ProcessBuddyList(const JSONNode &ev)
{
	for (auto &it : ev["groups"]) {
		CMStringW szGroup = it["name"].as_mstring();
		Clist_GroupCreate(0, szGroup);

		for (auto &buddy : it["buddies"]) {
			DWORD dwUin = _wtol(buddy["aimId"].as_mstring());

			auto *pCache = FindContactByUIN(dwUin);
			if (pCache == nullptr) {
				MCONTACT hContact = db_add_contact();
				Proto_AddToContact(hContact, m_szModuleName);
				setDword(hContact, "UIN", dwUin);
				pCache = new IcqCacheItem(dwUin, hContact);
				{
					mir_cslock l(m_csCache);
					m_arCache.insert(pCache);
				}
			}

			MCONTACT hContact = pCache->m_hContact;
			pCache->m_bInList = true;

			CMStringW wszNick(buddy["friendly"].as_mstring());
			if (!wszNick.IsEmpty())
				setWString(hContact, "Nick", wszNick);

			setDword(hContact, "Status", StatusFromString(buddy["state"].as_mstring()));

			int lastLogin = buddy["lastseen"].as_int();
			if (lastLogin)
				setDword(hContact, "LoginTS", lastLogin);

			CMStringW wszStatus(buddy["statusMsg"].as_mstring());
			if (wszStatus.IsEmpty())
				db_unset(hContact, "CList", "StatusMsg");
			else
				db_set_ws(hContact, "CList", "StatusMsg", wszStatus);

			CMStringW wszIconId(buddy["iconId"].as_mstring());
			CMStringW oldIconID(getMStringW(hContact, "IconId"));
			if (wszIconId != oldIconID) {
				setWString(hContact, "IconId", wszIconId);

				CMStringA szUrl(buddy["buddyIcon"].as_mstring());
				auto *p = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, szUrl, &CIcqProto::OnReceiveAvatar);
				p->pUserInfo = (void*)hContact;
				Push(p);
			}

			db_set_ws(hContact, "CList", "Group", szGroup);
		}
	}

	for (auto &it : m_arCache)
		if (!it->m_bInList)
			db_set_b(it->m_hContact, "CList", "NotOnList", 1);
}

void CIcqProto::ProcessEvent(const JSONNode &ev)
{
	const JSONNode &pData = ev["eventData"];
	CMStringW szType = ev["type"].as_mstring();
	if (szType == L"buddylist")
		ProcessBuddyList(pData);
	else if (szType == L"histDlgState")
		ProcessHistData(pData);
	else if (szType == L"myInfo")
		ProcessMyInfo(pData);
	else if (szType == L"presence")
		ProcessPresence(pData);
	else if (szType == L"typing")
		ProcessTyping(pData);
}

void CIcqProto::ProcessHistData(const JSONNode &ev)
{
	DWORD dwUin = _wtol(ev["sn"].as_mstring());

	IcqCacheItem *pCache = FindContactByUIN(dwUin);
	if (pCache == nullptr)
		return;

	for (auto &it : ev["tail"]["messages"]) {
		CMStringA msgId(it["msgId"].as_mstring());
		CMStringW type(it["mediaType"].as_mstring());
		if (type != "text")
			continue;

		// ignore duplicates
		MEVENT hDbEvent = db_event_getById(m_szModuleName, msgId);
		if (hDbEvent != 0)
			continue;

		// skip own messages, just set the server msgid
		bool bSkipped = false;
		CMStringA reqId(it["reqId"].as_mstring());
		for (auto &ownMsg : m_arOwnIds)
			if (!mir_strcmp(reqId, ownMsg->m_guid)) {
				bSkipped = true;
				ProtoBroadcastAck(ownMsg->m_hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)ownMsg->m_msgid, (LPARAM)msgId.c_str());
				m_arOwnIds.remove(m_arOwnIds.indexOf(&ownMsg));
				break;
			}

		if (!bSkipped) {
			bool bIsOutgoing = it["outgoing"].as_bool();
			ptrA szUtf(mir_utf8encodeW(it["text"].as_mstring()));

			PROTORECVEVENT pre = {};
			pre.flags = (bIsOutgoing) ? PREF_SENT : 0;
			pre.szMsgId = msgId;
			pre.timestamp = it["time"].as_int();
			pre.szMessage = szUtf;
			ProtoChainRecvMsg(pCache->m_hContact, &pre);
		}
	}
}

void CIcqProto::ProcessMyInfo(const JSONNode &ev)
{
	CMStringW wszNick(ev["friendly"].as_mstring());
	if (!wszNick.IsEmpty())
		setWString("Nick", wszNick);

	CMStringW wszIconId(ev["iconId"].as_mstring());
	CMStringW oldIconID(getMStringW("IconId"));
	if (wszIconId != oldIconID) {
		setWString("IconId", wszIconId);

		CMStringA szUrl(ev["buddyIcon"].as_mstring());
		Push(new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, szUrl, &CIcqProto::OnReceiveAvatar));
	}
}

void CIcqProto::ProcessPresence(const JSONNode &ev)
{
	DWORD dwUin = _wtol(ev["aimId"].as_mstring());
	int iStatus = StatusFromString(ev["state"].as_mstring());

	IcqCacheItem *pCache = FindContactByUIN(dwUin);
	if (pCache)
		setDword(pCache->m_hContact, "Status", iStatus);
}

void CIcqProto::ProcessTyping(const JSONNode &ev)
{
	DWORD dwUin = _wtol(ev["aimId"].as_mstring());
	CMStringW wszStatus = ev["typingStatus"].as_mstring();

	IcqCacheItem *pCache = FindContactByUIN(dwUin);
	if (pCache) {
		if (wszStatus == "typing")
			CallService(MS_PROTO_CONTACTISTYPING, pCache->m_hContact, 60);
		else 
			CallService(MS_PROTO_CONTACTISTYPING, pCache->m_hContact, PROTOTYPE_CONTACTTYPING_OFF);
	}
}

void CIcqProto::OnFetchEvents(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	JsonReply root(pReply);
	if (root.error() != 200) {
		ShutdownSession();
		return;
	}

	JSONNode &data = root.data();
	m_fetchBaseURL = data["fetchBaseURL"].as_mstring();

	for (auto &it : data["events"])
		ProcessEvent(it);
}

void __cdecl CIcqProto::PollThread(void*)
{
	debugLogA("Polling thread started");
	bool bFirst = true;

	while (!m_bTerminated) {
		CMStringA szUrl = m_fetchBaseURL;
		if (bFirst) {
			bFirst = false;
			szUrl.Append("&first=1");
		}
		else szUrl.Append("&timeout=60000");

		ExecuteRequest(new AsyncHttpRequest(CONN_FETCH, REQUEST_GET, szUrl, &CIcqProto::OnFetchEvents));
	}

	debugLogA("Polling thread ended");
	m_hPollThread = nullptr;
}
