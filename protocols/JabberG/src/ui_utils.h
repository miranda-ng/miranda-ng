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

#ifndef __jabber_ui_utils_h__
#define __jabber_ui_utils_h__

#pragma warning(disable:4355)

#ifndef LPLVCOLUMN
typedef struct tagNMLVSCROLL
{
	NMHDR   hdr;
	int     dx;
	int     dy;
} NMLVSCROLL;
typedef struct tagLVG
{
	UINT    cbSize;
	UINT    mask;
	LPWSTR  pszHeader;
	int     cchHeader;
	LPWSTR  pszFooter;
	int     cchFooter;
	int     iGroupId;
	UINT    stateMask;
	UINT    state;
	UINT    uAlign;
} LVGROUP, *PLVGROUP;
typedef struct tagLVGROUPMETRICS
{
	UINT cbSize;
	UINT mask;
	UINT Left;
	UINT Top;
	UINT Right;
	UINT Bottom;
	COLORREF crLeft;
	COLORREF crTop;
	COLORREF crRight;
	COLORREF crBottom;
	COLORREF crHeader;
	COLORREF crFooter;
} LVGROUPMETRICS, *PLVGROUPMETRICS;
typedef struct tagLVTILEVIEWINFO
{
	UINT cbSize;
	DWORD dwMask;
	DWORD dwFlags;
	SIZE sizeTile;
	int cLines;
	RECT rcLabelMargin;
} LVTILEVIEWINFO, *PLVTILEVIEWINFO;
typedef struct tagLVTILEINFO
{
	UINT cbSize;
	int iItem;
	UINT cColumns;
	PUINT puColumns;
} LVTILEINFO, *PLVTILEINFO;
typedef struct
{
	UINT cbSize;
	DWORD dwFlags;
	int iItem;
	DWORD dwReserved;
} LVINSERTMARK, * LPLVINSERTMARK;
typedef int (CALLBACK *PFNLVGROUPCOMPARE)(int, int, void *);
typedef struct tagLVINSERTGROUPSORTED
{
	PFNLVGROUPCOMPARE pfnGroupCompare;
	void *pvData;
	LVGROUP lvGroup;
} LVINSERTGROUPSORTED, *PLVINSERTGROUPSORTED;
typedef struct tagLVSETINFOTIP
{
	UINT cbSize;
	DWORD dwFlags;
	LPWSTR pszText;
	int iItem;
	int iSubItem;
} LVSETINFOTIP, *PLVSETINFOTIP;
#define LPLVCOLUMN LPLVCOLUMNA
#define LPLVITEM LPLVITEMA
#define LVN_BEGINSCROLL (LVN_FIRST-80)
#define LVN_ENDSCROLL (LVN_FIRST-81)
#define LVN_HOTTRACK (LVN_FIRST-21)
#define LVN_MARQUEEBEGIN (LVN_FIRST-56)
#define LVM_MAPINDEXTOID (LVM_FIRST + 180)
#define LVGF_HEADER 0x00000001
#define LVGF_GROUPID 0x00000010
#define ListView_MapIndexToID(hwnd, index) \
	(UINT)SendMessage((hwnd), LVM_MAPINDEXTOID, (WPARAM)index, 0)
#define TreeView_GetLineColor(hwnd) \
	(COLORREF)SendMessage((hwnd), TVM_GETLINECOLOR, 0, 0)
#define TreeView_SetLineColor(hwnd, clr) \
	(COLORREF)SendMessage((hwnd), TVM_SETLINECOLOR, 0, (LPARAM)(clr))
#endif

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

class CDataLink
{
protected:
	BYTE m_type;
	bool m_bSigned;

public:
	CDataLink(BYTE type, bool bSigned): m_type(type), m_bSigned(bSigned) {}
	virtual ~CDataLink() {}

	__inline BYTE GetDataType() { return m_type; }
	__inline BYTE GetDataSigned() { return m_bSigned; }

	virtual DWORD LoadUnsigned() = 0;
	virtual int LoadSigned() = 0;
	virtual void SaveInt(DWORD value) = 0;

	virtual TCHAR *LoadText() = 0;
	virtual void SaveText(TCHAR *value) = 0;
};

class CDbLink: public CDataLink
{
	char *m_szModule;
	char *m_szSetting;
	bool m_bSigned;

