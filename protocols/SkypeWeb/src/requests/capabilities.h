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

#ifndef _SKYPE_REQUEST_CAPS_H_
#define _SKYPE_REQUEST_CAPS_H_

struct SendCapabilitiesRequest : public AsyncHttpRequest
{
	SendCapabilitiesRequest(const char *hostname, CSkypeProto *ppro) :
		AsyncHttpRequest(REQUEST_PUT, HOST_DEFAULT, "/users/ME/endpoints/" + mir_urlEncode(ppro->m_szId) + "/presenceDocs/messagingService", &CSkypeProto::OnCapabilitiesSended)
	{
		JSONNode privateInfo; privateInfo.set_name("privateInfo");
		privateInfo << CHAR_PARAM("epname", hostname);

		JSONNode publicInfo; publicInfo.set_name("publicInfo");
		publicInfo << CHAR_PARAM("capabilities", "Audio|Video") << INT_PARAM("typ", 125)
			<< CHAR_PARAM("skypeNameVersion", "Miranda NG Skype") << CHAR_PARAM("nodeInfo", "xx") << CHAR_PARAM("version", g_szMirVer);

		JSONNode node;
		node << CHAR_PARAM("id", "messagingService")  << CHAR_PARAM("type", "EndpointPresenceDoc")
			<< CHAR_PARAM("selfLink", "uri") << privateInfo  << publicInfo;

		m_szParam = node.write().c_str();
	}
};

#endif //_SKYPE_REQUEST_CAPS_H_
