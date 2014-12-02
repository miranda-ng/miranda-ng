/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (c) 2012-14 Miranda NG project,
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

#include "..\commonheaders.h"

/*
 * The code for streaming the text is to a large extent copied from
 * the srmm module and then modified to fit the chat module.
 */

static char *szDivider = "\\strike----------------------------------------------------------------------------\\strike0";

/*
 * ieview MUC support - mostly from scriver
 */

/*
static char* u2a( const wchar_t* src, int codepage ) {
	int cbLen = WideCharToMultiByte( codepage, 0, src, -1, NULL, 0, NULL, NULL );
	char* result = ( char* )mir_alloc( cbLen+1 );
	if ( result == NULL )
		return NULL;

	WideCharToMultiByte( codepage, 0, src, -1, result, cbLen, NULL, NULL );
	result[ cbLen ] = 0;
	return result;
}

static char* t2acp( const TCHAR* src, int codepage ) {
	return u2a( src, codepage );
}

static TCHAR *a2tcp(const char *text, int cp) {
	if ( text != NULL ) {
		int cbLen = MultiByteToWideChar( cp, 0, text, -1, NULL, 0 );
		TCHAR* result = ( TCHAR* )mir_alloc( sizeof(TCHAR)*( cbLen+1 ));
		if ( result == NULL )
			return NULL;
		MultiByteToWideChar(cp, 0, text, -1, result, cbLen);
		return result;
	}
	return NULL;
}

static int Log_AppendIEView(LOGSTREAMDATA* streamData, BOOL simpleMode, TCHAR **buffer, int *cbBufferEnd, int *cbBufferAlloced, const TCHAR *fmt, ...)
{
	va_list va;
	int lineLen, textCharsCount=0;
	TCHAR* line = (TCHAR*)_alloca( 8001 * sizeof(TCHAR));
	TCHAR* d;
	MODULEINFO *mi = pci->MM_FindModule(streamData->si->pszModule);

	va_start(va, fmt);
	lineLen = mir_vsntprintf( line, 8000, fmt, va);
	if (lineLen < 0)
		return 0;
	line[lineLen] = 0;
	va_end(va);
	lineLen = lineLen*9 + 8;
	if (*cbBufferEnd + lineLen > *cbBufferAlloced) {
		cbBufferAlloced[0] += (lineLen + 1024 - lineLen % 1024);
		*buffer = (TCHAR*) mir_realloc(*buffer, *cbBufferAlloced * sizeof(TCHAR));
	}

	d = *buffer + *cbBufferEnd;

	for (; *line; line++, textCharsCount++) {
		if (*line == '%' && !simpleMode ) {
			TCHAR szTemp[200];

			szTemp[0] = '\0';
			switch ( *++line ) {
			case '\0':
			case '%':
				*d++ = '%';
				break;

			case 'c':
			case 'f':
				if (!g_Settings.bStripFormat && !streamData->bStripFormat) {
					if ( line[1] != '\0' && line[2] != '\0') {
						TCHAR szTemp3[3], c = *line;
						int col;
						szTemp3[0] = line[1];
						szTemp3[1] = line[2];
						szTemp3[2] = '\0';
						col = _ttoi(szTemp3);
						mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%%%c#%02X%02X%02X"), c, GetRValue(mi->crColors[col]), GetGValue(mi->crColors[col]), GetBValue(mi->crColors[col]));
					}
				}
				line += 2;
				break;
			case 'C':
			case 'F':
				if ( !g_Settings.bStripFormat && !streamData->bStripFormat) {
					mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%%%c"), *line );
				}
				break;
			case 'b':
			case 'u':
			case 'i':
			case 'B':
			case 'U':
			case 'I':
			case 'r':
				if ( !streamData->bStripFormat ) {
					mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%%%c"), *line );
				}
				break;
			}

			if ( szTemp[0] ) {
				size_t iLen = mir_tstrlen(szTemp);
				memcpy( d, szTemp, iLen * sizeof(TCHAR));
				d += iLen;
			}
		}
		else if (*line == '%') {
			*d++ = '%';
			*d++ = (char) *line;
		}
		else {
			*d++ = (TCHAR) *line;
		}
	}
	*d = '\0';
	*cbBufferEnd = (int) (d - *buffer);
	return textCharsCount;
}

static void AddEventTextToBufferIEView(TCHAR **buffer, int *bufferEnd, int *bufferAlloced, LOGSTREAMDATA *streamData)
{
	if (streamData->lin->ptszText)
		Log_AppendIEView(streamData, FALSE, buffer, bufferEnd, bufferAlloced, _T(": %s"), streamData->lin->ptszText);
}

static void AddEventToBufferIEView(TCHAR **buffer, int *bufferEnd, int *bufferAlloced, LOGSTREAMDATA *streamData, TCHAR *pszNick)
{

 	if ( streamData && streamData->lin ) {
		switch ( streamData->lin->iType ) {
		case GC_EVENT_MESSAGE:
			if ( streamData->lin->ptszText ) {
				TCHAR *ptszTemp = NULL;
				TCHAR *ptszText = streamData->lin->ptszText;
				if (streamData->dat->codePage != CP_ACP) {
					char *aText = t2acp(streamData->lin->ptszText, CP_ACP);
					ptszText = ptszTemp = a2tcp(aText, streamData->dat->codePage);
					mir_free(aText);
				}
				Log_AppendIEView( streamData, FALSE, buffer, bufferEnd, bufferAlloced, _T("%s"), ptszText );
				mir_free(ptszTemp);
			}
			break;
		case GC_EVENT_ACTION:
			if ( pszNick && streamData->lin->ptszText) {
				Log_AppendIEView(streamData, TRUE, buffer, bufferEnd, bufferAlloced, _T("%s "), streamData->lin->ptszNick);
				Log_AppendIEView(streamData, FALSE, buffer, bufferEnd, bufferAlloced, _T("%s"), streamData->lin->ptszText);
			}
			break;
		case GC_EVENT_JOIN:
			if (pszNick) {
				if (!streamData->lin->bIsMe)
				 	Log_AppendIEView(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("%s has joined"), pszNick);
				else
					Log_AppendIEView(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("You have joined %s"), streamData->si->ptszName);
			}
			break;
		case GC_EVENT_PART:
			if (pszNick)
				Log_AppendIEView(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("%s has left"), pszNick);
			AddEventTextToBufferIEView(buffer, bufferEnd, bufferAlloced, streamData);
			break;
		case GC_EVENT_QUIT:
			if (pszNick)
				Log_AppendIEView(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("%s has disconnected"), pszNick);
			AddEventTextToBufferIEView(buffer, bufferEnd, bufferAlloced, streamData);
			break;
		case GC_EVENT_NICK:
			if (pszNick && streamData->lin->ptszText) {
				if (!streamData->lin->bIsMe)
					Log_AppendIEView(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("%s is now known as %s"), pszNick, streamData->lin->ptszText);
				else
					Log_AppendIEView(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("You are now known as %s"), streamData->lin->ptszText);
			}
			break;
		case GC_EVENT_KICK:
			if (pszNick && streamData->lin->ptszStatus)
				Log_AppendIEView(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("%s kicked %s"), streamData->lin->ptszStatus, streamData->lin->ptszNick);
			AddEventTextToBufferIEView(buffer, bufferEnd, bufferAlloced, streamData);
			break;
		case GC_EVENT_NOTICE:
			if (pszNick && streamData->lin->ptszText) {
				Log_AppendIEView(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("Notice from %s"), pszNick );
				AddEventTextToBufferIEView(buffer, bufferEnd, bufferAlloced, streamData);
			}
			break;
		case GC_EVENT_TOPIC:
			if (streamData->lin->ptszText)
				Log_AppendIEView(streamData, FALSE, buffer, bufferEnd, bufferAlloced, TranslateT("The topic is '%s%s'"), streamData->lin->ptszText, _T("%r"));
			if (pszNick)
				Log_AppendIEView(streamData, TRUE, buffer, bufferEnd, bufferAlloced,
					streamData->lin->ptszUserInfo ? TranslateT(" (set by %s on %s)"): TranslateT(" (set by %s)"),
					pszNick, streamData->lin->ptszUserInfo);
			break;
		case GC_EVENT_INFORMATION:
			if (streamData->lin->ptszText)
				Log_AppendIEView(streamData, FALSE, buffer, bufferEnd, bufferAlloced, (streamData->lin->bIsMe) ? _T("--> %s") : _T("%s"), streamData->lin->ptszText);
			break;
		case GC_EVENT_ADDSTATUS:
			if (pszNick && streamData->lin->ptszText && streamData->lin->ptszStatus)
				Log_AppendIEView(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("%s enables '%s' status for %s"), streamData->lin->ptszText, streamData->lin->ptszStatus, streamData->lin->ptszNick);
			break;
		case GC_EVENT_REMOVESTATUS:
			if (pszNick && streamData->lin->ptszText && streamData->lin->ptszStatus)
				Log_AppendIEView(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("%s disables '%s' status for %s"), streamData->lin->ptszText , streamData->lin->ptszStatus, streamData->lin->ptszNick);
			break;
		}
	}
}

static void LogEventIEView(LOGSTREAMDATA *streamData, TCHAR *ptszNick)
{
	TCHAR *buffer = NULL;
	int bufferEnd = 0;
	int bufferAlloced = 0;
	IEVIEWEVENTDATA ied;
	IEVIEWEVENT event;
	memset(&event, 0, sizeof(event));
	event.cbSize = sizeof(event);
	event.dwFlags = 0;
	event.hwnd = streamData->dat->hwndIEView ? streamData->dat->hwndIEView : streamData->dat->hwndHPP;
	event.hContact = streamData->dat->hContact;
	event.codepage = streamData->dat->codePage;
	event.pszProto = streamData->si->pszModule;
	event.iType = IEE_LOG_MEM_EVENTS;
	event.eventData = &ied;
	event.count = 1;

	memset(&ied, 0, sizeof(ied));
	AddEventToBufferIEView(&buffer, &bufferEnd, &bufferAlloced, streamData, ptszNick);
	ied.ptszNick = ptszNick;
	ied.ptszText = buffer;
	ied.time = streamData->lin->time;
	ied.bIsMe = streamData->lin->bIsMe;

	switch ( streamData->lin->iType ) {
		case GC_EVENT_MESSAGE:
			ied.iType = IEED_GC_EVENT_MESSAGE;
			ied.dwData = IEEDD_GC_SHOW_NICK;
			break;
		case GC_EVENT_ACTION:
			ied.iType = IEED_GC_EVENT_ACTION;
			break;
		case GC_EVENT_JOIN:
			ied.iType = IEED_GC_EVENT_JOIN;
			break;
		case GC_EVENT_PART:
			ied.iType = IEED_GC_EVENT_PART;
			break;
		case GC_EVENT_QUIT:
			ied.iType = IEED_GC_EVENT_QUIT;
			break;
		case GC_EVENT_NICK:
			ied.iType = IEED_GC_EVENT_NICK;
			break;
		case GC_EVENT_KICK:
			ied.iType = IEED_GC_EVENT_KICK;
			break;
		case GC_EVENT_NOTICE:
			ied.iType = IEED_GC_EVENT_NOTICE;
			break;
		case GC_EVENT_TOPIC:
			ied.iType = IEED_GC_EVENT_TOPIC;
			break;
		case GC_EVENT_INFORMATION:
			ied.iType = IEED_GC_EVENT_INFORMATION;
			break;
		case GC_EVENT_ADDSTATUS:
			ied.iType = IEED_GC_EVENT_ADDSTATUS;
			break;
		case GC_EVENT_REMOVESTATUS:
			ied.iType = IEED_GC_EVENT_REMOVESTATUS;
			break;
	}
	ied.dwData |= g_Settings.bShowTime ? IEEDD_GC_SHOW_TIME : 0;
	ied.dwData |= IEEDD_GC_SHOW_ICON;
	ied.dwFlags = IEEDF_UNICODE_TEXT | IEEDF_UNICODE_NICK | IEEDF_UNICODE_TEXT2;
	ied.next = NULL;
	CallService(streamData->dat->hwndIEView ? MS_IEVIEW_EVENT : MS_HPP_EG_EVENT, 0, (LPARAM)&event);
	mir_free(buffer);
}

*/

