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

class CContactImportDlg : public CDlgBase
{
	friend INT_PTR ImportContact(WPARAM hContact, LPARAM);

	MCONTACT m_hContact;
	int m_flags = 0;
	CImportPattern *m_pPattern = 0;
	wchar_t m_wszFileName[MAX_PATH];

	CCtrlButton m_btnOpenFile;
	CCtrlCombo m_cmbFileType;
	CCtrlEdit edtFileName;

public:
	CContactImportDlg(MCONTACT hContact) :
		CDlgBase(g_plugin, IDD_IMPORT_CONTACT),
		m_hContact(hContact),
		edtFileName(this, IDC_FILENAME),
		m_cmbFileType(this, IDC_FILETYPE),
		m_btnOpenFile(this, IDC_OPEN_FILE)
	{
		m_wszFileName[0] = 0;

		m_btnOpenFile.OnClick = Callback(this, &CContactImportDlg::onClick_OpenFile);
	}

	bool OnInitDialog() override
	{
		CMStringW wszTitle(FORMAT, TranslateT("Import history for %s"), Clist_GetContactDisplayName(m_hContact));
		SetWindowTextW(m_hwnd, wszTitle);

		m_cmbFileType.AddString(TranslateT("Miranda NG database/mContacts"), -1);
		m_cmbFileType.AddString(TranslateT("JSON file"), -2);

		int iType = 1;
		for (auto &it : g_plugin.m_patterns)
			m_cmbFileType.AddString(it->wszName, iType++);

		return true;
	}

	bool OnApply() override
	{
		edtFileName.GetText(m_wszFileName, _countof(m_wszFileName));
		if (m_wszFileName[0] == 0)
			return false;

		if (IsDlgButtonChecked(m_hwnd, IDC_CHECK_DUPS))
			m_flags = IOPT_CHECKDUPS;	
		return true;
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
			m_pPattern = nullptr;
			break;

		case -2:
			text.AppendFormat(L"%s (*.json)%c*.json%c", cmbText.c_str(), 0, 0);
			m_pPattern = nullptr;
			break;

		default:
			auto &p = g_plugin.m_patterns[idx-1];
			text.AppendFormat(L"%s (*.%s)%c*.%s%c", cmbText.c_str(), p.wszExt.c_str(), 0, p.wszExt.c_str(), 0);
			m_pPattern = &p;
			break;
		}
		text.AppendFormat(L"%s (*.*)%c*.*%c%c", TranslateT("All Files"), 0, 0, 0);

		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
		ofn.lpstrFilter = text;
		ofn.lpstrDefExt = L"dat";
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT;
		ofn.lpstrFile = m_wszFileName;
		ofn.nMaxFile = _countof(m_wszFileName);
		if (!GetOpenFileNameW(&ofn)) {
			m_wszFileName[0] = 0;
			m_pPattern = nullptr;
		}
		else edtFileName.SetText(m_wszFileName);
	}
};

INT_PTR ImportContact(WPARAM hContact, LPARAM)
{
	CContactImportDlg dlg(hContact);
	if (!dlg.DoModal())
		return 0;

	g_pBatch = new CImportBatch();
	wcsncpy_s(g_pBatch->m_wszFileName, dlg.m_wszFileName, _TRUNCATE);
	g_pBatch->m_pPattern = dlg.m_pPattern;
	g_pBatch->m_hContact = hContact;
	g_pBatch->m_iOptions = IOPT_HISTORY + dlg.m_flags;
	return RunWizard(new CProgressPageDlg(), true);
}
