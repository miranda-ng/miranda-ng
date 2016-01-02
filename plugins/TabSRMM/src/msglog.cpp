/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (�) 2012-16 Miranda NG project,
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
// implements the richedit based message log and the template parser

#include "stdafx.h"

struct TCpTable cpTable[] = {
	{ 874, LPGENT("Thai") },
	{ 932, LPGENT("Japanese") },
	{ 936, LPGENT("Simplified Chinese") },
	{ 949, LPGENT("Korean") },
	{ 950, LPGENT("Traditional Chinese") },
	{ 1250, LPGENT("Central European") },
	{ 1251, LPGENT("Cyrillic") },
	{ 20866, LPGENT("Cyrillic KOI8-R") },
	{ 1252, LPGENT("Latin I") },
	{ 1253, LPGENT("Greek") },
	{ 1254, LPGENT("Turkish") },
	{ 1255, LPGENT("Hebrew") },
	{ 1256, LPGENT("Arabic") },
	{ 1257, LPGENT("Baltic") },
	{ 1258, LPGENT("Vietnamese") },
	{ 1361, LPGENT("Korean (Johab)") },
	{ (UINT)-1, NULL }
};

wchar_t* weekDays[7] = { LPGENT("Sunday"), LPGENT("Monday"), LPGENT("Tuesday"), LPGENT("Wednesday"), LPGENT("Thursday"), LPGENT("Friday"), LPGENT("Saturday") };

wchar_t* months[12] =
{
	LPGENT("January"), LPGENT("February"), LPGENT("March"), LPGENT("April"), LPGENT("May"), LPGENT("June"),
	LPGENT("July"), LPGENT("August"), LPGENT("September"), LPGENT("October"), LPGENT("November"), LPGENT("December")
};

static time_t today;

int g_groupBreak = TRUE;
static TCHAR *szMyName = NULL;
static TCHAR *szYourName = NULL;

static int logPixelSY;
static TCHAR szToday[22], szYesterday[22];
char  rtfFontsGlobal[MSGDLGFONTCOUNT + 2][RTFCACHELINESIZE];
char *rtfFonts;

LOGFONTA logfonts[MSGDLGFONTCOUNT + 2];
COLORREF fontcolors[MSGDLGFONTCOUNT + 2];

#define LOGICON_MSG  0
#define LOGICON_URL  1
#define LOGICON_FILE 2
#define LOGICON_OUT 3
#define LOGICON_IN 4
#define LOGICON_STATUS 5
#define LOGICON_ERROR 6

static HICON Logicons[NR_LOGICONS];

#define STREAMSTAGE_HEADER  0
#define STREAMSTAGE_EVENTS  1
#define STREAMSTAGE_TAIL    2
#define STREAMSTAGE_STOP    3
struct LogStreamData {
	int stage;
	MCONTACT hContact;
	MEVENT hDbEvent, hDbEventLast;
	char *buffer;
	int bufferOffset, bufferLen;
	int eventsToInsert;
	int isEmpty;
	int isAppend;
	TWindowData *dlgDat;
	DBEVENTINFO *dbei;
};

__forceinline char *GetRTFFont(DWORD dwIndex)
{
	return rtfFonts + (dwIndex * RTFCACHELINESIZE);
}

/*
 * remove any empty line at the end of a message to avoid some RichEdit "issues" with
 * the highlight code (individual background colors).
 * Doesn't touch the message for sure, but empty lines at the end are ugly anyway.
 */
static void TrimMessage(TCHAR *msg)
{
	size_t iLen = mir_tstrlen(msg) - 1;
	size_t i = iLen;

	while (i && (msg[i] == '\r' || msg[i] == '\n')) {
		i--;
	}
	if (i < iLen)
		msg[i + 1] = '\0';
}

void TSAPI CacheLogFonts()
{
	HDC hdc = GetDC(NULL);
	logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(NULL, hdc);

	memset(logfonts, 0, (sizeof(LOGFONTA) * (MSGDLGFONTCOUNT + 2)));
	for (int i = 0; i < MSGDLGFONTCOUNT; i++) {
		LoadLogfont(i, &logfonts[i], &fontcolors[i], FONTMODULE);
		mir_snprintf(rtfFontsGlobal[i], "\\f%u\\cf%u\\b%d\\i%d\\ul%d\\fs%u", i, i, logfonts[i].lfWeight >= FW_BOLD ? 1 : 0, logfonts[i].lfItalic, logfonts[i].lfUnderline, 2 * abs(logfonts[i].lfHeight) * 74 / logPixelSY);
	}
	mir_snprintf(rtfFontsGlobal[MSGDLGFONTCOUNT], "\\f%u\\cf%u\\b%d\\i%d\\fs%u", MSGDLGFONTCOUNT, MSGDLGFONTCOUNT, 0, 0, 0);

	_tcsncpy(szToday, TranslateT("Today"), 20);
	_tcsncpy(szYesterday, TranslateT("Yesterday"), 20);
	szToday[19] = szYesterday[19] = 0;

	// cache/create the info panel fonts
	for (int i = 0; i < IPFONTCOUNT; i++) {
		if (CInfoPanel::m_ipConfig.hFonts[i])
			DeleteObject(CInfoPanel::m_ipConfig.hFonts[i]);

		COLORREF clr;
		LOGFONTA lf;
		LoadLogfont(i + 100, &lf, &clr, FONTMODULE);
		lf.lfUnderline = 0;
		CInfoPanel::m_ipConfig.hFonts[i] = CreateFontIndirectA(&lf);
		CInfoPanel::m_ipConfig.clrs[i] = clr;
	}

	hdc = GetDC(PluginConfig.g_hwndHotkeyHandler);

	HFONT hOldFont = (HFONT)SelectObject(hdc, CInfoPanel::m_ipConfig.hFonts[IPFONTID_NICK]);

	SIZE sz;
	GetTextExtentPoint32(hdc, _T("WMA"), 3, &sz);
	CInfoPanel::m_ipConfig.height1 = sz.cy;
	SelectObject(hdc, CInfoPanel::m_ipConfig.hFonts[IPFONTID_UIN]);
	GetTextExtentPoint32(hdc, _T("WMA"), 3, &sz);
	CInfoPanel::m_ipConfig.height2 = sz.cy;

	SelectObject(hdc, hOldFont);
	ReleaseDC(PluginConfig.g_hwndHotkeyHandler, hdc);
	PluginConfig.hFontCaption = CInfoPanel::m_ipConfig.hFonts[IPFONTCOUNT - 1];

	PluginConfig.crIncoming = M.GetDword(FONTMODULE, "inbg", SRMSGDEFSET_BKGINCOLOUR);
	PluginConfig.crOutgoing = M.GetDword(FONTMODULE, "outbg", SRMSGDEFSET_BKGOUTCOLOUR);
	PluginConfig.crStatus = M.GetDword(FONTMODULE, "statbg", SRMSGDEFSET_BKGCOLOUR);
	PluginConfig.crOldIncoming = M.GetDword(FONTMODULE, "oldinbg", SRMSGDEFSET_BKGINCOLOUR);
	PluginConfig.crOldOutgoing = M.GetDword(FONTMODULE, "oldoutbg", SRMSGDEFSET_BKGOUTCOLOUR);
}

