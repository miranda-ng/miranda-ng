/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
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

static int EventToIndex(LOGINFO *lin)
{
	switch (lin->iType) {
	case GC_EVENT_MESSAGE:
		if (lin->bIsMe)
			return 10;
		else
			return 9;

	case GC_EVENT_JOIN:
		return 3;
	case GC_EVENT_PART:
		return 4;
	case GC_EVENT_QUIT:
		return 5;
	case GC_EVENT_NICK:
		return 7;
	case GC_EVENT_KICK:
		return 6;
	case GC_EVENT_NOTICE:
		return 8;
	case GC_EVENT_TOPIC:
		return 11;
	case GC_EVENT_INFORMATION:
		return 12;
	case GC_EVENT_ADDSTATUS:
		return 13;
	case GC_EVENT_REMOVESTATUS:
		return 14;
	case GC_EVENT_ACTION:
		return 15;
	}
	return 0;
}

static uint8_t EventToSymbol(LOGINFO *lin)
{
	switch (lin->iType) {
	case GC_EVENT_MESSAGE:
		return (lin->bIsMe) ? 0x37 : 0x38;
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

static int EventToIcon(LOGINFO *lin)
{
	switch (lin->iType) {
	case GC_EVENT_MESSAGE:
		if (lin->bIsMe)
			return ICON_MESSAGEOUT;
		else
			return ICON_MESSAGE;

	case GC_EVENT_JOIN:
		return ICON_JOIN;
	case GC_EVENT_PART:
		return ICON_PART;
	case GC_EVENT_QUIT:
		return ICON_QUIT;
	case GC_EVENT_NICK:
		return ICON_NICK;
	case GC_EVENT_KICK:
		return ICON_KICK;
	case GC_EVENT_NOTICE:
		return ICON_NOTICE;
	case GC_EVENT_TOPIC:
		return ICON_TOPIC;
	case GC_EVENT_INFORMATION:
		return ICON_INFO;
	case GC_EVENT_ADDSTATUS:
		return ICON_ADDSTATUS;
	case GC_EVENT_REMOVESTATUS:
		return ICON_REMSTATUS;
	case GC_EVENT_ACTION:
		return ICON_ACTION;
	}
	return 0;
}

static void Log_AppendRTF(LOGSTREAMDATA *streamData, bool simpleMode, CMStringA &str, const wchar_t *line)
{
	int textCharsCount = 0;

	CMStringA res;

	for (; *line; line++, textCharsCount++) {
		if (*line == '\r' && line[1] == '\n') {
			res.Append("\\par ");
			line++;
		}
		else if (*line == '\n') {
			res.Append("\\line ");
		}
		else if (*line == '%' && !simpleMode) {
			char szTemp[200]; szTemp[0] = '\0';
			switch (*++line) {
			case '\0':
			case '%':
				res.AppendChar('%');
				break;

			case 'c':
			case 'f':
				if (g_Settings.bStripFormat || streamData->bStripFormat)
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
					res.AppendFormat((c == 'c') ? "\\cf%u " : "\\highlight%u ", col);
				}
				break;
			case 'C':
			case 'F':
				if (!g_Settings.bStripFormat && !streamData->bStripFormat) {
					int j = streamData->lin->bIsHighlighted ? 16 : EventToIndex(streamData->lin);
					if (*line == 'C')
						res.AppendFormat("\\cf%u ", j + 1);
					else
						res.Append("\\highlight0 ");
				}
				break;
			case 'b':
			case 'u':
			case 'i':
				if (!streamData->bStripFormat)
					res.AppendFormat((*line == 'u') ? "\\%cl " : "\\%c ", *line);
				break;

			case 'B':
			case 'U':
			case 'I':
				if (!streamData->bStripFormat)
					res.AppendFormat((*line == 'U') ? "\\%cl0 " : "\\%c0 ", tolower(*line));
				break;

			case 'r':
				if (!streamData->bStripFormat) {
					int index = EventToIndex(streamData->lin);
					res.AppendFormat("%s ", g_chatApi.Log_SetStyle(index));
				}
				break;
			}
		}
		else if (*line == '\t' && !streamData->bStripFormat) {
			res.Append("\\tab ");
		}
		else if ((*line == '\\' || *line == '{' || *line == '}') && !streamData->bStripFormat) {
			res.AppendChar('\\');
			res.AppendChar(*line);
		}
		else if (*line > 0 && *line < 128) {
			res.AppendChar((char)*line);
		}
		else res.AppendFormat("\\u%u ?", (uint16_t)*line);
	}

	str += res;
}

static void AddEventToBuffer(CMStringA &str, LOGSTREAMDATA *streamData)
{
	if (streamData == nullptr || streamData->lin == nullptr)
		return;

	CMStringW wszCaption;
	bool bTextUsed = Chat_GetDefaultEventDescr(streamData->si, streamData->lin, wszCaption);
	if (!wszCaption.IsEmpty())
		Log_AppendRTF(streamData, !bTextUsed, str, wszCaption);
	if (!bTextUsed && streamData->lin->ptszText) {
		if (!wszCaption.IsEmpty())
			Log_AppendRTF(streamData, false, str, L" ");
		Log_AppendRTF(streamData, false, str, streamData->lin->ptszText);
	}
}

char* Log_CreateRtfHeader(void)
{
	// get the number of pixels per logical inch
	if (g_chatApi.logPixelSY == 0) {
		HDC hdc = GetDC(nullptr);
		g_chatApi.logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		g_chatApi.logPixelSX = GetDeviceCaps(hdc, LOGPIXELSX);
		ReleaseDC(nullptr, hdc);
	}

	// ### RTF HEADER

	// font table
	CMStringA str("{\\rtf1\\ansi\\deff0{\\fonttbl");
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

	return str.Detach();
}

char* Log_CreateRTF(LOGSTREAMDATA *streamData)
{
	LOGINFO *lin = streamData->lin;
	SESSION_INFO *si = streamData->si;
	MODULEINFO *mi = si->pMI;

	// ### RTF HEADER

	if (mi->pszHeader == nullptr)
		mi->pszHeader = Log_CreateRtfHeader();

	char *header = mi->pszHeader;
	streamData->crCount = 0;

	CMStringA str;
	if (header)
		str.Append(header);

	// ### RTF BODY (one iteration per event that should be streamed in)
	while (lin) {
		// filter
		if ((si->iType != GCW_CHATROOM && si->iType != GCW_PRIVMESS) || !si->pDlg->m_bFilterEnabled || (si->pDlg->m_iLogFilterFlags & lin->iType) != 0) {
			if (lin->next != nullptr)
				str.Append("\\par ");

			if (streamData->dat->m_bDividerWanted) {
				static char szStyle_div[128] = "\0";
				if (szStyle_div[0] == 0)
					mir_snprintf(szStyle_div, "\\f%u\\cf%u\\ul0\\b%d\\i%d\\fs%u", 17, 18, 0, 0, 5);

				if (lin->prev || !streamData->bRedraw)
					str.AppendFormat("\\qc\\sl-1\\highlight%d %s ---------------------------------------------------------------------------------------\\par ", 18, szStyle_div);
				streamData->dat->m_bDividerWanted = false;
			}
			// create new line, and set font and color
			str.AppendFormat("\\ql\\sl0%s ", g_chatApi.Log_SetStyle(0));
			str.AppendFormat("\\v~-+%p+-~\\v0 ", lin);

			// Insert icon
			if (g_Settings.bLogSymbols)                // use symbols
				str.AppendFormat("%s %c", g_chatApi.Log_SetStyle(17), EventToSymbol(lin));
			else if (g_Settings.dwIconFlags) {
				int iIndex = lin->bIsHighlighted ? ICON_HIGHLIGHT : EventToIcon(lin);
				str.Append("\\f0\\fs14");
				str.Append(g_chatApi.pLogIconBmpBits[iIndex]);
			}

			if (g_Settings.bTimeStampEventColour) {
				// colored timestamps
				static char szStyle[256];
				LOGFONT &F = g_chatApi.aFonts[0].lf;
				int iii;
				if (lin->ptszNick && lin->iType == GC_EVENT_MESSAGE) {
					iii = lin->bIsHighlighted ? 16 : (lin->bIsMe ? 2 : 1);
					mir_snprintf(szStyle, "\\f0\\cf%u\\ul0\\highlight0\\b%d\\i%d\\ul%d\\fs%u",
						iii + 1, F.lfWeight >= FW_BOLD ? 1 : 0, F.lfItalic, F.lfUnderline, 2 * abs(F.lfHeight) * 74 / g_chatApi.logPixelSY);
					str.Append(szStyle);
				}
				else {
					iii = lin->bIsHighlighted ? 16 : EventToIndex(lin);
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

				wcsncpy_s(szTimeStamp, g_chatApi.MakeTimeStamp(g_Settings.pszTimeStamp, lin->time), _TRUNCATE);
				wcsncpy_s(szOldTimeStamp, g_chatApi.MakeTimeStamp(g_Settings.pszTimeStamp, si->LastTime), _TRUNCATE);
				if (!g_Settings.bShowTimeIfChanged || si->LastTime == 0 || mir_wstrcmp(szTimeStamp, szOldTimeStamp)) {
					si->LastTime = lin->time;
					Log_AppendRTF(streamData, TRUE, str, szTimeStamp);
				}
				str.Append("\\tab ");
			}

			// Insert the nick
			if (lin->ptszNick && lin->iType == GC_EVENT_MESSAGE) {
				char pszIndicator[3] = "\0\0";
				int  crNickIndex = 0;

				if (g_Settings.bLogClassicIndicators || g_Settings.bColorizeNicksInLog)
					pszIndicator[0] = GetIndicator(si, lin->ptszNick, &crNickIndex);

				str.Append(g_chatApi.Log_SetStyle(lin->bIsMe ? 2 : 1));
				str.AppendChar(' ');

				if (g_Settings.bLogClassicIndicators)
					str.Append(pszIndicator);

				CMStringW pszTemp(lin->bIsMe ? g_Settings.pszOutgoingNick : g_Settings.pszIncomingNick);
				if (!lin->bIsMe) {
					if (g_Settings.bClickableNicks)
						pszTemp.Replace(L"%n", CLICKNICK_BEGIN L"%n" CLICKNICK_END);

					if (g_Settings.bColorizeNicksInLog && pszIndicator[0])
						str.AppendFormat("\\cf%u ", OPTIONS_FONTCOUNT + Utils::rtf_clrs.getCount() + streamData->crCount + crNickIndex);
				}
				pszTemp.Replace(L"%n", lin->ptszNick);

				if (g_Settings.bNewLineAfterNames)
					pszTemp.AppendChar('\n');

				Log_AppendRTF(streamData, TRUE, str, pszTemp);
				str.AppendChar(' ');
			}

			// Insert the message
			str.Append(g_chatApi.Log_SetStyle(lin->bIsHighlighted ? 16 : EventToIndex(lin)));
			str.AppendChar(' ');

			streamData->lin = lin;
			AddEventToBuffer(str, streamData);
		}
		lin = lin->prev;
	}

	// ### RTF END
	if (streamData->bRedraw)
		str.Append("\\par}");
	else
		str.Append("}");
	return str.Detach();
}
