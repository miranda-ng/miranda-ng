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

#ifndef _SKYPE_REQUEST_ENDPOINT_H_
#define _SKYPE_REQUEST_ENDPOINT_H_

class CreateEndpointRequest : public HttpRequest
{
public:
	CreateEndpointRequest(CSkypeProto *ppro) :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/endpoints", ppro->m_szServer)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< FORMAT_VALUE("Authentication", "skypetoken=%s", ppro->m_szApiToken.get());

		Body << VALUE("{}");
	}
};

class DeleteEndpointRequest : public HttpRequest
{
public:
	DeleteEndpointRequest(CSkypeProto *ppro) :
	  HttpRequest(REQUEST_DELETE, FORMAT, "%s/v1/users/ME/endpoints/%s", ppro->m_szServer, mir_urlEncode(ppro->m_szId).c_str())
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", ppro->m_szToken.get());
	}
};


#endif //_SKYPE_REQUEST_ENDPOINT_H_
