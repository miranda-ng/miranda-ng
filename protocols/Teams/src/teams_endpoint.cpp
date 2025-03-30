/*
Copyright (c) 2015-25 Miranda NG team (https://miranda-ng.org)

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

void CTeamsProto::CheckConvert()
{
	m_szSkypename = getMStringA(SKYPE_SETTINGS_ID);
	if (m_szSkypename.IsEmpty()) {
		m_szSkypename = getMStringA(SKYPE_SETTINGS_LOGIN);
		if (!m_szSkypename.IsEmpty()) { // old settings format, need to update all settings
			m_szSkypename.Insert(0, "8:");
			setString(SKYPE_SETTINGS_ID, m_szSkypename);

			for (auto &hContact : AccContacts()) {
				CMStringA id(ptrA(getUStringA(hContact, "Skypename")));
				if (!id.IsEmpty())
					setString(hContact, SKYPE_SETTINGS_ID, (isChatRoom(hContact)) ? "19:" + id : "8:" + id);

				ptrW wszNick(getWStringA(hContact, "Nick"));
				if (wszNick == nullptr)
					setUString(hContact, "Nick", id);

				delSetting(hContact, "Skypename");
			}
		}
	}
}

void CTeamsProto::ProcessTimer()
{
	if (!IsOnline())
		return;

	PushRequest(new GetContactListRequest());
	SendPresence();
}

void CTeamsProto::SendCreateEndpoint()
{
	auto *pReq = new AsyncHttpRequest(REQUEST_POST, HOST_DEFAULT, "/users/ME/endpoints", &CTeamsProto::OnEndpointCreated);
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

	case 301:
	case 302: // redirect to the closest data center
		if (auto *hdr = response->FindHeader("Location")) {
			CMStringA szUrl(hdr+8);
			int iEnd = szUrl.Find('/');
			// g_plugin.szDefaultServer = (iEnd != -1) ? szUrl.Left(iEnd) : szUrl;
		}
		SendCreateEndpoint();
		return;

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
				m_szId = val.Detach();
		}
	}

	if (m_szId && m_hPollingThread == nullptr)
		ForkThread(&CTeamsProto::PollingThread);

	PushRequest(new CreateSubscriptionsRequest());
}

void CTeamsProto::OnEndpointDeleted(MHttpResponse *, AsyncHttpRequest *)
{
	m_szId = nullptr;
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
	PushRequest(new GetContactListRequest());
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
