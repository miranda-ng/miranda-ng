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

#define MIRANDA_0_5

#define LOGICON_MSG_IN      0
#define LOGICON_MSG_OUT     1
#define LOGICON_MSG_NOTICE  2

extern int RTL_Detect(WCHAR *pszwText);
static int logPixelSY;
static PBYTE pLogIconBmpBits[3];
static int logIconBmpSize[SIZEOF(pLogIconBmpBits)];
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
		char *pszNick;		// Nick, usage depends on type of event
		wchar_t *pszNickW;    // Nick - Unicode
		TCHAR *pszNickT;
	};
	union {
		char *pszText;			// Text, usage depends on type of event
		wchar_t *pszTextW;			// Text - Unicode
		TCHAR *pszTextT;
	};
	union {
		char *pszText2;			// Text, usage depends on type of event
		wchar_t *pszText2W;			// Text - Unicode
		TCHAR *pszText2T;
	};
	DWORD	time;
	DWORD	eventType;
	int   codePage;
	BOOL  custom;
	EventData *next;
};

struct LogStreamData
{
	int stage;
	MCONTACT hContact;
	HANDLE hDbEvent, hDbEventLast;
	char *buffer;
	int bufferOffset, bufferLen;
	int eventsToInsert;
	int isFirst;
	SrmmWindowData *dlgDat;
	GlobalMessageData *gdat;
	EventData *events;
};

TCHAR *GetNickname(MCONTACT hContact, const char* szProto)
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
					if (!_tcscmp((TCHAR *)ci.pszVal, TranslateW(_T("'(Unknown Contact)'")))) {
						ci.dwFlag &= ~CNF_UNICODE;
						if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci))
							szName = mir_a2t((char*)ci.pszVal);
					}
					else szName = mir_tstrdup((TCHAR *)ci.pszVal);
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

	return mir_tstrdup(TranslateT("Unknown Contact"));
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

int DbEventIsShown(DBEVENTINFO * dbei, SrmmWindowData *dat)
{
	switch (dbei->eventType) {
	case EVENTTYPE_MESSAGE:
		return 1;

	case EVENTTYPE_JABBER_CHATSTATES:
	case EVENTTYPE_JABBER_PRESENCE:
		return 0;

	case EVENTTYPE_FILE:
	case EVENTTYPE_URL:
		return 1;
	}

	return DbEventIsCustomForMsgWindow(dbei);
}

