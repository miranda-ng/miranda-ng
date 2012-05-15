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
#include "HTMLBuilder.h"
#include "m_MathModule.h"
#include "m_metacontacts.h"
#include "Utils.h"
//#include "Smiley.h"
#include "Options.h"


int HTMLBuilder::mimFlags = 0;

HTMLBuilder::HTMLBuilder() {
	lastIEViewEvent.cbSize = sizeof (IEVIEWEVENT);
	lastIEViewEvent.iType = IEE_LOG_MEM_EVENTS;
	lastIEViewEvent.codepage = CP_ACP;
	lastIEViewEvent.pszProto = NULL;
	lastIEViewEvent.count = 0;
	lastIEViewEvent.dwFlags = 0;
	lastIEViewEvent.hContact = NULL;
	lastIEViewEvent.hwnd = NULL;
	lastIEViewEvent.eventData = NULL;
}

HTMLBuilder::~HTMLBuilder() {
	if (lastIEViewEvent.pszProto != NULL) {
		delete (char*)lastIEViewEvent.pszProto;
	}
}

bool HTMLBuilder::encode(HANDLE hContact, const char *proto, const wchar_t *text, wchar_t **output, int *outputSize,  int level, int flags, bool isSent) {
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
		if ((Options::getGeneralFlags()&Options::GENERAL_ENABLE_MATHMODULE) && Options::isMathModule()) {
			token = TextToken::tokenizeMath(text);
			break;
		}
		level++;
	case 3:
		token = TextToken::tokenizeLinks(text);
		break;
	case 4:
		if ((flags & ENF_SMILEYS) ||
				((Options::getGeneralFlags() & Options::GENERAL_SMILEYINNAMES) &&  (flags & ENF_NAMESMILEYS))) {
			token = TextToken::tokenizeSmileys(hContact, proto, text, isSent);
		}
		break;
	}
	if (token!=NULL) {
		for (token2 = token;token!=NULL;token=token2) {
			bool skip = false;
			token2 = token->getNext();
			if (token->getType() == TextToken::TEXT) {
				skip = encode(hContact, proto, token->getTextW(), output, outputSize, level+1, flags, isSent);
			}
			if (!skip) {
				token->toString(output, outputSize);
			}
			delete token;
		}
		return true;
	}
	return false;
}

wchar_t * HTMLBuilder::encode(HANDLE hContact, const char *proto, const wchar_t *text, int flags, bool isSent) {
	int outputSize;
	wchar_t *output = NULL;
	if (text != NULL) {
		encode(hContact, proto, text, &output, &outputSize, 0, flags, isSent);
	}
	return output;
}

char * HTMLBuilder::encodeUTF8(HANDLE hContact, const char *proto, const wchar_t *wtext, int flags, bool isSent) {
	char *outputStr = NULL;
	if (wtext != NULL) {
		wchar_t *output = encode(hContact, proto, wtext, flags, isSent);
		outputStr = Utils::UTF8Encode(output);
		if (output != NULL) {
			free(output);
		}
	}
	return outputStr;
}

char * HTMLBuilder::encodeUTF8(HANDLE hContact, const char *proto, const char *text, int flags, bool isSent) {
	char *outputStr = NULL;
	if (text != NULL) {
		wchar_t *wtext = Utils::convertToWCS(text);
		outputStr = encodeUTF8(hContact, proto, wtext, flags, isSent);
		delete wtext;
	}
	return outputStr;
}

char * HTMLBuilder::encodeUTF8(HANDLE hContact, const char *proto, const char *text, int cp, int flags, bool isSent) {
	char * outputStr = NULL;
	if (text != NULL) {
		wchar_t *wtext = Utils::convertToWCS(text, cp);
		outputStr = encodeUTF8(hContact, proto, wtext, flags, isSent);
		delete wtext;
	}
	return outputStr;
}

char *HTMLBuilder::getProto(HANDLE hContact) {
	return Utils::dupString((char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0));
}

char *HTMLBuilder::getProto(const char *proto, HANDLE hContact) {
	if (proto != NULL) {
		return Utils::dupString(proto);
	}
	return Utils::dupString((char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0));
}

