/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007-09  Maxim Mluhov
Copyright (c) 2007-09  Victor Pavlychko
Copyright (ñ) 2012-15 Miranda NG project

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

#pragma once

#ifndef __M_GUI_H
#define __M_GUI_H

#include <m_protoint.h>
#include <m_clc.h>

#pragma warning(disable:4355 4251)

/////////////////////////////////////////////////////////////////////////////////////////
// helpers for the option's visualization

template<int Size>
struct CMDBTraits
{
};

template<>
struct CMDBTraits<1>
{
	typedef BYTE DBType;
	enum { DBTypeId = DBVT_BYTE };
	static __forceinline DBType Get(PROTO_INTERFACE *pPro, char *szSetting, DBType value)
	{
		return pPro->getByte(szSetting, value);
	}
	static __forceinline void Set(PROTO_INTERFACE *pPro, char *szSetting, DBType value)
	{
		pPro->setByte(szSetting, value);
	}
};

template<>
struct CMDBTraits<2>
{
	typedef WORD DBType;
	enum { DBTypeId = DBVT_WORD };
	static __forceinline DBType Get(PROTO_INTERFACE *pPro, char *szSetting, DBType value)
	{
		pPro->getWord(szSetting, value);
	}
	static __forceinline void Set(PROTO_INTERFACE *pPro, char *szSetting, DBType value)
	{
		pPro->setWord(szSetting, value);
	}
};

template<>
struct CMDBTraits<4>
{
	typedef DWORD DBType;
	enum { DBTypeId = DBVT_DWORD };
	static __forceinline DBType Get(PROTO_INTERFACE *pPro, char *szSetting, DBType value)
	{
		return pPro->getDword(szSetting, value);
	}
	static __forceinline void Set(PROTO_INTERFACE *pPro, char *szSetting, DBType value)
	{
		pPro->setDword(szSetting, value);
	}
};

class CMOptionBase
{
public:
	__forceinline char* GetDBModuleName() const { return m_proto->m_szModuleName; }
	__forceinline char* GetDBSettingName() const { return m_szSetting; }

protected:
	__forceinline CMOptionBase(PROTO_INTERFACE *proto, char *szSetting) :
		m_proto(proto), m_szSetting(szSetting)
	{}

	PROTO_INTERFACE *m_proto;
	char *m_szSetting;

private:
	CMOptionBase(const CMOptionBase &) {}
	void operator= (const CMOptionBase &) {}
};

template<class T>
class CMOption : public CMOptionBase
{
public:
	typedef T Type;

	__forceinline CMOption(PROTO_INTERFACE *proto, char *szSetting, Type defValue) :
		CMOptionBase(proto, szSetting), m_default(defValue)
	{}

	__forceinline operator Type()
	{
		return (Type)CMDBTraits<sizeof(Type)>::Get(m_proto, m_szSetting, m_default);
	}
	__forceinline Type operator= (Type value)
	{
		CMDBTraits<sizeof(Type)>::Set(m_proto, m_szSetting, (CMDBTraits<sizeof(Type)>::DBType)value);
		return value;
	}

private:
	Type m_default;

	CMOption(const CMOption &) : CMOptionBase(NULL, NULL, DBVT_DELETED) {}
	void operator= (const CMOption &) {}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Callbacks

struct CCallbackImp
{
	struct CDummy
	{	int foo;
	};

public:
	__inline CCallbackImp(): m_object(NULL), m_func(NULL) {}

	__inline CCallbackImp(const CCallbackImp &other): m_object(other.m_object), m_func(other.m_func) {}
	__inline CCallbackImp &operator=(const CCallbackImp &other) { m_object = other.m_object; m_func = other.m_func; return *this; }

	__inline bool operator==(const CCallbackImp &other) const { return (m_object == other.m_object) && (m_func == other.m_func); }
	__inline bool operator!=(const CCallbackImp &other) const { return (m_object != other.m_object) || (m_func != other.m_func); }

	__inline operator bool() const { return m_object && m_func; }

	__inline bool CheckObject(void *object) const { return (object == m_object) ? true : false; }

protected:
	template<typename TClass, typename TArgument>
	__inline CCallbackImp(TClass *object, void (TClass::*func)(TArgument *argument)): m_object((CDummy*)object), m_func((TFnCallback)func) {}

	__inline void Invoke(void *argument) const { if (m_func && m_object) (m_object->*m_func)(argument); }

private:
	typedef void (CDummy::*TFnCallback)(void *argument);

	CDummy* m_object;
	TFnCallback m_func;
};

template<typename TArgument>
struct CCallback: public CCallbackImp
{
	typedef CCallbackImp CSuper;

public:
	__inline CCallback() {}

	template<typename TClass>
	__inline CCallback(TClass *object, void (TClass::*func)(TArgument *argument)): CCallbackImp(object, func) {}

	__inline CCallback& operator=(const CCallbackImp& x) { CSuper::operator =(x); return *this; }

	__inline void operator()(TArgument *argument) const { Invoke((void*)argument); }
};

template<typename TClass, typename TArgument>
__inline CCallback<TArgument> Callback(TClass *object, void (TClass::*func)(TArgument *argument))
	{ return CCallback<TArgument>(object, func); }

/////////////////////////////////////////////////////////////////////////////////////////
// CDbLink

class MIR_CORE_EXPORT CDataLink
{
protected:
	BYTE m_type;

public:
	__inline CDataLink(BYTE type) : m_type(type) {}
	virtual ~CDataLink() {}

	__inline BYTE GetDataType() { return m_type; }

	virtual DWORD LoadInt() = 0;
	virtual void  SaveInt(DWORD value) = 0;

