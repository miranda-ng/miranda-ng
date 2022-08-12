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

#include "stdafx.h"

#include "chat.h"

// The code for streaming the text is to a large extent copied from
// the srmm module and then modified to fit the chat module.

char *pLogIconBmpBits[14];

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

static int Log_AppendRTF(LOGSTREAMDATA *streamData, bool simpleMode, CMStringA &buf, const wchar_t *line)
{
	int textCharsCount = 0;

	for (; *line; line++, textCharsCount++) {
		if (*line == '\r' && line[1] == '\n') {
			buf.Append("\\par ");
			line++;
		}
		else if (*line == '\n') {
			buf.Append("\\line ");
		}
		else if (*line == '%' && !simpleMode) {
			char szTemp[200];

			szTemp[0] = '\0';
			switch (*++line) {
			case '\0':
			case '%':
				buf.AppendChar('%');
				break;

			case 'c':
			case 'f':
				if (g_Settings->bStripFormat || streamData->bStripFormat)
					line += 2;

				else if (line[1] != '\0' && line[2] != '\0') {
					wchar_t szTemp3[3], c = *line;
					int col;
					szTemp3[0] = line[1];
					szTemp3[1] = line[2];
					szTemp3[2] = '\0';
					line += 2;

					col = _wtoi(szTemp3);
					col += (OPTIONS_FONTCOUNT + 1);
					mir_snprintf(szTemp, (c == 'c') ? "\\cf%u " : "\\highlight%u ", col);
				}
				break;
			case 'C':
			case 'F':
				if (!g_Settings->bStripFormat && !streamData->bStripFormat) {
					int j = streamData->lin->bIsHighlighted ? 16 : EventToIndex(streamData->lin);
					if (*line == 'C')
						mir_snprintf(szTemp, "\\cf%u ", j + 1);
					else
						mir_snprintf(szTemp, "\\highlight0 ");
				}
				break;
			case 'b':
			case 'u':
			case 'i':
				if (!streamData->bStripFormat)
					mir_snprintf(szTemp, (*line == 'u') ? "\\%cl " : "\\%c ", *line);
				break;

			case 'B':
			case 'U':
			case 'I':
				if (!streamData->bStripFormat) {
					mir_snprintf(szTemp, (*line == 'U') ? "\\%cl0 " : "\\%c0 ", *line);
					CharLowerA(szTemp);
				}
				break;

			case 'r':
				if (!streamData->bStripFormat) {
					int index = EventToIndex(streamData->lin);
					mir_snprintf(szTemp, "%s ", Log_SetStyle(index));
				}
				break;
			}

			if (szTemp[0])
				buf.Append(szTemp);
		}
		else if (*line == '\t' && !streamData->bStripFormat) {
			buf.Append("\\tab ");
		}
		else if ((*line == '\\' || *line == '{' || *line == '}') && !streamData->bStripFormat) {
			buf.AppendChar('\\');
			buf.AppendChar(*line);
		}
		else if (*line > 0 && *line < 128) {
			buf.AppendChar(*line);
		}
		else buf.AppendFormat("\\u%u ?", (uint16_t)*line);
	}

	return textCharsCount;
}

