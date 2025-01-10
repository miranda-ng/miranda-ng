/*
Copyright (c) 2015-25 Miranda NG team (https://miranda-ng.org)

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

void CSkypeProto::OnGetServerHistory(MHttpResponse *response, AsyncHttpRequest *pRequest)
{
	JsonReply reply(response);
	if (reply.error())
		return;

	auto *pOrig = (GetHistoryRequest *)pRequest;
	auto &root = reply.data();
	const JSONNode &metadata = root["_metadata"];

	int totalCount = metadata["totalCount"].as_int();
	std::string syncState = metadata["syncState"].as_string();

	bool bOperative = pRequest->pUserInfo != 0;
	bool bUseLocalTime = !m_bUseServerTime && bOperative;
	bool bSetLastTime = false;

	int64_t lastMsgTime = 0; // max timestamp on this page
	time_t iLocalTime = time(0);

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
		dbei.iTimestamp = (bUseLocalTime) ? iLocalTime : IsoToUnixTime(message["composetime"].as_string());
		dbei.szId = szMessageId;
		if (iUserType == 19) {
			dbei.szUserId = szFrom;

			CMStringA szType(message["messagetype"].as_mstring());
			if (szType.Left(15) == "ThreadActivity/")
				continue;
		}

		dbei.flags = DBEF_UTF;
		if (!bOperative && !dbei.getEvent())
			dbei.flags |= DBEF_READ;
		if (IsMe(szFrom))
			dbei.flags |= DBEF_SENT;

		if (ParseMessage(message, dbei)) {
			if (dbei)
				db_event_edit(dbei.getEvent(), &dbei, true);
			else
				db_event_add(hContact, &dbei);
		}
	}

	if (bSetLastTime && lastMsgTime > getLastTime(pOrig->hContact))
		setLastTime(pOrig->hContact, lastMsgTime);

	if (totalCount >= 99 || conv.size() >= 99)
		PushRequest(new GetHistoryRequest(pOrig->hContact, pOrig->m_who, 100, lastMsgTime, pRequest->pUserInfo != 0));
}

INT_PTR CSkypeProto::SvcLoadHistory(WPARAM hContact, LPARAM)
{
	PushRequest(new GetHistoryRequest(hContact, getId(hContact), 100, 0, false));
	return 0;
}

void CSkypeProto::OnSyncConversations(MHttpResponse *response, AsyncHttpRequest*)
{
	JsonReply reply(response);
	if (reply.error())
		return;

	auto &root = reply.data();
	const JSONNode &metadata = root["_metadata"];
	const JSONNode &conversations = root["conversations"].as_array();

	// int totalCount = metadata["totalCount"].as_int();
	std::string syncState = metadata["syncState"].as_string();

	for (auto &it: conversations) {
		const JSONNode &lastMessage = it["lastMessage"];
		if (!lastMessage)
			continue;

		int iUserType;
		std::string strConversationLink = lastMessage["conversationLink"].as_string();
		CMStringA szSkypename = UrlToSkypeId(strConversationLink.c_str(), &iUserType);
		switch (iUserType) {
		case 19:
			{
				auto &props = it["threadProperties"];
				if (!props["lastleaveat"])
					StartChatRoom(it["id"].as_mstring(), props["topic"].as_mstring(), props["version"].as_string().c_str());
			}
			__fallthrough;

		case 8:
		case 2:
			int64_t id = _atoi64(lastMessage["id"].as_string().c_str());

			MCONTACT hContact = FindContact(szSkypename);
			if (hContact != NULL) {
				auto lastMsgTime = getLastTime(hContact);
				if (lastMsgTime && lastMsgTime < id && m_bAutoHistorySync)
					PushRequest(new GetHistoryRequest(hContact, szSkypename, 100, lastMsgTime, true));
			}
		}
	}

	m_bHistorySynced = true;
}

//////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CSkypeProto::SvcEmptyHistory(WPARAM hContact, LPARAM flags)
{
	if (flags & CDF_DEL_HISTORY) {
		PushRequest(new EmptyHistoryRequest(getId(hContact)));
	}
	return 0;
}
