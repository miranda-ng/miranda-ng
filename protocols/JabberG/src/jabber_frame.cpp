/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
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
#include "jabber_caps.h"

/////////////////////////////////////////////////////////////////////////////////////////
// CJabberInfoFrame

class CJabberInfoFrameItem : public MZeroedObject
{
public:
	char  *m_pszName;
	HANDLE m_hIcolibIcon;
	wchar_t *m_pszText;
	LPARAM m_pUserData;
	bool   m_bCompact;
	bool   m_bShow;
	RECT   m_rcItem;
	int    m_tooltipId;

	void (CJabberProto::*m_onEvent)(CJabberInfoFrame_Event*);

public:
	CJabberInfoFrameItem(char *pszName, bool bCompact = false, LPARAM pUserData = 0) :
		m_bShow(true), m_bCompact(bCompact), m_pUserData(pUserData)
	{
		m_pszName = mir_strdup(pszName);
	}

	~CJabberInfoFrameItem()
	{
		mir_free(m_pszName);
		mir_free(m_pszText);
	}

	void SetInfo(HANDLE hIcolibIcon, wchar_t *pszText)
	{
		mir_free(m_pszText);
		m_pszText = pszText ? mir_wstrdup(pszText) : nullptr;
		m_hIcolibIcon = hIcolibIcon;
	}

	static int cmp(const CJabberInfoFrameItem *p1, const CJabberInfoFrameItem *p2)
	{
		return mir_strcmp(p1->m_pszName, p2->m_pszName);
	}
};

CJabberInfoFrame::CJabberInfoFrame(CJabberProto *proto) :
	m_pItems(3, CJabberInfoFrameItem::cmp), m_compact(false)
{
	m_proto = proto;
	m_clickedItem = -1;

	InitClass();

	CLISTFrame frame = { sizeof(frame) };
	HWND hwndClist = g_clistApi.hwndContactList;
	frame.hWnd = CreateWindowEx(0, L"JabberInfoFrameClass", nullptr, WS_CHILD | WS_VISIBLE, 0, 0, 100, 100, hwndClist, nullptr, g_plugin.getInst(), this);
	frame.align = alBottom;
	frame.height = 2 * SZ_FRAMEPADDING + GetSystemMetrics(SM_CYSMICON) + SZ_LINEPADDING; // compact height by default
	frame.Flags = F_VISIBLE | F_LOCKED | F_NOBORDER | F_UNICODE;
	frame.szName.w = mir_a2u(proto->m_szModuleName);
	frame.szTBname.w = proto->m_tszUserName;
	m_frameId = g_plugin.addFrame(&frame);
	mir_free(frame.szName.w);
	if (m_frameId == -1) {
		DestroyWindow(frame.hWnd);
		return;
	}

	m_hhkFontsChanged = HookEventMessage(ME_FONT_RELOAD, m_hwnd, WM_APP);
	ReloadFonts();

	m_hwndToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, nullptr,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		m_hwnd, nullptr, g_plugin.getInst(), nullptr);
	SetWindowPos(m_hwndToolTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	CreateInfoItem("$", true);
	UpdateInfoItem("$", proto->m_hProtoIcon, proto->m_tszUserName);

	CreateInfoItem("$/JID", true);
	UpdateInfoItem("$/JID", Skin_GetIconHandle(SKINICON_OTHER_USERDETAILS), L"Offline");
	SetInfoItemCallback("$/JID", &CJabberProto::InfoFrame_OnSetup);
}

CJabberInfoFrame::~CJabberInfoFrame()
{
	CallService(MS_CLIST_FRAMES_REMOVEFRAME, (WPARAM)m_frameId, 0);

	if (m_hhkFontsChanged)
		UnhookEvent(m_hhkFontsChanged);

	if (m_hwnd != nullptr) {
		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);
		DestroyWindow(m_hwnd);
		DestroyWindow(m_hwndToolTip);
		DeleteObject(m_hfntText);
		DeleteObject(m_hfntTitle);
		m_hwnd = nullptr;
	}
}

void CJabberInfoFrame::InitClass()
{
	static bool bClassRegistered = false;
	if (bClassRegistered)
		return;

	WNDCLASSEX wcx = { 0 };
	wcx.cbSize = sizeof(wcx);
	wcx.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = GlobalWndProc;
	wcx.hInstance = g_plugin.getInst();
	wcx.lpszClassName = L"JabberInfoFrameClass";
	wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
	RegisterClassEx(&wcx);
	bClassRegistered = true;
}

