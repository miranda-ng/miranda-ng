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
#include <m_srmm_int.h>

#include "cmdlist.h"
#include "msgs.h"
#include "globals.h"
#include "version.h"

#define EM_ACTIVATE   (WM_USER+0x102)

#define GC_SWITCHNEXTTAB (WM_USER+0x103)
#define GC_SWITCHPREVTAB (WM_USER+0x104)
#define GC_TABCHANGE     (WM_USER+0x105)
#define GC_SWITCHTAB     (WM_USER+0x106)

extern HINSTANCE g_hInst;

/////////////////////////////////////////////////////////////////////////////////////////

struct MODULEINFO : public GCModuleInfoBase
{
	HICON hOnlineIcon, hOnlineTalkIcon;
	HICON hOfflineIcon, hOfflineTalkIcon;

	int OnlineIconIndex;
	int OfflineIconIndex;
};

struct SESSION_INFO : public GCSessionInfoBase {};
struct LOGSTREAMDATA : public GCLogStreamDataBase {};

struct GlobalLogSettings : public GlobalLogSettingsBase
{
	int iX, iY;
	int iSplitterX, iSplitterY;
	bool bTabsEnable, bTabsAtBottom, bTabCloseOnDblClick, bTabRestore;

	HFONT MessageAreaFont;
	COLORREF MessageAreaColor;
};

/////////////////////////////////////////////////////////////////////////////////////////

extern GlobalLogSettings g_Settings;
extern HMENU g_hMenu;
extern HIMAGELIST hIconsList;

extern HINSTANCE g_hInst;
extern BOOL SmileyAddInstalled, PopupInstalled;
extern int g_iMessageIconIndex;

// main.cpp
void LoadIcons(void);
void Unload_ChatModule(void);
void Load_ChatModule(void);

// log.cpp
char* Log_CreateRtfHeader(void);

// window.cpp
SESSION_INFO* SM_GetPrevWindow(SESSION_INFO *si);
SESSION_INFO* SM_GetNextWindow(SESSION_INFO *si);

// options.cpp
void AddIcons(void);
HICON LoadIconEx(const char *pszIcoLibName, bool big);
HANDLE GetIconHandle(const char *pszIcolibName);

// tools.cpp
void SetButtonsPos(HWND hwndDlg, bool bIsChat);

/////////////////////////////////////////////////////////////////////////////////////////
// tabs.cpp

class CTabbedWindow : public CDlgBase
{
	void SaveWindowPosition(bool bUpdateSession);
	void SetWindowPosition();

	int iX, iY;
	int iWidth, iHeight;
	int m_windowWasCascaded;

public:
	CCtrlPages m_tab;
	HWND m_hwndStatus;
	CMsgDialog *m_pEmbed;

	CTabbedWindow();

	CTabbedWindow* AddPage(MCONTACT hContact, wchar_t *pwszText = nullptr, int iActivate = -1);
	void AddPage(SESSION_INFO*, int insertAt = -1);
	void FixTabIcons(CSrmmBaseDialog*);
	void SetMessageHighlight(CChatRoomDlg*);
	void SetTabHighlight(CChatRoomDlg*);
	void TabClicked();

	virtual void OnInitDialog() override;
	virtual void OnDestroy() override;

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual int Resizer(UTILRESIZECONTROL *urc) override;
};

extern CTabbedWindow *pDialog;

void UninitTabs(void);
CTabbedWindow* GetContainer();

void TB_SaveSession(SESSION_INFO *si);

#pragma comment(lib,"comctl32.lib")

/////////////////////////////////////////////////////////////////////////////////////////

#define GC_ADDTAB		  (WM_USER+200)
#define GC_REMOVETAB   (WM_USER+201)
#define GC_DROPPEDTAB  (WM_USER+202)
#define GC_RENAMETAB   (WM_USER+203)

class CChatRoomDlg : public CMsgDialog
{
	typedef CMsgDialog CSuper;
	friend class CTabbedWindow;

	static INT_PTR CALLBACK FilterWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual LRESULT WndProc_Log(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual LRESULT WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual LRESULT WndProc_Nicklist(UINT msg, WPARAM wParam, LPARAM lParam) override;

	wchar_t szTabSave[20];

	CCtrlButton m_btnOk;
	CSplitter m_splitterX, m_splitterY;
	CTabbedWindow *m_pOwner;

	int m_iSplitterX, m_iSplitterY;

	void onActivate(void);

public:
	CChatRoomDlg(CTabbedWindow*, SESSION_INFO*);

	virtual void OnInitDialog() override;
	virtual void OnDestroy() override;

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual int Resizer(UTILRESIZECONTROL *urc) override;
	
	virtual void CloseTab() override;
	virtual void LoadSettings() override;
	virtual void RedrawLog() override;
	virtual void StreamInEvents(LOGINFO *lin, bool bRedraw) override;
	virtual void ScrollToBottom() override;
	virtual void ShowFilterMenu() override;
	virtual void UpdateNickList() override;
	virtual void UpdateOptions() override;
	virtual void UpdateStatusBar() override;
	virtual void UpdateTitle() override;

	void onClick_Ok(CCtrlButton*);

	void onClick_Filter(CCtrlButton*);
	void onClick_NickList(CCtrlButton*);

	void onSplitterX(CSplitter*);
	void onSplitterY(CSplitter*);
};
