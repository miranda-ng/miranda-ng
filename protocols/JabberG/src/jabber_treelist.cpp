/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Victor Pavlychko
Copyright (C) 2012-25 Miranda NG team

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

#define TLIF_VISIBLE	0x01
#define TLIF_EXPANDED	0x02
#define TLIF_MODIFIED	0x04
#define TLIF_ROOT		0X08
#define TLIF_HASITEM	0X10
#define TLIF_REBUILD	0x20
#define TLIF_FAKEPARENT	0x40
#define TLIF_FILTERED	0x80

struct TTreeList_ItemInfo
{
	uint8_t flags;
	int indent, sortIndex;

	struct TTreeList_ItemInfo *parent;
	int iIcon, iOverlay;
	LIST<wchar_t> text;
	LPARAM data;
	LIST<TTreeList_ItemInfo> subItems;

	TTreeList_ItemInfo(int columns = 3, int children = 5) :
		text(columns), subItems(children), parent(nullptr),
		flags(0), indent(0), sortIndex(0), iIcon(0), iOverlay(0), data(0)
	{
	}
	~TTreeList_ItemInfo()
	{
		for (auto &it : text)
			mir_free(it);

		for (auto &it : subItems)
			delete it;
	}
};

struct TTreeList_Data
{
	int mode, sortMode;
	wchar_t *filter;
	HTREELISTITEM hItemSelected;
	TTreeList_ItemInfo *root;

	TTreeList_Data()
	{
		sortMode = 0;
		filter = nullptr;
		mode = TLM_TREE;
		root = nullptr;
		hItemSelected = nullptr;
	}
	~TTreeList_Data()
	{
		if (root) delete root;
		if (filter) mir_free(filter);
	}
};

// static utilities
static void sttTreeList_ResetIndex(HTREELISTITEM hItem, LPARAM data);
static void sttTreeList_SortItems(HTREELISTITEM hItem, LPARAM data);
static void sttTreeList_FilterItems(HTREELISTITEM hItem, LPARAM data);
static void sttTreeList_CreateItems(HTREELISTITEM hItem, LPARAM data);
static void sttTreeList_CreateItems_List(HTREELISTITEM hItem, LPARAM data);
static int CALLBACK sttTreeList_SortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

static __forceinline void sttTreeList_SeWindowData(HWND hwnd, HANDLE data)
{
	SetPropA(hwnd, "Miranda.TreeList", (HANDLE)data);
}

static __forceinline HANDLE sttTreeList_GeWindowData(HWND hwnd)
{
	return GetPropA(hwnd, "Miranda.TreeList");
}

// tree list implementation
LPARAM TreeList_GetData(HTREELISTITEM hItem)
{
	return hItem->data;
}

HTREELISTITEM TreeList_GetRoot(HWND hwnd)
{
	TTreeList_Data *data = (TTreeList_Data *)sttTreeList_GeWindowData(hwnd);
	return data->root;
}

int TreeList_GetChildrenCount(HTREELISTITEM hItem)
{
	return hItem->subItems.getCount();
}

HTREELISTITEM TreeList_GetChild(HTREELISTITEM hItem, int i)
{
	return hItem->subItems[i];
}

void TreeList_Create(HWND hwnd)
{
	TTreeList_Data *data = new TTreeList_Data;
	data->root = new TTreeList_ItemInfo;
	data->root->flags = TLIF_EXPANDED | TLIF_VISIBLE | TLIF_ROOT;
	data->root->indent = -1;
	data->hItemSelected = data->root;
	sttTreeList_SeWindowData(hwnd, data);

	ListView_SetExtendedListViewStyle(hwnd, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);

	HIMAGELIST hIml;
	hIml = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 2, 1);
	ListView_SetImageList(hwnd, hIml, LVSIL_SMALL);

	hIml = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 2, 1);
	ImageList_AddSkinIcon(hIml, SKINICON_OTHER_GROUPOPEN);
	ImageList_AddSkinIcon(hIml, SKINICON_OTHER_GROUPSHUT);
	ImageList_AddSkinIcon(hIml, SKINICON_OTHER_DOWNARROW);
	ListView_SetImageList(hwnd, hIml, LVSIL_STATE);
}

