/*
	Options.cpp
	Copyright (c) 2005-2008 Chervov Dmitry

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

#include "stdafx.h"
#include "Options.h"

static CString sEmptyString("");

COptPage::COptPage(const COptPage &Item)
{
	*this = Item;
}

COptPage::~COptPage()
{
	for (int i = 0; i < Items.GetSize(); i++)
		delete Items[i];

	Items.RemoveAll();
}

void COptPage::MemToPage(int OnlyEnable)
{
	_ASSERT(hWnd);
	for (int i = 0; i < Items.GetSize(); i++) {
		if (OnlyEnable)
			Items[i]->COptItem::MemToWnd(hWnd);
		else
			Items[i]->MemToWnd(hWnd);
	}
}

void COptPage::PageToMem()
{
	_ASSERT(hWnd);
	for (int i = 0; i < Items.GetSize(); i++)
		Items[i]->WndToMem(hWnd);
}

void COptPage::DBToMem()
{
	_ASSERT(sModule != "");
	for (int i = 0; i < Items.GetSize(); i++)
		Items[i]->DBToMem(sModule, &sDBSettingPrefix);
}

void COptPage::MemToDB()
{
	_ASSERT(sModule != "");
	for (int i = 0; i < Items.GetSize(); i++)
		Items[i]->MemToDB(sModule, &sDBSettingPrefix);
}

void COptPage::CleanDBSettings()
{
	_ASSERT(sModule != "");
	for (int i = 0; i < Items.GetSize(); i++)
		Items[i]->CleanDBSettings(sModule, &sDBSettingPrefix);
}

bool COptPage::GetModified()
{
	_ASSERT(sModule != "");
	for (int i = 0; i < Items.GetSize(); i++)
		if (Items[i]->GetModified())
			return true;

	return false;
}

void COptPage::SetModified(bool m_bModified)
{
	_ASSERT(sModule != "");
	for (int i = 0; i < Items.GetSize(); i++)
		Items[i]->SetModified(m_bModified);
}

COptItem *COptPage::Find(int m_dlgItemID)
{
	for (int i = 0; i < Items.GetSize(); i++)
		if (Items[i]->GetID() == m_dlgItemID)
			return Items[i];

	_ASSERT(0);
	return nullptr;
}

COptPage& COptPage::operator=(const COptPage& Page)
{
	hWnd = Page.hWnd;
	sModule = Page.sModule;
	sDBSettingPrefix = Page.sDBSettingPrefix;
	Items.RemoveAll();
	for (int i = 0; i < Page.Items.GetSize(); i++)
		Items.AddElem(Page.Items[i]->Copy());

	return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////
// default procedure for reading value from DB; used only for integral types

int COptItem::GetIntDBVal(const CString &sModule, int bSigned, CString*)
{
	if (sDBSetting != nullptr) {
		_ASSERT(nValueSize == DBVT_BYTE || nValueSize == DBVT_WORD || nValueSize == DBVT_DWORD);
		DBVARIANT dbv;
		if (db_get(NULL, sModule, sDBSetting, &dbv))
			return (int)GetDefValue();

		return (nValueSize == DBVT_BYTE) ? (bSigned ? (signed char)dbv.bVal : (unsigned char)dbv.bVal) : ((nValueSize == DBVT_WORD) ? (bSigned ? (signed short)dbv.wVal : (unsigned short)dbv.wVal) : dbv.dVal);
	}
	return (int)GetDefValue();
}

/////////////////////////////////////////////////////////////////////////////////////////
// default procedure for writing value to the DB; used only for integral types

void COptItem::SetIntDBVal(const CString &sModule, int m_value, CString*)
{
	if (sDBSetting != nullptr && !m_bReadOnly) {
		_ASSERT(nValueSize == DBVT_BYTE || nValueSize == DBVT_WORD || nValueSize == DBVT_DWORD);

		DBVARIANT dbv;
		dbv.type = nValueSize;
		dbv.dVal = m_value;
		db_set(NULL, sModule, sDBSetting, &dbv);
	}
}

TCString COptItem::GetStrDBVal(const CString &sModule, CString *sDBSettingPrefix)
{
	if (sDBSetting != nullptr) {
		_ASSERT(GetDefValue());
		return db_get_s(0, sModule, sDBSettingPrefix ? (*sDBSettingPrefix + sDBSetting) : sDBSetting, *(TCString*)GetDefValue());
	}
	return *(TCString*)GetDefValue();
}

void COptItem::SetStrDBVal(const CString &sModule, TCString &Str, CString *sDBSettingPrefix)
{
	if (sDBSetting != nullptr && !m_bReadOnly) {
		db_set_ws(0, sModule, sDBSettingPrefix ? (*sDBSettingPrefix + sDBSetting) : sDBSetting, Str);
	}
}

void COptItem_Checkbox::DBToMem(const CString &sModule, CString *sDBSettingPrefix)
{
	if (m_valueMask)
		m_value = (GetIntDBVal(sModule, false, sDBSettingPrefix) & m_valueMask) ? BST_CHECKED : BST_UNCHECKED;
	else
		m_value = GetIntDBVal(sModule, false, sDBSettingPrefix);

	COptItem::DBToMem(sModule, sDBSettingPrefix);
}

void COptItem_Checkbox::MemToDB(const CString &sModule, CString *sDBSettingPrefix)
{
	if (m_valueMask) {
		if (m_value == BST_CHECKED)
			SetIntDBVal(sModule, GetIntDBVal(sModule, false, sDBSettingPrefix) | m_valueMask, sDBSettingPrefix);
		else
			SetIntDBVal(sModule, GetIntDBVal(sModule, false, sDBSettingPrefix) & ~m_valueMask, sDBSettingPrefix);
	}
	else SetIntDBVal(sModule, m_value, sDBSettingPrefix);

	COptItem::MemToDB(sModule, sDBSettingPrefix);
}

void COptItem_Checkbox::WndToMem(HWND hWnd)
{
	m_value = IsDlgButtonChecked(hWnd, m_dlgItemID);
	// tri-state checkboxes in combination with m_valueMask != 0 are not supported ;)
	_ASSERT(!m_valueMask || m_value != BST_INDETERMINATE);
	COptItem::WndToMem(hWnd);
}

void COptItem_Checkbox::MemToWnd(HWND hWnd)
{
	CheckDlgButton(hWnd, m_dlgItemID, m_value ? BST_CHECKED : BST_UNCHECKED);
	COptItem::MemToWnd(hWnd);
}

void COptItem_BitDBSetting::DBToMem(const CString &sModule, CString *sDBSettingPrefix)
{
	if (m_valueMask)
		m_value = (GetIntDBVal(sModule, false, sDBSettingPrefix) & m_valueMask) != 0;
	else
		m_value = GetIntDBVal(sModule, false, sDBSettingPrefix);

	COptItem::DBToMem(sModule, sDBSettingPrefix);
}

void COptItem_BitDBSetting::MemToDB(const CString &sModule, CString *sDBSettingPrefix)
{
	if (m_valueMask) {
		if (m_value)
			SetIntDBVal(sModule, GetIntDBVal(sModule, false, sDBSettingPrefix) | m_valueMask, sDBSettingPrefix);
		else
			SetIntDBVal(sModule, GetIntDBVal(sModule, false, sDBSettingPrefix) & ~m_valueMask, sDBSettingPrefix);
	}
	else SetIntDBVal(sModule, m_value, sDBSettingPrefix);

	COptItem::MemToDB(sModule, sDBSettingPrefix);
}

// ================================================ COptItem_TreeCtrl ================================================

int COptItem_TreeCtrl::IDToOrder(int ID)
{
	for (int i = 0; i < RootItems.GetSize(); i++)
		if (RootItems[i].ID == ID)
			return ROOT_INDEX_TO_ORDER(i);

	for (int i = 0; i < m_value.GetSize(); i++)
		if (m_value[i].ID == ID)
			return i;

	return -1;
}

int COptItem_TreeCtrl::hItemToOrder(HTREEITEM hItem)
{
	for (int i = 0; i < RootItems.GetSize(); i++)
		if (RootItems[i].hItem == hItem)
			return ROOT_INDEX_TO_ORDER(i);

	for (int i = 0; i < m_value.GetSize(); i++)
		if (m_value[i].hItem == hItem)
			return i;

	return -1;
}

int COptItem_TreeCtrl::GenerateID()
{
	int ID = 0;
	while (IDToOrder(ID) != -1)
		ID++;

	return ID;
}

struct sTreeReadEnumData
{
	sTreeReadEnumData(COptItem_TreeCtrl *_p1, const CString &_p2, const CString &_p3) :
		TreeCtrl(_p1),
		sModule(_p2),
		sDBSettingPrefix(_p3)
		{}

	COptItem_TreeCtrl *TreeCtrl;
	const CString &sModule, &sDBSettingPrefix;
};

int TreeReadEnum(const char *szSetting, void *lParam)
{
	sTreeReadEnumData *pData = (sTreeReadEnumData*)lParam;
	int Len = pData->TreeCtrl->sDBSetting.GetLen() + _countof(TREEITEM_DBSTR_TITLE) - 1;
	if (!strncmp(szSetting, pData->TreeCtrl->sDBSetting + TREEITEM_DBSTR_TITLE, Len) && isdigit(szSetting[Len])) {
		int ID = atol(szSetting + Len);
		short ParentID = (pData->TreeCtrl->TreeFlags & TREECTRL_FLAG_IS_SINGLE_LEVEL) ? 0 : db_get_w(0, pData->sModule,
			pData->sDBSettingPrefix + pData->TreeCtrl->sDBSetting + TREEITEM_DBSTR_PARENT + (szSetting + Len), -1);
		short Order = db_get_w(0, pData->sModule,
			pData->sDBSettingPrefix + pData->TreeCtrl->sDBSetting + TREEITEM_DBSTR_ORDER + (szSetting + Len), -1);
		char Flags = (pData->TreeCtrl->TreeFlags & TREECTRL_FLAG_IS_SINGLE_LEVEL && !(pData->TreeCtrl->TreeFlags & TREECTRL_FLAG_HAS_CHECKBOXES)) ? 0 : db_get_b(0, pData->sModule,
			pData->sDBSettingPrefix + pData->TreeCtrl->sDBSetting + TREEITEM_DBSTR_FLAGS + (szSetting + Len), 0);
		if (ParentID >= 0 && Order >= 0) {
			CString tmp(pData->sDBSettingPrefix + pData->TreeCtrl->sDBSetting + pData->TreeCtrl->User_Str1_DBName + CString(szSetting + Len));

			CTreeItem &pItem = pData->TreeCtrl->m_value.SetAtGrow(Order);
			pItem.ID = ID;
			pItem.ParentID = ParentID;
			pItem.Flags = Flags;
			pItem.hItem = nullptr;
			pItem.Title = db_get_s(0, pData->sModule, *pData->sDBSettingPrefix + szSetting, L"");
			pItem.User_Str1 = (pData->TreeCtrl->User_Str1_DBName == nullptr) ? nullptr : db_get_s(0, pData->sModule, tmp, (wchar_t*)nullptr);
		}
	}
	return 0;
}

void COptItem_TreeCtrl::DBToMem(const CString &sModule, CString *sDBSettingPrefix)
{
	if (!sDBSettingPrefix)
		sDBSettingPrefix = &sEmptyString;

	m_value.RemoveAll();
	sTreeReadEnumData pData(this, sModule, *sDBSettingPrefix);
	db_enum_settings(NULL, TreeReadEnum, sModule, &pData);

	if (!m_value.GetSize()) {
		m_value = m_defValue;
	}
	else {
		for (int i = 0; i < m_value.GetSize(); i++) {
			if (m_value[i].Title == nullptr) {
				m_value.RemoveElem(i);
				i--;
			}
		}
	}
	COptItem::DBToMem(sModule, sDBSettingPrefix);
}

void COptItem_TreeCtrl::MemToDB(const CString &sModule, CString *sDBSettingPrefix)
{
	if (!m_bReadOnly && m_bModified) {
		if (!sDBSettingPrefix)
			sDBSettingPrefix = &sEmptyString;

		CleanDBSettings(sModule, sDBSettingPrefix);
		for (int i = 0; i < m_value.GetSize(); i++) {
			CString StrID;
			_itoa(m_value[i].ID, StrID.GetBuffer(64), 10);
			StrID.ReleaseBuffer();
			db_set_ws(0, sModule, *sDBSettingPrefix + sDBSetting + TREEITEM_DBSTR_TITLE + StrID, m_value[i].Title);
			if (!(TreeFlags & TREECTRL_FLAG_IS_SINGLE_LEVEL))
				db_set_w(0, sModule, *sDBSettingPrefix + sDBSetting + TREEITEM_DBSTR_PARENT + StrID, m_value[i].ParentID);

			db_set_w(0, sModule, *sDBSettingPrefix + sDBSetting + TREEITEM_DBSTR_ORDER + StrID, i);
			if (!(TreeFlags & TREECTRL_FLAG_IS_SINGLE_LEVEL) || TreeFlags & TREECTRL_FLAG_HAS_CHECKBOXES)
				db_set_b(0, sModule, *sDBSettingPrefix + sDBSetting + TREEITEM_DBSTR_FLAGS + StrID, m_value[i].Flags);

			if (User_Str1_DBName != nullptr && m_value[i].User_Str1 != nullptr)
				db_set_ws(0, sModule, *sDBSettingPrefix + sDBSetting + User_Str1_DBName + StrID, m_value[i].User_Str1);
		}
		COptItem::MemToDB(sModule, sDBSettingPrefix);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// only need to gather info of items state (expanded/collapsed, checked/unchecked)

void COptItem_TreeCtrl::WndToMem(HWND hWnd)
{
	HWND hTreeView = GetDlgItem(hWnd, m_dlgItemID);
	for (int i = 0; i < m_value.GetSize(); i++) {
		uint32_t State = TreeView_GetItemState(hTreeView, m_value[i].hItem, TVIS_EXPANDED | TVIS_STATEIMAGEMASK);
		int OldFlags = m_value[i].Flags;
		if (State & TVIS_EXPANDED)
			m_value[i].Flags |= TIF_EXPANDED;
		else
			m_value[i].Flags &= ~TIF_EXPANDED;

		if (TreeFlags & TREECTRL_FLAG_HAS_CHECKBOXES && (State >> 12) - 1)
			m_value[i].Flags |= TIF_ENABLED;
		else
			m_value[i].Flags &= ~TIF_ENABLED;

		if (m_value[i].Flags != OldFlags)
			m_bModified = true;
	}
	COptItem::WndToMem(hWnd);
}

void COptItem_TreeCtrl::MemToWnd(HWND hWnd)
{
	HWND hTreeView = GetDlgItem(hWnd, m_dlgItemID);
	if (TreeFlags & TREECTRL_FLAG_HAS_CHECKBOXES) { // have to set this in run-time as it's specified in MSDN
		LONG_PTR Style = GetWindowLongPtr(hTreeView, GWL_STYLE);
		SetWindowLongPtr(hTreeView, GWL_STYLE, Style & ~TVS_CHECKBOXES);
		SetWindowLongPtr(hTreeView, GWL_STYLE, Style | TVS_CHECKBOXES);
	}

	TVINSERTSTRUCT tvIn = {};
	int ScrollPos = GetScrollPos(hTreeView, SB_VERT);
	int SelectOrder = IDToOrder(GetSelectedItemID(hWnd));
	SendMessage(hTreeView, WM_SETREDRAW, false, 0);
	TreeView_DeleteAllItems(hTreeView);
	_ASSERT(RootItems.GetSize());

	if (!(TreeFlags & TREECTRL_FLAG_IS_SINGLE_LEVEL)) {
		for (int i = 0; i < RootItems.GetSize(); i++) {
			tvIn.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
			RootItems[i].Flags |= TIF_GROUP;
			tvIn.item.state = tvIn.item.stateMask = TVIS_BOLD | ((RootItems[i].Flags & TIF_EXPANDED) ? TVIS_EXPANDED : 0);
			tvIn.item.pszText = RootItems[i].Title;
			tvIn.hParent = TVI_ROOT;
			tvIn.hInsertAfter = TVI_LAST;
			tvIn.item.lParam = RootItems[i].ID;
			RootItems[i].hItem = TreeView_InsertItem(hTreeView, &tvIn);
		}
	}
	for (int i = 0; i < m_value.GetSize(); i++)
		m_value[i].hItem = RootItems[0].hItem; // put an item to first group in case of some strange error

	for (int i = 0; i < m_value.GetSize(); i++) {
		tvIn.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
		tvIn.item.state = tvIn.item.stateMask = (m_value[i].Flags & TIF_GROUP) ? (TVIS_BOLD | ((m_value[i].Flags & TIF_EXPANDED) ? TVIS_EXPANDED : 0)) : 0;
		if (TreeFlags & TREECTRL_FLAG_HAS_CHECKBOXES) {
			tvIn.item.stateMask |= TVIS_STATEIMAGEMASK;
			tvIn.item.state |= INDEXTOSTATEIMAGEMASK((m_value[i].Flags & TIF_ENABLED) ? 2 : 1);
		}
		tvIn.item.pszText = m_value[i].Title;
		int Order = IDToOrder(m_value[i].ParentID);
		if (Order != -1) {
			tvIn.hParent = (Order <= TREECTRL_ROOTORDEROFFS) ? RootItems[ROOT_ORDER_TO_INDEX(Order)].hItem : m_value[Order].hItem;
			tvIn.hInsertAfter = TVI_LAST;
			tvIn.item.lParam = m_value[i].ID;
			m_value[i].hItem = TreeView_InsertItem(hTreeView, &tvIn);
		}
		else { // found an orphan item; probably it's better just to delete it
			m_value.RemoveElem(i);
			i--;
		}
	}
	TreeView_SelectItem(hTreeView, (SelectOrder >= 0) ? m_value[SelectOrder].hItem : ((SelectOrder <= TREECTRL_ROOTORDEROFFS) ? RootItems[ROOT_ORDER_TO_INDEX(SelectOrder)].hItem : nullptr));
	SendMessage(hTreeView, WM_SETREDRAW, true, 0);
	SCROLLBARINFO sbi;
	sbi.cbSize = sizeof(sbi);
	GetScrollBarInfo(hTreeView, OBJID_VSCROLL, &sbi);
	if (!(sbi.rgstate[0] & STATE_SYSTEM_INVISIBLE)) {
		int MinPos, MaxPos;
		GetScrollRange(hTreeView, SB_VERT, &MinPos, &MaxPos);
		if (ScrollPos < MinPos) {
			ScrollPos = MinPos;
		}
		else if (ScrollPos > MaxPos) {
			ScrollPos = MaxPos;
		}
		SetScrollPos(hTreeView, SB_VERT, ScrollPos, true);
	}
	COptItem::MemToWnd(hWnd);
}

struct sTreeDeleteEnumData
{
	TMyArray<CString> TreeSettings;
	COptItem_TreeCtrl *TreeCtrl;
	CString *sDBSettingPrefix;
};

int TreeDeleteEnum(const char *szSetting, void *lParam)
{
	sTreeDeleteEnumData *TreeDeleteEnumData = (sTreeDeleteEnumData*)lParam;
	CString CurSetting = *TreeDeleteEnumData->sDBSettingPrefix + TreeDeleteEnumData->TreeCtrl->sDBSetting + TREEITEM_DBSTR_TITLE;
	if (!strncmp(szSetting, CurSetting, CurSetting.GetLen()))
		TreeDeleteEnumData->TreeSettings.AddElem(szSetting);

	CurSetting = *TreeDeleteEnumData->sDBSettingPrefix + TreeDeleteEnumData->TreeCtrl->sDBSetting + TREEITEM_DBSTR_PARENT;
	if (!strncmp(szSetting, CurSetting, CurSetting.GetLen()))
		TreeDeleteEnumData->TreeSettings.AddElem(szSetting);

	CurSetting = *TreeDeleteEnumData->sDBSettingPrefix + TreeDeleteEnumData->TreeCtrl->sDBSetting + TREEITEM_DBSTR_ORDER;
	if (!strncmp(szSetting, CurSetting, CurSetting.GetLen()))
		TreeDeleteEnumData->TreeSettings.AddElem(szSetting);

	CurSetting = *TreeDeleteEnumData->sDBSettingPrefix + TreeDeleteEnumData->TreeCtrl->sDBSetting + TREEITEM_DBSTR_FLAGS;
	if (!strncmp(szSetting, CurSetting, CurSetting.GetLen()))
		TreeDeleteEnumData->TreeSettings.AddElem(szSetting);

	if (TreeDeleteEnumData->TreeCtrl->User_Str1_DBName != nullptr) {
		CurSetting = *TreeDeleteEnumData->sDBSettingPrefix + TreeDeleteEnumData->TreeCtrl->sDBSetting + TreeDeleteEnumData->TreeCtrl->User_Str1_DBName;
		if (!strncmp(szSetting, CurSetting, CurSetting.GetLen()))
			TreeDeleteEnumData->TreeSettings.AddElem(szSetting);
	}
	return 0;
}

void COptItem_TreeCtrl::CleanDBSettings(const CString &sModule, CString *sDBSettingPrefix)
{
	if (!sDBSettingPrefix)
		sDBSettingPrefix = &sEmptyString;

	sTreeDeleteEnumData TreeDeleteEnumData;
	TreeDeleteEnumData.TreeCtrl = this;
	TreeDeleteEnumData.sDBSettingPrefix = sDBSettingPrefix;
	db_enum_settings(NULL, TreeDeleteEnum, sModule, &TreeDeleteEnumData);

	for (int i = 0; i < TreeDeleteEnumData.TreeSettings.GetSize(); i++)
		db_unset(0, sModule, TreeDeleteEnumData.TreeSettings[i]);
}

int COptItem_TreeCtrl::GetSelectedItemID(HWND hWnd)
{
	HWND hTreeView = GetDlgItem(hWnd, m_dlgItemID);
	TVITEM tvi = { 0 };
	tvi.hItem = TreeView_GetSelection(hTreeView);
	if (!tvi.hItem)
		return -1;

	tvi.mask = TVIF_HANDLE | TVIF_PARAM;
	TreeView_GetItem(hTreeView, &tvi);
	return tvi.lParam;
}

void COptItem_TreeCtrl::Delete(HWND hWnd, int ID)
{
	int SelectedOrder = IDToOrder(ID);
	_ASSERT(SelectedOrder >= 0);
	RecursiveDelete(hWnd, SelectedOrder);
	m_bModified = true;
}

void COptItem_TreeCtrl::RecursiveDelete(HWND hWnd, int i)
{
	if (m_value[i].Flags & TIF_GROUP)
		for (int j = i + 1; j < m_value.GetSize(); j++)
			if (m_value[j].ParentID == m_value[i].ID)
				RecursiveDelete(hWnd, j--);

	HWND hTreeView = GetDlgItem(hWnd, m_dlgItemID);
	TreeView_DeleteItem(hTreeView, m_value[i].hItem);
	m_value.RemoveElem(i);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Item's ID and ParentID are not used (the new item position is determined by current selection in the tree)
// returns a pointer to the newly inserted item info

CTreeItem* COptItem_TreeCtrl::InsertItem(HWND hWnd, CTreeItem &Item)
{
	_ASSERT(!(TreeFlags & TREECTRL_FLAG_IS_SINGLE_LEVEL) || !(Item.Flags & TIF_GROUP));
	HWND hTreeView = GetDlgItem(hWnd, m_dlgItemID);
	TVITEM tvi;
	int SelOrder = -1;
	Item.ParentID = RootItems[0].ID;
	TVINSERTSTRUCT tvIn = {};
	tvIn.hParent = RootItems[0].hItem;
	tvIn.hInsertAfter = TVI_FIRST;
	if (tvi.hItem = TreeView_GetSelection(hTreeView)) {
		tvi.mask = TVIF_HANDLE | TVIF_PARAM;
		TreeView_GetItem(hTreeView, &tvi);
		SelOrder = IDToOrder(tvi.lParam);
		if (SelOrder <= TREECTRL_ROOTORDEROFFS) {
			Item.ParentID = RootItems[ROOT_ORDER_TO_INDEX(SelOrder)].ID;
			tvIn.hParent = RootItems[ROOT_ORDER_TO_INDEX(SelOrder)].hItem;
			SelOrder = -1;
		}
		else {
			if (m_value[SelOrder].Flags & TIF_GROUP) {
				Item.ParentID = m_value[SelOrder].ID;
				tvIn.hParent = m_value[SelOrder].hItem;
			}
			else {
				Item.ParentID = m_value[SelOrder].ParentID;
				int Order = IDToOrder(m_value[SelOrder].ParentID);
				tvIn.hParent = (Order <= TREECTRL_ROOTORDEROFFS) ? RootItems[ROOT_ORDER_TO_INDEX(Order)].hItem : m_value[Order].hItem;
				tvIn.hInsertAfter = m_value[SelOrder].hItem;
			}
		}
	}
	tvIn.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
	tvIn.item.state = tvIn.item.stateMask = (Item.Flags & TIF_GROUP) ? (TVIS_BOLD | ((Item.Flags & TIF_EXPANDED) ? TVIS_EXPANDED : 0)) : 0;
	if (TreeFlags & TREECTRL_FLAG_HAS_CHECKBOXES) {
		tvIn.item.stateMask |= TVIS_STATEIMAGEMASK;
		tvIn.item.state |= INDEXTOSTATEIMAGEMASK((Item.Flags & TIF_ENABLED) ? 2 : 1);
	}
	tvIn.item.pszText = Item.Title;
	tvIn.item.lParam = Item.ID = GenerateID();
	m_value.InsertElem(Item, SelOrder + 1);
	m_value[SelOrder + 1].hItem = TreeView_InsertItem(hTreeView, &tvIn);
	TreeView_SelectItem(hTreeView, m_value[SelOrder + 1].hItem);
	m_bModified = true;
	return &m_value[SelOrder + 1];
}

/////////////////////////////////////////////////////////////////////////////////////////
// ItemOrder must be a movable item (i.e. ItemOrder >= 0)
// InsertAtOrder must be >= 0 too.

int COptItem_TreeCtrl::RecursiveMove(int ItemOrder, int ParentID, int InsertAtOrder)
{
	int ItemsMoved = 1;
	m_value.MoveElem(ItemOrder, InsertAtOrder);
	m_value[InsertAtOrder].ParentID = ParentID;
	if (m_value[InsertAtOrder].Flags & TIF_GROUP) { // need to ensure that no items were left before their group by an order.
		int GroupID = m_value[InsertAtOrder].ID;
		for (int i = ItemOrder; i < InsertAtOrder; i++) { // if ItemOrder > InsertAtOrder then there is simply nothing to do
			if (m_value[i].ParentID == GroupID) {
				int CurrentItemsMoved = RecursiveMove(i, GroupID, InsertAtOrder);
				ItemsMoved += CurrentItemsMoved;
				InsertAtOrder -= CurrentItemsMoved;
				i--;
			}
		}
	}
	return ItemsMoved;
}

/////////////////////////////////////////////////////////////////////////////////////////
// hMoveTo can be NULL and it means that we must move hItem to the beginning of the list

void COptItem_TreeCtrl::MoveItem(HWND hWnd, HTREEITEM hItem, HTREEITEM hMoveTo)
{
	_ASSERT(hItem && (hMoveTo || TreeFlags & TREECTRL_FLAG_IS_SINGLE_LEVEL));
	if (hItem == hMoveTo)
		return;

	HWND hTreeView = GetDlgItem(hWnd, m_dlgItemID);
	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_PARAM;
	tvi.hItem = hItem;
	TreeView_GetItem(hTreeView, &tvi);
	int ItemOrder = IDToOrder(tvi.lParam);
	_ASSERT(ItemOrder != -1);
	int MoveToOrder;
	if (hMoveTo) {
		tvi.hItem = hMoveTo;
		TreeView_GetItem(hTreeView, &tvi);
		MoveToOrder = IDToOrder(tvi.lParam);
		_ASSERT(MoveToOrder != -1);
	}
	else MoveToOrder = -1;

	// can't move root items
	if (ItemOrder <= TREECTRL_ROOTORDEROFFS)
		return;
	
	// need to check for a case when trying to move a group to its own subgroup.
	if (m_value[ItemOrder].Flags & TIF_GROUP) {
		int Order = MoveToOrder;
		while (Order >= 0) {
			Order = IDToOrder(m_value[Order].ParentID);
			if (Order == ItemOrder)
				return;
		}
	}
	// well, everything is ok, we really can move that item.
	WndToMem(hWnd); // save groups state (expanded/collapsed)
	if (MoveToOrder != -1 && ((MoveToOrder <= TREECTRL_ROOTORDEROFFS) ? RootItems[ROOT_ORDER_TO_INDEX(MoveToOrder)].Flags : m_value[MoveToOrder].Flags) & TIF_GROUP) // if the destination is a group, then move the item to that group
		RecursiveMove(ItemOrder, (MoveToOrder <= TREECTRL_ROOTORDEROFFS) ? RootItems[ROOT_ORDER_TO_INDEX(MoveToOrder)].ID : m_value[MoveToOrder].ID, (MoveToOrder >= 0) ? ((ItemOrder < MoveToOrder) ? MoveToOrder : (MoveToOrder + 1)) : 0);
	else // else place the item after the destination item
		RecursiveMove(ItemOrder, (MoveToOrder == -1) ? 0 : m_value[MoveToOrder].ParentID, (ItemOrder < MoveToOrder) ? MoveToOrder : (MoveToOrder + 1)); // when TREECTRL_FLAG_IS_SINGLE_LEVEL, we always have a root item with ID = 0.

	MemToWnd(hWnd); // update the tree
	m_bModified = true;
}


// ================================================ COptItem_ListCtrl ================================================

struct sListReadEnumData
{
	sListReadEnumData(COptItem_ListCtrl *p1, const CString &p2, const CString &p3) :
		ListCtrl(p1),
		sModule(p2),
		sDBSettingPrefix(p3)
	{}

	COptItem_ListCtrl *ListCtrl;
	const CString &sModule, &sDBSettingPrefix;
};

int ListReadEnum(const char *szSetting, void *lParam)
{
	sListReadEnumData *pData = (sListReadEnumData*)lParam;
	int Len = pData->sDBSettingPrefix.GetLen() + pData->ListCtrl->sDBSetting.GetLen() + _countof(LISTITEM_DBSTR_TEXT) - 1;
	if (!strncmp(szSetting, pData->sDBSettingPrefix + pData->ListCtrl->sDBSetting + LISTITEM_DBSTR_TEXT, Len) && isdigit(szSetting[Len])) {
		int ID = atol(szSetting + Len);
		pData->ListCtrl->m_value.SetAtGrow(ID).Text = db_get_s(0, pData->sModule, *pData->sDBSettingPrefix + szSetting, L"");
	}
	return 0;
}

void COptItem_ListCtrl::DBToMem(const CString &sModule, CString *sDBSettingPrefix)
{
	if (!sDBSettingPrefix)
		sDBSettingPrefix = &sEmptyString;

	m_value.RemoveAll();
	sListReadEnumData pData(this, sModule, *sDBSettingPrefix);
	db_enum_settings(NULL, ListReadEnum, sModule, &pData);

	if (!m_value.GetSize())
		m_value = m_defValue;
	else {
		for (int i = 0; i < m_value.GetSize(); i++) {
			if (m_value[i].Text == nullptr) {
				m_value.RemoveElem(i);
				i--;
			}
		}
	}
	COptItem::DBToMem(sModule, sDBSettingPrefix);
}

void COptItem_ListCtrl::MemToDB(const CString &sModule, CString *sDBSettingPrefix)
{
	if (!m_bReadOnly && m_bModified) {
		if (!sDBSettingPrefix)
			sDBSettingPrefix = &sEmptyString;

		CleanDBSettings(sModule, sDBSettingPrefix);

		for (int i = 0; i < m_value.GetSize(); i++) {
			CString StrID;
			_itoa(i, StrID.GetBuffer(64), 10);
			StrID.ReleaseBuffer();
			db_set_ws(0, sModule, *sDBSettingPrefix + sDBSetting + LISTITEM_DBSTR_TEXT + StrID, m_value[i].Text);
		}
		COptItem::MemToDB(sModule, sDBSettingPrefix);
	}
}

void COptItem_ListCtrl::WndToMem(HWND hWnd)
{
	// nothing to do
	COptItem::WndToMem(hWnd);
}

void COptItem_ListCtrl::MemToWnd(HWND hWnd)
{
	HWND hListView = GetDlgItem(hWnd, m_dlgItemID);
	SendMessage(hListView, WM_SETREDRAW, false, 0);
	SendMessage(hListView, LB_RESETCONTENT, 0, 0);

	for (int i = 0; i < m_value.GetSize(); i++)
		SendMessage(hListView, LB_INSERTSTRING, -1, (LPARAM)(wchar_t*)m_value[i].Text);

	SendMessage(hListView, WM_SETREDRAW, true, 0);
	COptItem::MemToWnd(hWnd);
}

typedef struct
{
	TMyArray<CString> ListSettings;
	COptItem_ListCtrl *ListCtrl;
	CString *sDBSettingPrefix;
} sListDeleteEnumData;

int ListDeleteEnum(const char *szSetting, void *lParam)
{
	sListDeleteEnumData *ListDeleteEnumData = (sListDeleteEnumData*)lParam;
	CString CurSetting = *ListDeleteEnumData->sDBSettingPrefix + ListDeleteEnumData->ListCtrl->sDBSetting + LISTITEM_DBSTR_TEXT;
	if (!strncmp(szSetting, CurSetting, CurSetting.GetLen()))
		ListDeleteEnumData->ListSettings.AddElem(szSetting);
	return 0;
}

void COptItem_ListCtrl::CleanDBSettings(const CString &sModule, CString *sDBSettingPrefix)
{
	if (!sDBSettingPrefix)
		sDBSettingPrefix = &sEmptyString;

	sListDeleteEnumData ListDeleteEnumData;
	ListDeleteEnumData.ListCtrl = this;
	ListDeleteEnumData.sDBSettingPrefix = sDBSettingPrefix;
	db_enum_settings(NULL, ListDeleteEnum, sModule, &ListDeleteEnumData);

	for (int i = 0; i < ListDeleteEnumData.ListSettings.GetSize(); i++)
		db_unset(0, sModule, ListDeleteEnumData.ListSettings[i]);
}

int COptItem_ListCtrl::GetSelectedItemID(HWND hWnd)
{
	int Res = SendDlgItemMessage(hWnd, m_dlgItemID, LB_GETCURSEL, 0, 0);
	return (Res == LB_ERR) ? -1 : Res; // i know that LB_ERR = -1 ;)
}

int COptItem_ListCtrl::SetSelectedItemID(HWND hWnd, int ID)
{
	int Res = SendDlgItemMessage(hWnd, m_dlgItemID, LB_SETCURSEL, ID, 0);
	return (Res == LB_ERR) ? -1 : Res;
}

void COptItem_ListCtrl::Delete(HWND hWnd, int ID)
{
	_ASSERT(ID >= 0);
	HWND hListView = GetDlgItem(hWnd, m_dlgItemID);
	int Res = SendMessage(hListView, LB_DELETESTRING, ID, 0);
	_ASSERT(Res != LB_ERR);
	m_value.RemoveElem(ID);
	m_bModified = true;
}

void COptItem_ListCtrl::ModifyItem(HWND hWnd, int ID, CListItem &Item)
{ // changes the text of item with the specified ID
	_ASSERT(ID >= 0);
	HWND hListView = GetDlgItem(hWnd, m_dlgItemID);
	SendMessage(hListView, WM_SETREDRAW, false, 0);
	int CurSel = SendMessage(hListView, LB_GETCURSEL, 0, 0);
	int TopIndex = SendMessage(hListView, LB_GETTOPINDEX, 0, 0);
	int Res = SendMessage(hListView, LB_DELETESTRING, ID, 0);
	_ASSERT(Res != LB_ERR);
	Res = SendMessage(hListView, LB_INSERTSTRING, ID, (LPARAM)(wchar_t*)(Item.Text));
	_ASSERT(Res != LB_ERR && Res != LB_ERRSPACE);
	SendMessage(hListView, LB_SETCURSEL, CurSel, 0);
	SendMessage(hListView, LB_SETTOPINDEX, TopIndex, 0);
	SendMessage(hListView, WM_SETREDRAW, true, 0);
	m_value[ID].Text = Item.Text;
	m_bModified = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns a pointer to the newly inserted item info
// ID is position at which to insert the item; -1 = add to the end of the list

CListItem* COptItem_ListCtrl::InsertItem(HWND hWnd, int ID, CListItem &Item)
{
	HWND hListView = GetDlgItem(hWnd, m_dlgItemID);
	int Res = SendMessage(hListView, LB_INSERTSTRING, ID, (LPARAM)(wchar_t*)(Item.Text)); // LB_INSERTSTRING doesn't sort the lists even with LBS_SORT style
	_ASSERT(Res != LB_ERR && Res != LB_ERRSPACE);
	int i = m_value.AddElem(Item);
	m_bModified = true;
	return &m_value[i];
}
