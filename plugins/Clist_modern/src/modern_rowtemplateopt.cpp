/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#include "hdr/modern_commonheaders.h"
#include "hdr/modern_clist.h"
#include "hdr/modern_commonprototypes.h"
#include "hdr/modern_row.h"

void RefreshTree(HWND hwndDlg, HTREEITEM hti);
static char* rowOptTmplStr;
static ROWCELL* rowOptTmplRoot;
static ROWCELL* rowOptTA[100];
TCHAR *types[] = {
	_T("none"), _T("text1"), _T("text2"), _T("text3"), _T("status"),
	_T("avatar"), _T("extra"), _T("extra1"), _T("extra2"), _T("extra3"),
	_T("extra4"), _T("extra5"), _T("extra6"), _T("extra7"), _T("extra8"),
	_T("extra9"), _T("time"), _T("space"), _T("fspace")
};

RECT da = { 205, 58, 440, 130 }; // Draw area

void rowOptBuildTA(pROWCELL cell, pROWCELL* TA, int* i)
{
	if (!cell) return;
	TA[(*i)++] = cell;
	rowOptBuildTA(cell->child, TA, i);
	rowOptBuildTA(cell->next, TA, i);
}

void rowOptShowSettings(HWND hwnd)
{
	TVITEM tvi;
	pROWCELL cell;
	HTREEITEM node = TreeView_GetSelection(GetDlgItem(hwnd, IDC_ROWTREE));
	int param;

	tvi.hItem = node;
	tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

	TreeView_GetItem(GetDlgItem(hwnd, IDC_ROWTREE), &tvi);
	cell = (pROWCELL)tvi.lParam;

	if (!tvi.hItem)  {
		EnableWindow(GetDlgItem(hwnd, IDC_CONTTYPE), 0);
		EnableWindow(GetDlgItem(hwnd, IDC_VALIGN), 0);
		EnableWindow(GetDlgItem(hwnd, IDC_HALIGN), 0);
		EnableWindow(GetDlgItem(hwnd, IDC_CONTWIDTH), 0);
		EnableWindow(GetDlgItem(hwnd, IDC_CONTHEIGHT), 0);
		EnableWindow(GetDlgItem(hwnd, IDC_SPINCONTWIDTH), 0);
		EnableWindow(GetDlgItem(hwnd, IDC_SPINCONTHEIGHT), 0);
		SendDlgItemMessage(hwnd, IDC_SPINCONTWIDTH, UDM_SETPOS, 0, 0);
		SendDlgItemMessage(hwnd, IDC_SPINCONTHEIGHT, UDM_SETPOS, 0, 0);
		EnableWindow(GetDlgItem(hwnd, IDC_CONTLAYER), 0);
		return;
	}

	EnableWindow(GetDlgItem(hwnd, IDC_CONTTYPE), 1);
	EnableWindow(GetDlgItem(hwnd, IDC_VALIGN), 1);
	EnableWindow(GetDlgItem(hwnd, IDC_HALIGN), 1);
	EnableWindow(GetDlgItem(hwnd, IDC_CONTLAYER), 1);

	if (cell->type != TC_SPACE && cell->type != TC_FIXED) {
		EnableWindow(GetDlgItem(hwnd, IDC_CONTWIDTH), 0);
		EnableWindow(GetDlgItem(hwnd, IDC_CONTHEIGHT), 0);
		EnableWindow(GetDlgItem(hwnd, IDC_SPINCONTWIDTH), 0);
		EnableWindow(GetDlgItem(hwnd, IDC_SPINCONTHEIGHT), 0);
		SendDlgItemMessage(hwnd, IDC_SPINCONTWIDTH, UDM_SETPOS, 0, 0);
		SendDlgItemMessage(hwnd, IDC_SPINCONTHEIGHT, UDM_SETPOS, 0, 0);
	}
	else {
		EnableWindow(GetDlgItem(hwnd, IDC_CONTWIDTH), 1);
		EnableWindow(GetDlgItem(hwnd, IDC_CONTHEIGHT), 1);
		EnableWindow(GetDlgItem(hwnd, IDC_SPINCONTWIDTH), 1);
		EnableWindow(GetDlgItem(hwnd, IDC_SPINCONTHEIGHT), 1);
		SendDlgItemMessage(hwnd, IDC_SPINCONTWIDTH, UDM_SETPOS, 0, MAKELONG(cell->w, 0));
		SendDlgItemMessage(hwnd, IDC_SPINCONTHEIGHT, UDM_SETPOS, 0, MAKELONG(cell->h, 0));
	}

	SendDlgItemMessage(hwnd, IDC_CONTTYPE, CB_SETCURSEL, cell->type, 0);

	switch (cell->valign) {
	case TC_VCENTER:
		param = 1;
		break;
	case TC_BOTTOM:
		param = 2;
		break;
	default:
		param = 0;
	}
	SendDlgItemMessage(hwnd, IDC_VALIGN, CB_SETCURSEL, param, 0);

	switch (cell->halign) {
	case TC_HCENTER:
		param = 1;
		break;
	case TC_RIGHT:
		param = 2;
		break;
	default:
		param = 0;
	}
	SendDlgItemMessage(hwnd, IDC_HALIGN, CB_SETCURSEL, param, 0);

	CheckDlgButton(hwnd, IDC_CONTLAYER, cell->layer ? BST_CHECKED : BST_UNCHECKED);
}

