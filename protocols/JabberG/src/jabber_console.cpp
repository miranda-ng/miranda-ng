/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (c) 2007     Victor Pavlychko
Copyright (c) 2012-14  Miranda NG project

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

#include "jabber.h"

#define JCPF_IN    0x01UL
#define JCPF_OUT   0x02UL
#define JCPF_ERROR 0x04UL

#define JCPF_TCHAR 0x00UL

#define WM_CREATECONSOLE  WM_USER+1000

#ifndef SES_EXTENDBACKCOLOR
#define	SES_EXTENDBACKCOLOR 4
#endif

/* increment buffer with 1K steps */
#define STRINGBUF_INCREMENT		1024

struct StringBuf
{
	char *buf;
	int size;
	int offset;
	int streamOffset;
};

static void sttAppendBufRaw(StringBuf *buf, const char *str);
static void sttAppendBufW(StringBuf *buf, const WCHAR *str);
#define sttAppendBufT(a,b)		(sttAppendBufW((a),(b)))
static void sttEmptyBuf(StringBuf *buf);

#define RTF_HEADER	\
			"{\\rtf1\\ansi{\\colortbl;"	\
				"\\red128\\green0\\blue0;"	\
				"\\red0\\green0\\blue128;"	\
				"\\red245\\green255\\blue245;"	\
				"\\red245\\green245\\blue255;"	\
				"\\red128\\green128\\blue128;"	\
				"\\red255\\green235\\blue235;"	\
			"}"
#define RTF_FOOTER			"}"
#define RTF_BEGINTAG		"\\pard "
#define RTF_INDENT_FMT		"\\fi-100\\li%d "
#define RTF_ENDTAG			"\\par"
#define RTF_BEGINTAGNAME	"\\cf1\\b "
#define RTF_ENDTAGNAME		"\\cf0\\b0 "
#define RTF_BEGINATTRNAME	"\\cf2\\b "
#define RTF_ENDATTRNAME		"\\cf0\\b0 "
#define RTF_BEGINATTRVAL	"\\b0 "
#define RTF_ENDATTRVAL		""
#define RTF_BEGINTEXT		"\\pard "
#define RTF_TEXTINDENT_FMT	"\\fi0\\li%d "
#define RTF_ENDTEXT			"\\par"
#define RTF_BEGINPLAINXML	"\\pard\\fi0\\li100\\highlight6\\cf0 "
#define RTF_ENDPLAINXML		"\\par"
#define RTF_SEPARATOR		"\\sl-1\\slmult0\\highlight5\\cf5\\-\\par\\sl0"

static void sttRtfAppendXml(StringBuf *buf, HXML node, DWORD flags, int indent);

void CJabberProto::OnConsoleProcessXml(HXML node, DWORD flags)
{
	if (node && m_pDlgConsole) {
		if (xmlGetName(node)) {
			if (FilterXml(node, flags)) {
				StringBuf buf = {0};
				sttAppendBufRaw(&buf, RTF_HEADER);
				sttRtfAppendXml(&buf, node, flags, 1);
				sttAppendBufRaw(&buf, RTF_SEPARATOR);
				sttAppendBufRaw(&buf, RTF_FOOTER);
				SendMessage(m_pDlgConsole->GetHwnd(), WM_JABBER_REFRESH, 0, (LPARAM)&buf);
				sttEmptyBuf(&buf);
			}
		}
		else {
			for (int i=0; i < xmlGetChildCount(node); i++)
				OnConsoleProcessXml(xmlGetChild(node, i), flags);
		}
	}
}

bool CJabberProto::RecursiveCheckFilter(HXML node, DWORD flags)
{
	int i;

	for (i = 0; i < xmlGetAttrCount(node); i++)
		if (JabberStrIStr(xmlGetAttr(node, i), m_filterInfo.pattern))
			return true;

	for (i = 0; i < xmlGetChildCount(node); i++)
		if (RecursiveCheckFilter(xmlGetChild(node, i), flags))
			return true;

	return false;
}

