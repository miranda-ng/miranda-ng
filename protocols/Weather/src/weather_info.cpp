/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG team
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
static void INIInfo(HWND hwndDlg)
{
	wchar_t str[16];
	size_t memused = 0;

	HWND hIniList = GetDlgItem(hwndDlg, IDC_INFOLIST);

	ListView_DeleteAllItems(hIniList);

	LVITEM lvi = {};
	lvi.mask = LVIF_TEXT;
	lvi.iItem = 0;
	for (WIDATALIST *Item = WIHead; Item != nullptr; Item = Item->next) {
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
		lvi.pszText = GetINIVersionNum(Item->Data.InternalVer);
		ListView_SetItem(hIniList, &lvi);
		lvi.iSubItem = 4;
		lvi.pszText = _ltow(Item->Data.UpdateDataCount, str, 10);
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
	SetDlgItemText(hwndDlg, IDC_INICOUNT, _itow(lvi.iItem, str, 10));
	SetDlgItemText(hwndDlg, IDC_MEMUSED, _ltow((long)memused, str, 10));
}

struct
{
	const wchar_t *name;
	unsigned size;
}
static columns[] =
{
	{ LPGENW("Name"), 70 },
	{ LPGENW("Author"), 100 },
	{ LPGENW("File Version"), 70 },
	{ LPGENW("INI Version"), 70 },
	{ LPGENW("Items"), 40 },
	{ LPGENW("Display Name"), 200 },
	{ LPGENW("File Name"), 150 },
};

WeatherMyDetailsDlg::WeatherMyDetailsDlg() :
	CUserInfoPageDlg(g_plugin, IDD_INFO),
	btnReload(this, IDC_RELOADINI)
{
	btnReload.OnClick = Callback(this, &WeatherMyDetailsDlg::onClick_Reload);
}

bool WeatherMyDetailsDlg::OnInitDialog()
{
	HWND hIniList = GetDlgItem(m_hwnd, IDC_INFOLIST);

	LVCOLUMN lvc = {};
	lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvc.fmt = LVCFMT_LEFT;
	for (auto &it : columns) {
		lvc.iSubItem = int(&it - columns);
		lvc.pszText = TranslateW(it.name);
		lvc.cx = it.size;
		ListView_InsertColumn(hIniList, lvc.iSubItem, &lvc);
	}
	
	INIInfo(m_hwnd);
	return true;
}

void WeatherMyDetailsDlg::onClick_Reload(CCtrlButton*)
{
	DestroyWIList();
	LoadWIData(true);
	INIInfo(m_hwnd);
}

// get the info of individual ini file
// pszSvc = the internal name of the service to get the data

wchar_t* GetINIVersionNum(int iVersion)
{
	switch (iVersion) {
	case 1: return L"1.0";
	case 2: return L"1.1";
	case 3: return L"1.1a";
	case 4: return L"1.2";
	case 5: return L"1.3";
	case 6: return L"1.4";
	case 7: return L"1.5";
	}
	return L"";
}

void GetINIInfo(wchar_t *pszSvc)
{
	CMStringW str;
	WIDATA *sData = GetWIData(pszSvc);
	// if the service does not exist among the loaded INI's
	if (sData == nullptr) {
		str.Format(TranslateT("The corresponding INI file for \"%s\" is not found."), pszSvc);
	}
	// if exist, get the information
	else {
		str.AppendFormat(TranslateT("Weather INI information for \"%s\":"), pszSvc);
		str += L"\n\n";
		str.AppendFormat(L"%s\t%s\n", TranslateT("Name:"), sData->DisplayName);
		str.AppendFormat(L"%s\t%s\n", TranslateT("Internal Name:"), sData->InternalName);
		str.AppendFormat(L"%s\t%s\n", TranslateT("Author:"), sData->Author);
		str.AppendFormat(L"%s\t%s\n", TranslateT("Version:"), sData->Version);
		str.AppendFormat(L"%s\t%s\n", TranslateT("INI Version:"), GetINIVersionNum(sData->InternalVer));
		str.AppendFormat(L"%s\t%s\n", TranslateT("File Name:"), sData->ShortFileName);
		str.AppendFormat(L"%s\t%i\n", TranslateT("Item Count:"), sData->UpdateDataCount);
		str.AppendFormat(L"%s\t%i %s\n\n", TranslateT("Memory Used:"), (int)sData->MemUsed, TranslateT("bytes"));
		str.AppendFormat(L"%s\n%s", TranslateT("Description:"), sData->Description);
	}

	MessageBox(nullptr, str, TranslateT("Weather INI information"), MB_OK | MB_ICONINFORMATION);
}

//============  DISPLAY A LIST FOR CUSTOM VARIABLES  ============
//
// a message box for displaying the list of custom variables
// can be found when click on "More" in text option dialog
void MoreVarList(void)
{
	// heading
	CMStringW str(TranslateT("Here is a list of custom variables that are currently available"));
	str += L"\n\n";

	// loop through all weather services to find custom variables
	bool bFirst = true;
	for (WIDATALIST *Item = WIHead; Item != nullptr; Item = Item->next) {
		// loop through all update items in a service
		for (WIDATAITEMLIST *WItem = Item->Data.UpdateData; WItem != nullptr; WItem = WItem->Next) {
			// the custom variable is defined as "%[<variable name>]"
			// ignore the "hi" item and hidden items
			if (mir_wstrcmp(WItem->Item.Name, L"Ignore") && WItem->Item.Name[0] != '#') {
				wchar_t tempstr[1024];
				mir_snwprintf(tempstr, L"%c[%s]", '%', WItem->Item.Name);
				auto *find = wcsstr(str, tempstr);
				// if the custom variable does not exist in the list, add it to the list
				if (find == nullptr) {
					if (bFirst)
						bFirst = false;
					else
						str += L", ";
					str += tempstr;
				}
			}
		}
	}

	// display the list in a message box
	MessageBox(nullptr, str, TranslateT("More Variables"), MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
}
