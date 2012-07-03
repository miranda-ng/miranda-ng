/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
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
#ifndef _M_MWCLC_
#define _M_MWCLC_

//add a new hotkey so it has a default and can be changed in the options dialog
//wParam=0
//lParam=(LPARAM)(SKINHOTKEYDESC*)ssd;
//returns 0 on success, nonzero otherwise
typedef struct {
	int cbSize;
	const char *pszName;		   //name to refer to sound when playing and in db
	const char *pszDescription;	   //description for options dialog
//    const char *pszDefaultFile;    //default sound file to use
    const char *pszSection;        //section name used to group sounds (NULL is acceptable)
	const char *pszService;        //Service to call when HotKey Pressed

	int DefHotKey; //default hot key for action
} SKINHOTKEYDESCEX;

#define MS_SKIN_ADDHOTKEY      "Skin/HotKeys/AddNew"
#define MS_SKIN_PLAYHOTKEY		"Skin/HotKeys/Run"

#endif