void TreeList_Destroy(HWND hwnd)
{
	ListView_DeleteAllItems(hwnd);
	TTreeList_Data *data = (TTreeList_Data *)sttTreeList_GeWindowData(hwnd);
	delete data;
}

void TreeList_Reset(HWND hwnd)
{
	ListView_DeleteAllItems(hwnd);
	TTreeList_Data *data = (TTreeList_Data *)sttTreeList_GeWindowData(hwnd);
	delete data->root;
	data->root = new TTreeList_ItemInfo;
	data->root->flags = TLIF_EXPANDED | TLIF_VISIBLE | TLIF_ROOT;
	data->root->indent = -1;
	data->hItemSelected = data->root;
}

void TreeList_SetMode(HWND hwnd, int mode)
{
	TTreeList_Data *data = (TTreeList_Data *)sttTreeList_GeWindowData(hwnd);
	data->mode = mode;
	ListView_DeleteAllItems(hwnd);
	TreeList_Update(hwnd);
}

void TreeList_SetSortMode(HWND hwnd, int col, BOOL descending)
{
	TTreeList_Data *data = (TTreeList_Data *)sttTreeList_GeWindowData(hwnd);
	if ((col >= 0) && (col < 2))
		data->sortMode = 1 + col * 2 + (descending ? 1 : 0);
	else
		data->sortMode = 0;
	TreeList_Update(hwnd);
}

void TreeList_SetFilter(HWND hwnd, wchar_t *filter)
{
	TTreeList_Data *data = (TTreeList_Data *)sttTreeList_GeWindowData(hwnd);
	if (data->filter) mir_free(data->filter);
	data->filter = nullptr;
	if (filter) data->filter = mir_wstrdup(filter);
	TreeList_Update(hwnd);
}

HTREELISTITEM TreeList_GetActiveItem(HWND hwnd)
{
	TTreeList_Data *data = (TTreeList_Data *)sttTreeList_GeWindowData(hwnd);
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_PARAM;
	lvi.iItem = ListView_GetNextItem(hwnd, -1, LVNI_SELECTED);
	if (lvi.iItem < 0)
		return (data->hItemSelected->flags & TLIF_ROOT) ? nullptr : data->hItemSelected;
	ListView_GetItem(hwnd, &lvi);
	return (HTREELISTITEM)lvi.lParam;
}

HTREELISTITEM TreeList_AddItem(HWND hwnd, HTREELISTITEM hParent, wchar_t *text, LPARAM nodeDdata)
{
	TTreeList_Data *data = (TTreeList_Data *)sttTreeList_GeWindowData(hwnd);
	if (!hParent) hParent = data->root;

	TTreeList_ItemInfo *item = new TTreeList_ItemInfo;
	item->data = nodeDdata;
	item->parent = hParent;
	item->text.insert(mir_wstrdup(text));
	item->flags |= TLIF_MODIFIED;
	if (hParent->flags & TLIF_ROOT) {
		item->flags |= TLIF_EXPANDED;
		data->hItemSelected = item;
	}
	item->indent = hParent->indent + 1;
	hParent->subItems.insert(item);
	return item;
}

void TreeList_ResetItem(HWND hwnd, HTREELISTITEM hParent)
{
	TTreeList_Data *data = (TTreeList_Data *)sttTreeList_GeWindowData(hwnd);

	for (auto &it : hParent->subItems)
		delete it;
	hParent->subItems.destroy();

	data->hItemSelected = hParent;
	ListView_DeleteAllItems(hwnd);
}

void TreeList_MakeFakeParent(HTREELISTITEM hItem, BOOL flag)
{
	if (flag)
		hItem->flags |= TLIF_FAKEPARENT;
	else
		hItem->flags &= ~TLIF_FAKEPARENT;
	hItem->flags |= TLIF_MODIFIED;
}

void TreeList_AppendColumn(HTREELISTITEM hItem, wchar_t *text)
{
	hItem->text.insert(mir_wstrdup(text));
	hItem->flags |= TLIF_MODIFIED;
}

