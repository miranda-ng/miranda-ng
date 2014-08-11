/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"
#include "profilemanager.h"

static bool bModuleInitialized = false;
static HANDLE hIniChangeNotification;

static INT_PTR CALLBACK InstallIniDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetDlgItemText(hwndDlg, IDC_ININAME, (TCHAR*)lParam);
		{
			TCHAR szSecurity[11];
			const TCHAR *pszSecurityInfo;

			GetPrivateProfileString(_T("AutoExec"), _T("Warn"), _T("notsafe"), szSecurity, SIZEOF(szSecurity), mirandabootini);
			if (!lstrcmpi(szSecurity, _T("all")))
				pszSecurityInfo = LPGENT("Security systems to prevent malicious changes are in place and you will be warned before every change that is made.");
			else if (!lstrcmpi(szSecurity, _T("onlyunsafe")))
				pszSecurityInfo = LPGENT("Security systems to prevent malicious changes are in place and you will be warned before changes that are known to be unsafe.");
			else if (!lstrcmpi(szSecurity, _T("none")))
				pszSecurityInfo = LPGENT("Security systems to prevent malicious changes have been disabled. You will receive no further warnings.");
			else pszSecurityInfo = NULL;
			if (pszSecurityInfo) SetDlgItemText(hwndDlg, IDC_SECURITYINFO, TranslateTS(pszSecurityInfo));
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_VIEWINI:
			{
				TCHAR szPath[MAX_PATH];
				GetDlgItemText(hwndDlg, IDC_ININAME, szPath, SIZEOF(szPath));
				ShellExecute(hwndDlg, _T("open"), szPath, NULL, NULL, SW_SHOW);
			}
			break;

		case IDOK:
		case IDCANCEL:
		case IDC_NOTOALL:
			EndDialog(hwndDlg, LOWORD(wParam));
			break;
		}
		break;
	}
	return FALSE;
}

static bool IsInSpaceSeparatedList(const char *szWord, const char *szList)
{
	const char *szItem, *szEnd;
	int wordLen = lstrlenA(szWord);

	for (szItem = szList;;) {
		szEnd = strchr(szItem, ' ');
		if (szEnd == NULL)
			return !lstrcmpA(szItem, szWord);

		if (szEnd - szItem == wordLen)
			if (!strncmp(szItem, szWord, wordLen))
				return true;

		szItem = szEnd + 1;
	}
}

struct warnSettingChangeInfo_t {
	TCHAR *szIniPath;
	char *szSection;
	char *szSafeSections;
	char *szUnsafeSections;
	char *szName;
	char *szValue;
	int warnNoMore, cancel;
};

