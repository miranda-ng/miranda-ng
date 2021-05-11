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
#include "langpack.h"

static void CALLBACK OpenOptions(void*)
{
	g_plugin.openOptions(L"Customize", L"Languages");
}

static void ReloadOptions(void *hWnd)
{
	while (IsWindow((HWND)hWnd))
		Sleep(50);

	CallFunctionAsync(OpenOptions, nullptr);
}

MIR_CORE_DLL(int) LoadLangPackDescr(const wchar_t *szLangPack, LANGPACK_INFO *lpInfo);

class CLangpackDlg : public CDlgBase
{
	CCtrlCombo m_languages;
	CCtrlBase m_infoFrame;
	CCtrlBase m_authors;
	CCtrlBase m_lastModUsing;
	CCtrlBase m_date;
	CCtrlBase m_locale;
	CCtrlButton m_reload;

	CCtrlHyperlink m_more;

	void LoadLangpacks();
	void LoadLangpack(LANGPACK_INFO *pack);
	void DisplayPackInfo(const LANGPACK_INFO *pack);

protected:
	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;

	void Languages_OnChange(CCtrlBase*);
	void Reload_OnClick(CCtrlBase*);

public:
	CLangpackDlg();
};

CLangpackDlg::CLangpackDlg()
	: CDlgBase(g_plugin, IDD_OPT_LANGUAGES),
	m_languages(this, IDC_LANGUAGES), m_infoFrame(this, IDC_LANGINFOFRAME),
	m_authors(this, IDC_LANGAUTHORS),
	m_locale(this, IDC_LANGLOCALE), m_lastModUsing(this, IDC_LANGMODUSING),
	m_date(this, IDC_LANGDATE), m_reload(this, IDC_RELOAD),
	m_more(this, IDC_MORELANG, "https://wiki.miranda-ng.org/index.php?title=Langpacks/en#Download")
{
	m_languages.OnChange = Callback(this, &CLangpackDlg::Languages_OnChange);
	m_reload.OnClick = Callback(this, &CLangpackDlg::Reload_OnClick);
}

bool CLangpackDlg::OnInitDialog()
{
	m_languages.ResetContent();
	LoadLangpacks();
	return true;
}

void CLangpackDlg::LoadLangpacks()
{
	ptrW langpack(db_get_wsa(0, "Langpack", "Current"));

	wchar_t tszFullPath[MAX_PATH];
	PathToAbsoluteW(L"\\Languages\\langpack_*.txt", tszFullPath);

	bool isPackFound = false;
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(tszFullPath, &wfd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			PathToAbsoluteW(L"\\Languages\\", tszFullPath);
			mir_wstrcat(tszFullPath, wfd.cFileName);

			LANGPACK_INFO pack;
			if (!LoadLangPackDescr(tszFullPath, &pack)) {
				pack.ftFileDate = wfd.ftLastWriteTime;
				if (langpack && !mir_wstrcmpi(langpack, wfd.cFileName)) {
					if (!isPackFound) pack.flags |= LPF_ENABLED;
					isPackFound = true;
				}
				LoadLangpack(&pack);
			}
		} while (FindNextFile(hFind, &wfd));
		FindClose(hFind);
	}

	{ // default langpack: English
		LANGPACK_INFO pack;
		pack.flags = LPF_DEFAULT;
		pack.Locale = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
		mir_wstrcpy(pack.tszLanguage, L"English");
		pack.szAuthors = "Miranda NG team";
		DWORD v = Miranda_GetVersion();
		pack.szLastModifiedUsing.Format("%d.%d.%d", ((v >> 24) & 0xFF), ((v >> 16) & 0xFF), ((v >> 8) & 0xFF));

		if (GetModuleFileName(nullptr, pack.tszFullPath, _countof(pack.tszFullPath))) {
			mir_wstrcpy(pack.tszFileName, L"default");
			HANDLE hFile = CreateFile(pack.tszFileName, 0, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
			if (hFile != INVALID_HANDLE_VALUE) {
				GetFileTime(hFile, nullptr, nullptr, &pack.ftFileDate);
				CloseHandle(hFile);
			}
		}

		if (!isPackFound)
			pack.flags |= LPF_ENABLED;

		LoadLangpack(&pack);
	}
}

void CLangpackDlg::LoadLangpack(LANGPACK_INFO *pack)
{
	LANGPACK_INFO *pack2 = new LANGPACK_INFO();
	*pack2 = *pack;

	CMStringW wszName(FORMAT, L"%s [%s]", pack->tszLanguage, (pack->flags & LPF_DEFAULT) ? TranslateT("built-in") : pack->tszFileName);

	int idx = (pack->flags & LPF_DEFAULT)
		? m_languages.InsertString(wszName, 0, (LPARAM)pack2)
		: m_languages.AddString(wszName, (LPARAM)pack2);
	if (pack->flags & LPF_ENABLED) {
		m_languages.SetCurSel(idx);
		DisplayPackInfo(pack);
		m_reload.Enable(!(pack->flags & LPF_DEFAULT));
	}
}

