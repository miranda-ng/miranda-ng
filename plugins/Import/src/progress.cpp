/*

Import plugin for Miranda NG

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org)

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

#define PROGM_START (WM_USER+100)

static CProgressPageDlg *pDlg;

void MirandaImport(void);

CProgressPageDlg::CProgressPageDlg() :
	CWizardPageDlg(IDD_PROGRESS),
	m_list(this, IDC_STATUS),
	m_timer(this, 1)
{
	m_timer.OnEvent = Callback(this, &CProgressPageDlg::OnTimer);
}

bool CProgressPageDlg::OnInitDialog()
{
	pDlg = this;
	SendMessage(m_hwndParent, WIZM_DISABLEBUTTON, 0, 0);
	SendMessage(m_hwndParent, WIZM_DISABLEBUTTON, 1, 0);
	SendMessage(m_hwndParent, WIZM_DISABLEBUTTON, 2, 0);
	SendDlgItemMessage(m_hwnd, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
	m_timer.Start(50);
	return true;
}

void CProgressPageDlg::OnDestroy()
{
	pDlg = nullptr;
}

void CProgressPageDlg::OnNext()
{
	PostMessage(m_hwndParent, WIZM_GOTOPAGE, 0, (LPARAM)new CFinishedPageDlg());
}

void CProgressPageDlg::OnTimer(CTimer*)
{
	m_timer.Stop();

	MirandaImport();
	if (g_bServiceMode && !g_bSendQuit)
		DestroyWindow(g_hwndWizard);
	else {
		SendMessage(m_hwndParent, WIZM_ENABLEBUTTON, 1, 0);
		SendMessage(m_hwndParent, WIZM_ENABLEBUTTON, 2, 0);
	}
}

void CProgressPageDlg::AddMessage(const wchar_t *pMsg)
{
	m_list.SendMsg(LB_SETTOPINDEX, m_list.AddString(pMsg, 0), 0);
}

void CProgressPageDlg::SetProgress(int n)
{
	SendDlgItemMessage(m_hwnd, IDC_PROGRESS, PBM_SETPOS, n, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

void AddMessage(const wchar_t *fmt, ...)
{
	va_list args;
	wchar_t msgBuf[4096];
	va_start(args, fmt);
	mir_vsnwprintf(msgBuf, TranslateW(fmt), args);

	if (pDlg)
		pDlg->AddMessage(msgBuf);
}

void SetProgress(int n)
{
	if (pDlg)
		pDlg->SetProgress(n);
}

