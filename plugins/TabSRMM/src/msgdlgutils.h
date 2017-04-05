/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-17 Miranda NG project,
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

#define WANT_IEVIEW_LOG 1
#define WANT_HPP_LOG 2

void     TSAPI ProcessAvatarChange(HWND hwnd, LPARAM lParam);
int      TSAPI CheckValidSmileyPack(const char *szProto, MCONTACT hContact);
wchar_t* TSAPI QuoteText(const wchar_t *text);
UINT     TSAPI GetIEViewMode(MCONTACT hContact);
void     TSAPI LoadOverrideTheme(TContainerData *pContainer);
void     TSAPI LoadThemeDefaults(TContainerData *pContainer);
int      TSAPI CutContactName(const wchar_t *szold, wchar_t *sznew, size_t size);
LRESULT  TSAPI GetSendButtonState(HWND hwnd);
void     TSAPI RearrangeTab(HWND hwndDlg, const CTabBaseDlg *dat, int iMode, BOOL fSavePos);
bool     TSAPI IsStatusEvent(int eventType);
bool     TSAPI IsCustomEvent(int eventType);
		   
void	   TSAPI CleanTempFiles();

extern INT_PTR CALLBACK SelectContainerDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK DlgProcContainerOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
