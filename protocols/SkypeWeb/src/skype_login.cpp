/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

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

void CSkypeProto::OnLoginFirst(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
	{
		debugLogA(__FUNCTION__ ": failed to get login page");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	std::regex regex;
	std::smatch match;

	std::string content = response->pData;
	regex = "<input type=\"hidden\" name=\"pie\" id=\"pie\" value=\"(.+?)\"/>";
	if (!std::regex_search(content, match, regex))
	{
		debugLogA(__FUNCTION__ ": failed to get pie");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
	}
	std::string pie = match[1];

	regex = "<input type=\"hidden\" name=\"etm\" id=\"etm\" value=\"(.+?)\"/>";
	if (!std::regex_search(content, match, regex))
	{
		debugLogA(__FUNCTION__ ": failed to get etm");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}
	std::string etm = match[1];

	ptrA skypename(getStringA(SKYPE_SETTINGS_ID));
	ptrA password(getStringA(SKYPE_SETTINGS_PASSWORD));
	SendRequest(new LoginRequest(skypename, password, pie.c_str(), etm.c_str()), &CSkypeProto::OnLoginSecond);
}

void CSkypeProto::OnLoginSecond(const NETLIBHTTPREQUEST *response)
{
	m_iStatus++;

	if (response == NULL)
	{
		debugLogA(__FUNCTION__ ": failed to login");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	std::regex regex;
	std::smatch match;

	std::string content = response->pData;
	regex = "<input type=\"hidden\" name=\"skypetoken\" value=\"(.+?)\"/>";
	if (!std::regex_search(content, match, regex))
	{
		debugLogA(__FUNCTION__ ": failed to get skype token");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}
	std::string token = match[1];
	setString("TokenSecret", token.c_str());

	regex = "<input type=\"hidden\" name=\"expires_in\" value=\"(.+?)\"/>";
	if (std::regex_search(content, match, regex))
	{
		std::string expiresIn = match[1];
		int seconds = atoi(expiresIn.c_str());
		setDword("TokenExpiresIn", time(NULL) + seconds);
	}

	for (int i = 0; i < response->headersCount; i++)
	{
		if (mir_strcmpi(response->headers[i].szName, "Set-Cookie"))
			continue;

		regex = "^(.+?)=(.+?);";
		content = response->headers[i].szValue;
		if (std::regex_search(content, match, regex))
			cookies[match[1]] = match[2];
	}
	OnLoginSuccess();
}

void CSkypeProto::OnLoginSuccess()
{
	SelfSkypeName = getStringA(SKYPE_SETTINGS_ID);
	TokenSecret = getStringA("TokenSecret");
	Server = getStringA("Server") != NULL ? getStringA("Server") : SKYPE_ENDPOINTS_HOST;
	SendRequest(new CreateEndpointRequest(TokenSecret, Server), &CSkypeProto::OnEndpointCreated);
	PushRequest(new GetProfileRequest(TokenSecret), &CSkypeProto::LoadProfile);

	if (!m_timer)
		SkypeSetTimer(this);
}

void CSkypeProto::OnEndpointCreated(const NETLIBHTTPREQUEST *response)
{
	m_iStatus++;

	if (response == NULL)
	{
		debugLogA(__FUNCTION__ ": failed to get create endpoint");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	for (int i = 0; i < response->headersCount; i++)
	{
		if (!mir_strcmpi(response->headers[i].szName, "Set-RegistrationToken"))
		{
			CMStringA szValue = response->headers[i].szValue, szCookieName, szCookieVal;
			int iStart = 0;
			while (true)
			{
				CMStringA szToken = szValue.Tokenize(";", iStart).Trim();
				if (iStart == -1)
					break;
				int iStart2 = 0;
				szCookieName = szToken.Tokenize("=", iStart2);
				szCookieVal = szToken.Mid(iStart2);
				setString(szCookieName, szCookieVal);
			}
		}
		else if (!mir_strcmpi(response->headers[i].szName, "Location"))
		{
			CMStringA szValue = response->headers[i].szValue, szCookieName, szCookieVal;
			Server = GetServerFromUrl(szValue);
			setString("Server", Server);
		}

	}

	if (m_iStatus++ > SKYPE_MAX_CONNECT_RETRIES)
	{
		debugLogA(__FUNCTION__ ": failed to get create endpoint");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	if (response->resultCode != 201)
	{
		if (response->resultCode == 401)
		{
			delSetting("TokenExpiresIn");
			SendRequest(new LoginRequest(), &CSkypeProto::OnLoginFirst);
			return;
		}
		else //it should be rewritten
		{
			SendRequest(new CreateEndpointRequest(TokenSecret, Server), &CSkypeProto::OnEndpointCreated);
			return;
		}
	}

	RegToken = getStringA("registrationToken");
	EndpointId = getStringA("endpointId");
	SendRequest(new CreateSubscriptionsRequest(RegToken, Server), &CSkypeProto::OnSubscriptionsCreated);
	SendRequest(new CreateTrouterRequest(), &CSkypeProto::OnCreateTrouter);
}

void CSkypeProto::OnSubscriptionsCreated(const NETLIBHTTPREQUEST *response)
{
	m_iStatus++;

	if (response == NULL)
	{
		debugLogA(__FUNCTION__ ": failed to create subscription");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	PushRequest(new SendCapabilitiesRequest(RegToken, EndpointId, Server), &CSkypeProto::OnCapabilitiesSended);
}

void CSkypeProto::OnCapabilitiesSended(const NETLIBHTTPREQUEST *response)
{
	SendRequest(new SetStatusRequest(RegToken, MirandaToSkypeStatus(m_iDesiredStatus), Server), &CSkypeProto::OnStatusChanged);

	LIST<char> skypenames(1);
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		if (!isChatRoom(hContact))
			skypenames.insert(getStringA(hContact, SKYPE_SETTINGS_ID));
	}
	SendRequest(new CreateContactsSubscriptionRequest(RegToken, skypenames, Server));
	for (int i = 0; i < skypenames.getCount(); i++)
		mir_free(skypenames[i]);
	skypenames.destroy();

	m_hPollingThread = ForkThreadEx(&CSkypeProto::PollingThread, 0, NULL);

	PushRequest(new GetAvatarRequest(ptrA(getStringA("AvatarUrl"))), &CSkypeProto::OnReceiveAvatar, NULL);
	PushRequest(new GetContactListRequest(TokenSecret), &CSkypeProto::LoadContactList);

	SendRequest(new LoadChatsRequest(RegToken, Server), &CSkypeProto::OnLoadChats);
	if (getBool("AutoSync", true))
		PushRequest(new SyncHistoryFirstRequest(RegToken, 100, Server), &CSkypeProto::OnSyncHistory);

	if (response == NULL || response->pData == NULL)
		return;

	JSONNode root = JSONNode::parse(response->pData);
	if (root)
		setString("SelfEndpointName", ptrA(SelfUrlToName(root["selfLink"].as_string().c_str())));
}

void CSkypeProto::OnStatusChanged(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
	{
		debugLogA(__FUNCTION__ ": failed to change status");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	JSONNode json = JSONNode::parse(response->pData);
	if (!json)
	{
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
	if (iNewStatus == ID_STATUS_OFFLINE)
	{
		debugLogA(__FUNCTION__ ": failed to change status");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	int oldStatus = m_iStatus;
	m_iStatus = m_iDesiredStatus = iNewStatus;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
	ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_SUCCESS, NULL, 0);
}