EventData* getEventFromDB(SrmmWindowData *dat, MCONTACT hContact, HANDLE hDbEvent)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.cbBlob = db_event_getBlobSize(hDbEvent);
	if (dbei.cbBlob == -1)
		return NULL;
	dbei.pBlob = (PBYTE)mir_alloc(dbei.cbBlob);
	db_event_get(hDbEvent, &dbei);
	if (!DbEventIsShown(&dbei, dat)) {
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
	evt->codePage = dat->windowData.codePage;

	if (evt->dwFlags & IEEDF_SENT)
		evt->pszNickT = GetNickname(NULL, dat->szProto);
	else
		evt->pszNickT = GetNickname(hContact, dat->szProto);

	if (evt->eventType == EVENTTYPE_FILE) {
		char *filename = ((char*)dbei.pBlob) + sizeof(DWORD);
		char *descr = filename + lstrlenA(filename) + 1;
		evt->pszTextT = DbGetEventStringT(&dbei, filename);
		if (*descr != 0)
			evt->pszText2T = DbGetEventStringT(&dbei, descr);
	}
	else evt->pszTextT = DbGetEventTextT(&dbei, dat->windowData.codePage);

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
	evt->pszNickT = mir_tstrdup(LPGENT("Me"));
	evt->pszTextT = mir_tstrdup(LPGENT("O Lord, bless this Thy hand grenade that with it Thou mayest blow Thine enemies"));

	evt = GetTestEvent(IEEDF_SENT);
	evt->pszNickT = mir_tstrdup(LPGENT("Me"));
	evt->pszTextT = mir_tstrdup(LPGENT("to tiny bits, in Thy mercy"));
	prevEvent->next = evt;
	prevEvent = evt;

	evt = GetTestEvent(0);
	evt->pszNickT = mir_tstrdup(LPGENT("My Contact"));
	evt->pszTextT = mir_tstrdup(LPGENT("Lorem ipsum dolor sit amet,"));
	prevEvent->next = evt;
	prevEvent = evt;

	evt = GetTestEvent(0);
	evt->pszNickT = mir_tstrdup(LPGENT("My Contact"));
	evt->pszTextT = mir_tstrdup(LPGENT("consectetur adipisicing elit"));
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

static int AppendUnicodeOrAnsiiToBufferL(char **buffer, int *cbBufferEnd, int *cbBufferAlloced, WCHAR * line, int maxLen, BOOL isAnsii)
{
	int textCharsCount = 0;
	int wasEOL = 0;
	WCHAR *maxLine = line + maxLen;
	int lineLen = (int)wcslen(line) * 9 + 8;
	if (*cbBufferEnd + lineLen > *cbBufferAlloced) {
		cbBufferAlloced[0] += (lineLen + 1024 - lineLen % 1024);
		*buffer = (char*)mir_realloc(*buffer, *cbBufferAlloced);
	}

	char *d = *buffer + *cbBufferEnd;
	if (isAnsii) {
		strcpy(d, "{");
		d++;
	}
	else {
		strcpy(d, "{\\uc1 ");
		d += 6;
	}

	for (; *line && (maxLen < 0 || line < maxLine); line++, textCharsCount++) {
		wasEOL = 0;
		if (*line == '\r' && line[1] == '\n') {
			CopyMemory(d, "\\line ", 6);
			wasEOL = 1;
			d += 6;
			line++;
		}
		else if (*line == '\n') {
			CopyMemory(d, "\\line ", 6);
			wasEOL = 1;
			d += 6;
		}
		else if (*line == '\t') {
			CopyMemory(d, "\\tab ", 5);
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
			d += sprintf(d, "\\'%02x", (*line) & 0xFF); //!!!!!!!!!!
		}
		else {
			d += sprintf(d, "\\u%d ?", *line); //!!!!!!!!!!
		}
	}
	if (wasEOL) {
		CopyMemory(d, " ", 1);
		d++;
	}
	strcpy(d, "}");
	d++;

	*cbBufferEnd = (int)(d - *buffer);
	return textCharsCount;
}

static int AppendAnsiToBufferL(char **buffer, int *cbBufferEnd, int *cbBufferAlloced, const char * line, int maxLen)
{
	WCHAR *wline = a2w(line, maxLen);
	int i = AppendUnicodeOrAnsiiToBufferL(buffer, cbBufferEnd, cbBufferAlloced, wline, maxLen, TRUE);
	mir_free(wline);
	return i;
}

static int AppendUnicodeToBufferL(char **buffer, int *cbBufferEnd, int *cbBufferAlloced, WCHAR * line, int maxLen)
{
	return AppendUnicodeOrAnsiiToBufferL(buffer, cbBufferEnd, cbBufferAlloced, line, maxLen, FALSE);
}

static int AppendAnsiToBuffer(char **buffer, int *cbBufferEnd, int *cbBufferAlloced, const char * line)
{
	return AppendAnsiToBufferL(buffer, cbBufferEnd, cbBufferAlloced, line, -1);
}

static int AppendUnicodeToBuffer(char **buffer, int *cbBufferEnd, int *cbBufferAlloced, WCHAR * line)
{
	return AppendUnicodeToBufferL(buffer, cbBufferEnd, cbBufferAlloced, line, -1);
}

static int AppendTToBuffer(char **buffer, int *cbBufferEnd, int *cbBufferAlloced, TCHAR * line)
{
	return AppendUnicodeToBuffer(buffer, cbBufferEnd, cbBufferAlloced, line);
}

//mir_free() the return value
static char *CreateRTFHeader(SrmmWindowData *dat, struct GlobalMessageData *gdat)
{
	char *buffer;
	int bufferAlloced, bufferEnd;
	int i;
	LOGFONT lf;
	COLORREF colour;
	HDC hdc;
	int charset = 0;
	BOOL forceCharset = FALSE;

	hdc = GetDC(NULL);
	logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(NULL, hdc);
	bufferEnd = 0;
	bufferAlloced = 1024;
	buffer = (char*) mir_alloc(bufferAlloced);
	buffer[0] = '\0';
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced,"{\\rtf1\\ansi\\deff0{\\fonttbl");
	for (i = 0; i < fontOptionsListSize; i++) {
		LoadMsgDlgFont(i, &lf, NULL);
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "{\\f%u\\fnil\\fcharset%u %S;}", i,
			(!forceCharset) ? lf.lfCharSet : charset, lf.lfFaceName);
	}
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "}{\\colortbl ");
	for (i = 0; i < fontOptionsListSize; i++) {
		LoadMsgDlgFont(i, NULL, &colour);
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	}
	if (GetSysColorBrush(COLOR_HOTLIGHT) == NULL)
		colour = RGB(0, 0, 255);
	else
		colour = GetSysColor(COLOR_HOTLIGHT);
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = db_get_dw(NULL, SRMMMOD, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR);
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = db_get_dw(NULL, SRMMMOD, SRMSGSET_INCOMINGBKGCOLOUR, SRMSGDEFSET_INCOMINGBKGCOLOUR);
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = db_get_dw(NULL, SRMMMOD, SRMSGSET_OUTGOINGBKGCOLOUR, SRMSGDEFSET_OUTGOINGBKGCOLOUR);
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = db_get_dw(NULL, SRMMMOD, SRMSGSET_LINECOLOUR, SRMSGDEFSET_LINECOLOUR);
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "}");
	return buffer;
}

