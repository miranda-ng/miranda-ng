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
// implements the richedit based message log and the template parser

#include "stdafx.h"

struct TCpTable cpTable[] = {
	{ 874, LPGENW("Thai") },
	{ 932, LPGENW("Japanese") },
	{ 936, LPGENW("Simplified Chinese") },
	{ 949, LPGENW("Korean") },
	{ 950, LPGENW("Traditional Chinese") },
	{ 1250, LPGENW("Central European") },
	{ 1251, LPGENW("Cyrillic") },
	{ 20866, LPGENW("Cyrillic KOI8-R") },
	{ 1252, LPGENW("Latin I") },
	{ 1253, LPGENW("Greek") },
	{ 1254, LPGENW("Turkish") },
	{ 1255, LPGENW("Hebrew") },
	{ 1256, LPGENW("Arabic") },
	{ 1257, LPGENW("Baltic") },
	{ 1258, LPGENW("Vietnamese") },
	{ 1361, LPGENW("Korean (Johab)") },
	{ (UINT)-1, nullptr }
};

wchar_t* weekDays[7] = { LPGENW("Sunday"), LPGENW("Monday"), LPGENW("Tuesday"), LPGENW("Wednesday"), LPGENW("Thursday"), LPGENW("Friday"), LPGENW("Saturday") };

wchar_t* months[12] =
{
	LPGENW("January"), LPGENW("February"), LPGENW("March"), LPGENW("April"), LPGENW("May"), LPGENW("June"),
	LPGENW("July"), LPGENW("August"), LPGENW("September"), LPGENW("October"), LPGENW("November"), LPGENW("December")
};

static time_t today;

int g_groupBreak = TRUE;
static wchar_t *szMyName = nullptr;
static wchar_t *szYourName = nullptr;

static int logPixelSY;
static wchar_t szToday[22], szYesterday[22];
char  rtfFontsGlobal[MSGDLGFONTCOUNT + 2][RTFCACHELINESIZE];
char *rtfFonts;

LOGFONTW logfonts[MSGDLGFONTCOUNT + 2];
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
struct LogStreamData
{
	int stage;
	MCONTACT hContact;
	MEVENT hDbEvent, hDbEventLast;
	char *buffer;
	int bufferOffset, bufferLen;
	int eventsToInsert;
	int isEmpty;
	int isAppend;
	CMsgDialog *dlgDat;
	DBEVENTINFO *dbei;
};

__forceinline char* GetRTFFont(uint32_t dwIndex)
{
	return rtfFonts + (dwIndex * RTFCACHELINESIZE);
}

void TSAPI CacheLogFonts()
{
	HDC hdc = GetDC(nullptr);
	logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(nullptr, hdc);

	memset(logfonts, 0, (sizeof(LOGFONTA) * (MSGDLGFONTCOUNT + 2)));
	for (int i = 0; i < MSGDLGFONTCOUNT; i++) {
		LoadMsgDlgFont(FONTSECTION_IM, i, &logfonts[i], &fontcolors[i]);
		mir_snprintf(rtfFontsGlobal[i], "\\f%u\\cf%u\\b%d\\i%d\\ul%d\\fs%u", i, i, logfonts[i].lfWeight >= FW_BOLD ? 1 : 0, logfonts[i].lfItalic, logfonts[i].lfUnderline, 2 * abs(logfonts[i].lfHeight) * 74 / logPixelSY);
	}
	mir_snprintf(rtfFontsGlobal[MSGDLGFONTCOUNT], "\\f%u\\cf%u\\b%d\\i%d\\fs%u", MSGDLGFONTCOUNT, MSGDLGFONTCOUNT, 0, 0, 0);

	wcsncpy_s(szToday, TranslateT("Today"), _TRUNCATE);
	wcsncpy_s(szYesterday, TranslateT("Yesterday"), _TRUNCATE);
	szToday[19] = szYesterday[19] = 0;

	// cache/create the info panel fonts
	for (int i = 0; i < IPFONTCOUNT; i++) {
		if (CInfoPanel::m_ipConfig.hFonts[i])
			DeleteObject(CInfoPanel::m_ipConfig.hFonts[i]);

		COLORREF clr;
		LOGFONTW lf;
		LoadMsgDlgFont(FONTSECTION_IP, i, &lf, &clr);
		lf.lfUnderline = 0;
		CInfoPanel::m_ipConfig.hFonts[i] = CreateFontIndirectW(&lf);
		CInfoPanel::m_ipConfig.clrs[i] = clr;
	}

	hdc = GetDC(PluginConfig.g_hwndHotkeyHandler);

	HFONT hOldFont = (HFONT)SelectObject(hdc, CInfoPanel::m_ipConfig.hFonts[IPFONTID_NICK]);

	SIZE sz;
	GetTextExtentPoint32(hdc, L"WMA", 3, &sz);
	CInfoPanel::m_ipConfig.height1 = sz.cy;
	SelectObject(hdc, CInfoPanel::m_ipConfig.hFonts[IPFONTID_UIN]);
	GetTextExtentPoint32(hdc, L"WMA", 3, &sz);
	CInfoPanel::m_ipConfig.height2 = sz.cy;

	SelectObject(hdc, hOldFont);
	ReleaseDC(PluginConfig.g_hwndHotkeyHandler, hdc);
	PluginConfig.hFontCaption = CInfoPanel::m_ipConfig.hFonts[IPFONTCOUNT - 1];

	PluginConfig.crIncoming = db_get_dw(0, FONTMODULE, "inbg", SRMSGDEFSET_BKGINCOLOUR);
	PluginConfig.crOutgoing = db_get_dw(0, FONTMODULE, "outbg", SRMSGDEFSET_BKGOUTCOLOUR);
	PluginConfig.crStatus = db_get_dw(0, FONTMODULE, "statbg", SRMSGDEFSET_BKGCOLOUR);
	PluginConfig.crOldIncoming = db_get_dw(0, FONTMODULE, "oldinbg", SRMSGDEFSET_BKGINCOLOUR);
	PluginConfig.crOldOutgoing = db_get_dw(0, FONTMODULE, "oldoutbg", SRMSGDEFSET_BKGOUTCOLOUR);
}

void FreeLogFonts()
{
	for (int i = 0; i < IPFONTCOUNT; i++)
		if (CInfoPanel::m_ipConfig.hFonts[i])
			DeleteObject(CInfoPanel::m_ipConfig.hFonts[i]);
}

/////////////////////////////////////////////////////////////////////////////////////////

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

