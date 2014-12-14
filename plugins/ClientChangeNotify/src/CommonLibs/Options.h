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

#ifndef lengthof
#define lengthof(s) (sizeof(s) / sizeof(*s))
#endif


class COptItem
{
public:
	COptItem() {}
	COptItem(int DlgItemID, char *szDBSetting, int nValueSize, int lParam = 0, bool ReadOnly = false):
		DlgItemID(DlgItemID), nValueSize(nValueSize), sDBSetting(szDBSetting), lParam(lParam), Enabled(true), ReadOnly(ReadOnly), Modified(false) {}
/*	COptItem(const COptItem &Item): DlgItemID(Item.DlgItemID), nValueSize(Item.nValueSize),
		sDBSetting(Item.szDBSetting), lParam(Item.lParam), Enabled(Item.Enabled) {};*/
	virtual ~COptItem() {}

	virtual void DBToMem(CString &sModule, CString *sDBSettingPrefix = NULL) {Modified = false;}
	virtual void MemToDB(CString &sModule, CString *sDBSettingPrefix = NULL) {Modified = false;}
	virtual void WndToMem(HWND hWnd) {}
	virtual void MemToWnd(HWND hWnd) {EnableWindow(GetDlgItem(hWnd, DlgItemID), Enabled);}
	void DBToMemToWnd(CString &sModule, HWND hWnd, CString *sDBSettingPrefix = NULL) {DBToMem(sModule, sDBSettingPrefix); MemToWnd(hWnd);}
	void WndToMemToDB(HWND hWnd, CString &sModule, CString *sDBSettingPrefix = NULL) {WndToMem(hWnd); MemToDB(sModule, sDBSettingPrefix);}
	virtual void CleanDBSettings(CString &sModule, CString *sDBSettingPrefix = NULL) {db_unset(NULL, sModule, sDBSettingPrefix ? (*sDBSettingPrefix + sDBSetting) : sDBSetting);}; // TODO: also set Value to DefValue?

	virtual void SetValue(int Value) {Modified = true;}
	virtual void SetDefValue(int DefValue) {}
	virtual int GetValue() {return 0;}
	virtual int GetDefValue() {return 0;}
	int GetDBValue(CString &sModule, CString *sDBSettingPrefix = NULL) {DBToMem(sModule, sDBSettingPrefix); return GetValue();}
	void SetDBValue(CString &sModule, int Value, CString *sDBSettingPrefix = NULL) {SetValue(Value); MemToDB(sModule, sDBSettingPrefix);}
	int GetDBValueCopy(CString &sModule, CString *sDBSettingPrefix = NULL) {COptItem* Item = Copy(); Item->DBToMem(sModule, sDBSettingPrefix); int Value = Item->GetValue(); delete Item; return Value;} // retrieves DB value, but doesn't affect current page/item state; beware! it doesn't work with string values / other dynamic pointers
	void SetDBValueCopy(CString &sModule, int Value, CString *sDBSettingPrefix = NULL) {COptItem* Item = Copy(); Item->SetValue(Value); Item->MemToDB(sModule, sDBSettingPrefix); delete Item;}
	int GetWndValue(HWND hWnd) {WndToMem(hWnd); return GetValue();}
	void SetWndValue(HWND hWnd, int Value) {SetValue(Value); MemToWnd(hWnd);}
	void SetDlgItemID(int DlgItemID) {this->DlgItemID = DlgItemID;}
	bool GetModified() {return Modified;}
	void SetModified(bool Modified) {this->Modified = Modified;}

	void Enable(int Enabled) {this->Enabled = Enabled;}
	int GetParam() {return lParam;}
	int GetID() {return DlgItemID;}

//	virtual COptItem& operator = (const COptItem& Item) {return *this;};
	virtual COptItem* Copy() {_ASSERT(0); return NULL;} // Attention! Free Copy() result when it's not needed anymore!

