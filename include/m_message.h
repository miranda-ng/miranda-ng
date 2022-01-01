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

#ifndef M_MESSAGE_H__
#define M_MESSAGE_H__ 1

#include <m_database.h>
#include <m_langpack.h>

/////////////////////////////////////////////////////////////////////////////////////////
// brings up the send message dialog for a contact
// wParam = (MCONTACT)hContact
// lParam = (LPARAM)(char*)szText
// returns 0 on success or nonzero on failure
// returns immediately, just after the dialog is shown
// szText is the text to put in the edit box of the window (but not send)
// szText = NULL will not use any text
// szText != NULL is only supported on v0.1.2.0+

#define MS_MSG_SENDMESSAGE   "SRMsg/SendCommand"
#define MS_MSG_SENDMESSAGEW  "SRMsg/SendCommandW"

/////////////////////////////////////////////////////////////////////////////////////////
// reads a message from contact list event structure
// wParam = 0 (unused)
// lParam = (CLISTEVT*)pEvent
// always returns 0

#define MS_MSG_READMESSAGE   "SRMsg/ReadMessage"
#define MS_MSG_TYPINGMESSAGE "SRMsg/TypingMessage"

/////////////////////////////////////////////////////////////////////////////////////////
// gets fired when a message window appears/disappears
// wparam = 0
// lparam = (WPARAM)(MessageWindowEventData*)hWindowEvent;

#define MSG_WINDOW_EVT_OPENING 1 // window is about to be opened
#define MSG_WINDOW_EVT_OPEN    2 // window has been opened
#define MSG_WINDOW_EVT_CLOSING 3 // window is about to be closed
#define MSG_WINDOW_EVT_CLOSE   4 // window has been closed

#define MSG_WINDOW_UFLAG_MSG_FROM 0x00000001
#define MSG_WINDOW_UFLAG_MSG_TO   0x00000002
#define MSG_WINDOW_UFLAG_MSG_BOTH 0x00000004

struct MessageWindowEventData
{
	MCONTACT hContact; 
	HWND hwndWindow; // top level window for the contact
	uint32_t uType;  // see event types above
	uint32_t uFlags; // used to indicate message direction for all event types except custom
	HWND hwndInput;  // input area window for the contact (or NULL if there is none)
	HWND hwndLog;    // log area window for the contact (or NULL if there is none)
};

#define ME_MSG_WINDOWEVENT "MessageAPI/WindowEvent"

/////////////////////////////////////////////////////////////////////////////////////////
// retrieves some particular info about a SRMM window by contact
// returns 0 if a window was found or an error code otherwise

#define MSG_WINDOW_STATE_EXISTS  0x00000001 // Window exists should always be true if hwndWindow exists
#define MSG_WINDOW_STATE_VISIBLE 0x00000002
#define MSG_WINDOW_STATE_FOCUS   0x00000004
#define MSG_WINDOW_STATE_ICONIC  0x00000008

struct MessageWindowData
{
	HWND hwndWindow; //top level window for the contact or NULL if no window exists
	int uState; // see window states
	class CSrmmBaseDialog *pDlg; // window class object
};

EXTERN_C MIR_APP_DLL(int) Srmm_GetWindowData(MCONTACT hContact, MessageWindowData &mwd);

/////////////////////////////////////////////////////////////////////////////////////////
// sets the status text & icon in a window associated with hContact

EXTERN_C MIR_APP_DLL(void) Srmm_SetStatusText(MCONTACT hContact, const wchar_t *wszText, HICON hIcon = nullptr);

/////////////////////////////////////////////////////////////////////////////////////////
// wparam = 0 (unused)
// lparam = (MessageWindowEvent*)
// fired before SRMM writes an entered message into the database
#define ME_MSG_PRECREATEEVENT    "MessageAPI/PreCreateEvent"