struct TLogIcon
{
	TLogIcon(HICON, COLORREF);
	~TLogIcon();

	HBITMAP m_hBmp;
	HDC m_hdc, m_hdcMem;
	HBRUSH m_hBkgBrush;
};

TLogIcon::TLogIcon(HICON hIcon, COLORREF backgroundColor)
{
	int IconSizeX = 0, IconSizeY = 0;
	Utils::getIconSize(hIcon, IconSizeX, IconSizeY);

	m_hBkgBrush = CreateSolidBrush(backgroundColor);

	BITMAPINFOHEADER bih = { 0 };
	bih.biSize = sizeof(bih);
	bih.biBitCount = 24;
	bih.biPlanes = 1;
	bih.biCompression = BI_RGB;
	bih.biHeight = IconSizeY;
	bih.biWidth = IconSizeX;

	RECT rc;
	rc.top = rc.left = 0;
	rc.right = bih.biWidth;
	rc.bottom = bih.biHeight;

	m_hdc = GetDC(nullptr);
	m_hBmp = CreateCompatibleBitmap(m_hdc, bih.biWidth, bih.biHeight);
	m_hdcMem = CreateCompatibleDC(m_hdc);

	HBITMAP hoBmp = (HBITMAP)SelectObject(m_hdcMem, m_hBmp);
	FillRect(m_hdcMem, &rc, m_hBkgBrush);
	DrawIconEx(m_hdcMem, 0, 0, hIcon, bih.biWidth, bih.biHeight, 0, nullptr, DI_NORMAL);
	SelectObject(m_hdcMem, hoBmp);
}

TLogIcon::~TLogIcon()
{
	DeleteDC(m_hdcMem);
	DeleteObject(m_hBmp);
	ReleaseDC(nullptr, m_hdc);
	DeleteObject(m_hBkgBrush);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int TSAPI GetColorIndex(char *rtffont)
{
	char *p;

	if ((p = strstr(rtffont, "\\cf")) != nullptr)
		return atoi(p + 3);
	return 0;
}

static int AppendUnicodeToBuffer(CMStringA &str, const wchar_t *line, int mode)
{
	str.Append("{\\uc1 ");

	int textCharsCount = 0;
	for (; *line; line++, textCharsCount++) {
		if (*line == 127 && line[1] != 0) {
			wchar_t code = line[2];
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

static void Build_RTF_Header(CMStringA &str, CMsgDialog *dat)
{
	int i;
	LOGFONTW *logFonts = dat->m_pContainer->m_theme.logFonts;
	COLORREF *fontColors = dat->m_pContainer->m_theme.fontColors;
	TLogTheme *theme = &dat->m_pContainer->m_theme;

	str.Append("{\\rtf1\\ansi\\deff0{\\fonttbl");

	for (i = 0; i < MSGDLGFONTCOUNT; i++)
		str.AppendFormat("{\\f%u\\fnil\\fcharset%u %S;}", i, logFonts[i].lfCharSet, logFonts[i].lfFaceName);
	str.AppendFormat("{\\f%u\\fnil\\fcharset%u %s;}", MSGDLGFONTCOUNT, logFonts[i].lfCharSet, "Arial");

	str.Append("}{\\colortbl ");
	for (i = 0; i < MSGDLGFONTCOUNT; i++)
		str.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(fontColors[i]), GetGValue(fontColors[i]), GetBValue(fontColors[i]));

	COLORREF colour = (GetSysColorBrush(COLOR_HOTLIGHT) == nullptr) ? RGB(0, 0, 255) : GetSysColor(COLOR_HOTLIGHT);
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
	for (auto &p : Utils::rtf_clrs)
		str.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(p->clr), GetGValue(p->clr), GetBValue(p->clr));

	// paragraph header
	str.AppendFormat("}");

	// indent
	if (!(dat->m_dwFlags & MWF_LOG_INDENT))
		str.AppendFormat("\\li%u\\ri%u\\fi%u\\tx%u", 2 * 15, 2 * 15, 0, 70 * 15);
}

// mir_free() the return value
static char* CreateRTFHeader(CMsgDialog *dat)
{
	CMStringA str;
	Build_RTF_Header(str, dat);
	return str.Detach();
}

static void AppendTimeStamp(wchar_t *szFinalTimestamp, int isSent, CMStringA &str, int skipFont, CMsgDialog *dat, int iFontIDOffset)
{
	if (!skipFont) {
		str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME)));
		str.AppendChar(' ');
	}

	AppendUnicodeToBuffer(str, szFinalTimestamp, MAKELONG(isSent, dat->m_bIsHistory));
}

