#include "common.h"

int CSkypeProto::OnModulesLoaded(WPARAM, LPARAM)
{
	return 0;
}

void CSkypeProto::OnLoginFirst(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
	{
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, (HANDLE)NULL, 1001);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	std::regex regex;
	std::smatch match;

	const std::string content = response->pData;

	regex = "<input type=\"hidden\" name=\"pie\" id=\"pie\" value=\"(.+?)\"/>";
	if (!std::regex_search(content, match, regex))
	{
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}
	std::string pie = match[1];

	regex = "<input type=\"hidden\" name=\"etm\" id=\"etm\" value=\"(.+?)\"/>";
	if (!std::regex_search(content, match, regex))
	{
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, (HANDLE)NULL, 1001);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}
	std::string etm = match[1];

	ptrA skypename(mir_utf8encodeT(ptrT(getTStringA(SKYPE_SETTINGS_ID))));
	ptrA password(mir_utf8encodeT(ptrT(getTStringA(SKYPE_SETTINGS_PASSWORD))));

	PushRequest(new LoginRequest(skypename, password, pie.c_str(), etm.c_str()), &CSkypeProto::OnLoginSecond);
}

void CSkypeProto::OnLoginSecond(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
	{
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, (HANDLE)NULL, 1001);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	std::regex regex;
	std::smatch match;

	std::string content = response->pData;

	regex = "<input type=\"hidden\" name=\"skypetoken\" value=\"(.+?)\"/>";
	if (!std::regex_search(content, match, regex))
	{
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, (HANDLE)NULL, 1001);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}
	std::string token = match[1];
	setString("TokenSecret", token.c_str());

	regex = "<input type=\"hidden\" name=\"expires_in\" value=\"(.+?)\"/>";
	if (!std::regex_search(content, match, regex))
	{
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, (HANDLE)NULL, 1001);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}
	std::string expiresIn = match[1];
	int seconds = atoi(expiresIn.c_str());
	setDword("TokenExpiresIn", time(NULL) + seconds);

	for (int i = 0; i < response->headersCount; i++)
	{
		if (mir_strcmpi(response->headers[i].szName, "Set-Cookie"))
			continue;

		regex = "^(.+?)=(.+?);";
		content = response->headers[i].szValue;

		if (std::regex_search(content, match, regex))
			cookies[match[1]] = match[2];
	}

	PushRequest(new GetRegInfoRequest(token.c_str()), &CSkypeProto::OnGetRegInfo);
	PushRequest(new GetProfileRequest(token.c_str()), &CSkypeProto::LoadProfile);
	PushRequest(new GetContactListRequest(token.c_str()), &CSkypeProto::LoadContactList);

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus = m_iDesiredStatus);
}

void CSkypeProto::OnGetRegInfo(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;

	std::regex regex;
	std::smatch match;
	std::string content = response->pData;
	for (int i = 0; i < response->headersCount; i++)
	{
		if (mir_strcmpi(response->headers[i].szName, "Set-RegistrationToken") == 0)
		{
			regex = "^(.+?)=(.+?);";
			content = response->headers[i].szValue;
			if (std::regex_search(content, match, regex))
				RegInfo[match[1]] = match[2];
		}
		else if (mir_strcmpi(response->headers[i].szName, "Location") == 0)
		{
			content = response->headers[i].szValue;
			RegInfo["Location"] = content;
		}
		else
			continue;
	}
	setString("RegistrationToken", RegInfo["registrationToken"].c_str());
	setString("Endpoint", urlDecode(RegInfo["Location"].c_str()).c_str());

	debugLogA(getStringA("RegistrationToken"));
	debugLogA(getStringA("Endpoint"));
	CMStringA endpointURL = getStringA("Endpoint");
	endpointURL += "/presenceDocs/messagingService";
	PushRequest(new GetEndpointRequest(ptrA(getStringA("RegistrationToken")), endpointURL));
	PushRequest(new SetStatusRequest(ptrA(getStringA("RegistrationToken")), true));
}

void CSkypeProto::OnSetStatus(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;

	JSONROOT root(response->pData);
	if (root == NULL)
		return;
	JSONNODE *status_json = json_get(root, "status");
	debugLog(json_as_string(status_json));
	
	const char* status = (const char *)json_as_string(status_json);
	debugLogA(status);

	int old_status = m_iStatus;
	int iNewStatus;
	if (mir_strcmp(status, "O")==0)
	{
		iNewStatus = ID_STATUS_ONLINE;
	} 
	else if (mir_strcmp(status,"H")==0)
	{
		iNewStatus = ID_STATUS_INVISIBLE;
	}
	else 
	{
		iNewStatus = ID_STATUS_ONLINE;
	}
	m_iStatus = iNewStatus;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
}