char *HTMLBuilder::getRealProto(HANDLE hContact) {
	if (hContact != NULL) {
		char *szProto = Utils::dupString((char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0));
		if (szProto!=NULL && !strcmp(szProto,"MetaContacts")) {
			hContact = (HANDLE) CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM) hContact, 0);
			if (hContact!=NULL) {
				delete szProto;
				szProto = Utils::dupString((char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0));
			}
		}
		return szProto;
	}
	return NULL;
}

char *HTMLBuilder::getRealProto(HANDLE hContact, const char *szProto) {
	if (szProto!=NULL && !strcmp(szProto,"MetaContacts")) {
		hContact = (HANDLE) CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM) hContact, 0);
		if (hContact!=NULL) {
			return Utils::dupString((char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0));
		}
	}
	return Utils::dupString(szProto);
}

HANDLE HTMLBuilder::getRealContact(HANDLE hContact) {
	char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (szProto != NULL && !strcmp(szProto,"MetaContacts")) {
		hContact = (HANDLE) CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM) hContact, 0);
	}
	return hContact;
}

int HTMLBuilder::getLastEventType() {
	return iLastEventType;
}

void HTMLBuilder::setLastEventType(int t) {
	iLastEventType = t;
}

DWORD HTMLBuilder::getLastEventTime() {
	return lastEventTime;
}

void HTMLBuilder::setLastEventTime(DWORD t) {
	lastEventTime = t;
}

bool HTMLBuilder::isSameDate(time_t time1, time_t time2) {
	struct tm tm_t1, tm_t2;
	tm_t1 = *localtime((time_t *)(&time1));
	tm_t2 = *localtime((time_t *)(&time2));
	if (tm_t1.tm_year == tm_t2.tm_year && tm_t1.tm_mon == tm_t2.tm_mon
		&& tm_t1.tm_mday == tm_t2.tm_mday) {
		return true;
	}
	return false;
}

void HTMLBuilder::getUINs(HANDLE hContact, char *&uinIn, char *&uinOut) {
	CONTACTINFO ci;
	char buf[128];
	char *szProto;
	hContact = getRealContact(hContact);
	szProto = getProto(hContact);
	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = hContact;
	ci.szProto = szProto;
	ci.dwFlag = CNF_UNIQUEID;
	buf[0] = 0;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
		switch (ci.type) {
		case CNFT_ASCIIZ:
			mir_snprintf(buf, sizeof(buf), "%s", ci.pszVal);
			miranda_sys_free(ci.pszVal);
			break;
		case CNFT_DWORD:
			mir_snprintf(buf, sizeof(buf), "%u", ci.dVal);
			break;
		}
	}
	uinIn = Utils::UTF8Encode(buf);
	ci.hContact = NULL;
	buf[0] = 0;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
		switch (ci.type) {
		case CNFT_ASCIIZ:
			mir_snprintf(buf, sizeof(buf), "%s", ci.pszVal);
			miranda_sys_free(ci.pszVal);
			break;
		case CNFT_DWORD:
			mir_snprintf(buf, sizeof(buf), "%u", ci.dVal);
			break;
		}
	}
	uinOut = Utils::UTF8Encode(buf);
	delete szProto;
}

wchar_t *HTMLBuilder::getContactName(HANDLE hContact, const char* szProto) {
	CONTACTINFO ci;
	wchar_t *szName = NULL;
	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = hContact;
	ci.szProto = (char *)szProto;
	ci.dwFlag = CNF_DISPLAY | CNF_UNICODE;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
		if (ci.type == CNFT_ASCIIZ) {
			if (ci.pszVal) {
				if(!wcscmp((wchar_t *)ci.pszVal, TranslateW(L"'(Unknown Contact)'"))) {
					ci.dwFlag &= ~CNF_UNICODE;
					if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
						szName = Utils::convertToWCS((char *)ci.pszVal);
					}
				} else {
					szName = Utils::dupString((wchar_t *)ci.pszVal);
				}
				miranda_sys_free(ci.pszVal);
			}
		}
	}
	if (szName != NULL) return szName;
	ci.dwFlag = CNF_UNIQUEID;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
		if (ci.type == CNFT_ASCIIZ) {
			if (ci.pszVal) {
				szName = Utils::convertToWCS((char *)ci.pszVal);
				miranda_sys_free(ci.pszVal);
			}
		}
	}
	if (szName != NULL) return szName;
	char *szNameStr = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
	if (szNameStr != NULL) {
		return Utils::convertToWCS(szNameStr);
	}
	return Utils::dupString(TranslateT("(Unknown Contact)"));
}

