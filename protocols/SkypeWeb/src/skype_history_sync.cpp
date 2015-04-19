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

#include "common.h"

/* HISTORY SYNC */

void CSkypeProto::OnGetServerHistory(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	JSONNODE *metadata = json_get(root, "_metadata");

	int totalCount = json_as_int(json_get(metadata, "totalCount"));
	ptrA syncState(mir_t2a(ptrT(json_as_string(json_get(metadata, "syncState")))));

	if (totalCount >= 99)
		PushRequest(new GetHistoryOnUrlRequest(syncState, RegToken), &CSkypeProto::OnGetServerHistory);

	JSONNODE *conversations = json_as_array(json_get(root, "messages"));
	for (size_t i = 0; i < json_size(conversations); i++)
	{
		JSONNODE *message = json_at(conversations, i);

		ptrA clientMsgId(mir_t2a(ptrT(json_as_string(json_get(message, "clientmessageid")))));
		ptrA skypeEditedId(mir_t2a(ptrT(json_as_string(json_get(message, "skypeeditedid")))));
		ptrA messageType(mir_t2a(ptrT(json_as_string(json_get(message, "messagetype")))));
		ptrA from(mir_t2a(ptrT(json_as_string(json_get(message, "from")))));
		ptrA content(mir_t2a(ptrT(json_as_string(json_get(message, "content")))));
		ptrT composeTime(json_as_string(json_get(message, "composetime")));
		ptrA conversationLink(mir_t2a(ptrT(json_as_string(json_get(message, "conversationLink")))));
		time_t timestamp = IsoToUnixTime(composeTime);
		bool isEdited = (json_get(message, "skypeeditedid") != NULL);
		if (conversationLink != NULL && strstr(conversationLink, "/8:"))
		{
			int emoteOffset = json_as_int(json_get(message, "skypeemoteoffset"));

			int flags = DBEF_UTF | DBEF_READ;

			ptrA skypename(ContactUrlToName(from));

			bool isMe = IsMe(skypename);
			if (isMe)
				flags |= DBEF_SENT;

			MCONTACT hContact = FindContact(ptrA(ContactUrlToName(conversationLink)));

			ptrA message(RemoveHtml(content));
			MEVENT dbevent =  GetMessageFromDb(hContact, skypeEditedId);

			if (isEdited && dbevent != NULL)
			{
				DBEVENTINFO dbei = { sizeof(dbei) };
				CMStringA msg;

				dbei.cbBlob = db_event_getBlobSize(dbevent);
				mir_ptr<BYTE> blob((PBYTE)mir_alloc(dbei.cbBlob));
				dbei.pBlob = blob;

				db_event_get(dbevent, &dbei);
				time_t dbEventTimestamp = dbei.timestamp;
				ptrA dbMsgText((char *)mir_alloc(dbei.cbBlob));

				mir_strcpy(dbMsgText, (char*)dbei.pBlob);

				msg.AppendFormat("%s\n%s [%s]:\n%s", dbMsgText, Translate("Edited at"), ptrA(mir_t2a(composeTime)), message);
				db_event_delete(hContact, dbevent);
				AddMessageToDb(hContact, dbEventTimestamp, flags, clientMsgId, msg.GetBuffer(), emoteOffset);
			}
			else
				AddMessageToDb(hContact, timestamp, flags, clientMsgId, message, emoteOffset);
		}
		else if (conversationLink != NULL && strstr(conversationLink, "/19:"))
		{
			ptrA chatname(ChatUrlToName(conversationLink));
			StartChatRoom(_A2T(chatname), _A2T(chatname));
			if (!mir_strcmpi(messageType, "Text") || !mir_strcmpi(messageType, "RichText"))
			{
				GCDEST gcd = { m_szModuleName, _A2T(chatname), GC_EVENT_MESSAGE };
				GCEVENT gce = { sizeof(GCEVENT), &gcd };
				gce.bIsMe = IsMe(ContactUrlToName(from));
				gce.ptszUID = mir_a2t(ContactUrlToName(from));
				gce.time = timestamp;
				gce.ptszNick = mir_a2t(ContactUrlToName(from));
				gce.ptszText = mir_a2t(content);
				gce.dwFlags = GCEF_NOTNOTIFY;
				CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
			}
		}
	}
}

INT_PTR CSkypeProto::GetContactHistory(WPARAM hContact, LPARAM)
{
	PushRequest(new GetHistoryRequest(RegToken, ptrA(db_get_sa(hContact, m_szModuleName, SKYPE_SETTINGS_ID)), 100, false, 0, Server), &CSkypeProto::OnGetServerHistory);
	return 0;
}

void CSkypeProto::SyncHistory()
{
	PushRequest(new SyncHistoryFirstRequest(RegToken, 100, Server), &CSkypeProto::OnSyncHistory);
}

void CSkypeProto::OnSyncHistory(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;
	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	JSONNODE *metadata = json_get(root, "_metadata");

	int totalCount = json_as_int(json_get(metadata, "totalCount"));
	ptrA syncState(mir_t2a(ptrT(json_as_string(json_get(metadata, "syncState")))));

	if (totalCount >= 99)
		PushRequest(new SyncHistoryFirstRequest(syncState, RegToken), &CSkypeProto::OnSyncHistory);

	JSONNODE *conversations = json_as_array(json_get(root, "conversations"));
	for (size_t i = 0; i < json_size(conversations); i++)
	{
		JSONNODE *conversation = json_at(conversations, i);
		JSONNODE *lastMessage = json_get(conversation, "lastMessage");
		if (json_empty(lastMessage))
			continue;

		char *clientMsgId = mir_t2a(json_as_string(json_get(lastMessage, "clientmessageid")));
		char *skypeEditedId = mir_t2a(json_as_string(json_get(lastMessage, "skypeeditedid")));
		bool isEdited = (skypeEditedId != NULL);
		char *conversationLink = mir_t2a(json_as_string(json_get(lastMessage, "conversationLink")));
		time_t composeTime(IsoToUnixTime(ptrT(json_as_string(json_get(lastMessage, "conversationLink")))));

		bool isChat = false;
		ptrA skypename;

		if (conversationLink != NULL && strstr(conversationLink, "/8:"))
		{
			skypename = ContactUrlToName(conversationLink);
		}
		else if (conversationLink != NULL && strstr(conversationLink, "/19:"))
		{
			skypename = ChatUrlToName(conversationLink);
			isChat = true;
		}
		else 
			continue;

		MCONTACT hContact = isChat ? NULL : AddContact(skypename);

		if (hContact == NULL || GetMessageFromDb(hContact, clientMsgId, composeTime) == NULL)
			PushRequest(new GetHistoryRequest(RegToken, skypename, !isChat ? 100 : 15, isChat, 0,Server), &CSkypeProto::OnGetServerHistory);
	}
}