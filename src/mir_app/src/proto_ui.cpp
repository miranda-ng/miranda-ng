/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team,
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

CMPluginBase& ProtoGetInstance(const char *szModuleName)
{
	PROTOACCOUNT *pa = Proto_GetAccount(szModuleName);
	if (pa == nullptr)
		return g_plugin;

	MBaseProto *p = Proto_GetProto(pa->szProtoName);
	return (p == nullptr) ? g_plugin : GetPluginByInstance(p->hInst);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Base protocol dialog

CProtoIntDlgBase::CProtoIntDlgBase(PROTO_INTERFACE *proto, int idDialog) :
	CDlgBase(::ProtoGetInstance(proto->m_szModuleName), idDialog),
	m_proto_interface(proto)
{
}

void CProtoIntDlgBase::CreateLink(CCtrlData& ctrl, const char *szSetting, uint8_t type, uint32_t iValue)
{
	ctrl.CreateDbLink(m_proto_interface->m_szModuleName, szSetting, type, iValue);
}

void CProtoIntDlgBase::CreateLink(CCtrlData& ctrl, const char *szSetting, wchar_t *szValue)
{
	ctrl.CreateDbLink(m_proto_interface->m_szModuleName, szSetting, szValue);
}

void CProtoIntDlgBase::OnProtoRefresh(WPARAM, LPARAM) {}
void CProtoIntDlgBase::OnProtoActivate(WPARAM, LPARAM) {}
void CProtoIntDlgBase::OnProtoCheckOnline(WPARAM, LPARAM) {}

void CProtoIntDlgBase::SetStatusText(const wchar_t *statusText)
{
	if (m_hwndStatus == nullptr) {
		m_hwndStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE, nullptr, m_hwnd, 999);
		SetWindowPos(m_hwndStatus, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		UpdateStatusBar();
	}

	SendMessage(m_hwndStatus, SB_SETTEXT, 0, (LPARAM)statusText);
}

INT_PTR CProtoIntDlgBase::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR result;

	switch (msg) {
	case WM_INITDIALOG: // call inherited init code first
		result = CSuper::DlgProc(msg, wParam, lParam);
		m_proto_interface->WindowSubscribe(m_hwnd);
		return result;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(m_hwnd);
		m_proto_interface->WindowUnsubscribe(m_hwnd);
		break;

	case WM_SIZE:
		if (m_hwndStatus) {
			RECT rcStatus; GetWindowRect(m_hwndStatus, &rcStatus);
			RECT rcClient; GetClientRect(m_hwnd, &rcClient);
			SetWindowPos(m_hwndStatus, nullptr, 0, rcClient.bottom - (rcStatus.bottom - rcStatus.top), rcClient.right, (rcStatus.bottom - rcStatus.top), SWP_NOZORDER);
			UpdateStatusBar();
		}
		break;

	// Protocol events
	case WM_PROTO_ACTIVATE:
		OnProtoActivate(wParam, lParam);
		return 0;

	case WM_PROTO_CHECK_ONLINE:
		if (m_hwndStatus)
			UpdateStatusBar();
		OnProtoCheckOnline(wParam, lParam);
		return 0;

	case WM_PROTO_REFRESH:
		OnProtoRefresh(wParam, lParam);
		return 0;
	}

	return CSuper::DlgProc(msg, wParam, lParam);
}
void CProtoIntDlgBase::UpdateStatusBar()
{
	SIZE sz;

	HDC hdc = GetDC(m_hwndStatus);
	HFONT hFntSave = (HFONT)SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
	GetTextExtentPoint32(hdc, m_proto_interface->m_tszUserName, (int)mir_wstrlen(m_proto_interface->m_tszUserName), &sz);
	sz.cx += GetSystemMetrics(SM_CXSMICON) * 3;
	SelectObject(hdc, hFntSave);
	ReleaseDC(m_hwndStatus, hdc);

	RECT rcStatus; GetWindowRect(m_hwndStatus, &rcStatus);
	int parts[] = { rcStatus.right - rcStatus.left - sz.cx, -1 };
	SendMessage(m_hwndStatus, SB_SETPARTS, 2, (LPARAM)parts);
	SendMessage(m_hwndStatus, SB_SETICON, 1, (LPARAM)Skin_LoadProtoIcon(m_proto_interface->m_szModuleName, m_proto_interface->m_iStatus));
	SendMessage(m_hwndStatus, SB_SETTEXT, 1, (LPARAM)m_proto_interface->m_tszUserName);
}
