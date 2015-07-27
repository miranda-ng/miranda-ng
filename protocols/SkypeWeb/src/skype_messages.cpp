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

#include "stdafx.h"

/* MESSAGE RECEIVING */

// incoming message flow
int CSkypeProto::OnReceiveMessage(const char *messageId, const char *url, time_t timestamp, char *content, int emoteOffset, bool isRead)
{
	CMStringA skypename(UrlToSkypename(url));
	debugLogA("Incoming message from %s", skypename);

	MCONTACT hContact = AddContact(skypename, true);
	if (hContact == NULL)
		return 0;

	PROTORECVEVENT recv = { 0 };
	recv.timestamp = timestamp;
	recv.szMessage = content;
	recv.lParam = emoteOffset;
	recv.pCustomData = (void*)messageId;
	recv.cbCustomDataSize = (DWORD)mir_strlen(messageId);

	if (isRead)
		recv.flags |= PREF_CREATEREAD;

	return ProtoChainRecvMsg(hContact, &recv);
}

/* MESSAGE SENDING */

struct SendMessageParam
{
	MCONTACT hContact;
	LONGLONG hMessage;
};

// outcoming message flow
int CSkypeProto::OnSendMessage(MCONTACT hContact, int, const char *szMessage)
{
	if (!IsOnline())
	{
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, NULL, (LPARAM)"You cannot send when you are offline.");
		return 0;
	}

	SendMessageParam *param = new SendMessageParam();
	param->hContact = hContact;
	param->hMessage = time(NULL);

	ptrA username(getStringA(hContact, "Skypename"));

	if (strncmp(szMessage, "/me ", 4) == 0)
		SendRequest(new SendActionRequest(m_szRegToken, username, m_szSelfSkypeName, param->hMessage, &szMessage[4], m_szServer), &CSkypeProto::OnMessageSent, param);
	else
		SendRequest(new SendMessageRequest(m_szRegToken, username, param->hMessage, szMessage, m_szServer), &CSkypeProto::OnMessageSent, param);

	return param->hMessage;
}

void CSkypeProto::OnMessageSent(const NETLIBHTTPREQUEST *response, void *arg)
{
	SendMessageParam *param = (SendMessageParam*)arg;
	MCONTACT hContact = param->hContact;
	HANDLE hMessage = (HANDLE)param->hMessage;
	delete param;

	if (response == NULL || response->resultCode != 201)
	{
		std::string error("Unknown error");
		if (response && response->pData != NULL)
		{
			JSONNode root = JSONNode::parse(response->pData);
			const JSONNode &node = root["errorCode"];
			if (node)
				error = node.as_string();
		}
		ptrT username(getTStringA(hContact, "Skypename"));
		debugLogA(__FUNCTION__": failed to send message for %s (%s)", username, error.c_str());
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, hMessage, (LPARAM)error.c_str());
	}
}

// preparing message/action to writing into db
int CSkypeProto::OnPreCreateMessage(WPARAM, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent*)lParam;
	if (mir_strcmp(GetContactProto(evt->hContact), m_szModuleName))
		return 0;

	char *message = (char*)evt->dbei->pBlob;
	if (strncmp(message, "/me ", 4) == 0)
	{
		evt->dbei->cbBlob = evt->dbei->cbBlob - 4;
		memmove(evt->dbei->pBlob, &evt->dbei->pBlob[4], evt->dbei->cbBlob);
		evt->dbei->eventType = SKYPE_DB_EVENT_TYPE_ACTION;
	}

	CMStringA messageId(FORMAT, "%d", evt->seq);
	evt->dbei->pBlob = (PBYTE)mir_realloc(evt->dbei->pBlob, evt->dbei->cbBlob + messageId.GetLength());
	memcpy(&evt->dbei->pBlob[evt->dbei->cbBlob], messageId, messageId.GetLength());
	evt->dbei->cbBlob += messageId.GetLength();

	return 0;
}

/* MESSAGE EVENT */

