/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

PSPBaseDlg::PSPBaseDlg(int idDialog) :
	CUserInfoPageDlg(g_plugin, idDialog),
	m_ctrlList(nullptr)
{
	m_bFixedSize = true;
}

bool PSPBaseDlg::OnInitDialog()
{
	m_ctrlList = CCtrlList::CreateObj(m_hwnd);

	SendDlgItemMessage(m_hwnd, IDC_PAGETITLE, WM_SETFONT, (WPARAM)GetBoldFont(), 0);
	return true;
}

bool PSPBaseDlg::OnRefresh()
{
	if (auto *pszProto = GetBaseProto())
		if (m_ctrlList)
			return m_ctrlList->OnInfoChanged(m_hContact, pszProto);

	return false;
}

bool PSPBaseDlg::OnApply()
{
	if (auto *pszProto = GetBaseProto())		
		m_ctrlList->OnApply(m_hContact, pszProto);
	
	return true;
}

void PSPBaseDlg::OnReset()
{
	m_ctrlList->OnReset();
}

void PSPBaseDlg::OnDestroy()
{
	m_ctrlList->Release();
}

// Default dialog procedure, which handles common functions
INT_PTR PSPBaseDlg::DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	// set propertysheet page's background white in aero mode
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORDLG:
		if (IsAeroMode())
			return (INT_PTR)GetStockBrush(WHITE_BRUSH);
		break;

	// Set text color of edit boxes according to the source of information they display.
	case WM_CTLCOLOREDIT:
		if (m_ctrlList)
			return m_ctrlList->OnSetTextColour((HWND)lParam, (HDC)wParam);
		break;

	case WM_COMMAND:
		if (m_ctrlList && !PspIsLocked(m_hwnd))
			m_ctrlList->OnChangedByUser(LOWORD(wParam), HIWORD(wParam));
		break;
	}
	
	return CUserInfoPageDlg::DlgProc(uMsg, wParam, lParam);
}

HFONT PSPBaseDlg::GetBoldFont() const
{
	HFONT res = nullptr;
	return SendMessage(m_hwndParent, PSM_GETBOLDFONT, INDEX_CURPAGE, LPARAM(&res)) ? res : nullptr;
}

MCONTACT PSPBaseDlg::GetContact() const
{
	MCONTACT res = 0;
	return SendMessage(m_hwndParent, PSM_GETCONTACT, INDEX_CURPAGE, LPARAM(&res)) ? res : 0;
}

const char *PSPBaseDlg::GetBaseProto() const
{
	const char *res = "";
	return (SendMessage(m_hwndParent, PSM_GETBASEPROTO, INDEX_CURPAGE, LPARAM(&res)) && *res) ? res : nullptr;
}

void PSPBaseDlg::UpdateCountryIcon(CCtrlCombo &pCombo)
{
	LPIDSTRLIST pd = (LPIDSTRLIST)pCombo.GetCurData();
	if (pd == nullptr)
		return;

	auto *pCtrl = FindControl(ICO_COUNTRY);
	HICON hIcon = LoadFlagIcon(pd->nID);
	HICON hOld = Static_SetIcon(pCtrl->GetHwnd(), hIcon);
	pCtrl->Show(hIcon != 0);
	IcoLib_ReleaseIcon(hOld);
}
