/*
Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org)

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

CMStringA CTeamsProto::GetTenant()
{
	CMStringA ret(getMStringA("Tenant"));
	return (ret.IsEmpty()) ? "consumers" : ret;
}

void CTeamsProto::Login()
{
	CMStringA szLogin(getMStringA("Login")), szPassword(getMStringA("Password"));
	if (szLogin.IsEmpty() || szPassword.IsEmpty()) {
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, 1001);
		return;
	}

	// login
	int oldStatus = m_iStatus;
	m_iStatus = ID_STATUS_CONNECTING;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

	StartQueue();

	RefreshToken("service::api.fl.teams.microsoft.com::MBI_SSL openid profile offline_access", &CTeamsProto::OnRefreshAccessToken);
	// RefreshToken("service::api.fl.spaces.skype.com::MBI_SSL openid profile offline_access", &CTeamsProto::OnRefreshAccessToken);
	// RefreshToken("https://substrate.office.com/M365.Access openid profile offline_access", &CTeamsProto::OnRefreshSubstrate);
}

void CTeamsProto::RefreshToken(const char *pszScope, AsyncHttpRequest::MTHttpRequestHandler pFunc)
{
	auto *pReq = new AsyncHttpRequest(REQUEST_POST, HOST_LOGIN, "/" + GetTenant() + "/oauth2/v2.0/token", pFunc);
	pReq << CHAR_PARAM("scope", pszScope) << CHAR_PARAM("client_id", "8ec6bc83-69c8-4392-8f08-b3c986009232")
		<< CHAR_PARAM("grant_type", "refresh_token") << CHAR_PARAM("refresh_token", getMStringA("Password"));
	PushRequest(pReq);
}

void CTeamsProto::OnRefreshAccessToken(MHttpResponse *response, AsyncHttpRequest *pRequest)
{}

void CTeamsProto::OnRefreshSubstrate(MHttpResponse *response, AsyncHttpRequest *pRequest)
{}
