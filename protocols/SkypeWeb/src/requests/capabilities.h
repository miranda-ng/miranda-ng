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

		int bitness = 32;
		char compName[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD size = SIZEOF(compName);
		GetComputerNameA(compName, &size);

#ifdef _WIN64
		bitness = 64;
#endif

		CMStringA data(::FORMAT, "{\"id\":\"messagingService\",\"type\":\"EndpointPresenceDoc\",\"selfLink\":\"uri\",\"privateInfo\":{\"epname\":\"%s\"},\"publicInfo\":{\"capabilities\":\"Audio|Video\",\"typ\":125,\"skypeNameVersion\":\"Miranda NG Skype\",\"nodeInfo\":\"xx\",\"version\":\"%s x%d\"}}", compName, g_szMirVer, bitness);

		Body <<
			VALUE(data);
	}
};
#endif //_SKYPE_REQUEST_CAPS_H_