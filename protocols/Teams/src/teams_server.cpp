/*
Copyright (C) 2025 Miranda NG team (https://miranda-ng.org)

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

/////////////////////////////////////////////////////////////////////////////////////////

void CTeamsProto::OnCapabilitiesSended(MHttpResponse *response, AsyncHttpRequest *)
{
	if (response == nullptr || response->body.IsEmpty()) {
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, 1001);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	CreateContactSubscription();
	ReceiveAvatar(0);
	PushRequest(new AsyncHttpRequest(REQUEST_GET, HOST_CONTACTS, "/users/SELF/contacts", &CTeamsProto::LoadContactList));
	PushRequest(new SyncConversations());

	JSONNode root = JSONNode::parse(response->body);
	if (root)
		m_szOwnSkypeId = UrlToSkypeId(root["selfLink"].as_string().c_str()).Detach();

	GetProfileInfo(0);
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

	JSONNode privateInfo; privateInfo.set_name("privateInfo");
	privateInfo << CHAR_PARAM("epname", epname);

	JSONNode publicInfo; publicInfo.set_name("publicInfo");
	publicInfo << CHAR_PARAM("capabilities", "Audio|Video") << INT_PARAM("typ", 125)
		<< CHAR_PARAM("skypeNameVersion", "Miranda NG Skype") << CHAR_PARAM("nodeInfo", "xx") << CHAR_PARAM("version", g_szMirVer);

	JSONNode node;
	node << CHAR_PARAM("id", "messagingService") << CHAR_PARAM("type", "EndpointPresenceDoc")
		<< CHAR_PARAM("selfLink", "uri") << privateInfo << publicInfo;

	auto *pReq = new AsyncHttpRequest(REQUEST_PUT, HOST_DEFAULT, "/users/ME/endpoints/" + mir_urlEncode(m_szEndpoint) + "/presenceDocs/messagingService",
		&CTeamsProto::OnCapabilitiesSended);
	pReq->m_szParam = node.write().c_str();
	pReq->AddHeader("RegistrationToken", CMStringA("registrationToken=") + m_szToken);
	PushRequest(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTeamsProto::OnStatusChanged(MHttpResponse *response, AsyncHttpRequest *)
{
	if (response == nullptr || response->resultCode != 200) {
		debugLogA(__FUNCTION__ ": failed to change status");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, 1001);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	int oldStatus = m_iStatus;
	m_iStatus = m_iDesiredStatus;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
}

void CTeamsProto::SetServerStatus(int iStatus)
{
	const char *pszAvailability;
	switch (iStatus) {
	case ID_STATUS_OFFLINE:
		pszAvailability = "Offline";
		break;
	case ID_STATUS_NA:
	case ID_STATUS_AWAY:
		pszAvailability = "Away";
		break;
	case ID_STATUS_DND:
		pszAvailability = "DoNotDisturb";
		break;
	case ID_STATUS_OCCUPIED:
		pszAvailability = "Busy";
		break;
	default:
		pszAvailability = "Available";
	}

	JSONNode node(JSON_NODE);
	node << CHAR_PARAM("availability", pszAvailability);

	auto *pReq = new AsyncHttpRequest(REQUEST_PUT, HOST_PRESENCE, "/me/forceavailability", &CTeamsProto::OnStatusChanged);
	pReq->m_szParam = node.write().c_str();
	PushRequest(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTeamsProto::OnSubscriptionsCreated(MHttpResponse *response, AsyncHttpRequest *)
{
	if (response == nullptr) {
		debugLogA(__FUNCTION__ ": failed to create subscription");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, 1001);
		SetStatus(ID_STATUS_OFFLINE);
	}
	else SendPresence();
}

void CTeamsProto::CreateSubscription()
{
	JSONNode interestedResources(JSON_ARRAY); interestedResources.set_name("interestedResources");
	interestedResources << CHAR_PARAM("", "/v1/users/ME/conversations/ALL/properties")
		<< CHAR_PARAM("", "/v1/users/ME/conversations/ALL/messages")
		<< CHAR_PARAM("", "/v1/users/ME/contacts/ALL")
		<< CHAR_PARAM("", "/v1/threads/ALL");

	JSONNode subscription, trouter;
	subscription << CHAR_PARAM("channelType", "HttpLongPoll") << interestedResources;
	trouter << CHAR_PARAM("channelType", "TrouterPush") << CHAR_PARAM("longPollUrl", m_szTrouterSurl) << interestedResources;

	JSONNode subscriptions(JSON_ARRAY); subscriptions.set_name("subscriptions");
	subscriptions << subscription << trouter;

	JSONNode node;
	node << INT_PARAM("startingTimeSpan", 0) << CHAR_PARAM("endpointFeatures", "Agent,Presence2015,MessageProperties,CustomUserProperties,NotificationStream,SupportsSkipRosterFromThreads")
		<< subscriptions;

	auto *pReq = new AsyncHttpRequest(REQUEST_PUT, HOST_DEFAULT_V2, "/users/ME/endpoints/" + m_szEndpoint, &CTeamsProto::OnSubscriptionsCreated);
	pReq->m_szParam = node.write().c_str();
	PushRequest(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTeamsProto::CreateContactSubscription()
{
	OBJLIST<char> skypenames(1);
	for (auto &hContact : AccContacts())
		if (!isChatRoom(hContact))
			skypenames.insert(newStr(getId(hContact)));

	JSONNode contacts(JSON_ARRAY); contacts.set_name("contacts");
	for (auto &it : skypenames) {
		JSONNode contact;
		contact << CHAR_PARAM("id", it);
		contacts << contact;
	}

	JSONNode node;
	node << contacts;

	auto *pReq = new AsyncHttpRequest(REQUEST_POST, HOST_DEFAULT, "/users/ME/contacts");
	pReq->m_szParam = node.write().c_str();
	PushRequest(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTeamsProto::GetProfileInfo(MCONTACT hContact)
{
	auto *pReq = new AsyncHttpRequest(REQUEST_GET, HOST_API, 0, &CTeamsProto::LoadProfile);
	pReq->m_szUrl.AppendFormat("/users/%s/profile", (hContact == 0) ? "self" : mir_urlEncode(getId(hContact)));
	pReq->pUserInfo = (void *)hContact;
	PushRequest(pReq);
}