	DWORD m_iDefault;
	TCHAR *m_szDefault;

	DBVARIANT dbv;

public:
	CDbLink(const char *szModule, const char *szSetting, BYTE type, DWORD iValue, bool bSigned = false);
	CDbLink(const char *szModule, const char *szSetting, BYTE type, TCHAR *szValue);
	~CDbLink();

	DWORD LoadUnsigned();
	int LoadSigned();
	void SaveInt(DWORD value);

	TCHAR *LoadText();
	void SaveText(TCHAR *value);
};

template<class T>
class CMOptionLink: public CDataLink
{
private:
	CMOption<T> *m_option;

public:
	CMOptionLink(CMOption<T> &option): CDataLink(CMDBTraits<sizeof(T)>::DBTypeId, CMIntTraits<T>::IsSigned()), m_option(&option) {}

	DWORD LoadUnsigned() { return (DWORD)(T)*m_option; }
	int LoadSigned() { return (int)(T)*m_option; }
	void SaveInt(DWORD value) { *m_option = (T)value; }

	TCHAR *LoadText() { return NULL; }
	void SaveText(TCHAR*) {}
};

/////////////////////////////////////////////////////////////////////////////////////////
// CDlgBase - base dialog class

class CDlgBase
{
	friend class CCtrlBase;
	friend class CCtrlData;

public:
	CDlgBase(int idDialog, HWND hwndParent);
	virtual ~CDlgBase();

	// general utilities
	void Create();
	void Show(int nCmdShow = SW_SHOW);
	int DoModal();

	__inline HWND GetHwnd() const { return m_hwnd; }
	__inline bool IsInitialized() const { return m_initialized; }
	__inline void Close() { SendMessage(m_hwnd, WM_CLOSE, 0, 0); }
	__inline const MSG *ActiveMessage() const { return &m_msg; }

	// dynamic creation support (mainly to avoid leaks in options)
	struct CreateParam
	{
		CDlgBase *(*create)(void *param);
		void *param;
	};
	static INT_PTR CALLBACK DynamicDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (msg == WM_INITDIALOG)
		{
			CreateParam *param = (CreateParam *)lParam;
			CDlgBase *wnd = param->create(param->param);
			SetWindowLongPtr(hwnd, DWLP_DLGPROC, (LONG_PTR)GlobalDlgProc);
			return GlobalDlgProc(hwnd, msg, wParam, (LPARAM)wnd);
		}

		return FALSE;
	}

	LRESULT m_lresult;

protected:
	HWND    m_hwnd;
	HWND    m_hwndParent;
	int     m_idDialog;
	MSG     m_msg;
	bool    m_isModal;
	bool    m_initialized;
	bool    m_forceResizable;

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

class CCtrlBase
{
	friend class CDlgBase;

public:
	CCtrlBase(CDlgBase *wnd, int idCtrl);
	virtual ~CCtrlBase() { Unsubclass(); }

	__inline HWND GetHwnd() const { return m_hwnd; }
	__inline CDlgBase *GetParent() { return m_parentWnd; }

	void Enable(int bIsEnable = true);
	__inline void Disable() { Enable(false); }
	BOOL Enabled(void) const;

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

	virtual void OnApply() {}
	virtual void OnReset() {}

	static int cmp(const CCtrlBase *c1, const CCtrlBase *c2)
	{
		if (c1->m_idCtrl < c2->m_idCtrl) return -1;
		if (c1->m_idCtrl > c2->m_idCtrl) return +1;
		return 0;
	}

protected:
	HWND m_hwnd;
	int m_idCtrl;
	CCtrlBase* m_next;
	CDlgBase* m_parentWnd;

	virtual LRESULT CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	void Subclass();
	void Unsubclass();

private:
	WNDPROC m_wndproc;
	static LRESULT CALLBACK GlobalSubclassWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (CCtrlBase *ctrl = (CCtrlBase*)GetWindowLongPtr(hwnd, GWLP_USERDATA))
			if (ctrl)
				return ctrl->CustomWndProc(msg, wParam, lParam);

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlButton

class CCtrlButton : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlButton(CDlgBase* dlg, int ctrlId);

	virtual BOOL OnCommand(HWND hwndCtrl, WORD idCtrl, WORD idCode);

	CCallback<CCtrlButton> OnClick;
};

