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

HANDLE CSkypeProto::SearchBasic(const PROTOCHAR* id)
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
	SendRequest(new GetSearchRequest(TokenSecret, string), &CSkypeProto::OnSearch);
}

void CSkypeProto::OnSearch(const NETLIBHTTPREQUEST *response)
{
	debugLogA(__FUNCTION__);
	if (response == NULL)
	{
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		return;
	}

	debugLogA("CSkypeProto::OnSearch %d", response->resultCode);
	if (response->resultCode != 200) {
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		return;
	}

	JSONROOT root(response->pData);
	if (root == NULL)
	{
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		return;
	}

	JSONNODE *items = json_as_array(root);
	for (size_t i = 0; i < json_size(items); i++)
	{
		JSONNODE *item = json_at(items, i);
		JSONNODE *ContactCards = json_get(item, "ContactCards");
		JSONNODE *Skype = json_get(ContactCards, "Skype");

		TCHAR *sDisplayName = json_as_string(json_get(Skype, "DisplayName"));
		TCHAR *sSkypeName = json_as_string(json_get(Skype, "SkypeName"));

		PROTOSEARCHRESULT psr = { sizeof(psr) };
		psr.flags = PSR_TCHAR;
		psr.id = mir_wstrdup(sSkypeName);
		psr.nick = mir_wstrdup(sDisplayName);
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);
	}
	json_free(items);
	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
}