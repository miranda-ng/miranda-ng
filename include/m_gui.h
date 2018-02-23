/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007-09  Maxim Mluhov
Copyright (c) 2007-09  Victor Pavlychko
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

#pragma once

#ifndef __M_GUI_H
#define __M_GUI_H

#include <CommCtrl.h>

#include <m_protoint.h>
#include <m_clc.h>

#pragma warning(disable:4355 4251 4481)

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
	static __forceinline DBType Get(char *szModule, char *szSetting, DBType value)
	{
		return db_get_b(0, szModule, szSetting, value);
	}
	static __forceinline void Set(char *szModule, char *szSetting, DBType value)
	{
		db_set_b(0, szModule, szSetting, value);
	}
};

template<>
struct CMDBTraits<2>
{
	typedef WORD DBType;
	enum { DBTypeId = DBVT_WORD };
	static __forceinline DBType Get(char *szModule, char *szSetting, DBType value)
	{
		return db_get_w(0, szModule, szSetting, value);
	}
	static __forceinline void Set(char *szModule, char *szSetting, DBType value)
	{
		db_set_w(0, szModule, szSetting, value);
	}
};

template<>
struct CMDBTraits<4>
{
	typedef DWORD DBType;
	enum { DBTypeId = DBVT_DWORD };
	static __forceinline DBType Get(char *szModule, char *szSetting, DBType value)
	{
		return db_get_dw(0, szModule, szSetting, value);
	}
	static __forceinline void Set(char *szModule, char *szSetting, DBType value)
	{
		db_set_dw(0, szModule, szSetting, value);
	}
};

template<>
struct CMDBTraits<8>
{
	typedef DWORD DBType;
	enum { DBTypeId = DBVT_DWORD };
	static __forceinline DBType Get(char *szModule, char *szSetting, DBType value)
	{
		return db_get_dw(0, szModule, szSetting, value);
	}
	static __forceinline void Set(char *szModule, char *szSetting, DBType value)
	{
		db_set_dw(0, szModule, szSetting, value);
	}
};

class CMOptionBase
{
public:
	__forceinline const char* GetDBModuleName() const { return m_szModuleName; }
	__forceinline const char* GetDBSettingName() const { return m_szSetting; }

protected:
	__forceinline CMOptionBase(PROTO_INTERFACE *proto, char *szSetting) :
		m_szModuleName(proto->m_szModuleName), m_szSetting(szSetting)
	{}

	__forceinline CMOptionBase(char *module, char *szSetting) :
		m_szModuleName(module), m_szSetting(szSetting)
	{}

	char *m_szModuleName;
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

	__forceinline CMOption(char *szModule, char *szSetting, Type defValue) :
		CMOptionBase(szModule, szSetting), m_default(defValue)
	{}

	__forceinline Type Default() const
	{
		return m_default;
	}

	__forceinline operator Type()
	{
		return (Type)CMDBTraits<sizeof(Type)>::Get(m_szModuleName, m_szSetting, m_default);
	}
	
	__forceinline Type operator= (Type value)
	{
		CMDBTraits<sizeof(Type)>::Set(m_szModuleName, m_szSetting, (CMDBTraits<sizeof(Type)>::DBType)value);
		return value;
	}

private:
	Type m_default;

	CMOption(const CMOption &) : CMOptionBase(NULL, NULL, DBVT_DELETED) {}
	void operator= (const CMOption &) {}
};

#ifdef M_SYSTEM_CPP_H__

template<>
class CMOption<char*> : public CMOptionBase
{
public:
	
	typedef char Type;

	__forceinline CMOption(PROTO_INTERFACE *proto, char *szSetting, const Type *defValue = nullptr) :
		CMOptionBase(proto, szSetting), m_default(defValue)
	{}

	__forceinline CMOption(char *szModule, char *szSetting, const Type *defValue = nullptr) :
		CMOptionBase(szModule, szSetting), m_default(defValue)
	{}

	__forceinline const Type* Default() const
	{
		return m_default;
	}

	__forceinline operator Type*()
	{
		m_value = db_get_sa(0, m_szModuleName, m_szSetting);
		if (!m_value) m_value = mir_strdup(m_default);
		return m_value;
	}

