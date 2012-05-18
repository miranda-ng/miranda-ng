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

#include "chat.h"


// The code for streaming the text is to a large extent copied from
// the srmm module and then modified to fit the chat module.

extern FONTINFO  aFonts[OPTIONS_FONTCOUNT];
extern HICON     hIcons[30];
extern BOOL      SmileyAddInstalled;

static PBYTE pLogIconBmpBits[14];
static int logIconBmpSize[ SIZEOF(pLogIconBmpBits) ];

static int logPixelSY = 0;
static int logPixelSX = 0;

static int EventToIndex(LOGINFO * lin)
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

static int EventToIcon(LOGINFO * lin)
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

static char *Log_SetStyle(int style, int fontindex)
{
	static char szStyle[128];
	mir_snprintf(szStyle, SIZEOF(szStyle), "\\f%u\\cf%u\\ul0\\highlight0\\b%d\\i%d\\fs%u", style, style+1, aFonts[fontindex].lf.lfWeight >= FW_BOLD ? 1 : 0, aFonts[fontindex].lf.lfItalic, 2 * abs(aFonts[fontindex].lf.lfHeight) * 74 / logPixelSY);
	return szStyle;
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
		*buffer = (char *) mir_realloc(*buffer, *cbBufferAlloced);
	}
	va_end(va);
	*cbBufferEnd += charsDone;
}

static int Log_AppendRTF(LOGSTREAMDATA* streamData, BOOL simpleMode, char **buffer, int *cbBufferEnd, int *cbBufferAlloced, const TCHAR *fmt, ...)
{
	va_list va;
	int lineLen, textCharsCount=0;
	TCHAR* line = (TCHAR*)alloca(8001 * sizeof(TCHAR));
	char* d;

	va_start(va, fmt);
	lineLen = _vsntprintf( line, 8000, fmt, va);
	if (lineLen < 0) lineLen = 8000;
	line[lineLen] = 0;
	va_end(va);

	lineLen = lineLen*20 + 8;
	if (*cbBufferEnd + lineLen > *cbBufferAlloced) {
		cbBufferAlloced[0] += (lineLen + 1024 - lineLen % 1024);
		*buffer = (char *) mir_realloc(*buffer, *cbBufferAlloced);
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
		else if (*line == '%' && !simpleMode ) {
			char szTemp[200];

			szTemp[0] = '\0';
			switch ( *++line ) {
			case '\0':
			case '%':
				*d++ = '%';
				break;

			case 'c':
			case 'f':
				if (g_Settings.StripFormat || streamData->bStripFormat)
					line += 2;

				else if ( line[1] != '\0' && line[2] != '\0') {
					TCHAR szTemp3[3], c = *line;
					int col;
					szTemp3[0] = line[1];
					szTemp3[1] = line[2];
					szTemp3[2] = '\0';
					line += 2;

					col = _ttoi(szTemp3);
					col += (OPTIONS_FONTCOUNT + 1);
					mir_snprintf(szTemp, SIZEOF(szTemp), ( c == 'c' ) ? "\\cf%u " : "\\highlight%u ", col);
				}
				break;
			case 'C':
			case 'F':
				if ( !g_Settings.StripFormat && !streamData->bStripFormat) {
					int j = streamData->lin->bIsHighlighted ? 16 : EventToIndex(streamData->lin);
					if ( *line == 'C' )
						mir_snprintf(szTemp, SIZEOF(szTemp), "\\cf%u ", j+1);
					else
						mir_snprintf(szTemp, SIZEOF(szTemp), "\\highlight0 ");
				}
				break;
			case 'b':
			case 'u':
			case 'i':
				if ( !streamData->bStripFormat )
					mir_snprintf(szTemp, SIZEOF(szTemp), (*line == 'u') ? "\\%cl " : "\\%c ", *line );
				break;

			case 'B':
			case 'U':
			case 'I':
				if ( !streamData->bStripFormat ) {
					mir_snprintf( szTemp, SIZEOF(szTemp), (*line == 'U') ? "\\%cl0 " : "\\%c0 ", *line );
					CharLowerA( szTemp );
				}
				break;

			case 'r':
				if ( !streamData->bStripFormat ) {
					int index = EventToIndex(streamData->lin);
					mir_snprintf(szTemp, SIZEOF(szTemp), "%s ", Log_SetStyle(index, index));
				}
				break;
			}

			if ( szTemp[0] ) {
				int iLen = lstrlenA(szTemp);
				memcpy( d, szTemp, iLen );
				d += iLen;
			}
		}
		else if (*line == '\t' && !streamData->bStripFormat) {
			CopyMemory(d, "\\tab ", 5);
			d += 5;
		}
		else if ((*line == '\\' || *line == '{' || *line == '}') && !streamData->bStripFormat) {
			*d++ = '\\';
			*d++ = (char) *line;
		}
		else if (*line > 0 && *line < 128) {
			*d++ = (char) *line;
		}
		#if defined( _UNICODE )
			else d += sprintf(d, "\\u%u ?", (WORD)*line);
		#else
			else d += sprintf(d, "\\'%02x", (BYTE)*line);
		#endif
	}

	*cbBufferEnd = (int) (d - *buffer);
	return textCharsCount;
}

