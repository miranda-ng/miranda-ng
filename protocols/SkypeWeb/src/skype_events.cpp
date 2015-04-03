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

	PushRequest(new GetRegInfoRequest(token.c_str(), getStringA("Server")), &CSkypeProto::OnGetRegInfo);
	PushRequest(new GetProfileRequest(token.c_str()), &CSkypeProto::LoadProfile);
	PushRequest(new GetContactListRequest(token.c_str()), &CSkypeProto::LoadContactList);
}

void CSkypeProto::OnGetRegInfo(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
		return;

	std::regex regex;
	std::smatch match;
	std::string content = response->pData;
	for (int i = 0; i < response->headersCount; i++) {
		if (!mir_strcmpi(response->headers[i].szName, "Set-RegistrationToken"))
		{
			CMStringA szValue = response->headers[i].szValue, szCookieName, szCookieVal;
			int iStart = 0;
			while (true) {
				bool bFirstToken = (iStart == 0);
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
			setString("Server", GetServerFromUrl(szValue));
		}

	}
	if (response->resultCode != 201)
	{
		PushRequest(new GetRegInfoRequest(getStringA("Token"), getStringA("Server")), &CSkypeProto::OnGetRegInfo);
		return;
	}

	PushRequest(new GetEndpointRequest(getStringA("registrationToken"), getStringA("endpointId"), getStringA("Server")), &CSkypeProto::OnGetEndpoint);
	isTerminated = false;
	m_hPollingThread = ForkThreadEx(&CSkypeProto::PollingThread, 0, NULL);

	m_iStatus = m_iDesiredStatus;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus);
	PushRequest(new SetStatusRequest(ptrA(getStringA("registrationToken")), MirandaToSkypeStatus(m_iStatus), getStringA("Server")), &CSkypeProto::OnSetStatus);
}

void CSkypeProto::OnGetEndpoint(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;

	JSONROOT root(response->pData);

	if (root == NULL)
		return;

	ptrA selfLink(mir_t2a(ptrT(json_as_string(json_get(root, "selfLink")))));

	setString("SelfEndpointName", SelfUrlToName(selfLink));
}

void CSkypeProto::OnSetStatus(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;

	JSONROOT root(response->pData);

	if (root == NULL)
		return;

	JSONNODE *status_json = json_get(root, "status");
	ptrA status(mir_t2a(ptrT(json_as_string(status_json))));
	int old_status = m_iStatus;
	int iNewStatus = SkypeToMirandaStatus(status);
	m_iStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE)
		SetStatus(ID_STATUS_OFFLINE);
	else 
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
}