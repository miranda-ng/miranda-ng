/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04 Santithorn Bunchua
Copyright (c) 2005-12 George Hazan
Copyright (c) 2007-09 Maxim Mluhov
Copyright (c) 2007-09 Victor Pavlychko
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

#pragma once

#ifndef __M_GUI_H
#define __M_GUI_H

#ifdef _MSC_VER
	#include <CommCtrl.h>
#endif // _WINDOWS

#include <m_system.h>
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
	typedef uint8_t DBType;
	enum { DBTypeId = DBVT_BYTE };
	static __forceinline DBType Get(const char *szModule, const char *szSetting, DBType value)
	{
		return db_get_b(0, szModule, szSetting, value);
	}
	static __forceinline void Set(const char *szModule, const char *szSetting, DBType value)
	{
		db_set_b(0, szModule, szSetting, value);
	}
};

template<>
struct CMDBTraits<2>
{
	typedef uint16_t DBType;
	enum { DBTypeId = DBVT_WORD };
	static __forceinline DBType Get(const char *szModule, const char *szSetting, DBType value)
	{
		return db_get_w(0, szModule, szSetting, value);
	}
	static __forceinline void Set(const char *szModule, const char *szSetting, DBType value)
	{
		db_set_w(0, szModule, szSetting, value);
	}
};

template<>
struct CMDBTraits<4>
{
	typedef uint32_t DBType;
	enum { DBTypeId = DBVT_DWORD };
	static __forceinline DBType Get(const char *szModule, const char *szSetting, DBType value)
	{
		return db_get_dw(0, szModule, szSetting, value);
	}
	static __forceinline void Set(const char *szModule, const char *szSetting, DBType value)
	{
		db_set_dw(0, szModule, szSetting, value);
	}
};

template<>
struct CMDBTraits<8>
{
	typedef uint32_t DBType;
	enum { DBTypeId = DBVT_DWORD };
	static __forceinline DBType Get(const char *szModule, const char *szSetting, DBType value)
	{
		return db_get_dw(0, szModule, szSetting, value);
	}
	static __forceinline void Set(const char *szModule, const char *szSetting, DBType value)
	{
		db_set_dw(0, szModule, szSetting, value);
	}
};

class CMOptionBase : public MNonCopyable
{
public:
	__forceinline const char* GetDBModuleName() const { return m_szModuleName; }
	__forceinline const char* GetDBSettingName() const { return m_szSetting; }

	__forceinline void Delete() const 
	{	db_unset(0, m_szModuleName, m_szSetting);
	}

protected:
	__forceinline CMOptionBase(PROTO_INTERFACE *proto, const char *szSetting) :
		m_szModuleName(proto->m_szModuleName), m_szSetting(szSetting)
	{}

	__forceinline CMOptionBase(const char *module, const char *szSetting) :
		m_szModuleName(module), m_szSetting(szSetting)
	{}

	const char *m_szModuleName;
	const char *m_szSetting;
};

template<class T>
class CMOption : public CMOptionBase
{
public:
	typedef T Type;

	__forceinline CMOption(PROTO_INTERFACE *proto, const char *szSetting, Type defValue) :
		CMOptionBase(proto, szSetting), m_default(defValue)
	{}

	__forceinline CMOption(const char *szModule, const char *szSetting, Type defValue) :
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
      #ifdef _MSC_VER
		CMDBTraits<sizeof(Type)>::Set(m_szModuleName, m_szSetting, (CMDBTraits<sizeof(Type)>::DBType)value);
		#else
		CMDBTraits<sizeof(Type)>::Set(m_szModuleName, m_szSetting, value);
		#endif
		return value;
	}

private:
	Type m_default;
};

template<>
class CMOption<char*> : public CMOptionBase
{
public:

	typedef char Type;

	__forceinline CMOption(PROTO_INTERFACE *proto, const char *szSetting, const Type *defValue = nullptr) :
		CMOptionBase(proto, szSetting), m_default(defValue)
	{}

	__forceinline CMOption(const char *szModule, const char *szSetting, const Type *defValue = nullptr) :
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
};

template<>
class CMOption<wchar_t*> : public CMOptionBase
{
public:

	typedef wchar_t Type;

	__forceinline CMOption(PROTO_INTERFACE *proto, const char *szSetting, const Type *defValue = nullptr) :
		CMOptionBase(proto, szSetting), m_default(defValue)
	{}

	__forceinline CMOption(const char *szModule, const char *szSetting, const Type *defValue = nullptr) :
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
};

/////////////////////////////////////////////////////////////////////////////////////////
// CDbLink

class MIR_CORE_EXPORT CDataLink
{
protected:
	uint8_t m_type;

public:
	__inline CDataLink(uint8_t type) : m_type(type) {}
	virtual ~CDataLink() {}

	__inline uint8_t GetDataType() const { return m_type; }

	virtual uint32_t LoadInt() = 0;
	virtual void  SaveInt(uint32_t value) = 0;

	virtual wchar_t* LoadText() = 0;
	virtual void   SaveText(wchar_t *value) = 0;
};

class MIR_CORE_EXPORT CDbLink : public CDataLink
{
	char *m_szModule;
	char *m_szSetting;

	uint32_t m_iDefault;
	wchar_t *m_szDefault;

	DBVARIANT dbv;

public:
	CDbLink(const char *szModule, const char *szSetting, uint8_t type, uint32_t iValue);
	CDbLink(const char *szModule, const char *szSetting, uint8_t type, wchar_t *szValue);
	~CDbLink();

