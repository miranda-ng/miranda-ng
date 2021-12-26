/*
Miranda Database Tool
Copyright 2000-2011 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#define WZM_GETOPTS (WM_USER+1)
#define WZM_GOTOPAGE (WM_USER+2)

HFONT hBoldFont = nullptr;

static BOOL CALLBACK MyControlsEnumChildren(HWND hwnd, LPARAM)
{
	uint32_t style = GetWindowLongPtr(hwnd, GWL_STYLE);
	uint32_t exstyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	char szClass[64];
	int makeBold = 0;

	GetClassNameA(hwnd, szClass, sizeof(szClass));
	if (!mir_strcmp(szClass, "Static")) {
		if (exstyle & WS_EX_CLIENTEDGE) {
			switch (style & SS_TYPEMASK) {
			case SS_LEFT: case SS_CENTER: case SS_RIGHT:
				makeBold = 1;
			}
		}
	}
	else if (!mir_strcmp(szClass, "Button")) {
		if (exstyle & WS_EX_CLIENTEDGE)
			makeBold = 1;
	}
	if (makeBold) {
		if (hBoldFont == nullptr) {
			LOGFONT lf;
			hBoldFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
			GetObject(hBoldFont, sizeof(lf), &lf);
			lf.lfWeight = FW_BOLD;
			hBoldFont = CreateFontIndirect(&lf);
		}
		SendMessage(hwnd, WM_SETFONT, (WPARAM)hBoldFont, 0);
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, exstyle&~WS_EX_CLIENTEDGE);
		SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Basic wizard dialog class

CWizardPageDlg::CWizardPageDlg(int iDlgId) :
	CSuper(g_plugin, iDlgId),
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

bool CWizardPageDlg::OnInitDialog()
{
	EnumChildWindows(m_hwnd, MyControlsEnumChildren, 0);
	return true;
}

void CWizardPageDlg::changePage(CWizardPageDlg *pNewPage)
{
	PostMessage(m_hwndParent, WZM_GOTOPAGE, 0, (LPARAM)pNewPage);
}

DbToolOptions* CWizardPageDlg::getOpts() const
{
	return (DbToolOptions *)SendMessage(m_hwndParent, WZM_GETOPTS, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Wizard dialog class

CWizardDlg::CWizardDlg(DbToolOptions *opts) :
	CDlgBase(g_plugin, IDD_WIZARD),
	m_opts(opts),
	btnCancel(this, IDCANCEL),
	timerStart(this, 1)
{
	m_autoClose = CLOSE_ON_OK;
	SetMinSize(450, 300);

	btnCancel.OnClick = Callback(this, &CWizardDlg::onClick_Cancel);

	timerStart.OnEvent = Callback(this, &CWizardDlg::onTimer);
}

bool CWizardDlg::OnInitDialog()
{
	Utils_RestoreWindowPosition(m_hwnd, 0, MODULENAME, "Wizard_");
	Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_DBTOOL));
	timerStart.Start(100);
	return true;
}

bool CWizardDlg::OnApply() 
{
	SendMessage(hwndPage, WZN_PAGECHANGING, 0, 0);
	SendMessage(hwndPage, WM_COMMAND, IDOK, 0);
	return false;
}

void CWizardDlg::OnDestroy() 
{
	Utils_SaveWindowPosition(m_hwnd, 0, MODULENAME, "Wizard_");

	if (m_opts->dbChecker) {
		m_opts->dbChecker->Destroy();
		m_opts->dbChecker = nullptr;
	}
	delete m_opts;

	if (hwndPage)
		DestroyWindow(hwndPage);

	if (hBoldFont != nullptr) {
		DeleteObject(hBoldFont);
		hBoldFont = nullptr;
	}
}

INT_PTR CWizardDlg::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WZM_GETOPTS:
		SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, (LPARAM)m_opts);
		return true;

	case WZM_GOTOPAGE:
		ChangePage((CWizardPageDlg *)lParam);
		return FALSE;
	}

	INT_PTR res = CDlgBase::DlgProc(msg, wParam, lParam);
	if (msg == WM_SIZE && hwndPage) {
		SetWindowPos(hwndPage, 0, 0, 0, m_splitterX, m_splitterY, SWP_NOZORDER | SWP_NOACTIVATE);
		SendMessage(hwndPage, WM_SIZE, wParam, lParam);
		InvalidateRect(hwndPage, 0, 0);
	}

	return res;
}

int CWizardDlg::Resizer(UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_SPLITTER:
		m_splitterX = urc->dlgNewSize.cx;
		m_splitterY = urc->dlgNewSize.cy - (urc->dlgOriginalSize.cy - urc->rcItem.top);
		return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;

	case IDOK:
	case IDCANCEL:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
	}

	return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
}

void CWizardDlg::onClick_Cancel(CCtrlButton *)
{
	SendMessage(hwndPage, WZN_CANCELCLICKED, 0, 0);
	EndModal(0);
}

void CWizardDlg::onTimer(CTimer *pTimer)
{
	pTimer->Stop();

	ChangePage(new COptionsPageDlg());
}

LRESULT CWizardDlg::ChangePage(CWizardPageDlg *pPage)
{
	if (hwndPage != nullptr)
		DestroyWindow(hwndPage);

	EnableWindow(GetDlgItem(m_hwnd, IDOK), TRUE);
	EnableWindow(GetDlgItem(m_hwnd, IDCANCEL), TRUE);
	SetDlgItemText(m_hwnd, IDCANCEL, TranslateT("Cancel"));
	{
		pPage->SetParent(m_hwnd);
		pPage->Show();
		hwndPage = pPage->GetHwnd();
	}
	SetWindowPos(hwndPage, nullptr, 0, 0, m_splitterX, m_splitterY, SWP_NOZORDER);

	ShowWindow(hwndPage, SW_SHOW);
	return 0;
}