char *HTMLBuilder::getEncodedContactName(HANDLE hContact, const char* szProto, const char* szSmileyProto) {
	char *szName = NULL;
	wchar_t *name = getContactName(hContact, szProto);
	if (name != NULL) {
		szName = encodeUTF8(hContact, szSmileyProto, name, ENF_NAMESMILEYS, true);
		delete name;
		return szName;
	}
	return encodeUTF8(hContact, szSmileyProto, TranslateT("(Unknown Contact)"), ENF_NAMESMILEYS, true);
}

void HTMLBuilder::appendEventNew(IEView *view, IEVIEWEVENT *event) {
	setLastIEViewEvent(event);
	appendEvent(view, event);
}

void HTMLBuilder::appendEventOld(IEView *view, IEVIEWEVENT *event) {
	IEVIEWEVENT newEvent;
	IEVIEWEVENTDATA* eventData;
	IEVIEWEVENTDATA* prevEventData = NULL;
	char *szProto = NULL;
	HANDLE hDbEvent = event->hDbEventFirst;
	event->hDbEventFirst = NULL;
	newEvent.cbSize = sizeof (IEVIEWEVENT);
	newEvent.iType = IEE_LOG_MEM_EVENTS;
	newEvent.codepage = CP_ACP;
	if (event->cbSize >= IEVIEWEVENT_SIZE_V2) {
		newEvent.codepage = event->codepage;
	}
	if (event->cbSize >= IEVIEWEVENT_SIZE_V3 && event->pszProto != NULL) {
		szProto = Utils::dupString(event->pszProto);
	} else {
		szProto = getProto(event->hContact);
	}
	newEvent.pszProto = szProto;
	newEvent.count = 0;
	newEvent.dwFlags = event->dwFlags;
	newEvent.hContact = event->hContact;
	newEvent.hwnd = event->hwnd;
	newEvent.eventData = NULL;
	for (int eventIdx = 0; hDbEvent!=NULL && (eventIdx < event->count || event->count==-1); eventIdx++) {
		DBEVENTINFO dbei = { 0 };
		dbei.cbSize = sizeof(dbei);
		dbei.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM) hDbEvent, 0);
		if (dbei.cbBlob == 0xFFFFFFFF) {
			hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDNEXT, (WPARAM) hDbEvent, 0);
			continue;
		}
		dbei.pBlob = (PBYTE) malloc(dbei.cbBlob);
		CallService(MS_DB_EVENT_GET, (WPARAM)  hDbEvent, (LPARAM) & dbei);
		if (!(dbei.flags & DBEF_SENT) && (dbei.eventType == EVENTTYPE_MESSAGE || dbei.eventType == EVENTTYPE_URL)) {
			CallService(MS_DB_EVENT_MARKREAD, (WPARAM) event->hContact, (LPARAM) hDbEvent);
			CallService(MS_CLIST_REMOVEEVENT, (WPARAM) event->hContact, (LPARAM) hDbEvent);
		} else if (dbei.eventType == EVENTTYPE_STATUSCHANGE) {
			CallService(MS_DB_EVENT_MARKREAD, (WPARAM) event->hContact, (LPARAM) hDbEvent);
		}
		if (!isDbEventShown(&dbei)) {
			free(dbei.pBlob);
			hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDNEXT, (WPARAM) hDbEvent, 0);
			continue;
		}
		eventData = new IEVIEWEVENTDATA;
		eventData->cbSize = sizeof(IEVIEWEVENTDATA);
		eventData->dwFlags = IEEDF_UNICODE_TEXT | IEEDF_UNICODE_NICK | IEEDF_UNICODE_TEXT2 |
							(dbei.flags & DBEF_READ ? IEEDF_READ : 0) | (dbei.flags & DBEF_SENT ? IEEDF_SENT : 0) | (dbei.flags & DBEF_RTL ? IEEDF_RTL : 0);
		if (event->dwFlags & IEEF_RTL) {
			eventData->dwFlags  |= IEEDF_RTL;
		}
		eventData->time = dbei.timestamp;
		eventData->pszNickW = NULL;
		eventData->pszTextW = NULL;
		eventData->pszText2W = NULL;
		if (dbei.flags & DBEF_SENT) {
			eventData->pszNickW = getContactName(NULL, szProto);
			eventData->bIsMe = TRUE;
		} else {
			eventData->pszNickW = getContactName(event->hContact, szProto);
			eventData->bIsMe = FALSE;
		}
		if (dbei.eventType == EVENTTYPE_MESSAGE || dbei.eventType == EVENTTYPE_URL || dbei.eventType == EVENTTYPE_STATUSCHANGE || dbei.eventType == EVENTTYPE_JABBER_CHATSTATES) {
			DBEVENTGETTEXT temp = { &dbei, DBVT_WCHAR + ((event->dwFlags & IEEF_NO_UNICODE) ? DBVTF_DENYUNICODE : 0), newEvent.codepage  };
			WCHAR* pwszEventText = (WCHAR*)CallService(MS_DB_EVENT_GETTEXT,0,(LPARAM)&temp);
			eventData->pszTextW = Utils::dupString( pwszEventText );
			mir_free( pwszEventText );
			if (dbei.eventType == EVENTTYPE_MESSAGE) {
				eventData->iType = IEED_EVENT_MESSAGE;
			} else if (dbei.eventType == EVENTTYPE_URL) {
				eventData->iType = IEED_EVENT_URL;
			} else {
				eventData->iType = IEED_EVENT_STATUSCHANGE;
			}
		} else if (dbei.eventType == EVENTTYPE_FILE) {
			//blob is: sequenceid(DWORD),filename(ASCIIZ),description(ASCIIZ)
			char* filename = ((char *)dbei.pBlob) + sizeof(DWORD);
			char* descr = filename + lstrlenA(filename) + 1;
			TCHAR *tStr = DbGetEventStringT(&dbei, filename);
			eventData->ptszText = Utils::dupString(tStr);
			mir_free(tStr);
			if (*descr != '\0') {
				tStr = DbGetEventStringT(&dbei, descr);
				eventData->ptszText2 = Utils::dupString(tStr);
				mir_free(tStr);
			}
			eventData->iType = IEED_EVENT_FILE;
		} else if (dbei.eventType == EVENTTYPE_AUTHREQUEST) {
			//blob is: uin(DWORD), hContact(DWORD), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ)
			eventData->ptszText = Utils::dupString(TranslateT(" requested authorisation"));
			TCHAR *tStr = DbGetEventStringT(&dbei, (char *)dbei.pBlob + 8);
			eventData->ptszNick = Utils::dupString(tStr);
			mir_free(tStr);
			eventData->iType = IEED_EVENT_SYSTEM;
		} else if (dbei.eventType == EVENTTYPE_ADDED) {
			//blob is: uin(DWORD), hContact(DWORD), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ)
			eventData->ptszText = Utils::dupString(TranslateT(" was added."));
			TCHAR *tStr = DbGetEventStringT(&dbei, (char *)dbei.pBlob + 8);
			eventData->ptszNick = Utils::dupString(tStr);
			mir_free(tStr);
			eventData->iType = IEED_EVENT_SYSTEM;
		}
		free(dbei.pBlob);
		eventData->next = NULL;
		if (prevEventData != NULL) {
			prevEventData->next = eventData;
		} else {
			newEvent.eventData = eventData;
		}
		prevEventData = eventData;
		newEvent.count++;
		event->hDbEventFirst = hDbEvent;
		hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDNEXT, (WPARAM) hDbEvent, 0);
	}
	appendEventNew(view, &newEvent);
	for ( IEVIEWEVENTDATA* eventData2 = newEvent.eventData; eventData2 != NULL; eventData2 = eventData) {
		eventData = eventData2->next;
		if (eventData2->pszTextW != NULL) {
			delete (wchar_t*)eventData2->pszTextW;
		}
		if (eventData2->pszText2W != NULL) {
			delete (wchar_t*)eventData2->pszText2W;
		}
		if (eventData2->pszNickW != NULL) {
			delete (wchar_t*)eventData2->pszNickW;
		}
		delete eventData2;
	}
	if (szProto != NULL) {
		delete szProto;
	}
}

