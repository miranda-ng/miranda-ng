/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2010  Piotr Piastucki

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

int HTMLBuilder::mimFlags = 0;

HTMLBuilder::HTMLBuilder()
{
	lastIEViewEvent.iType = IEE_LOG_MEM_EVENTS;
	lastIEViewEvent.codepage = CP_ACP;
	lastIEViewEvent.count = 0;
	lastIEViewEvent.dwFlags = 0;
	lastIEViewEvent.hContact = NULL;
	lastIEViewEvent.hwnd = nullptr;
	lastIEViewEvent.eventData = nullptr;
}

HTMLBuilder::~HTMLBuilder()
{
}

bool HTMLBuilder::encode(MCONTACT hContact, const wchar_t *text, CMStringW &str, int level, int flags, bool isSent)
{
	TextToken *token = nullptr, *token2;
	switch (level) {
	case 0:
		if (flags & ENF_CHAT_FORMATTING) {
			token = TextToken::tokenizeChatFormatting(text);
			break;
		}
		level++;
	case 1:
		if ((Options::generalFlags & Options::GENERAL_ENABLE_BBCODES) && (flags & ENF_BBCODES)) {
			token = TextToken::tokenizeBBCodes(text);
			break;
		}
		level++;
	case 2:
		token = TextToken::tokenizeLinks(text);
		break;
	case 3:
		if ((flags & ENF_SMILEYS) || ((Options::generalFlags & Options::GENERAL_SMILEYINNAMES) && (flags & ENF_NAMESMILEYS)))
			token = TextToken::tokenizeSmileys(hContact, text, isSent);
		break;
	}
	if (token != nullptr) {
		for (token2 = token; token != nullptr; token = token2) {
			bool skip = false;
			token2 = token->getNext();
			if (token->getType() == TextToken::TEXT)
				skip = encode(hContact, token->getTextW(), str, level + 1, flags, isSent);
			if (!skip)
				token->toString(str);
			delete token;
		}
		return true;
	}
	return false;
}

char* HTMLBuilder::encodeUTF8(MCONTACT hContact, const wchar_t *wtext, int flags, bool isSent)
{
	if (wtext == nullptr)
		return nullptr;

	CMStringW str;
	encode(hContact, wtext, str, 0, flags, isSent);
	return mir_utf8encodeW(str);
}

char* HTMLBuilder::encodeUTF8(MCONTACT hContact, const char *text, int flags, bool isSent)
{
	if (text == nullptr)
		return nullptr;

	return encodeUTF8(hContact, _A2T(text), flags, isSent);
}

char* HTMLBuilder::encodeUTF8(MCONTACT hContact, const char *text, int cp, int flags, bool isSent)
{
	if (text == nullptr)
		return nullptr;

	ptrW wtext(mir_a2u_cp(text, cp));
	return encodeUTF8(hContact, wtext, flags, isSent);
}

const char* HTMLBuilder::getRealProto(MCONTACT hContact)
{
	if (hContact == NULL)
		return nullptr;

	return Proto_GetBaseAccountName(getRealContact(hContact));
}

MCONTACT HTMLBuilder::getRealContact(MCONTACT hContact)
{
	if (db_mc_isMeta(hContact))
		hContact = db_mc_getMostOnline(hContact);
	return hContact;
}

int HTMLBuilder::getLastEventType()
{
	return iLastEventType;
}

void HTMLBuilder::setLastEventType(int t)
{
	iLastEventType = t;
}

uint32_t HTMLBuilder::getLastEventTime()
{
	return lastEventTime;
}

void HTMLBuilder::setLastEventTime(uint32_t t)
{
	lastEventTime = t;
}

bool HTMLBuilder::isSameDate(time_t time1, time_t time2)
{
	tm tm_t1 = *localtime(&time1), tm_t2 = *localtime(&time2);
	return tm_t1.tm_year == tm_t2.tm_year && tm_t1.tm_mon == tm_t2.tm_mon && tm_t1.tm_mday == tm_t2.tm_mday;

}

