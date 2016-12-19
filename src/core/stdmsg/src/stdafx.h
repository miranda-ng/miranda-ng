/*

Copyright 2000-12 Miranda IM, 2012-16 Miranda NG project,
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

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <ole2.h>
#include <shellapi.h>
#include <shlobj.h>
#include <commctrl.h>
#include <vssym32.h>

#include <Initguid.h>
#include <Oleacc.h>
#include <Uxtheme.h>

#include <malloc.h>
#include <time.h>

#include "resource.h"

#include <win2k.h>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_button.h>
#include <m_clist.h>
#include <m_clc.h>
#include <m_gui.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_utils.h>
#include <m_skin.h>
#include <m_contacts.h>
#include <m_userinfo.h>
#include <m_history.h>
#include <m_addcontact.h>
#include <m_chat_int.h>
#include <m_message.h>
#include <m_file.h>
#include <m_icolib.h>
#include <m_fontservice.h>
#include <m_timezones.h>
#include <m_avatars.h>
#include <m_metacontacts.h>
#include <m_ieview.h>
#include <m_smileyadd.h>
#include <m_popup.h>

#include "cmdlist.h"
#include "msgs.h"
#include "globals.h"
#include "richutil.h"
#include "version.h"

#define EM_SUBCLASSED (WM_USER+0x101)
#define EM_ACTIVATE   (WM_USER+0x102)

extern HINSTANCE g_hInst;
extern HCURSOR hCurSplitNS, hCurSplitWE, hCurHyperlinkHand;
extern HANDLE hHookWinEvt, hHookWinPopup, hHookWinWrite;
extern CREOleCallback reOleCallback;

/////////////////////////////////////////////////////////////////////////////////////////

struct MODULEINFO : public GCModuleInfoBase
{
	int OnlineIconIndex;
	int OfflineIconIndex;
};

struct SESSION_INFO : public GCSessionInfoBase
{
	int iX, iY;
	class CChatRoomDlg *pDlg;
};

struct LOGSTREAMDATA : public GCLogStreamDataBase {};

struct GlobalLogSettings : public GlobalLogSettingsBase
{
	int   iX, iY;
	bool  bTabsEnable, bTabsAtBottom, bTabCloseOnDblClick, bTabRestore;

	HFONT MessageAreaFont;
	COLORREF MessageAreaColor;
};

/////////////////////////////////////////////////////////////////////////////////////////

extern GlobalLogSettings g_Settings;
extern CHAT_MANAGER saveCI;
extern HMENU g_hMenu;
extern HIMAGELIST hIconsList;

extern HINSTANCE g_hInst;
extern BOOL SmileyAddInstalled, PopupInstalled;

// main.cpp
void LoadIcons(void);
void Unload_ChatModule(void);
void Load_ChatModule(void);

// log.cpp
void  Log_StreamInEvent(HWND hwndDlg, LOGINFO *lin, SESSION_INFO *si, BOOL bRedraw);
void  ValidateFilename(wchar_t *filename);
char* Log_CreateRtfHeader(MODULEINFO *mi);

// window.cpp
int GetTextPixelSize(wchar_t* pszText, HFONT hFont, BOOL bWidth);

SESSION_INFO* SM_GetPrevWindow(SESSION_INFO *si);
SESSION_INFO* SM_GetNextWindow(SESSION_INFO *si);

// options.cpp
void AddIcons(void);
HICON LoadIconEx(const char *pszIcoLibName, bool big);
HANDLE GetIconHandle(const char *pszIcolibName);

// services.cpp
void ShowRoom(SESSION_INFO *si);

// tools.cpp
int  GetColorIndex(const char* pszModule, COLORREF cr);
void CheckColorsInModule(const char* pszModule);
int  GetRichTextLength(HWND hwnd);
UINT CreateGCMenu(HWND hwndDlg, HMENU *hMenu, int iIndex, POINT pt, SESSION_INFO *si, wchar_t* pszUID, wchar_t* pszWordText);
void DestroyGCMenu(HMENU *hMenu, int iIndex);
bool LoadMessageFont(LOGFONT *lf, COLORREF *colour);
void SetButtonsPos(HWND hwndDlg, bool bIsChat);
int  RestoreWindowPosition(HWND hwnd, MCONTACT hContact, bool bHide);


// message.cpp
char* Message_GetFromStream(HWND hwndDlg, SESSION_INFO *si);
void  NotifyLocalWinEvent(MCONTACT hContact, HWND hwnd, unsigned int type);

// tabs.cpp
void InitTabs(void);
void UninitTabs(void);

void TB_SaveSession(SESSION_INFO *si);

#pragma comment(lib,"comctl32.lib")

/////////////////////////////////////////////////////////////////////////////////////////

#define GC_ADDTAB		  (WM_USER+200)
#define GC_REMOVETAB   (WM_USER+201)
#define GC_DROPPEDTAB  (WM_USER+202)
#define GC_RENAMETAB   (WM_USER+203)

class CChatRoomDlg : public CDlgBase
{
	friend struct CTabbedWindow;

	SESSION_INFO *m_si;

	CCtrlEdit m_message, m_log;
	CCtrlListBox m_nickList;
	CCtrlButton m_btnOk;
	CCtrlButton m_btnFilter, m_btnChannelMgr, m_btnNickList, m_btnHistory;
	CCtrlButton m_btnBold, m_btnItalic, m_btnUnderline, m_btnColor, m_btnBkColor;

	HWND getCaptionWindow() const
	{	return (g_Settings.bTabsEnable) ? GetParent(m_hwndParent) : m_hwnd;
	}

	void SaveWindowPosition(bool bUpdateSession);
	void SetWindowPosition();

public:
	CChatRoomDlg(SESSION_INFO*);

	virtual void OnInitDialog() override;
	virtual void OnDestroy() override;

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

	void OnClick_Bold(CCtrlButton*);
	void OnClick_Color(CCtrlButton*);
	void OnClick_BkColor(CCtrlButton*);

	void OnClick_Ok(CCtrlButton*);

	void OnClick_Filter(CCtrlButton*);
	void OnClick_History(CCtrlButton*);
	void OnClick_Options(CCtrlButton*);
	void OnClick_NickList(CCtrlButton*);

	void OnListDblclick(CCtrlListBox*);
};
