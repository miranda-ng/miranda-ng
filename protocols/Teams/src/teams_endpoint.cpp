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
	int oldStatus = m_iStatus;
	m_iStatus = m_iDesiredStatus;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

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
				m_szToken = val.Detach();
			else if (name == "endpointId")
				m_szEndpoint = val.Detach();
		}
	}

	StartTrouter();
	PushRequest(new CreateSubscriptionsRequest());
}

void CTeamsProto::OnEndpointDeleted(MHttpResponse *, AsyncHttpRequest *)
{
	m_szEndpoint = nullptr;
	m_szToken = nullptr;
}

void CTeamsProto::OnSubscriptionsCreated(MHttpResponse *response, AsyncHttpRequest*)
{
	if (response == nullptr) {
		debugLogA(__FUNCTION__ ": failed to create subscription");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, 1001);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	SendPresence();
}

void CTeamsProto::SendPresence()
{
	ptrA epname;

	if (!m_bUseHostnameAsPlace && m_wstrPlace && *m_wstrPlace)
		epname = mir_utf8encodeW(m_wstrPlace);
	else {
		wchar_t compName[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD size = _countof(compName);
		GetComputerName(compName, &size);
		epname = mir_utf8encodeW(compName);
	}

	PushRequest(new SendCapabilitiesRequest(epname, this));
}

void CTeamsProto::OnCapabilitiesSended(MHttpResponse *response, AsyncHttpRequest*)
{
	if (response == nullptr || response->body.IsEmpty()) {
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, 1001);
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

	ReceiveAvatar(0);
	PushRequest(new AsyncHttpRequest(REQUEST_GET, HOST_CONTACTS, "/users/SELF/contacts", &CTeamsProto::LoadContactList));
	PushRequest(new SyncConversations());

	JSONNode root = JSONNode::parse(response->body);
	if (root)
		m_szOwnSkypeId = UrlToSkypeId(root["selfLink"].as_string().c_str()).Detach();

	PushRequest(new GetProfileRequest(this, 0));
}

void CTeamsProto::OnStatusChanged(MHttpResponse *response, AsyncHttpRequest*)
{
	if (response == nullptr || response->body.IsEmpty()) {
		debugLogA(__FUNCTION__ ": failed to change status");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, 1001);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	JSONNode json = JSONNode::parse(response->body);
	if (!json) {
		debugLogA(__FUNCTION__ ": failed to change status");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, 1001);
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
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, 1001);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	int oldStatus = m_iStatus;
	m_iStatus = m_iDesiredStatus = iNewStatus;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
}
