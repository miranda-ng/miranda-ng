/*
Copyright (c) 2013-16 Miranda NG project (http://miranda-ng.org)

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

HANDLE CVkProto::SearchBasic(const wchar_t *id)
{
	ForkThread(&CVkProto::SearchBasicThread, (void *)id);
	return (HANDLE)1;
}

HANDLE CVkProto::SearchByEmail(const wchar_t *email)
{
	ForkThread(&CVkProto::SearchByMailThread, (void *)email);
	return (HANDLE)1;
}

HANDLE CVkProto::SearchByName(const wchar_t *nick, const wchar_t *firstName, const wchar_t *lastName)
{
	PROTOSEARCHBYNAME *psr = new (PROTOSEARCHBYNAME);

	psr->pszFirstName = mir_wstrdup(firstName);
	psr->pszLastName = mir_wstrdup(lastName);
	psr->pszNick = mir_wstrdup(nick);

	ForkThread(&CVkProto::SearchThread, (void *)psr);
	return (HANDLE)1;
}

void CVkProto::SearchBasicThread(void *id)
{
	debugLogA("CVkProto::OnSearchBasicThread");
	if (!IsOnline())
		return;
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/users.get.json", true, &CVkProto::OnSearch)
		<< TCHAR_PARAM("user_ids", (wchar_t *)id)
		<< CHAR_PARAM("fields", "nickname, domain");
	pReq->pUserInfo = NULL;
	Push(pReq);
}

void CVkProto::SearchByMailThread(void *email)
{
	debugLogA("CVkProto::OnSearchBasicThread");
	if (!IsOnline())
		return;
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/account.lookupContacts.json", true, &CVkProto::OnSearchByMail)
		<< TCHAR_PARAM("contacts", (wchar_t *)email)
		<< CHAR_PARAM("service", "email");
	Push(pReq);
}

void __cdecl CVkProto::SearchThread(void *p)
{
	PROTOSEARCHBYNAME *pParam = (PROTOSEARCHBYNAME *)p;

	wchar_t arg[200];
	mir_snwprintf(arg, L"%s %s %s", pParam->pszFirstName, pParam->pszNick, pParam->pszLastName);
	debugLog(L"CVkProto::SearchThread %s", arg);
	if (!IsOnline())
		return;

	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_GET, "/method/users.search.json", true, &CVkProto::OnSearch)
		<< TCHAR_PARAM("q", (wchar_t *)arg)
		<< CHAR_PARAM("fields", "nickname, domain")
		<< INT_PARAM("count", 200);
	pReq->pUserInfo = p;
	Push(pReq);
}

void CVkProto::FreeProtoShearchStruct(PROTOSEARCHBYNAME *pParam)
{
	if (!pParam)
		return;
	
	mir_free(pParam->pszFirstName);
	mir_free(pParam->pszLastName);
	mir_free(pParam->pszNick);
	delete pParam;
}

void CVkProto::OnSearch(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	PROTOSEARCHBYNAME *pParam = (PROTOSEARCHBYNAME *)pReq->pUserInfo;
	debugLogA("CVkProto::OnSearch %d", reply->resultCode);
	if (reply->resultCode != 200) {
		FreeProtoShearchStruct(pParam);
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1);
		return;
	}

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse) {
		FreeProtoShearchStruct(pParam);
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1);
		return;
	}

	const JSONNode &jnItems = !jnResponse["items"] ? jnResponse : jnResponse["items"];
	for (auto it = jnItems.begin(); it != jnItems.end(); ++it) {
		const JSONNode &jnRecord = (*it);
		if (!jnRecord)
			break;

		PROTOSEARCHRESULT psr = { sizeof(psr) };
		psr.flags = PSR_TCHAR;

		CMString Id(FORMAT, L"%d", jnRecord["id"].as_int());
		CMString FirstName(jnRecord["first_name"].as_mstring());
		CMString LastName(jnRecord["last_name"].as_mstring());
		CMString Nick(jnRecord["nickname"].as_mstring());
		CMString Domain(jnRecord["domain"].as_mstring());

		psr.id.w = mir_wstrdup(Id);
		psr.firstName.w = mir_wstrdup(FirstName);
		psr.lastName.w = mir_wstrdup(LastName);
		psr.nick.w = Nick.IsEmpty() ? mir_wstrdup(Domain) : mir_wstrdup(Nick);

		bool filter = true;
		if (pParam) {
			if (psr.firstName.w && pParam->pszFirstName)
				filter = tlstrstr(psr.firstName.w, pParam->pszFirstName) && filter;
			if (psr.lastName.w && pParam->pszLastName)
				filter = tlstrstr(psr.lastName.w, pParam->pszLastName) && filter;
			if (psr.nick.w && pParam->pszNick)
				filter = tlstrstr(psr.nick.w, pParam->pszNick) && filter;
		}

		if (filter)
			ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);
	}

	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1);
	FreeProtoShearchStruct(pParam);
}

void CVkProto::OnSearchByMail(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnSearch %d", reply->resultCode);
	if (reply->resultCode != 200) {
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1);
		return;
	}

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse) {
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1);
		return;
	}

	const JSONNode &jnItems = jnResponse["found"];
	if (!jnItems) {
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1);
		return;
	}

	for (auto it = jnItems.begin(); it != jnItems.end(); ++it) {
		const JSONNode &jnRecord = (*it);
		if (!jnRecord)
			break;

		PROTOSEARCHRESULT psr = { sizeof(psr) };
		psr.flags = PSR_TCHAR;
		
		CMString Id(FORMAT, L"%d", jnRecord["id"].as_int());
		CMString FirstName(jnRecord["first_name"].as_mstring());
		CMString LastName(jnRecord["last_name"].as_mstring());
		CMString Nick(jnRecord["nickname"].as_mstring());
		CMString Email(jnRecord["contact"].as_mstring());

		psr.id.w = mir_wstrdup(Id);
		psr.firstName.w = mir_wstrdup(FirstName);
		psr.lastName.w = mir_wstrdup(LastName);
		psr.nick.w = Nick.IsEmpty() ? mir_wstrdup(Email) : mir_wstrdup(Nick);
		psr.email.w = mir_wstrdup(Email);

		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);
	}

	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1);
}