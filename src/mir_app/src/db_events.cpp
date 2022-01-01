/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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
#include "profilemanager.h"

static int CompareEventTypes(const DBEVENTTYPEDESCR *p1, const DBEVENTTYPEDESCR *p2)
{
	int result = mir_strcmp(p1->module, p2->module);
	if (result)
		return result;

	return p1->eventType - p2->eventType;
}

static LIST<DBEVENTTYPEDESCR> eventTypes(10, CompareEventTypes);

void UnloadEventsModule()
{
	for (auto &p : eventTypes) {
		mir_free(p->module);
		mir_free(p->descr);
		mir_free(p->textService);
		mir_free(p->iconService);
		mir_free(p);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) DbEvent_RegisterType(DBEVENTTYPEDESCR *et)
{
	if (et == nullptr)
		return -1;

	if (eventTypes.getIndex(et) != -1)
		return -1;

	DBEVENTTYPEDESCR *p = (DBEVENTTYPEDESCR*)mir_calloc(sizeof(DBEVENTTYPEDESCR));
	p->module = mir_strdup(et->module);
	p->eventType = et->eventType;
	p->descr = mir_strdup(et->descr);
	p->eventIcon = et->eventIcon;
	p->flags = et->flags;

	char szServiceName[100];
	if (!et->textService) {
		mir_snprintf(szServiceName, "%s/GetEventText%d", p->module, p->eventType);
		p->textService = mir_strdup(szServiceName);
	}
	else p->textService = mir_strdup(et->textService);

	if (!et->iconService) {
		mir_snprintf(szServiceName, "%s/GetEventIcon%d", p->module, p->eventType);
		p->iconService = mir_strdup(szServiceName);
	}
	else p->iconService = mir_strdup(et->iconService);

	eventTypes.insert(p);
	return 0;
}

MIR_APP_DLL(DBEVENTTYPEDESCR*) DbEvent_GetType(const char *szModule, int eventType)
{
	DBEVENTTYPEDESCR tmp;
	tmp.module = (char*)szModule;
	tmp.eventType = eventType;
	return eventTypes.find(&tmp);
}

/////////////////////////////////////////////////////////////////////////////////////////

static wchar_t* getEventString(DBEVENTINFO *dbei, LPSTR &buf)
{
	LPSTR in = buf;
	buf += mir_strlen(buf) + 1;
	return (dbei->flags & DBEF_UTF) ? mir_utf8decodeW(in) : mir_a2u(in);
}

static INT_PTR DbEventGetTextWorker(DBEVENTINFO *dbei, int codepage, int datatype)
{
	if (dbei == nullptr || dbei->szModule == nullptr)
		return 0;

	DBEVENTTYPEDESCR *et = DbEvent_GetType(dbei->szModule, dbei->eventType);
	if (et && ServiceExists(et->textService))
		return CallService(et->textService, (WPARAM)dbei, datatype);

	if (!dbei->pBlob)
		return 0;

	if (dbei->eventType == EVENTTYPE_AUTHREQUEST || dbei->eventType == EVENTTYPE_ADDED) {
		DB::AUTH_BLOB blob(dbei->pBlob);

		ptrW tszNick(dbei->getString(blob.get_nick()));
		ptrW tszFirst(dbei->getString(blob.get_firstName()));
		ptrW tszLast(dbei->getString(blob.get_lastName()));
		ptrW tszEmail(dbei->getString(blob.get_email()));

		CMStringW nick, text;
		if (tszFirst || tszLast) {
			nick.AppendFormat(L"%s %s", tszFirst.get(), tszLast.get());
			nick.Trim();
		}
		if (tszEmail) {
			if (!nick.IsEmpty())
				nick.Append(L", ");
			nick.Append(tszEmail);
		}
		if (blob.get_uin() != 0) {
			if (!nick.IsEmpty())
				nick.Append(L", ");
			nick.AppendFormat(L"%d", blob.get_uin());
		}
		if (!nick.IsEmpty())
			nick = L"(" + nick + L")";

		if (dbei->eventType == EVENTTYPE_AUTHREQUEST) {
			ptrW tszReason(dbei->getString(blob.get_reason()));
			text.Format(TranslateT("Authorization request from %s%s: %s"), 
				(tszNick == nullptr) ? Clist_GetContactDisplayName(blob.get_contact()) : tszNick.get(), nick.c_str(), tszReason.get());
		}
		else text.Format(TranslateT("You were added by %s%s"),
			(tszNick == nullptr) ? Clist_GetContactDisplayName(blob.get_contact()) : tszNick.get(), nick.c_str());
		return (datatype == DBVT_WCHAR) ? (INT_PTR)mir_wstrdup(text) : (INT_PTR)mir_u2a(text);
	}

	if (dbei->eventType == EVENTTYPE_CONTACTS) {
		CMStringW text(TranslateT("Contacts: "));
		// blob is: [uin(ASCIIZ), nick(ASCIIZ)]*
		char *buf = LPSTR(dbei->pBlob), *limit = LPSTR(dbei->pBlob) + dbei->cbBlob;
		while (buf < limit) {
			ptrW tszUin(getEventString(dbei, buf));
			ptrW tszNick(getEventString(dbei, buf));
			if (tszNick && *tszNick)
				text.AppendFormat(L"\"%s\" ", tszNick.get());
			if (tszUin && *tszUin)
				text.AppendFormat(L"<%s>; ", tszUin.get());
		}
		return (datatype == DBVT_WCHAR) ? (INT_PTR)mir_wstrdup(text) : (INT_PTR)mir_u2a(text);
	}

	if (dbei->eventType == EVENTTYPE_FILE) {
		char *buf = LPSTR(dbei->pBlob) + sizeof(uint32_t);
		ptrW tszFileName(getEventString(dbei, buf));
		ptrW tszDescription(getEventString(dbei, buf));
		CMStringW wszText(tszFileName);
		if (mir_wstrlen(tszDescription) > 0) {
			wszText.Append(L": ");
			wszText.Append(tszDescription);
		}
		
		switch (datatype) {
		case DBVT_WCHAR:
			return (INT_PTR)wszText.Detach();
		case DBVT_ASCIIZ:
			return (INT_PTR)mir_u2a(wszText);
		}
		return 0;
	}

	// by default treat an event's blob as a string
	if (datatype == DBVT_WCHAR) {
		char *str = (char*)alloca(dbei->cbBlob + 1);
		memcpy(str, dbei->pBlob, dbei->cbBlob);
		str[dbei->cbBlob] = 0;

		if (dbei->flags & DBEF_UTF) {
			wchar_t *msg = nullptr;
			mir_utf8decodecp(str, codepage, &msg);
			if (msg)
				return (INT_PTR)msg;
		}

		return (INT_PTR)mir_a2u_cp(str, codepage);
	}

	if (datatype == DBVT_ASCIIZ) {
		char *msg = mir_strdup((char*)dbei->pBlob);
		if (dbei->flags & DBEF_UTF)
			mir_utf8decodecp(msg, codepage, nullptr);

		return (INT_PTR)msg;
	}
	return 0;
}

MIR_APP_DLL(char*) DbEvent_GetTextA(DBEVENTINFO *dbei, int codepage)
{
	return (char*)DbEventGetTextWorker(dbei, codepage, DBVT_ASCIIZ);
}

MIR_APP_DLL(wchar_t*) DbEvent_GetTextW(DBEVENTINFO *dbei, int codepage)
{
	return (wchar_t*)DbEventGetTextWorker(dbei, codepage, DBVT_WCHAR);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(HICON) DbEvent_GetIcon(DBEVENTINFO *dbei, int flags)
{
	DBEVENTTYPEDESCR *et = DbEvent_GetType(dbei->szModule, dbei->eventType);
	if (et && ServiceExists(et->iconService)) {
		HICON icon = (HICON)CallService(et->iconService, flags, (LPARAM)dbei);
		if (icon)
			return icon;
	}

	HICON icon = nullptr;
	if (et && et->eventIcon)
		icon = IcoLib_GetIconByHandle(et->eventIcon);
	if (!icon) {
		char szName[100];
		mir_snprintf(szName, "eventicon_%s%d", dbei->szModule, dbei->eventType);
		icon = IcoLib_GetIcon(szName);
	}

	if (!icon) {
		switch(dbei->eventType) {
		case EVENTTYPE_FILE:
			icon = Skin_LoadIcon(SKINICON_EVENT_FILE);
			break;

		default: // EVENTTYPE_MESSAGE and unknown types
			icon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
			break;
		}
	}

	return (flags & LR_SHARED) ? icon : CopyIcon(icon);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(wchar_t*) DbEvent_GetString(DBEVENTINFO *dbei, const char *str)
{
	if (dbei->flags & DBEF_UTF)
		return mir_utf8decodeW(str);

	return mir_a2u(str);
}

/////////////////////////////////////////////////////////////////////////////////////////

DB::AUTH_BLOB::AUTH_BLOB(MCONTACT hContact, LPCSTR nick, LPCSTR fname, LPCSTR lname, LPCSTR email, LPCSTR reason) :
	m_dwUin(0),
	m_hContact(hContact),
	m_szNick(mir_strdup(nick)),
	m_szFirstName(mir_strdup(fname)),
	m_szLastName(mir_strdup(lname)),
	m_szEmail(mir_strdup(email)),
	m_szReason(mir_strdup(reason))
{
	m_size = uint32_t(sizeof(uint32_t) * 2 + 5 + mir_strlen(m_szNick) + mir_strlen(m_szFirstName) + mir_strlen(m_szLastName) + mir_strlen(m_szEmail) + mir_strlen(m_szReason));
}

DB::AUTH_BLOB::AUTH_BLOB(uint8_t *blob)
{
	uint8_t *pCurBlob = blob;
	m_dwUin = *(uint32_t*)pCurBlob;
	pCurBlob += sizeof(uint32_t);
	m_hContact = *(uint32_t*)pCurBlob;
	pCurBlob += sizeof(uint32_t);
	m_szNick = mir_strdup((char*)pCurBlob); pCurBlob += mir_strlen((char*)pCurBlob) + 1;
	m_szFirstName = mir_strdup((char*)pCurBlob); pCurBlob += mir_strlen((char*)pCurBlob) + 1;
	m_szLastName = mir_strdup((char*)pCurBlob); pCurBlob += mir_strlen((char*)pCurBlob) + 1;
	m_szEmail = mir_strdup((char*)pCurBlob); pCurBlob += mir_strlen((char*)pCurBlob) + 1;
	m_szReason = mir_strdup((char*)pCurBlob); pCurBlob += mir_strlen((char*)pCurBlob) + 1;
	m_size = uint32_t(pCurBlob - blob);
}

DB::AUTH_BLOB::~AUTH_BLOB()
{
}

uint8_t* DB::AUTH_BLOB::makeBlob()
{
	uint8_t *pBlob, *pCurBlob;
	pCurBlob = pBlob = (uint8_t*)mir_alloc(m_size + 1);

	*((uint32_t*)pCurBlob) = m_dwUin;
	pCurBlob += sizeof(uint32_t);
	*((uint32_t*)pCurBlob) = (uint32_t)m_hContact;
	pCurBlob += sizeof(uint32_t);

	mir_snprintf((char*)pCurBlob, m_size - 8, "%s%c%s%c%s%c%s%c%s%c",
		(m_szNick) ? m_szNick.get() : "", 0,
		(m_szFirstName) ? m_szFirstName.get() : "", 0,
		(m_szLastName) ? m_szLastName.get() : "", 0,
		(m_szEmail) ? m_szEmail.get() : "", 0,
		(m_szReason) ? m_szReason.get() : "", 0);

	return pBlob;
}

