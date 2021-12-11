/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-21 Miranda NG team,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors

#ifndef _MSGDLGUTILS_H
#define _MSGDLGUTILS_H

#define WANT_BUILTIN_LOG 0
#define WANT_IEVIEW_LOG 1
#define WANT_HPP_LOG 2

void     TSAPI AddUnreadContact(MCONTACT hContact);
void     TSAPI ProcessAvatarChange(HWND hwnd, LPARAM lParam);
BOOL     TSAPI CheckCustomLink(HWND hwndRich, POINT *ptClient, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bUrlNeeded);
wchar_t* TSAPI QuoteText(const wchar_t *text);
void     TSAPI CutContactName(const wchar_t *szold, wchar_t *sznew, size_t size);
void     TSAPI RearrangeTab(HWND hwndDlg, const CMsgDialog *dat, int iMode, BOOL bSavePos);
bool     TSAPI IsStatusEvent(int eventType);
bool     TSAPI IsCustomEvent(int eventType);

void	   TSAPI CleanTempFiles();

#endif