static void AddEventToBuffer(char **buffer, int *bufferEnd, int *bufferAlloced, LOGSTREAMDATA *streamData)
{
	TCHAR szTemp[512], szTemp2[512];
	TCHAR* pszNick = NULL;
	if ( streamData->lin->ptszNick ) {
		if ( g_Settings.LogLimitNames && lstrlen( streamData->lin->ptszNick ) > 20 ) {
			lstrcpyn( szTemp2, streamData->lin->ptszNick, 20 );
			lstrcpyn( szTemp2+20, _T("..."), 4);
		}
		else lstrcpyn( szTemp2, streamData->lin->ptszNick, 511 );

		if ( streamData->lin->ptszUserInfo )
			mir_sntprintf( szTemp, SIZEOF(szTemp), _T("%s (%s)"), szTemp2, streamData->lin->ptszUserInfo );
		else
			mir_sntprintf( szTemp, SIZEOF(szTemp), _T("%s"), szTemp2 );
		pszNick = szTemp;
	}

	if ( streamData && streamData->lin ) {
		switch ( streamData->lin->iType ) {
		case GC_EVENT_MESSAGE:
			if ( streamData->lin->ptszText )
				Log_AppendRTF( streamData, FALSE, buffer, bufferEnd, bufferAlloced, _T("%s"), streamData->lin->ptszText );
			break;
		case GC_EVENT_ACTION:
			if ( streamData->lin->ptszNick && streamData->lin->ptszText) {
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
				Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("Notice from %s: "), pszNick );
				Log_AppendRTF(streamData, FALSE, buffer, bufferEnd, bufferAlloced, _T("%s"), streamData->lin->ptszText);
			}
			break;
		case GC_EVENT_TOPIC:
			if (streamData->lin->ptszText)
				Log_AppendRTF(streamData, FALSE, buffer, bufferEnd, bufferAlloced, TranslateT("The topic is \'%s%s\'"), streamData->lin->ptszText, _T("%r"));
			if (streamData->lin->ptszNick)
				Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced, 
					streamData->lin->ptszUserInfo ? TranslateT(" (set by %s on %s)"): TranslateT(" (set by %s)"),
					streamData->lin->ptszNick, streamData->lin->ptszUserInfo);
			break;
		case GC_EVENT_INFORMATION:
			if (streamData->lin->ptszText)
				Log_AppendRTF(streamData, FALSE, buffer, bufferEnd, bufferAlloced, (streamData->lin->bIsMe) ? _T("--> %s") : _T("%s"), streamData->lin->ptszText);
			break;
		case GC_EVENT_ADDSTATUS:
			if (streamData->lin->ptszNick && streamData->lin->ptszText && streamData->lin->ptszStatus)
				Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("%s enables \'%s\' status for %s"), streamData->lin->ptszText, streamData->lin->ptszStatus, streamData->lin->ptszNick);
			break;
		case GC_EVENT_REMOVESTATUS:
			if (streamData->lin->ptszNick && streamData->lin->ptszText && streamData->lin->ptszStatus)
				Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("%s disables \'%s\' status for %s"), streamData->lin->ptszText , streamData->lin->ptszStatus, streamData->lin->ptszNick);
			break;
}	}	}

TCHAR* MakeTimeStamp( TCHAR* pszStamp, time_t time)
{
	static TCHAR szTime[30];
	_tcsftime(szTime, 29, pszStamp, localtime(&time));
	return szTime;
}

