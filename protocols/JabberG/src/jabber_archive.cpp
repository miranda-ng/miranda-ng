/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (C) 2012-22 Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "stdafx.h"
#include "jabber_iq.h"
#include "jabber_caps.h"

bool operator==(const DBEVENTINFO &ev1, const DBEVENTINFO &ev2)
{
	return ev1.timestamp == ev2.timestamp && ev1.eventType == ev2.eventType && ev1.cbBlob == ev2.cbBlob && (ev1.flags & DBEF_SENT) == (ev2.flags & DBEF_SENT);
}

void CJabberProto::EnableArchive(bool bEnable)
{
	m_ThreadInfo->send(XmlNodeIq("set", SerialNext())
		<< XCHILDNS("auto", JABBER_FEAT_ARCHIVE) << XATTR("save", (bEnable) ? "true" : "false"));
}

void CJabberProto::RetrieveMessageArchive(MCONTACT hContact, JABBER_LIST_ITEM *pItem)
{
	if (pItem->bHistoryRead)
		return;

	pItem->bHistoryRead = true;

	XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResultGetCollectionList, JABBER_IQ_TYPE_GET));
	TiXmlElement *list = iq << XCHILDNS("list", JABBER_FEAT_ARCHIVE) << XATTR("with", pItem->jid);

	time_t tmLast = getDword(hContact, "LastCollection", 0);
	if (tmLast) {
		char buf[40];
		list << XATTR("start", time2str(tmLast, buf, _countof(buf)));
	}
	m_ThreadInfo->send(iq);
}

