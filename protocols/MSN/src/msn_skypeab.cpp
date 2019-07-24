/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2019 Miranda NG team
Copyright (c) 2007-2012 Boris Krasnovskiy.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "msn_proto.h"

bool CMsnProto::APISkypeComRequest(NETLIBHTTPREQUEST *nlhr, NETLIBHTTPHEADER *headers)
{
	const char *pszSkypeToken = authSkypeToken.XSkypetoken();

	if (!pszSkypeToken) return false;
	nlhr->cbSize = sizeof(NETLIBHTTPREQUEST);
	nlhr->flags = NLHRF_HTTP11 | NLHRF_DUMPASTEXT | NLHRF_PERSISTENT | NLHRF_REDIRECT;
	nlhr->nlc = hHttpsConnection;
	nlhr->headersCount = 3;
	nlhr->headers = headers;
	nlhr->headers[0].szName = "User-Agent";
	nlhr->headers[0].szValue = (char*)MSN_USER_AGENT;
	nlhr->headers[1].szName = "Accept";
	nlhr->headers[1].szValue = "application/json";
	nlhr->headers[2].szName = "X-Skypetoken";
	nlhr->headers[2].szValue = (char*)pszSkypeToken;
	return true;
}

static wchar_t* get_json_str(JSONNode *item, const char *pszValue)
{
	if (JSONNode *node = json_get(item, pszValue)) {
		wchar_t *ret = json_as_string(node);
		if (!mir_wstrcmp(ret, L"null")) {
			mir_free(ret);
			return nullptr;
		}
		return ret;
	}
	return nullptr;
}

bool CMsnProto::MSN_SKYABRefreshClist(void)
{
	NETLIBHTTPREQUEST nlhr = { 0 };
	NETLIBHTTPHEADER headers[3];
	CMStringA post;
	bool bRet = false;

	// initialize the netlib request
	if (!APISkypeComRequest(&nlhr, headers)) return false;
	nlhr.requestType = REQUEST_GET;
	nlhr.szUrl = "https://contacts.skype.com/contacts/v1/users/SELF/contacts";
	

	// Query addressbook
	mHttpsTS = clock();
	NLHR_PTR nlhrReply(Netlib_HttpTransaction(m_hNetlibUser, &nlhr));
	mHttpsTS = clock();
	if (nlhrReply) {
		hHttpsConnection = nlhrReply->nlc;
		if (nlhrReply->resultCode == 200 && nlhrReply->pData) {
			JSONROOT root(nlhrReply->pData);
			if (root == nullptr)
				return false;

			JSONNode *items = json_get(root, "contacts"), *item, *node;
			for (size_t i = 0; i < json_size(items); i++) {
				int lstId = LIST_FL, type = NETID_SKYPE;
				item = json_at(items, i);
				if (item == nullptr)
					break;

				ptrW value;
				ptrA skypename(mir_u2a(ptrW(json_as_string(json_get(item, "id")))));
				ptrA wlid(mir_u2a(ptrW(json_as_string(json_get(item, "person_id")))));
				MCONTACT hContact = MSN_HContactFromEmail(wlid, skypename, false, false);

				if (hContact) {
					if (!json_as_bool(json_get(item, "authorized"))) lstId = LIST_PL;
					if (!json_as_bool(json_get(item, "blocked"))) lstId = LIST_BL;
					if ((node = json_get(item, "name")) && !node->empty()) {
						if (value = get_json_str(node, "first")) setWString(hContact, "FirstName", value);
					}
					//if (value = get_json_str(item, "lastname")) setWString(hContact, "LastName", value);
					if ((node = json_get(item, "locations")) && !node->empty()) {
						if (value = get_json_str(node, "country")) setString(hContact, "Country", (char*)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)(char*)_T2A(value), 0));
						if (value = get_json_str(node, "city")) setWString(hContact, "City", value);
					}
					if (value = get_json_str(item, "mood")) db_set_ws(hContact, "CList", "StatusMsg", value);
					if ((value = get_json_str(item, "type")) && mir_wstrcmp(value, L"skype")) type = NETID_MSN;
					if (value = get_json_str(item, "display_name")) setWString(hContact, "Nick", value);
					Lists_Add(lstId, type, skypename, NULL, value?ptrA(mir_u2a(value)):skypename, NULL);
				}
			}
			bRet = true;
		}
	}
	else hHttpsConnection = nullptr;
	return bRet;
}