	virtual TCHAR* LoadText() = 0;
	virtual void   SaveText(TCHAR *value) = 0;
};

class MIR_CORE_EXPORT CDbLink : public CDataLink
{
	char *m_szModule;
	char *m_szSetting;
	bool m_bSigned;

	DWORD m_iDefault;
	TCHAR *m_szDefault;

	DBVARIANT dbv;

public:
	CDbLink(const char *szModule, const char *szSetting, BYTE type, DWORD iValue);
	CDbLink(const char *szModule, const char *szSetting, BYTE type, TCHAR *szValue);
	~CDbLink();

	DWORD LoadInt();
	void  SaveInt(DWORD value);

	TCHAR* LoadText();
	void   SaveText(TCHAR *value);
};

template<class T>
class CMOptionLink : public CDataLink
{
private:
	CMOption<T> *m_option;

public:
	__forceinline CMOptionLink(CMOption<T> &option) :
		CDataLink(CMDBTraits<sizeof(T)>::DBTypeId), m_option(&option)
	{}

	__forceinline DWORD LoadInt() { return (DWORD)(T)*m_option; }
	__forceinline void  SaveInt(DWORD value) { *m_option = (T)value; }

	__forceinline TCHAR* LoadText() { return NULL; }
	__forceinline void   SaveText(TCHAR*) {}
};

/////////////////////////////////////////////////////////////////////////////////////////
// CDlgBase - base dialog class

class MIR_CORE_EXPORT CDlgBase
{
	friend class CCtrlBase;
	friend class CCtrlData;

public:
	CDlgBase(HINSTANCE hInst, int idDialog);
	virtual ~CDlgBase();

	// general utilities
	void Create();
	void Show(int nCmdShow = SW_SHOW);
	int DoModal();

	void SetCaption(const TCHAR *ptszCaption);
	void NotifyChange(void); // sends a notification to a parent window

	__forceinline HINSTANCE GetInst() const { return m_hInst; }
	__forceinline HWND GetHwnd() const { return m_hwnd; }
	__forceinline bool IsInitialized() const { return m_initialized; }
	__forceinline void SetParent(HWND hwnd) { m_hwndParent = hwnd; }
	__forceinline void Close() { SendMessage(m_hwnd, WM_CLOSE, 0, 0); }

	static CDlgBase* Find(HWND hwnd);

protected:
	HWND      m_hwnd;  // must be the first data item
	HINSTANCE m_hInst;
	HWND      m_hwndParent;
	int       m_idDialog;
	bool      m_isModal;
	bool      m_initialized;
	bool      m_forceResizable;
	LRESULT   m_lresult;

	enum { CLOSE_ON_OK = 0x1, CLOSE_ON_CANCEL = 0x2 };
	BYTE    m_autoClose;    // automatically close dialog on IDOK/CANCEL commands. default: CLOSE_ON_OK|CLOSE_ON_CANCEL

	CCtrlBase* m_first;

	// override this handlers to provide custom functionality
	// general messages
	virtual void OnInitDialog() { }
	virtual void OnClose() { }
	virtual void OnDestroy() { }

	// miranda-related stuff
	virtual int Resizer(UTILRESIZECONTROL *urc);
	virtual void OnApply() {}
	virtual void OnReset() {}
	virtual void OnChange(CCtrlBase*) {}

	// main dialog procedure
	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	// resister controls
	void AddControl(CCtrlBase *ctrl);

	// win32 stuff
	void ThemeDialogBackground(BOOL tabbed);

private:
	LIST<CCtrlBase> m_controls;

	void NotifyControls(void (CCtrlBase::*fn)());
	CCtrlBase *FindControl(int idCtrl);

	static INT_PTR CALLBACK GlobalDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static int GlobalDlgResizer(HWND hwnd, LPARAM lParam, UTILRESIZECONTROL *urc);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlBase

class MIR_CORE_EXPORT CCtrlBase
{
	friend class CDlgBase;

public:
	CCtrlBase(CDlgBase *wnd, int idCtrl);
	virtual ~CCtrlBase() { }

	__forceinline HWND GetHwnd() const { return m_hwnd; }
	__forceinline int GetCtrlId() const { return m_idCtrl; }
	__forceinline CDlgBase *GetParent() { return m_parentWnd; }
	__forceinline bool IsChanged() const { return m_bChanged; }

	void Enable(int bIsEnable = true);
	__forceinline void Disable() { Enable(false); }
	BOOL Enabled(void) const;

	void NotifyChange();

	LRESULT SendMsg(UINT Msg, WPARAM wParam, LPARAM lParam);

	void SetText(const TCHAR *text);
	void SetTextA(const char *text);
	void SetInt(int value);

	TCHAR *GetText();
	char *GetTextA();

	TCHAR *GetText(TCHAR *buf, int size);
	char *GetTextA(char *buf, int size);

	int GetInt();

	virtual BOOL OnCommand(HWND /*hwndCtrl*/, WORD /*idCtrl*/, WORD /*idCode*/) { return FALSE; }
	virtual BOOL OnNotify(int /*idCtrl*/, NMHDR* /*pnmh*/) { return FALSE; }

	virtual BOOL OnMeasureItem(MEASUREITEMSTRUCT*) { return FALSE; }
	virtual BOOL OnDrawItem(DRAWITEMSTRUCT*) { return FALSE; }
	virtual BOOL OnDeleteItem(DELETEITEMSTRUCT*) { return FALSE; }

	virtual void OnInit();
	virtual void OnDestroy();

	virtual void OnApply();
	virtual void OnReset();

