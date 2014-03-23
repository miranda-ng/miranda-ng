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
#include "langpack.h"

static void DisplayPackInfo(HWND hwndDlg, const LANGPACK_INFO *pack)
{
	/* locale string */
	if (!(pack->flags & LPF_NOLOCALE)) {
		TCHAR szLocaleName[256], szLanguageName[128], szContryName[128];

		if (!GetLocaleInfo(pack->Locale, WINVER >= _WIN32_WINNT_WIN7 ? LOCALE_SENGLISHLANGUAGENAME : LOCALE_SENGLANGUAGE, szLanguageName, SIZEOF(szLanguageName)))
			szLanguageName[0] = _T('\0');
		if (!GetLocaleInfo(pack->Locale, WINVER >= _WIN32_WINNT_WIN7 ? LOCALE_SENGLISHCOUNTRYNAME : LOCALE_SENGCOUNTRY, szContryName, SIZEOF(szContryName)))
			szContryName[0] = _T('\0');
		
		/* add some note if its incompatible */
		if (szLanguageName[0] && szContryName[0]) {
			mir_sntprintf(szLocaleName, SIZEOF(szLocaleName), _T("%s (%s)"), TranslateTS(szLanguageName), TranslateTS(szContryName));
			if (!IsValidLocale(pack->Locale, LCID_INSTALLED)) {
				TCHAR *pszIncompat;
				pszIncompat = TranslateT("(incompatible)");
				szLocaleName[SIZEOF(szLocaleName) - lstrlen(pszIncompat) - 1] = 0;
				lstrcat(lstrcat(szLocaleName, _T(" ")), pszIncompat);
			}
			SetDlgItemText(hwndDlg, IDC_LANGLOCALE, szLocaleName);
		}
		else SetDlgItemText(hwndDlg, IDC_LANGLOCALE, TranslateT("Unknown"));
	}
	else SetDlgItemText(hwndDlg, IDC_LANGLOCALE, TranslateT("Unknown"));
	
	/* file date */
	SYSTEMTIME stFileDate;
	TCHAR szDate[128]; szDate[0] = 0;
	if (FileTimeToSystemTime(&pack->ftFileDate, &stFileDate))
		GetDateFormat((LCID)CallService(MS_LANGPACK_GETLOCALE, 0, 0), DATE_SHORTDATE, &stFileDate, NULL, szDate, SIZEOF(szDate));
	SetDlgItemText(hwndDlg, IDC_LANGDATE, szDate);
	
	/* plugins included */
	SetDlgItemTextA(hwndDlg, IDC_PLUGINSINCLUDED, pack->szPluginsIncluded);
	if (pack->szPluginsIncluded[0]) {
		if (!IsWindowVisible(GetDlgItem(hwndDlg, IDC_PLUGINSINCLUDEDLABEL))) {
			ShowWindow(GetDlgItem(hwndDlg,IDC_PLUGINSINCLUDEDLABEL), SW_SHOW);
			ShowWindow(GetDlgItem(hwndDlg, IDC_PLUGINSINCLUDED), SW_SHOW);
		}
	}
	else {
		ShowWindow(GetDlgItem(hwndDlg, IDC_PLUGINSINCLUDEDLABEL), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_PLUGINSINCLUDED), SW_HIDE);
	}
	
	/* general */
	SetDlgItemTextA(hwndDlg, IDC_LANGMODUSING, pack->szLastModifiedUsing);
	SetDlgItemTextA(hwndDlg, IDC_LANGAUTHORS, pack->szAuthors);
	SetDlgItemTextA(hwndDlg, IDC_LANGEMAIL, pack->szAuthorEmail);
	SetDlgItemText(hwndDlg, IDC_LANGINFOFRAME, TranslateTS(pack->tszLanguage));
}

static BOOL InsertPackItemEnumProc(LANGPACK_INFO *pack, WPARAM wParam, LPARAM lParam)
{
	LANGPACK_INFO *pack2 = (LANGPACK_INFO*)mir_alloc(sizeof(LANGPACK_INFO));
	if (pack2 == NULL) return FALSE;
	CopyMemory(pack2, pack, sizeof(LANGPACK_INFO));

	/* insert */
	TCHAR tszName[512];
	mir_sntprintf(tszName, sizeof(tszName), _T("%s [%s]"),
		TranslateTS(pack->tszLanguage),
		pack->flags & LPF_DEFAULT ? TranslateT("built-in") : pack->tszFileName);
	UINT message = pack->flags & LPF_DEFAULT ? CB_INSERTSTRING : CB_ADDSTRING;
	int idx = SendMessage((HWND)wParam, message, 0, (LPARAM)tszName);
	SendMessage((HWND)wParam, CB_SETITEMDATA, idx, (LPARAM)pack2);
	if (pack->flags & LPF_ENABLED) {
		SendMessage((HWND)wParam, CB_SETCURSEL, idx, 0);
		DisplayPackInfo(GetParent((HWND)wParam), pack);
	}

	return TRUE;
}

INT_PTR CALLBACK DlgLangpackOpt(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LANGUAGES);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		ComboBox_ResetContent(hwndList);
		EnumLangpacks(InsertPackItemEnumProc, (WPARAM)hwndList, (LPARAM)0);
		return TRUE;

	case WM_DESTROY:
		{
			int count = ListBox_GetCount(hwndList);
			for (int i = 0; i < count; i++)
				mir_free((LANGPACK_INFO*)ListBox_GetItemData(hwndList, i));
			ComboBox_ResetContent(hwndList);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_LANGEMAIL:
			{
				char buf[512];
				lstrcpyA(buf, "mailto:");
				if (GetWindowTextA(GetDlgItem(hwndDlg, LOWORD(wParam)), &buf[7], sizeof(buf)-7))
					CallService(MS_UTILS_OPENURL, FALSE, (LPARAM)buf);
			}
			break;

		case IDC_MORELANG:
			CallService(MS_UTILS_OPENURL, TRUE, (LPARAM)"http://wiki.miranda-ng.org/index.php?title=Langpacks/en#Download");
			break;

		case IDC_LANGUAGES:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				int idx = ComboBox_GetCurSel(hwndList);
				LANGPACK_INFO *pack = (LANGPACK_INFO*)ComboBox_GetItemData(hwndList, idx);
				DisplayPackInfo(hwndDlg, pack);
				if (!(pack->flags & LPF_ENABLED))
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		}
		break;

	case WM_NOTIFY:
		if (LPNMHDR(lParam)->code == PSN_APPLY) {
			TCHAR tszPath[MAX_PATH]; tszPath[0] = 0;
			int idx = ComboBox_GetCurSel(hwndList);
			int count = ComboBox_GetCount(hwndList);
			for (int i = 0; i < count; i++) {
				LANGPACK_INFO *pack = (LANGPACK_INFO*)ComboBox_GetItemData(hwndList, idx);
				if (i == idx) {
					db_set_ws(NULL, "Langpack", "Current", pack->tszFileName);
					lstrcpy(tszPath, pack->tszFullPath);
					pack->flags |= LPF_ENABLED;
				}
				else pack->flags &= ~LPF_ENABLED;
			}

			if (tszPath[0]) {
				CallService(MS_LANGPACK_RELOAD, 0, (LPARAM)tszPath);
				CloseWindow(GetParent(hwndDlg));
				DestroyWindow(GetParent(hwndDlg));
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

int LangpackOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.pfnDlgProc = DlgLangpackOpt;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_LANGUAGES);
	odp.position = -1300000000;
	odp.pszTitle = LPGEN("Languages");
	odp.pszGroup = LPGEN("Customize");
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}
