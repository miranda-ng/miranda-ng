/*
Scriver

Copyright (c) 2000-12 Miranda ICQ/IM project,

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

#define LOGICON_MSG_IN      0
#define LOGICON_MSG_OUT     1
#define LOGICON_MSG_NOTICE  2
#define LOGICON_MSG_SECURE  3
#define LOGICON_MSG_STRONG  4
#define LOGICON_MSG_MAX     5

static int logPixelSY;
static char* pLogIconBmpBits[LOGICON_MSG_MAX];
static HIMAGELIST g_hImageList;

struct RtfLogStreamData : public RtfLogStreamBase
{
	GlobalMessageData *gdat;
};

bool DbEventIsShown(const DB::EventInfo &dbei)
{
	switch (dbei.eventType) {
	case EVENTTYPE_JABBER_CHATSTATES:
	case EVENTTYPE_JABBER_PRESENCE:
		return 0;
	}

	return dbei.isSrmm();
}

static void AppendUnicodeToBuffer(CMStringA &buf, const wchar_t *line)
{
	buf.Append("{\\uc1 ");

	int wasEOL = 0;
	for (; *line; line++) {
		wasEOL = 0;
		if (*line == '\r' && line[1] == '\n') {
			buf.Append("\\line ");
			wasEOL = 1;
			line++;
		}
		else if (*line == '\n') {
			buf.Append("\\line ");
			wasEOL = 1;
		}
		else if (*line == '\t') {
			buf.Append("\\tab ");
		}
		else if (*line == '\\' || *line == '{' || *line == '}') {
			buf.AppendChar('\\');
			buf.AppendChar((char)*line);
		}
		else if (*line < 128) {
			buf.AppendChar((char)*line);
		}
		else {
			buf.AppendFormat("\\u%d ?", *line);
		}
	}
	if (wasEOL)
		buf.AppendChar(' ');

	buf.AppendChar('}');
}

// return value is static
static char* SetToStyle(int style)
{
	static char szStyle[128];
	LOGFONT lf;
	LoadMsgDlgFont(style, &lf, nullptr);
	mir_snprintf(szStyle, "\\f%u\\cf%u\\b%d\\i%d\\fs%u", style, style, lf.lfWeight >= FW_BOLD ? 1 : 0, lf.lfItalic, 2 * abs(lf.lfHeight) * 74 / logPixelSY);
	return szStyle;
}

// mode: 0 - date & time, 1 - date, 2 - time
static wchar_t* TimestampToString(WindowFlags flags, time_t check, int mode)
{
	static wchar_t szResult[512];
	wchar_t str[80];
	wchar_t format[20];

	szResult[0] = '\0';
	format[0] = '\0';
	if ((mode == 0 || mode == 1) && flags.bShowDate) {
		struct tm tm_now, tm_today;
		time_t now = time(0);
		time_t today;
		tm_now = *localtime(&now);
		tm_today = tm_now;
		tm_today.tm_hour = tm_today.tm_min = tm_today.tm_sec = 0;
		today = mktime(&tm_today);

		if (flags.bRelativeDate && check >= today) {
			mir_wstrcpy(szResult, TranslateT("Today"));
			if (mode == 0)
				mir_wstrcat(szResult, L",");
		}
		else if (flags.bRelativeDate && check > (today - 86400)) {
			mir_wstrcpy(szResult, TranslateT("Yesterday"));
			if (mode == 0)
				mir_wstrcat(szResult, L",");
		}
		else {
			if (flags.bLongDate)
				mir_wstrcpy(format, L"D");
			else
				mir_wstrcpy(format, L"d");
		}
	}
	if (mode == 0 || mode == 2) {
		if (mode == 0 && (flags.bShowDate))
			mir_wstrcat(format, L" ");

		mir_wstrcat(format, (flags.bShowSeconds) ? L"s" : L"t");
	}
	if (format[0] != '\0') {
		TimeZone_PrintTimeStamp(nullptr, check, format, str, _countof(str), 0);
		mir_wstrncat(szResult, str, _countof(szResult) - mir_wstrlen(szResult));
	}
	return szResult;
}

int isSameDate(time_t time1, time_t time2)
{
	struct tm tm_t1 = *localtime((time_t *)(&time1));
	struct tm tm_t2 = *localtime((time_t *)(&time2));
	if (tm_t1.tm_year == tm_t2.tm_year && tm_t1.tm_mon == tm_t2.tm_mon
		&& tm_t1.tm_mday == tm_t2.tm_mday) {
		return 1;
	}
	return 0;
}

static void AppendWithCustomLinks(const DB::EventInfo &dbei, int style, CMStringA &buf)
{
	if (dbei.pBlob == nullptr)
		return;

	wchar_t *wText;
	if ((dbei.flags & DBEF_UTF) == 0)
		wText = mir_a2u((char*)dbei.pBlob);
	else
		wText = mir_utf8decodeW((char *)dbei.pBlob);

	if (wText) {
		buf.AppendFormat("%s ", SetToStyle(style));
		AppendUnicodeToBuffer(buf, wText);
		mir_free(wText);
	}
}

#define RTFPICTHEADERMAXSIZE   78

void LoadMsgLogIcons(void)
{
	RECT rc;

	g_hImageList = ImageList_Create(10, 10, ILC_COLOR32 | ILC_MASK, _countof(pLogIconBmpBits), 0);
	HBRUSH hBkgBrush = CreateSolidBrush(g_plugin.getDword(SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR));
	HBRUSH hInBkgBrush = CreateSolidBrush(g_plugin.getDword(SRMSGSET_INCOMINGBKGCOLOUR, SRMSGDEFSET_INCOMINGBKGCOLOUR));
	HBRUSH hOutBkgBrush = CreateSolidBrush(g_plugin.getDword(SRMSGSET_OUTGOINGBKGCOLOUR, SRMSGDEFSET_OUTGOINGBKGCOLOUR));

	BITMAPINFOHEADER bih = { sizeof(bih) };
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;
	bih.biHeight = 10;
	bih.biPlanes = 1;
	bih.biWidth = 10;
	int widthBytes = ((bih.biWidth * bih.biBitCount + 31) >> 5) * 4;
	rc.top = rc.left = 0;
	rc.right = bih.biWidth;
	rc.bottom = bih.biHeight;
	HDC hdc = GetDC(nullptr);
	HBITMAP hBmp = CreateCompatibleBitmap(hdc, bih.biWidth, bih.biHeight);
	HDC hdcMem = CreateCompatibleDC(hdc);
	uint8_t *pBmpBits = (uint8_t*)mir_alloc(widthBytes * bih.biHeight);
	HBRUSH hBrush;
	for (int i = 0; i < _countof(pLogIconBmpBits); i++) {
		switch (i) {
		case LOGICON_MSG_IN:
			g_plugin.addImgListIcon(g_hImageList, IDI_INCOMING);
			hBrush = hInBkgBrush;
			break;
		case LOGICON_MSG_OUT:
			g_plugin.addImgListIcon(g_hImageList, IDI_OUTGOING);
			hBrush = hOutBkgBrush;
			break;
		case LOGICON_MSG_NOTICE:
			g_plugin.addImgListIcon(g_hImageList, IDI_NOTICE);
			hBrush = hBkgBrush;
			break;
		case LOGICON_MSG_SECURE:
			g_plugin.addImgListIcon(g_hImageList, IDI_SECURE);
			hBrush = hInBkgBrush;
			break;
		case LOGICON_MSG_STRONG:
			g_plugin.addImgListIcon(g_hImageList, IDI_STRONG);
			hBrush = hInBkgBrush;
			break;
		default:
			hBrush = nullptr; // warning fix, we never get here
			break;
		}

		pLogIconBmpBits[i] = (char*)mir_alloc(RTFPICTHEADERMAXSIZE + (bih.biSize + widthBytes * bih.biHeight) * 2);
		size_t rtfHeaderSize = sprintf(pLogIconBmpBits[i], "{\\pict\\dibitmap0\\wbmbitspixel%u\\wbmplanes1\\wbmwidthbytes%u\\picw%u\\pich%u ", bih.biBitCount, widthBytes, (UINT)bih.biWidth, (UINT)bih.biHeight); //!!!!!!!!!!!

		HICON hIcon = ImageList_GetIcon(g_hImageList, i, ILD_NORMAL);
		HBITMAP hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
		FillRect(hdcMem, &rc, hBrush);
		DrawIconEx(hdcMem, 0, 0, hIcon, bih.biWidth, bih.biHeight, 0, nullptr, DI_NORMAL);
		SelectObject(hdcMem, hoBmp);
		GetDIBits(hdc, hBmp, 0, bih.biHeight, pBmpBits, (BITMAPINFO *)& bih, DIB_RGB_COLORS);
		DestroyIcon(hIcon);

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
	DeleteObject(hInBkgBrush);
	DeleteObject(hOutBkgBrush);
}

void FreeMsgLogIcons(void)
{
	for (auto &it : pLogIconBmpBits)
		mir_free(it);

	ImageList_RemoveAll(g_hImageList);
	ImageList_Destroy(g_hImageList);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CLogWindow - built-in log window

const char *szBuiltinEvents[] = {
	"O Lord, bless this Thy hand grenade that with it Thou mayest blow Thine enemies",
	"to tiny bits, in Thy mercy",
	"Lorem ipsum dolor sit amet,",
	"consectetur adipisicing elit",
};

class CLogWindow : public CRtfLogWindow
{
	typedef CRtfLogWindow CSuper;

	int    m_isMixed = 0;
	int    m_lastEventType = -1;
	time_t m_startTime, m_lastEventTime;

public:
	CLogWindow(CMsgDialog &pDlg) :
		CSuper(pDlg)
	{
		m_lastEventTime = m_startTime = time(0);
	}

	void AppendUnicodeString(CMStringA &str, const wchar_t *pwszBuf) override
	{
		AppendUnicodeToBuffer(str, pwszBuf);
	}

	void Attach() override
	{
		CSuper::Attach();

		uint32_t dwExStyle = GetWindowLongPtr(m_rtf.GetHwnd(), GWL_EXSTYLE);
		SetWindowLongPtr(m_rtf.GetHwnd(), GWL_EXSTYLE, (m_pDlg.m_bUseRtl) ? dwExStyle | WS_EX_LEFTSCROLLBAR : dwExStyle & ~WS_EX_LEFTSCROLLBAR);

		// Workaround to make Richedit display RTL messages correctly
		PARAFORMAT2 pf2;
		memset(&pf2, 0, sizeof(pf2));
		pf2.cbSize = sizeof(pf2);
		pf2.dwMask = PFM_RTLPARA | PFM_OFFSETINDENT | PFM_RIGHTINDENT;
		pf2.wEffects = PFE_RTLPARA;
		pf2.dxStartIndent = 30;
		pf2.dxRightIndent = 30;
		m_rtf.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);

		pf2.dwMask = PFM_RTLPARA;
		pf2.wEffects = 0;
		m_rtf.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);

		m_rtf.SendMsg(EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_LINK | ENM_KEYEVENTS);
		m_rtf.SendMsg(EM_SETEDITSTYLE, SES_EXTENDBACKCOLOR, SES_EXTENDBACKCOLOR);
		m_rtf.SendMsg(EM_SETLANGOPTIONS, 0, (LPARAM)m_rtf.SendMsg(EM_GETLANGOPTIONS, 0, 0) & ~(IMF_AUTOKEYBOARD | IMF_AUTOFONTSIZEADJUST));
		m_rtf.SendMsg(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(0, 0));
		m_rtf.SendMsg(EM_LIMITTEXT, 0x7FFFFFFF, 0);
		m_rtf.SendMsg(EM_HIDESELECTION, TRUE, 0);
		m_rtf.SendMsg(EM_AUTOURLDETECT, TRUE, 0);
	}

	////////////////////////////////////////////////////////////////////////////////////////

	void CreateRtfHeader(RtfLogStreamData *streamData) override
	{
		HDC hdc = GetDC(nullptr);
		logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(nullptr, hdc);

		auto &buf = streamData->buf;

		buf.Append("{\\rtf1\\ansi\\deff0{\\fonttbl");
		for (int i = 0; i < fontOptionsListSize; i++) {
			LOGFONT lf;
			LoadMsgDlgFont(i, &lf, nullptr);
			buf.AppendFormat("{\\f%u\\fnil\\fcharset%u %S;}", i, lf.lfCharSet, lf.lfFaceName);
		}
		buf.Append("}{\\colortbl ");

		COLORREF colour;
		for (int i = 0; i < fontOptionsListSize; i++) {
			LoadMsgDlgFont(i, nullptr, &colour);
			buf.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
		}
		if (GetSysColorBrush(COLOR_HOTLIGHT) == nullptr)
			colour = RGB(0, 0, 255);
		else
			colour = GetSysColor(COLOR_HOTLIGHT);
		buf.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
		colour = g_plugin.getDword(SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR);
		buf.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
		colour = g_plugin.getDword(SRMSGSET_INCOMINGBKGCOLOUR, SRMSGDEFSET_INCOMINGBKGCOLOUR);
		buf.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
		colour = g_plugin.getDword(SRMSGSET_OUTGOINGBKGCOLOUR, SRMSGDEFSET_OUTGOINGBKGCOLOUR);
		buf.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
		colour = g_plugin.getDword(SRMSGSET_LINECOLOUR, SRMSGDEFSET_LINECOLOUR);
		buf.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
		buf.Append("}");
	}

	////////////////////////////////////////////////////////////////////////////////////////

	bool CreateRtfEvent(RtfLogStreamData *streamData, DB::EventInfo &dbei) override
	{
		if (!DbEventIsShown(dbei))
			return false;

		if (streamData->dbei) {
			if (streamData->eventsToInsert == _countof(szBuiltinEvents))
				return false;

			dbei.flags = DBEF_UTF | ((streamData->eventsToInsert < 2) ? DBEF_SENT : 0);
			dbei.pBlob = TranslateU(szBuiltinEvents[streamData->eventsToInsert]);
			dbei.cbBlob = (int)mir_strlen((char *)dbei.pBlob);
		}

		int style, showColon = 0;
		int isGroupBreak = TRUE;
		int highlight = 0;

		auto *gdat = streamData->gdat;
		auto &buf = streamData->buf;

		if ((gdat->flags.bGroupMessages) && dbei.flags == LOWORD(m_lastEventType) &&
			dbei.eventType == EVENTTYPE_MESSAGE && HIWORD(m_lastEventType) == EVENTTYPE_MESSAGE &&
			(isSameDate(dbei.getUnixtime(), m_lastEventTime)) && ((((int)dbei.getUnixtime() < m_startTime) == (m_lastEventTime < m_startTime)) || !dbei.bRead)) {
			isGroupBreak = FALSE;
		}

		bool bIsRtl = (dbei.flags & DBEF_RTL) != 0;
		ptrW wszText(dbei.getText()), wszNick;
		
		// test contact
		if (streamData->dbei == 0) {
			if (dbei.bSent)
				wszNick = Contact::GetInfo(CNF_DISPLAY, 0, m_pDlg.m_szProto);
			else
				wszNick = mir_wstrdup(Clist_GetContactDisplayName(m_pDlg.m_hContact));

			if (!m_pDlg.m_bUseRtl && Utils_IsRtl(wszText))
				bIsRtl = true;
		}
		else wszNick = mir_wstrdup(dbei.bSent ? TranslateT("Me") : TranslateT("My contact"));

		if (!streamData->isFirst && !m_isMixed) {
			if (isGroupBreak || gdat->flags.bMarkFollowups)
				buf.Append("\\par");
			else
				buf.Append("\\line");
		}

		if (bIsRtl)
			m_isMixed = 1;

		if (!streamData->isFirst && isGroupBreak && (gdat->flags.bDrawLines))
			buf.AppendFormat("\\sl-1\\slmult0\\highlight%d\\cf%d\\fs1  \\par\\sl0", fontOptionsListSize + 4, fontOptionsListSize + 4);

		buf.Append(bIsRtl ? "\\rtlpar" : "\\ltrpar");

		if (dbei.eventType == EVENTTYPE_MESSAGE)
			highlight = fontOptionsListSize + 2 + (dbei.bSent ? 1 : 0);
		else
			highlight = fontOptionsListSize + 1;

		buf.AppendFormat("\\highlight%d\\cf%d", highlight, highlight);
		if (!streamData->isFirst && m_isMixed) {
			if (isGroupBreak)
				buf.Append("\\sl-1 \\par\\sl0");
			else
				buf.Append("\\sl-1 \\line\\sl0");
		}
		streamData->isFirst = FALSE;
		if (m_isMixed) {
			if (bIsRtl)
				buf.Append("\\ltrch\\rtlch");
			else
				buf.Append("\\rtlch\\ltrch");
		}
		if ((gdat->flags.bShowIcons) && isGroupBreak) {
			int i = LOGICON_MSG_NOTICE;

			switch (dbei.eventType) {
			case EVENTTYPE_MESSAGE:
				if (dbei.flags & (DBEF_SECURE | DBEF_STRONG))
					i = (dbei.flags & DBEF_SECURE) ? LOGICON_MSG_SECURE : LOGICON_MSG_STRONG;
				else if (dbei.bSent)
					i = LOGICON_MSG_OUT;
				else
					i = LOGICON_MSG_IN;
				break;

			default:
				i = LOGICON_MSG_NOTICE;
				break;
			}

			buf.Append("\\fs1  ");
			buf.Append(pLogIconBmpBits[i]);
			buf.AppendChar(' ');
		}

		if (gdat->flags.bShowTime && (dbei.eventType != EVENTTYPE_MESSAGE ||
			(gdat->flags.bMarkFollowups || isGroupBreak || !(gdat->flags.bGroupMessages)))) {
			wchar_t *timestampString = nullptr;
			if (gdat->flags.bGroupMessages && dbei.eventType == EVENTTYPE_MESSAGE) {
				if (isGroupBreak) {
					if (!gdat->flags.bMarkFollowups)
						timestampString = TimestampToString(gdat->flags, dbei.getUnixtime(), 0);
					else if (gdat->flags.bShowDate)
						timestampString = TimestampToString(gdat->flags, dbei.getUnixtime(), 1);
				}
				else if (gdat->flags.bMarkFollowups)
					timestampString = TimestampToString(gdat->flags, dbei.getUnixtime(), 2);
			}
			else timestampString = TimestampToString(gdat->flags, dbei.getUnixtime(), 0);

			if (timestampString != nullptr) {
				buf.AppendFormat("%s ", SetToStyle(dbei.bSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME));
				AppendUnicodeToBuffer(buf, timestampString);
			}
			if (dbei.eventType != EVENTTYPE_MESSAGE)
				buf.AppendFormat("%s: ", SetToStyle(dbei.bSent ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON));
			showColon = 1;
		}
		if ((!(gdat->flags.bHideNames) && dbei.eventType == EVENTTYPE_MESSAGE && isGroupBreak) || dbei.eventType == EVENTTYPE_JABBER_CHATSTATES || dbei.eventType == EVENTTYPE_JABBER_PRESENCE) {
			if (dbei.eventType == EVENTTYPE_MESSAGE) {
				if (showColon)
					buf.AppendFormat(" %s ", SetToStyle(dbei.bSent ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME));
				else
					buf.AppendFormat("%s ", SetToStyle(dbei.bSent ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME));
			}
			else buf.AppendFormat("%s ", SetToStyle(MSGFONTID_NOTICE));

			AppendUnicodeToBuffer(buf, wszNick);

			showColon = 1;
			if (dbei.eventType == EVENTTYPE_MESSAGE && gdat->flags.bGroupMessages) {
				if (gdat->flags.bMarkFollowups)
					buf.Append("\\par");
				else
					buf.Append("\\line");
				showColon = 0;
			}
		}

		if (gdat->flags.bShowTime && gdat->flags.bGroupMessages && gdat->flags.bMarkFollowups && dbei.eventType == EVENTTYPE_MESSAGE && isGroupBreak) {
			buf.AppendFormat(" %s ", SetToStyle(dbei.bSent ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME));
			AppendUnicodeToBuffer(buf, TimestampToString(gdat->flags, dbei.getUnixtime(), 2));
			showColon = 1;
		}
		if (showColon && dbei.eventType == EVENTTYPE_MESSAGE) {
			if (bIsRtl)
				buf.AppendFormat("\\~%s: ", SetToStyle(dbei.bSent ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON));
			else
				buf.AppendFormat("%s: ", SetToStyle(dbei.bSent ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON));
		}
		switch (dbei.eventType) {
		case EVENTTYPE_JABBER_CHATSTATES:
		case EVENTTYPE_JABBER_PRESENCE:
		case EVENTTYPE_FILE:
			style = MSGFONTID_NOTICE;
			buf.AppendFormat("%s ", SetToStyle(style));
			if (dbei.eventType == EVENTTYPE_FILE) {
				DB::FILE_BLOB blob(dbei);
				if (blob.isOffline()) {
					InsertFileLink(buf, streamData->hDbEvent, blob);
					break;
				}

				if (dbei.bSent)
					AppendUnicodeToBuffer(buf, TranslateT("File sent"));
				else
					AppendUnicodeToBuffer(buf, TranslateT("File received"));
				AppendUnicodeToBuffer(buf, L":");
			}
			AppendUnicodeToBuffer(buf, L" ");

			if (wszText != nullptr)
				AppendUnicodeToBuffer(buf, wszText);
			break;

		default:
			if (gdat->flags.bMsgOnNewline && showColon)
				buf.Append("\\line");

			style = dbei.bSent ? MSGFONTID_MYMSG : MSGFONTID_YOURMSG;
			AppendWithCustomLinks(dbei, style, buf);
			break;
		}

		if (m_isMixed)
			buf.Append("\\par");

		m_lastEventTime = dbei.getUnixtime();
		m_lastEventType = MAKELONG(dbei.flags, dbei.eventType);
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////

	void LogEvents(MEVENT hDbEventFirst, int count, bool bAppend) override
	{
		if (!bAppend)
			m_lastEventType = -1;

		CHARRANGE oldSel, sel;
		m_rtf.SetDraw(false);
		m_rtf.SendMsg(EM_EXGETSEL, 0, (LPARAM)&oldSel);

		RtfLogStreamData streamData = {};
		streamData.hContact = m_pDlg.m_hContact;
		streamData.hDbEvent = hDbEventFirst;
		streamData.pLog = this;
		streamData.eventsToInsert = count;
		streamData.isFirst = bAppend ? m_rtf.GetRichTextLength() == 0 : 1;
		streamData.gdat = &g_dat;

		sel.cpMin = 0;

		POINT scrollPos;
		bool bottomScroll = (GetFocus() != m_rtf.GetHwnd());
		if (bottomScroll && (GetWindowLongPtr(m_rtf.GetHwnd(), GWL_STYLE) & WS_VSCROLL)) {
			SCROLLINFO si = {};
			si.cbSize = sizeof(si);
			si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
			GetScrollInfo(m_rtf.GetHwnd(), SB_VERT, &si);
			bottomScroll = (si.nPos + (int)si.nPage) >= si.nMax;
		}
		if (!bottomScroll)
			m_rtf.SendMsg(EM_GETSCROLLPOS, 0, (LPARAM)&scrollPos);

		FINDTEXTEXA fi;
		if (bAppend) {
			GETTEXTLENGTHEX gtxl = { 0 };
			gtxl.flags = GTL_DEFAULT | GTL_PRECISE | GTL_NUMCHARS;
			gtxl.codepage = 1200;
			fi.chrg.cpMin = m_rtf.SendMsg(EM_GETTEXTLENGTHEX, (WPARAM)&gtxl, 0);
			sel.cpMin = sel.cpMax = m_rtf.GetRichTextLength();
			m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
		}
		else {
			m_rtf.SetDraw(false);
			m_rtf.SetTextA("");
			sel.cpMin = 0;
			sel.cpMax = m_rtf.GetRichTextLength();
			m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
			fi.chrg.cpMin = 0;
			m_isMixed = 0;
		}

		StreamRtfEvents(&streamData, bAppend);

		if (bottomScroll) {
			sel.cpMin = sel.cpMax = -1;
			m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
		}
		else {
			m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&oldSel);
			m_rtf.SendMsg(EM_SETSCROLLPOS, 0, (LPARAM)&scrollPos);
		}

		if (g_dat.smileyAddInstalled) {
			SMADD_RICHEDIT smre;
			smre.hwndRichEditControl = m_rtf.GetHwnd();

			MCONTACT hContact = db_mc_getSrmmSub(m_pDlg.m_hContact);
			smre.Protocolname = (hContact != 0) ? Proto_GetBaseAccountName(hContact) : m_pDlg.m_szProto;

			if (fi.chrg.cpMin > 0) {
				sel.cpMin = fi.chrg.cpMin;
				sel.cpMax = -1;
				smre.rangeToReplace = &sel;
			}
			else smre.rangeToReplace = nullptr;

			smre.disableRedraw = TRUE;
			smre.hContact = m_pDlg.m_hContact;
			smre.flags = 0;
			CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM)&smre);
		}

		m_rtf.SetDraw(true);
		if (bottomScroll) {
			ScrollToBottom();
			RedrawWindow(m_rtf.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////

	void LogChatEvents(const LOGINFO *lin) override
	{
		if (m_rtf.GetHwnd() == nullptr)
			return;

		auto *si = m_pDlg.getChat();
		bool bRedraw = lin == nullptr;

		RtfChatLogStreamData streamData;
		streamData.pLog = this;
		streamData.si = si;
		streamData.lin = lin;
		streamData.bStripFormat = FALSE;
		streamData.bIsFirst = bRedraw ? 1 : m_rtf.GetRichTextLength() == 0;

		SCROLLINFO scroll;
		scroll.cbSize = sizeof(SCROLLINFO);
		scroll.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
		GetScrollInfo(m_rtf.GetHwnd(), SB_VERT, &scroll);

		POINT point = {};
		m_rtf.SendMsg(EM_GETSCROLLPOS, 0, (LPARAM)&point);

		// do not scroll to bottom if there is a selection
		CHARRANGE oldsel, sel, newsel;
		m_rtf.SendMsg(EM_EXGETSEL, 0, (LPARAM)&oldsel);
		if (oldsel.cpMax != oldsel.cpMin)
			m_rtf.SetDraw(false);

		// set the insertion point at the bottom
		sel.cpMin = sel.cpMax = m_rtf.GetRichTextLength();
		m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
		m_rtf.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);

		// fix for the indent... must be a M$ bug
		if (sel.cpMax == 0)
			bRedraw = TRUE;

		// get the number of pixels per logical inch
		bool bFlag = false;
		if (bRedraw) {
			m_rtf.SetDraw(false);
			bFlag = true;
		}

		// stream in the event(s)
		StreamChatRtfEvents(&streamData, bRedraw);

		// do smileys
		if (g_dat.smileyAddInstalled && (bRedraw || (lin && lin->ptszText && lin->iType != GC_EVENT_JOIN && lin->iType != GC_EVENT_NICK && lin->iType != GC_EVENT_ADDSTATUS && lin->iType != GC_EVENT_REMOVESTATUS))) {
			newsel.cpMax = -1;
			newsel.cpMin = sel.cpMin;
			if (newsel.cpMin < 0)
				newsel.cpMin = 0;

			SMADD_RICHEDIT sm = {};
			sm.hwndRichEditControl = m_rtf.GetHwnd();
			sm.Protocolname = si->pszModule;
			sm.rangeToReplace = bRedraw ? nullptr : &newsel;
			sm.flags = 0;
			sm.disableRedraw = TRUE;
			sm.hContact = m_pDlg.m_hContact;
			CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM)&sm);
		}

		// scroll log to bottom if the log was previously scrolled to bottom, else restore old position
		if (bRedraw || (UINT)scroll.nPos >= (UINT)scroll.nMax - scroll.nPage - 5 || scroll.nMax - scroll.nMin - scroll.nPage < 50)
			ScrollToBottom();
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

	void ScrollToBottom() override
	{
		if (GetWindowLongPtr(m_rtf.GetHwnd(), GWL_STYLE) & WS_VSCROLL) {
			SCROLLINFO si = { sizeof(si) };
			si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
			if (GetScrollInfo(m_rtf.GetHwnd(), SB_VERT, &si)) {
				if (m_rtf.GetHwnd() != GetFocus()) {
					si.fMask = SIF_POS;
					si.nPos = si.nMax - si.nPage + 1;
					SetScrollInfo(m_rtf.GetHwnd(), SB_VERT, &si, TRUE);

					PostMessage(m_rtf.GetHwnd(), WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
				}
			}
		}
	}

	void UpdateOptions() override
	{
		if (m_pDlg.isChat())
			m_rtf.SendMsg(EM_SETBKGNDCOLOR, 0, g_Settings.crLogBackground);
		else
			m_rtf.SendMsg(EM_SETBKGNDCOLOR, 0, g_plugin.getDword(SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR));

		PARAFORMAT2 pf2;
		memset(&pf2, 0, sizeof(pf2));
		pf2.cbSize = sizeof(pf2);
		pf2.dwMask = PFM_OFFSET;
		pf2.dxOffset = (g_dat.flags.bIndentText) ? g_dat.indentSize * 1440 / g_dat.logPixelSX : 0;
		m_rtf.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
		m_rtf.SendMsg(EM_SETLANGOPTIONS, 0, (LPARAM)m_rtf.SendMsg(EM_GETLANGOPTIONS, 0, 0) & ~(IMF_AUTOKEYBOARD | IMF_AUTOFONTSIZEADJUST));

		Clear();
	}

	INT_PTR WndProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		int result = m_pDlg.InputAreaShortcuts(m_rtf.GetHwnd(), msg, wParam, lParam);
		if (result != -1)
			return result;

		switch (msg) {
		case WM_MEASUREITEM:
			Menu_MeasureItem(lParam);
			return TRUE;

		case WM_DRAWITEM:
			return Menu_DrawItem(lParam);

		case WM_LBUTTONUP:
			if (g_plugin.bAutoCopy) {
				CHARRANGE sel;
				SendMessage(m_rtf.GetHwnd(), EM_EXGETSEL, 0, (LPARAM)&sel);
				if (sel.cpMin != sel.cpMax) {
					SendMessage(m_rtf.GetHwnd(), WM_COPY, 0, 0);
					sel.cpMin = sel.cpMax;
					SendMessage(m_rtf.GetHwnd(), EM_EXSETSEL, 0, (LPARAM)&sel);
				}
				SetFocus(m_pDlg.m_message.GetHwnd());
			}
			break;
		}

		return CSuper::WndProc(msg, wParam, lParam);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

void StreamInTestEvents(CDlgBase *pDlg, GlobalMessageData *gdat)
{
	DB::EventInfo dbei;
	dbei.flags = DBEF_UTF;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.iTimestamp = time(0);
	dbei.szModule = SRMM_MODULE;

	auto *pLog = new CLogWindow(*(CMsgDialog*)pDlg);

	RtfLogStreamData streamData = {};
	streamData.isFirst = TRUE;
	streamData.dbei = &dbei;
	streamData.pLog = pLog;
	streamData.gdat = gdat;
	pLog->StreamRtfEvents(&streamData, false);

	SendDlgItemMessage(pDlg->GetHwnd(), IDC_SRMM_LOG, EM_HIDESELECTION, FALSE, 0);

	dbei.pBlob = nullptr;
	delete pLog;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

CSrmmLogWindow* logBuilder(CMsgDialog &pDlg)
{
	return new CLogWindow(pDlg);
}
