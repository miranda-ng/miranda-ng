/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org),
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
#include "filter.h"

CPageList filterStrings(1);

void AddFilterString(const PageHash key, wchar_t *data)
{
	if (ContainsFilterString(key, data)) return;

	CPageKeywords *values = filterStrings[key];
	if (values == nullptr) {
		values = new CPageKeywords(key);
		filterStrings.insert(values);
	}
	values->AddKeyWord(data);
}

void ClearFilterStrings()
{
	filterStrings.destroy();
}

BOOL ContainsFilterString(const PageHash key, wchar_t *data)
{
	CPageKeywords *values = filterStrings[key];
	return (values) ? values->ContainsString(data) : FALSE;
}

void AddTreeViewNodes(HWND hWndDlg, PageHash key, HTREEITEM root)
{
	if (root) {
		wchar_t title[2048] = { 0 };

		TVITEM item = { 0 };
		item.mask = TVIF_TEXT;
		item.hItem = root;
		item.pszText = title;
		item.cchTextMax = _countof(title);

		if (TreeView_GetItem(hWndDlg, &item))
			if (mir_wstrlen(title) > 0)
				AddFilterString(key, title);

		HTREEITEM child = root;
		while (child) {
			child = TreeView_GetNextItem(hWndDlg, child, TVGN_CHILD);
			AddTreeViewNodes(hWndDlg, key, child);
		}

		AddTreeViewNodes(hWndDlg, key, TreeView_GetNextSibling(hWndDlg, root));
	}
}

void AddDialogString(HWND hWndDlg, const PageHash key)
{
	wchar_t title[2048];
	GetWindowText(hWndDlg, title, _countof(title));
	if (mir_wstrlen(title) > 0)
		AddFilterString(key, title);

	wchar_t szClass[64];
	GetClassName(hWndDlg, szClass, _countof(szClass));

	if (mir_wstrcmpi(szClass, L"SysTreeView32") == 0) {
		HTREEITEM hItem = TreeView_GetRoot(hWndDlg);
		AddTreeViewNodes(hWndDlg, key, hItem);
		return;
	}

	if (mir_wstrcmpi(szClass, L"listbox") == 0) {
		if (GetWindowStyle(hWndDlg) & LBS_HASSTRINGS) {
			int count = ListBox_GetCount(hWndDlg);
			for (int i = 0; i < count; i++) {
				title[0] = 0; //safety
				int res = ListBox_GetText(hWndDlg, i, title);
				if (res != LB_ERR) {
					title[_countof(title) - 1] = 0;
					if (mir_wstrlen(title) > 0)
						AddFilterString(key, title);
				}
			}
		}
		return;
	}

	if (mir_wstrcmpi(szClass, L"SysListView32") == 0) {
		int count = ListView_GetItemCount(hWndDlg);
		for (int i = 0; i < count; i++) {
			title[0] = 0; //safety
			ListView_GetItemText(hWndDlg, i, 0, title, _countof(title));

			if (mir_wstrlen(title) > 0)
				AddFilterString(key, title);
		}
		return;
	}

	if (mir_wstrcmpi(szClass, L"combobox") == 0) {
		if (GetWindowStyle(hWndDlg) & CBS_HASSTRINGS) {
			int count = ComboBox_GetCount(hWndDlg);
			for (int i = 0; i < count; i++) {
				title[0] = 0; //safety
				int res = ComboBox_GetLBText(hWndDlg, i, title);
				if (res != CB_ERR) {
					title[_countof(title) - 1] = 0;

					if (mir_wstrlen(title) > 0)
						AddFilterString(key, title);
				}
			}
		}
	}
}

static BOOL CALLBACK GetDialogStringsCallback(HWND hWnd, LPARAM lParam)
{
	AddDialogString(hWnd, lParam);

	return TRUE;
}

void GetDialogStrings(int enableKeywordFiltering, const PageHash key, wchar_t *pluginName, HWND hWnd, wchar_t *group, wchar_t *title, wchar_t *tab, wchar_t *name)
{
	AddFilterString(key, pluginName); //add the plugin name as keyword
	if (group) AddFilterString(key, group);
	if (title) AddFilterString(key, title);
	if (tab)   AddFilterString(key, tab);
	if (name)  AddFilterString(key, name);

	if ((enableKeywordFiltering) && (hWnd != nullptr)) {
		AddDialogString(hWnd, key);

		EnumChildWindows(hWnd, GetDialogStringsCallback, (LPARAM)key);
	}
}