MIR_APP_DLL(bool) Chat_GetDefaultEventDescr(const SESSION_INFO *si, const LOGINFO *lin, CMStringW &res)
{
	CMStringW wszNick;
	g_chatApi.CreateNick(si, lin, wszNick);

	switch (lin->iType) {
	case GC_EVENT_ACTION:
		if (lin->ptszNick)
			res = lin->ptszNick;
		break;

	case GC_EVENT_JOIN:
		if (!lin->bIsMe) {
			if (!wszNick.IsEmpty())
				res.AppendFormat(TranslateT("%s has joined"), wszNick.c_str());
		}
		else res.AppendFormat(TranslateT("You have joined %s"), si->ptszName);
		break;

	case GC_EVENT_PART:
		if (!wszNick.IsEmpty())
			res.AppendFormat(TranslateT("%s has left"), wszNick.c_str());
		break;

	case GC_EVENT_QUIT:
		if (!wszNick.IsEmpty())
			res.AppendFormat(TranslateT("%s has disconnected"), wszNick.c_str());
		break;

	case GC_EVENT_NICK:
		if (!lin->bIsMe) {
			if (!wszNick.IsEmpty())
				res.AppendFormat(TranslateT("%s is now known as %s"), wszNick.c_str(), lin->ptszText);
		}
		else res.AppendFormat(TranslateT("You are now known as %s"), lin->ptszText);
		return true;

	case GC_EVENT_KICK:
		if (lin->ptszNick && lin->ptszStatus)
			res.AppendFormat(TranslateT("%s kicked %s"), lin->ptszStatus, lin->ptszNick);
		break;

	case GC_EVENT_NOTICE:
		if (!wszNick.IsEmpty())
			res.AppendFormat(TranslateT("Notice from %s"), wszNick.c_str());
		break;

	case GC_EVENT_TOPIC:
		if (lin->ptszText)
			res.AppendFormat(TranslateT("The topic is '%s'"), lin->ptszText);
		if (lin->ptszNick)
			res.AppendFormat((lin->ptszUserInfo) ? TranslateT(" (set by %s on %s)") : TranslateT(" (set by %s)"), lin->ptszNick, lin->ptszUserInfo);
		return true;

	case GC_EVENT_INFORMATION:
		if (lin->ptszText)
			res.AppendFormat((lin->bIsMe) ? L"--> %s" : L"%s", lin->ptszText);
		return true;

	case GC_EVENT_ADDSTATUS:
		if (lin->ptszNick && lin->ptszText && lin->ptszStatus)
			res.AppendFormat(TranslateT("%s enables '%s' status for %s"), lin->ptszText, lin->ptszStatus, lin->ptszNick);
		return true;

	case GC_EVENT_REMOVESTATUS:
		if (lin->ptszNick && lin->ptszText && lin->ptszStatus)
			res.AppendFormat(TranslateT("%s disables '%s' status for %s"), lin->ptszText, lin->ptszStatus, lin->ptszNick);
		return true;
	}

	return false;
}

static void AddEventToBuffer(CMStringA &buf, LOGSTREAMDATA *streamData)
{
	CMStringW wszCaption;
	bool bTextUsed = Chat_GetDefaultEventDescr(streamData->si, streamData->lin, wszCaption);
	if (!wszCaption.IsEmpty())
		Log_AppendRTF(streamData, !bTextUsed, buf, wszCaption);
	if (!bTextUsed && streamData->lin->ptszText) {
		if (!wszCaption.IsEmpty())
			Log_AppendRTF(streamData, false, buf, L" ");
		Log_AppendRTF(streamData, false, buf, streamData->lin->ptszText);
	}
}

wchar_t* MakeTimeStamp(wchar_t *pszStamp, time_t time)
{
	static wchar_t szTime[100];
	if (!wcsftime(szTime, _countof(szTime)-1, pszStamp, localtime(&time)))
		wcsncpy_s(szTime, TranslateT("<invalid>"), _TRUNCATE);
	return szTime;
}

