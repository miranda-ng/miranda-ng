/*
Copyright (c) 2025 Miranda NG team (https://miranda-ng.org)

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

#include "stdafx.h"

HANDLE CTeamsProto::SearchBasic(const wchar_t *id)
{
	debugLogA("CTeamsProto::OnSearchBasicThread");
	if (!IsOnline())
		return 0;

	auto *pReq = new AsyncHttpRequest(REQUEST_POST, HOST_TEAMS_API, "/users/searchV2?includeDLs=true&includeBots=true&enableGuest=true&source=newChat&skypeTeamsInfo=true", &CTeamsProto::OnSearch);
	pReq->m_szParam = mir_urlEncode(T2Utf(id));
	PushRequest(pReq);

	return (HANDLE)1;
}

void CTeamsProto::OnSearch(MHttpResponse *response, AsyncHttpRequest*)
{
	debugLogA(__FUNCTION__);

	TeamsReply reply(response);
	if (reply.error()) {
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		return;
	}

	auto &root = reply.data();
	const JSONNode &items = root["results"].as_array();
	for (auto &it : items) {
		const JSONNode &item = it["nodeProfileData"];

		std::string skypeId = item["skypeId"].as_string();
		if (UrlToSkypeId(skypeId.c_str()).IsEmpty())
			skypeId = "8:" + skypeId;

		std::string name = item["name"].as_string();

		PROTOSEARCHRESULT psr = { sizeof(psr) };
		psr.flags = PSR_UTF8;
		psr.id.a = const_cast<char *>(skypeId.c_str());
		psr.nick.a = const_cast<char *>(name.c_str());
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);
	}

	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
}