static int EventToIndex(LOGINFO * lin)
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

static BYTE EventToSymbol(LOGINFO *lin)
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

static int EventToIcon(LOGINFO * lin)
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

/* replace pattern `ptrn' with the string `rplc' in string `src' points to */
static TCHAR * _tcsrplc(TCHAR **src, const TCHAR *ptrn, const TCHAR *rplc)
{
	size_t lSrc, lPtrn, lRplc;
	TCHAR *tszFound, *tszTail;

	lSrc = mir_tstrlen(*src);
	lPtrn = mir_tstrlen(ptrn);
	lRplc = mir_tstrlen(rplc);
	if (lPtrn && lSrc && lSrc >= lPtrn && (tszFound = _tcsstr(*src, ptrn)) != NULL) {
		if (lRplc > lPtrn)
			*src = (TCHAR*) mir_realloc((void*) * src,
									 sizeof(TCHAR) * (lSrc + lRplc - lPtrn + 1));
		if (tszTail = (TCHAR*) mir_alloc(sizeof(TCHAR) *
									   (lSrc - (tszFound - *src) - lPtrn + 1))) {
			/* save tail */
			_tcscpy(tszTail, tszFound + lPtrn);
			/* write replacement string */
			_tcscpy(tszFound, rplc);
			/* write tail */
			_tcscpy(tszFound + lRplc, tszTail);
			mir_free((void*) tszTail);
		}
	}
	return *src;
}

