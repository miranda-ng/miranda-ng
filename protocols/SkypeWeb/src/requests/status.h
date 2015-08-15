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

#ifndef _SKYPE_REQUEST_STATUS_H_
#define _SKYPE_REQUEST_STATUS_H_

class SetStatusRequest : public HttpRequest
{
public:
	SetStatusRequest(const char *status, LoginInfo &li) :
	  HttpRequest(REQUEST_PUT, FORMAT, "%s/v1/users/ME/presenceDocs/messagingService", li.endpoint.szServer)
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", li.endpoint.szToken)
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8");

		JSONNode node(JSON_NODE);
		node << JSONNode("status", status);

		Body << VALUE(node.write().c_str());
	}
};

#endif //_SKYPE_REQUEST_STATUS_H_
