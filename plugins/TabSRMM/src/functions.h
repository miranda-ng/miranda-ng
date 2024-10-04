/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-24 Miranda NG team,
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
//
// Global function prototypes

#ifndef _TABSRMM_FUNCTIONS_H
#define _TABSRMM_FUNCTIONS_H

class CImageItem;

int  AvatarChanged(WPARAM wParam, LPARAM lParam);
int  MyAvatarChanged(WPARAM wParam, LPARAM lParam);
int  IconsChanged(WPARAM wParam, LPARAM lParam);
int  IcoLibIconsChanged(WPARAM wParam, LPARAM lParam);
int  FontServiceFontsChanged(WPARAM wParam, LPARAM lParam);
int  SmileyAddOptionsChanged(WPARAM wParam, LPARAM lParam);
int  IEViewOptionsChanged(WPARAM wParam, LPARAM lParam);

void RegisterFontServiceFonts();

LONG_PTR CALLBACK HotkeyHandlerDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * gneric msgwindow functions(creation, container management etc.)
 */

CMsgDialog* TSAPI CreateNewTabForContact(
	TContainerData *pContainer,
	MCONTACT hContact, 
	bool bActivateTAb, 
	bool bPopupContainer,
	MEVENT hdbEvent = 0);

int   TSAPI ActivateTabFromHWND(HWND hwndTab, HWND hwnd);
void  TSAPI CreateImageList(bool bInitial);

TContainerData* TSAPI FindMatchingContainer(const wchar_t *szName);
TContainerData* TSAPI CreateContainer(const wchar_t *name, int iTemp, MCONTACT hContactFrom);
TContainerData* TSAPI FindContainerByName(const wchar_t *name);

CMsgDialog* TSAPI AutoCreateWindow(TContainerData*, MCONTACT hContact, bool bActivate = false);

int   TSAPI GetTabIndexFromHWND(HWND hwndTab, HWND hwnd);
HWND  TSAPI GetTabWindow(HWND hwndTab, int idx);
int   TSAPI GetTabItemFromMouse(HWND hwndTab, POINT *pt);
void  TSAPI CloseOtherTabs(HWND hwndTab, CMsgDialog &dat);
int   TSAPI ActivateTabFromHWND(HWND hwndTab, HWND hwnd);

void  TSAPI CloseAllContainers();
void  TSAPI DeleteContainer(int iIndex);
void  TSAPI RenameContainer(int iIndex, const wchar_t *newName);
void  TSAPI GetContainerNameForContact(MCONTACT hContact, wchar_t *szName, int iNameLen);
HMENU TSAPI BuildContainerMenu();

int TSAPI MessageWindowOpened(MCONTACT hContact, CMsgDialog *pDlg);

LRESULT CALLBACK HPPKFSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * skinning engine
 */
void  TSAPI DrawAlpha(HDC hdcwnd, PRECT rc, uint32_t basecolor, int alpha, uint32_t basecolor2,
	uint8_t transparent, uint8_t FLG_GRADIENT, uint8_t FLG_CORNER, uint32_t BORDERSTYLE, CImageItem *imageItem);
// the cached message log icons

void  TSAPI CacheMsgLogIcons();
void  TSAPI CacheLogFonts();
void  TSAPI LoadIconTheme();

bool DbEventIsShown(const DB::EventInfo &dbei);

// custom tab control

void  TSAPI ReloadTabConfig();
void  TSAPI FreeTabConfig();
int   TSAPI RegisterTabCtrlClass(void);

// buttons

int   TSAPI UnloadTSButtonModule();

/*
 * debugging support
 */
int   _DebugPopup(MCONTACT hContact, const wchar_t *fmt, ...);

// themes

const wchar_t* TSAPI GetThemeFileName(int iMode);
int   TSAPI CheckThemeVersion(const wchar_t *szIniFilename);
void  TSAPI WriteThemeToINI(const wchar_t *szIniFilename, CMsgDialog *dat);
void  TSAPI ReadThemeFromINI(const wchar_t *szIniFilename, TContainerData *dat, int noAdvanced, uint32_t dwFlags);

// TypingNotify
int   TN_ModuleInit();
int   TN_OptionsInitialize(WPARAM wParam, LPARAM lParam);
int   TN_ModuleDeInit();
void  TN_TypingMessage(MCONTACT hContact, int iMode);

void TSAPI DrawMenuItem(DRAWITEMSTRUCT *dis, HICON hIcon, uint32_t dwIdle);

#endif /* _TABSRMM_FUNCTIONS_H */
