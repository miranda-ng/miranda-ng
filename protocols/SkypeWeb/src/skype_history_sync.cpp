/*
Copyright (c) 2015-20 Miranda NG team (https://miranda-ng.org)

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

void CSkypeProto::OnGetServerHistory(const NETLIBHTTPREQUEST *response)
{
	JsonReply reply(response);
	if (reply.error())
		return;

	auto &root = reply.data();
	const JSONNode &metadata = root["_metadata"];
	const JSONNode &conversations = root["messages"].as_array();

	int totalCount = metadata["totalCount"].as_int();
	std::string syncState = metadata["syncState"].as_string();

	bool markAllAsUnread = getBool("MarkMesUnread", true);

	if (totalCount >= 99 || conversations.size() >= 99)
		PushRequest(new GetHistoryOnUrlRequest(syncState.c_str(), this), &CSkypeProto::OnGetServerHistory);

	for (int i = (int)conversations.size(); i >= 0; i--) {
		const JSONNode &message = conversations.at(i);

		CMStringA szMessageId = message["clientmessageid"] ? message["clientmessageid"].as_string().c_str() : message["skypeeditedid"].as_string().c_str();

		std::string messageType = message["messagetype"].as_string();
		std::string from = message["from"].as_string();
		std::string content = message["content"].as_string();
		std::string conversationLink = message["conversationLink"].as_string();
		int emoteOffset = message["skypeemoteoffset"].as_int();
		time_t timestamp = IsoToUnixTime(message["composetime"].as_string().c_str());
		CMStringA skypename(UrlToSkypename(from.c_str()));

		bool isEdited = message["skypeeditedid"];

		MCONTACT hContact = FindContact(UrlToSkypename(conversationLink.c_str()));

		if (timestamp > db_get_dw(hContact, m_szModuleName, "LastMsgTime", 0))
			db_set_dw(hContact, m_szModuleName, "LastMsgTime", (DWORD)timestamp);

		DWORD iFlags = DBEF_UTF;

		if (!markAllAsUnread)
			iFlags |= DBEF_READ;

		if (IsMe(skypename))
			iFlags |= DBEF_SENT;

		if (strstr(conversationLink.c_str(), "/8:")) {
			if (messageType == "Text" || messageType == "RichText") {
				ptrA szMessage(messageType == "RichText" ? RemoveHtml(content.c_str()) : mir_strdup(content.c_str()));
				MEVENT dbevent = GetMessageFromDb(szMessageId);
				if (isEdited && dbevent != NULL)
					EditEvent(hContact, dbevent, szMessage, timestamp);
				else
					AddDbEvent(emoteOffset == 0 ? EVENTTYPE_MESSAGE : SKYPE_DB_EVENT_TYPE_ACTION, hContact, timestamp, iFlags, &szMessage[emoteOffset], szMessageId);
			}
			else if (messageType == "Event/Call") {
				AddDbEvent(SKYPE_DB_EVENT_TYPE_CALL_INFO, hContact, timestamp, iFlags, content.c_str(), szMessageId);
			}
			else if (messageType == "RichText/Files") {
				AddDbEvent(SKYPE_DB_EVENT_TYPE_FILETRANSFER_INFO, hContact, timestamp, iFlags, content.c_str(), szMessageId);
			}
			else if (messageType == "RichText/UriObject") {
				AddDbEvent(SKYPE_DB_EVENT_TYPE_URIOBJ, hContact, timestamp, iFlags, content.c_str(), szMessageId);
			}
			else if (messageType == "RichText/Contacts") {
				ProcessContactRecv(hContact, timestamp, content.c_str(), szMessageId);
			}
			else if (messageType == "RichText/Media_Album") {
				// do nothing
			}
			else {
				AddDbEvent(SKYPE_DB_EVENT_TYPE_UNKNOWN, hContact, timestamp, iFlags, content.c_str(), szMessageId);
			}
		}
		else if (conversationLink.find("/19:") != -1) {
			CMStringA chatname(UrlToSkypename(conversationLink.c_str()));
			ptrA szMessage(messageType == "RichText" ? RemoveHtml(content.c_str()) : mir_strdup(content.c_str()));
			if (messageType == "Text" || messageType == "RichText") {
				AddMessageToChat(chatname, skypename, szMessage, emoteOffset != NULL, emoteOffset, timestamp, true);
			}
		}
	}
}

INT_PTR CSkypeProto::GetContactHistory(WPARAM hContact, LPARAM)
{
	PushRequest(new GetHistoryRequest(getId(hContact), 100, false, 0, this), &CSkypeProto::OnGetServerHistory);
	return 0;
}

void CSkypeProto::OnSyncHistory(const NETLIBHTTPREQUEST *response)
{
	JsonReply reply(response);
	if (reply.error())
		return;

	auto &root = reply.data();
	const JSONNode &metadata = root["_metadata"];
	const JSONNode &conversations = root["conversations"].as_array();

	int totalCount = metadata["totalCount"].as_int();
	std::string syncState = metadata["syncState"].as_string();

	if (totalCount >= 99 || conversations.size() >= 99)
		PushRequest(new SyncHistoryFirstRequest(syncState.c_str(), this), &CSkypeProto::OnSyncHistory);

	for (auto &conversation : conversations) {
		const JSONNode &lastMessage = conversation["lastMessage"];
		if (lastMessage) {
			std::string strConversationLink = lastMessage["conversationLink"].as_string();

			if (strConversationLink.find("/8:") != -1) {
				CMStringA szSkypename = UrlToSkypename(strConversationLink.c_str());
				time_t composeTime(IsoToUnixTime(lastMessage["composetime"].as_string().c_str()));

				MCONTACT hContact = FindContact(szSkypename);
				if (hContact != NULL) {
					if (getDword(hContact, "LastMsgTime", 0) < composeTime) {
						PushRequest(new GetHistoryRequest(szSkypename, 100, false, 0, this), &CSkypeProto::OnGetServerHistory);
					}
				}
			}
		}
	}

	m_bHistorySynced = true;
}