LRESULT CALLBACK CJabberInfoFrame::GlobalWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CJabberInfoFrame *pFrame;

	if (msg == WM_CREATE) {
		CREATESTRUCT *pcs = (CREATESTRUCT *)lParam;
		pFrame = (CJabberInfoFrame *)pcs->lpCreateParams;
		if (pFrame) pFrame->m_hwnd = hwnd;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pFrame);
	}
	else pFrame = (CJabberInfoFrame *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	return pFrame ? pFrame->WndProc(msg, wParam, lParam) : DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CJabberInfoFrame::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_APP:
		ReloadFonts();
		return 0;

	case WM_PAINT:
		{
			RECT rc; GetClientRect(m_hwnd, &rc);
			m_compact = rc.bottom < (2 * (GetSystemMetrics(SM_CYSMICON) + SZ_LINEPADDING) + SZ_LINESPACING + 2 * SZ_FRAMEPADDING);

			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(m_hwnd, &ps);
			m_compact ? PaintCompact(hdc) : PaintNormal(hdc);
			EndPaint(m_hwnd, &ps);
			return 0;
		}

	case WM_RBUTTONUP:
		{
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };
			MapWindowPoints(m_hwnd, nullptr, &pt, 1);
			HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDFRAMECONTEXT, m_frameId, 0);
			int res = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, g_clistApi.hwndContactList, nullptr);
			Clist_MenuProcessCommand(res, 0, m_frameId);
			return 0;
		}

	case WM_LBUTTONDOWN:
		{
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };
			for (auto &it : m_pItems)
				if (it->m_onEvent && PtInRect(&it->m_rcItem, pt)) {
					m_clickedItem = m_pItems.indexOf(&it);
					return 0;
				}
		}
		return 0;

	case WM_LBUTTONUP:
		{
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };
			if ((m_clickedItem >= 0) && (m_clickedItem < m_pItems.getCount()) && m_pItems[m_clickedItem].m_onEvent && PtInRect(&m_pItems[m_clickedItem].m_rcItem, pt)) {
				CJabberInfoFrame_Event evt;
				evt.m_event = CJabberInfoFrame_Event::CLICK;
				evt.m_pszName = m_pItems[m_clickedItem].m_pszName;
				evt.m_pUserData = m_pItems[m_clickedItem].m_pUserData;
				(m_proto->*m_pItems[m_clickedItem].m_onEvent)(&evt);
				return 0;
			}

			m_clickedItem = -1;

			return 0;
		}

	case WM_LBUTTONDBLCLK:
		m_compact = !m_compact;
		UpdateSize();
		return 0;
	}

	return DefWindowProc(m_hwnd, msg, wParam, lParam);
}

void CJabberInfoFrame::LockUpdates()
{
	m_bLocked = true;
}

void CJabberInfoFrame::Update()
{
	m_bLocked = false;
	UpdateSize();
}

void CJabberInfoFrame::ReloadFonts()
{
	LOGFONT lfFont;
	m_clTitle = Font_GetW(L"Jabber", L"Frame title", &lfFont);
	DeleteObject(m_hfntTitle);
	m_hfntTitle = CreateFontIndirect(&lfFont);

	m_clText = Font_GetW(L"Jabber", L"Frame text", &lfFont);
	DeleteObject(m_hfntText);
	m_hfntText = CreateFontIndirect(&lfFont);

	m_clBack = Colour_GetW(L"Jabber", L"Background");

	UpdateSize();
}

void CJabberInfoFrame::UpdateSize()
{
	if (!m_hwnd || m_bLocked)
		return;

	int line_count = m_compact ? 1 : (m_pItems.getCount() - m_hiddenItemCount);
	int height = 2 * SZ_FRAMEPADDING + line_count * (GetSystemMetrics(SM_CYSMICON) + SZ_LINEPADDING) + (line_count - 1) * SZ_LINESPACING;

	if (CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, m_frameId), 0) & F_VISIBLE) {
		if (!ServiceExists(MS_SKIN_DRAWGLYPH)) {
			// crazy resizing for clist_nicer...
			CallService(MS_CLIST_FRAMES_SHFRAME, m_frameId, 0);
			CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, m_frameId), height);
			CallService(MS_CLIST_FRAMES_SHFRAME, m_frameId, 0);
		}
		else {
			CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, m_frameId), height);
			RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE);
		}
	}
	else CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, m_frameId), height);
}

