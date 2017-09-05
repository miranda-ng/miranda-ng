/*

Copyright 2000-12 Miranda IM, 2012-17 Miranda NG project,
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
	void ShowAvatar(void);
	void ShowTime(void);
	void SetupStatusBar(void);
	void StreamInEvents(MEVENT hDbEventFirst, int count, bool bAppend);

	char *m_szProto;
	HFONT m_hFont;
	HBRUSH m_hBkgBrush;

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

#define MSGFONTID_MYMSG		  0
#define MSGFONTID_YOURMSG	  1
#define MSGFONTID_MYNAME	  2
#define MSGFONTID_MYTIME	  3
#define MSGFONTID_MYCOLON	  4
#define MSGFONTID_YOURNAME	  5
#define MSGFONTID_YOURTIME	  6
#define MSGFONTID_YOURCOLON	  7
#define MSGFONTID_MESSAGEAREA 8
#define MSGFONTID_NOTICE      9

bool LoadMsgDlgFont(int i, LOGFONT* lf, COLORREF* colour);

#define LOADHISTORY_UNREAD    0
#define LOADHISTORY_COUNT     1
#define LOADHISTORY_TIME      2

#define SRMMMOD 	"SRMM"
#define SRMSGMOD 	"SRMsg"
#define DBSAVEDMSG 	"SavedMsg"

#define SRMSGSET_DONOTSTEALFOCUS   "DoNotStealFocus"
#define SRMSGDEFSET_DONOTSTEALFOCUS 0
#define SRMSGSET_POPFLAGS          "PopupFlags"
#define SRMSGDEFSET_POPFLAGS       0
#define SRMSGSET_SHOWBUTTONLINE    "ShowButtonLine"
#define SRMSGDEFSET_SHOWBUTTONLINE 1
#define SRMSGSET_SHOWINFOLINE      "ShowInfoLine"
#define SRMSGDEFSET_SHOWINFOLINE   1
#define SRMSGSET_AUTOMIN           "AutoMin"
#define SRMSGDEFSET_AUTOMIN        0
#define SRMSGSET_AUTOCLOSE         "AutoClose"
#define SRMSGDEFSET_AUTOCLOSE      0
#define SRMSGSET_SAVEPERCONTACT    "SavePerContact"
#define SRMSGDEFSET_SAVEPERCONTACT 0
#define SRMSGSET_CASCADE           "Cascade"
#define SRMSGDEFSET_CASCADE        1
#define SRMSGSET_SENDONENTER       "SendOnEnter"
#define SRMSGDEFSET_SENDONENTER    1
#define SRMSGSET_SENDONDBLENTER    "SendOnDblEnter"
#define SRMSGDEFSET_SENDONDBLENTER 0
#define SRMSGSET_STATUSICON        "UseStatusWinIcon"
#define SRMSGDEFSET_STATUSICON     0
#define SRMSGSET_SENDBUTTON        "UseSendButton"
#define SRMSGDEFSET_SENDBUTTON     0
#define SRMSGSET_CHARCOUNT         "ShowCharCount"
#define SRMSGDEFSET_CHARCOUNT      0
#define SRMSGSET_CTRLSUPPORT       "SupportCtrlUpDn"
#define SRMSGDEFSET_CTRLSUPPORT    1
#define SRMSGSET_DELTEMP           "DeleteTempCont"
#define SRMSGDEFSET_DELTEMP        0
#define SRMSGSET_MSGTIMEOUT        "MessageTimeout"
#define SRMSGDEFSET_MSGTIMEOUT     65000
#define SRMSGSET_MSGTIMEOUT_MIN    5000 // minimum value (5 seconds)
#define SRMSGSET_FLASHCOUNT        "FlashMax"
#define SRMSGDEFSET_FLASHCOUNT     5
#define SRMSGSET_BUTTONGAP         "ButtonsBarGap"
#define SRMSGDEFSET_BUTTONGAP      1 

#define SRMSGSET_LOADHISTORY       "LoadHistory"
#define SRMSGDEFSET_LOADHISTORY    LOADHISTORY_UNREAD
#define SRMSGSET_LOADCOUNT         "LoadCount"
#define SRMSGDEFSET_LOADCOUNT      10
#define SRMSGSET_LOADTIME          "LoadTime"
#define SRMSGDEFSET_LOADTIME       10

#define SRMSGSET_SHOWLOGICONS      "ShowLogIcon"
#define SRMSGDEFSET_SHOWLOGICONS   1
#define SRMSGSET_HIDENAMES         "HideNames"
#define SRMSGDEFSET_HIDENAMES      1
#define SRMSGSET_SHOWTIME          "ShowTime"
#define SRMSGDEFSET_SHOWTIME       1
#define SRMSGSET_SHOWSECS          "ShowSeconds"
#define SRMSGDEFSET_SHOWSECS       1
#define SRMSGSET_SHOWDATE          "ShowDate"
#define SRMSGDEFSET_SHOWDATE       0
#define SRMSGSET_SHOWSTATUSCH      "ShowStatusChanges"
#define SRMSGDEFSET_SHOWSTATUSCH   1
#define SRMSGSET_SHOWFORMAT        "ShowFormatting"
#define SRMSGDEFSET_SHOWFORMAT     1
#define SRMSGSET_BKGCOLOUR         "BkgColour"
#define SRMSGDEFSET_BKGCOLOUR      GetSysColor(COLOR_WINDOW)

#define SRMSGSET_TYPING             "SupportTyping"
#define SRMSGSET_TYPINGNEW          "DefaultTyping"
#define SRMSGDEFSET_TYPINGNEW       1
#define SRMSGSET_TYPINGUNKNOWN      "UnknownTyping"
#define SRMSGDEFSET_TYPINGUNKNOWN   0
#define SRMSGSET_SHOWTYPING         "ShowTyping"
#define SRMSGDEFSET_SHOWTYPING      1
#define SRMSGSET_SHOWTYPINGWIN      "ShowTypingWin"
#define SRMSGDEFSET_SHOWTYPINGWIN   1
#define SRMSGSET_SHOWTYPINGNOWIN    "ShowTypingTray"
#define SRMSGDEFSET_SHOWTYPINGNOWIN 0
#define SRMSGSET_SHOWTYPINGCLIST    "ShowTypingClist"
#define SRMSGDEFSET_SHOWTYPINGCLIST 1


#define SRMSGSET_AVATARENABLE       "AvatarEnable"
#define SRMSGDEFSET_AVATARENABLE    1
#define SRMSGSET_LIMITAVHEIGHT      "AvatarLimitHeight"
#define SRMSGDEFSET_LIMITAVHEIGHT   1
#define SRMSGSET_AVHEIGHT          "AvatarHeight"
#define SRMSGDEFSET_AVHEIGHT        60

#endif