	static int cmp(const CCtrlBase *c1, const CCtrlBase *c2)
	{
		if (c1->m_idCtrl < c2->m_idCtrl) return -1;
		if (c1->m_idCtrl > c2->m_idCtrl) return +1;
		return 0;
	}

protected:
	HWND m_hwnd;  // must be the first data item
	int m_idCtrl;
	CCtrlBase* m_next;
	CDlgBase* m_parentWnd;
	bool m_bChanged;

public:
	CCallback<CCtrlBase> OnChange;

protected:
	virtual LRESULT CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	void Subclass();
	void Unsubclass();

private:
	static LRESULT CALLBACK GlobalSubclassWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlButton

class MIR_CORE_EXPORT CCtrlButton : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlButton(CDlgBase *dlg, int ctrlId);

	virtual BOOL OnCommand(HWND hwndCtrl, WORD idCtrl, WORD idCode);

	CCallback<CCtrlButton> OnClick;
};

class MIR_CORE_EXPORT CCtrlMButton : public CCtrlButton
{
	typedef CCtrlButton CSuper;

public:
	CCtrlMButton(CDlgBase *dlg, int ctrlId, HICON hIcon, const char* tooltip);
	CCtrlMButton(CDlgBase *dlg, int ctrlId, int iCoreIcon, const char* tooltip);
	~CCtrlMButton();

	void MakeFlat();
	void MakePush();

	virtual void OnInit();

protected:
	HICON m_hIcon;
	const char* m_toolTip;
};

class MIR_CORE_EXPORT CCtrlHyperlink : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlHyperlink(CDlgBase *dlg, int ctrlId, const char* url);

	virtual BOOL OnCommand(HWND hwndCtrl, WORD idCtrl, WORD idCode);

protected:
	const char* m_url;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlClc
class MIR_CORE_EXPORT CCtrlClc : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlClc(CDlgBase *dlg, int ctrlId);

	void       AddContact(MCONTACT hContact);
	void       AddGroup(HANDLE hGroup);
	void       AutoRebuild();
	void       DeleteItem(HANDLE hItem);
	void       EditLabel(HANDLE hItem);
	void       EndEditLabel(bool save);
	void       EnsureVisible(HANDLE hItem, bool partialOk);
	void       Expand(HANDLE hItem, DWORD flags);
	HANDLE     FindContact(MCONTACT hContact);
	HANDLE     FindGroup(HANDLE hGroup);
	COLORREF   GetBkColor();
	bool       GetCheck(HANDLE hItem);
	int        GetCount();
	HWND       GetEditControl();
	DWORD      GetExpand(HANDLE hItem);
	int        GetExtraColumns();
	BYTE       GetExtraImage(HANDLE hItem, int iColumn);
	HIMAGELIST GetExtraImageList();
	HFONT      GetFont(int iFontId);
	HANDLE     GetSelection();
	HANDLE     HitTest(int x, int y, DWORD *hitTest);
	void       SelectItem(HANDLE hItem);
	void       SetBkBitmap(DWORD mode, HBITMAP hBitmap);
	void       SetBkColor(COLORREF clBack);
	void       SetCheck(HANDLE hItem, bool check);
	void       SetExtraColumns(int iColumns);
	void       SetExtraImage(HANDLE hItem, int iColumn, int iImage);
	void       SetExtraImageList(HIMAGELIST hImgList);
	void       SetFont(int iFontId, HANDLE hFont, bool bRedraw);
	void       SetIndent(int iIndent);
	void       SetItemText(HANDLE hItem, char *szText);
	void       SetHideEmptyGroups(bool state);
	void       SetGreyoutFlags(DWORD flags);
	bool       GetHideOfflineRoot();
	void       SetHideOfflineRoot(bool state);
	void       SetUseGroups(bool state);
	void       SetOfflineModes(DWORD modes);
	DWORD      GetExStyle();
	void       SetExStyle(DWORD exStyle);
	int        GetLefrMargin();
	void       SetLeftMargin(int iMargin);
	HANDLE     AddInfoItem(CLCINFOITEM *cii);
	int        GetItemType(HANDLE hItem);
	HANDLE     GetNextItem(HANDLE hItem, DWORD flags);
	COLORREF   GetTextColor(int iFontId);
	void       SetTextColor(int iFontId, COLORREF clText);

	struct TEventInfo
	{
		CCtrlClc *ctrl;
		NMCLISTCONTROL *info;
	};

	CCallback<TEventInfo>	OnExpanded;
	CCallback<TEventInfo>	OnListRebuilt;
	CCallback<TEventInfo>	OnItemChecked;
	CCallback<TEventInfo>	OnDragging;
	CCallback<TEventInfo>	OnDropped;
	CCallback<TEventInfo>	OnListSizeChange;
	CCallback<TEventInfo>	OnOptionsChanged;
	CCallback<TEventInfo>	OnDragStop;
	CCallback<TEventInfo>	OnNewContact;
	CCallback<TEventInfo>	OnContactMoved;
	CCallback<TEventInfo>	OnCheckChanged;
	CCallback<TEventInfo>	OnClick;

protected:
	virtual BOOL OnNotify(int idCtrl, NMHDR *pnmh);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlData - data access controls base class

class MIR_CORE_EXPORT CCtrlData : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlData(CDlgBase *dlg, int ctrlId);
	virtual ~CCtrlData();

	void CreateDbLink(const char* szModuleName, const char* szSetting, BYTE type, DWORD iValue);
	void CreateDbLink(const char* szModuleName, const char* szSetting, TCHAR* szValue);
	void CreateDbLink(CDataLink *link) { m_dbLink = link; }

	virtual void OnInit();

protected:
	CDataLink *m_dbLink;

