/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson

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

#include "..\..\core\commonheaders.h"

#include "chat.h"

// The code for streaming the text is to a large extent copied from
// the srmm module and then modified to fit the chat module.

PBYTE pLogIconBmpBits[14];
int logIconBmpSize[ SIZEOF(pLogIconBmpBits) ];

#define RTFCACHELINESIZE 128
static char	CHAT_rtfFontsGlobal[OPTIONS_FONTCOUNT][RTFCACHELINESIZE];

static int EventToIndex(LOGINFO *lin)
{
	switch (lin->iType) {
		case GC_EVENT_MESSAGE:
			if (lin->bIsMe)
				return 10;
			else
				return 9;

		case GC_EVENT_JOIN: return 3;
		case GC_EVENT_PART: return 4;
		case GC_EVENT_QUIT: return 5;
		case GC_EVENT_NICK: return 7;
		case GC_EVENT_KICK: return 6;
		case GC_EVENT_NOTICE: return 8;
		case GC_EVENT_TOPIC: return 11;
		case GC_EVENT_INFORMATION:return 12;
		case GC_EVENT_ADDSTATUS: return 13;
		case GC_EVENT_REMOVESTATUS: return 14;
		case GC_EVENT_ACTION: return 15;
	}
	return 0;
}

static int EventToIcon(LOGINFO *lin)
{
	switch (lin->iType) {
		case GC_EVENT_MESSAGE:
			if (lin->bIsMe)
				return ICON_MESSAGEOUT;
			else
				return ICON_MESSAGE;

		case GC_EVENT_JOIN: return ICON_JOIN;
		case GC_EVENT_PART: return ICON_PART;
		case GC_EVENT_QUIT: return ICON_QUIT;
		case GC_EVENT_NICK: return ICON_NICK;
		case GC_EVENT_KICK: return ICON_KICK;
		case GC_EVENT_NOTICE: return ICON_NOTICE;
		case GC_EVENT_TOPIC: return ICON_TOPIC;
		case GC_EVENT_INFORMATION:return ICON_INFO;
		case GC_EVENT_ADDSTATUS: return ICON_ADDSTATUS;
		case GC_EVENT_REMOVESTATUS: return ICON_REMSTATUS;
		case GC_EVENT_ACTION: return ICON_ACTION;
	}
	return 0;
}

char* Log_SetStyle(int style)
{
	if (style < OPTIONS_FONTCOUNT)
		return CHAT_rtfFontsGlobal[style];

	return "";
}

static void Log_Append(char **buffer, int *cbBufferEnd, int *cbBufferAlloced, const char *fmt, ...)
{
	va_list va;
	int charsDone = 0;

	va_start(va, fmt);
	for (;;) {
		charsDone = mir_vsnprintf(*buffer + *cbBufferEnd, *cbBufferAlloced - *cbBufferEnd, fmt, va);
		if (charsDone >= 0)
			break;
		*cbBufferAlloced += 4096;
		*buffer = (char *)mir_realloc(*buffer, *cbBufferAlloced);
	}
	va_end(va);
	*cbBufferEnd += charsDone;
}