static wchar_t* Template_MakeRelativeDate(HANDLE hTimeZone, time_t check, wchar_t code)
{
	static wchar_t szResult[100];
	const wchar_t *szFormat;

	if ((code == (wchar_t)'R' || code == (wchar_t)'r') && check >= today)
		mir_wstrcpy(szResult, szToday);
	else if ((code == (wchar_t)'R' || code == (wchar_t)'r') && check > (today - 86400))
		mir_wstrcpy(szResult, szYesterday);
	else {
		if (code == 'D' || code == 'R')
			szFormat = L"D";
		else if (code == 'T')
			szFormat = L"s";
		else if (code == 't')
			szFormat = L"t";
		else
			szFormat = L"d";

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
	DBEVENTTYPEDESCR *et = DbEvent_GetType(dbei->szModule, dbei->eventType);
	return et && (et->flags & DETF_MSGWINDOW);
}

static char* Template_CreateRTFFromDbEvent(CMsgDialog *dat, MCONTACT hContact, MEVENT hDbEvent, LogStreamData *streamData)
{
	HANDLE hTimeZone = nullptr;
	struct tm event_time = { 0 };
	bool skipToNext = false, skipFont = false;
	bool isBold = false, isItalic = false, isUnderline = false;

	DBEVENTINFO dbei = {};
	if (streamData->dbei != nullptr)
		dbei = *(streamData->dbei);
	else {
		dbei.cbBlob = -1;
		db_event_get(hDbEvent, &dbei);
		if (!DbEventIsShown(&dbei)) {
			mir_free(dbei.pBlob);
			return nullptr;
		}
	}

	if (dbei.eventType == EVENTTYPE_MESSAGE && !dbei.markedRead())
		dat->m_cache->updateStats(TSessionStats::SET_LAST_RCV, mir_strlen((char *)dbei.pBlob));

	BOOL isSent = (dbei.flags & DBEF_SENT);
	BOOL bIsStatusChangeEvent = IsStatusEvent(dbei.eventType);
	if (!isSent && (bIsStatusChangeEvent || dbei.eventType == EVENTTYPE_MESSAGE || DbEventIsForMsgWindow(&dbei))) {
		db_event_markRead(hContact, hDbEvent);
		g_clistApi.pfnRemoveEvent(hContact, hDbEvent);
	}

	CMStringW msg(ptrW(DbEvent_GetTextW(&dbei, CP_UTF8)));
	if (msg.IsEmpty()) {
		mir_free(dbei.pBlob);
		return nullptr;
	}
	msg.TrimRight();
	dat->FormatRaw(msg, 1, FALSE);

	CMStringA str;

	// means: last \\par was deleted to avoid new line at end of log
	if (dat->m_bLastParaDeleted) {
		str.Append("\\par");
		dat->m_bLastParaDeleted = false;
	}

	if (dat->m_dwFlags & MWF_LOG_RTL)
		dbei.flags |= DBEF_RTL;

	if (dbei.flags & DBEF_RTL)
		dat->m_bRtlText = true;

	uint32_t dwEffectiveFlags = dat->m_dwFlags;

	dat->m_bIsHistory = (dbei.timestamp < dat->m_cache->getSessionStart() && dbei.markedRead());
	int iFontIDOffset = dat->m_bIsHistory ? 8 : 0;     // offset into the font table for either history (old) or new events... (# of fonts per configuration set)

	g_groupBreak = TRUE;

	if (dat->m_bDividerWanted) {
		static char szStyle_div[128] = "\0";
		if (szStyle_div[0] == 0)
			mir_snprintf(szStyle_div, "\\f%u\\cf%u\\ul0\\b%d\\i%d\\fs%u", H_MSGFONTID_DIVIDERS, H_MSGFONTID_DIVIDERS, 0, 0, 5);

		str.AppendFormat("\\sl-1\\slmult0\\highlight%d\\cf%d\\-\\par\\sl0", H_MSGFONTID_DIVIDERS, H_MSGFONTID_DIVIDERS);
		dat->m_bDividerWanted = false;
	}

	if (dwEffectiveFlags & MWF_LOG_GROUPMODE && ((dbei.flags & (DBEF_SENT | DBEF_READ | DBEF_RTL)) == LOWORD(dat->m_iLastEventType)) && dbei.eventType == EVENTTYPE_MESSAGE && HIWORD(dat->m_iLastEventType) == EVENTTYPE_MESSAGE && (dbei.timestamp - dat->m_lastEventTime) < 86400) {
		g_groupBreak = FALSE;
		if ((time_t)dbei.timestamp > today && dat->m_lastEventTime < today)
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
		str.AppendFormat("\\highlight%d\\cf%d", MSGDLGFONTCOUNT + (dat->m_bIsHistory ? 5 : 1) + ((isSent) ? 1 : 0), MSGDLGFONTCOUNT + (dat->m_bIsHistory ? 5 : 1) + ((isSent) ? 1 : 0));

	streamData->isEmpty = FALSE;

	if (dat->m_bRtlText) {
		if (dbei.flags & DBEF_RTL)
			str.Append("\\ltrch\\rtlch");
		else
			str.Append("\\rtlch\\ltrch");
	}

	// templated code starts here
	if (dwEffectiveFlags & MWF_LOG_SHOWTIME) {
		hTimeZone = ((dat->m_dwFlags & MWF_LOG_LOCALTIME) && !isSent) ? dat->m_hTimeZone : nullptr;
		time_t local_time = TimeZone_UtcToLocal(hTimeZone, dbei.timestamp);
		event_time = *gmtime(&local_time);
	}

	TTemplateSet *this_templateset = dbei.flags & DBEF_RTL ? dat->m_pContainer->m_rtl_templates : dat->m_pContainer->m_ltr_templates;

	wchar_t *szTemplate;
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

	size_t iTemplateLen = mir_wstrlen(szTemplate);
	BOOL showTime = dwEffectiveFlags & MWF_LOG_SHOWTIME;
	BOOL showDate = dwEffectiveFlags & MWF_LOG_SHOWDATES;

	if (dat->m_hHistoryEvents) {
		if (dat->m_curHistory == dat->m_maxHistory) {
			memmove(dat->m_hHistoryEvents, &dat->m_hHistoryEvents[1], sizeof(HANDLE) * (dat->m_maxHistory - 1));
			dat->m_curHistory--;
		}
		dat->m_hHistoryEvents[dat->m_curHistory++] = hDbEvent;
	}

	str.Append("\\ul0\\b0\\i0\\v0 ");

	for (size_t i = 0; i < iTemplateLen;) {
		wchar_t ci = szTemplate[i];
		if (ci == '%') {
			wchar_t cc = szTemplate[i + 1];
			skipToNext = skipFont = false;

			// handle modifiers
			while (cc == '#' || cc == '$' || cc == '&' || cc == '?' || cc == '\\') {
				switch (cc) {
				case '#':
					if (!dat->m_bIsHistory) {
						skipToNext = true;
						goto skip;
					}
					i++;
					cc = szTemplate[i + 1];
					continue;

				case '$':
					if (dat->m_bIsHistory) {
						skipToNext = true;
						goto skip;
					}
					i++;
					cc = szTemplate[i + 1];
					continue;

				case '&':
					i++;
					cc = szTemplate[i + 1];
					skipFont = true;
					break;

				case '?':
					if (dwEffectiveFlags & MWF_LOG_NORMALTEMPLATES) {
						i++;
						cc = szTemplate[i + 1];
						continue;
					}
					i++;
					skipToNext = true;
					goto skip;

				case '\\':
					if (!(dwEffectiveFlags & MWF_LOG_NORMALTEMPLATES)) {
						i++;
						cc = szTemplate[i + 1];
						continue;
					}
					i++;
					skipToNext = true;
					goto skip;
				}
			}

			wchar_t color, code;
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
				else skipToNext = true;
				break;
			case 'D': // long date
				if (showTime && showDate) {
					wchar_t *szFinalTimestamp = Template_MakeRelativeDate(hTimeZone, dbei.timestamp, 'D');
					AppendTimeStamp(szFinalTimestamp, isSent, str, skipFont, dat, iFontIDOffset);
				}
				else skipToNext = true;
				break;
			case 'E': // short date...
				if (showTime && showDate) {
					wchar_t *szFinalTimestamp = Template_MakeRelativeDate(hTimeZone, dbei.timestamp, 'E');
					AppendTimeStamp(szFinalTimestamp, isSent, str, skipFont, dat, iFontIDOffset);
				}
				else skipToNext = true;
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
				else skipToNext = true;
				break;
			case 'm': // minute
				if (showTime) {
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME)));
						str.AppendChar(' ');
					}
					str.AppendFormat("%02d", event_time.tm_min);
				}
				else skipToNext = true;
				break;
			case 's': //second
				if (showTime && (dwEffectiveFlags & MWF_LOG_SHOWSECONDS)) {
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME)));
						str.AppendChar(' ');
					}
					str.AppendFormat("%02d", event_time.tm_sec);
				}
				else skipToNext = true;
				break;
			case 'p': // am/pm symbol
				if (showTime) {
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME)));
						str.AppendChar(' ');
					}
					str.Append(event_time.tm_hour > 11 ? "PM" : "AM");
				}
				else skipToNext = true;
				break;
			case 'o':            // month
				if (showTime && showDate) {
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME)));
						str.AppendChar(' ');
					}
					str.AppendFormat("%02d", event_time.tm_mon + 1);
				}
				else skipToNext = true;
				break;
			case 'O': // month (name)
				if (showTime && showDate) {
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME)));
						str.AppendChar(' ');
					}
					AppendUnicodeToBuffer(str, TranslateW(months[event_time.tm_mon]), MAKELONG(isSent, dat->m_bIsHistory));
				}
				else skipToNext = true;
				break;
			case 'd': // day of month
				if (showTime && showDate) {
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME)));
						str.AppendChar(' ');
					}
					str.AppendFormat("%02d", event_time.tm_mday);
				}
				else skipToNext = true;
				break;
			case 'w': // day of week
				if (showTime && showDate) {
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME)));
						str.AppendChar(' ');
					}
					AppendUnicodeToBuffer(str, TranslateW(weekDays[event_time.tm_wday]), MAKELONG(isSent, dat->m_bIsHistory));
				}
				else skipToNext = true;
				break;
			case 'y': // year
				if (showTime && showDate) {
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME)));
						str.AppendChar(' ');
					}
					str.AppendFormat("%04d", event_time.tm_year + 1900);
				}
				else skipToNext = true;
				break;
			case 'R':
			case 'r': // long date
				if (showTime && showDate) {
					wchar_t *szFinalTimestamp = Template_MakeRelativeDate(hTimeZone, dbei.timestamp, cc);
					AppendTimeStamp(szFinalTimestamp, isSent, str, skipFont, dat, iFontIDOffset);
				}
				else skipToNext = true;
				break;
			case 't':
			case 'T':
				if (showTime) {
					wchar_t *szFinalTimestamp = Template_MakeRelativeDate(hTimeZone, dbei.timestamp, (wchar_t)((dwEffectiveFlags & MWF_LOG_SHOWSECONDS) ? cc : (wchar_t)'t'));
					AppendTimeStamp(szFinalTimestamp, isSent, str, skipFont, dat, iFontIDOffset);
				}
				else skipToNext = true;
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
				else skipToNext = true;
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
				AppendUnicodeToBuffer(str, (isSent) ? szMyName : szYourName, MAKELONG(isSent, dat->m_bIsHistory));
				break;
			case 'U': // UIN
				if (!skipFont) {
					str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME)));
					str.AppendChar(' ');
				}
				AppendUnicodeToBuffer(str, (isSent) ? dat->m_myUin : dat->m_cache->getUIN(), MAKELONG(isSent, dat->m_bIsHistory));
				break;
			case 'e': // error message
				str.Append(GetRTFFont(MSGFONTID_ERROR));
				str.AppendChar(' ');
				AppendUnicodeToBuffer(str, LPCTSTR(dbei.szModule), MAKELONG(isSent, dat->m_bIsHistory));
				break;
			case 'M': // message
				if (bIsStatusChangeEvent)
					dbei.eventType = EVENTTYPE_STATUSCHANGE;

				switch (dbei.eventType) {
				case EVENTTYPE_MESSAGE:
				case EVENTTYPE_ERRMSG:
				case EVENTTYPE_STATUSCHANGE:
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

					AppendUnicodeToBuffer(str, msg, MAKELONG(isSent, dat->m_bIsHistory));
					str.Append("\\b0\\ul0\\i0 ");
					break;

				case EVENTTYPE_FILE:
					if (!skipFont) {
						str.Append(GetRTFFont(iFontIDOffset + (isSent ? MSGFONTID_MYMISC : MSGFONTID_YOURMISC)));
						str.AppendChar(' ');
					}
					{
						char *szFileName = (char *)dbei.pBlob + sizeof(uint32_t);
						ptrW tszFileName(DbEvent_GetString(&dbei, szFileName));

						char *szDescr = szFileName + mir_strlen(szFileName) + 1;
						if (*szDescr != 0) {
							ptrW tszDescr(DbEvent_GetString(&dbei, szDescr));

							wchar_t buf[1000];
							mir_snwprintf(buf, L"%s (%s)", tszFileName.get(), tszDescr.get());
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

					ptrW tszText(DbEvent_GetTextW(&dbei, CP_ACP));
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
				else str.AppendFormat("\\highlight%d", (MSGDLGFONTCOUNT + (dat->m_bIsHistory ? 5 : 1) + ((isSent) ? 1 : 0)));
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
						fontindex = iFontIDOffset + (isSent ? MSGFONTID_MYMISC : MSGFONTID_YOURMISC);
						break;
					case 's':
						fontindex = isSent ? MSGFONTID_SYMBOLS_OUT : MSGFONTID_SYMBOLS_IN;
						break;
					}
					if (fontindex != -1) {
						i++;
						str.Append(GetRTFFont(fontindex));
					}
					else skipToNext = true;
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
				else skipToNext = true;
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
				while (szTemplate[i] != '%' && i < iTemplateLen)
					i++;
			}
			else i += 2;
		}
		else {
			if (ci >= 32 && ci < 128)
				str.AppendChar(ci);
			else
				str.AppendFormat("{\\uc1\\u%d?}", (int)ci);
			i++;
		}
	}

	if (dat->m_hHistoryEvents)
		str.AppendFormat(dat->m_szMicroLf, MSGDLGFONTCOUNT + 1 + ((isSent) ? 1 : 0), hDbEvent);

	str.Append("\\par");

	if (streamData->dbei == nullptr)
		mir_free(dbei.pBlob);

	dat->m_iLastEventType = MAKELONG((dbei.flags & (DBEF_SENT | DBEF_READ | DBEF_RTL)), dbei.eventType);
	dat->m_lastEventTime = dbei.timestamp;
	return str.Detach();
}