	CString sDBSetting;

protected:
	int GetIntDBVal(CString &sModule, int bSigned = false, CString *sDBSettingPrefix = NULL);
	void SetIntDBVal(CString &sModule, int Value, CString *sDBSettingPrefix = NULL);
	TCString GetStrDBVal(CString &sModule, CString *sDBSettingPrefix = NULL);
	void SetStrDBVal(CString &sModule, TCString &Str, CString *sDBSettingPrefix = NULL);

	int DlgItemID;
	int Enabled;
	bool ReadOnly;
	bool Modified;
	int nValueSize; // maximum pValue size in bytes
	int lParam;
};


class COptItem_Generic : public COptItem
{
public:
	COptItem_Generic() {}
	COptItem_Generic(int DlgItemID, int lParam = 0): COptItem(DlgItemID, NULL, 0, lParam) {}
	virtual COptItem* Copy() {return new COptItem_Generic(*this);}
};


class COptItem_Edit : public COptItem
{
public:
	COptItem_Edit() {}
	COptItem_Edit(int DlgItemID, char *szDBSetting, int nMaxLen, TCHAR *szDefValue, int lParam = 0, bool ReadOnly = false): COptItem(DlgItemID, szDBSetting, nMaxLen, lParam, ReadOnly), sDefValue(szDefValue) {}
//	COptItem_Edit(const COptItem_Edit &Item): sDefValue(Item.sDefValue), sValue(Item.sValue) {}
	void DBToMem(CString &sModule, CString *sDBSettingPrefix = NULL) {sValue = GetStrDBVal(sModule, sDBSettingPrefix); COptItem::DBToMem(sModule, sDBSettingPrefix);}
	void MemToDB(CString &sModule, CString *sDBSettingPrefix = NULL) {SetStrDBVal(sModule, sValue, sDBSettingPrefix); COptItem::MemToDB(sModule, sDBSettingPrefix);}
	void WndToMem(HWND hWnd) {GetDlgItemText(hWnd, DlgItemID, sValue.GetBuffer(nValueSize), nValueSize); sValue.ReleaseBuffer(); COptItem::MemToWnd(hWnd);}
	void MemToWnd(HWND hWnd) {SetDlgItemText(hWnd, DlgItemID, sValue); COptItem::MemToWnd(hWnd);}
	void SetValue(int Value) {sValue = *(TCString*)Value; COptItem::SetValue(Value);}
	void SetDefValue(int DefValue) {sDefValue = *(TCString*)DefValue; COptItem::SetDefValue(DefValue);}
	int GetValue() {return (int)&sValue;}
	int GetDefValue() {return (int)&sDefValue;}

//	COptItem_Edit& operator = (const COptItem_Edit& Item) {return *this;};
	virtual COptItem* Copy() {return new COptItem_Edit(*this);}

	TCString sDefValue;
	TCString sValue;
};


class COptItem_IntEdit : public COptItem
{
public:
	COptItem_IntEdit() {}
	COptItem_IntEdit(int DlgItemID, char *szDBSetting, int nValueSize = DBVT_BYTE, int bSigned = true, int DefValue = 0, int lParam = 0, bool ReadOnly = false): COptItem(DlgItemID, szDBSetting, nValueSize, lParam, ReadOnly), DefValue(DefValue), Value(0), bSigned(bSigned) {}
	void DBToMem(CString &sModule, CString *sDBSettingPrefix = NULL) {Value = GetIntDBVal(sModule, bSigned, sDBSettingPrefix); COptItem::DBToMem(sModule, sDBSettingPrefix);}
	void MemToDB(CString &sModule, CString *sDBSettingPrefix = NULL) {SetIntDBVal(sModule, Value, sDBSettingPrefix); COptItem::MemToDB(sModule, sDBSettingPrefix);}
	void WndToMem(HWND hWnd) {Value = GetDlgItemInt(hWnd, DlgItemID, NULL, bSigned); COptItem::WndToMem(hWnd);}
	void MemToWnd(HWND hWnd) {SetDlgItemInt(hWnd, DlgItemID, Value, bSigned); COptItem::MemToWnd(hWnd);}
	void SetValue(int Value) {this->Value = Value; COptItem::SetValue(Value);}
	void SetDefValue(int DefValue) {this->DefValue = DefValue; COptItem::SetDefValue(DefValue);}
	int GetValue() {return Value;}
	int GetDefValue() {return DefValue;}
	virtual COptItem* Copy() {return new COptItem_IntEdit(*this);}

