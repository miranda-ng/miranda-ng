/*
Scriver

Copyright (c) 2000-09 Miranda ICQ/IM project,

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

CErrorDlg::CErrorDlg(const wchar_t *pwszDescr, HWND hWnd, MessageSendQueueItem *pItem)
	: CDlgBase(g_hInst, IDD_MSGSENDERROR),
	m_wszText(mir_utf8decodeW(pItem->sendBuffer)),
	m_wszDescr(pwszDescr != nullptr ? pwszDescr : TranslateT("An unknown error has occurred.")),
	m_hwndParent(hWnd),
	m_queueItem(pItem),

	m_btnOk(this, IDOK),
	m_btnCancel(this, IDCANCEL)
{
	const wchar_t *pwszName = pcli->pfnGetContactDisplayName(pItem->hContact, 0);
	if (pwszName)
		m_wszName.Format(L"%s - %s", TranslateT("Send error"), pwszName);
	else
		m_wszName = TranslateT("Send error");

	m_btnOk.OnClick = Callback(this, &CErrorDlg::onOk);
	m_btnCancel.OnClick = Callback(this, &CErrorDlg::onCancel);
}

void CErrorDlg::OnInitDialog()
{
	ShowWindow(GetParent(m_hwndParent), SW_RESTORE);
	
	SetDlgItemText(m_hwnd, IDC_ERRORTEXT, m_wszDescr);
	SetWindowText(m_hwnd, m_wszName);

	SETTEXTEX st = { 0 };
	st.flags = ST_DEFAULT;
	st.codepage = 1200;
	SendDlgItemMessage(m_hwnd, IDC_MSGTEXT, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)m_wszText.get());

	RECT rc, rcParent;
	GetWindowRect(m_hwnd, &rc);
	GetWindowRect(GetParent(m_hwndParent), &rcParent);
	SetWindowPos(m_hwnd, HWND_TOP, rcParent.left + (rcParent.right - rcParent.left - rc.right + rc.left) / 2, rcParent.top + (rcParent.bottom - rcParent.top - rc.bottom + rc.top) / 2, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
}

void CErrorDlg::onOk(CCtrlButton*)
{
	SendMessage(m_hwndParent, DM_ERRORDECIDED, MSGERROR_RETRY, (LPARAM)m_queueItem);
}

void CErrorDlg::onCancel(CCtrlButton*)
{
	SendMessage(m_hwndParent, DM_ERRORDECIDED, MSGERROR_CANCEL, (LPARAM)m_queueItem);
}