void CLangpackDlg::DisplayPackInfo(const LANGPACK_INFO *pack)
{
	if (!(pack->flags & LPF_NOLOCALE)) {
		wchar_t szLocaleName[256], szLanguageName[128], szContryName[128];

		if (!GetLocaleInfo(pack->Locale, WINVER >= _WIN32_WINNT_WIN7 ? LOCALE_SENGLISHLANGUAGENAME : LOCALE_SENGLANGUAGE, szLanguageName, _countof(szLanguageName)))
			szLanguageName[0] = '\0';
		if (!GetLocaleInfo(pack->Locale, WINVER >= _WIN32_WINNT_WIN7 ? LOCALE_SENGLISHCOUNTRYNAME : LOCALE_SENGCOUNTRY, szContryName, _countof(szContryName)))
			szContryName[0] = '\0';

		if (szLanguageName[0] && szContryName[0]) {
			mir_snwprintf(szLocaleName, L"%s (%s)", TranslateW(szLanguageName), TranslateW(szContryName));
			if (!IsValidLocale(pack->Locale, LCID_INSTALLED)) {
				wchar_t *pszIncompat;
				pszIncompat = TranslateT("(incompatible)");
				szLocaleName[_countof(szLocaleName) - mir_wstrlen(pszIncompat) - 1] = 0;
				mir_wstrcat(mir_wstrcat(szLocaleName, L" "), pszIncompat);
			}
			m_locale.SetText(szLocaleName);
		}
		else m_locale.SetText(TranslateT("Unknown"));
	}
	else m_locale.SetText(TranslateT("Unknown"));

	SYSTEMTIME stFileDate;
	wchar_t szDate[128]; szDate[0] = 0;
	if (FileTimeToSystemTime(&pack->ftFileDate, &stFileDate))
		GetDateFormat(Langpack_GetDefaultLocale(), DATE_SHORTDATE, &stFileDate, nullptr, szDate, _countof(szDate));
	m_date.SetText(szDate);

	m_lastModUsing.SetText(ptrW(mir_utf8decodeW(pack->szLastModifiedUsing)));
	m_authors.SetText(ptrW(mir_utf8decodeW(pack->szAuthors)));
	m_infoFrame.SetText(TranslateW(pack->tszLanguage));
}

void CLangpackDlg::Languages_OnChange(CCtrlBase*)
{
	LANGPACK_INFO *pack = (LANGPACK_INFO*)m_languages.GetCurData();
	DisplayPackInfo(pack);
	if (!(pack->flags & LPF_ENABLED))
		SendMessage(GetParent(GetHwnd()), PSM_CHANGED, 0, 0);
	m_reload.Enable((pack->flags & LPF_ENABLED) && !(pack->flags & LPF_DEFAULT));
}

void CLangpackDlg::Reload_OnClick(CCtrlBase*)
{
	m_reload.Enable(FALSE);
	LANGPACK_INFO *pack = (LANGPACK_INFO*)m_languages.GetCurData();
	ReloadLangpack(pack->tszFullPath);
	DisplayPackInfo(pack);
	m_reload.Enable(TRUE);
}

bool CLangpackDlg::OnApply()
{
	wchar_t tszPath[MAX_PATH]; tszPath[0] = 0;
	int idx = m_languages.GetCurSel();
	int count = m_languages.GetCount();
	for (int i = 0; i < count; i++) {
		LANGPACK_INFO *pack = (LANGPACK_INFO*)m_languages.GetItemData(i);
		if (i == idx) {
			db_set_ws(0, "Langpack", "Current", pack->tszFileName);
			mir_wstrcpy(tszPath, pack->tszFullPath);
			pack->flags |= LPF_ENABLED;
		}
		else pack->flags &= ~LPF_ENABLED;
	}

	if (tszPath[0]) {
		ReloadLangpack(tszPath);
		
		if (m_bExiting) {
			HWND hwndParent = GetParent(GetHwnd());
			PostMessage(hwndParent, WM_CLOSE, 1, 0);
			mir_forkthread(ReloadOptions, hwndParent);
		}
	}
	return true;
}

void CLangpackDlg::OnDestroy()
{
	int count = m_languages.GetCount();
	for (int i = 0; i < count; i++)
		delete (LANGPACK_INFO*)m_languages.GetItemData(i);
	m_languages.ResetContent();
}

/////////////////////////////////////////////////////////////////////////////////////////

int LangpackOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = -1300000000;
	odp.szTitle.a = LPGEN("Languages");
	odp.szGroup.a = LPGEN("Customize");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pDialog = new CLangpackDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