static int Log_AppendRTF(LOGSTREAMDATA* streamData, BOOL simpleMode, char **buffer, int *cbBufferEnd, int *cbBufferAlloced, const TCHAR *fmt, ...)
{
	va_list va;
	int lineLen, textCharsCount = 0;
	TCHAR* line = (TCHAR*)alloca(8001 * sizeof(TCHAR));
	char* d;

	va_start(va, fmt);
	lineLen = mir_vsntprintf(line, 8000, fmt, va);
	if (lineLen < 0) lineLen = 8000;
	line[lineLen] = 0;
	va_end(va);

	lineLen = lineLen * 20 + 8;
	if (*cbBufferEnd + lineLen > *cbBufferAlloced) {
		cbBufferAlloced[0] += (lineLen + 1024 - lineLen % 1024);
		*buffer = (char *)mir_realloc(*buffer, *cbBufferAlloced);
	}

	d = *buffer + *cbBufferEnd;

	for (; *line; line++, textCharsCount++) {
		if (*line == '\r' && line[1] == '\n') {
			CopyMemory(d, "\\par ", 5);
			line++;
			d += 5;
		}
		else if (*line == '\n') {
			CopyMemory(d, "\\line ", 6);
			d += 6;
		}
		else if (*line == '%' && !simpleMode) {
			char szTemp[200];

			szTemp[0] = '\0';
			switch (*++line) {
			case '\0':
			case '%':
				*d++ = '%';
				break;

			case 'c':
			case 'f':
				if (g_Settings->bStripFormat || streamData->bStripFormat)
					line += 2;

				else if (line[1] != '\0' && line[2] != '\0') {
					TCHAR szTemp3[3], c = *line;
					int col;
					szTemp3[0] = line[1];
					szTemp3[1] = line[2];
					szTemp3[2] = '\0';
					line += 2;

					col = _ttoi(szTemp3);
					col += (OPTIONS_FONTCOUNT + 1);
					mir_snprintf(szTemp, SIZEOF(szTemp), (c == 'c') ? "\\cf%u " : "\\highlight%u ", col);
				}
				break;
			case 'C':
			case 'F':
				if (!g_Settings->bStripFormat && !streamData->bStripFormat) {
					int j = streamData->lin->bIsHighlighted ? 16 : EventToIndex(streamData->lin);
					if (*line == 'C')
						mir_snprintf(szTemp, SIZEOF(szTemp), "\\cf%u ", j + 1);
					else
						mir_snprintf(szTemp, SIZEOF(szTemp), "\\highlight0 ");
				}
				break;
			case 'b':
			case 'u':
			case 'i':
				if (!streamData->bStripFormat)
					mir_snprintf(szTemp, SIZEOF(szTemp), (*line == 'u') ? "\\%cl " : "\\%c ", *line);
				break;

			case 'B':
			case 'U':
			case 'I':
				if (!streamData->bStripFormat) {
					mir_snprintf(szTemp, SIZEOF(szTemp), (*line == 'U') ? "\\%cl0 " : "\\%c0 ", *line);
					CharLowerA(szTemp);
				}
				break;

			case 'r':
				if (!streamData->bStripFormat) {
					int index = EventToIndex(streamData->lin);
					mir_snprintf(szTemp, SIZEOF(szTemp), "%s ", Log_SetStyle(index));
				}
				break;
			}

			if (szTemp[0]) {
				size_t iLen = mir_strlen(szTemp);
				memcpy(d, szTemp, iLen);
				d += iLen;
			}
		}
		else if (*line == '\t' && !streamData->bStripFormat) {
			CopyMemory(d, "\\tab ", 5);
			d += 5;
		}
		else if ((*line == '\\' || *line == '{' || *line == '}') && !streamData->bStripFormat) {
			*d++ = '\\';
			*d++ = (char)*line;
		}
		else if (*line > 0 && *line < 128) {
			*d++ = (char)*line;
		}
		else d += sprintf(d, "\\u%u ?", (WORD)*line); //!!!!!!!!!!!
	}

	*cbBufferEnd = (int)(d - *buffer);
	return textCharsCount;
}

