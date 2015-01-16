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

#include "ieview_common.h"

int HTMLBuilder::mimFlags = 0;

HTMLBuilder::HTMLBuilder()
{
	lastIEViewEvent.cbSize = sizeof(IEVIEWEVENT);
	lastIEViewEvent.iType = IEE_LOG_MEM_EVENTS;
	lastIEViewEvent.codepage = CP_ACP;
	lastIEViewEvent.pszProto = NULL;
	lastIEViewEvent.count = 0;
	lastIEViewEvent.dwFlags = 0;
	lastIEViewEvent.hContact = NULL;
	lastIEViewEvent.hwnd = NULL;
	lastIEViewEvent.eventData = NULL;
}

HTMLBuilder::~HTMLBuilder()
{
	if (lastIEViewEvent.pszProto != NULL)
		mir_free((void*)lastIEViewEvent.pszProto);
}

bool HTMLBuilder::encode(MCONTACT hContact, const char *proto, const wchar_t *text, wchar_t **output, int *outputSize, int level, int flags, bool isSent)
{
	TextToken *token = NULL, *token2;
	switch (level) {
	case 0:
		if (flags & ENF_CHAT_FORMATTING) {
			token = TextToken::tokenizeChatFormatting(text);
			break;
		}
		level++;
	case 1:
		if ((Options::getGeneralFlags()&Options::GENERAL_ENABLE_BBCODES) && (flags & ENF_BBCODES)) {
			token = TextToken::tokenizeBBCodes(text);
			break;
		}
		level++;
	case 2:
		token = TextToken::tokenizeLinks(text);
		break;
	case 3:
		if ((flags & ENF_SMILEYS) || ((Options::getGeneralFlags() & Options::GENERAL_SMILEYINNAMES) && (flags & ENF_NAMESMILEYS)))
			token = TextToken::tokenizeSmileys(hContact, proto, text, isSent);
		break;
	}
	if (token != NULL) {
		for (token2 = token; token != NULL; token = token2) {
			bool skip = false;
			token2 = token->getNext();
			if (token->getType() == TextToken::TEXT)
				skip = encode(hContact, proto, token->getTextW(), output, outputSize, level + 1, flags, isSent);
			if (!skip)
				token->toString(output, outputSize);
			delete token;
		}
		return true;
	}
	return false;
}

wchar_t* HTMLBuilder::encode(MCONTACT hContact, const char *proto, const wchar_t *text, int flags, bool isSent)
{
	int outputSize;
	wchar_t *output = NULL;
	if (text != NULL)
		encode(hContact, proto, text, &output, &outputSize, 0, flags, isSent);
	return output;
}

char* HTMLBuilder::encodeUTF8(MCONTACT hContact, const char *proto, const wchar_t *wtext, int flags, bool isSent)
{
	char *outputStr = NULL;
	if (wtext != NULL) {
		wchar_t *output = encode(hContact, proto, wtext, flags, isSent);
		outputStr = mir_utf8encodeT(output);
		if (output != NULL)
			free(output);
	}
	return outputStr;
}

char* HTMLBuilder::encodeUTF8(MCONTACT hContact, const char *proto, const char *text, int flags, bool isSent)
{
	char *outputStr = NULL;
	if (text != NULL) {
		wchar_t *wtext = mir_a2t(text);
		outputStr = encodeUTF8(hContact, proto, wtext, flags, isSent);
		mir_free(wtext);
	}
	return outputStr;
}

char* HTMLBuilder::encodeUTF8(MCONTACT hContact, const char *proto, const char *text, int cp, int flags, bool isSent)
{
	char *outputStr = NULL;
	if (text != NULL) {
		wchar_t *wtext = mir_a2t_cp(text, cp);
		outputStr = encodeUTF8(hContact, proto, wtext, flags, isSent);
		mir_free(wtext);
	}
	return outputStr;
}

char* HTMLBuilder::getProto(MCONTACT hContact)
{
	return mir_strdup(GetContactProto(hContact));
}

char* HTMLBuilder::getProto(const char *proto, MCONTACT hContact)
{
	if (proto != NULL)
		return mir_strdup(proto);

	return mir_strdup(GetContactProto(hContact));
}

char* HTMLBuilder::getRealProto(MCONTACT hContact)
{
	if (hContact == NULL)
		return NULL;

	char *szProto = mir_strdup(GetContactProto(hContact));
	if (szProto != NULL && !strcmp(szProto, META_PROTO)) {
		hContact = db_mc_getMostOnline(hContact);
		if (hContact != NULL) {
			mir_free(szProto);
			szProto = mir_strdup(GetContactProto(hContact));
		}
	}
	return szProto;
}