struct MessageWindowEvent
{
	int seq;      // number returned by PSS_MESSAGE
	MCONTACT hContact;
	DBEVENTINFO *dbei; // database event written on the basis of message sent
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SRMM popup menu

// wParam = 0
// lParam = (MessageWindowPopupData *)&MessageWindowPopupData;
// Fired to allow plugins to add items to the msg window popup menu
// Always fired twice: once with MSG_WINDOWPOPUP_SHOWING and once with MSG_WINDOWPOPUP_SELECTED.
// This is done to allow cleaning of resources.

#define ME_MSG_WINDOWPOPUP		"MessageAPI/WindowPopupRequested"

#define MSG_WINDOWPOPUP_SHOWING  1
#define MSG_WINDOWPOPUP_SELECTED 2

#define MSG_WINDOWPOPUP_INPUT    1
#define MSG_WINDOWPOPUP_LOG      2

struct MessageWindowPopupData
{
	unsigned int uType; // see popup types above
	unsigned int uFlags; // used to indicate in which window the popup was requested
	MCONTACT hContact;
	HWND hwnd; // window where the popup was requested
	HMENU hMenu;	// The handle to the menu
	POINT pt; // The point, in screen coords
	int selection; // The menu control id or 0 if no one was selected
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// status icons

#define MBF_DISABLED       0x01
#define MBF_HIDDEN         0x02
#define MBF_UNICODE        0x04

struct StatusIconData
{
	const char *szModule;            // used in combo with the dwId below to create a unique identifier
	uint32_t dwId;                      // uniquely defines a button inside a module
	HICON hIcon, hIconDisabled;      // hIconDisabled is optional - if null, will use hIcon in the disabled state
	int   flags;                     // bitwize OR of MBF_* flags above
	MAllCStrings szTooltip;          // controlled by MBF_UNICODE
};

#define MBCF_RIGHTBUTTON   0x01     // if this flag is specified, the click was a right button - otherwize it was a left click

// adds an icon
EXTERN_C MIR_APP_DLL(int) Srmm_AddIcon(StatusIconData *sid, HPLUGIN pPlugin);

// removes an icon
EXTERN_C MIR_APP_DLL(void) Srmm_RemoveIcon(const char *szProto, uint32_t iconId);

// if hContact is null, icon is modified for all contacts
// if either hIcon or pwszTooltip is null, they will not be modified
EXTERN_C MIR_APP_DLL(void) Srmm_ModifyIcon(MCONTACT hContact, const char *szModule, uint32_t iconId, HICON hIcon, const wchar_t *pwszToolTip);

// if hContact is null, flags are modified for all contacts
EXTERN_C MIR_APP_DLL(void) Srmm_SetIconFlags(MCONTACT hContact, const char *szModule, uint32_t iconId, int flags);

// idx is zero-based index of a visible icon
// returns (StatusIconData*)icon description filled for the required contact or NULL if there're no more icons
// don't free this memory.
EXTERN_C MIR_APP_DLL(StatusIconData*) Srmm_GetNthIcon(MCONTACT hContact, int idx);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// status icons click notification

struct StatusIconClickData
{
	POINT clickLocation;             // click location, in screen coordinates
	const char *szModule;
	uint32_t dwId;
	int   flags;                       // bitwize OR of MBCF_* flags above
};

// wParam = (HANDLE)hContact;
// lParam = (StatusIconClickData *)&StatusIconClickData;
// catch to show a popup menu, etc.
#define ME_MSG_ICONPRESSED		"MessageAPI/IconPressed"

// wParam = (HANDLE)hContact;
// lParam = (StatusIconkData*)pIcon
// catch to be notified about the icon list's change.
#define ME_MSG_ICONSCHANGED   "MessageAPI/IconsChanged"

// emulates click on a status bar icon
EXTERN_C MIR_APP_DLL(void) Srmm_ClickStatusIcon(MCONTACT hContact, const StatusIconClickData *sid);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// srmm toolbar icons' support

// default section for all SRMM toolbar hotkeys
#define BB_HK_SECTION         LPGEN("Message window toolbar")

// button state flags
#define BBSF_HIDDEN           (1<<0)
#define BBSF_DISABLED         (1<<1)
#define BBSF_PUSHED           (1<<2)
#define BBSF_RELEASED         (1<<3)

// button flags
#define BBBF_DISABLED			(1<<0)
#define BBBF_HIDDEN				(1<<1)
#define BBBF_ISPUSHBUTTON		(1<<2)
#define BBBF_ISARROWBUTTON		(1<<3)
#define BBBF_ISCHATBUTTON		(1<<4)
#define BBBF_ISIMBUTTON			(1<<5)
#define BBBF_ISRSIDEBUTTON		(1<<7)
#define BBBF_CANBEHIDDEN		(1<<8)
#define BBBF_ISSEPARATOR		(1<<9)
#define BBBF_CANTBEHIDDEN		(1<<10)
#define BBBF_CREATEBYID			(1<<11)  // only for the internal use

struct BBButton
{
	const char    *pszModuleName;  // module name without spaces and underline symbols (e.g. "tabsrmm")
	int            dwButtonID;     // your button ID, will be combined with pszModuleName for storing settings, etc...

