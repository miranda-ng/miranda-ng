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
		if (conversationLink != NULL && strstr(conversationLink, "/8:"))
		{
			int emoteOffset = json_as_int(json_get(message, "skypeemoteoffset"));

			int flags = DBEF_UTF | DBEF_READ;

			ptrA skypename(ContactUrlToName(from));

			bool isMe = IsMe(skypename);
			if (isMe)
				flags |= DBEF_SENT;

			MCONTACT hContact = IsMe(skypename)
				? FindContact(ptrA(ContactUrlToName(conversationLink)))
				: FindContact(skypename);

			ptrA message(RemoveHtml(content));

			AddMessageToDb(hContact, timestamp, flags, clientMsgId, message, emoteOffset);
		}
	}
}

INT_PTR CSkypeProto::GetContactHistory(WPARAM hContact, LPARAM lParam)
{
	PushRequest(new GetHistoryRequest(ptrA(getStringA("registrationToken")), ptrA(db_get_sa(hContact, m_szModuleName, SKYPE_SETTINGS_ID)), ptrA(getStringA("Server"))), &CSkypeProto::OnGetServerHistory);
	return 0;
}

void CSkypeProto::SyncHistory()
{
	PushRequest(new SyncHistoryFirstRequest(ptrA(getStringA("registrationToken")), ptrA(getStringA("Server"))), &CSkypeProto::OnSyncHistory);
}

void CSkypeProto::OnSyncHistory(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;
	JSONROOT root(response->pData);
	if (root == NULL)
		return;
	JSONNODE *conversations = json_as_array(json_get(root, "conversations"));
	for (size_t i = 0; i < json_size(conversations); i++)
	{
		JSONNODE *conversation = json_at(conversations, i);
		JSONNODE *lastMessage = json_get(conversation, "lastMessage");
		if (lastMessage == NULL)
			continue;

		char *clientMsgId = mir_t2a(json_as_string(json_get(lastMessage, "clientmessageid")));
		char *conversationLink = mir_t2a(json_as_string(json_get(lastMessage, "conversationLink")));
		time_t composeTime(IsoToUnixTime(ptrT(json_as_string(json_get(lastMessage, "conversationLink")))));

		ptrA skypename(ContactUrlToName(conversationLink));
		if (skypename == NULL)
			continue;
		MCONTACT hContact = FindContact(skypename);
		if (hContact == NULL && !IsMe(skypename))
			hContact = AddContact(skypename, true);
		if (GetMessageFromDb(hContact, clientMsgId, composeTime) == NULL)
			PushRequest(new GetHistoryRequest(ptrA(getStringA("registrationToken")), skypename, ptrA(getStringA("Server"))), &CSkypeProto::OnGetServerHistory);
	}
}