static DWORD CALLBACK LogStreamInEvents(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	LogStreamData *dat = (LogStreamData*)dwCookie;

	if (dat->buffer == nullptr) {
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
				} while (dat->buffer == nullptr && dat->hDbEvent);

				if (dat->buffer)
					break;
			}
			dat->stage = STREAMSTAGE_TAIL;
			__fallthrough;

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
		replaceStr(dat->buffer, nullptr);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CLogWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	bool isCtrl, isShift, isAlt;

	switch (msg) {
	case WM_KILLFOCUS:
		if (wParam != (WPARAM)m_rtf.GetHwnd() && 0 != wParam) {
			CHARRANGE cr;
			m_rtf.SendMsg(EM_EXGETSEL, 0, (LPARAM)&cr);
			if (cr.cpMax != cr.cpMin) {
				cr.cpMin = cr.cpMax;
				m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&cr);
			}
		}
		break;

	case WM_CHAR:
		m_pDlg.KbdState(isShift, isCtrl, isAlt);
		if (wParam == 0x03 && isCtrl) // Ctrl+C
			return m_pDlg.WMCopyHandler(msg, wParam, lParam);
		if (wParam == 0x11 && isCtrl) // Ctrl+Q
			m_pDlg.m_btnQuote.Click();
		break;

	case WM_LBUTTONUP:
		if (m_pDlg.isChat() && g_Settings.bClickableNicks) {
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };
			CheckCustomLink(m_rtf.GetHwnd(), &pt, msg, wParam, lParam, TRUE);
		}
		
		if (g_plugin.bAutoCopy) {
			CHARRANGE sel;
			SendMessage(m_rtf.GetHwnd(), EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin != sel.cpMax) {
				SendMessage(m_rtf.GetHwnd(), WM_COPY, 0, 0);
				sel.cpMin = sel.cpMax;
				SendMessage(m_rtf.GetHwnd(), EM_EXSETSEL, 0, (LPARAM)&sel);
				SetFocus(m_pDlg.m_message.GetHwnd());
			}
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
		if (m_pDlg.isChat() && g_Settings.bClickableNicks) {
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };
			CheckCustomLink(m_rtf.GetHwnd(), &pt, msg, wParam, lParam, TRUE);
		}
		break;

	case WM_SETCURSOR:
		if (g_Settings.bClickableNicks && m_pDlg.isChat() && (LOWORD(lParam) == HTCLIENT)) {
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(m_rtf.GetHwnd(), &pt);
			if (CheckCustomLink(m_rtf.GetHwnd(), &pt, msg, wParam, lParam, FALSE))
				return TRUE;
		}
		break;

	case WM_SYSKEYUP:
		if (wParam == VK_MENU) {
			m_pDlg.ProcessHotkeysByMsgFilter(m_rtf, msg, wParam, lParam);
			return 0;
		}
		break;

	case WM_SYSKEYDOWN:
		m_pDlg.m_bkeyProcessed = false;
		if (m_pDlg.ProcessHotkeysByMsgFilter(m_rtf, msg, wParam, lParam)) {
			m_pDlg.m_bkeyProcessed = true;
			return 0;
		}
		break;

	case WM_SYSCHAR:
		if (m_pDlg.m_bkeyProcessed) {
			m_pDlg.m_bkeyProcessed = false;
			return 0;
		}
		break;

	case WM_KEYDOWN:
		m_pDlg.KbdState(isShift, isCtrl, isAlt);
		if (wParam == VK_INSERT && isCtrl)
			return m_pDlg.WMCopyHandler(msg, wParam, lParam);

		if (wParam == 0x57 && isCtrl) { // ctrl-w (close window)
			PostMessage(m_pDlg.m_hwnd, WM_CLOSE, 0, 1);
			return TRUE;
		}

		break;

	case WM_COPY:
		return m_pDlg.WMCopyHandler(msg, wParam, lParam);

	case WM_NCCALCSIZE:
		return CSkin::NcCalcRichEditFrame(m_rtf.GetHwnd(), &m_pDlg, ID_EXTBKHISTORY, msg, wParam, lParam, stubLogProc);

	case WM_NCPAINT:
		return CSkin::DrawRichEditFrame(m_rtf.GetHwnd(), &m_pDlg, ID_EXTBKHISTORY, msg, wParam, lParam, stubLogProc);

	case WM_CONTEXTMENU:
		if (!m_pDlg.isChat()) {
			POINT pt;
			if (lParam == 0xFFFFFFFF) {
				CHARRANGE sel;
				m_rtf.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
				m_rtf.SendMsg(EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)sel.cpMax);
				ClientToScreen(m_rtf.GetHwnd(), &pt);
			}
			else {
				pt.x = GET_X_LPARAM(lParam);
				pt.y = GET_Y_LPARAM(lParam);
			}

			m_pDlg.ShowPopupMenu(m_rtf, pt);
			return TRUE;
		}
	}

	return CSuper::WndProc(msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CLogWindow::Attach()
{
	CSuper::Attach();

	m_rtf.SendMsg(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(0, 0));
	m_rtf.SendMsg(EM_AUTOURLDETECT, TRUE, 0);
	m_rtf.SendMsg(EM_EXLIMITTEXT, 0, 0x7FFFFFFF);
	m_rtf.SendMsg(EM_SETUNDOLIMIT, 0, 0);
	m_rtf.SendMsg(EM_HIDESELECTION, TRUE, 0);
	m_rtf.SendMsg(EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_KEYEVENTS | ENM_LINK);
	m_rtf.SendMsg(EM_SETEDITSTYLE, SES_EXTENDBACKCOLOR, SES_EXTENDBACKCOLOR);
	m_rtf.SendMsg(EM_SETLANGOPTIONS, 0, m_rtf.SendMsg(EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOFONTSIZEADJUST);
	m_rtf.SendMsg(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));
}

void CLogWindow::LogEvents(MEVENT hDbEventFirst, int count, bool fAppend)
{
	LogEvents(hDbEventFirst, count, fAppend, nullptr);
}

void CLogWindow::LogEvents(MEVENT hDbEventFirst, int count, bool fAppend, DBEVENTINFO *dbei_s)
{
	CHARRANGE oldSel, sel;

	// calc time limit for grouping
	rtfFonts = m_pDlg.m_pContainer->m_theme.rtfFonts ? m_pDlg.m_pContainer->m_theme.rtfFonts : &(rtfFontsGlobal[0][0]);
	time_t now = time(0);

	struct tm tm_now = *localtime(&now);
	struct tm tm_today = tm_now;
	tm_today.tm_hour = tm_today.tm_min = tm_today.tm_sec = 0;
	today = mktime(&tm_today);

	// separator strings used for grid lines, message separation and so on...
	m_pDlg.m_bClrAdded = false;

	if (m_pDlg.m_szMicroLf[0] == 0) {
		if (m_pDlg.m_hHistoryEvents)
			strncpy_s(m_pDlg.m_szMicroLf, "\\v\\cf%d \\ ~-+%d+-~\\v0 ", _TRUNCATE);
		else
			mir_snprintf(m_pDlg.m_szMicroLf, "%s\\par\\ltrpar\\sl-1%s ", GetRTFFont(MSGDLGFONTCOUNT), GetRTFFont(MSGDLGFONTCOUNT));
	}

	szYourName = const_cast<wchar_t *>(m_pDlg.m_cache->getNick());
	szMyName = m_pDlg.m_wszMyNickname;

	m_rtf.SendMsg(EM_HIDESELECTION, TRUE, 0);
	m_rtf.SendMsg(EM_EXGETSEL, 0, (LPARAM)&oldSel);

	LogStreamData streamData = { 0 };
	streamData.hContact = m_pDlg.m_hContact;
	streamData.hDbEvent = hDbEventFirst;
	streamData.dlgDat = &m_pDlg;
	streamData.eventsToInsert = count;
	streamData.isEmpty = fAppend ? GetWindowTextLength(m_rtf.GetHwnd()) == 0 : 1;
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
		startAt = m_rtf.SendMsg(EM_GETTEXTLENGTHEX, (WPARAM)&gtxl, 0);
		sel.cpMin = sel.cpMax = GetWindowTextLength(m_rtf.GetHwnd());
		m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
	}
	else {
		Clear();

		sel.cpMin = 0;
		sel.cpMax = GetWindowTextLength(m_rtf.GetHwnd());
		m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
		startAt = 0;
		m_pDlg.m_bLastParaDeleted = m_pDlg.m_bRtlText = false;
	}

	// begin to draw
	m_rtf.SetDraw(false);
	m_rtf.SendMsg(EM_STREAMIN, fAppend ? SFF_SELECTION | SF_RTF : SFF_SELECTION | SF_RTF, (LPARAM)&stream);

	m_pDlg.m_hDbEventLast = streamData.hDbEventLast;

	if (m_pDlg.m_bRtlText)
		m_rtf.SendMsg(EM_SETBKGNDCOLOR, 0, (LOWORD(m_pDlg.m_iLastEventType) & DBEF_SENT)
			? (fAppend ? m_pDlg.m_pContainer->m_theme.outbg : m_pDlg.m_pContainer->m_theme.oldoutbg)
			: (fAppend ? m_pDlg.m_pContainer->m_theme.inbg : m_pDlg.m_pContainer->m_theme.oldinbg));

	if (!dbei_s && !m_pDlg.m_bRtlText) {
		GETTEXTLENGTHEX gtxl = { 0 };
		gtxl.codepage = 1200;
		gtxl.flags = GTL_DEFAULT | GTL_PRECISE | GTL_NUMCHARS;

		sel.cpMax = m_rtf.SendMsg(EM_GETTEXTLENGTHEX, (WPARAM)&gtxl, 0);
		sel.cpMin = sel.cpMax - 1;
		m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
		m_rtf.SendMsg(EM_REPLACESEL, FALSE, (LPARAM)L"");
		m_pDlg.m_bLastParaDeleted = true;
	}

	BOOL isSent;
	if (streamData.dbei != nullptr)
		isSent = (streamData.dbei->flags & DBEF_SENT) != 0;
	else {
		DBEVENTINFO dbei = {};
		db_event_get(hDbEventFirst, &dbei);
		isSent = (dbei.flags & DBEF_SENT) != 0;
	}

	ReplaceIcons(startAt, fAppend, isSent);
	m_pDlg.m_bClrAdded = false;

	if (!m_pDlg.m_bScrollingDisabled) {
		int len = GetWindowTextLength(m_rtf.GetHwnd()) - 1;
		m_rtf.SendMsg(EM_SETSEL, len, len);
	}
	else m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&oldSel);

	m_rtf.SendMsg(EM_HIDESELECTION, FALSE, 0);

	m_rtf.SetDraw(true);
	InvalidateRect(m_rtf.GetHwnd(), nullptr, FALSE);
	EnableWindow(GetDlgItem(m_pDlg.m_hwnd, IDC_QUOTE), m_pDlg.m_hDbEventLast != 0);
	mir_free(streamData.buffer);
}

