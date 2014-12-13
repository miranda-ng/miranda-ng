/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (c) 2012-14 Miranda NG project,
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

void   TSAPI CalcDynamicAvatarSize(TWindowData *dat, BITMAP *bminfo);
char*  TSAPI GetCurrentMetaContactProto(TWindowData *dat);
int    TSAPI MsgWindowUpdateMenu(TWindowData *dat, HMENU submenu, int menuID);
int    TSAPI MsgWindowMenuHandler(TWindowData *dat, int selection, int menuId);
bool   TSAPI GetAvatarVisibility(HWND hwndDlg, TWindowData *dat);
void   TSAPI ProcessAvatarChange(HWND hwnd, LPARAM lParam);
void   TSAPI UpdateStatusBar(const TWindowData *dat);
int    TSAPI CheckValidSmileyPack(const char *szProto, MCONTACT hContact);
TCHAR* TSAPI QuoteText(const TCHAR *text, int charsPerLine, int removeExistingQuotes);
void   TSAPI UpdateReadChars(const TWindowData *dat);
void   TSAPI ShowPicture(TWindowData *dat, BOOL showNewPic);
void   TSAPI AdjustBottomAvatarDisplay(TWindowData *dat);
void   TSAPI SetDialogToType(HWND hwndDlg);
void   TSAPI FlashOnClist(HWND hwndDlg, TWindowData *dat, HANDLE hEvent, DBEVENTINFO *dbei);
char*  TSAPI Message_GetFromStream(HWND hwndDlg, const TWindowData *dat, DWORD dwPassedFlags);
BOOL   TSAPI DoRtfToTags(TCHAR * pszText, const TWindowData *dat);
void   TSAPI GetMYUIN(TWindowData *dat);
void   TSAPI SetMessageLog(TWindowData *dat);
void   TSAPI SwitchMessageLog(TWindowData *dat, int iMode);
UINT   TSAPI GetIEViewMode(MCONTACT hContact);
void   TSAPI FindFirstEvent(TWindowData *dat);
void   TSAPI SaveSplitter(TWindowData *dat);
void   TSAPI LoadSplitter(TWindowData *dat);
void   TSAPI PlayIncomingSound(const TWindowData *dat);
void   TSAPI GetSendFormat(TWindowData *dat, int mode);
void   TSAPI GetLocaleID(TWindowData *dat, const TCHAR *szKLName);
void   TSAPI LoadOwnAvatar(TWindowData *dat);
void   TSAPI LoadContactAvatar(TWindowData *dat);
void   TSAPI LoadTimeZone(TWindowData *dat);
void   TSAPI HandlePasteAndSend(const TWindowData *dat);
int    TSAPI MsgWindowDrawHandler(WPARAM wParam, LPARAM lParam, TWindowData *dat);
void   TSAPI LoadOverrideTheme(TContainerData *pContainer);
void   TSAPI LoadThemeDefaults(TContainerData *pContainer);
void   TSAPI ConfigureSmileyButton(TWindowData *dat);
int    TSAPI CutContactName(const TCHAR *szold, TCHAR *sznew, unsigned int size);
void   TSAPI SendNudge(const TWindowData *dat);
void   TSAPI EnableSendButton(const TWindowData *dat, int iMode);
LRESULT TSAPI GetSendButtonState(HWND hwnd);
HICON  TSAPI GetXStatusIcon(const TWindowData *dat);
void   TSAPI FlashTab(TWindowData *dat, HWND hwndTab, int iTabindex, BOOL *bState, BOOL mode, HICON origImage);
void   TSAPI GetClientIcon(TWindowData *dat);
void   TSAPI RearrangeTab(HWND hwndDlg, const TWindowData *dat, int iMode, BOOL fSavePos);
void   TSAPI GetCachedStatusMsg(TWindowData *dat);
bool   TSAPI IsStatusEvent(int eventType);
bool   TSAPI IsCustomEvent(int eventType);
void   TSAPI GetMyNick(TWindowData *dat);
HICON  TSAPI MY_GetContactIcon(const TWindowData *dat, LPCSTR szSetting);
void   TSAPI CheckAndDestroyIEView(TWindowData *dat);
void   TSAPI KbdState(TWindowData *dat, BOOL& isShift, BOOL& isControl, BOOL& isAlt);
void   TSAPI ClearLog(TWindowData *dat);
bool   TSAPI IsAutoSplitEnabled(const TWindowData *dat);
LONG   TSAPI GetDefaultMinimumInputHeight(const TWindowData *dat);
void   TSAPI DetermineMinHeight(TWindowData *dat);

void	 TSAPI CleanTempFiles();
void 	 TSAPI SendHBitmapAsFile(const TWindowData *dat, HBITMAP hbmp);

extern INT_PTR CALLBACK SelectContainerDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK DlgProcContainerOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
