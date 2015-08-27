/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG Team
Copyright (c) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (c) 2002-2005 Calvin Che

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


/*
This file contain the source for displaying information for the
ini files, as well as function that are used for debug purpose
regrading the loading of ini contents
*/

#include "stdafx.h"

//============  INI INFORMATION  ============

// List INI Information for all loaded INI files
void INIInfo(HWND hwndDlg)
{
	TCHAR str[16];
	size_t memused = 0;


	HWND hIniList = GetDlgItem(hwndDlg, IDC_INFOLIST);

	ListView_DeleteAllItems(hIniList);

	LVITEM lvi = { 0 };
	lvi.mask = LVIF_TEXT;
	lvi.iItem = 0;
	for (WIDATALIST *Item = WIHead; Item != NULL; Item = Item->next) {
		// get the data for the ini file
		lvi.iSubItem = 0;
		lvi.pszText = Item->Data.InternalName;
		ListView_InsertItem(hIniList, &lvi);
		lvi.iSubItem = 1;
		lvi.pszText = Item->Data.Author;
		ListView_SetItem(hIniList, &lvi);
		lvi.iSubItem = 2;
		lvi.pszText = Item->Data.Version;
		ListView_SetItem(hIniList, &lvi);
		lvi.iSubItem = 3;
		switch (Item->Data.InternalVer) {
		case 1:  lvi.pszText = _T("1.0");  break;
		case 2:  lvi.pszText = _T("1.1");  break;
		case 3:  lvi.pszText = _T("1.1a"); break;
		case 4:  lvi.pszText = _T("1.2");  break;
		case 5:  lvi.pszText = _T("1.3");  break;
		case 6:  lvi.pszText = _T("1.4");  break;
		case 7:  lvi.pszText = _T("1.5");  break;
		default: lvi.pszText = _T("");     break;
		}
		ListView_SetItem(hIniList, &lvi);
		lvi.iSubItem = 4;
		lvi.pszText = _ltot(Item->Data.UpdateDataCount, str, 10);
		ListView_SetItem(hIniList, &lvi);
		lvi.iSubItem = 5;
		lvi.pszText = Item->Data.DisplayName;
		ListView_SetItem(hIniList, &lvi);
		lvi.iSubItem = 6;
		lvi.pszText = Item->Data.ShortFileName;
		ListView_SetItem(hIniList, &lvi);

		memused += Item->Data.MemUsed;

		++lvi.iItem;
	}
	SetDlgItemText(hwndDlg, IDC_INICOUNT, _itot(lvi.iItem, str, 10));
	SetDlgItemText(hwndDlg, IDC_MEMUSED, _ltot((long)memused, str, 10));
}

static const struct tag_Columns
{
	const TCHAR *name;
	unsigned size;
}
columns[] =
{
	{ LPGENT("Name"), 70 },
	{ LPGENT("Author"), 100 },
	{ LPGENT("File Version"), 70 },
	{ LPGENT("INI Version"), 70 },
	{ LPGENT("Items"), 40 },
	{ LPGENT("Display Name"), 200 },
	{ LPGENT("File Name"), 150 },
};


INT_PTR CALLBACK DlgProcINIPage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			HWND hIniList = GetDlgItem(hwndDlg, IDC_INFOLIST);
			LVCOLUMN lvc = { 0 };

			lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
			lvc.fmt = LVCFMT_LEFT;
			for (int i = 0; i < 7; ++i) {
				lvc.iSubItem = i;
				lvc.pszText = TranslateTS(columns[i].name);
				lvc.cx = columns[i].size;
				ListView_InsertColumn(hIniList, i, &lvc);
			}
			INIInfo(hwndDlg);
		}

		break;

	case WM_DESTROY:
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED &&
			LOWORD(wParam) == IDC_RELOADINI) {
			DestroyWIList();
			LoadWIData(true);
			INIInfo(hwndDlg);
		}
		break;
	}
	return 0;
}