static INT_PTR CALLBACK WarnIniChangeDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static warnSettingChangeInfo_t *warnInfo;

	switch(message) {
	case WM_INITDIALOG:
		{
			char szSettingName[256];
			const TCHAR *pszSecurityInfo;
			warnInfo = (warnSettingChangeInfo_t*)lParam;
			TranslateDialogDefault(hwndDlg);
			SetDlgItemText(hwndDlg, IDC_ININAME, warnInfo->szIniPath);
			lstrcpyA(szSettingName, warnInfo->szSection);
			lstrcatA(szSettingName, " / ");
			lstrcatA(szSettingName, warnInfo->szName);
			SetDlgItemTextA(hwndDlg, IDC_SETTINGNAME, szSettingName);
			SetDlgItemTextA(hwndDlg, IDC_NEWVALUE, warnInfo->szValue);
			if (IsInSpaceSeparatedList(warnInfo->szSection, warnInfo->szSafeSections))
				pszSecurityInfo = LPGENT("This change is known to be safe.");
			else if (IsInSpaceSeparatedList(warnInfo->szSection, warnInfo->szUnsafeSections))
				pszSecurityInfo = LPGENT("This change is known to be potentially hazardous.");
			else
				pszSecurityInfo = LPGENT("This change is not known to be safe.");
			SetDlgItemText(hwndDlg, IDC_SECURITYINFO, TranslateTS(pszSecurityInfo));
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			warnInfo->cancel = 1;
		case IDYES:
		case IDNO:
			warnInfo->warnNoMore = IsDlgButtonChecked(hwndDlg, IDC_WARNNOMORE);
			EndDialog(hwndDlg, LOWORD(wParam));
			break;
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK IniImportDoneDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR szIniPath[MAX_PATH];

	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetDlgItemText(hwndDlg, IDC_ININAME, (TCHAR*)lParam);
		SetDlgItemText(hwndDlg, IDC_NEWNAME, (TCHAR*)lParam);
		return TRUE;

	case WM_COMMAND:
		GetDlgItemText(hwndDlg, IDC_ININAME, szIniPath, SIZEOF(szIniPath));
		switch (LOWORD(wParam)) {
		case IDC_DELETE:
			DeleteFile(szIniPath);
		case IDC_LEAVE:
			EndDialog(hwndDlg, LOWORD(wParam));
			break;
		case IDC_RECYCLE:
			{
				SHFILEOPSTRUCT shfo = { 0 };
				shfo.wFunc = FO_DELETE;
				shfo.pFrom = szIniPath;
				szIniPath[lstrlen(szIniPath) + 1] = '\0';
				shfo.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT | FOF_ALLOWUNDO;
				SHFileOperation(&shfo);
			}
			EndDialog(hwndDlg, LOWORD(wParam));
			break;
		case IDC_MOVE:
			TCHAR szNewPath[MAX_PATH];
			GetDlgItemText(hwndDlg, IDC_NEWNAME, szNewPath, SIZEOF(szNewPath));
			MoveFile(szIniPath, szNewPath);
			EndDialog(hwndDlg, LOWORD(wParam));
			break;
		}
		break;
	}
	return FALSE;
}

// settings:
struct SettingsList
{
	char *name;
	SettingsList *next;
} *setting_items = NULL;

int SettingsEnumProc(const char *szSetting, LPARAM lParam)
{
	SettingsList *newItem = (SettingsList *)mir_alloc(sizeof(SettingsList));
	newItem->name = mir_strdup(szSetting);
	newItem->next = setting_items;
	setting_items = newItem;
	return 0;
}

static void ConvertBackslashes(char *str, UINT fileCp)
{
	char *pstr;
	for (pstr = str; *pstr; pstr = CharNextExA(fileCp, pstr, 0)) {
		if (*pstr == '\\') {
			switch (pstr[1]) {
			case 'n': *pstr = '\n'; break;
			case 't': *pstr = '\t'; break;
			case 'r': *pstr = '\r'; break;
			default:  *pstr = pstr[1]; break;
			}
			memmove(pstr + 1, pstr + 2, strlen(pstr + 2) + 1);
		}
	}
}

