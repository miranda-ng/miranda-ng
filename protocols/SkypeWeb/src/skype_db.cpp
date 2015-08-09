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

MEVENT CSkypeProto::GetMessageFromDb(MCONTACT hContact, const char *messageId, LONGLONG timestamp)
{
	if (messageId == NULL)
		return NULL;

	timestamp -= 600; // we check events written 10 minutes ago
	size_t messageIdLength = mir_strlen(messageId);

	mir_cslock lock(messageSyncLock);
	for (MEVENT hDbEvent = db_event_last(hContact); hDbEvent; hDbEvent = db_event_prev(hContact, hDbEvent))
	{
		DBEVENTINFO dbei = { sizeof(dbei) };
		dbei.cbBlob = db_event_getBlobSize(hDbEvent);

		if (dbei.cbBlob < messageIdLength)
			continue;

		mir_ptr<BYTE> blob((PBYTE)mir_alloc(dbei.cbBlob));
		dbei.pBlob = blob;
		db_event_get(hDbEvent, &dbei);

		size_t cbLen = mir_strlen((char*)dbei.pBlob);
		if (memcmp(&dbei.pBlob[cbLen + 1], messageId, messageIdLength) == 0)
			return hDbEvent;

		if (dbei.timestamp < timestamp)
			break;
	}

	return NULL;
}

MEVENT CSkypeProto::AddDbEvent(WORD type, MCONTACT hContact, DWORD timestamp, DWORD flags, const char *content, const char *uid)
{
	if (MEVENT hDbEvent = GetMessageFromDb(hContact, uid, timestamp))
		return hDbEvent;
	size_t messageLength = mir_strlen(content) + 1;
	size_t messageIdLength = mir_strlen(uid);
	size_t cbBlob = messageLength + messageIdLength;
	PBYTE pBlob = (PBYTE)mir_alloc(cbBlob);
	memcpy(pBlob, content, messageLength);
	memcpy(pBlob + messageLength, uid, messageIdLength);

	return AddEventToDb(hContact, type, timestamp, flags, (DWORD)cbBlob, pBlob);
}

MEVENT CSkypeProto::AppendDBEvent(MCONTACT hContact, MEVENT hEvent, const char *szContent, const char *szUid, time_t edit_time)
{
	mir_cslock lck(m_AppendMessageLock);
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.cbBlob = db_event_getBlobSize(hEvent);
	dbei.pBlob = mir_ptr<BYTE>((PBYTE)mir_alloc(dbei.cbBlob));
	db_event_get(hEvent, &dbei);

	JSONNode jMsg = JSONNode::parse((char*)dbei.pBlob);
	if (jMsg)
	{
		JSONNode &jEdits = jMsg["edits"];
		if (jEdits)
		{
			for (auto it = jEdits.begin(); it != jEdits.end(); ++it)
			{
				const JSONNode &jEdit = *it;

				if (jEdit["time"].as_int() == edit_time)
					return hEvent;
			}
			JSONNode jEdit;
			jEdit 
				<< JSONNode("time", (long)edit_time)
				<< JSONNode("text", szContent);

			jEdits << jEdit;
		}
	}
	else
	{
		jMsg = JSONNode();
		JSONNode jOriginalMsg; jOriginalMsg.set_name("original_message");
		JSONNode jEdits(JSON_ARRAY); jEdits.set_name("edits");
		JSONNode jEdit;

		jOriginalMsg
			<< JSONNode("time", (long)dbei.timestamp)
			<< JSONNode("text", (char*)dbei.pBlob);

		jMsg << jOriginalMsg;

		jEdit 
			<< JSONNode("time", (long)edit_time)
			<< JSONNode("text", szContent);

		jEdits << jEdit;	
		jMsg   << jEdits;


	}
	db_event_delete(hContact, hEvent);	
	return AddDbEvent(SKYPE_DB_EVENT_TYPE_EDITED_MESSAGE, hContact, dbei.timestamp, DBEF_UTF, jMsg.write().c_str(), szUid);
}

MEVENT CSkypeProto::AddEventToDb(MCONTACT hContact, WORD type, DWORD timestamp, DWORD flags, DWORD cbBlob, PBYTE pBlob)
{
	DBEVENTINFO dbei;
	dbei.cbSize    = sizeof(dbei);
	dbei.szModule  = m_szModuleName;
	dbei.timestamp = timestamp;
	dbei.eventType = type;
	dbei.cbBlob    = cbBlob;
	dbei.pBlob     = pBlob;
	dbei.flags     = flags;
	return db_event_add(hContact, &dbei);
}

void CSkypeProto::InitDBEvents()
{
	db_set_resident(m_szModuleName, "LastAuthRequestTime");

	// custom event
	DBEVENTTYPEDESCR dbEventType = { sizeof(dbEventType) };
	dbEventType.module = m_szModuleName;
	dbEventType.flags = DETF_HISTORY | DETF_MSGWINDOW;
	dbEventType.iconService = MODULE "/GetEventIcon";
	dbEventType.textService = MODULE "/GetEventText";

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_EDITED_MESSAGE;
	dbEventType.descr = Translate("Edited message");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_ACTION;
	dbEventType.descr = Translate("Action");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_CALL_INFO;
	dbEventType.descr = Translate("Call information");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_FILETRANSFER_INFO;
	dbEventType.descr = Translate("File transfer information");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_URIOBJ;
	dbEventType.descr = Translate("URI object");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_UNKNOWN;
	dbEventType.descr = Translate("Unknown event");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_INCOMING_CALL;
	dbEventType.descr = Translate("Incoming call");
	dbEventType.flags |= DETF_NONOTIFY;
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);
}