static void AddEventToBuffer(char **buffer, int *bufferEnd, int *bufferAlloced, LOGSTREAMDATA *streamData)
{
	TCHAR szTemp[512], szTemp2[512];
	TCHAR* pszNick = NULL;
	if (streamData->lin->ptszNick) {
		if (g_Settings->bLogLimitNames && mir_tstrlen(streamData->lin->ptszNick) > 20) {
			mir_tstrncpy(szTemp2, streamData->lin->ptszNick, 20);
			mir_tstrncpy(szTemp2 + 20, _T("..."), 4);
		}
		else mir_tstrncpy(szTemp2, streamData->lin->ptszNick, 511);

		if (streamData->lin->ptszUserInfo)
			mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%s (%s)"), szTemp2, streamData->lin->ptszUserInfo);
		else
			mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%s"), szTemp2);
		pszNick = szTemp;
	}

	switch (streamData->lin->iType) {
	case GC_EVENT_MESSAGE:
		if (streamData->lin->ptszText)
			Log_AppendRTF(streamData, FALSE, buffer, bufferEnd, bufferAlloced, _T("%s"), streamData->lin->ptszText);
		break;
	case GC_EVENT_ACTION:
		if (streamData->lin->ptszNick && streamData->lin->ptszText) {
			Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced, _T("%s "), streamData->lin->ptszNick);
			Log_AppendRTF(streamData, FALSE, buffer, bufferEnd, bufferAlloced, _T("%s"), streamData->lin->ptszText);
		}
		break;
	case GC_EVENT_JOIN:
		if (pszNick) {
			if (!streamData->lin->bIsMe)
				Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("%s has joined"), pszNick);
			else
				Log_AppendRTF(streamData, FALSE, buffer, bufferEnd, bufferAlloced, TranslateT("You have joined %s"), streamData->si->ptszName);
		}
		break;
	case GC_EVENT_PART:
		if (pszNick)
			Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("%s has left"), pszNick);
		if (streamData->lin->ptszText)
			Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced, _T(": %s"), streamData->lin->ptszText);
		break;
	case GC_EVENT_QUIT:
		if (pszNick)
			Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("%s has disconnected"), pszNick);
		if (streamData->lin->ptszText)
			Log_AppendRTF(streamData, FALSE, buffer, bufferEnd, bufferAlloced, _T(": %s"), streamData->lin->ptszText);
		break;
	case GC_EVENT_NICK:
		if (pszNick && streamData->lin->ptszText) {
			if (!streamData->lin->bIsMe)
				Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("%s is now known as %s"), pszNick, streamData->lin->ptszText);
			else
				Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("You are now known as %s"), streamData->lin->ptszText);
		}
		break;
	case GC_EVENT_KICK:
		if (streamData->lin->ptszNick && streamData->lin->ptszStatus)
			Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("%s kicked %s"), streamData->lin->ptszStatus, streamData->lin->ptszNick);
		if (streamData->lin->ptszText)
			Log_AppendRTF(streamData, FALSE, buffer, bufferEnd, bufferAlloced, _T(": %s"), streamData->lin->ptszText);
		break;
	case GC_EVENT_NOTICE:
		if (pszNick && streamData->lin->ptszText) {
			Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("Notice from %s: "), pszNick);
			Log_AppendRTF(streamData, FALSE, buffer, bufferEnd, bufferAlloced, _T("%s"), streamData->lin->ptszText);
		}
		break;
	case GC_EVENT_TOPIC:
		if (streamData->lin->ptszText)
			Log_AppendRTF(streamData, FALSE, buffer, bufferEnd, bufferAlloced, TranslateT("The topic is '%s%s'"), streamData->lin->ptszText, _T("%r"));
		if (streamData->lin->ptszNick)
			Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced,
			streamData->lin->ptszUserInfo ? TranslateT(" (set by %s on %s)") : TranslateT(" (set by %s)"),
			streamData->lin->ptszNick, streamData->lin->ptszUserInfo);
		break;
	case GC_EVENT_INFORMATION:
		if (streamData->lin->ptszText)
			Log_AppendRTF(streamData, FALSE, buffer, bufferEnd, bufferAlloced, (streamData->lin->bIsMe) ? _T("--> %s") : _T("%s"), streamData->lin->ptszText);
		break;
	case GC_EVENT_ADDSTATUS:
		if (streamData->lin->ptszNick && streamData->lin->ptszText && streamData->lin->ptszStatus)
			Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("%s enables '%s' status for %s"), streamData->lin->ptszText, streamData->lin->ptszStatus, streamData->lin->ptszNick);
		break;
	case GC_EVENT_REMOVESTATUS:
		if (streamData->lin->ptszNick && streamData->lin->ptszText && streamData->lin->ptszStatus)
			Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("%s disables '%s' status for %s"), streamData->lin->ptszText, streamData->lin->ptszStatus, streamData->lin->ptszNick);
		break;
	}
}