int TreeList_AddIcon(HWND hwnd, HICON hIcon, int iOverlay)
{
	HIMAGELIST hIml = ListView_GetImageList(hwnd, LVSIL_SMALL);
	int idx = ImageList_AddIcon(hIml, hIcon);
	IcoLib_ReleaseIcon(hIcon);
	if (iOverlay) ImageList_SetOverlayImage(hIml, idx, iOverlay);
	return idx;
}

void TreeList_SetIcon(HTREELISTITEM hItem, int iIcon, int iOverlay)
{
	if (iIcon >= 0) hItem->iIcon = iIcon;
	if (iOverlay >= 0) hItem->iOverlay = iOverlay;
	if ((iIcon >= 0) || (iOverlay >= 0)) hItem->flags |= TLIF_MODIFIED;
}

void TreeList_RecursiveApply(HTREELISTITEM hItem, void(*func)(HTREELISTITEM, LPARAM), LPARAM data)
{
	for (auto &it : hItem->subItems) {
		func(it, data);
		TreeList_RecursiveApply(it, func, data);
	}
}

void TreeList_Update(HWND hwnd)
{
	TTreeList_Data *data = (TTreeList_Data *)sttTreeList_GeWindowData(hwnd);
	HTREELISTITEM hItem = data->root;
	int sortIndex = 0;

	SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
	if (data->sortMode)
		TreeList_RecursiveApply(hItem, sttTreeList_SortItems, (LPARAM)data->sortMode);
	TreeList_RecursiveApply(hItem, sttTreeList_ResetIndex, (LPARAM)&sortIndex);
	if (data->filter)
		TreeList_RecursiveApply(hItem, sttTreeList_FilterItems, (LPARAM)data->filter);
	for (int i = ListView_GetItemCount(hwnd); i--;) {
		LVITEM lvi = { 0 };
		lvi.mask = LVIF_PARAM;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		ListView_GetItem(hwnd, &lvi);

		HTREELISTITEM ptli = (HTREELISTITEM)lvi.lParam;
		if ((ptli->flags & TLIF_VISIBLE) && (!data->filter || (ptli->flags & TLIF_FILTERED))) {
			ptli->flags |= TLIF_HASITEM;
			if (ptli->flags & TLIF_MODIFIED) {
				lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE | LVIF_TEXT;
				lvi.iItem = i;
				lvi.iSubItem = 0;
				lvi.pszText = ptli->text[0];
				lvi.stateMask = LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK;
				lvi.iImage = ptli->iIcon;
				if (data->mode == TLM_TREE)
					lvi.state =
					INDEXTOSTATEIMAGEMASK(
					((ptli->subItems.getCount() == 0) && !(ptli->flags & TLIF_FAKEPARENT)) ? 0 :
						(ptli->flags & TLIF_EXPANDED) ? 1 : 2) |
					INDEXTOOVERLAYMASK(ptli->iOverlay);
				else
					lvi.state =
					INDEXTOSTATEIMAGEMASK(
					((ptli->subItems.getCount() == 0) && !(ptli->flags & TLIF_FAKEPARENT)) ? 0 : 3) |
					INDEXTOOVERLAYMASK(ptli->iOverlay);

				ListView_SetItem(hwnd, &lvi);
				for (int j = 1; j < ptli->text.getCount(); ++j)
					ListView_SetItemText(hwnd, i, j, ptli->text[j]);
			}
		}
		else ListView_DeleteItem(hwnd, i);
	}
	if (data->mode == TLM_TREE)
		TreeList_RecursiveApply(hItem, sttTreeList_CreateItems, (LPARAM)hwnd);
	else {
		for (auto &it : data->hItemSelected->subItems)
			sttTreeList_CreateItems_List(it, (LPARAM)hwnd);

		for (HTREELISTITEM p = data->hItemSelected; !(p->flags & TLIF_ROOT); p = p->parent)
			sttTreeList_CreateItems_List(p, (LPARAM)hwnd);
	}
	ListView_SortItems(hwnd, sttTreeList_SortFunc, 0);
	SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
	UpdateWindow(hwnd);
}