// pszPOST = contacts[]={skypename1}&contacts[]={skypename2}&...
bool CMsnProto::MSN_SKYABGetProfiles(const char *pszPOST)
{
	NETLIBHTTPREQUEST nlhr = { 0 };
	NETLIBHTTPHEADER headers[4];
	bool bRet = false;

	// initialize the netlib request
	if (!APISkypeComRequest(&nlhr, headers)) return false;
	nlhr.requestType = REQUEST_POST;
	nlhr.szUrl = "https://api.skype.com/users/self/contacts/profiles";
	nlhr.dataLength = (int)mir_strlen(pszPOST);
	nlhr.pData = (char*)pszPOST;

	mHttpsTS = clock();
	NLHR_PTR nlhrReply(Netlib_HttpTransaction(m_hNetlibUser, &nlhr));
	mHttpsTS = clock();
	if (nlhrReply) {
		hHttpsConnection = nlhrReply->nlc;
		if (nlhrReply->resultCode == 200 && nlhrReply->pData) {
			JSONROOT root(nlhrReply->pData);
			if (root == nullptr) return false;

			JSONNode *items = json_as_array(root), *item, *node;
			for (size_t i = 0; i < json_size(items); i++) {
				item = json_at(items, i);
				if (item == nullptr)
					break;

				node = json_get(item, "username");
				ptrA skypename(mir_u2a(ptrW(json_as_string(node))));
				ptrW value;
				char szWLId[128];
				mir_snprintf(szWLId, sizeof(szWLId), "%d:%s", NETID_SKYPE, skypename);
				MCONTACT hContact = MSN_HContactFromEmail(szWLId, skypename, false, false);

				if (hContact) {
					if (value = get_json_str(item, "firstname")) setWString(hContact, "FirstName", value);
					if (value = get_json_str(item, "lastname")) setWString(hContact, "LastName", value);
					if (value = get_json_str(item, "displayname")) setWString(hContact, "Nick", value);
					if (value = get_json_str(item, "country")) setString(hContact, "Country", (char*)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)(char*)_T2A(value), 0));
					if (value = get_json_str(item, "city")) setWString(hContact, "City", value);
					if (value = get_json_str(item, "mood")) db_set_ws(hContact, "CList", "StatusMsg", value);
				}
			}
			json_delete(items);
			bRet = true;
		}
	}
	else hHttpsConnection = nullptr;
	return bRet;
}

bool CMsnProto::MSN_SKYABGetProfile(const char *wlid)
{
	NETLIBHTTPREQUEST nlhr = { 0 };
	NETLIBHTTPHEADER headers[4];
	bool bRet = false;
	char szURL[256];

	// initialize the netlib request
	if (!APISkypeComRequest(&nlhr, headers)) return false;
	nlhr.requestType = REQUEST_GET;
	mir_snprintf(szURL, sizeof(szURL), "https://api.skype.com/users/%s/profile", wlid);
	nlhr.szUrl = szURL;

	mHttpsTS = clock();
	NLHR_PTR nlhrReply(Netlib_HttpTransaction(m_hNetlibUser, &nlhr));
	mHttpsTS = clock();
	if (nlhrReply) {
		hHttpsConnection = nlhrReply->nlc;
		if (nlhrReply->resultCode == 200 && nlhrReply->pData) {
			JSONROOT item(nlhrReply->pData);
			if (item == nullptr)
				return false;

			ptrA skypename(mir_u2a(ptrW(json_as_string(json_get(item, "username")))));
			ptrW value;
			char szWLId[128];
			mir_snprintf(szWLId, sizeof(szWLId), "%d:%s", NETID_SKYPE, skypename);
			MCONTACT hContact = MSN_HContactFromEmail(szWLId, skypename, false, false);

			if (hContact) {
				if (value = get_json_str(item, "firstname")) setWString(hContact, "FirstName", value);
				if (value = get_json_str(item, "lastname")) setWString(hContact, "LastName", value);
				if (value = get_json_str(item, "displayname")) setWString(hContact, "Nick", value);
				if (value = get_json_str(item, "gender")) setByte(hContact, "Gender", (BYTE)(_wtoi(value) == 1 ? 'M' : 'F'));
				if (value = get_json_str(item, "birthday")) {
					int d, m, y;
					swscanf(value, L"%d-%d-%d", &y, &m, &d);
					setWord(hContact, "BirthYear", y);
					setByte(hContact, "BirthDay", d);
					setByte(hContact, "BirthMonth", m);
				}
				if (value = get_json_str(item, "country")) setString(hContact, "Country", (char*)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)(char*)_T2A(value), 0));
				if (value = get_json_str(item, "province")) setWString(hContact, "State", value);
				if (value = get_json_str(item, "city")) setWString(hContact, "City", value);
				if (value = get_json_str(item, "homepage")) setWString(hContact, "Homepage", value);
				if (value = get_json_str(item, "about")) setWString(hContact, "About", value);

				JSONNode *node = json_get(item, "emails");
				if (node && !node->empty()) {
					int num = 0;
					for (auto it = node->begin(); it != node->end(); ++it, ++num) {
						if (num == 3)
							break;

						char szName[16];
						sprintf(szName, "e-mail%d", num);
						setStringUtf(hContact, szName, (*it).as_string().c_str());
					}
				}
				if (value = get_json_str(item, "phoneMobile")) setWString(hContact, "Cellular", value);
				if (value = get_json_str(item, "phone")) setWString(hContact, "Phone", value);
				if (value = get_json_str(item, "phoneOffice")) setWString(hContact, "CompanyPhone", value);
				if (value = get_json_str(item, "mood")) db_set_ws(hContact, "CList", "StatusMsg", value);
			}
			bRet = true;
		}
	}
	else hHttpsConnection = nullptr;
	return bRet;
}

