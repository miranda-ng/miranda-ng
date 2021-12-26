/*
	Options.h
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

#pragma once

#include "CString.h"
#include "TMyArray.h"

class COptItem
{
public:
	COptItem() {}
	COptItem(int m_dlgItemID, char *szDBSetting, int nValueSize, int lParam = 0, bool m_bReadOnly = false) :
		m_dlgItemID(m_dlgItemID), nValueSize(nValueSize), sDBSetting(szDBSetting), lParam(lParam), m_bEnabled(true), m_bReadOnly(m_bReadOnly), m_bModified(false)
	{
	}

	virtual ~COptItem() {}

	virtual void DBToMem(const CString&, CString* = nullptr) { m_bModified = false; }
	virtual void MemToDB(const CString&, CString* = nullptr) { m_bModified = false; }
	virtual void WndToMem(HWND) {}
	virtual void MemToWnd(HWND hWnd) { EnableWindow(GetDlgItem(hWnd, m_dlgItemID), m_bEnabled); }
	void DBToMemToWnd(const CString &sModule, HWND hWnd, CString *sDBSettingPrefix = nullptr) { DBToMem(sModule, sDBSettingPrefix); MemToWnd(hWnd); }
	void WndToMemToDB(HWND hWnd, const CString &sModule, CString *sDBSettingPrefix = nullptr) { WndToMem(hWnd); MemToDB(sModule, sDBSettingPrefix); }
	virtual void CleanDBSettings(const CString &sModule, CString *sDBSettingPrefix = nullptr) { db_unset(0, sModule, sDBSettingPrefix ? (*sDBSettingPrefix + sDBSetting) : sDBSetting); }; // TODO: also set m_value to m_defValue?

	virtual void SetValue(INT_PTR) { m_bModified = true; }
	virtual void SetDefValue(INT_PTR) {}
	virtual INT_PTR GetValue() { return 0; }
	virtual INT_PTR GetDefValue() { return 0; }
	
	INT_PTR GetDBValue(const CString &sModule, CString *sDBSettingPrefix = nullptr) { DBToMem(sModule, sDBSettingPrefix); return GetValue(); }
	void    SetDBValue(const CString &sModule, INT_PTR m_value, CString *sDBSettingPrefix = nullptr) { SetValue(m_value); MemToDB(sModule, sDBSettingPrefix); }
	INT_PTR GetDBValueCopy(const CString &sModule, CString *sDBSettingPrefix = nullptr) { COptItem* Item = Copy(); Item->DBToMem(sModule, sDBSettingPrefix); INT_PTR m_value = Item->GetValue(); delete Item; return m_value; } // retrieves DB value, but doesn't affect current page/item state; beware! it doesn't work with string values / other dynamic pointers
	void    SetDBValueCopy(const CString &sModule, INT_PTR m_value, CString *sDBSettingPrefix = nullptr) { COptItem* Item = Copy(); Item->SetValue(m_value); Item->MemToDB(sModule, sDBSettingPrefix); delete Item; }
	INT_PTR GetWndValue(HWND hWnd) { WndToMem(hWnd); return GetValue(); }
	void    SetWndValue(HWND hWnd, INT_PTR m_value) { SetValue(m_value); MemToWnd(hWnd); }
	
	void SetDlgItemID(int _DlgItemID) { m_dlgItemID = _DlgItemID; }
	bool GetModified() { return m_bModified; }
	void SetModified(bool _Modified) { m_bModified = _Modified; }

	void Enable(bool bEnabled) { m_bEnabled = bEnabled; }
	int GetParam() { return lParam; }
	int GetID() { return m_dlgItemID; }

	virtual COptItem* Copy() { _ASSERT(0); return nullptr; } // Attention! Free Copy() result when it's not needed anymore!

	CString sDBSetting;

protected:
	int GetIntDBVal(const CString &sModule, int bSigned = false, CString *sDBSettingPrefix = nullptr);
	void SetIntDBVal(const CString &sModule, int m_value, CString *sDBSettingPrefix = nullptr);
	TCString GetStrDBVal(const CString &sModule, CString *sDBSettingPrefix = nullptr);
	void SetStrDBVal(const CString &sModule, TCString &Str, CString *sDBSettingPrefix = nullptr);

	int m_dlgItemID;
	bool m_bEnabled;
	bool m_bReadOnly;
	bool m_bModified;
	int nValueSize; // maximum pValue size in bytes
	int lParam;
};

class COptItem_Generic : public COptItem
{
public:
	COptItem_Generic() {}
	COptItem_Generic(int m_dlgItemID, int lParam = 0) : COptItem(m_dlgItemID, nullptr, 0, lParam) {}
	virtual COptItem* Copy() { return new COptItem_Generic(*this); }
};

class COptItem_Edit : public COptItem
{
public:
	COptItem_Edit() {}
	COptItem_Edit(int m_dlgItemID, char *szDBSetting, int nMaxLen, wchar_t *szDefValue, int lParam = 0, bool m_bReadOnly = false)
		: COptItem(m_dlgItemID, szDBSetting, nMaxLen, lParam, m_bReadOnly), sDefValue(szDefValue)
		{}

	void DBToMem(const CString &sModule, CString *sDBSettingPrefix = nullptr) { sValue = GetStrDBVal(sModule, sDBSettingPrefix); COptItem::DBToMem(sModule, sDBSettingPrefix); }
	void MemToDB(const CString &sModule, CString *sDBSettingPrefix = nullptr) { SetStrDBVal(sModule, sValue, sDBSettingPrefix); COptItem::MemToDB(sModule, sDBSettingPrefix); }
	void WndToMem(HWND hWnd) { GetDlgItemText(hWnd, m_dlgItemID, sValue.GetBuffer(nValueSize), nValueSize); sValue.ReleaseBuffer(); COptItem::MemToWnd(hWnd); }
	void MemToWnd(HWND hWnd) { SetDlgItemText(hWnd, m_dlgItemID, sValue); COptItem::MemToWnd(hWnd); }
	
	virtual void SetValue(INT_PTR m_value) { sValue = *(TCString*)m_value; COptItem::SetValue(m_value); }
	virtual void SetDefValue(INT_PTR m_defValue) { sDefValue = *(TCString*)m_defValue; COptItem::SetDefValue(m_defValue); }
	
	virtual INT_PTR GetValue() { return (INT_PTR)&sValue; }
	virtual INT_PTR GetDefValue() { return (INT_PTR)&sDefValue; }

	virtual COptItem* Copy() { return new COptItem_Edit(*this); }

	TCString sDefValue;
	TCString sValue;
};


class COptItem_IntEdit : public COptItem
{
public:
	COptItem_IntEdit() {}
	COptItem_IntEdit(int m_dlgItemID, char *szDBSetting, int nValueSize = DBVT_BYTE, int bSigned = true, int m_defValue = 0, int lParam = 0, bool m_bReadOnly = false)
		: COptItem(m_dlgItemID, szDBSetting, nValueSize, lParam, m_bReadOnly), m_defValue(m_defValue), m_value(0), bSigned(bSigned)
		{}
	
	void DBToMem(const CString &sModule, CString *sDBSettingPrefix = nullptr) { m_value = GetIntDBVal(sModule, bSigned, sDBSettingPrefix); COptItem::DBToMem(sModule, sDBSettingPrefix); }
	void MemToDB(const CString &sModule, CString *sDBSettingPrefix = nullptr) { SetIntDBVal(sModule, m_value, sDBSettingPrefix); COptItem::MemToDB(sModule, sDBSettingPrefix); }
	void WndToMem(HWND hWnd) { m_value = GetDlgItemInt(hWnd, m_dlgItemID, nullptr, bSigned); COptItem::WndToMem(hWnd); }
	void MemToWnd(HWND hWnd) { SetDlgItemInt(hWnd, m_dlgItemID, m_value, bSigned); COptItem::MemToWnd(hWnd); }
	
	virtual void SetValue(INT_PTR _Value) { this->m_value = _Value; COptItem::SetValue(_Value); }
	virtual void SetDefValue(INT_PTR _DefValue) { this->m_defValue = _DefValue; COptItem::SetDefValue(_DefValue); }
	
	virtual INT_PTR GetValue() { return m_value; }
	virtual INT_PTR GetDefValue() { return m_defValue; }
	virtual COptItem* Copy() { return new COptItem_IntEdit(*this); }

	int m_defValue;
	int m_value;
	int bSigned;
};


class COptItem_Checkbox : public COptItem
{
public:
	COptItem_Checkbox() {}
	COptItem_Checkbox(int m_dlgItemID, char *szDBSetting, int nValueSize = DBVT_BYTE, int m_defValue = 0, int m_valueMask = 0, int lParam = 0, bool m_bReadOnly = false)
		: COptItem(m_dlgItemID, szDBSetting, nValueSize, lParam, m_bReadOnly), m_defValue(m_defValue), m_value(0), m_valueMask(m_valueMask)
		{}

	void DBToMem(const CString &sModule, CString *sDBSettingPrefix = nullptr);
	void MemToDB(const CString &sModule, CString *sDBSettingPrefix = nullptr);
	void WndToMem(HWND hWnd);
	void MemToWnd(HWND hWnd);

	virtual void SetValue(INT_PTR _Value) { this->m_value = _Value; COptItem::SetValue(_Value); }
	virtual void SetDefValue(INT_PTR _DefValue) { this->m_defValue = _DefValue; COptItem::SetDefValue(_DefValue); }
	
	virtual INT_PTR GetValue() { return m_value; }
	virtual INT_PTR GetDefValue() { return m_defValue; }
	
	virtual COptItem* Copy() { return new COptItem_Checkbox(*this); }

	int m_value;
	int m_defValue;
	int m_valueMask;
};

class COptItem_Radiobutton : public COptItem
{
public:
	COptItem_Radiobutton() {}
	COptItem_Radiobutton(int m_dlgItemID, char *szDBSetting, int nValueSize, int m_defValue, int m_valueMask, int lParam = 0, bool m_bReadOnly = false)
		: COptItem(m_dlgItemID, szDBSetting, nValueSize, lParam, m_bReadOnly), m_defValue(m_defValue), m_value(0), m_valueMask(m_valueMask)
		{}

	void DBToMem(const CString &sModule, CString *sDBSettingPrefix = nullptr) { m_value = (GetIntDBVal(sModule, false, sDBSettingPrefix) == m_valueMask) ? BST_CHECKED : BST_UNCHECKED; COptItem::DBToMem(sModule, sDBSettingPrefix); }
	void MemToDB(const CString &sModule, CString *sDBSettingPrefix = nullptr) { if ((m_value == BST_CHECKED)) SetIntDBVal(sModule, m_valueMask, sDBSettingPrefix); COptItem::MemToDB(sModule, sDBSettingPrefix); }
	void WndToMem(HWND hWnd) { m_value = IsDlgButtonChecked(hWnd, m_dlgItemID); COptItem::WndToMem(hWnd); }
	void MemToWnd(HWND hWnd) { CheckDlgButton(hWnd, m_dlgItemID, m_value ? BST_CHECKED : BST_UNCHECKED); COptItem::MemToWnd(hWnd); }

	virtual void SetValue(INT_PTR _Value) { this->m_value = _Value; COptItem::SetValue(_Value); }
	virtual void SetDefValue(INT_PTR _DefValue) { this->m_defValue = _DefValue; COptItem::SetDefValue(_DefValue); }
	
	virtual INT_PTR GetValue() { return m_value; }
	virtual INT_PTR GetDefValue() { return m_defValue; }
	
	virtual COptItem* Copy() { return new COptItem_Radiobutton(*this); }

	int m_value;
	int m_defValue;
	int m_valueMask;
};

class COptItem_Combobox : public COptItem
{
public:
	COptItem_Combobox() {}
	COptItem_Combobox(int m_dlgItemID, char *szDBSetting, int nValueSize = DBVT_BYTE, int m_defValue = 0, int lParam = 0, bool m_bReadOnly = false)
		: COptItem(m_dlgItemID, szDBSetting, nValueSize, lParam, m_bReadOnly), m_defValue(m_defValue), m_value(0)
		{}
	
	void DBToMem(const CString &sModule, CString *sDBSettingPrefix = nullptr) { m_value = GetIntDBVal(sModule, false, sDBSettingPrefix); COptItem::DBToMem(sModule, sDBSettingPrefix); }
	void MemToDB(const CString &sModule, CString *sDBSettingPrefix = nullptr) { SetIntDBVal(sModule, m_value, sDBSettingPrefix); COptItem::MemToDB(sModule, sDBSettingPrefix); }
	void WndToMem(HWND hWnd) { m_value = SendDlgItemMessage(hWnd, m_dlgItemID, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hWnd, m_dlgItemID, CB_GETCURSEL, 0, 0), 0); COptItem::WndToMem(hWnd); }
	void MemToWnd(HWND hWnd) { SendDlgItemMessage(hWnd, m_dlgItemID, CB_SETCURSEL, m_value, 0); COptItem::MemToWnd(hWnd); }
	
	virtual void SetValue(INT_PTR _Value) { this->m_value = _Value; COptItem::SetValue(_Value); }
	virtual void SetDefValue(INT_PTR _DefValue) { this->m_defValue = _DefValue; COptItem::SetDefValue(_DefValue); }
	
	virtual INT_PTR GetValue() { return m_value; }
	virtual INT_PTR GetDefValue() { return m_defValue; }
	
	virtual COptItem* Copy() { return new COptItem_Combobox(*this); }

	int m_defValue;
	int m_value;
};

class COptItem_Colourpicker : public COptItem
{
public:
	COptItem_Colourpicker() {}
	COptItem_Colourpicker(int m_dlgItemID, char *szDBSetting, int m_defValue = 0, int lParam = 0, bool m_bReadOnly = false)
		: COptItem(m_dlgItemID, szDBSetting, DBVT_DWORD, lParam, m_bReadOnly), m_defValue(m_defValue), m_value(0)
		{}
	
	void DBToMem(const CString &sModule, CString *sDBSettingPrefix = nullptr) { m_value = GetIntDBVal(sModule, false, sDBSettingPrefix); COptItem::DBToMem(sModule, sDBSettingPrefix); }
	void MemToDB(const CString &sModule, CString *sDBSettingPrefix = nullptr) { SetIntDBVal(sModule, m_value, sDBSettingPrefix); COptItem::MemToDB(sModule, sDBSettingPrefix); }
	void WndToMem(HWND hWnd) { m_value = SendDlgItemMessage(hWnd, m_dlgItemID, CPM_GETCOLOUR, 0, 0); COptItem::WndToMem(hWnd); }
	void MemToWnd(HWND hWnd) { SendDlgItemMessage(hWnd, m_dlgItemID, CPM_SETCOLOUR, 0, m_value); COptItem::MemToWnd(hWnd); }
	
	virtual void SetValue(INT_PTR _Value) { this->m_value = _Value; COptItem::SetValue(_Value); }
	virtual void SetDefValue(INT_PTR _DefValue) { this->m_defValue = _DefValue; COptItem::SetDefValue(_DefValue); }
	
	virtual INT_PTR GetValue() { return m_value; }
	virtual INT_PTR GetDefValue() { return m_defValue; }
	
	virtual COptItem* Copy() { return new COptItem_Colourpicker(*this); }

	uint32_t m_defValue;
	uint32_t m_value;
};


class COptItem_Slider : public COptItem
{
public:
	COptItem_Slider() {}
	COptItem_Slider(int m_dlgItemID, char *szDBSetting, int nValueSize = DBVT_BYTE, int m_defValue = 0, int lParam = 0, bool m_bReadOnly = false)
		: COptItem(m_dlgItemID, szDBSetting, nValueSize, lParam, m_bReadOnly), m_defValue(m_defValue), m_value(0)
		{}
	
	void DBToMem(const CString &sModule, CString *sDBSettingPrefix = nullptr) { m_value = GetIntDBVal(sModule, false, sDBSettingPrefix); COptItem::DBToMem(sModule, sDBSettingPrefix); }
	void MemToDB(const CString &sModule, CString *sDBSettingPrefix = nullptr) { SetIntDBVal(sModule, m_value, sDBSettingPrefix); COptItem::MemToDB(sModule, sDBSettingPrefix); }
	void WndToMem(HWND hWnd) { m_value = SendDlgItemMessage(hWnd, m_dlgItemID, TBM_GETPOS, 0, 0); COptItem::WndToMem(hWnd); }
	void MemToWnd(HWND hWnd) { SendDlgItemMessage(hWnd, m_dlgItemID, TBM_SETPOS, true, m_value); COptItem::MemToWnd(hWnd); }
	
	virtual void SetValue(INT_PTR _Value) { this->m_value = _Value; COptItem::SetValue(_Value); }
	virtual void SetDefValue(INT_PTR _DefValue) { this->m_defValue = _DefValue; COptItem::SetDefValue(_DefValue); }
	
	virtual INT_PTR GetValue() { return m_value; }
	virtual INT_PTR GetDefValue() { return m_defValue; }
	virtual COptItem* Copy() { return new COptItem_Slider(*this); }

	int m_defValue;
	int m_value;
};


class COptItem_IntDBSetting : public COptItem
{
public:
	COptItem_IntDBSetting() {}
	COptItem_IntDBSetting(int m_dlgItemID, char *szDBSetting, int nValueSize = DBVT_BYTE, int bSigned = true, int m_defValue = 0, int lParam = 0, bool m_bReadOnly = false)
		: COptItem(m_dlgItemID, szDBSetting, nValueSize, lParam, m_bReadOnly), m_defValue(m_defValue), m_value(0), bSigned(bSigned)
		{}
	
	void DBToMem(const CString &sModule, CString *sDBSettingPrefix = nullptr) { m_value = GetIntDBVal(sModule, bSigned, sDBSettingPrefix); COptItem::DBToMem(sModule, sDBSettingPrefix); }
	void MemToDB(const CString &sModule, CString *sDBSettingPrefix = nullptr) { SetIntDBVal(sModule, m_value, sDBSettingPrefix); COptItem::MemToDB(sModule, sDBSettingPrefix); }
	void WndToMem(HWND hWnd) { COptItem::WndToMem(hWnd); }
	void MemToWnd(HWND hWnd) { COptItem::MemToWnd(hWnd); }
	
	virtual void SetValue(INT_PTR _Value) { this->m_value = _Value; COptItem::SetValue(_Value); }
	virtual void SetDefValue(INT_PTR _DefValue) { this->m_defValue = _DefValue; COptItem::SetDefValue(_DefValue); }

	virtual INT_PTR GetValue() { return m_value; }
	virtual INT_PTR GetDefValue() { return m_defValue; }
	virtual COptItem* Copy() { return new COptItem_IntDBSetting(*this); }

	int m_value;
	int m_defValue;
	int bSigned;
};


class COptItem_BitDBSetting : public COptItem
{
public:
	COptItem_BitDBSetting() {}
	COptItem_BitDBSetting(int m_dlgItemID, char *szDBSetting, int nValueSize = DBVT_BYTE, int m_defValue = 0, int m_valueMask = 0, int lParam = 0, bool m_bReadOnly = false) : COptItem(m_dlgItemID, szDBSetting, nValueSize, lParam, m_bReadOnly), m_defValue(m_defValue), m_value(0), m_valueMask(m_valueMask) {}

	void DBToMem(const CString &sModule, CString *sDBSettingPrefix = nullptr);
	void MemToDB(const CString &sModule, CString *sDBSettingPrefix = nullptr);
	void WndToMem(HWND hWnd) { COptItem::WndToMem(hWnd); }
	void MemToWnd(HWND hWnd) { COptItem::MemToWnd(hWnd); }

	virtual void SetValue(INT_PTR _Value) { this->m_value = _Value; COptItem::SetValue(_Value); }
	virtual void SetDefValue(INT_PTR _DefValue) { this->m_defValue = _DefValue; COptItem::SetDefValue(_DefValue); }

	virtual INT_PTR GetValue() { return m_value; }
	virtual INT_PTR GetDefValue() { return m_defValue; }
	virtual COptItem* Copy() { return new COptItem_BitDBSetting(*this); }

	int m_value;
	int m_defValue;
	int m_valueMask;
};


class COptItem_StrDBSetting : public COptItem
{
public:
	COptItem_StrDBSetting() {}
	COptItem_StrDBSetting(int m_dlgItemID, char *szDBSetting, int nMaxLen, wchar_t *szDefValue, int lParam = 0, bool m_bReadOnly = false) : COptItem(m_dlgItemID, szDBSetting, nMaxLen, lParam, m_bReadOnly), sDefValue(szDefValue) {}
	void DBToMem(const CString &sModule, CString *sDBSettingPrefix = nullptr) { sValue = GetStrDBVal(sModule, sDBSettingPrefix); COptItem::DBToMem(sModule, sDBSettingPrefix); }
	void MemToDB(const CString &sModule, CString *sDBSettingPrefix = nullptr) { SetStrDBVal(sModule, sValue, sDBSettingPrefix); COptItem::MemToDB(sModule, sDBSettingPrefix); }
	void WndToMem(HWND hWnd) { COptItem::WndToMem(hWnd); }
	void MemToWnd(HWND hWnd) { COptItem::MemToWnd(hWnd); }
	
	virtual void SetValue(INT_PTR _Value) { sValue = *(TCString*)_Value; COptItem::SetValue(_Value); }
	virtual void SetDefValue(INT_PTR _DefValue) { sDefValue = *(TCString*)_DefValue; COptItem::SetDefValue(_DefValue); }

	virtual INT_PTR GetValue() { return (INT_PTR)&sValue; }
	virtual INT_PTR GetDefValue() { return (INT_PTR)&sDefValue; }
	
	virtual COptItem* Copy() { return new COptItem_StrDBSetting(*this); }

	TCString sDefValue;
	TCString sValue;
};


// Tree item flags
#define TIF_GROUP 1 // is a group
#define TIF_EXPANDED 2 // item is expanded
#define TIF_ENABLED 4 // item is checked (has sense when the tree has checkboxes)
#define TIF_ROOTITEM 0x80 // item is a root item

class CBaseTreeItem
{
public:
	CBaseTreeItem();
	CBaseTreeItem(TCString Title, int ID, int Flags) : Title(Title), ID(ID), Flags(Flags), hItem(nullptr) {}

	TCString Title;
	int ID;
	int Flags;
	HTREEITEM hItem;
};

class CTreeItem : public CBaseTreeItem
{
public:
	CTreeItem();
	CTreeItem(TCString Title, int ParentID, int ID, int Flags = 0, TCString User_Str1 = nullptr) :
		CBaseTreeItem(Title, ID, Flags & ~TIF_ROOTITEM), ParentID(ParentID), User_Str1(User_Str1)
	{
	}

	int ParentID;
	TCString User_Str1;
};

class CTreeRootItem : public CBaseTreeItem
{
public:
	CTreeRootItem();
	CTreeRootItem(TCString Title, int ID, int Flags) : CBaseTreeItem(Title, ID, Flags | TIF_ROOTITEM) {}
};

typedef TMyArray<CTreeItem> TreeItemArray;
typedef TMyArray<CTreeRootItem> TreeRootItemArray;

#define TREECTRL_ROOTORDEROFFS -2
#define ROOT_INDEX_TO_ORDER(i) (TREECTRL_ROOTORDEROFFS - (i))
#define ROOT_ORDER_TO_INDEX(i) (TREECTRL_ROOTORDEROFFS - (i))

#define TREEITEMTITLE_MAXLEN 128
#define TREEITEM_DBSTR_TITLE "Title"
#define TREEITEM_DBSTR_PARENT "Parent"
#define TREEITEM_DBSTR_ORDER "Order"
#define TREEITEM_DBSTR_FLAGS "Flags"

// Tree control flags
#define TREECTRL_FLAG_IS_SINGLE_LEVEL 1 // means that the tree items can't have children, i.e. the tree is a plain list of items
#define TREECTRL_FLAG_HAS_CHECKBOXES 2
//#define TREECTRL_FLAG_UNORDERED 4 TODO?

class COptItem_TreeCtrl : public COptItem
{
public:
	COptItem_TreeCtrl() {}
	COptItem_TreeCtrl(int m_dlgItemID, char *szDBSetting, TreeItemArray &m_defValue, TreeRootItemArray RootItems, int lParam = 0, CString User_Str1_DBName = nullptr, bool m_bReadOnly = false, int TreeFlags = 0) : COptItem(m_dlgItemID, szDBSetting, DBVT_DWORD, lParam, m_bReadOnly), m_defValue(m_defValue), RootItems(RootItems), User_Str1_DBName(User_Str1_DBName), TreeFlags(TreeFlags)
	{
		if (TreeFlags & TREECTRL_FLAG_IS_SINGLE_LEVEL) {
			_ASSERT(!RootItems.GetSize()); // there can't be any root items when the tree is a plain list
			this->RootItems.AddElem(CTreeRootItem(L"", 0, TIF_EXPANDED)); // TODO??
			this->RootItems[0].hItem = TVI_ROOT;
		}
	}
	~COptItem_TreeCtrl() {}
	void DBToMem(const CString &sModule, CString *sDBSettingPrefix = nullptr);
	void MemToDB(const CString &sModule, CString *sDBSettingPrefix = nullptr);
	void WndToMem(HWND hWnd);
	void MemToWnd(HWND hWnd);
	void CleanDBSettings(const CString &sModule, CString *sDBSettingPrefix = nullptr);
	
	virtual void SetValue(INT_PTR _Value) { this->m_value = *(TreeItemArray*)_Value; COptItem::SetValue(_Value); }
	virtual void SetDefValue(INT_PTR _DefValue) { this->m_defValue = *(TreeItemArray*)_DefValue; COptItem::SetDefValue(_DefValue); }
	
	virtual INT_PTR GetValue() { return (INT_PTR)&m_value; }
	virtual INT_PTR GetDefValue() { return (INT_PTR)&m_defValue; }
	
	virtual COptItem* Copy() { return new COptItem_TreeCtrl(*this); }

	int IDToOrder(int ID);
	int hItemToOrder(HTREEITEM hItem);
	int GenerateID();
	int GetSelectedItemID(HWND hWnd);
	void Delete(HWND hWnd, int ID);
	CTreeItem* InsertItem(HWND hWnd, CTreeItem &Item);
	void MoveItem(HWND hWnd, HTREEITEM hItem, HTREEITEM hMoveTo);

	TreeItemArray m_defValue, m_value;
	TreeRootItemArray RootItems;
	CString User_Str1_DBName;
	int TreeFlags;

protected:
	void RecursiveDelete(HWND hWnd, int i);
	int RecursiveMove(int ItemOrder, int ParentID, int InsertAtOrder);
};


class CListItem
{
public:
	CListItem();
	CListItem(TCString Text) : Text(Text) {}

	TCString Text;
};

typedef TMyArray<CListItem> ListItemArray;

#define LISTITEM_DBSTR_TEXT "Text"

class COptItem_ListCtrl : public COptItem
{
public:
	COptItem_ListCtrl() {}
	COptItem_ListCtrl(int m_dlgItemID, char *szDBSetting, ListItemArray &m_defValue, int lParam = 0, bool m_bReadOnly = false) : COptItem(m_dlgItemID, szDBSetting, DBVT_DWORD, lParam, m_bReadOnly), m_defValue(m_defValue) {}
	~COptItem_ListCtrl() {}
	void DBToMem(const CString &sModule, CString *sDBSettingPrefix = nullptr);
	void MemToDB(const CString &sModule, CString *sDBSettingPrefix = nullptr);
	void WndToMem(HWND hWnd);
	void MemToWnd(HWND hWnd);
	void CleanDBSettings(const CString &sModule, CString *sDBSettingPrefix = nullptr);
	
	virtual void SetValue(INT_PTR _Value) { this->m_value = *(ListItemArray*)_Value; COptItem::SetValue(_Value); }
	virtual void SetDefValue(INT_PTR _DefValue) { this->m_defValue = *(ListItemArray*)_DefValue; COptItem::SetDefValue(_DefValue); }
	
	virtual INT_PTR GetValue() { return (INT_PTR)&m_value; }
	virtual INT_PTR GetDefValue() { return (INT_PTR)&m_defValue; }
	
	virtual COptItem* Copy() { return new COptItem_ListCtrl(*this); }

	int GetSelectedItemID(HWND hWnd); // returns -1 if there's no selection
	int SetSelectedItemID(HWND hWnd, int ID);
	void Delete(HWND hWnd, int ID);
	CListItem* InsertItem(HWND hWnd, int ID, CListItem &Item);
	void ModifyItem(HWND hWnd, int ID, CListItem &Item);

	ListItemArray m_defValue, m_value;
};


class COptPage
{
public:
	COptPage() : hWnd(nullptr), sDBSettingPrefix("") {}
	COptPage(char *szModule, HWND hWnd, CString sDBSettingPrefix = "") : sModule(szModule), hWnd(hWnd), sDBSettingPrefix(sDBSettingPrefix) {}
	COptPage(const COptPage &Item);
	~COptPage();

	void DBToMem();
	void MemToDB();
	void MemToPage(int OnlyEnable = 0);
	void PageToMem();
	void DBToMemToPage() { DBToMem(); MemToPage(); }
	void PageToMemToDB() { PageToMem(); MemToDB(); }
	void CleanDBSettings();

	COptItem* Find(int m_dlgItemID);
	INT_PTR GetValue(int m_dlgItemID) { return Find(m_dlgItemID)->GetValue(); }
	void SetValue(int m_dlgItemID, INT_PTR m_value) { Find(m_dlgItemID)->SetValue(m_value); }
	
	INT_PTR GetDBValue(int m_dlgItemID) { return Find(m_dlgItemID)->GetDBValue(sModule, &sDBSettingPrefix); }
	void SetDBValue(int m_dlgItemID, INT_PTR m_value) { Find(m_dlgItemID)->SetDBValue(sModule, m_value, &sDBSettingPrefix); }
	
	INT_PTR GetDBValueCopy(int m_dlgItemID) { return Find(m_dlgItemID)->GetDBValueCopy(sModule, &sDBSettingPrefix); }
	void SetDBValueCopy(int m_dlgItemID, INT_PTR m_value) { Find(m_dlgItemID)->SetDBValueCopy(sModule, m_value, &sDBSettingPrefix); }
	
	INT_PTR GetWndValue(int m_dlgItemID) { return Find(m_dlgItemID)->GetWndValue(hWnd); }
	void SetWndValue(int m_dlgItemID, INT_PTR m_value) { Find(m_dlgItemID)->SetWndValue(hWnd, m_value); }
	
	HWND GetWnd() { return hWnd; }
	void SetWnd(HWND _hWnd) { _ASSERT(!this->hWnd || !_hWnd); this->hWnd = _hWnd; }
	
	void Enable(int m_dlgItemID, bool m_bEnabled) { Find(m_dlgItemID)->Enable(m_bEnabled); }
	
	bool GetModified();
	void SetModified(bool m_bModified);

	COptPage& operator=(const COptPage& Page);

	HWND hWnd;
	CString sModule, sDBSettingPrefix;
	TMyArray<COptItem*, COptItem*> Items;
};