	const wchar_t *pwszText;       // button's text, might be NULL
	const wchar_t *pwszTooltip;    // button's tooltip, might be NULL
	uint32_t       dwDefPos;       // default order pos of button, counted from window edge (left or right)
	                               // use value >100, because internal buttons using 10,20,30... 80, etc
	uint32_t       bbbFlags;       // combine of BBBF_ flags above
	HANDLE         hIcon;          // Handle to icolib registered icon
	const char    *pszHotkey;      // name of the registered hotkey or NULL
};

// adds a new toolbar button
// returns button's handle on success or NULL otherwise
EXTERN_C MIR_APP_DLL(HANDLE) Srmm_AddButton(const BBButton *bbdi, HPLUGIN);

// modifies the existing toolbar button
// returns 0 on success and nonzero value otherwise
EXTERN_C MIR_APP_DLL(int) Srmm_ModifyButton(BBButton *bbdi);

// removes a toolbar button identified by a structure
// returns 0 on success and nonzero value otherwise
EXTERN_C MIR_APP_DLL(int) Srmm_RemoveButton(BBButton *bbdi);

// retrieves data from a toolbar button to a structure
// returns 0 on success and nonzero value otherwise
EXTERN_C MIR_APP_DLL(int) Srmm_GetButtonState(HWND hwndDlg, BBButton *bbdi);

// applies a new data to a toolbar button
// returns 0 on success and nonzero value otherwise
EXTERN_C MIR_APP_DLL(int) Srmm_SetButtonState(MCONTACT hContact, BBButton *bbdi);

// resets toolbar settings to these default values
// returns 0 on success and nonzero value otherwise
EXTERN_C MIR_APP_DLL(void) Srmm_ResetToolbar();

// updates all toolbar icons in a message dialog
EXTERN_C MIR_APP_DLL(void) Srmm_UpdateToolbarIcons(HWND hdlg);

// draws all toolbar icons in a message dialog
EXTERN_C MIR_APP_DLL(void) Srmm_RedrawToolbarIcons(HWND hwndDlg);

// ToolBar loaded event
// This event will be send after module loaded and after each toolbar reset
// You should add your buttons on this event
// NOTE: this event is temporary, you need to hook it using HookTemporaryEvent
#define ME_MSG_TOOLBARLOADED "SRMM/ButtonsBar/ModuleLoaded"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// toolbar button clicked event

// wParam = (HANDLE)hContact;
// lParam = (CustomButtonClickData*) pointer to the click data;
// catch to show a popup menu, etc.
#define ME_MSG_BUTTONPRESSED "SRMM/ButtonsBar/ButtonPressed"

// event flags
#define BBCF_RIGHTBUTTON	(1<<0)
#define BBCF_SHIFTPRESSED	(1<<1)
#define BBCF_CONTROLPRESSED	(1<<2)
#define BBCF_ARROWCLICKED	(1<<3)

struct CustomButtonClickData
{
	char    *pszModule;   // button owners name
	POINT    pt;          // screen coordinates for menus
	uint32_t dwButtonId;  // registered button ID
	HWND     hwndFrom;    // button parents HWND
	MCONTACT hContact;
	uint32_t flags;       // BBCF_ flags
};

/////////////////////////////////////////////////////////////////////////////////////////
// color table for embedded color chooser

EXTERN_C MIR_APP_DLL(COLORREF*) Srmm_GetColorTable(int *pTableSize = nullptr);

#endif // M_MESSAGE_H__