// pszAction: "block" or "unblock"
bool CMsnProto::MSN_SKYABBlockContact(const char *wlid, const char *pszAction)
{
	NETLIBHTTPREQUEST nlhr = { 0 };
	NETLIBHTTPHEADER headers[4];
	// initialize the netlib request
	if (!APISkypeComRequest(&nlhr, headers))
		return false;
	
	char szURL[256], szPOST[128];
	mir_snprintf(szURL, sizeof(szURL), "https://api.skype.com/users/self/contacts/%s/%s", wlid, pszAction);

	nlhr.requestType = REQUEST_PUT;
	nlhr.szUrl = szURL;
	nlhr.headers[3].szName = "Content-type";
	nlhr.headers[3].szValue = "application/x-www-form-urlencoded";
	nlhr.headersCount++;
	nlhr.dataLength = mir_snprintf(szPOST, sizeof(szPOST), "reporterIp=123.123.123.123&uiVersion=%s", msnProductVer);
	nlhr.pData = szPOST;

	mHttpsTS = clock();
	NLHR_PTR nlhrReply(Netlib_HttpTransaction(m_hNetlibUser, &nlhr));
	mHttpsTS = clock();
	if (!nlhrReply) {
		hHttpsConnection = nullptr;
		return false;
	}
	
	hHttpsConnection = nlhrReply->nlc;
	return true;
}

bool CMsnProto::MSN_SKYABDeleteContact(const char *wlid)
{
	// initialize the netlib request
	NETLIBHTTPREQUEST nlhr = { 0 };
	NETLIBHTTPHEADER headers[4];
	if (!APISkypeComRequest(&nlhr, headers))
		return false;

	char szURL[256];
	mir_snprintf(szURL, sizeof(szURL), "https://contacts.skype.com/contacts/v2/users/SELF/contacts/%s", wlid);
	nlhr.requestType = REQUEST_DELETE;
	nlhr.szUrl = szURL;
	nlhr.headers[3].szName = "Content-type";
	nlhr.headers[3].szValue = "application/x-www-form-urlencoded";
	nlhr.headersCount++;

	mHttpsTS = clock();
	NLHR_PTR nlhrReply(Netlib_HttpTransaction(m_hNetlibUser, &nlhr));
	mHttpsTS = clock();
	if (!nlhrReply) {
		hHttpsConnection = nullptr;
		return false;
	}

	hHttpsConnection = nlhrReply->nlc;
	return true;
}

