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

static volatile long g_order = 1;

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlPages

struct CCtrlPages::TPageInfo : public MZeroedObject
{
	TPageInfo()
	{
		m_iOrder = InterlockedIncrement(&g_order);
	}

	~TPageInfo()
	{
		if (m_hIcon)
			DestroyIcon(m_hIcon);
	}

	int m_iOrder;
	ptrW m_ptszHeader;
	HICON m_hIcon;
	bool m_bChanged, m_bScheduledResize;
	CDlgBase *m_pDlg;
};

CCtrlPages::CCtrlPages(CDlgBase *dlg, int ctrlId)
	: CCtrlBase(dlg, ctrlId),
	m_hIml(nullptr),
	m_pActivePage(nullptr),
	m_pages(4, NumericKeySortT)
{}

void CCtrlPages::OnInit()
{
	CSuper::OnInit();
	Subclass();

	for (auto &it : m_pages)
		InsertPage(it);
	m_pages.destroy();

	::SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, ::GetWindowLongPtr(m_hwnd, GWL_EXSTYLE) | WS_EX_CONTROLPARENT);

	TPageInfo *info = GetCurrPage();
	if (info) {
		m_pActivePage = info->m_pDlg;
		ShowPage(m_pActivePage);

		PSHNOTIFY pshn;
		pshn.hdr.code = PSN_INFOCHANGED;
		pshn.hdr.hwndFrom = m_pActivePage->GetHwnd();
		pshn.hdr.idFrom = 0;
		pshn.lParam = 0;
		SendMessage(pshn.hdr.hwndFrom, WM_NOTIFY, 0, (LPARAM)&pshn);
	}
}

LRESULT CCtrlPages::CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	int tabCount;

	switch (msg) {
	case WM_SIZE:
		if (TPageInfo *pCurrInfo = GetCurrPage()) {
			tabCount = GetCount();
			for (int i = 0; i < tabCount; i++) {
				TPageInfo *p = GetItemPage(i);
				if (p == nullptr)
					continue;
				if (p == pCurrInfo) {
					RECT rc;
					GetClientRect(m_hwnd, &rc);
					TabCtrl_AdjustRect(m_hwnd, FALSE, &rc);
					SetWindowPos(p->m_pDlg->GetHwnd(), nullptr, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_NOZORDER);
				}
				else p->m_bScheduledResize = true;
			}
		}
		break;

	case PSM_CHANGED:
		if (TPageInfo *info = GetCurrPage())
			info->m_bChanged = TRUE;
		return TRUE;

	case PSM_FORCECHANGED:
		tabCount = GetCount();

		PSHNOTIFY pshn;
		pshn.hdr.code = PSN_INFOCHANGED;
		pshn.hdr.idFrom = 0;
		pshn.lParam = 0;
		for (int i = 0; i < tabCount; i++) {
			TPageInfo *p = GetItemPage(i);
			if (p) {
				pshn.hdr.hwndFrom = p->m_pDlg->GetHwnd();
				if (pshn.hdr.hwndFrom != nullptr)
					SendMessage(pshn.hdr.hwndFrom, WM_NOTIFY, 0, (LPARAM)&pshn);
			}
		}
		break;
	}

	return CSuper::CustomWndProc(msg, wParam, lParam);
}

void CCtrlPages::AddPage(const wchar_t *ptszName, HICON hIcon, CDlgBase *pDlg)
{
	TPageInfo *info = new TPageInfo;
	info->m_pDlg = pDlg;
	info->m_hIcon = hIcon;
	info->m_ptszHeader = mir_wstrdup(ptszName);

	if (m_hwnd != nullptr) {
		InsertPage(info);

		if (GetCount() == 1) {
			m_pActivePage = info->m_pDlg;
			ShowPage(m_pActivePage);
		}
	}
	m_pages.insert(info);
}

void CCtrlPages::ActivatePage(int iPage)
{
	TPageInfo *info = GetItemPage(iPage);
	if (info == nullptr || info->m_pDlg == nullptr)
		return;

	if (m_pActivePage != nullptr)
		ShowWindow(m_pActivePage->GetHwnd(), SW_HIDE);

	m_pActivePage = info->m_pDlg;
	if (m_pActivePage->GetHwnd() && info->m_bScheduledResize) {
		RECT rc;
		GetClientRect(m_hwnd, &rc);
		TabCtrl_AdjustRect(m_hwnd, FALSE, &rc);
		SetWindowPos(m_pActivePage->GetHwnd(), nullptr, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_NOZORDER);
	}

	TabCtrl_SetCurSel(m_hwnd, iPage);
	ShowPage(m_pActivePage);
	::SendMessage(m_pActivePage->GetHwnd(), WM_MOUSEACTIVATE, 0, 0);
}