	__forceinline Type* operator= (Type *value)
	{
		db_set_s(0, m_szModuleName, m_szSetting, value);
		return value;
	}

private:
	const Type *m_default;
	mir_ptr<Type> m_value;

	CMOption(const CMOption &) : CMOptionBase((char*)nullptr, nullptr) {}
	void operator= (const CMOption &) {}
};

template<>
class CMOption<wchar_t*> : public CMOptionBase
{
public:

	typedef wchar_t Type;

	__forceinline CMOption(PROTO_INTERFACE *proto, char *szSetting, const Type *defValue = nullptr) :
		CMOptionBase(proto, szSetting), m_default(defValue)
	{}

	__forceinline CMOption(char *szModule, char *szSetting, const Type *defValue = nullptr) :
		CMOptionBase(szModule, szSetting), m_default(defValue)
	{}

	__forceinline const Type* Default() const
	{
		return m_default;
	}

	__forceinline operator Type*()
	{
		m_value = db_get_wsa(0, m_szModuleName, m_szSetting);
		if (!m_value) m_value = mir_wstrdup(m_default);
		return m_value;
	}

	__forceinline const Type* operator= (const Type *value)
	{
		db_set_ws(0, m_szModuleName, m_szSetting, value);
		return value;
	}

private:
	const Type *m_default;
	mir_ptr<Type> m_value;

	CMOption(const CMOption &) : CMOptionBase((char*)nullptr, nullptr) {}
	void operator= (const CMOption &) {}
};

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

	virtual wchar_t* LoadText() = 0;
	virtual void   SaveText(wchar_t *value) = 0;
};

class MIR_CORE_EXPORT CDbLink : public CDataLink
{
	char *m_szModule;
	char *m_szSetting;
	bool m_bSigned;

	DWORD m_iDefault;
	wchar_t *m_szDefault;

	DBVARIANT dbv;

public:
	CDbLink(const char *szModule, const char *szSetting, BYTE type, DWORD iValue);
	CDbLink(const char *szModule, const char *szSetting, BYTE type, wchar_t *szValue);
	~CDbLink();

	DWORD LoadInt();
	void  SaveInt(DWORD value);

	wchar_t* LoadText();
	void   SaveText(wchar_t *value);
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

	__forceinline wchar_t* LoadText() { return NULL; }
	__forceinline void   SaveText(wchar_t*) {}
};

template<>
class CMOptionLink<wchar_t*> : public CDataLink
{
private:
	typedef wchar_t *T;
	CMOption<T> *m_option;

public:
	__forceinline CMOptionLink(CMOption<T> &option) :
		CDataLink(DBVT_WCHAR), m_option(&option)
	{}

	__forceinline DWORD LoadInt() { return 0; }
	__forceinline void  SaveInt(DWORD) { }

	__forceinline wchar_t* LoadText() { return *m_option; }
	__forceinline void   SaveText(wchar_t *value) { *m_option = value; }
};

/////////////////////////////////////////////////////////////////////////////////////////
// CDlgBase - base dialog class

class MIR_CORE_EXPORT CDlgBase
{
	friend class CTimer;
	friend class CCtrlBase;
	friend class CCtrlData;

public:
	CDlgBase(HINSTANCE hInst, int idDialog);
	virtual ~CDlgBase();

	// general utilities
	void Close();
	void Resize();
	void Create();
	void Show(int nCmdShow = SW_SHOW);
	int  DoModal();
	void EndModal(INT_PTR nResult);

	CCtrlBase* FindControl(int idCtrl);
	CCtrlBase* FindControl(HWND hwnd);

	void SetCaption(const wchar_t *ptszCaption);
	void NotifyChange(void); // sends a notification to a parent window

	__forceinline void Fail() { m_lresult = false; }
	__forceinline HINSTANCE GetInst() const { return m_hInst; }
	__forceinline HWND GetHwnd() const { return m_hwnd; }
	__forceinline void Hide() { Show(SW_HIDE); }
	__forceinline bool IsInitialized() const { return m_initialized; }
	__forceinline void SetParent(HWND hwnd) { m_hwndParent = hwnd; }

	__forceinline CCtrlBase* operator[](int iControlId) { return FindControl(iControlId); }