char* Log_CreateRTF(LOGSTREAMDATA *streamData)
{
	SESSION_INFO *si = streamData->si;
	MODULEINFO *mi = si->pMI;

	// guesstimate amount of memory for the RTF
	CMStringA buf;

	// ### RTF HEADER
	char *header = mi->pszHeader;
	if (header)
		buf.Append(header);

	// ### RTF BODY (one iteration per event that should be streamed in)
	for (LOGINFO *lin = streamData->lin; lin; lin = lin->prev) {
		// filter
		if (si->iType == GCW_CHATROOM || si->iType == GCW_PRIVMESS)
			if (si->pDlg->m_bFilterEnabled && (si->pDlg->m_iLogFilterFlags & lin->iType) == 0)
				continue;

		// create new line, and set font and color
		if (lin->next != nullptr)
			buf.Append("\\par ");
		buf.AppendFormat("%s ", Log_SetStyle(0));

		// Insert icon
		if ((lin->iType & g_Settings->dwIconFlags) || lin->bIsHighlighted && (g_Settings->dwIconFlags & GC_EVENT_HIGHLIGHT)) {
			int iIndex = (lin->bIsHighlighted && (g_Settings->dwIconFlags & GC_EVENT_HIGHLIGHT)) ? ICON_HIGHLIGHT : EventToIcon(lin);
			buf.Append("\\f0\\fs14");
			buf.Append(pLogIconBmpBits[iIndex]);
		}

		if (g_Settings->bTimeStampEventColour) {
			LOGFONT &lf = g_chatApi.aFonts[0].lf;

			// colored timestamps
			if (lin->ptszNick && lin->iType == GC_EVENT_MESSAGE) {
				int iii = lin->bIsHighlighted ? 16 : (lin->bIsMe ? 2 : 1);
				buf.AppendFormat("\\f0\\cf%u\\ul0\\highlight0\\b%d\\i%d\\fs%u", iii + 1, lf.lfWeight >= FW_BOLD ? 1 : 0, lf.lfItalic, 2 * abs(lf.lfHeight) * 74 / g_chatApi.logPixelSY);
			}
			else {
				int iii = lin->bIsHighlighted ? 16 : EventToIndex(lin);
				buf.AppendFormat("\\f0\\cf%u\\ul0\\highlight0\\b%d\\i%d\\fs%u", iii + 1, lf.lfWeight >= FW_BOLD ? 1 : 0, lf.lfItalic, 2 * abs(lf.lfHeight) * 74 / g_chatApi.logPixelSY);
			}
		}
		else buf.AppendFormat("%s ", Log_SetStyle(0));

		if (g_Settings->dwIconFlags)
			buf.Append("\\tab ");

		//insert timestamp
		if (g_Settings->bShowTime) {
			wchar_t szTimeStamp[100], szOldTimeStamp[100];

			wcsncpy_s(szTimeStamp, MakeTimeStamp(g_Settings->pszTimeStamp, lin->time), _TRUNCATE);
			wcsncpy_s(szOldTimeStamp, MakeTimeStamp(g_Settings->pszTimeStamp, si->LastTime), _TRUNCATE);
			if (!g_Settings->bShowTimeIfChanged || si->LastTime == 0 || mir_wstrcmp(szTimeStamp, szOldTimeStamp)) {
				si->LastTime = lin->time;
				Log_AppendRTF(streamData, true, buf, szTimeStamp);
			}
			buf.Append("\\tab ");
		}

		// Insert the nick
		if (lin->ptszNick && lin->iType == GC_EVENT_MESSAGE) {
			buf.AppendFormat("%s ", Log_SetStyle(lin->bIsMe ? 2 : 1));

			CMStringW tmp((lin->bIsMe) ? g_Settings->pszOutgoingNick : g_Settings->pszIncomingNick);
			tmp.Replace(L"%n", lin->ptszNick);
			Log_AppendRTF(streamData, TRUE, buf, tmp);
			buf.AppendChar(' ');
		}

		// Insert the message
		buf.AppendFormat("%s ", Log_SetStyle(lin->bIsHighlighted ? 16 : EventToIndex(lin)));
		streamData->lin = lin;
		AddEventToBuffer(buf, streamData);
	}

	// ### RTF END
	if (streamData->bRedraw)
		buf.Append("\\par}");
	else
		buf.Append("}");
	return buf.Detach();
}