/*
 * replace pattern `ptrn' with the string `rplc' in string `src',
 * `src' is supposed to be `n' character long (or no checking is done if n < 0).
 * This function is useful for statically allocated buffers
 */
static TCHAR * _tcsnrplc(TCHAR *src, size_t n, const TCHAR *ptrn, const TCHAR *rplc)
{
	size_t lSrc, lPtrn, lRplc;
	TCHAR *tszFound, *tszTail;

	lSrc = mir_tstrlen(src);
	lPtrn = mir_tstrlen(ptrn);
	lRplc = mir_tstrlen(rplc);
	if (lPtrn && lSrc && lSrc >= lPtrn && /* lengths are ok */
			(tszFound = _tcsstr(src, ptrn)) != NULL && /* pattern was found in string */
			(n < 0 || lSrc - lPtrn + lRplc < n) && /* there is enough room in the string */
			(tszTail = (TCHAR*) mir_alloc(sizeof(TCHAR) *
										(lSrc - (tszFound - src) - lPtrn + 1))) != NULL) {
		/* save tail */
		_tcscpy(tszTail, tszFound + lPtrn);
		/* write replacement string */
		_tcscpy(tszFound, rplc);
		/* write tail */
		_tcscpy(tszFound + lRplc, tszTail);
		mir_free(tszTail);
	}
	return src;
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
	int textCharsCount = 0;
	char *d;
	TCHAR *line = (TCHAR*)_alloca(8001 * sizeof(TCHAR));

	va_start(va, fmt);
	int lineLen = mir_vsntprintf(line, 8000, fmt, va);
	if (lineLen < 0)
		lineLen = 8000;
	line[lineLen] = 0;
	va_end(va);

	lineLen = lineLen * 20 + 8;
	if (*cbBufferEnd + lineLen > *cbBufferAlloced) {
		cbBufferAlloced[0] += (lineLen + 1024 - lineLen % 1024);
		if ((d = (char *) mir_realloc(*buffer, *cbBufferAlloced)) == 0)
			return 0;
		*buffer = d;
	}

	d = *buffer + *cbBufferEnd;

	for (; *line; line++, textCharsCount++) {
		if (*line == '\r' && line[1] == '\n') {
			CopyMemory(d, "\\par ", 5);
			line++;
			d += 5;
		} else if (*line == '\n') {
			CopyMemory(d, "\\line ", 6);
			d += 6;
		} else if (*line == '%' && !simpleMode) {
			char szTemp[200]; szTemp[0] = '\0';
			switch (*++line) {
			case '\0':
			case '%':
				*d++ = '%';
				break;

			case 'c':
			case 'f':
				if (g_Settings.bStripFormat || streamData->bStripFormat)
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
				if (!g_Settings.bStripFormat && !streamData->bStripFormat) {
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
					mir_snprintf(szTemp, SIZEOF(szTemp), "%s ", pci->Log_SetStyle(index));
				}
				break;
			}

			if (szTemp[0]) {
				size_t iLen = mir_strlen(szTemp);
				memcpy(d, szTemp, iLen);
				d += iLen;
			}
		} else if (*line == '\t' && !streamData->bStripFormat) {
			CopyMemory(d, "\\tab ", 5);
			d += 5;
		} else if ((*line == '\\' || *line == '{' || *line == '}') && !streamData->bStripFormat) {
			*d++ = '\\';
			*d++ = (char) * line;
		} else if (*line > 0 && *line < 128) {
			*d++ = (char) * line;
		}
		else d += sprintf(d, "\\u%u ?", (WORD) * line); //!!!!!!!!!
	}

	*cbBufferEnd = (int)(d - *buffer);
	return textCharsCount;
}