void rowOptGenerateTreeView(pROWCELL cell, HTREEITEM node, HWND hwnd)
{
	if (!cell) return;

	TVINSERTSTRUCT tvis;
	tvis.hParent = node;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvis.item.stateMask = TVIS_STATEIMAGEMASK;
	tvis.item.lParam = (LPARAM)cell;

	switch (cell->cont) {
	case TC_ROW:
		tvis.item.pszText = _T("Line");
		break;
	case TC_COL:
		tvis.item.pszText = _T("Column");
	}

	tvis.item.iImage = cell->child ? 1 : 2;
	tvis.item.iSelectedImage = cell->child ? 0 : 2;

	HTREEITEM pnode = TreeView_InsertItem(hwnd, &tvis);
	rowOptGenerateTreeView(cell->child, pnode, hwnd);
	rowOptGenerateTreeView(cell->next, node, hwnd);
}

int rowOptFillRowTree(HWND hwnd)
{
	TreeView_DeleteAllItems(hwnd);
	rowOptGenerateTreeView(rowOptTmplRoot, NULL, hwnd);
	TreeView_Expand(hwnd, TreeView_GetRoot(hwnd), TVM_EXPAND);
	return 0;
}

void rowOptAddContainer(HWND htree, HTREEITEM hti)
{
	TVINSERTSTRUCT tvis;
	TVITEM tviparent;
	ROWCELL *cell = NULL;

	if (!hti) {
		if (TreeView_GetRoot(htree))
			return;

		rowAddCell(rowOptTmplRoot, TC_ROW);
		tvis.hParent = NULL;
		tvis.hInsertAfter = TVI_ROOT;
		tvis.item.pszText = _T("Line");
		tvis.item.lParam = (LPARAM)rowOptTmplRoot;
		cell = rowOptTmplRoot;
	}
	else {
		// Get parent item
		tviparent.hItem = hti;
		tviparent.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		TreeView_GetItem(htree, &tviparent);
		cell = (pROWCELL)tviparent.lParam;

		if (cell->cont == TC_ROW)
			tvis.item.pszText = _T("Column");
		else
			tvis.item.pszText = _T("Line");

		if (cell->child) {
			cell = cell->child;

			while (cell->next)
				cell = cell->next;

			rowAddCell(cell->next, ((ROWCELL*)tviparent.lParam)->cont == TC_ROW ? TC_COL : TC_ROW);
			cell = cell->next;
		}
		else {
			rowAddCell(cell->child, ((ROWCELL*)tviparent.lParam)->cont == TC_ROW ? TC_COL : TC_ROW);
			cell = cell->child;
		}

		tvis.hInsertAfter = TVI_LAST;
	}

	tvis.item.lParam = (LPARAM)cell;
	tvis.hParent = hti;

	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvis.item.stateMask = TVIS_STATEIMAGEMASK;
	tvis.item.iImage = 2;
	tvis.item.iSelectedImage = 2;

	TreeView_InsertItem(htree, &tvis);

	TreeView_Expand(htree, hti, TVM_EXPAND);

	// Change icon at parent item
	tviparent.iImage = 1;
	tviparent.iSelectedImage = 0;
	TreeView_SetItem(htree, &tviparent);

	int i = 0;
	memset(rowOptTA, 0, sizeof(rowOptTA));
	rowOptBuildTA(rowOptTmplRoot, (pROWCELL*)&rowOptTA, &i);
}

