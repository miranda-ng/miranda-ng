/*
Copyright (c) 2013-14 Miranda NG project (http://miranda-ng.org)

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

void CVkProto::SearchBasicThread(void* id)
{
	debugLogA("CVkProto::OnSearchBasicThread");
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/users.get.json", true, &CVkProto::OnSearchBasic)
		<< TCHAR_PARAM("user_ids", (TCHAR *) id)
		<< VER_API);

}

void CVkProto::OnSearchBasic(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnSearchBasic %d", reply->resultCode);
	if (reply->resultCode != 200){
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		return;
	}

	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	if (pResponse == NULL){
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		return;
	}

	for (size_t i = 0;; i++) {
		PROTOSEARCHRESULT psr = { sizeof(psr) };
		psr.flags = PSR_TCHAR;

		JSONNODE *pRecord = json_at(pResponse, i);
		if (pRecord == NULL)
			break;
		
		CMString tszNick;
		psr.id = mir_wstrdup(json_as_string(json_get(pRecord, "id")));
		psr.firstName = mir_wstrdup(json_as_string(json_get(pRecord, "first_name")));
		psr.lastName = mir_wstrdup(json_as_string(json_get(pRecord, "last_name")));
		tszNick.Append(psr.firstName);
		tszNick.AppendChar(' ');
		tszNick.Append(psr.lastName);
		psr.nick = mir_wstrdup(tszNick.GetBuffer());
		
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);
	}

	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
}