ProtocolSettings* HTMLBuilder::getSRMMProtocolSettings(const char *protocolName) {
	ProtocolSettings *protoSettings =  Options::getProtocolSettings(protocolName);
	if (protoSettings == NULL || !protoSettings->isSRMMEnable()) {
		protoSettings =  Options::getProtocolSettings();
	}
	return protoSettings;
}

ProtocolSettings* HTMLBuilder::getSRMMProtocolSettings(HANDLE hContact) {
	char *szRealProto = getRealProto(hContact);
	ProtocolSettings *protoSettings =  getSRMMProtocolSettings(szRealProto);
	delete szRealProto;
	return protoSettings;
}

ProtocolSettings* HTMLBuilder::getHistoryProtocolSettings(const char *protocolName) {
	ProtocolSettings *protoSettings =  Options::getProtocolSettings(protocolName);
	if (protoSettings == NULL || !protoSettings->isHistoryEnable()) {
		protoSettings =  Options::getProtocolSettings();
	}
	return protoSettings;
}

ProtocolSettings* HTMLBuilder::getHistoryProtocolSettings(HANDLE hContact) {
	ProtocolSettings *protoSettings;
	if (hContact != NULL) {
		char *szRealProto = getRealProto(hContact);
		protoSettings =  getHistoryProtocolSettings(szRealProto);
		delete szRealProto;
	} else {
		protoSettings =  Options::getProtocolSettings();
	}
	return protoSettings;
}

