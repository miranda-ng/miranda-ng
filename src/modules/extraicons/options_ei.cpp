/*

Copyright (C) 2009 Ricardo Pescuma Domenecci
Copyright (C) 2012-15 Miranda NG project

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "..\..\core\commonheaders.h"

#include "extraicons.h"

#define ICON_SIZE 				16

int SortFunc(const ExtraIcon *p1, const ExtraIcon *p2);

struct intlist
{
	intlist() : count(0), data(0) {}
	~intlist() { mir_free(data); }

	void add(int val)
	{
		data = (int*)mir_realloc(data, sizeof(int)*(count + 1));
		data[count++] = val;
	}

	int count;
	int *data;
};

// Functions //////////////////////////////////////////////////////////////////////////////////////

BOOL ScreenToClient(HWND hWnd, LPRECT lpRect)
{
	POINT pt;
	pt.x = lpRect->left;
	pt.y = lpRect->top;

	BOOL ret = ScreenToClient(hWnd, &pt);
	if (!ret)
		return ret;

	lpRect->left = pt.x;
	lpRect->top = pt.y;

	pt.x = lpRect->right;
	pt.y = lpRect->bottom;

	ret = ScreenToClient(hWnd, &pt);

	lpRect->right = pt.x;
	lpRect->bottom = pt.y;

	return ret;
}

static void RemoveExtraIcons(int slot)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		Clist_SetExtraIcon(hContact, slot, INVALID_HANDLE_VALUE);
}

#ifndef TVIS_FOCUSED
#define TVIS_FOCUSED	1
#endif

static bool IsSelected(HWND tree, HTREEITEM hItem)
{
	return (TVIS_SELECTED & TreeView_GetItemState(tree, hItem, TVIS_SELECTED)) == TVIS_SELECTED;
}

static void Tree_Select(HWND tree, HTREEITEM hItem)
{
	TreeView_SetItemState(tree, hItem, TVIS_SELECTED, TVIS_SELECTED);
}

static void Tree_Unselect(HWND tree, HTREEITEM hItem)
{
	TreeView_SetItemState(tree, hItem, 0, TVIS_SELECTED);
}

static void Tree_DropHilite(HWND tree, HTREEITEM hItem)
{
	TreeView_SetItemState(tree, hItem, TVIS_DROPHILITED, TVIS_DROPHILITED);
}

static void Tree_DropUnhilite(HWND tree, HTREEITEM hItem)
{
	TreeView_SetItemState(tree, hItem, 0, TVIS_DROPHILITED);
}

static void UnselectAll(HWND tree)
{
	TreeView_SelectItem(tree, NULL);

	HTREEITEM hItem = TreeView_GetRoot(tree);
	while (hItem) {
		Tree_Unselect(tree, hItem);
		hItem = TreeView_GetNextSibling(tree, hItem);
	}
}

static void Tree_SelectRange(HWND tree, HTREEITEM hStart, HTREEITEM hEnd)
{
	int start = 0;
	int end = 0;
	int i = 0;
	HTREEITEM hItem = TreeView_GetRoot(tree);
	while (hItem) {
		if (hItem == hStart)
			start = i;
		if (hItem == hEnd)
			end = i;

		i++;
		hItem = TreeView_GetNextSibling(tree, hItem);
	}

	if (end < start) {
		int tmp = start;
		start = end;
		end = tmp;
	}

	i = 0;
	hItem = TreeView_GetRoot(tree);
	while (hItem) {
		if (i >= start)
			Tree_Select(tree, hItem);
		if (i == end)
			break;

		i++;
		hItem = TreeView_GetNextSibling(tree, hItem);
	}
}

static int GetNumSelected(HWND tree)
{
	int ret = 0;
	for (HTREEITEM hItem = TreeView_GetRoot(tree); hItem; hItem = TreeView_GetNextSibling(tree, hItem))
		if ( IsSelected(tree, hItem))
			ret++;

	return ret;
}

static void Tree_GetSelected(HWND tree, LIST<_TREEITEM> &selected)
{
	HTREEITEM hItem = TreeView_GetRoot(tree);
	while (hItem) {
		if (IsSelected(tree, hItem))
			selected.insert(hItem);
		hItem = TreeView_GetNextSibling(tree, hItem);
	}
}

static void Tree_Select(HWND tree, LIST<_TREEITEM> &selected)
{
	for (int i = 0; i < selected.getCount(); i++)
		if (selected[i] != NULL)
			Tree_Select(tree, selected[i]);
}

LRESULT CALLBACK TreeProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_LBUTTONDOWN:
		{
			DWORD pos = (DWORD) lParam;

			TVHITTESTINFO hti;
			hti.pt.x = (short) LOWORD(pos);
			hti.pt.y = (short) HIWORD(pos);
			if (!TreeView_HitTest(hwndDlg, &hti)) {
				UnselectAll(hwndDlg);
				break;
			}

			if (!(wParam & (MK_CONTROL | MK_SHIFT)) || !(hti.flags & (TVHT_ONITEMICON | TVHT_ONITEMLABEL | TVHT_ONITEMRIGHT))) {
				UnselectAll(hwndDlg);
				TreeView_SelectItem(hwndDlg, hti.hItem);
				break;
			}

			if (wParam & MK_CONTROL) {
				LIST<_TREEITEM> selected(1);
				Tree_GetSelected(hwndDlg, selected);

				// Check if have to deselect it
				for (int i = 0; i < selected.getCount(); i++) {
					if (selected[i] == hti.hItem) {
						// Deselect it
						UnselectAll(hwndDlg);
						selected.remove(i);

						if (i > 0)
							hti.hItem = selected[0];
						else if (i < selected.getCount())
							hti.hItem = selected[i];
						else
							hti.hItem = NULL;
						break;
					}
				}

				TreeView_SelectItem(hwndDlg, hti.hItem);
				Tree_Select(hwndDlg, selected);
			}
			else if (wParam & MK_SHIFT) {
				HTREEITEM hItem = TreeView_GetSelection(hwndDlg);
				if (hItem == NULL)
					break;

				LIST<_TREEITEM> selected(1);
				Tree_GetSelected(hwndDlg, selected);

				TreeView_SelectItem(hwndDlg, hti.hItem);
				Tree_Select(hwndDlg, selected);
				Tree_SelectRange(hwndDlg, hItem, hti.hItem);
			}

			return 0;
		}
	}

	return mir_callNextSubclass(hwndDlg, TreeProc, msg, wParam, lParam);
}

static intlist* Tree_GetIDs(HWND tree, HTREEITEM hItem)
{
	TVITEM tvi = { 0 };
	tvi.mask = TVIF_HANDLE | TVIF_PARAM;
	tvi.hItem = hItem;
	TreeView_GetItem(tree, &tvi);
	return (intlist*)tvi.lParam;
}

static HTREEITEM Tree_AddExtraIcon(HWND tree, BaseExtraIcon *extra, bool selected, HTREEITEM hAfter = TVI_LAST)
{
	intlist *ids = new intlist();
	ids->add(extra->getID());

	TVINSERTSTRUCT tvis = { 0 };
	tvis.hInsertAfter = hAfter;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
	tvis.item.stateMask = TVIS_STATEIMAGEMASK;
	tvis.item.iSelectedImage = tvis.item.iImage = extra->getID();
	tvis.item.lParam = (LPARAM)ids;
	tvis.item.pszText = (LPTSTR)extra->getDescription();
	tvis.item.state = INDEXTOSTATEIMAGEMASK(selected ? 2 : 1);
	return TreeView_InsertItem(tree, &tvis);
}

static HTREEITEM Tree_AddExtraIconGroup(HWND tree, intlist &group, bool selected, HTREEITEM hAfter = TVI_LAST)
{
	intlist *ids = new intlist();
	CMString desc;
	int img = 0;
	for (int i = 0; i < group.count; i++) {
		BaseExtraIcon *extra = registeredExtraIcons[group.data[i] - 1];
		ids->add(extra->getID());

		if (img == 0 && !IsEmpty(extra->getDescIcon()))
			img = extra->getID();

		if (i > 0)
			desc += _T(" / ");
		desc += extra->getDescription();
	}

	TVINSERTSTRUCT tvis = { 0 };
	tvis.hInsertAfter = hAfter;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
	tvis.item.stateMask = TVIS_STATEIMAGEMASK;
	tvis.item.iSelectedImage = tvis.item.iImage = img;
	tvis.item.lParam = (LPARAM)ids;
	tvis.item.pszText = (TCHAR*)desc.c_str();
	tvis.item.state = INDEXTOSTATEIMAGEMASK(selected ? 2 : 1);
	return TreeView_InsertItem(tree, &tvis);
}

static void GroupSelectedItems(HWND tree)
{
	LIST<_TREEITEM> toRemove(1);
	intlist ids;
	bool selected = false;
	HTREEITEM hPlace = NULL;

	// Find items
	HTREEITEM hItem = TreeView_GetRoot(tree);
	TVITEM tvi = { 0 };
	tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
	while (hItem) {
		if (IsSelected(tree, hItem)) {
			if (hPlace == NULL)
				hPlace = hItem;

			tvi.hItem = hItem;
			TreeView_GetItem(tree, &tvi);

			intlist *iids = (intlist*)tvi.lParam;
			for (int i = 0; i < iids->count; i++)
				ids.add(iids->data[i]);

			if ((tvi.state & INDEXTOSTATEIMAGEMASK(3)) == INDEXTOSTATEIMAGEMASK(2))
				selected = true;

			toRemove.insert(hItem);
		}

		hItem = TreeView_GetNextSibling(tree, hItem);
	}

	if (hPlace != NULL) {
		// Add new
		HTREEITEM hNew = Tree_AddExtraIconGroup(tree, ids, selected, hPlace);

		// Remove old
		for (int i = 0; i < toRemove.getCount(); i++) {
			delete Tree_GetIDs(tree, toRemove[i]);
			TreeView_DeleteItem(tree, toRemove[i]);
		}

		// Select
		UnselectAll(tree);
		TreeView_SelectItem(tree, hNew);
	}
}

static void UngroupSelectedItems(HWND tree)
{
	HTREEITEM hItem = TreeView_GetSelection(tree);
	if (hItem == NULL)
		return;

	intlist *ids = Tree_GetIDs(tree, hItem);
	if (ids->count < 2)
		return;

	bool selected = IsSelected(tree, hItem);

	for (int i = ids->count - 1; i >= 0; i--) {
		BaseExtraIcon *extra = registeredExtraIcons[ids->data[i] - 1];
		Tree_AddExtraIcon(tree, extra, selected, hItem);
	}

	delete ids;
	TreeView_DeleteItem(tree, hItem);

	UnselectAll(tree);
}

static int ShowPopup(HWND hwndDlg, int popup)
{
	// Fix selection
	HWND tree = GetDlgItem(hwndDlg, IDC_EXTRAORDER);
	HTREEITEM hSelected = (HTREEITEM) SendMessage(tree, TVM_GETNEXTITEM, TVGN_DROPHILITE, 0);
	HTREEITEM hItem = TreeView_GetRoot(tree);
	while (hItem) {
		if (hItem != hSelected && IsSelected(tree, hItem))
			Tree_DropHilite(tree, hItem);

		hItem = TreeView_GetNextSibling(tree, hItem);
	}
	//	InvalidateRect(tree, NULL, FALSE);

	HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_OPT_POPUP));
	HMENU submenu = GetSubMenu(menu, popup);
	TranslateMenu(submenu);

	DWORD pos = GetMessagePos();
	int ret = TrackPopupMenu(submenu, TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_LEFTALIGN, LOWORD(pos),
			HIWORD(pos), 0, hwndDlg, NULL);

	DestroyMenu(menu);

	// Revert selection
	hItem = TreeView_GetRoot(tree);
	while (hItem) {
		if (hItem != hSelected && IsSelected(tree, hItem))
			Tree_DropUnhilite(tree, hItem);
		hItem = TreeView_GetNextSibling(tree, hItem);
	}

	return ret;
}

static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	intlist*a = (intlist*)lParam1;
	intlist*b = (intlist*)lParam2;
	return SortFunc(registeredExtraIcons[a->data[0]-1], registeredExtraIcons[b->data[0]-1]);
}

static INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND tree = GetDlgItem(hwndDlg, IDC_EXTRAORDER);
	LPNMHDR lpnmhdr;

	static int dragging = 0;
	static HANDLE hDragItem = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			int numSlots = GetNumberOfSlots();
			if (numSlots < (int)registeredExtraIcons.getCount()) {
				TCHAR txt[512];
				mir_sntprintf(txt, SIZEOF(txt), TranslateT("*only the first %d icons will be shown"), numSlots);

				HWND label = GetDlgItem(hwndDlg, IDC_MAX_ICONS_L);
				SetWindowText(label, txt);
				ShowWindow(label, SW_SHOW);
			}

			SetWindowLongPtr(tree, GWL_STYLE, GetWindowLongPtr(tree, GWL_STYLE) | TVS_NOHSCROLL);

			int cx = GetSystemMetrics(SM_CXSMICON);
			HIMAGELIST hImageList = ImageList_Create(cx, cx, ILC_COLOR32 | ILC_MASK, 2, 2);

			HICON hBlankIcon = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_BLANK), IMAGE_ICON, cx, cx, 0);
			ImageList_AddIcon(hImageList, hBlankIcon);

			for (int i = 0; i < registeredExtraIcons.getCount(); i++) {
				ExtraIcon *extra = registeredExtraIcons[i];

				HICON hIcon = Skin_GetIcon(extra->getDescIcon());
				if (hIcon == NULL)
					ImageList_AddIcon(hImageList, hBlankIcon);
				else {
					ImageList_AddIcon(hImageList, hIcon);
					Skin_ReleaseIcon(hIcon);
				}
			}
			TreeView_SetImageList(tree, hImageList, TVSIL_NORMAL);
			DestroyIcon(hBlankIcon);

			for (int k = 0; k < extraIconsBySlot.getCount(); k++) {
				ExtraIcon *extra = extraIconsBySlot[k];

				if (extra->getType() == EXTRAICON_TYPE_GROUP) {
					ExtraIconGroup *group = (ExtraIconGroup *)extra;
					intlist ids;
					for (int j = 0; j < group->items.getCount(); j++)
						ids.add(group->items[j]->getID());
					Tree_AddExtraIconGroup(tree, ids, extra->isEnabled());
				}
				else Tree_AddExtraIcon(tree, (BaseExtraIcon *)extra, extra->isEnabled());
			}

			TVSORTCB sort = { 0 };
			sort.hParent = NULL;
			sort.lParam = 0;
			sort.lpfnCompare = CompareFunc;
			TreeView_SortChildrenCB(tree, &sort, 0);

			mir_subclassWindow(tree, TreeProc);
		}
		return TRUE;

	case WM_NOTIFY:
		lpnmhdr = (LPNMHDR)lParam;
		if (lpnmhdr->idFrom == 0 && lpnmhdr->code == (UINT) PSN_APPLY) {
			// Store old slots
			int *oldSlots = new int[registeredExtraIcons.getCount()];
			int lastUsedSlot = -1;
			for (int i = 0; i < registeredExtraIcons.getCount(); i++) {
				if (extraIconsByHandle[i] == registeredExtraIcons[i])
					oldSlots[i] = registeredExtraIcons[i]->getSlot();
				else
					// Remove old slot for groups to re-set images
					oldSlots[i] = -1;
				lastUsedSlot = MAX(lastUsedSlot, registeredExtraIcons[i]->getSlot());
			}
			lastUsedSlot = MIN(lastUsedSlot, GetNumberOfSlots());

			// Get user data and create new groups
			LIST<ExtraIconGroup> groups(1);

			BYTE pos = 0;
			int firstEmptySlot = 0;
			HTREEITEM ht = TreeView_GetRoot(tree);
			TVITEM tvi = { 0 };
			tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
			tvi.stateMask = TVIS_STATEIMAGEMASK;
			while (ht) {
				tvi.hItem = ht;
				TreeView_GetItem(tree, &tvi);

				intlist*ids = (intlist*) tvi.lParam;
				if (ids == NULL || ids->count < 1)
					continue; // ???

				bool enabled = ((tvi.state & INDEXTOSTATEIMAGEMASK(3)) == INDEXTOSTATEIMAGEMASK(2));
				int slot = (enabled ? firstEmptySlot++ : -1);
				if (slot >= GetNumberOfSlots())
					slot = -1;

				if (ids->count == 1) {
					BaseExtraIcon *extra = registeredExtraIcons[ids->data[0] - 1];
					extra->setPosition(pos++);
					extra->setSlot(slot);
				}
				else {
					char name[128];
					mir_snprintf(name, SIZEOF(name), "__group_%d", groups.getCount());

					ExtraIconGroup *group = new ExtraIconGroup(name);

					for (int i=0; i < ids->count; i++) {
						BaseExtraIcon *extra = registeredExtraIcons[ids->data[i] - 1];
						extra->setPosition(pos++);

						group->addExtraIcon(extra);
					}

					group->setSlot(slot);
					groups.insert(group);
				}

				ht = TreeView_GetNextSibling(tree, ht);
			}

			// Store data
			for (int i = 0; i < registeredExtraIcons.getCount(); i++) {
				BaseExtraIcon *extra = registeredExtraIcons[i];

				char setting[512];
				mir_snprintf(setting, SIZEOF(setting), "Position_%s", extra->getName());
				db_set_w(NULL, MODULE_NAME, setting, extra->getPosition());

				mir_snprintf(setting, SIZEOF(setting), "Slot_%s", extra->getName());
				db_set_w(NULL, MODULE_NAME, setting, extra->getSlot());
			}

			CallService(MS_DB_MODULE_DELETE, 0, (LPARAM) MODULE_NAME "Groups");
			db_set_w(NULL, MODULE_NAME "Groups", "Count", groups.getCount());
			for (int k = 0; k < groups.getCount(); k++) {
				ExtraIconGroup *group = groups[k];

				char setting[512];
				mir_snprintf(setting, SIZEOF(setting), "%d_count", k);
				db_set_w(NULL, MODULE_NAME "Groups", setting, (WORD)group->items.getCount());

				for (int j = 0; j < group->items.getCount(); j++) {
					BaseExtraIcon *extra = group->items[j];

					mir_snprintf(setting, SIZEOF(setting), "%d_%d", k, j);
					db_set_s(NULL, MODULE_NAME "Groups", setting, extra->getName());
				}
			}

			// Clean removed slots
			for (int j = firstEmptySlot; j <= lastUsedSlot; j++)
				RemoveExtraIcons(j);

			// Apply icons to new slots
			RebuildListsBasedOnGroups(groups);
			for (int n = 0; n < extraIconsBySlot.getCount(); n++) {
				ExtraIcon *extra = extraIconsBySlot[n];
				if (extra->getType() != EXTRAICON_TYPE_GROUP)
					if (oldSlots[((BaseExtraIcon *) extra)->getID() - 1] == extra->getSlot())
						continue;

				if (extra->isEnabled())
					extra->applyIcons();
			}

			delete[] oldSlots;
			return TRUE;
		}
		
		if (lpnmhdr->idFrom == IDC_EXTRAORDER) {
			switch (lpnmhdr->code) {
			case TVN_BEGINDRAGW:
			case TVN_BEGINDRAGA:
				SetCapture(hwndDlg);
				dragging = 1;
				hDragItem = ((LPNMTREEVIEWA) lParam)->itemNew.hItem;
				TreeView_SelectItem(tree, hDragItem);
				break;

			case NM_CLICK:
				{
					DWORD pos = GetMessagePos();

					TVHITTESTINFO hti;
					hti.pt.x = (short) LOWORD(pos);
					hti.pt.y = (short) HIWORD(pos);
					ScreenToClient(lpnmhdr->hwndFrom, &hti.pt);
					if (TreeView_HitTest(lpnmhdr->hwndFrom, &hti)) {
						if (hti.flags & TVHT_ONITEMSTATEICON) {
							TreeView_SelectItem(tree, hti.hItem);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM) hwndDlg, 0);
						}
					}
				}
				break;

			case TVN_KEYDOWN:
				{
					TV_KEYDOWN *nmkd = (TV_KEYDOWN *) lpnmhdr;
					if (nmkd->wVKey == VK_SPACE) {
						// Determine the selected tree item.
						HTREEITEM hItem = TreeView_GetSelection(tree);
						if (hItem != NULL)
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM) hwndDlg, 0);
					}
				}
				break;

			case NM_RCLICK:
				HTREEITEM hSelected = (HTREEITEM) SendMessage(tree, TVM_GETNEXTITEM, TVGN_DROPHILITE, 0);
				if (hSelected != NULL && !IsSelected(tree, hSelected)) {
					UnselectAll(tree);
					TreeView_SelectItem(tree, hSelected);
				}

				int sels = GetNumSelected(tree);
				if (sels > 1) {
					if (ShowPopup(hwndDlg, 0) == ID_GROUP) {
						GroupSelectedItems(tree);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM) hwndDlg, 0);
					}
				}
				else if (sels == 1) {
					HTREEITEM hItem = TreeView_GetSelection(tree);
					intlist*ids = Tree_GetIDs(tree, hItem);
					if (ids->count > 1) {
						if (ShowPopup(hwndDlg, 1) == ID_UNGROUP) {
							UngroupSelectedItems(tree);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM) hwndDlg, 0);
						}
					}
				}
			}
		}
		break;

	case WM_MOUSEMOVE:
		if (dragging) {
			TVHITTESTINFO hti;
			hti.pt.x = (short) LOWORD(lParam);
			hti.pt.y = (short) HIWORD(lParam);
			ClientToScreen(hwndDlg, &hti.pt);
			ScreenToClient(tree, &hti.pt);
			TreeView_HitTest(tree, &hti);
			if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) {
				HTREEITEM it = hti.hItem;
				hti.pt.y -= TreeView_GetItemHeight(tree) / 2;
				TreeView_HitTest(tree, &hti);
				if (!(hti.flags & TVHT_ABOVE))
					TreeView_SetInsertMark(tree, hti.hItem, 1);
				else
					TreeView_SetInsertMark(tree, it, 0);
			}
			else {
				if (hti.flags & TVHT_ABOVE)
					SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, WM_VSCROLL, MAKEWPARAM(SB_LINEUP,0), 0);
				if (hti.flags & TVHT_BELOW)
					SendDlgItemMessage(hwndDlg, IDC_EXTRAORDER, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN,0), 0);
				TreeView_SetInsertMark(tree, NULL, 0);
			}
		}
		break;

	case WM_LBUTTONUP:
		if (dragging) {
			TreeView_SetInsertMark(tree, NULL, 0);
			dragging = 0;
			ReleaseCapture();

			TVHITTESTINFO hti;
			hti.pt.x = (short) LOWORD(lParam);
			hti.pt.y = (short) HIWORD(lParam);
			ClientToScreen(hwndDlg, &hti.pt);
			ScreenToClient(tree, &hti.pt);
			hti.pt.y -= TreeView_GetItemHeight(tree) / 2;
			TreeView_HitTest(tree,&hti);
			if (hDragItem == hti.hItem)
				break;

			if (!(hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT | TVHT_ABOVE | TVHT_BELOW)))
				break;

			if (hti.flags & TVHT_ABOVE)
				hti.hItem = TVI_FIRST;
			else if (hti.flags & TVHT_BELOW)
				hti.hItem = TVI_LAST;

			TVINSERTSTRUCT tvis;
			TCHAR name[512];
			tvis.item.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
			tvis.item.stateMask = 0xFFFFFFFF;
			tvis.item.pszText = name;
			tvis.item.cchTextMax = SIZEOF(name);
			tvis.item.hItem = (HTREEITEM) hDragItem;
			TreeView_GetItem(tree, &tvis.item);

			TreeView_DeleteItem(tree, hDragItem);

			tvis.hParent = NULL;
			tvis.hInsertAfter = hti.hItem;
			TreeView_SelectItem(tree, TreeView_InsertItem(tree, &tvis));

			SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM) hwndDlg, 0);
		}
		break;

	case WM_DESTROY:
		HTREEITEM hItem = TreeView_GetRoot(tree);
		while (hItem) {
			delete Tree_GetIDs(tree, hItem);
			hItem = TreeView_GetNextSibling(tree, hItem);
		}

		ImageList_Destroy( TreeView_GetImageList(tree, TVSIL_NORMAL));
		break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int InitOptionsCallback(WPARAM wParam, LPARAM lParam)
{
	if (GetNumberOfSlots() < 1)
		return 0;

	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = hInst;
	odp.pszGroup = LPGEN("Contact list");
	odp.pszTitle = LPGEN("Extra icons");
	odp.pszTab = LPGEN("General");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_EI_OPTIONS);
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}
