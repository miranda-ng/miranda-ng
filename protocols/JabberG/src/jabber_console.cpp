/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (c) 2007     Victor Pavlychko
Copyright (C) 2012-22 Miranda NG team

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

#ifndef SES_EXTENDBACKCOLOR
#define	SES_EXTENDBACKCOLOR 4
#endif

/* increment buffer with 1K steps */
#define STRINGBUF_INCREMENT		1024

struct StringBuf
{
	char *buf;
	size_t size, offset, streamOffset;
};

static void sttAppendBufRaw(StringBuf *buf, const char *str);
static void sttAppendBufW(StringBuf *buf, const wchar_t *str);
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

static void sttRtfAppendXml(StringBuf *buf, const TiXmlElement *node, uint32_t flags, int indent);

void CJabberProto::OnConsoleProcessXml(const TiXmlElement *node, uint32_t flags)
{
	if (node && m_pDlgConsole) {
		if (node->Name()) {
			if (FilterXml(node, flags)) {
				StringBuf buf = {};
				sttAppendBufRaw(&buf, RTF_HEADER);
				sttRtfAppendXml(&buf, node, flags, 1);
				sttAppendBufRaw(&buf, RTF_SEPARATOR);
				sttAppendBufRaw(&buf, RTF_FOOTER);
				m_pDlgConsole->OnProtoRefresh(0, (LPARAM)&buf);
				sttEmptyBuf(&buf);
			}
		}
		else {
			for (auto *it : TiXmlEnum(node))
				OnConsoleProcessXml(it, flags);
		}
	}
}

bool CJabberProto::RecursiveCheckFilter(const TiXmlElement *node, uint32_t flags)
{
	for (auto *p = node->FirstAttribute(); p; p = p->Next())
		if (JabberStrIStr(Utf2T(p->Value()), m_filterInfo.pattern))
			return true;

	for (auto *it : TiXmlEnum(node))
		if (RecursiveCheckFilter(it, flags))
			return true;

	return false;
}

bool CJabberProto::FilterXml(const TiXmlElement *node, uint32_t flags)
{
	if (!m_filterInfo.msg && !mir_strcmp(node->Name(), "message")) return false;
	if (!m_filterInfo.presence && !mir_strcmp(node->Name(), "presence")) return false;
	if (!m_filterInfo.iq && !mir_strcmp(node->Name(), "iq")) return false;
	if (m_filterInfo.type == TFilterInfo::T_OFF) return true;

	mir_cslock lck(m_filterInfo.csPatternLock);

	const char *attrValue;
	switch (m_filterInfo.type) {
	case TFilterInfo::T_JID:
		attrValue = XmlGetAttr(node, (flags & JCPF_OUT) ? "to" : "from");
		if (attrValue)
			return JabberStrIStr(Utf2T(attrValue), m_filterInfo.pattern) != nullptr;
		break;

	case TFilterInfo::T_XMLNS:
		attrValue = XmlGetAttr(XmlFirstChild(node), "xmlns");
		if (attrValue)
			return JabberStrIStr(Utf2T(attrValue), m_filterInfo.pattern) != nullptr;
		break;

	case TFilterInfo::T_ANY:
		return RecursiveCheckFilter(node, flags);
	}

	return false;
}

static void sttAppendBufRaw(StringBuf *buf, const char *str)
{
	if (!str) return;

	size_t length = mir_strlen(str);
	if (buf->size - buf->offset < length + 1) {
		buf->size += (length + STRINGBUF_INCREMENT);
		buf->buf = (char *)mir_realloc(buf->buf, buf->size);
	}
	mir_strcpy(buf->buf + buf->offset, str);
	buf->offset += length;
}

static void sttAppendBufW(StringBuf *buf, const wchar_t *str)
{
	char tmp[32];

	if (!str) return;

	sttAppendBufRaw(buf, "{\\uc1 ");
	for (const wchar_t *p = str; *p; ++p) {
		if ((*p == '\\') || (*p == '{') || (*p == '}')) {
			tmp[0] = '\\';
			tmp[1] = (char)*p;
			tmp[2] = 0;
		}
		else if (*p < 128) {
			tmp[0] = (char)*p;
			tmp[1] = 0;
		}
		else mir_snprintf(tmp, "\\u%d ?", (int)*p);

		sttAppendBufRaw(buf, tmp);
	}
	sttAppendBufRaw(buf, "}");
}

static void sttEmptyBuf(StringBuf *buf)
{
	if (buf->buf) mir_free(buf->buf);
	buf->buf = nullptr;
	buf->size = 0;
	buf->offset = 0;
}

