/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-24 Miranda NG team

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
// OnCacheInit - cache initialization

void CJabberProto::OnCacheInit()
{
	for (auto &hContact : AccContacts()) {
		m_bCacheInited = true;

		SetContactOfflineStatus(hContact);

		if (getByte(hContact, "IsTransport", 0)) {
			ptrA jid(getUStringA(hContact, "jid"));
			if (jid == nullptr)
				continue;

			char *resourcepos = strchr(jid, '/');
			if (resourcepos != nullptr)
				*resourcepos = '\0';

			m_lstTransports.insert(mir_strdup(jid));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnContactDeleted - processes a contact deletion

bool CJabberProto::OnContactDeleted(MCONTACT hContact, uint32_t)
{
	if (!m_bJabberOnline)	// should never happen
		return false;

	ptrA jid(ContactToJID(hContact));
	if (jid == nullptr)
		return false;

	if (ListGetItemPtr(LIST_ROSTER, jid)) {
		if (!strchr(jid, '@')) {
			char szStrippedJid[JABBER_MAX_JID_LEN];
			JabberStripJid(m_ThreadInfo->fullJID, szStrippedJid, _countof(szStrippedJid));
			char *szDog = strchr(szStrippedJid, '@');
			if (szDog && mir_strcmpi(szDog + 1, jid))
				m_ThreadInfo->send(XmlNodeIq("set", SerialNext(), jid) << XQUERY(JABBER_FEAT_REGISTER) << XCHILD("remove"));
		}

		// Remove from a roster, the server also handles the presence unsubscription process.
		m_ThreadInfo->send(XmlNodeIq("set", SerialNext())
			<< XQUERY(JABBER_FEAT_IQ_ROSTER) << XCHILD("item") << XATTR("jid", jid) << XATTR("subscription", "remove"));

		ListRemove(LIST_ROSTER, jid);
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnMarkRead - mark an event read

void CJabberProto::OnMarkRead(MCONTACT hContact, MEVENT hDbEvent)
{
	auto *pMark = m_arChatMarks.find((CChatMark *)&hDbEvent);
	if (pMark) {
		if (IsSendAck(hContact)) {
			XmlNode reply("message"); reply << XATTR("to", pMark->szFrom) << XATTR("id", pMark->szId)
				<< XCHILDNS("displayed", JABBER_FEAT_CHAT_MARKERS) << XATTR("id", pMark->szId);
			m_ThreadInfo->send(reply);
		}

		m_arChatMarks.remove(pMark);
	}
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
