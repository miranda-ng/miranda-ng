/*

Object UI extensions
Copyright (c) 2008  Victor Pavlychko, George Hazan
Copyright (C) 2012-25 Miranda NG team

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
// CCtrlProgress

CCtrlProgress::CCtrlProgress(CDlgBase *wnd, int idCtrl)
	: CCtrlBase(wnd, idCtrl)
{
}

void CCtrlProgress::SetRange(uint16_t max, uint16_t min)
{
	SendMsg(PBM_SETRANGE, 0, MAKELPARAM(min, max));
}

void CCtrlProgress::SetPosition(uint16_t value)
{
	SendMsg(PBM_SETPOS, value, 0);
}

void CCtrlProgress::SetStep(uint16_t value)
{
	SendMsg(PBM_SETSTEP, value, 0);
}

uint16_t CCtrlProgress::Move(uint16_t delta)
{
	return delta == 0
		? SendMsg(PBM_STEPIT, 0, 0)
		: SendMsg(PBM_DELTAPOS, delta, 0);
}
