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
// CProgress

CProgress::CProgress(CDlgBase *wnd, int idCtrl)
	: CCtrlBase(wnd, idCtrl)
{
}

void CProgress::SetRange(WORD max, WORD min)
{
	SendMsg(PBM_SETRANGE, 0, MAKELPARAM(min, max));
}

void CProgress::SetPosition(WORD value)
{
	SendMsg(PBM_SETPOS, value, 0);
}

void CProgress::SetStep(WORD value)
{
	SendMsg(PBM_SETSTEP, value, 0);
}

WORD CProgress::Move(WORD delta)
{
	return delta == 0
		? SendMsg(PBM_STEPIT, 0, 0)
		: SendMsg(PBM_DELTAPOS, delta, 0);
}
