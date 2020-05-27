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

#ifndef _SKYPE_REQUEST_CAPS_H_
#define _SKYPE_REQUEST_CAPS_H_

struct SendCapabilitiesRequest : public AsyncHttpRequest
{
	SendCapabilitiesRequest(const char *hostname, CSkypeProto *ppro) :
		AsyncHttpRequest(REQUEST_PUT, 0, &CSkypeProto::OnCapabilitiesSended)
	{
		m_szUrl.Format("/users/ME/endpoints/%s/presenceDocs/messagingService", mir_urlEncode(ppro->m_szId).c_str());

		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset=UTF-8");
		AddRegistrationToken(ppro);

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

		m_szParam = node.write().c_str();
	}
};
#endif //_SKYPE_REQUEST_CAPS_H_