void CSkypeProto::OnPrivateMessageEvent(const JSONNode &node)
{
	CMStringA szMessageId = node["clientmessageid"] ? node["clientmessageid"].as_string().c_str() : node["skypeeditedid"].as_string().c_str();

	bool isEdited = node["skypeeditedid"];
	time_t timestamp = getByte("UseLocalTime", 0) ? time(NULL) : IsoToUnixTime(node["composetime"].as_string().c_str());

	CMStringA skypename(UrlToSkypename(node["conversationLink"].as_string().c_str()));
	CMStringA from(UrlToSkypename(node["from"].as_string().c_str()));

	std::string content = node["content"].as_string();
	int emoteOffset = node["skypeemoteoffset"].as_int();

	ptrA message(RemoveHtml(content.c_str()));

	std::string messageType = node["messagetype"].as_string();
	MCONTACT hContact = AddContact(skypename, true);

	if (HistorySynced)
		db_set_dw(hContact, m_szModuleName, "LastMsgTime", (DWORD)timestamp);

	if (messageType == "Control/Typing")
		CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_INFINITE);

	else if (messageType == "Control/ClearTyping")
		CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);

	else if (messageType == "Text" || messageType == "RichText")
	{
		if (IsMe(from))
		{
			long hMessage = atol(szMessageId);
			ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)hMessage, 0);
			debugLogA(__FUNCTION__" timestamp = %d clientmsgid = %s", timestamp, szMessageId);

			AddDbEvent(emoteOffset == 0 ? EVENTTYPE_MESSAGE : SKYPE_DB_EVENT_TYPE_ACTION, hContact, 
				timestamp, DBEF_UTF | DBEF_SENT, &message[emoteOffset], szMessageId);

			return;
		}
		CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);

		debugLogA(__FUNCTION__" timestamp = %d clientmsgid = %s", timestamp, szMessageId);
		MEVENT dbevent = GetMessageFromDb(hContact, szMessageId);
		if (isEdited && dbevent != NULL)
		{
			AppendDBEvent(hContact, dbevent, message, szMessageId, timestamp);
		}
		else OnReceiveMessage(szMessageId, node["conversationLink"].as_string().c_str(), timestamp, message, emoteOffset);
	}
	else if (messageType == "Event/SkypeVideoMessage") {}
	else if (messageType == "Event/Call")
	{
		AddDbEvent(SKYPE_DB_EVENT_TYPE_CALL_INFO, hContact, timestamp, DBEF_UTF, content.c_str(), szMessageId);
	}
	else if (messageType == "RichText/Files")
	{		
		AddDbEvent(SKYPE_DB_EVENT_TYPE_FILETRANSFER_INFO, hContact, timestamp, DBEF_UTF, content.c_str(), szMessageId);
	}
	else if (messageType == "RichText/UriObject")
	{
		AddDbEvent(SKYPE_DB_EVENT_TYPE_URIOBJ, hContact, timestamp, DBEF_UTF, content.c_str(), szMessageId);
	}
	//else if (!mir_strcmpi(messageType.c_str(), "RichText/Contacts")) {}
	//else if (!mir_strcmpi(messageType.c_str(), "RichText/Location")) {}
	else
	{
		AddDbEvent(SKYPE_DB_EVENT_TYPE_UNKNOWN, hContact, timestamp, DBEF_UTF, content.c_str(), szMessageId);
	}
}

int CSkypeProto::OnDbEventRead(WPARAM hContact, LPARAM hDbEvent)
{
	debugLogA(__FUNCTION__);
	if (IsOnline() && !isChatRoom(hContact) && !mir_strcmp(GetContactProto(hContact), m_szModuleName))
		MarkMessagesRead(hContact, hDbEvent);
	return 0;
}

void CSkypeProto::MarkMessagesRead(MCONTACT hContact, MEVENT hDbEvent)
{
	debugLogA(__FUNCTION__);
	ptrA username(db_get_sa(hContact, m_szModuleName, SKYPE_SETTINGS_ID));
	
	DBEVENTINFO dbei = { sizeof(dbei) };
	db_event_get(hDbEvent, &dbei);
	time_t timestamp = dbei.timestamp;

	if(db_get_dw(hContact, m_szModuleName, "LastMsgTime", 0) > (timestamp - 300))
		PushRequest(new MarkMessageReadRequest(username, m_szRegToken, timestamp, timestamp, false, m_szServer));
}
