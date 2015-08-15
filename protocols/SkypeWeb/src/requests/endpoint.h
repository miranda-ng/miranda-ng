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

#ifndef _SKYPE_REQUEST_ENDPOINT_H_
#define _SKYPE_REQUEST_ENDPOINT_H_

class CreateEndpointRequest : public HttpRequest
{
public:
	CreateEndpointRequest(LoginInfo &li) :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/endpoints", li.endpoint.szServer)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< FORMAT_VALUE("Authentication", "skypetoken=%s", li.api.szToken);

		Body << VALUE("{}");
	}
};

class DeleteEndpointRequest : public HttpRequest
{
public:
	DeleteEndpointRequest(LoginInfo &li) :
	  HttpRequest(REQUEST_DELETE, FORMAT, "%s/v1/users/ME/endpoints/%s", li.endpoint.szServer, ptrA(mir_urlEncode(li.endpoint.szId)))
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", li.endpoint.szToken);
	}
};


#endif //_SKYPE_REQUEST_ENDPOINT_H_
