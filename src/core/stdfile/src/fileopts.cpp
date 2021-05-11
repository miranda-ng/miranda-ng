/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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
#include "file.h"

#define VSCAN_MCAFEE      1
#define VSCAN_DRSOLOMON   2
#define VSCAN_NORTON      3
#define VSCAN_CA          4

struct {
	const wchar_t *szProductName;
	const wchar_t *szExeRegPath;
	const wchar_t *szExeRegValue;
	const wchar_t *szCommandLine;
}
static virusScanners[] =
{
	{L"Network Associates/McAfee VirusScan", L"SOFTWARE\\McAfee\\VirusScan", L"Scan32EXE", L"\"%s\" %%f /nosplash /comp /autoscan /autoexit /noboot"},
	{L"Dr Solomon's VirusScan (Network Associates)", L"SOFTWARE\\Network Associates\\TVD\\VirusScan\\AVConsol\\General", L"szScannerExe", L"\"%s\" %%f /uinone /noboot /comp /prompt /autoexit"},
	{L"Norton AntiVirus", L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\Navw32.exe", nullptr, L"\"%s\" %%f /b- /m- /s+ /noresults"},
	{L"Computer Associates/Inoculate IT", L"Software\\Antivirus", L"ImageFilename", L"\"%s\" %%f /display = progress /exit"},
	{L"Computer Associates eTrust", L"SOFTWARE\\ComputerAssociates\\Anti-Virus\\Resident", L"VetPath", L"\"%s\" %%f /display = progress /exit"},
	{L"Kaspersky Anti-Virus", L"SOFTWARE\\KasperskyLab\\Components\\101", L"EXEName", L"\"%s\" /S /Q %%f"},
	{L"Kaspersky Anti-Virus", L"SOFTWARE\\KasperskyLab\\SetupFolders", L"KAV8", L"\"%savp.exe\" SCAN %%f"},
	{L"Kaspersky Anti-Virus", L"SOFTWARE\\KasperskyLab\\SetupFolders", L"KAV9", L"\"%savp.exe\" SCAN %%f"},
	{L"AntiVir PersonalEdition Classic", L"SOFTWARE\\Avira\\AntiVir PersonalEdition Classic", L"Path", L"\"%savscan.exe\" /GUIMODE = 2 /PATH = \"%%f\""},
	{L"ESET NOD32 Antivirus", L"SOFTWARE\\ESET\\ESET Security\\CurrentVersion\\Info", L"InstallDir", L"\"%secls.exe\" /log-all /aind /no-boots /adware /sfx /unsafe /unwanted /heur /adv-heur /action = clean \"%%f\""},
};

#ifndef SHACF_FILESYS_DIRS
	#define SHACF_FILESYS_DIRS  0x00000020
#endif