bool CJabberProto::FilterXml(HXML node, DWORD flags)
{
	if (!m_filterInfo.msg && !lstrcmp(xmlGetName(node), _T("message"))) return false;
	if (!m_filterInfo.presence && !lstrcmp(xmlGetName(node), _T("presence"))) return false;
	if (!m_filterInfo.iq && !lstrcmp(xmlGetName(node), _T("iq"))) return false;
	if (m_filterInfo.type == TFilterInfo::T_OFF) return true;

	mir_cslock lck(m_filterInfo.csPatternLock);

	const TCHAR *attrValue;
	switch (m_filterInfo.type) {
	case TFilterInfo::T_JID:
		attrValue = xmlGetAttrValue(node, (flags & JCPF_OUT) ? _T("to") : _T("from"));
		if (attrValue)
			return JabberStrIStr(attrValue, m_filterInfo.pattern) != NULL;
		break;

	case TFilterInfo::T_XMLNS:
		if (xmlGetChildCount(node)) {
			attrValue = xmlGetAttrValue(xmlGetChild(node, 0), _T("xmlns"));
			if (attrValue)
				return JabberStrIStr(attrValue, m_filterInfo.pattern) != NULL;
		}
		break;

	case TFilterInfo::T_ANY:
		return RecursiveCheckFilter(node, flags);
	}

	return false;
}

static void sttAppendBufRaw(StringBuf *buf, const char *str)
{
	if (!str) return;

	int length = lstrlenA(str);
	if (buf->size - buf->offset < length + 1) {
		buf->size += (length + STRINGBUF_INCREMENT);
		buf->buf = (char *)mir_realloc(buf->buf, buf->size);
	}
	lstrcpyA(buf->buf + buf->offset, str);
	buf->offset += length;
}

static void sttAppendBufW(StringBuf *buf, const WCHAR *str)
{
	char tmp[32];

	if (!str) return;

	sttAppendBufRaw(buf, "{\\uc1 ");
	for (const WCHAR *p = str; *p; ++p) {
		if ((*p == '\\') || (*p == '{') || (*p == '}')) {
			tmp[0] = '\\';
			tmp[1] = (char)*p;
			tmp[2] = 0;
		}
		else if (*p < 128) {
			tmp[0] = (char)*p;
			tmp[1] = 0;
		}
		else mir_snprintf(tmp, sizeof(tmp), "\\u%d ?", (int)*p);

		sttAppendBufRaw(buf, tmp);
	}
	sttAppendBufRaw(buf, "}");
}

static void sttEmptyBuf(StringBuf *buf)
{
	if (buf->buf) mir_free(buf->buf);
	buf->buf = 0;
	buf->size = 0;
	buf->offset = 0;
}