static char* Log_CreateRTF(LOGSTREAMDATA *streamData)
{
	char *buffer, *header;
	int bufferAlloced, bufferEnd, i, me = 0;
	LOGINFO * lin = streamData->lin;
	MODULEINFO *mi = MM_FindModule(streamData->si->pszModule);

	// guesstimate amount of memory for the RTF
	bufferEnd = 0;
	bufferAlloced = streamData->bRedraw ? 1024 * (streamData->si->iEventCount+2) : 2048;
	buffer = (char *) mir_alloc(bufferAlloced);
	buffer[0] = '\0';

	// ### RTF HEADER
	header = mi->pszHeader;

	if (header)
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, header);


	// ### RTF BODY (one iteration per event that should be streamed in)
	while ( lin )
	{
		// filter
		if (streamData->si->iType != GCW_CHATROOM || !streamData->si->bFilterEnabled || (streamData->si->iLogFilterFlags&lin->iType) != 0)
		{
			// create new line, and set font and color
			Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\par%s ", Log_SetStyle(0, 0));

			// Insert icon
			if (lin->iType&g_Settings.dwIconFlags || lin->bIsHighlighted&&g_Settings.dwIconFlags&GC_EVENT_HIGHLIGHT)
			{
				int iIndex = (lin->bIsHighlighted&&g_Settings.dwIconFlags&GC_EVENT_HIGHLIGHT) ? ICON_HIGHLIGHT : EventToIcon(lin);
				Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\f0\\fs14");
				while (bufferAlloced - bufferEnd < logIconBmpSize[0])
					bufferAlloced += 4096;
				buffer = (char *) mir_realloc(buffer, bufferAlloced);
				CopyMemory(buffer + bufferEnd, pLogIconBmpBits[iIndex], logIconBmpSize[iIndex]);
				bufferEnd += logIconBmpSize[iIndex];
			}

			if (g_Settings.TimeStampEventColour)
			{
				// colored timestamps
				static char szStyle[256];
				int iii;
				if (lin->ptszNick && lin->iType == GC_EVENT_MESSAGE)
				{
					iii = lin->bIsHighlighted?16:(lin->bIsMe ? 2 : 1);
					mir_snprintf(szStyle, SIZEOF(szStyle), "\\f0\\cf%u\\ul0\\highlight0\\b%d\\i%d\\fs%u", iii+1, aFonts[0].lf.lfWeight >= FW_BOLD ? 1 : 0, aFonts[0].lf.lfItalic, 2 * abs(aFonts[0].lf.lfHeight) * 74 / logPixelSY);
					Log_Append(&buffer, &bufferEnd, &bufferAlloced, "%s ", szStyle);
				}
				else
				{
					iii = lin->bIsHighlighted?16:EventToIndex(lin);
					mir_snprintf(szStyle, SIZEOF(szStyle), "\\f0\\cf%u\\ul0\\highlight0\\b%d\\i%d\\fs%u", iii+1, aFonts[0].lf.lfWeight >= FW_BOLD ? 1 : 0, aFonts[0].lf.lfItalic, 2 * abs(aFonts[0].lf.lfHeight) * 74 / logPixelSY);
					Log_Append(&buffer, &bufferEnd, &bufferAlloced, "%s ", szStyle);
				}
			}
			else
				Log_Append(&buffer, &bufferEnd, &bufferAlloced, "%s ", Log_SetStyle(0, 0 ));
			// insert a TAB if necessary to put the timestamp in the right position
			if (g_Settings.dwIconFlags)
				Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\tab ");

			//insert timestamp
			if (g_Settings.ShowTime)
			{
				TCHAR szTimeStamp[30], szOldTimeStamp[30];

				lstrcpyn( szTimeStamp, MakeTimeStamp(g_Settings.pszTimeStamp, lin->time), 30);
				lstrcpyn( szOldTimeStamp, MakeTimeStamp(g_Settings.pszTimeStamp, streamData->si->LastTime), 30);
				if ( !g_Settings.ShowTimeIfChanged || streamData->si->LastTime == 0 || lstrcmp(szTimeStamp, szOldTimeStamp )) {
					streamData->si->LastTime = lin->time;
					Log_AppendRTF( streamData, TRUE, &buffer, &bufferEnd, &bufferAlloced, _T("%s"), szTimeStamp );
				}
				Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\tab ");
			}

			// Insert the nick
			if (lin->ptszNick && lin->iType == GC_EVENT_MESSAGE)
			{
				TCHAR pszTemp[300], *p1;

				Log_Append(&buffer, &bufferEnd, &bufferAlloced, "%s ", Log_SetStyle(lin->bIsMe ? 2 : 1, lin->bIsMe ? 2 : 1));
				lstrcpyn(pszTemp, lin->bIsMe ? g_Settings.pszOutgoingNick : g_Settings.pszIncomingNick, 299);
				p1 = _tcsstr(pszTemp, _T("%n"));
				if (p1)
					p1[1] = 's';

				Log_AppendRTF(streamData, TRUE, &buffer, &bufferEnd, &bufferAlloced, pszTemp, lin->ptszNick);
				Log_Append(&buffer, &bufferEnd, &bufferAlloced, " ");
			}

			// Insert the message
			{
				i = lin->bIsHighlighted?16:EventToIndex(lin);
				Log_Append(&buffer, &bufferEnd, &bufferAlloced, "%s ", Log_SetStyle(i, i));
				streamData->lin = lin;
				AddEventToBuffer(&buffer, &bufferEnd, &bufferAlloced, streamData);
			}

		}
		lin = lin->prev;
	}

	// ### RTF END
	Log_Append(&buffer, &bufferEnd, &bufferAlloced, "}");
	return buffer;
}