static void AddEventToBuffer(char **buffer, int *bufferEnd, int *bufferAlloced, LOGSTREAMDATA *streamData)
{
	TCHAR szTemp[512], szTemp2[512];
	TCHAR* pszNick = NULL;

	if (streamData == NULL)
		return;

	if (streamData->lin == NULL)
		return;

	if (streamData->lin->ptszNick) {
		if (g_Settings.bLogLimitNames && mir_tstrlen(streamData->lin->ptszNick) > 20) {
			_tcsncpy_s(szTemp, 20, streamData->lin->ptszNick, _TRUNCATE);
			_tcsncpy_s(szTemp + 20, 4, _T("..."), _TRUNCATE);
		}
		else _tcsncpy_s(szTemp, streamData->lin->ptszNick, _TRUNCATE);

		if (g_Settings.bClickableNicks)
			mir_sntprintf(szTemp2, SIZEOF(szTemp2), _T("~~++#%s#++~~"), szTemp);
		else
			_tcsncpy_s(szTemp2, szTemp, _TRUNCATE);

		if (streamData->lin->ptszUserInfo && streamData->lin->iType != GC_EVENT_TOPIC)
			mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%s (%s)"), szTemp2, streamData->lin->ptszUserInfo);
		else
			_tcsncpy_s(szTemp, szTemp2, _TRUNCATE);
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
				/* replace nick of a newcomer with a link */
					Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("%s has joined"), pszNick);
			else
				Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("You have joined %s"), streamData->si->ptszName);
		}
		break;
	case GC_EVENT_PART:
		if (pszNick)
			Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced, TranslateT("%s has left"), pszNick);
		if (streamData->lin->ptszText)
			Log_AppendRTF(streamData, FALSE, buffer, bufferEnd, bufferAlloced, _T(": %s"), streamData->lin->ptszText);
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
		if (pszNick && streamData->lin->ptszStatus)
			Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced,
			TranslateT("%s kicked %s"), streamData->lin->ptszStatus, pszNick);

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
		if (pszNick)
			Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced,
			(streamData->lin->ptszUserInfo) ? TranslateT(" (set by %s on %s)") :
			TranslateT(" (set by %s)"),
			pszNick, streamData->lin->ptszUserInfo);
		break;
	case GC_EVENT_INFORMATION:
		if (streamData->lin->ptszText)
			Log_AppendRTF(streamData, FALSE, buffer, bufferEnd, bufferAlloced, (streamData->lin->bIsMe) ? _T("--> %s") : _T("%s"), streamData->lin->ptszText);
		break;
	case GC_EVENT_ADDSTATUS:
		if (pszNick && streamData->lin->ptszText && streamData->lin->ptszStatus)
			Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced,
			TranslateT("%s enables '%s' status for %s"),
			streamData->lin->ptszText, streamData->lin->ptszStatus, pszNick);
		break;
	case GC_EVENT_REMOVESTATUS:
		if (pszNick && streamData->lin->ptszText && streamData->lin->ptszStatus) {
			Log_AppendRTF(streamData, TRUE, buffer, bufferEnd, bufferAlloced,
				TranslateT("%s disables '%s' status for %s"),
				streamData->lin->ptszText , streamData->lin->ptszStatus, pszNick);
		}
		break;
	}
}

