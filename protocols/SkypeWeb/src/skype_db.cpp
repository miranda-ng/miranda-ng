/*
Copyright (c) 2015-19 Miranda NG team (https://miranda-ng.org)

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

struct SkypeDBType { int type; char *name; DWORD flags; } g_SkypeDBTypes[] =
{
	{ SKYPE_DB_EVENT_TYPE_INCOMING_CALL, LPGEN("Incoming call"), DETF_NONOTIFY },
	{ SKYPE_DB_EVENT_TYPE_EDITED_MESSAGE, LPGEN("Edited message"), 0 },
	{ SKYPE_DB_EVENT_TYPE_ACTION, LPGEN("Action"), 0 },
	{ SKYPE_DB_EVENT_TYPE_CALL_INFO, LPGEN("Call information"), 0 },
	{ SKYPE_DB_EVENT_TYPE_FILETRANSFER_INFO, LPGEN("File transfer information"), 0 },
	{ SKYPE_DB_EVENT_TYPE_URIOBJ, LPGEN("URI object"), 0 },
	{ SKYPE_DB_EVENT_TYPE_MOJI, LPGEN("Moji"), 0 },
	{ SKYPE_DB_EVENT_TYPE_FILE, LPGEN("File"), 0 },
	{ SKYPE_DB_EVENT_TYPE_UNKNOWN, LPGEN("Unknown event"), 0 },
};

MEVENT CSkypeProto::GetMessageFromDb(const char *messageId)
{
	if (messageId == nullptr)
		return NULL;

	return db_event_getById(m_szModuleName, messageId);
}

MEVENT CSkypeProto::AddDbEvent(WORD type, MCONTACT hContact, DWORD timestamp, DWORD flags, const char *content, const char *uid)
{
	if (MEVENT hDbEvent = GetMessageFromDb(uid))
		return hDbEvent;

	MEVENT ret = AddEventToDb(hContact, type, timestamp, flags, (DWORD)mir_strlen(content)+1, (BYTE*)content);
	if (uid && ret)
		db_event_setId(m_szModuleName, ret, uid);
	return ret;
}

void CSkypeProto::EditEvent(MCONTACT hContact, MEVENT hEvent, const char *szContent, time_t edit_time)
{
	mir_cslock lck(m_AppendMessageLock);
	DBEVENTINFO dbei = {};
	dbei.cbBlob = db_event_getBlobSize(hEvent);
	mir_ptr<BYTE> blob((PBYTE)mir_alloc(dbei.cbBlob));
	dbei.pBlob = blob;
	db_event_get(hEvent, &dbei);

	JSONNode jMsg = JSONNode::parse((char*)dbei.pBlob);
	if (jMsg) {
		JSONNode &jEdits = jMsg["edits"];
		if (jEdits) {
			for (auto it = jEdits.begin(); it != jEdits.end(); ++it) {
				const JSONNode &jEdit = *it;

				if (jEdit["time"].as_int() == edit_time)
					return;
			}
			JSONNode jEdit;
			jEdit
				<< JSONNode("time", (long)edit_time)
				<< JSONNode("text", szContent);

			jEdits << jEdit;
		}
	}
	else {
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
		jMsg << jEdits;
	}
	
	std::string newMsg = jMsg.write().c_str();
	dbei.cbBlob = int(newMsg.size() + 1);
	dbei.pBlob = (PBYTE)newMsg.c_str();
	db_event_edit(hContact, hEvent, &dbei);
}

MEVENT CSkypeProto::AddEventToDb(MCONTACT hContact, WORD type, DWORD timestamp, DWORD flags, DWORD cbBlob, PBYTE pBlob)
{
	DBEVENTINFO dbei = {};
	dbei.szModule = m_szModuleName;
	dbei.timestamp = timestamp;
	dbei.eventType = type;
	dbei.cbBlob = cbBlob;
	dbei.pBlob = pBlob;
	dbei.flags = flags;
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

	for (auto &cur : g_SkypeDBTypes) {
		dbEventType.eventType = cur.type;
		dbEventType.descr = Translate(cur.name);
		dbEventType.flags |= cur.flags;

		DbEvent_RegisterType(&dbEventType);

		dbEventType.flags &= (~cur.flags);
	}
}