static void ProcessIniFile(TCHAR* szIniPath, char *szSafeSections, char *szUnsafeSections, int secur, bool secFN)
{
	FILE *fp = _tfopen(szIniPath, _T("rt"));
	if (fp == NULL)
		return;

	bool warnThisSection = false;
	char szSection[128]; szSection[0] = 0;

	while (!feof(fp)) {
		char szLine[2048];
		if (fgets(szLine, sizeof(szLine), fp) == NULL)
			break;
LBL_NewLine:
		int lineLength = lstrlenA(szLine);
		while (lineLength && (BYTE)(szLine[lineLength - 1]) <= ' ')
			szLine[--lineLength] = '\0';

		if (szLine[0] == ';' || szLine[0] <= ' ')
			continue;

		if (szLine[0] == '[') {
			char *szEnd = strchr(szLine + 1, ']');
			if (szEnd == NULL)
				continue;

			if (szLine[1] == '!')
				szSection[0] = '\0';
			else {
				lstrcpynA(szSection, szLine + 1, min(sizeof(szSection), (int)(szEnd - szLine)));
				switch (secur) {
				case 0:
					warnThisSection = false;
					break;

				case 1:
					warnThisSection = !IsInSpaceSeparatedList(szSection, szSafeSections);
					break;

				case 2:
					warnThisSection = IsInSpaceSeparatedList(szSection, szUnsafeSections);
					break;

				default:
					warnThisSection = true;
					break;
				}
				if (secFN) warnThisSection = 0;
			}
			if (szLine[1] == '?') {
				DBCONTACTENUMSETTINGS dbces;
				dbces.pfnEnumProc = SettingsEnumProc;
				lstrcpynA(szSection, szLine+2, min(sizeof(szSection), (int)(szEnd-szLine-1)));
				dbces.szModule = szSection;
				dbces.ofsSettings = 0;
				CallService(MS_DB_CONTACT_ENUMSETTINGS, 0, (LPARAM)&dbces);
				while (setting_items) {
					SettingsList *next = setting_items->next;

					db_unset(NULL, szSection, setting_items->name);

					mir_free(setting_items->name);
					mir_free(setting_items);
					setting_items = next;
				}
			}
			continue;
		}

		if (szSection[0] == '\0')
			continue;

		char *szValue = strchr(szLine, '=');
		if (szValue == NULL)
			continue;

		char szName[128];
		lstrcpynA(szName, szLine, min(sizeof(szName), (int)(szValue-szLine+1)));
		szValue++;
		{
			warnSettingChangeInfo_t warnInfo;
			warnInfo.szIniPath = szIniPath;
			warnInfo.szName = szName;
			warnInfo.szSafeSections = szSafeSections;
			warnInfo.szSection = szSection;
			warnInfo.szUnsafeSections = szUnsafeSections;
			warnInfo.szValue = szValue;
			warnInfo.warnNoMore = 0;
			warnInfo.cancel = 0;
			if (warnThisSection && IDNO == DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_WARNINICHANGE), NULL, WarnIniChangeDlgProc, (LPARAM)&warnInfo))
				continue;
			if (warnInfo.cancel)
				break;
			if (warnInfo.warnNoMore)
				warnThisSection = 0;
		}

		switch (szValue[0]) {
		case 'b':
		case 'B':
			db_set_b(NULL, szSection, szName, (BYTE)strtol(szValue+1, NULL, 0));
			break;
		case 'w':
		case 'W':
			db_set_w(NULL, szSection, szName, (WORD)strtol(szValue+1, NULL, 0));
			break;
		case 'd':
		case 'D':
			db_set_dw(NULL, szSection, szName, (DWORD)strtoul(szValue+1, NULL, 0));
			break;
		case 'l':
		case 'L':
		case '-':
			db_unset(NULL, szSection, szName);
			break;
		case 'e':
		case 'E':
			ConvertBackslashes(szValue+1, Langpack_GetDefaultCodePage());
		case 's':
		case 'S':
			db_set_s(NULL, szSection, szName, szValue+1);
			break;
		case 'g':
		case 'G':
			for (char *pstr = szValue + 1; *pstr; pstr++) {
				if (*pstr == '\\') {
					switch (pstr[1]) {
					case 'n': *pstr = '\n'; break;
					case 't': *pstr = '\t'; break;
					case 'r': *pstr = '\r'; break;
					default:  *pstr = pstr[1]; break;
					}
					MoveMemory(pstr + 1, pstr + 2, lstrlenA(pstr + 2) + 1);
				}
			}
		case 'u':
		case 'U':
			db_set_utf(NULL, szSection, szName, szValue + 1);
			break;
		case 'm':
		case 'M':
			{
				CMStringA memo(szValue + 1);
				memo.Append("\r\n");
				while (fgets(szLine, sizeof(szLine), fp) != NULL) {
					switch (szLine[0]) {
					case 0: case '\r': case '\n': case ' ': case '\t':
						break;
					default:
						db_set_utf(NULL, szSection, szName, memo);
						goto LBL_NewLine;
					}

					memo.Append(rtrim(szLine + 1));
					memo.Append("\r\n");
				}
				db_set_utf(NULL, szSection, szName, memo);
			}
			break;
		case 'n':
		case 'h':
		case 'N':
		case 'H':
			{
				int len;
				char *pszValue, *pszEnd;

				PBYTE buf = (PBYTE)mir_alloc(lstrlenA(szValue + 1));
				for (len = 0, pszValue = szValue + 1;; len++) {
					buf[len] = (BYTE)strtol(pszValue, &pszEnd, 0x10);
					if (pszValue == pszEnd)
						break;
					pszValue = pszEnd;
				}
				db_set_blob(NULL, szSection, szName, buf, len);
				mir_free(buf);
			}
			break;
		default:
			TCHAR buf[250];
			mir_sntprintf(buf, SIZEOF(buf), TranslateT("Invalid setting type for '%s'. The first character of every value must be b, w, d, l, s, e, u, g, h or n."), _A2T(szName));
			MessageBox(NULL, buf, TranslateT("Install database settings"), MB_ICONWARNING | MB_OK);
			break;
		}
	}
	fclose(fp);
}

