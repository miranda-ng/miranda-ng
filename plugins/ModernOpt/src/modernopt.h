/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-07 Miranda ICQ/IM project,
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

#ifndef MODERNOPT_H__
#define MODERNOPT_H__ 1

struct ModernOptMainPageInfo
{
	int		idcButton;
	BOOL	bShowTab;
	int		iIcon;
	TCHAR	*lpzTitle;
	TCHAR	*lpzDescription;
	int		iPageType;
	BOOL	bShow;
};

extern struct ModernOptMainPageInfo g_ModernOptPages[MODERNOPT_PAGE_COUNT];

extern HMODULE hInst;

INT_PTR CALLBACK ModernOptHome_DlgProc(HWND hwndDlg, UINT  msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ModernOptSelector_DlgProc(HWND hwndDlg, UINT  msg, WPARAM wParam, LPARAM lParam);

void ModernOptIgnore_AddItem(MODERNOPTOBJECT *obj);

#endif // MODERNOPT_H__
