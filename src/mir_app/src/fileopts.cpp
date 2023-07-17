/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org),
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

#ifndef SHACF_FILESYS_DIRS
	#define SHACF_FILESYS_DIRS  0x00000020
#endif

class CFileGeneralOptsDlg : public CDlgBase
{
	CCtrlButton btnFileDir;
	CCtrlCheck chkAutoMin, chkAutoClear, chkAutoClose, chkAutoAccept, chkReverseOrder;
	CCtrlCombo cmbFileExists;

public:
	CFileGeneralOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_FILEGENERAL),
		btnFileDir(this, IDC_FILEDIRBROWSE),
		chkAutoMin(this, IDC_AUTOMIN),
		chkAutoClear(this, IDC_AUTOCLEAR),
		chkAutoClose(this, IDC_AUTOCLOSE),
		chkAutoAccept(this, IDC_AUTOACCEPT),
		cmbFileExists(this, IDC_FILEEXISTS),
		chkReverseOrder(this, IDC_REVERSE_ORDER)
	{
		CreateLink(chkAutoMin, File::bAutoMin);
		CreateLink(chkAutoClear, File::bAutoClear);
		CreateLink(chkAutoClose, File::bAutoClose);
		CreateLink(chkAutoAccept, File::bAutoAccept);
		CreateLink(chkReverseOrder, File::bReverseOrder);

		btnFileDir.OnClick = Callback(this, &CFileGeneralOptsDlg::onClick_FileDir);

		chkAutoAccept.OnChange = Callback(this, &CFileGeneralOptsDlg::onChange_AutoAccept);
	}

	bool OnInitDialog() override
	{
		SHAutoComplete(GetDlgItem(m_hwnd, IDC_FILEDIR), SHACF_FILESYS_DIRS);

		cmbFileExists.AddString(TranslateT("Ask me"), 0);
		cmbFileExists.AddString(TranslateT("Resume"), FILERESUME_RESUMEALL);
		cmbFileExists.AddString(TranslateT("Overwrite"), FILERESUME_OVERWRITEALL);
		cmbFileExists.AddString(TranslateT("Rename (append \" (1)\", etc.)"), FILERESUME_RENAMEALL);
		cmbFileExists.SelectData(File::iIfExists);

		wchar_t str[MAX_PATH];
		GetContactReceivedFilesDir(NULL, str, _countof(str), FALSE);
		SetDlgItemText(m_hwnd, IDC_FILEDIR, str);
		return true;
	}

	bool OnApply() override
	{
		wchar_t str[512];
		GetDlgItemText(m_hwnd, IDC_FILEDIR, str, _countof(str));
		RemoveInvalidPathChars(str);
		File::wszSaveDir = str;

		File::iIfExists = cmbFileExists.GetCurData();
		return TRUE;
	}

	void onChange_AutoAccept(CCtrlCheck *)
	{
		chkAutoMin.Enable(chkAutoAccept.GetState());
	}

	void onClick_FileDir(CCtrlButton*)
	{
		wchar_t str[MAX_PATH];
		GetDlgItemText(m_hwnd, IDC_FILEDIR, str, _countof(str));
		if (BrowseForFolder(m_hwnd, str))
			SetDlgItemText(m_hwnd, IDC_FILEDIR, str);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// File security options

#define VSCAN_MCAFEE      1
#define VSCAN_DRSOLOMON   2
#define VSCAN_NORTON      3
#define VSCAN_CA          4

struct
{
	const wchar_t *szProductName;
	const wchar_t *szExeRegPath;
	const wchar_t *szExeRegValue;
	const wchar_t *szCommandLine;
}
static virusScanners[] =
{
	{ L"Network Associates/McAfee VirusScan", L"SOFTWARE\\McAfee\\VirusScan", L"Scan32EXE", L"\"%s\" %%f /nosplash /comp /autoscan /autoexit /noboot" },
	{ L"Dr Solomon's VirusScan (Network Associates)", L"SOFTWARE\\Network Associates\\TVD\\VirusScan\\AVConsol\\General", L"szScannerExe", L"\"%s\" %%f /uinone /noboot /comp /prompt /autoexit" },
	{ L"Norton AntiVirus", L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\Navw32.exe", nullptr, L"\"%s\" %%f /b- /m- /s+ /noresults" },
	{ L"Computer Associates/Inoculate IT", L"Software\\Antivirus", L"ImageFilename", L"\"%s\" %%f /display = progress /exit" },
	{ L"Computer Associates eTrust", L"SOFTWARE\\ComputerAssociates\\Anti-Virus\\Resident", L"VetPath", L"\"%s\" %%f /display = progress /exit" },
	{ L"Kaspersky Anti-Virus", L"SOFTWARE\\KasperskyLab\\Components\\101", L"EXEName", L"\"%s\" /S /Q %%f" },
	{ L"Kaspersky Anti-Virus", L"SOFTWARE\\KasperskyLab\\SetupFolders", L"KAV8", L"\"%savp.exe\" SCAN %%f" },
	{ L"Kaspersky Anti-Virus", L"SOFTWARE\\KasperskyLab\\SetupFolders", L"KAV9", L"\"%savp.exe\" SCAN %%f" },
	{ L"AntiVir PersonalEdition Classic", L"SOFTWARE\\Avira\\AntiVir PersonalEdition Classic", L"Path", L"\"%savscan.exe\" /GUIMODE = 2 /PATH = \"%%f\"" },
	{ L"ESET NOD32 Antivirus", L"SOFTWARE\\ESET\\ESET Security\\CurrentVersion\\Info", L"InstallDir", L"\"%secls.exe\" /log-all /aind /no-boots /adware /sfx /unsafe /unwanted /heur /adv-heur /action = clean \"%%f\"" },
};

class CFileSecurityOptsDlg : public CDlgBase
{
	CCtrlButton btnScanCmdLine;
	CCtrlCheck chkNoScanner, chkScanDuringDl, chkScanAfterDl, chkWarnBeforeOpening;
	CCtrlCombo cmbScanCmdLine;

public:
	CFileSecurityOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_FILESECURITY),
		chkNoScanner(this, IDC_NOSCANNER),
		chkScanAfterDl(this, IDC_SCANAFTERDL),
		chkScanDuringDl(this, IDC_SCANDURINGDL),
		cmbScanCmdLine(this, IDC_SCANCMDLINE),
		btnScanCmdLine(this, IDC_SCANCMDLINEBROWSE),
		chkWarnBeforeOpening(this, IDC_WARNBEFOREOPENING)
	{
		CreateLink(chkWarnBeforeOpening, File::bWarnBeforeOpening);

		btnScanCmdLine.OnClick = Callback(this, &CFileSecurityOptsDlg::onClick_ScanCmdLine);

		chkNoScanner.OnChange = Callback(this, &CFileSecurityOptsDlg::onChange_NoScanner);

		cmbScanCmdLine.OnSelChanged = Callback(this, &CFileSecurityOptsDlg::onSelChanged_Combo);
	}

	bool OnInitDialog() override
	{
		switch (File::iUseScanner) {
		case VIRUSSCAN_AFTERDL: chkScanAfterDl.SetState(true); break;
		case VIRUSSCAN_DURINGDL: chkScanDuringDl.SetState(true); break;
		default: chkNoScanner.SetState(true); break;
		}

		for (int i = 0; i < _countof(virusScanners); i++) {
			wchar_t szScanExe[MAX_PATH];
			if (SRFile_GetRegValue(HKEY_LOCAL_MACHINE, virusScanners[i].szExeRegPath, virusScanners[i].szExeRegValue, szScanExe, _countof(szScanExe)))
				cmbScanCmdLine.AddString(virusScanners[i].szProductName, i);
		}

		if (!cmbScanCmdLine.GetCount())
			cmbScanCmdLine.AddString(L"", -1);

		if (mir_wstrlen(File::wszScanCmdLine)) {
			cmbScanCmdLine.SetText(File::wszScanCmdLine);
		}
		else if (cmbScanCmdLine.GetCount()) {
			cmbScanCmdLine.SetCurSel(0);
			onSelChanged_Combo(0);
		}

		return true;
	}

	bool OnApply() override
	{
		File::wszScanCmdLine = ptrW(cmbScanCmdLine.GetText());

		File::iUseScanner = chkScanAfterDl.GetState() ? VIRUSSCAN_AFTERDL : (chkScanDuringDl.GetState() ? VIRUSSCAN_DURINGDL : VIRUSSCAN_DISABLE);
		return true;
	}

	void onChange_NoScanner(CCtrlCheck *)
	{
		bool bEnabled = chkNoScanner.GetState();
		btnScanCmdLine.Enable(bEnabled);
		cmbScanCmdLine.Enable(bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_ST_CMDLINE), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_ST_CMDLINEHELP), bEnabled);
	}

	void onSelChanged_Combo(CCtrlCombo *)
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

	void onClick_ScanCmdLine(CCtrlButton *)
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

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

int SRFileOptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.position = 900000000;
	odp.szTitle.a = LPGEN("File transfers");
	odp.szGroup.a = LPGEN("Events");

	odp.szTab.a = LPGEN("General");
	odp.pDialog = new CFileGeneralOptsDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Security");
	odp.pDialog = new CFileSecurityOptsDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
