/*

Copyright 2000-12 Miranda IM, 2012-22 Miranda NG team,
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

extern IconItem iconList[];

#define LOGICON_MSG_IN      0
#define LOGICON_MSG_OUT     1
#define LOGICON_MSG_NOTICE  2

static char *pLogIconBmpBits[3];

#define STREAMSTAGE_HEADER  0
#define STREAMSTAGE_EVENTS  1
#define STREAMSTAGE_TAIL    2
#define STREAMSTAGE_STOP    3

struct LogStreamData
{
	int stage;
	MEVENT hDbEvent, hDbEventLast;
	MCONTACT hContact;
	CMStringA buf;
	int eventsToInsert;
	bool isEmpty;
	CMsgDialog *dlgDat;
};

static int logPixelSY;
static char szSep2[40], szSep2_RTL[50];

static const wchar_t *bbcodes[] = { L"[b]", L"[i]", L"[u]", L"[s]", L"[/b]", L"[/i]", L"[/u]", L"[/s]" };
static const char *bbcodefmt[] = { "\\b ", "\\i ", "\\ul ", "\\strike ", "\\b0 ", "\\i0 ", "\\ul0 ", "\\strike0 " };

static void AppendPlainUnicode(CMStringA &buf, const wchar_t *str)
{
	for (; *str; str++) {
		if (*str < 128)
			buf.AppendChar((char)*str);
		else
			buf.AppendFormat("\\u%d ?", *str);
	}
}

static void AppendToBufferWithRTF(CMStringA &buf, const wchar_t *line)
{
	if (line == nullptr)
		return;

	buf.Append("{\\uc1 ");

	for (; *line; line++) {
		if (*line == '\r' && line[1] == '\n') {
			buf.Append("\\par ");
			line++;
		}
		else if (*line == '\n') {
			buf.Append("\\par ");
		}
		else if (*line == '\t') {
			buf.Append("\\tab ");
		}
		else if (*line == '\\' || *line == '{' || *line == '}') {
			buf.AppendChar('\\');
			buf.AppendChar(*line);
		}
		else if (*line == '[' && (g_plugin.bShowFormat)) {
			int i, found = 0;
			for (i = 0; i < _countof(bbcodes); ++i) {
				if (line[1] == bbcodes[i][1]) {
					size_t lenb = mir_wstrlen(bbcodes[i]);
					if (!wcsnicmp(line, bbcodes[i], lenb)) {
						buf.Append(bbcodefmt[i]);
						line += lenb - 1;
						found = 1;
						break;
					}
				}
			}
			if (!found) {
				if (!wcsnicmp(line, L"[url", 4)) {
					const wchar_t *tag = wcschr(line + 4, ']');
					if (tag) {
						const wchar_t *tagu = (line[4] == '=') ? line + 5 : tag + 1;
						const wchar_t *tage = wcsstr(tag, L"[/url]");
						if (!tage) tage = wcsstr(tag, L"[/URL]");
						if (tage) {
							*(wchar_t*)tag = 0;
							*(wchar_t*)tage = 0;
							buf.Append("{\\field{\\*\\fldinst HYPERLINK \"");
							AppendPlainUnicode(buf, tagu);
							buf.Append("\"}{\\fldrslt ");
							AppendPlainUnicode(buf, tag+1);
							buf.Append("}}");
							line = tage + 5;
							found = 1;
						}
					}
				}
				else if (!wcsnicmp(line, L"[color=", 7)) {
					const wchar_t* tag = wcschr(line + 7, ']');
					if (tag) {
						line = tag;
						found = 1;
					}
				}
				else if (!wcsnicmp(line, L"[/color]", 8)) {
					line += 7;
					found = 1;
				}
			}
			if (!found) {
				if (*line < 128)
					buf.AppendChar((char)*line);
				else
					buf.AppendFormat("\\u%d ?", *line);
			}
		}
		else if (*line < 128)
			buf.AppendChar((char)*line);
		else
			buf.AppendFormat("\\u%d ?", *line);
	}

	buf.AppendChar('}');
}

#define FONT_FORMAT "{\\f%u\\fnil\\fcharset%u %S;}"

static void CreateRTFHeader(CMStringA &buffer)
{
	HDC hdc = GetDC(nullptr);
	logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(nullptr, hdc);

	buffer.Append("{\\rtf1\\ansi\\deff0{\\fonttbl");

	LOGFONT lf;
	for (int i = 0; LoadMsgDlgFont(i, &lf, nullptr); i++)
		buffer.AppendFormat(FONT_FORMAT, i, lf.lfCharSet, lf.lfFaceName);

	buffer.Append("}{\\colortbl ");
	COLORREF colour;
	for (int i = 0; LoadMsgDlgFont(i, nullptr, &colour); i++)
		buffer.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));

	if (GetSysColorBrush(COLOR_HOTLIGHT) == nullptr)
		colour = RGB(0, 0, 255);
	else
		colour = GetSysColor(COLOR_HOTLIGHT);
	buffer.AppendFormat("\\red%u\\green%u\\blue%u;", GetRValue(colour), GetGValue(colour), GetBValue(colour));
	buffer.Append("}");
}

// mir_free() the return value
static void CreateRTFTail(CMStringA &buffer)
{
	buffer = "}";
}

//return value is static
static void SetToStyle(int style, CMStringA &dest)
{
	LOGFONT lf;
	LoadMsgDlgFont(style, &lf, nullptr);
	if (style != MSGFONTID_MYCOLON && style != MSGFONTID_YOURCOLON)
		dest.AppendChar(' ');
	dest.AppendFormat("\\f%u\\cf%u\\b%d\\i%d\\fs%u ", style, style, lf.lfWeight >= FW_BOLD ? 1 : 0, lf.lfItalic, 2 * abs(lf.lfHeight) * 74 / logPixelSY);
}

int DbEventIsForMsgWindow(DBEVENTINFO *dbei)
{
	DBEVENTTYPEDESCR *et = DbEvent_GetType(dbei->szModule, dbei->eventType);
	return et && (et->flags & DETF_MSGWINDOW);
}

int DbEventIsShown(DBEVENTINFO *dbei)
{
	return (dbei->eventType==EVENTTYPE_MESSAGE) ||  DbEventIsForMsgWindow(dbei);
}

//mir_free() the return value
static bool CreateRTFFromDbEvent(LogStreamData *dat)
{
	DB::EventInfo dbei;
	dbei.cbBlob = -1;
	if (db_event_get(dat->hDbEvent, &dbei))
		return false;

	if (!DbEventIsShown(&dbei))
		return false;
	
	if (!(dbei.flags & DBEF_SENT) && (dbei.eventType == EVENTTYPE_MESSAGE || DbEventIsForMsgWindow(&dbei))) {
		db_event_markRead(dat->hContact, dat->hDbEvent);
		g_clistApi.pfnRemoveEvent(dat->hContact, dat->hDbEvent);
	}
	else if (dbei.eventType == EVENTTYPE_JABBER_CHATSTATES || dbei.eventType == EVENTTYPE_JABBER_PRESENCE) {
		db_event_markRead(dat->hContact, dat->hDbEvent);
	}

	CMStringA &buf = dat->buf;
	bool bIsRtl = dat->dlgDat->m_bIsAutoRTL;
	if (!bIsRtl && !dat->isEmpty)
		buf.Append("\\par");

	if (dbei.flags & DBEF_RTL) {
		buf.Append("\\rtlpar");
		bIsRtl = true;
	}
	else buf.Append("\\ltrpar");

	dat->isEmpty = false;

	if (bIsRtl) {
		if (dbei.flags & DBEF_RTL)
			buf.Append("\\ltrch\\rtlch");
		else
			buf.Append("\\rtlch\\ltrch");
	}

	if (g_plugin.bShowIcons) {
		int i = ((dbei.eventType == EVENTTYPE_MESSAGE) ? ((dbei.flags & DBEF_SENT) ? LOGICON_MSG_OUT : LOGICON_MSG_IN): LOGICON_MSG_NOTICE);
		
		buf.Append("\\f0\\fs14");
		buf.Append(pLogIconBmpBits[i]);
	}

	int showColon = 0;
	if (g_plugin.bShowTime) {
		const wchar_t* szFormat;
		wchar_t str[64];

		if (g_plugin.bShowSecs)
			szFormat = g_plugin.bShowDate ? L"d s" : L"s";
		else
			szFormat = g_plugin.bShowDate ? L"d t" : L"t";

		TimeZone_PrintTimeStamp(nullptr, dbei.timestamp, szFormat, str, _countof(str), 0);

		SetToStyle((dbei.flags & DBEF_SENT) ? MSGFONTID_MYTIME : MSGFONTID_YOURTIME, buf);
		AppendToBufferWithRTF(buf, str);
		showColon = 1;
	}

	if (g_plugin.bShowNames && dbei.eventType != EVENTTYPE_JABBER_CHATSTATES && dbei.eventType != EVENTTYPE_JABBER_PRESENCE) {
		wchar_t *szName;

		if (dbei.flags & DBEF_SENT) {
			if (wchar_t *p = Contact::GetInfo(CNF_DISPLAY, 0, dbei.szModule))
				szName = NEWWSTR_ALLOCA(p);
			else
				szName = TranslateT("Me");
		}
		else szName = Clist_GetContactDisplayName(dat->hContact);

		SetToStyle((dbei.flags & DBEF_SENT) ? MSGFONTID_MYNAME : MSGFONTID_YOURNAME, buf);
		AppendToBufferWithRTF(buf, szName);
		showColon = 1;
	}

	if (showColon)
		SetToStyle((dbei.flags & DBEF_SENT) ? MSGFONTID_MYCOLON : MSGFONTID_YOURCOLON, buf);

	wchar_t *msg, *szName;
	switch (dbei.eventType) {
	case EVENTTYPE_JABBER_CHATSTATES:
	case EVENTTYPE_JABBER_PRESENCE:
		if (dbei.flags & DBEF_SENT) {
			if (wchar_t *p = Contact::GetInfo(CNF_DISPLAY, 0, dbei.szModule)) {
				szName = NEWWSTR_ALLOCA(p);
				mir_free(p);
			}
			else szName = L"";
		}
		else szName = Clist_GetContactDisplayName(dat->hContact);

		SetToStyle(MSGFONTID_NOTICE, buf);
		AppendToBufferWithRTF(buf, szName);
		AppendToBufferWithRTF(buf, L" ");

		msg = DbEvent_GetTextW(&dbei, CP_ACP);
		if (msg) {
			AppendToBufferWithRTF(buf, msg);
			mir_free(msg);
		}
		break;

	case EVENTTYPE_FILE:
		{
			char *filename = (char*)dbei.pBlob + sizeof(uint32_t);
			char *descr = filename + mir_strlen(filename) + 1;
			
			SetToStyle(MSGFONTID_NOTICE, buf);
			AppendToBufferWithRTF(buf, (dbei.flags & DBEF_SENT) ? TranslateT("File sent") : TranslateT("File received"));
			buf.Append(": ");
			AppendToBufferWithRTF(buf, ptrW(DbEvent_GetString(&dbei, filename)));

			if (*descr != 0) {
				ptrW ptszDescr(DbEvent_GetString(&dbei, descr));
				buf.Append(" (");
				AppendToBufferWithRTF(buf, ptszDescr);
				buf.Append(")");
			}
		}
		break;

	case EVENTTYPE_MESSAGE:
	default:
		msg = DbEvent_GetTextW(&dbei, CP_ACP);
		SetToStyle((dbei.eventType == EVENTTYPE_MESSAGE) ? ((dbei.flags & DBEF_SENT) ? MSGFONTID_MYMSG : MSGFONTID_YOURMSG) : MSGFONTID_NOTICE, buf);
		AppendToBufferWithRTF(buf, msg);
		mir_free(msg);
	}

	if (bIsRtl)
		buf.Append("\\par");

	return true;
}

static DWORD CALLBACK LogStreamInEvents(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	LogStreamData *dat = (LogStreamData*)dwCookie;

	if (dat->buf.IsEmpty()) {
		switch (dat->stage) {
		case STREAMSTAGE_HEADER:
			CreateRTFHeader(dat->buf);
			dat->stage = STREAMSTAGE_EVENTS;
			break;

		case STREAMSTAGE_EVENTS:
			if (dat->eventsToInsert) {
				bool bOk;
				do {
					bOk = CreateRTFFromDbEvent(dat);
					if (bOk)
						dat->hDbEventLast = dat->hDbEvent;
					dat->hDbEvent = db_event_next(dat->hContact, dat->hDbEvent);
					if (--dat->eventsToInsert == 0)
						break;
				} while (!bOk && dat->hDbEvent);

				if (bOk) {
					dat->isEmpty = false;
					break;
				}
			}
			dat->stage = STREAMSTAGE_TAIL;
			__fallthrough;

		case STREAMSTAGE_TAIL:
			CreateRTFTail(dat->buf);
			dat->stage = STREAMSTAGE_STOP;
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

#define RTFPICTHEADERMAXSIZE   78
void LoadMsgLogIcons(void)
{
	HBRUSH hBkgBrush = CreateSolidBrush(g_plugin.getDword(SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR));

	BITMAPINFOHEADER bih = { sizeof(bih) };
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;
	bih.biHeight = 10;
	bih.biPlanes = 1;
	bih.biWidth = 10;
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
		HICON hIcon = IcoLib_GetIconByHandle(iconList[i].hIcolib);
		size_t size = RTFPICTHEADERMAXSIZE + (bih.biSize + widthBytes * bih.biHeight) * 2;
		pLogIconBmpBits[i] = (char*)mir_alloc(size);
		size_t rtfHeaderSize = mir_snprintf(pLogIconBmpBits[i], size, "{\\pict\\dibitmap0\\wbmbitspixel%u\\wbmplanes1\\wbmwidthbytes%u\\picw%u\\pich%u ", bih.biBitCount, widthBytes, bih.biWidth, bih.biHeight);
		HBITMAP hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
		FillRect(hdcMem, &rc, hBkgBrush);
		DrawIconEx(hdcMem, 0, 0, hIcon, bih.biWidth, bih.biHeight, 0, nullptr, DI_NORMAL);
		IcoLib_ReleaseIcon(hIcon);

		SelectObject(hdcMem, hoBmp);
		GetDIBits(hdc, hBmp, 0, bih.biHeight, pBmpBits, (BITMAPINFO*)&bih, DIB_RGB_COLORS);

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
}

void FreeMsgLogIcons(void)
{
	for (auto &it : pLogIconBmpBits)
		mir_free(it);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CLogWindow::Attach()
{
	CSuper::Attach();

	// get around a lame bug in the Windows template resource code where richedits are limited to 0x7FFF
	m_rtf.SendMsg(EM_LIMITTEXT, sizeof(wchar_t) * 0x7FFFFFFF, 0);
	m_rtf.SendMsg(EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_LINK | ENM_SCROLL);
	m_rtf.SendMsg(EM_AUTOURLDETECT, TRUE, 0);
}

void CLogWindow::LogEvents(MEVENT hDbEventFirst, int count, bool bAppend)
{
	CHARRANGE oldSel, sel;
	BOOL bottomScroll = TRUE;
	POINT scrollPos;

	m_rtf.SetDraw(false);
	m_rtf.SendMsg(EM_EXGETSEL, 0, (LPARAM)&oldSel);

	LogStreamData streamData = {};
	streamData.hContact = m_pDlg.m_hContact;
	streamData.hDbEvent = hDbEventFirst;
	streamData.dlgDat = &m_pDlg;
	streamData.eventsToInsert = count;
	streamData.isEmpty = !bAppend || GetWindowTextLength(m_rtf.GetHwnd()) == 0;

	EDITSTREAM stream = {};
	stream.pfnCallback = LogStreamInEvents;
	stream.dwCookie = (DWORD_PTR)&streamData;

	if (!streamData.isEmpty) {
		bottomScroll = (GetFocus() != m_rtf.GetHwnd());
		if (bottomScroll && (GetWindowLongPtr(m_rtf.GetHwnd(), GWL_STYLE) & WS_VSCROLL)) {
			SCROLLINFO si = {};
			si.cbSize = sizeof(si);
			si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
			GetScrollInfo(m_rtf.GetHwnd(), SB_VERT, &si);
			bottomScroll = (si.nPos + (int)si.nPage) >= si.nMax;
		}
		if (!bottomScroll)
			m_rtf.SendMsg(EM_GETSCROLLPOS, 0, (LPARAM)&scrollPos);
	}

	FINDTEXTEXA fi;
	if (bAppend) {
		sel.cpMin = sel.cpMax = -1;
		m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
		fi.chrg.cpMin = 0;
	}
	else {
		GETTEXTLENGTHEX gtxl = { 0 };
		gtxl.flags = GTL_DEFAULT | GTL_PRECISE | GTL_NUMCHARS;
		gtxl.codepage = 1200;
		fi.chrg.cpMin = m_rtf.SendMsg(EM_GETTEXTLENGTHEX, (WPARAM)&gtxl, 0);

		sel.cpMin = sel.cpMax = m_rtf.GetRichTextLength();
		m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
	}

	mir_strcpy(szSep2, bAppend ? "\\par\\sl0" : "\\sl1000");
	mir_strcpy(szSep2_RTL, bAppend ? "\\rtlpar\\rtlmark\\par\\sl1000" : "\\sl1000");

	m_rtf.SendMsg(EM_STREAMIN, bAppend ? SFF_SELECTION | SF_RTF : SF_RTF, (LPARAM)&stream);
	if (bottomScroll) {
		sel.cpMin = sel.cpMax = -1;
		m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
	}
	else {
		m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&oldSel);
		m_rtf.SendMsg(EM_SETSCROLLPOS, 0, (LPARAM)&scrollPos);
	}

	if (g_plugin.bSmileyInstalled) {
		SMADD_RICHEDIT3 smre;
		smre.cbSize = sizeof(SMADD_RICHEDIT3);
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

	m_pDlg.m_hDbEventLast = streamData.hDbEventLast;
}

void CLogWindow::UpdateOptions()
{
	if (m_pDlg.isChat())
		m_rtf.SendMsg(EM_SETBKGNDCOLOR, 0, g_Settings.crLogBackground);
	else {
		// configure message history for proper RTL formatting
		PARAFORMAT2 pf2;
		memset(&pf2, 0, sizeof(pf2));
		pf2.cbSize = sizeof(pf2);

		pf2.wEffects = PFE_RTLPARA;
		pf2.dwMask = PFM_RTLPARA;
		m_rtf.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);

		pf2.wEffects = 0;
		m_rtf.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);

		m_rtf.SendMsg(EM_SETLANGOPTIONS, 0, m_rtf.SendMsg(EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);
		m_rtf.SendMsg(EM_SETBKGNDCOLOR, 0, g_plugin.getDword(SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static const CHARRANGE rangeAll = { 0, -1 };

INT_PTR CLogWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	CHARRANGE sel;

	switch (msg) {
	case WM_CONTEXTMENU:
		// we display context menu here only for private chats, group chats are processed by the core
		if (!m_pDlg.isChat()) {
			POINT pt;
			GetCursorPos(&pt);

			SetFocus(m_rtf.GetHwnd());

			HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_CONTEXT));
			HMENU hSubMenu = GetSubMenu(hMenu, 0);
			TranslateMenu(hSubMenu);

			CHARRANGE all = { 0, -1 };
			m_rtf.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin == sel.cpMax)
				EnableMenuItem(hSubMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);

			switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_pDlg.m_hwnd, nullptr)) {
			case IDM_COPY:
				m_rtf.SendMsg(WM_COPY, 0, 0);
				break;
			case IDM_COPYALL:
				m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&all);
				m_rtf.SendMsg(WM_COPY, 0, 0);
				m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
				break;
			case IDM_SELECTALL:
				m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&all);
				break;
			case IDM_CLEAR:
				Clear();
				m_pDlg.m_hDbEventFirst = 0;
				break;
			}
			DestroyMenu(hSubMenu);
			DestroyMenu(hMenu);
			return TRUE;
		}
		break;

	case WM_LBUTTONUP:
		if (m_pDlg.isChat()) {
			m_rtf.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin != sel.cpMax) {
				m_rtf.SendMsg(WM_COPY, 0, 0);
				sel.cpMin = sel.cpMax;
				m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
			}
			SetFocus(m_pDlg.m_message.GetHwnd());
		}
		break;

	case WM_KEYDOWN:
		bool isShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
		bool isCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		bool isAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;

		if (wParam == 0x57 && isCtrl && !isAlt) { // ctrl-w (close window)
			m_pDlg.CloseTab();
			return TRUE;
		}

		if (m_pDlg.ProcessHotkeys(wParam, isShift, isCtrl, isAlt))
			return FALSE;
	}

	return CSuper::WndProc(msg, wParam, lParam);
}

CSrmmLogWindow *logBuilder(CMsgDialog &pDlg)
{
	return new CLogWindow(pDlg);
}
