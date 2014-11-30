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

static void SetDlgItemText_CP(HWND hwndDlg, int ctrlID, LPCSTR str)
{
	SetDlgItemText(hwndDlg, ctrlID, ptrT(mir_utf8decodeT(str)));
}

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
				szLocaleName[SIZEOF(szLocaleName) - mir_tstrlen(pszIncompat) - 1] = 0;
				mir_tstrcat(mir_tstrcat(szLocaleName, _T(" ")), pszIncompat);
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
	
	/* general */
	SetDlgItemText_CP(hwndDlg, IDC_LANGMODUSING, pack->szLastModifiedUsing);
	SetDlgItemText_CP(hwndDlg, IDC_LANGAUTHORS, pack->szAuthors);
	SetDlgItemText_CP(hwndDlg, IDC_LANGEMAIL, pack->szAuthorEmail);
	SetDlgItemText(hwndDlg, IDC_LANGINFOFRAME, TranslateTS(pack->tszLanguage));
}

static BOOL InsertPackItemEnumProc(LANGPACK_INFO *pack, WPARAM wParam, LPARAM lParam)
{
	LANGPACK_INFO *pack2 = new LANGPACK_INFO();
	*pack2 = *pack;

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
		EnableWindow(GetDlgItem(GetParent((HWND)wParam), IDC_RELOAD), !(pack->flags & LPF_DEFAULT));
	}

	return TRUE;
}

static void CALLBACK OpenOptions(void*)
{
	OPENOPTIONSDIALOG ood = { sizeof(ood) };
	ood.pszGroup = "Customize";
	ood.pszPage = "Languages";
	Options_Open(&ood);
}

static void ReloadOptions(void *hWnd)
{
	while (IsWindow((HWND)hWnd))
		Sleep(50);

	CallFunctionAsync(OpenOptions, 0);
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

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_LANGEMAIL:
			{
				char buf[512];
				mir_strcpy(buf, "mailto:");
				if (GetWindowTextA(GetDlgItem(hwndDlg, LOWORD(wParam)), &buf[7], SIZEOF(buf)-7))
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
				EnableWindow(GetDlgItem(hwndDlg, IDC_RELOAD), (pack->flags & LPF_ENABLED) && !(pack->flags & LPF_DEFAULT));
			}
			break;

		case IDC_RELOAD:
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_RELOAD), FALSE);
				int idx = ComboBox_GetCurSel(hwndList);
				LANGPACK_INFO *pack = (LANGPACK_INFO*)ComboBox_GetItemData(hwndList, idx);
				ReloadLangpack(pack->tszFullPath);
				DisplayPackInfo(hwndDlg, pack);
				EnableWindow(GetDlgItem(hwndDlg, IDC_RELOAD), TRUE);
			}
			break;
		}
		break;

	case WM_NOTIFY:
		if (LPNMHDR(lParam)->code == PSN_APPLY) {
			TCHAR tszPath[MAX_PATH]; tszPath[0] = 0;
			int idx = ComboBox_GetCurSel(hwndList);
			int count = ComboBox_GetCount(hwndList);
			for (int i = 0; i < count; i++) {
				LANGPACK_INFO *pack = (LANGPACK_INFO*)ComboBox_GetItemData(hwndList, i);
				if (i == idx) {
					db_set_ts(NULL, "Langpack", "Current", pack->tszFileName);
					mir_tstrcpy(tszPath, pack->tszFullPath);
					pack->flags |= LPF_ENABLED;
				}
				else pack->flags &= ~LPF_ENABLED;
			}

			if (tszPath[0]) {
				ReloadLangpack(tszPath);

				if (LPPSHNOTIFY(lParam)->lParam == IDC_APPLY) {
					HWND hwndParent = GetParent(hwndDlg);
					PostMessage(hwndParent, WM_CLOSE, 1, 0);
					mir_forkthread(ReloadOptions, hwndParent);
				}
			}
		}
		break;

	case WM_DESTROY:
		int count = ListBox_GetCount(hwndList);
		for (int i = 0; i < count; i++)
			delete (LANGPACK_INFO*)ListBox_GetItemData(hwndList, i);
		ComboBox_ResetContent(hwndList);
		return TRUE;
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