void FreeLogFonts()
{
	for (int i = 0; i < IPFONTCOUNT; i++)
		if (CInfoPanel::m_ipConfig.hFonts[i])
			DeleteObject(CInfoPanel::m_ipConfig.hFonts[i]);
}

void TSAPI CacheMsgLogIcons()
{
	Logicons[0] = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
	Logicons[1] = Skin_LoadIcon(SKINICON_EVENT_URL);
	Logicons[2] = Skin_LoadIcon(SKINICON_EVENT_FILE);
	Logicons[3] = PluginConfig.g_iconOut;
	Logicons[4] = PluginConfig.g_iconIn;
	Logicons[5] = PluginConfig.g_iconStatus;
	Logicons[6] = PluginConfig.g_iconErr;
}

static int TSAPI GetColorIndex(char *rtffont)
{
	char *p;

	if ((p = strstr(rtffont, "\\cf")) != NULL)
		return atoi(p + 3);
	return 0;
}

static int AppendUnicodeToBuffer(CMStringA &str, const TCHAR *line, int mode)
{
	str.Append("{\\uc1 ");

	int textCharsCount = 0;
	for (; *line; line++, textCharsCount++) {
		if (*line == 127 && line[1] != 0) {
			TCHAR code = line[2];
			if (((code == '0' || code == '1') && line[3] == ' ') || (line[1] == 'c' && code == 'x')) {
				int begin = (code == '1');
				switch (line[1]) {
				case 'b':
					str.Append(begin ? "\\b " : "\\b0 ");
					line += 3;
					continue;
				case 'i':
					str.Append(begin ? "\\i " : "\\i0 ");
					line += 3;
					continue;
				case 'u':
					str.Append(begin ? "\\ul " : "\\ul0 ");
					line += 3;
					continue;
				case 's':
					str.Append(begin ? "\\strike " : "\\strike0 ");
					line += 3;
					continue;
				case 'c':
					begin = (code == 'x');
					str.Append("\\cf");
					if (begin) {
						str.AppendChar((char)line[3]);
						str.AppendChar((char)line[4]);
					}
					else {
						char szTemp[10];
						int colindex = GetColorIndex(GetRTFFont(LOWORD(mode) ? (MSGFONTID_MYMSG + (HIWORD(mode) ? 8 : 0)) : (MSGFONTID_YOURMSG + (HIWORD(mode) ? 8 : 0))));
						mir_snprintf(szTemp, "%02d", colindex);
						str.Append(szTemp);
					}
					str.AppendChar(' ');

					line += (begin ? 6 : 3);
					continue;
				}
			}
		}
		if (*line == '\r' && line[1] == '\n') {
			str.Append("\\line ");
			line++;
		}
		else if (*line == '\n') {
			str.Append("\\line ");
		}
		else if (*line == '\t') {
			str.Append("\\tab ");
		}
		else if (*line == '\\' || *line == '{' || *line == '}') {
			str.AppendChar('\\');
			str.AppendChar((char)*line);
		}
		else if (*line < 128) {
			str.AppendChar((char)*line);
		}
		else str.AppendFormat("\\u%d ?", *line);
	}

	str.AppendChar('}');
	return textCharsCount;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void Build_RTF_Header(CMStringA &str, TWindowData *dat)
{
	int i;
	LOGFONTA *logFonts = dat->pContainer->theme.logFonts;
	COLORREF *fontColors = dat->pContainer->theme.fontColors;
	TLogTheme *theme = &dat->pContainer->theme;

	str.Append("{\\rtf1\\ansi\\deff0{\\fonttbl");

	for (i = 0; i < MSGDLGFONTCOUNT; i++)
		str.AppendFormat("{\\f%u\\fnil\\fcharset%u %s;}", i, logFonts[i].lfCharSet, logFonts[i].lfFaceName);
	str.AppendFormat("{\\f%u\\fnil\\fcharset%u %s;}", MSGDLGFONTCOUNT, logFonts[i].lfCharSet, "Arial");

	str.Append("}{\\colortbl ");
	for (i = 0; i < MSGDLGFONTCOUNT; i++)
		str.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(fontColors[i]), GetGValue(fontColors[i]), GetBValue(fontColors[i]));

	COLORREF colour = (GetSysColorBrush(COLOR_HOTLIGHT) == NULL) ? RGB(0, 0, 255) : GetSysColor(COLOR_HOTLIGHT);
	str.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));

	// OnO: Create incoming and outcoming colours
	colour = theme->inbg;
	str.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = theme->outbg;
	str.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = theme->bg;
	str.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = theme->hgrid;
	str.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = theme->oldinbg;
	str.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = theme->oldoutbg;
	str.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	colour = theme->statbg;
	str.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));

	// custom template colors...
	for (i = 1; i <= 5; i++) {
		colour = theme->custom_colors[i - 1];
		if (colour == 0)
			colour = RGB(1, 1, 1);
		str.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	}

	// bbcode colors...
	for (i = 0; i < Utils::rtf_ctable_size; i++)
		str.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(Utils::rtf_ctable[i].clr), GetGValue(Utils::rtf_ctable[i].clr), GetBValue(Utils::rtf_ctable[i].clr));

	// paragraph header
	str.AppendFormat("}");

	// indent:
	// real indent is set in msgdialog.c (DM_OPTIONSAPPLIED)
	if (!(dat->dwFlags & MWF_LOG_INDENT))
		str.AppendFormat("\\li%u\\ri%u\\fi%u\\tx%u", 2 * 15, 2 * 15, 0, 70 * 15);
}

// mir_free() the return value
static char* CreateRTFHeader(TWindowData *dat)
{
	CMStringA str;
	Build_RTF_Header(str, dat);
	return str.Detach();
}

static void AppendTimeStamp(TCHAR *szFinalTimestamp, int isSent, CMStringA &str, int skipFont, TWindowData *dat, int iFontIDOffset)
{
	if (skipFont)
		AppendUnicodeToBuffer(str, szFinalTimestamp, MAKELONG(isSent, dat->bIsHistory));
	else {
		str.Append(GetRTFFont(isSent ? MSGFONTID_MYTIME + iFontIDOffset : MSGFONTID_YOURTIME + iFontIDOffset));
		str.AppendChar(' ');
		AppendUnicodeToBuffer(str, szFinalTimestamp, MAKELONG(isSent, dat->bIsHistory));
	}
}

