/*
Copyright (c) 2015-20 Miranda NG team (https://miranda-ng.org)

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

void CSkypeProto::Login()
{
	// login
	m_iStatus = ID_STATUS_CONNECTING;
	requestQueue->Start();
	int tokenExpires(getDword("TokenExpiresIn", 0));

	m_szSkypename = getMStringA(SKYPE_SETTINGS_ID);

	pass_ptrA szPassword(getStringA(SKYPE_SETTINGS_PASSWORD));
	if (m_szSkypename.IsEmpty() || szPassword == NULL) {
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		return;
	}

	m_bHistorySynced = m_bThreadsTerminated = false;
	if ((tokenExpires - 1800) > time(0))
		OnLoginSuccess();

	PushRequest(new OAuthRequest(), &CSkypeProto::OnOAuthStart);
}

void CSkypeProto::OnLoginOAuth(const NETLIBHTTPREQUEST *response)
{
	if (!IsStatusConnecting(m_iStatus)) return;

	if (response == nullptr || response->pData == nullptr) {
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	JSONNode json = JSONNode::parse(response->pData);
	if (!json) {
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	if (response->resultCode != 200) {
		int error = 0;
		if (json["status"]) {
			const JSONNode &status = json["status"];
			if (status["code"]) {
				switch (status["code"].as_int()) {
				case 40002:
					ShowNotification(L"Skype", TranslateT("Authentication failed. Invalid username."), NULL, 1);
					error = LOGINERR_BADUSERID;
					break;

				case 40120:
					ShowNotification(L"Skype", TranslateT("Authentication failed. Bad username or password."), NULL, 1);
					error = LOGINERR_WRONGPASSWORD;
					break;

				case 40121:
					ShowNotification(L"Skype", TranslateT("Too many failed authentication attempts with given username or IP."), NULL, 1);
					error = LOGIN_ERROR_TOOMANY_REQUESTS;
					break;

				default:
					ShowNotification(L"Skype", status["text"] ? status["text"].as_mstring() : TranslateT("Authentication failed. Unknown error."), NULL, 1);
					error = LOGIN_ERROR_UNKNOWN;
				}
			}
		}

		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, error);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	if (!json["skypetoken"] || !json["expiresIn"]) {
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	setString("TokenSecret", json["skypetoken"].as_string().c_str());
	setDword("TokenExpiresIn", time(NULL) + json["expiresIn"].as_int());

	OnLoginSuccess();
}

void CSkypeProto::OnLoginSuccess()
{
	if (!IsStatusConnecting(m_iStatus))
		return;

	m_bThreadsTerminated = false;
	ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_SUCCESS, NULL, 0);

	m_szApiToken = getStringA("TokenSecret");

	m_impl.m_heartBeat.StartSafe(600 * 1000);

	SendRequest(new CreateEndpointRequest(this), &CSkypeProto::OnEndpointCreated);
}

void CSkypeProto::OnEndpointCreated(const NETLIBHTTPREQUEST *response)
{
	if (!IsStatusConnecting(m_iStatus))
		return;

	m_iStatus++;

	if (response == nullptr) {
		debugLogA(__FUNCTION__ ": failed to get create endpoint");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	switch (response->resultCode) {
	case 200:
	case 201: // ok, endpoint created
	case 301:
	case 302: // redirect
		break;

	case 401: // unauthorized
		if (auto *szStatus = Netlib_GetHeader(response, "StatusText"))
			if (!strstr(szStatus, "SkypeTokenExpired"))
				delSetting("TokenSecret");
		delSetting("TokenExpiresIn");
		SendRequest(new LoginOAuthRequest(m_szSkypename, pass_ptrA(getStringA(SKYPE_SETTINGS_PASSWORD))), &CSkypeProto::OnLoginOAuth);
		return;

	case 400:
		delSetting("TokenExpiresIn");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;

	default: // it should be rewritten
		SendRequest(new CreateEndpointRequest(this), &CSkypeProto::OnEndpointCreated);
		return;
	}

	if (m_iStatus++ > SKYPE_MAX_CONNECT_RETRIES) {
		debugLogA(__FUNCTION__ ": failed to create endpoint (too many connect retries)");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	if (auto *hdr = Netlib_GetHeader(response, "Set-RegistrationToken")) {
		CMStringA szValue = hdr;
		int iStart = 0;
		while (true) {
			CMStringA szToken = szValue.Tokenize(";", iStart).Trim();
			if (iStart == -1)
				break;
			
			int iStart2 = 0;
			CMStringA name = szToken.Tokenize("=", iStart2);
			CMStringA val = szToken.Mid(iStart2);
			setString(name, val);

			if (name == "registrationToken")
				m_szToken = val.Detach();
			else if (name == "endpointId")
				m_szId = val.Detach();
		}
	}
	
	if (auto *hdr = Netlib_GetHeader(response, "Location"))
		m_szServer = GetServerFromUrl(hdr).Detach();
	
	RefreshStatuses();

	SendRequest(new CreateSubscriptionsRequest(this), &CSkypeProto::OnSubscriptionsCreated);
}

void CSkypeProto::OnSubscriptionsCreated(const NETLIBHTTPREQUEST *response)
{
	if (!IsStatusConnecting(m_iStatus))
		return;

	if (response == nullptr) {
		debugLogA(__FUNCTION__ ": failed to create subscription");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	SendPresence(true);
}

void CSkypeProto::SendPresence(bool isLogin)
{
	ptrA epname;

	if (!m_opts.bUseHostnameAsPlace && m_opts.wstrPlace && *m_opts.wstrPlace)
		epname = mir_utf8encodeW(m_opts.wstrPlace);
	else {
		wchar_t compName[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD size = _countof(compName);
		GetComputerName(compName, &size);
		epname = mir_utf8encodeW(compName);
	}

	if (isLogin)
		SendRequest(new SendCapabilitiesRequest(epname, this), &CSkypeProto::OnCapabilitiesSended);
	else
		PushRequest(new SendCapabilitiesRequest(epname, this));
}

void CSkypeProto::OnCapabilitiesSended(const NETLIBHTTPREQUEST *response)
{
	if (!IsStatusConnecting(m_iStatus))
		return;

	if (response == nullptr || response->pData == nullptr) {
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	SendRequest(new SetStatusRequest(MirandaToSkypeStatus(m_iDesiredStatus), this), &CSkypeProto::OnStatusChanged);

	LIST<char> skypenames(1);
	for (auto &hContact : AccContacts())
		if (!isChatRoom(hContact))
			skypenames.insert(getStringA(hContact, SKYPE_SETTINGS_ID));

	SendRequest(new CreateContactsSubscriptionRequest(skypenames, this));
	FreeList(skypenames);
	skypenames.destroy();

	m_hPollingEvent.Set();

	SendRequest(new LoadChatsRequest(this), &CSkypeProto::OnLoadChats);
	SendRequest(new CreateTrouterRequest(), &CSkypeProto::OnCreateTrouter);
	PushRequest(new GetContactListRequest(this, nullptr), &CSkypeProto::LoadContactList);
	PushRequest(new GetAvatarRequest(ptrA(getStringA("AvatarUrl"))), &CSkypeProto::OnReceiveAvatar, NULL);

	if (m_opts.bAutoHistorySync)
		PushRequest(new SyncHistoryFirstRequest(100, this), &CSkypeProto::OnSyncHistory);

	JSONNode root = JSONNode::parse(response->pData);
	if (root)
		setString("SelfEndpointName", UrlToSkypename(root["selfLink"].as_string().c_str()));

	PushRequest(new GetProfileRequest(this), &CSkypeProto::LoadProfile, nullptr);
}

void CSkypeProto::OnStatusChanged(const NETLIBHTTPREQUEST *response)
{
	if (response == nullptr || response->pData == nullptr) {
		debugLogA(__FUNCTION__ ": failed to change status");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	JSONNode json = JSONNode::parse(response->pData);
	if (!json) {
		debugLogA(__FUNCTION__ ": failed to change status");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	const JSONNode &nStatus = json["status"];
	if (!nStatus) {
		debugLogA(__FUNCTION__ ": result contains no valid status to switch to");
		return;
	}

	int iNewStatus = SkypeToMirandaStatus(nStatus.as_string().c_str());
	if (iNewStatus == ID_STATUS_OFFLINE) {
		debugLogA(__FUNCTION__ ": failed to change status");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	int oldStatus = m_iStatus;
	m_iStatus = m_iDesiredStatus = iNewStatus;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
}