// get the info of individual ini file
// pszSvc = the internal name of the service to get the data
void GetINIInfo(TCHAR *pszSvc)
{
	TCHAR str2[2048];
	WIDATA *sData = GetWIData(pszSvc);
	// if the service does not exist among the loaded INI's
	if (sData == NULL) {
		mir_sntprintf(str2, _countof(str2), TranslateT("The corresponding INI file for \"%s\" is not found."), pszSvc);
		MessageBox(NULL, str2, TranslateT("Weather INI information"), MB_OK | MB_ICONINFORMATION);
	}
	// if exist, get the information
	else {
		mir_sntprintf(str2, _countof(str2), TranslateT("Weather INI information for \"%s\":"), pszSvc);
		mir_tstrncat(str2, _T("\n\n"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, TranslateT("Name:"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, _T("\t\t"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, sData->DisplayName, _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, _T("\n"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, TranslateT("Internal Name:"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, _T("\t"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, sData->InternalName, _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, _T("\n"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, TranslateT("Author:"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, _T("\t\t"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, sData->Author, _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, _T("\n"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, TranslateT("Version:"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, _T("\t\t"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, sData->Version, _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, _T("\n"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, TranslateT("INI Version:"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, _T("\t"), _countof(str2) - mir_tstrlen(str2));
		switch (sData->InternalVer) {
		case 1: mir_tstrncat(str2, _T("1.0"), _countof(str2) - mir_tstrlen(str2)); break;
		case 2: mir_tstrncat(str2, _T("1.1"), _countof(str2) - mir_tstrlen(str2)); break;
		case 3: mir_tstrncat(str2, _T("1.1a"), _countof(str2) - mir_tstrlen(str2)); break;
		case 4: mir_tstrncat(str2, _T("1.2"), _countof(str2) - mir_tstrlen(str2)); break;
		case 5: mir_tstrncat(str2, _T("1.3"), _countof(str2) - mir_tstrlen(str2)); break;
		case 6: mir_tstrncat(str2, _T("1.4"), _countof(str2) - mir_tstrlen(str2)); break;
		case 7: mir_tstrncat(str2, _T("1.5"), _countof(str2) - mir_tstrlen(str2)); break;
		}
		mir_tstrncat(str2, _T("\n"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, TranslateT("File Name:"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, _T("\t"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, sData->ShortFileName, _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, _T("\n"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, TranslateT("Item Count:"), _countof(str2) - mir_tstrlen(str2));
		mir_sntprintf(str2, _countof(str2), _T("%s\t%i\n"), str2, sData->UpdateDataCount);
		mir_tstrncat(str2, TranslateT("Memory Used:"), _countof(str2) - mir_tstrlen(str2));
		mir_sntprintf(str2, _countof(str2), _T("%s\t%i "), str2, sData->MemUsed);
		mir_tstrncat(str2, TranslateT("bytes"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, _T("\n\n"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, TranslateT("Description:"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, _T("\n"), _countof(str2) - mir_tstrlen(str2));
		mir_tstrncat(str2, sData->Description, _countof(str2) - mir_tstrlen(str2));

		// display the message box and quit
		MessageBox(NULL, str2, TranslateT("Weather INI information"), MB_OK | MB_ICONINFORMATION);
	}
}

//============  DISPLAY A LIST FOR CUSTOM VARIABLES  ============

// a message box for displaying the list of custom variables
// can be found when click on "More" in text option dialog
void MoreVarList(void)
{
	TCHAR str[10240], tempstr[1024];

	// heading
	_tcsncpy(str, VARS_LIST, _countof(str) - 1);
	mir_tstrncat(str, _T("\n\n"), _countof(str) - mir_tstrlen(str));
	// loop through all weather services to find custom variables
	for (WIDATALIST *Item = WIHead; Item != NULL; Item = Item->next) {
		// loop through all update items in a service
		for (WIDATAITEMLIST *WItem = Item->Data.UpdateData; WItem != NULL; WItem = WItem->Next) {
			// the custom variable is defined as "%[<variable name>]"
			// ignore the "hi" item and hidden items
			if (mir_tstrcmp(WItem->Item.Name, _T("Ignore")) && WItem->Item.Name[0] != '#') {
				mir_sntprintf(tempstr, _countof(tempstr), _T("%c[%s]"), '%', WItem->Item.Name);
				TCHAR *find = _tcsstr(str, tempstr);
				// if the custom variable does not exist in the list, add it to the list
				if (find == NULL) {
					mir_tstrncat(str, tempstr, _countof(str) - mir_tstrlen(str));
					mir_tstrncat(str, _T(", "), _countof(str) - mir_tstrlen(str));
				}
			}
		}
	}
	// remove the last comma in the list
	TCHAR* find = _tcsrchr(str, ',');
	if (find != NULL)
		*find = '\0';

	// display the list in a message box
	MessageBox(NULL, str, TranslateT("More Variables"), MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
}