	__inline BYTE GetDataType() { return m_dbLink ? m_dbLink->GetDataType() : DBVT_DELETED; }
	__inline DWORD LoadInt() { return m_dbLink ? m_dbLink->LoadInt() : 0; }
	__inline void SaveInt(DWORD value) { if (m_dbLink) m_dbLink->SaveInt(value); }
	__inline const TCHAR *LoadText() { return m_dbLink ? m_dbLink->LoadText() : _T(""); }
	__inline void SaveText(TCHAR *value) { if (m_dbLink) m_dbLink->SaveText(value); }
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlCheck

class MIR_CORE_EXPORT CCtrlCheck : public CCtrlData
{
	typedef CCtrlData CSuper;

public:
	CCtrlCheck(CDlgBase *dlg, int ctrlId);
	virtual BOOL OnCommand(HWND /*hwndCtrl*/, WORD /*idCtrl*/, WORD /*idCode*/);

	virtual void OnApply();
	virtual void OnReset();

	int GetState();
	void SetState(int state);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlEdit

class MIR_CORE_EXPORT CCtrlEdit : public CCtrlData
{
	typedef CCtrlData CSuper;

public:
	CCtrlEdit(CDlgBase *dlg, int ctrlId);
	virtual BOOL OnCommand(HWND /*hwndCtrl*/, WORD /*idCtrl*/, WORD idCode);

	virtual void OnApply();
	virtual void OnReset();
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlListBox

class MIR_CORE_EXPORT CCtrlListBox : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlListBox(CDlgBase *dlg, int ctrlId);

	int    AddString(TCHAR *text, LPARAM data=0);
	void   DeleteString(int index);
	int    FindString(TCHAR *str, int index = -1, bool exact = false);
	int    GetCount();
	int    GetCurSel();
	LPARAM GetItemData(int index);
	TCHAR* GetItemText(int index);
	TCHAR* GetItemText(int index, TCHAR *buf, int size);
	bool   GetSel(int index);
	int    GetSelCount();
	int*   GetSelItems(int *items, int count);
	int*   GetSelItems();
	int    InsertString(TCHAR *text, int pos, LPARAM data=0);
	void   ResetContent();
	int    SelectString(TCHAR *str);
	int    SetCurSel(int index);
	void   SetItemData(int index, LPARAM data);
	void   SetSel(int index, bool sel=true);

	// Events
	CCallback<CCtrlListBox>	OnDblClick;
	CCallback<CCtrlListBox>	OnSelCancel;
	CCallback<CCtrlListBox>	OnSelChange;

protected:
	BOOL OnCommand(HWND hwndCtrl, WORD idCtrl, WORD idCode);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlCombo

class MIR_CORE_EXPORT CCtrlCombo : public CCtrlData
{
	typedef CCtrlData CSuper;

public:
	CCtrlCombo(CDlgBase *dlg, int ctrlId);

	virtual BOOL OnCommand(HWND /*hwndCtrl*/, WORD /*idCtrl*/, WORD idCode);
	virtual void OnInit();
	virtual void OnApply();
	virtual void OnReset();

	// Control interface
	int    AddString(const TCHAR *text, LPARAM data = 0);
	int    AddStringA(const char *text, LPARAM data = 0);
	void   DeleteString(int index);
	int    FindString(const TCHAR *str, int index = -1, bool exact = false);
	int    FindStringA(const char *str, int index = -1, bool exact = false);
	int    GetCount();
	int    GetCurSel();
	bool   GetDroppedState();
	LPARAM GetItemData(int index);
	TCHAR* GetItemText(int index);
	TCHAR* GetItemText(int index, TCHAR *buf, int size);
	int    InsertString(TCHAR *text, int pos, LPARAM data=0);
	void   ResetContent();
	int    SelectString(TCHAR *str);
	int    SetCurSel(int index);
	void   SetItemData(int index, LPARAM data);
	void   ShowDropdown(bool show = true);

	// Events
	CCallback<CCtrlCombo>	OnCloseup;
	CCallback<CCtrlCombo>	OnDropdown;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlListView

class MIR_CORE_EXPORT CCtrlListView : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlListView(CDlgBase *dlg, int ctrlId);