void CLogWindow::LogEvents(LOGINFO *lin, bool bRedraw)
{
	auto *si = m_pDlg.m_si;
	if (m_rtf.GetHwnd() == nullptr || lin == nullptr || si == nullptr)
		return;

	if (!bRedraw && m_pDlg.AllowTyping() && m_pDlg.m_bFilterEnabled && (m_pDlg.m_iLogFilterFlags & lin->iType) == 0)
		return;

	bool bFlag = false, bDoReplace, bAtBottom = AtBottom();

	LOGSTREAMDATA streamData;
	memset(&streamData, 0, sizeof(streamData));
	streamData.hwnd = m_rtf.GetHwnd();
	streamData.si = si;
	streamData.lin = lin;
	streamData.bStripFormat = FALSE;
	streamData.dat = &m_pDlg;

	EDITSTREAM stream = {};
	stream.pfnCallback = Srmm_LogStreamCallback;
	stream.dwCookie = (DWORD_PTR)&streamData;

	POINT point = { 0 };
	m_rtf.SendMsg(EM_GETSCROLLPOS, 0, (LPARAM)&point);

	// do not scroll to bottom if there is a selection
	CHARRANGE oldsel, sel, newsel;
	m_rtf.SendMsg(EM_EXGETSEL, 0, (LPARAM)&oldsel);
	if (oldsel.cpMax != oldsel.cpMin)
		m_rtf.SetDraw(false);

	// set the insertion point at the bottom
	sel.cpMin = sel.cpMax = m_rtf.GetRichTextLength();
	m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);

	// fix for the indent... must be a M$ bug
	if (sel.cpMax == 0)
		bRedraw = TRUE;

	// should the event(s) be appended to the current log
	WPARAM wp = bRedraw ? SF_RTF : SFF_SELECTION | SF_RTF;

	// get the number of pixels per logical inch
	if (bRedraw) {
		HDC hdc = GetDC(nullptr);
		g_chatApi.logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		g_chatApi.logPixelSX = GetDeviceCaps(hdc, LOGPIXELSX);
		ReleaseDC(nullptr, hdc);
		m_rtf.SetDraw(false);
		bFlag = true;
	}

	// stream in the event(s)
	streamData.lin = lin;
	streamData.bRedraw = bRedraw;
	m_rtf.SendMsg(EM_STREAMIN, wp, (LPARAM)&stream);

	// for new added events, only replace in message or action events.
	// no need to replace smileys or math formulas elsewhere
	bDoReplace = (bRedraw || (lin->ptszText && (lin->iType == GC_EVENT_MESSAGE || lin->iType == GC_EVENT_ACTION)));

	// replace marked nicknames with hyperlinks to make the nicks clickable
	if (g_Settings.bClickableNicks) {
		FINDTEXTEX fi, fi2;
		fi.chrg.cpMin = bRedraw ? 0 : sel.cpMin;
		fi.chrg.cpMax = -1;
		fi.lpstrText = CLICKNICK_BEGIN;

		while (m_rtf.SendMsg(EM_FINDTEXTEX, FR_DOWN, (LPARAM)&fi) > -1) {
			fi2.chrg.cpMin = fi.chrgText.cpMin;
			fi2.chrg.cpMax = -1;
			fi2.lpstrText = CLICKNICK_END;

			if (m_rtf.SendMsg(EM_FINDTEXTEX, FR_DOWN, (LPARAM)&fi2) > -1) {
				m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&fi.chrgText);
				m_rtf.SendMsg(EM_REPLACESEL, TRUE, (LPARAM)L"");

				fi2.chrgText.cpMin -= fi.chrgText.cpMax - fi.chrgText.cpMin;
				fi2.chrgText.cpMax -= fi.chrgText.cpMax - fi.chrgText.cpMin;
				m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&fi2.chrgText);
				m_rtf.SendMsg(EM_REPLACESEL, TRUE, (LPARAM)L"");

				fi2.chrgText.cpMax = fi2.chrgText.cpMin;
				fi2.chrgText.cpMin = fi.chrgText.cpMin;
				m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&fi2.chrgText);

				CHARFORMAT2 cf2;
				memset(&cf2, 0, sizeof(CHARFORMAT2));
				cf2.cbSize = sizeof(cf2);
				cf2.dwMask = CFM_PROTECTED;
				cf2.dwEffects = CFE_PROTECTED;
				m_rtf.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2);
			}
			fi.chrg.cpMin = fi.chrgText.cpMax;
		}
		m_rtf.SendMsg(EM_SETSEL, -1, -1);
	}

	// run smileyadd
	if (PluginConfig.g_SmileyAddAvail && bDoReplace) {
		newsel.cpMax = -1;
		newsel.cpMin = sel.cpMin;
		if (newsel.cpMin < 0)
			newsel.cpMin = 0;

		SMADD_RICHEDIT3 sm = { sizeof(sm) };
		sm.hwndRichEditControl = m_rtf.GetHwnd();
		sm.Protocolname = si->pszModule;
		sm.rangeToReplace = bRedraw ? nullptr : &newsel;
		sm.disableRedraw = TRUE;
		sm.hContact = si->hContact;
		CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM)&sm);
	}

	// trim the message log to the number of most recent events
	// this uses hidden marks in the rich text to find the events which should be deleted
	if (si->bTrimmed) {
		wchar_t szPattern[50];
		mir_snwprintf(szPattern, L"~-+%p+-~", si->pLogEnd);

		FINDTEXTEX fi;
		fi.lpstrText = szPattern;
		fi.chrg.cpMin = 0;
		fi.chrg.cpMax = -1;
		if (m_rtf.SendMsg(EM_FINDTEXTEX, FR_DOWN, (LPARAM)&fi) != 0) {
			m_rtf.SendMsg(EM_SETSEL, 0, fi.chrgText.cpMax + 1);
			m_rtf.SendMsg(EM_REPLACESEL, TRUE, (LPARAM)L"");
		}
		si->bTrimmed = false;
	}

	// scroll log to bottom if the log was previously scrolled to bottom, else restore old position
	if (bRedraw || bAtBottom)
		ScrollToBottom(false, false);
	else
		m_rtf.SendMsg(EM_SETSCROLLPOS, 0, (LPARAM)&point);

	// do we need to restore the selection
	if (oldsel.cpMax != oldsel.cpMin) {
		m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&oldsel);
		m_rtf.SetDraw(true);
		InvalidateRect(m_rtf.GetHwnd(), nullptr, TRUE);
	}

	// need to invalidate the window
	if (bFlag) {
		sel.cpMin = sel.cpMax = m_rtf.GetRichTextLength();
		m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
		m_rtf.SetDraw(true);
		InvalidateRect(m_rtf.GetHwnd(), nullptr, TRUE);
	}
}