TCHAR* MakeTimeStamp(TCHAR *pszStamp, time_t time)
{
	static TCHAR szTime[30];
	if (!_tcsftime(szTime, SIZEOF(szTime)-1, pszStamp, localtime(&time)))
		_tcsncpy(szTime, TranslateT("<invalid>"), SIZEOF(szTime));
	return szTime;
}

char* Log_CreateRTF(LOGSTREAMDATA *streamData)
{
	char *buffer, *header;
	int bufferAlloced, bufferEnd, me = 0;
	LOGINFO *lin = streamData->lin;
	MODULEINFO *mi = ci.MM_FindModule(streamData->si->pszModule);

	// guesstimate amount of memory for the RTF
	bufferEnd = 0;
	bufferAlloced = streamData->bRedraw ? 1024 * (streamData->si->iEventCount + 2) : 2048;
	buffer = (char *)mir_alloc(bufferAlloced);
	buffer[0] = '\0';

	// ### RTF HEADER
	header = mi->pszHeader;
	if (header)
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, header);

	// ### RTF BODY (one iteration per event that should be streamed in)
	while (lin) {
		// filter
		if ((streamData->si->iType != GCW_CHATROOM && streamData->si->iType != GCW_PRIVMESS) || !streamData->si->bFilterEnabled || (streamData->si->iLogFilterFlags&lin->iType) != 0) {
			// create new line, and set font and color
			Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\par%s ", Log_SetStyle(0));

			// Insert icon
			if ((lin->iType & g_Settings->dwIconFlags) || lin->bIsHighlighted && (g_Settings->dwIconFlags & GC_EVENT_HIGHLIGHT)) {
				int iIndex = (lin->bIsHighlighted && g_Settings->dwIconFlags & GC_EVENT_HIGHLIGHT) ? ICON_HIGHLIGHT : EventToIcon(lin);
				Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\f0\\fs14");
				while (bufferAlloced - bufferEnd < logIconBmpSize[0])
					bufferAlloced += 4096;
				buffer = (char *)mir_realloc(buffer, bufferAlloced);
				CopyMemory(buffer + bufferEnd, pLogIconBmpBits[iIndex], logIconBmpSize[iIndex]);
				bufferEnd += logIconBmpSize[iIndex];
			}

			if (g_Settings->bTimeStampEventColour) {
				LOGFONT &lf = ci.aFonts[0].lf;

				// colored timestamps
				static char szStyle[256];
				int iii;
				if (lin->ptszNick && lin->iType == GC_EVENT_MESSAGE) {
					iii = lin->bIsHighlighted ? 16 : (lin->bIsMe ? 2 : 1);
					mir_snprintf(szStyle, SIZEOF(szStyle), "\\f0\\cf%u\\ul0\\highlight0\\b%d\\i%d\\fs%u", iii + 1, lf.lfWeight >= FW_BOLD ? 1 : 0, lf.lfItalic, 2 * abs(lf.lfHeight) * 74 / ci.logPixelSY);
					Log_Append(&buffer, &bufferEnd, &bufferAlloced, "%s ", szStyle);
				}
				else {
					iii = lin->bIsHighlighted ? 16 : EventToIndex(lin);
					mir_snprintf(szStyle, SIZEOF(szStyle), "\\f0\\cf%u\\ul0\\highlight0\\b%d\\i%d\\fs%u", iii + 1, lf.lfWeight >= FW_BOLD ? 1 : 0, lf.lfItalic, 2 * abs(lf.lfHeight) * 74 / ci.logPixelSY);
					Log_Append(&buffer, &bufferEnd, &bufferAlloced, "%s ", szStyle);
				}
			}
			else Log_Append(&buffer, &bufferEnd, &bufferAlloced, "%s ", Log_SetStyle(0));

			// insert a TAB if necessary to put the timestamp in the right position
			if (g_Settings->dwIconFlags)
				Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\tab ");

			//insert timestamp
			if (g_Settings->bShowTime) {
				TCHAR szTimeStamp[30], szOldTimeStamp[30];

				mir_tstrncpy(szTimeStamp, MakeTimeStamp(g_Settings->pszTimeStamp, lin->time), 30);
				mir_tstrncpy(szOldTimeStamp, MakeTimeStamp(g_Settings->pszTimeStamp, streamData->si->LastTime), 30);
				if (!g_Settings->bShowTimeIfChanged || streamData->si->LastTime == 0 || mir_tstrcmp(szTimeStamp, szOldTimeStamp)) {
					streamData->si->LastTime = lin->time;
					Log_AppendRTF(streamData, TRUE, &buffer, &bufferEnd, &bufferAlloced, _T("%s"), szTimeStamp);
				}
				Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\tab ");
			}

			// Insert the nick
			if (lin->ptszNick && lin->iType == GC_EVENT_MESSAGE) {
				TCHAR pszTemp[300], *p1;

				Log_Append(&buffer, &bufferEnd, &bufferAlloced, "%s ", Log_SetStyle(lin->bIsMe ? 2 : 1));
				mir_tstrncpy(pszTemp, lin->bIsMe ? g_Settings->pszOutgoingNick : g_Settings->pszIncomingNick, 299);
				p1 = _tcsstr(pszTemp, _T("%n"));
				if (p1)
					p1[1] = 's';

				Log_AppendRTF(streamData, TRUE, &buffer, &bufferEnd, &bufferAlloced, pszTemp, lin->ptszNick);
				Log_Append(&buffer, &bufferEnd, &bufferAlloced, " ");
			}

			// Insert the message
			Log_Append(&buffer, &bufferEnd, &bufferAlloced, "%s ", Log_SetStyle(lin->bIsHighlighted ? 16 : EventToIndex(lin)));
			streamData->lin = lin;
			AddEventToBuffer(&buffer, &bufferEnd, &bufferAlloced, streamData);
		}
		lin = lin->prev;
	}

	// ### RTF END
	Log_Append(&buffer, &bufferEnd, &bufferAlloced, "}");
	return buffer;
}