// pszAction: "accept" or "decline"
bool CMsnProto::MSN_SKYABAuthRsp(const char *wlid, const char *pszAction)
{
	// initialize the netlib request
	NETLIBHTTPREQUEST nlhr = { 0 };
	NETLIBHTTPHEADER headers[3];
	if (!APISkypeComRequest(&nlhr, headers))
		return false;

	char szURL[256];
	mir_snprintf(szURL, sizeof(szURL), "https://contacts.skype.com/contacts/v2/users/SELF/invites/%s/%s", wlid, pszAction);
	nlhr.requestType = REQUEST_PUT;
	nlhr.szUrl = szURL;

	mHttpsTS = clock();
	NLHR_PTR nlhrReply(Netlib_HttpTransaction(m_hNetlibUser, &nlhr));
	mHttpsTS = clock();
	if (!nlhrReply) {
		hHttpsConnection = nullptr;
		return false;
	}
	hHttpsConnection = nlhrReply->nlc;
	return true;
}

bool CMsnProto::MSN_SKYABAuthRq(const char *wlid, const char *pszGreeting)
{
	// initialize the netlib request
	NETLIBHTTPHEADER headers[4];
	NETLIBHTTPREQUEST nlhr = { 0 };
	if (!APISkypeComRequest(&nlhr, headers))
		return false;
	
	nlhr.requestType = REQUEST_PUT;
	nlhr.szUrl = "https://contacts.skype.com/contacts/v2/users/SELF/contacts";
	nlhr.headers[3].szName = "Content-type";
	nlhr.headers[3].szValue = "application/x-www-form-urlencoded";
	nlhr.headersCount++;
	JSONNode node;
	node 
		<< JSONNode("mri", wlid)
		<< JSONNode("greeting", pszGreeting);

	CMStringA post = node.write().c_str();
	nlhr.dataLength = post.GetLength();
	nlhr.pData = (char*)(const char*)post;

	mHttpsTS = clock();
	NLHR_PTR nlhrReply(Netlib_HttpTransaction(m_hNetlibUser, &nlhr));
	mHttpsTS = clock();
	if (nlhrReply) {
		hHttpsConnection = nlhrReply->nlc;
		return true;
	}
	
	hHttpsConnection = nullptr;
	return false;
}

bool CMsnProto::MSN_SKYABSearch(const char *keyWord, HANDLE hSearch)
{
	NETLIBHTTPREQUEST nlhr = { 0 };
	NETLIBHTTPHEADER headers[4];
	bool bRet = false;

	// initialize the netlib request
	if (!APISkypeComRequest(&nlhr, headers)) {
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, hSearch);
		return false;
	}

	char szURL[256];
	mir_snprintf(szURL, sizeof(szURL), "https://api.skype.com/search/users/any?keyWord=%s&contactTypes[]=skype", keyWord);

	nlhr.requestType = REQUEST_GET;
	nlhr.szUrl = szURL;
	nlhr.headers[3].szName = "Connection";
	nlhr.headers[3].szValue = "keep-alive";
	nlhr.headersCount++;

	mHttpsTS = clock();
	NLHR_PTR nlhrReply(Netlib_HttpTransaction(m_hNetlibUser, &nlhr));
	mHttpsTS = clock();
	if (nlhrReply) {
		hHttpsConnection = nlhrReply->nlc;
		if (nlhrReply->resultCode == 200 && nlhrReply->pData) {
			JSONROOT root(nlhrReply->pData);
			if (root == nullptr) {
				ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, hSearch);
				return false;
			}

			JSONNode *items = json_as_array(root);
			for (size_t i = 0; i < json_size(items); i++) {
				JSONNode *item = json_at(items, i);
				JSONNode *ContactCards = json_get(item, "ContactCards");
				JSONNode *Skype = json_get(ContactCards, "Skype");

				wchar_t *sDisplayName = json_as_string(json_get(Skype, "DisplayName"));
				wchar_t *sSkypeName = json_as_string(json_get(Skype, "SkypeName"));

				PROTOSEARCHRESULT psr = { sizeof(psr) };
				psr.flags = PSR_UNICODE;
				psr.id.w = sSkypeName;
				psr.nick.w = sDisplayName;
				ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, hSearch, (LPARAM)&psr);
			}
			json_free(items);
			ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, hSearch);
			bRet = true;
		}
	}
	else hHttpsConnection = nullptr;
	return bRet;
}