void CCtrlPages::CheckRowCount()
{
	int iRowCount = TabCtrl_GetRowCount(m_hwnd);
	if (m_numRows != iRowCount) {
		m_numRows = iRowCount;
		for (auto &p : m_pages)
			p->m_bScheduledResize = true;
	}
}

int CCtrlPages::GetCount()
{
	return TabCtrl_GetItemCount(m_hwnd);
}

CDlgBase* CCtrlPages::GetNthPage(int iPage)
{
	TPageInfo *info = GetItemPage(iPage);
	return (info == nullptr) ? nullptr : info->m_pDlg;
}

CCtrlPages::TPageInfo* CCtrlPages::GetCurrPage()
{
	TCITEM tci = { 0 };
	tci.mask = TCIF_PARAM;
	if (!TabCtrl_GetItem(m_hwnd, TabCtrl_GetCurSel(m_hwnd), &tci))
		return nullptr;

	return (TPageInfo*)tci.lParam;
}

CCtrlPages::TPageInfo* CCtrlPages::GetItemPage(int iPage)
{
	TCITEM tci = { 0 };
	tci.mask = TCIF_PARAM;
	if (!TabCtrl_GetItem(m_hwnd, iPage, &tci))
		return nullptr;

	return (TPageInfo*)tci.lParam;
}

int CCtrlPages::GetDlgIndex(CDlgBase *pDlg)
{
	int tabCount = TabCtrl_GetItemCount(m_hwnd);
	for (int i = 0; i < tabCount; i++) {
		TCITEM tci;
		tci.mask = TCIF_PARAM | TCIF_IMAGE;
		TabCtrl_GetItem(m_hwnd, i, &tci);
		TPageInfo *pPage = (TPageInfo *)tci.lParam;
		if (pPage == nullptr)
			continue;

		if (pPage->m_pDlg == pDlg)
			return i;
	}

	return -1;
}

void CCtrlPages::InsertPage(TPageInfo *pPage)
{
	TCITEM tci = { 0 };
	tci.mask = TCIF_PARAM | TCIF_TEXT;
	tci.lParam = (LPARAM)pPage;
	tci.pszText = TranslateW_LP(pPage->m_ptszHeader);
	if (pPage->m_hIcon) {
		if (!m_hIml) {
			m_hIml = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 1);
			TabCtrl_SetImageList(m_hwnd, m_hIml);
		}

		tci.mask |= TCIF_IMAGE;
		tci.iImage = ImageList_AddIcon(m_hIml, pPage->m_hIcon);
	}

	TabCtrl_InsertItem(m_hwnd, TabCtrl_GetItemCount(m_hwnd), &tci);

	CheckRowCount();
}

void CCtrlPages::RemovePage(int iPage)
{
	TPageInfo *p = GetItemPage(iPage);
	if (p == nullptr)
		return;

	TabCtrl_DeleteItem(m_hwnd, iPage);
	m_pages.remove(p);
	delete p;

	CheckRowCount();
}

void CCtrlPages::ShowPage(CDlgBase *pDlg)
{
	if (pDlg->GetHwnd() == nullptr) {
		pDlg->SetParent(m_hwnd);
		pDlg->Create();

		RECT rc;
		GetClientRect(m_hwnd, &rc);
		TabCtrl_AdjustRect(m_hwnd, FALSE, &rc);
		SetWindowPos(pDlg->GetHwnd(), HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE);

		EnableThemeDialogTexture(pDlg->GetHwnd(), ETDT_ENABLETAB);

		PSHNOTIFY pshn;
		pshn.hdr.code = PSN_INFOCHANGED;
		pshn.hdr.hwndFrom = pDlg->GetHwnd();
		pshn.hdr.idFrom = 0;
		pshn.lParam = 0;
		SendMessage(pshn.hdr.hwndFrom, WM_NOTIFY, 0, (LPARAM)&pshn);
	}
	ShowWindow(pDlg->GetHwnd(), SW_SHOW);
}