//mir_free() the return value
static char* CreateRTFTail()
{
	int bufferAlloced = 1024, bufferEnd = 0;
	char *buffer = (char*)mir_alloc(bufferAlloced);
	buffer[0] = '\0';
	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "}");
	return buffer;
}

//return value is static
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
			lstrcpy(szResult, TranslateT("Today"));
			if (mode == 0)
				lstrcat(szResult, _T(","));
		}
		else if (dwFlags & SMF_RELATIVEDATE && check > (today - 86400)) {
			lstrcpy(szResult, TranslateT("Yesterday"));
			if (mode == 0)
				lstrcat(szResult, _T(","));
		}
		else {
			if (dwFlags & SMF_LONGDATE)
				lstrcpy(format, _T("D"));
			else
				lstrcpy(format, _T("d"));
		}
	}
	if (mode == 0 || mode == 2) {
		if (mode == 0 && (dwFlags & SMF_SHOWDATE))
			lstrcat(format, _T(" "));

		lstrcat(format, (dwFlags & SMF_SHOWSECONDS) ? _T("s") : _T("t"));
	}
	if (format[0] != '\0') {
		tmi.printTimeStamp(NULL, check, format, str, SIZEOF(str), 0);
		_tcsncat(szResult, str, 500);
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

static void AppendWithCustomLinks(EventData *evt, int style, char **buffer, int *bufferEnd, int *bufferAlloced)
{
	int lasttoken = 0;
	int laststart = 0;
	int j, len;
	WCHAR *wText;
	BOOL isAnsii = (evt->dwFlags & IEEDF_UNICODE_TEXT) == 0;

	if (evt->pszText == NULL)
		return;

	if (isAnsii) {
		len = (int)strlen(evt->pszText);
		wText = a2w(evt->pszText, len);
	}
	else {
		wText = evt->pszTextW;
		len = (int)wcslen(evt->pszTextW);
	}
	for (j = 0; j < len; j++) {
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
static char* CreateRTFFromEvent(SrmmWindowData *dat, EventData *evt, struct GlobalMessageData *gdat, struct LogStreamData *streamData)
{
	char *buffer;
	int bufferAlloced, bufferEnd;
	int style, showColon = 0;
	int isGroupBreak = TRUE;
	int highlight = 0;
	bufferEnd = 0;
	bufferAlloced = 1024;
	buffer = (char*) mir_alloc(bufferAlloced);
	buffer[0] = '\0';

 	if ((gdat->flags & SMF_GROUPMESSAGES) && evt->dwFlags == LOWORD(dat->lastEventType) &&
			evt->eventType == EVENTTYPE_MESSAGE && HIWORD(dat->lastEventType) == EVENTTYPE_MESSAGE &&
			(isSameDate(evt->time, dat->lastEventTime)) && ((((int)evt->time < dat->startTime) == (dat->lastEventTime < dat->startTime)) || !(evt->dwFlags & IEEDF_READ)))
	{
		isGroupBreak = FALSE;
	}
	
	if (!streamData->isFirst && !dat->isMixed) {
		if (isGroupBreak || gdat->flags & SMF_MARKFOLLOWUPS)
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\par");
		else
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\line");
	}

	if (evt->dwFlags & IEEDF_RTL)
		dat->isMixed = 1;

	if (!streamData->isFirst && isGroupBreak && (gdat->flags & SMF_DRAWLINES))
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\sl-1\\slmult0\\highlight%d\\cf%d\\fs1  \\par\\sl0", fontOptionsListSize + 4, fontOptionsListSize + 4);

	if (streamData->isFirst) {
		if (evt->dwFlags & IEEDF_RTL)
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\rtlpar");
		else
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\ltrpar");
	}
	else {
		if (evt->dwFlags & IEEDF_RTL)
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\rtlpar");
		else
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\ltrpar");
	}
	if (evt->eventType == EVENTTYPE_MESSAGE)
		highlight = fontOptionsListSize + 2 + ((evt->dwFlags & IEEDF_SENT) ? 1 : 0);
	else
		highlight = fontOptionsListSize + 1;

	AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\highlight%d\\cf%d", highlight, highlight);
	if (!streamData->isFirst && dat->isMixed) {
		if (isGroupBreak)
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\sl-1 \\par\\sl0");
		else
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\sl-1 \\line\\sl0");
	}
	streamData->isFirst = FALSE;
	if (dat->isMixed) {
		if (evt->dwFlags & IEEDF_RTL)
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\ltrch\\rtlch");
		else
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\rtlch\\ltrch");
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

		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\fs1  ");
		while (bufferAlloced - bufferEnd < logIconBmpSize[i])
			bufferAlloced += 1024;
		buffer = (char*)mir_realloc(buffer, bufferAlloced);
		CopyMemory(buffer + bufferEnd, pLogIconBmpBits[i], logIconBmpSize[i]);
		bufferEnd += logIconBmpSize[i];
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, " ");
	}

	if (gdat->flags&SMF_SHOWTIME && (evt->eventType != EVENTTYPE_MESSAGE ||
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
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "%s ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME));
			AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, timestampString);
		}
		if (evt->eventType != EVENTTYPE_MESSAGE)
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "%s: ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON));
		showColon = 1;
	}
	if ((!(gdat->flags & SMF_HIDENAMES) && evt->eventType == EVENTTYPE_MESSAGE && isGroupBreak) || evt->eventType == EVENTTYPE_JABBER_CHATSTATES || evt->eventType == EVENTTYPE_JABBER_PRESENCE) {
		if (evt->eventType == EVENTTYPE_MESSAGE) {
			if (showColon)
				AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, " %s ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME));
			else
				AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "%s ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME));
		}
		else AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "%s ", SetToStyle(MSGFONTID_NOTICE));

		if (evt->dwFlags & IEEDF_UNICODE_NICK)
			AppendUnicodeToBuffer(&buffer, &bufferEnd, &bufferAlloced, evt->pszNickW);
		else
			AppendAnsiToBuffer(&buffer, &bufferEnd, &bufferAlloced, evt->pszNick);

		showColon = 1;
		if (evt->eventType == EVENTTYPE_MESSAGE && gdat->flags & SMF_GROUPMESSAGES) {
			if (gdat->flags & SMF_MARKFOLLOWUPS)
				AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\par");
			else
				AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\line");
			showColon = 0;
		}
	}

	if (gdat->flags&SMF_SHOWTIME && gdat->flags & SMF_GROUPMESSAGES && gdat->flags & SMF_MARKFOLLOWUPS && evt->eventType == EVENTTYPE_MESSAGE && isGroupBreak) {
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, " %s ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME));
		AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, TimestampToString(gdat->flags, evt->time, 2));
		showColon = 1;
	}
	if (showColon && evt->eventType == EVENTTYPE_MESSAGE) {
		if (evt->dwFlags & IEEDF_RTL)
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\~%s: ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON));
		else
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "%s: ", SetToStyle(evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON));
	}
	switch (evt->eventType) {
	case EVENTTYPE_JABBER_CHATSTATES:
	case EVENTTYPE_JABBER_PRESENCE:
	case EVENTTYPE_URL:
	case EVENTTYPE_FILE:
		style = MSGFONTID_NOTICE;
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "%s ", SetToStyle(style));
		if (evt->eventType == EVENTTYPE_FILE) {
			if (evt->dwFlags & IEEDF_SENT)
				AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, TranslateT("File sent"));
			else
				AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, TranslateT("File received"));
			AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, _T(":"));
		}
		else if (evt->eventType == EVENTTYPE_URL) {
			if (evt->dwFlags & IEEDF_SENT)
				AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, TranslateT("URL sent"));
			else
				AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, TranslateT("URL received"));
			AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, _T(":"));
		}
		AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, _T(" "));

		if (evt->pszTextW != NULL) {
			if (evt->dwFlags & IEEDF_UNICODE_TEXT)
				AppendUnicodeToBuffer(&buffer, &bufferEnd, &bufferAlloced, evt->pszTextW);
			else
				AppendAnsiToBuffer(&buffer, &bufferEnd, &bufferAlloced, evt->pszText);
		}

		if (evt->pszText2W != NULL) {
			AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, _T(" ("));
			if (evt->dwFlags & IEEDF_UNICODE_TEXT2)
				AppendUnicodeToBuffer(&buffer, &bufferEnd, &bufferAlloced, evt->pszText2W);
			else
				AppendAnsiToBuffer(&buffer, &bufferEnd, &bufferAlloced, evt->pszText2);
			AppendTToBuffer(&buffer, &bufferEnd, &bufferAlloced, _T(")"));
		}
		break;
	default:
		if (gdat->flags & SMF_MSGONNEWLINE && showColon)
			AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\line");

		style = evt->dwFlags & IEEDF_SENT ? MSGFONTID_MYMSG : MSGFONTID_YOURMSG;
		AppendWithCustomLinks(evt, style, &buffer, &bufferEnd, &bufferAlloced);
		break;
	}
	if (dat->isMixed)
		AppendToBuffer(&buffer, &bufferEnd, &bufferAlloced, "\\par");

	dat->lastEventTime = evt->time;
	dat->lastEventType = MAKELONG(evt->dwFlags, evt->eventType);
	return buffer;
}

