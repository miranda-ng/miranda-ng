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

CContactImportDlg::CContactImportDlg(MCONTACT hContact) :
	CDlgBase(g_plugin, IDD_IMPORT_CONTACT),
	m_hContact(hContact),
	edtFileName(this, IDC_FILENAME),
	m_btnOk(this, IDOK),
	m_btnOpenFile(this, IDC_OPEN_FILE)
{
	m_btnOk.OnClick = Callback(this, &CContactImportDlg::onClick_Ok);
	m_btnOpenFile.OnClick = Callback(this, &CContactImportDlg::onClick_OpenFile);
}

bool CContactImportDlg::OnInitDialog()
{
	CMStringW wszTitle(FORMAT, TranslateT("Import history for %s"), Clist_GetContactDisplayName(m_hContact));
	SetWindowTextW(m_hwnd, wszTitle);
	return true;
}

bool CContactImportDlg::OnApply()
{
	edtFileName.GetText(importFile, _countof(importFile));
	if (importFile[0] == 0)
		return false;

	if (IsDlgButtonChecked(m_hwnd, IDC_CHECK_DUPS))
		m_flags = IOPT_CHECKDUPS;	
	return true;
}

void CContactImportDlg::onClick_Ok(CCtrlButton*)
{
	EndModal(1);
}

void CContactImportDlg::onClick_OpenFile(CCtrlButton*)
{
	CMStringW text;
	text.AppendFormat(L"%s (*.dat,*.bak)%c*.dat;*.bak%c", TranslateT("Miranda NG database"), 0, 0);
	text.AppendFormat(L"%s (*.json)%c*.json%c", TranslateT("JSON file"), 0, 0);
	text.AppendFormat(L"%s (*.*)%c*.*%c%c", TranslateT("All Files"), 0, 0, 0);

	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.lpstrFilter = text;
	ofn.lpstrDefExt = L"dat";
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT;
	ofn.lpstrFile = importFile;
	ofn.nMaxFile = _countof(importFile);
	if (!GetOpenFileName(&ofn))
		importFile[0] = 0;
	else
		edtFileName.SetText(importFile);
}
