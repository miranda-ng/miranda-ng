/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Victor Pavlychko
Copyright (ñ) 2012-15 Miranda NG project

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

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlFilterListView

#define FILTER_BOX_HEIGHT		21

struct CFilterData : public MZeroedObject
{
	HFONT m_hfntNormal;
	HFONT m_hfntEmpty;
	COLORREF m_clGray;
	TCHAR *m_filterText;

	RECT m_rcButtonClear;
	RECT m_rcEditBox;

	WNDPROC m_oldWndProc;
	HWND m_hwndOwner;
	HWND m_hwndEditBox;

	void ReleaseFilterData()
	{
		DeleteObject(m_hfntEmpty);	m_hfntEmpty = NULL;
	}

	~CFilterData()
	{
		ReleaseFilterData();
	}
};

CCtrlFilterListView::CCtrlFilterListView(CDlgBase* dlg, int ctrlId, bool trackFilter, bool keepHiglight):
	CCtrlListView(dlg, ctrlId),
	m_trackFilter(trackFilter),
	m_keepHiglight(keepHiglight)
{
	fdat = new CFilterData;
}

CCtrlFilterListView::~CCtrlFilterListView()
{
	if (fdat->m_filterText) mir_free(fdat->m_filterText);
	delete fdat;
}

TCHAR *CCtrlFilterListView::GetFilterText()
{
	return fdat->m_filterText;
}

void CCtrlFilterListView::OnInit()
{
	CSuper::OnInit();
	Subclass();
}

static LRESULT CALLBACK sttEditBoxSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CFilterData *fdat = (CFilterData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (fdat == NULL)
		return DefWindowProc(hwnd, msg, wParam, lParam);

	switch (msg) {
	case WM_GETDLGCODE:
		if ((wParam == VK_RETURN) || (wParam == VK_ESCAPE))
			return DLGC_WANTMESSAGE;
		break;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			if (fdat->m_filterText) mir_free(fdat->m_filterText);
			int length = GetWindowTextLength(hwnd) + 1;
			if (length == 1)
				fdat->m_filterText = 0;
			else {
				fdat->m_filterText = (TCHAR *)mir_alloc(sizeof(TCHAR) * length);
				GetWindowText(hwnd, fdat->m_filterText, length);
			}

			DestroyWindow(hwnd);
			RedrawWindow(fdat->m_hwndOwner, NULL, NULL, RDW_INVALIDATE|RDW_FRAME);
			PostMessage(fdat->m_hwndOwner, WM_APP, 0, 0);
		}
		else if (wParam == VK_ESCAPE) {
			DestroyWindow(hwnd);
			return 0;
		}

		PostMessage(fdat->m_hwndOwner, WM_APP, 1, 0);
		break;

	case WM_KILLFOCUS:
		DestroyWindow(hwnd);
		return 0;

	case WM_DESTROY:
		fdat->m_hwndEditBox = NULL;
	}

	return CallWindowProc(fdat->m_oldWndProc, hwnd, msg, wParam, lParam);
}

void CCtrlFilterListView::FilterHighlight(TCHAR *str)
{
	TCHAR buf[256];
	int count = GetItemCount();
	for (int i=0; i < count; i++) {
		bool found = false;
		if (str) {
			for (int j = 0; j < 10; ++j) {
				GetItemText(i, j, buf, SIZEOF(buf));
				if (!*buf)
					break;

				if (_tcsstr(buf, str)) {
					found = true;
					break;
				}
			}
		}

		SetItemState(i, found ? LVIS_DROPHILITED  : 0, LVIS_DROPHILITED);
	}
}