void HTMLBuilder::getUINs(MCONTACT hContact, char *&uinIn, char *&uinOut)
{
	hContact = getRealContact(hContact);

	ptrW id(Contact::GetInfo(CNF_UNIQUEID, hContact));
	uinIn = mir_utf8encodeW(id ? id.get() : L"");

	id = Contact::GetInfo(CNF_UNIQUEID, NULL);
	uinOut = mir_utf8encodeW(id ? id.get() : L"");
}

wchar_t* HTMLBuilder::getContactName(MCONTACT hContact, const char *szProto)
{
	wchar_t *str = Contact::GetInfo(CNF_DISPLAY, hContact, szProto);
	if (str != nullptr)
		return str;

	str = Contact::GetInfo(CNF_UNIQUEID, hContact, szProto);
	if (str != nullptr)
		return str;

	return mir_wstrdup(Clist_GetContactDisplayName(hContact));
}

char* HTMLBuilder::getEncodedContactName(MCONTACT hContact, const char *szProto)
{
	return encodeUTF8(hContact, ptrW(getContactName(hContact, szProto)), ENF_NAMESMILEYS, true);
}

void HTMLBuilder::appendEventNew(IEView *view, IEVIEWEVENT *event)
{
	setLastIEViewEvent(event);
	appendEvent(view, event);
}

void HTMLBuilder::appendEventOld(IEView *view, IEVIEWEVENT *event)
{
	IEVIEWEVENTDATA *prevEventData = nullptr;
	MEVENT hDbEvent = event->hDbEventFirst;
	event->hDbEventFirst = NULL;

	const char *szProto = Proto_GetBaseAccountName(event->hContact);

	IEVIEWEVENT newEvent = {};
	newEvent.iType = IEE_LOG_MEM_EVENTS;
	newEvent.codepage = CP_ACP;
	newEvent.codepage = event->codepage;
	newEvent.dwFlags = event->dwFlags;
	newEvent.hContact = event->hContact;
	newEvent.hwnd = event->hwnd;

	for (int eventIdx = 0; hDbEvent != NULL && (eventIdx < event->count || event->count == -1); eventIdx++) {
		DB::EventInfo dbei;
		dbei.cbBlob = -1;
		if (db_event_get(hDbEvent, &dbei)) {
			hDbEvent = db_event_next(event->hContact, hDbEvent);
			continue;
		}

		if (!(dbei.flags & DBEF_SENT) && dbei.eventType == EVENTTYPE_MESSAGE) {
			db_event_markRead(event->hContact, hDbEvent);
			g_clistApi.pfnRemoveEvent(event->hContact, hDbEvent);
		}

		if (!isDbEventShown(&dbei)) {
			hDbEvent = db_event_next(event->hContact, hDbEvent);
			continue;
		}
	
		IEVIEWEVENTDATA *eventData = new IEVIEWEVENTDATA;
		eventData->dwFlags = IEEDF_UNICODE_TEXT | IEEDF_UNICODE_NICK |
			(dbei.flags & DBEF_READ ? IEEDF_READ : 0) | (dbei.flags & DBEF_SENT ? IEEDF_SENT : 0) | (dbei.flags & DBEF_RTL ? IEEDF_RTL : 0);
		if (event->dwFlags & IEEF_RTL)
			eventData->dwFlags |= IEEDF_RTL;

		eventData->time = dbei.timestamp;
		eventData->szNick.a = eventData->szText.a = nullptr;
		if (dbei.flags & DBEF_SENT) {
			eventData->szNick.w = getContactName(NULL, szProto);
			eventData->bIsMe = TRUE;
		}
		else {
			eventData->szNick.w = getContactName(event->hContact, szProto);
			eventData->bIsMe = FALSE;
		}
		if (dbei.eventType == EVENTTYPE_MESSAGE || Utils::DbEventIsForMsgWindow(&dbei)) {
			eventData->szText.w = DbEvent_GetTextW(&dbei, newEvent.codepage);
			if (dbei.eventType == EVENTTYPE_MESSAGE)
				eventData->iType = IEED_EVENT_MESSAGE;
			else
				eventData->iType = IEED_EVENT_STATUSCHANGE;
		}
		else if (dbei.eventType == EVENTTYPE_FILE) {
			// blob is: sequenceid(uint32_t),filename(ASCIIZ),description(ASCIIZ)
			char* filename = ((char *)dbei.pBlob) + sizeof(uint32_t);
			char* descr = filename + mir_strlen(filename) + 1;
			eventData->szText.w = DbEvent_GetString(&dbei, filename);
			if (*descr != '\0') {
				CMStringW tmp(FORMAT, L"%s (%s)", eventData->szText.w, ptrW(DbEvent_GetString(&dbei, descr)).get());
				mir_free((void*)eventData->szText.w);
				eventData->szText.w = tmp.Detach();
			}
			eventData->iType = IEED_EVENT_FILE;
		}
		else if (dbei.eventType == EVENTTYPE_AUTHREQUEST) {
			// blob is: uin(uint32_t), hContact(uint32_t), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ)
			eventData->szText.w = mir_wstrdup(TranslateT(" requested authorization"));
			eventData->szNick.w = DbEvent_GetString(&dbei, (char *)dbei.pBlob + 8);
			eventData->iType = IEED_EVENT_SYSTEM;
		}
		else if (dbei.eventType == EVENTTYPE_ADDED) {
			//blob is: uin(uint32_t), hContact(uint32_t), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ)
			eventData->szText.w = mir_wstrdup(TranslateT(" was added."));
			eventData->szNick.w = DbEvent_GetString(&dbei, (char *)dbei.pBlob + 8);
			eventData->iType = IEED_EVENT_SYSTEM;
		}
		else { // custom event
			eventData->szText.w = DbEvent_GetTextW(&dbei, newEvent.codepage);
			eventData->iType = IEED_EVENT_MESSAGE;
		}

		eventData->next = nullptr;
		if (prevEventData != nullptr)
			prevEventData->next = eventData;
		else
			newEvent.eventData = eventData;

		prevEventData = eventData;
		newEvent.count++;
		event->hDbEventFirst = hDbEvent;
		hDbEvent = db_event_next(event->hContact, hDbEvent);
	}
	appendEventNew(view, &newEvent);
	for (IEVIEWEVENTDATA* eventData2 = newEvent.eventData; eventData2 != nullptr;) {
		IEVIEWEVENTDATA *eventData = eventData2->next;
		mir_free((void*)eventData2->szText.w);
		mir_free((void*)eventData2->szNick.w);
		delete eventData2;
		eventData2 = eventData;
	}
}

