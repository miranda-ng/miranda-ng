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

//=======================================================================================
// Profile selection dialog

CMirandaPageDlg::CMirandaPageDlg() :
	CWizardPageDlg(IDD_MIRANDADB),
	m_list(this, IDC_LIST),
	btnBack(this, IDC_BACK),
	btnPath(this, IDC_DIRECTORY),
	btnOther(this, IDC_OTHER),
	m_cmbFileType(this, IDC_FILETYPE)
{
	btnBack.OnClick = Callback(this, &CMirandaPageDlg::onClick_Back);
	btnPath.OnClick = Callback(this, &CMirandaPageDlg::onClick_Path);
	btnOther.OnClick = Callback(this, &CMirandaPageDlg::onClick_Other);

	m_cmbFileType.OnChange = Callback(this, &CMirandaPageDlg::onChange_Pattern);
}

bool CMirandaPageDlg::OnInitDialog()
{
	m_cmbFileType.AddString(TranslateT("Miranda NG database"), -1);

	int iType = 1;
	for (auto& it : g_plugin.m_patterns)
		m_cmbFileType.AddString(it->wszName, iType++);
	m_cmbFileType.SetCurSel(0);

	btnPath.Hide();
	m_list.Disable();
	SendMessage(m_hwndParent, WIZM_DISABLEBUTTON, 1, 0);
	return true;
}

int CMirandaPageDlg::Resizer(UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_STATICTEXT1:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;

	case IDC_STATICTEXT2:
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;

	case IDC_STATICTEXT3:
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;

	case IDC_FILENAME:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;

	case IDC_FILETYPE:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;

	case IDC_LIST:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
	}
	return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
}

void CMirandaPageDlg::OnDestroy()
{
	for (int i = SendDlgItemMessage(m_hwnd, IDC_LIST, LB_GETCOUNT, 0, 0) - 1; i >= 0; i--)
		mir_free((char*)SendDlgItemMessage(m_hwnd, IDC_LIST, LB_GETITEMDATA, i, 0));
}

void CMirandaPageDlg::OnNext()
{
	wchar_t filename[MAX_PATH];
	GetDlgItemText(m_hwnd, IDC_FILENAME, filename, _countof(filename));
	if (_waccess(filename, 4)) {
		MessageBox(m_hwnd, TranslateT("The given file does not exist. Please check that you have entered the name correctly."), TranslateT("Miranda Import"), MB_OK);
		return;
	}
	mir_wstrcpy(g_pBatch->m_wszFileName, filename);
	PostMessage(m_hwndParent, WIZM_GOTOPAGE, 0, (LPARAM)new CMirandaOptionsPageDlg());
}

void CMirandaPageDlg::onClick_Back(CCtrlButton*)
{
	PostMessage(m_hwndParent, WIZM_GOTOPAGE, 0, (LPARAM)new CIntroPageDlg());
}

void CMirandaPageDlg::onClick_Other(CCtrlButton*)
{
	ptrW pfd;
	wchar_t str[MAX_PATH], ext[100], text[256];
	GetDlgItemText(m_hwnd, IDC_FILENAME, str, _countof(str));

	const wchar_t *pwszName;
	if (m_iFileType == -1) {
		wcsncpy_s(ext, L"*.dat;*.bak", _TRUNCATE);
		pfd = Utils_ReplaceVarsW(L"%miranda_profilesdir%");
		pwszName = TranslateT("Miranda NG database");
	}
	else {
		mir_snwprintf(ext, L"*.%s", g_plugin.m_patterns[m_iFileType - 1].wszExt.c_str());
		pwszName = g_pBatch->m_pPattern->wszName;
	}

	mir_snwprintf(text, L"%s (%s)%c%s%c%s (*.*)%c*.*%c%c", pwszName, ext, 0, ext, 0, TranslateT("All Files"), 0, 0, 0);

	OPENFILENAME ofn = {};
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.hwndOwner = m_hwnd;
	ofn.lpstrFilter = text;
	ofn.lpstrDefExt = L"dat";
	ofn.lpstrFile = str;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT;
	ofn.nMaxFile = _countof(str);
	ofn.lpstrInitialDir = pfd;
	if (GetOpenFileName(&ofn)) {
		if (!mir_wstrcmpi(str, VARSW(L"%miranda_userdata%\\%miranda_profilename%.dat"))) {
			MessageBoxW(nullptr, TranslateT("You cannot import your current profile into itself"), L"Miranda NG", MB_OK | MB_ICONERROR);
			return;
		}

		SetDlgItemText(m_hwnd, IDC_FILENAME, str);
		m_list.SetCurSel(-1);
		SendMessage(m_hwndParent, WIZM_ENABLEBUTTON, 1, 0);
	}
}

