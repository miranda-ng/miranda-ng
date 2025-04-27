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

/* HISTORY SYNC */

void CTeamsProto::RefreshConversations()
{
	auto *pReq = new AsyncHttpRequest(REQUEST_GET, HOST_DEFAULT, "/users/ME/conversations", &CTeamsProto::OnSyncConversations);
	pReq << INT_PARAM("startTime", getLastTime(0)) << INT_PARAM("pageSize", 100)
		<< CHAR_PARAM("view", "msnp24Equivalent") << CHAR_PARAM("targetType", "Passport|Skype|Lync|Thread|PSTN|Agent");

	PushRequest(pReq);
}

void CTeamsProto::FetchMissingHistory(const JSONNode &node, MCONTACT hContact)
{
	const JSONNode &lastMessage = node["lastMessage"];
	if (lastMessage && hContact) {
		int64_t id = _atoi64(lastMessage["id"].as_string().c_str());
		auto lastMsgTime = getLastTime(hContact);
		if (lastMsgTime && lastMsgTime < id && m_bAutoHistorySync)
			GetServerHistory(hContact, 100, lastMsgTime, true);
	}
}

void CTeamsProto::OnSyncConversations(MHttpResponse *response, AsyncHttpRequest *)
{
	TeamsReply reply(response);
	if (reply.error())
		return;

	auto &root = reply.data();
	const JSONNode &metadata = root["_metadata"];
	const JSONNode &conversations = root["conversations"].as_array();

	for (auto &it : conversations) {
		CMStringA szSkypename = it["id"].as_mstring();
		int iUserType = atoi(szSkypename);
		MCONTACT hContact = FindContact(szSkypename);

		switch (iUserType) {
		case 19:
			{
				auto &props = it["threadProperties"];
				if (!props["lastleaveat"])
					StartChatRoom(it["id"].as_mstring(), props["topic"].as_mstring(), props["version"].as_string().c_str());
			}
			FetchMissingHistory(it, hContact);
			break;

		case 8:
		case 2:
			CMStringA szChatId(it["properties"]["onetoonethreadid"].as_mstring());
			if (!szChatId.IsEmpty() && hContact) {
				if (szChatId.Left(3) != "19:")
					szChatId.Insert(0, "19:");
				setString(hContact, "ChatId", szChatId);
			}

			FetchMissingHistory(it, hContact);
		}
	}

	std::string syncState = metadata["syncState"].as_string();

	m_bHistorySynced = true;
}

//////////////////////////////////////////////////////////////////////////////////////////

void CTeamsProto::GetServerHistory(MCONTACT hContact, int pageSize, int64_t timestamp, bool bOperative)
{
	CMStringA szChatId(getMStringA(hContact, "ChatId"));
	if (szChatId.IsEmpty())
		szChatId = getId(hContact);

	auto *pReq = new AsyncHttpRequest(REQUEST_GET, HOST_DEFAULT, "/users/ME/conversations/" + mir_urlEncode(szChatId) + "/messages", &CTeamsProto::OnGetServerHistory);
	pReq->hContact = hContact;
	if (bOperative)
		pReq->pUserInfo = this;

	pReq << INT64_PARAM("startTime", timestamp) << INT_PARAM("pageSize", pageSize)
		<< CHAR_PARAM("view", "msnp24Equivalent") << CHAR_PARAM("targetType", "Passport|Skype|Lync|Thread");

	PushRequest(pReq);
}

void CTeamsProto::OnGetServerHistory(MHttpResponse *response, AsyncHttpRequest *pRequest)
{
	TeamsReply reply(response);
	if (reply.error())
		return;

	auto &root = reply.data();
	const JSONNode &metadata = root["_metadata"];

	int totalCount = metadata["totalCount"].as_int();
	std::string syncState = metadata["syncState"].as_string();

	bool bOperative = pRequest->pUserInfo != 0;
	bool bSetLastTime = false;

	int64_t lastMsgTime = 0; // max timestamp on this page

	auto &conv = root["messages"];
	for (auto it = conv.rbegin(); it != conv.rend(); ++it) {
		auto &message = *it;
		CMStringA szId = message["id"].as_mstring();
		int64_t id = _atoi64(szId);
		if (id > lastMsgTime) {
			bSetLastTime = true;
			lastMsgTime = id;
		}

		int iUserType;
		CMStringA szMessageId(getMessageId(message));
		CMStringA szChatId = UrlToSkypeId(message["conversationLink"].as_mstring(), &iUserType);
		CMStringA szFrom = UrlToSkypeId(message["from"].as_mstring());

		MCONTACT hContact = FindContact(szChatId);

		DB::EventInfo dbei(db_event_getById(m_szModuleName, szMessageId));
		dbei.hContact = hContact;
		dbei.szModule = m_szModuleName;
		dbei.szId = szMessageId;
		dbei.bSent = IsMe(szFrom);
		dbei.bMsec = dbei.bUtf = true;
		dbei.iTimestamp = _wtoi64(message["id"].as_mstring());

		if (iUserType == 19) {
			dbei.szUserId = szFrom;

			CMStringA szType(message["messagetype"].as_mstring());
			if (szType.Left(15) == "ThreadActivity/")
				continue;
		}

		if (!bOperative && !dbei.getEvent())
			dbei.bRead = true;

		if (ParseMessage(message, dbei)) {
			if (dbei)
				db_event_edit(dbei.getEvent(), &dbei, true);
			else
				db_event_add(hContact, &dbei);
		}
	}

	if (bSetLastTime && lastMsgTime > getLastTime(pRequest->hContact))
		setLastTime(pRequest->hContact, lastMsgTime);

	if (totalCount >= 99 || conv.size() >= 99)
		GetServerHistory(pRequest->hContact, 100, lastMsgTime, pRequest->pUserInfo != 0);
}

INT_PTR CTeamsProto::SvcLoadHistory(WPARAM hContact, LPARAM)
{
	GetServerHistory(hContact, 100, 0, false);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CTeamsProto::SvcEmptyHistory(WPARAM hContact, LPARAM flags)
{
	if (flags & CDF_DEL_HISTORY)
		PushRequest(new AsyncHttpRequest(REQUEST_DELETE, HOST_DEFAULT, "/users/ME/conversations/" + mir_urlEncode(getId(hContact)) + "/messages"));

	return 0;
}