char* Log_CreateRtfHeader(MODULEINFO *mi)
{
	int bufferAlloced, bufferEnd, i = 0;

	// guesstimate amount of memory for the RTF header
	bufferEnd = 0;
	bufferAlloced = 4096;
	char *buffer = (char *)mir_realloc(mi->pszHeader, bufferAlloced);
	buffer[0] = '\0';


	// get the number of pixels per logical inch
	if (pci->logPixelSY == 0) {
		HDC hdc = GetDC(NULL);
		pci->logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		pci->logPixelSX = GetDeviceCaps(hdc, LOGPIXELSX);
		ReleaseDC(NULL, hdc);
	}

	// ### RTF HEADER

	// font table
	Log_Append(&buffer, &bufferEnd, &bufferAlloced, "{\\rtf1\\ansi\\deff0{\\fonttbl");
	for (i = 0; i < OPTIONS_FONTCOUNT; i++)
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "{\\f%u\\fnil\\fcharset%u%S;}", i, pci->aFonts[i].lf.lfCharSet, pci->aFonts[i].lf.lfFaceName);

	// colour table
	Log_Append(&buffer, &bufferEnd, &bufferAlloced, "}{\\colortbl ;");

	for (i = 0; i < OPTIONS_FONTCOUNT; i++)
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(pci->aFonts[i].color), GetGValue(pci->aFonts[i].color), GetBValue(pci->aFonts[i].color));

	for (i = 0; i < mi->nColorCount; i++)
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(mi->crColors[i]), GetGValue(mi->crColors[i]), GetBValue(mi->crColors[i]));

	for (i = 0; i < STATUSICONCOUNT; i++)
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\red%u\\green%u\\blue%u;", GetRValue(g_Settings.nickColors[i]), GetGValue(g_Settings.nickColors[i]), GetBValue(g_Settings.nickColors[i]));

	// new paragraph
	Log_Append(&buffer, &bufferEnd, &bufferAlloced, "}\\pard\\sl%d", 1000);

	// set tabs and indents
	int iIndent = 0;

	if (g_Settings.bLogSymbols) {
		TCHAR szString[2];
		LOGFONT lf;

		szString[1] = 0;
		szString[0] = 0x28;
		pci->LoadMsgDlgFont(17, &lf, NULL);
		HFONT hFont = CreateFontIndirect(&lf);
		int iText = GetTextPixelSize(szString, hFont, true) + 3;
		DeleteObject(hFont);
		iIndent += (iText * 1440) / pci->logPixelSX;
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\tx%u", iIndent);
	}
	else if (g_Settings.dwIconFlags) {
		iIndent += ((g_Settings.bScaleIcons ? 14 : 20) * 1440) / pci->logPixelSX;
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\tx%u", iIndent);
	}
	if (g_Settings.bShowTime) {
		int iSize = (g_Settings.LogTextIndent * 1440) / pci->logPixelSX;
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\tx%u", iIndent + iSize);
		if (g_Settings.bLogIndentEnabled)
			iIndent += iSize;
	}
	Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\fi-%u\\li%u", iIndent, iIndent);

	return buffer;
}