char* Log_CreateRtfHeader(MODULEINFO *mi)
{
	int bufferAlloced, bufferEnd, i;

	// guesstimate amount of memory for the RTF header
	bufferEnd = 0;
	bufferAlloced = 4096;
	char *buffer = (char *)mir_realloc(mi->pszHeader, bufferAlloced);
	buffer[0] = '\0';

	// get the number of pixels per logical inch
	HDC hdc = GetDC(NULL);
	ci.logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
	ci.logPixelSX = GetDeviceCaps(hdc, LOGPIXELSX);
	ReleaseDC(NULL, hdc);

	// ### RTF HEADER

	// font table
	Log_Append(&buffer, &bufferEnd, &bufferAlloced, "{\\rtf1\\ansi\\deff0{\\fonttbl");
	for (i = 0; i < OPTIONS_FONTCOUNT; i++)
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "{\\f%u\\fnil\\fcharset%u%S;}", i, ci.aFonts[i].lf.lfCharSet, ci.aFonts[i].lf.lfFaceName);

	// colour table
	Log_Append(&buffer, &bufferEnd, &bufferAlloced, "}{\\colortbl ;");

	for (i = 0; i < OPTIONS_FONTCOUNT; i++)
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(ci.aFonts[i].color), GetGValue(ci.aFonts[i].color), GetBValue(ci.aFonts[i].color));

	for (i = 0; i < mi->nColorCount; i++)
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(mi->crColors[i]), GetGValue(mi->crColors[i]), GetBValue(mi->crColors[i]));

	// new paragraph
	Log_Append(&buffer, &bufferEnd, &bufferAlloced, "}\\pard");

	// set tabs and indents
	int iIndent = 0;

	if (g_Settings->dwIconFlags) {
		iIndent += (14 * 1440) / ci.logPixelSX;
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\tx%u", iIndent);
	}
	if (g_Settings->bShowTime) {
		int iSize = (g_Settings->LogTextIndent * 1440) / ci.logPixelSX;
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\tx%u", iIndent + iSize);
		if (g_Settings->bLogIndentEnabled)
			iIndent += iSize;
	}

	Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\fi-%u\\li%u", iIndent, iIndent);
	return buffer;
}

