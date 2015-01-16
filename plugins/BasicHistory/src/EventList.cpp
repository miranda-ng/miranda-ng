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

EventList::EventList()
	:hWnd(NULL),
	isWnd(false),
	hContact(NULL),
	deltaTime(0),
	isFlat(false),
	useImportedMessages(true)
{
	memset(&gdbei, 0, sizeof(DBEVENTINFO));
	gdbei.cbSize = sizeof(DBEVENTINFO);
	goldBlobSize = 0;
}

EventList::EventList(MCONTACT _hContact, int filter)
	:hWnd(NULL),
	isWnd(false),
	hContact(_hContact),
	deltaTime(0),
	isFlat(false),
	useImportedMessages(true)
{
	memset(&gdbei, 0, sizeof(DBEVENTINFO));
	gdbei.cbSize = sizeof(DBEVENTINFO);
	goldBlobSize = 0;
	SetDefFilter(filter);
}

EventList::~EventList()
{
	mir_free(gdbei.pBlob);
	eventList.clear();
}

bool EventList::CanShowHistory(DBEVENTINFO* dbei)
{
	if (deltaTime != 0) {
		if (deltaTime > 0) {
			if (now - deltaTime < dbei->timestamp)
				return false;
		}
		else {
			if (now + deltaTime > dbei->timestamp)
				return false;
		}
	}

	if (hContact == NULL || defFilter == 1)
		return true;

	if (defFilter < 1) {
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

	if (filterMap.find(dbei->eventType) != filterMap.end()) {
		if (onlyInFilter)
			return !(dbei->flags & DBEF_SENT);

		if (onlyOutFilter)
			return (dbei->flags & DBEF_SENT) != 0;

		return true;
	}
	return false;
}

bool EventList::CanShowHistory(const IImport::ExternalMessage& message)
{
	if (deltaTime != 0) {
		if (deltaTime > 0) {
			if (now - deltaTime < message.timestamp)
				return false;
		}
		else {
			if (now + deltaTime > message.timestamp)
				return false;
		}
	}

	if (hContact == NULL || defFilter == 1)
		return true;

	if (defFilter < 1) {
		switch(message.eventType ) {
		case EVENTTYPE_MESSAGE:
		case EVENTTYPE_URL:
		case EVENTTYPE_FILE:
			return true;
		}

		return false;
	}

	if (filterMap.find(message.eventType) != filterMap.end()) {
		if (onlyInFilter)
			return !(message.flags & DBEF_SENT);

		if (onlyOutFilter)
			return (message.flags & DBEF_SENT) != 0;

		return true;
	}
	return false;
}

void EventList::InitFilters()
{
	filterMap.clear();
	onlyInFilter = false;
	onlyOutFilter = false;
	if (defFilter >= 2) {
		defFilter = 0;
		for (int i = 0; i < (int)Options::instance->customFilters.size(); ++i) {
			if (filterName == Options::instance->customFilters[i].name) {
				defFilter = i + 2;
				if (Options::instance->customFilters[i].onlyIncomming && !Options::instance->customFilters[i].onlyOutgoing)
					onlyInFilter = true;
				else if (Options::instance->customFilters[i].onlyOutgoing && !Options::instance->customFilters[i].onlyIncomming)
					onlyOutFilter = true;

				for (std::vector<int>::iterator it = Options::instance->customFilters[i].events.begin(); it != Options::instance->customFilters[i].events.end(); ++it)
					filterMap[*it] = true;
				break;
			}
		}
	}
	else filterName = L"";
}

void EventList::SetDefFilter(int filter)
{
	defFilter = filter;
	if (filter >= 2 && filter - 2 < (int)Options::instance->customFilters.size())
		filterName = Options::instance->customFilters[filter - 2].name;
	else if (filter == 1)
		filterName = TranslateT("All events");
	else
		filterName = TranslateT("Default history events");
}

int EventList::GetFilterNr()
{
	return defFilter;
}

std::wstring EventList::GetFilterName()
{
	return filterName;
}

void EventList::GetTempList(std::list<EventTempIndex>& tempList, bool noFilter, bool noExt, MCONTACT _hContact)
{
	bool isWndLocal = isWnd;
	EventTempIndex ti;
	EventData data;
	EventIndex ei;
	ti.isExternal = false;
	ei.isExternal = false;
	MEVENT hDbEvent = db_event_first(_hContact);
	while (hDbEvent != NULL) {
		if (isWndLocal && !IsWindow(hWnd))
			break;

		ei.hEvent = hDbEvent;
		if (GetEventData(ei, data)) {
			if (noFilter || CanShowHistory(&gdbei)) {
				ti.hEvent = hDbEvent;
				ti.timestamp = data.timestamp;
				tempList.push_back(ti);
			}
		}
		hDbEvent = db_event_next(_hContact, hDbEvent);
	}

	if (!noExt) {
		std::list<EventTempIndex>::iterator itL = tempList.begin();
		ti.isExternal = true;
		for (int i = 0; i < (int)importedMessages.size(); ++i) {
			if (noFilter || CanShowHistory(importedMessages[i])) {
				DWORD ts = importedMessages[i].timestamp;
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

void EventList::RefreshEventList()
{
	InitNames();
	InitFilters();

	if (useImportedMessages) {
		std::vector<IImport::ExternalMessage> messages;
		{
			mir_cslock lck(csEventList);
			std::map<MCONTACT, EventList::ImportDiscData>::iterator it = contactFileMap.find(hContact);
			if (it != contactFileMap.end()) {
				ExportManager imp(hWnd, hContact, 1);
				imp.SetAutoImport(it->second.file);
				if (!imp.Import(it->second.type, messages, NULL))
					messages.clear();
			}
		}
		ImportMessages(messages);
	}

	std::list<EventTempIndex> tempList;
	GetTempList(tempList, false, false, hContact);
	std::list<EventTempIndex> revTempList;
	std::list<EventTempIndex>& nrTempList = tempList;
	bool isNewOnTop = Options::instance->groupNewOnTop;
	if (isNewOnTop) {
		revTempList.insert(revTempList.begin(), tempList.rbegin(), tempList.rend());
		nrTempList = revTempList;
	}

	eventList.clear();
	eventList.push_back(std::deque<EventIndex>());
	DWORD lastTime = MAXDWORD;
	DWORD groupTime = Options::instance->groupTime * 60 * 60;
	int maxMess = Options::instance->groupMessagesNumber;
	int limitator = 0;
	EventIndex ei;
	for (std::list<EventTempIndex>::iterator itL = nrTempList.begin(); itL != nrTempList.end(); ++itL) {
		DWORD tm = isNewOnTop ? lastTime - itL->timestamp : itL->timestamp - lastTime;
		if (isFlat || tm < groupTime && limitator < maxMess) {
			lastTime = itL->timestamp;
			ei.isExternal = itL->isExternal;
			ei.hEvent = itL->hEvent;
			if (isNewOnTop)
				eventList.back().push_front(ei);
			else
				eventList.back().push_back(ei);
			++limitator;
		}
		else {
			limitator = 0;
			lastTime = itL->timestamp;
			if (!eventList.back().empty()) {
				ei = eventList.back().front();
				AddGroup(ei);
				eventList.push_back(std::deque<EventIndex>());
			}
			ei.isExternal = itL->isExternal;
			ei.hEvent = itL->hEvent;
			eventList.back().push_front(ei);
		}
	}

	if (!eventList.back().empty()) {
		ei = eventList.back().front();
		AddGroup(ei);
	}
}

bool EventList::SearchInContact(MCONTACT hContact, TCHAR *strFind, ComparatorInterface* compFun)
{
	InitFilters();

	if (useImportedMessages) {
		std::vector<IImport::ExternalMessage> messages;
		{
			mir_cslock lck(csEventList);
			std::map<MCONTACT, EventList::ImportDiscData>::iterator it = contactFileMap.find(hContact);
			if (it != contactFileMap.end()) {
				ExportManager imp(hWnd, hContact, 1);
				imp.SetAutoImport(it->second.file);
				if (!imp.Import(it->second.type, messages, NULL))
					messages.clear();
			}
		}

		for (int i = 0; i < (int)importedMessages.size(); ++i)
			if (compFun->Compare((importedMessages[i].flags & DBEF_SENT) != 0, importedMessages[i].message, strFind))
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

void EventList::InitNames()
{
	TCHAR str[200];
	if (hContact) {
		_tcscpy_s(contactName, 256, (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR ));
		mir_sntprintf(str, SIZEOF(str), TranslateT("History for %s"),contactName);
	}
	else {
		_tcscpy_s(contactName, 256, TranslateT("System"));
		mir_sntprintf(str, SIZEOF(str), TranslateT("History"));
	}

	if (isWnd)
		SetWindowText(hWnd,str);

	_tcscpy_s(myName, GetMyName().c_str());
}

void EventList::AddGroup(const EventIndex& ev)
{
	EventData data;
	GetEventData(ev, data);
	TCHAR eventText[256];
	int i;
	eventText[0] = 0;
	tmi.printTimeStamp(NULL, data.timestamp, _T("d t"), eventText, 64, 0);
	std::wstring time = eventText;
	std::wstring user;
	if (data.isMe)
		user = myName;
	else
		user = contactName;
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

std::wstring EventList::GetContactName()
{
	if (hContact)
		return (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR );

	return TranslateT("System");
}

void GetInfo(CONTACTINFO& ci, std::wstring& str)
{
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
		if (ci.type == CNFT_ASCIIZ) {
			str =  ci.pszVal;
			mir_free(ci.pszVal);
		}
		else if (ci.type == CNFT_DWORD) {
			TCHAR buf[20];
			_ltot_s(ci.dVal, buf, 10 );
			str = buf;
		}
		else if (ci.type == CNFT_WORD) {
			TCHAR buf[20];
			_ltot_s(ci.wVal, buf, 10 );
			str = buf;
		}
	}
}

std::wstring EventList::GetMyName()
{
	std::wstring myName;
	CONTACTINFO ci;
	memset(&ci, 0, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.szProto = GetContactProto(hContact);
	ci.hContact = 0;
	ci.dwFlag = CNF_DISPLAY | CNF_TCHAR;
	GetInfo(ci, myName);
	if (myName.empty())
		return TranslateT("Me");

	return myName;
}

inline std::wstring GetProtocolName(MCONTACT hContact)
{
	char* ac = (char *)CallService(MS_PROTO_GETCONTACTBASEACCOUNT, hContact, 0);
	std::wstring proto1;
	if (ac != NULL) {
		PROTOACCOUNT* acnt = ProtoGetAccount(ac);
		if (acnt != NULL && acnt->szModuleName != NULL) {
			wchar_t* proto = mir_a2u(acnt->szProtoName);
			proto1 = proto;
			mir_free(proto);
		}
	}

	return proto1;
}

std::wstring EventList::GetProtocolName()
{
	return ::GetProtocolName(hContact);
}

std::string EventList::GetBaseProtocol()
{
	char* proto = GetContactProto(hContact);
	return proto == NULL ? "" : proto;
}

std::wstring EventList::GetMyId()
{
	std::wstring myId;
	CONTACTINFO ci;
	memset(&ci, 0, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.szProto = GetContactProto(hContact);
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

std::wstring EventList::GetContactId()
{
	return ::GetContactId(hContact);
}

static void GetMessageDescription( DBEVENTINFO *dbei, TCHAR* buf, int cbBuf )
{
	TCHAR *msg = DbGetEventTextT(dbei, CP_ACP);
	_tcsncpy_s(buf, cbBuf, msg ? msg : TranslateT("Invalid Message"), _TRUNCATE);
	buf[cbBuf - 1] = 0;
	mir_free(msg);
}

void EventList::GetObjectDescription( DBEVENTINFO *dbei, TCHAR* str, int cbStr )
{
	GetMessageDescription( dbei, str, cbStr );
}

bool EventList::GetEventIcon(bool isMe, int eventType, int &id)
{
	switch(eventType) {
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

void EventList::ImportMessages(const std::vector<IImport::ExternalMessage>& messages)
{
	DWORD lastTime = 0;
	importedMessages.clear();
	for (int i = 0; i < (int)messages.size(); ++i) {
		if (messages[i].timestamp >= lastTime) {
			importedMessages.push_back(messages[i]);
			lastTime = messages[i].timestamp;
		}
		else
		{
			assert(FALSE);
		}
	}
}

void EventList::MargeMessages(const std::vector<IImport::ExternalMessage>& messages)
{
	ImportMessages(messages);
	std::list<EventTempIndex> tempList;
	GetTempList(tempList, true, false, hContact);

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = GetContactProto(hContact);

	CallService(MS_DB_SETSAFETYMODE, FALSE, 0);
	for (std::list<EventTempIndex>::iterator it = tempList.begin(); it != tempList.end(); ++it) {
		if (it->isExternal) {
			IImport::ExternalMessage& msg = importedMessages[it->exIdx];
			dbei.flags |= DBEF_READ;
			dbei.timestamp = msg.timestamp;
			// For now I do not convert event data from string to blob, and event type must be message to handle it properly
			dbei.eventType = EVENTTYPE_MESSAGE;
			UINT cp = dbei.flags & DBEF_UTF ? CP_UTF8 : CP_ACP;
			dbei.cbBlob = WideCharToMultiByte(cp, 0, msg.message.c_str(), (int)msg.message.length() + 1, NULL, 0, NULL, NULL);
			char* buf = new char[dbei.cbBlob];
			dbei.cbBlob = WideCharToMultiByte(cp, 0, msg.message.c_str(), (int)msg.message.length() + 1, buf, dbei.cbBlob, NULL, NULL);
			dbei.pBlob = (PBYTE)buf;
			db_event_add(hContact, &dbei);
			delete [] buf;
		}
	}

	CallService(MS_DB_SETSAFETYMODE, TRUE, 0);
	std::vector<IImport::ExternalMessage> emessages;
	ImportMessages(emessages);
}

bool EventList::GetEventData(const EventIndex& ev, EventData& data)
{
	if (!ev.isExternal) {
		DWORD newBlobSize = db_event_getBlobSize(ev.hEvent);
		if (newBlobSize > goldBlobSize) {
			gdbei.pBlob = (PBYTE)mir_realloc(gdbei.pBlob,newBlobSize);
			goldBlobSize = newBlobSize;
		}

		gdbei.cbBlob = goldBlobSize;
		if (db_event_get(ev.hEvent, &gdbei) == 0) {
			data.isMe = (gdbei.flags & DBEF_SENT) != 0;
			data.eventType = gdbei.eventType;
			data.timestamp = gdbei.timestamp;
			return true;
		}
	}
	else if (ev.exIdx >= 0 && ev.exIdx < (int)importedMessages.size()) {
		IImport::ExternalMessage& em = importedMessages[ev.exIdx];
		data.isMe = (em.flags & DBEF_SENT) != 0;
		data.eventType = em.eventType;
		data.timestamp = em.timestamp;
		return true;
	}

	return false;
}

void EventList::GetExtEventDBei(const EventIndex& ev)
{
	IImport::ExternalMessage& em = importedMessages[ev.exIdx];
	gdbei.flags = em.flags | 0x800;
	gdbei.eventType = em.eventType;
	gdbei.timestamp = em.timestamp;
}

HICON EventList::GetEventCoreIcon(const EventIndex& ev)
{
	if (ev.isExternal)
		return NULL;
	
	HICON ico = (HICON)CallService(MS_DB_EVENT_GETICON, LR_SHARED, (LPARAM)&gdbei);
	HICON icoMsg = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	if (icoMsg == ico)
		return NULL;

	return ico;
}

void EventList::RebuildGroup(int selected)
{
	std::deque<EventIndex> newGroup;
	for (size_t i = 0; i < eventList[selected].size(); ++i) {
		EventIndex& ev = eventList[selected][i];
		if (!ev.isExternal) {
			// If event exist, we add it to new group
			if (db_event_getBlobSize(ev.hEvent) >= 0)
				newGroup.push_back(eventList[selected][i]);
		}
		else newGroup.push_back(eventList[selected][i]);
	}
	eventList[selected].clear();
	eventList[selected].insert(eventList[selected].begin(), newGroup.begin(), newGroup.end());
}

std::map<MCONTACT, EventList::ImportDiscData> EventList::contactFileMap;
std::wstring EventList::contactFileDir;

void EventList::AddImporter(MCONTACT hContact, IImport::ImportType type, const std::wstring& file)
{
	mir_cslock lck(csEventList);

	TCHAR buf[32];
	mir_sntprintf(buf, SIZEOF(buf), _T("%016llx"), (unsigned long long int)hContact);
	ImportDiscData data;
	data.file = contactFileDir + buf;
	data.type = type;
	CopyFile(file.c_str(), data.file.c_str(), FALSE);
	contactFileMap[hContact] = data;
}

void EventList::Init()
{
	TCHAR temp[MAX_PATH];
	temp[0] = 0;
	GetTempPath(MAX_PATH, temp);
	contactFileDir = temp;
	contactFileDir += L"BasicHistoryImportDir\\";
	DeleteDirectory(contactFileDir.c_str());
	CreateDirectory(contactFileDir.c_str(), NULL);
}

int EventList::GetContactMessageNumber(MCONTACT hContact)
{
	int count = db_event_count(hContact);

	mir_cslock lck(csEventList);
	std::map<MCONTACT, EventList::ImportDiscData>::iterator it = contactFileMap.find(hContact);
	if (it != contactFileMap.end())
		++count;
	return count;
}

bool EventList::IsImportedHistory(MCONTACT hContact)
{
	bool count = false;

	mir_cslock lck(csEventList);
	std::map<MCONTACT, EventList::ImportDiscData>::iterator it = contactFileMap.find(hContact);
	if (it != contactFileMap.end())
		count = true;
	return count;
}

void EventList::DeleteImporter(MCONTACT hContact)
{
	mir_cslock lck(csEventList);

	std::map<MCONTACT, EventList::ImportDiscData>::iterator it = contactFileMap.find(hContact);
	if (it != contactFileMap.end()) {
		DeleteFile(it->second.file.c_str());
		contactFileMap.erase(it);
	}
}
