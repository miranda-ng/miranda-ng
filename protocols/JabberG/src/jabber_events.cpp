/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
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
#include "jabber_disco.h"

/////////////////////////////////////////////////////////////////////////////////////////
// OnContactDeleted - processes a contact deletion

void CJabberProto::OnContactDeleted(MCONTACT hContact)
{
	if (!m_bJabberOnline)	// should never happen
		return;

	ptrA jid(ContactToJID(hContact));
	if (jid == nullptr)
		return;

	if (ListGetItemPtr(LIST_ROSTER, jid)) {
		if (!strchr(jid, '@')) {
			char szStrippedJid[JABBER_MAX_JID_LEN];
			JabberStripJid(m_ThreadInfo->fullJID, szStrippedJid, _countof(szStrippedJid));
			char *szDog = strchr(szStrippedJid, '@');
			if (szDog && mir_strcmpi(szDog + 1, jid))
				m_ThreadInfo->send(XmlNodeIq("set", SerialNext(), jid) << XQUERY(JABBER_FEAT_REGISTER) << XCHILD("remove"));
		}

		// Remove from roster, server also handles the presence unsubscription process.
		m_ThreadInfo->send(XmlNodeIq("set", SerialNext())
			<< XQUERY(JABBER_FEAT_IQ_ROSTER) << XCHILD("item") << XATTR("jid", jid) << XATTR("subscription", "remove"));

		ListRemove(LIST_ROSTER, jid);
	}

	if (isChatRoom(hContact))
		ListRemove(LIST_CHATROOM, jid);
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberDbSettingChanged - process database changes

static char* sttSettingToTchar(DBCONTACTWRITESETTING *cws)
{
	switch (cws->value.type) {
	case DBVT_ASCIIZ:
		return mir_utf8encode(cws->value.pszVal);

	case DBVT_UTF8:
		return mir_strdup(cws->value.pszVal);

	case DBVT_WCHAR:
		return mir_utf8encodeW(cws->value.pwszVal);
	}
	return nullptr;
}

void __cdecl CJabberProto::OnRenameGroup(DBCONTACTWRITESETTING *cws, MCONTACT hContact)
{
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, ptrA(getUStringA(hContact, "jid")));
	if (item == nullptr)
		return;

	ptrA tszNick(db_get_utfa(hContact, "CList", "MyHandle"));
	if (tszNick == nullptr)
		tszNick = getUStringA(hContact, "Nick");
	if (tszNick == nullptr)
		tszNick = JabberNickFromJID(item->jid);
	if (tszNick == nullptr)
		return;

	if (cws->value.type == DBVT_DELETED) {
		if (item->group != nullptr) {
			debugLogA("Group set to nothing");
			AddContactToRoster(item->jid, tszNick, nullptr);
		}
	}
	else {
		char *p = sttSettingToTchar(cws);
		if (cws->value.pszVal != nullptr && mir_strcmp(p, item->group)) {
			debugLogA("Group set to %s", p);
			if (p)
				AddContactToRoster(item->jid, tszNick, p);
		}
		mir_free(p);
	}
}

void __cdecl CJabberProto::OnRenameContact(DBCONTACTWRITESETTING *cws, MCONTACT hContact)
{
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, ptrA(getUStringA(hContact, "jid")));
	if (item == nullptr)
		return;

	if (cws->value.type == DBVT_DELETED) {
		wchar_t *nick = Clist_GetContactDisplayName(hContact, GCDNF_NOMYHANDLE);
		AddContactToRoster(item->jid, T2Utf(nick), item->group);
		mir_free(nick);
		return;
	}

	ptrA newNick(sttSettingToTchar(cws));
	if (newNick && mir_strcmp(item->nick, newNick)) {
		debugLogA("Renaming contact %s: %s -> %s", item->jid, item->nick, newNick.get());
		AddContactToRoster(item->jid, newNick, item->group);
	}
}

void __cdecl CJabberProto::OnAddContactForever(MCONTACT hContact)
{
	ptrA jid(getUStringA(hContact, "jid"));
	if (jid == nullptr)
		return;

	debugLogA("Add %s permanently to list", jid.get());
	ptrA nick(db_get_utfa(hContact, "CList", "MyHandle"));
	if (nick == nullptr)
		nick = getUStringA(hContact, "Nick");
	if (nick == nullptr)
		nick = JabberNickFromJID(jid);
	if (nick == nullptr)
		return;

	AddContactToRoster(jid, nick, T2Utf(ptrW(Clist_GetGroup(hContact))));

	XmlNode xPresence("presence"); xPresence << XATTR("to", jid) << XATTR("type", "subscribe");
	ptrA myNick(getUStringA("Nick"));
	if (myNick != nullptr && !m_bIgnoreRoster)
		xPresence << XCHILD("nick", myNick) << XATTR("xmlns", JABBER_FEAT_NICK);
	m_ThreadInfo->send(xPresence);

	SendGetVcard(hContact);

	Contact_Hide(hContact, false);
}

int __cdecl CJabberProto::OnDbMarkedRead(WPARAM, LPARAM hDbEvent)
{
	MCONTACT hContact = db_event_getContact(hDbEvent);
	if (!hContact)
		return 0;

	// filter out only events of my protocol
	const char *szProto = Proto_GetBaseAccountName(hContact);
	if (mir_strcmp(szProto, m_szModuleName))
		return 0;

	auto *pMark = m_arChatMarks.find((CChatMark *)&hDbEvent);
	if (pMark) {
		if (IsSendAck(hContact)) {
			XmlNode reply("message"); reply << XATTR("to", pMark->szFrom) << XATTR("id", pMark->szId) 
				<< XCHILDNS("displayed", JABBER_FEAT_CHAT_MARKERS) << XATTR("id", pMark->szId);
			m_ThreadInfo->send(reply);
		}
		
		m_arChatMarks.remove(pMark);
	}
	return 0;
}

int __cdecl CJabberProto::OnDbSettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0 || !m_bJabberOnline)
		return 0;

	DBCONTACTWRITESETTING* cws = (DBCONTACTWRITESETTING*)lParam;
	if (strcmp(cws->szModule, "CList"))
		return 0;

	if (!strcmp(cws->szSetting, "Group"))
		OnRenameGroup(cws, hContact);
	else if (!strcmp(cws->szSetting, "MyHandle"))
		OnRenameContact(cws, hContact);
	else if (!strcmp(cws->szSetting, "NotOnList")) {
		if (cws->value.type == DBVT_DELETED || (cws->value.type == DBVT_BYTE && cws->value.bVal == 0))
			OnAddContactForever(hContact);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnIdleChanged - tracks idle start time for XEP-0012 support

int CJabberProto::OnIdleChanged(WPARAM, LPARAM lParam)
{
	// don't report idle time, if user disabled
	if (lParam & IDF_PRIVACY) {
		m_tmJabberIdleStartTime = 0;
		return 0;
	}

	if (lParam & IDF_ISIDLE) {
		MIRANDA_IDLE_INFO mii;
		Idle_GetInfo(mii);
		m_tmJabberIdleStartTime = time(0) - mii.idleTime * 60;

		if (m_bCisAvailable)
			m_ThreadInfo->send(XmlNode("inactive") << XATTR("xmlns", JABBER_FEAT_CSI));
	}
	else {
		m_tmJabberIdleStartTime = 0;

		if (m_bCisAvailable)
			m_ThreadInfo->send(XmlNode("active") << XATTR("xmlns", JABBER_FEAT_CSI));
	}

	return 0;
}