char* Log_CreateRtfHeader()
{
	// guesstimate amount of memory for the RTF header
	CMStringA buf;

	// get the number of pixels per logical inch
	HDC hdc = GetDC(nullptr);
	g_chatApi.logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
	g_chatApi.logPixelSX = GetDeviceCaps(hdc, LOGPIXELSX);
	ReleaseDC(nullptr, hdc);

	// ### RTF HEADER

	// font table
	buf.Append("{\\rtf1\\ansi\\deff0{\\fonttbl");
	for (int i = 0; i < OPTIONS_FONTCOUNT; i++)
		buf.AppendFormat("{\\f%u\\fnil\\fcharset%u%S;}", i, g_chatApi.aFonts[i].lf.lfCharSet, g_chatApi.aFonts[i].lf.lfFaceName);

	// colour table
	buf.Append("}{\\colortbl ;");

	for (int i = 0; i < OPTIONS_FONTCOUNT; i++)
		buf.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(g_chatApi.aFonts[i].color), GetGValue(g_chatApi.aFonts[i].color), GetBValue(g_chatApi.aFonts[i].color));

	// new paragraph
	buf.Append("}\\pard");

	// set tabs and indents
	int iIndent = 0;

	if (g_Settings) {
		if (g_Settings->dwIconFlags) {
			iIndent += (14 * 1440) / g_chatApi.logPixelSX;
			buf.AppendFormat("\\tx%u", iIndent);
		}
		if (g_Settings->bShowTime) {
			int iSize = (g_Settings->LogTextIndent * 1440) / g_chatApi.logPixelSX;
			buf.AppendFormat("\\tx%u", iIndent + iSize);
			if (g_Settings->bLogIndentEnabled)
				iIndent += iSize;
		}
	}

	buf.AppendFormat("\\fi-%u\\li%u", iIndent, iIndent);
	return buf.Detach();
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

	HDC hdc = GetDC(nullptr);
	HBITMAP hBmp = CreateCompatibleBitmap(hdc, bih.biWidth, bih.biHeight);
	HDC hdcMem = CreateCompatibleDC(hdc);
	uint8_t *pBmpBits = (uint8_t*)mir_alloc(widthBytes * bih.biHeight);
	for (int i = 0; i < _countof(pLogIconBmpBits); i++) {
		size_t size = RTFPICTHEADERMAXSIZE + (bih.biSize + widthBytes * bih.biHeight) * 2;
		pLogIconBmpBits[i] = (char*)mir_alloc(size);
		size_t rtfHeaderSize = mir_snprintf((char *)pLogIconBmpBits[i], size, "{\\pict\\dibitmap0\\wbmbitspixel%u\\wbmplanes1\\wbmwidthbytes%u\\picw%u\\pich%u ", bih.biBitCount, widthBytes, bih.biWidth, bih.biHeight);

		HICON hIcon = g_hChatIcons[i];
		HBITMAP hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
		FillRect(hdcMem, &rc, hBkgBrush);
		DrawIconEx(hdcMem, 0, 0, hIcon, bih.biWidth, bih.biHeight, 0, nullptr, DI_NORMAL);
		SelectObject(hdcMem, hoBmp);
		GetDIBits(hdc, hBmp, 0, bih.biHeight, pBmpBits, (BITMAPINFO *)& bih, DIB_RGB_COLORS);

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

	if (g_chatApi.logPixelSY == 0) {
		hdc = GetDC(nullptr);
		g_chatApi.logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		g_chatApi.logPixelSX = GetDeviceCaps(hdc, LOGPIXELSX);
		ReleaseDC(nullptr, hdc);
	}

	for (int i = 0; i < OPTIONS_FONTCOUNT; i++) {
		LOGFONT &F = g_chatApi.aFonts[i].lf;
		mir_snprintf(CHAT_rtfFontsGlobal[i], RTFCACHELINESIZE,
			"\\f%u\\cf%u\\ul0\\highlight0\\b%d\\i%d\\ul%d\\fs%u", i, i + 1,
			F.lfWeight >= FW_BOLD ? 1 : 0, F.lfItalic, F.lfUnderline, 2 * abs(F.lfHeight) * 74 / g_chatApi.logPixelSY);
	}
}

void FreeMsgLogBitmaps(void)
{
	for (int i = 0; i < _countof(pLogIconBmpBits); i++)
		mir_free(pLogIconBmpBits[i]);
}