	static CDlgBase* Find(HWND hwnd);

protected:
	HWND      m_hwnd;  // must be the first data item
	HINSTANCE m_hInst;
	HWND      m_hwndParent;
	int       m_idDialog;
	bool      m_isModal;
	bool      m_initialized;
	bool      m_forceResizable;
	bool      m_bExiting; // window received WM_CLOSE and gonna die soon
	LRESULT   m_lresult;

	enum { CLOSE_ON_OK = 0x1, CLOSE_ON_CANCEL = 0x2 };
	BYTE    m_autoClose;    // automatically close dialog on IDOK/CANCEL commands. default: CLOSE_ON_OK|CLOSE_ON_CANCEL

	// override this handlers to provide custom functionality
	// general messages
	virtual void OnInitDialog() { }
	virtual void OnClose() { }
	virtual void OnDestroy() { }

	virtual void OnTimer(CTimer*) {}

	// miranda-related stuff
	virtual int Resizer(UTILRESIZECONTROL *urc);
	virtual void OnApply() {}
	virtual void OnReset() {}
	virtual void OnChange(CCtrlBase*) {}

	// main dialog procedure
	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	// register controls
	void AddControl(CCtrlBase *ctrl);
	void AddTimer(CTimer *timer);

	// win32 stuff
	void ThemeDialogBackground(BOOL tabbed);

private:
	LIST<CTimer> m_timers;
	LIST<CCtrlBase> m_controls;

	void NotifyControls(void (CCtrlBase::*fn)());

	CTimer* FindTimer(int idEvent);

	static BOOL CALLBACK GlobalFieldEnum(HWND hwnd, LPARAM lParam);
	static INT_PTR CALLBACK GlobalDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static int GlobalDlgResizer(HWND hwnd, LPARAM lParam, UTILRESIZECONTROL *urc);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CTimer

class MIR_CORE_EXPORT CTimer
{
	friend class CDlgBase;

public:
	CTimer(CDlgBase* wnd, int idEvent);

	__forceinline int GetEventId() const { return m_idEvent; }

	virtual BOOL OnTimer();

	void Start(int elapse);
	void Stop();

	CCallback<CTimer> OnEvent;

protected:
	int m_idEvent;
	CDlgBase* m_wnd;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlBase

class MIR_CORE_EXPORT CCtrlBase
{
	friend class CDlgBase;

	__forceinline CCtrlBase(const CCtrlBase&) {}
	__forceinline CCtrlBase& operator=(const CCtrlBase&) { return *this; }

	__forceinline CCtrlBase(HWND hwnd) : m_hwnd(hwnd) {}

public:
	CCtrlBase(CDlgBase *wnd, int idCtrl);
	virtual ~CCtrlBase();

	__forceinline HWND GetHwnd() const { return m_hwnd; }
	__forceinline int GetCtrlId() const { return m_idCtrl; }
	__forceinline CDlgBase *GetParent() const { return m_parentWnd; }
	__forceinline bool IsChanged() const { return m_bChanged; }
	__forceinline void SetSilent() { m_bSilent = true; }
	__forceinline void UseSystemColors() { m_bUseSystemColors = true; }

	void Show(bool bShow = true);
	__forceinline void Hide() { Show(false); }

	void Enable(bool bIsEnable = true);
	__forceinline void Disable() { Enable(false); }
	bool Enabled(void) const;

	void NotifyChange();

	LRESULT SendMsg(UINT Msg, WPARAM wParam, LPARAM lParam) const;

	void SetText(const wchar_t *text);
	void SetTextA(const char *text);
	void SetInt(int value);

	wchar_t *GetText();
	char *GetTextA();

	wchar_t *GetText(wchar_t *buf, int size);
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

protected:
	HWND m_hwnd;  // must be the first data item
	int m_idCtrl;
	CDlgBase* m_parentWnd;
	bool m_bChanged, m_bSilent, m_bUseSystemColors;

public:
	CCallback<CCtrlBase> OnChange;
	CCallback<CCtrlBase> OnBuildMenu;

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

	virtual BOOL OnCommand(HWND hwndCtrl, WORD idCtrl, WORD idCode) override;

	CCallback<CCtrlButton> OnClick;