static void sttRtfAppendXml(StringBuf *buf, HXML node, DWORD flags, int indent)
{
	char *indentLevel = (char *)mir_alloc(128);
	mir_snprintf(indentLevel, 128, RTF_INDENT_FMT, (int)(indent*200));

	sttAppendBufRaw(buf, RTF_BEGINTAG);
	sttAppendBufRaw(buf, indentLevel);
	if (flags&JCPF_IN)	sttAppendBufRaw(buf, "\\highlight3 ");
	if (flags&JCPF_OUT)	sttAppendBufRaw(buf, "\\highlight4 ");
	sttAppendBufRaw(buf, "<");
	sttAppendBufRaw(buf, RTF_BEGINTAGNAME);
	sttAppendBufW(buf, (TCHAR*)xmlGetName(node));
	sttAppendBufRaw(buf, RTF_ENDTAGNAME);

	for (int i = 0; i < xmlGetAttrCount(node); i++) {
		TCHAR *attr = (TCHAR*)xmlGetAttrName(node, i);
		sttAppendBufRaw(buf, " ");
		sttAppendBufRaw(buf, RTF_BEGINATTRNAME);
		sttAppendBufW(buf, attr);
		sttAppendBufRaw(buf, RTF_ENDATTRNAME);
		sttAppendBufRaw(buf, "=\"");
		sttAppendBufRaw(buf, RTF_BEGINATTRVAL);
		sttAppendBufT(buf, (TCHAR*)xmlGetAttr(node, i));
		sttAppendBufRaw(buf, "\"");
		sttAppendBufRaw(buf, RTF_ENDATTRVAL);
	}

	if (xmlGetChild(node) || xmlGetText(node)) {
		sttAppendBufRaw(buf, ">");
		if (xmlGetChild(node))
			sttAppendBufRaw(buf, RTF_ENDTAG);
	}

	if (xmlGetText(node)) {
		if (xmlGetChildCount(node)) {
			sttAppendBufRaw(buf, RTF_BEGINTEXT);
			char *indentTextLevel = (char *)mir_alloc(128);
			mir_snprintf(indentTextLevel, 128, RTF_TEXTINDENT_FMT, (int)((indent + 1) * 200));
			sttAppendBufRaw(buf, indentTextLevel);
			mir_free(indentTextLevel);
		}

		sttAppendBufT(buf, xmlGetText(node));
		if (xmlGetChild(node))
			sttAppendBufRaw(buf, RTF_ENDTEXT);
	}

	for (int i = 0; i < xmlGetChildCount(node); i++)
		sttRtfAppendXml(buf, xmlGetChild(node, i), flags & ~(JCPF_IN | JCPF_OUT), indent + 1);

	if (xmlGetChildCount(node) || xmlGetText(node)) {
		sttAppendBufRaw(buf, RTF_BEGINTAG);
		sttAppendBufRaw(buf, indentLevel);
		sttAppendBufRaw(buf, "</");
		sttAppendBufRaw(buf, RTF_BEGINTAGNAME);
		sttAppendBufT(buf, xmlGetName(node));
		sttAppendBufRaw(buf, RTF_ENDTAGNAME);
		sttAppendBufRaw(buf, ">");
	}
	else sttAppendBufRaw(buf, " />");

	sttAppendBufRaw(buf, RTF_ENDTAG);
	mir_free(indentLevel);
}

DWORD CALLBACK sttStreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	StringBuf *buf = (StringBuf *)dwCookie;
	*pcb = 0;

	if (buf->streamOffset < buf->offset) {
		*pcb = min(cb, buf->offset - buf->streamOffset);
		memcpy(pbBuff, buf->buf + buf->streamOffset, *pcb);
		buf->streamOffset += *pcb;
	}

	return 0;
}

static void sttJabberConsoleRebuildStrings(CJabberProto *ppro, HWND hwndCombo)
{
	int i;
	JABBER_LIST_ITEM *item = NULL;

	int len = GetWindowTextLength(hwndCombo) + 1;
	TCHAR *buf = (TCHAR *)_alloca(len * sizeof(TCHAR));
	GetWindowText(hwndCombo, buf, len);

	SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0);

	for (i=0; g_JabberFeatCapPairs[i].szFeature; i++)
		SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)g_JabberFeatCapPairs[i].szFeature);
	for (i=0; g_JabberFeatCapPairsExt[i].szFeature; i++)
		SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)g_JabberFeatCapPairsExt[i].szFeature);

	LISTFOREACH_NODEF(i, ppro, LIST_ROSTER)
		if (item = ppro->ListGetItemPtrFromIndex(i))
			SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)item->jid);
	LISTFOREACH_NODEF(i, ppro, LIST_CHATROOM)
		if (item = ppro->ListGetItemPtrFromIndex(i))
			SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)item->jid);

	SetWindowText(hwndCombo, buf);
}

///////////////////////////////////////////////////////////////////////////////
// CJabberDlgConsole class

struct
{
	int type;
	TCHAR *title;
	char *icon;
}
static filter_modes[] =
{
	{ TFilterInfo::T_JID,   _T("JID"),            "main" },
	{ TFilterInfo::T_XMLNS, _T("xmlns"),          "xmlconsole" },
	{ TFilterInfo::T_ANY,   _T("all attributes"), "sd_filter_apply" },
	{ TFilterInfo::T_OFF,   _T("disabled"),       "sd_filter_reset" },
};

class CJabberDlgConsole: public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

public:
	CJabberDlgConsole(CJabberProto *proto);

protected:
	void OnInitDialog();
	void OnClose();
	void OnDestroy();
	void OnProtoRefresh(WPARAM wParam, LPARAM lParam);
	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
	int Resizer(UTILRESIZECONTROL *urc);
};

