/*
Copyright (c) 2015-24 Miranda NG team (https://miranda-ng.org)

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

/* MESSAGE SENDING */

// outcoming message flow
int CSkypeProto::SendMsg(MCONTACT hContact, MEVENT, const char *szMessage)
{
	if (!IsOnline())
		return -1;

	SendMessageParam *param = new SendMessageParam();
	param->hContact = hContact;
	Utils_GetRandom(&param->hMessage, sizeof(param->hMessage));
	param->hMessage &= ~0x80000000;

	CMStringA id(getId(hContact));

	AsyncHttpRequest *pReq;
	if (strncmp(szMessage, "/me ", 4) == 0)
		pReq = new SendActionRequest(id, param->hMessage, &szMessage[4], this);
	else
		pReq = new SendMessageRequest(id, param->hMessage, szMessage);
	pReq->pUserInfo = param;
	PushRequest(pReq);

	mir_cslock lck(m_lckOutMessagesList);
	m_OutMessages.insert((void*)param->hMessage);
	return param->hMessage;
}

void CSkypeProto::OnMessageSent(MHttpResponse *response, AsyncHttpRequest *pRequest)
{
	auto *param = (SendMessageParam*)pRequest->pUserInfo;
	MCONTACT hContact = param->hContact;
	HANDLE hMessage = (HANDLE)param->hMessage;
	delete param;

	if (response != nullptr) {
		if (response->resultCode != 201) {
			std::string strError = Translate("Unknown error!");

			if (!response->body.IsEmpty()) {
				JSONNode jRoot = JSONNode::parse(response->body);
				const JSONNode &jErr = jRoot["errorCode"];
				if (jErr)
					strError = jErr.as_string();
			}

			ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, hMessage, _A2T(strError.c_str()));
		}
	}
	else ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, hMessage, (LPARAM)TranslateT("Network error!"));
}

// preparing message/action to be written into db
int CSkypeProto::OnPreCreateMessage(WPARAM, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent*)lParam;
	if (mir_strcmp(Proto_GetBaseAccountName(evt->hContact), m_szModuleName))
		return 0;

	char *message = (char*)evt->dbei->pBlob;
	if (strncmp(message, "/me ", 4) == 0) {
		evt->dbei->cbBlob = evt->dbei->cbBlob - 4;
		memmove(evt->dbei->pBlob, &evt->dbei->pBlob[4], evt->dbei->cbBlob);
		evt->dbei->eventType = SKYPE_DB_EVENT_TYPE_ACTION;
	}
	return 0;
}

/* MESSAGE EVENT */

