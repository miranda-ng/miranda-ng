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
// CCtrlSpin class

CCtrlSpin::CCtrlSpin(CDlgBase *dlg, int ctrlId, WORD wMax, WORD wMin) :
	CCtrlData(dlg, ctrlId),
	m_wMin(wMin),
	m_wMax(wMax),
	m_wCurr(0)
{}

BOOL CCtrlSpin::OnNotify(int, NMHDR *pnmh)
{
	if (pnmh->code == UDN_DELTAPOS) {
		auto *pEvent = (NMUPDOWN *)pnmh;
		m_wCurr = pEvent->iPos + pEvent->iDelta;

		NotifyChange();
		return TRUE;
	}
	return FALSE;
}

bool CCtrlSpin::OnApply()
{
	CSuper::OnApply();

	m_wCurr = SendMsg(UDM_GETPOS, 0, 0);
	if (m_dbLink != nullptr)
		SaveInt(m_wCurr);

	HWND hwndBuddy = (HWND)SendMsg(UDM_GETBUDDY, 0, 0);
	if (hwndBuddy) {
		wchar_t buf[100];
		_itow(m_wCurr, buf, 10);
		::SendMessage(hwndBuddy, WM_SETTEXT, 0, LPARAM(buf));
	}

	return true;
}

void CCtrlSpin::OnReset()
{
	SendMsg(UDM_SETRANGE, 0, MAKELPARAM(m_wMax, m_wMin));

	if (m_dbLink != nullptr)
		SetPosition(LoadInt());
}

WORD CCtrlSpin::GetPosition()
{
	return m_wCurr;
}

void CCtrlSpin::SetPosition(WORD wPos)
{
	SendMsg(UDM_SETPOS, 0, m_wCurr = wPos);
}