static TCHAR* Template_MakeRelativeDate(HANDLE hTimeZone, time_t check, TCHAR code)
{
	static TCHAR szResult[100];
	const TCHAR *szFormat;

	if ((code == (TCHAR)'R' || code == (TCHAR)'r') && check >= today)
		mir_tstrcpy(szResult, szToday);
	else if ((code == (TCHAR)'R' || code == (TCHAR)'r') && check > (today - 86400))
		mir_tstrcpy(szResult, szYesterday);
	else {
		if (code == 'D' || code == 'R')
			szFormat = _T("D");
		else if (code == 'T')
			szFormat = _T("s");
		else if (code == 't')
			szFormat = _T("t");
		else
			szFormat = _T("d");

		TimeZone_PrintTimeStamp(hTimeZone, check, szFormat, szResult, _countof(szResult), 0);
	}
	return szResult;
}

// mir_free() the return value
static char *CreateRTFTail()
{
	return mir_strdup("}");
}

int TSAPI DbEventIsShown(DBEVENTINFO *dbei)
{
	if (!IsCustomEvent(dbei->eventType) || DbEventIsForMsgWindow(dbei))
		return 1;

	return IsStatusEvent(dbei->eventType);
}

int DbEventIsForMsgWindow(DBEVENTINFO *dbei)
{
	DBEVENTTYPEDESCR *et = (DBEVENTTYPEDESCR*)CallService(MS_DB_EVENT_GETTYPE, (WPARAM)dbei->szModule, (LPARAM)dbei->eventType);
	return et && (et->flags & DETF_MSGWINDOW);
}

