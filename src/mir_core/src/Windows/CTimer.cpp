/*

Object UI extensions
Copyright (c) 2008  Victor Pavlychko, George Hazan
Copyright (C) 2012-22 Miranda NG team

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
// CTimer

CTimer::CTimer(CDlgBase *wnd, UINT_PTR idEvent)
	: m_wnd(wnd), m_idEvent(idEvent)
{
	if (wnd)
		wnd->AddTimer(this);
}

CTimer::~CTimer()
{
	if (m_wnd)
		m_wnd->RemoveTimer(m_idEvent);
}

BOOL CTimer::OnTimer()
{
	OnEvent(this);
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTimer::Start(int elapse)
{
	::SetTimer(m_wnd->GetHwnd(), m_idEvent, elapse, nullptr);
}

bool CTimer::Stop()
{
	return 0 != ::KillTimer(m_wnd->GetHwnd(), m_idEvent);
}

/////////////////////////////////////////////////////////////////////////////////////////

struct TStartParam
{
	CTimer *pTimer;
	int period;
};

static INT_PTR CALLBACK stubStart(void *param)
{
	auto *p = (TStartParam *)param;
	return ::SetTimer(p->pTimer->GetHwnd(), p->pTimer->GetEventId(), p->period, nullptr);
}

void CTimer::StartSafe(int elapse)
{
	TStartParam param = { this, elapse };
	CallFunctionSync(stubStart, &param);
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK stubStop(void *param)
{
	auto *p = (CTimer*)param;
	return ::KillTimer(p->GetHwnd(), p->GetEventId());
}

void CTimer::StopSafe()
{
	CallFunctionSync(stubStop, this);
}
