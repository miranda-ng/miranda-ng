/*
UserinfoEx plugin for Miranda NG

Copyright:
© 2012-22 Miranda NG team (https://miranda-ng.org)
© 2006-10 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

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

struct MSGBOX
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
};

INT_PTR CALLBACK	MsgBox(HWND hParent, UINT uType, LPCTSTR pszTitle, LPCTSTR pszInfo, LPCTSTR pszFormat, ...);
INT_PTR CALLBACK	MsgErr(HWND hParent, LPCTSTR pszFormat, ...);
INT_PTR				MsgBoxService(WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK	PopupProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif /* _SVC_DLG_INCLUDED_ */