static DWORD CALLBACK LogStreamInEvents(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	struct LogStreamData *dat = (struct LogStreamData *)dwCookie;

	if (dat->buffer == NULL) {
		dat->bufferOffset = 0;
		switch (dat->stage) {
		case STREAMSTAGE_HEADER:
			dat->buffer = CreateRTFHeader(dat->dlgDat, dat->gdat);
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
		dat->bufferLen = lstrlenA(dat->buffer);
	}
	*pcb = min(cb, dat->bufferLen - dat->bufferOffset);
	CopyMemory(pbBuff, dat->buffer + dat->bufferOffset, *pcb);
	dat->bufferOffset += *pcb;
	if (dat->bufferOffset == dat->bufferLen) {
		mir_free(dat->buffer);
		dat->buffer = NULL;
	}
	return 0;
}

void StreamInTestEvents(HWND hEditWnd, struct GlobalMessageData *gdat)
{
	SrmmWindowData dat = { 0 };
	struct LogStreamData streamData = { 0 };
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

void StreamInEvents(HWND hwndDlg, HANDLE hDbEventFirst, int count, int fAppend)
{
	FINDTEXTEXA fi;
	EDITSTREAM stream = { 0 };
	struct LogStreamData streamData = { 0 };
	SrmmWindowData *dat = (SrmmWindowData *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	CHARRANGE oldSel, sel;

	// IEVIew MOD Begin
	if (dat->windowData.hwndLog != NULL) {
		IEVIEWEVENT evt;
		IEVIEWWINDOW ieWindow;
		ZeroMemory(&evt, sizeof(evt));
		evt.cbSize = sizeof(evt);
		evt.dwFlags = ((dat->flags & SMF_RTL) ? IEEF_RTL : 0);
		evt.hwnd = dat->windowData.hwndLog;
		evt.hContact = dat->windowData.hContact;
		evt.codepage = dat->windowData.codePage;
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

		ZeroMemory(&ieWindow, sizeof(ieWindow));
		ieWindow.cbSize = sizeof(ieWindow);
		ieWindow.iType = IEW_SCROLLBOTTOM;
		ieWindow.hwnd = dat->windowData.hwndLog;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
		return;
	}

	// IEVIew MOD End
	SendDlgItemMessage(hwndDlg, IDC_LOG, EM_HIDESELECTION, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDC_LOG, EM_EXGETSEL, 0, (LPARAM)&oldSel);
	streamData.hContact = dat->windowData.hContact;
	streamData.hDbEvent = hDbEventFirst;
	streamData.hDbEventLast = dat->hDbEventLast;
	streamData.dlgDat = dat;
	streamData.eventsToInsert = count;
	streamData.isFirst = fAppend ? GetRichTextLength(GetDlgItem(hwndDlg, IDC_LOG), dat->windowData.codePage, FALSE) == 0 : 1;
	streamData.gdat = &g_dat;
	stream.pfnCallback = LogStreamInEvents;
	stream.dwCookie = (DWORD_PTR)& streamData;
	sel.cpMin = 0;
	if (fAppend) {
		GETTEXTLENGTHEX gtxl = { 0 };
		gtxl.flags = GTL_DEFAULT | GTL_PRECISE | GTL_NUMCHARS;
		gtxl.codepage = 1200;
		gtxl.codepage = 1200;
		fi.chrg.cpMin = SendDlgItemMessage(hwndDlg, IDC_LOG, EM_GETTEXTLENGTHEX, (WPARAM)&gtxl, 0);
		sel.cpMin = sel.cpMax = GetRichTextLength(GetDlgItem(hwndDlg, IDC_LOG), dat->windowData.codePage, FALSE);
		SendDlgItemMessage(hwndDlg, IDC_LOG, EM_EXSETSEL, 0, (LPARAM)&sel);
	}
	else {
		SendDlgItemMessage(hwndDlg, IDC_LOG, WM_SETREDRAW, FALSE, 0);
		SetDlgItemText(hwndDlg, IDC_LOG, _T(""));
		sel.cpMin = 0;
		sel.cpMax = GetRichTextLength(GetDlgItem(hwndDlg, IDC_LOG), dat->windowData.codePage, FALSE);
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
		smre.Protocolname = dat->szProto;
		if (dat->szProto != NULL && strcmp(dat->szProto, META_PROTO) == 0) {
			MCONTACT hContact = db_mc_getMostOnline(dat->windowData.hContact);
			if (hContact != NULL)
				smre.Protocolname = GetContactProto(hContact);
		}
		if (fi.chrg.cpMin > 0) {
			sel.cpMin = fi.chrg.cpMin;
			sel.cpMax = -1;
			smre.rangeToReplace = &sel;
		}
		else smre.rangeToReplace = NULL;

		smre.disableRedraw = TRUE;
		smre.hContact = dat->windowData.hContact;
		smre.flags = 0;
		CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM)&smre);
	}

	int len = GetRichTextLength(GetDlgItem(hwndDlg, IDC_LOG), dat->windowData.codePage, FALSE);
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
		pLogIconBmpBits[i] = (PBYTE)mir_alloc(RTFPICTHEADERMAXSIZE + (bih.biSize + widthBytes * bih.biHeight) * 2);
		//I can't seem to get binary mode working. No matter.
		int rtfHeaderSize = sprintf((char*)pLogIconBmpBits[i], "{\\pict\\dibitmap0\\wbmbitspixel%u\\wbmplanes1\\wbmwidthbytes%u\\picw%u\\pich%u ", bih.biBitCount, widthBytes, (UINT)bih.biWidth, (UINT)bih.biHeight); //!!!!!!!!!!!
		HBITMAP hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
		FillRect(hdcMem, &rc, hBrush);
		DrawIconEx(hdcMem, 0, 0, hIcon, bih.biWidth, bih.biHeight, 0, NULL, DI_NORMAL);
		SelectObject(hdcMem, hoBmp);
		GetDIBits(hdc, hBmp, 0, bih.biHeight, pBmpBits, (BITMAPINFO *)& bih, DIB_RGB_COLORS);
		DestroyIcon(hIcon);

		int n;
		for (n = 0; n < sizeof(BITMAPINFOHEADER); n++)
			sprintf((char*)pLogIconBmpBits[i] + rtfHeaderSize + n * 2, "%02X", ((PBYTE)& bih)[n]); //!!!!!!!!!!!!!!
		for (n = 0; n < widthBytes * bih.biHeight; n += 4)
			sprintf((char*)pLogIconBmpBits[i] + rtfHeaderSize + (bih.biSize + n) * 2, "%02X%02X%02X%02X", pBmpBits[n], pBmpBits[n + 1], pBmpBits[n + 2], pBmpBits[n + 3]); //!!!!!!!!!!!!!!!!

		logIconBmpSize[i] = rtfHeaderSize + (bih.biSize + widthBytes * bih.biHeight) * 2 + 1;
		pLogIconBmpBits[i][logIconBmpSize[i] - 1] = '}';
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