CJabberDlgConsole::CJabberDlgConsole(CJabberProto *proto):
	CJabberDlgBase(proto, IDD_CONSOLE, NULL)
{
}

void CJabberDlgConsole::OnInitDialog()
{
	CSuper::OnInitDialog();

	WindowSetIcon(m_hwnd, m_proto, "xmlconsole");
	SendDlgItemMessage(m_hwnd, IDC_CONSOLE, EM_SETEDITSTYLE, SES_EXTENDBACKCOLOR, SES_EXTENDBACKCOLOR);
	SendDlgItemMessage(m_hwnd, IDC_CONSOLE, EM_EXLIMITTEXT, 0, 0x80000000);

	m_proto->m_filterInfo.msg = m_proto->getByte("consoleWnd_msg", TRUE);
	m_proto->m_filterInfo.presence = m_proto->getByte("consoleWnd_presence", TRUE);
	m_proto->m_filterInfo.iq = m_proto->getByte("consoleWnd_iq", TRUE);
	m_proto->m_filterInfo.type = (TFilterInfo::Type)m_proto->getByte("consoleWnd_ftype", TFilterInfo::T_OFF);

	*m_proto->m_filterInfo.pattern = 0;
	ptrT tszPattern( m_proto->getTStringA("consoleWnd_fpattern"));
	if (tszPattern != NULL)
		lstrcpyn(m_proto->m_filterInfo.pattern, tszPattern, SIZEOF(m_proto->m_filterInfo.pattern));

	sttJabberConsoleRebuildStrings(m_proto, GetDlgItem(m_hwnd, IDC_CB_FILTER));
	SetWindowText(GetDlgItem(m_hwnd, IDC_CB_FILTER), m_proto->m_filterInfo.pattern);

	struct
	{
		int idc;
		char *title;
		char *icon;
		bool push;
		BOOL pushed;
	}
	static buttons[] =
	{
		{ IDC_BTN_MSG,            "Messages",     "pl_msg_allow",    true,  m_proto->m_filterInfo.msg},
		{ IDC_BTN_PRESENCE,       "Presences",    "pl_prin_allow",   true,  m_proto->m_filterInfo.presence},
		{ IDC_BTN_IQ,             "Queries",      "pl_iq_allow",     true,  m_proto->m_filterInfo.iq},
		{ IDC_BTN_FILTER,         "Filter mode",  "sd_filter_apply", true,  FALSE},
		{ IDC_BTN_FILTER_REFRESH, "Refresh list", "sd_nav_refresh",  false, FALSE},
	};

	int i;
	for (i=0; i < SIZEOF(buttons); i++) {
		SendDlgItemMessage(m_hwnd, buttons[i].idc, BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_proto->LoadIconEx(buttons[i].icon));
		SendDlgItemMessage(m_hwnd, buttons[i].idc, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(m_hwnd, buttons[i].idc, BUTTONADDTOOLTIP, (WPARAM)buttons[i].title, 0);
		if (buttons[i].push) SendDlgItemMessage(m_hwnd, buttons[i].idc, BUTTONSETASPUSHBTN, TRUE, 0);
		if (buttons[i].pushed) CheckDlgButton(m_hwnd, buttons[i].idc, TRUE);
	}

	for (i=0; i < SIZEOF(filter_modes); i++)
		if (filter_modes[i].type == m_proto->m_filterInfo.type) {
			g_ReleaseIcon((HICON)SendDlgItemMessage(m_hwnd, IDC_BTN_FILTER, BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_proto->LoadIconEx(filter_modes[i].icon)));
			SendDlgItemMessage(m_hwnd, IDC_BTN_FILTER, BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_proto->LoadIconEx(filter_modes[i].icon));
			break;
		}

	EnableWindow(GetDlgItem(m_hwnd, IDC_CB_FILTER), (m_proto->m_filterInfo.type == TFilterInfo::T_OFF) ? FALSE : TRUE);
	EnableWindow(GetDlgItem(m_hwnd, IDC_BTN_FILTER_REFRESH), (m_proto->m_filterInfo.type == TFilterInfo::T_OFF) ? FALSE : TRUE);

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "consoleWnd_");
}

