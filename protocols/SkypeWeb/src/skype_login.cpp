/*
Copyright (c) 2015-22 Miranda NG team (https://miranda-ng.org)

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
	StartQueue();
	int tokenExpires = getDword("TokenExpiresIn");

	pass_ptrA szPassword(getStringA(SKYPE_SETTINGS_PASSWORD));
	if (m_szSkypename.IsEmpty() || szPassword == NULL) {
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		return;
	}

	m_bHistorySynced = m_bThreadsTerminated = false;
	if ((tokenExpires - 1800) > time(0))
		OnLoginSuccess();
	else
		PushRequest(new OAuthRequest());
}

void CSkypeProto::OnLoginOAuth(NETLIBHTTPREQUEST *response, AsyncHttpRequest*)
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

	PushRequest(new CreateEndpointRequest(this));
}

void CSkypeProto::OnEndpointCreated(NETLIBHTTPREQUEST *response, AsyncHttpRequest*)
{
	if (IsStatusConnecting(m_iStatus))
		m_iStatus++;

	if (response == nullptr) {
		debugLogA(__FUNCTION__ ": failed to get create endpoint");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	switch (response->resultCode) {
	case 200:
	case 201: // okay, endpoint created
		break;

	case 301:
	case 302: // redirect to the closest data center
		if (auto *hdr = Netlib_GetHeader(response, "Location")) {
			CMStringA szUrl(hdr+8);
			int iEnd = szUrl.Find('/');
			g_plugin.szDefaultServer = (iEnd != -1) ? szUrl.Left(iEnd) : szUrl;
		}
		PushRequest(new CreateEndpointRequest(this));
		return;

	case 401: // unauthorized
		if (auto *szStatus = Netlib_GetHeader(response, "StatusText"))
			if (!strstr(szStatus, "SkypeTokenExpired"))
				delSetting("TokenSecret");
		delSetting("TokenExpiresIn");
		PushRequest(new LoginOAuthRequest(m_szSkypename, pass_ptrA(getStringA(SKYPE_SETTINGS_PASSWORD))));
		return;

	default:
		delSetting("TokenExpiresIn");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	// Succeeded, decode the answer
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

			if (name == "registrationToken")
				m_szToken = val.Detach();
			else if (name == "endpointId")
				m_szId = val.Detach();
		}
	}

	RefreshStatuses();

	PushRequest(new CreateSubscriptionsRequest());
}

void CSkypeProto::OnEndpointDeleted(NETLIBHTTPREQUEST *, AsyncHttpRequest *)
{
	m_szId = nullptr;
	m_szToken = nullptr;
}

void CSkypeProto::OnSubscriptionsCreated(NETLIBHTTPREQUEST *response, AsyncHttpRequest*)
{
	if (!IsStatusConnecting(m_iStatus))
		return;

	if (response == nullptr) {
		debugLogA(__FUNCTION__ ": failed to create subscription");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	SendPresence();
}

void CSkypeProto::SendPresence()
{
	ptrA epname;

	if (!bUseHostnameAsPlace && wstrPlace && *wstrPlace)
		epname = mir_utf8encodeW(wstrPlace);
	else {
		wchar_t compName[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD size = _countof(compName);
		GetComputerName(compName, &size);
		epname = mir_utf8encodeW(compName);
	}

	PushRequest(new SendCapabilitiesRequest(epname, this));
}

void CSkypeProto::OnCapabilitiesSended(NETLIBHTTPREQUEST *response, AsyncHttpRequest*)
{
	if (!IsStatusConnecting(m_iStatus))
		return;

	if (response == nullptr || response->pData == nullptr) {
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	PushRequest(new SetStatusRequest(MirandaToSkypeStatus(m_iDesiredStatus)));

	LIST<char> skypenames(1);
	for (auto &hContact : AccContacts())
		if (!isChatRoom(hContact))
			skypenames.insert(getId(hContact).Detach());

	PushRequest(new CreateContactsSubscriptionRequest(skypenames));
	FreeList(skypenames);
	skypenames.destroy();

	m_hPollingEvent.Set();

	PushRequest(new LoadChatsRequest());
	PushRequest(new GetContactListRequest(this, nullptr));
	PushRequest(new GetAvatarRequest(ptrA(getStringA("AvatarUrl")), 0));

	if (bAutoHistorySync)
		PushRequest(new SyncHistoryFirstRequest(100));

	JSONNode root = JSONNode::parse(response->pData);
	if (root)
		setString("SelfEndpointName", UrlToSkypeId(root["selfLink"].as_string().c_str()));

	PushRequest(new GetProfileRequest(this, 0));
}

void CSkypeProto::OnStatusChanged(NETLIBHTTPREQUEST *response, AsyncHttpRequest*)
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
