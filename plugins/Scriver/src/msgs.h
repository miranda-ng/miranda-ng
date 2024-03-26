/*
Scriver

Copyright (c) 2000-12 Miranda ICQ/IM project,

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
#ifndef SRMM_MSGS_H
#define SRMM_MSGS_H

/////////////////////////////////////////////////////////////////////////////////////////

#define MSGERROR_CANCEL	0
#define MSGERROR_RETRY	1
#define MSGERROR_DONE	2

struct ToolbarButton
{
	wchar_t *name;
	UINT controlId;
	int alignment;
	int spacing;
	int width;
};

#define NMWLP_INCOMING 1

class CMsgDialog : public CSrmmBaseDialog
{
	typedef CSrmmBaseDialog CSuper;

	friend class CLogWindow;
	friend struct ParentWindowData;
	friend INT_PTR CALLBACK InfobarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void   ClearLog(void);
	HICON  GetTabIcon(void);
	void   GetTitlebarIcon(struct TitleBarData *tbd);
	void   Init(void);
	int    InputAreaShortcuts(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void   MessageDialogResize(int w, int h);
	void   MessageSend(const SendQueue::Item &msg);
	void   ShowAvatar(void);
	void   SetDialogToType(void);
	void   SetStatusIcon(void);
	void   ToggleRtl();
	void   UpdateIcon(void);
	void   UpdateReadChars(void);

	bool   IsTypingNotificationEnabled(void);
	bool   IsTypingNotificationSupported(void);
	void   NotifyTyping(int mode);

	static INT_PTR CALLBACK FilterWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	bool   m_bIncoming, m_bWindowCascaded;

	MEVENT m_hDbUnreadEventFirst;
	int    m_minLogBoxHeight, m_minEditBoxHeight;
	int    m_iShowUnread;
	uint16_t   m_wStatus;
	uint32_t  m_lastMessage;
	int    m_iMessagesInProgress;
	int    m_iSendAllConfirm;
	HICON  m_hStatusIcon, m_hStatusIconBig, m_hStatusIconOverlay;

	char  *m_szProto;
	bool   m_bUseRtl;

	HBITMAP m_hbmpAvatarPic;
	AVATARCACHEENTRY *m_ace;

	TCmdList *cmdList, *cmdListCurrent;
	ParentWindowData *m_pParent;

	// info bar support
	HWND   m_hwndInfo;
	HWND   m_hXStatusTip;

	void   CreateInfobar();
	void   SetupInfobar();
	void   RefreshInfobar();

	// autocomplete
	void   TabAutoComplete(void);
	wchar_t *m_wszSearchQuery, *m_wszSearchResult;
	SESSION_INFO *m_pLastSession;

	CCtrlButton m_btnAdd, m_btnUserMenu, m_btnQuote, m_btnDetails;
	CSplitter m_splitterX, m_splitterY;

public:
	CMsgDialog(MCONTACT hContact, bool bIncoming);
	CMsgDialog(SESSION_INFO *);

	bool OnInitDialog() override;
	void OnDestroy() override;

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

	void onClick_Ok(CCtrlButton *);
	void onClick_Add(CCtrlButton *);
	void onClick_Filter(CCtrlButton *);
	void onClick_Details(CCtrlButton *);
	void onClick_Quote(CCtrlButton *);
	void onClick_UserMenu(CCtrlButton *);
	void onClick_ShowList(CCtrlButton *);

	void onChange_Message(CCtrlEdit *);

	void onChange_SplitterX(CSplitter *);
	void onChange_SplitterY(CSplitter *);

	void onType(CTimer *);

	void CloseTab() override;
	void DrawNickList(USERINFO *ui, DRAWITEMSTRUCT *dis) override;
	void EventAdded(MEVENT, const DB::EventInfo &dbei) override;
	bool GetFirstEvent() override;
	void LoadSettings() override;
	void OnOptionsApplied() override;
	void RemakeLog() override;
	void SetStatusText(const wchar_t *, HICON) override;
	void ShowFilterMenu() override;
	void UpdateFilterButton() override;
	void UpdateStatusBar() override;
	void UpdateTitle() override;

	void HandleError(bool bRetry, SendQueue::Item *pItem);
	void ShowError(const wchar_t *pwszMsg, SendQueue::Item *pItem);

	void FixTabIcons();
	void GetAvatar();
	void SwitchTyping(void);
	void UpdateTabControl(void);
	void UserIsTyping(int iState);

	void StartMessageSending(void);
	void StopMessageSending(void);
	void ShowMessageSending(void);
	
	LRESULT WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam) override;
	LRESULT WndProc_Nicklist(UINT msg, WPARAM wParam, LPARAM lParam) override;

	__forceinline bool IsActive() const {
		return GetActiveWindow() == m_hwndParent && GetForegroundWindow() == m_hwndParent && m_pParent->m_hwndActive == m_hwnd;
	}

	__forceinline void PopupWindow(bool bIncoming = false) {
		m_pParent->PopupWindow(this, bIncoming);
	}

	__forceinline void StartFlashing() const {
		m_pParent->StartFlashing();
	}

	wchar_t *m_wszInitialText;

	void Reattach(HWND hwndContainer);
};

#define HM_ACKEVENT            (WM_USER+11)

#define DM_REMAKELOG           (WM_USER+12)
#define DM_CASCADENEWWINDOW    (WM_USER+13)
#define DM_OPTIONSAPPLIED      (WM_USER+14)
#define DM_CHANGEICONS         (WM_USER+24)

#define DM_STATUSICONCHANGE    (WM_USER+56)

#define DM_AVATARCHANGED       (WM_USER+64)

#define EM_SUBCLASSED          (WM_USER+0x101)
#define EM_UNSUBCLASSED        (WM_USER+0x102)

#define EVENTTYPE_JABBER_CHATSTATES	2000
#define EVENTTYPE_JABBER_PRESENCE	2001

bool DbEventIsShown(const DB::EventInfo &dbei);
void LoadMsgLogIcons(void);
void FreeMsgLogIcons(void);
int IsAutoPopup(MCONTACT hContact);

#define MSGFONTID_MYMSG           0
#define MSGFONTID_YOURMSG         1
#define MSGFONTID_MYNAME          2
#define MSGFONTID_MYTIME          3
#define MSGFONTID_MYCOLON         4
#define MSGFONTID_YOURNAME        5
#define MSGFONTID_YOURTIME        6
#define MSGFONTID_YOURCOLON       7
#define MSGFONTID_MESSAGEAREA     8
#define MSGFONTID_NOTICE          9
#define MSGFONTID_INFOBAR_NAME   10
#define MSGFONTID_INFOBAR_STATUS 11

void LoadMsgDlgFont(int i, LOGFONT *lf, COLORREF *colour);
extern int fontOptionsListSize;

#define LOADHISTORY_UNREAD		0
#define LOADHISTORY_COUNT		1
#define LOADHISTORY_TIME		2

#define SRMM_MODULE						"SRMM"

#define SRMSGSET_BKGCOLOUR				"BkgColour"
#define SRMSGDEFSET_BKGCOLOUR			GetSysColor(COLOR_WINDOW)
#define SRMSGSET_INPUTBKGCOLOUR			"InputBkgColour"
#define SRMSGDEFSET_INPUTBKGCOLOUR		GetSysColor(COLOR_WINDOW)
#define SRMSGSET_INCOMINGBKGCOLOUR		"IncomingBkgColour"
#define SRMSGDEFSET_INCOMINGBKGCOLOUR	GetSysColor(COLOR_WINDOW)
#define SRMSGSET_OUTGOINGBKGCOLOUR		"OutgoingBkgColour"
#define SRMSGDEFSET_OUTGOINGBKGCOLOUR	GetSysColor(COLOR_WINDOW)
#define SRMSGSET_INFOBARBKGCOLOUR		"InfobarBkgColour"
#define SRMSGDEFSET_INFOBARBKGCOLOUR	GetSysColor(COLOR_3DLIGHT)
#define SRMSGSET_LINECOLOUR				"LineColour"
#define SRMSGDEFSET_LINECOLOUR			GetSysColor(COLOR_WINDOW)

#define SRMSGSET_TYPING				"SupportTyping"

#define SRMSGSET_WINDOWTITLE		"WindowTitle"

#define SRMSGSET_SENDMODE "SendMode"
#define SRMSGDEFSET_SENDMODE 0

#endif
