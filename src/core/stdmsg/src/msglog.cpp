/*

Copyright 2000-12 Miranda IM, 2012-15 Miranda NG project,
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

extern IconItem iconList[];

#define LOGICON_MSG_IN      0
#define LOGICON_MSG_OUT     1
#define LOGICON_MSG_NOTICE  2

static char *pLogIconBmpBits[3];
static size_t logIconBmpSize[ SIZEOF(pLogIconBmpBits) ];

#define STREAMSTAGE_HEADER  0
#define STREAMSTAGE_EVENTS  1
#define STREAMSTAGE_TAIL    2
#define STREAMSTAGE_STOP    3

struct LogStreamData
{
	int stage;
	MCONTACT hContact;
	MEVENT hDbEvent, hDbEventLast;
	char *buffer;
	int bufferOffset, bufferLen;
	int eventsToInsert;
	int isEmpty;
	SrmmWindowData *dlgDat;
};

static int logPixelSY;
static char szSep2[40], szSep2_RTL[50];

static void AppendToBuffer(char *&buffer, size_t &cbBufferEnd, size_t &cbBufferAlloced, const char *fmt, ...)
{
	va_list va;
	int charsDone;

	va_start(va, fmt);
	for (;;) {
		charsDone = mir_vsnprintf(buffer + cbBufferEnd, cbBufferAlloced - cbBufferEnd, fmt, va);
		if (charsDone >= 0)
			break;
		cbBufferAlloced += 1024;
		buffer = (char*)mir_realloc(buffer, cbBufferAlloced);
	}
	va_end(va);
	cbBufferEnd += charsDone;
}

static const TCHAR *bbcodes[] = { _T("[b]"), _T("[i]"), _T("[u]"), _T("[s]"), _T("[/b]"), _T("[/i]"), _T("[/u]"), _T("[/s]") };
static const char *bbcodefmt[] = { "\\b ", "\\i ", "\\ul ", "\\strike ", "\\b0 ", "\\i0 ", "\\ul0 ", "\\strike0 " };

static int AppendToBufferWithRTF(char *&buffer, size_t &cbBufferEnd, size_t &cbBufferAlloced, TCHAR* line)
{
	DWORD textCharsCount = 0;
	char *d;

	if (line == NULL)
		return 0;

	size_t lineLen = _tcslen(line) * 9 + 8;
	if (cbBufferEnd + lineLen > cbBufferAlloced) {
		cbBufferAlloced += lineLen + 1024 - lineLen % 1024;
		buffer = (char*)mir_realloc(buffer, cbBufferAlloced);
	}

	d = buffer + cbBufferEnd;
	strcpy(d, "{\\uc1 ");
	d += 6;

	for (; *line; line++, textCharsCount++) {
		if (*line == '\r' && line[1] == '\n') {
			memcpy(d, "\\par ", 5);
			line++;
			d += 5;
		}
		else if (*line == '\n') {
			memcpy(d, "\\par ", 5);
			d += 5;
		}
		else if (*line == '\t') {
			memcpy(d, "\\tab ", 5);
			d += 5;
		}
		else if (*line == '\\' || *line == '{' || *line == '}') {
			*d++ = '\\';
			*d++ = (char)*line;
		}
		else if (*line == '[' && (g_dat.flags & SMF_SHOWFORMAT)) {
			int i, found = 0;
			for (i = 0; i < SIZEOF(bbcodes); ++i) {
				if (line[1] == bbcodes[i][1]) {
					size_t lenb = _tcslen(bbcodes[i]);
					if (!_tcsnicmp(line, bbcodes[i], lenb)) {
						size_t len = strlen(bbcodefmt[i]);
						memcpy(d, bbcodefmt[i], len);
						d += len;
						line += lenb - 1;
						found = 1;
						break;
					}
				}
			}
			if (!found) {
				if (!_tcsnicmp(line, _T("[url"), 4)) {
					TCHAR* tag = _tcschr(line + 4, ']');
					if (tag) {
						TCHAR *tagu = (line[4] == '=') ? line + 5 : tag + 1;
						TCHAR *tage = _tcsstr(tag, _T("[/url]"));
						if (!tage) tage = _tcsstr(tag, _T("[/URL]"));
						if (tage) {
							*tag = 0;
							*tage = 0;
							d += sprintf(d, "{\\field{\\*\\fldinst HYPERLINK \"%s\"}{\\fldrslt %s}}", mir_t2a(tagu), mir_t2a(tag + 1)); //!!!!!!!!!!
							line = tage + 5;
							found = 1;
						}
					}
				}
				else if (!_tcsnicmp(line, _T("[color="), 7)) {
					TCHAR* tag = _tcschr(line + 7, ']');
					if (tag) {
						line = tag;
						found = 1;
					}
				}
				else if (!_tcsnicmp(line, _T("[/color]"), 8)) {
					line += 7;
					found = 1;
				}
			}
			if (!found) {
				if (*line < 128)  *d++ = (char)*line;
				else d += sprintf(d, "\\u%d ?", *line); //!!!!!!!!!!
			}
		}
		else if (*line < 128) *d++ = (char)*line;
		else d += sprintf(d, "\\u%d ?", *line); //!!!!!!!!!!
	}

	*(d++) = '}';
	*d = 0;

	cbBufferEnd = (int)(d - buffer);
	return textCharsCount;
}

#define FONT_FORMAT "{\\f%u\\fnil\\fcharset%u %S;}"

static char *CreateRTFHeader(SrmmWindowData *dat)
{
	HDC hdc = GetDC(NULL);
	logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(NULL, hdc);
	size_t bufferEnd = 0, bufferAlloced = 1024;
	char *buffer = (char *)mir_alloc(bufferAlloced);
	buffer[0] = '\0';
	AppendToBuffer(buffer, bufferEnd, bufferAlloced, "{\\rtf1\\ansi\\deff0{\\fonttbl");

	LOGFONT lf;
	for (int i = 0; LoadMsgDlgFont(i, &lf, NULL); i++)
		AppendToBuffer(buffer, bufferEnd, bufferAlloced, FONT_FORMAT, i, lf.lfCharSet, lf.lfFaceName);

	AppendToBuffer(buffer, bufferEnd, bufferAlloced, "}{\\colortbl ");
	COLORREF colour;
	for (int i = 0; LoadMsgDlgFont(i, NULL, &colour); i++)
		AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));

	if (GetSysColorBrush(COLOR_HOTLIGHT) == NULL)
		colour = RGB(0, 0, 255);
	else
		colour = GetSysColor(COLOR_HOTLIGHT);
	AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	AppendToBuffer(buffer, bufferEnd, bufferAlloced, "}");
	return buffer;
}

//mir_free() the return value
static char *CreateRTFTail(SrmmWindowData *dat)
{
	size_t bufferEnd = 0, bufferAlloced = 1024;
	char *buffer = (char *)mir_alloc(bufferAlloced);
	buffer[0] = '\0';
	AppendToBuffer(buffer, bufferEnd, bufferAlloced, "}");
	return buffer;
}

//return value is static
static char *SetToStyle(int style)
{
	static char szStyle[128];
	LOGFONT lf;

	LoadMsgDlgFont(style, &lf, NULL);
	mir_snprintf(szStyle, SIZEOF(szStyle), "\\f%u\\cf%u\\b%d\\i%d\\fs%u", style, style, lf.lfWeight >= FW_BOLD ? 1 : 0, lf.lfItalic, 2 * abs(lf.lfHeight) * 74 / logPixelSY);
	return szStyle;
}

int DbEventIsForMsgWindow(DBEVENTINFO *dbei)
{
	DBEVENTTYPEDESCR *et = (DBEVENTTYPEDESCR*)CallService(MS_DB_EVENT_GETTYPE, (WPARAM)dbei->szModule, (LPARAM)dbei->eventType);
	return et && (et->flags & DETF_MSGWINDOW);
}

int DbEventIsShown(DBEVENTINFO *dbei)
{
	return (dbei->eventType==EVENTTYPE_MESSAGE) ||  DbEventIsForMsgWindow(dbei);
}

//mir_free() the return value
static char *CreateRTFFromDbEvent(SrmmWindowData *dat, MCONTACT hContact, MEVENT hDbEvent, struct LogStreamData *streamData)
{
	int showColon = 0;

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.cbBlob = db_event_getBlobSize(hDbEvent);
	if (dbei.cbBlob == -1)
		return NULL;

	dbei.pBlob = (PBYTE)mir_alloc(dbei.cbBlob);
	db_event_get(hDbEvent, &dbei);
	if (!DbEventIsShown(&dbei)) {
		mir_free(dbei.pBlob);
		return NULL;
	}
	if (!(dbei.flags & DBEF_SENT) && (dbei.eventType == EVENTTYPE_MESSAGE || DbEventIsForMsgWindow(&dbei))) {
		db_event_markRead(hContact, hDbEvent);
		CallService(MS_CLIST_REMOVEEVENT, hContact, (LPARAM)hDbEvent);
	}
	else if (dbei.eventType == EVENTTYPE_JABBER_CHATSTATES || dbei.eventType == EVENTTYPE_JABBER_PRESENCE) {
		db_event_markRead(hContact, hDbEvent);
	}
	
	size_t bufferEnd = 0, bufferAlloced = 1024;
	char *buffer = (char *)mir_alloc(bufferAlloced);
	buffer[0] = '\0';

	if (!dat->bIsAutoRTL && !streamData->isEmpty)
		AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\par");

	if (dbei.flags & DBEF_RTL) {
		AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\rtlpar");
		dat->bIsAutoRTL = TRUE;
	}
	else AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\ltrpar");

	streamData->isEmpty = 0;

	if (dat->bIsAutoRTL) {
		if (dbei.flags & DBEF_RTL)
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\ltrch\\rtlch");
		else
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\rtlch\\ltrch");
	}

	if (g_dat.flags & SMF_SHOWICONS) {
		int i = ((dbei.eventType == EVENTTYPE_MESSAGE) ? ((dbei.flags & DBEF_SENT) ? LOGICON_MSG_OUT : LOGICON_MSG_IN): LOGICON_MSG_NOTICE);
		
		AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\f0\\fs14");
		while (bufferAlloced - bufferEnd < logIconBmpSize[i])
			bufferAlloced += 1024;
		buffer = (char *)mir_realloc(buffer, bufferAlloced);
		memcpy(buffer + bufferEnd, pLogIconBmpBits[i], logIconBmpSize[i]);
		bufferEnd += logIconBmpSize[i];
	}

	if (g_dat.flags & SMF_SHOWTIME) {
		const TCHAR* szFormat;
		TCHAR str[64];

		if (g_dat.flags & SMF_SHOWSECS)
			szFormat = g_dat.flags & SMF_SHOWDATE ? _T("d s") : _T("s");
		else
			szFormat = g_dat.flags & SMF_SHOWDATE ? _T("d t") : _T("t");

		tmi.printTimeStamp(NULL, dbei.timestamp, szFormat, str, SIZEOF(str), 0);

		AppendToBuffer(buffer, bufferEnd, bufferAlloced, " %s ", SetToStyle(dbei.flags & DBEF_SENT ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME));
		AppendToBufferWithRTF(buffer, bufferEnd, bufferAlloced, str);
		showColon = 1;
	}

	if (!(g_dat.flags & SMF_HIDENAMES) && dbei.eventType != EVENTTYPE_JABBER_CHATSTATES && dbei.eventType != EVENTTYPE_JABBER_PRESENCE) {
		TCHAR *szName;
		CONTACTINFO ci = { 0 };

		if (dbei.flags & DBEF_SENT) {
			ci.cbSize = sizeof(ci);
			ci.szProto = dbei.szModule;
			ci.dwFlag = CNF_DISPLAY | CNF_TCHAR;
			if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
				// CNF_DISPLAY always returns a string type
				szName = ci.pszVal;
			}
			else
				// Shouldn't happen?
				szName = mir_tstrdup(TranslateT("Me"));
		}
		else szName = pcli->pfnGetContactDisplayName(hContact, 0);

		AppendToBuffer(buffer, bufferEnd, bufferAlloced, " %s ", SetToStyle(dbei.flags & DBEF_SENT ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME));
		AppendToBufferWithRTF(buffer, bufferEnd, bufferAlloced, szName);
		showColon = 1;
		if (ci.pszVal)
			mir_free(ci.pszVal);
	}

	if (showColon)
		AppendToBuffer(buffer, bufferEnd, bufferAlloced, "%s :", SetToStyle(dbei.flags & DBEF_SENT ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON));

	TCHAR *msg, *szName;
	switch (dbei.eventType) {
	case EVENTTYPE_JABBER_CHATSTATES:
	case EVENTTYPE_JABBER_PRESENCE:
		if (dbei.flags & DBEF_SENT) {
			CONTACTINFO ci = { sizeof(ci) };
			ci.hContact = NULL;
			ci.szProto = dbei.szModule;
			ci.dwFlag = CNF_DISPLAY | CNF_TCHAR;

			// CNF_DISPLAY always returns a string type
			if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)& ci))
				szName = NEWTSTR_ALLOCA(ci.pszVal);
			else
				szName = _T("");

			mir_free(ci.pszVal);
		}
		else szName = pcli->pfnGetContactDisplayName(hContact, 0);

		AppendToBuffer(buffer, bufferEnd, bufferAlloced, " %s ", SetToStyle(MSGFONTID_NOTICE));
		AppendToBufferWithRTF(buffer, bufferEnd, bufferAlloced, szName);
		AppendToBufferWithRTF(buffer, bufferEnd, bufferAlloced, _T(" "));

		msg = DbGetEventTextT(&dbei, CP_ACP);
		if (msg) {
			AppendToBufferWithRTF(buffer, bufferEnd, bufferAlloced, msg);
			mir_free(msg);
		}
		break;

	case EVENTTYPE_FILE:
		{
			char* filename = (char*)dbei.pBlob + sizeof(DWORD);
			char* descr = filename + strlen(filename) + 1;
			
			ptrT ptszFileName(DbGetEventStringT(&dbei, filename));
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, " %s ", SetToStyle(MSGFONTID_NOTICE));
			AppendToBufferWithRTF(buffer, bufferEnd, bufferAlloced, (dbei.flags & DBEF_SENT) ? TranslateT("File sent") : TranslateT("File received"));
			AppendToBuffer(buffer, bufferEnd, bufferAlloced, ": ");
			AppendToBufferWithRTF(buffer, bufferEnd, bufferAlloced, ptszFileName);

			if (*descr != 0) {
				ptrT ptszDescr(DbGetEventStringT(&dbei, descr));
				AppendToBuffer(buffer, bufferEnd, bufferAlloced, " (");
				AppendToBufferWithRTF(buffer, bufferEnd, bufferAlloced, ptszDescr);
				AppendToBuffer(buffer, bufferEnd, bufferAlloced, ")");
			}
		}
		break;

	case EVENTTYPE_MESSAGE:
	default:
		msg = DbGetEventTextT(&dbei, CP_ACP);
		AppendToBuffer(buffer, bufferEnd, bufferAlloced, " %s ", SetToStyle((dbei.eventType == EVENTTYPE_MESSAGE) ? ((dbei.flags & DBEF_SENT) ? MSGFONTID_MYMSG : MSGFONTID_YOURMSG) : MSGFONTID_NOTICE));
		AppendToBufferWithRTF(buffer, bufferEnd, bufferAlloced, msg);
		mir_free(msg);
	}

	if (dat->bIsAutoRTL)
		AppendToBuffer(buffer, bufferEnd, bufferAlloced, "\\par");

	mir_free(dbei.pBlob);
	return buffer;
}

static DWORD CALLBACK LogStreamInEvents(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	struct LogStreamData *dat = (struct LogStreamData *) dwCookie;

	if (dat->buffer == NULL) {
		dat->bufferOffset = 0;
		switch (dat->stage) {
		case STREAMSTAGE_HEADER:
			dat->buffer = CreateRTFHeader(dat->dlgDat);
			dat->stage = STREAMSTAGE_EVENTS;
			break;

		case STREAMSTAGE_EVENTS:
			if (dat->eventsToInsert) {
				do {
					dat->buffer = CreateRTFFromDbEvent(dat->dlgDat, dat->hContact, dat->hDbEvent, dat);
					if (dat->buffer)
						dat->hDbEventLast = dat->hDbEvent;
					dat->hDbEvent = db_event_next(dat->hContact, dat->hDbEvent);
					if (--dat->eventsToInsert == 0)
						break;
				}
				while (dat->buffer == NULL && dat->hDbEvent);
				if (dat->buffer) {
					dat->isEmpty = 0;
					break;
				}
			}
			dat->stage = STREAMSTAGE_TAIL;
			//fall through
		case STREAMSTAGE_TAIL:
			dat->buffer = CreateRTFTail(dat->dlgDat);
			dat->stage = STREAMSTAGE_STOP;
			break;
		case STREAMSTAGE_STOP:
			*pcb = 0;
			return 0;
		}
		dat->bufferLen = (int)strlen(dat->buffer);
	}
	*pcb = min(cb, dat->bufferLen - dat->bufferOffset);
	memcpy(pbBuff, dat->buffer + dat->bufferOffset, *pcb);
	dat->bufferOffset += *pcb;
	if (dat->bufferOffset == dat->bufferLen) {
		mir_free(dat->buffer);
		dat->buffer = NULL;
	}
	return 0;
}

void StreamInEvents(HWND hwndDlg, MEVENT hDbEventFirst, int count, int fAppend)
{
	EDITSTREAM stream = { 0 };
	struct LogStreamData streamData = { 0 };
	SrmmWindowData *dat = (SrmmWindowData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	CHARRANGE oldSel, sel;
	POINT scrollPos;
	BOOL bottomScroll = TRUE;

	HWND hwndLog = GetDlgItem(hwndDlg, IDC_LOG);

	SendMessage(hwndLog, WM_SETREDRAW, FALSE, 0);
	SendMessage(hwndLog, EM_EXGETSEL, 0, (LPARAM)& oldSel);
	streamData.hContact = dat->hContact;
	streamData.hDbEvent = hDbEventFirst;
	streamData.dlgDat = dat;
	streamData.eventsToInsert = count;
	streamData.isEmpty = !fAppend || GetWindowTextLength(hwndLog) == 0;
	stream.pfnCallback = LogStreamInEvents;
	stream.dwCookie = (DWORD_PTR)&streamData;

	if (!streamData.isEmpty) {
		bottomScroll = (GetFocus() != hwndLog);
		if (bottomScroll && (GetWindowLongPtr(hwndLog, GWL_STYLE) & WS_VSCROLL)) {
			SCROLLINFO si = { 0 };
			si.cbSize = sizeof(si);
			si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
			GetScrollInfo(hwndLog, SB_VERT, &si);
			bottomScroll = (si.nPos + (int)si.nPage) >= si.nMax;
		}
		if (!bottomScroll)
			SendMessage(hwndLog, EM_GETSCROLLPOS, 0, (LPARAM)& scrollPos);
	}
	if (fAppend) {
		sel.cpMin = sel.cpMax = -1;
		SendMessage(hwndLog, EM_EXSETSEL, 0, (LPARAM)& sel);
	}

	strcpy(szSep2, fAppend ? "\\par\\sl0" : "\\sl1000");
	strcpy(szSep2_RTL, fAppend ? "\\rtlpar\\rtlmark\\par\\sl1000" : "\\sl1000");

	SendMessage(hwndLog, EM_STREAMIN, fAppend ? SFF_SELECTION | SF_RTF : SF_RTF, (LPARAM)& stream);
	if (bottomScroll) {
		sel.cpMin = sel.cpMax = -1;
		SendMessage(hwndLog, EM_EXSETSEL, 0, (LPARAM)& sel);
		if (GetWindowLongPtr(hwndLog, GWL_STYLE) & WS_VSCROLL) {
			SendMessage(hwndDlg, DM_SCROLLLOGTOBOTTOM, 0, 0);
			PostMessage(hwndDlg, DM_SCROLLLOGTOBOTTOM, 0, 0);
		}
	}
	else {
		SendMessage(hwndLog, EM_EXSETSEL, 0, (LPARAM)& oldSel);
		SendMessage(hwndLog, EM_SETSCROLLPOS, 0, (LPARAM)& scrollPos);
	}

	SendMessage(hwndLog, WM_SETREDRAW, TRUE, 0);
	if (bottomScroll)
		RedrawWindow(hwndLog, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

	dat->hDbEventLast = streamData.hDbEventLast;
}

#define RTFPICTHEADERMAXSIZE   78
void LoadMsgLogIcons(void)
{
	HBRUSH hBkgBrush = CreateSolidBrush(db_get_dw(NULL, SRMMMOD, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR));

	BITMAPINFOHEADER bih = { sizeof(bih) };
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;
	bih.biHeight = 10;
	bih.biPlanes = 1;
	bih.biWidth = 10;
	int widthBytes = ((bih.biWidth * bih.biBitCount + 31) >> 5) * 4;

	RECT rc;
	rc.top = rc.left = 0;
	rc.right = bih.biWidth;
	rc.bottom = bih.biHeight;

	HDC hdc = GetDC(NULL);
	HBITMAP hBmp = CreateCompatibleBitmap(hdc, bih.biWidth, bih.biHeight);
	HDC hdcMem = CreateCompatibleDC(hdc);
	PBYTE pBmpBits = (PBYTE)mir_alloc(widthBytes * bih.biHeight);

	for (int i = 0; i < SIZEOF(pLogIconBmpBits); i++) {
		HICON hIcon = Skin_GetIconByHandle(iconList[i].hIcolib);
		size_t size = RTFPICTHEADERMAXSIZE + (bih.biSize + widthBytes * bih.biHeight) * 2;
		pLogIconBmpBits[i] = (char*)mir_alloc(size);
		size_t rtfHeaderSize = mir_snprintf(pLogIconBmpBits[i], size, "{\\pict\\dibitmap0\\wbmbitspixel%u\\wbmplanes1\\wbmwidthbytes%u\\picw%u\\pich%u ", bih.biBitCount, widthBytes, bih.biWidth, bih.biHeight);
		HBITMAP hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
		FillRect(hdcMem, &rc, hBkgBrush);
		DrawIconEx(hdcMem, 0, 0, hIcon, bih.biWidth, bih.biHeight, 0, NULL, DI_NORMAL);
		Skin_ReleaseIcon(hIcon);

		SelectObject(hdcMem, hoBmp);
		GetDIBits(hdc, hBmp, 0, bih.biHeight, pBmpBits, (BITMAPINFO*)&bih, DIB_RGB_COLORS);

		char *szDest = pLogIconBmpBits[i] + rtfHeaderSize;
		bin2hex(&bih, sizeof(bih), szDest); szDest += sizeof(bih) * 2;
		bin2hex(pBmpBits, widthBytes * bih.biHeight, szDest); szDest += widthBytes * bih.biHeight * 2;
		strcpy(szDest, "}");

		logIconBmpSize[i] = size_t(szDest - pLogIconBmpBits[i]) + 1;
	}
	mir_free(pBmpBits);
	DeleteDC(hdcMem);
	DeleteObject(hBmp);
	ReleaseDC(NULL, hdc);
	DeleteObject(hBkgBrush);
}

void FreeMsgLogIcons(void)
{
	for (int i = 0; i < SIZEOF(pLogIconBmpBits); i++)
		mir_free(pLogIconBmpBits[i]);
}
