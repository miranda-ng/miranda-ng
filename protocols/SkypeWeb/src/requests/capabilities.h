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
	SendCapabilitiesRequest(const char *regToken, const char *endpointID, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_PUT, FORMAT, "%s/v1/users/ME/endpoints/%s/presenceDocs/messagingService", server, ptrA(mir_urlEncode(endpointID)))
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken);


		TCHAR compName[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD size = SIZEOF(compName);
		GetComputerName(compName, &size);

		JSONNode privateInfo(JSON_NODE);
		privateInfo.set_name("privateInfo");
		privateInfo.push_back(JSONNode("epname", T2Utf(compName)));

		JSONNode publicInfo(JSON_NODE);
		publicInfo.set_name("publicInfo");
		publicInfo.push_back(JSONNode("capabilities", "Audio|Video"));
		publicInfo.push_back(JSONNode("typ", 125));
		publicInfo.push_back(JSONNode("skypeNameVersion", "Miranda NG Skype"));
		publicInfo.push_back(JSONNode("nodeInfo", "xx"));
		publicInfo.push_back(JSONNode("version", g_szMirVer));

		JSONNode node(JSON_NODE);
		node.push_back(JSONNode("id", "messagingService"));
		node.push_back(JSONNode("type", "EndpointPresenceDoc"));
		node.push_back(JSONNode("selfLink", "uri"));
		node.push_back(privateInfo);
		node.push_back(publicInfo);

		Body << VALUE(node.write().c_str());
	}
};
#endif //_SKYPE_REQUEST_CAPS_H_