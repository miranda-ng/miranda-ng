/*
 * Miranda NG: the free IM client for Microsoft* Windows*
 *
 * Copyright (c) 2000-09 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * global include file, used to build the precompiled header.
 *
 */

#ifndef __COMMONHEADERS_H
#define __COMMONHEADERS_H

#define WINVER 0x0600
#define _WIN32_WINNT 0x0600
#define _WIN32_IE 0x0501
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <uxtheme.h>
#include <tom.h>
#include <time.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <assert.h>
#include <mbstring.h>
#include <richedit.h>
#include <richole.h>
#include <string>

#include <msapi\vsstyle.h>
#include <m_avatars.h>
#include <m_message.h>
#include <win2k.h>
#include <newpluginapi.h>
#include <m_imgsrvc.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_button_int.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_contacts.h>
#include <m_icolib.h>
#include <m_clc.h>
#include <m_clui.h>
#include <m_clistint.h>
#include <m_userinfo.h>
#include <m_history.h>
#include <m_addcontact.h>
#include <m_file.h>
#include <m_fontservice.h>
#include <m_acc.h>
#include <m_chat_int.h>
#include <m_protomod.h>
#include <m_hotkeys.h>
#include <m_genmenu.h>
#include <m_popup.h>
#include <m_timezones.h>
#include <m_modernopt.h>
#include <m_xstatus.h>
#include <m_toptoolbar.h>
#include <m_string.h>
#include <m_db_int.h>

#include <m_ieview.h>
#include <m_metacontacts.h>
#include <m_fingerprint.h>
#include <m_nudge.h>
#include <m_folders.h>
#include <m_msg_buttonsbar.h>
#include <m_smileyadd.h>

#define TSAPI __stdcall
#define FASTCALL __fastcall

typedef HANDLE HTHUMBNAIL;
typedef HTHUMBNAIL* PHTHUMBNAIL;

typedef struct _DWM_BLURBEHIND
{
	DWORD dwFlags;
	BOOL fEnable;
	HRGN hRgnBlur;
	BOOL fTransitionOnMaximized;
} DWM_BLURBEHIND, *PDWM_BLURBEHIND;

typedef struct _DWM_THUMBNAIL_PROPERTIES
{
	DWORD dwFlags;
	RECT rcDestination;
	RECT rcSource;
	BYTE opacity;
	BOOL fVisible;
	BOOL fSourceClientAreaOnly;
} DWM_THUMBNAIL_PROPERTIES, *PDWM_THUMBNAIL_PROPERTIES;

#include "m_cln_skinedit.h"

#include "resource.h"
#include "version.h"
#include "buttonbar.h"
#include "msgs.h"
#include "msgdlgutils.h"
#include "typingnotify.h"
#include "generic_msghandlers.h"
#include "nen.h"
#include "functions.h"
#include "chat/chat.h"
#include "contactcache.h"
#include "translator.h"
#include "themes.h"
#include "globals.h"
#include "mim.h"
#include "sendqueue.h"
#include "taskbar.h"
#include "controls.h"
#include "infopanel.h"
#include "sidebar.h"
#include "utils.h"
#include "sendlater.h"
#include "ImageDataObject.h"
#include "chat/muchighlight.h"


/*
* text shadow types (DrawThemeTextEx() / Vista+ uxtheme)
*/
#define TST_NONE			0
#define TST_SINGLE			1
#define TST_CONTINUOUS		2

enum DWMWINDOWATTRIBUTE
{
    DWMWA_NCRENDERING_ENABLED = 1,      // [get] Is non-client rendering enabled/disabled
    DWMWA_NCRENDERING_POLICY,           // [set] Non-client rendering policy
    DWMWA_TRANSITIONS_FORCEDISABLED,    // [set] Potentially enable/forcibly disable transitions
    DWMWA_ALLOW_NCPAINT,                // [set] Allow contents rendered in the non-client area to be visible on the DWM-drawn frame.
    DWMWA_CAPTION_BUTTON_BOUNDS,        // [get] Bounds of the caption button area in window-relative space.
    DWMWA_NONCLIENT_RTL_LAYOUT,         // [set] Is non-client content RTL mirrored
    DWMWA_FORCE_ICONIC_REPRESENTATION,  // [set] Force this window to display iconic thumbnails.
    DWMWA_FLIP3D_POLICY,                // [set] Designates how Flip3D will treat the window.
    DWMWA_EXTENDED_FRAME_BOUNDS,        // [get] Gets the extended frame bounds rectangle in screen space
    DWMWA_HAS_ICONIC_BITMAP,            // [set] Indicates an available bitmap when there is no better thumbnail representation.
    DWMWA_DISALLOW_PEEK,                // [set] Don't invoke Peek on the window.
    DWMWA_EXCLUDED_FROM_PEEK,           // [set] LivePreview exclusion information
    DWMWA_LAST
};

