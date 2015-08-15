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

#ifndef _SKYPE_REQUEST_CAPS_H_
#define _SKYPE_REQUEST_CAPS_H_

class SendCapabilitiesRequest : public HttpRequest
{
public:
	SendCapabilitiesRequest(const char *hostname, LoginInfo &li) :
	  HttpRequest(REQUEST_PUT, FORMAT, "%s/v1/users/ME/endpoints/%s/presenceDocs/messagingService", li.endpoint.szServer, ptrA(mir_urlEncode(li.endpoint.szId)))
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", li.endpoint.szToken);

		JSONNode privateInfo; privateInfo.set_name("privateInfo");
		privateInfo << JSONNode("epname", hostname);

		JSONNode publicInfo; publicInfo.set_name("publicInfo");
		publicInfo 
			<< JSONNode("capabilities", "Audio|Video")
			<< JSONNode("typ", 125)
			<< JSONNode("skypeNameVersion", "Miranda NG Skype")
			<< JSONNode("nodeInfo", "xx")
			<< JSONNode("version", g_szMirVer);

		JSONNode node;
		node
			<< JSONNode("id", "messagingService") 
			<< JSONNode("type", "EndpointPresenceDoc")
			<< JSONNode("selfLink", "uri")
			<< privateInfo 
			<< publicInfo;

		Body << VALUE(node.write().c_str());
	}
};
#endif //_SKYPE_REQUEST_CAPS_H_