	void Click();
	bool IsPushed() const;
	void Push(bool bPushed);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlMButton

class MIR_CORE_EXPORT CCtrlMButton : public CCtrlButton
{
	typedef CCtrlButton CSuper;

public:
	CCtrlMButton(CDlgBase *dlg, int ctrlId, HICON hIcon, const char* tooltip);
	CCtrlMButton(CDlgBase *dlg, int ctrlId, int iCoreIcon, const char* tooltip);
	~CCtrlMButton();

	void MakeFlat();
	void MakePush();

	virtual void OnInit() override;

protected:
	HICON m_hIcon;
	const char* m_toolTip;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CSplitter

class MIR_CORE_EXPORT CSplitter : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CSplitter(CDlgBase *dlg, int ctrlId);

	__forceinline int GetPos() const { return m_iPosition; }

protected:
	virtual LRESULT CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnInit() override;

	int m_iPosition;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlHyperlink

class MIR_CORE_EXPORT CCtrlHyperlink : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlHyperlink(CDlgBase *dlg, int ctrlId, const char* url);

	virtual BOOL OnCommand(HWND hwndCtrl, WORD idCtrl, WORD idCode) override;

	CCallback<CCtrlHyperlink> OnClick;

	void SetUrl(const char *url);
	const char *GetUrl();

protected:
	const char* m_url;

	void Default_OnClick(CCtrlHyperlink*);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CProgress

class MIR_CORE_EXPORT CProgress : public CCtrlBase
{
public:
	CProgress(CDlgBase *dlg, int ctrlId);

	void SetRange(WORD max, WORD min = 0);
	void SetPosition(WORD value);
	void SetStep(WORD value);
	WORD Move(WORD delta = 0);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlClc

#if !defined(MGROUP)
	typedef int MGROUP;
#endif

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
	HANDLE     FindGroup(MGROUP hGroup);
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
	virtual BOOL OnNotify(int idCtrl, NMHDR *pnmh) override;
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
	void CreateDbLink(const char* szModuleName, const char* szSetting, wchar_t* szValue);
	void CreateDbLink(CDataLink *link) { m_dbLink = link; }

	virtual void OnInit() override;

protected:
	CDataLink *m_dbLink;

	__inline BYTE GetDataType() { return m_dbLink ? m_dbLink->GetDataType() : DBVT_DELETED; }
	__inline DWORD LoadInt() { return m_dbLink ? m_dbLink->LoadInt() : 0; }
	__inline void SaveInt(DWORD value) { if (m_dbLink) m_dbLink->SaveInt(value); }
	__inline const wchar_t *LoadText() { return m_dbLink ? m_dbLink->LoadText() : L""; }
	__inline void SaveText(wchar_t *value) { if (m_dbLink) m_dbLink->SaveText(value); }
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlCheck

class MIR_CORE_EXPORT CCtrlCheck : public CCtrlData
{
	typedef CCtrlData CSuper;

public:
	CCtrlCheck(CDlgBase *dlg, int ctrlId);
	virtual BOOL OnCommand(HWND /*hwndCtrl*/, WORD /*idCtrl*/, WORD /*idCode*/) override;

	virtual void OnApply() override;
	virtual void OnReset() override;

	int GetState();
	void SetState(int state);

	bool IsChecked();
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlEdit

class MIR_CORE_EXPORT CCtrlEdit : public CCtrlData
{
	typedef CCtrlData CSuper;

public:
	CCtrlEdit(CDlgBase *dlg, int ctrlId);
	virtual BOOL OnCommand(HWND /*hwndCtrl*/, WORD /*idCtrl*/, WORD idCode) override;

	virtual void OnApply() override;
	virtual void OnReset() override;

	void SetMaxLength(unsigned int len);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlRichEdit

class MIR_CORE_EXPORT CCtrlRichEdit : public CCtrlEdit
{
	typedef CCtrlEdit CSuper;

public:
	CCtrlRichEdit(CDlgBase *dlg, int ctrlId);

	// returns text length in bytes if a parameter is omitted or in symbols, if not
	int GetRichTextLength(int iCodePage = CP_ACP) const;

	// returns a buffer that should be freed using mir_free() or ptrA/ptrW
	char* GetRichTextRtf(bool bText = false, bool bSelection = false) const; // returns text with formatting
	wchar_t* GetRichText() const;	// returns only text in ucs2