	int DefValue;
	int Value;
	int bSigned;
};


class COptItem_Checkbox : public COptItem
{
public:
	COptItem_Checkbox() {}
	COptItem_Checkbox(int DlgItemID, char *szDBSetting, int nValueSize = DBVT_BYTE, int DefValue = 0, int ValueMask = 0, int lParam = 0, bool ReadOnly = false): COptItem(DlgItemID, szDBSetting, nValueSize, lParam, ReadOnly), DefValue(DefValue), Value(0), ValueMask(ValueMask) {}

	void DBToMem(CString &sModule, CString *sDBSettingPrefix = NULL);
	void MemToDB(CString &sModule, CString *sDBSettingPrefix = NULL);
	void WndToMem(HWND hWnd);
	void MemToWnd(HWND hWnd);

	void SetValue(int Value) {this->Value = Value; COptItem::SetValue(Value);}
	void SetDefValue(int DefValue) {this->DefValue = DefValue; COptItem::SetDefValue(DefValue);}
	int GetValue() {return Value;}
	int GetDefValue() {return DefValue;}
	virtual COptItem* Copy() {return new COptItem_Checkbox(*this);}

	int Value;
	int DefValue;
	int ValueMask;
};


class COptItem_Radiobutton : public COptItem
{
public:
	COptItem_Radiobutton() {}
	COptItem_Radiobutton(int DlgItemID, char *szDBSetting, int nValueSize, int DefValue, int ValueMask, int lParam = 0, bool ReadOnly = false): COptItem(DlgItemID, szDBSetting, nValueSize, lParam, ReadOnly), DefValue(DefValue), Value(0), ValueMask(ValueMask) {}

	void DBToMem(CString &sModule, CString *sDBSettingPrefix = NULL) {Value = (GetIntDBVal(sModule, false, sDBSettingPrefix) == ValueMask) ? BST_CHECKED : BST_UNCHECKED; COptItem::DBToMem(sModule, sDBSettingPrefix);}
	void MemToDB(CString &sModule, CString *sDBSettingPrefix = NULL) {if ((Value == BST_CHECKED)) SetIntDBVal(sModule, ValueMask, sDBSettingPrefix); COptItem::MemToDB(sModule, sDBSettingPrefix);}
	void WndToMem(HWND hWnd) {Value = IsDlgButtonChecked(hWnd, DlgItemID); COptItem::WndToMem(hWnd);}
	void MemToWnd(HWND hWnd) {CheckDlgButton(hWnd, DlgItemID, Value ? BST_CHECKED : BST_UNCHECKED); COptItem::MemToWnd(hWnd);}

	void SetValue(int Value) {this->Value = Value; COptItem::SetValue(Value);}
	void SetDefValue(int DefValue) {this->DefValue = DefValue; COptItem::SetDefValue(DefValue);}
	int GetValue() {return Value;}
	int GetDefValue() {return DefValue;}
	virtual COptItem* Copy() {return new COptItem_Radiobutton(*this);}

	int Value;
	int DefValue;
	int ValueMask;
};


