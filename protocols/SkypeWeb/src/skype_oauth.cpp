/*
Copyright (c) 2015-22 Miranda NG team (https://miranda-ng.org)

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

static std::string sub(const std::string &str, const char *start, const char *end)
{
	size_t i1 = str.find(start);
	if (i1 == -1)
		return "";

	i1 += strlen(start);
	size_t i2 = str.find(end, i1);
	return (i2 == -1) ? "" : str.substr(i1, i2 - i1);
}

void CSkypeProto::OnOAuthStart(NETLIBHTTPREQUEST *response, AsyncHttpRequest*)
{
	if (response == nullptr || response->pData == nullptr) {
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	std::regex regex;
	std::smatch match;
	std::string content = response->pData;

	regex = "<input.+?type=\"hidden\".+?name=\"PPFT\".+?id=\"i0327\".+?value=\"(.+?)\".*?/>";

	if (!std::regex_search(content, match, regex)) {
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}
	std::string PPFT = match[1];

	std::map<std::string, std::string> scookies;
	regex = "^(.+?)=(.*?);";

	for (int i = 0; i < response->headersCount; i++) {
		if (mir_strcmpi(response->headers[i].szName, "Set-Cookie"))
			continue;

		content = response->headers[i].szValue;
		if (std::regex_search(content, match, regex))
			scookies[match[1]] = match[2];
	}

	ptrA login(getStringA(SKYPE_SETTINGS_ID));
	ptrA password(getStringA(SKYPE_SETTINGS_PASSWORD));
	CMStringA mscookies(FORMAT, "MSPRequ=%s;MSPOK=%s;CkTst=G%lld;", scookies["MSPRequ"].c_str(), scookies["MSPOK"].c_str(), time(NULL));

	cookies["MSPRequ"] = scookies["MSPRequ"];

	PushRequest(new OAuthRequest(login, password, mscookies.c_str(), PPFT.c_str()));
}

bool CSkypeProto::CheckOauth(const char *szResponse)
{
	std::string content = szResponse;
	std::smatch match;
	if (!std::regex_search(content, match, std::regex("<input.+?type=\"hidden\".+?name=\"t\".+?id=\"t\".+?value=\"(.+?)\".*?>")))
		if (!std::regex_search(content, match, std::regex("<input.+?type=\"hidden\".+?name=\"ipt\".+?id=\"ipt\".+?value=\"(.+?)\".*?>")))
			return false;

	std::string t = match[1];
	PushRequest(new OAuthRequest(t.c_str()));
	return true;
}

void CSkypeProto::OnOAuthConfirm(NETLIBHTTPREQUEST *response, AsyncHttpRequest *)
{
	if (response == nullptr || response->pData == nullptr) {
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	if (CheckOauth(response->pData))
		return;

	std::string content = response->pData;
	std::string PPFT = sub(content, "sFT:'", "'");
	std::string opid = sub(content, "opid=", "&");
	if (PPFT.empty() || opid.empty()) {
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	std::regex regex("^(.+?=.*?;)");
	std::smatch match;
	CMStringA mscookies;

	for (int i = 0; i < response->headersCount; i++) {
		if (mir_strcmpi(response->headers[i].szName, "Set-Cookie"))
			continue;

		content = response->headers[i].szValue;
		if (std::regex_search(content, match, regex))
			mscookies.Append(match[1].str().c_str());
	}

	PushRequest(new OAuthRequest(mscookies.c_str(), PPFT.c_str(), opid.c_str()));
}

void CSkypeProto::OnOAuthAuthorize(NETLIBHTTPREQUEST *response, AsyncHttpRequest*)
{
	if (response == nullptr || response->pData == nullptr) {
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	if (!CheckOauth(response->pData)) {
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
	}
}

void CSkypeProto::OnOAuthEnd(NETLIBHTTPREQUEST *response, AsyncHttpRequest*)
{
	if (response == nullptr || response->pData == nullptr) {
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	std::regex regex;
	std::smatch match;
	std::string content = response->pData;

	regex = "<input.+?type=\"hidden\".+?name=\"skypetoken\".+?value=\"(.+?)\".*?/>";
	if (!std::regex_search(content, match, regex)) {
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGIN_ERROR_UNKNOWN);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}
	std::string token = match[1];
	setString("TokenSecret", token.c_str());
	regex = "<input.+?type=\"hidden\".+?name=\"expires_in\".+?value=\"(.+?)\".*?/>";

	if (std::regex_search(content, match, regex)) {
		std::string expiresIn = match[1];
		int seconds = atoi(expiresIn.c_str());
		setDword("TokenExpiresIn", time(NULL) + seconds);
	}

	OnLoginSuccess();
}
