/*

Object UI extensions
Copyright (c) 2008  Victor Pavlychko, George Hazan
Copyright (c) 2012-17 Miranda NG project

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

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlSpin class

CCtrlSpin::CCtrlSpin(CDlgBase *dlg, int ctrlId)
	: CCtrlBase(dlg, ctrlId)
{}

BOOL CCtrlSpin::OnNotify(int, NMHDR *pnmh)
{
	if (pnmh->code == UDN_DELTAPOS) {
		NotifyChange();
		return TRUE;
	}

	return FALSE;
}

WORD CCtrlSpin::GetPosition()
{
	return SendMsg(UDM_GETPOS, 0, 0);
}

void CCtrlSpin::SetPosition(WORD wPos)
{
	SendMsg(UDM_SETPOS, 0, wPos);
}

void CCtrlSpin::SetRange(WORD wMax, WORD wMin)
{
	SendMsg(UDM_SETRANGE, 0, MAKELPARAM(wMax, wMin));
}
