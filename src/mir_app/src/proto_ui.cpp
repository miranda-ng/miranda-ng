/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-15 Miranda NG project,
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

HINSTANCE ProtoGetInstance(const char *szModuleName)
{
	PROTOACCOUNT *pa = Proto_GetAccount(szModuleName);
	if (pa == NULL)
		return NULL;

	PROTOCOLDESCRIPTOR *p = Proto_IsProtocolLoaded(pa->szProtoName);
	return (p == NULL) ? NULL : GetInstByAddress(p->fnInit);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Base protocol dialog

CProtoIntDlgBase::CProtoIntDlgBase(PROTO_INTERFACE *proto, int idDialog, bool show_label)
	: CDlgBase(::ProtoGetInstance(proto->m_szModuleName), idDialog),
	m_proto_interface(proto),
	m_show_label(show_label),
	m_hwndStatus(NULL)
{}

void CProtoIntDlgBase::CreateLink(CCtrlData& ctrl, char *szSetting, BYTE type, DWORD iValue)
{
	ctrl.CreateDbLink(m_proto_interface->m_szModuleName, szSetting, type, iValue);
}

void CProtoIntDlgBase::CreateLink(CCtrlData& ctrl, const char *szSetting, TCHAR *szValue)
{
	ctrl.CreateDbLink(m_proto_interface->m_szModuleName, szSetting, szValue);
}

void CProtoIntDlgBase::OnProtoRefresh(WPARAM, LPARAM) {}
void CProtoIntDlgBase::OnProtoActivate(WPARAM, LPARAM) {}
void CProtoIntDlgBase::OnProtoCheckOnline(WPARAM, LPARAM) {}

void CProtoIntDlgBase::SetStatusText(const TCHAR *statusText)
{
	if (m_hwndStatus)
		SendMessage(m_hwndStatus, SB_SETTEXT, 0, (LPARAM)statusText);
}

INT_PTR CProtoIntDlgBase::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR result;

	switch (msg) {
	case WM_INITDIALOG: // call inherited init code first
		result = CSuper::DlgProc(msg, wParam, lParam);
		m_proto_interface->WindowSubscribe(m_hwnd);
		if (m_show_label) {
			m_hwndStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE, NULL, m_hwnd, 999);
			SetWindowPos(m_hwndStatus, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			UpdateStatusBar();
			UpdateProtoTitle();
		}
		return result;

	case WM_DESTROY:
		IcoLib_ReleaseIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_BIG, 0));
		IcoLib_ReleaseIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, 0));
		m_proto_interface->WindowUnsubscribe(m_hwnd);
		break;

	case WM_SETTEXT:
		if (m_show_label && IsWindowUnicode(m_hwnd)) {
			TCHAR *szTitle = (TCHAR *)lParam;
			if (!_tcsstr(szTitle, m_proto_interface->m_tszUserName)) {
				UpdateProtoTitle(szTitle);
				return TRUE;
			}
		}
		break;

	case WM_SIZE:
		if (m_hwndStatus) {
			RECT rcStatus; GetWindowRect(m_hwndStatus, &rcStatus);
			RECT rcClient; GetClientRect(m_hwnd, &rcClient);
			SetWindowPos(m_hwndStatus, NULL, 0, rcClient.bottom - (rcStatus.bottom - rcStatus.top), rcClient.right, (rcStatus.bottom - rcStatus.top), SWP_NOZORDER);
			UpdateStatusBar();
		}
		break;

		// Protocol events
	case WM_PROTO_ACTIVATE:
		OnProtoActivate(wParam, lParam);
		return m_lresult;

	case WM_PROTO_CHECK_ONLINE:
		if (m_hwndStatus)
			UpdateStatusBar();
		OnProtoCheckOnline(wParam, lParam);
		return m_lresult;

	case WM_PROTO_REFRESH:
		OnProtoRefresh(wParam, lParam);
		return m_lresult;
	}

	return CSuper::DlgProc(msg, wParam, lParam);
}

void CProtoIntDlgBase::UpdateProtoTitle(const TCHAR *szText)
{
	if (!m_show_label)
		return;

	int curLength;
	const TCHAR *curText;

	if (szText) {
		curText = szText;
		curLength = (int)mir_tstrlen(curText);
	}
	else {
		curLength = GetWindowTextLength(m_hwnd) + 1;
		TCHAR *tmp = (TCHAR *)_alloca(curLength * sizeof(TCHAR));
		GetWindowText(m_hwnd, tmp, curLength);
		curText = tmp;
	}

	if (!_tcsstr(curText, m_proto_interface->m_tszUserName)) {
		size_t length = curLength + mir_tstrlen(m_proto_interface->m_tszUserName) + 256;
		TCHAR *text = (TCHAR *)_alloca(length * sizeof(TCHAR));
		mir_sntprintf(text, length, _T("%s [%s: %s]"), curText, TranslateT("Account"), m_proto_interface->m_tszUserName);
		SetWindowText(m_hwnd, text);
	}
}

void CProtoIntDlgBase::UpdateStatusBar()
{
	SIZE sz;

	HDC hdc = GetDC(m_hwndStatus);
	HFONT hFntSave = (HFONT)SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
	GetTextExtentPoint32(hdc, m_proto_interface->m_tszUserName, (int)mir_tstrlen(m_proto_interface->m_tszUserName), &sz);
	sz.cx += GetSystemMetrics(SM_CXSMICON) * 3;
	SelectObject(hdc, hFntSave);
	ReleaseDC(m_hwndStatus, hdc);

	RECT rcStatus; GetWindowRect(m_hwndStatus, &rcStatus);
	int parts[] = { rcStatus.right - rcStatus.left - sz.cx, -1 };
	SendMessage(m_hwndStatus, SB_SETPARTS, 2, (LPARAM)parts);
	SendMessage(m_hwndStatus, SB_SETICON, 1, (LPARAM)Skin_LoadProtoIcon(m_proto_interface->m_szModuleName, m_proto_interface->m_iStatus));
	SendMessage(m_hwndStatus, SB_SETTEXT, 1, (LPARAM)m_proto_interface->m_tszUserName);
}
