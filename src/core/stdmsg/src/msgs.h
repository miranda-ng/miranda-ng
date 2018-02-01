/*

Copyright 2000-12 Miranda IM, 2012-18 Miranda NG team,
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

#include <richedit.h>
#include <richole.h>

#define DM_REMAKELOG         (WM_USER+11)
#define HM_DBEVENTADDED      (WM_USER+12)
#define DM_CASCADENEWWINDOW  (WM_USER+13)
#define DM_OPTIONSAPPLIED    (WM_USER+14)
#define DM_CLOSETAB          (WM_USER+15)
#define DM_UPDATETITLE       (WM_USER+16)
#define DM_APPENDTOLOG       (WM_USER+17)
#define DM_NEWTIMEZONE       (WM_USER+18)
#define DM_TYPING            (WM_USER+20)
#define DM_UPDATEWINICON     (WM_USER+21)
#define DM_UPDATELASTMESSAGE (WM_USER+22)
#define DM_USERNAMETOCLIP    (WM_USER+23)
#define DM_AVATARSIZECHANGE  (WM_USER+24)
#define DM_AVATARCALCSIZE    (WM_USER+25)
#define DM_GETAVATAR         (WM_USER+26)
#define DM_UPDATESIZEBAR     (WM_USER+27)
#define HM_AVATARACK         (WM_USER+28)
#define DM_STATUSICONCHANGE  (WM_USER+31)

#define EVENTTYPE_JABBER_CHATSTATES     2000
#define EVENTTYPE_JABBER_PRESENCE       2001

class CMsgDialog : public CSrmmBaseDialog
{
	typedef CSrmmBaseDialog CSuper;
	friend class CTabbedWindow;

protected:
	CCtrlButton m_btnOk;

	CMsgDialog(int idDialog, SESSION_INFO *si = nullptr);
};

class CSrmmWindow : public CMsgDialog
{
	friend class CTabbedWindow;
	typedef CMsgDialog CSuper;
	
	static LRESULT CALLBACK TabSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	virtual LRESULT WndProc_Log(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual LRESULT WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam) override;

	CCtrlBase m_avatar;
	CSplitter m_splitter;

	void NotifyTyping(int mode);
	void ProcessFileDrop(HDROP hDrop);
	void ShowAvatar(void);
	void ShowTime(void);
	void SetupStatusBar(void);
	void StreamInEvents(MEVENT hDbEventFirst, int count, bool bAppend);

	char *m_szProto;
	HICON m_hStatusIcon = nullptr;
	HFONT m_hFont = nullptr;
	HBRUSH m_hBkgBrush = nullptr;

	SIZE m_minEditBoxSize;
	RECT m_minEditInit;

	int m_windowWasCascaded;
	DWORD m_nFlash;
	int m_nTypeSecs, m_nTypeMode;
	int m_limitAvatarH;
	DWORD m_nLastTyping;
	DWORD m_lastMessage;
	HANDLE m_hTimeZone;
	WORD m_wStatus, m_wOldStatus;
	WORD m_wMinute;
	bool m_bIsMeta, m_bShowTyping;

public:
	bool m_bIsAutoRTL, m_bNoActivate;
	MEVENT m_hDbEventFirst, m_hDbEventLast;

	int m_avatarWidth, m_avatarHeight;
	int m_splitterPos, m_originalSplitterPos;
	int m_lineHeight;

	int m_cmdListInd;
	LIST<wchar_t> m_cmdList;

	HBITMAP m_avatarPic;
	wchar_t *m_wszInitialText;
	CTabbedWindow *m_pOwner;

public:
	CSrmmWindow(CTabbedWindow*, MCONTACT hContact);

	virtual void OnInitDialog() override;
	virtual void OnDestroy() override;

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual int Resizer(UTILRESIZECONTROL *urc) override;
	
	virtual void CloseTab() override;
	virtual void LoadSettings() override {}
	virtual void ScrollToBottom() override;
	virtual void SetStatusText(const wchar_t*, HICON) override;
	virtual void UpdateTitle() override {}

	void OnSplitterMoved(CSplitter*);

	void onClick_Ok(CCtrlButton*);

	void OnOptionsApplied(bool bUpdateAvatar);

	void UpdateReadChars(void);

	__forceinline MCONTACT getActiveContact() const
	{	return (m_bIsMeta) ? db_mc_getSrmmSub(m_hContact) : m_hContact;
	}
};

INT_PTR CALLBACK ErrorDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
int  DbEventIsForMsgWindow(DBEVENTINFO *dbei);
int  DbEventIsShown(DBEVENTINFO *dbei);
int  SendMessageDirect(const wchar_t *szMsg, MCONTACT hContact);
INT_PTR SendMessageCmd(MCONTACT hContact, wchar_t *msg);

void LoadMsgLogIcons(void);
void FreeMsgLogIcons(void);

void InitOptions(void);

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