	// these methods return text length in Unicode chars
	int SetRichText(const wchar_t *text);
	int SetRichTextRtf(const char *text);

	// enables or disables content editing
	void SetReadOnly(bool bReadOnly);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlSpin

class MIR_CORE_EXPORT CCtrlSpin : public CCtrlData
{
	typedef CCtrlData CSuper;

	virtual BOOL OnNotify(int, NMHDR*) override;

public:
	CCtrlSpin(CDlgBase *dlg, int ctrlId);

	virtual void OnApply() override;
	virtual void OnReset() override;

	WORD GetPosition();
	void SetPosition(WORD pos);
	void SetRange(WORD max, WORD min = 0);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlListBox

class MIR_CORE_EXPORT CCtrlListBox : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlListBox(CDlgBase *dlg, int ctrlId);

	int    AddString(wchar_t *text, LPARAM data=0);
	void   DeleteString(int index);
	int    FindString(wchar_t *str, int index = -1, bool exact = false);
	int    GetCount();
	int    GetCurSel();
	LPARAM GetItemData(int index);
	int    GetItemRect(int index, RECT *pResult);
	wchar_t* GetItemText(int index);
	wchar_t* GetItemText(int index, wchar_t *buf, int size);
	bool   GetSel(int index);
	int    GetSelCount();
	int*   GetSelItems(int *items, int count);
	int*   GetSelItems();
	int    InsertString(wchar_t *text, int pos, LPARAM data=0);
	void   ResetContent();
	int    SelectString(wchar_t *str);
	int    SetCurSel(int index);
	void   SetItemData(int index, LPARAM data);
	void   SetItemHeight(int index, int iHeight);
	void   SetSel(int index, bool sel = true);

	// Events
	CCallback<CCtrlListBox>	OnDblClick;
	CCallback<CCtrlListBox>	OnSelCancel;
	CCallback<CCtrlListBox>	OnSelChange;

protected:
	BOOL OnCommand(HWND hwndCtrl, WORD idCtrl, WORD idCode) override;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlCombo

class MIR_CORE_EXPORT CCtrlCombo : public CCtrlData
{
	typedef CCtrlData CSuper;

public:
	CCtrlCombo(CDlgBase *dlg, int ctrlId);

	virtual BOOL OnCommand(HWND /*hwndCtrl*/, WORD /*idCtrl*/, WORD idCode) override;
	virtual void OnInit() override;
	virtual void OnApply() override;
	virtual void OnReset() override;

	// Control interface
	int    AddString(const wchar_t *text, LPARAM data = 0);
	int    AddStringA(const char *text, LPARAM data = 0);
	void   DeleteString(int index);
	int    FindString(const wchar_t *str, int index = -1, bool exact = false);
	int    FindStringA(const char *str, int index = -1, bool exact = false);
	int    GetCount();
	int    GetCurSel();
	bool   GetDroppedState();
	LPARAM GetItemData(int index);
	wchar_t* GetItemText(int index);
	wchar_t* GetItemText(int index, wchar_t *buf, int size);
	int    InsertString(wchar_t *text, int pos, LPARAM data=0);
	void   ResetContent();
	int    SelectString(wchar_t *str);
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
	void       SetItemText(int i, int iSubItem, wchar_t *pszText);
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
	void       AddColumn(int iSubItem, wchar_t *name, int cx);
	void       AddGroup(int iGroupId, wchar_t *name);
	int        AddItem(wchar_t *text, int iIcon, LPARAM lParam = 0, int iGroupId = -1);
	void       SetItem(int iItem, int iSubItem, wchar_t *text, int iIcon = -1);
	LPARAM     GetItemData(int iItem);

	// Events
	struct TEventInfo {
		CCtrlListView *treeviewctrl;
		union {
			NMHDR          *nmhdr;
			NMLISTVIEW     *nmlv;
			NMLVDISPINFO   *nmlvdi;
			NMLVSCROLL     *nmlvscr;
			NMLVGETINFOTIP *nmlvit;
			NMLVFINDITEM   *nmlvfi;
			NMITEMACTIVATE *nmlvia;
			NMLVKEYDOWN    *nmlvkey;
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
	virtual BOOL OnNotify(int idCtrl, NMHDR *pnmh) override;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlTreeView

#undef GetNextSibling
#undef GetPrevSibling

#define MTREE_CHECKBOX    0x0001
#define MTREE_DND         0x0002
#define MTREE_MULTISELECT 0x0004

class MIR_CORE_EXPORT CCtrlTreeView : public CCtrlBase
{
	typedef CCtrlBase CSuper;

