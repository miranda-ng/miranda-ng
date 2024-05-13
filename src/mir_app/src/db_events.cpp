/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org),
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

MIR_APP_DLL(wchar_t*) DbEvent_GetText(const DBEVENTINFO *dbei)
{
	if (dbei == nullptr || dbei->szModule == nullptr)
		return 0;

	DB::EventInfo tmp;
	memcpy(&tmp, dbei, sizeof(*dbei));
	return tmp.getText();
}

/////////////////////////////////////////////////////////////////////////////////////////

#ifdef _WINDOWS
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
#endif

/////////////////////////////////////////////////////////////////////////////////////////

DB::EventInfo::EventInfo(MEVENT hEvent, bool bFetchBlob)
{
	memset(this, 0, sizeof(*this));
	m_hEvent = hEvent;
	fetch(bFetchBlob);
}

DB::EventInfo::EventInfo() :
	m_bValid(false)
{
	memset(this, 0, sizeof(*this));
}

DB::EventInfo& DB::EventInfo::operator=(MEVENT hEvent)
{
	m_hEvent = hEvent;
	m_bValid = false;
	return *this;
}

DB::EventInfo::~EventInfo()
{
	delete m_json;

	if (m_bValid)
		mir_free(pBlob);
}

bool DB::EventInfo::fetch(bool bFetchBlob)
{
	if (bFetchBlob)
		cbBlob = -1;
	
	if (m_bValid = ::db_event_get(m_hEvent, this) == 0)
		if ((flags & DBEF_JSON) && pBlob)
			m_json = new JSONNode(JSONNode::parse((const char *)pBlob));

	return m_bValid;
}

void DB::EventInfo::unload()
{
	if (pBlob) {
		mir_free(pBlob);
		pBlob = nullptr;
	}
	m_bValid = false;
}

void DB::EventInfo::wipeNotify()
{
	if (!m_bValid)
		return;

	if (!markedRead())
		db_event_markRead(hContact, m_hEvent);
	Clist_RemoveEvent(-1, m_hEvent);
}

// could be displayed in a SRMM window
bool DB::EventInfo::isSrmm() const
{
	if (auto *et = DbEvent_GetType(szModule, eventType))
		return (et->flags & DETF_MSGWINDOW) != 0;
	
	return (eventType == EVENTTYPE_MESSAGE || eventType == EVENTTYPE_FILE);
}

// could be displayed in a history window
bool DB::EventInfo::isHistory() const
{
	if (auto *et = DbEvent_GetType(szModule, eventType))
		return (et->flags & DETF_HISTORY) != 0;

	return (eventType == EVENTTYPE_MESSAGE || eventType == EVENTTYPE_FILE);
}

bool DB::EventInfo::isAlertable() const
{
	if (eventType == EVENTTYPE_MESSAGE)
		return true;

	if (eventType == EVENTTYPE_FILE) {
		DB::FILE_BLOB blob(*this);
		return blob.isOffline();
	}

	return false;
}

wchar_t* DB::EventInfo::getString(const char *str) const
{
	if (flags & DBEF_UTF)
		return mir_utf8decodeW(str);

	return mir_a2u(str);
}

/////////////////////////////////////////////////////////////////////////////////////////

static wchar_t* getEventString(const DB::EventInfo *dbei, LPSTR &buf)
{
	LPSTR in = buf;
	buf += mir_strlen(buf) + 1;
	return dbei->getString(in);
}

