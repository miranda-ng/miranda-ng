/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
aLONG with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA	02111-1307, USA.

*/
#ifndef _UINFOEX_BOTTONS_H_INCLUDED_
#define _UINFOEX_BOTTONS_H_INCLUDED_ 1

// theme procedures
extern HANDLE	(WINAPI *OpenThemeData)(HWND,LPCWSTR);
extern HRESULT	(WINAPI *CloseThemeData)(HANDLE);
extern BOOL		(WINAPI *IsThemeBackgroundPartiallyTransparent)(HANDLE,INT,INT);
extern HRESULT	(WINAPI *DrawThemeParentBackground)(HWND,HDC,RECT *);
extern HRESULT	(WINAPI *DrawThemeBackground)(HANDLE,HDC,INT,INT,const RECT *,const RECT *);
extern HRESULT	(WINAPI *DrawThemeText)(HANDLE,HDC,INT,INT,LPCWSTR,INT,DWORD,DWORD,const RECT *);
extern HRESULT	(WINAPI *GetThemeTextExtent)(HANDLE,HDC,INT,INT,LPCWSTR,INT,DWORD,OPTIONAL const RECT*, RECT *);
extern HRESULT	(WINAPI *GetThemeBackgroundRegion)(HANDLE,HDC,INT,INT,const RECT *,HRGN *);


VOID CtrlButtonLoadModule();
VOID CtrlButtonUnloadModule();

BOOLEAN __fastcall ThemeSupport();

#endif /* _UINFOEX_BOTTONS_H_INCLUDED_ */