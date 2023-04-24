/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-23 Miranda NG team,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// Implements the richedit-based message history display for the group
// chat window.

#include "stdafx.h"

/*
 * The code for streaming the text is to a large extent copied from
 * the srmm module and then modified to fit the chat module.
 */

static uint8_t EventToSymbol(const LOGINFO &lin)
{
	switch (lin.iType) {
	case GC_EVENT_MESSAGE:
		return (lin.bIsMe) ? 0x37 : 0x38;
	case GC_EVENT_JOIN:
		return 0x34;
	case GC_EVENT_PART:
		return 0x33;
	case GC_EVENT_QUIT:
		return 0x39;
	case GC_EVENT_NICK:
		return 0x71;
	case GC_EVENT_KICK:
		return 0x72;
	case GC_EVENT_NOTICE:
		return 0x28;
	case GC_EVENT_INFORMATION:
		return 0x69;
	case GC_EVENT_ADDSTATUS:
		return 0x35;
	case GC_EVENT_REMOVESTATUS:
		return 0x36;
	case GC_EVENT_ACTION:
		return 0x60;
	}
	return 0x73;
}

/////////////////////////////////////////////////////////////////////////////////////////
// RTF header

void CLogWindow::CreateChatRtfHeader(RtfChatLogStreamData *streamData)
{
	// get the number of pixels per logical inch
	if (g_chatApi.logPixelSY == 0) {
		HDC hdc = GetDC(nullptr);
		g_chatApi.logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		g_chatApi.logPixelSX = GetDeviceCaps(hdc, LOGPIXELSX);
		ReleaseDC(nullptr, hdc);
	}

	// ### RTF HEADER
	CMStringA &str = streamData->buf;

	// font table
	str.Append("{\\rtf1\\ansi\\deff0{\\fonttbl");
	for (int i = 0; i < OPTIONS_FONTCOUNT; i++)
		str.AppendFormat("{\\f%u\\fnil\\fcharset%u%S;}", i, g_chatApi.aFonts[i].lf.lfCharSet, g_chatApi.aFonts[i].lf.lfFaceName);

	// colour table
	str.Append("}{\\colortbl ;");

	for (auto &it : g_chatApi.aFonts)
		str.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(it.color), GetGValue(it.color), GetBValue(it.color));

	for (auto &it : Utils::rtf_clrs)
		str.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(it->clr), GetGValue(it->clr), GetBValue(it->clr));

	for (int i = 0; i < STATUSICONCOUNT; i++)
		str.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(g_Settings.nickColors[i]), GetGValue(g_Settings.nickColors[i]), GetBValue(g_Settings.nickColors[i]));

	// new paragraph
	str.AppendFormat("}\\pard\\sl%d", 1000);

	// set tabs and indents
	int iIndent = 0;

	if (g_Settings.bLogSymbols) {
		wchar_t szString[2];
		LOGFONT lf;

		szString[1] = 0;
		szString[0] = 0x28;
		g_chatApi.LoadMsgDlgFont(17, &lf, nullptr);
		lf.lfCharSet = SYMBOL_CHARSET;
		wcsncpy_s(lf.lfFaceName, L"Webdings", _TRUNCATE);

		HFONT hFont = CreateFontIndirect(&lf);
		int iText = Chat_GetTextPixelSize(szString, hFont, true) + 3;
		DeleteObject(hFont);
		iIndent += (iText * 1440) / g_chatApi.logPixelSX;
		str.AppendFormat("\\tx%u", iIndent);
	}
	else if (g_Settings.dwIconFlags) {
		iIndent += ((g_Settings.bScaleIcons ? 14 : 20) * 1440) / g_chatApi.logPixelSX;
		str.AppendFormat("\\tx%u", iIndent);
	}
	if (g_Settings.bShowTime) {
		int iSize = (g_Settings.LogTextIndent * 1440) / g_chatApi.logPixelSX;
		str.AppendFormat("\\tx%u", iIndent + iSize);
		if (g_Settings.bLogIndentEnabled)
			iIndent += iSize;
	}
	str.AppendFormat("\\fi-%u\\li%u", iIndent, iIndent);
}

/////////////////////////////////////////////////////////////////////////////////////////
// RTF event

