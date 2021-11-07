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
// CCtrlCheck class

CCtrlCheck::CCtrlCheck(CDlgBase *dlg, int ctrlId)
	: CCtrlData(dlg, ctrlId)
{
	m_bNotifiable = true;
}

BOOL CCtrlCheck::OnCommand(HWND, WORD, WORD)
{
	NotifyChange();
	return TRUE;
}

bool CCtrlCheck::OnApply()
{
	CSuper::OnApply();

	if (m_dbLink != nullptr)
		SaveInt(GetState());
	return true;
}

void CCtrlCheck::OnReset()
{
	if (m_dbLink != nullptr)
		SetState(LoadInt());
}

int CCtrlCheck::GetState() const
{
	return ::SendMessage(m_hwnd, BM_GETCHECK, 0, 0);
}

void CCtrlCheck::SetState(int state)
{
	::SendMessage(m_hwnd, BM_SETCHECK, state, 0);
}

bool CCtrlCheck::IsChecked()
{
	return GetState() == BST_CHECKED;
}
