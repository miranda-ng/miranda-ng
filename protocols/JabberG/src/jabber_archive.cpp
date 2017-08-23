/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (ñ) 2012-17 Miranda NG project

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
	m_ThreadInfo->send(XmlNodeIq(L"set", SerialNext())
		<< XCHILDNS(L"auto", JABBER_FEAT_ARCHIVE) << XATTR(L"save", (bEnable) ? L"true" : L"false"));
}

void CJabberProto::RetrieveMessageArchive(MCONTACT hContact, JABBER_LIST_ITEM *pItem)
{
	if (pItem->bHistoryRead)
		return;

	pItem->bHistoryRead = true;

	XmlNodeIq iq(AddIQ(&CJabberProto::OnIqResultGetCollectionList, JABBER_IQ_TYPE_GET));
	HXML list = iq << XCHILDNS(L"list", JABBER_FEAT_ARCHIVE) << XATTR(L"with", pItem->jid);

	time_t tmLast = getDword(hContact, "LastCollection", 0);
	if (tmLast) {
		wchar_t buf[40];
		list << XATTR(L"start", time2str(tmLast, buf, _countof(buf)));
	}
	m_ThreadInfo->send(iq);
}

void CJabberProto::OnIqResultGetCollectionList(HXML iqNode, CJabberIqInfo*)
{
	const wchar_t *to = XmlGetAttrValue(iqNode, L"to");
	if (to == nullptr || mir_wstrcmp(XmlGetAttrValue(iqNode, L"type"), L"result"))
		return;

	HXML list = XmlGetChild(iqNode, "list");
	if (!list || mir_wstrcmp(XmlGetAttrValue(list, L"xmlns"), JABBER_FEAT_ARCHIVE))
		return;

	for (int nodeIdx = 1;; nodeIdx++) {
		HXML itemNode = XmlGetNthChild(list, L"chat", nodeIdx);
		if (!itemNode)
			break;

		const wchar_t* start = XmlGetAttrValue(itemNode, L"start");
		const wchar_t* with = XmlGetAttrValue(itemNode, L"with");
		if (!start || !with)
			continue;

		m_ThreadInfo->send(XmlNodeIq(AddIQ(&CJabberProto::OnIqResultGetCollection, JABBER_IQ_TYPE_GET))
			<< XCHILDNS(L"retrieve", JABBER_FEAT_ARCHIVE) << XATTR(L"with", with) << XATTR(L"start", start));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static DWORD dwPreviousTimeStamp = -1;
static MCONTACT hPreviousContact = INVALID_CONTACT_ID;
static MEVENT hPreviousDbEvent = 0;

// Returns TRUE if the event already exist in the database
BOOL IsDuplicateEvent(MCONTACT hContact, DBEVENTINFO& dbei)
{
	// get last event
	MEVENT hExistingDbEvent = db_event_last(hContact);
	if (!hExistingDbEvent)
		return FALSE;

	DBEVENTINFO dbeiExisting = {};
	db_event_get(hExistingDbEvent, &dbeiExisting);
	DWORD dwEventTimeStamp = dbeiExisting.timestamp;

	// compare with last timestamp
	if (dbei.timestamp > dwEventTimeStamp) {
		// remember event
		hPreviousDbEvent = hExistingDbEvent;
		dwPreviousTimeStamp = dwEventTimeStamp;
		return FALSE;
	}

	if (hContact != hPreviousContact) {
		hPreviousContact = hContact;
		// remember event
		hPreviousDbEvent = hExistingDbEvent;
		dwPreviousTimeStamp = dwEventTimeStamp;

		// get first event
		if (!(hExistingDbEvent = db_event_first(hContact)))
			return FALSE;

		memset(&dbeiExisting, 0, sizeof(dbeiExisting));
		db_event_get(hExistingDbEvent, &dbeiExisting);
		dwEventTimeStamp = dbeiExisting.timestamp;

		// compare with first timestamp
		if (dbei.timestamp <= dwEventTimeStamp) {
			// remember event
			dwPreviousTimeStamp = dwEventTimeStamp;
			hPreviousDbEvent = hExistingDbEvent;

			if (dbei.timestamp != dwEventTimeStamp)
				return FALSE;
		}
	}

	// check for equal timestamps
	if (dbei.timestamp == dwPreviousTimeStamp) {
		memset(&dbeiExisting, 0, sizeof(dbeiExisting));
		db_event_get(hPreviousDbEvent, &dbeiExisting);

		if (dbei == dbeiExisting)
			return TRUE;

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
				return FALSE;
			}

			// Compare event with import candidate
			if (dbei == dbeiExisting) {
				// remember event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				return TRUE;
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
				return FALSE;
			}

			// Compare event with import candidate
			if (dbei == dbeiExisting) {
				// remember event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				return TRUE;
			}

			// Get next event in chain
			hExistingDbEvent = db_event_next(hContact, hExistingDbEvent);
		}
	}
	// reset last event
	hPreviousContact = INVALID_CONTACT_ID;
	return FALSE;
}

void CJabberProto::OnIqResultGetCollection(HXML iqNode, CJabberIqInfo*)
{
	if (mir_wstrcmp(XmlGetAttrValue(iqNode, L"type"), L"result"))
		return;

	HXML chatNode = XmlGetChild(iqNode, "chat");
	if (!chatNode || mir_wstrcmp(XmlGetAttrValue(chatNode, L"xmlns"), JABBER_FEAT_ARCHIVE))
		return;

	const wchar_t* start = XmlGetAttrValue(chatNode, L"start");
	const wchar_t* with = XmlGetAttrValue(chatNode, L"with");
	if (!start || !with)
		return;

	_tzset();

	MCONTACT hContact = HContactFromJID(with);
	time_t tmStart = str2time(start);
	if (hContact == 0 || tmStart == 0)
		return;

	time_t tmLast = getDword(hContact, "LastCollection", 0);

	for (int nodeIdx = 0;; nodeIdx++) {
		HXML itemNode = XmlGetChild(chatNode, nodeIdx);
		if (!itemNode)
			break;

		int from;
		const wchar_t *itemName = XmlGetName(itemNode);
		if (!mir_wstrcmp(itemName, L"to"))
			from = DBEF_SENT;
		else if (!mir_wstrcmp(itemName, L"from"))
			from = 0;
		else
			continue;

		HXML body = XmlGetChild(itemNode, "body");
		if (!body)
			continue;

		const wchar_t *tszBody = XmlGetText(body);
		const wchar_t *tszSecs = XmlGetAttrValue(itemNode, L"secs");
		if (!tszBody || !tszSecs)
			continue;

		T2Utf szEventText(tszBody);

		DBEVENTINFO dbei = {};
		dbei.eventType = EVENTTYPE_MESSAGE;
		dbei.szModule = m_szModuleName;
		dbei.cbBlob = (DWORD)mir_strlen(szEventText)+1;
		dbei.flags = DBEF_READ + DBEF_UTF + from;
		dbei.pBlob = szEventText;
		dbei.timestamp = tmStart + _wtol(tszSecs);
		if (!IsDuplicateEvent(hContact, dbei))
			db_event_add(hContact, &dbei);

		tmStart = dbei.timestamp;
		if (dbei.timestamp > tmLast)
			tmLast = dbei.timestamp;
	}

	if (tmLast != 0)
		setDword(hContact, "LastCollection", tmLast + 1);
}