char *HTMLBuilder::getRealProto(MCONTACT hContact, const char *szProto)
{
	if (szProto != NULL && !strcmp(szProto, META_PROTO)) {
		hContact = db_mc_getMostOnline(hContact);
		if (hContact != NULL)
			return mir_strdup(GetContactProto(hContact));
	}
	return mir_strdup(szProto);
}

MCONTACT HTMLBuilder::getRealContact(MCONTACT hContact)
{
	char *szProto = GetContactProto(hContact);
	if (szProto != NULL && !strcmp(szProto, META_PROTO))
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

DWORD HTMLBuilder::getLastEventTime()
{
	return lastEventTime;
}

void HTMLBuilder::setLastEventTime(DWORD t)
{
	lastEventTime = t;
}

bool HTMLBuilder::isSameDate(time_t time1, time_t time2)
{
	struct tm tm_t1, tm_t2;
	tm_t1 = *localtime((time_t *)(&time1));
	tm_t2 = *localtime((time_t *)(&time2));
	if (tm_t1.tm_year == tm_t2.tm_year && tm_t1.tm_mon == tm_t2.tm_mon
		&& tm_t1.tm_mday == tm_t2.tm_mday) {
		return true;
	}
	return false;
}

void HTMLBuilder::getUINs(MCONTACT hContact, char *&uinIn, char *&uinOut)
{
	CONTACTINFO ci;
	char buf[128];
	char *szProto;
	hContact = getRealContact(hContact);
	szProto = getProto(hContact);
	memset(&ci, 0, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = hContact;
	ci.szProto = szProto;
	ci.dwFlag = CNF_UNIQUEID;
	buf[0] = 0;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)& ci)) {
		switch (ci.type) {
		case CNFT_ASCIIZ:
			strncpy_s(buf, (char*)ci.pszVal, _TRUNCATE);
			mir_free(ci.pszVal);
			break;
		case CNFT_DWORD:
			mir_snprintf(buf, SIZEOF(buf), "%u", ci.dVal);
			break;
		}
	}
	uinIn = mir_utf8encode(buf);
	ci.hContact = NULL;
	buf[0] = 0;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)& ci)) {
		switch (ci.type) {
		case CNFT_ASCIIZ:
			strncpy_s(buf, (char*)ci.pszVal, _TRUNCATE);
			mir_free(ci.pszVal);
			break;
		case CNFT_DWORD:
			mir_snprintf(buf, SIZEOF(buf), "%u", ci.dVal);
			break;
		}
	}
	uinOut = mir_utf8encode(buf);
	mir_free(szProto);
}

wchar_t *HTMLBuilder::getContactName(MCONTACT hContact, const char *szProto)
{
	CONTACTINFO ci = { 0 };
	wchar_t *szName = NULL;

	ci.cbSize = sizeof(ci);
	ci.hContact = hContact;
	ci.szProto = (char *)szProto;
	ci.dwFlag = CNF_DISPLAY | CNF_UNICODE;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
		if (ci.type == CNFT_ASCIIZ) {
			if (ci.pszVal) {
				szName = mir_tstrdup(ci.pszVal);
				mir_free(ci.pszVal);
			}
		}
	}
	if (szName != NULL) return szName;

	ci.dwFlag = CNF_UNIQUEID;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
		if (ci.type == CNFT_ASCIIZ) {
			if (ci.pszVal) {
				szName = mir_tstrdup(ci.pszVal);
				mir_free(ci.pszVal);
			}
		}
	}
	if (szName != NULL) return szName;

	TCHAR *szNameStr = pcli->pfnGetContactDisplayName(hContact, 0);
	if (szNameStr != NULL)
		return mir_tstrdup(szNameStr);

	return mir_tstrdup(TranslateT("(Unknown Contact)"));
}

char *HTMLBuilder::getEncodedContactName(MCONTACT hContact, const char* szProto, const char* szSmileyProto)
{
	char *szName = NULL;
	wchar_t *name = getContactName(hContact, szProto);
	if (name != NULL) {
		szName = encodeUTF8(hContact, szSmileyProto, name, ENF_NAMESMILEYS, true);
		mir_free(name);
		return szName;
	}
	return encodeUTF8(hContact, szSmileyProto, TranslateT("(Unknown Contact)"), ENF_NAMESMILEYS, true);
}

void HTMLBuilder::appendEventNew(IEView *view, IEVIEWEVENT *event)
{
	setLastIEViewEvent(event);
	appendEvent(view, event);
}

