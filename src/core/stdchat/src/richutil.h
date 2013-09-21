/*
SRMM

Copyright 2000-2005 Miranda IM project, 
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
#ifndef SRMM_RICHUTIL_H
#define SRMM_RICHUTIL_H

#define RWinVerMajor()      LOBYTE(LOWORD(GetVersion()))

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED		0x031A
#endif
#ifndef EP_EDITTEXT
#define EP_EDITTEXT 1
#endif
#ifndef ETS_NORMAL
#define ETS_NORMAL 1
#endif
#ifndef ETS_DISABLED
#define ETS_DISABLED 4
#endif
#ifndef ETS_READONLY
#define ETS_READONLY 6
#endif

typedef struct {
	HWND hwnd;
	RECT rect;
	int hasUglyBorder;
} TRichUtil;

void RichUtil_Load();
void RichUtil_Unload();
int RichUtil_SubClass(HWND hwndEdit);

#endif
