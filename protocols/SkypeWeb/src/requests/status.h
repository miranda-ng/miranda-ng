/*
Copyright (c) 2015-20 Miranda NG team (https://miranda-ng.org)

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

#ifndef _SKYPE_REQUEST_STATUS_H_
#define _SKYPE_REQUEST_STATUS_H_

class GetStatusRequest : public HttpRequest
{
public:
	GetStatusRequest(CSkypeProto *ppro) :
		HttpRequest(REQUEST_GET, FORMAT, "%s/v1/users/ME/contacts/ALL/presenceDocs/messagingService", ppro->m_szServer)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", ppro->m_szToken.get())
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");
	}
};

class SetStatusRequest : public HttpRequest
{
public:
	SetStatusRequest(const char *status, CSkypeProto *ppro) :
	  HttpRequest(REQUEST_PUT, FORMAT, "%s/v1/users/ME/presenceDocs/messagingService", ppro->m_szServer)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", ppro->m_szToken.get())
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");

		JSONNode node(JSON_NODE);
		node << JSONNode("status", status);

		Body << VALUE(node.write().c_str());
	}
};

class SetStatusMsgRequest : public HttpRequest
{
public:
	SetStatusMsgRequest(const char *status, CSkypeProto *ppro) :
		HttpRequest(REQUEST_POST, "api.skype.com/users/self/profile/partial")
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("X-Skypetoken", ppro->m_szApiToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");

		JSONNode node, payload;
		payload.set_name("payload");
		node << (payload << JSONNode("mood", status));


		Body << VALUE(node.write().c_str());
	}
};

#endif //_SKYPE_REQUEST_STATUS_H_
