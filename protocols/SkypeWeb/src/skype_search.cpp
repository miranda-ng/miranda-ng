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

#include "stdafx.h"

HANDLE CSkypeProto::SearchBasic(const TCHAR* id)
{
	ForkThread(&CSkypeProto::SearchBasicThread, (void *)id);
	return (HANDLE)1;
}

void CSkypeProto::SearchBasicThread(void* id)
{
	debugLogA("CSkypeProto::OnSearchBasicThread");
	if (!IsOnline())
		return;

	ptrT idT((TCHAR *)id);
	ptrA string(mir_urlEncode(T2Utf(idT)));
	SendRequest(new GetSearchRequest(m_szTokenSecret, string), &CSkypeProto::OnSearch);
}

void CSkypeProto::OnSearch(const NETLIBHTTPREQUEST *response)
{
	debugLogA(__FUNCTION__);
	if (response == NULL) {
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		return;
	}

	debugLogA("CSkypeProto::OnSearch %d", response->resultCode);
	if (response->resultCode != 200) {
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		return;
	}

	JSONNode root = JSONNode::parse(response->pData);
	if (!root) {
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		return;
	}

	const JSONNode &items = root.as_array();
	for (size_t i = 0; i < items.size(); i++)
	{
		const JSONNode &item = items.at(i);
		const JSONNode &ContactCards = item["ContactCards"];
		const JSONNode &Skype = ContactCards["Skype"];

		CMStringA displayName(Skype["DisplayName"].as_string().c_str());
		CMStringA nick(Skype["SkypeName"].as_string().c_str());

		PROTOSEARCHRESULT psr = { sizeof(psr) };
		psr.flags = PSR_UTF8;
		psr.id.a = displayName.GetBuffer();
		psr.nick.a = nick.GetBuffer();
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);
	}
	
	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
}