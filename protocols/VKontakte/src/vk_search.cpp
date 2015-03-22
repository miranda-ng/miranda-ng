/*
Copyright (c) 2013-15 Miranda NG project (http://miranda-ng.org)

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

HANDLE CVkProto::SearchBasic(const PROTOCHAR* id)
{
	ForkThread(&CVkProto::SearchBasicThread, (void *)id);
	return (HANDLE)1;
}

HANDLE CVkProto::SearchByEmail(const PROTOCHAR* email)
{
	ForkThread(&CVkProto::SearchByMailThread, (void *)email);
	return (HANDLE)1;
}

HANDLE CVkProto::SearchByName(const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName)
{
	PROTOSEARCHBYNAME * psr = new (PROTOSEARCHBYNAME);

	psr->pszFirstName = mir_wstrdup(firstName);
	psr->pszLastName = mir_wstrdup(lastName);
	psr->pszNick = mir_wstrdup(nick);

	ForkThread(&CVkProto::SearchThread, (void *)psr);
	return (HANDLE)1;
}

void CVkProto::SearchBasicThread(void* id)
{
	debugLogA("CVkProto::OnSearchBasicThread");
	if (!IsOnline())
		return;
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
	if (!IsOnline())
		return;
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
	if (!IsOnline())
		return;

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
	if (reply->resultCode != 200) {
		if (pParam) {
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
	if (pResponse == NULL) {
		if (pParam) {
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
	if (!pItems) {
		pItems = pResponse;
		iCount = 1;
	} 

	for (int i = 0; i<iCount; i++) {
		JSONNODE *pRecord = json_at(pItems, i);
		if (pRecord == NULL)
			break;
		
		PROTOSEARCHRESULT psr = { sizeof(psr) };
		psr.flags = PSR_TCHAR;

		ptrT pId(json_as_string(json_get(pRecord, "id")));
		ptrT pFirstName(json_as_string(json_get(pRecord, "first_name")));
		ptrT pLastName(json_as_string(json_get(pRecord, "last_name")));
		ptrT pNick(json_as_string(json_get(pRecord, "nickname")));
		ptrT pDomain(json_as_string(json_get(pRecord, "domain")));

		psr.id = mir_wstrdup(pId);
		psr.firstName = mir_wstrdup(pFirstName);
		psr.lastName = mir_wstrdup(pLastName);
		psr.nick = mir_wstrdup(pNick);
		if (!psr.nick || !psr.nick[0])
			psr.nick = mir_wstrdup(pDomain);
		
		bool filter = true;
		if (pParam) {
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
	if (pParam) {
		mir_free(pParam->pszFirstName);
		mir_free(pParam->pszLastName);
		mir_free(pParam->pszNick);
		delete pParam;
	}
}

void CVkProto::OnSearchByMail(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnSearch %d", reply->resultCode);
	if (reply->resultCode != 200) {
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		return;
	}

	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	if (pResponse == NULL) {
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		return;
	}

	JSONNODE *pItems = json_get(pResponse, "found");
	if (!pItems) {
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		return;
	}
	
	for (int i = 0;; i++) {
		JSONNODE *pRecord = json_at(pItems, i);
		if (pRecord == NULL)
			break;

		PROTOSEARCHRESULT psr = { sizeof(psr) };
		psr.flags = PSR_TCHAR;

		ptrT pId(json_as_string(json_get(pRecord, "id")));
		ptrT pFirstName(json_as_string(json_get(pRecord, "first_name")));
		ptrT pLastName(json_as_string(json_get(pRecord, "last_name")));
		ptrT pNick(json_as_string(json_get(pRecord, "nickname")));
		ptrT pEmail(json_as_string(json_get(pRecord, "contact")));

		psr.id = mir_wstrdup(pId);
		psr.firstName = mir_wstrdup(pFirstName);
		psr.lastName = mir_wstrdup(pLastName);
		psr.nick = mir_wstrdup(pNick);
		psr.email = mir_wstrdup(pEmail);
		if (!psr.nick || !psr.nick[0])
				psr.nick = psr.email;
			
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);
	}

	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
}