static void DoAutoExec(void)
{
	TCHAR szUse[7], szIniPath[MAX_PATH], szFindPath[MAX_PATH];
	TCHAR buf[2048], szSecurity[11], szOverrideSecurityFilename[MAX_PATH], szOnCreateFilename[MAX_PATH];
	char *szSafeSections, *szUnsafeSections;
	int secur;

	GetPrivateProfileString(_T("AutoExec"), _T("Use"), _T("prompt"), szUse, SIZEOF(szUse), mirandabootini);
	if (!lstrcmpi(szUse, _T("no"))) return;
	GetPrivateProfileString(_T("AutoExec"), _T("Safe"), _T("CLC Icons CLUI CList SkinSounds PluginUpdater"), buf, SIZEOF(buf), mirandabootini);
	szSafeSections = mir_t2a(buf);
	GetPrivateProfileString(_T("AutoExec"), _T("Unsafe"), _T("AIM Facebook GG ICQ IRC JABBER MRA MSN SKYPE Tlen TWITTER VKontakte XFire"), buf, SIZEOF(buf), mirandabootini);
	szUnsafeSections = mir_t2a(buf);
	GetPrivateProfileString(_T("AutoExec"), _T("Warn"), _T("notsafe"), szSecurity, SIZEOF(szSecurity), mirandabootini);
	if (!lstrcmpi(szSecurity, _T("none"))) secur = 0;
	else if (!lstrcmpi(szSecurity, _T("notsafe"))) secur = 1;
	else if (!lstrcmpi(szSecurity, _T("onlyunsafe"))) secur = 2;

	GetPrivateProfileString(_T("AutoExec"), _T("OverrideSecurityFilename"), _T(""), szOverrideSecurityFilename, SIZEOF(szOverrideSecurityFilename), mirandabootini);
	GetPrivateProfileString(_T("AutoExec"), _T("OnCreateFilename"), _T(""), szOnCreateFilename, SIZEOF(szOnCreateFilename), mirandabootini);
	GetPrivateProfileString(_T("AutoExec"), _T("Glob"), _T("autoexec_*.ini"), szFindPath, SIZEOF(szFindPath), mirandabootini);

	if (g_bDbCreated && szOnCreateFilename[0]) {
		PathToAbsoluteT(VARST(szOnCreateFilename), szIniPath);
		ProcessIniFile(szIniPath, szSafeSections, szUnsafeSections, 0, 1);
	}

	PathToAbsoluteT(VARST(szFindPath), szFindPath);

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(szFindPath, &fd);
	if (hFind == INVALID_HANDLE_VALUE) {
		mir_free(szSafeSections);
		mir_free(szUnsafeSections);
		return;
	}

	TCHAR *str2 = _tcsrchr(szFindPath, '\\');
	if (str2 == NULL) szFindPath[0] = 0;
	else str2[1] = 0;

	do {
		bool secFN = lstrcmpi(fd.cFileName, szOverrideSecurityFilename) == 0;

		mir_sntprintf(szIniPath, SIZEOF(szIniPath), _T("%s%s"), szFindPath, fd.cFileName);
		if (!lstrcmpi(szUse, _T("prompt")) && !secFN) {
			int result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_INSTALLINI), NULL, InstallIniDlgProc, (LPARAM)szIniPath);
			if (result == IDC_NOTOALL) break;
			if (result == IDCANCEL) continue;
		}

		ProcessIniFile(szIniPath, szSafeSections, szUnsafeSections, secur, secFN);

		if (secFN)
			DeleteFile(szIniPath);
		else {
			TCHAR szOnCompletion[8];
			GetPrivateProfileString(_T("AutoExec"), _T("OnCompletion"), _T("recycle"), szOnCompletion, SIZEOF(szOnCompletion), mirandabootini);
			if (!lstrcmpi(szOnCompletion, _T("delete")))
				DeleteFile(szIniPath);
			else if (!lstrcmpi(szOnCompletion, _T("recycle"))) {
				SHFILEOPSTRUCT shfo = { 0 };
				shfo.wFunc = FO_DELETE;
				shfo.pFrom = szIniPath;
				szIniPath[lstrlen(szIniPath) + 1] = 0;
				shfo.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT | FOF_ALLOWUNDO;
				SHFileOperation(&shfo);
			}
			else if (!lstrcmpi(szOnCompletion, _T("rename"))) {
				TCHAR szRenamePrefix[MAX_PATH];
				TCHAR szNewPath[MAX_PATH];
				GetPrivateProfileString(_T("AutoExec"), _T("RenamePrefix"), _T("done_"), szRenamePrefix, SIZEOF(szRenamePrefix), mirandabootini);
				lstrcpy(szNewPath, szFindPath);
				lstrcat(szNewPath, szRenamePrefix);
				lstrcat(szNewPath, fd.cFileName);
				MoveFile(szIniPath, szNewPath);
			}
			else if (!lstrcmpi(szOnCompletion, _T("ask")))
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_INIIMPORTDONE), NULL, IniImportDoneDlgProc, (LPARAM)szIniPath);
		}
	}
		while (FindNextFile(hFind, &fd));

	FindClose(hFind);
	mir_free(szSafeSections);
	mir_free(szUnsafeSections);
}

static INT_PTR CheckIniImportNow(WPARAM, LPARAM)
{
	DoAutoExec();
	FindNextChangeNotification(hIniChangeNotification);
	return 0;
}

int InitIni(void)
{
	bModuleInitialized = true;

	DoAutoExec();

	TCHAR szMirandaDir[MAX_PATH];
	PathToAbsoluteT(_T("."), szMirandaDir);
	hIniChangeNotification = FindFirstChangeNotification(szMirandaDir, 0, FILE_NOTIFY_CHANGE_FILE_NAME);
	if (hIniChangeNotification != INVALID_HANDLE_VALUE) {
		CreateServiceFunction("DB/Ini/CheckImportNow", CheckIniImportNow);
		CallService(MS_SYSTEM_WAITONHANDLE, (WPARAM)hIniChangeNotification, (LPARAM)"DB/Ini/CheckImportNow");
	}
	return 0;
}

void UninitIni(void)
{
	if (!bModuleInitialized)
		return;

	CallService(MS_SYSTEM_REMOVEWAIT, (WPARAM)hIniChangeNotification, 0);
	FindCloseChangeNotification(hIniChangeNotification);
}
