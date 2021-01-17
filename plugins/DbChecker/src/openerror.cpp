/*
Miranda Database Tool
Copyright (C) 2001-2005  Richard Hughes

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

COpenErrorDlg::COpenErrorDlg() :
	CSuper(IDD_OPENERROR)
{
}

bool COpenErrorDlg::OnInitDialog()
{
	CSuper::OnInitDialog();

	auto *opts = getOpts();

	wchar_t szError[256];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, opts->error, 0, szError, _countof(szError), nullptr);
	SetDlgItemText(m_hwnd, IDC_ERRORTEXT, szError);

	if (opts->error == ERROR_SHARING_VIOLATION)
		ShowWindow(GetDlgItem(m_hwnd, IDC_INUSE), SW_SHOW);
	SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_FILE), GWL_STYLE, GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_FILE), GWL_STYLE) | SS_PATHELLIPSIS);
	SetDlgItemText(m_hwnd, IDC_FILE, opts->filename);
	return true;
}

int COpenErrorDlg::Resizer(UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_SPLITTER:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;
	}

	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

void COpenErrorDlg::OnNext()
{
	changePage(new COptionsPageDlg());
}
