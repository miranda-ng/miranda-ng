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

#include "common.h"

int CSkypeProto::OnModulesLoaded(WPARAM, LPARAM) { return 0; }

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
	PushRequest(new LoginRequest(skypename, password, pie.c_str(), etm.c_str()), &CSkypeProto::OnLoginSecond);
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
	PushRequest(new CreateEndpointRequest(ptrA(getStringA("TokenSecret"))), &CSkypeProto::OnEndpointCreated);
	PushRequest(new GetProfileRequest(ptrA(getStringA("TokenSecret"))), &CSkypeProto::LoadProfile);
	PushRequest(new GetAvatarRequest(ptrA(getStringA("AvatarUrl"))), &CSkypeProto::OnReceiveAvatar, NULL);
	PushRequest(new GetContactListRequest(ptrA(getStringA("TokenSecret"))), &CSkypeProto::LoadContactList);
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
			setString("Server", ptrA(GetServerFromUrl(szValue)));
		}

	}

	if (m_iStatus++ > SKYPE_MAX_CONNECT_RETRIES)
	{
		debugLogA(__FUNCTION__ ": failed to get create endpoint");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	ptrA server(getStringA("Server"));
	if (response->resultCode != 201)
	{
		ptrA token(getStringA("TokenSecret"));
		PushRequest(new CreateEndpointRequest(token, server), &CSkypeProto::OnEndpointCreated);
		return;
	}

	ptrA regToken(getStringA("registrationToken"));
	PushRequest(new CreateSubscriptionsRequest(regToken, server), &CSkypeProto::OnSubscriptionsCreated);
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

	ptrA regToken(getStringA("registrationToken"));
	ptrA skypename(getStringA(SKYPE_SETTINGS_ID));
	ptrA endpoint(getStringA("endpointId"));
	ptrA server(getStringA("Server"));
	PushRequest(new SendCapabilitiesRequest(regToken, endpoint, server));
	PushRequest(new SetStatusRequest(regToken, MirandaToSkypeStatus(m_iDesiredStatus), server), &CSkypeProto::OnStatusChanged);

	LIST<char> skypenames(1);
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
		skypenames.insert(getStringA(hContact, SKYPE_SETTINGS_ID));
	PushRequest(new CreateContactsRequest(regToken, skypenames, server));
	for (int i = 0; i < skypenames.getCount(); i++)
		mir_free(skypenames[i]);
	skypenames.destroy();

	m_hPollingThread = ForkThreadEx(&CSkypeProto::PollingThread, 0, NULL);

	if (getByte("AutoSync", 1))
		SyncHistory();
}

void CSkypeProto::OnStatusChanged(const NETLIBHTTPREQUEST *response)
{
	m_iStatus++;

	if (response == NULL)
	{
		debugLogA(__FUNCTION__ ": failed to change status");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}
	
	JSONROOT json(response->pData);
	if (json == NULL)
	{
		debugLogA(__FUNCTION__ ": failed to change status");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	ptrT status(json_as_string(json_get(json, "status")));
	int iNewStatus = SkypeToMirandaStatus(_T2A(status));
	if (iNewStatus == ID_STATUS_OFFLINE)
	{
		debugLogA(__FUNCTION__ ": failed to change status");
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	m_iStatus = m_iDesiredStatus = iNewStatus;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus);
	ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_SUCCESS, NULL, 0);
}