class CCtrlMButton : public CCtrlButton
{
	typedef CCtrlButton CSuper;

public:
	CCtrlMButton(CDlgBase* dlg, int ctrlId, HICON hIcon, const char* tooltip);
	CCtrlMButton(CDlgBase* dlg, int ctrlId, int iCoreIcon, const char* tooltip);
	~CCtrlMButton();

	void MakeFlat();
	void MakePush();

	virtual void OnInit();

protected:
	char m_flags;
	HICON m_hIcon;
	const char* m_toolTip;
};

class CCtrlHyperlink : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlHyperlink(CDlgBase* dlg, int ctrlId, const char* url);

	virtual BOOL OnCommand(HWND hwndCtrl, WORD idCtrl, WORD idCode);

protected:
	const char* m_url;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlClc
class CCtrlClc: public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlClc(CDlgBase* dlg, int ctrlId);

	void AddContact(MCONTACT hContact);
	void AddGroup(HANDLE hGroup);
	void AutoRebuild();
	void DeleteItem(HANDLE hItem);
	void EditLabel(HANDLE hItem);
	void EndEditLabel(bool save);
	void EnsureVisible(HANDLE hItem, bool partialOk);
	void Expand(HANDLE hItem, DWORD flags);
	HANDLE FindContact(MCONTACT hContact);
	HANDLE FindGroup(HANDLE hGroup);
	COLORREF GetBkColor();
	bool GetCheck(HANDLE hItem);
	int GetCount();
	HWND GetEditControl();
	DWORD GetExpand(HANDLE hItem);
	int GetExtraColumns();
	BYTE GetExtraImage(HANDLE hItem, int iColumn);
	HIMAGELIST GetExtraImageList();
	HFONT GetFont(int iFontId);
	HANDLE GetSelection();
	HANDLE HitTest(int x, int y, DWORD *hitTest);
	void SelectItem(HANDLE hItem);
	void SetBkBitmap(DWORD mode, HBITMAP hBitmap);
	void SetBkColor(COLORREF clBack);
	void SetCheck(HANDLE hItem, bool check);
	void SetExtraColumns(int iColumns);
	void SetExtraImage(HANDLE hItem, int iColumn, int iImage);
	void SetExtraImageList(HIMAGELIST hImgList);
	void SetFont(int iFontId, HANDLE hFont, bool bRedraw);
	void SetIndent(int iIndent);
	void SetItemText(HANDLE hItem, char *szText);
	void SetHideEmptyGroups(bool state);
	void SetGreyoutFlags(DWORD flags);
	bool GetHideOfflineRoot();
	void SetHideOfflineRoot(bool state);
	void SetUseGroups(bool state);
	void SetOfflineModes(DWORD modes);
	DWORD GetExStyle();
	void SetExStyle(DWORD exStyle);
	int GetLefrMargin();
	void SetLeftMargin(int iMargin);
	HANDLE AddInfoItem(CLCINFOITEM *cii);
	int GetItemType(HANDLE hItem);
	HANDLE GetNextItem(HANDLE hItem, DWORD flags);
	COLORREF GetTextColot(int iFontId);
	void SetTextColor(int iFontId, COLORREF clText);

	struct TEventInfo
	{
		CCtrlClc		*ctrl;
		NMCLISTCONTROL	*info;
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
	BOOL OnNotify(int idCtrl, NMHDR *pnmh);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlData - data access controls base class

class CCtrlData : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlData(CDlgBase* dlg, int ctrlId);

	virtual ~CCtrlData()
	{
		if (m_dbLink) delete m_dbLink;
	}

	__inline bool IsChanged() const { return m_changed; }

	void CreateDbLink(const char* szModuleName, const char* szSetting, BYTE type, DWORD iValue, bool bSigned = false);
	void CreateDbLink(const char* szModuleName, const char* szSetting, TCHAR* szValue);
	void CreateDbLink(CDataLink *link) { m_dbLink = link; }

	virtual void OnInit();

	// Events
	CCallback<CCtrlData> OnChange;

protected:
	CDataLink *m_dbLink;
	bool m_changed;

	void NotifyChange();

	__inline BYTE GetDataType() { return m_dbLink ? m_dbLink->GetDataType() : DBVT_DELETED; }
	__inline bool GetDataSigned() { return m_dbLink ? m_dbLink->GetDataSigned() ? true : false : false; }
	__inline DWORD LoadUnsigned() { return m_dbLink ? m_dbLink->LoadUnsigned() : 0; }
	__inline int LoadSigned() { return m_dbLink ? m_dbLink->LoadSigned() : 0; }
	__inline void SaveInt(DWORD value) { if (m_dbLink) m_dbLink->SaveInt(value); }
	__inline const TCHAR *LoadText() { return m_dbLink ? m_dbLink->LoadText() : _T(""); }
	__inline void SaveText(TCHAR *value) { if (m_dbLink) m_dbLink->SaveText(value); }
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlCheck

class CCtrlCheck : public CCtrlData
{
	typedef CCtrlData CSuper;

public:
	CCtrlCheck(CDlgBase* dlg, int ctrlId);
	virtual BOOL OnCommand(HWND /*hwndCtrl*/, WORD /*idCtrl*/, WORD /*idCode*/) { NotifyChange(); return TRUE; }
	virtual void OnInit()
	{
		CSuper::OnInit();
		OnReset();
	}
	virtual void OnApply()
	{
		SaveInt(GetState());
	}
	virtual void OnReset()
	{
		SetState(LoadUnsigned());
	}

	int GetState();
	void SetState(int state);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlEdit

class CCtrlEdit : public CCtrlData
{
	typedef CCtrlData CSuper;

public:
	CCtrlEdit(CDlgBase* dlg, int ctrlId);
	virtual BOOL OnCommand(HWND /*hwndCtrl*/, WORD /*idCtrl*/, WORD idCode)
	{
		if (idCode == EN_CHANGE)
			NotifyChange();
		return TRUE;
	}
	virtual void OnInit()
	{
		CSuper::OnInit();
		OnReset();
	}
	virtual void OnApply()
	{
		if (GetDataType() == DBVT_TCHAR)
		{
			int len = GetWindowTextLength(m_hwnd) + 1;
			TCHAR *buf = (TCHAR *)_alloca(sizeof(TCHAR) * len);
			GetWindowText(m_hwnd, buf, len);
			SaveText(buf);
		}
		else if (GetDataType() != DBVT_DELETED)
		{
			SaveInt(GetInt());
		}
	}
	virtual void OnReset()
	{
		if (GetDataType() == DBVT_TCHAR)
			SetText(LoadText());
		else if (GetDataType() != DBVT_DELETED)
			SetInt(GetDataSigned() ? LoadSigned() : LoadUnsigned());
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlListBox

class CCtrlListBox : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlListBox(CDlgBase* dlg, int ctrlId);

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

class CCtrlCombo : public CCtrlData
{
	typedef CCtrlData CSuper;

public:
	CCtrlCombo(CDlgBase* dlg, int ctrlId);

	virtual BOOL OnCommand(HWND /*hwndCtrl*/, WORD /*idCtrl*/, WORD idCode)
	{
		switch (idCode)
		{
			case CBN_CLOSEUP:  OnCloseup(this);  break;
			case CBN_DROPDOWN: OnDropdown(this); break;

			case CBN_EDITCHANGE:
			case CBN_EDITUPDATE:
			case CBN_SELCHANGE:
			case CBN_SELENDOK:
				NotifyChange();
				break;
		}
		return TRUE;
	}

	virtual void OnInit()
	{
		CSuper::OnInit();
		OnReset();
	}
	virtual void OnApply()
	{
		if (GetDataType() == DBVT_TCHAR)
		{
			int len = GetWindowTextLength(m_hwnd) + 1;
			TCHAR *buf = (TCHAR *)_alloca(sizeof(TCHAR) * len);
			GetWindowText(m_hwnd, buf, len);
			SaveText(buf);
		}
		else if (GetDataType() != DBVT_DELETED)
		{
			SaveInt(GetInt());
		}
	}
	virtual void OnReset()
	{
		if (GetDataType() == DBVT_TCHAR)
			SetText(LoadText());
		else if (GetDataType() != DBVT_DELETED)
			SetInt(LoadUnsigned());
	}

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

class CCtrlListView : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlListView(CDlgBase* dlg, int ctrlId);

	// Classic LV interface
	DWORD ApproximateViewRect(int cx, int cy, int iCount);
	void Arrange(UINT code);
	void CancelEditLabel();
	HIMAGELIST CreateDragImage(int iItem, LPPOINT lpptUpLeft);
	void DeleteAllItems();
	void DeleteColumn(int iCol);
	void DeleteItem(int iItem);
	HWND EditLabel(int iItem);
	int EnableGroupView(BOOL fEnable);
	BOOL EnsureVisible(int i, BOOL fPartialOK);
	int FindItem(int iStart, const LVFINDINFO *plvfi);
	COLORREF GetBkColor();
	void GetBkImage(LPLVBKIMAGE plvbki);
	UINT GetCallbackMask();
	BOOL GetCheckState(UINT iIndex);
	void GetColumn(int iCol, LPLVCOLUMN pcol);
	void GetColumnOrderArray(int iCount, int *lpiArray);
	int GetColumnWidth(int iCol);
	int GetCountPerPage();
	HWND GetEditControl();
	//void GetEmptyText(PWSTR pszText, UINT cchText);
	DWORD GetExtendedListViewStyle();
	INT GetFocusedGroup();
	//void GetFooterInfo(LVFOOTERINFO *plvfi);
	//void GetFooterItem(UINT iItem, LVFOOTERITEM *pfi);
	//void GetFooterItemRect(UINT iItem,  RECT *prc);
	//void GetFooterRect(RECT *prc);
	int GetGroupCount();
	//HIMAGELIST GetGroupHeaderImageList();
	void GetGroupInfo(int iGroupId, PLVGROUP pgrp);
	void GetGroupInfoByIndex(int iIndex, PLVGROUP pgrp);
	void GetGroupMetrics(LVGROUPMETRICS *pGroupMetrics);
	//BOOL GetGroupRect(int iGroupId, RECT *prc);
	UINT GetGroupState(UINT dwGroupId, UINT dwMask);
	HWND GetHeader();
	HCURSOR GetHotCursor();
	INT GetHotItem();
	DWORD GetHoverTime();
	HIMAGELIST GetImageList(int iImageList);
	BOOL GetInsertMark(LVINSERTMARK *plvim);
	COLORREF GetInsertMarkColor();
	int GetInsertMarkRect(LPRECT prc);
	BOOL GetISearchString(LPSTR lpsz);
	void GetItem(LPLVITEM pitem);
	int GetItemCount();
	//void GetItemIndexRect(LVITEMINDEX *plvii, LONG iSubItem, LONG code, LPRECT prc);
	void GetItemPosition(int i, POINT *ppt);
	void GetItemRect(int i, RECT *prc, int code);
	DWORD GetItemSpacing(BOOL fSmall);
	UINT GetItemState(int i, UINT mask);
	void GetItemText(int iItem, int iSubItem, LPTSTR pszText, int cchTextMax);
	int GetNextItem(int iStart, UINT flags);
	//BOOL GetNextItemIndex(LVITEMINDEX *plvii, LPARAM flags);
	BOOL GetNumberOfWorkAreas(LPUINT lpuWorkAreas);
	BOOL GetOrigin(LPPOINT lpptOrg);
	COLORREF GetOutlineColor();
	UINT GetSelectedColumn();
	UINT GetSelectedCount();
	INT GetSelectionMark();
	int GetStringWidth(LPCSTR psz);
	BOOL GetSubItemRect(int iItem, int iSubItem, int code, LPRECT lpRect);
	COLORREF GetTextBkColor();
	COLORREF GetTextColor();
	void GetTileInfo(PLVTILEINFO plvtinfo);
	void GetTileViewInfo(PLVTILEVIEWINFO plvtvinfo);
	HWND GetToolTips();
	int GetTopIndex();
	BOOL GetUnicodeFormat();
	DWORD GetView();
	BOOL GetViewRect(RECT *prc);
	void GetWorkAreas(INT nWorkAreas, LPRECT lprc);
	BOOL HasGroup(int dwGroupId);
	int HitTest(LPLVHITTESTINFO pinfo);
	int HitTestEx(LPLVHITTESTINFO pinfo);
	int InsertColumn(int iCol, const LPLVCOLUMN pcol);
	int InsertGroup(int index, PLVGROUP pgrp);
	void InsertGroupSorted(PLVINSERTGROUPSORTED structInsert);
	int InsertItem(const LPLVITEM pitem);
	BOOL InsertMarkHitTest(LPPOINT point, LVINSERTMARK *plvim);
	BOOL IsGroupViewEnabled();
	UINT IsItemVisible(UINT index);
	UINT MapIDToIndex(UINT id);
	UINT MapIndexToID(UINT index);
	BOOL RedrawItems(int iFirst, int iLast);
	void RemoveAllGroups();
	int RemoveGroup(int iGroupId);
	BOOL Scroll(int dx, int dy);
	BOOL SetBkColor(COLORREF clrBk);
	BOOL SetBkImage(LPLVBKIMAGE plvbki);
	BOOL SetCallbackMask(UINT mask);
	void SetCheckState(UINT iIndex, BOOL fCheck);
	BOOL SetColumn(int iCol, LPLVCOLUMN pcol);
	BOOL SetColumnOrderArray(int iCount, int *lpiArray);
	BOOL SetColumnWidth(int iCol, int cx);
	void SetExtendedListViewStyle(DWORD dwExStyle);
	void SetExtendedListViewStyleEx(DWORD dwExMask, DWORD dwExStyle);
	//HIMAGELIST SetGroupHeaderImageList(HIMAGELIST himl);
	int SetGroupInfo(int iGroupId, PLVGROUP pgrp);
	void SetGroupMetrics(PLVGROUPMETRICS pGroupMetrics);
	void SetGroupState(UINT dwGroupId, UINT dwMask, UINT dwState);
	HCURSOR SetHotCursor(HCURSOR hCursor);
	INT SetHotItem(INT iIndex);
	void SetHoverTime(DWORD dwHoverTime);
	DWORD SetIconSpacing(int cx, int cy);
	HIMAGELIST SetImageList(HIMAGELIST himl, int iImageList);
	BOOL SetInfoTip(PLVSETINFOTIP plvSetInfoTip);
	BOOL SetInsertMark(LVINSERTMARK *plvim);
	COLORREF SetInsertMarkColor(COLORREF color);
	BOOL SetItem(const LPLVITEM pitem);
	void SetItemCount(int cItems);
	void SetItemCountEx(int cItems, DWORD dwFlags);
	//HRESULT SetItemIndexState(LVITEMINDEX *plvii, UINT data, UINT mask);
	BOOL SetItemPosition(int i, int x, int y);
	void SetItemPosition32(int iItem, int x, int y);
	void SetItemState(int i, UINT state, UINT mask);
	void SetItemText(int i, int iSubItem, TCHAR *pszText);
	COLORREF SetOutlineColor(COLORREF color);
	void SetSelectedColumn(int iCol);
	INT SetSelectionMark(INT iIndex);
	BOOL SetTextBkColor(COLORREF clrText);
	BOOL SetTextColor(COLORREF clrText);
	BOOL SetTileInfo(PLVTILEINFO plvtinfo);
	BOOL SetTileViewInfo(PLVTILEVIEWINFO plvtvinfo);
	HWND SetToolTips(HWND ToolTip);
	BOOL SetUnicodeFormat(BOOL fUnicode);
	int SetView(DWORD iView);
	void SetWorkAreas(INT nWorkAreas, LPRECT lprc);
	int SortGroups(PFNLVGROUPCOMPARE pfnGroupCompare, LPVOID plv);
	BOOL SortItems(PFNLVCOMPARE pfnCompare, LPARAM lParamSort);
	BOOL SortItemsEx(PFNLVCOMPARE pfnCompare, LPARAM lParamSort);
	INT SubItemHitTest(LPLVHITTESTINFO pInfo);
	INT SubItemHitTestEx(LPLVHITTESTINFO plvhti);
	BOOL Update(int iItem);

	// Additional APIs
	HIMAGELIST CreateImageList(int iImageList);
	void AddColumn(int iSubItem, TCHAR *name, int cx);
	void AddGroup(int iGroupId, TCHAR *name);
	int AddItem(TCHAR *text, int iIcon, LPARAM lParam = 0, int iGroupId = -1);
	void SetItem(int iItem, int iSubItem, TCHAR *text, int iIcon = -1);
	LPARAM GetItemData(int iItem);

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
	//CCallback<TEventInfo> OnColumnDropdown;
	//CCallback<TEventInfo> OnColumnOverflowClick;
	CCallback<TEventInfo> OnDeleteAllItems;
	CCallback<TEventInfo> OnDeleteItem;
	CCallback<TEventInfo> OnDoubleClick;
	CCallback<TEventInfo> OnEndLabelEdit;
	CCallback<TEventInfo> OnEndScroll;
	CCallback<TEventInfo> OnGetDispInfo;
	//CCallback<TEventInfo> OnGetEmptyMarkup;
	CCallback<TEventInfo> OnGetInfoTip;
	CCallback<TEventInfo> OnHotTrack;
	CCallback<TEventInfo> OnIncrementalSearch;
	CCallback<TEventInfo> OnInsertItem;
	CCallback<TEventInfo> OnItemActivate;
	CCallback<TEventInfo> OnItemChanged;
	CCallback<TEventInfo> OnItemChanging;
	CCallback<TEventInfo> OnKeyDown;
	//CCallback<TEventInfo> OnLinkClick;
	CCallback<TEventInfo> OnMarqueeBegin;
	CCallback<TEventInfo> OnSetDispInfo;

protected:
	BOOL OnNotify(int idCtrl, NMHDR *pnmh);
};

struct CFilterData;
class CCtrlFilterListView : public CCtrlListView
{
	typedef CCtrlListView CSuper;

public:
	CCtrlFilterListView(CDlgBase* dlg, int ctrlId, bool trackFilter, bool keepHiglight);
	~CCtrlFilterListView();

	TCHAR *GetFilterText();
	CCallback<CCtrlFilterListView> OnFilterChanged;

protected:
	CFilterData *fdat;
	bool m_trackFilter;
	bool m_keepHiglight;

	void OnInit();
	LRESULT CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam);
	void FilterHighlight(TCHAR *filter);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlTreeView

class CCtrlTreeView : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlTreeView(CDlgBase* dlg, int ctrlId);

	// Classic TV interface
	HIMAGELIST CreateDragImage(HTREEITEM hItem);
	void DeleteAllItems();
	void DeleteItem(HTREEITEM hItem);
	HWND EditLabel(HTREEITEM hItem);
	void EndEditLabelNow(BOOL cancel);
	void EnsureVisible(HTREEITEM hItem);
	void Expand(HTREEITEM hItem, DWORD flag);
	COLORREF GetBkColor();
	DWORD GetCheckState(HTREEITEM hItem);
	HTREEITEM GetChild(HTREEITEM hItem);
	int GetCount();
	HTREEITEM GetDropHilight();
	HWND GetEditControl();
	HTREEITEM GetFirstVisible();
	HIMAGELIST GetImageList(int iImage);
	int GetIndent();
	COLORREF GetInsertMarkColor();
	void GetItem(TVITEMEX *tvi);
	int GetItemHeight();
	void GetItemRect(HTREEITEM hItem, RECT *rcItem, BOOL fItemRect);
	DWORD GetItemState(HTREEITEM hItem, DWORD stateMask);
	HTREEITEM GetLastVisible();
	COLORREF GetLineColor();
	HTREEITEM GetNextItem(HTREEITEM hItem, DWORD flag);
	HTREEITEM GetNextSibling(HTREEITEM hItem);
	HTREEITEM GetNextVisible(HTREEITEM hItem);
	HTREEITEM GetParent(HTREEITEM hItem);
	HTREEITEM GetPrevSibling(HTREEITEM hItem);
	HTREEITEM GetPrevVisible(HTREEITEM hItem);
	HTREEITEM GetRoot();
	DWORD GetScrollTime();
	HTREEITEM GetSelection();
	COLORREF GetTextColor();
	HWND GetToolTips();
	BOOL GetUnicodeFormat();
	unsigned GetVisibleCount();
	HTREEITEM HitTest(TVHITTESTINFO *hti);
	HTREEITEM InsertItem(TVINSERTSTRUCT *tvis);
	//HTREEITEM MapAccIDToHTREEITEM(UINT id);
	//UINT MapHTREEITEMtoAccID(HTREEITEM hItem);
	void Select(HTREEITEM hItem, DWORD flag);
	void SelectDropTarget(HTREEITEM hItem);
	void SelectItem(HTREEITEM hItem);
	void SelectSetFirstVisible(HTREEITEM hItem);
	COLORREF SetBkColor(COLORREF clBack);
	void SetCheckState(HTREEITEM hItem, DWORD state);
	void SetImageList(HIMAGELIST hIml, int iImage);
	void SetIndent(int iIndent);
	void SetInsertMark(HTREEITEM hItem, BOOL fAfter);
	COLORREF SetInsertMarkColor(COLORREF clMark);
	void SetItem(TVITEMEX *tvi);
	void SetItemHeight(short cyItem);
	void SetItemState(HTREEITEM hItem, DWORD state, DWORD stateMask);
	COLORREF SetLineColor(COLORREF clLine);
	void SetScrollTime(UINT uMaxScrollTime);
	COLORREF SetTextColor(COLORREF clText);
	HWND SetToolTips(HWND hwndToolTips);
	BOOL SetUnicodeFormat(BOOL fUnicode);
	void SortChildren(HTREEITEM hItem, BOOL fRecurse);
	void SortChildrenCB(TVSORTCB *cb, BOOL fRecurse);

	// Additional stuff
	void TranslateItem(HTREEITEM hItem);
	void TranslateTree();
	HTREEITEM FindNamedItem(HTREEITEM hItem, const TCHAR *name);
	void GetItem(HTREEITEM hItem, TVITEMEX *tvi);
	void GetItem(HTREEITEM hItem, TVITEMEX *tvi, TCHAR *szText, int iTextLength);

	// Events
	struct TEventInfo {
		CCtrlTreeView *treeviewctrl;
		union {
			NMHDR			*nmhdr;
			NMTREEVIEW		*nmtv;
			NMTVDISPINFO	*nmtvdi;
			NMTVGETINFOTIP	*nmtvit;
			NMTVKEYDOWN		*nmtvkey;
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
	BOOL OnNotify(int idCtrl, NMHDR *pnmh);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlTreeView

class CCtrlPages: public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlPages(CDlgBase* dlg, int ctrlId);

	void AddPage(TCHAR *ptszName, HICON hIcon, CCallback<void> onCreate = CCallback<void>(), void *param = NULL);
	void AttachDialog(int iPage, CDlgBase *pDlg);

	void ActivatePage(int iPage);


protected:
	BOOL OnNotify(int idCtrl, NMHDR *pnmh);
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
class CCtrlCustom : public CCtrlBase
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

class CProtoIntDlgBase : public CDlgBase
{
	typedef CDlgBase CSuper;

public:
	__inline CProtoIntDlgBase(PROTO_INTERFACE *proto, int idDialog, HWND parent, bool show_label=true) :
		CDlgBase(idDialog, parent),
		m_proto_interface(proto),
		m_show_label(show_label),
		m_hwndStatus(NULL)
	{
	}

	__inline void CreateLink(CCtrlData& ctrl, char *szSetting, BYTE type, DWORD iValue, bool bSigned = false)
	{
		ctrl.CreateDbLink(m_proto_interface->m_szModuleName, szSetting, type, iValue, bSigned);
	}
	__inline void CreateLink(CCtrlData& ctrl, const char *szSetting, TCHAR *szValue)
	{
		ctrl.CreateDbLink(m_proto_interface->m_szModuleName, szSetting, szValue);
	}

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

	virtual void OnProtoRefresh(WPARAM, LPARAM) {}
	virtual void OnProtoActivate(WPARAM, LPARAM) {}
	virtual void OnProtoCheckOnline(WPARAM, LPARAM) {}

private:
	void UpdateProtoTitle(const TCHAR *szText = NULL);
	void UpdateStatusBar();
};

template<typename TProto>
class CProtoDlgBase : public CProtoIntDlgBase
{
	typedef CProtoIntDlgBase CSuper;

public:
	__inline CProtoDlgBase<TProto>(TProto *proto, int idDialog, HWND parent, bool show_label=true) :
		CProtoIntDlgBase(proto, idDialog, parent, show_label),
		m_proto(proto)
	{
	}

	__inline TProto *GetProto() { return m_proto; }

protected:
	TProto* m_proto;

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
			case WM_INITDIALOG:
				m_proto->WindowSubscribe(m_hwnd);
				break;
			case WM_DESTROY:
				WindowFreeIcon(m_hwnd);
				m_proto->WindowUnsubscribe(m_hwnd);
				break;
		}

		return CSuper::DlgProc(msg, wParam, lParam);
	}
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

/////////////////////////////////////////////////////////////////////////////////////////
// Misc utitlities
int UIEmulateBtnClick(HWND hwndDlg, UINT idcButton);
void UIShowControls(HWND hwndDlg, int *idList, int nCmdShow);

#endif // __jabber_ui_utils_h__