ProtocolSettings* HTMLBuilder::getSRMMProtocolSettings(MCONTACT hContact)
{
	auto *protoSettings = Options::getProtocolSettings(getRealProto(hContact));
	if (protoSettings != nullptr && protoSettings->isSRMMEnable())
		return protoSettings;

	return Options::getDefaultSettings();
}

ProtocolSettings* HTMLBuilder::getHistoryProtocolSettings(MCONTACT hContact)
{
	if (hContact != NULL) {
		auto *protoSettings = Options::getProtocolSettings(getRealProto(hContact));
		if (protoSettings != nullptr && protoSettings->isHistoryEnable())
			return protoSettings;
	}

	return Options::getDefaultSettings();
}

ProtocolSettings* HTMLBuilder::getChatProtocolSettings(MCONTACT hContact)
{
	auto *protoSettings = Options::getProtocolSettings(getRealProto(hContact));
	if (protoSettings != nullptr && protoSettings->isChatEnable())
		return protoSettings;

	return Options::getDefaultSettings();
}

void HTMLBuilder::setLastIEViewEvent(IEVIEWEVENT *event)
{
	lastIEViewEvent.iType = event->iType;
	lastIEViewEvent.codepage = CP_ACP;
	lastIEViewEvent.codepage = event->codepage;
	lastIEViewEvent.count = 0;
	lastIEViewEvent.dwFlags = event->dwFlags;
	lastIEViewEvent.hContact = event->hContact;
	lastIEViewEvent.hwnd = event->hwnd;
	lastIEViewEvent.eventData = nullptr;
}

void HTMLBuilder::clear(IEView *view, IEVIEWEVENT *event)
{
	if (event != nullptr) {
		setLastIEViewEvent(event);

		if (event->hContact == NULL || Proto_GetBaseAccountName(lastIEViewEvent.hContact))
			buildHead(view, &lastIEViewEvent);
	}
}
