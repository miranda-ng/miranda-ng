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

void CTeamsProto::SendCreateEndpoint()
{
	auto *pReq = new AsyncHttpRequest(REQUEST_POST, HOST_DEFAULT, "/users/ME/endpoints", &CTeamsProto::OnEndpointCreated);
	pReq->flags |= NLHRF_REDIRECT;
	pReq->m_szParam = "{\"endpointFeatures\":\"Agent,Presence2015,MessageProperties,CustomUserProperties,Casts,ModernBots,AutoIdleForWebApi,secureThreads,notificationStream,InviteFree,SupportsReadReceipts,ued\"}";
	pReq->AddHeader("Origin", "https://web.skype.com");
	pReq->AddHeader("Referer", "https://web.skype.com/");
	pReq->AddAuthentication(this);
	
	PushRequest(pReq);
}

void CTeamsProto::OnEndpointCreated(MHttpResponse *response, AsyncHttpRequest*)
{
	if (IsStatusConnecting(m_iStatus))
		m_iStatus++;

	if (response == nullptr) {
		debugLogA(__FUNCTION__ ": failed to get create endpoint");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, 1001);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	switch (response->resultCode) {
	case 200:
	case 201: // okay, endpoint created
		break;

	case 401: // unauthorized
	default:
		delSetting("TokenExpiresIn");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, 1001);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	// Succeeded, decode the answer
	if (auto *hdr = response->FindHeader("Set-RegistrationToken")) {
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
				m_szRegToken = val;
			else if (name == "endpointId") {
				val.Replace("{", "");
				val.Replace("}", "");
				m_szEndpoint = val;
			}
		}
	}

	LoggedIn();
}

void CTeamsProto::OnEndpointDeleted(MHttpResponse *, AsyncHttpRequest *)
{
	m_szEndpoint.Empty();
	m_szRegToken.Empty();
}