static DWORD CALLBACK Log_StreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	LOGSTREAMDATA *lstrdat = (LOGSTREAMDATA *) dwCookie;

	if (lstrdat)
	{
		// create the RTF
		if (lstrdat->buffer == NULL)
		{
			lstrdat->bufferOffset = 0;
			lstrdat->buffer = Log_CreateRTF(lstrdat);
			lstrdat->bufferLen = lstrlenA(lstrdat->buffer);
		}

		// give the RTF to the RE control
		*pcb = min(cb, lstrdat->bufferLen - lstrdat->bufferOffset);
		CopyMemory(pbBuff, lstrdat->buffer + lstrdat->bufferOffset, *pcb);
		lstrdat->bufferOffset += *pcb;

		// free stuff if the streaming operation is complete
		if (lstrdat->bufferOffset == lstrdat->bufferLen)
		{
			mir_free(lstrdat->buffer);
			lstrdat->buffer = NULL;
		}
	}

	return 0;
}

void Log_StreamInEvent(HWND hwndDlg,  LOGINFO* lin, SESSION_INFO* si, BOOL bRedraw, BOOL bPhaseTwo)
{
	EDITSTREAM stream;
	LOGSTREAMDATA streamData;
	CHARRANGE oldsel, sel, newsel;
	POINT point ={0};
	SCROLLINFO scroll;
	WPARAM wp;
	HWND hwndRich;

	if (hwndDlg == 0 || lin == 0 || si == 0)
		return;

	hwndRich = GetDlgItem(hwndDlg, IDC_LOG);
	ZeroMemory(&streamData, sizeof(LOGSTREAMDATA));
	streamData.hwnd = hwndRich;
	streamData.si = si;
	streamData.lin = lin;
	streamData.bStripFormat = FALSE;

	//	bPhaseTwo = bRedraw && bPhaseTwo;

	if (bRedraw || si->iType != GCW_CHATROOM || !si->bFilterEnabled || (si->iLogFilterFlags&lin->iType) != 0)
	{
		BOOL bFlag = FALSE;

		ZeroMemory(&stream, sizeof(stream));
		stream.pfnCallback = Log_StreamCallback;
		stream.dwCookie = (DWORD_PTR) & streamData;
		scroll.cbSize= sizeof(SCROLLINFO);
		scroll.fMask= SIF_RANGE | SIF_POS|SIF_PAGE;
		GetScrollInfo(GetDlgItem(hwndDlg, IDC_LOG), SB_VERT, &scroll);
		SendMessage(hwndRich, EM_GETSCROLLPOS, 0, (LPARAM) &point);

		// do not scroll to bottom if there is a selection
		SendMessage(hwndRich, EM_EXGETSEL, 0, (LPARAM) &oldsel);
		if (oldsel.cpMax != oldsel.cpMin)
			SendMessage(hwndRich, WM_SETREDRAW, FALSE, 0);

		//set the insertion point at the bottom
		sel.cpMin = sel.cpMax = GetRichTextLength(hwndRich);
		SendMessage(hwndRich, EM_EXSETSEL, 0, (LPARAM) & sel);

		// fix for the indent... must be a M$ bug
		if (sel.cpMax == 0)
			bRedraw = TRUE;

		// should the event(s) be appended to the current log
		wp = bRedraw?SF_RTF:SFF_SELECTION|SF_RTF;

		//get the number of pixels per logical inch
		if (bRedraw)
		{
			HDC hdc;
			hdc = GetDC(NULL);
			logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
			logPixelSX = GetDeviceCaps(hdc, LOGPIXELSX);
			ReleaseDC (NULL, hdc);
			SendMessage(hwndRich, WM_SETREDRAW, FALSE, 0);
			bFlag = TRUE;
			//			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}

		// stream in the event(s)
		streamData.lin = lin;
		streamData.bRedraw = bRedraw;
		SendMessage(hwndRich, EM_STREAMIN, wp, (LPARAM) & stream);

		// do smileys
		if (SmileyAddInstalled && (bRedraw
			|| (lin->ptszText
			&& lin->iType != GC_EVENT_JOIN
			&& lin->iType != GC_EVENT_NICK
			&& lin->iType != GC_EVENT_ADDSTATUS
			&& lin->iType != GC_EVENT_REMOVESTATUS )))
		{
			SMADD_RICHEDIT3 sm = {0};

			newsel.cpMax = -1;
			newsel.cpMin = sel.cpMin;
			if (newsel.cpMin < 0)
				newsel.cpMin = 0;
			ZeroMemory(&sm, sizeof(sm));
			sm.cbSize = sizeof(sm);
			sm.hwndRichEditControl = hwndRich;
			sm.Protocolname = si->pszModule;
			sm.rangeToReplace = bRedraw?NULL:&newsel;
			sm.disableRedraw = TRUE;
			sm.hContact = si->hContact;
			CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM)&sm);
		}

		// scroll log to bottom if the log was previously scrolled to bottom, else restore old position
		if (bRedraw ||  (UINT)scroll.nPos >= (UINT)scroll.nMax-scroll.nPage-5 || scroll.nMax-scroll.nMin-scroll.nPage < 50)
		{
			SendMessage(GetParent(hwndRich), GC_SCROLLTOBOTTOM, 0, 0);
		}
		else
			SendMessage(hwndRich, EM_SETSCROLLPOS, 0, (LPARAM) &point);

		// do we need to restore the selection
		if (oldsel.cpMax != oldsel.cpMin)
		{
			SendMessage(hwndRich, EM_EXSETSEL, 0, (LPARAM) & oldsel);
			SendMessage(hwndRich, WM_SETREDRAW, TRUE, 0);
			InvalidateRect(hwndRich, NULL, TRUE);
		}

		// need to invalidate the window
		if (bFlag)
		{
			sel.cpMin = sel.cpMax = GetRichTextLength(hwndRich);
			SendMessage(hwndRich, EM_EXSETSEL, 0, (LPARAM) & sel);
			SendMessage(hwndRich, WM_SETREDRAW, TRUE, 0);
			InvalidateRect(hwndRich, NULL, TRUE);
}	}	}

