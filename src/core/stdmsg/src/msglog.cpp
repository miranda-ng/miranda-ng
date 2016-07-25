/*

Copyright 2000-12 Miranda IM, 2012-16 Miranda NG project,
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

extern IconItem iconList[];

#define LOGICON_MSG_IN      0
#define LOGICON_MSG_OUT     1
#define LOGICON_MSG_NOTICE  2

static char *pLogIconBmpBits[3];
static size_t logIconBmpSize[ _countof(pLogIconBmpBits) ];

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

static const TCHAR *bbcodes[] = { L"[b]", L"[i]", L"[u]", L"[s]", L"[/b]", L"[/i]", L"[/u]", L"[/s]" };
static const char *bbcodefmt[] = { "\\b ", "\\i ", "\\ul ", "\\strike ", "\\b0 ", "\\i0 ", "\\ul0 ", "\\strike0 " };

static void AppendPlainUnicode(CMStringA &buf, const TCHAR *str)
{
	for (; *str; str++) {
		if (*str < 128)
			buf.AppendChar((char)*str);
		else
			buf.AppendFormat("\\u%d ?", *str);
	}
}

static void AppendToBufferWithRTF(CMStringA &buf, const TCHAR *line)
{
	if (line == NULL)
		return;

	buf.Append("{\\uc1 ");

	for (; *line; line++) {
		if (*line == '\r' && line[1] == '\n') {
			buf.Append("\\par ");
			line++;
		}
		else if (*line == '\n') {
			buf.Append("\\par ");
		}
		else if (*line == '\t') {
			buf.Append("\\tab ");
		}
		else if (*line == '\\' || *line == '{' || *line == '}') {
			buf.AppendChar('\\');
			buf.AppendChar(*line);
		}
		else if (*line == '[' && (g_dat.flags & SMF_SHOWFORMAT)) {
			int i, found = 0;
			for (i = 0; i < _countof(bbcodes); ++i) {
				if (line[1] == bbcodes[i][1]) {
					size_t lenb = mir_tstrlen(bbcodes[i]);
					if (!_tcsnicmp(line, bbcodes[i], lenb)) {
						buf.Append(bbcodefmt[i]);
						line += lenb - 1;
						found = 1;
						break;
					}
				}
			}
			if (!found) {
				if (!_tcsnicmp(line, L"[url", 4)) {
					const TCHAR* tag = _tcschr(line + 4, ']');
					if (tag) {
						const TCHAR *tagu = (line[4] == '=') ? line + 5 : tag + 1;
						const TCHAR *tage = _tcsstr(tag, L"[/url]");
						if (!tage) tage = _tcsstr(tag, L"[/URL]");
						if (tage) {
							*(TCHAR*)tag = 0;
							*(TCHAR*)tage = 0;
							buf.Append("{\\field{\\*\\fldinst HYPERLINK \"");
							AppendPlainUnicode(buf, tagu);
							buf.Append("\"}{\\fldrslt ");
							AppendPlainUnicode(buf, tag+1);
							buf.Append("}}");
							line = tage + 5;
							found = 1;
						}
					}
				}
				else if (!_tcsnicmp(line, L"[color=", 7)) {
					const TCHAR* tag = _tcschr(line + 7, ']');
					if (tag) {
						line = tag;
						found = 1;
					}
				}
				else if (!_tcsnicmp(line, L"[/color]", 8)) {
					line += 7;
					found = 1;
				}
			}
			if (!found) {
				if (*line < 128)
					buf.AppendChar((char)*line);
				else
					buf.AppendFormat("\\u%d ?", *line);
			}
		}
		else if (*line < 128)
			buf.AppendChar((char)*line);
		else
			buf.AppendFormat("\\u%d ?", *line);
	}

	buf.AppendChar('}');
}

#define FONT_FORMAT "{\\f%u\\fnil\\fcharset%u %S;}"

static char* CreateRTFHeader(SrmmWindowData*)
{
	HDC hdc = GetDC(NULL);
	logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(NULL, hdc);

	CMStringA buffer;
	buffer.Append("{\\rtf1\\ansi\\deff0{\\fonttbl");

	LOGFONT lf;
	for (int i = 0; LoadMsgDlgFont(i, &lf, NULL); i++)
		buffer.AppendFormat(FONT_FORMAT, i, lf.lfCharSet, lf.lfFaceName);

	buffer.Append("}{\\colortbl ");
	COLORREF colour;
	for (int i = 0; LoadMsgDlgFont(i, NULL, &colour); i++)
		buffer.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));

	if (GetSysColorBrush(COLOR_HOTLIGHT) == NULL)
		colour = RGB(0, 0, 255);
	else
		colour = GetSysColor(COLOR_HOTLIGHT);
	buffer.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	buffer.Append("}");
	return buffer.Detach();
}

// mir_free() the return value
static char* CreateRTFTail(SrmmWindowData*)
{
	return mir_strdup("}");
}

//return value is static
static char* SetToStyle(int style)
{
	static char szStyle[128];
	LOGFONT lf;

	LoadMsgDlgFont(style, &lf, NULL);
	mir_snprintf(szStyle, "\\f%u\\cf%u\\b%d\\i%d\\fs%u", style, style, lf.lfWeight >= FW_BOLD ? 1 : 0, lf.lfItalic, 2 * abs(lf.lfHeight) * 74 / logPixelSY);
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
static char* CreateRTFFromDbEvent(SrmmWindowData *dat, MCONTACT hContact, MEVENT hDbEvent, struct LogStreamData *streamData)
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
		pcli->pfnRemoveEvent(hContact, hDbEvent);
	}
	else if (dbei.eventType == EVENTTYPE_JABBER_CHATSTATES || dbei.eventType == EVENTTYPE_JABBER_PRESENCE) {
		db_event_markRead(hContact, hDbEvent);
	}

	CMStringA buffer;
	if (!dat->bIsAutoRTL && !streamData->isEmpty)
		buffer.Append("\\par");

	if (dbei.flags & DBEF_RTL) {
		buffer.Append("\\rtlpar");
		dat->bIsAutoRTL = TRUE;
	}
	else buffer.Append("\\ltrpar");

	streamData->isEmpty = 0;

	if (dat->bIsAutoRTL) {
		if (dbei.flags & DBEF_RTL)
			buffer.Append("\\ltrch\\rtlch");
		else
			buffer.Append("\\rtlch\\ltrch");
	}

	if (g_dat.flags & SMF_SHOWICONS) {
		int i = ((dbei.eventType == EVENTTYPE_MESSAGE) ? ((dbei.flags & DBEF_SENT) ? LOGICON_MSG_OUT : LOGICON_MSG_IN): LOGICON_MSG_NOTICE);
		
		buffer.Append("\\f0\\fs14");
		buffer.Append(pLogIconBmpBits[i]);
	}

	if (g_dat.flags & SMF_SHOWTIME) {
		const TCHAR* szFormat;
		TCHAR str[64];

		if (g_dat.flags & SMF_SHOWSECS)
			szFormat = g_dat.flags & SMF_SHOWDATE ? L"d s" : L"s";
		else
			szFormat = g_dat.flags & SMF_SHOWDATE ? L"d t" : L"t";

		TimeZone_PrintTimeStamp(NULL, dbei.timestamp, szFormat, str, _countof(str), 0);

		buffer.AppendFormat(" %s ", SetToStyle(dbei.flags & DBEF_SENT ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME));
		AppendToBufferWithRTF(buffer, str);
		showColon = 1;
	}

	if (!(g_dat.flags & SMF_HIDENAMES) && dbei.eventType != EVENTTYPE_JABBER_CHATSTATES && dbei.eventType != EVENTTYPE_JABBER_PRESENCE) {
		TCHAR *szName;

		if (dbei.flags & DBEF_SENT) {
			if (TCHAR *p = Contact_GetInfo(CNF_DISPLAY, NULL, dbei.szModule))
				szName = NEWTSTR_ALLOCA(p);
			else
				szName = TranslateT("Me");
		}
		else szName = pcli->pfnGetContactDisplayName(hContact, 0);

		buffer.AppendFormat(" %s ", SetToStyle(dbei.flags & DBEF_SENT ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME));
		AppendToBufferWithRTF(buffer, szName);
		showColon = 1;
	}

	if (showColon)
		buffer.AppendFormat("%s :", SetToStyle(dbei.flags & DBEF_SENT ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON));

	TCHAR *msg, *szName;
	switch (dbei.eventType) {
	case EVENTTYPE_JABBER_CHATSTATES:
	case EVENTTYPE_JABBER_PRESENCE:
		if (dbei.flags & DBEF_SENT) {
			if (TCHAR *p = Contact_GetInfo(CNF_DISPLAY, NULL, dbei.szModule)) {
				szName = NEWTSTR_ALLOCA(p);
				mir_free(p);
			}
			else szName = L"";
		}
		else szName = pcli->pfnGetContactDisplayName(hContact, 0);

		buffer.AppendFormat(" %s ", SetToStyle(MSGFONTID_NOTICE));
		AppendToBufferWithRTF(buffer, szName);
		AppendToBufferWithRTF(buffer, L" ");

		msg = DbGetEventTextT(&dbei, CP_ACP);
		if (msg) {
			AppendToBufferWithRTF(buffer, msg);
			mir_free(msg);
		}
		break;

	case EVENTTYPE_FILE:
		{
			char* filename = (char*)dbei.pBlob + sizeof(DWORD);
			char* descr = filename + mir_strlen(filename) + 1;
			
			ptrT ptszFileName(DbGetEventStringT(&dbei, filename));
			buffer.AppendFormat(" %s ", SetToStyle(MSGFONTID_NOTICE));
			AppendToBufferWithRTF(buffer, (dbei.flags & DBEF_SENT) ? TranslateT("File sent") : TranslateT("File received"));
			buffer.Append(": ");
			AppendToBufferWithRTF(buffer, ptszFileName);

			if (*descr != 0) {
				ptrT ptszDescr(DbGetEventStringT(&dbei, descr));
				buffer.Append(" (");
				AppendToBufferWithRTF(buffer, ptszDescr);
				buffer.Append(")");
			}
		}
		break;

	case EVENTTYPE_MESSAGE:
	default:
		msg = DbGetEventTextT(&dbei, CP_ACP);
		buffer.AppendFormat(" %s ", SetToStyle((dbei.eventType == EVENTTYPE_MESSAGE) ? ((dbei.flags & DBEF_SENT) ? MSGFONTID_MYMSG : MSGFONTID_YOURMSG) : MSGFONTID_NOTICE));
		AppendToBufferWithRTF(buffer, msg);
		mir_free(msg);
	}

	if (dat->bIsAutoRTL)
		buffer.Append("\\par");

	mir_free(dbei.pBlob);
	return buffer.Detach();
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
		dat->bufferLen = (int)mir_strlen(dat->buffer);
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
	SrmmWindowData *dat = (SrmmWindowData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	CHARRANGE oldSel, sel;
	BOOL bottomScroll = TRUE;
	POINT scrollPos;

	HWND hwndLog = GetDlgItem(hwndDlg, IDC_LOG);

	SendMessage(hwndLog, WM_SETREDRAW, FALSE, 0);
	SendMessage(hwndLog, EM_EXGETSEL, 0, (LPARAM)& oldSel);

	LogStreamData streamData = { 0 };
	streamData.hContact = dat->hContact;
	streamData.hDbEvent = hDbEventFirst;
	streamData.dlgDat = dat;
	streamData.eventsToInsert = count;
	streamData.isEmpty = !fAppend || GetWindowTextLength(hwndLog) == 0;

	EDITSTREAM stream = { 0 };
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
			SendMessage(hwndLog, EM_GETSCROLLPOS, 0, (LPARAM)&scrollPos);
	}
	if (fAppend) {
		sel.cpMin = sel.cpMax = -1;
		SendMessage(hwndLog, EM_EXSETSEL, 0, (LPARAM)& sel);
	}

	mir_strcpy(szSep2, fAppend ? "\\par\\sl0" : "\\sl1000");
	mir_strcpy(szSep2_RTL, fAppend ? "\\rtlpar\\rtlmark\\par\\sl1000" : "\\sl1000");

	SendMessage(hwndLog, EM_STREAMIN, fAppend ? SFF_SELECTION | SF_RTF : SF_RTF, (LPARAM)& stream);
	if (bottomScroll) {
		sel.cpMin = sel.cpMax = -1;
		SendMessage(hwndLog, EM_EXSETSEL, 0, (LPARAM)&sel);
		if (GetWindowLongPtr(hwndLog, GWL_STYLE) & WS_VSCROLL) {
			SendMessage(hwndDlg, DM_SCROLLLOGTOBOTTOM, 0, 0);
			PostMessage(hwndDlg, DM_SCROLLLOGTOBOTTOM, 0, 0);
		}
	}
	else {
		SendMessage(hwndLog, EM_EXSETSEL, 0, (LPARAM)&oldSel);
		SendMessage(hwndLog, EM_SETSCROLLPOS, 0, (LPARAM)&scrollPos);
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

	for (int i = 0; i < _countof(pLogIconBmpBits); i++) {
		HICON hIcon = IcoLib_GetIconByHandle(iconList[i].hIcolib);
		size_t size = RTFPICTHEADERMAXSIZE + (bih.biSize + widthBytes * bih.biHeight) * 2;
		pLogIconBmpBits[i] = (char*)mir_alloc(size);
		size_t rtfHeaderSize = mir_snprintf(pLogIconBmpBits[i], size, "{\\pict\\dibitmap0\\wbmbitspixel%u\\wbmplanes1\\wbmwidthbytes%u\\picw%u\\pich%u ", bih.biBitCount, widthBytes, bih.biWidth, bih.biHeight);
		HBITMAP hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
		FillRect(hdcMem, &rc, hBkgBrush);
		DrawIconEx(hdcMem, 0, 0, hIcon, bih.biWidth, bih.biHeight, 0, NULL, DI_NORMAL);
		IcoLib_ReleaseIcon(hIcon);

		SelectObject(hdcMem, hoBmp);
		GetDIBits(hdc, hBmp, 0, bih.biHeight, pBmpBits, (BITMAPINFO*)&bih, DIB_RGB_COLORS);

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
}

void FreeMsgLogIcons(void)
{
	for (int i = 0; i < _countof(pLogIconBmpBits); i++)
		mir_free(pLogIconBmpBits[i]);
}
