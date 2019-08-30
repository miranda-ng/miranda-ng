/*
Scriver

Copyright (c) 2000-12 Miranda ICQ/IM project,

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

#define LOGICON_MSG_IN      0
#define LOGICON_MSG_OUT     1
#define LOGICON_MSG_NOTICE  2

static int logPixelSY;
static char* pLogIconBmpBits[3];
static HIMAGELIST g_hImageList;

#define STREAMSTAGE_HEADER  0
#define STREAMSTAGE_EVENTS  1
#define STREAMSTAGE_TAIL    2
#define STREAMSTAGE_STOP    3

struct EventData
{
	int cbSize;
	int iType;
	DWORD	dwFlags;
	const char *fontName;
	int fontSize;
	int fontStyle;
	COLORREF	color;
	union {
		char *pszNick;     // Nick, usage depends on type of event
		wchar_t *pszNickT;
		wchar_t *pszNickW; // Nick - Unicode
	};
	union {
		char *pszText;     // Text, usage depends on type of event
		wchar_t *pszTextT;
		wchar_t *pszTextW; // Text - Unicode
	};
	union {
		char *pszText2;     // Text, usage depends on type of event
		wchar_t *pszText2T;
		wchar_t *pszText2W; // Text - Unicode
	};
	DWORD	time;
	DWORD	eventType;
	BOOL  custom;
	EventData *next;
};

struct LogStreamData
{
	int      stage;
	MCONTACT hContact;
	MEVENT   hDbEvent, hDbEventLast;
	char    *buffer;
	size_t   bufferOffset, bufferLen;
	int      eventsToInsert;
	int      isFirst;
	CSrmmWindow *dlgDat;
	GlobalMessageData *gdat;
	EventData *events;
};

int DbEventIsCustomForMsgWindow(DBEVENTINFO *dbei)
{
	DBEVENTTYPEDESCR *et = DbEvent_GetType(dbei->szModule, dbei->eventType);
	return et && (et->flags & DETF_MSGWINDOW);
}

int DbEventIsMessageOrCustom(DBEVENTINFO* dbei)
{
	return dbei->eventType == EVENTTYPE_MESSAGE || DbEventIsCustomForMsgWindow(dbei);
}

int DbEventIsShown(DBEVENTINFO &dbei)
{
	switch (dbei.eventType) {
	case EVENTTYPE_MESSAGE:
	case EVENTTYPE_FILE:
		return 1;

	case EVENTTYPE_JABBER_CHATSTATES:
	case EVENTTYPE_JABBER_PRESENCE:
		return 0;
	}

	return DbEventIsCustomForMsgWindow(&dbei);
}

EventData* getEventFromDB(CSrmmWindow *dat, MCONTACT hContact, MEVENT hDbEvent)
{
	DBEVENTINFO dbei = {};
	dbei.cbBlob = db_event_getBlobSize(hDbEvent);
	if (dbei.cbBlob == -1)
		return nullptr;

	dbei.pBlob = (PBYTE)mir_alloc(dbei.cbBlob);
	db_event_get(hDbEvent, &dbei);
	if (!DbEventIsShown(dbei)) {
		mir_free(dbei.pBlob);
		return nullptr;
	}

	EventData *evt = (EventData*)mir_calloc(sizeof(EventData));
	evt->custom = DbEventIsCustomForMsgWindow(&dbei);
	if (!(dbei.flags & DBEF_SENT) && (dbei.eventType == EVENTTYPE_MESSAGE || evt->custom)) {
		db_event_markRead(hContact, hDbEvent);
		g_clistApi.pfnRemoveEvent(hContact, hDbEvent);
	}
	else if (dbei.eventType == EVENTTYPE_JABBER_CHATSTATES || dbei.eventType == EVENTTYPE_JABBER_PRESENCE)
		db_event_markRead(hContact, hDbEvent);

	evt->eventType = dbei.eventType;
	evt->dwFlags = (dbei.flags & DBEF_READ ? IEEDF_READ : 0) | (dbei.flags & DBEF_SENT ? IEEDF_SENT : 0) | (dbei.flags & DBEF_RTL ? IEEDF_RTL : 0);
	evt->dwFlags |= IEEDF_UNICODE_TEXT | IEEDF_UNICODE_NICK | IEEDF_UNICODE_TEXT2;

	if (dat->m_bUseRtl)
		evt->dwFlags |= IEEDF_RTL;

	evt->time = dbei.timestamp;
	evt->pszNick = nullptr;
	if (evt->dwFlags & IEEDF_SENT)
		evt->pszNickT = Contact_GetInfo(CNF_DISPLAY, 0, dat->m_szProto);
	else
		evt->pszNickT = mir_wstrdup(Clist_GetContactDisplayName(hContact));

	if (evt->eventType == EVENTTYPE_FILE) {
		char *filename = ((char*)dbei.pBlob) + sizeof(DWORD);
		char *descr = filename + mir_strlen(filename) + 1;
		evt->pszTextT = DbEvent_GetString(&dbei, filename);
		if (*descr != 0)
			evt->pszText2T = DbEvent_GetString(&dbei, descr);
	}
	else evt->pszTextT = DbEvent_GetTextW(&dbei, CP_UTF8);

	if (!dat->m_bUseRtl && Utils_IsRtl(evt->pszTextT))
		evt->dwFlags |= IEEDF_RTL;

	mir_free(dbei.pBlob);
	return evt;
}

static EventData* GetTestEvent(DWORD flags)
{
	EventData *evt = (EventData *)mir_calloc(sizeof(EventData));
	evt->eventType = EVENTTYPE_MESSAGE;
	evt->dwFlags = IEEDF_READ | flags;
	evt->dwFlags |= IEEDF_UNICODE_TEXT | IEEDF_UNICODE_NICK | IEEDF_UNICODE_TEXT2;
	evt->time = time(0);
	return evt;
}

static EventData* GetTestEvents()
{
	EventData *evt, *firstEvent, *prevEvent;
	firstEvent = prevEvent = evt = GetTestEvent(IEEDF_SENT);
	evt->pszNickT = mir_wstrdup(TranslateT("Me"));
	evt->pszTextT = mir_wstrdup(TranslateT("O Lord, bless this Thy hand grenade that with it Thou mayest blow Thine enemies"));

	evt = GetTestEvent(IEEDF_SENT);
	evt->pszNickT = mir_wstrdup(TranslateT("Me"));
	evt->pszTextT = mir_wstrdup(TranslateT("to tiny bits, in Thy mercy"));
	prevEvent->next = evt;
	prevEvent = evt;

	evt = GetTestEvent(0);
	evt->pszNickT = mir_wstrdup(TranslateT("My contact"));
	evt->pszTextT = mir_wstrdup(TranslateT("Lorem ipsum dolor sit amet,"));
	prevEvent->next = evt;
	prevEvent = evt;

	evt = GetTestEvent(0);
	evt->pszNickT = mir_wstrdup(TranslateT("My contact"));
	evt->pszTextT = mir_wstrdup(TranslateT("consectetur adipisicing elit"));
	prevEvent->next = evt;
	prevEvent = evt;
	return firstEvent;
}

static void freeEvent(EventData *evt)
{
	mir_free(evt->pszNickT);
	mir_free(evt->pszTextT);
	mir_free(evt->pszText2T);
	mir_free(evt);
}

static int AppendUnicodeOrAnsiiToBufferL(CMStringA &buf, const WCHAR *line, size_t maxLen, BOOL isAnsii)
{
	if (maxLen == -1)
		maxLen = mir_wstrlen(line);

	const WCHAR *maxLine = line + maxLen;

	if (isAnsii)
		buf.Append("{");
	else
		buf.Append("{\\uc1 ");

	int wasEOL = 0, textCharsCount = 0;
	for (; line < maxLine; line++, textCharsCount++) {
		wasEOL = 0;
		if (*line == '\r' && line[1] == '\n') {
			buf.Append("\\line ");
			wasEOL = 1;
			line++;
		}
		else if (*line == '\n') {
			buf.Append("\\line ");
			wasEOL = 1;
		}
		else if (*line == '\t') {
			buf.Append("\\tab ");
		}
		else if (*line == '\\' || *line == '{' || *line == '}') {
			buf.AppendChar('\\');
			buf.AppendChar((char)*line);
		}
		else if (*line < 128) {
			buf.AppendChar((char)*line);
		}
		else if (isAnsii) {
			buf.AppendFormat("\\'%02x", (*line) & 0xFF);
		}
		else {
			buf.AppendFormat("\\u%d ?", *line);
		}
	}
	if (wasEOL)
		buf.AppendChar(' ');

	buf.AppendChar('}');

	return textCharsCount;
}

static int AppendAnsiToBuffer(CMStringA &buf, const char *line)
{
	return AppendUnicodeOrAnsiiToBufferL(buf, _A2T(line), -1, true);
}

static int AppendUnicodeToBuffer(CMStringA &buf, const WCHAR *line)
{
	return AppendUnicodeOrAnsiiToBufferL(buf, line, -1, false);
}

// mir_free() the return value
static char* CreateRTFHeader()
{
	HDC hdc = GetDC(nullptr);
	logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(nullptr, hdc);

	CMStringA buf;

	buf.Append("{\\rtf1\\ansi\\deff0{\\fonttbl");
	for (int i = 0; i < fontOptionsListSize; i++) {
		LOGFONT lf;
		LoadMsgDlgFont(i, &lf, nullptr);
		buf.AppendFormat("{\\f%u\\fnil\\fcharset%u %S;}", i, lf.lfCharSet, lf.lfFaceName);
	}
	buf.Append("}{\\colortbl ");

	COLORREF colour;
	for (int i = 0; i < fontOptionsListSize; i++) {
		LoadMsgDlgFont(i, nullptr, &colour);
		buf.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	}
	if (GetSysColorBrush(COLOR_HOTLIGHT) == nullptr)
		colour = RGB(0, 0, 255);
	else
		colour = GetSysColor(COLOR_HOTLIGHT);
	buf.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = g_plugin.getDword(SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR);
	buf.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = g_plugin.getDword(SRMSGSET_INCOMINGBKGCOLOUR, SRMSGDEFSET_INCOMINGBKGCOLOUR);
	buf.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = g_plugin.getDword(SRMSGSET_OUTGOINGBKGCOLOUR, SRMSGDEFSET_OUTGOINGBKGCOLOUR);
	buf.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = g_plugin.getDword(SRMSGSET_LINECOLOUR, SRMSGDEFSET_LINECOLOUR);
	buf.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	buf.Append("}");
	return buf.Detach();
}

// mir_free() the return value
static char* CreateRTFTail()
{
	return mir_strdup("}");
}

// return value is static
static char* SetToStyle(int style)
{
	static char szStyle[128];
	LOGFONT lf;
	LoadMsgDlgFont(style, &lf, nullptr);
	mir_snprintf(szStyle, "\\f%u\\cf%u\\b%d\\i%d\\fs%u", style, style, lf.lfWeight >= FW_BOLD ? 1 : 0, lf.lfItalic, 2 * abs(lf.lfHeight) * 74 / logPixelSY);
	return szStyle;
}

// mode: 0 - date & time, 1 - date, 2 - time
static wchar_t* TimestampToString(WindowFlags flags, time_t check, int mode)
{
	static wchar_t szResult[512];
	wchar_t str[80];
	wchar_t format[20];

	szResult[0] = '\0';
	format[0] = '\0';
	if ((mode == 0 || mode == 1) && flags.bShowDate) {
		struct tm tm_now, tm_today;
		time_t now = time(0);
		time_t today;
		tm_now = *localtime(&now);
		tm_today = tm_now;
		tm_today.tm_hour = tm_today.tm_min = tm_today.tm_sec = 0;
		today = mktime(&tm_today);

		if (flags.bRelativeDate && check >= today) {
			mir_wstrcpy(szResult, TranslateT("Today"));
			if (mode == 0)
				mir_wstrcat(szResult, L",");
		}
		else if (flags.bRelativeDate && check > (today - 86400)) {
			mir_wstrcpy(szResult, TranslateT("Yesterday"));
			if (mode == 0)
				mir_wstrcat(szResult, L",");
		}
		else {
			if (flags.bLongDate)
				mir_wstrcpy(format, L"D");
			else
				mir_wstrcpy(format, L"d");
		}
	}
	if (mode == 0 || mode == 2) {
		if (mode == 0 && (flags.bShowDate))
			mir_wstrcat(format, L" ");

		mir_wstrcat(format, (flags.bShowSeconds) ? L"s" : L"t");
	}
	if (format[0] != '\0') {
		TimeZone_PrintTimeStamp(nullptr, check, format, str, _countof(str), 0);
		mir_wstrncat(szResult, str, _countof(szResult) - mir_wstrlen(szResult));
	}
	return szResult;
}

int isSameDate(time_t time1, time_t time2)
{
	struct tm tm_t1 = *localtime((time_t *)(&time1));
	struct tm tm_t2 = *localtime((time_t *)(&time2));
	if (tm_t1.tm_year == tm_t2.tm_year && tm_t1.tm_mon == tm_t2.tm_mon
		&& tm_t1.tm_mday == tm_t2.tm_mday) {
		return 1;
	}
	return 0;
}

static void AppendWithCustomLinks(EventData *evt, int style, CMStringA &buf)
{
	if (evt->pszText == nullptr)
		return;

	BOOL isAnsii = (evt->dwFlags & IEEDF_UNICODE_TEXT) == 0;
	WCHAR *wText;
	size_t len;
	if (isAnsii) {
		len = mir_strlen(evt->pszText);
		wText = mir_a2u(evt->pszText);
	}
	else {
		wText = evt->pszTextW;
		len = (int)mir_wstrlen(evt->pszTextW);
	}

	if (len > 0) {
		buf.AppendFormat("%s ", SetToStyle(style));
		AppendUnicodeOrAnsiiToBufferL(buf, wText, len, isAnsii);
	}

	if (isAnsii)
		mir_free(wText);
}

// mir_free() the return value
static char* CreateRTFFromEvent(CSrmmWindow *dat, EventData *evt, GlobalMessageData *gdat, LogStreamData *streamData)
{
	int style, showColon = 0;
	int isGroupBreak = TRUE;
	int highlight = 0;

	if ((gdat->flags.bGroupMessages) && evt->dwFlags == LOWORD(dat->m_lastEventType) &&
		evt->eventType == EVENTTYPE_MESSAGE && HIWORD(dat->m_lastEventType) == EVENTTYPE_MESSAGE &&
		(isSameDate(evt->time, dat->m_lastEventTime)) && ((((int)evt->time < dat->m_startTime) == (dat->m_lastEventTime < dat->m_startTime)) || !(evt->dwFlags & IEEDF_READ))) {
		isGroupBreak = FALSE;
	}

	CMStringA buf;
	if (!streamData->isFirst && !dat->m_isMixed) {
		if (isGroupBreak || gdat->flags.bMarkFollowups)
			buf.Append("\\par");
		else
			buf.Append("\\line");
	}

	if (evt->dwFlags & IEEDF_RTL)
		dat->m_isMixed = 1;

	if (!streamData->isFirst && isGroupBreak && (gdat->flags.bDrawLines))
		buf.AppendFormat("\\sl-1\\slmult0\\highlight%d\\cf%d\\fs1  \\par\\sl0", fontOptionsListSize + 4, fontOptionsListSize + 4);

	buf.Append((evt->dwFlags & IEEDF_RTL) ? "\\rtlpar" : "\\ltrpar");

	if (evt->eventType == EVENTTYPE_MESSAGE)
		highlight = fontOptionsListSize + 2 + ((evt->dwFlags & IEEDF_SENT) ? 1 : 0);
	else
		highlight = fontOptionsListSize + 1;

	buf.AppendFormat("\\highlight%d\\cf%d", highlight, highlight);
	if (!streamData->isFirst && dat->m_isMixed) {
		if (isGroupBreak)
			buf.Append("\\sl-1 \\par\\sl0");
		else
			buf.Append("\\sl-1 \\line\\sl0");
	}
	streamData->isFirst = FALSE;
	if (dat->m_isMixed) {
		if (evt->dwFlags & IEEDF_RTL)
			buf.Append("\\ltrch\\rtlch");
		else
			buf.Append("\\rtlch\\ltrch");
	}
	if ((gdat->flags.bShowIcons) && isGroupBreak) {
		int i = LOGICON_MSG_NOTICE;

		switch (evt->eventType) {
		case EVENTTYPE_MESSAGE:
			if (evt->dwFlags & IEEDF_SENT)
				i = LOGICON_MSG_OUT;
			else
				i = LOGICON_MSG_IN;
			break;

		default:
			i = LOGICON_MSG_NOTICE;
			break;
		}

		buf.Append("\\fs1  ");
		buf.Append(pLogIconBmpBits[i]);
		buf.AppendChar(' ');
	}

	if (gdat->flags.bShowTime && (evt->eventType != EVENTTYPE_MESSAGE ||
		(gdat->flags.bMarkFollowups || isGroupBreak || !(gdat->flags.bGroupMessages)))) {
		wchar_t *timestampString = nullptr;
		if (gdat->flags.bGroupMessages && evt->eventType == EVENTTYPE_MESSAGE) {
			if (isGroupBreak) {
				if (!gdat->flags.bMarkFollowups)
					timestampString = TimestampToString(gdat->flags, evt->time, 0);
				else if (gdat->flags.bShowDate)
					timestampString = TimestampToString(gdat->flags, evt->time, 1);
			}
			else if (gdat->flags.bMarkFollowups)
				timestampString = TimestampToString(gdat->flags, evt->time, 2);
		}
		else timestampString = TimestampToString(gdat->flags, evt->time, 0);

		if (timestampString != nullptr) {
			buf.AppendFormat("%s ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME));
			AppendUnicodeToBuffer(buf, timestampString);
		}
		if (evt->eventType != EVENTTYPE_MESSAGE)
			buf.AppendFormat("%s: ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON));
		showColon = 1;
	}
	if ((!(gdat->flags.bHideNames) && evt->eventType == EVENTTYPE_MESSAGE && isGroupBreak) || evt->eventType == EVENTTYPE_JABBER_CHATSTATES || evt->eventType == EVENTTYPE_JABBER_PRESENCE) {
		if (evt->eventType == EVENTTYPE_MESSAGE) {
			if (showColon)
				buf.AppendFormat(" %s ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME));
			else
				buf.AppendFormat("%s ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME));
		}
		else buf.AppendFormat("%s ", SetToStyle(MSGFONTID_NOTICE));

		if (evt->dwFlags & IEEDF_UNICODE_NICK)
			AppendUnicodeToBuffer(buf, evt->pszNickW);
		else
			AppendAnsiToBuffer(buf, evt->pszNick);

		showColon = 1;
		if (evt->eventType == EVENTTYPE_MESSAGE && gdat->flags.bGroupMessages) {
			if (gdat->flags.bMarkFollowups)
				buf.Append("\\par");
			else
				buf.Append("\\line");
			showColon = 0;
		}
	}

	if (gdat->flags.bShowTime && gdat->flags.bGroupMessages && gdat->flags.bMarkFollowups && evt->eventType == EVENTTYPE_MESSAGE && isGroupBreak) {
		buf.AppendFormat(" %s ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME));
		AppendUnicodeToBuffer(buf, TimestampToString(gdat->flags, evt->time, 2));
		showColon = 1;
	}
	if (showColon && evt->eventType == EVENTTYPE_MESSAGE) {
		if (evt->dwFlags & IEEDF_RTL)
			buf.AppendFormat("\\~%s: ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON));
		else
			buf.AppendFormat("%s: ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON));
	}
	switch (evt->eventType) {
	case EVENTTYPE_JABBER_CHATSTATES:
	case EVENTTYPE_JABBER_PRESENCE:
	case EVENTTYPE_FILE:
		style = MSGFONTID_NOTICE;
		buf.AppendFormat("%s ", SetToStyle(style));
		if (evt->eventType == EVENTTYPE_FILE) {
			if (evt->dwFlags & IEEDF_SENT)
				AppendUnicodeToBuffer(buf, TranslateT("File sent"));
			else
				AppendUnicodeToBuffer(buf, TranslateT("File received"));
			AppendUnicodeToBuffer(buf, L":");
		}
		AppendUnicodeToBuffer(buf, L" ");

		if (evt->pszTextW != nullptr) {
			if (evt->dwFlags & IEEDF_UNICODE_TEXT)
				AppendUnicodeToBuffer(buf, evt->pszTextW);
			else
				AppendAnsiToBuffer(buf, evt->pszText);
		}

		if (evt->pszText2W != nullptr) {
			AppendUnicodeToBuffer(buf, L" (");
			if (evt->dwFlags & IEEDF_UNICODE_TEXT2)
				AppendUnicodeToBuffer(buf, evt->pszText2W);
			else
				AppendAnsiToBuffer(buf, evt->pszText2);
			AppendUnicodeToBuffer(buf, L")");
		}
		break;
	default:
		if (gdat->flags.bMsgOnNewline && showColon)
			buf.Append("\\line");

		style = evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYMSG : MSGFONTID_YOURMSG;
		AppendWithCustomLinks(evt, style, buf);
		break;
	}
	if (dat->m_isMixed)
		buf.Append("\\par");

	dat->m_lastEventTime = evt->time;
	dat->m_lastEventType = MAKELONG(evt->dwFlags, evt->eventType);
	return buf.Detach();
}

static DWORD CALLBACK LogStreamInEvents(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	LogStreamData *dat = (LogStreamData*)dwCookie;

	if (dat->buffer == nullptr) {
		dat->bufferOffset = 0;
		switch (dat->stage) {
		case STREAMSTAGE_HEADER:
			dat->buffer = CreateRTFHeader();
			dat->stage = STREAMSTAGE_EVENTS;
			break;
		case STREAMSTAGE_EVENTS:
			if (dat->events != nullptr) {
				EventData *evt = dat->events;
				dat->buffer = nullptr;
				dat->buffer = CreateRTFFromEvent(dat->dlgDat, evt, dat->gdat, dat);
				dat->events = evt->next;
				freeEvent(evt);
			}
			else if (dat->eventsToInsert) {
				do {
					EventData *evt = getEventFromDB(dat->dlgDat, dat->hContact, dat->hDbEvent);
					dat->buffer = nullptr;
					if (evt != nullptr) {
						dat->buffer = CreateRTFFromEvent(dat->dlgDat, evt, dat->gdat, dat);
						freeEvent(evt);
					}
					if (dat->buffer)
						dat->hDbEventLast = dat->hDbEvent;
					dat->hDbEvent = db_event_next(dat->hContact, dat->hDbEvent);
					if (--dat->eventsToInsert == 0)
						break;
				} while (dat->buffer == nullptr && dat->hDbEvent);
			}
			if (dat->buffer)
				break;

			dat->stage = STREAMSTAGE_TAIL;
			__fallthrough;

		case STREAMSTAGE_TAIL:
			dat->buffer = CreateRTFTail();
			dat->stage = STREAMSTAGE_STOP;
			break;
		case STREAMSTAGE_STOP:
			*pcb = 0;
			return 0;
		}
		dat->bufferLen = mir_strlen(dat->buffer);
	}
	*pcb = min(cb, LONG(dat->bufferLen - dat->bufferOffset));
	memcpy(pbBuff, dat->buffer + dat->bufferOffset, *pcb);
	dat->bufferOffset += *pcb;
	if (dat->bufferOffset == dat->bufferLen) {
		mir_free(dat->buffer);
		dat->buffer = nullptr;
	}
	return 0;
}

void StreamInTestEvents(HWND hEditWnd, GlobalMessageData *gdat)
{
	CSrmmWindow *dat = new CSrmmWindow(0, false);

	LogStreamData streamData = { 0 };
	streamData.isFirst = TRUE;
	streamData.events = GetTestEvents();
	streamData.dlgDat = dat;
	streamData.gdat = gdat;

	EDITSTREAM stream = { 0 };
	stream.pfnCallback = LogStreamInEvents;
	stream.dwCookie = (DWORD_PTR)&streamData;
	SendMessage(hEditWnd, EM_STREAMIN, SF_RTF, (LPARAM)&stream);
	SendMessage(hEditWnd, EM_HIDESELECTION, FALSE, 0);

	delete dat;
}

void CSrmmWindow::StreamInEvents(MEVENT hDbEventFirst, int count, int bAppend)
{
	// IEVIew MOD Begin
	if (m_hwndIeview != nullptr) {
		IEVIEWEVENT evt;
		IEVIEWWINDOW ieWindow;
		memset(&evt, 0, sizeof(evt));
		evt.cbSize = sizeof(evt);
		evt.dwFlags = (m_bUseRtl) ? IEEF_RTL : 0;
		evt.hwnd = m_hwndIeview;
		evt.hContact = m_hContact;
		evt.pszProto = m_szProto;
		if (!bAppend) {
			evt.iType = IEE_CLEAR_LOG;
			CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&evt);
		}
		evt.iType = IEE_LOG_DB_EVENTS;
		evt.hDbEventFirst = hDbEventFirst;
		evt.count = count;
		CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&evt);
		m_hDbEventLast = evt.hDbEventFirst != 0 ? evt.hDbEventFirst : m_hDbEventLast;

		memset(&ieWindow, 0, sizeof(ieWindow));
		ieWindow.cbSize = sizeof(ieWindow);
		ieWindow.iType = IEW_SCROLLBOTTOM;
		ieWindow.hwnd = m_hwndIeview;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
		return;
	}
	// IEVIew MOD End

	CHARRANGE oldSel, sel;
	m_log.SendMsg(WM_SETREDRAW, FALSE, 0);
	m_log.SendMsg(EM_EXGETSEL, 0, (LPARAM)&oldSel);

	LogStreamData streamData = {};
	streamData.hContact = m_hContact;
	streamData.hDbEvent = hDbEventFirst;
	streamData.hDbEventLast = m_hDbEventLast;
	streamData.dlgDat = this;
	streamData.eventsToInsert = count;
	streamData.isFirst = bAppend ? m_log.GetRichTextLength() == 0 : 1;
	streamData.gdat = &g_dat;

	EDITSTREAM stream = {};
	stream.pfnCallback = LogStreamInEvents;
	stream.dwCookie = (DWORD_PTR)& streamData;
	sel.cpMin = 0;

	POINT scrollPos;
	bool bottomScroll = (GetFocus() != m_log.GetHwnd());
	if (bottomScroll && (GetWindowLongPtr(m_log.GetHwnd(), GWL_STYLE) & WS_VSCROLL)) {
		SCROLLINFO si = {};
		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
		GetScrollInfo(m_log.GetHwnd(), SB_VERT, &si);
		bottomScroll = (si.nPos + (int)si.nPage) >= si.nMax;
	}
	if (!bottomScroll)
		m_log.SendMsg(EM_GETSCROLLPOS, 0, (LPARAM)&scrollPos);

	FINDTEXTEXA fi;
	if (bAppend) {
		GETTEXTLENGTHEX gtxl = { 0 };
		gtxl.flags = GTL_DEFAULT | GTL_PRECISE | GTL_NUMCHARS;
		gtxl.codepage = 1200;
		fi.chrg.cpMin = m_log.SendMsg(EM_GETTEXTLENGTHEX, (WPARAM)&gtxl, 0);
		sel.cpMin = sel.cpMax = m_log.GetRichTextLength();
		m_log.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
	}
	else {
		m_log.SendMsg(WM_SETREDRAW, FALSE, 0);
		ClearLog();
		sel.cpMin = 0;
		sel.cpMax = m_log.GetRichTextLength();
		m_log.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
		fi.chrg.cpMin = 0;
		m_isMixed = 0;
	}

	m_log.SendMsg(EM_STREAMIN, bAppend ? SFF_SELECTION | SF_RTF : SFF_SELECTION | SF_RTF, (LPARAM)&stream);
	if (bottomScroll) {
		sel.cpMin = sel.cpMax = -1;
		m_log.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
	}
	else {
		m_log.SendMsg(EM_EXSETSEL, 0, (LPARAM)&oldSel);
		m_log.SendMsg(EM_SETSCROLLPOS, 0, (LPARAM)&scrollPos);
	}

	if (g_dat.smileyAddInstalled) {
		SMADD_RICHEDIT3 smre;
		smre.cbSize = sizeof(SMADD_RICHEDIT3);
		smre.hwndRichEditControl = m_log.GetHwnd();

		MCONTACT hContact = db_mc_getSrmmSub(m_hContact);
		smre.Protocolname = (hContact != 0) ? GetContactProto(hContact) : m_szProto;

		if (fi.chrg.cpMin > 0) {
			sel.cpMin = fi.chrg.cpMin;
			sel.cpMax = -1;
			smre.rangeToReplace = &sel;
		}
		else smre.rangeToReplace = nullptr;

		smre.disableRedraw = TRUE;
		smre.hContact = m_hContact;
		smre.flags = 0;
		CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM)&smre);
	}

	m_log.SendMsg(WM_SETREDRAW, TRUE, 0);
	if (bottomScroll) {
		ScrollToBottom();
		RedrawWindow(m_log.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	m_hDbEventLast = streamData.hDbEventLast;
}

#define RTFPICTHEADERMAXSIZE   78

void LoadMsgLogIcons(void)
{
	HICON hIcon = nullptr;
	RECT rc;

	g_hImageList = ImageList_Create(10, 10, ILC_COLOR32 | ILC_MASK, _countof(pLogIconBmpBits), 0);
	HBRUSH hBkgBrush = CreateSolidBrush(g_plugin.getDword(SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR));
	HBRUSH hInBkgBrush = CreateSolidBrush(g_plugin.getDword(SRMSGSET_INCOMINGBKGCOLOUR, SRMSGDEFSET_INCOMINGBKGCOLOUR));
	HBRUSH hOutBkgBrush = CreateSolidBrush(g_plugin.getDword(SRMSGSET_OUTGOINGBKGCOLOUR, SRMSGDEFSET_OUTGOINGBKGCOLOUR));

	BITMAPINFOHEADER bih = { sizeof(bih) };
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;
	bih.biHeight = 10;
	bih.biPlanes = 1;
	bih.biWidth = 10;
	int widthBytes = ((bih.biWidth * bih.biBitCount + 31) >> 5) * 4;
	rc.top = rc.left = 0;
	rc.right = bih.biWidth;
	rc.bottom = bih.biHeight;
	HDC hdc = GetDC(nullptr);
	HBITMAP hBmp = CreateCompatibleBitmap(hdc, bih.biWidth, bih.biHeight);
	HDC hdcMem = CreateCompatibleDC(hdc);
	PBYTE pBmpBits = (PBYTE)mir_alloc(widthBytes * bih.biHeight);
	HBRUSH hBrush = hBkgBrush;
	for (int i = 0; i < _countof(pLogIconBmpBits); i++) {
		switch (i) {
		case LOGICON_MSG_IN:
			g_plugin.addImgListIcon(g_hImageList, IDI_INCOMING);
			hIcon = ImageList_GetIcon(g_hImageList, LOGICON_MSG_IN, ILD_NORMAL);
			hBrush = hInBkgBrush;
			break;
		case LOGICON_MSG_OUT:
			g_plugin.addImgListIcon(g_hImageList, IDI_OUTGOING);
			hIcon = ImageList_GetIcon(g_hImageList, LOGICON_MSG_OUT, ILD_NORMAL);
			hBrush = hOutBkgBrush;
			break;
		case LOGICON_MSG_NOTICE:
			g_plugin.addImgListIcon(g_hImageList, IDI_NOTICE);
			hIcon = ImageList_GetIcon(g_hImageList, LOGICON_MSG_NOTICE, ILD_NORMAL);
			hBrush = hBkgBrush;
			break;
		}

		pLogIconBmpBits[i] = (char*)mir_alloc(RTFPICTHEADERMAXSIZE + (bih.biSize + widthBytes * bih.biHeight) * 2);
		size_t rtfHeaderSize = sprintf(pLogIconBmpBits[i], "{\\pict\\dibitmap0\\wbmbitspixel%u\\wbmplanes1\\wbmwidthbytes%u\\picw%u\\pich%u ", bih.biBitCount, widthBytes, (UINT)bih.biWidth, (UINT)bih.biHeight); //!!!!!!!!!!!

		HBITMAP hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
		FillRect(hdcMem, &rc, hBrush);
		DrawIconEx(hdcMem, 0, 0, hIcon, bih.biWidth, bih.biHeight, 0, nullptr, DI_NORMAL);
		SelectObject(hdcMem, hoBmp);
		GetDIBits(hdc, hBmp, 0, bih.biHeight, pBmpBits, (BITMAPINFO *)& bih, DIB_RGB_COLORS);
		DestroyIcon(hIcon);

		char *szDest = pLogIconBmpBits[i] + rtfHeaderSize;
		bin2hex(&bih, sizeof(bih), szDest); szDest += sizeof(bih) * 2;
		bin2hex(pBmpBits, widthBytes * bih.biHeight, szDest); szDest += widthBytes * bih.biHeight * 2;
		mir_strcpy(szDest, "}");
	}
	mir_free(pBmpBits);
	DeleteDC(hdcMem);
	DeleteObject(hBmp);
	ReleaseDC(nullptr, hdc);
	DeleteObject(hBkgBrush);
	DeleteObject(hInBkgBrush);
	DeleteObject(hOutBkgBrush);
}

void FreeMsgLogIcons(void)
{
	for (auto &it : pLogIconBmpBits)
		mir_free(it);

	ImageList_RemoveAll(g_hImageList);
	ImageList_Destroy(g_hImageList);
}