void CJabberDlgConsole::OnClose()
{
	m_proto->setByte("consoleWnd_msg", m_proto->m_filterInfo.msg);
	m_proto->setByte("consoleWnd_presence", m_proto->m_filterInfo.presence);
	m_proto->setByte("consoleWnd_iq", m_proto->m_filterInfo.iq);
	m_proto->setByte("consoleWnd_ftype", m_proto->m_filterInfo.type);
	m_proto->setTString("consoleWnd_fpattern", m_proto->m_filterInfo.pattern);

	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "consoleWnd_");
	DestroyWindow(m_hwnd);
	CSuper::OnClose();
}

void CJabberDlgConsole::OnDestroy()
{
	g_ReleaseIcon((HICON)SendDlgItemMessage(m_hwnd, IDC_BTN_MSG, BM_SETIMAGE, IMAGE_ICON, 0));
	g_ReleaseIcon((HICON)SendDlgItemMessage(m_hwnd, IDC_BTN_PRESENCE, BM_SETIMAGE, IMAGE_ICON, 0));
	g_ReleaseIcon((HICON)SendDlgItemMessage(m_hwnd, IDC_BTN_IQ, BM_SETIMAGE, IMAGE_ICON, 0));
	g_ReleaseIcon((HICON)SendDlgItemMessage(m_hwnd, IDC_BTN_FILTER, BM_SETIMAGE, IMAGE_ICON, 0));
	g_ReleaseIcon((HICON)SendDlgItemMessage(m_hwnd, IDC_BTN_FILTER_REFRESH, BM_SETIMAGE, IMAGE_ICON, 0));

	m_proto->m_pDlgConsole = NULL;
	CSuper::OnDestroy();
}

void CJabberDlgConsole::OnProtoRefresh(WPARAM, LPARAM lParam)
{
	SendDlgItemMessage(m_hwnd, IDC_CONSOLE, WM_SETREDRAW, FALSE, 0);

	StringBuf *buf = (StringBuf *)lParam;
	buf->streamOffset = 0;

	EDITSTREAM es = {0};
	es.dwCookie = (DWORD_PTR)buf;
	es.pfnCallback = sttStreamInCallback;

	SCROLLINFO si = {0};
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	GetScrollInfo(GetDlgItem(m_hwnd, IDC_CONSOLE), SB_VERT, &si);

	CHARRANGE oldSel, sel;
	POINT ptScroll;
	SendDlgItemMessage(m_hwnd, IDC_CONSOLE, EM_GETSCROLLPOS, 0, (LPARAM)&ptScroll);
	SendDlgItemMessage(m_hwnd, IDC_CONSOLE, EM_EXGETSEL, 0, (LPARAM)&oldSel);
	sel.cpMin = sel.cpMax = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_CONSOLE));
	SendDlgItemMessage(m_hwnd, IDC_CONSOLE, EM_EXSETSEL, 0, (LPARAM)&sel);
	SendDlgItemMessage(m_hwnd, IDC_CONSOLE, EM_STREAMIN, SF_RTF|SFF_SELECTION, (LPARAM)&es);
	SendDlgItemMessage(m_hwnd, IDC_CONSOLE, EM_EXSETSEL, 0, (LPARAM)&oldSel);

	// magic expression from tabsrmm :)
	if ((UINT)si.nPos >= (UINT)si.nMax - si.nPage - 5 || si.nMax - si.nMin - si.nPage < 50) {
		SendDlgItemMessage(m_hwnd, IDC_CONSOLE, WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
		sel.cpMin = sel.cpMax = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_CONSOLE));
		SendDlgItemMessage(m_hwnd, IDC_CONSOLE, EM_EXSETSEL, 0, (LPARAM)&sel);
	}
	else SendDlgItemMessage(m_hwnd, IDC_CONSOLE, EM_SETSCROLLPOS, 0, (LPARAM)&ptScroll);

	SendDlgItemMessage(m_hwnd, IDC_CONSOLE, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(GetDlgItem(m_hwnd, IDC_CONSOLE), NULL, FALSE);
}

