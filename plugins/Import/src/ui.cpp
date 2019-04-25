/*

Import plugin for Miranda NG

Copyright (C) 2012-19 Miranda NG team (https://miranda-ng.org)

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

class CContactImportDlg : public CDlgBase
{
	MCONTACT m_hContact;
	int m_flags = 0;

	CCtrlButton m_btnOpenFile, m_btnOk;
	CCtrlCombo m_cmbFileType;
	CCtrlEdit edtFileName;

public:
	CContactImportDlg(MCONTACT hContact) :
		CDlgBase(g_plugin, IDD_IMPORT_CONTACT),
		m_hContact(hContact),
		edtFileName(this, IDC_FILENAME),
		m_cmbFileType(this, IDC_FILETYPE),
		m_btnOk(this, IDOK),
		m_btnOpenFile(this, IDC_OPEN_FILE)
	{
		m_btnOk.OnClick = Callback(this, &CContactImportDlg::onClick_Ok);
		m_btnOpenFile.OnClick = Callback(this, &CContactImportDlg::onClick_OpenFile);
	}

	int getFlags() const { return m_flags; }

	bool OnInitDialog() override
	{
		CMStringW wszTitle(FORMAT, TranslateT("Import history for %s"), Clist_GetContactDisplayName(m_hContact));
		SetWindowTextW(m_hwnd, wszTitle);

		m_cmbFileType.AddString(TranslateT("Miranda NG database"), -1);
		m_cmbFileType.AddString(TranslateT("JSON file"), -2);

		int iType = 1;
		for (auto &it : g_plugin.m_patterns)
			m_cmbFileType.AddString(it->wszName, iType++);

		return true;
	}

	bool OnApply() override
	{
		edtFileName.GetText(g_wszImportFile, _countof(g_wszImportFile));
		if (g_wszImportFile[0] == 0)
			return false;

		if (IsDlgButtonChecked(m_hwnd, IDC_CHECK_DUPS))
			m_flags = IOPT_CHECKDUPS;	
		return true;
	}

	void onClick_Ok(CCtrlButton*)
	{
		EndModal(1);
	}

	void onClick_OpenFile(CCtrlButton*)
	{
		int iCur = m_cmbFileType.GetCurSel();
		if (iCur == -1)
			return;

		CMStringW text, cmbText(ptrW(m_cmbFileType.GetText()));
		switch(int idx = m_cmbFileType.GetItemData(iCur)) {
		case -1:
			text.AppendFormat(L"%s (*.dat,*.bak)%c*.dat;*.bak%c", cmbText.c_str(), 0, 0);
			g_pActivePattern = nullptr;
			break;

		case -2:
			text.AppendFormat(L"%s (*.json)%c*.json%c", cmbText.c_str(), 0, 0);
			g_pActivePattern = nullptr;
			break;

		default:
			auto &p = g_plugin.m_patterns[idx-1];
			text.AppendFormat(L"%s (*.%s)%c*.%s%c", cmbText.c_str(), p.wszExt.c_str(), 0, p.wszExt.c_str(), 0);
			g_pActivePattern = &p;
			break;
		}
		text.AppendFormat(L"%s (*.*)%c*.*%c%c", TranslateT("All Files"), 0, 0, 0);

		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
		ofn.lpstrFilter = text;
		ofn.lpstrDefExt = L"dat";
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT;
		ofn.lpstrFile = g_wszImportFile;
		ofn.nMaxFile = _countof(g_wszImportFile);
		if (!GetOpenFileNameW(&ofn)) {
			g_wszImportFile[0] = 0;
			g_pActivePattern = nullptr;
		}
		else edtFileName.SetText(g_wszImportFile);
	}
};

INT_PTR ImportContact(WPARAM hContact, LPARAM)
{
	CContactImportDlg dlg(hContact);
	if (!dlg.DoModal())
		return 0;

	g_hImportContact = hContact;
	g_iImportOptions = IOPT_HISTORY + dlg.getFlags();
	return RunWizard(new CProgressPageDlg(), true);
}