	// Classic LV interface
	DWORD      ApproximateViewRect(int cx, int cy, int iCount);
	void       Arrange(UINT code);
	void       CancelEditLabel();
	HIMAGELIST CreateDragImage(int iItem, LPPOINT lpptUpLeft);
	void       DeleteAllItems();
	void       DeleteColumn(int iCol);
	void       DeleteItem(int iItem);
	HWND       EditLabel(int iItem);
	int        EnableGroupView(BOOL fEnable);
	BOOL       EnsureVisible(int i, BOOL fPartialOK);
	int        FindItem(int iStart, const LVFINDINFO *plvfi);
	COLORREF   GetBkColor();
	void       GetBkImage(LPLVBKIMAGE plvbki);
	UINT       GetCallbackMask();
	BOOL       GetCheckState(UINT iIndex);
	void       GetColumn(int iCol, LPLVCOLUMN pcol);
	void       GetColumnOrderArray(int iCount, int *lpiArray);
	int        GetColumnWidth(int iCol);
	int        GetCountPerPage();
	HWND       GetEditControl();
	DWORD      GetExtendedListViewStyle();
	INT        GetFocusedGroup();
	int        GetGroupCount();
	void       GetGroupInfo(int iGroupId, PLVGROUP pgrp);
	void       GetGroupInfoByIndex(int iIndex, PLVGROUP pgrp);
	void       GetGroupMetrics(LVGROUPMETRICS *pGroupMetrics);
	UINT       GetGroupState(UINT dwGroupId, UINT dwMask);
	HWND       GetHeader();
	HCURSOR    GetHotCursor();
	INT        GetHotItem();
	DWORD      GetHoverTime();
	HIMAGELIST GetImageList(int iImageList);
	BOOL       GetInsertMark(LVINSERTMARK *plvim);
	COLORREF   GetInsertMarkColor();
	int        GetInsertMarkRect(LPRECT prc);
	BOOL       GetISearchString(LPSTR lpsz);
	bool       GetItem(LPLVITEM pitem);
	int        GetItemCount();
	void       GetItemPosition(int i, POINT *ppt);
	void       GetItemRect(int i, RECT *prc, int code);
	DWORD      GetItemSpacing(BOOL fSmall);
	UINT       GetItemState(int i, UINT mask);
	void       GetItemText(int iItem, int iSubItem, LPTSTR pszText, int cchTextMax);
	int        GetNextItem(int iStart, UINT flags);
	BOOL       GetNumberOfWorkAreas(LPUINT lpuWorkAreas);
	BOOL       GetOrigin(LPPOINT lpptOrg);
	COLORREF   GetOutlineColor();
	UINT       GetSelectedColumn();
	UINT       GetSelectedCount();
	INT        GetSelectionMark();
	int        GetStringWidth(LPCSTR psz);
	BOOL       GetSubItemRect(int iItem, int iSubItem, int code, LPRECT lpRect);
	COLORREF   GetTextBkColor();
	COLORREF   GetTextColor();
	void       GetTileInfo(PLVTILEINFO plvtinfo);
	void       GetTileViewInfo(PLVTILEVIEWINFO plvtvinfo);
	HWND       GetToolTips();
	int        GetTopIndex();
	BOOL       GetUnicodeFormat();
	DWORD      GetView();
	BOOL       GetViewRect(RECT *prc);
	void       GetWorkAreas(INT nWorkAreas, LPRECT lprc);
	BOOL       HasGroup(int dwGroupId);
	int        HitTest(LPLVHITTESTINFO pinfo);
	int        HitTestEx(LPLVHITTESTINFO pinfo);
	int        InsertColumn(int iCol, const LPLVCOLUMN pcol);
	int        InsertGroup(int index, PLVGROUP pgrp);
	void       InsertGroupSorted(PLVINSERTGROUPSORTED structInsert);
	int        InsertItem(const LPLVITEM pitem);
	BOOL       InsertMarkHitTest(LPPOINT point, LVINSERTMARK *plvim);
	BOOL       IsGroupViewEnabled();
	UINT       IsItemVisible(UINT index);
	UINT       MapIDToIndex(UINT id);
	UINT       MapIndexToID(UINT index);
	BOOL       RedrawItems(int iFirst, int iLast);
	void       RemoveAllGroups();
	int        RemoveGroup(int iGroupId);
	BOOL       Scroll(int dx, int dy);
	BOOL       SetBkColor(COLORREF clrBk);
	BOOL       SetBkImage(LPLVBKIMAGE plvbki);
	BOOL       SetCallbackMask(UINT mask);
	void       SetCheckState(UINT iIndex, BOOL fCheck);
	BOOL       SetColumn(int iCol, LPLVCOLUMN pcol);
	BOOL       SetColumnOrderArray(int iCount, int *lpiArray);
	BOOL       SetColumnWidth(int iCol, int cx);
	void       SetExtendedListViewStyle(DWORD dwExStyle);
	void       SetExtendedListViewStyleEx(DWORD dwExMask, DWORD dwExStyle);
	int        SetGroupInfo(int iGroupId, PLVGROUP pgrp);
	void       SetGroupMetrics(PLVGROUPMETRICS pGroupMetrics);
	void       SetGroupState(UINT dwGroupId, UINT dwMask, UINT dwState);
	HCURSOR    SetHotCursor(HCURSOR hCursor);
	INT        SetHotItem(INT iIndex);
	void       SetHoverTime(DWORD dwHoverTime);
	DWORD      SetIconSpacing(int cx, int cy);
	HIMAGELIST SetImageList(HIMAGELIST himl, int iImageList);
	BOOL       SetInfoTip(PLVSETINFOTIP plvSetInfoTip);
	BOOL       SetInsertMark(LVINSERTMARK *plvim);
	COLORREF   SetInsertMarkColor(COLORREF color);
	BOOL       SetItem(const LPLVITEM pitem);
	void       SetItemCount(int cItems);
	void       SetItemCountEx(int cItems, DWORD dwFlags);
	BOOL       SetItemPosition(int i, int x, int y);
	void       SetItemPosition32(int iItem, int x, int y);
	void       SetItemState(int i, UINT state, UINT mask);
	void       SetItemText(int i, int iSubItem, TCHAR *pszText);
	COLORREF   SetOutlineColor(COLORREF color);
	void       SetSelectedColumn(int iCol);
	INT        SetSelectionMark(INT iIndex);
	BOOL       SetTextBkColor(COLORREF clrText);
	BOOL       SetTextColor(COLORREF clrText);
	BOOL       SetTileInfo(PLVTILEINFO plvtinfo);
	BOOL       SetTileViewInfo(PLVTILEVIEWINFO plvtvinfo);
	HWND       SetToolTips(HWND ToolTip);
	BOOL       SetUnicodeFormat(BOOL fUnicode);
	int        SetView(DWORD iView);
	void       SetWorkAreas(INT nWorkAreas, LPRECT lprc);
	int        SortGroups(PFNLVGROUPCOMPARE pfnGroupCompare, LPVOID plv);
	BOOL       SortItems(PFNLVCOMPARE pfnCompare, LPARAM lParamSort);
	BOOL       SortItemsEx(PFNLVCOMPARE pfnCompare, LPARAM lParamSort);
	INT        SubItemHitTest(LPLVHITTESTINFO pInfo);
	INT        SubItemHitTestEx(LPLVHITTESTINFO plvhti);
	BOOL       Update(int iItem);

	// Additional APIs
	HIMAGELIST CreateImageList(int iImageList);
	void       AddColumn(int iSubItem, TCHAR *name, int cx);
	void       AddGroup(int iGroupId, TCHAR *name);
	int        AddItem(TCHAR *text, int iIcon, LPARAM lParam = 0, int iGroupId = -1);
	void       SetItem(int iItem, int iSubItem, TCHAR *text, int iIcon = -1);
	LPARAM     GetItemData(int iItem);

