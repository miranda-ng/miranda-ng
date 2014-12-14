/*
   IgnoreState plugin for Miranda-IM (www.miranda-im.org)
   (c) 2010 by Kildor

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "commonheaders.h"

#define MS_SKINENG_DRAWICONEXFIX "SkinEngine/DrawIconEx_Fix"

static bool bInitializing;

INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hTree=GetDlgItem(hwndDlg,IDC_FILTER);

	switch (msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			bInitializing = true;
			fill_filter();

			SetWindowLongPtr(hTree,GWL_STYLE,GetWindowLongPtr(hTree,GWL_STYLE)|TVS_NOHSCROLL);

			HIMAGELIST himlButtonIcons = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32|ILC_MASK,2,2);
			TreeView_SetImageList(hTree,himlButtonIcons,TVSIL_NORMAL);
			TreeView_DeleteAllItems(hTree);

			for (int i=2; i < nII; i++) { // we don`t need it IGNORE_ALL and IGNORE_MESSAGE
				TVINSERTSTRUCT tvis={0};
				int index = ImageList_AddIcon(himlButtonIcons, LoadSkinnedIcon(ii[i].icon));
				tvis.hParent=NULL;
				tvis.hInsertAfter=TVI_LAST;
				tvis.item.mask=TVIF_PARAM|TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_STATE;
				tvis.item.lParam=(LPARAM)(ii[i].type);
				tvis.item.pszText=TranslateTS(ii[i].name);
				tvis.item.iImage=tvis.item.iSelectedImage=index;
				HTREEITEM hti = TreeView_InsertItem(hTree,&tvis);
				TreeView_SetCheckState(hTree, hti, checkState(ii[i].type));
			}

			CheckDlgButton(hwndDlg, IDC_IGNORE_IGNOREALL, bUseMirandaSettings ? BST_CHECKED : BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILTER), !bUseMirandaSettings);
			bInitializing = false;
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_IGNORE_IGNOREALL:
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILTER),  BST_UNCHECKED == IsDlgButtonChecked(hwndDlg,IDC_IGNORE_IGNOREALL));
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				{
					DWORD flags=0;
					TVITEM tvi;
					tvi.mask=TVIF_HANDLE|TBIF_LPARAM;
					HWND hTree=GetDlgItem(hwndDlg,IDC_FILTER);
					tvi.hItem=TreeView_GetRoot(hTree); //check ignore all
					while(tvi.hItem) {
						TreeView_GetItem(hTree,&tvi);
						if(TreeView_GetCheckState(hTree,tvi.hItem)) flags|=1<<(tvi.lParam-1);
						tvi.hItem=TreeView_GetNextSibling(hTree,tvi.hItem);
					}
					db_set_dw(NULL, MODULENAME, "Filter", flags);

					bUseMirandaSettings = IsDlgButtonChecked(hwndDlg,IDC_IGNORE_IGNOREALL) ? 1 : 0;
					db_set_b(NULL, MODULENAME, "UseMirandaSettings", bUseMirandaSettings);

					fill_filter();
				}
			}
		case IDC_FILTER:
			if (((LPNMHDR)lParam)->code == NM_CLICK)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
	}
	return FALSE;
}

int onOptInitialise(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = g_hInst;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pszGroup = LPGEN("Icons");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_IGNORE_OPT);
	odp.pszTitle = LPGEN("Ignore State");
	odp.pfnDlgProc = DlgProcOptions;
	Options_AddPage(wParam, &odp);
	return 0;
}