void CMirandaPageDlg::onClick_Path(CCtrlButton*)
{
	wchar_t str[MAX_PATH];

	BROWSEINFOW br = {};
	br.hwndOwner = m_hwnd;
	br.pszDisplayName = str;
	br.lpszTitle = TranslateT("Importing whole directory");
	br.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	PIDLIST_ABSOLUTE pList = SHBrowseForFolderW(&br);
	if (pList == nullptr)
		return;

	SHGetPathFromIDListW(pList, str);
	CoTaskMemFree(pList);

	SetDlgItemText(m_hwnd, IDC_FILENAME, str);

	// find appropriate files and list them
	wchar_t searchspec[MAX_PATH];
	mir_snwprintf(searchspec, L"%s\\*.%s", str, g_pBatch->m_pPattern->wszExt.c_str());
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(searchspec, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// find all subfolders except "." and ".."
			if (!mir_wstrcmp(fd.cFileName, L".") || !mir_wstrcmp(fd.cFileName, L".."))
				continue;

			m_list.AddString(fd.cFileName);
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
		SendMessage(m_hwndParent, WIZM_ENABLEBUTTON, 1, 0);
	}
}

void CMirandaPageDlg::onChange_Pattern(CCtrlCombo*)
{
	// standard import for Miranda
	m_iFileType = m_cmbFileType.GetCurData();
	if (m_iFileType == -1) {
		g_pBatch->m_pPattern = nullptr;
		btnPath.Hide();
	}
	// custom pattern import
	else {
		g_pBatch->m_pPattern = &g_plugin.m_patterns[m_iFileType-1];
		btnPath.Show();
	}
}

//=======================================================================================
// Import options dialog

CMirandaOptionsPageDlg::CMirandaOptionsPageDlg() :
	CWizardPageDlg(IDD_OPTIONS),
	btnBack(this, IDC_BACK),
	chkDups(this, IDC_CHECK_DUPS)
{
	btnBack.OnClick = Callback(this, &CMirandaOptionsPageDlg::onClick_Back);
}

bool CMirandaOptionsPageDlg::OnInitDialog()
{
	EnableWindow(GetDlgItem(m_hwnd, IDC_RADIO_COMPLETE), TRUE);
	EnableWindow(GetDlgItem(m_hwnd, IDC_RADIO_ALL), TRUE);
	EnableWindow(GetDlgItem(m_hwnd, IDC_STATIC_ALL), TRUE);
	EnableWindow(GetDlgItem(m_hwnd, IDC_RADIO_CONTACTS), TRUE);
	EnableWindow(GetDlgItem(m_hwnd, IDC_STATIC_CONTACTS), TRUE);
	EnableWindow(GetDlgItem(m_hwnd, IDC_RADIO_CUSTOM), TRUE);
	EnableWindow(GetDlgItem(m_hwnd, IDC_STATIC_CUSTOM), TRUE);
	CheckDlgButton(m_hwnd, IDC_RADIO_ALL, BST_CHECKED);
	return true;
}

int CMirandaOptionsPageDlg::Resizer(UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_STATIC_ALL:
	case IDC_STATIC_CONTACTS:
	case IDC_STATIC_CUSTOM:
	case IDC_STATICTEXT1:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;
	}

	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

void CMirandaOptionsPageDlg::onClick_Back(CCtrlButton*)
{
	PostMessage(m_hwndParent, WIZM_GOTOPAGE, 0, (LPARAM)new CMirandaPageDlg());
}

