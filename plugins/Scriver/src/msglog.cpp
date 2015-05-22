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

#include "commonheaders.h"

#define LOGICON_MSG_IN      0
#define LOGICON_MSG_OUT     1
#define LOGICON_MSG_NOTICE  2

extern int RTL_Detect(WCHAR *pszwText);
static int logPixelSY;
static char* pLogIconBmpBits[3];
static size_t logIconBmpSize[SIZEOF(pLogIconBmpBits)];
static HIMAGELIST g_hImageList;

#define STREAMSTAGE_HEADER  0
#define STREAMSTAGE_EVENTS  1
#define STREAMSTAGE_TAIL    2
#define STREAMSTAGE_STOP    3

#define SMF_AFTERMASK (SMF_SHOWTIME | SMF_GROUPMESSAGES | SMF_MARKFOLLOWUPS)

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
		TCHAR *pszNickT;
		wchar_t *pszNickW; // Nick - Unicode
	};
	union {
		char *pszText;     // Text, usage depends on type of event
		TCHAR *pszTextT;
		wchar_t *pszTextW; // Text - Unicode
	};
	union {
		char *pszText2;     // Text, usage depends on type of event
		TCHAR *pszText2T;
		wchar_t *pszText2W; // Text - Unicode
	};
	DWORD	time;
	DWORD	eventType;
	int   codePage;
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
	SrmmWindowData *dlgDat;
	GlobalMessageData *gdat;
	EventData *events;
};

TCHAR* GetNickname(MCONTACT hContact, const char *szProto)
{
	CONTACTINFO ci = { sizeof(ci) };
	ci.hContact = hContact;
	ci.szProto = (char*)szProto;
	ci.dwFlag = CNF_DISPLAY;
	if (IsUnicodeMIM())
		ci.dwFlag |= CNF_UNICODE;

	TCHAR *szName = NULL;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
		if (ci.type == CNFT_ASCIIZ) {
			if (ci.pszVal) {
				if (IsUnicodeMIM()) {
					if (!mir_tstrcmp((TCHAR*)ci.pszVal, TranslateW(_T("'(Unknown Contact)'")))) {
						ci.dwFlag &= ~CNF_UNICODE;
						if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci))
							szName = mir_a2t((char*)ci.pszVal);
					}
					else szName = mir_tstrdup((TCHAR*)ci.pszVal);
				}
				else szName = mir_a2t((char*)ci.pszVal);

				mir_free(ci.pszVal);
				if (szName != NULL)
					return szName;
			}
		}
	}

	TCHAR *tszBaseNick = pcli->pfnGetContactDisplayName(hContact, 0);
	if (tszBaseNick != NULL)
		return mir_tstrdup(tszBaseNick);

	return mir_tstrdup(TranslateT("Unknown contact"));
}

int DbEventIsCustomForMsgWindow(DBEVENTINFO *dbei)
{
	DBEVENTTYPEDESCR *et = (DBEVENTTYPEDESCR*)CallService(MS_DB_EVENT_GETTYPE, (WPARAM)dbei->szModule, (LPARAM)dbei->eventType);
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
		return 1;

	case EVENTTYPE_JABBER_CHATSTATES:
	case EVENTTYPE_JABBER_PRESENCE:
		return 0;

	case EVENTTYPE_FILE:
	case EVENTTYPE_URL:
		return 1;
	}

	return DbEventIsCustomForMsgWindow(&dbei);
}

EventData* getEventFromDB(SrmmWindowData *dat, MCONTACT hContact, MEVENT hDbEvent)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.cbBlob = db_event_getBlobSize(hDbEvent);
	if (dbei.cbBlob == -1)
		return NULL;
	dbei.pBlob = (PBYTE)mir_alloc(dbei.cbBlob);
	db_event_get(hDbEvent, &dbei);
	if (!DbEventIsShown(dbei)) {
		mir_free(dbei.pBlob);
		return NULL;
	}

	EventData *evt = (EventData*)mir_calloc(sizeof(EventData));
	evt->custom = DbEventIsCustomForMsgWindow(&dbei);
	if (!(dbei.flags & DBEF_SENT) && (dbei.eventType == EVENTTYPE_MESSAGE || dbei.eventType == EVENTTYPE_URL || evt->custom)) {
		db_event_markRead(hContact, hDbEvent);
		CallService(MS_CLIST_REMOVEEVENT, hContact, (LPARAM)hDbEvent);
	}
	else if (dbei.eventType == EVENTTYPE_JABBER_CHATSTATES || dbei.eventType == EVENTTYPE_JABBER_PRESENCE)
		db_event_markRead(hContact, hDbEvent);

	evt->eventType = dbei.eventType;
	evt->dwFlags = (dbei.flags & DBEF_READ ? IEEDF_READ : 0) | (dbei.flags & DBEF_SENT ? IEEDF_SENT : 0) | (dbei.flags & DBEF_RTL ? IEEDF_RTL : 0);
	evt->dwFlags |= IEEDF_UNICODE_TEXT | IEEDF_UNICODE_NICK | IEEDF_UNICODE_TEXT2;

	if (dat->flags & SMF_RTL)
		evt->dwFlags |= IEEDF_RTL;

	evt->time = dbei.timestamp;
	evt->pszNick = NULL;
	evt->codePage = dat->codePage;

	if (evt->dwFlags & IEEDF_SENT)
		evt->pszNickT = GetNickname(NULL, dat->szProto);
	else
		evt->pszNickT = GetNickname(hContact, dat->szProto);

	if (evt->eventType == EVENTTYPE_FILE) {
		char *filename = ((char*)dbei.pBlob) + sizeof(DWORD);
		char *descr = filename + mir_strlen(filename) + 1;
		evt->pszTextT = DbGetEventStringT(&dbei, filename);
		if (*descr != 0)
			evt->pszText2T = DbGetEventStringT(&dbei, descr);
	}
	else evt->pszTextT = DbGetEventTextT(&dbei, dat->codePage);

	if (!(dat->flags & SMF_RTL) && RTL_Detect(evt->pszTextT))
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
	evt->time = time(NULL);
	evt->codePage = CP_ACP;
	return evt;
}

