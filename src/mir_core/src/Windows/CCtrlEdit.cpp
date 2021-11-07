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
// CCtrlEdit class

CCtrlEdit::CCtrlEdit(CDlgBase *dlg, int ctrlId)
	: CCtrlData(dlg, ctrlId)
{}

BOOL CCtrlEdit::OnCommand(HWND, WORD, WORD idCode)
{
	if (idCode == EN_CHANGE)
		NotifyChange();
	return TRUE;
}

bool CCtrlEdit::OnApply()
{
	CSuper::OnApply();

	if (GetDataType() == DBVT_WCHAR) {
		int len = GetWindowTextLength(m_hwnd) + 1;
		wchar_t *buf = (wchar_t *)_alloca(sizeof(wchar_t) * len);
		GetWindowText(m_hwnd, buf, len);
		SaveText(buf);
	}
	else if (GetDataType() != DBVT_DELETED) {
		SaveInt(GetInt());
	}
	return true;
}

void CCtrlEdit::OnReset()
{
	m_bSilent = (GetWindowLong(m_hwnd, GWL_STYLE) & ES_READONLY) != 0;

	if (GetDataType() == DBVT_WCHAR)
		SetText(LoadText());
	else if (GetDataType() != DBVT_DELETED)
		SetInt(LoadInt());
}

void CCtrlEdit::SetMaxLength(unsigned int len)
{
	SendMsg(EM_SETLIMITTEXT, len, 0);
}