class COptItem_Combobox : public COptItem
{
public:
	COptItem_Combobox() {}
	COptItem_Combobox(int DlgItemID, char *szDBSetting, int nValueSize = DBVT_BYTE, int DefValue = 0, int lParam = 0, bool ReadOnly = false): COptItem(DlgItemID, szDBSetting, nValueSize, lParam, ReadOnly), DefValue(DefValue), Value(0) {}
	void DBToMem(CString &sModule, CString *sDBSettingPrefix = NULL) {Value = GetIntDBVal(sModule, false, sDBSettingPrefix); COptItem::DBToMem(sModule, sDBSettingPrefix);}
	void MemToDB(CString &sModule, CString *sDBSettingPrefix = NULL) {SetIntDBVal(sModule, Value, sDBSettingPrefix); COptItem::MemToDB(sModule, sDBSettingPrefix);}
	void WndToMem(HWND hWnd) {Value = SendDlgItemMessage(hWnd, DlgItemID, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(hWnd, DlgItemID, CB_GETCURSEL, 0, 0), 0); COptItem::WndToMem(hWnd);}
	void MemToWnd(HWND hWnd) {SendDlgItemMessage(hWnd, DlgItemID, CB_SETCURSEL, Value, 0); COptItem::MemToWnd(hWnd);}
	void SetValue(int Value) {this->Value = Value; COptItem::SetValue(Value);}
	void SetDefValue(int DefValue) {this->DefValue = DefValue; COptItem::SetDefValue(DefValue);}
	int GetValue() {return Value;}
	int GetDefValue() {return DefValue;}
	virtual COptItem* Copy() {return new COptItem_Combobox(*this);}

	int DefValue;
	int Value;
};


class COptItem_Colourpicker : public COptItem
{
public:
	COptItem_Colourpicker() {}
	COptItem_Colourpicker(int DlgItemID, char *szDBSetting, int DefValue = 0, int lParam = 0, bool ReadOnly = false): COptItem(DlgItemID, szDBSetting, DBVT_DWORD, lParam, ReadOnly), DefValue(DefValue), Value(0) {}
	void DBToMem(CString &sModule, CString *sDBSettingPrefix = NULL) {Value = GetIntDBVal(sModule, false, sDBSettingPrefix); COptItem::DBToMem(sModule, sDBSettingPrefix);}
	void MemToDB(CString &sModule, CString *sDBSettingPrefix = NULL) {SetIntDBVal(sModule, Value, sDBSettingPrefix); COptItem::MemToDB(sModule, sDBSettingPrefix);}
	void WndToMem(HWND hWnd) {Value = SendDlgItemMessage(hWnd, DlgItemID, CPM_GETCOLOUR, 0, 0); COptItem::WndToMem(hWnd);}
	void MemToWnd(HWND hWnd) {SendDlgItemMessage(hWnd, DlgItemID, CPM_SETCOLOUR, 0, Value); COptItem::MemToWnd(hWnd);}
	void SetValue(int Value) {this->Value = Value; COptItem::SetValue(Value);}
	void SetDefValue(int DefValue) {this->DefValue = DefValue; COptItem::SetDefValue(DefValue);}
	int GetValue() {return Value;}
	int GetDefValue() {return DefValue;}
	virtual COptItem* Copy() {return new COptItem_Colourpicker(*this);}

	DWORD DefValue;
	DWORD Value;
};


class COptItem_Slider : public COptItem
{
public:
	COptItem_Slider() {}
	COptItem_Slider(int DlgItemID, char *szDBSetting, int nValueSize = DBVT_BYTE, int DefValue = 0, int lParam = 0, bool ReadOnly = false): COptItem(DlgItemID, szDBSetting, nValueSize, lParam, ReadOnly), DefValue(DefValue), Value(0) {}
	void DBToMem(CString &sModule, CString *sDBSettingPrefix = NULL) {Value = GetIntDBVal(sModule, false, sDBSettingPrefix); COptItem::DBToMem(sModule, sDBSettingPrefix);}
	void MemToDB(CString &sModule, CString *sDBSettingPrefix = NULL) {SetIntDBVal(sModule, Value, sDBSettingPrefix); COptItem::MemToDB(sModule, sDBSettingPrefix);}
	void WndToMem(HWND hWnd) {Value = SendDlgItemMessage(hWnd, DlgItemID, TBM_GETPOS, 0, 0); COptItem::WndToMem(hWnd);}
	void MemToWnd(HWND hWnd) {SendDlgItemMessage(hWnd, DlgItemID, TBM_SETPOS, true, Value); COptItem::MemToWnd(hWnd);}
	void SetValue(int Value) {this->Value = Value; COptItem::SetValue(Value);}
	void SetDefValue(int DefValue) {this->DefValue = DefValue; COptItem::SetDefValue(DefValue);}
	int GetValue() {return Value;}
	int GetDefValue() {return DefValue;}
	virtual COptItem* Copy() {return new COptItem_Slider(*this);}

