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

void CSkypeProto::Login()
{
	// login
	m_iStatus = ID_STATUS_CONNECTING;
	requestQueue->Start();
	int tokenExpires(getDword("TokenExpiresIn", 0));
	ptrA login(getStringA(SKYPE_SETTINGS_ID));
	ptrA password(getStringA(SKYPE_SETTINGS_PASSWORD));
	if (login == NULL || password == NULL)
	{
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		return;
	}	
	HistorySynced = isTerminated = false;
	if ((tokenExpires - 1800) > time(NULL))
		OnLoginSuccess();
	else
	{
		if (strstr(login, "@"))
			SendRequest(new LoginMSRequest(), &CSkypeProto::OnMSLoginFirst);
		else
			SendRequest(new LoginOAuthRequest(login, password), &CSkypeProto::OnLoginOAuth);
	}
}

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
		int error = 0;
		if (!json["status"].isnull())
		{
			const JSONNode &status = json["status"];
			if (!status["code"].isnull())
			{
				switch(status["code"].as_int())
				{
				case 40002:
					{
						ShowNotification(_T("Skype"), TranslateT("Authentication failed. Invalid username."), NULL, 1);
						error = LOGINERR_BADUSERID;
						break;
					}
				case 40120:
					{
						ShowNotification(_T("Skype"), TranslateT("Authentication failed. Bad username or password."), NULL, 1);
						error = LOGINERR_WRONGPASSWORD;
						break;
					}
				case 40121:
					{
						ShowNotification(_T("Skype"), TranslateT("Too many failed authentication attempts with given username or IP."), NULL, 1);
						error = LOGIN_ERROR_TOOMANY_REQUESTS;
						break;
					}
				default: 
					{
						ShowNotification(_T("Skype"), !status["text"].isnull() ? status["text"].as_mstring().GetBuffer() : TranslateT("Authentication failed. Unknown error."), NULL, 1);
						error = LOGIN_ERROR_UNKNOWN;
					}
				}
			}
		}

		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, error);
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
	isTerminated = false;
	ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_SUCCESS, NULL, 0);
	m_szSelfSkypeName = getStringA(SKYPE_SETTINGS_ID);
	m_szTokenSecret = getStringA("TokenSecret");
	m_szServer = getStringA("Server");
	if (m_szServer == NULL)
		m_szServer = mir_strdup(SKYPE_ENDPOINTS_HOST);
	SendRequest(new CreateEndpointRequest(m_szTokenSecret, m_szServer), &CSkypeProto::OnEndpointCreated);
	PushRequest(new GetProfileRequest(m_szTokenSecret), &CSkypeProto::LoadProfile);
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
			m_szServer = GetServerFromUrl(szValue).Detach();
			setString("Server", m_szServer);
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
			SendRequest(new LoginOAuthRequest(m_szSelfSkypeName, ptrA(getStringA(SKYPE_SETTINGS_PASSWORD))), &CSkypeProto::OnLoginOAuth);
			return;
		}
		else //it should be rewritten
		{
			SendRequest(new CreateEndpointRequest(m_szTokenSecret, m_szServer), &CSkypeProto::OnEndpointCreated);
			return;
		}
	}

	m_szRegToken = getStringA("registrationToken");
	m_szEndpointId = getStringA("endpointId");
	SendRequest(new CreateSubscriptionsRequest(m_szRegToken, m_szServer), &CSkypeProto::OnSubscriptionsCreated);
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
		DWORD size = _countof(compName);
		GetComputerName(compName, &size);
		epname = mir_utf8encodeT(compName);
	}

	if (isLogin)
		SendRequest(new SendCapabilitiesRequest(m_szRegToken, m_szEndpointId, epname, m_szServer), &CSkypeProto::OnCapabilitiesSended);
	else 
		PushRequest(new SendCapabilitiesRequest(m_szRegToken, m_szEndpointId, epname, m_szServer));
}

void CSkypeProto::OnCapabilitiesSended(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
	{
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	SendRequest(new SetStatusRequest(m_szRegToken, MirandaToSkypeStatus(m_iDesiredStatus), m_szServer), &CSkypeProto::OnStatusChanged);

	LIST<char> skypenames(1);
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		if (!isChatRoom(hContact))
			skypenames.insert(getStringA(hContact, SKYPE_SETTINGS_ID));
	}
	SendRequest(new CreateContactsSubscriptionRequest(m_szRegToken, skypenames, m_szServer));
	for (int i = 0; i < skypenames.getCount(); i++)
		mir_free(skypenames[i]);
	skypenames.destroy();


	m_hPollingThread = ForkThreadEx(&CSkypeProto::PollingThread, 0, NULL);

	PushRequest(new GetAvatarRequest(ptrA(getStringA("AvatarUrl"))), &CSkypeProto::OnReceiveAvatar, NULL);
	PushRequest(new GetContactListRequest(m_szTokenSecret, m_szSelfSkypeName, NULL), &CSkypeProto::LoadContactList);

	SendRequest(new LoadChatsRequest(m_szRegToken, m_szServer), &CSkypeProto::OnLoadChats);
	if (getBool("AutoSync", true))
		PushRequest(new SyncHistoryFirstRequest(m_szRegToken, 100, m_szServer), &CSkypeProto::OnSyncHistory);

	JSONNode root = JSONNode::parse(response->pData);
	if (root)
		setString("SelfEndpointName", UrlToSkypename(root["selfLink"].as_string().c_str()));
}

void CSkypeProto::OnStatusChanged(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
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
