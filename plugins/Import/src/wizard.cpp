/*

Import plugin for Miranda NG

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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
{
}

bool CIntroPageDlg::OnInitDialog()
{
	SendMessage(m_hwndParent, WIZM_DISABLEBUTTON, 0, 0);
	return true;
}

int CIntroPageDlg::Resizer(UTILRESIZECONTROL *urc)
{
	if (urc->wId == -1)
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;

	return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;
}

void CIntroPageDlg::OnNext()
{
	PostMessage(m_hwndParent, WIZM_GOTOPAGE, 0, (LPARAM)new CMirandaPageDlg());
}

/////////////////////////////////////////////////////////////////////////////////////////
// Final wizard page

CFinishedPageDlg::CFinishedPageDlg() :
	CSuper(IDD_FINISHED)
{}

bool CFinishedPageDlg::OnInitDialog()
{
	SendMessage(m_hwndParent, WIZM_DISABLEBUTTON, 0, 0);
	SendMessage(m_hwndParent, WIZM_SETCANCELTEXT, 0, (LPARAM)TranslateT("Finish"));
	CheckDlgButton(m_hwnd, IDC_DONTLOADPLUGIN, BST_UNCHECKED);
	return true;
}

int CFinishedPageDlg::Resizer(UTILRESIZECONTROL*)
{
	return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;
}

void CFinishedPageDlg::OnNext()
{
	PostMessage(m_hwndParent, WIZM_GOTOPAGE, 0, (LPARAM)new CMirandaPageDlg());
}

void CFinishedPageDlg::OnCancel()
{
	if (IsDlgButtonChecked(m_hwnd, IDC_DONTLOADPLUGIN))
		SetPluginOnWhiteList("import", false);

	CSuper::OnCancel();
}

/////////////////////////////////////////////////////////////////////////////////////////

CWizardPageDlg::CWizardPageDlg(int iDlgId) :
	CDlgBase(g_plugin, iDlgId),
	btnOk(this, IDOK),
	btnCancel(this, IDCANCEL)
{
	m_autoClose = 0; // disable built-in IDOK & IDCANCEL handlers;
	m_forceResizable = true;

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
	int m_splitterX = 0, m_splitterY = 0;

public:
	CWizardDlg(CWizardPageDlg *pPage) :
		CDlgBase(g_plugin, IDD_WIZARD),
		m_pFirstPage(pPage)
	{
		SetMinSize(330, 286);
		m_autoClose = CLOSE_ON_CANCEL;
	}

	bool OnInitDialog() override
	{
		Utils_RestoreWindowPosition(m_hwnd, 0, IMPORT_MODULE, "wiz");

		Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_IMPORT));
		g_hwndWizard = m_hwnd;

		if (m_pFirstPage)
			PostMessage(m_hwnd, WIZM_GOTOPAGE, 0, (LPARAM)m_pFirstPage);
		return true;
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_SPLITTER:
			m_splitterX = urc->dlgNewSize.cx;
			m_splitterY = urc->dlgNewSize.cy - (urc->dlgOriginalSize.cy - urc->rcItem.top);
			return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;

		case IDOK:
		case IDCANCEL:
		case IDC_BACK:
			return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
		}

		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
	}

	void OnDestroy() override
	{
		Utils_SaveWindowPosition(m_hwnd, 0, IMPORT_MODULE, "wiz");
		if (hwndPage)
			DestroyWindow(hwndPage);

		g_hwndWizard = nullptr;
		delete g_pBatch; g_pBatch = nullptr;

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
			SetWindowPos(hwndPage, nullptr, 0, 0, m_splitterX, m_splitterY, SWP_NOZORDER);
			if (bFirstLaunch)
				ShowWindow(m_hwnd, SW_SHOW);
			SetFocus(hwndPage);
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

		INT_PTR res = CDlgBase::DlgProc(uMsg, wParam, lParam);
		if (uMsg == WM_SIZE && hwndPage) {
			SetWindowPos(hwndPage, 0, 0, 0, m_splitterX, m_splitterY, SWP_NOZORDER | SWP_NOACTIVATE);
			SendMessage(hwndPage, WM_SIZE, wParam, lParam);
			InvalidateRect(hwndPage, 0, 0);
		}

		return res;
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