	uint32_t LoadInt() override;
	void  SaveInt(uint32_t value) override;

	wchar_t* LoadText() override;
	void   SaveText(wchar_t *value) override;
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

	__forceinline uint32_t LoadInt() override { return (uint32_t)(T)*m_option; }
	__forceinline void  SaveInt(uint32_t value) override { *m_option = (T)value; }

	__forceinline wchar_t* LoadText() override { return nullptr; }
	__forceinline void   SaveText(wchar_t*) override {}
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

	__forceinline uint32_t LoadInt() override { return 0; }
	__forceinline void  SaveInt(uint32_t) override { }

	__forceinline wchar_t* LoadText() override { return *m_option; }
	__forceinline void   SaveText(wchar_t *value) override { *m_option = value; }
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlBase

struct CContextMenuPos
{
	const class CCtrlBase *pCtrl;
	POINT pt;
	union {
		int iCurr; // int for list boxes
		HTREEITEM hItem;
	};
};

class MIR_CORE_EXPORT CCtrlBase
{
	friend class CDlgBase;

	__forceinline CCtrlBase(const CCtrlBase&) = delete;
	__forceinline CCtrlBase& operator=(const CCtrlBase&) = delete;

public:
	CCtrlBase(CDlgBase *wnd, int idCtrl);
	virtual ~CCtrlBase();

	__forceinline MWindow GetHwnd() const { return m_hwnd; }
	__forceinline int GetCtrlId() const { return m_idCtrl; }
	__forceinline CDlgBase *GetParent() const { return m_parentWnd; }
	__forceinline bool IsChanged() const { return m_bChanged; }
	__forceinline void SetSilent(bool bSilent = true) { m_bSilent = bSilent; }
	__forceinline void UseSystemColors() { m_bUseSystemColors = true; }

	void Show(bool bShow = true);
	__forceinline void Hide() { Show(false); }

	void Enable(bool bIsEnable = true);
	__forceinline void Disable() { Enable(false); }
	bool Enabled(void) const;

	void NotifyChange();
	void SetDraw(bool bEnable);

	LRESULT  SendMsg(UINT Msg, WPARAM wParam, LPARAM lParam) const;

	void     SetText(const wchar_t *text);
	void     SetTextA(const char *text);
	void     SetInt(int value);

	wchar_t* GetText() const;
	char*    GetTextA() const;
	char*    GetTextU() const;

	wchar_t* GetText(wchar_t *buf, size_t size) const;
	char*    GetTextA(char *buf, size_t size) const;
	char*    GetTextU(char *buf, size_t size) const;

	int      GetInt() const;

	virtual  BOOL OnCommand(MWindow /*hwndCtrl*/, uint16_t /*idCtrl*/, uint16_t /*idCode*/) { return FALSE; }
	virtual  BOOL OnNotify(int /*idCtrl*/, NMHDR* /*pnmh*/) { return FALSE; }

	virtual  BOOL OnMeasureItem(MEASUREITEMSTRUCT*) { return FALSE; }
	virtual  BOOL OnDrawItem(DRAWITEMSTRUCT*) { return FALSE; }
	virtual  BOOL OnDeleteItem(DELETEITEMSTRUCT*) { return FALSE; }

	virtual  void OnInit();
	virtual  void OnDestroy();

	virtual  bool OnApply();
	virtual  void OnReset();

protected:
	MWindow m_hwnd = nullptr;  // must be the first data item
	int m_idCtrl;
	bool m_bChanged = false, m_bSilent = false, m_bUseSystemColors = false, m_bNotifiable = false;
	CDlgBase *m_parentWnd;

public:
	CCallback<CCtrlBase> OnChange;
	CCallback<CContextMenuPos> OnBuildMenu;

protected:
	virtual void GetCaretPos(CContextMenuPos&) const;
	virtual LRESULT CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam);

	void Subclass();
	void Unsubclass();

private:
	static LRESULT CALLBACK GlobalSubclassWndProc(MWindow hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlData - data access controls base class

class MIR_CORE_EXPORT CCtrlData : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlData(CDlgBase *dlg, int ctrlId);
	~CCtrlData();

	void CreateDbLink(const char* szModuleName, const char* szSetting, uint8_t type, uint32_t iValue);
	void CreateDbLink(const char* szModuleName, const char* szSetting, wchar_t* szValue);
	void CreateDbLink(CDataLink *link) { m_dbLink = link; }

	void OnInit() override;

protected:
	CDataLink *m_dbLink;

	__inline uint8_t GetDataType() { return m_dbLink ? m_dbLink->GetDataType() : DBVT_DELETED; }
	__inline uint32_t LoadInt() { return m_dbLink ? m_dbLink->LoadInt() : 0; }
	__inline void SaveInt(uint32_t value) { if (m_dbLink) m_dbLink->SaveInt(value); }
	__inline const wchar_t *LoadText() { return m_dbLink ? m_dbLink->LoadText() : L""; }
	__inline void SaveText(wchar_t *value) { if (m_dbLink) m_dbLink->SaveText(value); }
};

/////////////////////////////////////////////////////////////////////////////////////////
// CDlgBase - base dialog class

class MIR_CORE_EXPORT CDlgBase
{
	friend class CTimer;
	friend class CCtrlBase;
	friend class CCtrlData;

public:
	CDlgBase(class CMPluginBase &pPlug, int idDialog);
	virtual ~CDlgBase();

