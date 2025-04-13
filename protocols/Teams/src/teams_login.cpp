/*
Copyright (c) 2025 Miranda NG team (https://miranda-ng.org)

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

#define TEAMS_OAUTH_RESOURCE "https://api.spaces.skype.com"
#define TEAMS_OAUTH_SCOPE "service::api.fl.teams.microsoft.com::MBI_SSL"
#define TEAMS_SKYPETOKEN_SCOPE "service::api.fl.spaces.skype.com::MBI_SSL"
#define TEAMS_PERSONAL_TENANT_ID "9188040d-6c67-4c5b-b112-36a304b66dad"
#define SCOPE_SUFFIX " openid profile offline_access"

void CTeamsProto::LoginError()
{
	ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, 1001);
	SetStatus(ID_STATUS_OFFLINE);

	if (m_iLoginExpires) {
		m_impl.m_loginPoll.StopSafe();
		m_iLoginExpires = 0;
	}
}

void CTeamsProto::LoggedIn()
{
	int oldStatus = m_iStatus;
	m_iStatus = m_iDesiredStatus;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

	SendCreateEndpoint();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTeamsProto::OnReceiveDevicePoll(MHttpResponse *response, AsyncHttpRequest *)
{
	JsonReply reply(response);
	if (!reply) {
		if (!strstr(response->body, "\"error\":\"authorization_pending\""))
			LoginError();
		return;
	}

	if (m_iLoginExpires) {
		m_impl.m_loginPoll.StopSafe();
		m_iLoginExpires = 0;
	}
	m_szDeviceCode.Empty();

	auto &root = reply.data();
	setWString(DBKEY_RTOKEN, root["refresh_token"].as_mstring());

	OauthRefreshServices();
}

void CTeamsProto::LoginPoll()
{
	if (time(0) >= m_iLoginExpires) {
		LoginError();
		return;
	}

	auto *pReq = new AsyncHttpRequest(REQUEST_POST, HOST_LOGIN, "/common/oauth2/token", &CTeamsProto::OnReceiveDevicePoll);
	pReq->AddHeader("Cookie", m_szDeviceCookie);
	pReq << CHAR_PARAM("client_id", TEAMS_CLIENT_ID) << CHAR_PARAM("grant_type", "urn:ietf:params:oauth:grant-type:device_code")
		<< CHAR_PARAM("code", m_szDeviceCode);
	PushRequest(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

const wchar_t wszLoginMessage[] =
LPGENW("To login into Teams you need to open '%S' in a browser and select your Teams account there.") L"\r\n\r\n"
LPGENW("Enter the following code then: %s.") L"\r\n\r\n"
LPGENW("Click Proceed to copy that code to clipboard and launch a browser");

class CDeviceCodeDlg : public CTeamsDlgBase
{
	bool bSucceeded = false;

public:
	CDeviceCodeDlg(CTeamsProto *ppro) :
		CTeamsDlgBase(ppro, IDD_DEVICECODE)
	{}

	bool OnInitDialog() override
	{
		CMStringW wszText(FORMAT, TranslateW(wszLoginMessage), m_proto->m_szVerificationUrl.c_str(), m_proto->m_wszUserCode.c_str());
		SetDlgItemTextW(m_hwnd, IDC_TEXT, wszText);
		return true;
	}

	bool OnApply() override
	{
		bSucceeded = true;
		Utils_OpenUrl(m_proto->m_szVerificationUrl);
		return true;
	}

	void OnDestroy() override
	{
		if (!bSucceeded)
			m_proto->LoginError();
	}
};

static void CALLBACK LaunchDialog(void *param)
{
	(new CDeviceCodeDlg((CTeamsProto *)param))->Show();
}

void CTeamsProto::OnReceiveDeviceToken(MHttpResponse *response, AsyncHttpRequest *)
{
	JsonReply reply(response);
	if (!reply) {
		LoginError();
		return;
	}

	auto &root = reply.data();
	m_wszUserCode = root["user_code"].as_mstring();
	m_szDeviceCode = root["device_code"].as_mstring();
	m_szVerificationUrl = root["verification_url"].as_mstring();
	m_iLoginExpires = time(0) + root["expires_in"].as_int();
	m_impl.m_loginPoll.StartSafe(root["interval"].as_int() * 1000);
	m_szDeviceCookie = response->GetCookies();

	Utils_ClipboardCopy(MClipUnicode(m_wszUserCode));
	CallFunctionAsync(LaunchDialog, this);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTeamsProto::OnReceiveSkypeToken(MHttpResponse *response, AsyncHttpRequest *)
{
	JsonReply reply(response);
	if (!reply) {
		LoginError();
		return;
	}

	auto &token = reply.data()["skypeToken"];
	m_szSkypeToken = token["skypetoken"].as_mstring();
	setWString(DBKEY_ID, token["skypeid"].as_mstring());

	LoggedIn();
}

void CTeamsProto::OnRefreshAccessToken(MHttpResponse *response, AsyncHttpRequest *)
{
	JsonReply reply(response);
	if (!reply) {
		LoginError();
		return;
	}

	auto &root = reply.data();
	m_szAccessToken = root["access_token"].as_mstring();
	setWString(DBKEY_RTOKEN, root["refresh_token"].as_mstring());
}

void CTeamsProto::OnRefreshSkypeToken(MHttpResponse *response, AsyncHttpRequest *)
{
	JsonReply reply(response);
	if (!reply) {
		LoginError();
		return;
	}

	auto &root = reply.data();
	CMStringA szAccessToken(root["access_token"].as_mstring());

	auto *pReq = new AsyncHttpRequest(REQUEST_POST, HOST_TEAMS, "/api/auth/v1.0/authz/consumer", &CTeamsProto::OnReceiveSkypeToken);
	pReq->AddHeader("Authorization", "Bearer " + szAccessToken);
	PushRequest(pReq);
}

void CTeamsProto::OnRefreshSubstrate(MHttpResponse *response, AsyncHttpRequest *)
{
	JsonReply reply(response);
	if (!reply) {
		LoginError();
		return;
	}

	auto &root = reply.data();
	m_szSubstrateToken = root["access_token"].as_mstring();
}

void CTeamsProto::RefreshToken(const char *pszScope, AsyncHttpRequest::MTHttpRequestHandler pFunc)
{
	auto *pReq = new AsyncHttpRequest(REQUEST_POST, HOST_LOGIN, "/" TEAMS_PERSONAL_TENANT_ID "/oauth2/v2.0/token", pFunc);
	pReq << CHAR_PARAM("scope", pszScope) << CHAR_PARAM("client_id", TEAMS_CLIENT_ID)
		<< CHAR_PARAM("grant_type", "refresh_token") << CHAR_PARAM("refresh_token", getMStringA(DBKEY_RTOKEN));
	PushRequest(pReq);
}

void CTeamsProto::OauthRefreshServices()
{
	RefreshToken(TEAMS_SKYPETOKEN_SCOPE SCOPE_SUFFIX, &CTeamsProto::OnRefreshSkypeToken);
	RefreshToken("https://substrate.office.com/M365.Access" SCOPE_SUFFIX, &CTeamsProto::OnRefreshSubstrate);
}

/////////////////////////////////////////////////////////////////////////////////////////
// module entry point

void CTeamsProto::Login()
{
	// set plugin status to connect
	int oldStatus = m_iStatus;
	m_iStatus = ID_STATUS_CONNECTING;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

	// launch http queue
	StartQueue();

	// if refresh token doesn't exist, perform a device code authentication
	m_szAccessToken = getMStringA(DBKEY_RTOKEN);
	if (m_szAccessToken.IsEmpty()) {
		auto *pReq = new AsyncHttpRequest(REQUEST_POST, HOST_LOGIN, "/common/oauth2/devicecode", &CTeamsProto::OnReceiveDeviceToken);
		pReq << CHAR_PARAM("client_id", TEAMS_CLIENT_ID) << CHAR_PARAM("resource", TEAMS_OAUTH_RESOURCE);
		PushRequest(pReq);
	}
	// or use a refresh token otherwise
	else {
		RefreshToken(TEAMS_OAUTH_SCOPE SCOPE_SUFFIX, &CTeamsProto::OnRefreshAccessToken);
		OauthRefreshServices();
	}
}