void CLogWindow::ReplaceIcons(LONG startAt, int fAppend, BOOL isSent)
{
	wchar_t trbuffer[40];
	TEXTRANGE tr;
	tr.lpstrText = trbuffer;

	FINDTEXTEX fi;
	fi.chrg.cpMin = startAt;
	if (m_pDlg.m_bClrAdded) {
		fi.lpstrText = L"##col##";
		fi.chrg.cpMax = -1;
		CHARFORMAT2 cf2;
		memset(&cf2, 0, sizeof(cf2));
		cf2.cbSize = sizeof(cf2);
		cf2.dwMask = CFM_COLOR;
		while (m_rtf.SendMsg(EM_FINDTEXTEX, FR_DOWN, (LPARAM)&fi) > -1) {
			tr.chrg.cpMin = fi.chrgText.cpMin;
			tr.chrg.cpMax = tr.chrg.cpMin + 18;
			trbuffer[0] = 0;
			m_rtf.SendMsg(EM_GETTEXTRANGE, 0, (LPARAM)&tr);
			trbuffer[18] = 0;

			CHARRANGE cr;
			cr.cpMin = fi.chrgText.cpMin;
			cr.cpMax = cr.cpMin + 18;
			m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&cr);
			m_rtf.SendMsg(EM_REPLACESEL, FALSE, (LPARAM)L"");
			UINT length = (unsigned int)_wtol(&trbuffer[7]);
			int index = _wtol(&trbuffer[14]);
			if (length > 0 && length < 20000 && index >= RTF_CTABLE_DEFSIZE && index < Utils::rtf_clrs.getCount()) {
				cf2.crTextColor = Utils::rtf_clrs[index].clr;
				cr.cpMin = fi.chrgText.cpMin;
				cr.cpMax = cr.cpMin + length;
				m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&cr);
				m_rtf.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2);
			}
		}
	}

	fi.chrg.cpMin = startAt;
	if (m_pDlg.m_dwFlags & MWF_LOG_SHOWICONS) {
		fi.lpstrText = L"#~#";
		fi.chrg.cpMax = -1;

		CHARFORMAT2 cf2;
		memset(&cf2, 0, sizeof(cf2));
		cf2.cbSize = sizeof(cf2);
		cf2.dwMask = CFM_BACKCOLOR;

		CComPtr<IRichEditOle> ole;
		m_rtf.SendMsg(EM_GETOLEINTERFACE, 0, (LPARAM)&ole);
		if (ole != nullptr) {
			while (m_rtf.SendMsg(EM_FINDTEXTEX, FR_DOWN, (LPARAM)&fi) > -1) {
				CHARRANGE cr;
				cr.cpMin = fi.chrgText.cpMin;
				cr.cpMax = fi.chrgText.cpMax + 2;
				m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&cr);

				tr.chrg.cpMin = fi.chrgText.cpMin + 3;
				tr.chrg.cpMax = fi.chrgText.cpMin + 5;
				m_rtf.SendMsg(EM_GETTEXTRANGE, 0, (LPARAM)&tr);

				int bIconIndex = trbuffer[0] - '0';
				if (bIconIndex >= NR_LOGICONS) {
					fi.chrg.cpMin = fi.chrgText.cpMax + 6;
					continue;
				}

				char bDirection = trbuffer[1];
				m_rtf.SendMsg(EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2);
				COLORREF crDefault;
				if (cf2.crBackColor != 0)
					crDefault = cf2.crBackColor;
				else if (bDirection == '>')
					crDefault = (fAppend) ? m_pDlg.m_pContainer->m_theme.outbg : m_pDlg.m_pContainer->m_theme.oldoutbg;
				else
					crDefault = (fAppend) ? m_pDlg.m_pContainer->m_theme.inbg : m_pDlg.m_pContainer->m_theme.oldinbg;

				TLogIcon theIcon(Logicons[bIconIndex], crDefault);
				CImageDataObject::InsertBitmap(ole, theIcon.m_hBmp);
				fi.chrg.cpMin = cr.cpMax + 6;
			}
		}
	}

	// do smiley replacing, using the service
	if (PluginConfig.g_SmileyAddAvail) {
		CHARRANGE sel;
		sel.cpMin = startAt;
		sel.cpMax = -1;

		SMADD_RICHEDIT3 smadd = { sizeof(smadd) };
		smadd.hwndRichEditControl = m_rtf.GetHwnd();
		smadd.Protocolname = const_cast<char *>(m_pDlg.m_cache->getActiveProto());
		smadd.hContact = m_pDlg.m_cache->getActiveContact();
		smadd.flags = isSent ? SAFLRE_OUTGOING : 0;
		if (startAt > 0)
			smadd.rangeToReplace = &sel;
		else
			smadd.rangeToReplace = nullptr;
		smadd.disableRedraw = TRUE;
		CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM)&smadd);
	}

	if (m_pDlg.m_hHistoryEvents && m_pDlg.m_curHistory == m_pDlg.m_maxHistory) {
		wchar_t szPattern[50];
		mir_snwprintf(szPattern, L"~-+%d+-~", m_pDlg.m_hHistoryEvents[0]);

		FINDTEXTEX ft;
		ft.lpstrText = szPattern;
		ft.chrg.cpMin = 0;
		ft.chrg.cpMax = -1;
		if (m_rtf.SendMsg(EM_FINDTEXTEX, FR_DOWN, (LPARAM)&ft) != 0) {
			CHARRANGE sel;
			sel.cpMin = 0;
			sel.cpMax = 20;
			m_rtf.SendMsg(EM_SETSEL, 0, ft.chrgText.cpMax + 1);
			m_rtf.SendMsg(EM_REPLACESEL, TRUE, (LPARAM)L"");
		}
	}
}

