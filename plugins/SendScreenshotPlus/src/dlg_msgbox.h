/*
Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef _SVC_DLG_INCLUDED_
#define _SVC_DLG_INCLUDED_ 1

#define SET_POPUPMSGBOX			"PopupMsgBox"
#define DEFVAL_POPUPMSGBOX		TRUE		//FALSE

/* UserInfo/MsgBox	v0.1.0.3+
Some little changed message box for nicer look of miranda's messages or questions :-)
wParam=hContact				- can be null
lParam=(_MSGBOX*)pMsg	- structure that holds information about the look of the message dialog
uType member of _MSGBOX can be a combination of the following values, where most of them are defined in winuser.h:
MB_OK
MB_OKCANCEL
MB_YESALLNO
MB_YESNO
For valid icon values use one of the following MB_ICON_...
Funktion returns: IDOK, IDYES, IDALL, IDNO or IDCANCEL
*/

/*
 Defined in winuser.h
 ********************

#define MB_OK					0x00000000L
#define MB_OKCANCEL				0x00000001L
#define MB_ABORTRETRYIGNORE		0x00000002L
#define MB_YESNOCANCEL			0x00000003L
#define MB_YESNO				0x00000004L
#define MB_RETRYCANCEL			0x00000005L
*/
#define MB_YESALLNO				0x00000007L
#define MB_TYPE(p)				((p)&MB_TYPEMASK)	

/*
valid predefined icon values
*/
#define MB_ICON_NONE			0x00000000L //	 0 - no icon
#define MB_ICON_ERROR			0x00000010L //	16 - error icon
#define MB_ICON_QUESTION		0x00000020L //	32 - question mark
#define MB_ICON_WARNING			0x00000030L //	48 - warning
#define MB_ICON_INFO			0x00000040L //	64 - info
#define MB_ICON_OTHER			0x00000080L // 240 - use icon _MSGBOX->hiMsg
#define MB_ICON_INDEX(p)		(((p)&MB_ICONMASK)>>4)

/*
flags
*/
#define MB_INFOBAR				0x00000100L
#define MB_NOPOPUP				0x00000200L
#define MB_CUSTOMCOLOR			0x00000300L

typedef struct _MSGBOX 
{
	UINT     cbSize;        // size of this structure
	UINT     uType;         // parameters
	HICON    hiLogo;        // right upper icon of the info bar
	HICON    hiMsg;         // icon left next to the message text
	LPTSTR   ptszTitle;
	LPTSTR   ptszInfoText;
	LPTSTR   ptszMsg;
	HWND     hParent;	      // parent window for the messagebox
	COLORREF colorBack;     // valid if MB_CUSTOMCOLOR is set 
	COLORREF colorText;     // valid if MB_CUSTOMCOLOR is set
} MSGBOX, *LPMSGBOX;


INT_PTR CALLBACK	MsgBox(HWND hParent, UINT uType, LPCTSTR pszTitle, LPCTSTR pszInfo, LPCTSTR pszFormat, ...);
INT_PTR CALLBACK	MsgErr(HWND hParent, LPCTSTR pszFormat, ...);
INT_PTR				MsgBoxService(WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK	PopupProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif /* _SVC_DLG_INCLUDED_ */