void CMirandaOptionsPageDlg::OnNext()
{
	int iFlags = chkDups.IsChecked() ? IOPT_CHECKDUPS : 0;

	if (IsDlgButtonChecked(m_hwnd, IDC_RADIO_COMPLETE)) {
		g_pBatch->m_iOptions = IOPT_ADDUNKNOWN | IOPT_COMPLETE | iFlags;
		PostMessage(m_hwndParent, WIZM_GOTOPAGE, IDD_PROGRESS, (LPARAM)new CProgressPageDlg());
	}
	else if (IsDlgButtonChecked(m_hwnd, IDC_RADIO_ALL)) {
		g_pBatch->m_iOptions = IOPT_HISTORY | IOPT_SYSTEM | IOPT_GROUPS | IOPT_CONTACTS | iFlags;
		PostMessage(m_hwndParent, WIZM_GOTOPAGE, IDD_PROGRESS, (LPARAM)new CProgressPageDlg());
	}
	else if (IsDlgButtonChecked(m_hwnd, IDC_RADIO_CONTACTS)) {
		g_pBatch->m_iOptions = IOPT_CONTACTS;
		PostMessage(m_hwndParent, WIZM_GOTOPAGE, IDD_PROGRESS, (LPARAM)new CProgressPageDlg());
	}
	else if (IsDlgButtonChecked(m_hwnd, IDC_RADIO_CUSTOM)) {
		g_pBatch->m_iOptions = iFlags;
		PostMessage(m_hwndParent, WIZM_GOTOPAGE, IDD_ADVOPTIONS, (LPARAM)new CMirandaAdvOptionsPageDlg());
	}
}

//=======================================================================================
// Advanced options dialog

static const UINT InControls[] = { IDC_IN_MSG, IDC_IN_FT, IDC_IN_OTHER };
static const UINT OutControls[] = { IDC_OUT_MSG, IDC_OUT_FT, IDC_OUT_OTHER };
static const UINT SysControls[] = { IDC_CONTACTS, IDC_SYSTEM };

CMirandaAdvOptionsPageDlg::CMirandaAdvOptionsPageDlg() :
	CWizardPageDlg(IDD_ADVOPTIONS),
	btnBack(this, IDC_BACK),
	chkFT(this, IDC_FT),
	chkAll(this, IDC_ALL),
	chkMsg(this, IDC_MSG),
	chkOther(this, IDC_OTHER),
	chkSince(this, IDC_SINCE),
	chkOutgoing(this, IDC_OUTGOING),
	chkIncoming(this, IDC_INCOMING)
{
	btnBack.OnClick = Callback(this, &CMirandaAdvOptionsPageDlg::onClick_Back);

	chkSince.OnChange = Callback(this, &CMirandaAdvOptionsPageDlg::onChange_Since);
	chkAll.OnChange = chkOutgoing.OnChange = chkIncoming.OnChange = Callback(this, &CMirandaAdvOptionsPageDlg::onChange_All);
	chkMsg.OnChange = Callback(this, &CMirandaAdvOptionsPageDlg::onChange_Msg);
	chkFT.OnChange = Callback(this, &CMirandaAdvOptionsPageDlg::onChange_FT);
	chkOther.OnChange = Callback(this, &CMirandaAdvOptionsPageDlg::onChange_Other);
}

bool CMirandaAdvOptionsPageDlg::OnInitDialog()
{
	g_pBatch->m_dwSinceDate = g_plugin.getDword("ImportSinceTS", time(0));
	struct tm *TM = localtime(&g_pBatch->m_dwSinceDate);

	SYSTEMTIME ST = { 0 };
	ST.wYear = TM->tm_year + 1900;
	ST.wMonth = TM->tm_mon + 1;
	ST.wDay = TM->tm_mday;
	DateTime_SetSystemtime(GetDlgItem(m_hwnd, IDC_DATETIMEPICKER), GDT_VALID, &ST);
	return true;
}

void CMirandaAdvOptionsPageDlg::onClick_Back(CCtrlButton*)
{
	PostMessage(m_hwndParent, WIZM_GOTOPAGE, 0, (LPARAM)new CMirandaOptionsPageDlg());
}

