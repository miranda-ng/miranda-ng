/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
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
#include "jabber_disco.h"

/////////////////////////////////////////////////////////////////////////////////////////
// OnContactDeleted - processes a contact deletion

int CJabberProto::OnContactDeleted(WPARAM hContact, LPARAM)
{
	if (!m_bJabberOnline)	// should never happen
		return 0;

	ptrW jid(getWStringA(hContact, isChatRoom(hContact) ? "ChatRoomID" : "jid"));
	if (jid == nullptr)
		return 0;

	if (ListGetItemPtr(LIST_ROSTER, jid)) {
		if (!wcschr(jid, '@')) {
			wchar_t szStrippedJid[JABBER_MAX_JID_LEN];
			JabberStripJid(m_ThreadInfo->fullJID, szStrippedJid, _countof(szStrippedJid));
			wchar_t *szDog = wcschr(szStrippedJid, '@');
			if (szDog && mir_wstrcmpi(szDog + 1, jid))
				m_ThreadInfo->send(XmlNodeIq(L"set", SerialNext(), jid) << XQUERY(JABBER_FEAT_REGISTER) << XCHILD(L"remove"));
		}

		// Remove from roster, server also handles the presence unsubscription process.
		m_ThreadInfo->send(XmlNodeIq(L"set", SerialNext()) << XQUERY(JABBER_FEAT_IQ_ROSTER)
			<< XCHILD(L"item") << XATTR(L"jid", jid) << XATTR(L"subscription", L"remove"));
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberDbSettingChanged - process database changes

static wchar_t* sttSettingToTchar(DBCONTACTWRITESETTING *cws)
{
	switch (cws->value.type) {
	case DBVT_ASCIIZ:
		return mir_a2u(cws->value.pszVal);

	case DBVT_UTF8:
		return mir_utf8decodeW(cws->value.pszVal);

	case DBVT_WCHAR:
		return mir_wstrdup(cws->value.pwszVal);
	}
	return nullptr;
}

void __cdecl CJabberProto::OnRenameGroup(DBCONTACTWRITESETTING *cws, MCONTACT hContact)
{
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, ptrW(getWStringA(hContact, "jid")));
	if (item == nullptr)
		return;

	ptrW tszNick(db_get_wsa(hContact, "CList", "MyHandle"));
	if (tszNick == nullptr)
		tszNick = getWStringA(hContact, "Nick");
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
		wchar_t *p = sttSettingToTchar(cws);
		if (cws->value.pszVal != nullptr && mir_wstrcmp(p, item->group)) {
			debugLogW(L"Group set to %s", p);
			if (p)
				AddContactToRoster(item->jid, tszNick, p);
		}
		mir_free(p);
	}
}

void __cdecl CJabberProto::OnRenameContact(DBCONTACTWRITESETTING *cws, MCONTACT hContact)
{
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, ptrW( getWStringA(hContact, "jid")));
	if (item == nullptr)
		return;

	if (cws->value.type == DBVT_DELETED) {
		wchar_t *nick = pcli->pfnGetContactDisplayName(hContact, GCDNF_NOMYHANDLE);
		AddContactToRoster(item->jid, nick, item->group);
		mir_free(nick);
		return;
	}

	ptrW newNick( sttSettingToTchar(cws));
	if (newNick && mir_wstrcmp(item->nick, newNick)) {
		debugLogW(L"Renaming contact %s: %s -> %s", item->jid, item->nick, newNick);
		AddContactToRoster(item->jid, newNick, item->group);
	}
}

void __cdecl CJabberProto::OnAddContactForever(DBCONTACTWRITESETTING *cws, MCONTACT hContact)
{
	if (cws->value.type != DBVT_DELETED && !(cws->value.type == DBVT_BYTE && cws->value.bVal == 0))
		return;

	ptrW jid(getWStringA(hContact, "jid"));
	if (jid == nullptr)
		return;

	debugLogW(L"Add %s permanently to list", jid);
	ptrW nick(db_get_wsa(hContact, "CList", "MyHandle"));
	if (nick == nullptr)
		nick = getWStringA(hContact, "Nick");
	if (nick == nullptr)
		nick = JabberNickFromJID(jid);
	if (nick == nullptr)
		return;

	AddContactToRoster(jid, nick, ptrW(db_get_wsa(hContact, "CList", "Group")));

	XmlNode xPresence(L"presence"); xPresence << XATTR(L"to", LPCTSTR(jid)) << XATTR(L"type", L"subscribe");
	ptrW myNick(getWStringA(0, "Nick"));
	if (myNick != nullptr)
		xPresence << XCHILD(L"nick", LPCTSTR(myNick)) << XATTR(L"xmlns", JABBER_FEAT_NICK);
	m_ThreadInfo->send(xPresence);

	SendGetVcard(jid);

	db_unset(hContact, "CList", "Hidden");
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
	else if (!strcmp(cws->szSetting, "NotOnList"))
		OnAddContactForever(cws, hContact);

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
		MIRANDA_IDLE_INFO mii = { sizeof(mii) };
		CallService(MS_IDLE_GETIDLEINFO, 0, (LPARAM)&mii);
		m_tmJabberIdleStartTime = time(0) - mii.idleTime * 60;
	}
	else m_tmJabberIdleStartTime = 0;

	return 0;
}
