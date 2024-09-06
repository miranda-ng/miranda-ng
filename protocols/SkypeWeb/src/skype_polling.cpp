/*
Copyright (c) 2015-24 Miranda NG team (https://miranda-ng.org)

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

void CSkypeProto::PollingThread(void *)
{
	debugLogA(__FUNCTION__ ": entering");

	int nErrors = 0;
	m_iPollingId = -1;

	while (true) {
		if (m_isTerminated || m_szId == nullptr)
			break;

		std::unique_ptr<PollRequest> request(new PollRequest(this));
		request->nlc = m_hPollingConn;
		NLHR_PTR response(DoSend(request.get()));
		if (m_isTerminated || m_szId == nullptr)
			break;

		if (response == nullptr || response->resultCode != 200) {
			if (nErrors < POLLING_ERRORS_LIMIT) {
				nErrors++;
				continue;
			}
			break;
		}

		m_hPollingConn = response->nlc;
		if (!response->body.IsEmpty())
			ParsePollData(response->body);
	}

	if (!m_isTerminated) {
		debugLogA(__FUNCTION__ ": unexpected termination; switching protocol to offline");
		SetStatus(ID_STATUS_OFFLINE);
	}

	m_hPollingConn = nullptr;
	m_hPollingThread = nullptr;
	debugLogA(__FUNCTION__ ": leaving");
}

void CSkypeProto::ParsePollData(const char *szData)
{
	debugLogA(__FUNCTION__);

	JSONNode data = JSONNode::parse(szData);
	if (!data)
		return;

	for (auto &message : data["eventMessages"]) {
		int eventId = message["id"].as_int();
		if (eventId > m_iPollingId)
			m_iPollingId = eventId;

		const JSONNode &resource = message["resource"];

		std::string resourceType = message["resourceType"].as_string();
		if (resourceType == "NewMessage")
			ProcessNewMessage(resource);
		else if (resourceType == "UserPresence")
			ProcessUserPresence(resource);
		else if (resourceType == "EndpointPresence")
			ProcessEndpointPresence(resource);
		else if (resourceType == "ConversationUpdate")
			ProcessConversationUpdate(resource);
		else if (resourceType == "ThreadUpdate")
			ProcessThreadUpdate(resource);
	}
}

void CSkypeProto::ProcessEndpointPresence(const JSONNode &node)
{
	debugLogA(__FUNCTION__);
	std::string selfLink = node["selfLink"].as_string();
	CMStringA skypename(UrlToSkypeId(selfLink.c_str()));

	MCONTACT hContact = FindContact(skypename);
	if (hContact == NULL)
		return;

	const JSONNode &publicInfo = node["publicInfo"];
	const JSONNode &privateInfo = node["privateInfo"];
	CMStringA MirVer;
	if (publicInfo) {
		std::string skypeNameVersion = publicInfo["skypeNameVersion"].as_string();
		std::string version = publicInfo["version"].as_string();
		std::string typ = publicInfo["typ"].as_string();
		int iTyp = atoi(typ.c_str());
		switch (iTyp) {
		case 0:
		case 1:
			MirVer.Append("Skype (Web) " + ParseUrl(version.c_str(), "/"));
			break;
		case 10:
			MirVer.Append("Skype (XBOX) " + ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 17:
			MirVer.Append("Skype (Android) " + ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 16:
			MirVer.Append("Skype (iOS) " + ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 12:
			MirVer.Append("Skype (WinRT) " + ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 15:
			MirVer.Append("Skype (WP) " + ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 13:
			MirVer.Append("Skype (OSX) " + ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 11:
			MirVer.Append("Skype (Windows) " + ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 14:
			MirVer.Append("Skype (Linux) " + ParseUrl(skypeNameVersion.c_str(), "/"));
			break;
		case 125:
			MirVer.AppendFormat("Miranda NG Skype %s", version.c_str());
			break;
		default:
			MirVer.Append("Skype (Unknown)");
		}
	}
	
	if (privateInfo != NULL) {
		std::string epname = privateInfo["epname"].as_string();
		if (!epname.empty())
			MirVer.AppendFormat(" [%s]", epname.c_str());
	}
	
	setString(hContact, "MirVer", MirVer);
}

void CSkypeProto::ProcessUserPresence(const JSONNode &node)
{
	debugLogA(__FUNCTION__);

	std::string selfLink = node["selfLink"].as_string();
	std::string status = node["status"].as_string();
	CMStringA skypename = UrlToSkypeId(selfLink.c_str());

	if (!skypename.IsEmpty()) {
		if (IsMe(skypename)) {
			int iNewStatus = SkypeToMirandaStatus(status.c_str());
			if (iNewStatus == ID_STATUS_OFFLINE) return;
			int old_status = m_iStatus;
			m_iDesiredStatus = iNewStatus;
			m_iStatus = iNewStatus;
			if (old_status != iNewStatus)
				ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, iNewStatus);
		}
		else {
			MCONTACT hContact = FindContact(skypename);
			if (hContact != NULL)
				SetContactStatus(hContact, SkypeToMirandaStatus(status.c_str()));
		}
	}
}

void CSkypeProto::ProcessConversationUpdate(const JSONNode &) {}
void CSkypeProto::ProcessThreadUpdate(const JSONNode &) {}