void CCtrlPages::SwapPages(int idx1, int idx2)
{
	TPageInfo *p1 = GetItemPage(idx1), *p2 = GetItemPage(idx2);
	if (p1 == nullptr || p2 == nullptr)
		return;

	TabCtrl_DeleteItem(m_hwnd, idx1);

	TCITEM tci = { 0 };
	tci.mask = TCIF_PARAM | TCIF_TEXT;
	tci.lParam = (LPARAM)p1;
	tci.pszText = TranslateW_LP(p1->m_ptszHeader);
	TabCtrl_InsertItem(m_hwnd, idx2, &tci);
}

BOOL CCtrlPages::OnNotify(int /*idCtrl*/, NMHDR *pnmh)
{
	TPageInfo *info;
	PSHNOTIFY pshn;

	switch (pnmh->code) {
	case TCN_SELCHANGING:
		if (info = GetCurrPage()) {
			pshn.hdr.code = PSN_KILLACTIVE;
			pshn.hdr.hwndFrom = info->m_pDlg->GetHwnd();
			pshn.hdr.idFrom = 0;
			pshn.lParam = 0;
			if (SendMessage(pshn.hdr.hwndFrom, WM_NOTIFY, 0, (LPARAM)&pshn)) {
				SetWindowLongPtr(GetParent()->GetHwnd(), DWLP_MSGRESULT, TRUE);
				return TRUE;
			}
		}
		return TRUE;

	case TCN_SELCHANGE:
		if (m_pActivePage != nullptr)
			m_pActivePage->Hide();

		if (info = GetCurrPage()) {
			m_pActivePage = info->m_pDlg;
			ShowPage(m_pActivePage);
		}
		else m_pActivePage = nullptr;
		return TRUE;
	}

	return FALSE;
}

void CCtrlPages::OnReset()
{
	CSuper::OnReset();

	PSHNOTIFY pshn;
	pshn.hdr.code = PSN_INFOCHANGED;
	pshn.hdr.idFrom = 0;
	pshn.lParam = 0;

	int tabCount = GetCount();
	for (int i = 0; i < tabCount; i++) {
		TPageInfo *p = GetItemPage(i);
		if (p->m_pDlg->GetHwnd() == nullptr || !p->m_bChanged)
			continue;

		pshn.hdr.hwndFrom = p->m_pDlg->GetHwnd();
		SendMessage(pshn.hdr.hwndFrom, WM_NOTIFY, 0, (LPARAM)&pshn);
	}
}

bool CCtrlPages::OnApply()
{
	PSHNOTIFY pshn;
	pshn.hdr.idFrom = 0;
	pshn.lParam = 0;

	if (m_pActivePage != nullptr) {
		pshn.hdr.code = PSN_KILLACTIVE;
		pshn.hdr.hwndFrom = m_pActivePage->GetHwnd();
		if (SendMessage(pshn.hdr.hwndFrom, WM_NOTIFY, 0, (LPARAM)&pshn))
			return false;
	}

	pshn.hdr.code = PSN_APPLY;
	int tabCount = GetCount();
	for (int i = 0; i < tabCount; i++) {
		TPageInfo *p = GetItemPage(i);
		if (p->m_pDlg->GetHwnd() == nullptr || !p->m_bChanged)
			continue;

		pshn.hdr.hwndFrom = p->m_pDlg->GetHwnd();
		SendMessage(pshn.hdr.hwndFrom, WM_NOTIFY, 0, (LPARAM)&pshn);
		if (GetWindowLongPtr(pshn.hdr.hwndFrom, DWLP_MSGRESULT) == PSNRET_INVALID_NOCHANGEPAGE) {
			TabCtrl_SetCurSel(m_hwnd, i);
			if (m_pActivePage != nullptr)
				m_pActivePage->Hide();
			m_pActivePage = p->m_pDlg;
			m_pActivePage->Show();
			return false;
		}
	}
	
	CSuper::OnApply();
	return true;
}

void CCtrlPages::OnDestroy()
{
	int tabCount = GetCount();
	for (int i = 0; i < tabCount; i++) {
		TPageInfo *p = GetItemPage(i);
		CDlgBase *pDlg = p->m_pDlg; p->m_pDlg = nullptr;
		if (pDlg->GetHwnd())
			pDlg->Close();
		delete p;
	}			

	TabCtrl_DeleteAllItems(m_hwnd);

	if (m_hIml) {
		TabCtrl_SetImageList(m_hwnd, nullptr);
		ImageList_Destroy(m_hIml);
	}

	CSuper::OnDestroy();
}