	// general utilities
	void Close();
	void Resize();
	void Create();
	void Show(int nCmdShow = SW_SHOW);
	int  DoModal();
	void EndModal(INT_PTR nResult);

	class CCtrlBase* FindControl(int idCtrl);
	class CCtrlBase* FindControl(MWindow hwnd);

	void SetCaption(const wchar_t *ptszCaption);
	void SetDraw(bool bEnable);
	void NotifyChange(void); // sends a notification to a parent window

	HINSTANCE GetInst() const;

	__forceinline MWindow GetHwnd() const { return m_hwnd; }
	__forceinline void Hide() { Show(SW_HIDE); }
	__forceinline bool IsInitialized() const { return m_bInitialized; }
	__forceinline void SetMinSize(int x, int y) { m_iMinWidth = x, m_iMinHeight = y; }
	__forceinline void SetParent(MWindow hwnd) { m_hwndParent = hwnd; }

	__forceinline CCtrlBase* operator[](int iControlId) { return FindControl(iControlId); }

	static CDlgBase* Find(MWindow hwnd);

protected:
	MWindow m_hwnd = nullptr;  // must be the first data item
	MWindow m_hwndParent = nullptr;
	int     m_idDialog;

	bool    m_isModal = false;
	bool    m_bInitialized = false;
	bool    m_forceResizable = false;
	bool    m_bFixedSize;
	bool    m_bSucceeded = false; // was IDOK pressed or not
	bool    m_bExiting = false; // window received WM_CLOSE and gonna die soon

	enum { CLOSE_ON_OK = 0x1, CLOSE_ON_CANCEL = 0x2 };
	uint8_t    m_autoClose;    // automatically close dialog on IDOK/CANCEL commands. default: CLOSE_ON_OK|CLOSE_ON_CANCEL

	CMPluginBase &m_pPlugin;

	// override this handlers to provide custom functionality
	// general messages
	virtual bool OnInitDialog();
	virtual bool OnApply();
	virtual bool OnClose();
	virtual void OnDestroy();

	virtual void OnTimer(class CTimer*);

	// miranda-related stuff
	virtual int Resizer(UTILRESIZECONTROL *urc);
	virtual void OnResize();
	virtual void OnReset();
	virtual void OnChange();

	// main dialog procedure
	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	CCallback<void> m_OnFinishWizard;

	// register controls
	void AddControl(CCtrlBase *ctrl);
	void RemoveControl(CCtrlBase *ctrl);

	// timers
	void AddTimer(CTimer *timer);
	void RemoveTimer(UINT_PTR idEvent);

	// options support
	void CreateLink(class CCtrlData& ctrl, const char *szSetting, uint8_t type, uint32_t iValue);
	void CreateLink(class CCtrlData& ctrl, const char *szSetting, wchar_t *szValue);

	template<class T>
	__inline void CreateLink(CCtrlData& ctrl, CMOption<T> &option)
	{
		ctrl.CreateDbLink(new CMOptionLink<T>(option));
	}

	// win32 stuff
	void ThemeDialogBackground(BOOL tabbed);

private:
	LIST<CTimer> m_timers;
	LIST<CCtrlBase> m_controls;

	void NotifyControls(void (CCtrlBase::*fn)());
	bool VerifyControls(bool (CCtrlBase::*fn)());

	CTimer* FindTimer(int idEvent);
	int m_iMinWidth = -1, m_iMinHeight = -1;

