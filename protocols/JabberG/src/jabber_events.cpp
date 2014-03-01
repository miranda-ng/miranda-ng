/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (c) 2012-14  Miranda NG project

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

#include "jabber.h"
#include "jabber_iq.h"
#include "jabber_caps.h"
#include "jabber_disco.h"

/////////////////////////////////////////////////////////////////////////////////////////
// OnContactDeleted - processes a contact deletion

int CJabberProto::OnContactDeleted(WPARAM hContact, LPARAM)
{
	if (!m_bJabberOnline)	// should never happen
		return 0;

	ptrT jid(getTStringA(hContact, isChatRoom(hContact) ? "ChatRoomID" : "jid"));
	if (jid == NULL)
		return 0;

	if (ListGetItemPtr(LIST_ROSTER, jid)) {
		if (!_tcschr(jid, _T('@'))) {
			TCHAR szStrippedJid[JABBER_MAX_JID_LEN];
			JabberStripJid(m_ThreadInfo->fullJID, szStrippedJid, SIZEOF(szStrippedJid));
			TCHAR *szDog = _tcschr(szStrippedJid, _T('@'));
			if (szDog && _tcsicmp(szDog + 1, jid))
				m_ThreadInfo->send(XmlNodeIq(_T("set"), SerialNext(), jid) << XQUERY(JABBER_FEAT_REGISTER) << XCHILD(_T("remove")));
		}

		// Remove from roster, server also handles the presence unsubscription process.
		m_ThreadInfo->send(XmlNodeIq(_T("set"), SerialNext()) << XQUERY(JABBER_FEAT_IQ_ROSTER)
			<< XCHILD(_T("item")) << XATTR(_T("jid"), jid) << XATTR(_T("subscription"), _T("remove")));
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberDbSettingChanged - process database changes

static TCHAR* sttSettingToTchar(DBCONTACTWRITESETTING *cws)
{
	switch (cws->value.type) {
	case DBVT_ASCIIZ:
		return mir_a2t(cws->value.pszVal);

	case DBVT_UTF8:
		return mir_utf8decodeT(cws->value.pszVal);

	case DBVT_WCHAR:
		return mir_u2t(cws->value.pwszVal);
	}
	return NULL;
}

void __cdecl CJabberProto::OnRenameGroup(DBCONTACTWRITESETTING *cws, MCONTACT hContact)
{
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, ptrT(getTStringA(hContact, "jid")));
	if (item == NULL)
		return;

	ptrT tszNick(db_get_tsa(hContact, "CList", "MyHandle"));
	if (tszNick == NULL)
		tszNick = getTStringA(hContact, "Nick");
	if (tszNick == NULL)
		tszNick = JabberNickFromJID(item->jid);
	if (tszNick == NULL)
		return;

	if (cws->value.type == DBVT_DELETED) {
		if (item->group != NULL) {
			debugLogA("Group set to nothing");
			AddContactToRoster(item->jid, tszNick, NULL);
		}
	}
	else {
		TCHAR *p = sttSettingToTchar(cws);
		if (cws->value.pszVal != NULL && lstrcmp(p, item->group)) {
			debugLog(_T("Group set to %s"), p);
			if (p)
				AddContactToRoster(item->jid, tszNick, p);
		}
		mir_free(p);
	}
}

void __cdecl CJabberProto::OnRenameContact(DBCONTACTWRITESETTING *cws, MCONTACT hContact)
{
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, ptrT( getTStringA(hContact, "jid")));
	if (item == NULL)
		return;

	if (cws->value.type == DBVT_DELETED) {
		TCHAR *nick = pcli->pfnGetContactDisplayName(hContact, GCDNF_NOMYHANDLE);
		AddContactToRoster(item->jid, nick, item->group);
		mir_free(nick);
		return;
	}

	ptrT newNick( sttSettingToTchar(cws));
	if (newNick && lstrcmp(item->nick, newNick)) {
		debugLog(_T("Renaming contact %s: %s -> %s"), item->jid, item->nick, newNick);
		AddContactToRoster(item->jid, newNick, item->group);
	}
}

void __cdecl CJabberProto::OnAddContactForever(DBCONTACTWRITESETTING *cws, MCONTACT hContact)
{
	if (cws->value.type != DBVT_DELETED && !(cws->value.type == DBVT_BYTE && cws->value.bVal == 0))
		return;

	ptrT jid(getTStringA(hContact, "jid"));
	if (jid == NULL)
		return;

	debugLog(_T("Add %s permanently to list"), jid);
	ptrT nick(db_get_tsa(hContact, "CList", "MyHandle"));
	if (nick == NULL)
		nick = getTStringA(hContact, "Nick");
	if (nick == NULL)
		nick = JabberNickFromJID(jid);
	if (nick == NULL)
		return;

	AddContactToRoster(jid, nick, ptrT(db_get_tsa(hContact, "CList", "Group")));

	XmlNode xPresence(_T("presence")); xPresence << XATTR(_T("to"), LPCTSTR(jid)) << XATTR(_T("type"), _T("subscribe"));
	ptrT myNick(getTStringA(NULL, "Nick"));
	if (myNick != NULL)
		xPresence << XCHILD(_T("nick"), LPCTSTR(myNick)) << XATTR(_T("xmlns"), JABBER_FEAT_NICK);
	m_ThreadInfo->send(xPresence);

	SendGetVcard(jid);

	db_unset(hContact, "CList", "Hidden");
}

int __cdecl CJabberProto::OnDbSettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (hContact == NULL || !m_bJabberOnline)
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
