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


static bool tlstrstr(TCHAR* _s1, TCHAR* _s2)
{
	TCHAR s1[200], s2[200];
	mir_sntprintf(s1, SIZEOF(s1), _T("%s"), _s1);
	mir_sntprintf(s2, SIZEOF(s2), _T("%s"), _s2);
	CharLowerBuff(s1, SIZEOF(s1));
	CharLowerBuff(s2, SIZEOF(s2));
	return (_tcsstr(s1, s2)==NULL);
}

void CVkProto::SearchBasicThread(void* id)
{
	debugLogA("CVkProto::OnSearchBasicThread");
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/users.get.json", true, &CVkProto::OnSearch)
		<< TCHAR_PARAM("user_ids", (TCHAR *)id)
		<< CHAR_PARAM("fields", "nickname, domain")
		<< VER_API;
	pReq->pUserInfo = NULL;
	Push(pReq);
}

void CVkProto::SearchByMailThread(void* email)
{
	debugLogA("CVkProto::OnSearchBasicThread");
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/account.lookupContacts.json", true, &CVkProto::OnSearchByMail)
		<< TCHAR_PARAM("contacts", (TCHAR *)email)
		<< CHAR_PARAM("service", "email")
		<< VER_API;
	Push(pReq);
}

void __cdecl CVkProto::SearchThread(void* p)
{
	PROTOSEARCHBYNAME *pParam = (PROTOSEARCHBYNAME *)p;
		
	TCHAR arg[200];
	mir_sntprintf(arg, SIZEOF(arg), _T("%s %s %s"), pParam->pszFirstName, pParam->pszNick, pParam->pszLastName);
	debugLog(_T("CVkProto::SearchThread %s"), arg);

	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/users.search.json", true, &CVkProto::OnSearch)
		<< TCHAR_PARAM("q", (TCHAR *)arg)
		<< CHAR_PARAM("fields", "nickname, domain")
		<< INT_PARAM("count", 200)
		<< VER_API;
	pReq->pUserInfo = p;
	Push(pReq);
}

void CVkProto::OnSearch(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	PROTOSEARCHBYNAME *pParam = (PROTOSEARCHBYNAME *)pReq->pUserInfo;
	debugLogA("CVkProto::OnSearch %d", reply->resultCode);
	if (reply->resultCode != 200){
		if (pParam){
			mir_free(pParam->pszFirstName);
			mir_free(pParam->pszLastName);
			mir_free(pParam->pszNick);
			delete pParam;
		}
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		return;
	}

	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	if (pResponse == NULL){
		if (pParam){
			mir_free(pParam->pszFirstName);
			mir_free(pParam->pszLastName);
			mir_free(pParam->pszNick);
			delete pParam;
		}
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		return;
	}

	int iCount = json_as_int(json_get(pResponse, "count"));
	JSONNODE *pItems = json_get(pResponse, "items");
	if (!pItems){
		pItems = pResponse;
		iCount = 1;
	} 

	for (int i = 0; i<iCount; i++) {
		JSONNODE *pRecord = json_at(pItems, i);
		if (pRecord == NULL)
			break;
		
		PROTOSEARCHRESULT psr = { sizeof(psr) };
		psr.flags = PSR_TCHAR;

		psr.id = mir_wstrdup(json_as_string(json_get(pRecord, "id")));
		psr.firstName = mir_wstrdup(json_as_string(json_get(pRecord, "first_name")));
		psr.lastName = mir_wstrdup(json_as_string(json_get(pRecord, "last_name")));
		psr.nick = mir_wstrdup(json_as_string(json_get(pRecord, "nickname")));
		if (!psr.nick || !psr.nick[0])
			psr.nick = mir_wstrdup(json_as_string(json_get(pRecord, "domain")));
		
		bool filter = true;
		if (pParam){
			if (psr.firstName&&pParam->pszFirstName)
				filter = tlstrstr(psr.firstName, pParam->pszFirstName) && filter;
			if (psr.lastName&&pParam->pszLastName)
				filter = tlstrstr(psr.lastName, pParam->pszLastName) && filter;
			if (psr.nick&&pParam->pszNick)
				filter = tlstrstr(psr.nick, pParam->pszNick) && filter;
		}

		if (filter)
			ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);
	}

	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
	if (pParam){
		mir_free(pParam->pszFirstName);
		mir_free(pParam->pszLastName);
		mir_free(pParam->pszNick);
		delete pParam;
	}
}

void CVkProto::OnSearchByMail(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnSearch %d", reply->resultCode);
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

	JSONNODE *pItems = json_get(pResponse, "found");
	if (!pItems){
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		return;
	}
	
	for (int i = 0;; i++) {
		JSONNODE *pRecord = json_at(pItems, i);
		if (pRecord == NULL)
			break;

		PROTOSEARCHRESULT psr = { sizeof(psr) };
		psr.flags = PSR_TCHAR;

		psr.id = mir_wstrdup(json_as_string(json_get(pRecord, "id")));
		psr.firstName = mir_wstrdup(json_as_string(json_get(pRecord, "first_name")));
		psr.lastName = mir_wstrdup(json_as_string(json_get(pRecord, "last_name")));
		psr.nick = mir_wstrdup(json_as_string(json_get(pRecord, "nickname")));
		psr.email = mir_wstrdup(json_as_string(json_get(pRecord, "contact")));
		if (!psr.nick || !psr.nick[0])
				psr.nick = psr.email;
			
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);
	}

	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
}
