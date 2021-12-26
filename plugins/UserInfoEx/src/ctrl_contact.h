/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

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

#ifndef _UI_CTRL_INCLUDE_
#define _UI_CTRL_INCLUDE_

/**
 *	ctrlContact.cpp
 **/

/* contact control	v0.1.0.6+
*/
#define UINFOCOMBOEXCLASS	L"UInfoComboExWndClass"

// messages
#define CBEXM_ADDITEM			 (WM_USER+4)
#define CBEXM_GETITEM			 (WM_USER+5)
#define CBEXM_SETITEM			 (WM_USER+6)
#define CBEXM_DELITEM			 (WM_USER+7)
#define CBEXM_DELALLITEMS	 (WM_USER+8)
#define CBEXM_ISCHANGED		 (WM_USER+9)
#define CBEXM_RESETCHANGED	(WM_USER+10)
#define CBEXM_SETCURSEL		 (WM_USER+11)
#define CBEXM_SORT					(WM_USER+12)
#define CBEXM_ENABLEITEM		(WM_USER+13)

// item masks
#define CBEXIM_FLAGS				0x0001
#define CBEXIM_CAT					0x0002
#define CBEXIM_VAL					0x0004
#define CBEXIM_ICONTEXT		 0x0008
#define CBEXIM_ID					 0x0010
#define CBEXIM_ALL					(CBEXIM_ID|CBEXIM_ICONTEXT|CBEXIM_CAT|CBEXIM_VAL|CBEXIM_FLAGS)

#define CBEXIF_CATREADONLY	(CTRLF_FIRST)
#define CBEXIF_SMS					(CTRLF_FIRST * 2)
#define CBEXIF_DELETED			(CTRLF_FIRST * 4)

struct CBEXITEM
{
	uint16_t     wMask;		// determines which element of this structure is valid
	uint16_t     wFlags;	 // standard control flags
	int      iItem;		// position of the item in the data array
	uint32_t	   dwID;		 // unique number for each setting read from db to identify it, new entries have dwID = 0
	LPTSTR   pszCat;	 // pointer to a descriptive category string to set or retrieve for the data entry
	uint16_t     ccCat;
	LPTSTR   pszVal;
	uint16_t     ccVal;
	int      iIcon;
};

int CtrlContactLoadModule();
int CtrlContactUnLoadModule();
int CtrlContactAddItemFromDB(HWND hCtrl, int iIcon, LPTSTR szItem, MCONTACT hContact, LPCSTR pszModule, LPCSTR pszProto, LPCSTR szSettingVal);
int CtrlContactAddMyItemsFromDB(HWND hCtrl, int iIcon, uint16_t wForcedFlags, MCONTACT hContact, LPCSTR pszModule, LPCSTR pszProto, LPCSTR szFormatCat, LPCSTR szFormatVal);
int CtrlContactWriteItemToDB(HWND hCtrl, MCONTACT hContact, LPCSTR pszModule, LPCSTR pszProto, LPCSTR pszSetting);
int CtrlContactWriteMyItemsToDB(HWND hCtrl, int iFirstItem, MCONTACT hContact, LPCSTR pszModule, LPCSTR pszProto, LPCSTR szFormatCat, LPCSTR szFormatVal);

#endif /* _UI_CTRL_INCLUDE_ */