class CFileOptsDlg : public CDlgBase
{
	CCtrlButton btnFileDir, btnScanCmdLine;
	CCtrlCheck chkAutoMin, chkAutoClear, chkAutoClose, chkAutoAccept, chkReverseOrder;
	CCtrlCheck chkNoScanner, chkScanDuringDl, chkScanAfterDl;
	CCtrlCombo cmbScanCmdLine;

public:
	CFileOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_FILETRANSFER),
		btnFileDir(this, IDC_FILEDIRBROWSE),
		btnScanCmdLine(this, IDC_SCANCMDLINEBROWSE),
		chkAutoMin(this, IDC_AUTOMIN),
		chkAutoClear(this, IDC_AUTOCLEAR),
		chkAutoClose(this, IDC_AUTOCLOSE),
		chkAutoAccept(this, IDC_AUTOACCEPT),
		chkReverseOrder(this, IDC_REVERSE_ORDER),

		chkNoScanner(this, IDC_NOSCANNER),
		chkScanAfterDl(this, IDC_SCANAFTERDL),
		chkScanDuringDl(this, IDC_SCANDURINGDL),

		cmbScanCmdLine(this, IDC_SCANCMDLINE)
	{
		CreateLink(chkAutoMin, g_plugin.bAutoMin);
		CreateLink(chkAutoClear, g_plugin.bAutoClear);
		CreateLink(chkAutoClose, g_plugin.bAutoClose);
		CreateLink(chkAutoAccept, g_plugin.bAutoAccept);
		CreateLink(chkReverseOrder, g_plugin.bReverseOrder);

		btnFileDir.OnClick = Callback(this, &CFileOptsDlg::onClick_FileDir);
		btnScanCmdLine.OnClick = Callback(this, &CFileOptsDlg::onClick_ScanCmdLine);

		chkNoScanner.OnChange = Callback(this, &CFileOptsDlg::onChange_NoScanner);
		chkAutoAccept.OnChange = Callback(this, &CFileOptsDlg::onChange_AutoAccept);
		cmbScanCmdLine.OnSelChanged = Callback(this, &CFileOptsDlg::onSelChanged_Combo);
	}

	bool OnInitDialog() override
	{
		SHAutoComplete(GetDlgItem(m_hwnd, IDC_FILEDIR), SHACF_FILESYS_DIRS);

		wchar_t str[MAX_PATH];
		GetContactReceivedFilesDir(NULL, str, _countof(str), FALSE);
		SetDlgItemText(m_hwnd, IDC_FILEDIR, str);

		switch (g_plugin.getByte("UseScanner", VIRUSSCAN_DISABLE)) {
		case VIRUSSCAN_AFTERDL: chkScanAfterDl.SetState(true); break;
		case VIRUSSCAN_DURINGDL: chkScanDuringDl.SetState(true); break;
		default: chkNoScanner.SetState(true); break;
		}
		CheckDlgButton(m_hwnd, IDC_WARNBEFOREOPENING, g_plugin.getByte("WarnBeforeOpening", 1) ? BST_CHECKED : BST_UNCHECKED);

		for (int i = 0; i < _countof(virusScanners); i++) {
			wchar_t szScanExe[MAX_PATH];
			if (SRFile_GetRegValue(HKEY_LOCAL_MACHINE, virusScanners[i].szExeRegPath, virusScanners[i].szExeRegValue, szScanExe, _countof(szScanExe)))
				cmbScanCmdLine.AddString(virusScanners[i].szProductName, i);
		}
		
		if (!cmbScanCmdLine.GetCount())
			cmbScanCmdLine.AddString(L"", -1);

		DBVARIANT dbv;
		if (g_plugin.getWString("ScanCmdLine", &dbv) == 0) {
			cmbScanCmdLine.SetText(dbv.pwszVal);
			db_free(&dbv);
		}
		else if (cmbScanCmdLine.GetCount()) {
			cmbScanCmdLine.SetCurSel(0);
			onSelChanged_Combo(0);
		}

		switch (g_plugin.getByte("IfExists", FILERESUME_ASK)) {
		case FILERESUME_RESUMEALL: CheckDlgButton(m_hwnd, IDC_RESUME, BST_CHECKED); break;
		case FILERESUME_OVERWRITEALL: CheckDlgButton(m_hwnd, IDC_OVERWRITE, BST_CHECKED); break;
		case FILERESUME_RENAMEALL: CheckDlgButton(m_hwnd, IDC_RENAME, BST_CHECKED); break;
		default: CheckDlgButton(m_hwnd, IDC_ASK, BST_CHECKED); break;
		}

		return true;
	}

	bool OnApply() override
	{
		wchar_t str[512];
		GetDlgItemText(m_hwnd, IDC_FILEDIR, str, _countof(str));
		RemoveInvalidPathChars(str);
		g_plugin.setWString("RecvFilesDirAdv", str);

		cmbScanCmdLine.GetText(str, _countof(str));
		g_plugin.setWString("ScanCmdLine", str);

		g_plugin.setByte("UseScanner", chkScanAfterDl.GetState() ? VIRUSSCAN_AFTERDL : (chkScanDuringDl.GetState() ? VIRUSSCAN_DURINGDL : VIRUSSCAN_DISABLE));
		g_plugin.setByte("WarnBeforeOpening", (BYTE)IsDlgButtonChecked(m_hwnd, IDC_WARNBEFOREOPENING));
		g_plugin.setByte("IfExists", (BYTE)(IsDlgButtonChecked(m_hwnd, IDC_ASK) ? FILERESUME_ASK : 
			(IsDlgButtonChecked(m_hwnd, IDC_RESUME) ? FILERESUME_RESUMEALL : 
				(IsDlgButtonChecked(m_hwnd, IDC_OVERWRITE) ? FILERESUME_OVERWRITEALL : FILERESUME_RENAMEALL))));
		return TRUE;
	}

	void onChange_AutoAccept(CCtrlCheck *)
	{
		chkAutoMin.Enable(chkAutoAccept.GetState());
	}

	void onChange_NoScanner(CCtrlCheck *)
	{
		bool bEnabled = chkNoScanner.GetState();
		btnScanCmdLine.Enable(bEnabled);
		cmbScanCmdLine.Enable(bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_ST_CMDLINE), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_ST_CMDLINEHELP), bEnabled);
	}

	void onSelChanged_Combo(CCtrlCombo*)
	{
		int iScanner = cmbScanCmdLine.GetCurData();
		if (iScanner >= _countof(virusScanners) || iScanner < 0)
			return;

		wchar_t szScanExe[MAX_PATH], str[512];
		if (SRFile_GetRegValue(HKEY_LOCAL_MACHINE, virusScanners[iScanner].szExeRegPath, virusScanners[iScanner].szExeRegValue, szScanExe, _countof(szScanExe)))
			mir_snwprintf(str, virusScanners[iScanner].szCommandLine, szScanExe);
		else
			str[0] = 0;
		cmbScanCmdLine.SetText(str);
	}

	void onClick_FileDir(CCtrlButton*)
	{
		wchar_t str[MAX_PATH];
		GetDlgItemText(m_hwnd, IDC_FILEDIR, str, _countof(str));
		if (BrowseForFolder(m_hwnd, str))
			SetDlgItemText(m_hwnd, IDC_FILEDIR, str);
	}

	void onClick_ScanCmdLine(CCtrlButton*)
	{
		wchar_t str[MAX_PATH + 2];
		cmbScanCmdLine.GetText(str, _countof(str));

		CMStringW tszFilter;
		tszFilter.AppendFormat(L"%s (*.exe)%c*.exe%c", TranslateT("Executable files"), 0, 0);
		tszFilter.AppendFormat(L"%s (*)%c*%c", TranslateT("All files"), 0, 0);

		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
		ofn.hwndOwner = m_hwnd;
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_DONTADDTORECENT;
		ofn.lpstrFilter = tszFilter;
		ofn.lpstrFile = str;
		ofn.nMaxFile = _countof(str) - 2;
		if (str[0] == '"') {
			wchar_t *pszQuote = wcschr(str + 1, '"');
			if (pszQuote)
				*pszQuote = 0;
			memmove(str, str + 1, (mir_wstrlen(str) * sizeof(wchar_t)));
		}
		else {
			wchar_t *pszSpace = wcschr(str, ' ');
			if (pszSpace) *pszSpace = 0;
		}
		ofn.nMaxFileTitle = MAX_PATH;
		if (!GetOpenFileName(&ofn))
			return;

		if (wcschr(str, ' ') != nullptr) {
			memmove(str + 1, str, ((_countof(str) - 2) * sizeof(wchar_t)));
			str[0] = '"';
			mir_wstrcat(str, L"\"");
		}
		cmbScanCmdLine.SetText(str);
	}
};

int FileOptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 900000000;
	odp.szTitle.a = LPGEN("File transfers");
	odp.szGroup.a = LPGEN("Events");
	odp.pDialog = new CFileOptsDlg();
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
