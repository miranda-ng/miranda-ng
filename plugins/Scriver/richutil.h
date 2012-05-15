/*
SRMM

Copyright 2000-2009 Miranda ICQ/IM project,

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
#define RIsWinVerXPPlus()   (RWinVerMajor()>=5 && LOWORD(GetVersion())!=5)

typedef struct {
	HWND hwnd;
	RECT rect;
	int hasUglyBorder;
	WNDPROC origProc;
} TRichUtil;

void RichUtil_Load();
void RichUtil_Unload();
int RichUtil_SubClass(HWND hwndEdit);

extern HMODULE mTheme;
extern BOOL    (WINAPI *MyIsAppThemed)(VOID);
extern BOOL    (WINAPI *MyIsThemeActive)();
extern BOOL    (WINAPI *MyIsThemeBackgroundPartiallyTransparent)(HANDLE,int,int);
extern HANDLE  (WINAPI *MyOpenThemeData)(HWND,LPCWSTR);
extern HRESULT (WINAPI *MyCloseThemeData)(HANDLE);
extern HRESULT (WINAPI *MyDrawThemeBackground)(HANDLE,HDC,int,int,const RECT*,const RECT *);
extern HRESULT (WINAPI *MyGetThemeBackgroundContentRect)(HANDLE,HDC,int,int,const RECT *,RECT *);
extern HRESULT (WINAPI *MyDrawThemeParentBackground)(HWND,HDC,RECT*);
extern HRESULT (WINAPI *MyDrawThemeText)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, DWORD, const RECT *);
extern HRESULT (WINAPI *MyEnableThemeDialogTexture)(HWND, DWORD);

#endif
