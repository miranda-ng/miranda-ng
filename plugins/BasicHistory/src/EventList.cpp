/*
Basic History plugin
Copyright (C) 2011-2012 Krzysztof Kral

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "StdAfx.h"
#include "EventList.h"
#include "Options.h"
#include "ExportManager.h"
#include <assert.h>

extern int iconsNum;

static mir_cs csEventList;

bool DeleteDirectory(LPCTSTR lpszDir, bool noRecycleBin = true);
std::wstring GetName(const std::wstring &path);

HistoryEventList::HistoryEventList() :
	m_hWnd(NULL),
	m_isWnd(false),
	m_hContact(NULL),
	m_deltaTime(0),
	m_isFlat(false),
	m_useImportedMessages(true)
{
	memset(&m_dbei, 0, sizeof(DBEVENTINFO));
	m_dbei.cbSize = sizeof(DBEVENTINFO);
	m_oldBlobSize = 0;
}

HistoryEventList::HistoryEventList(MCONTACT _hContact, int filter) :
	m_hWnd(NULL),
	m_isWnd(false),
	m_hContact(_hContact),
	m_deltaTime(0),
	m_isFlat(false),
	m_useImportedMessages(true)
{
	memset(&m_dbei, 0, sizeof(DBEVENTINFO));
	m_dbei.cbSize = sizeof(DBEVENTINFO);
	m_oldBlobSize = 0;
	SetDefFilter(filter);
}

HistoryEventList::~HistoryEventList()
{
	mir_free(m_dbei.pBlob);
	m_eventList.clear();
}

bool HistoryEventList::CanShowHistory(DBEVENTINFO* dbei)
{
	if (m_deltaTime != 0) {
		if (m_deltaTime > 0) {
			if (m_now - m_deltaTime < dbei->timestamp)
				return false;
		}
		else {
			if (m_now + m_deltaTime > dbei->timestamp)
				return false;
		}
	}

	if (m_hContact == NULL || m_defFilter == 1)
		return true;

	if (m_defFilter < 1) {
		switch(dbei->eventType) {
		case EVENTTYPE_MESSAGE:
		case EVENTTYPE_URL:
		case EVENTTYPE_FILE:
			return true;

		default:
			DBEVENTTYPEDESCR *et = (DBEVENTTYPEDESCR*)CallService(MS_DB_EVENT_GETTYPE, ( WPARAM )dbei->szModule, ( LPARAM )dbei->eventType);
			if (et && ( et->flags & DETF_HISTORY))
				return true;
		}

		return false;
	}

	if (m_filterMap.find(dbei->eventType) != m_filterMap.end()) {
		if (m_onlyInFilter)
			return !(dbei->flags & DBEF_SENT);

		if (m_onlyOutFilter)
			return (dbei->flags & DBEF_SENT) != 0;

		return true;
	}
	return false;
}

bool HistoryEventList::CanShowHistory(const IImport::ExternalMessage &message)
{
	if (m_deltaTime != 0) {
		if (m_deltaTime > 0) {
			if (m_now - m_deltaTime < message.timestamp)
				return false;
		}
		else {
			if (m_now + m_deltaTime > message.timestamp)
				return false;
		}
	}

	if (m_hContact == NULL || m_defFilter == 1)
		return true;

	if (m_defFilter < 1) {
		switch(message.eventType ) {
		case EVENTTYPE_MESSAGE:
		case EVENTTYPE_URL:
		case EVENTTYPE_FILE:
			return true;
		}

		return false;
	}

	if (m_filterMap.find(message.eventType) != m_filterMap.end()) {
		if (m_onlyInFilter)
			return !(message.flags & DBEF_SENT);

		if (m_onlyOutFilter)
			return (message.flags & DBEF_SENT) != 0;

		return true;
	}
	return false;
}

void HistoryEventList::InitFilters()
{
	m_filterMap.clear();
	m_onlyInFilter = false;
	m_onlyOutFilter = false;
	if (m_defFilter >= 2) {
		m_defFilter = 0;
		for (int i = 0; i < (int)Options::instance->customFilters.size(); ++i) {
			if (m_filterName == Options::instance->customFilters[i].name) {
				m_defFilter = i + 2;
				if (Options::instance->customFilters[i].onlyIncomming && !Options::instance->customFilters[i].onlyOutgoing)
					m_onlyInFilter = true;
				else if (Options::instance->customFilters[i].onlyOutgoing && !Options::instance->customFilters[i].onlyIncomming)
					m_onlyOutFilter = true;

				for (std::vector<int>::iterator it = Options::instance->customFilters[i].events.begin(); it != Options::instance->customFilters[i].events.end(); ++it)
					m_filterMap[*it] = true;
				break;
			}
		}
	}
	else m_filterName = L"";
}

void HistoryEventList::SetDefFilter(int filter)
{
	m_defFilter = filter;
	if (filter >= 2 && filter - 2 < (int)Options::instance->customFilters.size())
		m_filterName = Options::instance->customFilters[filter - 2].name;
	else if (filter == 1)
		m_filterName = TranslateT("All events");
	else
		m_filterName = TranslateT("Default history events");
}

int HistoryEventList::GetFilterNr()
{
	return m_defFilter;
}

std::wstring HistoryEventList::GetFilterName()
{
	return m_filterName;
}

void HistoryEventList::GetTempList(std::list<EventTempIndex>& tempList, bool noFilter, bool noExt, MCONTACT hContact)
{
	bool isWndLocal = m_isWnd;
	EventTempIndex ti;
	EventData data;
	EventIndex ei;
	ti.isExternal = false;
	ei.isExternal = false;
	MEVENT hDbEvent = db_event_first(hContact);
	while (hDbEvent != NULL) {
		if (isWndLocal && !IsWindow(m_hWnd))
			break;

		ei.hEvent = hDbEvent;
		if (GetEventData(ei, data)) {
			if (noFilter || CanShowHistory(&m_dbei)) {
				ti.hEvent = hDbEvent;
				ti.timestamp = data.timestamp;
				tempList.push_back(ti);
			}
		}
		hDbEvent = db_event_next(hContact, hDbEvent);
	}

	if (!noExt) {
		std::list<EventTempIndex>::iterator itL = tempList.begin();
		ti.isExternal = true;
		for (int i = 0; i < (int)m_importedMessages.size(); ++i) {
			if (noFilter || CanShowHistory(m_importedMessages[i])) {
				DWORD ts = m_importedMessages[i].timestamp;
				while(itL != tempList.end() && itL->timestamp < ts)++itL;
				if (itL == tempList.end() || itL->timestamp > ts) {
					ti.exIdx = i;
					ti.timestamp = ts;
					tempList.insert(itL, ti);
				}
			}
		}
	}
}

void HistoryEventList::RefreshEventList()
{
	InitNames();
	InitFilters();

	if (m_useImportedMessages) {
		std::vector<IImport::ExternalMessage> messages;
		{
			mir_cslock lck(csEventList);
			std::map<MCONTACT, HistoryEventList::ImportDiscData>::iterator it = m_contactFileMap.find(m_hContact);
			if (it != m_contactFileMap.end()) {
				ExportManager imp(m_hWnd, m_hContact, 1);
				imp.SetAutoImport(it->second.file);
				if (!imp.Import(it->second.type, messages, NULL))
					messages.clear();
			}
		}
		ImportMessages(messages);
	}

	std::list<EventTempIndex> tempList;
	GetTempList(tempList, false, false, m_hContact);
	std::list<EventTempIndex> revTempList;
	std::list<EventTempIndex>& nrTempList = tempList;
	bool isNewOnTop = Options::instance->groupNewOnTop;
	if (isNewOnTop) {
		revTempList.insert(revTempList.begin(), tempList.rbegin(), tempList.rend());
		nrTempList = revTempList;
	}

	m_eventList.clear();
	m_eventList.push_back(std::deque<EventIndex>());
	DWORD lastTime = MAXDWORD;
	DWORD groupTime = Options::instance->groupTime * 60 * 60;
	int maxMess = Options::instance->groupMessagesNumber;
	int limitator = 0;
	EventIndex ei;
	for (std::list<EventTempIndex>::iterator itL = nrTempList.begin(); itL != nrTempList.end(); ++itL) {
		DWORD tm = isNewOnTop ? lastTime - itL->timestamp : itL->timestamp - lastTime;
		if (m_isFlat || tm < groupTime && limitator < maxMess) {
			lastTime = itL->timestamp;
			ei.isExternal = itL->isExternal;
			ei.hEvent = itL->hEvent;
			if (isNewOnTop)
				m_eventList.back().push_front(ei);
			else
				m_eventList.back().push_back(ei);
			++limitator;
		}
		else {
			limitator = 0;
			lastTime = itL->timestamp;
			if (!m_eventList.back().empty()) {
				ei = m_eventList.back().front();
				AddGroup(ei);
				m_eventList.push_back(std::deque<EventIndex>());
			}
			ei.isExternal = itL->isExternal;
			ei.hEvent = itL->hEvent;
			m_eventList.back().push_front(ei);
		}
	}

	if (!m_eventList.back().empty()) {
		ei = m_eventList.back().front();
		AddGroup(ei);
	}
}

bool HistoryEventList::SearchInContact(MCONTACT hContact, TCHAR *strFind, ComparatorInterface* compFun)
{
	InitFilters();

	if (m_useImportedMessages) {
		std::vector<IImport::ExternalMessage> messages;
		{
			mir_cslock lck(csEventList);
			std::map<MCONTACT, HistoryEventList::ImportDiscData>::iterator it = m_contactFileMap.find(hContact);
			if (it != m_contactFileMap.end()) {
				ExportManager imp(m_hWnd, hContact, 1);
				imp.SetAutoImport(it->second.file);
				if (!imp.Import(it->second.type, messages, NULL))
					messages.clear();
			}
		}

		for (int i = 0; i < (int)m_importedMessages.size(); ++i)
			if (compFun->Compare((m_importedMessages[i].flags & DBEF_SENT) != 0, m_importedMessages[i].message, strFind))
				return true;
	}

	std::list<EventTempIndex> tempList;
	GetTempList(tempList, false, true, hContact);

	EventIndex ei;
	EventData ed;
	TCHAR str[MAXSELECTSTR + 8]; // for safety reason
	for (std::list<EventTempIndex>::iterator itL = tempList.begin(); itL != tempList.end(); ++itL) {
		ei.isExternal = itL->isExternal;
		ei.hEvent = itL->hEvent;
		if (GetEventData(ei, ed)) {
			GetEventMessage(ei, str);
			if (compFun->Compare(ed.isMe, str, strFind))
				return true;
		}
	}

	return false;
}

void HistoryEventList::InitNames()
{
	TCHAR str[200];
	if (m_hContact) {
		_tcscpy_s(m_contactName, pcli->pfnGetContactDisplayName(m_hContact, 0));
		mir_sntprintf(str, _countof(str), TranslateT("History for %s"), m_contactName);
	}
	else {
		_tcscpy_s(m_contactName, TranslateT("System"));
		mir_sntprintf(str, _countof(str), TranslateT("History"));
	}

	if (m_isWnd)
		SetWindowText(m_hWnd, str);

	_tcscpy_s(m_myName, GetMyName().c_str());
}

void HistoryEventList::AddGroup(const EventIndex& ev)
{
	EventData data;
	GetEventData(ev, data);
	TCHAR eventText[256];
	int i;
	eventText[0] = 0;
	TimeZone_PrintTimeStamp(NULL, data.timestamp, _T("d t"), eventText, 64, 0);
	std::wstring time = eventText;
	std::wstring user;
	if (data.isMe)
		user = m_myName;
	else
		user = m_contactName;
	GetEventMessage(ev, eventText, 256);
	for (i = 0; eventText[i] != 0 && eventText[i] != _T('\r') && eventText[i] != _T('\n'); ++i);
	eventText[i] = 0;
	if (i > Options::instance->groupMessageLen) {
		eventText[Options::instance->groupMessageLen - 3] = '.';
		eventText[Options::instance->groupMessageLen - 2] = '.';
		eventText[Options::instance->groupMessageLen - 1] = '.';
		eventText[Options::instance->groupMessageLen] = 0;
	}

	int ico = 0;
	GetEventIcon(data.isMe, data.eventType, ico);
	AddGroup(data.isMe, time, user, eventText, ico);
}

std::wstring HistoryEventList::GetContactName()
{
	if (m_hContact)
		return pcli->pfnGetContactDisplayName(m_hContact, 0);

	return TranslateT("System");
}

void GetInfo(CONTACTINFO& ci, std::wstring& str)
{
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
		if (ci.type == CNFT_ASCIIZ) {
			str = ci.pszVal;
			mir_free(ci.pszVal);
		}
		else if (ci.type == CNFT_DWORD) {
			TCHAR buf[20];
			_ltot_s(ci.dVal, buf, 10);
			str = buf;
		}
		else if (ci.type == CNFT_WORD) {
			TCHAR buf[20];
			_ltot_s(ci.wVal, buf, 10);
			str = buf;
		}
	}
}

std::wstring HistoryEventList::GetMyName()
{
	std::wstring myName;
	CONTACTINFO ci;
	memset(&ci, 0, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.szProto = GetContactProto(m_hContact);
	ci.hContact = 0;
	ci.dwFlag = CNF_DISPLAY | CNF_TCHAR;
	GetInfo(ci, myName);
	if (myName.empty())
		return TranslateT("Me");

	return myName;
}

inline std::wstring GetProtocolName(MCONTACT hContact)
{
	char* ac = Proto_GetBaseAccountName(hContact);
	std::wstring proto1;
	if (ac != NULL) {
		PROTOACCOUNT* acnt = Proto_GetAccount(ac);
		if (acnt != NULL && acnt->szModuleName != NULL) {
			wchar_t* proto = mir_a2u(acnt->szProtoName);
			proto1 = proto;
			mir_free(proto);
		}
	}

	return proto1;
}

std::wstring HistoryEventList::GetProtocolName()
{
	return ::GetProtocolName(m_hContact);
}

std::string HistoryEventList::GetBaseProtocol()
{
	char* proto = GetContactProto(m_hContact);
	return proto == NULL ? "" : proto;
}

std::wstring HistoryEventList::GetMyId()
{
	std::wstring myId;
	CONTACTINFO ci;
	memset(&ci, 0, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.szProto = GetContactProto(m_hContact);
	ci.hContact = 0;
	ci.dwFlag = CNF_DISPLAYUID | CNF_TCHAR;
	GetInfo(ci, myId);
	return myId;
}

inline std::wstring GetContactId(MCONTACT hContact)
{
	std::wstring id;
	CONTACTINFO ci;
	memset(&ci, 0, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.szProto = GetContactProto(hContact);
	ci.hContact = hContact;
	ci.dwFlag = CNF_DISPLAYUID | CNF_TCHAR;
	GetInfo(ci, id);
	return id;
}

std::wstring HistoryEventList::GetContactId()
{
	return ::GetContactId(m_hContact);
}

static void GetMessageDescription(DBEVENTINFO *dbei, TCHAR* buf, int cbBuf)
{
	TCHAR *msg = DbGetEventTextT(dbei, CP_ACP);
	_tcsncpy_s(buf, cbBuf, msg ? msg : TranslateT("Invalid Message"), _TRUNCATE);
	buf[cbBuf - 1] = 0;
	mir_free(msg);
}

void HistoryEventList::GetObjectDescription(DBEVENTINFO *dbei, TCHAR* str, int cbStr)
{
	GetMessageDescription(dbei, str, cbStr);
}

bool HistoryEventList::GetEventIcon(bool isMe, int eventType, int &id)
{
	switch (eventType) {
	case EVENTTYPE_MESSAGE:
		id = isMe ? 1 : 0;
		return true;
	case EVENTTYPE_FILE:
		id = iconsNum;
		return true;
	case EVENTTYPE_URL:
		id = iconsNum + 1;
		return true;
	case EVENTTYPE_AUTHREQUEST:
		id = iconsNum + 2;
		return true;
	default:
		id = isMe ? 1 : 0;
		return false;
	}
}

void HistoryEventList::ImportMessages(const std::vector<IImport::ExternalMessage>& messages)
{
	DWORD lastTime = 0;
	m_importedMessages.clear();
	for (int i = 0; i < (int)messages.size(); ++i) {
		if (messages[i].timestamp >= lastTime) {
			m_importedMessages.push_back(messages[i]);
			lastTime = messages[i].timestamp;
		}
		else {
			assert(FALSE);
		}
	}
}

void HistoryEventList::MargeMessages(const std::vector<IImport::ExternalMessage>& messages)
{
	ImportMessages(messages);
	std::list<EventTempIndex> tempList;
	GetTempList(tempList, true, false, m_hContact);

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = GetContactProto(m_hContact);

	CallService(MS_DB_SETSAFETYMODE, FALSE, 0);
	for (std::list<EventTempIndex>::iterator it = tempList.begin(); it != tempList.end(); ++it) {
		if (it->isExternal) {
			IImport::ExternalMessage& msg = m_importedMessages[it->exIdx];
			dbei.flags |= DBEF_READ;
			dbei.timestamp = msg.timestamp;
			// For now I do not convert event data from string to blob, and event type must be message to handle it properly
			dbei.eventType = EVENTTYPE_MESSAGE;
			UINT cp = dbei.flags & DBEF_UTF ? CP_UTF8 : CP_ACP;
			dbei.cbBlob = WideCharToMultiByte(cp, 0, msg.message.c_str(), (int)msg.message.length() + 1, NULL, 0, NULL, NULL);
			char* buf = new char[dbei.cbBlob];
			dbei.cbBlob = WideCharToMultiByte(cp, 0, msg.message.c_str(), (int)msg.message.length() + 1, buf, dbei.cbBlob, NULL, NULL);
			dbei.pBlob = (PBYTE)buf;
			db_event_add(m_hContact, &dbei);
			delete[] buf;
		}
	}

	CallService(MS_DB_SETSAFETYMODE, TRUE, 0);
	std::vector<IImport::ExternalMessage> emessages;
	ImportMessages(emessages);
}

bool HistoryEventList::GetEventData(const EventIndex& ev, EventData& data)
{
	if (!ev.isExternal) {
		DWORD newBlobSize = db_event_getBlobSize(ev.hEvent);
		if (newBlobSize > m_oldBlobSize) {
			m_dbei.pBlob = (PBYTE)mir_realloc(m_dbei.pBlob, newBlobSize);
			m_oldBlobSize = newBlobSize;
		}

		m_dbei.cbBlob = m_oldBlobSize;
		if (db_event_get(ev.hEvent, &m_dbei) == 0) {
			data.isMe = (m_dbei.flags & DBEF_SENT) != 0;
			data.eventType = m_dbei.eventType;
			data.timestamp = m_dbei.timestamp;
			return true;
		}
	}
	else if (ev.exIdx >= 0 && ev.exIdx < (int)m_importedMessages.size()) {
		IImport::ExternalMessage& em = m_importedMessages[ev.exIdx];
		data.isMe = (em.flags & DBEF_SENT) != 0;
		data.eventType = em.eventType;
		data.timestamp = em.timestamp;
		return true;
	}

	return false;
}

void HistoryEventList::GetExtEventDBei(const EventIndex& ev)
{
	IImport::ExternalMessage& em = m_importedMessages[ev.exIdx];
	m_dbei.flags = em.flags | 0x800;
	m_dbei.eventType = em.eventType;
	m_dbei.timestamp = em.timestamp;
}

HICON HistoryEventList::GetEventCoreIcon(const EventIndex& ev)
{
	if (ev.isExternal)
		return NULL;

	HICON ico = (HICON)CallService(MS_DB_EVENT_GETICON, LR_SHARED, (LPARAM)&m_dbei);
	HICON icoMsg = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
	if (icoMsg == ico)
		return NULL;

	return ico;
}

void HistoryEventList::RebuildGroup(int selected)
{
	std::deque<EventIndex> newGroup;
	for (size_t i = 0; i < m_eventList[selected].size(); ++i) {
		EventIndex& ev = m_eventList[selected][i];
		if (!ev.isExternal) {
			// If event exist, we add it to new group
			if (db_event_getBlobSize(ev.hEvent) >= 0)
				newGroup.push_back(m_eventList[selected][i]);
		}
		else newGroup.push_back(m_eventList[selected][i]);
	}
	m_eventList[selected].clear();
	m_eventList[selected].insert(m_eventList[selected].begin(), newGroup.begin(), newGroup.end());
}

std::map<MCONTACT, HistoryEventList::ImportDiscData> HistoryEventList::m_contactFileMap;
std::wstring HistoryEventList::m_contactFileDir;

void HistoryEventList::AddImporter(MCONTACT hContact, IImport::ImportType type, const std::wstring& file)
{
	mir_cslock lck(csEventList);

	TCHAR buf[32];
	mir_sntprintf(buf, _T("%016llx"), (unsigned long long int)hContact);
	ImportDiscData data;
	data.file = m_contactFileDir + buf;
	data.type = type;
	CopyFile(file.c_str(), data.file.c_str(), FALSE);
	m_contactFileMap[hContact] = data;
}

void HistoryEventList::Init()
{
	TCHAR temp[MAX_PATH];
	temp[0] = 0;
	GetTempPath(MAX_PATH, temp);
	m_contactFileDir = temp;
	m_contactFileDir += L"BasicHistoryImportDir\\";
	DeleteDirectory(m_contactFileDir.c_str());
	CreateDirectory(m_contactFileDir.c_str(), NULL);
}

int HistoryEventList::GetContactMessageNumber(MCONTACT hContact)
{
	int count = db_event_count(hContact);

	mir_cslock lck(csEventList);
	std::map<MCONTACT, HistoryEventList::ImportDiscData>::iterator it = m_contactFileMap.find(hContact);
	if (it != m_contactFileMap.end())
		++count;
	return count;
}

bool HistoryEventList::IsImportedHistory(MCONTACT hContact)
{
	bool count = false;

	mir_cslock lck(csEventList);
	std::map<MCONTACT, HistoryEventList::ImportDiscData>::iterator it = m_contactFileMap.find(hContact);
	if (it != m_contactFileMap.end())
		count = true;
	return count;
}

void HistoryEventList::DeleteImporter(MCONTACT hContact)
{
	mir_cslock lck(csEventList);

	std::map<MCONTACT, HistoryEventList::ImportDiscData>::iterator it = m_contactFileMap.find(hContact);
	if (it != m_contactFileMap.end()) {
		DeleteFile(it->second.file.c_str());
		m_contactFileMap.erase(it);
	}
}
