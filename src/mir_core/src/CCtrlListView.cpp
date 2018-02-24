/*

Object UI extensions
Copyright (c) 2008  Victor Pavlychko, George Hazan
Copyright (c) 2012-18 Miranda NG team

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
		case LVN_ITEMCHANGED:       OnItemChanged(&evt);       return TRUE;
		case LVN_ITEMCHANGING:      OnItemChanging(&evt);      return TRUE;
		case LVN_KEYDOWN:           OnKeyDown(&evt);           return TRUE;
		case LVN_MARQUEEBEGIN:      OnMarqueeBegin(&evt);      return TRUE;
		case LVN_SETDISPINFO:       OnSetDispInfo(&evt);       return TRUE;
	}

	return FALSE;
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

void CCtrlListView::AddColumn(int iSubItem, wchar_t *name, int cx)
{
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvc.iImage = 0;
	lvc.pszText = name;
	lvc.cx = cx;
	lvc.iSubItem = iSubItem;
	InsertColumn(iSubItem, &lvc);
}

void CCtrlListView::AddGroup(int iGroupId, wchar_t *name)
{
	LVGROUP lvg = { 0 };
	lvg.cbSize = sizeof(lvg);
	lvg.mask = LVGF_HEADER | LVGF_GROUPID;
	lvg.pszHeader = name;
	lvg.cchHeader = (int)mir_wstrlen(lvg.pszHeader);
	lvg.iGroupId = iGroupId;
	InsertGroup(-1, &lvg);
}

int CCtrlListView::AddItem(wchar_t *text, int iIcon, LPARAM lParam, int iGroupId)
{
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
	lvi.iSubItem = 0;
	lvi.pszText = text;
	lvi.iImage = iIcon;
	lvi.lParam = lParam;
	if (iGroupId >= 0) {
		lvi.mask |= LVIF_GROUPID;
		lvi.iGroupId = iGroupId;
	}

	return InsertItem(&lvi);
}

void CCtrlListView::SetItem(int iItem, int iSubItem, wchar_t *text, int iIcon)
{
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_TEXT;
	lvi.iItem = iItem;
	lvi.iSubItem = iSubItem;
	lvi.pszText = text;
	if (iIcon >= 0) {
		lvi.mask |= LVIF_IMAGE;
		lvi.iImage = iIcon;
	}

	SetItem(&lvi);
}

LPARAM CCtrlListView::GetItemData(int iItem)
{
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_PARAM;
	lvi.iItem = iItem;
	GetItem(&lvi);
	return lvi.lParam;
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
COLORREF CCtrlListView::GetBkColor()
{	return ListView_GetBkColor(m_hwnd);
}
void CCtrlListView::GetBkImage(LPLVBKIMAGE plvbki)
{	ListView_GetBkImage(m_hwnd, plvbki);
}
UINT CCtrlListView::GetCallbackMask()
{	return ListView_GetCallbackMask(m_hwnd);
}
BOOL CCtrlListView::GetCheckState(UINT iIndex)
{	return ListView_GetCheckState(m_hwnd, iIndex);
}
void CCtrlListView::GetColumn(int iCol, LPLVCOLUMN pcol)
{	ListView_GetColumn(m_hwnd, iCol, pcol);
}
void CCtrlListView::GetColumnOrderArray(int iCount, int *lpiArray)
{	ListView_GetColumnOrderArray(m_hwnd, iCount, lpiArray);
}
int CCtrlListView::GetColumnWidth(int iCol)
{	return ListView_GetColumnWidth(m_hwnd, iCol);
}
int CCtrlListView::GetCountPerPage()
{	return ListView_GetCountPerPage(m_hwnd);
}
HWND CCtrlListView::GetEditControl()
{	return ListView_GetEditControl(m_hwnd);
}
DWORD CCtrlListView::GetExtendedListViewStyle()
{	return ListView_GetExtendedListViewStyle(m_hwnd);
}
void CCtrlListView::GetGroupMetrics(LVGROUPMETRICS *pGroupMetrics)
{	ListView_GetGroupMetrics(m_hwnd, pGroupMetrics);
}
HWND CCtrlListView::GetHeader()
{	return ListView_GetHeader(m_hwnd);
}
HCURSOR CCtrlListView::GetHotCursor()
{	return ListView_GetHotCursor(m_hwnd);
}
INT CCtrlListView::GetHotItem()
{	return ListView_GetHotItem(m_hwnd);
}
DWORD CCtrlListView::GetHoverTime()
{	return ListView_GetHoverTime(m_hwnd);
}
HIMAGELIST CCtrlListView::GetImageList(int iImageList)
{	return ListView_GetImageList(m_hwnd, iImageList);
}
BOOL CCtrlListView::GetInsertMark(LVINSERTMARK *plvim)
{	return ListView_GetInsertMark(m_hwnd, plvim);
}
COLORREF CCtrlListView::GetInsertMarkColor()
{	return ListView_GetInsertMarkColor(m_hwnd);
}
int CCtrlListView::GetInsertMarkRect(LPRECT prc)
{	return ListView_GetInsertMarkRect(m_hwnd, prc);
}
BOOL CCtrlListView::GetISearchString(LPSTR lpsz)
{	return ListView_GetISearchString(m_hwnd, lpsz);
}
bool CCtrlListView::GetItem(LPLVITEM pitem)
{	return ListView_GetItem(m_hwnd, pitem) == TRUE;
}
int CCtrlListView::GetItemCount()
{	return ListView_GetItemCount(m_hwnd);
}
void CCtrlListView::GetItemPosition(int i, POINT *ppt)
{	ListView_GetItemPosition(m_hwnd, i, ppt);
}
void CCtrlListView::GetItemRect(int i, RECT *prc, int code)
{	ListView_GetItemRect(m_hwnd, i, prc, code);
}
DWORD CCtrlListView::GetItemSpacing(BOOL fSmall)
{	return ListView_GetItemSpacing(m_hwnd, fSmall);
}
UINT CCtrlListView::GetItemState(int i, UINT mask)
{	return ListView_GetItemState(m_hwnd, i, mask);
}
void CCtrlListView::GetItemText(int iItem, int iSubItem, LPTSTR pszText, int cchTextMax)
{	ListView_GetItemText(m_hwnd, iItem, iSubItem, pszText, cchTextMax);
}
int CCtrlListView::GetNextItem(int iStart, UINT flags)
{	return ListView_GetNextItem(m_hwnd, iStart, flags);
}
BOOL CCtrlListView::GetNumberOfWorkAreas(LPUINT lpuWorkAreas)
{	return  ListView_GetNumberOfWorkAreas(m_hwnd, lpuWorkAreas);
}
BOOL CCtrlListView::GetOrigin(LPPOINT lpptOrg)
{	return ListView_GetOrigin(m_hwnd, lpptOrg);
}
COLORREF CCtrlListView::GetOutlineColor()
{	return ListView_GetOutlineColor(m_hwnd);
}
UINT CCtrlListView::GetSelectedColumn()
{	return ListView_GetSelectedColumn(m_hwnd);
}
UINT CCtrlListView::GetSelectedCount()
{	return ListView_GetSelectedCount(m_hwnd);
}
INT CCtrlListView::GetSelectionMark()
{	return ListView_GetSelectionMark(m_hwnd);
}
int CCtrlListView::GetStringWidth(LPCSTR psz)
{	return ListView_GetStringWidth(m_hwnd, psz);
}
BOOL CCtrlListView::GetSubItemRect(int iItem, int iSubItem, int code, LPRECT lpRect)
{	return ListView_GetSubItemRect(m_hwnd, iItem, iSubItem, code, lpRect);
}
COLORREF CCtrlListView::GetTextBkColor()
{	return ListView_GetTextBkColor(m_hwnd);
}
COLORREF CCtrlListView::GetTextColor()
{	return ListView_GetTextColor(m_hwnd);
}
void CCtrlListView::GetTileInfo(PLVTILEINFO plvtinfo)
{	ListView_GetTileInfo(m_hwnd, plvtinfo);
}
void CCtrlListView::GetTileViewInfo(PLVTILEVIEWINFO plvtvinfo)
{	ListView_GetTileViewInfo(m_hwnd, plvtvinfo);
}
HWND CCtrlListView::GetToolTips()
{	return ListView_GetToolTips(m_hwnd);
}
int CCtrlListView::GetTopIndex()
{	return ListView_GetTopIndex(m_hwnd);
}
BOOL CCtrlListView::GetUnicodeFormat()
{	return ListView_GetUnicodeFormat(m_hwnd);
}
DWORD CCtrlListView::GetView()
{	return ListView_GetView(m_hwnd);
}
BOOL CCtrlListView::GetViewRect(RECT *prc)
{	return ListView_GetViewRect(m_hwnd, prc);
}
void CCtrlListView::GetWorkAreas(INT nWorkAreas, LPRECT lprc)
{	ListView_GetWorkAreas(m_hwnd, nWorkAreas, lprc);
}
BOOL CCtrlListView::HasGroup(int dwGroupId)
{	return ListView_HasGroup(m_hwnd, dwGroupId);
}
int CCtrlListView::HitTest(LPLVHITTESTINFO pinfo)
{	return ListView_HitTest(m_hwnd, pinfo);
}
int CCtrlListView::InsertColumn(int iCol, const LPLVCOLUMN pcol)
{	return ListView_InsertColumn(m_hwnd, iCol, pcol);
}
int CCtrlListView::InsertGroup(int index, PLVGROUP pgrp)
{	return ListView_InsertGroup(m_hwnd, index, pgrp);
}
void CCtrlListView::InsertGroupSorted(PLVINSERTGROUPSORTED structInsert)
{	ListView_InsertGroupSorted(m_hwnd, structInsert);
}
int CCtrlListView::InsertItem(const LPLVITEM pitem)
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
BOOL CCtrlListView::SetItem(const LPLVITEM pitem)
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
void CCtrlListView::SetItemText(int i, int iSubItem, wchar_t *pszText)
{	ListView_SetItemText(m_hwnd, i, iSubItem, pszText);
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
INT CCtrlListView::SubItemHitTest(LPLVHITTESTINFO pInfo)
{	return ListView_SubItemHitTest(m_hwnd, pInfo);
}
BOOL CCtrlListView::Update(int iItem)
{	return ListView_Update(m_hwnd, iItem);
}
