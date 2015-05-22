/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (ñ) 2012-15 Miranda NG project

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

void CJabberProto::EnableArchive(bool bEnable)
{
	m_ThreadInfo->send( XmlNodeIq(_T("set"), SerialNext())
		<< XCHILDNS( _T("auto"), JABBER_FEAT_ARCHIVE) << XATTR(_T("save"), (bEnable) ? _T("true") : _T("false")));
}

void CJabberProto::RetrieveMessageArchive(MCONTACT hContact, JABBER_LIST_ITEM *pItem)
{
	if (pItem->bHistoryRead)
		return;

	pItem->bHistoryRead = TRUE;

	XmlNodeIq iq( AddIQ(&CJabberProto::OnIqResultGetCollectionList, JABBER_IQ_TYPE_GET));
	HXML list = iq << XCHILDNS( _T("list"), JABBER_FEAT_ARCHIVE) << XATTR(_T("with"), pItem->jid);

	time_t tmLast = getDword(hContact, "LastCollection", 0);
	if (tmLast) {
		TCHAR buf[40];
		list << XATTR(_T("start"), time2str(tmLast, buf, SIZEOF(buf)));
	}
	m_ThreadInfo->send(iq);
}

void CJabberProto::OnIqResultGetCollectionList(HXML iqNode, CJabberIqInfo*)
{
	const TCHAR *to = xmlGetAttrValue(iqNode, _T("to"));
	if (to == NULL || mir_tstrcmp( xmlGetAttrValue(iqNode, _T("type")), _T("result")))
		return;

	HXML list = xmlGetChild(iqNode, "list");
	if (!list || mir_tstrcmp( xmlGetAttrValue(list, _T("xmlns")), JABBER_FEAT_ARCHIVE))
		return;

	MCONTACT hContact = NULL;
	time_t tmLast = 0;

	for (int nodeIdx = 1; ; nodeIdx++) {
		HXML itemNode = xmlGetNthChild(list, _T("chat"), nodeIdx);
		if (!itemNode)
			break;

		const TCHAR* start = xmlGetAttrValue(itemNode, _T("start"));
		const TCHAR* with  = xmlGetAttrValue(itemNode, _T("with"));
		if (!start || !with)
			continue;

		if (hContact == NULL) {
			if ((hContact = HContactFromJID(with)) == NULL)
				continue;

			tmLast = getDword(hContact, "LastCollection", 0);
		}

		m_ThreadInfo->send(
			XmlNodeIq( AddIQ(&CJabberProto::OnIqResultGetCollection, JABBER_IQ_TYPE_GET))
				<< XCHILDNS( _T("retrieve"), JABBER_FEAT_ARCHIVE) << XATTR(_T("with"), with) << XATTR(_T("start"), start));

		time_t tmThis = str2time(start);
		if ( tmThis > tmLast) {
			tmLast = tmThis;
			setDword(hContact, "LastCollection", tmLast+1);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static DWORD dwPreviousTimeStamp = -1;
static MCONTACT hPreviousContact = INVALID_CONTACT_ID;
static MEVENT hPreviousDbEvent = NULL;

// Returns TRUE if the event already exist in the database
BOOL IsDuplicateEvent(MCONTACT hContact, DBEVENTINFO& dbei)
{
	MEVENT hExistingDbEvent;
	DWORD dwEventTimeStamp;

	// get last event
	if (!(hExistingDbEvent = db_event_last(hContact)))
		return FALSE;

	DBEVENTINFO dbeiExisting = { sizeof(dbeiExisting) };
	db_event_get(hExistingDbEvent, &dbeiExisting);
	dwEventTimeStamp = dbeiExisting.timestamp;

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
		dbeiExisting.cbSize = sizeof(dbeiExisting);
		db_event_get(hExistingDbEvent, &dbeiExisting);
		dwEventTimeStamp = dbeiExisting.timestamp;

		// compare with first timestamp
		if (dbei.timestamp <= dwEventTimeStamp) {
			// remember event
			dwPreviousTimeStamp = dwEventTimeStamp;
			hPreviousDbEvent = hExistingDbEvent;

			if ( dbei.timestamp != dwEventTimeStamp )
				return FALSE;
		}
	}

	// check for equal timestamps
	if (dbei.timestamp == dwPreviousTimeStamp) {
		memset(&dbeiExisting, 0, sizeof(dbeiExisting));
		dbeiExisting.cbSize = sizeof(dbeiExisting);
		db_event_get(hPreviousDbEvent, &dbeiExisting);

		if ((dbei.timestamp == dbeiExisting.timestamp) &&
			(dbei.eventType == dbeiExisting.eventType) &&
			(dbei.cbBlob == dbeiExisting.cbBlob) &&
			((dbei.flags&DBEF_SENT) == (dbeiExisting.flags&DBEF_SENT)))
			return TRUE;

		// find event with another timestamp
		hExistingDbEvent = db_event_next(hContact, hPreviousDbEvent);
		while (hExistingDbEvent != NULL) {
			memset(&dbeiExisting, 0, sizeof(dbeiExisting));
			dbeiExisting.cbSize = sizeof(dbeiExisting);
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
		while (hExistingDbEvent != NULL) {
			memset(&dbeiExisting, 0, sizeof(dbeiExisting));
			dbeiExisting.cbSize = sizeof(dbeiExisting);
			db_event_get(hExistingDbEvent, &dbeiExisting);

			if (dbei.timestamp > dbeiExisting.timestamp) {
				// remember event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				return FALSE;
			}

			// Compare event with import candidate
			if ((dbei.timestamp == dbeiExisting.timestamp) &&
				(dbei.eventType == dbeiExisting.eventType) &&
				(dbei.cbBlob == dbeiExisting.cbBlob) &&
				((dbei.flags & DBEF_SENT) == (dbeiExisting.flags & DBEF_SENT)))
			{
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
		while (hExistingDbEvent != NULL) {
			memset(&dbeiExisting, 0, sizeof(dbeiExisting));
			dbeiExisting.cbSize = sizeof(dbeiExisting);
			db_event_get(hExistingDbEvent, &dbeiExisting);

			if (dbei.timestamp < dbeiExisting.timestamp) {
				// remember event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				return FALSE;
			}

			// Compare event with import candidate
			if ((dbei.timestamp == dbeiExisting.timestamp) &&
				(dbei.eventType == dbeiExisting.eventType) &&
				(dbei.cbBlob == dbeiExisting.cbBlob) &&
				((dbei.flags&DBEF_SENT) == (dbeiExisting.flags&DBEF_SENT)))
			{
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
	if ( mir_tstrcmp( xmlGetAttrValue(iqNode, _T("type")), _T("result")))
		return;

	HXML chatNode = xmlGetChild(iqNode, "chat");
	if (!chatNode || mir_tstrcmp( xmlGetAttrValue(chatNode, _T("xmlns")), JABBER_FEAT_ARCHIVE))
		return;

	const TCHAR* start = xmlGetAttrValue(chatNode, _T("start"));
	const TCHAR* with  = xmlGetAttrValue(chatNode, _T("with"));
	if (!start || !with)
		return;

	MCONTACT hContact = HContactFromJID(with);
	time_t tmStart = str2time(start);
	if (hContact == 0 || tmStart == 0)
		return;

	_tzset();

	for (int nodeIdx = 0; ; nodeIdx++) {
		HXML itemNode = xmlGetChild(chatNode, nodeIdx);
		if (!itemNode)
			break;

		int from;
		const TCHAR *itemName = xmlGetName(itemNode);
		if (!mir_tstrcmp(itemName, _T("to")))
			from = DBEF_SENT;
		else if (!mir_tstrcmp(itemName, _T("from")))
			from = 0;
		else
			continue;

		HXML body = xmlGetChild(itemNode, "body");
		if (!body)
			continue;

		const TCHAR *tszBody = xmlGetText(body);
		const TCHAR *tszSecs = xmlGetAttrValue(itemNode, _T("secs"));
		if (!tszBody || !tszSecs)
			continue;

		ptrA szEventText( mir_utf8encodeT(tszBody));

		DBEVENTINFO dbei = { sizeof(DBEVENTINFO) };
		dbei.eventType = EVENTTYPE_MESSAGE;
		dbei.szModule = m_szModuleName;
		dbei.cbBlob = (DWORD)mir_strlen(szEventText);
		dbei.flags = DBEF_READ + DBEF_UTF + from;
		dbei.pBlob = (PBYTE)(char*)szEventText;
		dbei.timestamp = tmStart + _ttol(tszSecs) - timezone;
		if (!IsDuplicateEvent(hContact, dbei))
			db_event_add(hContact, &dbei);
	}
}
