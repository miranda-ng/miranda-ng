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

char* Log_SetStyle(int style)
{
	if (style < OPTIONS_FONTCOUNT)
		return CHAT_rtfFontsGlobal[style];

	return "";
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

wchar_t* MakeTimeStamp(wchar_t *pszStamp, time_t time)
{
	static wchar_t szTime[100];
	if (!wcsftime(szTime, _countof(szTime)-1, pszStamp, localtime(&time)))
		wcsncpy_s(szTime, TranslateT("<invalid>"), _TRUNCATE);
	return szTime;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Chat event streamer

static DWORD CALLBACK ChatLogStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	RtfChatLogStreamData *dat = (RtfChatLogStreamData *)dwCookie;
	auto *si = dat->si;

	if (dat->buf.IsEmpty()) {
		switch (dat->iStage) {
		case STREAMSTAGE_HEADER:
			dat->pLog->CreateChatRtfHeader(dat);
			dat->iStage = STREAMSTAGE_EVENTS;
			break;

		case STREAMSTAGE_EVENTS:
			if (!dat->lin) {
				auto &events = si->arEvents;
				if (dat->idx < events.getCount()) {
					auto &lin = events[dat->idx];
					if (si->iType == GCW_SERVER || (si->pDlg->m_iLogFilterFlags & lin.iType) != 0)
						dat->pLog->CreateChatRtfEvent(dat, lin);
					dat->idx++;
					break;
				}
			}
			else dat->pLog->CreateChatRtfEvent(dat, *dat->lin);

			dat->iStage = STREAMSTAGE_TAIL;
			__fallthrough;

		case STREAMSTAGE_TAIL:
			dat->pLog->CreateChatRtfTail(dat);
			dat->iStage = STREAMSTAGE_STOP;
			break;

		case STREAMSTAGE_STOP:
			*pcb = 0;
			return 0;
		}
	}

	*pcb = min(cb, dat->buf.GetLength());
	memcpy(pbBuff, dat->buf.GetBuffer(), *pcb);
	if (dat->buf.GetLength() == *pcb)
		dat->buf.Empty();
	else
		dat->buf.Delete(0, *pcb);

	return 0;
}

void CRtfLogWindow::StreamChatRtfEvents(RtfChatLogStreamData *streamData, bool bRedraw)
{
	streamData->bRedraw = bRedraw;
	if (streamData->lin)
		streamData->idx = streamData->si->arEvents.getCount()-1;

	EDITSTREAM stream = {};
	stream.pfnCallback = ChatLogStreamCallback;
	stream.dwCookie = (DWORD_PTR)streamData;
	m_rtf.SendMsg(EM_STREAMIN, (bRedraw) ? SF_RTF : SFF_SELECTION | SF_RTF, (LPARAM)&stream);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Chat event header

void CRtfLogWindow::CreateChatRtfHeader(RtfChatLogStreamData *streamData)
{
	CMStringA &buf = streamData->buf;

	// get the number of pixels per logical inch
	HDC hdc = GetDC(nullptr);
	g_chatApi.logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
	g_chatApi.logPixelSX = GetDeviceCaps(hdc, LOGPIXELSX);
	ReleaseDC(nullptr, hdc);

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
}

/////////////////////////////////////////////////////////////////////////////////////////
// Chat event body

void CRtfLogWindow::CreateChatRtfEvent(RtfChatLogStreamData *streamData, const LOGINFO &lin)
{
	SESSION_INFO *si = streamData->si;
	CMStringA &buf = streamData->buf;

	// create new line, and set font and color
	if (streamData->idx != 0)
		buf.Append("\\par ");
	buf.AppendFormat("%s ", Log_SetStyle(0));

	// Insert icon
	if ((lin.iType & g_Settings->dwIconFlags) || lin.bIsHighlighted && (g_Settings->dwIconFlags & GC_EVENT_HIGHLIGHT)) {
		int iIndex = (lin.bIsHighlighted && (g_Settings->dwIconFlags & GC_EVENT_HIGHLIGHT)) ? ICON_HIGHLIGHT : lin.getIcon();
		buf.Append("\\f0\\fs14");
		buf.Append(pLogIconBmpBits[iIndex]);
	}

	if (g_Settings->bTimeStampEventColour) {
		LOGFONT &lf = g_chatApi.aFonts[0].lf;

		// colored timestamps
		if (lin.ptszNick && lin.iType == GC_EVENT_MESSAGE) {
			int iii = lin.bIsHighlighted ? 16 : (lin.bIsMe ? 2 : 1);
			buf.AppendFormat("\\f0\\cf%u\\ul0\\highlight0\\b%d\\i%d\\fs%u", iii + 1, lf.lfWeight >= FW_BOLD ? 1 : 0, lf.lfItalic, 2 * abs(lf.lfHeight) * 74 / g_chatApi.logPixelSY);
		}
		else {
			int iii = lin.bIsHighlighted ? 16 : lin.getIndex();
			buf.AppendFormat("\\f0\\cf%u\\ul0\\highlight0\\b%d\\i%d\\fs%u", iii + 1, lf.lfWeight >= FW_BOLD ? 1 : 0, lf.lfItalic, 2 * abs(lf.lfHeight) * 74 / g_chatApi.logPixelSY);
		}
	}
	else buf.AppendFormat("%s ", Log_SetStyle(0));

	if (g_Settings->dwIconFlags)
		buf.Append("\\tab ");

	// insert timestamp
	if (g_Settings->bShowTime) {
		wchar_t szTimeStamp[100], szOldTimeStamp[100];

		wcsncpy_s(szTimeStamp, MakeTimeStamp(g_Settings->pszTimeStamp, lin.time), _TRUNCATE);
		wcsncpy_s(szOldTimeStamp, MakeTimeStamp(g_Settings->pszTimeStamp, si->LastTime), _TRUNCATE);
		if (!g_Settings->bShowTimeIfChanged || si->LastTime == 0 || mir_wstrcmp(szTimeStamp, szOldTimeStamp)) {
			si->LastTime = lin.time;
			lin.write(streamData, true, buf, szTimeStamp);
		}
		buf.Append("\\tab ");
	}

	// Insert the nick
	if (lin.ptszNick && lin.iType == GC_EVENT_MESSAGE) {
		buf.AppendFormat("%s ", Log_SetStyle(lin.bIsMe ? 2 : 1));

		CMStringW tmp((lin.bIsMe) ? g_Settings->pszOutgoingNick : g_Settings->pszIncomingNick);
		tmp.Replace(L"%n", lin.ptszNick);
		lin.write(streamData, true, buf, tmp);
		buf.AppendChar(' ');
	}

	// Insert the message
	CreateChatRtfMessage(streamData, lin, buf);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Chat event message

void CRtfLogWindow::CreateChatRtfMessage(RtfChatLogStreamData *streamData, const LOGINFO &lin, CMStringA &buf)
{
	buf.AppendFormat("%s ", Log_SetStyle(lin.bIsHighlighted ? 16 : lin.getIndex()));

	CMStringW wszCaption;
	bool bTextUsed = Chat_GetDefaultEventDescr(streamData->si, &lin, wszCaption);
	if (!wszCaption.IsEmpty()) {
		wszCaption.AppendChar(' ');
		lin.write(streamData, !bTextUsed, buf, wszCaption);
	}
	if (bTextUsed)
		return;

	if (lin.hEvent) {
		DB::EventInfo dbei(lin.hEvent);
		if (dbei) {
			if (dbei.eventType == EVENTTYPE_FILE) {
				DB::FILE_BLOB blob(dbei);
				if (blob.isOffline()) {
					InsertFileLink(buf, lin.hEvent, blob);
					return;
				}
			}
			lin.write(streamData, false, buf, ptrW(DbEvent_GetTextW(&dbei, CP_UTF8)));
			return;
		}
	}
	if (lin.ptszText)
		lin.write(streamData, false, buf, lin.ptszText);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Chat event tail

void CRtfLogWindow::CreateChatRtfTail(RtfChatLogStreamData *streamData)
{
	CMStringA &str = streamData->buf;

	if (streamData->bRedraw)
		str.Append("\\par}");
	else
		str.Append("}");;
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