static void sttRtfAppendXml(StringBuf *buf, const TiXmlElement *node, uint32_t flags, int indent)
{
	char indentLevel[128];
	mir_snprintf(indentLevel, RTF_INDENT_FMT, (int)(indent * 200));

	sttAppendBufRaw(buf, RTF_BEGINTAG);
	sttAppendBufRaw(buf, indentLevel);
	if (flags & JCPF_IN)
		sttAppendBufRaw(buf, "\\highlight3 ");
	if (flags & JCPF_OUT)
		sttAppendBufRaw(buf, "\\highlight4 ");
	sttAppendBufRaw(buf, "<");
	sttAppendBufRaw(buf, RTF_BEGINTAGNAME);
	sttAppendBufRaw(buf, node->Name());
	sttAppendBufRaw(buf, RTF_ENDTAGNAME);

	for (auto *p = node->FirstAttribute(); p; p = p->Next()) {
		sttAppendBufRaw(buf, " ");
		sttAppendBufRaw(buf, RTF_BEGINATTRNAME);
		sttAppendBufW(buf, Utf2T(p->Name()));
		sttAppendBufRaw(buf, RTF_ENDATTRNAME);
		sttAppendBufRaw(buf, "=\"");
		sttAppendBufRaw(buf, RTF_BEGINATTRVAL);
		sttAppendBufT(buf, Utf2T(p->Value()));
		sttAppendBufRaw(buf, "\"");
		sttAppendBufRaw(buf, RTF_ENDATTRVAL);
	}

	if (!node->NoChildren() || node->GetText()) {
		sttAppendBufRaw(buf, ">");
		if (!node->NoChildren())
			sttAppendBufRaw(buf, RTF_ENDTAG);
	}

	if (node->GetText()) {
		if (!node->NoChildren()) {
			sttAppendBufRaw(buf, RTF_BEGINTEXT);
			char indentTextLevel[128];
			mir_snprintf(indentTextLevel, RTF_TEXTINDENT_FMT, (int)((indent + 1) * 200));
			sttAppendBufRaw(buf, indentTextLevel);
		}

		sttAppendBufT(buf, Utf2T(node->GetText()));
		if (!node->NoChildren())
			sttAppendBufRaw(buf, RTF_ENDTEXT);
	}

	for (auto *it : TiXmlEnum(node))
		sttRtfAppendXml(buf, it, flags & ~(JCPF_IN | JCPF_OUT), indent + 1);

	if (!node->NoChildren() || node->GetText()) {
		sttAppendBufRaw(buf, RTF_BEGINTAG);
		sttAppendBufRaw(buf, indentLevel);
		sttAppendBufRaw(buf, "</");
		sttAppendBufRaw(buf, RTF_BEGINTAGNAME);
		sttAppendBufRaw(buf, node->Name());
		sttAppendBufRaw(buf, RTF_ENDTAGNAME);
		sttAppendBufRaw(buf, ">");
	}
	else sttAppendBufRaw(buf, " />");

	sttAppendBufRaw(buf, RTF_ENDTAG);
}

DWORD CALLBACK sttStreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	StringBuf *buf = (StringBuf *)dwCookie;

	if (buf->streamOffset < buf->offset) {
		size_t cbLen = min(size_t(cb), buf->offset - buf->streamOffset);
		memcpy(pbBuff, buf->buf + buf->streamOffset, cbLen);
		buf->streamOffset += cbLen;
		*pcb = (LONG)cbLen;
	}
	else *pcb = 0;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// CJabberDlgConsole class

struct
{
	int type;
	wchar_t *title;
	int icon;
}
static filter_modes[] =
{
	{ TFilterInfo::T_JID,   L"JID",            IDI_JABBER },
	{ TFilterInfo::T_XMLNS, L"xmlns",          IDI_CONSOLE },
	{ TFilterInfo::T_ANY,   L"all attributes", IDI_FILTER_APPLY },
	{ TFilterInfo::T_OFF,   L"disabled",       IDI_FILTER_RESET },
};

class CJabberDlgConsole : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	CCtrlEdit edtInput;
	CCtrlCombo cmbFilter;
	CCtrlButton btnReset;
	CCtrlMButton btnFilter, btnMessage, btnPresence, btnQueries, btnRefresh;

