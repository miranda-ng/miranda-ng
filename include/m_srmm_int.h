/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org)
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

#ifndef M_SRMM_INT_H__
#define M_SRMM_INT_H__ 1

#include <m_core.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// toolbar button internal representation

#define MIN_CBUTTONID      4000
#define MAX_CBUTTONID      5000

#define BBSF_IMBUTTON		(1<<0)
#define BBSF_CHATBUTTON		(1<<1)
#define BBSF_CANBEHIDDEN	(1<<2)
#define BBSF_NTBSWAPED		(1<<3)
#define BBSF_NTBDESTRUCT	(1<<4)

struct CustomButtonData : public MZeroedObject
{
	~CustomButtonData()
	{}

	int    m_dwPosition;    // default order pos of button, counted from window edge (left or right)

	int    m_dwButtonID;    // id of button used while button creation and to store button info in DB
	ptrA   m_pszModuleName; // module name without spaces and underline symbols (e.g. "tabsrmm")

	int    m_dwButtonCID;	// button's control id
	int    m_dwArrowCID;    // only use with BBBF_ISARROWBUTTON flag

	ptrW   m_pwszText;      // button's text
	ptrW   m_pwszTooltip;   // button's tooltip

	int    m_iButtonWidth;  // must be 22 for regular button and 33 for button with arrow
	HANDLE m_hIcon;         // Handle to icolib registred icon

	bool   m_bIMButton, m_bChatButton;
	bool   m_bCanBeHidden, m_bCantBeHidden, m_bHidden, m_bAutoHidden, m_bSeparator, m_bDisabled, m_bPushButton;
	bool   m_bRSided;
	BYTE   m_opFlags;
	int    m_hLangpack;
	DWORD  m_dwOrigPosition;
	struct THotkeyItem *m_hotkey;

	struct {
		bool bit1 : 1, bit2 : 1, bit3 : 1, bit4 : 1;
	} m_dwOrigFlags;
};

// gets the required button or NULL, if i is out of boundaries
EXTERN_C MIR_APP_DLL(CustomButtonData*) Srmm_GetNthButton(int i);

// retrieves total number of toolbar buttons
EXTERN_C MIR_APP_DLL(int) Srmm_GetButtonCount(void);

// emulates a click on a toolbar button
EXTERN_C MIR_APP_DLL(void) Srmm_ClickToolbarIcon(MCONTACT hContact, int idFrom, HWND hwndFrom, BOOL code);

// these messages are sent to the message windows if toolbar buttons are changed
#define WM_CBD_FIRST   (WM_USER+0x600)

// wParam = 0 (ignored)
// lParam = (CustomButtonData*)pointer to button or null if any button can be changed
#define WM_CBD_UPDATED (WM_CBD_FIRST+1)

// wParam = button id
// lParam = (CustomButtonData*)pointer to button
#define WM_CBD_REMOVED (WM_CBD_FIRST+2)

// wParam = 0 (ignored)
// lParam = 0 (ignored)
#define WM_CBD_LOADICONS (WM_CBD_FIRST+3)

#endif // M_MESSAGE_H__
