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

/////////////////////////////////////////////////////////////////////////////////////////
// Intro wizard page

CIntroPageDlg::CIntroPageDlg() :
	CWizardPageDlg(IDD_WIZARDINTRO)
{}

bool CIntroPageDlg::OnInitDialog()
{
	SendMessage(m_hwndParent, WIZM_DISABLEBUTTON, 0, 0);
	return true;
}

void CIntroPageDlg::OnNext()
{
	PostMessage(m_hwndParent, WIZM_GOTOPAGE, 0, (LPARAM)new CMirandaPageDlg());
}

/////////////////////////////////////////////////////////////////////////////////////////
// Final wizard page

CFinishedPageDlg::CFinishedPageDlg() :
	CWizardPageDlg(IDD_FINISHED)
{}

bool CFinishedPageDlg::OnInitDialog()
{
	SendMessage(m_hwndParent, WIZM_DISABLEBUTTON, 0, 0);
	SendMessage(m_hwndParent, WIZM_SETCANCELTEXT, 0, (LPARAM)TranslateT("Finish"));
	CheckDlgButton(m_hwnd, IDC_DONTLOADPLUGIN, BST_UNCHECKED);
	return true;
}

void CFinishedPageDlg::OnNext()
{
	PostMessage(m_hwndParent, WIZM_GOTOPAGE, 0, (LPARAM)new CMirandaPageDlg());
}

void CFinishedPageDlg::OnCancel()
{
	if (IsDlgButtonChecked(m_hwnd, IDC_DONTLOADPLUGIN))
		db_set_b(NULL, "PluginDisable", "import", 1);

	CWizardPageDlg::OnCancel();
}

/////////////////////////////////////////////////////////////////////////////////////////

CWizardPageDlg::CWizardPageDlg(int iDlgId) :
	CDlgBase(g_plugin, iDlgId),
	btnOk(this, IDOK),
	btnCancel(this, IDCANCEL)
{
	m_autoClose = 0; // disable built-in IDOK & IDCANCEL handlers;

	btnOk.OnClick = Callback(this, &CWizardPageDlg::onClick_Ok);
	btnCancel.OnClick = Callback(this, &CWizardPageDlg::onClick_Cancel);
}

void CWizardPageDlg::OnCancel()
{
	PostMessage(m_hwndParent, WM_CLOSE, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

class CWizardDlg : public CDlgBase
{
	CWizardPageDlg *m_pFirstPage;
	HWND hwndPage = nullptr;

public:
	CWizardDlg(CWizardPageDlg *pPage) :
		CDlgBase(g_plugin, IDD_WIZARD),
		m_pFirstPage(pPage)
	{
		m_autoClose = CLOSE_ON_CANCEL;
	}

	bool OnInitDialog() override
	{
		Window_SetIcon_IcoLib(m_hwnd, GetIconHandle(IDI_IMPORT));
		g_hwndWizard = m_hwnd;

		if (m_pFirstPage)
			PostMessage(m_hwnd, WIZM_GOTOPAGE, 0, (LPARAM)m_pFirstPage);
		return true;
	}

	bool OnClose() override
	{
		if (hwndPage)
			DestroyWindow(hwndPage);
		return true;
	}

	void OnDestroy() override
	{
		g_hwndWizard = nullptr;
		if (g_bSendQuit)
			PostQuitMessage(0);
	}

	INT_PTR DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		bool bFirstLaunch = hwndPage == nullptr;

		switch (uMsg) {
		case WIZM_GOTOPAGE:
			if (hwndPage)
				DestroyWindow(hwndPage);
			EnableWindow(GetDlgItem(m_hwnd, IDC_BACK), TRUE);
			EnableWindow(GetDlgItem(m_hwnd, IDOK), TRUE);
			EnableWindow(GetDlgItem(m_hwnd, IDCANCEL), TRUE);
			SetDlgItemText(m_hwnd, IDCANCEL, TranslateT("Cancel"));
			{
				CWizardPageDlg *pPage = (CWizardPageDlg*)lParam;
				pPage->SetParent(m_hwnd);
				pPage->Show();
				hwndPage = pPage->GetHwnd();
			}
			SetWindowPos(hwndPage, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			if (bFirstLaunch)
				ShowWindow(m_hwnd, SW_SHOW);
			break;

		case WIZM_DISABLEBUTTON:
			switch (wParam) {
			case 0:
				EnableWindow(GetDlgItem(m_hwnd, IDC_BACK), FALSE);
				break;

			case 1:
				EnableWindow(GetDlgItem(m_hwnd, IDOK), FALSE);
				break;

			case 2:
				EnableWindow(GetDlgItem(m_hwnd, IDCANCEL), FALSE);
				break;
			}
			break;

		case WIZM_ENABLEBUTTON:
			switch (wParam) {
			case 0:
				EnableWindow(GetDlgItem(m_hwnd, IDC_BACK), TRUE);
				break;

			case 1:
				EnableWindow(GetDlgItem(m_hwnd, IDOK), TRUE);
				break;

			case 2:
				EnableWindow(GetDlgItem(m_hwnd, IDCANCEL), TRUE);
				break;
			}
			break;

		case WIZM_SETCANCELTEXT:
			SetDlgItemText(m_hwnd, IDCANCEL, (wchar_t*)lParam);
			break;

		case WM_COMMAND:
			SendMessage(hwndPage, WM_COMMAND, wParam, lParam);
			break;
		}

		return CDlgBase::DlgProc(uMsg, wParam, lParam);
	}
};

LRESULT RunWizard(CWizardPageDlg *pPage, bool bModal)
{
	if (bModal)
		return CWizardDlg(pPage).DoModal();

	CWizardDlg *pDlg = new CWizardDlg(pPage);
	pDlg->Show();
	return 0;
}