	int DefValue;
	int Value;
};


class COptItem_IntDBSetting : public COptItem
{
public:
	COptItem_IntDBSetting() {}
	COptItem_IntDBSetting(int DlgItemID, char *szDBSetting, int nValueSize = DBVT_BYTE, int bSigned = true, int DefValue = 0, int lParam = 0, bool ReadOnly = false): COptItem(DlgItemID, szDBSetting, nValueSize, lParam, ReadOnly), DefValue(DefValue), Value(0), bSigned(bSigned) {}
	void DBToMem(CString &sModule, CString *sDBSettingPrefix = NULL) {Value = GetIntDBVal(sModule, bSigned, sDBSettingPrefix); COptItem::DBToMem(sModule, sDBSettingPrefix);}
	void MemToDB(CString &sModule, CString *sDBSettingPrefix = NULL) {SetIntDBVal(sModule, Value, sDBSettingPrefix); COptItem::MemToDB(sModule, sDBSettingPrefix);}
	void WndToMem(HWND hWnd) {COptItem::WndToMem(hWnd);}
	void MemToWnd(HWND hWnd) {COptItem::MemToWnd(hWnd);}
	void SetValue(int Value) {this->Value = Value; COptItem::SetValue(Value);}
	void SetDefValue(int DefValue) {this->DefValue = DefValue; COptItem::SetDefValue(DefValue);}
	int GetValue() {return Value;}
	int GetDefValue() {return DefValue;}
	virtual COptItem* Copy() {return new COptItem_IntDBSetting(*this);}

	int Value;
	int DefValue;
	int bSigned;
};


class COptItem_BitDBSetting : public COptItem
{
public:
	COptItem_BitDBSetting() {}
	COptItem_BitDBSetting(int DlgItemID, char *szDBSetting, int nValueSize = DBVT_BYTE, int DefValue = 0, int ValueMask = 0, int lParam = 0, bool ReadOnly = false): COptItem(DlgItemID, szDBSetting, nValueSize, lParam, ReadOnly), DefValue(DefValue), Value(0), ValueMask(ValueMask) {}

	void DBToMem(CString &sModule, CString *sDBSettingPrefix = NULL);
	void MemToDB(CString &sModule, CString *sDBSettingPrefix = NULL);
	void WndToMem(HWND hWnd) {COptItem::WndToMem(hWnd);}
	void MemToWnd(HWND hWnd) {COptItem::MemToWnd(hWnd);}

	void SetValue(int Value) {this->Value = Value; COptItem::SetValue(Value);}
	void SetDefValue(int DefValue) {this->DefValue = DefValue; COptItem::SetDefValue(DefValue);}
	int GetValue() {return Value;}
	int GetDefValue() {return DefValue;}
	virtual COptItem* Copy() {return new COptItem_BitDBSetting(*this);}

	int Value;
	int DefValue;
	int ValueMask;
};


class COptItem_StrDBSetting : public COptItem
{
public:
	COptItem_StrDBSetting() {}
	COptItem_StrDBSetting(int DlgItemID, char *szDBSetting, int nMaxLen, TCHAR *szDefValue, int lParam = 0, bool ReadOnly = false): COptItem(DlgItemID, szDBSetting, nMaxLen, lParam, ReadOnly), sDefValue(szDefValue) {}
	void DBToMem(CString &sModule, CString *sDBSettingPrefix = NULL) {sValue = GetStrDBVal(sModule, sDBSettingPrefix); COptItem::DBToMem(sModule, sDBSettingPrefix);}
	void MemToDB(CString &sModule, CString *sDBSettingPrefix = NULL) {SetStrDBVal(sModule, sValue, sDBSettingPrefix); COptItem::MemToDB(sModule, sDBSettingPrefix);}
	void WndToMem(HWND hWnd) {COptItem::WndToMem(hWnd);}
	void MemToWnd(HWND hWnd) {COptItem::MemToWnd(hWnd);}
	void SetValue(int Value) {sValue = *(TCString*)Value; COptItem::SetValue(Value);}
	void SetDefValue(int DefValue) {sDefValue = *(TCString*)DefValue; COptItem::SetDefValue(DefValue);}
	int GetValue() {return (int)&sValue;}
	int GetDefValue() {return (int)&sDefValue;}
	virtual COptItem* Copy() {return new COptItem_StrDBSetting(*this);}

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
	CBaseTreeItem(TCString Title, int ID, int Flags): Title(Title), ID(ID), Flags(Flags), hItem(NULL) {}

