/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (C) 2012-25 Miranda NG team

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

		XmppMsg msg(itemNode, this);

		const char *itemName = itemNode->Name();
		if (!mir_strcmp(itemName, "to"))
			msg.dbei.flags |= DBEF_SENT;
		else if (mir_strcmp(itemName, "from"))
			continue;

		const char *tszSecs = XmlGetAttr(itemNode, "secs");
		msg.msgTime = tmStart + atol(tszSecs);

		msg.process();


		tmStart = msg.dbei.timestamp;
		if (msg.dbei.timestamp > tmLast)
			tmLast = msg.dbei.timestamp;
	}

	if (tmLast != 0)
		setDword(hContact, "LastCollection", tmLast + 1);
}
