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
// CCtrlSlider class

CCtrlSlider::CCtrlSlider(CDlgBase *dlg, int ctrlId, int wMax, int wMin) :
	CCtrlData(dlg, ctrlId),
	m_wMin(wMin),
	m_wMax(wMax)
{
	m_bNotifiable = true;
}

BOOL CCtrlSlider::OnCommand(HWND, WORD, WORD idCode)
{
	if (idCode == WM_HSCROLL) {
		NotifyChange();
		return TRUE;
	}
	return FALSE;
}

bool CCtrlSlider::OnApply()
{
	CSuper::OnApply();

	if (m_dbLink != nullptr)
		SaveInt(GetPosition());
	return true;
}

void CCtrlSlider::OnReset()
{
	SendMsg(TBM_SETRANGE, 0, MAKELONG(m_wMin, m_wMax));

	if (m_dbLink != nullptr)
		SetPosition(LoadInt());
}

int CCtrlSlider::GetPosition() const
{
	return SendMsg(TBM_GETPOS, 0, 0);
}

void CCtrlSlider::SetPosition(int wPos)
{
	SendMsg(TBM_SETPOS, TRUE, wPos);
}