static char* Template_CreateRTFFromDbEvent(TWindowData *dat, MCONTACT hContact, MEVENT hDbEvent, LogStreamData *streamData)
{
	HANDLE hTimeZone = NULL;
	BOOL skipToNext = FALSE, skipFont = FALSE;
	struct tm event_time = { 0 };
	BOOL isBold = FALSE, isItalic = FALSE, isUnderline = FALSE;
	DWORD dwFormattingParams = MAKELONG(1, 0);

	DBEVENTINFO dbei = { 0 };
	if (streamData->dbei != 0)
		dbei = *(streamData->dbei);
	else {
		dbei.cbSize = sizeof(dbei);
		dbei.cbBlob = db_event_getBlobSize(hDbEvent);
		if (dbei.cbBlob == -1)
			return NULL;

		dbei.pBlob = (PBYTE)mir_alloc(dbei.cbBlob);
		db_event_get(hDbEvent, &dbei);
		if (!DbEventIsShown(&dbei)) {
			mir_free(dbei.pBlob);
			return NULL;
		}
	}

	if (dbei.eventType == EVENTTYPE_MESSAGE && !dbei.markedRead())
		dat->cache->updateStats(TSessionStats::SET_LAST_RCV, mir_strlen((char *)dbei.pBlob));

	TCHAR *formatted = NULL;
	TCHAR *msg = DbGetEventTextT(&dbei, CP_UTF8);
	if (!msg) {
		mir_free(dbei.pBlob);
		return NULL;
	}
	TrimMessage(msg);
	formatted = const_cast<TCHAR *>(Utils::FormatRaw(dat, msg, dwFormattingParams, FALSE));
	mir_free(msg);

	CMStringA str;
	BOOL bIsStatusChangeEvent = IsStatusEvent(dbei.eventType);

	if (dat->isAutoRTL & 2) {                                     // means: last \\par was deleted to avoid new line at end of log
		str.Append("\\par");
		dat->isAutoRTL &= ~2;
	}

	if (dat->dwFlags & MWF_LOG_RTL)
		dbei.flags |= DBEF_RTL;

	if (dbei.flags & DBEF_RTL)
		dat->isAutoRTL |= 1;

	DWORD dwEffectiveFlags = dat->dwFlags;

	dat->bIsHistory = (dbei.timestamp < dat->cache->getSessionStart() && dbei.markedRead());
	int iFontIDOffset = dat->bIsHistory ? 8 : 0;     // offset into the font table for either history (old) or new events... (# of fonts per configuration set)
	BOOL isSent = (dbei.flags & DBEF_SENT);

	if (!isSent && (bIsStatusChangeEvent || dbei.eventType == EVENTTYPE_MESSAGE || DbEventIsForMsgWindow(&dbei))) {
		db_event_markRead(hContact, hDbEvent);
		CallService(MS_CLIST_REMOVEEVENT, hContact, (LPARAM)hDbEvent);
	}

	g_groupBreak = TRUE;

	if (dwEffectiveFlags & MWF_DIVIDERWANTED) {
		static char szStyle_div[128] = "\0";
		if (szStyle_div[0] == 0)
			mir_snprintf(szStyle_div, "\\f%u\\cf%u\\ul0\\b%d\\i%d\\fs%u", H_MSGFONTID_DIVIDERS, H_MSGFONTID_DIVIDERS, 0, 0, 5);

		str.AppendFormat("\\sl-1\\slmult0\\highlight%d\\cf%d\\-\\par\\sl0", H_MSGFONTID_DIVIDERS, H_MSGFONTID_DIVIDERS);
		dat->dwFlags &= ~MWF_DIVIDERWANTED;
	}
	if (dwEffectiveFlags & MWF_LOG_GROUPMODE && ((dbei.flags & (DBEF_SENT | DBEF_READ | DBEF_RTL)) == LOWORD(dat->iLastEventType)) && dbei.eventType == EVENTTYPE_MESSAGE && HIWORD(dat->iLastEventType) == EVENTTYPE_MESSAGE && (dbei.timestamp - dat->lastEventTime) < 86400) {
		g_groupBreak = FALSE;
		if ((time_t)dbei.timestamp > today && dat->lastEventTime < today)
			g_groupBreak = TRUE;
	}
	if (!streamData->isEmpty && g_groupBreak && (dwEffectiveFlags & MWF_LOG_GRID))
		str.AppendFormat("\\sl-1\\slmult0\\highlight%d\\cf%d\\-\\par\\sl0", MSGDLGFONTCOUNT + 4, MSGDLGFONTCOUNT + 4);

	if (dbei.flags & DBEF_RTL)
		str.Append("\\rtlpar");
	else
		str.Append("\\ltrpar");

	/* OnO: highlight start */
	if (bIsStatusChangeEvent)
		str.AppendFormat("\\highlight%d\\cf%d", MSGDLGFONTCOUNT + 7, MSGDLGFONTCOUNT + 7);
	else
		str.AppendFormat("\\highlight%d\\cf%d", MSGDLGFONTCOUNT + (dat->bIsHistory ? 5 : 1) + ((isSent) ? 1 : 0), MSGDLGFONTCOUNT + (dat->bIsHistory ? 5 : 1) + ((isSent) ? 1 : 0));

	streamData->isEmpty = FALSE;

	if (dat->isAutoRTL & 1) {
		if (dbei.flags & DBEF_RTL)
			str.Append("\\ltrch\\rtlch");
		else
			str.Append("\\rtlch\\ltrch");
	}

	// templated code starts here
	if (dwEffectiveFlags & MWF_LOG_SHOWTIME) {
		hTimeZone = ((dat->dwFlags & MWF_LOG_LOCALTIME) && !isSent) ? dat->hTimeZone : NULL;
		time_t local_time = TimeZone_UtcToLocal(hTimeZone, dbei.timestamp);
		event_time = *gmtime(&local_time);
	}

	TTemplateSet *this_templateset = dbei.flags & DBEF_RTL ? dat->pContainer->rtl_templates : dat->pContainer->ltr_templates;

	TCHAR *szTemplate;
	if (bIsStatusChangeEvent)
		szTemplate = this_templateset->szTemplates[TMPL_STATUSCHG];
	else if (dbei.eventType == EVENTTYPE_ERRMSG)
		szTemplate = this_templateset->szTemplates[TMPL_ERRMSG];
	else {
		if (dwEffectiveFlags & MWF_LOG_GROUPMODE)
			szTemplate = isSent ? (g_groupBreak ? this_templateset->szTemplates[TMPL_GRPSTARTOUT] : this_templateset->szTemplates[TMPL_GRPINNEROUT]) :
			(g_groupBreak ? this_templateset->szTemplates[TMPL_GRPSTARTIN] : this_templateset->szTemplates[TMPL_GRPINNERIN]);
		else
			szTemplate = isSent ? this_templateset->szTemplates[TMPL_MSGOUT] : this_templateset->szTemplates[TMPL_MSGIN];
	}

	size_t iTemplateLen = mir_tstrlen(szTemplate);
	BOOL showTime = dwEffectiveFlags & MWF_LOG_SHOWTIME;
	BOOL showDate = dwEffectiveFlags & MWF_LOG_SHOWDATES;

	if (dat->hHistoryEvents) {
		if (dat->curHistory == dat->maxHistory) {
			memmove(dat->hHistoryEvents, &dat->hHistoryEvents[1], sizeof(HANDLE)* (dat->maxHistory - 1));
			dat->curHistory--;
		}
		dat->hHistoryEvents[dat->curHistory++] = hDbEvent;
	}

	str.Append("\\ul0\\b0\\i0\\v0 ");

	for (size_t i = 0; i < iTemplateLen;) {
		TCHAR ci = szTemplate[i];
		if (ci == '%') {
			TCHAR cc = szTemplate[i + 1];
			skipToNext = FALSE;
			skipFont = FALSE;

			// handle modifiers
			while (cc == '#' || cc == '$' || cc == '&' || cc == '?' || cc == '\\') {
				switch (cc) {
				case '#':
					if (!dat->bIsHistory) {
						skipToNext = TRUE;
						goto skip;
					}
					i++;
					cc = szTemplate[i + 1];
					continue;

				case '$':
					if (dat->bIsHistory) {
						skipToNext = TRUE;
						goto skip;
					}
					i++;
					cc = szTemplate[i + 1];
					continue;

				case '&':
					i++;
					cc = szTemplate[i + 1];
					skipFont = TRUE;
					break;

				case '?':
					if (dwEffectiveFlags & MWF_LOG_NORMALTEMPLATES) {
						i++;
						cc = szTemplate[i + 1];
						continue;
					}
					i++;
					skipToNext = TRUE;
					goto skip;

				case '\\':
					if (!(dwEffectiveFlags & MWF_LOG_NORMALTEMPLATES)) {
						i++;
						cc = szTemplate[i + 1];
						continue;
					}
					i++;
					skipToNext = TRUE;
					goto skip;
				}
			}

			TCHAR color, code;
			switch (cc) {
			case 'V':
				//str.Append("\\fs0\\\expnd-40 ~-%d-~", hDbEvent);
				break;
			case 'I':
				if (dwEffectiveFlags & MWF_LOG_SHOWICONS) {
					int icon;
					if ((dwEffectiveFlags & MWF_LOG_INOUTICONS) && dbei.eventType == EVENTTYPE_MESSAGE)
						icon = isSent ? LOGICON_OUT : LOGICON_IN;
					else {
						switch (dbei.eventType) {
						case EVENTTYPE_FILE:
							icon = LOGICON_FILE;
							break;
						case EVENTTYPE_ERRMSG:
							icon = LOGICON_ERROR;
							break;
						default:
							icon = LOGICON_MSG;
							break;
						}
						if (bIsStatusChangeEvent)
							icon = LOGICON_STATUS;
					}
					str.AppendFormat("%s\\fs1  #~#%01d%c%s ", GetRTFFont(MSGFONTID_SYMBOLS_IN), icon, isSent ? '>' : '<', GetRTFFont(isSent ? MSGFONTID_MYMSG + iFontIDOffset : MSGFONTID_YOURMSG + iFontIDOffset));
				}
				else skipToNext = TRUE;
				break;
			case 'D': // long date
				if (showTime && showDate) {
					TCHAR	*szFinalTimestamp = Template_MakeRelativeDate(hTimeZone, dbei.timestamp, 'D');
					AppendTimeStamp(szFinalTimestamp, isSent, str, skipFont, dat, iFontIDOffset);
				}
				else skipToNext = TRUE;
				break;
			case 'E': // short date...
				if (showTime && showDate) {
					TCHAR	*szFinalTimestamp = Template_MakeRelativeDate(hTimeZone, dbei.timestamp, 'E');
					AppendTimeStamp(szFinalTimestamp, isSent, str, skipFont, dat, iFontIDOffset);
				}
				else skipToNext = TRUE;
				break;
			case 'a': // 12 hour
			case 'h': // 24 hour
				if (showTime) {
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME)));
						str.AppendChar(' ');
					}
					str.AppendFormat(cc == 'h' ? "%02d" : "%2d", cc == 'h' ? event_time.tm_hour : (event_time.tm_hour > 12 ? event_time.tm_hour - 12 : event_time.tm_hour));
				}
				else skipToNext = TRUE;
				break;
			case 'm': // minute
				if (showTime) {
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME)));
						str.AppendChar(' ');
					}
					str.AppendFormat("%02d", event_time.tm_min);
				}
				else skipToNext = TRUE;
				break;
			case 's': //second
				if (showTime && (dwEffectiveFlags & MWF_LOG_SHOWSECONDS)) {
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME)));
						str.AppendChar(' ');
					}
					str.AppendFormat("%02d", event_time.tm_sec);
				}
				else skipToNext = TRUE;
				break;
			case 'p': // am/pm symbol
				if (showTime) {
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME)));
						str.AppendChar(' ');
					}
					str.Append(event_time.tm_hour > 11 ? "PM" : "AM");
				}
				else skipToNext = TRUE;
				break;
			case 'o':            // month
				if (showTime && showDate) {
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME)));
						str.AppendChar(' ');
					}
					str.AppendFormat("%02d", event_time.tm_mon + 1);
				}
				else skipToNext = TRUE;
				break;
			case 'O': // month (name)
				if (showTime && showDate) {
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME)));
						str.AppendChar(' ');
					}
					AppendUnicodeToBuffer(str, TranslateTS(months[event_time.tm_mon]), MAKELONG(isSent, dat->bIsHistory));
				}
				else skipToNext = TRUE;
				break;
			case 'd': // day of month
				if (showTime && showDate) {
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME)));
						str.AppendChar(' ');
					}
					str.AppendFormat("%02d", event_time.tm_mday);
				}
				else skipToNext = TRUE;
				break;
			case 'w': // day of week
				if (showTime && showDate) {
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME)));
						str.AppendChar(' ');
					}
					AppendUnicodeToBuffer(str, TranslateTS(weekDays[event_time.tm_wday]), MAKELONG(isSent, dat->bIsHistory));
				}
				else skipToNext = TRUE;
				break;
			case 'y': // year
				if (showTime && showDate) {
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME)));
						str.AppendChar(' ');
					}
					str.AppendFormat("%04d", event_time.tm_year + 1900);
				}
				else skipToNext = TRUE;
				break;
			case 'R':
			case 'r': // long date
				if (showTime && showDate) {
					TCHAR	*szFinalTimestamp = Template_MakeRelativeDate(hTimeZone, dbei.timestamp, cc);
					AppendTimeStamp(szFinalTimestamp, isSent, str, skipFont, dat, iFontIDOffset);
				}
				else skipToNext = TRUE;
				break;
			case 't':
			case 'T':
				if (showTime) {
					TCHAR	*szFinalTimestamp = Template_MakeRelativeDate(hTimeZone, dbei.timestamp, (TCHAR)((dwEffectiveFlags & MWF_LOG_SHOWSECONDS) ? cc : (TCHAR)'t'));
					AppendTimeStamp(szFinalTimestamp, isSent, str, skipFont, dat, iFontIDOffset);
				}
				else skipToNext = TRUE;
				break;
			case 'S': // symbol
				if (dwEffectiveFlags & MWF_LOG_SYMBOLS) {
					int c;
					if ((dwEffectiveFlags & MWF_LOG_INOUTICONS) && dbei.eventType == EVENTTYPE_MESSAGE)
						c = isSent ? 0x37 : 0x38;
					else {
						switch (dbei.eventType) {
						case EVENTTYPE_MESSAGE:
							c = 0xaa;
							break;
						case EVENTTYPE_FILE:
							c = 0xcd;
							break;
						case EVENTTYPE_ERRMSG:
							c = 0x72;
							break;
						default:
							c = 0xaa;
							break;
						}
						if (bIsStatusChangeEvent)
							c = 0x4e;
					}
					if (!skipFont) {
						str.Append(GetRTFFont(isSent ? MSGFONTID_SYMBOLS_OUT : MSGFONTID_SYMBOLS_IN));
						str.AppendChar(' ');
					}
					str.AppendFormat("%c%s ", c, GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYMSG : MSGFONTID_YOURMSG)));
				}
				else skipToNext = TRUE;
				break;
			case 'n': // hard line break
				str.Append(dbei.flags & DBEF_RTL ? "\\rtlpar\\par\\rtlpar" : "\\par\\ltrpar");
				break;
			case 'l': // soft line break
				str.Append("\\line");
				break;
			case 'N': // nickname
				if (!skipFont) {
					str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME)));
					str.AppendChar(' ');
				}
				AppendUnicodeToBuffer(str, (isSent) ? szMyName : szYourName, MAKELONG(isSent, dat->bIsHistory));
				break;
			case 'U': // UIN
				if (!skipFont) {
					str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME)));
					str.AppendChar(' ');
				}
				AppendUnicodeToBuffer(str, (isSent) ? dat->myUin : dat->cache->getUIN(), MAKELONG(isSent, dat->bIsHistory));
				break;
			case 'e': // error message
				str.Append(GetRTFFont(MSGFONTID_ERROR));
				str.AppendChar(' ');
				AppendUnicodeToBuffer(str, LPCTSTR(dbei.szModule), MAKELONG(isSent, dat->bIsHistory));
				break;
			case 'M': // message
				switch (dbei.eventType) {
				case EVENTTYPE_MESSAGE:
				case EVENTTYPE_ERRMSG:
					if (bIsStatusChangeEvent || dbei.eventType == EVENTTYPE_ERRMSG) {
						if (dbei.eventType == EVENTTYPE_ERRMSG && dbei.cbBlob == 0)
							break;
						if (dbei.eventType == EVENTTYPE_ERRMSG) {
							if (!skipFont)
								str.AppendFormat("\\line%s ", GetRTFFont(bIsStatusChangeEvent ? H_MSGFONTID_STATUSCHANGES : MSGFONTID_MYMSG));
							else
								str.Append("\\line ");
						}
						else if (!skipFont) {
							str.Append(GetRTFFont(bIsStatusChangeEvent ? H_MSGFONTID_STATUSCHANGES : MSGFONTID_MYMSG));
							str.AppendChar(' ');
						}
					}
					else if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYMSG : MSGFONTID_YOURMSG)));
						str.AppendChar(' ');
					}

					AppendUnicodeToBuffer(str, formatted, MAKELONG(isSent, dat->bIsHistory));
					str.Append("\\b0\\ul0\\i0 ");
					break;

				case EVENTTYPE_FILE:
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYMISC : MSGFONTID_YOURMISC)));
						str.AppendChar(' ');
					}
					{
						char *szFileName = (char *)dbei.pBlob + sizeof(DWORD);
						ptrT tszFileName(DbGetEventStringT(&dbei, szFileName));

						char *szDescr = szFileName + mir_strlen(szFileName) + 1;
						if (*szDescr != 0) {
							ptrT tszDescr(DbGetEventStringT(&dbei, szDescr));

							TCHAR buf[1000];
							mir_sntprintf(buf, _T("%s (%s)"), tszFileName, tszDescr);
							AppendUnicodeToBuffer(str, buf, 0);
						}
						else AppendUnicodeToBuffer(str, tszFileName, 0);
					}
					break;

				default:
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYMSG : MSGFONTID_YOURMSG)));
						str.AppendChar(' ');
					}

					ptrT tszText(DbGetEventTextT(&dbei, CP_ACP));
					AppendUnicodeToBuffer(str, tszText, 0);
				}
				break;
			case '*':       // bold
				str.Append(isBold ? "\\b0 " : "\\b ");
				isBold = !isBold;
				break;
			case '/': // italic
				str.Append(isItalic ? "\\i0 " : "\\i ");
				isItalic = !isItalic;
				break;
			case '_': // italic
				str.Append(isUnderline ? "\\ul0 " : "\\ul ");
				isUnderline = !isUnderline;
				break;
			case '-': // grid line
				color = szTemplate[i + 2];
				if (color >= '0' && color <= '4') {
					str.AppendFormat("\\par\\sl-1\\slmult0\\highlight%d\\cf%d\\-\\par\\sl0", MSGDLGFONTCOUNT + 8 + (color - '0'), MSGDLGFONTCOUNT + 7 + (color - '0'));
					i++;
				}
				else str.AppendFormat("\\par\\sl-1\\slmult0\\highlight%d\\cf%d\\-\\par\\sl0", MSGDLGFONTCOUNT + 4, MSGDLGFONTCOUNT + 4);
				break;
			case '~':       // font break (switch to default font...)
				str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYMSG : MSGFONTID_YOURMSG)));
				break;
			case 'H':        // highlight
				color = szTemplate[i + 2];
				if (color >= '0' && color <= '4') {
					str.AppendFormat("\\highlight%d", MSGDLGFONTCOUNT + 8 + (color - '0'));
					i++;
				}
				else str.AppendFormat("\\highlight%d", (MSGDLGFONTCOUNT + (dat->bIsHistory ? 5 : 1) + ((isSent) ? 1 : 0)));
				break;
			case '|':       // tab
				if (dwEffectiveFlags & MWF_LOG_INDENT)
					str.Append("\\tab");
				else
					str.Append(" ");
				break;
			case 'f':      // font tag...
				code = szTemplate[i + 2];
				{
					int fontindex = -1;
					switch (code) {
					case 'd':
						fontindex = iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME);
						break;
					case 'n':
						fontindex = iFontIDOffset + (isSent ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME);
						break;
					case 'm':
						fontindex = iFontIDOffset + (isSent ? MSGFONTID_MYMSG : MSGFONTID_YOURMSG);
						break;
					case 'M':
						fontindex = iFontIDOffset + (isSent ? MSGFONTID_MYMISC : MSGFONTID_YOURMSG);
						break;
					case 's':
						fontindex = isSent ? MSGFONTID_SYMBOLS_OUT : MSGFONTID_SYMBOLS_IN;
						break;
					}
					if (fontindex != -1) {
						i++;
						str.Append(GetRTFFont(fontindex));
					}
					else skipToNext = TRUE;
				}
				break;

			case 'c':      // font color (using one of the predefined 5 colors) or one of the standard font colors (m = message, d = date/time, n = nick)
				color = szTemplate[i + 2];
				if (color >= '0' && color <= '4') {
					str.AppendFormat("\\cf%d ", MSGDLGFONTCOUNT + 8 + (color - '0'));
					i++;
				}
				else if (color == 'd') {
					str.AppendFormat("\\cf%d ", iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME));
					i++;
				}
				else if (color == 'm') {
					str.AppendFormat("\\cf%d ", iFontIDOffset + (isSent ? MSGFONTID_MYMSG : MSGFONTID_YOURMSG));
					i++;
				}
				else if (color == 'n') {
					str.AppendFormat("\\cf%d ", iFontIDOffset + (isSent ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME));
					i++;
				}
				else if (color == 's') {
					str.AppendFormat("\\cf%d ", isSent ? MSGFONTID_SYMBOLS_OUT : MSGFONTID_SYMBOLS_IN);
					i++;
				}
				else skipToNext = TRUE;
				break;

			case '<':		// bidi tag
				str.Append("\\rtlmark\\rtlch ");
				break;
			case '>':		// bidi tag
				str.Append("\\ltrmark\\ltrch ");
				break;
			}
		skip:
			if (skipToNext) {
				i++;
				while (szTemplate[i] != '%' && i < iTemplateLen) i++;
			}
			else i += 2;
		}
		else {
			str.AppendFormat("{\\uc1\\u%d?}", (int)ci);
			i++;
		}
	}

	if (dat->hHistoryEvents)
		str.AppendFormat(dat->szMicroLf, MSGDLGFONTCOUNT + 1 + ((isSent) ? 1 : 0), hDbEvent);

	str.Append("\\par");

	if (streamData->dbei == 0)
		mir_free(dbei.pBlob);

	dat->iLastEventType = MAKELONG((dbei.flags & (DBEF_SENT | DBEF_READ | DBEF_RTL)), dbei.eventType);
	dat->lastEventTime = dbei.timestamp;
	return str.Detach();
}

