/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#ifndef M_SRMM_INT_H__
#define M_SRMM_INT_H__ 1

#include <m_gui.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// toolbar button internal representation

#define MIN_CBUTTONID      4000
#define MAX_CBUTTONID      5000

#define BBSF_IMBUTTON		(1<<0)
#define BBSF_CHATBUTTON		(1<<1)
#define BBSF_CANBEHIDDEN	(1<<2)
#define BBSF_NTBSWAPED		(1<<3)
#define BBSF_NTBDESTRUCT	(1<<4)

struct CustomButtonData : public MZeroedObject
{
	~CustomButtonData()
	{}

	int      m_dwPosition;    // default order pos of button, counted from window edge (left or right)

	int      m_dwButtonID;    // id of button used while button creation and to store button info in DB
	ptrA     m_pszModuleName; // module name without spaces and underline symbols (e.g. "tabsrmm")

	int      m_dwButtonCID;	// button's control id
	int      m_dwArrowCID;    // only use with BBBF_ISARROWBUTTON flag

	ptrW     m_pwszText;      // button's text
	ptrW     m_pwszTooltip;   // button's tooltip

	int      m_iButtonWidth;  // must be 22 for regular button and 33 for button with arrow
	HANDLE   m_hIcon;         // Handle to icolib registred icon

	bool     m_bIMButton, m_bChatButton;
	bool     m_bCanBeHidden, m_bCantBeHidden, m_bHidden, m_bSeparator, m_bDisabled, m_bPushButton;
	bool     m_bRSided;
	uint8_t  m_opFlags;
	HPLUGIN  m_pPlugin;
	uint32_t m_dwOrigPosition;
	struct   THotkeyItem *m_hotkey;

	struct {
		bool bit1 : 1, bit2 : 1, bit3 : 1, bit4 : 1;
	} m_dwOrigFlags;
};

// gets the required button or NULL, if i is out of boundaries
EXTERN_C MIR_APP_DLL(CustomButtonData*) Srmm_GetNthButton(int i);

// retrieves total number of toolbar buttons
EXTERN_C MIR_APP_DLL(int) Srmm_GetButtonCount(void);

// emulates a click on a toolbar button
EXTERN_C MIR_APP_DLL(void) Srmm_ClickToolbarIcon(MCONTACT hContact, int idFrom, HWND hwndFrom, BOOL code);

// these messages are sent to the message windows if toolbar buttons are changed
#define WM_CBD_FIRST   (WM_USER+0x600)

// wParam = 0 (ignored)
// lParam = (CustomButtonData*)pointer to button or null if any button can be changed
#define WM_CBD_UPDATED (WM_CBD_FIRST+1)

// wParam = button id
// lParam = (CustomButtonData*)pointer to button
#define WM_CBD_REMOVED (WM_CBD_FIRST+2)

// wParam = 0 (ignored)
// lParam = 0 (ignored)
#define WM_CBD_LOADICONS (WM_CBD_FIRST+3)

// wParam = 0 (ignored)
// lParam = 0 (ignored)
#define WM_CBD_RECREATE (WM_CBD_FIRST+4)

/////////////////////////////////////////////////////////////////////////////////////////
// SRMM log window container

class CMsgDialog;

class MIR_APP_EXPORT CSrmmLogWindow
{
	CSrmmLogWindow(const CSrmmLogWindow &) = delete;
	CSrmmLogWindow &operator=(const CSrmmLogWindow &) = delete;

protected:
	CMsgDialog &m_pDlg;

	CSrmmLogWindow(CMsgDialog &pDlg) :
		m_pDlg(pDlg)
	{}

public:
	virtual ~CSrmmLogWindow() {}

	virtual void     Attach() = 0;
	virtual void     Detach() = 0;
					     
	virtual bool     AtBottom() = 0;
	virtual void     Clear() = 0;
	virtual int      GetType() = 0;
	virtual HWND     GetHwnd() = 0;
	virtual wchar_t* GetSelection() = 0;
	virtual void     LogEvents(MEVENT hDbEventFirst, int count, bool bAppend) = 0;
	virtual void     LogEvents(struct LOGINFO *, bool) = 0;
	virtual void     Resize() = 0;
	virtual void     ScrollToBottom() = 0;
	virtual void     UpdateOptions() {};

	virtual INT_PTR Notify(WPARAM, LPARAM) { return 0; }
};

typedef CSrmmLogWindow *(MIR_CDECL *pfnSrmmLogCreator)(CMsgDialog &pDlg);

EXTERN_C MIR_APP_DLL(HANDLE) RegisterSrmmLog(const char *pszShortName, const wchar_t *pwszScreenName, pfnSrmmLogCreator fnBuilder);
EXTERN_C MIR_APP_DLL(void) UnregisterSrmmLog(HANDLE);

/////////////////////////////////////////////////////////////////////////////////////////
// Standard built-in RTF logger class

class MIR_APP_EXPORT CRtfLogWindow : public CSrmmLogWindow
{
protected:
	CCtrlRichEdit &m_rtf;

public:
	CRtfLogWindow(CMsgDialog &pDlg);
	~CRtfLogWindow() override;

	virtual INT_PTR WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

	////////////////////////////////////////////////////////////////////////////////////////
	void     Attach() override;
	void     Detach() override;
		      
