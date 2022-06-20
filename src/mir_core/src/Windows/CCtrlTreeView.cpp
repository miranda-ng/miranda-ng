/*

Object UI extensions
Copyright (c) 2008  Victor Pavlychko, George Hazan
Copyright (C) 2012-22 Miranda NG team

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

#include "../stdafx.h"

int ImageList_AddIcon_IconLibLoaded(HIMAGELIST hIml, int iconId)
{
	HICON hIcon = Skin_LoadIcon(iconId);
	int res = ImageList_AddIcon(hIml, hIcon);
	IcoLib_ReleaseIcon(hIcon);
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlTreeView

CCtrlTreeView::CCtrlTreeView(CDlgBase *dlg, int ctrlId) :
	CCtrlBase(dlg, ctrlId),
	m_dwFlags(0),
	m_hDragItem(nullptr)
{}

void CCtrlTreeView::SetFlags(uint32_t dwFlags)
{
	if (dwFlags & MTREE_CHECKBOX)
		m_bCheckBox = true;

	if (dwFlags & MTREE_MULTISELECT)
		m_bMultiSelect = true;

	if (dwFlags & MTREE_DND) {
		m_bDndEnabled = true;
		m_bDragging = false;
		m_hDragItem = nullptr;
	}
}

void CCtrlTreeView::OnInit()
{
	CSuper::OnInit();

	Subclass();

	if (m_bCheckBox) {
		HIMAGELIST himlCheckBoxes = ::ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2);
		::ImageList_AddIcon_IconLibLoaded(himlCheckBoxes, SKINICON_OTHER_NOTICK);
		::ImageList_AddIcon_IconLibLoaded(himlCheckBoxes, SKINICON_OTHER_TICK);
		SetImageList(himlCheckBoxes, TVSIL_NORMAL);
	}
}

void CCtrlTreeView::OnDestroy()
{
	if (m_bCheckBox)
		::ImageList_Destroy(GetImageList(TVSIL_NORMAL));

	CSuper::OnDestroy();
}

HTREEITEM CCtrlTreeView::MoveItemAbove(HTREEITEM hItem, HTREEITEM hInsertAfter, HTREEITEM hParent)
{
	if (hItem == nullptr || hInsertAfter == nullptr)
		return nullptr;

	if (hItem == hInsertAfter)
		return hItem;

	wchar_t name[128];
	TVINSERTSTRUCT tvis = {};
	tvis.itemex.mask = (UINT)-1;
	tvis.itemex.pszText = name;
	tvis.itemex.cchTextMax = _countof(name);
	tvis.itemex.hItem = hItem;
	if (!GetItem(&tvis.itemex))
		return nullptr;

	OBJLIST<TVINSERTSTRUCT> arChildren(1);
	for (HTREEITEM p = GetChild(hItem); p; p = GetNextSibling(p)) {
		wchar_t buf[128];
		TVINSERTSTRUCT tvis2 = {};
		tvis2.itemex.mask = (UINT)-1;
		tvis2.itemex.pszText = buf;
		tvis2.itemex.cchTextMax = _countof(buf);
		tvis2.itemex.hItem = p;
		if (GetItem(&tvis2.itemex)) {
			tvis2.itemex.pszText = mir_wstrdup(tvis2.itemex.pszText);
			arChildren.insert(new TVINSERTSTRUCT(tvis2));

			tvis2.itemex.lParam = 0;
			SetItem(&tvis2.itemex);
		}
	}

	// the pointed lParam will be freed inside TVN_DELETEITEM
	// so lets substitute it with 0
	LPARAM saveOldData = tvis.itemex.lParam;
	tvis.itemex.lParam = 0;
	SetItem(&tvis.itemex);

	// now current item contain lParam = 0 we can delete it. the memory will be kept.
	DeleteItem(hItem);

	for (auto &it : arChildren)
		DeleteItem(it->itemex.hItem);

	tvis.itemex.stateMask = tvis.itemex.state;
	tvis.itemex.lParam = saveOldData;
	tvis.hParent = hParent;
	tvis.hInsertAfter = hInsertAfter;
	auto hNewItem = InsertItem(&tvis);

	hInsertAfter = nullptr;
	for (auto &it : arChildren) {
		it->hParent = hNewItem;
		it->hInsertAfter = hInsertAfter;
		hInsertAfter = InsertItem(it);

		mir_free(it->itemex.pszText);
	}

	return hNewItem;
}

LRESULT CCtrlTreeView::CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	TVHITTESTINFO hti;

	switch (msg) {
	case WM_MOUSEMOVE:
		if (m_bDragging) {
			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam);
			HitTest(&hti);
			if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) {
				HTREEITEM it = hti.hItem;
				hti.pt.y -= GetItemHeight() / 2;
				HitTest(&hti);
				if (!(hti.flags & TVHT_ABOVE))
					SetInsertMark(hti.hItem, 1);
				else
					SetInsertMark(it, 0);
			}
			else {
				if (hti.flags & TVHT_ABOVE) SendMsg(WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
				if (hti.flags & TVHT_BELOW) SendMsg(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
				SetInsertMark(nullptr, 0);
			}
		}
		break;

	case WM_LBUTTONUP:
		if (m_bDragging) {
			SetInsertMark(nullptr, 0);
			m_bDragging = false;
			ReleaseCapture();

			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam) - GetItemHeight() / 2;
			HitTest(&hti);
			if (m_hDragItem == hti.hItem)
				break;

			if (hti.flags & TVHT_ABOVE)
				hti.hItem = TVI_FIRST;
			else if (hti.flags & TVHT_BELOW)
				hti.hItem = TVI_LAST;

			HTREEITEM insertAfter = hti.hItem, hParent;
			if (insertAfter != TVI_FIRST) {
				hParent = GetParent(insertAfter);
				if (GetChild(insertAfter) != nullptr) {
					hParent = insertAfter;
					insertAfter = TVI_FIRST;
				}
			}
			else hParent = nullptr;

			HTREEITEM FirstItem = nullptr;
			if (m_bMultiSelect) {
				LIST<_TREEITEM> arItems(10);
				GetSelected(arItems);

				// Proceed moving
				for (auto &it : arItems) {
					if (!insertAfter)
						break;
					if (GetParent(it) != hParent) // prevent subitems from being inserted at the same level
						continue;

					insertAfter = MoveItemAbove(it, insertAfter, hParent);
					if (it == arItems[0])
						FirstItem = insertAfter;
				}
			}
			else FirstItem = MoveItemAbove(m_hDragItem, insertAfter, hParent);
			if (FirstItem)
				SelectItem(FirstItem);

			NotifyChange();
		}
		break;

	case WM_LBUTTONDOWN:
		if (!m_bMultiSelect)
			break;

		hti.pt.x = (short)LOWORD(lParam);
		hti.pt.y = (short)HIWORD(lParam);
		if (!TreeView_HitTest(m_hwnd, &hti)) {
			UnselectAll();
			break;
		}

		if (!m_bDndEnabled)
			if (!(wParam & (MK_CONTROL | MK_SHIFT)) || !(hti.flags & (TVHT_ONITEMICON | TVHT_ONITEMLABEL | TVHT_ONITEMRIGHT))) {
				UnselectAll();
				TreeView_SelectItem(m_hwnd, hti.hItem);
				break;
			}

		if (wParam & MK_CONTROL) {
			LIST<_TREEITEM> selected(1);
			GetSelected(selected);

			// Check if have to deselect it
			for (int i = 0; i < selected.getCount(); i++) {
				if (selected[i] == hti.hItem) {
					// Deselect it
					UnselectAll();
					selected.remove(i);

					if (i > 0)
						hti.hItem = selected[0];
					else if (i < selected.getCount())
						hti.hItem = selected[i];
					else
						hti.hItem = nullptr;
					break;
				}
			}

			TreeView_SelectItem(m_hwnd, hti.hItem);
			Select(selected);
		}
		else if (wParam & MK_SHIFT) {
			HTREEITEM hItem = TreeView_GetSelection(m_hwnd);
			if (hItem == nullptr)
				break;

			LIST<_TREEITEM> selected(1);
			GetSelected(selected);

			TreeView_SelectItem(m_hwnd, hti.hItem);
			Select(selected);
			SelectRange(hItem, hti.hItem);
		}
		break;
	}

	return CSuper::CustomWndProc(msg, wParam, lParam);
}

BOOL CCtrlTreeView::OnNotify(int, NMHDR *pnmh)
{
	TEventInfo evt = { this, pnmh };

	switch (pnmh->code) {
	case NM_RCLICK:			 OnRightClick(&evt);      return TRUE;
	case NM_CUSTOMDRAW:      OnCustomDraw(&evt);      return TRUE;
	case TVN_BEGINLABELEDIT: OnBeginLabelEdit(&evt);  return TRUE;
	case TVN_BEGINRDRAG:     OnBeginRDrag(&evt);      return TRUE;
	case TVN_DELETEITEM:     OnDeleteItem(&evt);      return TRUE;
	case TVN_ENDLABELEDIT:   OnEndLabelEdit(&evt);    return TRUE;
	case TVN_GETDISPINFO:    OnGetDispInfo(&evt);     return TRUE;
	case TVN_GETINFOTIP:     OnGetInfoTip(&evt);      return TRUE;
	case TVN_ITEMEXPANDED:   OnItemExpanded(&evt);    return TRUE;
	case TVN_ITEMEXPANDING:  OnItemExpanding(&evt);   return TRUE;
	case TVN_SELCHANGED:     OnSelChanged(&evt);      return TRUE;
	case TVN_SELCHANGING:    OnSelChanging(&evt);     return TRUE;
	case TVN_SETDISPINFO:    OnSetDispInfo(&evt);     return TRUE;
	case TVN_SINGLEEXPAND:   OnSingleExpand(&evt);    return TRUE;

	case TVN_BEGINDRAG:
		OnBeginDrag(&evt);

		// user-defined can clear the event code to disable dragging
		if (m_bDndEnabled && pnmh->code) {
			::SetCapture(m_hwnd);
			m_bDragging = true;
			m_hDragItem = evt.nmtv->itemNew.hItem;
			SelectItem(m_hDragItem);
		}
		return TRUE;

	case TVN_KEYDOWN:
		if (evt.nmtvkey->wVKey == VK_SPACE) {
			evt.hItem = GetSelection();
			if (m_bCheckBox)
				InvertCheck(evt.hItem);
			OnItemChanged(&evt);
			NotifyChange();
		}

		OnKeyDown(&evt);
		return TRUE;
	}

	if (pnmh->code == NM_CLICK) {
		TVHITTESTINFO hti;
		hti.pt.x = (short)LOWORD(GetMessagePos());
		hti.pt.y = (short)HIWORD(GetMessagePos());
		ScreenToClient(pnmh->hwndFrom, &hti.pt);
		if (HitTest(&hti)) {
			if (m_bCheckBox && (hti.flags & TVHT_ONITEMICON) || !m_bCheckBox && (hti.flags & TVHT_ONITEMSTATEICON)) {
				if (m_bCheckBox)
					InvertCheck(hti.hItem);
				else
					SelectItem(hti.hItem);

				evt.hItem = hti.hItem;
				OnItemChanged(&evt);
				NotifyChange();
			}
		}
	}

	return FALSE;
}

void CCtrlTreeView::InvertCheck(HTREEITEM hItem)
{
	TVITEMEX tvi;
	tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATEEX;
	tvi.hItem = hItem;
	if (!GetItem(&tvi))
		return;

	if (IsWinVerVistaPlus() && (tvi.uStateEx & TVIS_EX_DISABLED))
		return;

	tvi.iImage = tvi.iSelectedImage = !tvi.iImage;
	SetItem(&tvi);
	
	SelectItem(hItem);
}

void CCtrlTreeView::TranslateItem(HTREEITEM hItem)
{
	TVITEMEX tvi;
	wchar_t buf[128];
	GetItem(hItem, &tvi, buf, _countof(buf));
	tvi.pszText = TranslateW_LP(tvi.pszText);
	SetItem(&tvi);
}

void CCtrlTreeView::TranslateTree()
{
	HTREEITEM hItem = GetRoot();
	while (hItem) {
		TranslateItem(hItem);

		HTREEITEM hItemTmp = nullptr;
		if (hItemTmp = GetChild(hItem))
			hItem = hItemTmp;
		else if (hItemTmp = GetNextSibling(hItem))
			hItem = hItemTmp;
		else {
			while (true) {
				if (!(hItem = GetParent(hItem)))
					break;
				if (hItemTmp = GetNextSibling(hItem)) {
					hItem = hItemTmp;
					break;
				}
			}
		}
	}
}

HTREEITEM CCtrlTreeView::FindNamedItem(HTREEITEM hItem, const wchar_t *name)
{
	TVITEMEX tvi = { 0 };
	wchar_t str[MAX_PATH];

	if (hItem)
		tvi.hItem = GetChild(hItem);
	else
		tvi.hItem = GetRoot();

	if (!name)
		return tvi.hItem;

	tvi.mask = TVIF_TEXT;
	tvi.pszText = str;
	tvi.cchTextMax = _countof(str);

	while (tvi.hItem) {
		GetItem(&tvi);

		if (!mir_wstrcmp(tvi.pszText, name))
			return tvi.hItem;

		tvi.hItem = GetNextSibling(tvi.hItem);
	}
	return nullptr;
}

void CCtrlTreeView::GetItem(HTREEITEM hItem, TVITEMEX *tvi) const
{
	memset(tvi, 0, sizeof(*tvi));
	tvi->mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE | TVIF_INTEGRAL | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_STATE;
	tvi->hItem = hItem;
	GetItem(tvi);
}

void CCtrlTreeView::GetItem(HTREEITEM hItem, TVITEMEX *tvi, wchar_t *szText, int iTextLength) const
{
	memset(tvi, 0, sizeof(*tvi));
	tvi->mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE | TVIF_INTEGRAL | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_TEXT;
	tvi->hItem = hItem;
	tvi->pszText = szText;
	tvi->cchTextMax = iTextLength;
	GetItem(tvi);
}

bool CCtrlTreeView::IsSelected(HTREEITEM hItem)
{
	return (TVIS_SELECTED & TreeView_GetItemState(m_hwnd, hItem, TVIS_SELECTED)) == TVIS_SELECTED;
}

void CCtrlTreeView::Select(HTREEITEM hItem)
{
	TreeView_SetItemState(m_hwnd, hItem, TVIS_SELECTED, TVIS_SELECTED);
}

void CCtrlTreeView::Unselect(HTREEITEM hItem)
{
	TreeView_SetItemState(m_hwnd, hItem, 0, TVIS_SELECTED);
}

void CCtrlTreeView::DropHilite(HTREEITEM hItem)
{
	TreeView_SetItemState(m_hwnd, hItem, TVIS_DROPHILITED, TVIS_DROPHILITED);
}

void CCtrlTreeView::DropUnhilite(HTREEITEM hItem)
{
	TreeView_SetItemState(m_hwnd, hItem, 0, TVIS_DROPHILITED);
}

void CCtrlTreeView::SelectAll()
{
	TreeView_SelectItem(m_hwnd, nullptr);

	HTREEITEM hItem = TreeView_GetRoot(m_hwnd);
	while (hItem) {
		Select(hItem);
		hItem = TreeView_GetNextSibling(m_hwnd, hItem);
	}
}

void CCtrlTreeView::UnselectAll()
{
	TreeView_SelectItem(m_hwnd, nullptr);

	HTREEITEM hItem = TreeView_GetRoot(m_hwnd);
	while (hItem) {
		Unselect(hItem);
		hItem = TreeView_GetNextSibling(m_hwnd, hItem);
	}
}

void CCtrlTreeView::SelectRange(HTREEITEM hStart, HTREEITEM hEnd)
{
	int start = 0, end = 0, i = 0;
	HTREEITEM hItem = TreeView_GetRoot(m_hwnd);
	while (hItem) {
		if (hItem == hStart)
			start = i;
		if (hItem == hEnd)
			end = i;

		i++;
		hItem = TreeView_GetNextSibling(m_hwnd, hItem);
	}

	if (end < start) {
		int tmp = start;
		start = end;
		end = tmp;
	}

	i = 0;
	hItem = TreeView_GetRoot(m_hwnd);
	while (hItem) {
		if (i >= start)
			Select(hItem);
		if (i == end)
			break;

		i++;
		hItem = TreeView_GetNextSibling(m_hwnd, hItem);
	}
}

int CCtrlTreeView::GetNumSelected()
{
	int ret = 0;
	for (HTREEITEM hItem = TreeView_GetRoot(m_hwnd); hItem; hItem = TreeView_GetNextSibling(m_hwnd, hItem))
		if (IsSelected(hItem))
			ret++;

	return ret;
}

void CCtrlTreeView::GetSelected(LIST<_TREEITEM> &selected)
{
	HTREEITEM hItem = TreeView_GetRoot(m_hwnd);
	while (hItem) {
		if (IsSelected(hItem))
			selected.insert(hItem);
		hItem = TreeView_GetNextSibling(m_hwnd, hItem);
	}
}

void CCtrlTreeView::Select(LIST<_TREEITEM> &selected)
{
	for (auto &it : selected)
		if (it != nullptr)
			Select(it);
}

void CCtrlTreeView::GetCaretPos(CContextMenuPos &pos) const
{
	pos.pCtrl = this;

	// position is empty, let's fill it using selection
	if (pos.pt.x == 0 && pos.pt.y == 0) {
		HTREEITEM hItem = GetSelection();
		if (hItem != nullptr) {
			pos.pCtrl = this;
			pos.hItem = hItem;

			RECT rc;
			GetItemRect(hItem, &rc, TRUE);
			pos.pt.x = rc.left + 8;
			pos.pt.y = rc.top + 8;
			ClientToScreen(m_hwnd, &pos.pt);
			return;
		}
	}
	// position is present, let's calculate current item
	else {
		TVHITTESTINFO hti;
		hti.pt = pos.pt;
		ScreenToClient(m_hwnd, &hti.pt);
		if (HitTest(&hti) && (hti.flags & TVHT_ONITEM)) {
			pos.hItem = hti.hItem;
			return;
		}
	}

	CSuper::GetCaretPos(pos);
}

/////////////////////////////////////////////////////////////////////////////////////////

HIMAGELIST CCtrlTreeView::CreateDragImage(HTREEITEM hItem)
{	return TreeView_CreateDragImage(m_hwnd, hItem);
}

void CCtrlTreeView::DeleteAllItems()
{	TreeView_DeleteAllItems(m_hwnd);
}

void CCtrlTreeView::DeleteItem(HTREEITEM hItem)
{	TreeView_DeleteItem(m_hwnd, hItem);
}

HWND CCtrlTreeView::EditLabel(HTREEITEM hItem)
{	return TreeView_EditLabel(m_hwnd, hItem);
}

void CCtrlTreeView::EndEditLabelNow(BOOL cancel)
{	TreeView_EndEditLabelNow(m_hwnd, cancel);
}

void CCtrlTreeView::EnsureVisible(HTREEITEM hItem)
{	TreeView_EnsureVisible(m_hwnd, hItem);
}

void CCtrlTreeView::Expand(HTREEITEM hItem, uint32_t flag)
{	TreeView_Expand(m_hwnd, hItem, flag);
}

COLORREF CCtrlTreeView::GetBkColor() const
{	return TreeView_GetBkColor(m_hwnd);
}

uint32_t CCtrlTreeView::GetCheckState(HTREEITEM hItem) const
{	return TreeView_GetCheckState(m_hwnd, hItem);
}

HTREEITEM CCtrlTreeView::GetChild(HTREEITEM hItem) const
{	return TreeView_GetChild(m_hwnd, hItem);
}

int CCtrlTreeView::GetCount() const
{	return TreeView_GetCount(m_hwnd);
}

HTREEITEM CCtrlTreeView::GetDropHilight() const
{	return TreeView_GetDropHilight(m_hwnd);
}

HWND CCtrlTreeView::GetEditControl() const
{	return TreeView_GetEditControl(m_hwnd);
}

HTREEITEM CCtrlTreeView::GetFirstVisible() const
{	return TreeView_GetFirstVisible(m_hwnd);
}

HIMAGELIST CCtrlTreeView::GetImageList(int iImage) const
{	return TreeView_GetImageList(m_hwnd, iImage);
}

int CCtrlTreeView::GetIndent() const
{	return TreeView_GetIndent(m_hwnd);
}

COLORREF CCtrlTreeView::GetInsertMarkColor() const
{	return TreeView_GetInsertMarkColor(m_hwnd);
}

bool CCtrlTreeView::GetItem(TVITEMEX *tvi) const
{	return TreeView_GetItem(m_hwnd, tvi) == TRUE;
}

int CCtrlTreeView::GetItemHeight() const
{	return TreeView_GetItemHeight(m_hwnd);
}

void CCtrlTreeView::GetItemRect(HTREEITEM hItem, RECT *rcItem, BOOL fItemRect) const
{	TreeView_GetItemRect(m_hwnd, hItem, rcItem, fItemRect);
}

uint32_t CCtrlTreeView::GetItemState(HTREEITEM hItem, uint32_t stateMask) const
{	return TreeView_GetItemState(m_hwnd, hItem, stateMask);
}

HTREEITEM CCtrlTreeView::GetLastVisible() const
{	return TreeView_GetLastVisible(m_hwnd);
}

COLORREF CCtrlTreeView::GetLineColor() const
{	return TreeView_GetLineColor(m_hwnd);
}

HTREEITEM CCtrlTreeView::GetNextItem(HTREEITEM hItem, uint32_t flag) const
{	return TreeView_GetNextItem(m_hwnd, hItem, flag);
}

HTREEITEM CCtrlTreeView::GetNextSibling(HTREEITEM hItem) const
{	return TreeView_GetNextSibling(m_hwnd, hItem);
}

HTREEITEM CCtrlTreeView::GetNextVisible(HTREEITEM hItem) const
{	return TreeView_GetNextVisible(m_hwnd, hItem);
}

HTREEITEM CCtrlTreeView::GetParent(HTREEITEM hItem) const
{	return TreeView_GetParent(m_hwnd, hItem);
}

HTREEITEM CCtrlTreeView::GetPrevSibling(HTREEITEM hItem) const
{	return TreeView_GetPrevSibling(m_hwnd, hItem);
}

HTREEITEM CCtrlTreeView::GetPrevVisible(HTREEITEM hItem) const
{	return TreeView_GetPrevVisible(m_hwnd, hItem);
}

HTREEITEM CCtrlTreeView::GetRoot() const
{	return TreeView_GetRoot(m_hwnd);
}

uint32_t CCtrlTreeView::GetScrollTime() const
{	return TreeView_GetScrollTime(m_hwnd);
}

HTREEITEM CCtrlTreeView::GetSelection() const
{	return TreeView_GetSelection(m_hwnd);
}

COLORREF CCtrlTreeView::GetTextColor() const
{	return TreeView_GetTextColor(m_hwnd);
}

HWND CCtrlTreeView::GetToolTips() const
{	return TreeView_GetToolTips(m_hwnd);
}

BOOL CCtrlTreeView::GetUnicodeFormat() const
{	return TreeView_GetUnicodeFormat(m_hwnd);
}

unsigned CCtrlTreeView::GetVisibleCount() const
{	return TreeView_GetVisibleCount(m_hwnd);
}

HTREEITEM CCtrlTreeView::HitTest(TVHITTESTINFO *hti) const
{	return TreeView_HitTest(m_hwnd, hti);
}

HTREEITEM CCtrlTreeView::InsertItem(TVINSERTSTRUCT *tvis)
{	return TreeView_InsertItem(m_hwnd, tvis);
}

void CCtrlTreeView::Select(HTREEITEM hItem, uint32_t flag)
{	TreeView_Select(m_hwnd, hItem, flag);
}

void CCtrlTreeView::SelectDropTarget(HTREEITEM hItem)
{	TreeView_SelectDropTarget(m_hwnd, hItem);
}

void CCtrlTreeView::SelectItem(HTREEITEM hItem)
{	TreeView_SelectItem(m_hwnd, hItem);
}

void CCtrlTreeView::SelectSetFirstVisible(HTREEITEM hItem)
{	TreeView_SelectSetFirstVisible(m_hwnd, hItem);
}

COLORREF CCtrlTreeView::SetBkColor(COLORREF clBack)
{	return TreeView_SetBkColor(m_hwnd, clBack);
}

void CCtrlTreeView::SetCheckState(HTREEITEM hItem, uint32_t state)
{	TreeView_SetCheckState(m_hwnd, hItem, state);
}

HIMAGELIST CCtrlTreeView::SetImageList(HIMAGELIST hIml, int iImage)
{	return TreeView_SetImageList(m_hwnd, hIml, iImage);
}

void CCtrlTreeView::SetIndent(int iIndent)
{	TreeView_SetIndent(m_hwnd, iIndent);
}

void CCtrlTreeView::SetInsertMark(HTREEITEM hItem, BOOL fAfter)
{	TreeView_SetInsertMark(m_hwnd, hItem, fAfter);
}

COLORREF CCtrlTreeView::SetInsertMarkColor(COLORREF clMark)
{	return TreeView_SetInsertMarkColor(m_hwnd, clMark);
}

void CCtrlTreeView::SetItem(TVITEMEX *tvi)
{	TreeView_SetItem(m_hwnd, tvi);
}

void CCtrlTreeView::SetItemHeight(short cyItem)
{	TreeView_SetItemHeight(m_hwnd, cyItem);
}

void CCtrlTreeView::SetItemState(HTREEITEM hItem, uint32_t state, uint32_t stateMask)
{	TreeView_SetItemState(m_hwnd, hItem, state, stateMask);
}

COLORREF CCtrlTreeView::SetLineColor(COLORREF clLine)
{	return TreeView_SetLineColor(m_hwnd, clLine);
}

void CCtrlTreeView::SetScrollTime(UINT uMaxScrollTime)
{	TreeView_SetScrollTime(m_hwnd, uMaxScrollTime);
}

COLORREF CCtrlTreeView::SetTextColor(COLORREF clText)
{	return TreeView_SetTextColor(m_hwnd, clText);
}

HWND CCtrlTreeView::SetToolTips(HWND hwndToolTips)
{	return TreeView_SetToolTips(m_hwnd, hwndToolTips);
}

BOOL CCtrlTreeView::SetUnicodeFormat(BOOL fUnicode)
{	return TreeView_SetUnicodeFormat(m_hwnd, fUnicode);
}

void CCtrlTreeView::SortChildren(HTREEITEM hItem, BOOL fRecurse)
{	TreeView_SortChildren(m_hwnd, hItem, fRecurse);
}

void CCtrlTreeView::SortChildrenCB(TVSORTCB *cb, BOOL fRecurse)
{	TreeView_SortChildrenCB(m_hwnd, cb, fRecurse);
}