	// Events
	struct TEventInfo {
		CCtrlListView *treeviewctrl;
		union {
			NMHDR			*nmhdr;
			NMLISTVIEW		*nmlv;
			NMLVDISPINFO	*nmlvdi;
			NMLVSCROLL		*nmlvscr;
			NMLVGETINFOTIP	*nmlvit;
			NMLVFINDITEM	*nmlvfi;
			NMITEMACTIVATE	*nmlvia;
			NMLVKEYDOWN		*nmlvkey;
		};
	};

	CCallback<TEventInfo> OnBeginDrag;
	CCallback<TEventInfo> OnBeginLabelEdit;
	CCallback<TEventInfo> OnBeginRDrag;
	CCallback<TEventInfo> OnBeginScroll;
	CCallback<TEventInfo> OnColumnClick;
	CCallback<TEventInfo> OnDeleteAllItems;
	CCallback<TEventInfo> OnDeleteItem;
	CCallback<TEventInfo> OnDoubleClick;
	CCallback<TEventInfo> OnEndLabelEdit;
	CCallback<TEventInfo> OnEndScroll;
	CCallback<TEventInfo> OnGetDispInfo;
	CCallback<TEventInfo> OnGetInfoTip;
	CCallback<TEventInfo> OnHotTrack;
	CCallback<TEventInfo> OnIncrementalSearch;
	CCallback<TEventInfo> OnInsertItem;
	CCallback<TEventInfo> OnItemActivate;
	CCallback<TEventInfo> OnItemChanged;
	CCallback<TEventInfo> OnItemChanging;
	CCallback<TEventInfo> OnKeyDown;
	CCallback<TEventInfo> OnMarqueeBegin;
	CCallback<TEventInfo> OnSetDispInfo;

protected:
	virtual BOOL OnNotify(int idCtrl, NMHDR *pnmh);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlTreeView

#undef GetNextSibling
#undef GetPrevSibling

#define MTREE_CHECKBOX 0x0001
#define MTREE_DND      0x0002

class MIR_CORE_EXPORT CCtrlTreeView : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlTreeView(CDlgBase *dlg, int ctrlId);

	void       SetFlags(uint32_t dwFlags); // MTREE_* combination

	// Classic TV interface
	HIMAGELIST CreateDragImage(HTREEITEM hItem);
	void       DeleteAllItems();
	void       DeleteItem(HTREEITEM hItem);
	HWND       EditLabel(HTREEITEM hItem);
	void       EndEditLabelNow(BOOL cancel);
	void       EnsureVisible(HTREEITEM hItem);
	void       Expand(HTREEITEM hItem, DWORD flag);
	COLORREF   GetBkColor();
	DWORD      GetCheckState(HTREEITEM hItem);
	HTREEITEM  GetChild(HTREEITEM hItem);
	int        GetCount();
	HTREEITEM  GetDropHilight();
	HWND       GetEditControl();
	HTREEITEM  GetFirstVisible();
	HIMAGELIST GetImageList(int iImage);
	int        GetIndent();
	COLORREF   GetInsertMarkColor();
	bool       GetItem(TVITEMEX *tvi);
	int        GetItemHeight();
	void       GetItemRect(HTREEITEM hItem, RECT *rcItem, BOOL fItemRect);
	DWORD      GetItemState(HTREEITEM hItem, DWORD stateMask);
	HTREEITEM  GetLastVisible();
	COLORREF   GetLineColor();
	HTREEITEM  GetNextItem(HTREEITEM hItem, DWORD flag);
	HTREEITEM  GetNextSibling(HTREEITEM hItem);
	HTREEITEM  GetNextVisible(HTREEITEM hItem);
	HTREEITEM  GetParent(HTREEITEM hItem);
	HTREEITEM  GetPrevSibling(HTREEITEM hItem);
	HTREEITEM  GetPrevVisible(HTREEITEM hItem);
	HTREEITEM  GetRoot();
	DWORD      GetScrollTime();
	HTREEITEM  GetSelection();
	COLORREF   GetTextColor();
	HWND       GetToolTips();
	BOOL       GetUnicodeFormat();
	unsigned   GetVisibleCount();
	HTREEITEM  HitTest(TVHITTESTINFO *hti);
	HTREEITEM  InsertItem(TVINSERTSTRUCT *tvis);
	void       Select(HTREEITEM hItem, DWORD flag);
	void       SelectDropTarget(HTREEITEM hItem);
	void       SelectItem(HTREEITEM hItem);
	void       SelectSetFirstVisible(HTREEITEM hItem);
	COLORREF   SetBkColor(COLORREF clBack);
	void       SetCheckState(HTREEITEM hItem, DWORD state);
	void       SetImageList(HIMAGELIST hIml, int iImage);
	void       SetIndent(int iIndent);
	void       SetInsertMark(HTREEITEM hItem, BOOL fAfter);
	COLORREF   SetInsertMarkColor(COLORREF clMark);
	void       SetItem(TVITEMEX *tvi);
	void       SetItemHeight(short cyItem);
	void       SetItemState(HTREEITEM hItem, DWORD state, DWORD stateMask);
	COLORREF   SetLineColor(COLORREF clLine);
	void       SetScrollTime(UINT uMaxScrollTime);
	COLORREF   SetTextColor(COLORREF clText);
	HWND       SetToolTips(HWND hwndToolTips);
	BOOL       SetUnicodeFormat(BOOL fUnicode);
	void       SortChildren(HTREEITEM hItem, BOOL fRecurse);
	void       SortChildrenCB(TVSORTCB *cb, BOOL fRecurse);

