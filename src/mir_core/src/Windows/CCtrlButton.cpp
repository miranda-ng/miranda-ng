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
// CCtrlButton

CCtrlButton::CCtrlButton(CDlgBase* wnd, int idCtrl)
	: CCtrlBase(wnd, idCtrl)
{}

BOOL CCtrlButton::OnCommand(HWND, WORD, WORD idCode)
{
	if (idCode == BN_CLICKED)
		OnClick(this);
	return FALSE;
}

void CCtrlButton::Click()
{
	if (Enabled())
		::SendMessage(m_parentWnd->GetHwnd(), WM_COMMAND, MAKELONG(m_idCtrl, BN_CLICKED), 0);
}

bool CCtrlButton::IsPushed() const
{
	return ::SendMessage(m_hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void CCtrlButton::Push(bool bPushed)
{
	if (Enabled())
		::SendMessage(m_hwnd, BM_SETCHECK, (bPushed) ? BST_CHECKED : BST_UNCHECKED, 0);
}