wchar_t* DB::EventInfo::getText() const
{
	DBEVENTTYPEDESCR *et = DbEvent_GetType(szModule, eventType);
	if (et && ServiceExists(et->textService))
		return (wchar_t *)CallService(et->textService, (WPARAM)this, DBVT_WCHAR);

	if (!pBlob)
		return 0;

	if (eventType == EVENTTYPE_AUTHREQUEST || eventType == EVENTTYPE_ADDED) {
		DB::AUTH_BLOB blob(pBlob);

		ptrW tszNick(getString(blob.get_nick()));
		ptrW tszFirst(getString(blob.get_firstName()));
		ptrW tszLast(getString(blob.get_lastName()));
		ptrW tszEmail(getString(blob.get_email()));

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

		if (eventType == EVENTTYPE_AUTHREQUEST) {
			ptrW tszReason(getString(blob.get_reason()));
			text.Format(TranslateT("Authorization request from %s%s: %s"),
				(tszNick == nullptr) ? Clist_GetContactDisplayName(blob.get_contact()) : tszNick.get(), nick.c_str(), tszReason.get());
		}
		else text.Format(TranslateT("You were added by %s%s"),
			(tszNick == nullptr) ? Clist_GetContactDisplayName(blob.get_contact()) : tszNick.get(), nick.c_str());
		return mir_wstrdup(text);
	}

	if (eventType == EVENTTYPE_CONTACTS) {
		CMStringW text(TranslateT("Contacts: "));
		// blob is: [uin(ASCIIZ), nick(ASCIIZ)]*
		char *buf = LPSTR(pBlob), *limit = LPSTR(pBlob) + cbBlob;
		while (buf < limit) {
			ptrW tszUin(getEventString(this, buf));
			ptrW tszNick(getEventString(this, buf));
			if (tszNick && *tszNick)
				text.AppendFormat(L"\"%s\" ", tszNick.get());
			if (tszUin && *tszUin)
				text.AppendFormat(L"<%s>; ", tszUin.get());
		}
		return mir_wstrdup(text);
	}

	if (eventType == EVENTTYPE_FILE) {
		DB::FILE_BLOB blob(*this);

		CMStringW wszText(blob.getName());
		if (mir_wstrlen(blob.getDescr())) {
			wszText.Append(L": ");
			wszText.Append(blob.getDescr());
		}
		return wszText.Detach();
	}

	if (flags & DBEF_JSON) {
		JSONNode json = getJson();
		std::string str = json["b"].as_string();
		return mir_utf8decodeW(str.c_str());
	}

	// by default treat an event's blob as a string
	char *str = (char *)alloca(cbBlob + 1);
	memcpy(str, pBlob, cbBlob);
	str[cbBlob] = 0;

	if (flags & DBEF_UTF)
		return mir_utf8decodeW(str);

	return mir_a2u_cp(str, CP_ACP);
}

void DB::EventInfo::flushJson()
{
	if (!m_json)
		return;

	std::string text = m_json->write();
	cbBlob = (int)text.size() + 1;
	pBlob = (char *)mir_realloc(pBlob, cbBlob);
	memcpy(pBlob, text.c_str(), cbBlob);
}

JSONNode& DB::EventInfo::setJson()
{
	if (!(flags & DBEF_JSON)) {
		flags |= DBEF_JSON;

		if (m_json == nullptr)
			m_json = new JSONNode(JSON_NODE);
		if (pBlob)
			*m_json << CHAR_PARAM("b", (char *)pBlob);
	}

	return *m_json;
}

void DB::EventInfo::addReaction(const char *emoji)
{
	if (!mir_strlen(emoji))
		return;

	auto &json = setJson();
	JSONNode &reactions = json["r"];
	if (!reactions) {
		reactions = JSONNode(JSON_NODE); reactions.set_name("r");
		json.push_back(reactions);
	}

	auto it = reactions.find(emoji);
	if (it == reactions.end())
		reactions << INT_PARAM(emoji, 1);
	else
		(*it) = JSONNode(emoji, (*it).as_int() + 1);

	flushJson();
}