void HTMLBuilder::appendEventOld(IEView *view, IEVIEWEVENT *event)
{
	IEVIEWEVENTDATA* eventData;
	IEVIEWEVENTDATA* prevEventData = NULL;
	MEVENT hDbEvent = event->hDbEventFirst;
	event->hDbEventFirst = NULL;

	ptrA szProto;
	if (event->cbSize >= IEVIEWEVENT_SIZE_V3 && event->pszProto != NULL)
		szProto = mir_strdup(event->pszProto);
	else
		szProto = getProto(event->hContact);

	IEVIEWEVENT newEvent = { sizeof(IEVIEWEVENT) };
	newEvent.iType = IEE_LOG_MEM_EVENTS;
	newEvent.codepage = CP_ACP;
	if (event->cbSize >= IEVIEWEVENT_SIZE_V2)
		newEvent.codepage = event->codepage;
	newEvent.pszProto = szProto;
	newEvent.count = 0;
	newEvent.dwFlags = event->dwFlags;
	newEvent.hContact = event->hContact;
	newEvent.hwnd = event->hwnd;
	newEvent.eventData = NULL;
	for (int eventIdx = 0; hDbEvent != NULL && (eventIdx < event->count || event->count == -1); eventIdx++) {
		DBEVENTINFO dbei = { sizeof(dbei) };
		dbei.cbBlob = db_event_getBlobSize(hDbEvent);
		if (dbei.cbBlob == 0xFFFFFFFF) {
			hDbEvent = db_event_next(event->hContact, hDbEvent);
			continue;
		}
		dbei.pBlob = (PBYTE)malloc(dbei.cbBlob);
		db_event_get(hDbEvent, &dbei);
		if (!(dbei.flags & DBEF_SENT) && (dbei.eventType == EVENTTYPE_MESSAGE || dbei.eventType == EVENTTYPE_URL)) {
			db_event_markRead(event->hContact, hDbEvent);
			CallService(MS_CLIST_REMOVEEVENT, event->hContact, (LPARAM)hDbEvent);
		}

		if (!isDbEventShown(&dbei)) {
			free(dbei.pBlob);
			hDbEvent = db_event_next(event->hContact, hDbEvent);
			continue;
		}
		eventData = new IEVIEWEVENTDATA;
		eventData->cbSize = sizeof(IEVIEWEVENTDATA);
		eventData->dwFlags = IEEDF_UNICODE_TEXT | IEEDF_UNICODE_NICK | IEEDF_UNICODE_TEXT2 |
			(dbei.flags & DBEF_READ ? IEEDF_READ : 0) | (dbei.flags & DBEF_SENT ? IEEDF_SENT : 0) | (dbei.flags & DBEF_RTL ? IEEDF_RTL : 0);
		if (event->dwFlags & IEEF_RTL)
			eventData->dwFlags |= IEEDF_RTL;

		eventData->time = dbei.timestamp;
		eventData->pszNickW = NULL;
		eventData->pszTextW = NULL;
		eventData->pszText2W = NULL;
		if (dbei.flags & DBEF_SENT) {
			eventData->pszNickW = getContactName(NULL, szProto);
			eventData->bIsMe = TRUE;
		}
		else {
			eventData->pszNickW = getContactName(event->hContact, szProto);
			eventData->bIsMe = FALSE;
		}
		if (dbei.eventType == EVENTTYPE_MESSAGE || dbei.eventType == EVENTTYPE_URL || Utils::DbEventIsForMsgWindow(&dbei)) {
			eventData->pszTextW = DbGetEventTextW(&dbei, newEvent.codepage);
			if (dbei.eventType == EVENTTYPE_MESSAGE)
				eventData->iType = IEED_EVENT_MESSAGE;
			else if (dbei.eventType == EVENTTYPE_URL)
				eventData->iType = IEED_EVENT_URL;
			else
				eventData->iType = IEED_EVENT_STATUSCHANGE;
		}
		else if (dbei.eventType == EVENTTYPE_FILE) {
			//blob is: sequenceid(DWORD),filename(ASCIIZ),description(ASCIIZ)
			char* filename = ((char *)dbei.pBlob) + sizeof(DWORD);
			char* descr = filename + mir_strlen(filename) + 1;
			eventData->ptszText = DbGetEventStringT(&dbei, filename);
			if (*descr != '\0')
				eventData->ptszText2 = DbGetEventStringT(&dbei, descr);
			eventData->iType = IEED_EVENT_FILE;
		}
		else if (dbei.eventType == EVENTTYPE_AUTHREQUEST) {
			//blob is: uin(DWORD), hContact(DWORD), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ)
			eventData->ptszText = mir_tstrdup(TranslateT(" requested authorization"));
			eventData->ptszNick = DbGetEventStringT(&dbei, (char *)dbei.pBlob + 8);
			eventData->iType = IEED_EVENT_SYSTEM;
		}
		else if (dbei.eventType == EVENTTYPE_ADDED) {
			//blob is: uin(DWORD), hContact(DWORD), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ)
			eventData->ptszText = mir_tstrdup(TranslateT(" was added."));
			eventData->ptszNick = DbGetEventStringT(&dbei, (char *)dbei.pBlob + 8);
			eventData->iType = IEED_EVENT_SYSTEM;
		}
		else { // custom event
			eventData->pszTextW = DbGetEventTextW(&dbei, newEvent.codepage);
			eventData->iType = IEED_EVENT_MESSAGE;
		}
		free(dbei.pBlob);
		eventData->next = NULL;
		if (prevEventData != NULL)
			prevEventData->next = eventData;
		else
			newEvent.eventData = eventData;

		prevEventData = eventData;
		newEvent.count++;
		event->hDbEventFirst = hDbEvent;
		hDbEvent = db_event_next(event->hContact, hDbEvent);
	}
	appendEventNew(view, &newEvent);
	for (IEVIEWEVENTDATA* eventData2 = newEvent.eventData; eventData2 != NULL; eventData2 = eventData) {
		eventData = eventData2->next;
		mir_free((void*)eventData2->pszTextW);
		mir_free((void*)eventData2->pszText2W);
		mir_free((void*)eventData2->pszNickW);
		delete eventData2;
	}
}

