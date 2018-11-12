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

time_t dwSinceDate = 0;

wchar_t importFile[MAX_PATH];

//=======================================================================================
// Profile selection dialog

CMirandaPageDlg::CMirandaPageDlg() :
	CWizardPageDlg(IDD_MIRANDADB),
	m_list(this, IDC_LIST),
	btnBack(this, IDC_BACK),
	btnOther(this, IDC_OTHER)
{
	m_list.OnSelChange = Callback(this, &CMirandaPageDlg::onSelChanged_list);

	btnBack.OnClick = Callback(this, &CMirandaPageDlg::onClick_Back);
	btnOther.OnClick = Callback(this, &CMirandaPageDlg::onClick_Other);
}

bool CMirandaPageDlg::OnInitDialog()
{
	VARSW pfd(L"%miranda_path%\\Profiles");
	VARSW pfd1(L"%miranda_path%");
	VARSW pfd2(L"%miranda_profilesdir%");
	VARSW pfn(L"%miranda_profilename%");

	SearchForLists(pfd2, pfn);
	SearchForLists(pfd1, nullptr);
	if (mir_wstrcmpi(pfd, pfd2))
		SearchForLists(pfd, nullptr);

	SendDlgItemMessage(m_hwnd, IDC_LIST, LB_SETCURSEL, 0, 0);
	SendMessage(m_hwnd, WM_COMMAND, MAKELONG(IDC_LIST, LBN_SELCHANGE), 0);
	
	wchar_t filename[MAX_PATH];
	GetDlgItemText(m_hwnd, IDC_FILENAME, filename, _countof(filename));
	if (_waccess(filename, 4))
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

	case IDC_FILENAME:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;

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
	mir_wstrcpy(importFile, filename);
	PostMessage(m_hwndParent, WIZM_GOTOPAGE, 0, (LPARAM)new CMirandaOptionsPageDlg());
}

void CMirandaPageDlg::onClick_Back(CCtrlButton*)
{
	PostMessage(m_hwndParent, WIZM_GOTOPAGE, 0, (LPARAM)new CIntroPageDlg());
}

void CMirandaPageDlg::onClick_Other(CCtrlButton*)
{
	ptrW pfd(Utils_ReplaceVarsW(L"%miranda_profilesdir%"));

	wchar_t str[MAX_PATH], text[256];
	GetDlgItemText(m_hwnd, IDC_FILENAME, str, _countof(str));
	mir_snwprintf(text, L"%s (*.dat, *.bak)%c*.dat;*.bak%c%s (*.*)%c*.*%c%c", TranslateT("Miranda NG database"), 0, 0, TranslateT("All Files"), 0, 0, 0);

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

void CMirandaPageDlg::onSelChanged_list(CCtrlListBox*)
{
	int sel = m_list.GetCurSel();
	if (sel != -1) {
		SetDlgItemText(m_hwnd, IDC_FILENAME, (wchar_t*)m_list.GetItemData(sel));
		SendMessage(m_hwndParent, WIZM_ENABLEBUTTON, 1, 0);
	}
}

void CMirandaPageDlg::SearchForLists(const wchar_t *mirandaPath, const wchar_t *mirandaProf)
{
	// find in Miranda profile subfolders
	wchar_t searchspec[MAX_PATH];
	mir_snwprintf(searchspec, L"%s\\*.*", mirandaPath);

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(searchspec, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do {
		// find all subfolders except "." and ".."
		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || !mir_wstrcmp(fd.cFileName, L".") || !mir_wstrcmp(fd.cFileName, L".."))
			continue;

		// skip the current profile too
		if (mirandaProf != nullptr && !mir_wstrcmpi(mirandaProf, fd.cFileName))
			continue;

		wchar_t buf[MAX_PATH], profile[MAX_PATH];
		mir_snwprintf(buf, L"%s\\%s\\%s.dat", mirandaPath, fd.cFileName, fd.cFileName);
		if (_waccess(buf, 0) == 0) {
			mir_snwprintf(profile, L"%s.dat", fd.cFileName);
			m_list.AddString(profile, (LPARAM)mir_wstrdup(buf));
		}
	} while (FindNextFile(hFind, &fd));

	FindClose(hFind);
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
		g_iImportOptions = IOPT_ADDUNKNOWN | IOPT_COMPLETE | iFlags;
		PostMessage(m_hwndParent, WIZM_GOTOPAGE, IDD_PROGRESS, (LPARAM)new CProgressPageDlg());
	}
	else if (IsDlgButtonChecked(m_hwnd, IDC_RADIO_ALL)) {
		g_iImportOptions = IOPT_HISTORY | IOPT_SYSTEM | IOPT_GROUPS | IOPT_CONTACTS | iFlags;
		PostMessage(m_hwndParent, WIZM_GOTOPAGE, IDD_PROGRESS, (LPARAM)new CProgressPageDlg());
	}
	else if (IsDlgButtonChecked(m_hwnd, IDC_RADIO_CONTACTS)) {
		g_iImportOptions = IOPT_CONTACTS;
		PostMessage(m_hwndParent, WIZM_GOTOPAGE, IDD_PROGRESS, (LPARAM)new CProgressPageDlg());
	}
	else if (IsDlgButtonChecked(m_hwnd, IDC_RADIO_CUSTOM)) {
		g_iImportOptions = iFlags;
		PostMessage(m_hwndParent, WIZM_GOTOPAGE, IDD_ADVOPTIONS, (LPARAM)new CMirandaAdvOptionsPageDlg());
	}
}

