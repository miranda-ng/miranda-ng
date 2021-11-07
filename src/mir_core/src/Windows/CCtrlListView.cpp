/*

Object UI extensions
Copyright (c) 2008  Victor Pavlychko, George Hazan
Copyright (C) 2012-21 Miranda NG team

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

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlListView

CCtrlListView::CCtrlListView(CDlgBase *dlg, int ctrlId)
	: CCtrlBase(dlg, ctrlId)
{}

BOOL CCtrlListView::OnNotify(int, NMHDR *pnmh)
{
	TEventInfo evt = { this, pnmh };

	switch (pnmh->code) {
	case NM_CLICK:              OnClick(&evt);             return TRUE;
	case NM_DBLCLK:             OnDoubleClick(&evt);       return TRUE;
	case NM_CUSTOMDRAW:         OnCustomDraw(&evt);        return TRUE;
	case LVN_BEGINDRAG:         OnBeginDrag(&evt);         return TRUE;
	case LVN_BEGINLABELEDIT:    OnBeginLabelEdit(&evt);    return TRUE;
	case LVN_BEGINRDRAG:        OnBeginRDrag(&evt);        return TRUE;
	case LVN_BEGINSCROLL:       OnBeginScroll(&evt);       return TRUE;
	case LVN_COLUMNCLICK:       OnColumnClick(&evt);       return TRUE;
	case LVN_DELETEALLITEMS:    OnDeleteAllItems(&evt);    return TRUE;
	case LVN_DELETEITEM:        OnDeleteItem(&evt);        return TRUE;
	case LVN_ENDLABELEDIT:      OnEndLabelEdit(&evt);      return TRUE;
	case LVN_ENDSCROLL:         OnEndScroll(&evt);         return TRUE;
	case LVN_GETDISPINFO:       OnGetDispInfo(&evt);       return TRUE;
	case LVN_GETINFOTIP:        OnGetInfoTip(&evt);        return TRUE;
	case LVN_HOTTRACK:          OnHotTrack(&evt);          return TRUE;
	case LVN_INSERTITEM:        OnInsertItem(&evt);        return TRUE;
	case LVN_ITEMACTIVATE:      OnItemActivate(&evt);      return TRUE;
	case LVN_ITEMCHANGING:      OnItemChanging(&evt);      return TRUE;
	case LVN_KEYDOWN:           OnKeyDown(&evt);           return TRUE;
	case LVN_MARQUEEBEGIN:      OnMarqueeBegin(&evt);      return TRUE;
	case LVN_SETDISPINFO:       OnSetDispInfo(&evt);       return TRUE;

	case LVN_ITEMCHANGED:
		if (!m_parentWnd || !m_parentWnd->IsInitialized())
			return FALSE;

		OnItemChanged(&evt);

		// item's state is calculated as 1/2 << 12, so we check it to filter out all non-state changes
		if (evt.nmlv->uChanged & LVIF_STATE)
			if ((evt.nmlv->uOldState >> 12) != 0 && (evt.nmlv->uNewState >> 12) != 0)
				NotifyChange();
		return TRUE;

	case LVN_ODSTATECHANGED:
		NotifyChange();
		return TRUE;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int CALLBACK LVMoveSortProc(LPARAM l1, LPARAM l2, LPARAM param)
{
	int result = l1 - l2;
	int newItem = HIWORD(param);
	int oldItem = LOWORD(param);
	if (newItem > oldItem)
		return (l1 == oldItem && l2 <= newItem) ? 1 : result;

	return (l2 == oldItem && l1 >= newItem) ? 1 : result;
}

int CCtrlListView::MoveItem(int idx, int direction)
{
	if ((direction > 0 && idx >= GetItemCount() - 1) || (direction < 0 && idx <= 0))
		return idx;

	if (idx < 0)
		idx = GetNextItem(-1, LVNI_FOCUSED);
	SortItemsEx(&LVMoveSortProc, MAKELONG(idx, idx + direction));
	return idx + direction;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CCtrlListView::SetCurSel(int idx)
{
	SetItemState(idx, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
}

// additional api
HIMAGELIST CCtrlListView::CreateImageList(int iImageList)
{
	HIMAGELIST hIml = GetImageList(iImageList);
	if (hIml)
		return hIml;

	hIml = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 1);
	SetImageList(hIml, iImageList);
	return hIml;
}

void CCtrlListView::AddColumn(int iSubItem, const wchar_t *name, int cx)
{
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvc.iImage = 0;
	lvc.pszText = (LPWSTR)name;
	lvc.cx = cx;
	lvc.iSubItem = iSubItem;
	InsertColumn(iSubItem, &lvc);
}

void CCtrlListView::AddGroup(int iGroupId, const wchar_t *name)
{
	LVGROUP lvg = { 0 };
	lvg.cbSize = sizeof(lvg);
	lvg.mask = LVGF_HEADER | LVGF_GROUPID;
	lvg.pszHeader = (LPWSTR)name;
	lvg.cchHeader = (int)mir_wstrlen(lvg.pszHeader);
	lvg.iGroupId = iGroupId;
	InsertGroup(-1, &lvg);
}

int CCtrlListView::AddItem(const wchar_t *text, int iIcon, LPARAM lParam, int iGroupId)
{
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
	lvi.iSubItem = 0;
	lvi.pszText = (LPWSTR)text;
	lvi.iImage = iIcon;
	lvi.lParam = lParam;
	if (iGroupId >= 0) {
		lvi.mask |= LVIF_GROUPID;
		lvi.iGroupId = iGroupId;
	}

	return InsertItem(&lvi);
}

void CCtrlListView::SetItem(int iItem, int iSubItem, const wchar_t *text, int iIcon)
{
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_TEXT;
	lvi.iItem = iItem;
	lvi.iSubItem = iSubItem;
	lvi.pszText = (LPWSTR)text;
	if (iIcon >= 0) {
		lvi.mask |= LVIF_IMAGE;
		lvi.iImage = iIcon;
	}

	SetItem(&lvi);
}

LPARAM CCtrlListView::GetItemData(int iItem) const
{
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_PARAM;
	lvi.iItem = iItem;
	return GetItem(&lvi) ? lvi.lParam : -1;
}

void CCtrlListView::GetCaretPos(CContextMenuPos &pos) const
{
	pos.pCtrl = this;

	// position is empty, let's fill it using selection
	if (pos.pt.x == 0 && pos.pt.y == 0) {
		pos.iCurr = GetSelectionMark();
		if (pos.iCurr != -1) {
			RECT rc;
			GetItemRect(pos.iCurr, &rc, TRUE);
			pos.pt.x = rc.left + 8;
			pos.pt.y = rc.top + 8;
			ClientToScreen(m_hwnd, &pos.pt);
			return;
		}
	}
	// position is present, let's calculate current item
	else {
		LVHITTESTINFO hti;
		hti.pt = pos.pt;
		ScreenToClient(m_hwnd, &hti.pt);
		if (SubItemHitTest(&hti) != -1) {
			pos.iCurr = hti.iItem;
			return;
		}
	}
	CSuper::GetCaretPos(pos);
}

// classic api
DWORD CCtrlListView::ApproximateViewRect(int cx, int cy, int iCount)
{	return ListView_ApproximateViewRect(m_hwnd, cx, cy, iCount);
}
void CCtrlListView::Arrange(UINT code)
{	ListView_Arrange(m_hwnd, code);
}
void CCtrlListView::CancelEditLabel()
{	ListView_CancelEditLabel(m_hwnd);
}
HIMAGELIST CCtrlListView::CreateDragImage(int iItem, LPPOINT lpptUpLeft)
{	return ListView_CreateDragImage(m_hwnd, iItem, lpptUpLeft);
}
void CCtrlListView::DeleteAllItems()
{	ListView_DeleteAllItems(m_hwnd);
}
void CCtrlListView::DeleteColumn(int iCol)
{	ListView_DeleteColumn(m_hwnd, iCol);
}
void CCtrlListView::DeleteItem(int iItem)
{	ListView_DeleteItem(m_hwnd, iItem);
}
HWND CCtrlListView::EditLabel(int iItem)
{	return ListView_EditLabel(m_hwnd, iItem);
}
int CCtrlListView::EnableGroupView(BOOL fEnable)
{	return ListView_EnableGroupView(m_hwnd, fEnable);
}
BOOL CCtrlListView::EnsureVisible(int i, BOOL fPartialOK)
{	return ListView_EnsureVisible(m_hwnd, i, fPartialOK);
}
int CCtrlListView::FindItem(int iStart, const LVFINDINFO *plvfi)
{	return ListView_FindItem(m_hwnd, iStart, plvfi);
}
COLORREF CCtrlListView::GetBkColor() const
{	return ListView_GetBkColor(m_hwnd);
}
void CCtrlListView::GetBkImage(LPLVBKIMAGE plvbki) const
{	ListView_GetBkImage(m_hwnd, plvbki);
}
UINT CCtrlListView::GetCallbackMask() const
{	return ListView_GetCallbackMask(m_hwnd);
}
BOOL CCtrlListView::GetCheckState(UINT iIndex) const
{	return ListView_GetCheckState(m_hwnd, iIndex);
}
void CCtrlListView::GetColumn(int iCol, LPLVCOLUMN pcol) const
{	ListView_GetColumn(m_hwnd, iCol, pcol);
}
void CCtrlListView::GetColumnOrderArray(int iCount, int *lpiArray) const
{	ListView_GetColumnOrderArray(m_hwnd, iCount, lpiArray);
}
int CCtrlListView::GetColumnWidth(int iCol) const
{	return ListView_GetColumnWidth(m_hwnd, iCol);
}
int CCtrlListView::GetCountPerPage() const
{	return ListView_GetCountPerPage(m_hwnd);
}
HWND CCtrlListView::GetEditControl() const
{	return ListView_GetEditControl(m_hwnd);
}
DWORD CCtrlListView::GetExtendedListViewStyle() const
{	return ListView_GetExtendedListViewStyle(m_hwnd);
}
void CCtrlListView::GetGroupMetrics(LVGROUPMETRICS *pGroupMetrics) const
{	ListView_GetGroupMetrics(m_hwnd, pGroupMetrics);
}
HWND CCtrlListView::GetHeader() const
{	return ListView_GetHeader(m_hwnd);
}
HCURSOR CCtrlListView::GetHotCursor() const
{	return ListView_GetHotCursor(m_hwnd);
}
INT CCtrlListView::GetHotItem() const
{	return ListView_GetHotItem(m_hwnd);
}
DWORD CCtrlListView::GetHoverTime() const
{	return ListView_GetHoverTime(m_hwnd);
}
HIMAGELIST CCtrlListView::GetImageList(int iImageList) const
{	return ListView_GetImageList(m_hwnd, iImageList);
}
BOOL CCtrlListView::GetInsertMark(LVINSERTMARK *plvim) const
{	return ListView_GetInsertMark(m_hwnd, plvim);
}
COLORREF CCtrlListView::GetInsertMarkColor() const
{	return ListView_GetInsertMarkColor(m_hwnd);
}
int CCtrlListView::GetInsertMarkRect(LPRECT prc) const
{	return ListView_GetInsertMarkRect(m_hwnd, prc);
}
BOOL CCtrlListView::GetISearchString(LPSTR lpsz) const
{	return ListView_GetISearchString(m_hwnd, lpsz);
}
bool CCtrlListView::GetItem(LPLVITEM pitem) const
{	return ListView_GetItem(m_hwnd, pitem) == TRUE;
}
int CCtrlListView::GetItemCount() const
{	return ListView_GetItemCount(m_hwnd);
}
void CCtrlListView::GetItemPosition(int i, POINT *ppt) const
{	ListView_GetItemPosition(m_hwnd, i, ppt);
}
void CCtrlListView::GetItemRect(int i, RECT *prc, int code) const
{	ListView_GetItemRect(m_hwnd, i, prc, code);
}
DWORD CCtrlListView::GetItemSpacing(BOOL fSmall) const
{	return ListView_GetItemSpacing(m_hwnd, fSmall);
}
UINT CCtrlListView::GetItemState(int i, UINT mask) const
{	return ListView_GetItemState(m_hwnd, i, mask);
}
void CCtrlListView::GetItemText(int iItem, int iSubItem, LPTSTR pszText, int cchTextMax) const
{	ListView_GetItemText(m_hwnd, iItem, iSubItem, pszText, cchTextMax);
}
int CCtrlListView::GetNextItem(int iStart, UINT flags) const
{	return ListView_GetNextItem(m_hwnd, iStart, flags);
}
BOOL CCtrlListView::GetNumberOfWorkAreas(LPUINT lpuWorkAreas) const
{	return  ListView_GetNumberOfWorkAreas(m_hwnd, lpuWorkAreas);
}
BOOL CCtrlListView::GetOrigin(LPPOINT lpptOrg) const
{	return ListView_GetOrigin(m_hwnd, lpptOrg);
}
COLORREF CCtrlListView::GetOutlineColor() const
{	return ListView_GetOutlineColor(m_hwnd);
}
UINT CCtrlListView::GetSelectedColumn() const
{	return ListView_GetSelectedColumn(m_hwnd);
}
UINT CCtrlListView::GetSelectedCount() const
{	return ListView_GetSelectedCount(m_hwnd);
}
INT CCtrlListView::GetSelectionMark() const
{	return ListView_GetSelectionMark(m_hwnd);
}
int CCtrlListView::GetStringWidth(LPCSTR psz) const
{	return ListView_GetStringWidth(m_hwnd, psz);
}
BOOL CCtrlListView::GetSubItemRect(int iItem, int iSubItem, int code, LPRECT lpRect) const
{	return ListView_GetSubItemRect(m_hwnd, iItem, iSubItem, code, lpRect);
}
COLORREF CCtrlListView::GetTextBkColor() const
{	return ListView_GetTextBkColor(m_hwnd);
}
COLORREF CCtrlListView::GetTextColor() const
{	return ListView_GetTextColor(m_hwnd);
}
void CCtrlListView::GetTileInfo(PLVTILEINFO plvtinfo) const
{	ListView_GetTileInfo(m_hwnd, plvtinfo);
}
void CCtrlListView::GetTileViewInfo(PLVTILEVIEWINFO plvtvinfo) const
{	ListView_GetTileViewInfo(m_hwnd, plvtvinfo);
}
HWND CCtrlListView::GetToolTips() const
{	return ListView_GetToolTips(m_hwnd);
}
int CCtrlListView::GetTopIndex() const
{	return ListView_GetTopIndex(m_hwnd);
}
BOOL CCtrlListView::GetUnicodeFormat() const
{	return ListView_GetUnicodeFormat(m_hwnd);
}
DWORD CCtrlListView::GetView() const
{	return ListView_GetView(m_hwnd);
}
BOOL CCtrlListView::GetViewRect(RECT *prc) const
{	return ListView_GetViewRect(m_hwnd, prc);
}
void CCtrlListView::GetWorkAreas(INT nWorkAreas, LPRECT lprc) const
{	ListView_GetWorkAreas(m_hwnd, nWorkAreas, lprc);
}
BOOL CCtrlListView::HasGroup(int dwGroupId)
{	return ListView_HasGroup(m_hwnd, dwGroupId);
}
int CCtrlListView::HitTest(LPLVHITTESTINFO pinfo) const
{	return ListView_HitTest(m_hwnd, pinfo);
}
int CCtrlListView::InsertColumn(int iCol, const LVCOLUMN *pcol)
{	return ListView_InsertColumn(m_hwnd, iCol, pcol);
}
int CCtrlListView::InsertGroup(int index, PLVGROUP pgrp)
{	return ListView_InsertGroup(m_hwnd, index, pgrp);
}
void CCtrlListView::InsertGroupSorted(PLVINSERTGROUPSORTED structInsert)
{	ListView_InsertGroupSorted(m_hwnd, structInsert);
}
int CCtrlListView::InsertItem(const LVITEM *pitem)
{	return ListView_InsertItem(m_hwnd, pitem);
}
BOOL CCtrlListView::InsertMarkHitTest(LPPOINT point, LVINSERTMARK *plvim)
{	return ListView_InsertMarkHitTest(m_hwnd, point, plvim);
}
BOOL CCtrlListView::IsGroupViewEnabled()
{	return ListView_IsGroupViewEnabled(m_hwnd);
}
UINT CCtrlListView::MapIDToIndex(UINT id)
{	return ListView_MapIDToIndex(m_hwnd, id);
}
UINT CCtrlListView::MapIndexToID(UINT index)
{	return ListView_MapIndexToID(m_hwnd, index);
}
BOOL CCtrlListView::RedrawItems(int iFirst, int iLast)
{	return ListView_RedrawItems(m_hwnd, iFirst, iLast);
}
void CCtrlListView::RemoveAllGroups()
{	ListView_RemoveAllGroups(m_hwnd);
}
int CCtrlListView::RemoveGroup(int iGroupId)
{	return ListView_RemoveGroup(m_hwnd, iGroupId);
}
BOOL CCtrlListView::Scroll(int dx, int dy)
{	return ListView_Scroll(m_hwnd, dx, dy);
}
BOOL CCtrlListView::SetBkColor(COLORREF clrBk)
{	return ListView_SetBkColor(m_hwnd, clrBk);
}
BOOL CCtrlListView::SetBkImage(LPLVBKIMAGE plvbki)
{	return ListView_SetBkImage(m_hwnd, plvbki);
}
BOOL CCtrlListView::SetCallbackMask(UINT mask)
{	return ListView_SetCallbackMask(m_hwnd, mask);
}
void CCtrlListView::SetCheckState(UINT iIndex, BOOL fCheck)
{	ListView_SetCheckState(m_hwnd, iIndex, fCheck);
}
BOOL CCtrlListView::SetColumn(int iCol, LPLVCOLUMN pcol)
{	return ListView_SetColumn(m_hwnd, iCol, pcol);
}
BOOL CCtrlListView::SetColumnOrderArray(int iCount, int *lpiArray)
{	return ListView_SetColumnOrderArray(m_hwnd, iCount, lpiArray);
}
BOOL CCtrlListView::SetColumnWidth(int iCol, int cx)
{	return ListView_SetColumnWidth(m_hwnd, iCol, cx);
}
void CCtrlListView::SetExtendedListViewStyle(DWORD dwExStyle)
{	ListView_SetExtendedListViewStyle(m_hwnd, dwExStyle);
}
void CCtrlListView::SetExtendedListViewStyleEx(DWORD dwExMask, DWORD dwExStyle)
{	ListView_SetExtendedListViewStyleEx(m_hwnd, dwExMask, dwExStyle);
}
int CCtrlListView::SetGroupInfo(int iGroupId, PLVGROUP pgrp)
{	return ListView_SetGroupInfo(m_hwnd, iGroupId, pgrp);
}
void CCtrlListView::SetGroupMetrics(PLVGROUPMETRICS pGroupMetrics)
{	ListView_SetGroupMetrics(m_hwnd, pGroupMetrics);
}
HCURSOR CCtrlListView::SetHotCursor(HCURSOR hCursor)
{	return ListView_SetHotCursor(m_hwnd, hCursor);
}
INT CCtrlListView::SetHotItem(INT iIndex)
{	return ListView_SetHotItem(m_hwnd, iIndex);
}
void CCtrlListView::SetHoverTime(DWORD dwHoverTime)
{	ListView_SetHoverTime(m_hwnd, dwHoverTime);
}
DWORD CCtrlListView::SetIconSpacing(int cx, int cy)
{	return ListView_SetIconSpacing(m_hwnd, cx, cy);
}
HIMAGELIST CCtrlListView::SetImageList(HIMAGELIST himl, int iImageList)
{	return ListView_SetImageList(m_hwnd, himl, iImageList);
}
BOOL CCtrlListView::SetInfoTip(PLVSETINFOTIP plvSetInfoTip)
{	return ListView_SetInfoTip(m_hwnd, plvSetInfoTip);
}
BOOL CCtrlListView::SetInsertMark(LVINSERTMARK *plvim)
{	return ListView_SetInsertMark(m_hwnd, plvim);
}
COLORREF CCtrlListView::SetInsertMarkColor(COLORREF color)
{	return ListView_SetInsertMarkColor(m_hwnd, color);
}
BOOL CCtrlListView::SetItem(const LVITEM *pitem)
{	return ListView_SetItem(m_hwnd, pitem);
}
void CCtrlListView::SetItemCount(int cItems)
{	ListView_SetItemCount(m_hwnd, cItems);
}
void CCtrlListView::SetItemCountEx(int cItems, DWORD dwFlags)
{	ListView_SetItemCountEx(m_hwnd, cItems, dwFlags);
}
BOOL CCtrlListView::SetItemPosition(int i, int x, int y)
{	return ListView_SetItemPosition(m_hwnd, i, x, y);
}
void CCtrlListView::SetItemPosition32(int iItem, int x, int y)
{	ListView_SetItemPosition32(m_hwnd, iItem, x, y);
}
void CCtrlListView::SetItemState(int i, UINT state, UINT mask)
{	ListView_SetItemState(m_hwnd, i, state, mask);
}
void CCtrlListView::SetItemText(int i, int iSubItem, const wchar_t *pszText)
{	ListView_SetItemText(m_hwnd, i, iSubItem, (LPWSTR)pszText);
}
COLORREF CCtrlListView::SetOutlineColor(COLORREF color)
{	return ListView_SetOutlineColor(m_hwnd, color);
}
void CCtrlListView::SetSelectedColumn(int iCol)
{	ListView_SetSelectedColumn(m_hwnd, iCol);
}
INT CCtrlListView::SetSelectionMark(INT iIndex)
{	return ListView_SetSelectionMark(m_hwnd, iIndex);
}
BOOL CCtrlListView::SetTextBkColor(COLORREF clrText)
{	return ListView_SetTextBkColor(m_hwnd, clrText);
}
BOOL CCtrlListView::SetTextColor(COLORREF clrText)
{	return ListView_SetTextColor(m_hwnd, clrText);
}
BOOL CCtrlListView::SetTileInfo(PLVTILEINFO plvtinfo)
{	return ListView_SetTileInfo(m_hwnd, plvtinfo);
}
BOOL CCtrlListView::SetTileViewInfo(PLVTILEVIEWINFO plvtvinfo)
{	return ListView_SetTileViewInfo(m_hwnd, plvtvinfo);
}
HWND CCtrlListView::SetToolTips(HWND ToolTip)
{	return ListView_SetToolTips(m_hwnd, ToolTip);
}
BOOL CCtrlListView::SetUnicodeFormat(BOOL fUnicode)
{	return ListView_SetUnicodeFormat(m_hwnd, fUnicode);
}
int CCtrlListView::SetView(DWORD iView)
{	return ListView_SetView(m_hwnd, iView);
}
void CCtrlListView::SetWorkAreas(INT nWorkAreas, LPRECT lprc)
{	ListView_SetWorkAreas(m_hwnd, nWorkAreas, lprc);
}
int CCtrlListView::SortGroups(PFNLVGROUPCOMPARE pfnGroupCompare, LPVOID plv)
{	return ListView_SortGroups(m_hwnd, pfnGroupCompare, plv);
}
BOOL CCtrlListView::SortItems(PFNLVCOMPARE pfnCompare, LPARAM lParamSort)
{	return ListView_SortItems(m_hwnd, pfnCompare, lParamSort);
}
BOOL CCtrlListView::SortItemsEx(PFNLVCOMPARE pfnCompare, LPARAM lParamSort)
{	return ListView_SortItemsEx(m_hwnd, pfnCompare, lParamSort);
}
INT CCtrlListView::SubItemHitTest(LPLVHITTESTINFO pInfo) const
{	return ListView_SubItemHitTest(m_hwnd, pInfo);
}
BOOL CCtrlListView::Update(int iItem)
{	return ListView_Update(m_hwnd, iItem);
}