static char* Log_CreateRTF(LOGSTREAMDATA *streamData)
{
	char *buffer, *header;
	int bufferAlloced, bufferEnd, me = 0;
	LOGINFO * lin = streamData->lin;
	MODULEINFO *mi = pci->MM_FindModule(streamData->si->pszModule);

	// guesstimate amount of memory for the RTF
	bufferEnd = 0;
	bufferAlloced = streamData->bRedraw ? 2048 * (streamData->si->iEventCount + 2) : 2048;
	buffer = (char *) mir_alloc(bufferAlloced);
	buffer[0] = '\0';

	// ### RTF HEADER

	if (0 == mi->pszHeader)
		mi->pszHeader = Log_CreateRtfHeader(mi);

	header = mi->pszHeader;
	streamData->crCount = mi->nColorCount;

	if (header)
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, header);


	// ### RTF BODY (one iteration per event that should be streamed in)
	while (lin) {
		// filter
		if ((streamData->si->iType != GCW_CHATROOM && streamData->si->iType != GCW_PRIVMESS) || !streamData->si->bFilterEnabled || (streamData->si->iLogFilterFlags & lin->iType) != 0) {
			if (lin->next != NULL)
				Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\par ");

			if (streamData->dat->dwFlags & MWF_DIVIDERWANTED || lin->dwFlags & MWF_DIVIDERWANTED) {
				static char szStyle_div[128] = "\0";
				if (szStyle_div[0] == 0)
					mir_snprintf(szStyle_div, 128, "\\f%u\\cf%u\\ul0\\b%d\\i%d\\fs%u", 17, 18, 0, 0, 5);

				lin->dwFlags |= MWF_DIVIDERWANTED;
				if (lin->prev || !streamData->bRedraw)
					Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\qc\\sl-1\\highlight%d %s ---------------------------------------------------------------------------------------\\par ", 18, szStyle_div);
				streamData->dat->dwFlags &= ~MWF_DIVIDERWANTED;
			}
			// create new line, and set font and color
			Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\ql\\sl0%s ", pci->Log_SetStyle(0));
			Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\v~-+%d+-~\\v0 ", lin);

			// Insert icon
			if (g_Settings.bLogSymbols)                // use symbols
				Log_Append(&buffer, &bufferEnd, &bufferAlloced, "%s %c", pci->Log_SetStyle(17), EventToSymbol(lin));
			else if (g_Settings.dwIconFlags) {
				int iIndex = lin->bIsHighlighted ? ICON_HIGHLIGHT : EventToIcon(lin);
				Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\f0\\fs14");
				while (bufferAlloced - bufferEnd < (pci->logIconBmpSize[0] + 20))
					bufferAlloced += 4096;
				buffer = (char *) mir_realloc(buffer, bufferAlloced);
				CopyMemory(buffer + bufferEnd, pci->pLogIconBmpBits[iIndex], pci->logIconBmpSize[iIndex]);
				bufferEnd += pci->logIconBmpSize[iIndex];
			}

			if (g_Settings.bTimeStampEventColour) {
				// colored timestamps
				static char szStyle[256];
				LOGFONT &F = pci->aFonts[0].lf;
				int iii;
				if (lin->ptszNick && lin->iType == GC_EVENT_MESSAGE) {
					iii = lin->bIsHighlighted ? 16 : (lin->bIsMe ? 2 : 1);
					mir_snprintf(szStyle, SIZEOF(szStyle), "\\f0\\cf%u\\ul0\\highlight0\\b%d\\i%d\\ul%d\\fs%u",
						iii + 1, F.lfWeight >= FW_BOLD ? 1 : 0,F.lfItalic,F.lfUnderline, 2 * abs(F.lfHeight) * 74 / pci->logPixelSY);
					Log_Append(&buffer, &bufferEnd, &bufferAlloced, "%s ", szStyle);
				}
				else {
					iii = lin->bIsHighlighted ? 16 : EventToIndex(lin);
					mir_snprintf(szStyle, SIZEOF(szStyle), "\\f0\\cf%u\\ul0\\highlight0\\b%d\\i%d\\ul%d\\fs%u",
						iii + 1, F.lfWeight >= FW_BOLD ? 1 : 0, F.lfItalic,F.lfUnderline ,2 * abs(F.lfHeight) * 74 / pci->logPixelSY);
					Log_Append(&buffer, &bufferEnd, &bufferAlloced, "%s ", szStyle);
				}
			}
			else Log_Append(&buffer, &bufferEnd, &bufferAlloced, "%s ", pci->Log_SetStyle(0));

			// insert a TAB if necessary to put the timestamp in the right position
			if (g_Settings.dwIconFlags)
				Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\tab ");

			//insert timestamp
			if (g_Settings.bShowTime) {
				TCHAR szTimeStamp[30], szOldTimeStamp[30];

				_tcsncpy_s(szTimeStamp, pci->MakeTimeStamp(g_Settings.pszTimeStamp, lin->time), _TRUNCATE);
				_tcsncpy_s(szOldTimeStamp, pci->MakeTimeStamp(g_Settings.pszTimeStamp, streamData->si->LastTime), _TRUNCATE);
				if (!g_Settings.bShowTimeIfChanged || streamData->si->LastTime == 0 || mir_tstrcmp(szTimeStamp, szOldTimeStamp)) {
					streamData->si->LastTime = lin->time;
					Log_AppendRTF(streamData, TRUE, &buffer, &bufferEnd, &bufferAlloced, _T("%s"), szTimeStamp);
				}
				Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\tab ");
			}

			// Insert the nick
			if (lin->ptszNick && lin->iType == GC_EVENT_MESSAGE) {
				TCHAR pszTemp[300], *p1;
				char pszIndicator[3] = "\0\0";
				int  crNickIndex = 0;

				if (g_Settings.bLogClassicIndicators || g_Settings.bColorizeNicksInLog)
					pszIndicator[0] = GetIndicator(streamData->si, lin->ptszNick, &crNickIndex);

				Log_Append(&buffer, &bufferEnd, &bufferAlloced, "%s ", pci->Log_SetStyle(lin->bIsMe ? 2 : 1));

				if (g_Settings.bLogClassicIndicators)
					Log_Append(&buffer, &bufferEnd, &bufferAlloced, "%s", pszIndicator);

				mir_tstrncpy(pszTemp, lin->bIsMe ? g_Settings.pszOutgoingNick : g_Settings.pszIncomingNick, SIZEOF(pszTemp));
				p1 = _tcsstr(pszTemp, _T("%n"));
				if (p1)
					p1[1] = 's';

				if (!lin->bIsMe) {
					if (g_Settings.bClickableNicks) {
						_tcsnrplc(pszTemp, 300, _T("%s"), _T("~~++#%s#++~~"));
						pszTemp[299] = 0;
					}

					if (g_Settings.bColorizeNicksInLog && pszIndicator[0])
						Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\cf%u ", OPTIONS_FONTCOUNT + streamData->crCount + crNickIndex);
				}

				Log_AppendRTF(streamData, TRUE, &buffer, &bufferEnd, &bufferAlloced, pszTemp, lin->ptszNick);
				Log_Append(&buffer, &bufferEnd, &bufferAlloced, " ");
			}

			// Insert the message
			Log_Append(&buffer, &bufferEnd, &bufferAlloced, "%s ", pci->Log_SetStyle(lin->bIsHighlighted ? 16 : EventToIndex(lin)));
			streamData->lin = lin;
			AddEventToBuffer(&buffer, &bufferEnd, &bufferAlloced, streamData);
		}
		lin = lin->prev;
	}

	// ### RTF END
	if (streamData->bRedraw)
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "\\par}");
	else
		Log_Append(&buffer, &bufferEnd, &bufferAlloced, "}");
	return buffer;
}

