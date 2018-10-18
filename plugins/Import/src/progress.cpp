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

void MirandaImport(HWND);

CProgressPageDlg::CProgressPageDlg() :
	CWizardPageDlg(IDD_PROGRESS)
{}

bool CProgressPageDlg::OnInitDialog()
{
	SendMessage(m_hwndParent, WIZM_DISABLEBUTTON, 0, 0);
	SendMessage(m_hwndParent, WIZM_DISABLEBUTTON, 1, 0);
	SendMessage(m_hwndParent, WIZM_DISABLEBUTTON, 2, 0);
	SendDlgItemMessage(m_hwnd, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
	PostMessage(m_hwnd, PROGM_START, 0, 0);
	return true;
}

void CProgressPageDlg::OnNext()
{
	PostMessage(m_hwndParent, WIZM_GOTOPAGE, 0, (LPARAM)new CFinishedPageDlg());
}

INT_PTR CProgressPageDlg::DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case PROGM_SETPROGRESS:
		SendDlgItemMessage(m_hwnd, IDC_PROGRESS, PBM_SETPOS, wParam, 0);
		break;

	case PROGM_ADDMESSAGE:
		{
			int i = SendDlgItemMessage(m_hwnd, IDC_STATUS, LB_ADDSTRING, 0, lParam);
			SendDlgItemMessage(m_hwnd, IDC_STATUS, LB_SETTOPINDEX, i, 0);
		}
		break;

	case PROGM_START:
		MirandaImport(m_hwnd);
		if (g_bServiceMode && !g_bSendQuit)
			DestroyWindow(g_hwndWizard);
		else {
			SendMessage(m_hwndParent, WIZM_ENABLEBUTTON, 1, 0);
			SendMessage(m_hwndParent, WIZM_ENABLEBUTTON, 2, 0);
		}
		break;
	}

	return CWizardPageDlg::DlgProc(uMsg, wParam, lParam);
}