void CSkypeProto::ProcessNewMessage(const JSONNode &node)
{
	int iUserType;
	UrlToSkypeId(node["conversationLink"].as_string().c_str(), &iUserType);

	CMStringA szMessageId = node["clientmessageid"] ? node["clientmessageid"].as_mstring() : node["skypeeditedid"].as_mstring();
	CMStringA szConversationName(UrlToSkypeId(node["conversationLink"].as_string().c_str()));
	CMStringA szFromSkypename(UrlToSkypeId(node["from"].as_mstring()));

	CMStringW wszContent = node["content"].as_mstring();

	std::string strMessageType = node["messagetype"].as_string();
	if (strMessageType == "RichText")
		wszContent = RemoveHtml(wszContent);

	time_t timestamp = time(0); // fuck the server time, we need to place events in the order of our local time

	int nEmoteOffset = node["skypeemoteoffset"].as_int();

	MCONTACT hContact = AddContact(szConversationName, nullptr, true);

	if (m_bHistorySynced)
		setDword(hContact, "LastMsgTime", timestamp);

	uint32_t dwFlags = DBEF_UTF;
	if (IsMe(szFromSkypename))
		dwFlags |= DBEF_SENT;

	if (strMessageType == "Control/Typing") {
		CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_INFINITE);
		return;
	}
	if (strMessageType == "Control/ClearTyping") {
		CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);
		return;
	}
	
	T2Utf szMsg(wszContent);

	DB::EventInfo dbei(db_event_getById(m_szModuleName, szMessageId));
	dbei.timestamp = timestamp;
	dbei.pBlob = szMsg;
	dbei.cbBlob = (uint32_t)mir_strlen(szMsg);
	dbei.szId = szMessageId;
	if (iUserType == 19)
		dbei.szUserId = szFromSkypename;

	if (strMessageType == "Text" || strMessageType == "RichText") {
		if (IsMe(szFromSkypename)) {
			HANDLE hMessage = (HANDLE)atoi(szMessageId);
			if (m_OutMessages.getIndex(hMessage) != -1) {
				ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, hMessage, (LPARAM)szMessageId.c_str());

				mir_cslock lck(m_lckOutMessagesList);
				m_OutMessages.remove(hMessage);
				return;
			}
		}
		else CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);

		dbei.eventType = nEmoteOffset == 0 ? EVENTTYPE_MESSAGE : SKYPE_DB_EVENT_TYPE_ACTION;
	}
	else if (strMessageType == "Event/Call") {
		dbei.eventType = SKYPE_DB_EVENT_TYPE_CALL_INFO;
	}
	else if (strMessageType == "RichText/Files") {
		dbei.eventType = SKYPE_DB_EVENT_TYPE_FILETRANSFER_INFO;
	}
	else if (strMessageType == "RichText/UriObject") {
		dbei.eventType = SKYPE_DB_EVENT_TYPE_URIOBJ;
	}
	else if (strMessageType == "RichText/Contacts") {
		ProcessContactRecv(hContact, timestamp, T2Utf(wszContent), szMessageId);
		return;
	}
	else if (strMessageType == "RichText/Media_FlikMsg") {
		dbei.eventType = SKYPE_DB_EVENT_TYPE_MOJI;
	}
	else if (strMessageType == "RichText/Media_GenericFile") {
		dbei.eventType = SKYPE_DB_EVENT_TYPE_FILE;
	}
	else if (strMessageType == "RichText/Media_Album") {
		// do nothing
	}
	else if (iUserType == 19) {
		OnChatEvent(node);
		return;
	}
	else {
		dbei.eventType = SKYPE_DB_EVENT_TYPE_UNKNOWN;
	}

	if (dbei) {
		db_event_edit(dbei.getEvent(), &dbei, true);
		dbei.pBlob = nullptr;
	}
	else ProtoChainRecvMsg(hContact, dbei);
}

void CSkypeProto::OnMarkRead(MCONTACT hContact, MEVENT hDbEvent)
{
	if (IsOnline() && !isChatRoom(hContact))
		MarkMessagesRead(hContact, hDbEvent);
}

void CSkypeProto::MarkMessagesRead(MCONTACT hContact, MEVENT hDbEvent)
{
	debugLogA(__FUNCTION__);

	DBEVENTINFO dbei = {};
	db_event_get(hDbEvent, &dbei);
	time_t timestamp = dbei.timestamp;

	if (getDword(hContact, "LastMsgTime") > (timestamp - 300))
		PushRequest(new MarkMessageReadRequest(getId(hContact), timestamp, timestamp));
}

void CSkypeProto::ProcessContactRecv(MCONTACT hContact, time_t timestamp, const char *szContent, const char *szMessageId)
{
	TiXmlDocument doc;
	if (0 != doc.Parse(szContent))
		return;

	auto *xmlNode = doc.FirstChildElement("contacts");
	if (xmlNode == nullptr)
		return;

	int nCount = 0;
	for (auto *it : TiXmlEnum(xmlNode)) {
		UNREFERENCED_PARAMETER(it);
		nCount++;
	}

	PROTOSEARCHRESULT **psr = (PROTOSEARCHRESULT**)mir_calloc(sizeof(PROTOSEARCHRESULT*) * nCount);
	
	nCount = 0;
	for (auto *xmlContact : TiXmlFilter(xmlNode, "c")) {
		psr[nCount] = (PROTOSEARCHRESULT*)mir_calloc(sizeof(PROTOSEARCHRESULT));
		psr[nCount]->cbSize = sizeof(psr);
		psr[nCount]->id.a = mir_strdup(xmlContact->Attribute("s"));
		nCount++;
	}

	if (nCount) {
		DB::EventInfo dbei;
		dbei.timestamp = (uint32_t)timestamp;
		dbei.pBlob = (char*)psr;
		dbei.cbBlob = nCount;
		dbei.szId = szMessageId;

		ProtoChainRecv(hContact, PSR_CONTACTS, 0, (LPARAM)&dbei);
		for (int i = 0; i < nCount; i++) {
			mir_free(psr[i]->id.a);
			mir_free(psr[i]);
		}
	}
	mir_free(psr);
}