public:
	CJabberDlgConsole(CJabberProto *proto) :
		CJabberDlgBase(proto, IDD_CONSOLE),
		btnReset(this, IDC_RESET),
		btnFilter(this, IDC_BTN_FILTER, g_plugin.getIcon(IDI_FILTER_APPLY), LPGEN("Filter mode")),
		btnQueries(this, IDC_BTN_IQ, g_plugin.getIcon(IDI_PL_QUERY_ALLOW), LPGEN("Queries")),
		btnMessage(this, IDC_BTN_MSG, g_plugin.getIcon(IDI_PL_MSG_ALLOW), LPGEN("Messages")),
		btnRefresh(this, IDC_BTN_FILTER_REFRESH, g_plugin.getIcon(IDI_NAV_REFRESH), LPGEN("Refresh list")),
		btnPresence(this, IDC_BTN_PRESENCE, g_plugin.getIcon(IDI_PL_PRIN_ALLOW), LPGEN("Presences")),
		edtInput(this, IDC_CONSOLEIN),
		cmbFilter(this, IDC_CB_FILTER)
	{
		SetMinSize(300, 400);

		btnReset.OnClick = Callback(this, &CJabberDlgConsole::onClick_Reset);
		btnFilter.OnClick = Callback(this, &CJabberDlgConsole::onClick_Filter);
		btnRefresh.OnClick = Callback(this, &CJabberDlgConsole::onClick_Refresh);
		btnQueries.OnClick = btnMessage.OnClick = btnPresence.OnClick = Callback(this, &CJabberDlgConsole::onClick_Options);

		cmbFilter.OnChange = Callback(this, &CJabberDlgConsole::onChange_Filter);
		cmbFilter.OnSelChanged = Callback(this, &CJabberDlgConsole::onSelChange_Filter);
	}

	bool OnInitDialog() override
	{
		CSuper::OnInitDialog();

		Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_CONSOLE));
		SendDlgItemMessage(m_hwnd, IDC_CONSOLE, EM_SETEDITSTYLE, SES_EXTENDBACKCOLOR, SES_EXTENDBACKCOLOR);
		SendDlgItemMessage(m_hwnd, IDC_CONSOLE, EM_EXLIMITTEXT, 0, 0x80000000);

		m_proto->m_filterInfo.msg = m_proto->getByte("consoleWnd_msg", TRUE);
		m_proto->m_filterInfo.presence = m_proto->getByte("consoleWnd_presence", TRUE);
		m_proto->m_filterInfo.iq = m_proto->getByte("consoleWnd_iq", TRUE);
		m_proto->m_filterInfo.type = (TFilterInfo::Type)m_proto->getByte("consoleWnd_ftype", TFilterInfo::T_OFF);

		*m_proto->m_filterInfo.pattern = 0;
		ptrW tszPattern(m_proto->getWStringA("consoleWnd_fpattern"));
		if (tszPattern != nullptr)
			wcsncpy_s(m_proto->m_filterInfo.pattern, tszPattern, _TRUNCATE);

		onClick_Refresh();
		cmbFilter.SetText(m_proto->m_filterInfo.pattern);

		btnFilter.MakeFlat(); btnFilter.MakePush(); btnFilter.Push(false);
		btnQueries.MakeFlat(); btnQueries.MakePush(); btnQueries.Push(m_proto->m_filterInfo.iq);
		btnMessage.MakeFlat(); btnMessage.MakePush(); btnMessage.Push(m_proto->m_filterInfo.msg);
		btnPresence.MakeFlat(); btnPresence.MakePush(); btnPresence.Push(m_proto->m_filterInfo.presence);
		btnRefresh.MakeFlat();

		for (auto &it : filter_modes)
			if (it.type == m_proto->m_filterInfo.type) {
				btnFilter.SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(it.icon));
				break;
			}

		cmbFilter.Enable(m_proto->m_filterInfo.type != TFilterInfo::T_OFF);
		btnRefresh.Enable(m_proto->m_filterInfo.type != TFilterInfo::T_OFF);

		Utils_RestoreWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "consoleWnd_");
		return true;
	}

	bool OnApply() override
	{
		if (!m_proto->m_bJabberOnline)
			MessageBox(m_hwnd, TranslateT("Can't send data while you are offline."), TranslateT("Jabber Error"), MB_ICONSTOP | MB_OK);
		else {
			ptrW textToSend(edtInput.GetText());

			TiXmlDocument doc;
			if (0 == doc.Parse(T2Utf(textToSend)))
				m_proto->m_ThreadInfo->send(doc.RootElement());
			else {
				StringBuf buf = {};
				sttAppendBufRaw(&buf, RTF_HEADER);
				sttAppendBufRaw(&buf, RTF_BEGINPLAINXML);
				sttAppendBufT(&buf, TranslateT("Outgoing XML parsing error"));
				sttAppendBufRaw(&buf, RTF_ENDPLAINXML);
				sttAppendBufRaw(&buf, RTF_SEPARATOR);
				sttAppendBufRaw(&buf, RTF_FOOTER);
				OnProtoRefresh(0, (LPARAM)&buf);
				sttEmptyBuf(&buf);
			}

			edtInput.SetText(L"");
		}
		return false;
	}

	void OnDestroy() override
	{
		m_proto->setByte("consoleWnd_iq", m_proto->m_filterInfo.iq);
		m_proto->setByte("consoleWnd_msg", m_proto->m_filterInfo.msg);
		m_proto->setByte("consoleWnd_ftype", m_proto->m_filterInfo.type);
		m_proto->setByte("consoleWnd_presence", m_proto->m_filterInfo.presence);
		m_proto->setWString("consoleWnd_fpattern", m_proto->m_filterInfo.pattern);

		Utils_SaveWindowPosition(m_hwnd, 0, m_proto->m_szModuleName, "consoleWnd_");

		m_proto->m_pDlgConsole = nullptr;

		PostThreadMessage(m_proto->m_dwConsoleThreadId, WM_QUIT, 0, 0);
	}

	int Resizer(UTILRESIZECONTROL *urc) override
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

	void onClick_Options(CCtrlMButton *)
	{
		m_proto->m_filterInfo.iq = btnQueries.IsPushed();
		m_proto->m_filterInfo.msg = btnMessage.IsPushed();
		m_proto->m_filterInfo.presence = btnPresence.IsPushed();
	}

	void onClick_Filter(CCtrlMButton *)
	{
		HMENU hMenu = CreatePopupMenu();
		for (auto &it : filter_modes)
			AppendMenu(hMenu, MF_STRING | ((it.type == m_proto->m_filterInfo.type) ? MF_CHECKED : 0), it.type + 1, TranslateW(it.title));

		RECT rc; 
		GetWindowRect(btnFilter.GetHwnd(), &rc);
		btnFilter.Push(true);
		int res = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_BOTTOMALIGN, rc.left, rc.top, 0, m_hwnd, nullptr);
		btnFilter.Push(false);
		DestroyMenu(hMenu);

		if (res) {
			m_proto->m_filterInfo.type = (TFilterInfo::Type)(res - 1);
			for (auto &it : filter_modes) {
				if (it.type == m_proto->m_filterInfo.type) {
					btnFilter.SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(it.icon));
					break;
				}
			}

			onClick_Refresh();
			cmbFilter.Enable(m_proto->m_filterInfo.type != TFilterInfo::T_OFF);
			btnRefresh.Enable(m_proto->m_filterInfo.type != TFilterInfo::T_OFF);
		}
	}

	void onClick_Refresh(CCtrlMButton* = nullptr)
	{
		ptrW wszSaveText(cmbFilter.GetText());

		cmbFilter.ResetContent();

		switch (m_proto->m_filterInfo.type) {
		case TFilterInfo::T_XMLNS:
		case TFilterInfo::T_ANY:
			for (int i = 0; i < g_cJabberFeatCapPairs; i++)
				cmbFilter.AddStringA(g_JabberFeatCapPairs[i].szFeature);
			for (int i = 0; i < g_cJabberFeatCapPairsExt; i++)
				cmbFilter.AddStringA(g_JabberFeatCapPairsExt[i].szFeature);
		}

		switch (m_proto->m_filterInfo.type) {
		case TFilterInfo::T_JID:
		case TFilterInfo::T_ANY:
			LISTFOREACH(i, m_proto, LIST_ROSTER)
				if (auto *item = m_proto->ListGetItemPtrFromIndex(i))
					cmbFilter.AddString(Utf2T(item->jid));
			LISTFOREACH(i, m_proto, LIST_CHATROOM)
				if (auto *item = m_proto->ListGetItemPtrFromIndex(i))
					cmbFilter.AddString(Utf2T(item->jid));
		}

		cmbFilter.SetText(wszSaveText);
	}

	void onClick_Reset(CCtrlButton *)
	{
		SetDlgItemText(m_hwnd, IDC_CONSOLE, L"");
	}

	void onChange_Filter(CCtrlCombo *)
	{
		mir_cslock lck(m_proto->m_filterInfo.csPatternLock);
		cmbFilter.GetText(m_proto->m_filterInfo.pattern, _countof(m_proto->m_filterInfo.pattern));
	}

	void onSelChange_Filter(CCtrlCombo *)
	{
		int idx = cmbFilter.GetCurSel();
		int len = cmbFilter.SendMsg(CB_GETLBTEXTLEN, idx, 0) + 1;

		mir_cslock lck(m_proto->m_filterInfo.csPatternLock);

		if (len > _countof(m_proto->m_filterInfo.pattern)) {
			wchar_t *buf = (wchar_t *)_alloca(len * sizeof(wchar_t));
			cmbFilter.SendMsg(CB_GETLBTEXT, idx, (LPARAM)buf);
			mir_wstrncpy(m_proto->m_filterInfo.pattern, buf, _countof(m_proto->m_filterInfo.pattern));
		}
		else cmbFilter.SendMsg(CB_GETLBTEXT, idx, (LPARAM)m_proto->m_filterInfo.pattern);
	}

	void OnProtoRefresh(WPARAM, LPARAM lParam) override
	{
		SendDlgItemMessage(m_hwnd, IDC_CONSOLE, WM_SETREDRAW, FALSE, 0);

		StringBuf *buf = (StringBuf *)lParam;
		buf->streamOffset = 0;

		EDITSTREAM es = { 0 };
		es.dwCookie = (DWORD_PTR)buf;
		es.pfnCallback = sttStreamInCallback;

		SCROLLINFO si = { 0 };
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(GetDlgItem(m_hwnd, IDC_CONSOLE), SB_VERT, &si);

		CHARRANGE oldSel, sel;
		POINT ptScroll;
		SendDlgItemMessage(m_hwnd, IDC_CONSOLE, EM_GETSCROLLPOS, 0, (LPARAM)&ptScroll);
		SendDlgItemMessage(m_hwnd, IDC_CONSOLE, EM_EXGETSEL, 0, (LPARAM)&oldSel);
		sel.cpMin = sel.cpMax = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_CONSOLE));
		SendDlgItemMessage(m_hwnd, IDC_CONSOLE, EM_EXSETSEL, 0, (LPARAM)&sel);
		SendDlgItemMessage(m_hwnd, IDC_CONSOLE, EM_STREAMIN, SF_RTF | SFF_SELECTION, (LPARAM)&es);
		SendDlgItemMessage(m_hwnd, IDC_CONSOLE, EM_EXSETSEL, 0, (LPARAM)&oldSel);

		// magic expression from tabsrmm :)
		if ((UINT)si.nPos >= (UINT)si.nMax - si.nPage - 5 || si.nMax - si.nMin - si.nPage < 50) {
			SendDlgItemMessage(m_hwnd, IDC_CONSOLE, WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
			sel.cpMin = sel.cpMax = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_CONSOLE));
			SendDlgItemMessage(m_hwnd, IDC_CONSOLE, EM_EXSETSEL, 0, (LPARAM)&sel);
		}
		else SendDlgItemMessage(m_hwnd, IDC_CONSOLE, EM_SETSCROLLPOS, 0, (LPARAM)&ptScroll);

		SendDlgItemMessage(m_hwnd, IDC_CONSOLE, WM_SETREDRAW, TRUE, 0);
		InvalidateRect(GetDlgItem(m_hwnd, IDC_CONSOLE), nullptr, FALSE);
	}
};

