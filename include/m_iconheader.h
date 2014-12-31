/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-2007 Miranda ICQ/IM project,
Copyright (c) 2007 Artem Shpynov

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

#ifndef M_ICONHEADER_H__
#define M_ICONHEADER_H__ 1

#define MIRANDAICOTABCLASS _T("MirandaIcoTabClass")

#define MITCF_SHAREDICON	0x01
#define MITCF_UNICODE		0x02
#ifdef _UNICODE
	#define	MITCF_TCHAR		MITCF_UNICODE
#else
	#define	MITCF_TCHAR		0
#endif

#define ITCM_FIRST			(WM_USER+1024)
#define ITCM_LAST			(ITCM_FIRST+64)

#define ITCM_SETBACKGROUND	(ITCM_FIRST+1)		//LPARAM is HBITMAP
#define ITCM_ADDITEM		(ITCM_FIRST+2)		//LPARAM is pointer to MIcoTab
#define ITCM_SETSEL			(ITCM_FIRST+3)		//WPARAM is new selected index
#define ITCM_GETSEL			(ITCM_FIRST+4)		//result is selected item index
#define ITCM_GETITEMDATA	(ITCM_FIRST+5)		//WPARAM is item index, result is custom data

#define ITCN_SELCHANGED		1
#define ITCN_SELCHANGEDKBD	2

// structure is used for storing info about single tab
typedef struct {
	HICON	hIcon;
	union {
		TCHAR	*tcsName;
		TCHAR	*lptzName;
		char	*lpzName;
		WCHAR	*lpwzName;
	};
	DWORD	flag;
	LPARAM	data;
} MIcoTab;

#define MIcoTab_SetBackground(hwnd, hBmp)	\
	(SendMessage((hwnd), ITCM_SETBACKGROUND, 0, (LPARAM)(hBmp)))
#define MIcoTab_SetSel(hwnd, idx)	\
	(SendMessage((hwnd), ITCM_SETSEL, (idx), 0))
#define MIcoTab_GetSel(hwnd)	\
	(SendMessage((hwnd), ITCM_GETSEL, 0, 0))
#define MIcoTab_GetItemData(hwnd, idx)	\
	(SendMessage((hwnd), ITCM_GETITEMDATA, (idx), 0))

static __forceinline void MIcoTab_AddItem(HWND hwnd, TCHAR *lptzName, HICON hIcon, LPARAM data, BOOL bSharedIcon)
{
	MIcoTab mit = {0};
	mit.flag = (bSharedIcon?MITCF_SHAREDICON:0)|MITCF_TCHAR;
	mit.hIcon = hIcon;
	mit.tcsName = lptzName;
	mit.data = data;
	SendMessage(hwnd, ITCM_ADDITEM, (WPARAM)&mit, 0);
}

#endif // M_ICONHEADER_H__