void CLogWindow::ScrollToBottom()
{
	ScrollToBottom(false, false);
}

void CLogWindow::ScrollToBottom(bool bImmediate, bool bRedraw)
{
	if (bRedraw)
		SendMessage(m_rtf.GetHwnd(), WM_SIZE, 0, 0);

	if (bImmediate && bRedraw) {
		int len = GetWindowTextLength(m_rtf.GetHwnd());
		SendMessage(m_rtf.GetHwnd(), EM_SETSEL, len - 1, len - 1);
	}

	if (bImmediate)
		SendMessage(m_rtf.GetHwnd(), WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
	else
		PostMessage(m_rtf.GetHwnd(), WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);

	if (bRedraw)
		InvalidateRect(m_rtf.GetHwnd(), nullptr, FALSE);
}

void CLogWindow::UpdateOptions()
{
	COLORREF colour = m_pDlg.isChat() ? g_Settings.crLogBackground : db_get_dw(0, FONTMODULE, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR);
	m_rtf.SendMsg(EM_SETBKGNDCOLOR, 0, colour);

	if (!m_pDlg.isChat()) {
		PARAFORMAT2 pf2;
		pf2.wEffects = PFE_RTLPARA;
		pf2.dwMask |= PFM_OFFSET;
		if (m_pDlg.m_bInitMode) {
			pf2.dwMask |= (PFM_RIGHTINDENT | PFM_OFFSETINDENT);
			pf2.dxStartIndent = 30;
			pf2.dxRightIndent = 30;
		}
		pf2.dxOffset = m_pDlg.m_pContainer->m_theme.left_indent + 30;
		m_rtf.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);

		m_rtf.SendMsg(EM_SETLANGOPTIONS, 0, (LPARAM)m_rtf.SendMsg(EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);

		// set the scrollbars etc to RTL/LTR (only for manual RTL mode)
		if (m_pDlg.m_dwFlags & MWF_LOG_RTL)
			SetWindowLongPtr(m_rtf.GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(m_rtf.GetHwnd(), GWL_EXSTYLE) | WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR);
		else
			SetWindowLongPtr(m_rtf.GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(m_rtf.GetHwnd(), GWL_EXSTYLE) & ~(WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR));
	}
}

CSrmmLogWindow *logBuilder(CMsgDialog &pDlg)
{
	return new CLogWindow(pDlg);
}
