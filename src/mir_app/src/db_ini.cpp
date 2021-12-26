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
#include "profilemanager.h"

static bool bModuleInitialized = false;
static HANDLE hIniChangeNotification;

static void MyDeleteFile(const wchar_t *pwszFileName)
{
	if (PU::PrepareEscalation())
		PU::SafeDeleteFile(pwszFileName);
}

//////////////////////////////////////////////////////

class CInstallIniDlg : public CDlgBase
{
	wchar_t *m_szIniPath;

	CCtrlButton m_viewIni;
	CCtrlButton m_noToAll;

	CCtrlBase m_iniName;
	CCtrlBase m_securityInfo;

protected:
	bool OnInitDialog() override
	{
		m_iniName.SetText(m_szIniPath);

		wchar_t szSecurity[11];
		Profile_GetSetting(L"AutoExec/Warn", szSecurity, L"notsafe");

		const wchar_t *pszSecurityInfo;
		if (!mir_wstrcmpi(szSecurity, L"all"))
			pszSecurityInfo = LPGENW("Security systems to prevent malicious changes are in place and you will be warned before every change that is made.");
		else if (!mir_wstrcmpi(szSecurity, L"onlyunsafe"))
			pszSecurityInfo = LPGENW("Security systems to prevent malicious changes are in place and you will be warned before changes that are known to be unsafe.");
		else if (!mir_wstrcmpi(szSecurity, L"none"))
			pszSecurityInfo = LPGENW("Security systems to prevent malicious changes have been disabled. You will receive no further warnings.");
		else
			pszSecurityInfo = nullptr;

		if (pszSecurityInfo)
			m_securityInfo.SetText(TranslateW(pszSecurityInfo));
		return true;
	}

	void ViewIni_OnClick(CCtrlBase*)
	{
		ptrW szPath(m_iniName.GetText());
		ShellExecute(m_hwnd, L"open", szPath, nullptr, nullptr, SW_SHOW);
	}

	void NoToAll_OnClick(CCtrlBase*)
	{
		EndModal(IDC_NOTOALL);
	}

public:
	CInstallIniDlg(wchar_t *szIniPath) :
		CDlgBase(g_plugin, IDD_INSTALLINI),
		m_noToAll(this, IDC_NOTOALL), m_viewIni(this, IDC_VIEWINI),
		m_iniName(this, IDC_ININAME), m_securityInfo(this, IDC_SECURITYINFO)
	{
		m_szIniPath = szIniPath;

		m_noToAll.OnClick = Callback(this, &CInstallIniDlg::NoToAll_OnClick);
		m_viewIni.OnClick = Callback(this, &CInstallIniDlg::ViewIni_OnClick);
	}
};

//////////////////////////////////////////////////////

static bool IsInSpaceSeparatedList(const char *szWord, const char *szList)
{
	const char *szItem, *szEnd;
	size_t wordLen = mir_strlen(szWord);

	for (szItem = szList;;) {
		szEnd = strchr(szItem, ' ');
		if (szEnd == nullptr)
			return !mir_strcmp(szItem, szWord);

		if (size_t(szEnd - szItem) == wordLen)
			if (!strncmp(szItem, szWord, wordLen))
				return true;

		szItem = szEnd + 1;
	}
}

struct warnSettingChangeInfo_t
{
	wchar_t *szIniPath;
	char *szSection;
	char *szSafeSections;
	char *szUnsafeSections;
	char *szName;
	char *szValue;
	int warnNoMore, cancel;
};

class CWarnIniChangeDlg : public CDlgBase
{
	warnSettingChangeInfo_t *m_warnInfo;

	CCtrlButton m_yes;
	CCtrlButton m_no;

	CCtrlCheck m_noWarn;

	CCtrlBase m_iniName;
	CCtrlBase m_settingName;
	CCtrlBase m_securityInfo;
	CCtrlBase m_newValue;

protected:
	bool OnInitDialog() override
	{
		char szSettingName[256];
		m_iniName.SetText(m_warnInfo->szIniPath);
		mir_strcpy(szSettingName, m_warnInfo->szSection);
		mir_strcat(szSettingName, " / ");
		mir_strcat(szSettingName, m_warnInfo->szName);
		m_settingName.SetTextA(szSettingName);
		m_newValue.SetTextA(m_warnInfo->szValue);

		const wchar_t *pszSecurityInfo;
		if (IsInSpaceSeparatedList(m_warnInfo->szSection, m_warnInfo->szSafeSections))
			pszSecurityInfo = LPGENW("This change is known to be safe.");
		else if (IsInSpaceSeparatedList(m_warnInfo->szSection, m_warnInfo->szUnsafeSections))
			pszSecurityInfo = LPGENW("This change is known to be potentially hazardous.");
		else
			pszSecurityInfo = LPGENW("This change is not known to be safe.");
		m_securityInfo.SetText(TranslateW(pszSecurityInfo));
		return true;
	}