	TCString Title;
	int ID;
	int Flags;
	HTREEITEM hItem;
};

class CTreeItem : public CBaseTreeItem
{
public:
	CTreeItem();
	CTreeItem(TCString Title, int ParentID, int ID, int Flags = 0, TCString User_Str1 = NULL):
		CBaseTreeItem(Title, ID, Flags & ~TIF_ROOTITEM), ParentID(ParentID), User_Str1(User_Str1) {}

	int ParentID;
	TCString User_Str1;
};

class CTreeRootItem : public CBaseTreeItem
{
public:
	CTreeRootItem();
	CTreeRootItem(TCString Title, int ID, int Flags): CBaseTreeItem(Title, ID, Flags | TIF_ROOTITEM) {}
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
	COptItem_TreeCtrl(int DlgItemID, char *szDBSetting, TreeItemArray &DefValue, TreeRootItemArray RootItems, int lParam = 0, CString User_Str1_DBName = NULL, bool ReadOnly = false, int TreeFlags = 0): COptItem(DlgItemID, szDBSetting, DBVT_DWORD, lParam, ReadOnly), DefValue(DefValue), RootItems(RootItems), User_Str1_DBName(User_Str1_DBName), TreeFlags(TreeFlags)
	{
		if (TreeFlags & TREECTRL_FLAG_IS_SINGLE_LEVEL)
		{
			_ASSERT(!RootItems.GetSize()); // there can't be any root items when the tree is a plain list
			this->RootItems.AddElem(CTreeRootItem(_T(""), 0, TIF_EXPANDED)); // TODO??
			this->RootItems[0].hItem = TVI_ROOT;
		}
	}
	~COptItem_TreeCtrl() {}
	void DBToMem(CString &sModule, CString *sDBSettingPrefix = NULL);
	void MemToDB(CString &sModule, CString *sDBSettingPrefix = NULL);
	void WndToMem(HWND hWnd);
	void MemToWnd(HWND hWnd);
	void CleanDBSettings(CString &sModule, CString *sDBSettingPrefix = NULL);
	void SetValue(int Value) {this->Value = *(TreeItemArray*)Value; COptItem::SetValue(Value);}
	void SetDefValue(int DefValue) {this->DefValue = *(TreeItemArray*)DefValue; COptItem::SetDefValue(DefValue);}
	int GetValue() {return (int)&Value;}
	int GetDefValue() {return (int)&DefValue;}
	virtual COptItem* Copy() {return new COptItem_TreeCtrl(*this);}

	int IDToOrder(int ID);
	int hItemToOrder(HTREEITEM hItem);
	int GenerateID();
	int GetSelectedItemID(HWND hWnd);
	void Delete(HWND hWnd, int ID);
	CTreeItem* InsertItem(HWND hWnd, CTreeItem &Item);
	void MoveItem(HWND hWnd, HTREEITEM hItem, HTREEITEM hMoveTo);

	TreeItemArray DefValue, Value;
	TreeRootItemArray RootItems;
	CString User_Str1_DBName;
	int TreeFlags;

protected:
	void RecursiveDelete(HWND hWnd, int I);
	int RecursiveMove(int ItemOrder, int ParentID, int InsertAtOrder);
};


class CListItem
{
public:
	CListItem();
	CListItem(TCString Text): Text(Text) {}

