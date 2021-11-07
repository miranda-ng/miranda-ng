/*

Object UI extensions
Copyright (c) 2008  Victor Pavlychko, George Hazan
Copyright (C) 2012-21 Miranda NG team

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

#include "../stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// CSplitter

CSplitter::CSplitter(CDlgBase *wnd, int idCtrl)
	: CCtrlBase(wnd, idCtrl),
	m_iPosition(0)
{
}

void CSplitter::OnInit()
{
	CSuper::OnInit();
	Subclass();
}

LRESULT CSplitter::CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_NCHITTEST:
		return HTCLIENT;

	case WM_SETCURSOR:
		RECT rc;
		GetClientRect(m_hwnd, &rc);
		SetCursor(rc.right > rc.bottom ? g_hCursorNS : g_hCursorWE);
		return TRUE;

	case WM_LBUTTONDOWN:
		SetCapture(m_hwnd);
		return 0;

	case WM_MOUSEMOVE:
		if (GetCapture() == m_hwnd) {
			POINT pt = { 0, 0 };
			GetClientRect(m_hwnd, &rc);
			if (rc.right > rc.bottom) {
				pt.y = HIWORD(GetMessagePos()) + rc.bottom / 2;
				ScreenToClient(m_parentWnd->GetHwnd(), &pt);
				m_iPosition = pt.y;
			}
			else {
				pt.x = LOWORD(GetMessagePos()) + rc.right / 2;
				ScreenToClient(m_parentWnd->GetHwnd(), &pt);
				m_iPosition = pt.x;
			}

			OnChange(this);
			PostMessage(m_parentWnd->GetHwnd(), WM_SIZE, 0, 0);
		}
		return 0;

	case WM_LBUTTONUP:
		ReleaseCapture();
		PostMessage(m_parentWnd->GetHwnd(), WM_SIZE, 0, 0);
		return 0;
	}

	return CSuper::CustomWndProc(msg, wParam, lParam);
}