	HTREEITEM MoveItemAbove(HTREEITEM hItem, HTREEITEM hInsertAfter, HTREEITEM hParent);

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
	HTREEITEM  FindNamedItem(HTREEITEM hItem, const wchar_t *name);
	void       GetItem(HTREEITEM hItem, TVITEMEX *tvi);
	void       GetItem(HTREEITEM hItem, TVITEMEX *tvi, wchar_t *szText, int iTextLength);
	void       InvertCheck(HTREEITEM hItem);

	bool       IsSelected(HTREEITEM hItem);
	int        GetNumSelected();
	void       GetSelected(LIST<_TREEITEM> &selected);

	void       Select(HTREEITEM hItem);
	void       Select(LIST<_TREEITEM> &selected);
	void       SelectAll();
	void       SelectRange(HTREEITEM hStart, HTREEITEM hEnd);

	void       Unselect(HTREEITEM hItem);
	void       UnselectAll();

	void       DropHilite(HTREEITEM hItem);
	void       DropUnhilite(HTREEITEM hItem);

	// Events
	struct TEventInfo {
		CCtrlTreeView *treeviewctrl;
		union {
			NMHDR *nmhdr;
			NMTREEVIEW *nmtv;
			NMTVKEYDOWN *nmtvkey;
			NMTVDISPINFO *nmtvdi;
			NMTVGETINFOTIP *nmtvit;
			HTREEITEM hItem; // for OnItemChanged
		};
	};

	CCallback<TEventInfo> OnBeginDrag;
	CCallback<TEventInfo> OnBeginLabelEdit;
	CCallback<TEventInfo> OnBeginRDrag;
	CCallback<TEventInfo> OnDeleteItem;
	CCallback<TEventInfo> OnEndLabelEdit;
	CCallback<TEventInfo> OnGetDispInfo;
	CCallback<TEventInfo> OnGetInfoTip;
	CCallback<TEventInfo> OnItemChanged;
	CCallback<TEventInfo> OnItemExpanded;
	CCallback<TEventInfo> OnItemExpanding;
	CCallback<TEventInfo> OnKeyDown;
	CCallback<TEventInfo> OnSelChanged;
	CCallback<TEventInfo> OnSelChanging;
	CCallback<TEventInfo> OnSetDispInfo;
	CCallback<TEventInfo> OnSingleExpand;

protected:
	virtual void OnInit() override;
	virtual void OnDestroy() override;
	virtual BOOL OnNotify(int idCtrl, NMHDR *pnmh) override;
	
	virtual LRESULT CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

	union {
		uint32_t m_dwFlags;
		struct {
			bool m_bDndEnabled : 1;
			bool m_bDragging : 1;
			bool m_bCheckBox : 1;
			bool m_bMultiSelect : 1;
		};
	};
	HTREEITEM m_hDragItem; // valid if m_bDragging == true
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlTreeView

#define PSN_INFOCHANGED    1
#define PSN_PARAMCHANGED   2

// force-send a PSN_INFOCHANGED to all pages
#define PSM_FORCECHANGED  (WM_USER+100)

class MIR_CORE_EXPORT CCtrlPages : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlPages(CDlgBase *dlg, int ctrlId);

	void AddPage(wchar_t *ptszName, HICON hIcon, CDlgBase *pDlg);
	void ActivatePage(int iPage);
	int  GetCount(void);
	int  GetDlgIndex(CDlgBase*);
	CDlgBase* GetNthPage(int iPage);
	void RemovePage(int iPage);
	void SwapPages(int idx1, int idx2);

	__forceinline CDlgBase* GetActivePage() const
	{	return m_pActivePage;
	}

protected:
	virtual BOOL OnNotify(int idCtrl, NMHDR *pnmh) override;
	
	virtual void OnInit() override;
	virtual void OnDestroy() override;