	// Additional stuff
	void       TranslateItem(HTREEITEM hItem);
	void       TranslateTree();
	HTREEITEM  FindNamedItem(HTREEITEM hItem, const TCHAR *name);
	void       GetItem(HTREEITEM hItem, TVITEMEX *tvi);
	void       GetItem(HTREEITEM hItem, TVITEMEX *tvi, TCHAR *szText, int iTextLength);
	void       InvertCheck(HTREEITEM hItem);

	// Events
	struct TEventInfo {
		CCtrlTreeView *treeviewctrl;
		union {
			NMHDR *nmhdr;
			NMTREEVIEW *nmtv;
			NMTVKEYDOWN *nmtvkey;
			NMTVDISPINFO *nmtvdi;
			NMTVGETINFOTIP *nmtvit;
		};
	};

	CCallback<TEventInfo> OnBeginDrag;
	CCallback<TEventInfo> OnBeginLabelEdit;
	CCallback<TEventInfo> OnBeginRDrag;
	CCallback<TEventInfo> OnDeleteItem;
	CCallback<TEventInfo> OnEndLabelEdit;
	CCallback<TEventInfo> OnGetDispInfo;
	CCallback<TEventInfo> OnGetInfoTip;
	CCallback<TEventInfo> OnItemExpanded;
	CCallback<TEventInfo> OnItemExpanding;
	CCallback<TEventInfo> OnKeyDown;
	CCallback<TEventInfo> OnSelChanged;
	CCallback<TEventInfo> OnSelChanging;
	CCallback<TEventInfo> OnSetDispInfo;
	CCallback<TEventInfo> OnSingleExpand;

protected:
	virtual void OnInit();
	virtual BOOL OnNotify(int idCtrl, NMHDR *pnmh);
	
	virtual LRESULT CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam);

	union {
		uint32_t m_dwFlags;
		struct {
			bool m_bDndEnabled : 1;
			bool m_bDragging : 1;
			bool m_bCheckBox : 1;
		};
	};
	HTREEITEM m_hDragItem; // valid if m_bDragging == true
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlTreeView

class MIR_CORE_EXPORT CCtrlPages : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlPages(CDlgBase *dlg, int ctrlId);

	void AddPage(TCHAR *ptszName, HICON hIcon, CCallback<void> onCreate = CCallback<void>(), void *param = NULL);
	void AttachDialog(int iPage, CDlgBase *pDlg);

	void ActivatePage(int iPage);


protected:
	virtual BOOL OnNotify(int idCtrl, NMHDR *pnmh);
	void OnInit();
	void OnDestroy();

	virtual LRESULT CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam);

private:
	HIMAGELIST m_hIml;
	CDlgBase *m_pActivePage;

	struct TPageInfo
	{
		CCallback<void> m_onCreate;
		void *m_param;
		CDlgBase *m_pDlg;
	};

	void ShowPage(CDlgBase *pDlg);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlCustom

template<typename TDlg>
class MIR_CORE_EXPORT CCtrlCustom : public CCtrlBase
{
	typedef CCtrlBase CSuper;

private:
	void (TDlg::*m_pfnOnCommand)(HWND hwndCtrl, WORD idCtrl, WORD idCode);
	void (TDlg::*m_pfnOnNotify)(int idCtrl, NMHDR *pnmh);
	void (TDlg::*m_pfnOnMeasureItem)(MEASUREITEMSTRUCT *param);
	void (TDlg::*m_pfnOnDrawItem)(DRAWITEMSTRUCT *param);
	void (TDlg::*m_pfnOnDeleteItem)(DELETEITEMSTRUCT *param);

public:
	CCtrlCustom(TDlg *wnd, int idCtrl,
		void (TDlg::*pfnOnCommand)(HWND hwndCtrl, WORD idCtrl, WORD idCode),
		void (TDlg::*pfnOnNotify)(int idCtrl, NMHDR *pnmh),
		void (TDlg::*pfnOnMeasureItem)(MEASUREITEMSTRUCT *param) = NULL,
		void (TDlg::*pfnOnDrawItem)(DRAWITEMSTRUCT *param) = NULL,
		void (TDlg::*pfnOnDeleteItem)(DELETEITEMSTRUCT *param) = NULL): CCtrlBase(wnd, idCtrl)
	{
		m_pfnOnCommand		= pfnOnCommand;
		m_pfnOnNotify		= pfnOnNotify;
		m_pfnOnMeasureItem	= pfnOnMeasureItem;
		m_pfnOnDrawItem		= pfnOnDrawItem;
		m_pfnOnDeleteItem	= pfnOnDeleteItem;
	}

	virtual BOOL OnCommand(HWND hwndCtrl, WORD idCtrl, WORD idCode)
	{
		if (m_parentWnd && m_pfnOnCommand) {
			m_parentWnd->m_lresult = 0;
			(((TDlg *)m_parentWnd)->*m_pfnOnCommand)(hwndCtrl, idCtrl, idCode);
			return m_parentWnd->m_lresult;
		}
		return FALSE;
	}
	virtual BOOL OnNotify(int idCtrl, NMHDR *pnmh)
	{
		if (m_parentWnd && m_pfnOnNotify) {
			m_parentWnd->m_lresult = 0;
			(((TDlg *)m_parentWnd)->*m_pfnOnNotify)(idCtrl, pnmh);
			return m_parentWnd->m_lresult;
		}
		return FALSE;
	}

