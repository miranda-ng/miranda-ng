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
// CCtrlDate class

CCtrlDate::CCtrlDate(CDlgBase *dlg, int ctrlId) :
	CCtrlData(dlg, ctrlId)
{}

BOOL CCtrlDate::OnNotify(int, NMHDR *pnmh)
{
	if (pnmh->code == DTN_DATETIMECHANGE) {
		NotifyChange();
		return TRUE;
	}
	return FALSE;
}

void CCtrlDate::GetTime(SYSTEMTIME *pDate)
{
   ::SendMessage(m_hwnd, DTM_GETSYSTEMTIME, 0, (LPARAM)pDate);
}

void CCtrlDate::SetTime(SYSTEMTIME *pDate)
{
   ::SendMessage(m_hwnd, DTM_SETSYSTEMTIME, 0, (LPARAM)pDate);
}
