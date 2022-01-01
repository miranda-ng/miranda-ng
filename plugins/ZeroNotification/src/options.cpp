/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

struct CheckBoxValues_t
{
	uint32_t style;
	wchar_t *szDescr;
}
static const statusValues[] =
{
	{ PF2_ONLINE, TEXT("Online") },
	{ PF2_SHORTAWAY, TEXT("Away") },
	{ PF2_LONGAWAY, TEXT("Not available") },
	{ PF2_LIGHTDND, TEXT("Occupied") },
	{ PF2_HEAVYDND, TEXT("Do not disturb") },
	{ PF2_FREECHAT, TEXT("Free for chat") },
	{ PF2_INVISIBLE, TEXT("Invisible") }
};

static void FillCheckBoxTree(HWND hwndTree, const struct CheckBoxValues_t *values, int nValues, uint32_t style)
{
	TVINSERTSTRUCT tvis;
	tvis.hParent = nullptr;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
	for (int i = 0; i < nValues; i++) {
		tvis.item.lParam = values[i].style;
		tvis.item.pszText = TranslateW(values[i].szDescr);
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		tvis.item.state = INDEXTOSTATEIMAGEMASK((style & tvis.item.lParam) != 0 ? 2 : 1);
		TreeView_InsertItem(hwndTree, &tvis);
	}
}

static uint32_t MakeCheckBoxTreeFlags(HWND hwndTree)
{
	uint32_t flags = 0;

	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
	tvi.hItem = TreeView_GetRoot(hwndTree);
	while (tvi.hItem) {
		TreeView_GetItem(hwndTree, &tvi);
		if (((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2))
			flags |= tvi.lParam;
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	return flags;
}

static INT_PTR CALLBACK DlgProcNoSoundOpts(HWND hwndDlg, UINT msg, WPARAM, LPARAM lParam)
{
	uint32_t test;
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_NOSOUND), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_NOSOUND), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_NOBLINK), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_NOBLINK), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_NOCLCBLINK), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_NOCLCBLINK), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);

		FillCheckBoxTree(GetDlgItem(hwndDlg, IDC_NOSOUND), statusValues, sizeof(statusValues) / sizeof(statusValues[0]), g_plugin.getDword("NoSound", DEFAULT_NOSOUND));
		FillCheckBoxTree(GetDlgItem(hwndDlg, IDC_NOBLINK), statusValues, sizeof(statusValues) / sizeof(statusValues[0]), g_plugin.getDword("NoBlink", DEFAULT_NOBLINK));
		FillCheckBoxTree(GetDlgItem(hwndDlg, IDC_NOCLCBLINK), statusValues, sizeof(statusValues) / sizeof(statusValues[0]), g_plugin.getDword("NoCLCBlink", DEFAULT_NOCLCBLINK));
		return TRUE;
	
	case WM_COMMAND:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_NOSOUND:
		case IDC_NOBLINK:
		case IDC_NOCLCBLINK:
			if (((LPNMHDR)lParam)->code == NM_CLICK) {
				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
				if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti)) {
					if (hti.flags & TVHT_ONITEMSTATEICON) {
						TVITEM tvi;
						tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
						tvi.hItem = hti.hItem;
						TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
						tvi.iImage = tvi.iSelectedImage = tvi.iImage == 1 ? 2 : 1;
						TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
				}
			}
			break;
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				g_plugin.setDword("NoSound", MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg, IDC_NOSOUND)));
				g_plugin.setDword("NoBlink", MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg, IDC_NOBLINK)));
				g_plugin.setDword("NoCLCBlink", MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg, IDC_NOCLCBLINK)));

				test = db_get_w(0, "CList", "Status", 0);
				SetNotify(Proto_Status2Flag(db_get_w(0, "CList", "Status", 0)));
				return TRUE;
			}
			break;
		}
	}
	return FALSE;
}

int OptionsInitialize(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 100000000;
	odp.flags = ODPF_UNICODE;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_NOSOUND);
	odp.szTitle.w = LPGENW("Zero Notifications");
	odp.szGroup.w = LPGENW("Plugins");
	odp.pfnDlgProc = DlgProcNoSoundOpts;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
