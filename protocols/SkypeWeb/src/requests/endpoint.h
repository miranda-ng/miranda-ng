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

struct CreateEndpointRequest : public AsyncHttpRequest
{
	CreateEndpointRequest(CSkypeProto *ppro) :
		AsyncHttpRequest(REQUEST_POST, "/users/ME/endpoints", &CSkypeProto::OnEndpointCreated)
	{
		m_szParam = "{}";

		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset=UTF-8");
		AddHeader("Authentication", CMStringA(FORMAT, "skypetoken=%s", ppro->m_szApiToken.get()));
	}
};

struct DeleteEndpointRequest : public AsyncHttpRequest
{
	DeleteEndpointRequest(CSkypeProto *ppro) :
	  AsyncHttpRequest(REQUEST_DELETE)
	{
		m_szUrl.Format("/users/ME/endpoints/%s", mir_urlEncode(ppro->m_szId).c_str());

		AddHeader("Accept", "application/json, text/javascript");
		AddRegistrationToken(ppro);
	}
};

#endif //_SKYPE_REQUEST_ENDPOINT_H_
