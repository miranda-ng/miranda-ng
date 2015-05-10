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

		short bitness = 32;

		char compName[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD size = SIZEOF(compName);
		GetComputerNameA(compName, &size);

#ifdef _WIN64
		bitness = 64;
#endif

		CMStringA verString(::FORMAT, "%s x%d", g_szMirVer, bitness);

		JSONNODE *node = json_new(5);
		JSONNODE *privateInfo = json_new(5);
		JSONNODE *publicInfo = json_new(5);

		json_set_name(privateInfo, "privateInfo");
		json_set_name(publicInfo, "publicInfo");

		json_push_back(node, json_new_a("id", "messagingService"));
		json_push_back(node, json_new_a("type", "EndpointPresenceDoc"));
		json_push_back(node, json_new_a("selfLink", "uri"));

		json_push_back(privateInfo, json_new_a("epname", compName));

		json_push_back(publicInfo, json_new_a("capabilities", "Audio|Video"));
		json_push_back(publicInfo, json_new_i("typ", 125));
		json_push_back(publicInfo, json_new_a("skypeNameVersion", "Miranda NG Skype"));
		json_push_back(publicInfo, json_new_a("nodeInfo", "xx"));
		json_push_back(publicInfo, json_new_a("version", verString.GetBuffer()));

		json_push_back(node, privateInfo);
		json_push_back(node, publicInfo);

		ptrA data(mir_utf8encodeT(ptrT(json_write(node))));

		Body <<
			VALUE(data);

		json_delete(node);
	}
};
#endif //_SKYPE_REQUEST_CAPS_H_