	void OnDestroy() override
	{
		m_warnInfo->cancel = !m_bSucceeded;
		m_warnInfo->warnNoMore = m_noWarn.GetState();
	}

	void YesNo_OnClick(CCtrlBase *pButton)
	{
		EndModal(pButton->GetCtrlId());
	}

public:
	CWarnIniChangeDlg(warnSettingChangeInfo_t *warnInfo) :
		CDlgBase(g_plugin, IDD_WARNINICHANGE),
		m_yes(this, IDOK), m_no(this, IDNO),
		m_noWarn(this, IDC_WARNNOMORE),
		m_iniName(this, IDC_ININAME), m_settingName(this, IDC_SETTINGNAME),
		m_newValue(this, IDC_NEWVALUE), m_securityInfo(this, IDC_SECURITYINFO)
	{
		m_warnInfo = warnInfo;

		m_yes.OnClick = Callback(this, &CWarnIniChangeDlg::YesNo_OnClick);
		m_no.OnClick = Callback(this, &CWarnIniChangeDlg::YesNo_OnClick);
	}
};

//////////////////////////////////////////////////////

class CIniImportDoneDlg : public CDlgBase
{
	wchar_t *m_path;

	CCtrlButton m_delete;
	CCtrlButton m_leave;
	CCtrlButton m_recycle;
	CCtrlButton m_move;

	CCtrlBase m_iniPath;
	CCtrlEdit m_newPath;

protected:
	bool OnInitDialog() override
	{
		m_iniPath.SetText(m_path);
		m_newPath.SetText(m_path);
		return true;
	}

	void Delete_OnClick(CCtrlBase*)
	{
		ptrW szIniPath(m_iniPath.GetText());
		MyDeleteFile(szIniPath);
		Close();
	}

	void Leave_OnClick(CCtrlBase*)
	{
		Close();
	}

	void Recycle_OnClick(CCtrlBase*)
	{
		DeleteDirectoryTreeW(ptrW(m_iniPath.GetText()), true);
		Close();
	}

	void Move_OnClick(CCtrlBase*)
	{
		ptrW szIniPath(m_iniPath.GetText());
		ptrW szNewPath(m_newPath.GetText());
		MoveFile(szIniPath, szNewPath);
		Close();
	}

public:
	CIniImportDoneDlg(wchar_t *path) :
		CDlgBase(g_plugin, IDD_INIIMPORTDONE),
		m_delete(this, IDC_DELETE), m_leave(this, IDC_LEAVE),
		m_recycle(this, IDC_RECYCLE), m_move(this, IDC_MOVE),
		m_iniPath(this, IDC_ININAME), m_newPath(this, IDC_NEWNAME)
	{
		m_path = path;
	}
};

//////////////////////////////////////////////////////

// settings:
struct SettingsList
{
	char *name;
	SettingsList *next;
} *setting_items = nullptr;

int SettingsEnumProc(const char *szSetting, void *)
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
			memmove(pstr + 1, pstr + 2, mir_strlen(pstr + 2) + 1);
		}
	}
}

struct ESFDParam
{
	LIST<char> *pList;
	const char *pMask;
};

static int EnumSettingsForDeletion(const char *szSetting, void *param)
{
	ESFDParam *pParam = (ESFDParam*)param;
	if (wildcmpi(szSetting, pParam->pMask))
		pParam->pList->insert(mir_strdup(szSetting));
	return 0;
}