int CJabberDlgConsole::Resizer(UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_CONSOLE:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
	case IDC_CONSOLEIN:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;

	case IDC_BTN_MSG:
	case IDC_BTN_PRESENCE:
	case IDC_BTN_IQ:
	case IDC_BTN_FILTER:
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;

	case IDC_RESET:
	case IDOK:
	case IDC_BTN_FILTER_REFRESH:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;

	case IDC_CB_FILTER:
		RECT rc;
		GetWindowRect(GetDlgItem(m_hwnd, urc->wId), &rc);
		urc->rcItem.right += (urc->dlgNewSize.cx - urc->dlgOriginalSize.cx);
		urc->rcItem.top += (urc->dlgNewSize.cy - urc->dlgOriginalSize.cy);
		urc->rcItem.bottom = urc->rcItem.top + rc.bottom - rc.top;
		return 0;
	}
	return CSuper::Resizer(urc);
}

INT_PTR CJabberDlgConsole::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
			lpmmi->ptMinTrackSize.x = 300;
			lpmmi->ptMinTrackSize.y = 400;
			return 0;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			if (!m_proto->m_bJabberOnline)
				MessageBox(m_hwnd, TranslateT("Can't send data while you are offline."), TranslateT("Jabber Error"), MB_ICONSTOP | MB_OK);
			else {
				int length = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_CONSOLEIN)) + 1;
				TCHAR *textToSend = (TCHAR *)mir_alloc(length * sizeof(TCHAR));
				GetWindowText(GetDlgItem(m_hwnd, IDC_CONSOLEIN), textToSend, length);

				int bytesProcessed = 0;
				XmlNode xmlTmp(textToSend, &bytesProcessed, NULL);
				if (xmlTmp)
					m_proto->m_ThreadInfo->send(xmlTmp);
				else {
					StringBuf buf = { 0 };
					sttAppendBufRaw(&buf, RTF_HEADER);
					sttAppendBufRaw(&buf, RTF_BEGINPLAINXML);
					sttAppendBufT(&buf, TranslateT("Outgoing XML parsing error"));
					sttAppendBufRaw(&buf, RTF_ENDPLAINXML);
					sttAppendBufRaw(&buf, RTF_SEPARATOR);
					sttAppendBufRaw(&buf, RTF_FOOTER);
					SendMessage(m_hwnd, WM_JABBER_REFRESH, 0, (LPARAM)&buf);
					sttEmptyBuf(&buf);
				}

				mir_free(textToSend);

				SendDlgItemMessage(m_hwnd, IDC_CONSOLEIN, WM_SETTEXT, 0, (LPARAM)_T(""));
			}
			return TRUE;

		case IDC_RESET:
			SetDlgItemText(m_hwnd, IDC_CONSOLE, _T(""));
			break;

		case IDC_BTN_MSG:
		case IDC_BTN_PRESENCE:
		case IDC_BTN_IQ:
			m_proto->m_filterInfo.msg = IsDlgButtonChecked(m_hwnd, IDC_BTN_MSG);
			m_proto->m_filterInfo.presence = IsDlgButtonChecked(m_hwnd, IDC_BTN_PRESENCE);
			m_proto->m_filterInfo.iq = IsDlgButtonChecked(m_hwnd, IDC_BTN_IQ);
			break;

		case IDC_BTN_FILTER_REFRESH:
			sttJabberConsoleRebuildStrings(m_proto, GetDlgItem(m_hwnd, IDC_CB_FILTER));
			break;

		case IDC_CB_FILTER:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				int idx = SendDlgItemMessage(m_hwnd, IDC_CB_FILTER, CB_GETCURSEL, 0, 0);
				int len = SendDlgItemMessage(m_hwnd, IDC_CB_FILTER, CB_GETLBTEXTLEN, idx, 0) + 1;

				mir_cslock lck(m_proto->m_filterInfo.csPatternLock);

				if (len > SIZEOF(m_proto->m_filterInfo.pattern)) {
					TCHAR *buf = (TCHAR *)_alloca(len * sizeof(TCHAR));
					SendDlgItemMessage(m_hwnd, IDC_CB_FILTER, CB_GETLBTEXT, idx, (LPARAM)buf);
					lstrcpyn(m_proto->m_filterInfo.pattern, buf, SIZEOF(m_proto->m_filterInfo.pattern));
				}
				else SendDlgItemMessage(m_hwnd, IDC_CB_FILTER, CB_GETLBTEXT, idx, (LPARAM)m_proto->m_filterInfo.pattern);
			}
			else if (HIWORD(wParam) == CBN_EDITCHANGE) {
				mir_cslock lck(m_proto->m_filterInfo.csPatternLock);
				GetWindowText(GetDlgItem(m_hwnd, IDC_CB_FILTER), m_proto->m_filterInfo.pattern, SIZEOF(m_proto->m_filterInfo.pattern));
			}
			break;

		case IDC_BTN_FILTER:
			int i;
			HMENU hMenu = CreatePopupMenu();
			for (i = 0; i < SIZEOF(filter_modes); i++)
				AppendMenu(hMenu,
					MF_STRING | ((filter_modes[i].type == m_proto->m_filterInfo.type) ? MF_CHECKED : 0),
					filter_modes[i].type + 1, TranslateTS(filter_modes[i].title));

			RECT rc; GetWindowRect(GetDlgItem(m_hwnd, IDC_BTN_FILTER), &rc);
			CheckDlgButton(m_hwnd, IDC_BTN_FILTER, TRUE);
			int res = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_BOTTOMALIGN, rc.left, rc.top, 0, m_hwnd, NULL);
			CheckDlgButton(m_hwnd, IDC_BTN_FILTER, FALSE);
			DestroyMenu(hMenu);

			if (res) {
				m_proto->m_filterInfo.type = (TFilterInfo::Type)(res - 1);
				for (i = 0; i < SIZEOF(filter_modes); i++) {
					if (filter_modes[i].type == m_proto->m_filterInfo.type) {
						g_ReleaseIcon((HICON)SendDlgItemMessage(m_hwnd, IDC_BTN_FILTER, BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_proto->LoadIconEx(filter_modes[i].icon)));
						break;
					}
				}
				EnableWindow(GetDlgItem(m_hwnd, IDC_CB_FILTER), (m_proto->m_filterInfo.type == TFilterInfo::T_OFF) ? FALSE : TRUE);
				EnableWindow(GetDlgItem(m_hwnd, IDC_BTN_FILTER_REFRESH), (m_proto->m_filterInfo.type == TFilterInfo::T_OFF) ? FALSE : TRUE);
			}
		}
		break;
	}

	return CSuper::DlgProc(msg, wParam, lParam);
}