BOOL TreeList_ProcessMessage(HWND hwnd, UINT msg, WPARAM, LPARAM lparam, UINT idc, BOOL*)
{
	LVITEM lvi = { 0 };

	switch (msg) {
	case WM_NOTIFY:
		{
			if (((LPNMHDR)lparam)->idFrom != idc)
				break;

			TTreeList_Data *data = (TTreeList_Data *)sttTreeList_GeWindowData(GetDlgItem(hwnd, idc));
			switch (((LPNMHDR)lparam)->code) {
			case LVN_COLUMNCLICK:
				{
					LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)lparam;
					TreeList_SetSortMode(lpnmlv->hdr.hwndFrom, lpnmlv->iSubItem, FALSE);
				}
				break;

			case LVN_ITEMACTIVATE:
				if (data->mode == TLM_REPORT) {
					LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lparam;
					lvi.mask = LVIF_PARAM;
					lvi.iItem = lpnmia->iItem;
					ListView_GetItem(lpnmia->hdr.hwndFrom, &lvi);

					HTREELISTITEM hItem = (lvi.iItem < 0) ? data->root : (HTREELISTITEM)lvi.lParam;
					if (!hItem->subItems.getCount() && !(hItem->flags & TLIF_FAKEPARENT)) break;
					data->hItemSelected = hItem;

					NMTREEVIEW nmtv;
					nmtv.hdr.code = TVN_ITEMEXPANDED;
					nmtv.hdr.hwndFrom = lpnmia->hdr.hwndFrom;
					nmtv.hdr.idFrom = lpnmia->hdr.idFrom;
					nmtv.itemNew.hItem = (HTREEITEM)lvi.lParam;
					SendMessage(hwnd, WM_NOTIFY, lpnmia->hdr.idFrom, (LPARAM)&nmtv);

					ListView_DeleteAllItems(lpnmia->hdr.hwndFrom);
					TreeList_Update(lpnmia->hdr.hwndFrom);
				}
				break;

			case LVN_KEYDOWN:
				if (data->mode == TLM_TREE) {
					LPNMLVKEYDOWN lpnmlvk = (LPNMLVKEYDOWN)lparam;

					lvi.mask = LVIF_PARAM | LVIF_INDENT;
					lvi.iItem = ListView_GetNextItem(lpnmlvk->hdr.hwndFrom, -1, LVNI_SELECTED);
					if (lvi.iItem < 0) return FALSE;
					lvi.iSubItem = 0;
					ListView_GetItem(lpnmlvk->hdr.hwndFrom, &lvi);
					HTREELISTITEM hItem = (HTREELISTITEM)lvi.lParam;

					switch (lpnmlvk->wVKey) {
					case VK_SUBTRACT:
					case VK_LEFT:
						if (hItem->subItems.getCount() && (hItem->flags & TLIF_EXPANDED)) {
							hItem->flags &= ~TLIF_EXPANDED;
							hItem->flags |= TLIF_MODIFIED;
							TreeList_Update(lpnmlvk->hdr.hwndFrom);
						}
						else if (hItem->indent && (lpnmlvk->wVKey != VK_SUBTRACT)) {
							for (int i = lvi.iItem; i--;) {
								lvi.mask = LVIF_INDENT;
								lvi.iItem = i;
								lvi.iSubItem = 0;
								ListView_GetItem(lpnmlvk->hdr.hwndFrom, &lvi);
								if (lvi.iIndent < hItem->indent) {
									lvi.mask = LVIF_STATE;
									lvi.iItem = i;
									lvi.iSubItem = 0;
									lvi.state = lvi.stateMask = LVIS_FOCUSED | LVNI_SELECTED;
									ListView_SetItem(lpnmlvk->hdr.hwndFrom, &lvi);
									break;
								}
							}
						}
						break;

					case VK_ADD:
					case VK_RIGHT:
						if ((hItem->subItems.getCount() || (hItem->flags & TLIF_FAKEPARENT)) &&
							!(hItem->flags & TLIF_EXPANDED)) {
							hItem->flags |= TLIF_EXPANDED;
							hItem->flags |= TLIF_MODIFIED;

							NMTREEVIEW nmtv;
							nmtv.hdr.code = TVN_ITEMEXPANDED;
							nmtv.hdr.hwndFrom = lpnmlvk->hdr.hwndFrom;
							nmtv.hdr.idFrom = lpnmlvk->hdr.idFrom;
							nmtv.itemNew.hItem = (HTREEITEM)hItem;
							SendMessage(hwnd, WM_NOTIFY, lpnmlvk->hdr.idFrom, (LPARAM)&nmtv);
							TreeList_Update(lpnmlvk->hdr.hwndFrom);
						}
						break;
					}
				}
				break;

			case NM_CLICK:
				if (data->mode == TLM_TREE) {
					LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lparam;
					LVHITTESTINFO lvhti = { 0 };
					lvi.mask = LVIF_PARAM;
					lvi.iItem = lpnmia->iItem;
					ListView_GetItem(lpnmia->hdr.hwndFrom, &lvi);
					lvhti.pt = lpnmia->ptAction;
					ListView_HitTest(lpnmia->hdr.hwndFrom, &lvhti);

					HTREELISTITEM ptli = (HTREELISTITEM)lvi.lParam;
					if ((lvhti.iSubItem == 0) && ((lvhti.flags&LVHT_ONITEM) == LVHT_ONITEMSTATEICON) &&
						(ptli->subItems.getCount() || ptli->flags & TLIF_FAKEPARENT)) {
						if (ptli->flags & TLIF_EXPANDED)
							ptli->flags &= ~TLIF_EXPANDED;
						else {
							ptli->flags |= TLIF_EXPANDED;

							NMTREEVIEW nmtv;
							nmtv.hdr.code = TVN_ITEMEXPANDED;
							nmtv.hdr.hwndFrom = lpnmia->hdr.hwndFrom;
							nmtv.hdr.idFrom = lpnmia->hdr.idFrom;
							nmtv.itemNew.hItem = (HTREEITEM)lvi.lParam;
							SendMessage(hwnd, WM_NOTIFY, lpnmia->hdr.idFrom, (LPARAM)&nmtv);
						}
						ptli->flags |= TLIF_MODIFIED;
						TreeList_Update(lpnmia->hdr.hwndFrom);
					}
				}
				break;
			}
			break;
		}
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////
static int sttTreeList_SortItems_Cmp0(const void *p1, const void *p2) { return  mir_wstrcmp((*(HTREELISTITEM *)p1)->text[0], (*(HTREELISTITEM *)p2)->text[0]); }
static int sttTreeList_SortItems_Cmp1(const void *p1, const void *p2) { return -mir_wstrcmp((*(HTREELISTITEM *)p1)->text[0], (*(HTREELISTITEM *)p2)->text[0]); }
static int sttTreeList_SortItems_Cmp2(const void *p1, const void *p2) { return  mir_wstrcmp((*(HTREELISTITEM *)p1)->text[1], (*(HTREELISTITEM *)p2)->text[1]); }
static int sttTreeList_SortItems_Cmp3(const void *p1, const void *p2) { return -mir_wstrcmp((*(HTREELISTITEM *)p1)->text[1], (*(HTREELISTITEM *)p2)->text[1]); }
static int sttTreeList_SortItems_Cmp4(const void *p1, const void *p2) { return  mir_wstrcmp((*(HTREELISTITEM *)p1)->text[2], (*(HTREELISTITEM *)p2)->text[2]); }
static int sttTreeList_SortItems_Cmp5(const void *p1, const void *p2) { return -mir_wstrcmp((*(HTREELISTITEM *)p1)->text[2], (*(HTREELISTITEM *)p2)->text[2]); }

static void sttTreeList_SortItems(HTREELISTITEM hItem, LPARAM data)
{
	if (!hItem->subItems.getCount()) return;

	typedef int(__cdecl *TQSortCmp)(const void *, const void *);
	static TQSortCmp funcs[] =
	{
		sttTreeList_SortItems_Cmp0,
		sttTreeList_SortItems_Cmp1,
		sttTreeList_SortItems_Cmp2,
		sttTreeList_SortItems_Cmp3,
		sttTreeList_SortItems_Cmp4,
		sttTreeList_SortItems_Cmp5,
	};
	qsort(((SortedList *)&hItem->subItems)->items, hItem->subItems.getCount(), sizeof(void*), funcs[data - 1]);
}

static void sttTreeList_ResetIndex(HTREELISTITEM hItem, LPARAM data)
{
	hItem->flags &= ~TLIF_HASITEM;

	if (!hItem->parent || (hItem->parent->flags & TLIF_VISIBLE) && (hItem->parent->flags & TLIF_EXPANDED))
		hItem->flags |= TLIF_VISIBLE;
	else
		hItem->flags &= ~TLIF_VISIBLE;

	hItem->sortIndex = (*(int *)data)++;
}

static void sttTreeList_FilterItems(HTREELISTITEM hItem, LPARAM data)
{
	int i;
	for (i = 0; i < hItem->text.getCount(); i++)
		if (JabberStrIStr(hItem->text[i], (wchar_t *)data))
			break;

	if (i < hItem->text.getCount()) {
		while (!(hItem->flags & TLIF_ROOT)) {
			hItem->flags |= TLIF_FILTERED;
			hItem = hItem->parent;
		}
	}
	else hItem->flags &= ~TLIF_FILTERED;
}

static void sttTreeList_CreateItems(HTREELISTITEM hItem, LPARAM data)
{
	TTreeList_Data *listData = (TTreeList_Data *)sttTreeList_GeWindowData((HWND)data);
	if ((hItem->flags & TLIF_VISIBLE) && (!listData->filter || (hItem->flags & TLIF_FILTERED)) && !(hItem->flags & TLIF_HASITEM) && !(hItem->flags & TLIF_ROOT)) {
		LVITEM lvi = { 0 };
		lvi.mask = LVIF_INDENT | LVIF_PARAM | LVIF_IMAGE | LVIF_TEXT | LVIF_STATE;
		lvi.iIndent = hItem->indent;
		lvi.lParam = (LPARAM)hItem;
		lvi.pszText = hItem->text[0];
		lvi.stateMask = LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK;
		lvi.iImage = hItem->iIcon;
		lvi.state =
			INDEXTOSTATEIMAGEMASK(
			((hItem->subItems.getCount() == 0) && !(hItem->flags & TLIF_FAKEPARENT)) ? 0 :
				(hItem->flags & TLIF_EXPANDED) ? 1 : 2) |
			INDEXTOOVERLAYMASK(hItem->iOverlay);

		int idx = ListView_InsertItem((HWND)data, &lvi);
		for (int i = 1; i < hItem->text.getCount(); i++)
			ListView_SetItemText((HWND)data, idx, i, hItem->text[i]);
	}
}

static void sttTreeList_CreateItems_List(HTREELISTITEM hItem, LPARAM data)
{
	TTreeList_Data *listData = (TTreeList_Data *)sttTreeList_GeWindowData((HWND)data);
	if ((!listData->filter || (hItem->flags & TLIF_FILTERED)) && !(hItem->flags & TLIF_HASITEM) && !(hItem->flags & TLIF_ROOT)) {
		LVITEM lvi = { 0 };
		lvi.mask = LVIF_INDENT | LVIF_PARAM | LVIF_IMAGE | LVIF_TEXT | LVIF_STATE;
		lvi.iIndent = hItem->indent;
		lvi.lParam = (LPARAM)hItem;
		lvi.pszText = hItem->text[0];
		lvi.stateMask = LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK;
		lvi.iImage = hItem->iIcon;
		lvi.state =
			INDEXTOSTATEIMAGEMASK(
			((hItem->subItems.getCount() == 0) && !(hItem->flags & TLIF_FAKEPARENT)) ? 0 : 3) |
			INDEXTOOVERLAYMASK(hItem->iOverlay);

		int idx = ListView_InsertItem((HWND)data, &lvi);
		for (int i = 1; i < hItem->text.getCount(); i++)
			ListView_SetItemText((HWND)data, idx, i, hItem->text[i]);
	}
}

static int CALLBACK sttTreeList_SortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM)
{
	HTREELISTITEM p1 = (HTREELISTITEM)lParam1, p2 = (HTREELISTITEM)lParam2;
	if (p1->sortIndex < p2->sortIndex)
		return -1;

	if (p1->sortIndex > p2->sortIndex)
		return +1;

	return 0;
}
