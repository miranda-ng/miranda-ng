/*

Copyright 2000-12 Miranda IM, 2012-22 Miranda NG team,
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

#define HM_DBEVENTADDED      (WM_USER+12)
#define DM_OPTIONSAPPLIED    (WM_USER+14)
#define DM_UPDATETITLE       (WM_USER+16)
#define DM_NEWTIMEZONE       (WM_USER+18)
#define HM_AVATARACK         (WM_USER+28)
#define DM_STATUSICONCHANGE  (WM_USER+31)

#define EVENTTYPE_JABBER_CHATSTATES     2000
#define EVENTTYPE_JABBER_PRESENCE       2001

class CLogWindow : public CRtfLogWindow
{
	typedef CRtfLogWindow CSuper;

public:
	CLogWindow(CMsgDialog &pDlg) :
		CSuper(pDlg)
	{}

	void Attach() override;
	void LogEvents(MEVENT hDbEventFirst, int count, bool bAppend) override;
	void LogEvents(struct LOGINFO *, bool) override;
	void UpdateOptions() override;

	INT_PTR WndProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
};

class CMsgDialog : public CSrmmBaseDialog
{
	friend class CLogWindow;
	friend class CTabbedWindow;
	typedef CSrmmBaseDialog CSuper;

	void Init(void);
	void NotifyTyping(int mode);
	void SetButtonsPos(void);
	void ShowAvatar(void);
	void ShowTime(bool bForce);
	void SetupStatusBar(void);
	void UpdateIcon(WPARAM wParam);
	void UpdateLastMessage(void);
	void UpdateSizeBar(void);

	static INT_PTR CALLBACK FilterWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	CCtrlBase m_avatar;
	
	void OnFlash(CTimer *);
	void OnType(CTimer *);

	CTabbedWindow *m_pOwner;
	uint32_t m_nFlash = 0;
	char *m_szProto = nullptr;

	// splitters
	CSplitter m_splitterX, m_splitterY;
	int  m_iSplitterX, m_iSplitterY;
	SIZE m_minEditBoxSize;
	RECT m_minEditInit;
	RECT m_rcLog;

	// tab autocomplete
	int m_iTabStart = 0;
	wchar_t m_szTabSave[20];
	void TabAutoComplete();

	// entered messages
	int m_cmdListInd = 0;
	LIST<wchar_t> m_cmdList;

	HFONT m_hFont = nullptr;

	int m_limitAvatarH = 0;
	uint32_t m_lastMessage = 0;
	HANDLE m_hTimeZone = 0;
	uint16_t m_wStatus = ID_STATUS_OFFLINE, m_wOldStatus = ID_STATUS_OFFLINE;
	uint16_t m_wMinute = 0;
	bool m_bIsMeta = false, m_bWindowCascaded = false, m_bNoActivate = false;
	int m_iBBarHeight = 28;

public:
	CMsgDialog(CTabbedWindow *pOwner, MCONTACT hContact);
	CMsgDialog(CTabbedWindow *pOwner, SESSION_INFO *si);

	bool OnInitDialog() override;
	void OnDestroy() override;
	int Resizer(UTILRESIZECONTROL *urc) override;

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
	LRESULT WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam) override;
	LRESULT WndProc_Nicklist(UINT msg, WPARAM wParam, LPARAM lParam) override;

	void OnActivate();
	void RemakeLog();
	void UpdateAvatar();
	void UserTyping(int nSecs);

	void onSplitterX(CSplitter *);
	void onSplitterY(CSplitter *);

	void onChange_Text(CCtrlEdit *);

	void onClick_Ok(CCtrlButton *);
	void onClick_Filter(CCtrlButton *);
	void onClick_NickList(CCtrlButton *);

	void OnOptionsApplied(bool bUpdateAvatar);

	void UpdateReadChars(void);

	__forceinline MCONTACT getActiveContact() const {
		return (m_bIsMeta) ? db_mc_getSrmmSub(m_hContact) : m_hContact;
	}

	__forceinline CLogWindow* LOG() {
		return ((CLogWindow *)m_pLog);
	}

	MEVENT m_hDbEventFirst, m_hDbEventLast;

	int m_avatarWidth = 0, m_avatarHeight = 0;

	bool m_bIsAutoRTL = false;
	HBITMAP m_avatarPic = 0;
	wchar_t *m_wszInitialText = 0;

	int GetImageId() const;

	void __forceinline FixTabIcons()
	{	m_pOwner->FixTabIcons(this);
	}

	void CloseTab() override;
	bool IsActive() const override;
	void LoadSettings() override;
	void SetStatusText(const wchar_t *, HICON) override;
	void ShowFilterMenu() override;
	void UpdateNickList() override;
	void UpdateOptions() override;
	void UpdateStatusBar() override;
	void UpdateTitle() override;

	void StartFlash();
	void StopFlash();
};

extern LIST<CMsgDialog> g_arDialogs;

/////////////////////////////////////////////////////////////////////////////////////////

int  DbEventIsForMsgWindow(DBEVENTINFO *dbei);
int  DbEventIsShown(DBEVENTINFO *dbei);
int  SendMessageDirect(const wchar_t *szMsg, MCONTACT hContact);
INT_PTR SendMessageCmd(MCONTACT hContact, wchar_t *msg);

void LoadMsgLogIcons(void);
void FreeMsgLogIcons(void);

int OptInitialise(WPARAM, LPARAM);

#define MSGFONTID_MYMSG       0
#define MSGFONTID_YOURMSG     1
#define MSGFONTID_MYNAME      2
#define MSGFONTID_MYTIME      3
#define MSGFONTID_MYCOLON     4
#define MSGFONTID_YOURNAME    5
#define MSGFONTID_YOURTIME    6
#define MSGFONTID_YOURCOLON   7
#define MSGFONTID_MESSAGEAREA 8
#define MSGFONTID_NOTICE      9

bool LoadMsgDlgFont(int i, LOGFONT* lf, COLORREF* colour);

#define LOADHISTORY_UNREAD    0
#define LOADHISTORY_COUNT     1
#define LOADHISTORY_TIME      2

#define SRMMMOD    "SRMM"
#define SRMSGMOD   "SRMsg"
#define DBSAVEDMSG "SavedMsg"

#define SRMSGSET_TYPING  "SupportTyping"

#define SRMSGSET_BKGCOLOUR         "BkgColour"
#define SRMSGDEFSET_BKGCOLOUR      GetSysColor(COLOR_WINDOW)

#endif
