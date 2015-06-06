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

UINT_PTR CSkypeProto::m_timer;

void CSkypeProto::OnLoginOAuth(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
	{
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	JSONNode json = JSONNode::parse(response->pData);
	if (!json)
	{
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	if (response->resultCode != 200)
	{
		if (!json["status"].isnull())
		{
			const JSONNode &status = json["status"];
			if (!status["code"].isnull())
			{
				switch(status["code"].as_int())
				{
				case 40120:
					{
						ShowNotification(_T("Skype"), TranslateT("Authentication failed. Bad username or password."), NULL, 1);
						break;
					}
				case 40121:
					{
						ShowNotification(_T("Skype"), TranslateT("Too many failed authentication attempts with given username or IP."), NULL, 1);
						break;
					}
				default: 
					{
						ShowNotification(_T("Skype"), !status["text"].isnull() ? status["text"].as_mstring().GetBuffer() : TranslateT("Authentication failed. Unknown error."), NULL, 1);
					}
				}
			}
		}

		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	if (!json["skypetoken"] || !json["expiresIn"])
	{
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}
	std::string token = json["skypetoken"].as_string();
	setString("TokenSecret", token.c_str());

	int expiresIn = json["expiresIn"].as_int();
	setDword("TokenExpiresIn", time(NULL) + expiresIn);

	OnLoginSuccess();
}
void CSkypeProto::OnLoginSuccess()
{
	ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_SUCCESS, NULL, 0);
	replaceStr(SelfSkypeName, getStringA(SKYPE_SETTINGS_ID));
	replaceStr(TokenSecret, getStringA("TokenSecret"));
	replaceStr(Server, getStringA("Server") != NULL ? getStringA("Server") : SKYPE_ENDPOINTS_HOST);
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
			CMStringA szValue = response->headers[i].szValue;
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
			SendRequest(new LoginOAuthRequest(SelfSkypeName, ptrA(getStringA(SKYPE_SETTINGS_PASSWORD))), &CSkypeProto::OnLoginOAuth);
			return;
		}
		else //it should be rewritten
		{
			SendRequest(new CreateEndpointRequest(TokenSecret, Server), &CSkypeProto::OnEndpointCreated);
			return;
		}
	}

	replaceStr(RegToken, getStringA("registrationToken"));
	replaceStr(EndpointId, getStringA("endpointId"));
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
	SendPresence(true);
}

void CSkypeProto::SendPresence(bool isLogin)
{
	ptrA epname;
	ptrT place(getTStringA("Place"));

	if (!getBool("UseHostName", false) && place && *place)
		epname = mir_utf8encodeT(place);
	else
	{
		TCHAR compName[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD size = SIZEOF(compName);
		GetComputerName(compName, &size);
		epname = mir_utf8encodeT(compName);
	}
	if (isLogin)
		PushRequest(new SendCapabilitiesRequest(RegToken, EndpointId, epname, Server), &CSkypeProto::OnCapabilitiesSended);
	else 
		PushRequest(new SendCapabilitiesRequest(RegToken, EndpointId, epname, Server));
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
}