	bool     AtBottom() override;
	void     Clear() override;
	HWND     GetHwnd() override;
	wchar_t* GetSelection() override;
	int      GetType() override;
	void     Resize() override;
	void     ScrollToBottom() override;

	INT_PTR Notify(WPARAM, LPARAM) override;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Basic SRMM window dialog

#include <chat_resource.h>

// message procedures' stubs
EXTERN_C MIR_APP_DLL(LRESULT) CALLBACK stubLogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
EXTERN_C MIR_APP_DLL(LRESULT) CALLBACK stubMessageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
EXTERN_C MIR_APP_DLL(LRESULT) CALLBACK stubNicklistProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

class MIR_APP_EXPORT CSrmmBaseDialog : public CDlgBase
{
	friend class CRtfLogWindow;

	CSrmmBaseDialog(const CSrmmBaseDialog &) = delete;
	CSrmmBaseDialog &operator=(const CSrmmBaseDialog &) = delete;

protected:
	CSrmmBaseDialog(CMPluginBase &pPlugin, int idDialog, struct SESSION_INFO *si = nullptr);

	bool OnInitDialog() override;
	void OnDestroy() override;

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

	bool AllowTyping() const;
	int  NotifyEvent(int code);
	#ifdef _WINDOWS
	bool ProcessFileDrop(HDROP hDrop, MCONTACT hContact);
	bool PasteFilesAsURL(HDROP hDrop);
	#endif
	bool ProcessHotkeys(int key, bool bShift, bool bCtrl, bool bAlt);
	void RefreshButtonStatus(void);
	void RunUserMenu(HWND hwndOwner, struct USERINFO *ui, const POINT &pt);

protected:
	CSrmmLogWindow *m_pLog = nullptr;
	CCtrlRichEdit m_message;
	SESSION_INFO *m_si;
	COLORREF m_clrInputBG, m_clrInputFG;
	time_t m_iLastEnterTime;

	// user typing support;
	uint32_t m_nLastTyping = 0;
	uint8_t m_bShowTyping = 0;
	int m_nTypeSecs = 0, m_nTypeMode = 0;
	const USERINFO* m_pUserTyping = nullptr;

	CCtrlListBox m_nickList;
	CCtrlButton m_btnColor, m_btnBkColor;
	CCtrlButton m_btnBold, m_btnItalic, m_btnUnderline;
	CCtrlButton m_btnHistory, m_btnChannelMgr, m_btnNickList, m_btnFilter;

	void onClick_BIU(CCtrlButton *);
	void onClick_Color(CCtrlButton *);
	void onClick_BkColor(CCtrlButton *);

	void onClick_ChanMgr(CCtrlButton *);
	void onClick_History(CCtrlButton *);

	void onDblClick_List(CCtrlListBox *);

public:
	MCONTACT m_hContact;
	int m_iLogFilterFlags;
	bool m_bFilterEnabled, m_bNicklistEnabled;
	bool m_bFGSet, m_bBGSet;
	bool m_bInMenu;
	COLORREF m_iFG, m_iBG;
	CTimer timerFlash, timerType;

	void ClearLog();
	void RedrawLog();
	void ShowColorChooser(int iCtrlId);

	virtual void AddLog();
	virtual void CloseTab() {}
	virtual bool IsActive() const PURE;
	virtual void LoadSettings() PURE;
	virtual void SetStatusText(const wchar_t *, HICON) {}
	virtual void ShowFilterMenu() {}
	virtual void UpdateNickList() {}
	virtual void UpdateOptions();
	virtual void UpdateStatusBar() {}
	virtual void UpdateTitle() PURE;

	virtual LRESULT WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT WndProc_Nicklist(UINT msg, WPARAM wParam, LPARAM lParam);

	__forceinline bool isChat() const { return m_si != nullptr; }
	__forceinline SESSION_INFO *getChat() const { return m_si; }
	__forceinline CSrmmLogWindow *log() const { return m_pLog; }
	
	__forceinline void setTyping(int nSecs, const USERINFO* pUser = nullptr) {
		m_pUserTyping = pUser;
		m_nTypeSecs = nSecs;
	}

	__inline void* operator new(size_t size) { return calloc(1, size); }
	__inline void operator delete(void *p) { free(p); }
};

#ifndef SRMM_OWN_STRUCTURES
class CMsgDialog : public CSrmmBaseDialog {};
#endif 

/////////////////////////////////////////////////////////////////////////////////////////
// receives LOGSTREAMDATA* as the first parameter

EXTERN_C MIR_APP_DLL(DWORD) CALLBACK Srmm_LogStreamCallback(DWORD_PTR dwCookie, uint8_t *pbBuff, LONG cb, LONG *pcb);

/////////////////////////////////////////////////////////////////////////////////////////
// sends a message to all SRMM windows

EXTERN_C MIR_APP_DLL(void) Srmm_Broadcast(UINT, WPARAM, LPARAM);

/////////////////////////////////////////////////////////////////////////////////////////
// finds a SRMM window using hContact

EXTERN_C MIR_APP_DLL(HWND) Srmm_FindWindow(MCONTACT hContact);
EXTERN_C MIR_APP_DLL(CMsgDialog*) Srmm_FindDialog(MCONTACT hContact);

#endif // M_MESSAGE_H__