void CJabberInfoFrame::RemoveTooltip(int id)
{
	TOOLINFO ti = { 0 };
	ti.cbSize = sizeof(TOOLINFO);

	ti.hwnd = m_hwnd;
	ti.uId = id;
	SendMessage(m_hwndToolTip, TTM_DELTOOLW, 0, (LPARAM)&ti);
}

void CJabberInfoFrame::SetToolTip(int id, RECT *rc, wchar_t *pszText)
{
	TOOLINFO ti = { 0 };
	ti.cbSize = sizeof(TOOLINFO);

	ti.hwnd = m_hwnd;
	ti.uId = id;
	SendMessage(m_hwndToolTip, TTM_DELTOOLW, 0, (LPARAM)&ti);

	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = m_hwnd;
	ti.uId = id;
	ti.hinst = g_plugin.getInst();
	ti.lpszText = pszText;
	ti.rect = *rc;
	SendMessage(m_hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
}

void CJabberInfoFrame::PaintSkinGlyph(HDC hdc, RECT *rc, char **glyphs, COLORREF fallback)
{
	if (ServiceExists(MS_SKIN_DRAWGLYPH)) {
		SKINDRAWREQUEST rq = { 0 };
		rq.hDC = hdc;
		rq.rcDestRect = *rc;
		rq.rcClipRect = *rc;

		for (; *glyphs; ++glyphs) {
			strncpy_s(rq.szObjectID, *glyphs, _TRUNCATE);
			if (!CallService(MS_SKIN_DRAWGLYPH, (WPARAM)&rq, 0))
				return;
		}
	}

	if (fallback != 0xFFFFFFFF) {
		HBRUSH hbr = CreateSolidBrush(fallback);
		FillRect(hdc, rc, hbr);
		DeleteObject(hbr);
	}
}

void CJabberInfoFrame::PaintCompact(HDC hdc)
{
	RECT rc; GetClientRect(m_hwnd, &rc);
	char *glyphs[] = { "Main,ID=ProtoInfo", "Main,ID=EventArea", "Main,ID=StatusBar", nullptr };
	PaintSkinGlyph(hdc, &rc, glyphs, m_clBack);

	HFONT hfntSave = (HFONT)SelectObject(hdc, m_hfntTitle);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, m_clTitle);

	int cx_icon = GetSystemMetrics(SM_CXSMICON);
	int cy_icon = GetSystemMetrics(SM_CYSMICON);

	int cx = rc.right - cx_icon - SZ_FRAMEPADDING;
	for (auto &it : m_pItems.rev_iter()) {
		SetRect(&it->m_rcItem, 0, 0, 0, 0);
		if (!it->m_bShow) continue;
		if (!it->m_bCompact) continue;

		int depth = 0;
		for (char *p = it->m_pszName; p = strchr(p + 1, '/'); ++depth);

		if (depth == 0) {
			if (it->m_hIcolibIcon) {
				HICON hIcon = IcoLib_GetIconByHandle(it->m_hIcolibIcon);
				if (hIcon) {
					DrawIconEx(hdc, SZ_FRAMEPADDING, (rc.bottom - cy_icon) / 2, hIcon, cx_icon, cy_icon, 0, nullptr, DI_NORMAL);
					IcoLib_ReleaseIcon(hIcon);
				}
			}

			RECT rcText; SetRect(&rcText, cx_icon + SZ_FRAMEPADDING + SZ_ICONSPACING, 0, rc.right - SZ_FRAMEPADDING, rc.bottom);
			DrawText(hdc, it->m_pszText, -1, &rcText, DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
		}
		else {
			if (it->m_hIcolibIcon) {
				HICON hIcon = IcoLib_GetIconByHandle(it->m_hIcolibIcon);
				if (hIcon) {
					SetRect(&it->m_rcItem, cx, (rc.bottom - cy_icon) / 2, cx + cx_icon, (rc.bottom - cy_icon) / 2 + cy_icon);
					DrawIconEx(hdc, cx, (rc.bottom - cy_icon) / 2, hIcon, cx_icon, cy_icon, 0, nullptr, DI_NORMAL);
					cx -= cx_icon;

					IcoLib_ReleaseIcon(hIcon);

					SetToolTip(it->m_tooltipId, &it->m_rcItem, it->m_pszText);
				}
			}
		}
	}

	SelectObject(hdc, hfntSave);
}

void CJabberInfoFrame::PaintNormal(HDC hdc)
{
	RECT rc; GetClientRect(m_hwnd, &rc);
	char *glyphs[] = { "Main,ID=ProtoInfo", "Main,ID=EventArea", "Main,ID=StatusBar", nullptr };
	PaintSkinGlyph(hdc, &rc, glyphs, m_clBack);

	HFONT hfntSave = (HFONT)SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
	SetBkMode(hdc, TRANSPARENT);

	int cx_icon = GetSystemMetrics(SM_CXSMICON);
	int cy_icon = GetSystemMetrics(SM_CYSMICON);
	int line_height = cy_icon + SZ_LINEPADDING;
	int cy = SZ_FRAMEPADDING;

	for (auto &it : m_pItems) {
		if (!it->m_bShow) {
			SetRect(&it->m_rcItem, 0, 0, 0, 0);
			continue;
		}

		int cx = SZ_FRAMEPADDING;
		int depth = 0;
		for (char *p = it->m_pszName; p = strchr(p + 1, '/'); cx += cx_icon) ++depth;

		SetRect(&it->m_rcItem, cx, cy, rc.right - SZ_FRAMEPADDING, cy + line_height);

		if (it->m_hIcolibIcon) {
			HICON hIcon = IcoLib_GetIconByHandle(it->m_hIcolibIcon);
			if (hIcon) {
				DrawIconEx(hdc, cx, cy + (line_height - cy_icon) / 2, hIcon, cx_icon, cy_icon, 0, nullptr, DI_NORMAL);
				cx += cx_icon + SZ_ICONSPACING;

				IcoLib_ReleaseIcon(hIcon);
			}
		}

		SelectObject(hdc, depth ? m_hfntText : m_hfntTitle);
		SetTextColor(hdc, depth ? m_clText : m_clTitle);

		RECT rcText; SetRect(&rcText, cx, cy, rc.right - SZ_FRAMEPADDING, cy + line_height);
		DrawText(hdc, it->m_pszText, -1, &rcText, DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

		RemoveTooltip(it->m_tooltipId);

		cy += line_height + SZ_LINESPACING;
	}

	SelectObject(hdc, hfntSave);
}

void CJabberInfoFrame::CreateInfoItem(char *pszName, bool bCompact, LPARAM pUserData)
{
	if (m_pItems.find((CJabberInfoFrameItem*)&pszName))
		return;

	CJabberInfoFrameItem *newItem = new CJabberInfoFrameItem(pszName, bCompact, pUserData);
	newItem->m_tooltipId = m_nextTooltipId++;
	m_pItems.insert(newItem);
	UpdateSize();
}

void CJabberInfoFrame::SetInfoItemCallback(char *pszName, void (CJabberProto::*onEvent)(CJabberInfoFrame_Event *))
{
	if (CJabberInfoFrameItem *pItem = m_pItems.find((CJabberInfoFrameItem*)&pszName))
		pItem->m_onEvent = onEvent;
}

void CJabberInfoFrame::UpdateInfoItem(char *pszName, HANDLE hIcolibIcon, wchar_t *pszText)
{
	if (CJabberInfoFrameItem *pItem = m_pItems.find((CJabberInfoFrameItem*)&pszName))
		pItem->SetInfo(hIcolibIcon, pszText);
	if (m_hwnd)
		RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE);
}

void CJabberInfoFrame::ShowInfoItem(char *pszName, bool bShow)
{
	bool bUpdate = false;
	size_t length = mir_strlen(pszName);
	for (auto &it : m_pItems)
		if ((it->m_bShow != bShow) && !strncmp(it->m_pszName, pszName, length)) {
			it->m_bShow = bShow;
			m_hiddenItemCount += bShow ? -1 : 1;
			bUpdate = true;
		}

	if (bUpdate)
		UpdateSize();
}

void CJabberInfoFrame::RemoveInfoItem(char *pszName)
{
	bool bUpdate = false;
	size_t length = mir_strlen(pszName);

	for (auto &p : m_pItems.rev_iter()) {
		if (!strncmp(p->m_pszName, pszName, length)) {
			if (!p->m_bShow)
				--m_hiddenItemCount;
			RemoveTooltip(p->m_tooltipId);
			m_pItems.removeItem(&p);
			bUpdate = true;
		}
	}

	if (bUpdate)
		UpdateSize();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CJabberProto::InitInfoFrame()
{
	if (!ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
		return;

	if (!m_bDisableFrame)
		m_pInfoFrame = new CJabberInfoFrame(this);
	else {
		delete m_pInfoFrame;
		m_pInfoFrame = nullptr;
	}
}