void CLogWindow::CreateChatRtfEvent(RtfChatLogStreamData *streamData, const LOGINFO &lin)
{
	SESSION_INFO *si = streamData->si;
	CMStringA &str = streamData->buf;

	if (streamData->idx != 0)
		str.Append("\\par ");

	if (m_pDlg.m_bDividerWanted) {
		static char szStyle_div[128] = "\0";
		if (szStyle_div[0] == 0)
			mir_snprintf(szStyle_div, "\\f%u\\cf%u\\ul0\\b%d\\i%d\\fs%u", 17, 18, 0, 0, 5);

		if (streamData->idx != si->arEvents.getCount()-1 || !streamData->bRedraw)
			str.AppendFormat("\\qc\\sl-1\\highlight%d %s ---------------------------------------------------------------------------------------\\par ", 18, szStyle_div);
		m_pDlg.m_bDividerWanted = false;
	}
	// create new line, and set font and color
	str.AppendFormat("\\ql\\sl0%s ", g_chatApi.Log_SetStyle(0));
	str.AppendFormat("\\v~-+%p+-~\\v0 ", &lin);

	// Insert icon
	if (g_Settings.bLogSymbols)                // use symbols
		str.AppendFormat("%s %c", g_chatApi.Log_SetStyle(17), EventToSymbol(lin));
	else if (g_Settings.dwIconFlags) {
		int iIndex = lin.bIsHighlighted ? ICON_HIGHLIGHT : lin.getIcon();
		str.Append("\\f0\\fs14");
		str.Append(g_chatApi.pLogIconBmpBits[iIndex]);
	}

	if (g_Settings.bTimeStampEventColour) {
		// colored timestamps
		static char szStyle[256];
		LOGFONT &F = g_chatApi.aFonts[0].lf;
		int iii;
		if (lin.ptszNick && lin.iType == GC_EVENT_MESSAGE) {
			iii = lin.bIsHighlighted ? 16 : (lin.bIsMe ? 2 : 1);
			mir_snprintf(szStyle, "\\f0\\cf%u\\ul0\\highlight0\\b%d\\i%d\\ul%d\\fs%u",
				iii + 1, F.lfWeight >= FW_BOLD ? 1 : 0, F.lfItalic, F.lfUnderline, 2 * abs(F.lfHeight) * 74 / g_chatApi.logPixelSY);
			str.Append(szStyle);
		}
		else {
			iii = lin.bIsHighlighted ? 16 : lin.getIndex();
			mir_snprintf(szStyle, "\\f0\\cf%u\\ul0\\highlight0\\b%d\\i%d\\ul%d\\fs%u",
				iii + 1, F.lfWeight >= FW_BOLD ? 1 : 0, F.lfItalic, F.lfUnderline, 2 * abs(F.lfHeight) * 74 / g_chatApi.logPixelSY);
			str.Append(szStyle);
		}
	}
	else str.Append(g_chatApi.Log_SetStyle(0));
	str.AppendChar(' ');

	// insert a TAB if necessary to put the timestamp in the right position
	if (g_Settings.dwIconFlags)
		str.Append("\\tab ");

	// insert timestamp
	if (g_Settings.bShowTime) {
		wchar_t szTimeStamp[30], szOldTimeStamp[30];

		wcsncpy_s(szTimeStamp, g_chatApi.MakeTimeStamp(g_Settings.pszTimeStamp, lin.time), _TRUNCATE);
		wcsncpy_s(szOldTimeStamp, g_chatApi.MakeTimeStamp(g_Settings.pszTimeStamp, si->LastTime), _TRUNCATE);
		if (!g_Settings.bShowTimeIfChanged || si->LastTime == 0 || mir_wstrcmp(szTimeStamp, szOldTimeStamp)) {
			si->LastTime = lin.time;
			lin.write(streamData, true, str, szTimeStamp);
		}
		str.Append("\\tab ");
	}

	// Insert the nick
	if (lin.ptszNick && lin.iType == GC_EVENT_MESSAGE) {
		char pszIndicator[3] = "\0\0";
		int  crNickIndex = 0;

		if (g_Settings.bLogClassicIndicators || g_Settings.bColorizeNicksInLog)
			pszIndicator[0] = GetIndicator(si, lin.ptszNick, &crNickIndex);

		str.Append(g_chatApi.Log_SetStyle(lin.bIsMe ? 2 : 1));
		str.AppendChar(' ');

		if (g_Settings.bLogClassicIndicators)
			str.Append(pszIndicator);

		CMStringW pszTemp(lin.bIsMe ? g_Settings.pszOutgoingNick : g_Settings.pszIncomingNick);
		if (!lin.bIsMe) {
			if (g_Settings.bClickableNicks)
				pszTemp.Replace(L"%n", CLICKNICK_BEGIN L"%n" CLICKNICK_END);

			if (g_Settings.bColorizeNicksInLog && pszIndicator[0])
				str.AppendFormat("\\cf%u ", OPTIONS_FONTCOUNT + Utils::rtf_clrs.getCount() + crNickIndex);
		}
		pszTemp.Replace(L"%n", lin.ptszNick);

		if (g_Settings.bNewLineAfterNames)
			pszTemp.AppendChar('\n');

		lin.write(streamData, true, str, pszTemp);
		str.AppendChar(' ');
	}

	// Insert the message
	CreateChatRtfMessage(streamData, lin, str);
}