void CMirandaAdvOptionsPageDlg::OnNext()
{
	// clear all another flags but duplicates
	g_pBatch->m_iOptions &= IOPT_CHECKDUPS;

	if (IsDlgButtonChecked(m_hwnd, IDC_CONTACTS))
		g_pBatch->m_iOptions |= IOPT_CONTACTS | IOPT_GROUPS;
	if (IsDlgButtonChecked(m_hwnd, IDC_SYSTEM))
		g_pBatch->m_iOptions |= IOPT_SYSTEM;

	// incoming
	if (IsDlgButtonChecked(m_hwnd, IDC_IN_MSG))
		g_pBatch->m_iOptions |= IOPT_MSGRECV;
	if (IsDlgButtonChecked(m_hwnd, IDC_IN_FT))
		g_pBatch->m_iOptions |= IOPT_FILERECV;
	if (IsDlgButtonChecked(m_hwnd, IDC_IN_OTHER))
		g_pBatch->m_iOptions |= IOPT_OTHERRECV;

	// outgoing
	if (IsDlgButtonChecked(m_hwnd, IDC_OUT_MSG))
		g_pBatch->m_iOptions |= IOPT_MSGSENT;
	if (IsDlgButtonChecked(m_hwnd, IDC_OUT_FT))
		g_pBatch->m_iOptions |= IOPT_FILESENT;
	if (IsDlgButtonChecked(m_hwnd, IDC_OUT_OTHER))
		g_pBatch->m_iOptions |= IOPT_OTHERSENT;

	// since date
	g_pBatch->m_dwSinceDate = 0;

	if (chkSince.IsChecked()) {
		SYSTEMTIME ST = { 0 };

		if (DateTime_GetSystemtime(GetDlgItem(m_hwnd, IDC_DATETIMEPICKER), &ST) == GDT_VALID) {
			struct tm TM = { 0 };

			TM.tm_mday = ST.wDay;
			TM.tm_mon = ST.wMonth - 1;
			TM.tm_year = ST.wYear - 1900;

			g_pBatch->m_dwSinceDate = mktime(&TM);

			g_plugin.setDword("ImportSinceTS", g_pBatch->m_dwSinceDate);
		}
	}

	if (g_pBatch->m_iOptions)
		PostMessage(m_hwndParent, WIZM_GOTOPAGE, 0, (LPARAM)new CProgressPageDlg());
}

void CMirandaAdvOptionsPageDlg::onChange_Since(CCtrlCheck*)
{
	EnableWindow(GetDlgItem(m_hwnd, IDC_DATETIMEPICKER), chkSince.IsChecked());
}

void CMirandaAdvOptionsPageDlg::onChange_All(CCtrlCheck *pCheck)
{
	if (pCheck->GetCtrlId() == IDC_ALL)
		for (auto &it : SysControls)
			CheckDlgButton(m_hwnd, it, IsDlgButtonChecked(m_hwnd, it) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);

	if (pCheck->GetCtrlId() != IDC_OUTGOING)
		for (auto &it : InControls)
			CheckDlgButton(m_hwnd, it, IsDlgButtonChecked(m_hwnd, it) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);

	if (pCheck->GetCtrlId() != IDC_INCOMING)
		for (auto &it : OutControls)
			CheckDlgButton(m_hwnd, it, IsDlgButtonChecked(m_hwnd, it) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
}

void CMirandaAdvOptionsPageDlg::onChange_Msg(CCtrlCheck*)
{
	CheckDlgButton(m_hwnd, IDC_IN_MSG, IsDlgButtonChecked(m_hwnd, IDC_IN_MSG) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(m_hwnd, IDC_OUT_MSG, IsDlgButtonChecked(m_hwnd, IDC_OUT_MSG) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
}

void CMirandaAdvOptionsPageDlg::onChange_FT(CCtrlCheck*)
{
	CheckDlgButton(m_hwnd, IDC_IN_FT, IsDlgButtonChecked(m_hwnd, IDC_IN_FT) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(m_hwnd, IDC_OUT_FT, IsDlgButtonChecked(m_hwnd, IDC_OUT_FT) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
}

void CMirandaAdvOptionsPageDlg::onChange_Other(CCtrlCheck*)
{
	CheckDlgButton(m_hwnd, IDC_IN_OTHER, IsDlgButtonChecked(m_hwnd, IDC_IN_OTHER) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(m_hwnd, IDC_OUT_OTHER, IsDlgButtonChecked(m_hwnd, IDC_OUT_OTHER) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
}