void rowOptDelContainer(HWND htree, HTREEITEM hti)
{
	HTREEITEM prev = TreeView_GetPrevSibling(htree, hti);
	HTREEITEM prnt = TreeView_GetParent(htree, hti);
	TVITEM	  tvi, tvpi;

	if (!hti) return;

	// Get current tree item
	tvi.hItem = hti;
	tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	TreeView_GetItem(htree, &tvi);

	tvpi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

	if (prev)
	{
		tvpi.hItem = prev;
		TreeView_GetItem(htree, &tvpi);
		((pROWCELL)tvpi.lParam)->next = ((pROWCELL)tvi.lParam)->next;
	}
	else
	{
		if (prnt)
		{
			tvpi.hItem = prnt;
			TreeView_GetItem(htree, &tvpi);
			((pROWCELL)tvpi.lParam)->child = ((pROWCELL)tvi.lParam)->next;
			prev = prnt;
		}
		else
		{

			tvpi.lParam = 0;
			rowOptTmplRoot = (pROWCELL)tvpi.lParam;
		}

	}

	((pROWCELL)tvi.lParam)->next = NULL;
	rowDeleteTree((pROWCELL)tvi.lParam);

	{
		int i = 0;
		memset(rowOptTA, 0, sizeof(rowOptTA));
		rowOptBuildTA((pROWCELL)tvpi.lParam, (pROWCELL*)&rowOptTA, &i);
	}

	TreeView_DeleteItem(htree, hti);


	// Change icon at parent item
	if (!prnt || (prnt != prev)) return;

	if (TreeView_GetChild(htree, prnt))
	{
		tvpi.iImage = 1;
		tvpi.iSelectedImage = 0;
	}
	else
	{
		tvpi.iImage = 2;
		tvpi.iSelectedImage = 2;
	}
	TreeView_SetItem(htree, &tvpi);

}

void RefreshTree(HWND hwndDlg, HTREEITEM hti)
{
	HWND htree = GetDlgItem(hwndDlg, IDC_ROWTREE);
	pROWCELL  cell;
	TVITEM    tvi = { 0 };
	if (hti == NULL) hti = TreeView_GetRoot(htree);
	while (hti)
	{
		tvi.hItem = hti;
		tvi.mask = TVIF_HANDLE;//|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT;
		TreeView_GetItem(htree, &tvi);
		cell = (pROWCELL)tvi.lParam;
		if (cell)
		{
			TCHAR buf[200] = { 0 };
			if (!cell->child)
			{
				if (cell->type == 0)
					mir_sntprintf(buf, SIZEOF(buf), TranslateT("Empty %s cell"), cell->cont == TC_COL ? TranslateT("column") : TranslateT("line"));
				else
					_tcsncpy_s(buf, TranslateTS(types[cell->type]), _TRUNCATE);
			}
			else
			{
				if (cell->type == 0)
					_tcsncpy_s(buf, (cell->cont != TC_COL ? TranslateT("columns") : TranslateT("lines")), _TRUNCATE);
				else
					mir_sntprintf(buf, SIZEOF(buf), TranslateT("%s, contain %s"), TranslateTS(types[cell->type]), cell->cont != TC_COL ? TranslateT("columns") : TranslateT("lines"));
			}
			if (cell->layer) _tcsncat_s(buf, TranslateT(" layered"),_TRUNCATE);
			tvi.mask = TVIF_HANDLE | TVIF_TEXT;
			tvi.pszText = buf;
			TreeView_SetItem(htree, &tvi);
		}
		{
			HTREEITEM child_hti = TreeView_GetChild(htree, hti);
			if (child_hti) RefreshTree(hwndDlg, child_hti);
		}
		hti = TreeView_GetNextSibling(htree, hti);
	}

	RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
}