	static BOOL CALLBACK GlobalFieldEnum(MWindow hwnd, LPARAM lParam);
	static INT_PTR CALLBACK GlobalDlgProc(MWindow hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static int GlobalDlgResizer(MWindow hwnd, LPARAM lParam, UTILRESIZECONTROL *urc);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CTimer

class MIR_CORE_EXPORT CTimer
{
	friend class CDlgBase;

public:
	CTimer(CDlgBase* wnd, UINT_PTR idEvent);
	~CTimer();

	__forceinline UINT_PTR GetEventId() const { return m_idEvent; }
	__forceinline MWindow GetHwnd() const { return m_wnd->GetHwnd(); }

	virtual BOOL OnTimer();

	void Start(int elapse);
	bool Stop(); // returns true if timer was active

	void StartSafe(int elapse);
	void StopSafe();

	CCallback<CTimer> OnEvent;

protected:
	UINT_PTR  m_idEvent;
	CDlgBase* m_wnd;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlLabel

class MIR_CORE_EXPORT CCtrlLabel : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlLabel(CDlgBase *dlg, int ctrlId);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlButton

class MIR_CORE_EXPORT CCtrlButton : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlButton(CDlgBase *dlg, int ctrlId);

	BOOL OnCommand(MWindow hwndCtrl, uint16_t idCtrl, uint16_t idCode) override;

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

	void OnInit() override;

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
	LRESULT CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
	void OnInit() override;

	int m_iPosition;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlHyperlink

class MIR_CORE_EXPORT CCtrlHyperlink : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlHyperlink(CDlgBase *dlg, int ctrlId, const char* url = nullptr);

	BOOL OnCommand(MWindow hwndCtrl, uint16_t idCtrl, uint16_t idCode) override;

	CCallback<CCtrlHyperlink> OnClick;

	void SetUrl(const char *url);
	const char *GetUrl();

protected:
	const char* m_url;

	void Default_OnClick(CCtrlHyperlink*);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CProgress

class MIR_CORE_EXPORT CCtrlProgress : public CCtrlBase
{
public:
	CCtrlProgress(CDlgBase *dlg, int ctrlId);

	void SetRange(uint16_t max, uint16_t min = 0);
	void SetPosition(uint16_t value);
	void SetStep(uint16_t value);
	uint16_t Move(uint16_t delta = 0);
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
	HANDLE     AddInfoItem(CLCINFOITEM *cii);
	void       AutoRebuild();
	void       DeleteItem(HANDLE hItem);
	void       EditLabel(HANDLE hItem);
	void       EndEditLabel(bool save);
	void       EnsureVisible(HANDLE hItem, bool partialOk);
	void       Expand(HANDLE hItem, uint32_t flags);
	HANDLE     FindContact(MCONTACT hContact);
	HANDLE     FindGroup(MGROUP hGroup);
	COLORREF   GetBkColor() const;
	bool       GetCheck(HANDLE hItem) const;
	int        GetCount() const;
	MWindow    GetEditControl() const;
	uint32_t   GetExStyle() const;
	uint32_t   GetExpand(HANDLE hItem) const;
	int        GetExtraColumns() const;
	uint8_t    GetExtraImage(HANDLE hItem, int iColumn) const;
	HIMAGELIST GetExtraImageList() const;
	HFONT      GetFont(int iFontId) const;
	bool       GetHideOfflineRoot() const;
	int        GetItemType(HANDLE hItem) const;
	HANDLE     GetNextItem(HANDLE hItem, uint32_t flags) const;
	HANDLE     GetSelection() const;
	HANDLE     HitTest(int x, int y, uint32_t *hitTest) const;
	void       SelectItem(HANDLE hItem);
	void       SetBkColor(COLORREF clBack);
	void       SetCheck(HANDLE hItem, bool check);
	void       SetExStyle(uint32_t exStyle);
	void       SetExtraColumns(int iColumns);
	void       SetExtraImage(HANDLE hItem, int iColumn, int iImage);
	void       SetExtraImageList(HIMAGELIST hImgList);
	void       SetFont(int iFontId, HANDLE hFont, bool bRedraw);
	void       SetItemText(HANDLE hItem, char *szText);
	void       SetHideEmptyGroups(bool state);
	void       SetHideOfflineRoot(bool state);
	void       SetOfflineModes(uint32_t modes);
	void       SetUseGroups(bool state);

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
	BOOL OnNotify(int idCtrl, NMHDR *pnmh) override;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlCheck

class MIR_CORE_EXPORT CCtrlCheck : public CCtrlData
{
	typedef CCtrlData CSuper;

public:
	CCtrlCheck(CDlgBase *dlg, int ctrlId);
	BOOL OnCommand(MWindow /*hwndCtrl*/, uint16_t /*idCtrl*/, uint16_t /*idCode*/) override;

	bool OnApply() override;
	void OnReset() override;

	int GetState() const;
	void SetState(int state);

	bool IsChecked();
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlColor - color picker

class MIR_CORE_EXPORT CCtrlColor : public CCtrlData
{
	typedef CCtrlData CSuper;

public:
	CCtrlColor(CDlgBase *dlg, int ctrlId);
	BOOL OnCommand(MWindow /*hwndCtrl*/, uint16_t /*idCtrl*/, uint16_t /*idCode*/) override;

	bool OnApply() override;
	void OnReset() override;

	uint32_t GetColor();
	void SetColor(uint32_t dwValue);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlDate - date & time picker

class MIR_CORE_EXPORT CCtrlDate : public CCtrlData
{
	typedef CCtrlData CSuper;

	BOOL OnNotify(int, NMHDR*) override;

public:
	CCtrlDate(CDlgBase *dlg, int ctrlId);

	void GetTime(SYSTEMTIME*);
	void SetTime(SYSTEMTIME*);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlEdit

class MIR_CORE_EXPORT CCtrlEdit : public CCtrlData
{
	typedef CCtrlData CSuper;

public:
	CCtrlEdit(CDlgBase *dlg, int ctrlId);
	BOOL OnCommand(MWindow /*hwndCtrl*/, uint16_t /*idCtrl*/, uint16_t idCode) override;

	bool OnApply() override;
	void OnReset() override;

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
// CCtrlSlider

class MIR_CORE_EXPORT CCtrlSlider : public CCtrlData
{
	typedef CCtrlData CSuper;

	int m_wMin, m_wMax;

protected:
	BOOL OnCommand(MWindow hwndCtrl, uint16_t idCtrl, uint16_t idCode) override;

public:
	CCtrlSlider(CDlgBase *dlg, int ctrlId, int max = 100, int min = 0);

	bool OnApply() override;
	void OnReset() override;

	int  GetPosition() const;
	void SetPosition(int pos);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlSpin

class MIR_CORE_EXPORT CCtrlSpin : public CCtrlData
{
	typedef CCtrlData CSuper;

	int16_t m_wMin, m_wMax, m_wCurr;

	BOOL OnNotify(int, NMHDR*) override;

public:
	CCtrlSpin(CDlgBase *dlg, int ctrlId, int16_t max = 100, int16_t min = 0);

	bool OnApply() override;
	void OnReset() override;

	int16_t GetPosition();
	void SetPosition(int16_t pos);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlListBox

class MIR_CORE_EXPORT CCtrlListBox : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlListBox(CDlgBase *dlg, int ctrlId);

	int      AddString(const wchar_t *text, LPARAM data=0);
	void     DeleteString(int index);
	int      FindString(const wchar_t *str, int index = -1, bool exact = false);
	int      GetCount() const;
	int      GetCurSel() const;
	LPARAM   GetItemData(int index) const;
	int      GetItemRect(int index, RECT *pResult) const;
	wchar_t* GetItemText(int index) const;
	wchar_t* GetItemText(int index, wchar_t *buf, int size) const;
	bool     GetSel(int index) const;
	int      GetSelCount() const;
	int*     GetSelItems(int *items, int count) const;
	int*     GetSelItems() const;
	int      InsertString(const wchar_t *text, int pos, LPARAM data=0);
	void     ResetContent();
	int      SelectString(const wchar_t *str);
	int      SetCurSel(int index);
	void     SetItemData(int index, LPARAM data);
	void     SetItemHeight(int index, int iHeight);
	void     SetSel(int index, bool sel = true);

	// Events
	CCallback<CCtrlListBox>	OnDblClick;
	CCallback<CCtrlListBox>	OnSelCancel;
	CCallback<CCtrlListBox>	OnSelChange;

protected:
	BOOL OnCommand(MWindow hwndCtrl, uint16_t idCtrl, uint16_t idCode) override;
	void GetCaretPos(CContextMenuPos&) const override;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlCombo

class MIR_CORE_EXPORT CCtrlCombo : public CCtrlData
{
	typedef CCtrlData CSuper;

public:
	CCtrlCombo(CDlgBase *dlg, int ctrlId);

	BOOL OnCommand(MWindow /*hwndCtrl*/, uint16_t /*idCtrl*/, uint16_t idCode) override;
	void OnInit() override;
	bool OnApply() override;
	void OnReset() override;

	// returns item data associated with the selected item or -1
	LPARAM   GetCurData() const;

	// selects line with userdata passed. returns index of this line or -1
	int      SelectData(LPARAM data);

	// Control interface
	int      AddString(const wchar_t *text, LPARAM data = 0);
	int      AddStringA(const char *text, LPARAM data = 0);
	void     DeleteString(int index);
	int      FindString(const wchar_t *str, int index = -1, bool exact = false);
	int      FindStringA(const char *str, int index = -1, bool exact = false);
	int      GetCount() const;
	int      GetCurSel() const;
	bool     GetDroppedState() const;
	LPARAM   GetItemData(int index) const;
	wchar_t* GetItemText(int index) const;
	wchar_t* GetItemText(int index, wchar_t *buf, int size) const;
	int      InsertString(const wchar_t *text, int pos, LPARAM data=0);
	void     ResetContent();
	int      SelectString(const wchar_t *str);
	int      SetCurSel(int index);
	void     SetItemData(int index, LPARAM data);
	void     ShowDropdown(bool show = true);

	// Events
	CCallback<CCtrlCombo>	OnCloseup;
	CCallback<CCtrlCombo>	OnDropdown;
	CCallback<CCtrlCombo>	OnKillFocus;
	CCallback<CCtrlCombo>	OnSelChanged;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlListView

class MIR_CORE_EXPORT CCtrlListView : public CCtrlBase
{
	typedef CCtrlBase CSuper;

public:
	CCtrlListView(CDlgBase *dlg, int ctrlId);

	// direction = -1 or 1. returns new item index
	int MoveItem(int idx, int direction);

	void SetCurSel(int idx);

	// Classic LV interface
	uint32_t   ApproximateViewRect(int cx, int cy, int iCount);
	void       Arrange(UINT code);
	void       CancelEditLabel();
	HIMAGELIST CreateDragImage(int iItem, POINT *lpptUpLeft);
	void       DeleteAllItems();
	void       DeleteColumn(int iCol);
	void       DeleteItem(int iItem);
	MWindow    EditLabel(int iItem);
	int        EnableGroupView(BOOL fEnable);
	BOOL       EnsureVisible(int i, BOOL fPartialOK);
	int        FindItem(int iStart, const LVFINDINFO *plvfi);
	COLORREF   GetBkColor() const;
	void       GetBkImage(LVBKIMAGE *plvbki) const;
	UINT       GetCallbackMask() const;
	BOOL       GetCheckState(UINT iIndex) const;
	void       GetColumn(int iCol, LVCOLUMN *pcol) const;
	void       GetColumnOrderArray(int iCount, int *lpiArray) const;
	int        GetColumnWidth(int iCol) const;
	int        GetCountPerPage() const;
	MWindow    GetEditControl() const;
	uint32_t   GetExtendedListViewStyle() const;
	int        GetFocusedGroup() const;
	int        GetGroupCount() const;
	void       GetGroupInfo(int iGroupId, LVGROUP *pgrp) const;
	void       GetGroupInfoByIndex(int iIndex, LVGROUP *pgrp) const;
	void       GetGroupMetrics(LVGROUPMETRICS *pGroupMetrics) const;
	UINT       GetGroupState(UINT dwGroupId, UINT dwMask) const;
	MWindow    GetHeader() const;
	HCURSOR    GetHotCursor() const;
	int        GetHotItem() const;
	uint32_t   GetHoverTime() const;
	HIMAGELIST GetImageList(int iImageList) const;
	BOOL       GetInsertMark(LVINSERTMARK *plvim) const;
	COLORREF   GetInsertMarkColor() const;
	int        GetInsertMarkRect(RECT *prc) const;
	BOOL       GetISearchString(LPSTR lpsz) const;
	bool       GetItem(LVITEM *pitem) const;
	int        GetItemCount() const;
	void       GetItemPosition(int i, POINT *ppt) const;
	void       GetItemRect(int i, RECT *prc, int code) const;
	uint32_t   GetItemSpacing(BOOL fSmall) const;
	UINT       GetItemState(int i, UINT mask) const;
	void       GetItemText(int iItem, int iSubItem, LPTSTR pszText, int cchTextMax) const;
	int        GetNextItem(int iStart, UINT flags) const;
	BOOL       GetNumberOfWorkAreas(UINT *lpuWorkAreas) const;
	BOOL       GetOrigin(POINT *lpptOrg) const;
	COLORREF   GetOutlineColor() const;
	UINT       GetSelectedColumn() const;
	UINT       GetSelectedCount() const;
	int        GetSelectionMark() const;
	int        GetStringWidth(LPCSTR psz) const;
	BOOL       GetSubItemRect(int iItem, int iSubItem, int code, RECT *lpRect) const;
	COLORREF   GetTextBkColor() const;
	COLORREF   GetTextColor() const;
	void       GetTileInfo(LVTILEINFO *plvtinfo) const;
	void       GetTileViewInfo(LVTILEVIEWINFO *plvtvinfo) const;
	MWindow    GetToolTips() const;
	int        GetTopIndex() const;
	BOOL       GetUnicodeFormat() const;
	uint32_t   GetView() const;
	BOOL       GetViewRect(RECT *prc) const;
	void       GetWorkAreas(int nWorkAreas, RECT *lprc) const;
	BOOL       HasGroup(int dwGroupId);
	int        HitTest(LVHITTESTINFO *pinfo) const;
	int        HitTestEx(LVHITTESTINFO *pinfo);
	int        InsertColumn(int iCol, const LVCOLUMN *pcol);
	int        InsertGroup(int index, LVGROUP *pgrp);
	void       InsertGroupSorted(LVINSERTGROUPSORTED *structInsert);
	int        InsertItem(const LVITEM *pitem);
	BOOL       InsertMarkHitTest(POINT *point, LVINSERTMARK *plvim);
	BOOL       IsGroupViewEnabled();
	UINT       IsItemVisible(UINT index);
	UINT       MapIDToIndex(UINT id);
	UINT       MapIndexToID(UINT index);
	BOOL       RedrawItems(int iFirst, int iLast);
	void       RemoveAllGroups();
	int        RemoveGroup(int iGroupId);
	BOOL       Scroll(int dx, int dy);
	BOOL       SetBkColor(COLORREF clrBk);
	BOOL       SetBkImage(LVBKIMAGE *plvbki);
	BOOL       SetCallbackMask(UINT mask);
	void       SetCheckState(UINT iIndex, BOOL fCheck);
	BOOL       SetColumn(int iCol, LVCOLUMN *pcol);
	BOOL       SetColumnOrderArray(int iCount, int *lpiArray);
	BOOL       SetColumnWidth(int iCol, int cx);
	void       SetExtendedListViewStyle(uint32_t dwExStyle);
	void       SetExtendedListViewStyleEx(uint32_t dwExMask, uint32_t dwExStyle);
	int        SetGroupInfo(int iGroupId, LVGROUP *pgrp);
	void       SetGroupMetrics(LVGROUPMETRICS *pGroupMetrics);
	void       SetGroupState(UINT dwGroupId, UINT dwMask, UINT dwState);
	HCURSOR    SetHotCursor(HCURSOR hCursor);
	int        SetHotItem(int iIndex);
	void       SetHoverTime(uint32_t dwHoverTime);
	uint32_t   SetIconSpacing(int cx, int cy);
	HIMAGELIST SetImageList(HIMAGELIST himl, int iImageList);
	BOOL       SetInfoTip(LVSETINFOTIP *plvSetInfoTip);
	BOOL       SetInsertMark(LVINSERTMARK *plvim);
	COLORREF   SetInsertMarkColor(COLORREF color);
	BOOL       SetItem(const LVITEM *pitem);
	void       SetItemCount(int cItems);
	void       SetItemCountEx(int cItems, uint32_t dwFlags);
	BOOL       SetItemPosition(int i, int x, int y);
	void       SetItemPosition32(int iItem, int x, int y);
	void       SetItemState(int i, UINT state, UINT mask);
	void       SetItemText(int i, int iSubItem, const wchar_t *pszText);
	COLORREF   SetOutlineColor(COLORREF color);
	void       SetSelectedColumn(int iCol);
	int        SetSelectionMark(int iIndex);
	BOOL       SetTextBkColor(COLORREF clrText);
	BOOL       SetTextColor(COLORREF clrText);
	BOOL       SetTileInfo(LVTILEINFO *plvtinfo);
	BOOL       SetTileViewInfo(LVTILEVIEWINFO *plvtvinfo);
	MWindow    SetToolTips(MWindow ToolTip);
	BOOL       SetUnicodeFormat(BOOL fUnicode);
	int        SetView(uint32_t iView);
	void       SetWorkAreas(int nWorkAreas, RECT *lprc);
	int        SubItemHitTest(LVHITTESTINFO *pInfo) const;
	int        SubItemHitTestEx(LVHITTESTINFO *plvhti);
	BOOL       Update(int iItem);

	#ifdef _MSC_VER
	int        SortGroups(PFNLVGROUPCOMPARE pfnGroupCompare, LPVOID plv);
	BOOL       SortItems(PFNLVCOMPARE pfnCompare, LPARAM lParamSort);
	BOOL       SortItemsEx(PFNLVCOMPARE pfnCompare, LPARAM lParamSort);
	#endif // _MSC_VER

	// Additional APIs
	HIMAGELIST CreateImageList(int iImageList);
	void       AddColumn(int iSubItem, const wchar_t *name, int cx);
	void       AddGroup(int iGroupId, const wchar_t *name);
	int        AddItem(const wchar_t *text, int iIcon, LPARAM lParam = 0, int iGroupId = -1);
	void       SetItem(int iItem, int iSubItem, const wchar_t *text, int iIcon = -1);
	LPARAM     GetItemData(int iItem) const;

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
			NMLVCUSTOMDRAW *nmcd;
		};
	};

	CCallback<TEventInfo> OnBeginDrag;
	CCallback<TEventInfo> OnBeginLabelEdit;
	CCallback<TEventInfo> OnBeginRDrag;
	CCallback<TEventInfo> OnBeginScroll;
	CCallback<TEventInfo> OnColumnClick;
	CCallback<TEventInfo> OnCustomDraw;
	CCallback<TEventInfo> OnDeleteAllItems;
	CCallback<TEventInfo> OnDeleteItem;
	CCallback<TEventInfo> OnClick;
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
	BOOL OnNotify(int idCtrl, NMHDR *pnmh) override;
	void GetCaretPos(CContextMenuPos&) const override;
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
	MWindow    EditLabel(HTREEITEM hItem);
	void       EndEditLabelNow(BOOL cancel);
	void       EnsureVisible(HTREEITEM hItem);
	void       Expand(HTREEITEM hItem, uint32_t flag);
	COLORREF   GetBkColor() const;
	uint32_t   GetCheckState(HTREEITEM hItem) const;
	HTREEITEM  GetChild(HTREEITEM hItem) const;
	int        GetCount() const;
	HTREEITEM  GetDropHilight() const;
	MWindow    GetEditControl() const;
	HTREEITEM  GetFirstVisible() const;
	HIMAGELIST GetImageList(int iImage) const;
	int        GetIndent() const;
	COLORREF   GetInsertMarkColor() const;
	bool       GetItem(TVITEMEX *tvi) const;
	int        GetItemHeight() const;
	void       GetItemRect(HTREEITEM hItem, RECT *rcItem, BOOL fItemRect) const;
	uint32_t   GetItemState(HTREEITEM hItem, uint32_t stateMask) const;
	HTREEITEM  GetLastVisible() const;
	COLORREF   GetLineColor() const;
	HTREEITEM  GetNextItem(HTREEITEM hItem, uint32_t flag) const;
	HTREEITEM  GetNextSibling(HTREEITEM hItem) const;
	HTREEITEM  GetNextVisible(HTREEITEM hItem) const;
	HTREEITEM  GetParent(HTREEITEM hItem) const;
	HTREEITEM  GetPrevSibling(HTREEITEM hItem) const;
	HTREEITEM  GetPrevVisible(HTREEITEM hItem) const;
	HTREEITEM  GetRoot() const;
	uint32_t   GetScrollTime() const;
	HTREEITEM  GetSelection() const;
	COLORREF   GetTextColor() const;
	MWindow    GetToolTips() const;
	BOOL       GetUnicodeFormat() const;
	unsigned   GetVisibleCount() const;
	HTREEITEM  HitTest(TVHITTESTINFO *hti) const;
	HTREEITEM  InsertItem(TVINSERTSTRUCT *tvis);
	void       Select(HTREEITEM hItem, uint32_t flag);
	void       SelectDropTarget(HTREEITEM hItem);
	void       SelectItem(HTREEITEM hItem);
	void       SelectSetFirstVisible(HTREEITEM hItem);
	COLORREF   SetBkColor(COLORREF clBack);
	void       SetCheckState(HTREEITEM hItem, uint32_t state);
	HIMAGELIST SetImageList(HIMAGELIST hIml, int iImage);
	void       SetIndent(int iIndent);
	void       SetInsertMark(HTREEITEM hItem, BOOL fAfter);
	COLORREF   SetInsertMarkColor(COLORREF clMark);
	void       SetItem(TVITEMEX *tvi);
	void       SetItemHeight(short cyItem);
	void       SetItemState(HTREEITEM hItem, uint32_t state, uint32_t stateMask);
	COLORREF   SetLineColor(COLORREF clLine);
	void       SetScrollTime(UINT uMaxScrollTime);
	COLORREF   SetTextColor(COLORREF clText);
	MWindow    SetToolTips(MWindow hwndToolTips);
	BOOL       SetUnicodeFormat(BOOL fUnicode);
	void       SortChildren(HTREEITEM hItem, BOOL fRecurse);
	void       SortChildrenCB(TVSORTCB *cb, BOOL fRecurse);

	// Additional stuff
	void       TranslateItem(HTREEITEM hItem);
	void       TranslateTree();
	HTREEITEM  FindNamedItem(HTREEITEM hItem, const wchar_t *name);
	void       GetItem(HTREEITEM hItem, TVITEMEX *tvi) const;
	void       GetItem(HTREEITEM hItem, TVITEMEX *tvi, wchar_t *szText, int iTextLength) const;
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
			NMTVCUSTOMDRAW *nmcd;
			HTREEITEM hItem; // for OnItemChanged
		};
	};

	CCallback<TEventInfo> OnBeginDrag;
	CCallback<TEventInfo> OnBeginLabelEdit;
	CCallback<TEventInfo> OnBeginRDrag;
	CCallback<TEventInfo> OnCustomDraw;
	CCallback<TEventInfo> OnDeleteItem;
	CCallback<TEventInfo> OnEndLabelEdit;
	CCallback<TEventInfo> OnGetDispInfo;
	CCallback<TEventInfo> OnGetInfoTip;
	CCallback<TEventInfo> OnItemChanged;
	CCallback<TEventInfo> OnItemExpanded;
	CCallback<TEventInfo> OnItemExpanding;
	CCallback<TEventInfo> OnKeyDown;
	CCallback<TEventInfo> OnRightClick;
	CCallback<TEventInfo> OnSelChanged;
	CCallback<TEventInfo> OnSelChanging;
	CCallback<TEventInfo> OnSetDispInfo;
	CCallback<TEventInfo> OnSingleExpand;

protected:
	void OnInit() override;
	void OnDestroy() override;
	BOOL OnNotify(int idCtrl, NMHDR *pnmh) override;

	void GetCaretPos(CContextMenuPos&) const override;
	LRESULT CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

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
// CCtrlTreeOpts - array of options with sections

class MIR_CORE_EXPORT CCtrlTreeOpts : public CCtrlTreeView
{
	typedef CCtrlTreeView CSuper;

public:
	CCtrlTreeOpts(CDlgBase *dlg, int ctrlId);
	~CCtrlTreeOpts();

	void AddOption(const wchar_t *pwszSection, const wchar_t *pwszName, CMOption<bool> &option);
	void AddOption(const wchar_t *pwszSection, const wchar_t *pwszName, bool &option);
	void AddOption(const wchar_t *pwszSection, const wchar_t *pwszName, uint32_t &option, uint32_t mask);

	BOOL OnNotify(int idCtrl, NMHDR *pnmh) override;
	void OnDestroy() override;
	void OnInit() override;
	bool OnApply() override;

protected:
	struct COptionsItem
	{
		const wchar_t *m_pwszSection, *m_pwszName;

		union
		{
			CMOption<bool> *m_option;
			bool *m_pBool;
			struct
			{
				uint32_t *m_pDword;
				uint32_t m_mask;
			};
		};

		HTREEITEM m_hItem = nullptr;
		enum OptionItemType { CMOPTION = 1, BOOL = 2, MASK = 3 };
		OptionItemType m_type;

		COptionsItem(const wchar_t *pwszSection, const wchar_t *pwszName, OptionItemType type) :
			m_pwszSection(pwszSection),
			m_pwszName(pwszName),
			m_type(type)
		{}
	};

	OBJLIST<COptionsItem> m_options;

	void ProcessItemClick(HTREEITEM hti);
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlPages

#define PSN_INFOCHANGED    1

// force-send a PSN_INFOCHANGED to all pages
#define PSM_FORCECHANGED  (WM_USER+100)

class MIR_CORE_EXPORT CCtrlPages : public CCtrlBase
{
	typedef CCtrlBase CSuper;

	HIMAGELIST m_hIml;
	CDlgBase *m_pActivePage;
	int m_numRows = 1;

	struct TPageInfo;
	void InsertPage(TPageInfo *pPage);
	void ShowPage(CDlgBase *pDlg);

	void CheckRowCount();
	TPageInfo* GetCurrPage();
	TPageInfo* GetItemPage(int iPage);
	LIST<TPageInfo> m_pages;

public:
	CCtrlPages(CDlgBase *dlg, int ctrlId);

	void AddPage(const wchar_t *ptszName, HICON hIcon, CDlgBase *pDlg);
	void ActivatePage(int iPage);
	int  GetCount(void);
	int  GetDlgIndex(CDlgBase*);
	void RemovePage(int iPage);
	void SwapPages(int idx1, int idx2);

	CDlgBase* GetNthPage(int iPage);

	__forceinline CDlgBase* GetActivePage() const
	{	return m_pActivePage;
	}

protected:
	BOOL OnNotify(int idCtrl, NMHDR *pnmh) override;

	void OnInit() override;
	void OnDestroy() override;

	bool OnApply() override;
	void OnReset() override;

	LRESULT CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
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
	CProtoIntDlgBase(PROTO_INTERFACE *proto, int idDialog);

	void CreateLink(CCtrlData &ctrl, const char *szSetting, uint8_t type, uint32_t iValue);
	void CreateLink(CCtrlData &ctrl, const char *szSetting, wchar_t *szValue);

	template<class T>
	__inline void CreateLink(CCtrlData &ctrl, CMOption<T> &option)
	{
		ctrl.CreateDbLink(new CMOptionLink<T>(option));
	}

	__inline PROTO_INTERFACE *GetProtoInterface() { return m_proto_interface; }

	void SetStatusText(const wchar_t *statusText);

protected:
	PROTO_INTERFACE *m_proto_interface;
	MWindow m_hwndStatus = nullptr;

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

	virtual void OnProtoRefresh(WPARAM, LPARAM);
	virtual void OnProtoActivate(WPARAM, LPARAM);
	virtual void OnProtoCheckOnline(WPARAM, LPARAM);

private:
	void UpdateStatusBar();
};

template<typename TProto>
class CProtoDlgBase : public CProtoIntDlgBase
{
	typedef CProtoIntDlgBase CSuper;

public:
	__inline CProtoDlgBase<TProto>(TProto *proto, int idDialog) :
		CProtoIntDlgBase(proto, idDialog),
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