void __cdecl CJabberProto::ConsoleThread(void*)
{
	Thread_SetName("Jabber: ConsoleThread");
	MThreadLock threadLock(m_hThreadConsole);
	m_dwConsoleThreadId = ::GetCurrentThreadId();

	m_pDlgConsole = new CJabberDlgConsole(this);
	m_pDlgConsole->Show();

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	m_dwConsoleThreadId = 0;
}

void CJabberProto::ConsoleUninit()
{
	if (m_hThreadConsole) {
		PostThreadMessage(m_dwConsoleThreadId, WM_QUIT, 0, 0);
		if (MsgWaitForMultipleObjects(1, &m_hThreadConsole, FALSE, 5000, TRUE) == WAIT_TIMEOUT)
			TerminateThread(m_hThreadConsole, 0);

		if (m_hThreadConsole) {
			CloseHandle(m_hThreadConsole);
			m_hThreadConsole = nullptr;
		}
	}

	m_filterInfo.iq = m_filterInfo.msg = m_filterInfo.presence = false;
	m_filterInfo.type = TFilterInfo::T_OFF;
}

INT_PTR __cdecl CJabberProto::OnMenuHandleConsole(WPARAM, LPARAM)
{
	if (m_pDlgConsole)
		SetForegroundWindow(m_pDlgConsole->GetHwnd());
	else
		m_hThreadConsole = ForkThreadEx(&CJabberProto::ConsoleThread, 0, &m_dwConsoleThreadId);

	return 0;
}