void __cdecl CJabberProto::ConsoleThread(void*)
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (msg.message == WM_CREATECONSOLE) {
			m_pDlgConsole = new CJabberDlgConsole(this);
			m_pDlgConsole->Show();
			continue;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	m_dwConsoleThreadId = 0;
}

void CJabberProto::ConsoleInit()
{
	LoadLibraryA("riched20.dll");
	m_hThreadConsole = ForkThreadEx(&CJabberProto::ConsoleThread, 0, &m_dwConsoleThreadId);
}

void CJabberProto::ConsoleUninit()
{
	if (m_hThreadConsole) {
		PostThreadMessage(m_dwConsoleThreadId, WM_QUIT, 0, 0);
		if (WaitForSingleObject(m_hThreadConsole, 5000) == WAIT_TIMEOUT) {
			TerminateThread(m_hThreadConsole, 0);
		}
		CloseHandle(m_hThreadConsole);
		m_hThreadConsole = NULL;
	}

	m_filterInfo.iq = m_filterInfo.msg = m_filterInfo.presence = FALSE;
	m_filterInfo.type = TFilterInfo::T_OFF;
}

INT_PTR __cdecl CJabberProto::OnMenuHandleConsole(WPARAM, LPARAM)
{
	if (m_pDlgConsole)
		SetForegroundWindow(m_pDlgConsole->GetHwnd());
	else if (m_hThreadConsole)
		PostThreadMessage(m_dwConsoleThreadId, WM_CREATECONSOLE, 0, 0);
	return 0;
}
