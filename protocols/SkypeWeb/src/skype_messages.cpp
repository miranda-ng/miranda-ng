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
int CSkypeProto::OnReceiveMessage(MCONTACT hContact, const char *szContent, const char *szMessageId, time_t timestamp,  int emoteOffset, bool isRead)
{
	PROTORECVEVENT recv = { 0 };
	recv.timestamp = timestamp;
	recv.szMessage = mir_strdup(szContent);
	recv.lParam = emoteOffset;
	recv.pCustomData = (void*)mir_strdup(szMessageId);
	recv.cbCustomDataSize = (DWORD)mir_strlen(szMessageId);

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
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, NULL, (LPARAM)Translate("You cannot send when you are offline."));
		return 0;
	}

	SendMessageParam *param = new SendMessageParam();
	param->hContact = hContact;
	param->hMessage = time(NULL);

	ptrA username(getStringA(hContact, "Skypename"));

	if (strncmp(szMessage, "/me ", 4) == 0)
		SendRequest(new SendActionRequest(username, param->hMessage, &szMessage[4], li), &CSkypeProto::OnMessageSent, param);
	else
		SendRequest(new SendMessageRequest(username, param->hMessage, szMessage, li), &CSkypeProto::OnMessageSent, param);

	{
		mir_cslock lck(m_lckOutMessagesList);
		m_OutMessages.insert((void*)param->hMessage);
	}
	return param->hMessage;
}

void CSkypeProto::OnMessageSent(const NETLIBHTTPREQUEST *response, void *arg)
{
	SendMessageParam *param = (SendMessageParam*)arg;
	MCONTACT hContact = param->hContact;
	HANDLE hMessage = (HANDLE)param->hMessage;
	delete param;

	if (response != NULL)
	{
		if (response->resultCode == 201)
		{
			if (m_OutMessages.getIndex(hMessage) != -1)
			{
				if (response->pData != NULL)
				{
					JSONNode jRoot = JSONNode::parse(response->pData);
					auto it = m_mpOutMessages.find(hMessage);
					if (it == m_mpOutMessages.end())
					{
						m_mpOutMessages[hMessage] = std::stoull(jRoot["OriginalArrivalTime"].as_string()) / 1000;
					}
				}
				ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, hMessage, 0);
				{
					mir_cslock lck(m_lckOutMessagesList);
					m_OutMessages.remove(hMessage);
				}
			}
		}
		else
		{
			std::string strError = Translate("Unknown error!");

			if (response->pData != NULL)
			{
				JSONNode jRoot = JSONNode::parse(response->pData);
				const JSONNode &jErr = jRoot["errorCode"];

				if(jErr)
				{
					strError = jErr.as_string();
				}
			}

			ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, hMessage, (LPARAM)strError.c_str());
		}
	}
	else
	{
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, hMessage, (LPARAM)(Translate("Network error!")));
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

	auto it = m_mpOutMessages.find((HANDLE)evt->seq);
	if (it != m_mpOutMessages.end())
	{
		evt->dbei->timestamp = it->second;
		m_mpOutMessages.erase(it);
	}

	return 0;
}

/* MESSAGE EVENT */

void CSkypeProto::OnPrivateMessageEvent(const JSONNode &node)
{
	CMStringA szMessageId = node["clientmessageid"] ? node["clientmessageid"].as_string().c_str() : node["skypeeditedid"].as_string().c_str();
	CMStringA szConversationName(UrlToSkypename(node["conversationLink"].as_string().c_str()));
	CMStringA szFromSkypename(UrlToSkypename(node["from"].as_string().c_str()));

	std::string strMessageType = node["messagetype"].as_string();
	std::string strContent = node["content"].as_string();
	ptrA szClearedContent(RemoveHtml(strContent.c_str()));

	bool bEdited = node["skypeeditedid"];
	time_t timestamp =  IsoToUnixTime(node["composetime"].as_string().c_str());

	int nEmoteOffset = atoi(node["skypeemoteoffset"].as_string().c_str());

	MCONTACT hContact = AddContact(szConversationName, true);

	if (HistorySynced)
		db_set_dw(hContact, m_szModuleName, "LastMsgTime", (DWORD)timestamp);

	if (strMessageType == "Control/Typing")
	{
		CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_INFINITE);
	}
	else if (strMessageType == "Control/ClearTyping")
	{
		CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);
	}
	else if (strMessageType == "Text" || strMessageType == "RichText")
	{
		if (IsMe(szFromSkypename))
		{
			HANDLE hMessage = (HANDLE)std::stoull(szMessageId.GetString());
			if (m_OutMessages.getIndex(hMessage) != -1)
			{
				auto it = m_mpOutMessages.find(hMessage);
				if (it == m_mpOutMessages.end())
				{
					m_mpOutMessages[hMessage] = timestamp;
				}
				ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, hMessage, 0);
				{
					mir_cslock lck(m_lckOutMessagesList);
					m_OutMessages.remove(hMessage);
				}
			}
			else
			{
				AddDbEvent(nEmoteOffset == 0 ? EVENTTYPE_MESSAGE : SKYPE_DB_EVENT_TYPE_ACTION, hContact, 
					timestamp, DBEF_UTF | DBEF_SENT, &szClearedContent[nEmoteOffset], szMessageId);
			}
		}
		else
		{
			CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);
			MEVENT hDbEvent = GetMessageFromDb(hContact, szMessageId);

			if (bEdited && hDbEvent != NULL)
			{
				AppendDBEvent(hContact, hDbEvent, szClearedContent, szMessageId, timestamp);
			}
			else 
			{
				OnReceiveMessage(hContact, szClearedContent, szMessageId, timestamp, nEmoteOffset);
			}
		}
	}
	else if (strMessageType == "Event/Call")
	{
		AddDbEvent(SKYPE_DB_EVENT_TYPE_CALL_INFO, hContact, timestamp, DBEF_UTF, strContent.c_str(), szMessageId);
	}
	else if (strMessageType == "RichText/Files")
	{		
		AddDbEvent(SKYPE_DB_EVENT_TYPE_FILETRANSFER_INFO, hContact, timestamp, DBEF_UTF, strContent.c_str(), szMessageId);
	}
	else if (strMessageType == "RichText/UriObject")
	{
		AddDbEvent(SKYPE_DB_EVENT_TYPE_URIOBJ, hContact, timestamp, DBEF_UTF, strContent.c_str(), szMessageId);
	}
	//else if (messageType == "Event/SkypeVideoMessage") {}
	//else if (messageType.c_str() == "RichText/Contacts") {}
	//else if (messageType.c_str() == "RichText/Location") {}
	else
	{
		AddDbEvent(SKYPE_DB_EVENT_TYPE_UNKNOWN, hContact, timestamp, DBEF_UTF, strContent.c_str(), szMessageId);
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
		PushRequest(new MarkMessageReadRequest(username, timestamp, timestamp, false, li));
}