ProtocolSettings* HTMLBuilder::getSRMMProtocolSettings(const char *protocolName)
{
	ProtocolSettings *protoSettings = Options::getProtocolSettings(protocolName);
	if (protoSettings == NULL || !protoSettings->isSRMMEnable())
		protoSettings = Options::getProtocolSettings();

	return protoSettings;
}

ProtocolSettings* HTMLBuilder::getSRMMProtocolSettings(MCONTACT hContact)
{
	return getSRMMProtocolSettings(ptrA(getRealProto(hContact)));
}

ProtocolSettings* HTMLBuilder::getHistoryProtocolSettings(const char *protocolName)
{
	ProtocolSettings *protoSettings = Options::getProtocolSettings(protocolName);
	if (protoSettings == NULL || !protoSettings->isHistoryEnable())
		protoSettings = Options::getProtocolSettings();

	return protoSettings;
}

ProtocolSettings* HTMLBuilder::getHistoryProtocolSettings(MCONTACT hContact)
{
	if (hContact != NULL)
		return getHistoryProtocolSettings(ptrA(getRealProto(hContact)));

	return Options::getProtocolSettings();
}

ProtocolSettings* HTMLBuilder::getChatProtocolSettings(const char *protocolName)
{
	ProtocolSettings *protoSettings = Options::getProtocolSettings(protocolName);
	if (protoSettings == NULL || !protoSettings->isChatEnable())
		protoSettings = Options::getProtocolSettings();

	return protoSettings;
}

ProtocolSettings* HTMLBuilder::getChatProtocolSettings(MCONTACT hContact)
{
	return getChatProtocolSettings(ptrA(getRealProto(hContact)));
}

void HTMLBuilder::setLastIEViewEvent(IEVIEWEVENT *event)
{
	lastIEViewEvent.cbSize = sizeof(IEVIEWEVENT);
	lastIEViewEvent.iType = event->iType;
	lastIEViewEvent.codepage = CP_ACP;
	if (event->cbSize >= IEVIEWEVENT_SIZE_V2)
		lastIEViewEvent.codepage = event->codepage;

	lastIEViewEvent.count = 0;
	lastIEViewEvent.dwFlags = event->dwFlags;
	lastIEViewEvent.hContact = event->hContact;
	lastIEViewEvent.hwnd = event->hwnd;
	lastIEViewEvent.eventData = NULL;
	if (lastIEViewEvent.pszProto != NULL)
		mir_free((void*)lastIEViewEvent.pszProto);

	if (event->cbSize >= IEVIEWEVENT_SIZE_V3 && event->pszProto != NULL)
		lastIEViewEvent.pszProto = mir_strdup(event->pszProto);
	else
		lastIEViewEvent.pszProto = getProto(event->hContact);
}

void HTMLBuilder::clear(IEView *view, IEVIEWEVENT *event)
{
	if (event != NULL)
		setLastIEViewEvent(event);

	if (lastIEViewEvent.pszProto != NULL || event->hContact == NULL)
		buildHead(view, &lastIEViewEvent);
}