ProtocolSettings* HTMLBuilder::getChatProtocolSettings(const char *protocolName) {
	ProtocolSettings *protoSettings =  Options::getProtocolSettings(protocolName);
	if (protoSettings == NULL || !protoSettings->isChatEnable()) {
		protoSettings =  Options::getProtocolSettings();
	}
	return protoSettings;
}

ProtocolSettings* HTMLBuilder::getChatProtocolSettings(HANDLE hContact) {
	char *szRealProto = getRealProto(hContact);
	ProtocolSettings *protoSettings =  getChatProtocolSettings(szRealProto);
	delete szRealProto;
	return protoSettings;
}

void HTMLBuilder::setLastIEViewEvent(IEVIEWEVENT *event) {
	lastIEViewEvent.cbSize = sizeof (IEVIEWEVENT);
	lastIEViewEvent.iType = event->iType;
	lastIEViewEvent.codepage = CP_ACP;
	if (event->cbSize >= IEVIEWEVENT_SIZE_V2) {
		lastIEViewEvent.codepage = event->codepage;
	}
	lastIEViewEvent.count = 0;
	lastIEViewEvent.dwFlags = event->dwFlags;
	lastIEViewEvent.hContact = event->hContact;
	lastIEViewEvent.hwnd = event->hwnd;
	lastIEViewEvent.eventData = NULL;
	if (lastIEViewEvent.pszProto != NULL) {
		delete (char *)lastIEViewEvent.pszProto ;
	}
	if (event->cbSize >= IEVIEWEVENT_SIZE_V3 && event->pszProto != NULL) {
		lastIEViewEvent.pszProto = Utils::dupString(event->pszProto);
	} else {
		lastIEViewEvent.pszProto = getProto(event->hContact);
	}
}

void HTMLBuilder::clear(IEView *view, IEVIEWEVENT *event) {
	if (event != NULL) {
		setLastIEViewEvent(event);
	}
	if (lastIEViewEvent.pszProto != NULL || event->hContact == NULL) {
		buildHead(view, &lastIEViewEvent);
	}
}