#define DWM_TNP_RECTDESTINATION	0x00000001
#define DWM_TNP_RECTSOURCE 0x00000002
#define DWM_TNP_OPACITY	0x00000004
#define DWM_TNP_VISIBLE	0x00000008
#define DWM_TNP_SOURCECLIENTAREAONLY 0x00000010

#define DWM_SIT_DISPLAYFRAME    0x00000001  // Display a window frame around the provided bitmap


#ifndef BPPF_ERASE
	typedef enum _BP_BUFFERFORMAT
	{
		BPBF_COMPATIBLEBITMAP,    // Compatible bitmap
		BPBF_DIB,                 // Device-independent bitmap
		BPBF_TOPDOWNDIB,          // Top-down device-independent bitmap
		BPBF_TOPDOWNMONODIB       // Top-down monochrome device-independent bitmap
	} BP_BUFFERFORMAT;


	typedef struct _BP_PAINTPARAMS
	{
		DWORD                       cbSize;
		DWORD                       dwFlags; // BPPF_ flags
		const RECT            *     prcExclude;
		const BLENDFUNCTION        * pBlendFunction;
	} BP_PAINTPARAMS, *PBP_PAINTPARAMS;

	#define BPPF_ERASE               1
	#define BPPF_NOCLIP              2
	#define BPPF_NONCLIENT           4
#endif


#define DWM_BB_ENABLE 1

#ifndef LOCALE_SISO3166CTRYNAME2
	#define LOCALE_SISO3166CTRYNAME2      0x00000068   // 3 character ISO country name, eg "USA Vista+
	#define LOCALE_SISO639LANGNAME2       0x00000067   // 3 character ISO abbreviated language name, eg "eng"
#endif

#ifndef WM_DWMCOMPOSITIONCHANGED
	#define WM_DWMCOMPOSITIONCHANGED        0x031E
	#define WM_DWMCOLORIZATIONCOLORCHANGED  0x0320
#endif

#ifndef WM_DWMSENDICONICTHUMBNAIL
	#define WM_DWMSENDICONICTHUMBNAIL           0x0323
	#define WM_DWMSENDICONICLIVEPREVIEWBITMAP   0x0326
#endif

#if !defined(_WIN64) && !defined(_USE_32BIT_TIME_T)
	#define _USE_32BIT_TIME_T
#else
	#undef _USE_32BIT_TIME_T
#endif

/*
* tchar-like std::string
*/
typedef std::basic_string<TCHAR> tstring;

extern NEN_OPTIONS	nen_options;
extern HINSTANCE g_hInst;
extern CSkinItem SkinItems[];
extern TContainerData *pFirstContainer, *pLastActiveContainer;
extern HANDLE hTypingNotify;
extern ButtonSet g_ButtonSet;
extern HANDLE g_hEvent;
extern RECT rcLastStatusBarClick;
extern PLUGININFOEX pluginInfo;
extern TTemplateSet RTL_Active, LTR_Active;
extern LOGFONTA logfonts[MSGDLGFONTCOUNT + 2];
extern COLORREF fontcolors[MSGDLGFONTCOUNT + 2];
extern HINSTANCE hinstance;
extern BOOL g_bIMGtagButton;
extern HIMAGELIST g_himlOptions;
extern HANDLE hHookToolBarLoadedEvt;
extern TLogIcon msgLogIcons[NR_LOGICONS * 3];
extern const TCHAR *pszIDCSAVE_save, *pszIDCSAVE_close;
extern char *TemplateNames[];
extern HANDLE hUserPrefsWindowList;
extern TCpTable cpTable[];
extern HMODULE g_hIconDLL;
extern HMENU g_hMenu;
extern bool g_bShutdown;

int  LoadSendRecvMessageModule(void);
int  SplitmsgShutdown(void);
void LogErrorMessage(HWND hwndDlg, TWindowData *dat, int i, TCHAR *szMsg);
int  Chat_Load(), Chat_Unload();
void FreeLogFonts();
void ImageDataInsertBitmap(IRichEditOle *ole, HBITMAP hBm);
int  CacheIconToBMP(TLogIcon *theIcon, HICON hIcon, COLORREF backgroundColor, int sizeX, int sizeY);
void DeleteCachedIcon(TLogIcon *theIcon);

INT_PTR SendMessageCommand(WPARAM wParam, LPARAM lParam);
INT_PTR SendMessageCommand_W(WPARAM wParam, LPARAM lParam);

HIMAGELIST CreateStateImageList();

INT_PTR CALLBACK DlgProcSetupStatusModes(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcPopupOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcTabConfig(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcTemplateEditor(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcToolBar(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PlusOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcOptions1(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcOptions2(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcOptions3(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcUserPrefsFrame(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

int TSAPI TBStateConvert2Flat(int state);
int TSAPI RBStateConvert2Flat(int state);
void TSAPI FillTabBackground(const HDC hdc, int iStateId, const TWindowData *dat, RECT* rc);

#define IS_EXTKEY(a) (a & (1 << 24))

#endif /* __COMMONHEADERS_H */