static DWORD CALLBACK LogStreamInEvents(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	LogStreamData *dat = (LogStreamData*)dwCookie;

	if (dat->buffer == NULL) {
		dat->bufferOffset = 0;
		switch (dat->stage) {
		case STREAMSTAGE_HEADER:
			mir_free(dat->buffer);
			dat->buffer = CreateRTFHeader(dat->dlgDat);
			dat->stage = STREAMSTAGE_EVENTS;
			break;

		case STREAMSTAGE_EVENTS:
			if (dat->eventsToInsert) {
				do {
					mir_free(dat->buffer);
					dat->buffer = Template_CreateRTFFromDbEvent(dat->dlgDat, dat->hContact, dat->hDbEvent, dat);
					if (dat->buffer)
						dat->hDbEventLast = dat->hDbEvent;
					dat->hDbEvent = db_event_next(dat->hContact, dat->hDbEvent);
					if (--dat->eventsToInsert == 0)
						break;
				} while (dat->buffer == NULL && dat->hDbEvent);

				if (dat->buffer)
					break;
			}
			dat->stage = STREAMSTAGE_TAIL;

		// fall through
		case STREAMSTAGE_TAIL:
			mir_free(dat->buffer);
			dat->buffer = CreateRTFTail();
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
	if (dat->bufferOffset == dat->bufferLen)
		replaceStr(dat->buffer, NULL);
	return 0;
}

static void SetupLogFormatting(TWindowData *dat)
{
	if (dat->hHistoryEvents)
		strncpy_s(dat->szMicroLf, "\\v\\cf%d \\ ~-+%d+-~\\v0 ", _TRUNCATE);
	else
		mir_snprintf(dat->szMicroLf, "%s\\par\\ltrpar\\sl-1%s ", GetRTFFont(MSGDLGFONTCOUNT), GetRTFFont(MSGDLGFONTCOUNT));
}

static void ReplaceIcons(HWND hwndDlg, TWindowData *dat, LONG startAt, int fAppend, BOOL isSent)
{
	DWORD dwScale = M.GetDword("iconscale", 0);

	TCHAR trbuffer[40];
	TEXTRANGE tr;
	tr.lpstrText = trbuffer;

	HWND hwndrtf = GetDlgItem(hwndDlg, IDC_LOG);

	FINDTEXTEX fi;
	fi.chrg.cpMin = startAt;
	if (dat->clr_added) {
		fi.lpstrText = _T("##col##");
		fi.chrg.cpMax = -1;
		CHARFORMAT2 cf2;
		memset(&cf2, 0, sizeof(cf2));
		cf2.cbSize = sizeof(cf2);
		cf2.dwMask = CFM_COLOR;
		while (SendMessage(hwndrtf, EM_FINDTEXTEX, FR_DOWN, (LPARAM)&fi) > -1) {
			tr.chrg.cpMin = fi.chrgText.cpMin;
			tr.chrg.cpMax = tr.chrg.cpMin + 18;
			trbuffer[0] = 0;
			SendMessage(hwndrtf, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
			trbuffer[18] = 0;

			CHARRANGE cr;
			cr.cpMin = fi.chrgText.cpMin;
			cr.cpMax = cr.cpMin + 18;
			SendMessage(hwndrtf, EM_EXSETSEL, 0, (LPARAM)&cr);
			SendMessage(hwndrtf, EM_REPLACESEL, FALSE, (LPARAM)_T(""));
			UINT length = (unsigned int)_ttol(&trbuffer[7]);
			int index = _ttol(&trbuffer[14]);
			if (length > 0 && length < 20000 && index >= RTF_CTABLE_DEFSIZE && index < Utils::rtf_ctable_size) {
				cf2.crTextColor = Utils::rtf_ctable[index].clr;
				cr.cpMin = fi.chrgText.cpMin;
				cr.cpMax = cr.cpMin + length;
				SendMessage(hwndrtf, EM_EXSETSEL, 0, (LPARAM)&cr);
				SendMessage(hwndrtf, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2);
			}
		}
	}

	fi.chrg.cpMin = startAt;
	if (dat->dwFlags & MWF_LOG_SHOWICONS) {
		BYTE bIconIndex = 0;
		char bDirection = 0;
		fi.lpstrText = _T("#~#");
		fi.chrg.cpMax = -1;

		CHARFORMAT2 cf2;
		memset(&cf2, 0, sizeof(cf2));
		cf2.cbSize = sizeof(cf2);
		cf2.dwMask = CFM_BACKCOLOR;

		IRichEditOle *ole;
		SendMessage(hwndrtf, EM_GETOLEINTERFACE, 0, (LPARAM)&ole);
		while (SendMessageA(hwndrtf, EM_FINDTEXTEX, FR_DOWN, (LPARAM)&fi) > -1) {
			CHARRANGE cr;
			cr.cpMin = fi.chrgText.cpMin;
			cr.cpMax = fi.chrgText.cpMax + 2;
			SendMessage(hwndrtf, EM_EXSETSEL, 0, (LPARAM)&cr);

			tr.chrg.cpMin = fi.chrgText.cpMin + 3;
			tr.chrg.cpMax = fi.chrgText.cpMin + 5;
			SendMessage(hwndrtf, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
			bIconIndex = ((BYTE)trbuffer[0] - (BYTE)'0');
			if (bIconIndex >= NR_LOGICONS) {
				fi.chrg.cpMin = fi.chrgText.cpMax + 6;
				continue;
			}
			bDirection = trbuffer[1];
			SendMessage(hwndrtf, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2);
			COLORREF crDefault = cf2.crBackColor == 0 ? (true ? (bDirection == '>' ? (fAppend ? dat->pContainer->theme.outbg : dat->pContainer->theme.oldoutbg) :
				(fAppend ? dat->pContainer->theme.inbg : dat->pContainer->theme.oldinbg)) : dat->pContainer->theme.bg) : cf2.crBackColor;

			TLogIcon theIcon;
			CacheIconToBMP(&theIcon, Logicons[bIconIndex], crDefault, dwScale, dwScale);
			ImageDataInsertBitmap(ole, theIcon.hBmp);
			DeleteCachedIcon(&theIcon);
			fi.chrg.cpMin = cr.cpMax + 6;
		}
		ole->Release();
	}

	// do smiley replacing, using the service
	if (PluginConfig.g_SmileyAddAvail) {
		CHARRANGE sel;
		sel.cpMin = startAt;
		sel.cpMax = -1;

		SMADD_RICHEDIT3 smadd = { sizeof(smadd) };
		smadd.hwndRichEditControl = hwndrtf;
		smadd.Protocolname = const_cast<char *>(dat->cache->getActiveProto());
		smadd.hContact = dat->cache->getActiveContact();
		smadd.flags = isSent ? SAFLRE_OUTGOING : 0;
		if (startAt > 0)
			smadd.rangeToReplace = &sel;
		else
			smadd.rangeToReplace = NULL;
		smadd.disableRedraw = TRUE;
		if (dat->bShowSmileys)
			CallService(MS_SMILEYADD_REPLACESMILEYS, TABSRMM_SMILEYADD_BKGCOLORMODE, (LPARAM)&smadd);
	}

	if (dat->hHistoryEvents && dat->curHistory == dat->maxHistory) {
		char szPattern[50];
		mir_snprintf(szPattern, "~-+%d+-~", (INT_PTR)dat->hHistoryEvents[0]);

		FINDTEXTEXA ft;
		ft.lpstrText = szPattern;
		ft.chrg.cpMin = 0;
		ft.chrg.cpMax = -1;
		if (SendMessageA(hwndrtf, EM_FINDTEXTEX, FR_DOWN, (LPARAM)&ft) != 0) {
			CHARRANGE sel;
			sel.cpMin = 0;
			sel.cpMax = 20;
			SendMessage(hwndrtf, EM_SETSEL, 0, ft.chrgText.cpMax + 1);
			SendMessageA(hwndrtf, EM_REPLACESEL, TRUE, (LPARAM)"");
		}
	}
}

void TSAPI StreamInEvents(HWND hwndDlg, MEVENT hDbEventFirst, int count, int fAppend, DBEVENTINFO *dbei_s)
{
	TWindowData *dat = (TWindowData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	CHARRANGE oldSel, sel;

	// calc time limit for grouping
	HWND hwndrtf = dat->hwndIEView ? dat->hwndIWebBrowserControl : GetDlgItem(hwndDlg, IDC_LOG);

	rtfFonts = dat->pContainer->theme.rtfFonts ? dat->pContainer->theme.rtfFonts : &(rtfFontsGlobal[0][0]);
	time_t now = time(NULL);

	struct tm tm_now = *localtime(&now);
	struct tm tm_today = tm_now;
	tm_today.tm_hour = tm_today.tm_min = tm_today.tm_sec = 0;
	today = mktime(&tm_today);

	if (dat->hwndIEView != NULL || dat->hwndHPP != NULL) {
		const char *pszService;
		IEVIEWEVENT event = { 0 };
		event.cbSize = sizeof(IEVIEWEVENT);
		event.hContact = dat->hContact;
		if (dat->hwndIEView != NULL) {
			event.pszProto = dat->szProto;
			event.hwnd = dat->hwndIEView;
			pszService = MS_IEVIEW_EVENT;
		}
		else {
			event.hwnd = dat->hwndHPP;
			pszService = MS_HPP_EG_EVENT;
		}

		if (dat->dwFlags & MWF_LOG_RTL)
			event.dwFlags = IEEF_RTL;

		if (!fAppend) {
			event.iType = IEE_CLEAR_LOG;
			CallService(pszService, 0, (LPARAM)&event);
		}

		IEVIEWEVENTDATA evData = { 0 };
		if (dbei_s != NULL && hDbEventFirst == 0) {
			evData.cbSize = sizeof(evData);
			event.iType = IEE_LOG_MEM_EVENTS;
			if (dbei_s->flags & DBEF_SENT)
				evData.dwFlags = IEEDF_SENT;
			else {
				evData.dwFlags = IEEDF_UNICODE_NICK;
				evData.ptszNick = pcli->pfnGetContactDisplayName(dat->hContact, 0);
			}
			switch (dbei_s->eventType) {
				case EVENTTYPE_STATUSCHANGE: evData.iType = IEED_EVENT_STATUSCHANGE; break;
				case EVENTTYPE_FILE: evData.iType = IEED_EVENT_FILE; break;
				case EVENTTYPE_ERRMSG: evData.iType = IEED_EVENT_ERRMSG; break;
				default: evData.iType = IEED_EVENT_MESSAGE; break;
			}
			evData.pszText = (char*)dbei_s->pBlob;
			evData.time = dbei_s->timestamp;
			event.eventData = &evData;
			event.codepage = CP_UTF8;
		}
		else {
			event.iType = IEE_LOG_DB_EVENTS;
			event.hDbEventFirst = hDbEventFirst;
		}
		event.count = count;
		CallService(pszService, 0, (LPARAM)&event);
		DM_ScrollToBottom(dat, 0, 0);
		if (fAppend && hDbEventFirst)
			dat->hDbEventLast = hDbEventFirst;
		else
			dat->hDbEventLast = db_event_last(dat->hContact);
		return;
	}

	// separator strings used for grid lines, message separation and so on...
	dat->clr_added = FALSE;

	if (dat->szMicroLf[0] == 0)
		SetupLogFormatting(dat);

	szYourName = const_cast<TCHAR *>(dat->cache->getNick());
	szMyName = dat->szMyNickname;

	SendMessage(hwndrtf, EM_HIDESELECTION, TRUE, 0);
	SendMessage(hwndrtf, EM_EXGETSEL, 0, (LPARAM)&oldSel);

	LogStreamData streamData = { 0 };
	streamData.hContact = dat->hContact;
	streamData.hDbEvent = hDbEventFirst;
	streamData.dlgDat = dat;
	streamData.eventsToInsert = count;
	streamData.isEmpty = fAppend ? GetWindowTextLength(hwndrtf) == 0 : 1;
	streamData.dbei = dbei_s;
	streamData.isAppend = fAppend;

	EDITSTREAM stream = { 0 };
	stream.pfnCallback = LogStreamInEvents;
	stream.dwCookie = (DWORD_PTR)&streamData;

	LONG startAt;
	if (fAppend) {
		GETTEXTLENGTHEX gtxl = { 0 };
		gtxl.codepage = 1200;
		gtxl.flags = GTL_DEFAULT | GTL_PRECISE | GTL_NUMCHARS;
		startAt = SendMessage(hwndrtf, EM_GETTEXTLENGTHEX, (WPARAM)&gtxl, 0);
		sel.cpMin = sel.cpMax = GetWindowTextLength(hwndrtf);
		SendMessage(hwndrtf, EM_EXSETSEL, 0, (LPARAM)&sel);
	}
	else {
		SetWindowText(hwndrtf, _T(""));
		sel.cpMin = 0;
		sel.cpMax = GetWindowTextLength(hwndrtf);
		SendMessage(hwndrtf, EM_EXSETSEL, 0, (LPARAM)&sel);
		startAt = 0;
		dat->isAutoRTL = 0;
	}

	// begin to draw
	SendMessage(hwndrtf, WM_SETREDRAW, FALSE, 0);

	SendMessage(hwndrtf, EM_STREAMIN, fAppend ? SFF_SELECTION | SF_RTF : SFF_SELECTION | SF_RTF, (LPARAM)&stream);
	SendMessage(hwndrtf, EM_EXSETSEL, 0, (LPARAM)&oldSel);
	SendMessage(hwndrtf, EM_HIDESELECTION, FALSE, 0);
	dat->hDbEventLast = streamData.hDbEventLast;

	if (dat->isAutoRTL & 1)
		SendMessage(hwndrtf, EM_SETBKGNDCOLOR, 0, (LOWORD(dat->iLastEventType) & DBEF_SENT) ? (fAppend ? dat->pContainer->theme.outbg : dat->pContainer->theme.oldoutbg) :
		(fAppend ? dat->pContainer->theme.inbg : dat->pContainer->theme.oldinbg));

	if (!(dat->isAutoRTL & 1)) {
		GETTEXTLENGTHEX gtxl = { 0 };
		gtxl.codepage = 1200;
		gtxl.flags = GTL_DEFAULT | GTL_PRECISE | GTL_NUMCHARS;

		sel.cpMax = SendMessage(hwndrtf, EM_GETTEXTLENGTHEX, (WPARAM)&gtxl, 0);
		sel.cpMin = sel.cpMax - 1;
		SendMessage(hwndrtf, EM_EXSETSEL, 0, (LPARAM)&sel);
		SendMessage(hwndrtf, EM_REPLACESEL, FALSE, (LPARAM)_T(""));
		dat->isAutoRTL |= 2;
	}

	BOOL isSent;
	if (streamData.dbei != 0)
		isSent = (streamData.dbei->flags & DBEF_SENT) != 0;
	else {
		DBEVENTINFO dbei = { sizeof(dbei) };
		db_event_get(hDbEventFirst, &dbei);
		isSent = (dbei.flags & DBEF_SENT) != 0;
	}

	ReplaceIcons(hwndDlg, dat, startAt, fAppend, isSent);
	dat->clr_added = FALSE;

	if (dat->hwndIEView == NULL && dat->hwndHPP == NULL) {
		int len = GetWindowTextLength(hwndrtf);
		SendMessage(hwndrtf, EM_SETSEL, len - 1, len - 1);
	}

	DM_ScrollToBottom(dat, 0, 0);

	SendMessage(hwndrtf, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(hwndrtf, NULL, FALSE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_QUOTE), dat->hDbEventLast != NULL);
	mir_free(streamData.buffer);
}