static void ProcessIniFile(wchar_t* szIniPath, char *szSafeSections, char *szUnsafeSections, int secur, bool secFN)
{
	FILE *fp = _wfopen(szIniPath, L"rt");
	if (fp == nullptr)
		return;

	bool warnThisSection = false;
	char szSection[128]; szSection[0] = 0;

	while (!feof(fp)) {
		char szLine[2048];
		if (fgets(szLine, sizeof(szLine), fp) == nullptr)
			break;
LBL_NewLine:
		size_t lineLength = mir_strlen(szLine);
		while (lineLength && (BYTE)(szLine[lineLength - 1]) <= ' ')
			szLine[--lineLength] = '\0';

		if (szLine[0] == ';' || szLine[0] <= ' ')
			continue;

		if (szLine[0] == '[') {
			char *szEnd = strchr(szLine + 1, ']');
			if (szEnd == nullptr)
				continue;

			if (szLine[1] == '!')
				szSection[0] = '\0';
			else {
				mir_strncpy(szSection, szLine + 1, min(sizeof(szSection), (size_t)(szEnd - szLine)));
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
			if (szLine[1] == '?' || szLine[1] == '-') {
				mir_strncpy(szSection, szLine + 2, min(sizeof(szSection), (size_t)(szEnd - szLine - 1)));
				db_enum_settings(0, SettingsEnumProc, szSection);
				while (setting_items) {
					SettingsList *next = setting_items->next;

					db_unset(0, szSection, setting_items->name);

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
		if (szValue == nullptr)
			continue;

		char szName[128];
		mir_strncpy(szName, szLine, min(sizeof(szName), (size_t)(szValue - szLine + 1)));
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
			CWarnIniChangeDlg dlg(&warnInfo);
			if (warnThisSection && IDNO == dlg.DoModal())
				continue;
			if (warnInfo.cancel)
				break;
			if (warnInfo.warnNoMore)
				warnThisSection = 0;
		}

		switch (szValue[0]) {
		case 'b':
		case 'B':
			db_set_b(0, szSection, szName, (BYTE)strtol(szValue + 1, nullptr, 0));
			break;
		case 'w':
		case 'W':
			db_set_w(0, szSection, szName, (WORD)strtol(szValue + 1, nullptr, 0));
			break;
		case 'd':
		case 'D':
			db_set_dw(0, szSection, szName, (DWORD)strtoul(szValue + 1, nullptr, 0));
			break;
		case 'l':
		case 'L':
		case '-':
			if (szValue[1] == '*') {
				LIST<char> arSettings(1);
				ESFDParam param = { &arSettings, szName };
				db_enum_settings(0, EnumSettingsForDeletion, szSection, &param);
				
				for (auto &it : arSettings) {
					db_unset(0, szSection, it);
					mir_free(it);
				}
			}
			db_unset(0, szSection, szName);
			break;
		case 'e':
		case 'E':
			ConvertBackslashes(szValue + 1, Langpack_GetDefaultCodePage());
		case 's':
		case 'S':
			db_set_s(0, szSection, szName, szValue + 1);
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
					memmove(pstr + 1, pstr + 2, mir_strlen(pstr + 2) + 1);
				}
			}
		case 'u':
		case 'U':
			db_set_utf(0, szSection, szName, szValue + 1);
			break;
		case 'm':
		case 'M':
			{
				CMStringA memo(szValue + 1);
				memo.Append("\r\n");
				while (fgets(szLine, sizeof(szLine), fp) != nullptr) {
					switch (szLine[0]) {
					case 0: case '\r': case '\n': case ' ': case '\t':
						break;
					default:
						db_set_utf(0, szSection, szName, memo);
						goto LBL_NewLine;
					}

					memo.Append(rtrim(szLine + 1));
					memo.Append("\r\n");
				}
				db_set_utf(0, szSection, szName, memo);
			}
			break;
		case 'n':
		case 'h':
		case 'N':
		case 'H':
			{
				int len;
				char *pszValue, *pszEnd;

				uint8_t *buf = (uint8_t*)mir_alloc(mir_strlen(szValue + 1));
				for (len = 0, pszValue = szValue + 1;; len++) {
					buf[len] = (BYTE)strtol(pszValue, &pszEnd, 0x10);
					if (pszValue == pszEnd)
						break;
					pszValue = pszEnd;
				}
				db_set_blob(0, szSection, szName, buf, len);
				mir_free(buf);
			}
			break;
		default:
			wchar_t buf[250];
			mir_snwprintf(buf, TranslateT("Invalid setting type for '%s'. The first character of every value must be b, w, d, l, s, e, u, g, h or n."), _A2T(szName).get());
			MessageBox(nullptr, buf, TranslateT("Install database settings"), MB_ICONWARNING | MB_OK);
			break;
		}
	}
	fclose(fp);
}

static void DoAutoExec(void)
{
	wchar_t szUse[7], szIniPath[MAX_PATH], szFindPath[MAX_PATH];
	wchar_t buf[2048], szSecurity[11], szOverrideSecurityFilename[MAX_PATH], szOnCreateFilename[MAX_PATH];

	Profile_GetSetting(L"AutoExec/Use", szUse, L"prompt");
	if (!mir_wstrcmpi(szUse, L"no"))
		return;
	
	Profile_GetSetting(L"AutoExec/Safe", buf, L"CLC Icons CLUI CList SkinSounds");
	ptrA szSafeSections(mir_u2a(buf));

	Profile_GetSetting(L"AutoExec/Unsafe", buf, L"Facebook GG ICQ IRC JABBER SKYPE TWITTER VKontakte");
	ptrA szUnsafeSections(mir_u2a(buf));

	Profile_GetSetting(L"AutoExec/Warn", szSecurity, L"notsafe");

	int secur = 0;
	if (!mir_wstrcmpi(szSecurity, L"none")) secur = 0;
	else if (!mir_wstrcmpi(szSecurity, L"notsafe")) secur = 1;
	else if (!mir_wstrcmpi(szSecurity, L"onlyunsafe")) secur = 2;

	Profile_GetSetting(L"AutoExec/OverrideSecurityFilename", szOverrideSecurityFilename);
	Profile_GetSetting(L"AutoExec/OnCreateFilename", szOnCreateFilename);
	Profile_GetSetting(L"AutoExec/Glob", szFindPath, L"autoexec_*.ini");

	if (g_bDbCreated && szOnCreateFilename[0]) {
		PathToAbsoluteW(VARSW(szOnCreateFilename), szIniPath);
		ProcessIniFile(szIniPath, szSafeSections, szUnsafeSections, 0, 1);
	}

	PathToAbsoluteW(VARSW(szFindPath), szFindPath);

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(szFindPath, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	wchar_t *str2 = wcsrchr(szFindPath, '\\');
	if (str2 == nullptr)
		szFindPath[0] = 0;
	else
		str2[1] = 0;

	do {
		bool secFN = mir_wstrcmpi(fd.cFileName, szOverrideSecurityFilename) == 0;

		mir_snwprintf(szIniPath, L"%s%s", szFindPath, fd.cFileName);
		if (!mir_wstrcmpi(szUse, L"prompt") && !secFN) {
			CInstallIniDlg dlg(szIniPath);
			int result = dlg.DoModal();
			if (result == IDC_NOTOALL) break;
			if (result == IDCANCEL) continue;
		}

		ProcessIniFile(szIniPath, szSafeSections, szUnsafeSections, secur, secFN);

		if (secFN)
			MyDeleteFile(szIniPath);
		else {
			wchar_t szOnCompletion[8];
			Profile_GetSetting(L"AutoExec/OnCompletion", szOnCompletion, L"recycle");
			if (!mir_wstrcmpi(szOnCompletion, L"delete"))
				MyDeleteFile(szIniPath);
			else if (!mir_wstrcmpi(szOnCompletion, L"recycle")) {
				DeleteDirectoryTreeW(szIniPath, true);
			}
			else if (!mir_wstrcmpi(szOnCompletion, L"rename")) {
				wchar_t szRenamePrefix[MAX_PATH], szNewPath[MAX_PATH];
				Profile_GetSetting(L"AutoExec/RenamePrefix", szRenamePrefix, L"done_");
				mir_wstrcpy(szNewPath, szFindPath);
				mir_wstrcat(szNewPath, szRenamePrefix);
				mir_wstrcat(szNewPath, fd.cFileName);
				MoveFile(szIniPath, szNewPath);
			}
			else if (!mir_wstrcmpi(szOnCompletion, L"ask")) {
				CIniImportDoneDlg dlg(szIniPath);
				dlg.DoModal();
			}
		}
	} while (FindNextFile(hFind, &fd));

	FindClose(hFind);
}

static void CALLBACK CheckIniImportNow()
{
	DoAutoExec();
	FindNextChangeNotification(hIniChangeNotification);
}

static INT_PTR ImportINI(WPARAM wParam, LPARAM)
{
	ptrW tszIniPath(mir_utf8decodeW((char*)wParam));
	ProcessIniFile(tszIniPath, "", "", 0, 0);
	return 0;
}

int InitIni(void)
{
	bModuleInitialized = true;
	CreateServiceFunction("DB/Ini/ImportFile", ImportINI);
	DoAutoExec();

	wchar_t szMirandaDir[MAX_PATH];
	PathToAbsoluteW(L".", szMirandaDir);
	hIniChangeNotification = FindFirstChangeNotification(szMirandaDir, 0, FILE_NOTIFY_CHANGE_FILE_NAME);
	if (hIniChangeNotification != INVALID_HANDLE_VALUE)
		Miranda_WaitOnHandle(CheckIniImportNow, hIniChangeNotification);

	return 0;
}

void UninitIni(void)
{
	if (!bModuleInitialized)
		return;

	FindCloseChangeNotification(hIniChangeNotification);
}
