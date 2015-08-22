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

void CSkypeProto::Login(void*)
{
	// login
	m_iStatus = ID_STATUS_CONNECTING;
	requestQueue->Start();
	int tokenExpires(getDword("TokenExpiresIn", 0));

	li.szSkypename = getStringA(SKYPE_SETTINGS_ID);

	pass_ptrA szPassword(getStringA(SKYPE_SETTINGS_PASSWORD));
	if (li.szSkypename == NULL || szPassword == NULL)
	{
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		return;
	}	

	mir_cslock lck(m_LoginLock);

	if (m_iStatus > ID_STATUS_OFFLINE) return;

	HistorySynced = isTerminated = false;
	if ((tokenExpires - 1800) > time(NULL))
		OnLoginSuccess();
	else
	{
		if (strchr(li.szSkypename, '@'))
			SendRequest(new LoginMSRequest(), &CSkypeProto::OnMSLoginFirst);
		else
			SendRequest(new LoginOAuthRequest(li.szSkypename, szPassword), &CSkypeProto::OnLoginOAuth);
	}

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_CONNECTING);

	WaitForSingleObject(m_hLoginEvent, INFINITE);
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
		if (json["status"])
		{
			const JSONNode &status = json["status"];
			if (status["code"])
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
						ShowNotification(_T("Skype"), status["text"] ? status["text"].as_mstring() : TranslateT("Authentication failed. Unknown error."), NULL, 1);
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

	li.api.szToken = getStringA("TokenSecret");

	li.endpoint.szServer = ((ptrA(getStringA("Server")) == NULL) ? mir_strdup(SKYPE_ENDPOINTS_HOST) : getStringA("Server"));

	SendRequest(new CreateEndpointRequest(li), &CSkypeProto::OnEndpointCreated);
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
			li.endpoint.szServer = GetServerFromUrl(szValue).Detach();
			setString("Server", li.endpoint.szServer);
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
			SendRequest(new LoginOAuthRequest(li.szSkypename, ptrA(getStringA(SKYPE_SETTINGS_PASSWORD))), &CSkypeProto::OnLoginOAuth);
			return;
		}
		else //it should be rewritten
		{
			SendRequest(new CreateEndpointRequest(li), &CSkypeProto::OnEndpointCreated);
			return;
		}
	}

	li.endpoint.szToken = getStringA("registrationToken");
	li.endpoint.szId = getStringA("endpointId");

	SendRequest(new CreateSubscriptionsRequest(li), &CSkypeProto::OnSubscriptionsCreated);
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
		SendRequest(new SendCapabilitiesRequest(epname, li), &CSkypeProto::OnCapabilitiesSended);
	else 
		PushRequest(new SendCapabilitiesRequest(epname, li));
}

void CSkypeProto::OnCapabilitiesSended(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
	{
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	SendRequest(new SetStatusRequest(MirandaToSkypeStatus(m_iDesiredStatus), li), &CSkypeProto::OnStatusChanged);

	LIST<char> skypenames(1);
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		if (!isChatRoom(hContact))
			skypenames.insert(getStringA(hContact, SKYPE_SETTINGS_ID));
	}
	SendRequest(new CreateContactsSubscriptionRequest(skypenames, li));
	FreeList(skypenames);
	skypenames.destroy();

	m_hPollingThread = ForkThreadEx(&CSkypeProto::PollingThread, 0, NULL);

	SetEvent(m_hLoginEvent);

	SendRequest(new LoadChatsRequest(li), &CSkypeProto::OnLoadChats);
	SendRequest(new CreateTrouterRequest(), &CSkypeProto::OnCreateTrouter);
	PushRequest(new GetContactListRequest(li, NULL), &CSkypeProto::LoadContactList);
	PushRequest(new GetAvatarRequest(ptrA(getStringA("AvatarUrl"))), &CSkypeProto::OnReceiveAvatar, NULL);
	
	if (getBool("AutoSync", true))
		PushRequest(new SyncHistoryFirstRequest(100, li), &CSkypeProto::OnSyncHistory);

	JSONNode root = JSONNode::parse(response->pData);
	if (root)
		setString("SelfEndpointName", UrlToSkypename(root["selfLink"].as_string().c_str()));

	PushRequest(new GetProfileRequest(li), &CSkypeProto::LoadProfile);
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