char * Log_CreateRtfHeader(MODULEINFO * mi)
{
	char *buffer;
	int bufferAlloced, bufferEnd, i = 0;

	// guesstimate amount of memory for the RTF header
	bufferEnd = 0;
	bufferAlloced = 4096;
	buffer = (char *) mir_realloc(mi->pszHeader, bufferAlloced);
	buffer[0] = '\0';


	//get the number of pixels per logical inch
	{
		HDC hdc;
		hdc = GetDC(NULL);
		logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		logPixelSX = GetDeviceCaps(hdc, LOGPIXELSX);
		ReleaseDC(NULL, hdc);
	}

	// ### RTF HEADER

	// font table
	Log_Append(&buffer, &bufferEnd, &bufferAlloced, "{\\rtf1\\ansi\\deff0{\\fonttbl");
	for (i = 0; i < OPTIONS_FONTCOUNT; i++)
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "{\\f%u\\fnil\\fcharset%u" TCHAR_STR_PARAM ";}", i, aFonts[i].lf.lfCharSet, aFonts[i].lf.lfFaceName);

	// colour table
	Log_Append(&buffer, &bufferEnd, &bufferAlloced, "}{\\colortbl ;");

	for (i = 0; i < OPTIONS_FONTCOUNT; i++)
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(aFonts[i].color), GetGValue(aFonts[i].color), GetBValue(aFonts[i].color));

	for(i = 0; i < mi->nColorCount; i++)
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(mi->crColors[i]), GetGValue(mi->crColors[i]), GetBValue(mi->crColors[i]));

	// new paragraph
	Log_Append(&buffer, &bufferEnd, &bufferAlloced, "}\\pard");

	// set tabs and indents
	{
		int iIndent = 0;

		if (g_Settings.dwIconFlags)
		{
			iIndent += (14*1440)/logPixelSX;
			Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\tx%u", iIndent);
		}
		if (g_Settings.ShowTime)
		{
			int iSize = (g_Settings.LogTextIndent*1440)/logPixelSX;
			Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\tx%u", iIndent + iSize );
			if (g_Settings.LogIndentEnabled)
				iIndent += iSize;
		}
		/*
		{ // text indent
		int iSize = (135*1440)/logPixelSX;
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\tx%u", iIndent + iSize );
		if (g_Settings.LogIndentEnabled)
		iIndent += iSize;

		}
		*/
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\fi-%u\\li%u", iIndent, iIndent);
	}
	return buffer;
}

