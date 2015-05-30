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
	ptrA skypename(ContactUrlToName(url));
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

	debugLogA(__FUNCTION__ " clientmsgid = %d", param->hMessage);

	if (strncmp(szMessage, "/me ", 4) == 0)
		SendRequest(new SendActionRequest(RegToken, SelfSkypeName, param->hMessage, &szMessage[4], Server), &CSkypeProto::OnMessageSent, param);
	else
		SendRequest(new SendMessageRequest(RegToken, username, param->hMessage, szMessage, Server), &CSkypeProto::OnMessageSent, param);

	return param->hMessage;
}

void CSkypeProto::OnMessageSent(const NETLIBHTTPREQUEST *response, void *arg)
{
	SendMessageParam *param = (SendMessageParam*)arg;
	MCONTACT hContact = param->hContact;
	HANDLE hMessage = (HANDLE)param->hMessage;
	delete param;

	if (response == NULL || (response->resultCode != 200 && response->resultCode != 201))
	{
		std::string error("Unknown error");
		if (response)
		{
			JSONNode root = JSONNode::parse(response->pData);
			const JSONNode &node = root["errorCode"];
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
	std::string clientMsgId = node["clientmessageid"].as_string();
	std::string skypeEditedId = node["skypeeditedid"].as_string();

	bool isEdited = node["skypeeditedid"];

	std::string composeTime = node["composetime"].as_string();
	time_t timestamp = getByte("UseLocalTime", 0) ? time(NULL) : IsoToUnixTime(composeTime.c_str());

	std::string conversationLink = node["conversationLink"].as_string();
	std::string fromLink = node["from"].as_string();

	ptrA skypename(ContactUrlToName(conversationLink.c_str()));
	ptrA from(ContactUrlToName(fromLink.c_str()));

	std::string content = node["content"].as_string();
	int emoteOffset = node["skypeemoteoffset"].as_int();

	ptrA message(RemoveHtml(content.c_str()));

	std::string messageType= node["messagetype"].as_string();
	MCONTACT hContact = AddContact(skypename, true);

	if (HistorySynced)
		db_set_dw(hContact, m_szModuleName, "LastMsgTime", (DWORD)timestamp);

	if (!mir_strcmpi(messageType.c_str(), "Control/Typing"))
		CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_INFINITE);

	else if (!mir_strcmpi(messageType.c_str(), "Control/ClearTyping"))
		CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);

	else if (!mir_strcmpi(messageType.c_str(), "Text") || !mir_strcmpi(messageType.c_str(), "RichText"))
	{
		if (IsMe(from))
		{
			int hMessage = atoi(clientMsgId.c_str());
			ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)hMessage, 0);
			debugLogA(__FUNCTION__" timestamp = %d clientmsgid = %s", timestamp, clientMsgId);
			AddMessageToDb(hContact, timestamp, DBEF_UTF | DBEF_SENT, clientMsgId.c_str(), message, emoteOffset);
			return;
		}
		debugLogA(__FUNCTION__" timestamp = %d clientmsgid = %s", timestamp, clientMsgId);
		MEVENT dbevent = GetMessageFromDb(hContact, skypeEditedId.c_str());
		if (isEdited && dbevent != NULL)
		{
			DBEVENTINFO dbei = { sizeof(dbei) };
			CMStringA msg;
			dbei.cbBlob = db_event_getBlobSize(dbevent);
			mir_ptr<BYTE> blob((PBYTE)mir_alloc(dbei.cbBlob));
			dbei.pBlob = blob;

			db_event_get(dbevent, &dbei);

			time_t dbEventTimestamp = dbei.timestamp;

			ptrA dbMsgText(NEWSTR_ALLOCA((char *)dbei.pBlob));

			TCHAR time[64];
			_locale_t locale = _create_locale(LC_ALL, "");
			_tcsftime_l(time, sizeof(time), L"%X %x", localtime(&timestamp), locale);
			_free_locale(locale);

			msg.AppendFormat("%s\n%s %s:\n%s", dbMsgText, Translate("Edited at"), _T2A(time), message);
			db_event_delete(hContact, dbevent);
			AddMessageToDb(hContact, dbEventTimestamp, DBEF_UTF, skypeEditedId.c_str(), msg.GetBuffer());
		}
		else OnReceiveMessage(clientMsgId.c_str(), conversationLink.c_str(), timestamp, message, emoteOffset);
	}
	else if (!mir_strcmpi(messageType.c_str(), "Event/SkypeVideoMessage")) {}
	else if (!mir_strcmpi(messageType.c_str(), "Event/Call"))
	{
		//content=<partlist type="ended" alt=""><part identity="username"><name>user name</name><duration>6</duration></part>
		//<part identity="echo123"><name>Echo / Sound Test Service</name><duration>6</duration></part></partlist>
		//content=<partlist type="started" alt=""><part identity="username"><name>user name</name></part></partlist>
		int iType = 3, iDuration = 0;
		HXML xml = xi.parseString(ptrT(mir_a2t(content.c_str())), 0, _T("partlist"));
		if (xml != NULL)
		{

			ptrA type(mir_t2a(xi.getAttrValue(xml, _T("type"))));

			if (!mir_strcmpi(type, "ended")) iType = 0;
			else if (!mir_strcmpi(type, "started")) iType = 1;

			HXML xmlNode = xi.getChildByPath(xml, _T("part"), 0);
			HXML duration = xmlNode == NULL ? NULL : xi.getChildByPath(xmlNode, _T("duration"), 0);
			iDuration = duration != NULL ? atoi(mir_t2a(xi.getText(duration))) : NULL;

			xi.destroyNode(xml);
		}
		CMStringA text = "";
		if (iType == 1)
			text.Append(Translate("Call started"));
		else if (iType == 0)
		{
			CMStringA chours = "", cmins = "", csec = "";
			int hours = 0, mins = 0, sec = 0;
			if (iDuration != NULL)
			{
				hours = iDuration / 3600;
				mins = iDuration / 60;
				sec = iDuration % 60;
			}
			else
				hours = mins = sec = 0;

			chours.AppendFormat(hours < 10 ? "0%d" : "%d", hours);
			cmins.AppendFormat(mins < 10 ? "0%d" : "%d", mins);
			csec.AppendFormat(sec < 10 ? "0%d" : "%d", sec);
			text.AppendFormat("%s\n%s: %s:%s:%s", Translate("Call ended"), Translate("Duration"), chours, cmins, csec);
		}

		int flags = DBEF_UTF;
		if (IsMe(from)) flags |= DBEF_SENT;

		AddCallInfoToDb(hContact, timestamp, flags, clientMsgId.c_str(), text.GetBuffer());
	}
	else if (!mir_strcmpi(messageType.c_str(), "RichText/Files"))
	{
		//content=<files alt="отправил (-а) файл &quot;run.bat&quot;"><file size="97" index="0" tid="4197760077">run.bat</file></files>
		HXML xml = xi.parseString(ptrT(mir_a2t(content.c_str())), 0, _T("files"));
		if (xml != NULL)
		{
			for (int i = 0; i < xi.getChildCount(xml); i++)
			{
				int fileSize;
				HXML xmlNode = xi.getNthChild(xml, L"file", i);
				if (xmlNode == NULL)
					break;
				fileSize = atoi(_T2A(xi.getAttrValue(xmlNode, L"size")));
				ptrA fileName(mir_t2a(xi.getText(xmlNode)));
				if (fileName == NULL || fileSize == NULL)
					continue;

				CMStringA msg(FORMAT, "%s:\n\t%s: %s\n\t%s: %d %s", Translate("File transfer"), Translate("File name"), fileName, Translate("Size"), fileSize, Translate("bytes"));
				AddMessageToDb(hContact, timestamp, DBEF_UTF | DBEF_READ, clientMsgId.c_str(), msg.GetBuffer());
			}
		}
	}
	else if (!mir_strcmpi(messageType.c_str(), "RichText/Location")) {}
	else if (!mir_strcmpi(messageType.c_str(), "RichText/UriObject"))
	{
		//content=<URIObject type="Picture.1" uri="https://api.asm.skype.com/v1//objects/0-weu-d1-262f0a1ee256d03b8e4b8360d9208834" url_thumbnail="https://api.asm.skype.com/v1//objects/0-weu-d1-262f0a1ee256d03b8e4b8360d9208834/views/imgt1"><Title></Title><Description></Description>Для просмотра этого общего фото перейдите по ссылке: https://api.asm.skype.com/s/i?0-weu-d1-262f0a1ee256d03b8e4b8360d9208834<meta type="photo" originalName="ysd7ZE4BqOg.jpg"/><OriginalName v="ysd7ZE4BqOg.jpg"/></URIObject>
		HXML xml = xi.parseString(ptrT(mir_a2t(content.c_str())), 0, _T("URIObject"));
		if (xml != NULL)
		{
			ptrA url(mir_t2a(xi.getAttrValue(xml, L"uri")));
			ptrA object(ParseUrl(url, "/objects/"));

			CMStringA data(FORMAT, "%s: https://api.asm.skype.com/s/i?%s", Translate("Image"), object);

			AddMessageToDb(hContact, timestamp, DBEF_UTF, clientMsgId.c_str(), data.GetBuffer());
		}
	}
	else if (!mir_strcmpi(messageType.c_str(), "RichText/Contacts")) {}

	//if (clientMsgId && (!mir_strcmpi(messageType, "Text") || !mir_strcmpi(messageType, "RichText")))
	//{
	//	PushRequest(new MarkMessageReadRequest(skypename, RegToken, _ttoi(json_as_string(json_get(node, "id"))), timestamp, false, Server));
	//}
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

	PushRequest(new MarkMessageReadRequest(username, RegToken, timestamp, timestamp, false, Server));
}
