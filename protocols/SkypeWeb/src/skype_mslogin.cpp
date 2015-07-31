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

void CSkypeProto::OnMSLoginFirst(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
	{
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	std::regex regex;
	std::smatch match;
	std::map<std::string, std::string> scookies;
	std::string content = response->pData;

	regex = "<input type=\"hidden\" name=\"PPFT\" id=\"i0327\" value=\"(.+?)\"/>";

	if (!std::regex_search(content, match, regex))
	{
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}
	std::string PPTF = match[1];
	
	for (int i = 0; i < response->headersCount; i++)
	{
		if (mir_strcmpi(response->headers[i].szName, "Set-Cookie"))
			continue;

		regex = "^(.+?)=(.+?);";
		content = response->headers[i].szValue;
		if (std::regex_search(content, match, regex))
			scookies[match[1]] = match[2];
	}

	CMStringA mscookies(FORMAT, "MSPRequ=%s;MSPOK=%s;CkTst=G%lld;", scookies["MSPRequ"].c_str(), scookies["MSPOK"].c_str(), time(NULL));

	SendRequest (new LoginMSRequest(ptrA(getStringA(SKYPE_SETTINGS_ID)), ptrA(getStringA(SKYPE_SETTINGS_PASSWORD)), mscookies.c_str(), PPTF.c_str()), &CSkypeProto::OnMSLoginSecond);
}

void CSkypeProto::OnMSLoginSecond(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
	{
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	std::regex regex;
	std::smatch match;
	std::string content = response->pData;
	ptrA szContent(response->pData);

	regex = "<meta name=\"PageID\" content=\"(.+?)\"/>";
	if (std::regex_search(content, match, regex))
	{
		if (match[1] == "i5600")
		{
			CMStringA szCookies;
			for (int i = 0; i < response->headersCount; i++)
			{
				if (mir_strcmpi(response->headers[i].szName, "Set-Cookie"))
					continue;

				regex = "^(.+?)=(.+?);";
				content = response->headers[i].szValue;
				if (std::regex_search(content, match, regex))
					if (!std::string(match[2]).empty() && std::string(match[2]) != " ")
						szCookies.AppendFormat("%s=%s;", std::string(match[1]).c_str(), std::string(match[2]).c_str());
			}

			CMStringA url(GetStringChunk(szContent, "urlPost:'", "'"));
			CMStringA ppft(GetStringChunk(szContent, "sFT:'", "'"));
			

			ptrA code(mir_utf8encodeT(RunConfirmationCode()));

			SendRequest(new LoginMSRequest(url.c_str(), ptrA(getStringA(SKYPE_SETTINGS_ID)), szCookies.c_str(), ppft.c_str(), code), &CSkypeProto::OnMSLoginEnd);
			return;
		}
	}


	regex = "<input type=\"hidden\" name=\"t\" id=\"t\" value=\"(.+?)\">";

	if (!std::regex_search(content, match, regex))
	{
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}
	std::string t = match[1];

	SendRequest(new LoginMSRequest(t.c_str(), 0), &CSkypeProto::OnMSLoginThird);
}

void CSkypeProto::OnMSLoginThird(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
	{
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	std::regex regex;
	std::smatch match;
	std::string content = response->pData;

	regex = "<input type=\"hidden\" name=\"t\" value=\"(.+?)\"/>";

	if (!std::regex_search(content, match, regex))
	{
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}
	std::string t = match[1];

	SendRequest(new LoginMSRequest(t.c_str()), &CSkypeProto::OnMSLoginEnd);
}

void CSkypeProto::OnMSLoginEnd(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
	{
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

	OnLoginSuccess();
}

CMString CSkypeProto::RunConfirmationCode()
{
	CMString caption(FORMAT, _T("[%s] %s"), _A2T(m_szModuleName), TranslateT("Enter confirmation code"));
	ENTER_STRING pForm = { sizeof(pForm) };
	pForm.type = ESF_PASSWORD;
	pForm.caption = caption;
	pForm.ptszInitVal = NULL;
	pForm.szModuleName = m_szModuleName;
	return (!EnterString(&pForm)) ? CMString() : CMString(ptrT(pForm.ptszResult));
}