static EventData* GetTestEvents()
{
	EventData *evt, *firstEvent, *prevEvent;
	firstEvent = prevEvent = evt = GetTestEvent(IEEDF_SENT);
	evt->pszNickT = mir_tstrdup(TranslateT("Me"));
	evt->pszTextT = mir_tstrdup(TranslateT("O Lord, bless this Thy hand grenade that with it Thou mayest blow Thine enemies"));

	evt = GetTestEvent(IEEDF_SENT);
	evt->pszNickT = mir_tstrdup(TranslateT("Me"));
	evt->pszTextT = mir_tstrdup(TranslateT("to tiny bits, in Thy mercy"));
	prevEvent->next = evt;
	prevEvent = evt;

	evt = GetTestEvent(0);
	evt->pszNickT = mir_tstrdup(TranslateT("My contact"));
	evt->pszTextT = mir_tstrdup(TranslateT("Lorem ipsum dolor sit amet,"));
	prevEvent->next = evt;
	prevEvent = evt;

	evt = GetTestEvent(0);
	evt->pszNickT = mir_tstrdup(TranslateT("My contact"));
	evt->pszTextT = mir_tstrdup(TranslateT("consectetur adipisicing elit"));
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

static int AppendUnicodeOrAnsiiToBufferL(char *&buffer, size_t &cbBufferEnd, size_t &cbBufferAlloced, const WCHAR *line, size_t maxLen, BOOL isAnsii)
{
	if (maxLen == -1)
		maxLen = wcslen(line);
	
	const WCHAR *maxLine = line + maxLen;
	size_t lineLen = maxLen*9 + 8;
	if (cbBufferEnd + lineLen > cbBufferAlloced) {
		cbBufferAlloced += lineLen + 1024 - lineLen % 1024;
		buffer = (char*)mir_realloc(buffer, cbBufferAlloced);
	}

	char *d = buffer + cbBufferEnd;
	if (isAnsii) {
		mir_strcpy(d, "{");
		d++;
	}
	else {
		mir_strcpy(d, "{\\uc1 ");
		d += 6;
	}

	int wasEOL = 0, textCharsCount = 0;
	for (; line < maxLine; line++, textCharsCount++) {
		wasEOL = 0;
		if (*line == '\r' && line[1] == '\n') {
			memcpy(d, "\\line ", 6);
			wasEOL = 1;
			d += 6;
			line++;
		}
		else if (*line == '\n') {
			memcpy(d, "\\line ", 6);
			wasEOL = 1;
			d += 6;
		}
		else if (*line == '\t') {
			memcpy(d, "\\tab ", 5);
			d += 5;
		}
		else if (*line == '\\' || *line == '{' || *line == '}') {
			*d++ = '\\';
			*d++ = (char)*line;
		}
		else if (*line < 128) {
			*d++ = (char)*line;
		}
		else if (isAnsii) {
			d += sprintf(d, "\\'%02x", (*line) & 0xFF);
		}
		else {
			d += sprintf(d, "\\u%d ?", *line);
		}
	}
	if (wasEOL) {
		memcpy(d, " ", 1);
		d++;
	}
	mir_strcpy(d, "}");
	d++;

	cbBufferEnd = (int)(d - buffer);
	return textCharsCount;
}

static int AppendAnsiToBuffer(char *&buffer, size_t &cbBufferEnd, size_t &cbBufferAlloced, const char *line)
{
	return AppendUnicodeOrAnsiiToBufferL(buffer, cbBufferEnd, cbBufferAlloced, _A2T(line), -1, TRUE);
}

static int AppendUnicodeToBuffer(char *&buffer, size_t &cbBufferEnd, size_t &cbBufferAlloced, const WCHAR *line)
{
	return AppendUnicodeOrAnsiiToBufferL(buffer, cbBufferEnd, cbBufferAlloced, line, -1, FALSE);
}

// mir_free() the return value
static char* CreateRTFHeader()
{
	HDC hdc = GetDC(NULL);
	logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(NULL, hdc);

	size_t bufferEnd = 0, bufferAlloced = 1024;
	char *buffer = (char*)mir_alloc(bufferAlloced);
	buffer[0] = '\0';

	AppendToBuffer(buffer, bufferEnd, bufferAlloced,"{\\rtf1\\ansi\\deff0{\\fonttbl");
	for (int i = 0; i < fontOptionsListSize; i++) {
		LOGFONT lf;
		LoadMsgDlgFont(i, &lf, NULL);
		AppendToBuffer(buffer, bufferEnd, bufferAlloced, "{\\f%u\\fnil\\fcharset%u %S;}", i, lf.lfCharSet, lf.lfFaceName);
	}
	AppendToBuffer(buffer, bufferEnd, bufferAlloced, "}{\\colortbl ");

	COLORREF colour;
	for (int i = 0; i < fontOptionsListSize; i++) {
		LoadMsgDlgFont(i, NULL, &colour);
		AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	}
	if (GetSysColorBrush(COLOR_HOTLIGHT) == NULL)
		colour = RGB(0, 0, 255);
	else
		colour = GetSysColor(COLOR_HOTLIGHT);
	AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = db_get_dw(NULL, SRMMMOD, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR);
	AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = db_get_dw(NULL, SRMMMOD, SRMSGSET_INCOMINGBKGCOLOUR, SRMSGDEFSET_INCOMINGBKGCOLOUR);
	AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = db_get_dw(NULL, SRMMMOD, SRMSGSET_OUTGOINGBKGCOLOUR, SRMSGDEFSET_OUTGOINGBKGCOLOUR);
	AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = db_get_dw(NULL, SRMMMOD, SRMSGSET_LINECOLOUR, SRMSGDEFSET_LINECOLOUR);
	AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	AppendToBuffer(buffer, bufferEnd, bufferAlloced, "}");
	return buffer;
}

// mir_free() the return value
static char* CreateRTFTail()
{
	size_t bufferAlloced = 1024, bufferEnd = 0;
	char *buffer = (char*)mir_alloc(bufferAlloced); buffer[0] = '\0';
	AppendToBuffer(buffer, bufferEnd, bufferAlloced, "}");
	return buffer;
}

// return value is static
static char* SetToStyle(int style)
{
	static char szStyle[128];
	LOGFONT lf;
	LoadMsgDlgFont(style, &lf, NULL);
	mir_snprintf(szStyle, SIZEOF(szStyle), "\\f%u\\cf%u\\b%d\\i%d\\fs%u", style, style, lf.lfWeight >= FW_BOLD ? 1 : 0, lf.lfItalic, 2 * abs(lf.lfHeight) * 74 / logPixelSY);
	return szStyle;
}

// mode: 0 - date & time, 1 - date, 2 - time
TCHAR* TimestampToString(DWORD dwFlags, time_t check, int mode)
{
	static TCHAR szResult[512];
	TCHAR str[80];
	TCHAR format[20];

	szResult[0] = '\0';
	format[0] = '\0';
	if ((mode == 0 || mode == 1) && (dwFlags & SMF_SHOWDATE)) {
		struct tm tm_now, tm_today;
		time_t now = time(NULL);
		time_t today;
		tm_now = *localtime(&now);
		tm_today = tm_now;
		tm_today.tm_hour = tm_today.tm_min = tm_today.tm_sec = 0;
		today = mktime(&tm_today);

		if (dwFlags & SMF_RELATIVEDATE && check >= today) {
			mir_tstrcpy(szResult, TranslateT("Today"));
			if (mode == 0)
				mir_tstrcat(szResult, _T(","));
		}
		else if (dwFlags & SMF_RELATIVEDATE && check > (today - 86400)) {
			mir_tstrcpy(szResult, TranslateT("Yesterday"));
			if (mode == 0)
				mir_tstrcat(szResult, _T(","));
		}
		else {
			if (dwFlags & SMF_LONGDATE)
				mir_tstrcpy(format, _T("D"));
			else
				mir_tstrcpy(format, _T("d"));
		}
	}
	if (mode == 0 || mode == 2) {
		if (mode == 0 && (dwFlags & SMF_SHOWDATE))
			mir_tstrcat(format, _T(" "));

		mir_tstrcat(format, (dwFlags & SMF_SHOWSECONDS) ? _T("s") : _T("t"));
	}
	if (format[0] != '\0') {
		tmi.printTimeStamp(NULL, check, format, str, SIZEOF(str), 0);
		_tcsncat(szResult, str, SIZEOF(szResult) - mir_tstrlen(szResult));
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

static int DetectURL(wchar_t *text, BOOL firstChar) {
	wchar_t c;
	struct prefix_s {
		wchar_t *text;
		int length;
	} prefixes[12] = {
		{L"http:", 5},
		{L"file:", 5},
		{L"mailto:", 7},
		{L"ftp:", 4},
		{L"https:", 6},
		{L"gopher:", 7},
		{L"nntp:", 5},
		{L"prospero:", 9},
		{L"telnet:", 7},
		{L"news:", 5},
		{L"wais:", 5},
		{L"www.", 4}
	};
	c = firstChar ? ' ' : text[-1];
	if (!((c >= '0' && c<='9') || (c >= 'A' && c<='Z') || (c >= 'a' && c<='z'))) {
		int found = 0;
		int i, len = 0;
		int prefixlen = SIZEOF(prefixes);
		for (i = 0; i < prefixlen; i++) {
			if (!wcsncmp(text, prefixes[i].text, prefixes[i].length)) {
				len = prefixes[i].length;
				found = 1;
				break;
			}
		}
		if (found) {
			for (; text[len]!='\n' && text[len]!='\r' && text[len]!='\t' && text[len]!=' ' && text[len]!='\0';  len++);
			for (; len > 0; len --)
				if ((text[len-1] >= '0' && text[len-1]<='9') || iswalpha(text[len-1]))
					break;

			return len;
		}
	}
	return 0;
}

static void AppendWithCustomLinks(EventData *evt, int style, char *&buffer, size_t &bufferEnd, size_t &bufferAlloced)
{
	if (evt->pszText == NULL)
		return;

	BOOL isAnsii = (evt->dwFlags & IEEDF_UNICODE_TEXT) == 0;
	WCHAR *wText;
	int lasttoken = 0;
	size_t len, laststart = 0;
	if (isAnsii) {
		len = mir_strlen(evt->pszText);
		wText = mir_a2u(evt->pszText);
	}
	else {
		wText = evt->pszTextW;
		len = (int)wcslen(evt->pszTextW);
	}
	for (size_t j = 0; j < len; j++) {
		int newtoken = 0;
		int l = DetectURL(wText + j, j == 0);
		if (l > 0)
			newtoken = 1;

		if (j == 0)
			lasttoken = newtoken;

		if (newtoken != lasttoken) {
			if (lasttoken == 0)
				AppendToBuffer(buffer, bufferEnd, bufferAlloced, "%s ", SetToStyle(style));
			else
				AppendToBuffer(buffer, bufferEnd, bufferAlloced, "%s ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYURL : MSGFONTID_YOURURL));

			AppendUnicodeOrAnsiiToBufferL(buffer, bufferEnd, bufferAlloced, wText + laststart, j - laststart, isAnsii);
			laststart = j;
			lasttoken = newtoken;
		}
	}
	if (len - laststart > 0) {
		if (lasttoken == 0)
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, "%s ", SetToStyle(style));
		else
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, "%s ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYURL : MSGFONTID_YOURURL));

		AppendUnicodeOrAnsiiToBufferL(buffer, bufferEnd, bufferAlloced, wText + laststart, len - laststart, isAnsii);
	}
	if (isAnsii)
		mir_free(wText);
}

//mir_free() the return value
static char* CreateRTFFromEvent(SrmmWindowData *dat, EventData *evt, GlobalMessageData *gdat, LogStreamData *streamData)
{
	int style, showColon = 0;
	int isGroupBreak = TRUE;
	int highlight = 0;
	
	size_t bufferEnd = 0, bufferAlloced = 1024;
	char *buffer = (char*)mir_alloc(bufferAlloced); buffer[0] = '\0';

 	if ((gdat->flags & SMF_GROUPMESSAGES) && evt->dwFlags == LOWORD(dat->lastEventType) &&
			evt->eventType == EVENTTYPE_MESSAGE && HIWORD(dat->lastEventType) == EVENTTYPE_MESSAGE &&
			(isSameDate(evt->time, dat->lastEventTime)) && ((((int)evt->time < dat->startTime) == (dat->lastEventTime < dat->startTime)) || !(evt->dwFlags & IEEDF_READ)))
	{
		isGroupBreak = FALSE;
	}
	
	if (!streamData->isFirst && !dat->isMixed) {
		if (isGroupBreak || gdat->flags & SMF_MARKFOLLOWUPS)
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\par");
		else
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\line");
	}

	if (evt->dwFlags & IEEDF_RTL)
		dat->isMixed = 1;

	if (!streamData->isFirst && isGroupBreak && (gdat->flags & SMF_DRAWLINES))
		AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\sl-1\\slmult0\\highlight%d\\cf%d\\fs1  \\par\\sl0", fontOptionsListSize + 4, fontOptionsListSize + 4);

	AppendToBuffer(buffer, bufferEnd, bufferAlloced, (evt->dwFlags & IEEDF_RTL) ? "\\rtlpar" : "\\ltrpar");

	if (evt->eventType == EVENTTYPE_MESSAGE)
		highlight = fontOptionsListSize + 2 + ((evt->dwFlags & IEEDF_SENT) ? 1 : 0);
	else
		highlight = fontOptionsListSize + 1;

	AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\highlight%d\\cf%d", highlight, highlight);
	if (!streamData->isFirst && dat->isMixed) {
		if (isGroupBreak)
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\sl-1 \\par\\sl0");
		else
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\sl-1 \\line\\sl0");
	}
	streamData->isFirst = FALSE;
	if (dat->isMixed) {
		if (evt->dwFlags & IEEDF_RTL)
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\ltrch\\rtlch");
		else
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\rtlch\\ltrch");
	}
	if ((gdat->flags & SMF_SHOWICONS) && isGroupBreak) {
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

		AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\fs1  ");
		while (bufferAlloced - bufferEnd < logIconBmpSize[i])
			bufferAlloced += 1024;
		buffer = (char*)mir_realloc(buffer, bufferAlloced);
		memcpy(buffer + bufferEnd, pLogIconBmpBits[i], logIconBmpSize[i]);
		bufferEnd += logIconBmpSize[i];
		AppendToBuffer(buffer, bufferEnd, bufferAlloced, " ");
	}

	if (gdat->flags & SMF_SHOWTIME && (evt->eventType != EVENTTYPE_MESSAGE ||
		(gdat->flags & SMF_MARKFOLLOWUPS || isGroupBreak || !(gdat->flags & SMF_GROUPMESSAGES)))) {
		TCHAR *timestampString = NULL;
		if (gdat->flags & SMF_GROUPMESSAGES && evt->eventType == EVENTTYPE_MESSAGE) {
			if (isGroupBreak) {
				if (!(gdat->flags & SMF_MARKFOLLOWUPS))
					timestampString = TimestampToString(gdat->flags, evt->time, 0);
				else if (gdat->flags & SMF_SHOWDATE)
					timestampString = TimestampToString(gdat->flags, evt->time, 1);
			}
			else if (gdat->flags & SMF_MARKFOLLOWUPS)
				timestampString = TimestampToString(gdat->flags, evt->time, 2);
		}
		else timestampString = TimestampToString(gdat->flags, evt->time, 0);

		if (timestampString != NULL) {
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, "%s ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME));
			AppendUnicodeToBuffer(buffer, bufferEnd, bufferAlloced, timestampString);
		}
		if (evt->eventType != EVENTTYPE_MESSAGE)
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, "%s: ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON));
		showColon = 1;
	}
	if ((!(gdat->flags & SMF_HIDENAMES) && evt->eventType == EVENTTYPE_MESSAGE && isGroupBreak) || evt->eventType == EVENTTYPE_JABBER_CHATSTATES || evt->eventType == EVENTTYPE_JABBER_PRESENCE) {
		if (evt->eventType == EVENTTYPE_MESSAGE) {
			if (showColon)
				AppendToBuffer(buffer, bufferEnd, bufferAlloced, " %s ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME));
			else
				AppendToBuffer(buffer, bufferEnd, bufferAlloced, "%s ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME));
		}
		else AppendToBuffer(buffer, bufferEnd, bufferAlloced, "%s ", SetToStyle(MSGFONTID_NOTICE));

		if (evt->dwFlags & IEEDF_UNICODE_NICK)
			AppendUnicodeToBuffer(buffer, bufferEnd, bufferAlloced, evt->pszNickW);
		else
			AppendAnsiToBuffer(buffer, bufferEnd, bufferAlloced, evt->pszNick);

		showColon = 1;
		if (evt->eventType == EVENTTYPE_MESSAGE && gdat->flags & SMF_GROUPMESSAGES) {
			if (gdat->flags & SMF_MARKFOLLOWUPS)
				AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\par");
			else
				AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\line");
			showColon = 0;
		}
	}

	if ((gdat->flags & SMF_AFTERMASK) == SMF_AFTERMASK && evt->eventType == EVENTTYPE_MESSAGE && isGroupBreak) {
		AppendToBuffer(buffer, bufferEnd, bufferAlloced, " %s ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME));
		AppendUnicodeToBuffer(buffer, bufferEnd, bufferAlloced, TimestampToString(gdat->flags, evt->time, 2));
		showColon = 1;
	}
	if (showColon && evt->eventType == EVENTTYPE_MESSAGE) {
		if (evt->dwFlags & IEEDF_RTL)
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\~%s: ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON));
		else
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, "%s: ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON));
	}
	switch (evt->eventType) {
	case EVENTTYPE_JABBER_CHATSTATES:
	case EVENTTYPE_JABBER_PRESENCE:
	case EVENTTYPE_URL:
	case EVENTTYPE_FILE:
		style = MSGFONTID_NOTICE;
		AppendToBuffer(buffer, bufferEnd, bufferAlloced, "%s ", SetToStyle(style));
		if (evt->eventType == EVENTTYPE_FILE) {
			if (evt->dwFlags & IEEDF_SENT)
				AppendUnicodeToBuffer(buffer, bufferEnd, bufferAlloced, TranslateT("File sent"));
			else
				AppendUnicodeToBuffer(buffer, bufferEnd, bufferAlloced, TranslateT("File received"));
			AppendUnicodeToBuffer(buffer, bufferEnd, bufferAlloced, _T(":"));
		}
		else if (evt->eventType == EVENTTYPE_URL) {
			if (evt->dwFlags & IEEDF_SENT)
				AppendUnicodeToBuffer(buffer, bufferEnd, bufferAlloced, TranslateT("URL sent"));
			else
				AppendUnicodeToBuffer(buffer, bufferEnd, bufferAlloced, TranslateT("URL received"));
			AppendUnicodeToBuffer(buffer, bufferEnd, bufferAlloced, _T(":"));
		}
		AppendUnicodeToBuffer(buffer, bufferEnd, bufferAlloced, _T(" "));

		if (evt->pszTextW != NULL) {
			if (evt->dwFlags & IEEDF_UNICODE_TEXT)
				AppendUnicodeToBuffer(buffer, bufferEnd, bufferAlloced, evt->pszTextW);
			else
				AppendAnsiToBuffer(buffer, bufferEnd, bufferAlloced, evt->pszText);
		}

		if (evt->pszText2W != NULL) {
			AppendUnicodeToBuffer(buffer, bufferEnd, bufferAlloced, _T(" ("));
			if (evt->dwFlags & IEEDF_UNICODE_TEXT2)
				AppendUnicodeToBuffer(buffer, bufferEnd, bufferAlloced, evt->pszText2W);
			else
				AppendAnsiToBuffer(buffer, bufferEnd, bufferAlloced, evt->pszText2);
			AppendUnicodeToBuffer(buffer, bufferEnd, bufferAlloced, _T(")"));
		}
		break;
	default:
		if (gdat->flags & SMF_MSGONNEWLINE && showColon)
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\line");

		style = evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYMSG : MSGFONTID_YOURMSG;
		AppendWithCustomLinks(evt, style, buffer, bufferEnd, bufferAlloced);
		break;
	}
	if (dat->isMixed)
		AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\par");

	dat->lastEventTime = evt->time;
	dat->lastEventType = MAKELONG(evt->dwFlags, evt->eventType);
	return buffer;
}

static DWORD CALLBACK LogStreamInEvents(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	LogStreamData *dat = (LogStreamData*)dwCookie;

	if (dat->buffer == NULL) {
		dat->bufferOffset = 0;
		switch (dat->stage) {
		case STREAMSTAGE_HEADER:
			dat->buffer = CreateRTFHeader();
			dat->stage = STREAMSTAGE_EVENTS;
			break;
		case STREAMSTAGE_EVENTS:
			if (dat->events != NULL) {
				EventData *evt = dat->events;
				dat->buffer = NULL;
				dat->buffer = CreateRTFFromEvent(dat->dlgDat, evt, dat->gdat, dat);
				dat->events = evt->next;
				freeEvent(evt);
			}
			else if (dat->eventsToInsert) {
				do {
					EventData *evt = getEventFromDB(dat->dlgDat, dat->hContact, dat->hDbEvent);
					dat->buffer = NULL;
					if (evt != NULL) {
						dat->buffer = CreateRTFFromEvent(dat->dlgDat, evt, dat->gdat, dat);
						freeEvent(evt);
					}
					if (dat->buffer)
						dat->hDbEventLast = dat->hDbEvent;
					dat->hDbEvent = db_event_next(dat->hContact, dat->hDbEvent);
					if (--dat->eventsToInsert == 0)
						break;
				}
					while (dat->buffer == NULL && dat->hDbEvent);
			}
			if (dat->buffer)
				break;

			dat->stage = STREAMSTAGE_TAIL;
			//fall through
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
		dat->buffer = NULL;
	}
	return 0;
}

void StreamInTestEvents(HWND hEditWnd, GlobalMessageData *gdat)
{
	SrmmWindowData dat;
	memset(&dat, 0, sizeof(dat));

	LogStreamData streamData = { 0 };
	streamData.isFirst = TRUE;
	streamData.events = GetTestEvents();
	streamData.dlgDat = &dat;
	streamData.gdat = gdat;

	EDITSTREAM stream = { 0 };
	stream.pfnCallback = LogStreamInEvents;
	stream.dwCookie = (DWORD_PTR)&streamData;
	SendMessage(hEditWnd, EM_STREAMIN, SF_RTF, (LPARAM)&stream);
	SendMessage(hEditWnd, EM_HIDESELECTION, FALSE, 0);
}

void StreamInEvents(HWND hwndDlg, MEVENT hDbEventFirst, int count, int fAppend)
{
	FINDTEXTEXA fi;
	EDITSTREAM stream = { 0 };
	LogStreamData streamData = { 0 };
	SrmmWindowData *dat = (SrmmWindowData *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	CHARRANGE oldSel, sel;

	// IEVIew MOD Begin
	if (dat->hwndLog != NULL) {
		IEVIEWEVENT evt;
		IEVIEWWINDOW ieWindow;
		memset(&evt, 0, sizeof(evt));
		evt.cbSize = sizeof(evt);
		evt.dwFlags = ((dat->flags & SMF_RTL) ? IEEF_RTL : 0);
		evt.hwnd = dat->hwndLog;
		evt.hContact = dat->hContact;
		evt.codepage = dat->codePage;
		evt.pszProto = dat->szProto;
		if (!fAppend) {
			evt.iType = IEE_CLEAR_LOG;
			CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&evt);
		}
		evt.iType = IEE_LOG_DB_EVENTS;
		evt.hDbEventFirst = hDbEventFirst;
		evt.count = count;
		CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&evt);
		dat->hDbEventLast = evt.hDbEventFirst != NULL ? evt.hDbEventFirst : dat->hDbEventLast;

		memset(&ieWindow, 0, sizeof(ieWindow));
		ieWindow.cbSize = sizeof(ieWindow);
		ieWindow.iType = IEW_SCROLLBOTTOM;
		ieWindow.hwnd = dat->hwndLog;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
		return;
	}

	// IEVIew MOD End
	SendDlgItemMessage(hwndDlg, IDC_LOG, EM_HIDESELECTION, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDC_LOG, EM_EXGETSEL, 0, (LPARAM)&oldSel);
	streamData.hContact = dat->hContact;
	streamData.hDbEvent = hDbEventFirst;
	streamData.hDbEventLast = dat->hDbEventLast;
	streamData.dlgDat = dat;
	streamData.eventsToInsert = count;
	streamData.isFirst = fAppend ? GetRichTextLength(GetDlgItem(hwndDlg, IDC_LOG), dat->codePage, FALSE) == 0 : 1;
	streamData.gdat = &g_dat;
	stream.pfnCallback = LogStreamInEvents;
	stream.dwCookie = (DWORD_PTR)& streamData;
	sel.cpMin = 0;
	if (fAppend) {
		GETTEXTLENGTHEX gtxl = { 0 };
		gtxl.flags = GTL_DEFAULT | GTL_PRECISE | GTL_NUMCHARS;
		gtxl.codepage = 1200;
		fi.chrg.cpMin = SendDlgItemMessage(hwndDlg, IDC_LOG, EM_GETTEXTLENGTHEX, (WPARAM)&gtxl, 0);
		sel.cpMin = sel.cpMax = GetRichTextLength(GetDlgItem(hwndDlg, IDC_LOG), dat->codePage, FALSE);
		SendDlgItemMessage(hwndDlg, IDC_LOG, EM_EXSETSEL, 0, (LPARAM)&sel);
	}
	else {
		SendDlgItemMessage(hwndDlg, IDC_LOG, WM_SETREDRAW, FALSE, 0);
		SetDlgItemText(hwndDlg, IDC_LOG, _T(""));
		sel.cpMin = 0;
		sel.cpMax = GetRichTextLength(GetDlgItem(hwndDlg, IDC_LOG), dat->codePage, FALSE);
		SendDlgItemMessage(hwndDlg, IDC_LOG, EM_EXSETSEL, 0, (LPARAM)&sel);
		fi.chrg.cpMin = 0;
		dat->isMixed = 0;
	}

	SendDlgItemMessage(hwndDlg, IDC_LOG, EM_STREAMIN, fAppend ? SFF_SELECTION | SF_RTF : SFF_SELECTION | SF_RTF, (LPARAM)&stream);
	SendDlgItemMessage(hwndDlg, IDC_LOG, EM_EXSETSEL, 0, (LPARAM)&oldSel);
	SendDlgItemMessage(hwndDlg, IDC_LOG, EM_HIDESELECTION, FALSE, 0);
	if (g_dat.smileyAddInstalled) {
		SMADD_RICHEDIT3 smre;
		smre.cbSize = sizeof(SMADD_RICHEDIT3);
		smre.hwndRichEditControl = GetDlgItem(hwndDlg, IDC_LOG);
		
		MCONTACT hContact = db_mc_getSrmmSub(dat->hContact);
		smre.Protocolname = (hContact != NULL) ? GetContactProto(hContact) : dat->szProto;
		
		if (fi.chrg.cpMin > 0) {
			sel.cpMin = fi.chrg.cpMin;
			sel.cpMax = -1;
			smre.rangeToReplace = &sel;
		}
		else smre.rangeToReplace = NULL;

		smre.disableRedraw = TRUE;
		smre.hContact = dat->hContact;
		smre.flags = 0;
		CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM)&smre);
	}

	int len = GetRichTextLength(GetDlgItem(hwndDlg, IDC_LOG), dat->codePage, FALSE);
	SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETSEL, len - 1, len - 1);

	if (!fAppend)
		SendDlgItemMessage(hwndDlg, IDC_LOG, WM_SETREDRAW, TRUE, 0);

	dat->hDbEventLast = streamData.hDbEventLast;
	PostMessage(hwndDlg, DM_SCROLLLOGTOBOTTOM, 0, 0);
}

#define RTFPICTHEADERMAXSIZE   78

void LoadMsgLogIcons(void)
{
	HICON hIcon = NULL;
	RECT rc;

	g_hImageList = ImageList_Create(10, 10, ILC_COLOR32 | ILC_MASK, SIZEOF(pLogIconBmpBits), 0);
	HBRUSH hBkgBrush = CreateSolidBrush(db_get_dw(NULL, SRMMMOD, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR));
	HBRUSH hInBkgBrush = CreateSolidBrush(db_get_dw(NULL, SRMMMOD, SRMSGSET_INCOMINGBKGCOLOUR, SRMSGDEFSET_INCOMINGBKGCOLOUR));
	HBRUSH hOutBkgBrush = CreateSolidBrush(db_get_dw(NULL, SRMMMOD, SRMSGSET_OUTGOINGBKGCOLOUR, SRMSGDEFSET_OUTGOINGBKGCOLOUR));

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
	HDC hdc = GetDC(NULL);
	HBITMAP hBmp = CreateCompatibleBitmap(hdc, bih.biWidth, bih.biHeight);
	HDC hdcMem = CreateCompatibleDC(hdc);
	PBYTE pBmpBits = (PBYTE)mir_alloc(widthBytes * bih.biHeight);
	HBRUSH hBrush = hBkgBrush;
	for (int i = 0; i < SIZEOF(pLogIconBmpBits); i++) {
		switch (i) {
		case LOGICON_MSG_IN:
			ImageList_AddIcon(g_hImageList, GetCachedIcon("scriver_INCOMING"));
			hIcon = ImageList_GetIcon(g_hImageList, LOGICON_MSG_IN, ILD_NORMAL);
			hBrush = hInBkgBrush;
			break;
		case LOGICON_MSG_OUT:
			ImageList_AddIcon(g_hImageList, GetCachedIcon("scriver_OUTGOING"));
			hIcon = ImageList_GetIcon(g_hImageList, LOGICON_MSG_OUT, ILD_NORMAL);
			hBrush = hOutBkgBrush;
			break;
		case LOGICON_MSG_NOTICE:
			ImageList_AddIcon(g_hImageList, GetCachedIcon("scriver_NOTICE"));
			hIcon = ImageList_GetIcon(g_hImageList, LOGICON_MSG_NOTICE, ILD_NORMAL);
			hBrush = hBkgBrush;
			break;
		}
		
		pLogIconBmpBits[i] = (char*)mir_alloc(RTFPICTHEADERMAXSIZE + (bih.biSize + widthBytes * bih.biHeight) * 2);
		size_t rtfHeaderSize = sprintf(pLogIconBmpBits[i], "{\\pict\\dibitmap0\\wbmbitspixel%u\\wbmplanes1\\wbmwidthbytes%u\\picw%u\\pich%u ", bih.biBitCount, widthBytes, (UINT)bih.biWidth, (UINT)bih.biHeight); //!!!!!!!!!!!

		HBITMAP hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
		FillRect(hdcMem, &rc, hBrush);
		DrawIconEx(hdcMem, 0, 0, hIcon, bih.biWidth, bih.biHeight, 0, NULL, DI_NORMAL);
		SelectObject(hdcMem, hoBmp);
		GetDIBits(hdc, hBmp, 0, bih.biHeight, pBmpBits, (BITMAPINFO *)& bih, DIB_RGB_COLORS);
		DestroyIcon(hIcon);

		char *szDest = pLogIconBmpBits[i] + rtfHeaderSize;
		bin2hex(&bih, sizeof(bih), szDest); szDest += sizeof(bih) * 2;
		bin2hex(pBmpBits, widthBytes * bih.biHeight, szDest); szDest += widthBytes * bih.biHeight * 2;
		mir_strcpy(szDest, "}");

		logIconBmpSize[i] = size_t(szDest - pLogIconBmpBits[i]) + 1;
	}
	mir_free(pBmpBits);
	DeleteDC(hdcMem);
	DeleteObject(hBmp);
	ReleaseDC(NULL, hdc);
	DeleteObject(hBkgBrush);
	DeleteObject(hInBkgBrush);
	DeleteObject(hOutBkgBrush);
}

void FreeMsgLogIcons(void)
{
	for (int i = 0; i < SIZEOF(pLogIconBmpBits); i++)
		mir_free(pLogIconBmpBits[i]);

	ImageList_RemoveAll(g_hImageList);
	ImageList_Destroy(g_hImageList);
}