#define RTFPICTHEADERMAXSIZE 78

void LoadMsgLogBitmaps(void)
{
	HBRUSH hBkgBrush = CreateSolidBrush(g_Settings->crLogBackground);

	BITMAPINFOHEADER bih = { 0 };
	bih.biSize = sizeof(bih);
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;
	bih.biHeight = bih.biWidth = g_Settings->LogIconSize;
	bih.biPlanes = 1;
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
		HICON hIcon = ci.hIcons[i];
		size_t size = RTFPICTHEADERMAXSIZE + (bih.biSize + widthBytes * bih.biHeight) * 2;
		pLogIconBmpBits[i] = (PBYTE)mir_alloc(size);
		int rtfHeaderSize = mir_snprintf((char *)pLogIconBmpBits[i], size, "{\\pict\\dibitmap0\\wbmbitspixel%u\\wbmplanes1\\wbmwidthbytes%u\\picw%u\\pich%u ", bih.biBitCount, widthBytes, bih.biWidth, bih.biHeight);
		HBITMAP hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
		FillRect(hdcMem, &rc, hBkgBrush);
		DrawIconEx(hdcMem, 0, 0, hIcon, bih.biWidth, bih.biHeight, 0, NULL, DI_NORMAL);
		SelectObject(hdcMem, hoBmp);
		GetDIBits(hdc, hBmp, 0, bih.biHeight, pBmpBits, (BITMAPINFO *)& bih, DIB_RGB_COLORS);

		int n;
		for (n = 0; n < sizeof(BITMAPINFOHEADER); n++)
			sprintf((char *)pLogIconBmpBits[i] + rtfHeaderSize + n * 2, "%02X", ((PBYTE)& bih)[n]); //!!!!!!!!!!!!!
		for (n = 0; n < widthBytes * bih.biHeight; n += 4)
			sprintf((char *)pLogIconBmpBits[i] + rtfHeaderSize + (bih.biSize + n) * 2, "%02X%02X%02X%02X", pBmpBits[n], pBmpBits[n + 1], pBmpBits[n + 2], pBmpBits[n + 3]); //!!!!!!!!!!!!!

		logIconBmpSize[i] = rtfHeaderSize + (bih.biSize + widthBytes * bih.biHeight) * 2 + 1;
		pLogIconBmpBits[i][logIconBmpSize[i] - 1] = '}';
	}
	mir_free(pBmpBits);
	DeleteDC(hdcMem);
	DeleteObject(hBmp);
	ReleaseDC(NULL, hdc);
	DeleteObject(hBkgBrush);

	if (ci.logPixelSY == 0) {
		HDC hdc;
		hdc = GetDC(NULL);
		ci.logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		ci.logPixelSX = GetDeviceCaps(hdc, LOGPIXELSX);
		ReleaseDC(NULL, hdc);
	}

	for (int i = 0; i < OPTIONS_FONTCOUNT; i++) {
		LOGFONT &F = ci.aFonts[i].lf;
		mir_snprintf(CHAT_rtfFontsGlobal[i], RTFCACHELINESIZE,
			"\\f%u\\cf%u\\ul0\\highlight0\\b%d\\i%d\\ul%d\\fs%u", i, i + 1,
			F.lfWeight >= FW_BOLD ? 1 : 0, F.lfItalic, F.lfUnderline, 2 * abs(F.lfHeight) * 74 / ci.logPixelSY);
	}
}

void FreeMsgLogBitmaps(void)
{
	for (int i = 0; i < SIZEOF(pLogIconBmpBits); i++)
		mir_free(pLogIconBmpBits[i]);
}
