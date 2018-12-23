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
	if (m_hAPIConnection)
		Netlib_Shutdown(m_hAPIConnection);

	OnLoggedOut();
}

/////////////////////////////////////////////////////////////////////////////////////////

#define CAPS "094613504c7f11d18222444553540000,094613514c7f11d18222444553540000,094613534c7f11d18222444553540000,094613544c7f11d18222444553540000,094613594c7f11d18222444553540000,0946135b4c7f11d18222444553540000,0946135a4c7f11d18222444553540000"
#define EVENTS "myInfo,presence,buddylist,typing,dataIM,userAddedToBuddyList,service,webrtcMsg,mchat,hist,hiddenChat,diff,permitDeny,imState,notification,apps"
#define FIELDS "aimId,buddyIcon,bigBuddyIcon,iconId,bigIconId,largeIconId,displayId,friendly,offlineMsg,state,statusMsg,userType,phoneNumber,cellNumber,smsNumber,workNumber,otherNumber,capabilities,ssl,abPhoneNumber,moodIcon,lastName,abPhones,abContactName,lastseen,mute,livechat,official"

void CIcqProto::OnCheckPassword(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	if (pReply->resultCode != 200 || pReply->pData == nullptr) {
		ConnectionFailed(LOGINERR_WRONGPROTOCOL);
		return;
	}

	JSONROOT root(pReply->pData);
	if (!root) {
		ConnectionFailed(LOGINERR_WRONGPROTOCOL);
		return;
	}

	JSONNode response = (*root)["response"];
	switch (response["statusCode"].as_int()) {
	case 200:
		{
			JSONNode data = response["data"];
			m_szAToken = data["token"]["a"].as_mstring();
			m_szSessionSecret = data["sessionSecret"].as_mstring();
		
			CMStringA szUin = data["loginId"].as_mstring();
			if (szUin)
				setDword("UIN", atoi(szUin));
		}
		break;

	case 440:
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		return;

	default:
		ConnectionFailed(LOGINERR_WRONGPROTOCOL);
		return;
	}

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

	auto *pReq = new AsyncHttpRequest(REQUEST_POST, "https://api.icq.net/aim/startSession", &CIcqProto::OnStartSession);

	RPC_CSTR szId;
	UuidToStringA(&pReq->m_reqId, &szId);

	pReq << CHAR_PARAM("a", m_szAToken) << INT_PARAM("activeTimeout", 180) << CHAR_PARAM("assertCaps", CAPS)
		<< INT_PARAM("buildNumber", __BUILD_NUM) << CHAR_PARAM("clientName", "Miranda NG") << INT_PARAM("clientVersion", 5000)
		<< CHAR_PARAM("deviceId", szDeviceId) << CHAR_PARAM("events", EVENTS) << CHAR_PARAM("f", "json") << CHAR_PARAM("imf", "plain")
		<< CHAR_PARAM("inactiveView", "offline") << CHAR_PARAM("includePresenceFields", FIELDS) << CHAR_PARAM("invisible", "false")
		<< CHAR_PARAM("k", "ic1nmMjqg7Yu-0hL") << INT_PARAM("majorVersion", __MAJOR_VERSION) << INT_PARAM("minorVersion", __MINOR_VERSION)
		<< INT_PARAM("mobile", 0) << CHAR_PARAM("nonce", nonce) << INT_PARAM("pointVersion", 0) << CHAR_PARAM("r", (char*)szId) 
		<< INT_PARAM("rawMsg", 0) << INT_PARAM("sessionTimeout", 7776000) << INT_PARAM("ts", ts) << CHAR_PARAM("view", "online");

	ptrA szPassword(getStringA("Password"));

	BYTE hashOut[MIR_SHA256_HASH_SIZE];
	unsigned int len = sizeof(hashOut);
	HMAC(EVP_sha256(), szPassword.get(), mir_strlen(szPassword), (BYTE*)m_szSessionSecret.c_str(), m_szSessionSecret.GetLength(), hashOut, &len);
	ptrA szSessionKey(mir_base64_encode(hashOut, sizeof(hashOut)));

	CMStringA hashData(FORMAT, "POST&%s&%s", ptrA(mir_urlEncode(pReq->szUrl)), ptrA(mir_urlEncode(pReq->m_szParam)));
	HMAC(EVP_sha256(), szSessionKey.get(), mir_strlen(szSessionKey), (BYTE*)hashData.c_str(), hashData.GetLength(), hashOut, &len);

	pReq->m_szParam.Empty();
	pReq << CHAR_PARAM("a", m_szAToken) << INT_PARAM("activeTimeout", 180) << CHAR_PARAM("assertCaps", CAPS)
		<< INT_PARAM("buildNumber", __BUILD_NUM) << CHAR_PARAM("clientName", "Miranda NG") << INT_PARAM("clientVersion", 5000)
		<< CHAR_PARAM("deviceId", szDeviceId) << CHAR_PARAM("events", EVENTS) << CHAR_PARAM("f", "json") << CHAR_PARAM("imf", "plain")
		<< CHAR_PARAM("inactiveView", "offline") << CHAR_PARAM("includePresenceFields", FIELDS) << CHAR_PARAM("invisible", "false")
		<< CHAR_PARAM("k", "ic1nmMjqg7Yu-0hL") << INT_PARAM("majorVersion", __MAJOR_VERSION) << INT_PARAM("minorVersion", __MINOR_VERSION)
		<< INT_PARAM("mobile", 0) << CHAR_PARAM("nonce", nonce) << INT_PARAM("pointVersion", 0) << CHAR_PARAM("r", (char*)szId) 
		<< INT_PARAM("rawMsg", 0) << INT_PARAM("sessionTimeout", 7776000) << CHAR_PARAM("sig_sha256", ptrA(mir_base64_encode(hashOut, sizeof(hashOut)))) 
		<< INT_PARAM("ts", ts) << CHAR_PARAM("view", "online");

	Push(pReq);
	RpcStringFreeA(&szId);
}

void CIcqProto::OnStartSession(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	if (pReply->resultCode != 200 || pReply->pData == nullptr) {
		ConnectionFailed(LOGINERR_WRONGPROTOCOL);
		return;
	}

	JSONROOT root(pReply->pData);
	if (!root) {
		ConnectionFailed(LOGINERR_WRONGPROTOCOL);
		return;
	}

	JSONNode response = (*root)["response"];
	switch (response["statusCode"].as_int()) {
	case 200:
		OnLoggedIn();
		break;

	case 401:
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		break;

	default:
		ConnectionFailed(LOGINERR_WRONGPROTOCOL);
	}
}