	TCString Text;
};

typedef TMyArray<CListItem> ListItemArray;

#define LISTITEM_DBSTR_TEXT "Text"

class COptItem_ListCtrl : public COptItem
{
public:
	COptItem_ListCtrl() {}
	COptItem_ListCtrl(int DlgItemID, char *szDBSetting, ListItemArray &DefValue, int lParam = 0, bool ReadOnly = false): COptItem(DlgItemID, szDBSetting, DBVT_DWORD, lParam, ReadOnly), DefValue(DefValue) {}
	~COptItem_ListCtrl() {}
	void DBToMem(CString &sModule, CString *sDBSettingPrefix = NULL);
	void MemToDB(CString &sModule, CString *sDBSettingPrefix = NULL);
	void WndToMem(HWND hWnd);
	void MemToWnd(HWND hWnd);
	void CleanDBSettings(CString &sModule, CString *sDBSettingPrefix = NULL);
	void SetValue(int Value) {this->Value = *(ListItemArray*)Value; COptItem::SetValue(Value);}
	void SetDefValue(int DefValue) {this->DefValue = *(ListItemArray*)DefValue; COptItem::SetDefValue(DefValue);}
	int GetValue() {return (int)&Value;}
	int GetDefValue() {return (int)&DefValue;}
	virtual COptItem* Copy() {return new COptItem_ListCtrl(*this);}

	int GetSelectedItemID(HWND hWnd); // returns -1 if there's no selection
	int SetSelectedItemID(HWND hWnd, int ID);
	void Delete(HWND hWnd, int ID);
	CListItem* InsertItem(HWND hWnd, int ID, CListItem &Item);
	void ModifyItem(HWND hWnd, int ID, CListItem &Item);

	ListItemArray DefValue, Value;
};


class COptPage
{
public:
	COptPage(): hWnd(NULL), sDBSettingPrefix("") {}
	COptPage(char *szModule, HWND hWnd, CString sDBSettingPrefix = ""): sModule(szModule), hWnd(hWnd), sDBSettingPrefix(sDBSettingPrefix) {}
	COptPage(const COptPage &Item);
	~COptPage();

	void DBToMem();
	void MemToDB();
	void MemToPage(int OnlyEnable = 0);
	void PageToMem();
	void DBToMemToPage() {DBToMem(); MemToPage();}
	void PageToMemToDB() {PageToMem(); MemToDB();}
	void CleanDBSettings();

	COptItem *Find(int DlgItemID);
	int GetValue(int DlgItemID) {return Find(DlgItemID)->GetValue();}
	void SetValue(int DlgItemID, int Value) {Find(DlgItemID)->SetValue(Value);}
	int GetDBValue(int DlgItemID) {return Find(DlgItemID)->GetDBValue(sModule, &sDBSettingPrefix);}
	void SetDBValue(int DlgItemID, int Value) {Find(DlgItemID)->SetDBValue(sModule, Value, &sDBSettingPrefix);}
	int GetDBValueCopy(int DlgItemID) {return Find(DlgItemID)->GetDBValueCopy(sModule, &sDBSettingPrefix);}
	void SetDBValueCopy(int DlgItemID, int Value) {Find(DlgItemID)->SetDBValueCopy(sModule, Value, &sDBSettingPrefix);}
	int GetWndValue(int DlgItemID) {return Find(DlgItemID)->GetWndValue(hWnd);}
	void SetWndValue(int DlgItemID, int Value) {Find(DlgItemID)->SetWndValue(hWnd, Value);}
	HWND GetWnd() {return hWnd;}
	void SetWnd(HWND hWnd) {_ASSERT(!this->hWnd || !hWnd); this->hWnd = hWnd;}
	void Enable(int DlgItemID, int Enabled) {Find(DlgItemID)->Enable(Enabled);}
	bool GetModified();
	void SetModified(bool Modified);

	COptPage& operator = (const COptPage& Page);

	HWND hWnd;
	CString sModule, sDBSettingPrefix;
	TMyArray<COptItem*, COptItem*> Items;
};
