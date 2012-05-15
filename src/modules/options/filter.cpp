/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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

#include "commonheaders.h"
#include "filter.h"

HANDLE hOptionsInitialize;

int HookFilterEvents()
{
	hOptionsInitialize = HookEvent(ME_OPT_INITIALISE, OnOptionsInitialise);
	return 0;
}

int UnhookFilterEvents()
{
	UnhookEvent(hOptionsInitialize);
	return 0;
}

INT_PTR CALLBACK DlgProcOptSearch(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWnd);
		
		CheckDlgButton(hWnd, IDC_ENABLE_KEYWORDFILTERING, DBGetContactSettingWord(NULL, "Options", "EnableKeywordFiltering", TRUE) ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;
	
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ENABLE_KEYWORDFILTERING:
			SendMessage(GetParent(hWnd), PSM_CHANGED,0,0);
			break;
		}
		break;

	case WM_SETFOCUS:
		SetFocus(GetDlgItem(hWnd, IDC_ENABLE_KEYWORDFILTERING));
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				DBWriteContactSettingWord(NULL, "Options", "EnableKeywordFiltering", IsDlgButtonChecked(hWnd, IDC_ENABLE_KEYWORDFILTERING));
				break;
			}
			break;
		}
		break;
	}
	
	return 0;
}

int OnOptionsInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {0};
	
	odp.cbSize = sizeof(odp);
	odp.position = -190000000;
	odp.hInstance = hMirandaInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_KEYWORDFILTER);
	odp.ptszTitle = TranslateT("Options search");
	odp.ptszGroup = TranslateT("Customize");
	odp.groupPosition = 810000000;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.pfnDlgProc = DlgProcOptSearch;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	
	return 0;
}

CPageList filterStrings(1);

void AddFilterString(const PageHash key, TCHAR *data)
{
	if (ContainsFilterString(key, data)) return;

	CPageKeywords * values = filterStrings[key];
	if ( values == NULL ) {
		values = new CPageKeywords( key );
		filterStrings.insert( values );
	}
	values->AddKeyWord( data );
}

void ClearFilterStrings()
{
    filterStrings.destroy();
}

BOOL ContainsFilterString(const PageHash key, TCHAR *data)
{
	CPageKeywords* values = filterStrings[key];
	return (values) ? values->ContainsString( data ) : FALSE;	
}

void AddTreeViewNodes(HWND hWndDlg, PageHash key, HTREEITEM root)
{
	if (root) {
		TCHAR title[2048] = {0};
		
		TVITEM item = {0};
		item.mask = TVIF_TEXT;
		item.hItem = root;
		item.pszText = title;
		item.cchTextMax = SIZEOF(title);
		
		if (TreeView_GetItem(hWndDlg, &item))
			if (_tcslen(title) > 0)
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
	TCHAR title[2048];
	GetWindowText(hWndDlg, title, SIZEOF( title ));
	if (_tcslen(title) > 0)
		AddFilterString(key, title);
	
	TCHAR szClass[64];
	GetClassName(hWndDlg,szClass, SIZEOF(szClass));
	
	if (lstrcmpi(szClass, _T("SysTreeView32")) == 0) {
		HTREEITEM hItem = TreeView_GetRoot(hWndDlg);
		AddTreeViewNodes(hWndDlg, key, hItem);
	}
	else {
		if (lstrcmpi(szClass, _T("listbox")) == 0) {
			if (GetWindowStyle(hWndDlg) & LBS_HASSTRINGS) {
				int count = ListBox_GetCount(hWndDlg);
				for (int i = 0; i < count; i++) {
					title[0] = 0; //safety
					int res = ListBox_GetText(hWndDlg, i, title);
					if (res != LB_ERR) {
						title[SIZEOF(title) - 1] = 0;
						if (_tcslen(title) > 0)
							AddFilterString(key, title);
			}	}	}
		}
		else {
			if (lstrcmpi(szClass, _T("SysListView32")) == 0) {
				int count = ListView_GetItemCount(hWndDlg);
				for (int i = 0; i < count; i++) {
					title[0] = 0; //safety
					ListView_GetItemText(hWndDlg, i, 0, title, SIZEOF(title));
					
					if (_tcslen(title) > 0)
						AddFilterString(key, title);
			}	}

			if (lstrcmpi(szClass, _T("combobox")) == 0) {
				if (GetWindowStyle(hWndDlg) & CBS_HASSTRINGS) { 
					int count = ComboBox_GetCount(hWndDlg);
					for (int i = 0; i < count; i++) {
						title[0] = 0; //safety
						int res = ComboBox_GetLBText(hWndDlg, i, title);
						if (res != CB_ERR) {
							title[SIZEOF(title) - 1] = 0;
							
							if (_tcslen(title) > 0)
								AddFilterString(key, title);
}	}	}	}	}	}	}

static BOOL CALLBACK GetDialogStringsCallback(HWND hWnd,LPARAM lParam)
{
	AddDialogString(hWnd, lParam);
	
	return TRUE;
}

void GetDialogStrings(int enableKeywordFiltering, const PageHash key, TCHAR *pluginName, HWND hWnd, TCHAR * group, TCHAR * title, TCHAR * tab, TCHAR * name )
{
	AddFilterString(key, pluginName); //add the plugin name as keyword
	if ( group ) AddFilterString(key, group);
	if ( title ) AddFilterString(key, title);
	if ( tab )   AddFilterString(key, tab);
	if ( name )  AddFilterString(key, name);

	if ((enableKeywordFiltering) && (hWnd != 0)) {
		AddDialogString(hWnd, key);
		
		EnumChildWindows(hWnd, GetDialogStringsCallback, (LPARAM) key);
	}
}