void CJabberProto::OnIqResultGetCollectionList(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	const char *to = XmlGetAttr(iqNode, "to");
	if (to == nullptr || mir_strcmp(XmlGetAttr(iqNode, "type"), "result"))
		return;

	auto *list = XmlFirstChild(iqNode, "list");
	if (mir_strcmp(XmlGetAttr(list, "xmlns"), JABBER_FEAT_ARCHIVE))
		return;

	for (auto *itemNode : TiXmlFilter(list, "chat")) {
		const char *start = XmlGetAttr(itemNode, "start");
		const char *with = XmlGetAttr(itemNode, "with");
		if (!start || !with)
			continue;

		m_ThreadInfo->send(XmlNodeIq(AddIQ(&CJabberProto::OnIqResultGetCollection, JABBER_IQ_TYPE_GET))
			<< XCHILDNS("retrieve", JABBER_FEAT_ARCHIVE) << XATTR("with", with) << XATTR("start", start));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static uint32_t dwPreviousTimeStamp = -1;
static MCONTACT hPreviousContact = INVALID_CONTACT_ID;
static MEVENT hPreviousDbEvent = 0;

// Returns TRUE if the event already exist in the database
bool IsDuplicateEvent(MCONTACT hContact, DBEVENTINFO& dbei)
{
	// get last event
	MEVENT hExistingDbEvent = db_event_last(hContact);
	if (!hExistingDbEvent)
		return false;

	DBEVENTINFO dbeiExisting = {};
	db_event_get(hExistingDbEvent, &dbeiExisting);
	uint32_t dwEventTimeStamp = dbeiExisting.timestamp;

	// compare with last timestamp
	if (dbei.timestamp > dwEventTimeStamp) {
		// remember event
		hPreviousDbEvent = hExistingDbEvent;
		dwPreviousTimeStamp = dwEventTimeStamp;
		return false;
	}

	if (hContact != hPreviousContact) {
		hPreviousContact = hContact;
		// remember event
		hPreviousDbEvent = hExistingDbEvent;
		dwPreviousTimeStamp = dwEventTimeStamp;

		// get first event
		if (!(hExistingDbEvent = db_event_first(hContact)))
			return false;

		memset(&dbeiExisting, 0, sizeof(dbeiExisting));
		db_event_get(hExistingDbEvent, &dbeiExisting);
		dwEventTimeStamp = dbeiExisting.timestamp;

		// compare with first timestamp
		if (dbei.timestamp <= dwEventTimeStamp) {
			// remember event
			dwPreviousTimeStamp = dwEventTimeStamp;
			hPreviousDbEvent = hExistingDbEvent;

			if (dbei.timestamp != dwEventTimeStamp)
				return false;
		}
	}

	// check for equal timestamps
	if (dbei.timestamp == dwPreviousTimeStamp) {
		memset(&dbeiExisting, 0, sizeof(dbeiExisting));
		db_event_get(hPreviousDbEvent, &dbeiExisting);

		if (dbei == dbeiExisting)
			return true;

		// find event with another timestamp
		hExistingDbEvent = db_event_next(hContact, hPreviousDbEvent);
		while (hExistingDbEvent != 0) {
			memset(&dbeiExisting, 0, sizeof(dbeiExisting));
			db_event_get(hExistingDbEvent, &dbeiExisting);

			if (dbeiExisting.timestamp != dwPreviousTimeStamp) {
				// use found event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				break;
			}

			hPreviousDbEvent = hExistingDbEvent;
			hExistingDbEvent = db_event_next(hContact, hExistingDbEvent);
		}
	}

	hExistingDbEvent = hPreviousDbEvent;

	if (dbei.timestamp <= dwPreviousTimeStamp) {
		// look back
		while (hExistingDbEvent != 0) {
			memset(&dbeiExisting, 0, sizeof(dbeiExisting));
			db_event_get(hExistingDbEvent, &dbeiExisting);

			if (dbei.timestamp > dbeiExisting.timestamp) {
				// remember event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				return false;
			}

			// Compare event with import candidate
			if (dbei == dbeiExisting) {
				// remember event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				return true;
			}

			// Get previous event in chain
			hExistingDbEvent = db_event_prev(hContact, hExistingDbEvent);
		}
	}
	else {
		// look forward
		while (hExistingDbEvent != 0) {
			memset(&dbeiExisting, 0, sizeof(dbeiExisting));
			db_event_get(hExistingDbEvent, &dbeiExisting);

			if (dbei.timestamp < dbeiExisting.timestamp) {
				// remember event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				return false;
			}

			// Compare event with import candidate
			if (dbei == dbeiExisting) {
				// remember event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				return true;
			}

			// Get next event in chain
			hExistingDbEvent = db_event_next(hContact, hExistingDbEvent);
		}
	}
	// reset last event
	hPreviousContact = INVALID_CONTACT_ID;
	return false;
}

void CJabberProto::OnIqResultGetCollection(const TiXmlElement *iqNode, CJabberIqInfo*)
{
	if (mir_strcmp(XmlGetAttr(iqNode, "type"), "result"))
		return;

	auto *chatNode = XmlFirstChild(iqNode, "chat");
	if (!chatNode || mir_strcmp(XmlGetAttr(chatNode, "xmlns"), JABBER_FEAT_ARCHIVE))
		return;

	const char* start = XmlGetAttr(chatNode, "start");
	const char* with = XmlGetAttr(chatNode, "with");
	if (!start || !with)
		return;

	_tzset();

	MCONTACT hContact = HContactFromJID(with);
	time_t tmStart = str2time(start);
	if (hContact == 0 || tmStart == 0)
		return;

	time_t tmLast = getDword(hContact, "LastCollection", 0);

	for (auto *itemNode : TiXmlEnum(chatNode)) {
		int from;
		const char *itemName = itemNode->Name();
		if (!mir_strcmp(itemName, "to"))
			from = DBEF_SENT;
		else if (!mir_strcmp(itemName, "from"))
			from = 0;
		else
			continue;

		const TiXmlElement *body = XmlFirstChild(itemNode, "body");
		if (!body)
			continue;

		const char *tszBody = body->GetText();
		const char *tszSecs = XmlGetAttr(itemNode, "secs");
		if (!tszBody || !tszSecs)
			continue;

		DBEVENTINFO dbei = {};
		dbei.eventType = EVENTTYPE_MESSAGE;
		dbei.szModule = m_szModuleName;
		dbei.cbBlob = (uint32_t)mir_strlen(tszBody) + 1;
		dbei.flags = DBEF_READ + DBEF_UTF + from;
		dbei.pBlob = (uint8_t*)tszBody;
		dbei.timestamp = tmStart + atol(tszSecs);
		if (!IsDuplicateEvent(hContact, dbei))
			db_event_add(hContact, &dbei);

		tmStart = dbei.timestamp;
		if (dbei.timestamp > tmLast)
			tmLast = dbei.timestamp;
	}

	if (tmLast != 0)
		setDword(hContact, "LastCollection", tmLast + 1);
}