LRESULT CCtrlFilterListView::CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	POINT pt;

	switch (msg) {
	case WM_APP:
		switch (wParam) {
		case 0:
			OnFilterChanged(this);
			if (!m_keepHiglight)
				FilterHighlight(NULL);
			break;

		case 1:
			if (m_trackFilter && fdat->m_hwndEditBox) {
				TCHAR *str = 0;
				int length = GetWindowTextLength(fdat->m_hwndEditBox) + 1;
				if (length == 1)
					str = 0;
				else {
					str = (TCHAR *)mir_alloc(sizeof(TCHAR) * length);
					GetWindowText(fdat->m_hwndEditBox, str, length);
				}
				FilterHighlight(str);
				if (str) mir_free(str);
			}
			break;

		case 2:
			fdat->m_hwndOwner = m_hwnd;
			fdat->m_hwndEditBox = CreateWindow(_T("edit"), fdat->m_filterText,
				WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_LEFT|ES_AUTOHSCROLL,
				0, 0, 0, 0,
				::GetParent(m_hwnd), (HMENU)-1, hInst, NULL);

			SendMessage(fdat->m_hwndEditBox, WM_SETFONT, (WPARAM)fdat->m_hfntNormal, 0);

			RECT rc = fdat->m_rcEditBox;
			MapWindowPoints(m_hwnd, ::GetParent(m_hwnd), (LPPOINT)&rc, 2);
			SetWindowPos(fdat->m_hwndEditBox, HWND_TOP, rc.left-5, rc.top+2, rc.right-rc.left, rc.bottom-rc.top-4, SWP_SHOWWINDOW);
			SendMessage(fdat->m_hwndEditBox, EM_SETSEL, 0, -1);

			fdat->m_oldWndProc = (WNDPROC)GetWindowLongPtr(fdat->m_hwndEditBox, GWLP_WNDPROC);
			SetWindowLongPtr(fdat->m_hwndEditBox, GWLP_USERDATA, (LONG_PTR)fdat);
			SetWindowLongPtr(fdat->m_hwndEditBox, GWLP_WNDPROC, (LONG_PTR)sttEditBoxSubclassProc);

			SetFocus(m_hwnd); // hack to avoid popping of list over the box...
			SetFocus(fdat->m_hwndEditBox);
		}
		break;

	case WM_NCCALCSIZE:
		{
			RECT *prect = (RECT *)lParam;

			CSuper::CustomWndProc(msg, wParam, lParam);
			prect->bottom -= FILTER_BOX_HEIGHT;

			fdat->ReleaseFilterData();

			fdat->m_hfntNormal = (HFONT)SendMessage(m_hwnd, WM_GETFONT, 0, 0);
			if (!fdat->m_hfntNormal)
				fdat->m_hfntNormal = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

			LOGFONT lf;
			GetObject(fdat->m_hfntNormal, sizeof(lf), &lf);
			lf.lfItalic = TRUE;
			fdat->m_hfntEmpty = CreateFontIndirect(&lf);

			COLORREF clText = GetSysColor(COLOR_WINDOWTEXT);
			COLORREF clBack = GetSysColor(COLOR_WINDOW);
			fdat->m_clGray = RGB(
				(GetRValue(clBack) + 2*GetRValue(clText)) / 3,
				(GetGValue(clBack) + 2*GetGValue(clText)) / 3,
				(GetBValue(clBack) + 2*GetBValue(clText)) / 3);

			if (fdat->m_hwndEditBox)
				DestroyWindow(fdat->m_hwndEditBox);
		}
		return 0;

	case WM_NCPAINT:
		CSuper::CustomWndProc(msg, wParam, lParam);
		{
			RECT rc;
			GetWindowRect(m_hwnd, &rc);
			OffsetRect(&rc, -rc.left, -rc.top);
			InflateRect(&rc, -1, -1);
			rc.top = rc.bottom - FILTER_BOX_HEIGHT;

			POINT pts[] = {
				{rc.left, rc.top},
				{rc.left+FILTER_BOX_HEIGHT, rc.top},
				{rc.left+FILTER_BOX_HEIGHT+FILTER_BOX_HEIGHT/2+1, rc.top+FILTER_BOX_HEIGHT/2+1},
				{rc.left+FILTER_BOX_HEIGHT, rc.top+FILTER_BOX_HEIGHT},
				{rc.left, rc.top+FILTER_BOX_HEIGHT},
			};
			HRGN hrgnFilter = CreatePolygonRgn(pts, SIZEOF(pts), ALTERNATE);

			HDC hdc = GetWindowDC(m_hwnd);

			FillRect(hdc, &rc, GetSysColorBrush(COLOR_WINDOW));
			FillRgn(hdc, hrgnFilter, GetSysColorBrush(COLOR_BTNFACE));

			SetBkMode(hdc, TRANSPARENT);

			if (fdat->m_filterText) {
				SetRect(&fdat->m_rcButtonClear,
					rc.right - FILTER_BOX_HEIGHT + (FILTER_BOX_HEIGHT-16)/2, rc.top + (FILTER_BOX_HEIGHT-16)/2,
					rc.right - FILTER_BOX_HEIGHT + (FILTER_BOX_HEIGHT-16)/2 + 16, rc.top + (FILTER_BOX_HEIGHT-16)/2 + 16);

				DrawIconEx(hdc, rc.left + (FILTER_BOX_HEIGHT-16)/2, rc.top + (FILTER_BOX_HEIGHT-16)/2, g_LoadIconEx("sd_filter_apply"), 16, 16, 0, NULL, DI_NORMAL);
				DrawIconEx(hdc, rc.right - FILTER_BOX_HEIGHT + (FILTER_BOX_HEIGHT-16)/2, rc.top + (FILTER_BOX_HEIGHT-16)/2, LoadSkinnedIcon(SKINICON_OTHER_DELETE), 16, 16, 0, NULL, DI_NORMAL);

				rc.left += 5*FILTER_BOX_HEIGHT/3;
				rc.right -= 5*FILTER_BOX_HEIGHT/3;

				fdat->m_rcEditBox = rc;

				SelectObject(hdc, fdat->m_hfntNormal);
				::SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
				DrawText(hdc, fdat->m_filterText, -1, &rc, DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS);
			}
			else {
				SetRect(&fdat->m_rcButtonClear, 0, 0, 0, 0);

				DrawIconEx(hdc, rc.left + (FILTER_BOX_HEIGHT-16)/2, rc.top + (FILTER_BOX_HEIGHT-16)/2, g_LoadIconEx("sd_filter_reset"), 16, 16, 0, NULL, DI_NORMAL);

				rc.left += 5*FILTER_BOX_HEIGHT/3;
				rc.right -= 5;

				fdat->m_rcEditBox = rc;

				SelectObject(hdc, fdat->m_hfntEmpty);
				::SetTextColor(hdc, fdat->m_clGray);
				DrawText(hdc, TranslateT("Set filter..."), -1, &rc, DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS);
			}

			ReleaseDC(m_hwnd, hdc);

			DeleteObject(hrgnFilter);
		}
		return 0;

	case WM_NCHITTEST:
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		MapWindowPoints(NULL, m_hwnd, &pt, 1);

		if (PtInRect(&fdat->m_rcButtonClear, pt))
			return HTBORDER;
		if (PtInRect(&fdat->m_rcEditBox, pt))
			return HTBORDER;
		break;

	case WM_NCLBUTTONUP:
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		MapWindowPoints(NULL, m_hwnd, &pt, 1);

		if (PtInRect(&fdat->m_rcButtonClear, pt)) {
			SetFocus(m_hwnd);
			if (fdat->m_filterText) mir_free(fdat->m_filterText);
			fdat->m_filterText = NULL;
			RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE|RDW_FRAME);
			OnFilterChanged(this);
			FilterHighlight(NULL);
		}
		else if (PtInRect(&fdat->m_rcEditBox, pt))
			PostMessage(m_hwnd, WM_APP, 2, 0);
		break;

	case WM_KEYDOWN:
		if (wParam == 'F' && GetAsyncKeyState(VK_CONTROL))
			PostMessage(m_hwnd, WM_APP, 2, 0);
		break;
	}

	return CSuper::CustomWndProc(msg, wParam, lParam);
}
