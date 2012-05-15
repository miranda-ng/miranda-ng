/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
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

//brings up the send message dialog for a contact
//wParam=(WPARAM)(HANDLE)hContact
//lParam=(LPARAM)(char*)szText
//returns 0 on success or nonzero on failure
//returns immediately, just after the dialog is shown
//szText is the text to put in the edit box of the window (but not send)
//szText=NULL will not use any text
//szText!=NULL is only supported on v0.1.2.0+
#define MS_MSG_SENDMESSAGE   "SRMsg/SendCommand"
#define MS_MSG_SENDMESSAGEW  "SRMsg/SendCommandW"

#ifdef _UNICODE
#define MS_MSG_SENDMESSAGET MS_MSG_SENDMESSAGEW
#else
#define MS_MSG_SENDMESSAGET MS_MSG_SENDMESSAGE
#endif

#define ME_MSG_WINDOWEVENT "MessageAPI/WindowEvent"
//wparam=0
//lparam=(WPARAM)(MessageWindowEventData*)hWindowEvent;
//Event types
#define MSG_WINDOW_EVT_OPENING 1 //window is about to be opened
#define MSG_WINDOW_EVT_OPEN    2 //window has been opened
#define MSG_WINDOW_EVT_CLOSING 3 //window is about to be closed
#define MSG_WINDOW_EVT_CLOSE   4 //window has been closed
#define MSG_WINDOW_EVT_CUSTOM  5 //custom event for message plugins to use (custom uFlags may be used)

#define MSG_WINDOW_UFLAG_MSG_FROM 0x00000001
#define MSG_WINDOW_UFLAG_MSG_TO   0x00000002
#define MSG_WINDOW_UFLAG_MSG_BOTH 0x00000004

typedef struct {
   int cbSize;
   HANDLE hContact;
   HWND hwndWindow; // top level window for the contact
   const char* szModule; // used to get plugin type (which means you could use local if needed)
   unsigned int uType; // see event types above
   unsigned int uFlags; // used to indicate message direction for all event types except custom
   void *local; // used to store pointer to custom data
   HWND hwndInput; // input area window for the contact (or NULL if there is none)
   HWND hwndLog; // log area window for the contact (or NULL if there is none)
} MessageWindowEventData;

#define MS_MSG_GETWINDOWAPI "MessageAPI/WindowAPI"
//wparam=0
//lparam=0
//Returns a dword with the current message api version
//Current version is 0,0,0,4

#define MS_MSG_GETWINDOWCLASS "MessageAPI/WindowClass"
//wparam=(char*)szBuf
//lparam=(int)cbSize size of buffer
//Sets the window class name in wParam (ex. "SRMM" for srmm.dll)

typedef struct {
	int cbSize;
	HANDLE hContact;
	int uFlags; // see uflags above
} MessageWindowInputData;

#define MSG_WINDOW_STATE_EXISTS  0x00000001 // Window exists should always be true if hwndWindow exists
#define MSG_WINDOW_STATE_VISIBLE 0x00000002
#define MSG_WINDOW_STATE_FOCUS   0x00000004
#define MSG_WINDOW_STATE_ICONIC  0x00000008

typedef struct {
	int cbSize;
	HANDLE hContact;
	int uFlags;  // should be same as input data unless 0, then it will be the actual type
	HWND hwndWindow; //top level window for the contact or NULL if no window exists
	int uState; // see window states
	void *local; // used to store pointer to custom data
} MessageWindowData;

#define MS_MSG_GETWINDOWDATA "MessageAPI/GetWindowData"
//wparam=(MessageWindowInputData*)
//lparam=(MessageWindowData*)
//returns 0 on success and returns non-zero (1) on error or if no window data exists for that hcontact


#define ME_MSG_WINDOWPOPUP		"MessageAPI/WindowPopupRequested"
// wParam = 0
// lParam = (MessageWindowPopupData *)&MessageWindowPopupData;
// Fired to allow plugins to add items to the msg window popup menu
// Always fired twice: once with MSG_WINDOWPOPUP_SHOWING and once with MSG_WINDOWPOPUP_SELECTED.
// This is done to allow cleaning of resources.
#define MSG_WINDOWPOPUP_SHOWING  1
#define MSG_WINDOWPOPUP_SELECTED 2

#define MSG_WINDOWPOPUP_INPUT    1
#define MSG_WINDOWPOPUP_LOG      2

typedef struct {
   int cbSize;
   unsigned int uType; // see popup types above
   unsigned int uFlags; // used to indicate in which window the popup was requested
   HANDLE hContact;
   HWND hwnd; // window where the popup was requested
   HMENU hMenu;	// The handle to the menu
   POINT pt; // The point, in screen coords
   int selection; // The menu control id or 0 if no one was selected
} MessageWindowPopupData;

// status icons - HICONs will be automatically destroyed when removed or when miranda exits

#define MBF_DISABLED       0x01
#define MBF_HIDDEN         0x02

typedef struct {
	int cbSize;
	char *szModule;						// used in combo with the dwId below to create a unique identifier
	DWORD dwId;
	HICON hIcon, hIconDisabled;		// hIconDisabled is optional - if null, will use hIcon in the disabled state
	int flags;								// bitwize OR of MBF_* flags above
	char *szTooltip;
} StatusIconData;

#define MBCF_RIGHTBUTTON   0x01		// if this flag is specified, the click was a right button - otherwize it was a left click

typedef struct {
	int cbSize;
	POINT clickLocation;					// click location, in screen coordinates
	char *szModule;
	DWORD dwId;
	int flags;								// bitwize OR of MBCF_* flags above
} StatusIconClickData;

#define MS_MSG_ADDICON			"MessageAPI/AddIcon"
// lParam = (StatusIconData *)&StatusIconData

#define MS_MSG_REMOVEICON		"MessageAPI/RemoveIcon"
// lParam = (StatusIconData *)&StatusIconData
// only szModule and szId are used

#define MS_MSG_MODIFYICON		"MessageAPI/ModifyIcon"
// wParam = (HANDLE)hContact
// lParam = (StatusIconData *)&StatusIconData
// if hContact is null, icon is modified for all contacts
// otherwise, only the flags field is valid
// if either hIcon, hIconDisabled or szTooltip is null, they will not be modified

#define ME_MSG_ICONPRESSED		"MessageAPI/IconPressed"
// wParam = (HANDLE)hContact;
// lParam = (StatusIconClickData *)&StatusIconClickData;
// catch to show a popup menu, etc.

#endif // M_MESSAGE_H__