INT_PTR CALLBACK DlgTmplEditorOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
	{
		int hbuf = 0, seq = 0;
		HWND htree = GetDlgItem(hwndDlg, IDC_ROWTREE);

		TranslateDialogDefault(hwndDlg);
		rowOptTmplStr = db_get_sa(NULL, "ModernData", "RowTemplate");
		if (!rowOptTmplStr)
			rowOptTmplStr = mir_strdup("<TR />");

		SendDlgItemMessage(hwndDlg, IDC_SPINCONTWIDTH, UDM_SETRANGE, 0, MAKELONG(999, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPINCONTHEIGHT, UDM_SETRANGE, 0, MAKELONG(999, 0));

		int i, item;

		for (i = 0; i < SIZEOF(types); i++) {
			item = SendDlgItemMessage(hwndDlg, IDC_CONTTYPE, CB_ADDSTRING, 0, (LPARAM)TranslateTS(types[i]));
			SendDlgItemMessage(hwndDlg, IDC_CONTTYPE, CB_SETITEMDATA, item, 0);
		}
		SendDlgItemMessage(hwndDlg, IDC_CONTTYPE, CB_SETCURSEL, 0, 0);

		TCHAR *h_alignment[] = { _T("left"), _T("hCenter"), _T("right") };
		for (i = 0; i < SIZEOF(h_alignment); i++) {
			item = SendDlgItemMessage(hwndDlg, IDC_HALIGN, CB_ADDSTRING, 0, (LPARAM)TranslateTS(h_alignment[i]));
			SendDlgItemMessage(hwndDlg, IDC_HALIGN, CB_SETITEMDATA, item, 0);
		}
		SendDlgItemMessage(hwndDlg, IDC_HALIGN, CB_SETCURSEL, 0, 0);

		TCHAR *v_alignment[] = { _T("top"), _T("vCenter"), _T("bottom") };
		for (i = 0; i < SIZEOF(v_alignment); i++) {
			item = SendDlgItemMessage(hwndDlg, IDC_VALIGN, CB_ADDSTRING, 0, (LPARAM)TranslateTS(v_alignment[i]));
			SendDlgItemMessage(hwndDlg, IDC_VALIGN, CB_SETITEMDATA, item, 0);
		}
		SendDlgItemMessage(hwndDlg, IDC_VALIGN, CB_SETCURSEL, 0, 0);

		rowDeleteTree(rowOptTmplRoot);
		rowOptTmplRoot = NULL;
		rowParse(rowOptTmplRoot, rowOptTmplRoot, rowOptTmplStr, hbuf, seq, rowOptTA);
		seq = 0;
		memset(rowOptTA, 0, sizeof(rowOptTA));
		rowOptBuildTA(rowOptTmplRoot, (pROWCELL*)&rowOptTA, &seq);

		rowOptFillRowTree(htree);
		RefreshTree(hwndDlg, NULL);
		TreeView_SelectItem(GetDlgItem(hwndDlg, IDC_ROWTREE), TreeView_GetRoot(GetDlgItem(hwndDlg, IDC_ROWTREE)));
		rowOptShowSettings(hwndDlg);
	}
	return TRUE;

	case WM_COMMAND:
	{
		HWND htree = GetDlgItem(hwndDlg, IDC_ROWTREE);
		pROWCELL  cell;
		TVITEM    tvi = { 0 };
		HTREEITEM hti = TreeView_GetSelection(htree);


		tvi.hItem = hti;
		tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		TreeView_GetItem(htree, &tvi);
		cell = (pROWCELL)tvi.lParam;

		switch (LOWORD(wParam)) {
		case IDC_CONTTYPE:
			if (HIWORD(wParam) == CBN_SELENDOK) {
				int index = SendDlgItemMessage(hwndDlg, IDC_CONTTYPE, CB_GETCURSEL, 0, 0);
				cell->type = index;
				RefreshTree(hwndDlg, NULL);
			}

		case IDC_VALIGN:
			if (HIWORD(wParam) == CBN_SELENDOK) {
				switch (SendDlgItemMessage(hwndDlg, IDC_VALIGN, CB_GETCURSEL, 0, 0)) {
				case 0:
					cell->valign = TC_TOP;
					break;
				case 1:
					cell->valign = TC_VCENTER;
					break;
				case 2:
					cell->valign = TC_BOTTOM;
					break;
				}
				RefreshTree(hwndDlg, NULL);
			}

		case IDC_HALIGN:
			if (HIWORD(wParam) == CBN_SELENDOK) {
				switch (SendDlgItemMessage(hwndDlg, IDC_HALIGN, CB_GETCURSEL, 0, 0)) {
				case 0:
					cell->halign = TC_LEFT;
					break;
				case 1:
					cell->halign = TC_HCENTER;
					break;
				case 2:
					cell->halign = TC_RIGHT;
					break;
				}
				RefreshTree(hwndDlg, NULL);
			}
		}

		if (HIWORD(wParam) == BN_CLICKED) {
			if (lParam == (LPARAM)GetDlgItem(hwndDlg, IDC_ADDCONTAINER))
				// Adding new container
			{
				rowOptAddContainer(htree, hti);
			}
			else if (lParam == (LPARAM)GetDlgItem(hwndDlg, IDC_DELCONTAINER))
				// Deleting container
			{
				rowOptDelContainer(htree, hti);
			}
			else if (lParam == (LPARAM)GetDlgItem(hwndDlg, IDC_CONTUP))
				// Moving container to up
			{
				RedrawWindow(htree, &da, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
			}
			RefreshTree(hwndDlg, NULL);
			RedrawWindow(GetParent(hwndDlg), NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
		return TRUE;
	}

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_ROWTREE:
			if (((LPNMHDR)lParam)->code == NM_SETCURSOR)
				rowOptShowSettings(hwndDlg);
			if (((LPNMHDR)lParam)->code == NM_CLICK)
				RedrawWindow(hwndDlg, &da, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
			break;

		case 0: // Apply or Ok button is pressed
			return FALSE; // Temporary
			break;
		}
		return TRUE;

	case WM_PAINT:
		if (rowOptTmplRoot) {
			int i = 0;

			// Drawning row template at properties page
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwndDlg, &ps);

			TVITEM curItem;
			curItem.hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_ROWTREE));
			curItem.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			TreeView_GetItem(GetDlgItem(hwndDlg, IDC_ROWTREE), &curItem);

			while (rowOptTA[i]) {
				switch (rowOptTA[i]->type) {
				case TC_AVATAR:
					rowOptTA[i]->w = 25;
					rowOptTA[i]->h = 30;
					break;

				case TC_STATUS:
				case TC_EXTRA1:
				case TC_EXTRA2:
				case TC_EXTRA3:
				case TC_EXTRA4:
				case TC_EXTRA5:
				case TC_EXTRA6:
				case TC_EXTRA7:
				case TC_EXTRA8:
				case TC_EXTRA9:
					rowOptTA[i]->w = 16;
					rowOptTA[i]->h = 16;
					break;

				case TC_EXTRA:
					rowOptTA[i]->w = 112;
					rowOptTA[i]->h = 16;
					break;

				case TC_TEXT1:
				case TC_TEXT2:
				case TC_TEXT3:
					rowOptTA[i]->w = 200;
					rowOptTA[i]->h = 16;
					break;

				case TC_TIME:
					rowOptTA[i]->w = 30;
					rowOptTA[i]->h = 10;
				default:
					rowOptTA[i]->w = 0;
					rowOptTA[i]->h = 0;
				}
				i++;
			}

			rowSizeWithReposition(rowOptTmplRoot, da.right - da.left);

			i = -1;
			while (rowOptTA[++i]) {

				switch (rowOptTA[i]->type) {
				case TC_SPACE:
				case TC_FIXED:
				case 0:
					//if (rowOptTA[i] != (pROWCELL)curItem.lParam)
					continue;
				}
				Rectangle(hdc,
					rowOptTA[i]->r.left + da.left,
					rowOptTA[i]->r.top + da.top,
					rowOptTA[i]->r.right + da.left,
					rowOptTA[i]->r.bottom + da.top);
			}

			EndPaint(hwndDlg, &ps);
			return FALSE;
		}

	}
	return FALSE;
}
