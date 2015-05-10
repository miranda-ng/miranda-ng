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

		if (dbei.eventType != EVENTTYPE_MESSAGE && dbei.eventType != SKYPE_DB_EVENT_TYPE_ACTION && dbei.eventType != SKYPE_DB_EVENT_TYPE_CALL_INFO)
			continue;

		size_t cbLen = strlen((char*)dbei.pBlob);
		if (memcmp(&dbei.pBlob[cbLen + 1], messageId, messageIdLength) == 0)
			return hDbEvent;

		if (dbei.timestamp < timestamp)
			break;
	}

	return NULL;
}

MEVENT CSkypeProto::AddMessageToDb(MCONTACT hContact, DWORD timestamp, DWORD flags, const char *messageId, char *content, int emoteOffset)
{
	if (MEVENT hDbEvent = GetMessageFromDb(hContact, messageId, timestamp))
		return hDbEvent;
	size_t messageLength = mir_strlen(&content[emoteOffset]) + 1;
	size_t messageIdLength = mir_strlen(messageId);
	size_t cbBlob = messageLength + messageIdLength;
	PBYTE pBlob = (PBYTE)mir_alloc(cbBlob);
	memcpy(pBlob, &content[emoteOffset], messageLength);
	memcpy(pBlob + messageLength, messageId, messageIdLength);

	return AddEventToDb(hContact, emoteOffset == 0 ? EVENTTYPE_MESSAGE : SKYPE_DB_EVENT_TYPE_ACTION, timestamp, flags, (DWORD)cbBlob, pBlob);
}

MEVENT CSkypeProto::AddCallInfoToDb(MCONTACT hContact, DWORD timestamp, DWORD flags, const char *messageId, char *content)
{
	if (MEVENT hDbEvent = GetMessageFromDb(hContact, messageId, timestamp))
		return hDbEvent;
	size_t messageLength = mir_strlen(content) + 1;
	size_t messageIdLength = mir_strlen(messageId);
	size_t cbBlob = messageLength + messageIdLength;
	PBYTE pBlob = (PBYTE)mir_alloc(cbBlob);
	memcpy(pBlob, content, messageLength);
	memcpy(pBlob + messageLength, messageId, messageIdLength);

	return AddEventToDb(hContact, SKYPE_DB_EVENT_TYPE_CALL_INFO, timestamp, flags, (DWORD)cbBlob, pBlob);
}

MEVENT CSkypeProto::AddCallToDb(MCONTACT hContact, DWORD timestamp, DWORD flags, const char *callId, const char *gp)
{
	size_t callIdLength = mir_strlen(callId);
	size_t messageLength = mir_strlen(gp) + 1;
	size_t cbBlob = messageLength + callIdLength;
	PBYTE pBlob = (PBYTE)mir_alloc(cbBlob);
	memcpy(pBlob, gp, messageLength);
	memcpy(pBlob + messageLength, callId, callIdLength);

	return AddEventToDb(hContact, SKYPE_DB_EVENT_TYPE_INCOMING_CALL, timestamp, flags, (DWORD)cbBlob, pBlob);
}

MEVENT CSkypeProto::AddEventToDb(MCONTACT hContact, WORD type, DWORD timestamp, DWORD flags, DWORD cbBlob, PBYTE pBlob)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = this->m_szModuleName;
	dbei.timestamp = timestamp;
	dbei.eventType = type;
	dbei.cbBlob = cbBlob;
	dbei.pBlob = pBlob;
	dbei.flags = flags;
	return db_event_add(hContact, &dbei);
}

time_t CSkypeProto::GetLastMessageTime(MCONTACT hContact)
{
	MEVENT hDbEvent = db_event_last(hContact);
	if (hDbEvent != NULL)
	{
		DBEVENTINFO dbei = { sizeof(dbei) };
		db_event_get(hDbEvent, &dbei);
		return dbei.timestamp;
	}
	return 0;
}