//=======================================================================================
// Advanced options dialog

static const UINT InControls[] = { IDC_IN_MSG, IDC_IN_URL, IDC_IN_FT, IDC_IN_OTHER };
static const UINT OutControls[] = { IDC_OUT_MSG, IDC_OUT_URL, IDC_OUT_FT, IDC_OUT_OTHER };
static const UINT SysControls[] = { IDC_CONTACTS, IDC_SYSTEM };

CMirandaAdvOptionsPageDlg::CMirandaAdvOptionsPageDlg() :
	CWizardPageDlg(IDD_ADVOPTIONS),
	btnBack(this, IDC_BACK),
	chkFT(this, IDC_FT),
	chkAll(this, IDC_ALL),
	chkMsg(this, IDC_MSG),
	chkUrl(this, IDC_URL),
	chkOther(this, IDC_OTHER),
	chkSince(this, IDC_SINCE),
	chkOutgoing(this, IDC_OUTGOING),
	chkIncoming(this, IDC_INCOMING)
{
	btnBack.OnClick = Callback(this, &CMirandaAdvOptionsPageDlg::onClick_Back);

	chkSince.OnChange = Callback(this, &CMirandaAdvOptionsPageDlg::onChange_Since);
	chkAll.OnChange = chkOutgoing.OnChange = chkIncoming.OnChange = Callback(this, &CMirandaAdvOptionsPageDlg::onChange_All);
	chkMsg.OnChange = Callback(this, &CMirandaAdvOptionsPageDlg::onChange_Msg);
	chkUrl.OnChange = Callback(this, &CMirandaAdvOptionsPageDlg::onChange_Url);
	chkFT.OnChange = Callback(this, &CMirandaAdvOptionsPageDlg::onChange_FT);
	chkOther.OnChange = Callback(this, &CMirandaAdvOptionsPageDlg::onChange_Other);
}

bool CMirandaAdvOptionsPageDlg::OnInitDialog()
{
	dwSinceDate = g_plugin.getDword("ImportSinceTS", time(0));
	struct tm *TM = localtime(&dwSinceDate);

	struct _SYSTEMTIME ST = { 0 };
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
	g_iImportOptions &= IOPT_CHECKDUPS;

	if (IsDlgButtonChecked(m_hwnd, IDC_CONTACTS))
		g_iImportOptions |= IOPT_CONTACTS | IOPT_GROUPS;
	if (IsDlgButtonChecked(m_hwnd, IDC_SYSTEM))
		g_iImportOptions |= IOPT_SYSTEM;

	// incoming
	if (IsDlgButtonChecked(m_hwnd, IDC_IN_MSG))
		g_iImportOptions |= IOPT_MSGRECV;
	if (IsDlgButtonChecked(m_hwnd, IDC_IN_URL))
		g_iImportOptions |= IOPT_URLRECV;
	if (IsDlgButtonChecked(m_hwnd, IDC_IN_FT))
		g_iImportOptions |= IOPT_FILERECV;
	if (IsDlgButtonChecked(m_hwnd, IDC_IN_OTHER))
		g_iImportOptions |= IOPT_OTHERRECV;

	// outgoing
	if (IsDlgButtonChecked(m_hwnd, IDC_OUT_MSG))
		g_iImportOptions |= IOPT_MSGSENT;
	if (IsDlgButtonChecked(m_hwnd, IDC_OUT_URL))
		g_iImportOptions |= IOPT_URLSENT;
	if (IsDlgButtonChecked(m_hwnd, IDC_OUT_FT))
		g_iImportOptions |= IOPT_FILESENT;
	if (IsDlgButtonChecked(m_hwnd, IDC_OUT_OTHER))
		g_iImportOptions |= IOPT_OTHERSENT;

	// since date
	dwSinceDate = 0;

	if (chkSince.IsChecked()) {
		struct _SYSTEMTIME ST = { 0 };

		if (DateTime_GetSystemtime(GetDlgItem(m_hwnd, IDC_DATETIMEPICKER), &ST) == GDT_VALID) {
			struct tm TM = { 0 };

			TM.tm_mday = ST.wDay;
			TM.tm_mon = ST.wMonth - 1;
			TM.tm_year = ST.wYear - 1900;

			dwSinceDate = mktime(&TM);

			g_plugin.setDword("ImportSinceTS", dwSinceDate);
		}
	}

	if (g_iImportOptions)
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

void CMirandaAdvOptionsPageDlg::onChange_Url(CCtrlCheck*)
{
	CheckDlgButton(m_hwnd, IDC_IN_URL, IsDlgButtonChecked(m_hwnd, IDC_IN_URL) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(m_hwnd, IDC_OUT_URL, IsDlgButtonChecked(m_hwnd, IDC_OUT_URL) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
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