void DB::EventInfo::delReaction(const char *emoji)
{
	if (!mir_strlen(emoji))
		return;

	auto &json = setJson();
	auto &reactions = json["r"];
	auto it = reactions.find(emoji);
	if (it != reactions.end()) {
		JSONNode &n = *it;
		if (n.as_int() > 1)
			n = JSONNode(emoji, n.as_int() - 1);
		else
			reactions.erase(it);

		flushJson();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// File blob helper

DB::FILE_BLOB::FILE_BLOB(void *pUserInfo, const char *pszName, const char *pszDescr, bool bUtf) :
	m_wszFileName(bUtf ? mir_utf8decodeW(pszName) : mir_a2u(pszName)),
	m_wszDescription(bUtf ? mir_utf8decodeW(pszDescr) : mir_a2u(pszDescr)),
	m_pUserInfo(pUserInfo)
{}

DB::FILE_BLOB::FILE_BLOB(const wchar_t *pwszName, const wchar_t *pwszDescr) :
	m_wszFileName(mir_wstrdup(pwszName)),
	m_wszDescription(mir_wstrdup(pwszDescr))
{}

DB::FILE_BLOB::FILE_BLOB(const DB::EventInfo &dbei)
{
	auto &root = dbei.getJson();
	if (&root) {
		m_wszFileName = root["f"].as_mstring().Detach();
		m_wszDescription = root["d"].as_mstring().Detach();

		if (auto &node = root["lf"]) {
			CMStringW tmp = node.as_mstring();
			tmp.Replace(L"\\\\", L"\\");
			m_wszLocalName = tmp.Detach();
		}

		CMStringA szProtoString(root["u"].as_mstring());
		if (!szProtoString.IsEmpty()) {
			m_szProtoString = szProtoString.Detach();
			m_iFileSize = root["fs"].as_int();
			m_iTransferred = root["ft"].as_int();
		}		
	}
}

DB::FILE_BLOB::~FILE_BLOB()
{}

bool DB::FILE_BLOB::isCompleted() const
{
	if (m_iFileSize == -1)
		return false;

	return m_iFileSize != 0 && m_iFileSize == m_iTransferred;
}

void DB::FILE_BLOB::write(DB::EventInfo &dbei)
{
	auto &root = dbei.setJson();
	root << WCHAR_PARAM("f", m_wszFileName) << WCHAR_PARAM("d", m_wszDescription ? m_wszDescription : L"");
	if (isOffline()) {
		root << CHAR_PARAM("u", m_szProtoString) << INT_PARAM("fs", m_iFileSize) << INT_PARAM("ft", m_iTransferred);
		if (mir_wstrlen(m_wszLocalName))
			root << WCHAR_PARAM("lf", m_wszLocalName);
	}

	dbei.flushJson();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Auth blob helper

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

DB::AUTH_BLOB::AUTH_BLOB(char *blob)
{
	char *pCurBlob = blob;
	m_dwUin = *(uint32_t*)pCurBlob;
	pCurBlob += sizeof(uint32_t);
	m_hContact = *(uint32_t*)pCurBlob;
	pCurBlob += sizeof(uint32_t);
	m_szNick = mir_strdup(pCurBlob); pCurBlob += mir_strlen(pCurBlob) + 1;
	m_szFirstName = mir_strdup(pCurBlob); pCurBlob += mir_strlen(pCurBlob) + 1;
	m_szLastName = mir_strdup(pCurBlob); pCurBlob += mir_strlen(pCurBlob) + 1;
	m_szEmail = mir_strdup(pCurBlob); pCurBlob += mir_strlen(pCurBlob) + 1;
	m_szReason = mir_strdup(pCurBlob); pCurBlob += mir_strlen(pCurBlob) + 1;
	m_size = uint32_t(pCurBlob - blob);
}

DB::AUTH_BLOB::~AUTH_BLOB()
{
}

char* DB::AUTH_BLOB::makeBlob()
{
	char *pBlob, *pCurBlob;
	pCurBlob = pBlob = (char *)mir_alloc(m_size + 1);

	*((uint32_t*)pCurBlob) = m_dwUin;
	pCurBlob += sizeof(uint32_t);
	*((uint32_t*)pCurBlob) = (uint32_t)m_hContact;
	pCurBlob += sizeof(uint32_t);

	mir_snprintf(pCurBlob, m_size - 8, "%s%c%s%c%s%c%s%c%s%c",
		(m_szNick) ? m_szNick.get() : "", 0,
		(m_szFirstName) ? m_szFirstName.get() : "", 0,
		(m_szLastName) ? m_szLastName.get() : "", 0,
		(m_szEmail) ? m_szEmail.get() : "", 0,
		(m_szReason) ? m_szReason.get() : "", 0);

	return pBlob;
}

/////////////////////////////////////////////////////////////////////////////////////////

static uint32_t dwPreviousTimeStamp = -1;
static MCONTACT hPreviousContact = INVALID_CONTACT_ID;
static MEVENT hPreviousDbEvent = 0;

// Returns TRUE if the event already exist in the database
MIR_APP_DLL(bool) DB::IsDuplicateEvent(MCONTACT hContact, DBEVENTINFO &dbei)
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