	virtual BOOL OnMeasureItem(MEASUREITEMSTRUCT *param)
	{
		if (m_parentWnd && m_pfnOnMeasureItem) {
			m_parentWnd->m_lresult = 0;
			(((TDlg *)m_parentWnd)->*m_pfnOnMeasureItem)(param);
			return m_parentWnd->m_lresult;
		}
		return FALSE;
	}
	virtual BOOL OnDrawItem(DRAWITEMSTRUCT *param)
	{
		if (m_parentWnd && m_pfnOnDrawItem) {
			m_parentWnd->m_lresult = 0;
			(((TDlg *)m_parentWnd)->*m_pfnOnDrawItem)(param);
			return m_parentWnd->m_lresult;
		}
		return FALSE;
	}
	virtual BOOL OnDeleteItem(DELETEITEMSTRUCT *param)
	{
		if (m_parentWnd && m_pfnOnDeleteItem) {
			m_parentWnd->m_lresult = 0;
			(((TDlg *)m_parentWnd)->*m_pfnOnDeleteItem)(param);
			return m_parentWnd->m_lresult;
		}
		return FALSE;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// CProtoDlgBase

#define WM_PROTO_REFRESH              (WM_USER + 100)
#define WM_PROTO_CHECK_ONLINE         (WM_USER + 101)
#define WM_PROTO_ACTIVATE             (WM_USER + 102)
#define WM_PROTO_LAST                 (WM_USER + 200)

struct PROTO_INTERFACE;

class MIR_CORE_EXPORT CProtoIntDlgBase : public CDlgBase
{
	typedef CDlgBase CSuper;

public:
	CProtoIntDlgBase(PROTO_INTERFACE *proto, int idDialog, bool show_label = true);

	void CreateLink(CCtrlData& ctrl, char *szSetting, BYTE type, DWORD iValue);
	void CreateLink(CCtrlData& ctrl, const char *szSetting, TCHAR *szValue);

	template<class T>
	__inline void CreateLink(CCtrlData& ctrl, CMOption<T> &option)
	{
		ctrl.CreateDbLink(new CMOptionLink<T>(option));
	}

	__inline PROTO_INTERFACE *GetProtoInterface() { return m_proto_interface; }

	void SetStatusText(const TCHAR *statusText);

protected:
	PROTO_INTERFACE *m_proto_interface;
	bool m_show_label;
	HWND m_hwndStatus;

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	virtual void OnProtoRefresh(WPARAM, LPARAM);
	virtual void OnProtoActivate(WPARAM, LPARAM);
	virtual void OnProtoCheckOnline(WPARAM, LPARAM);

private:
	void UpdateProtoTitle(const TCHAR *szText = NULL);
	void UpdateStatusBar();
};

template<typename TProto>
class CProtoDlgBase : public CProtoIntDlgBase
{
	typedef CProtoIntDlgBase CSuper;

public:
	__inline CProtoDlgBase<TProto>(TProto *proto, int idDialog, bool show_label=true) :
		CProtoIntDlgBase(proto, idDialog, show_label),
		m_proto(proto)
	{
	}

	__inline TProto *GetProto() { return m_proto; }

protected:
	TProto* m_proto;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Safe open/close dialogs
#define UI_SAFE_OPEN(dlgClass, dlgPtr)	\
	{	\
		if (dlgPtr)	\
		{	\
			SetForegroundWindow((dlgPtr)->GetHwnd());	\
		} else	\
		{	\
			(dlgPtr) = new dlgClass(this);	\
			(dlgPtr)->Show();	\
		}	\
	}

#define UI_SAFE_OPEN_EX(dlgClass, dlgPtr, dlgLocal)	\
	if (dlgPtr)	\
	{	\
		::SetForegroundWindow((dlgPtr)->GetHwnd());	\
	} else	\
	{	\
		(dlgPtr) = new dlgClass(this);	\
		(dlgPtr)->Show();	\
	}	\
	dlgClass *dlgLocal = (dlgClass *)(dlgPtr);

#define UI_SAFE_CLOSE(dlg)	\
	{	\
		if (dlg) {	\
			(dlg)->Close();	\
			(dlg) = NULL;	\
		}	\
	}

#define UI_SAFE_CLOSE_HWND(hwnd)	\
	{	\
		if (hwnd) {	\
			::SendMessage((hwnd), WM_CLOSE, 0, 0);	\
			(hwnd) = NULL;	\
		}	\
	}

/////////////////////////////////////////////////////////////////////////////////////////
// NULL-Safe dialog notifications
#define UI_SAFE_NOTIFY(dlg, msg)	\
	{	\
		if (dlg)	\
			::SendMessage((dlg)->GetHwnd(), msg, 0, 0);	\
	}

#define UI_SAFE_NOTIFY_HWND(hwnd, msg)	\
	{	\
		if (hwnd)	\
			::SendMessage((hwnd), msg, 0, 0);	\
	}

/////////////////////////////////////////////////////////////////////////////////////////
// Define message maps
#define UI_MESSAGE_MAP(dlgClass, baseDlgClass)	\
	typedef baseDlgClass CMessageMapSuperClass;	\
	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)	\
	{	\
		switch (msg)	\
		{	\
		case 0:	\
			break	/* just to handle ";" symbol after macro */

#define UI_MESSAGE(msg, proc)	\
		case msg:	\
			proc(msg, wParam, lParam);	\
			break

#define UI_MESSAGE_EX(msg, func)	\
		case msg:	\
			return func(msg, wParam, lParam)

#define UI_POSTPROCESS_MESSAGE(msg, proc)	\
		case msg:	\
			CMessageMapSuperClass::DlgProc(msg, wParam, lParam);	\
			return FALSE

#define UI_POSTPROCESS_MESSAGE_EX(msg, func)	\
		case msg:	\
			CMessageMapSuperClass::DlgProc(msg, wParam, lParam);	\
			return func(msg, wParam, lParam)

#define UI_MESSAGE_MAP_END()	\
		}	\
		return CMessageMapSuperClass::DlgProc(msg, wParam, lParam);	\
	}

#endif // __M_GUI_H