	virtual void OnApply() override;
	virtual void OnReset() override;

	virtual LRESULT CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

private:
	HIMAGELIST m_hIml;
	CDlgBase *m_pActivePage;

	struct TPageInfo;

	void InsertPage(TPageInfo *pPage);
	void ShowPage(CDlgBase *pDlg);

	TPageInfo* GetCurrPage();
	TPageInfo* GetItemPage(int iPage);
	LIST<TPageInfo> m_pages;
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

	virtual BOOL OnCommand(HWND hwndCtrl, WORD idCtrl, WORD idCode) override
	{
		if (m_parentWnd && m_pfnOnCommand) {
			m_parentWnd->m_lresult = 0;
			(((TDlg *)m_parentWnd)->*m_pfnOnCommand)(hwndCtrl, idCtrl, idCode);
			return m_parentWnd->m_lresult;
		}
		return FALSE;
	}
	virtual BOOL OnNotify(int idCtrl, NMHDR *pnmh) override
	{
		if (m_parentWnd && m_pfnOnNotify) {
			m_parentWnd->m_lresult = 0;
			(((TDlg *)m_parentWnd)->*m_pfnOnNotify)(idCtrl, pnmh);
			return m_parentWnd->m_lresult;
		}
		return FALSE;
	}

	virtual BOOL OnMeasureItem(MEASUREITEMSTRUCT *param) override
	{
		if (m_parentWnd && m_pfnOnMeasureItem) {
			m_parentWnd->m_lresult = 0;
			(((TDlg *)m_parentWnd)->*m_pfnOnMeasureItem)(param);
			return m_parentWnd->m_lresult;
		}
		return FALSE;
	}
	virtual BOOL OnDrawItem(DRAWITEMSTRUCT *param) override
	{
		if (m_parentWnd && m_pfnOnDrawItem) {
			m_parentWnd->m_lresult = 0;
			(((TDlg *)m_parentWnd)->*m_pfnOnDrawItem)(param);
			return m_parentWnd->m_lresult;
		}
		return FALSE;
	}
	virtual BOOL OnDeleteItem(DELETEITEMSTRUCT *param) override
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

class MIR_APP_EXPORT CProtoIntDlgBase : public CDlgBase
{
	typedef CDlgBase CSuper;

public:
	CProtoIntDlgBase(PROTO_INTERFACE *proto, int idDialog, bool show_label = true);

	void CreateLink(CCtrlData& ctrl, char *szSetting, BYTE type, DWORD iValue);
	void CreateLink(CCtrlData& ctrl, const char *szSetting, wchar_t *szValue);

	template<class T>
	__inline void CreateLink(CCtrlData& ctrl, CMOption<T> &option)
	{
		ctrl.CreateDbLink(new CMOptionLink<T>(option));
	}

	__inline PROTO_INTERFACE *GetProtoInterface() { return m_proto_interface; }

	void SetStatusText(const wchar_t *statusText);

protected:
	PROTO_INTERFACE *m_proto_interface;
	bool m_show_label;
	HWND m_hwndStatus;

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	virtual void OnProtoRefresh(WPARAM, LPARAM);
	virtual void OnProtoActivate(WPARAM, LPARAM);
	virtual void OnProtoCheckOnline(WPARAM, LPARAM);

private:
	void UpdateProtoTitle(const wchar_t *szText = NULL);
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

class CPluginDlgBase : public CDlgBase
{
	const char *m_szModule;
public:
	CPluginDlgBase(HINSTANCE hInst, int idDialog, const char *module) : CDlgBase(hInst, idDialog), m_szModule(module) {};

	void CreateLink(CCtrlData& ctrl, const char *szSetting, BYTE type, DWORD iValue)
	{
		ctrl.CreateDbLink(m_szModule, szSetting, type, iValue);
	}
	void CreateLink(CCtrlData& ctrl, const char *szSetting, wchar_t *szValue)
	{
		ctrl.CreateDbLink(m_szModule, szSetting, szValue);
	}
	template<class T>
	__inline void CreateLink(CCtrlData& ctrl, CMOption<T> &option)
	{
		ctrl.CreateDbLink(new CMOptionLink<T>(option));
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

#endif // __M_GUI_H