static DWORD CALLBACK Log_StreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	LOGSTREAMDATA *lstrdat = (LOGSTREAMDATA*)dwCookie;

	if (lstrdat) {
		// create the RTF
		if (lstrdat->buffer == NULL) {
			lstrdat->bufferOffset = 0;
			lstrdat->buffer = Log_CreateRTF(lstrdat);
			lstrdat->bufferLen = (int)mir_strlen(lstrdat->buffer);
		}

		// give the RTF to the RE control
		*pcb = min(cb, lstrdat->bufferLen - lstrdat->bufferOffset);
		CopyMemory(pbBuff, lstrdat->buffer + lstrdat->bufferOffset, *pcb);
		lstrdat->bufferOffset += *pcb;

		// mir_free stuff if the streaming operation is complete
		if (lstrdat->bufferOffset == lstrdat->bufferLen) {
			mir_free(lstrdat->buffer);
			lstrdat->buffer = NULL;
		}
	}

	return 0;
}

void Log_StreamInEvent(HWND hwndDlg,  LOGINFO* lin, SESSION_INFO *si, bool bRedraw, bool bPhaseTwo)
{
	CHARRANGE oldsel, sel, newsel;
	POINT point = {0};

	TWindowData *dat = (TWindowData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (hwndDlg == 0 || lin == 0 || si == 0 || dat == 0)
		return;

	HWND hwndRich = GetDlgItem(hwndDlg, IDC_CHAT_LOG);

	LOGSTREAMDATA streamData;
	memset(&streamData, 0, sizeof(streamData));
	streamData.hwnd = hwndRich;
	streamData.si = si;
	streamData.lin = lin;
	streamData.bStripFormat = FALSE;
	streamData.dat = dat;

	if (!bRedraw && (si->iType == GCW_CHATROOM || si->iType == GCW_PRIVMESS) && si->bFilterEnabled && (si->iLogFilterFlags & lin->iType) == 0)
		return;

	bool bFlag = false, fDoReplace;

	EDITSTREAM stream = { 0 };
	stream.pfnCallback = Log_StreamCallback;
	stream.dwCookie = (DWORD_PTR) & streamData;

	SCROLLINFO scroll = { 0 };
	scroll.cbSize = sizeof(SCROLLINFO);
	scroll.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
	GetScrollInfo(GetDlgItem(hwndDlg, IDC_CHAT_LOG), SB_VERT, &scroll);
	SendMessage(hwndRich, EM_GETSCROLLPOS, 0, (LPARAM)&point);

	// do not scroll to bottom if there is a selection
	SendMessage(hwndRich, EM_EXGETSEL, 0, (LPARAM)&oldsel);
	if (oldsel.cpMax != oldsel.cpMin)
		SendMessage(hwndRich, WM_SETREDRAW, FALSE, 0);

	//set the insertion point at the bottom
	sel.cpMin = sel.cpMax = GetRichTextLength(hwndRich);
	SendMessage(hwndRich, EM_EXSETSEL, 0, (LPARAM)&sel);

	// fix for the indent... must be a M$ bug
	if (sel.cpMax == 0)
		bRedraw = TRUE;

	// should the event(s) be appended to the current log
	WPARAM wp = bRedraw ? SF_RTF : SFF_SELECTION | SF_RTF;

	//get the number of pixels per logical inch
	if (bRedraw) {
		HDC hdc = GetDC(NULL);
		pci->logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		pci->logPixelSX = GetDeviceCaps(hdc, LOGPIXELSX);
		ReleaseDC(NULL, hdc);
		SendMessage(hwndRich, WM_SETREDRAW, FALSE, 0);
		bFlag = true;
		//			SetCursor(LoadCursor(NULL, IDC_ARROW));
	}

	// stream in the event(s)
	streamData.lin = lin;
	streamData.bRedraw = bRedraw;
	SendMessage(hwndRich, EM_STREAMIN, wp, (LPARAM)&stream);

	// for new added events, only replace in message or action events.
	// no need to replace smileys or math formulas elsewhere
	fDoReplace = (bRedraw || (lin->ptszText && (lin->iType == GC_EVENT_MESSAGE || lin->iType == GC_EVENT_ACTION)));

	// replace marked nicknames with hyperlinks to make the nicks clickable
	if (g_Settings.bClickableNicks) {
		FINDTEXTEX fi, fi2;

		CHARFORMAT2 cf2;
		memset(&cf2, 0, sizeof(CHARFORMAT2));
		cf2.cbSize = sizeof(cf2);

		fi2.lpstrText = _T("#++~~");
		fi.chrg.cpMin = bRedraw ? 0 : sel.cpMin;
		fi.chrg.cpMax = -1;
		fi.lpstrText = _T("~~++#");

		while (SendMessage(hwndRich, EM_FINDTEXTEX, FR_DOWN, (LPARAM)&fi) > -1) {
			fi2.chrg.cpMin = fi.chrgText.cpMin;
			fi2.chrg.cpMax = -1;

			if (SendMessage(hwndRich, EM_FINDTEXTEX, FR_DOWN, (LPARAM)&fi2) > -1) {

				SendMessage(hwndRich, EM_EXSETSEL, 0, (LPARAM)&fi.chrgText);
				SendMessage(hwndRich, EM_REPLACESEL, TRUE, (LPARAM)_T(""));
				fi2.chrgText.cpMin -= fi.chrgText.cpMax - fi.chrgText.cpMin;
				fi2.chrgText.cpMax -= fi.chrgText.cpMax - fi.chrgText.cpMin;
				SendMessage(hwndRich, EM_EXSETSEL, 0, (LPARAM)&fi2.chrgText);
				SendMessage(hwndRich, EM_REPLACESEL, TRUE, (LPARAM)_T(""));
				fi2.chrgText.cpMax = fi2.chrgText.cpMin;

				fi2.chrgText.cpMin = fi.chrgText.cpMin;
				SendMessage(hwndRich, EM_EXSETSEL, 0, (LPARAM)&fi2.chrgText);
				cf2.dwMask = CFM_PROTECTED;
				cf2.dwEffects = CFE_PROTECTED;
				SendMessage(hwndRich, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2);
			}
			fi.chrg.cpMin = fi.chrgText.cpMax;
		}
		SendMessage(hwndRich, EM_SETSEL, -1, -1);
	}

	// run smileyadd
	if (PluginConfig.g_SmileyAddAvail && fDoReplace) {
		newsel.cpMax = -1;
		newsel.cpMin = sel.cpMin;
		if (newsel.cpMin < 0)
			newsel.cpMin = 0;

		SMADD_RICHEDIT3 sm = { sizeof(sm) };
		sm.hwndRichEditControl = hwndRich;
		sm.Protocolname = si->pszModule;
		sm.rangeToReplace = bRedraw ? NULL : &newsel;
		sm.disableRedraw = TRUE;
		sm.hContact = si->hContact;
		CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM)&sm);
	}

	// trim the message log to the number of most recent events
	// this uses hidden marks in the rich text to find the events which should be deleted
	if (si->bTrimmed) {
		TCHAR szPattern[50];
		FINDTEXTEX fi;

		mir_sntprintf(szPattern, SIZEOF(szPattern), _T("~-+%d+-~"), si->pLogEnd);
		fi.lpstrText = szPattern;
		fi.chrg.cpMin = 0;
		fi.chrg.cpMax = -1;
		if (SendMessage(hwndRich, EM_FINDTEXTEX, FR_DOWN, (LPARAM)&fi) != 0) {
			CHARRANGE sel;
			sel.cpMin = 0;
			sel.cpMax = 20;
			SendMessage(hwndRich, EM_SETSEL, 0, fi.chrgText.cpMax + 1);
			SendMessage(hwndRich, EM_REPLACESEL, TRUE, (LPARAM)_T(""));
		}
		si->bTrimmed = FALSE;
	}

	// scroll log to bottom if the log was previously scrolled to bottom, else restore old position
	if ((bRedraw || (UINT)scroll.nPos >= (UINT)scroll.nMax - scroll.nPage - 5 || scroll.nMax - scroll.nMin - scroll.nPage < 50))
		SendMessage(GetParent(hwndRich), GC_SCROLLTOBOTTOM, 0, 0);
	else
		SendMessage(hwndRich, EM_SETSCROLLPOS, 0, (LPARAM)&point);

	// do we need to restore the selection
	if (oldsel.cpMax != oldsel.cpMin) {
		SendMessage(hwndRich, EM_EXSETSEL, 0, (LPARAM)&oldsel);
		SendMessage(hwndRich, WM_SETREDRAW, TRUE, 0);
		InvalidateRect(hwndRich, NULL, TRUE);
	}

	// need to invalidate the window
	if (bFlag) {
		sel.cpMin = sel.cpMax = GetRichTextLength(hwndRich);
		SendMessage(hwndRich, EM_EXSETSEL, 0, (LPARAM)&sel);
		SendMessage(hwndRich, WM_SETREDRAW, TRUE, 0);
		InvalidateRect(hwndRich, NULL, TRUE);
	}
}