#define RTFPICTHEADERMAXSIZE   78
void LoadMsgLogBitmaps(void)
{
	HICON hIcon;
	HBITMAP hBmp, hoBmp;
	HDC hdc, hdcMem;
	BITMAPINFOHEADER bih = { 0 };
	int widthBytes, i;
	RECT rc;
	HBRUSH hBkgBrush;
	int rtfHeaderSize;
	PBYTE pBmpBits;

	hBkgBrush = CreateSolidBrush(DBGetContactSettingDword(NULL, "Chat", "ColorLogBG", GetSysColor(COLOR_WINDOW)));
	bih.biSize = sizeof(bih);
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;
	bih.biHeight = 10; //GetSystemMetrics(SM_CYSMICON);
	bih.biPlanes = 1;
	bih.biWidth = 10; //GetSystemMetrics(SM_CXSMICON);
	widthBytes = ((bih.biWidth * bih.biBitCount + 31) >> 5) * 4;
	rc.top = rc.left = 0;
	rc.right = bih.biWidth;
	rc.bottom = bih.biHeight;
	hdc = GetDC(NULL);
	hBmp = CreateCompatibleBitmap(hdc, bih.biWidth, bih.biHeight);
	hdcMem = CreateCompatibleDC(hdc);
	pBmpBits = (PBYTE) mir_alloc(widthBytes * bih.biHeight);
	for (i = 0; i < SIZEOF(pLogIconBmpBits); i++) {
		hIcon = hIcons[i];
		pLogIconBmpBits[i] = (PBYTE) mir_alloc(RTFPICTHEADERMAXSIZE + (bih.biSize + widthBytes * bih.biHeight) * 2);
		rtfHeaderSize = sprintf(pLogIconBmpBits[i], "{\\pict\\dibitmap0\\wbmbitspixel%u\\wbmplanes1\\wbmwidthbytes%u\\picw%u\\pich%u ", bih.biBitCount, widthBytes, bih.biWidth, bih.biHeight);
		hoBmp = (HBITMAP) SelectObject(hdcMem, hBmp);
		FillRect(hdcMem, &rc, hBkgBrush);
		DrawIconEx(hdcMem, 0, 0, hIcon, bih.biWidth, bih.biHeight, 0, NULL, DI_NORMAL);
		SelectObject(hdcMem, hoBmp);
		GetDIBits(hdc, hBmp, 0, bih.biHeight, pBmpBits, (BITMAPINFO *) & bih, DIB_RGB_COLORS);
		{
			int n;
			for (n = 0; n < sizeof(BITMAPINFOHEADER); n++)
				sprintf(pLogIconBmpBits[i] + rtfHeaderSize + n * 2, "%02X", ((PBYTE) & bih)[n]);
			for (n = 0; n < widthBytes * bih.biHeight; n += 4)
				sprintf(pLogIconBmpBits[i] + rtfHeaderSize + (bih.biSize + n) * 2, "%02X%02X%02X%02X", pBmpBits[n], pBmpBits[n + 1], pBmpBits[n + 2], pBmpBits[n + 3]);
		}
		logIconBmpSize[i] = rtfHeaderSize + (bih.biSize + widthBytes * bih.biHeight) * 2 + 1;
		pLogIconBmpBits[i][logIconBmpSize[i] - 1] = '}';
	}
	mir_free(pBmpBits);
	DeleteDC(hdcMem);
	DeleteObject(hBmp);
	ReleaseDC(NULL, hdc);
	DeleteObject(hBkgBrush);
}

void FreeMsgLogBitmaps(void)
{
	int i;
	for (i = 0; i < SIZEOF(pLogIconBmpBits); i++)
		mir_free(pLogIconBmpBits[i]);
}
