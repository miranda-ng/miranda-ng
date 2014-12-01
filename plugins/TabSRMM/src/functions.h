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
//
// Global function prototypes

#ifndef _TABSRMM_FUNCTIONS_H
#define _TABSRMM_FUNCTIONS_H

int  MyAvatarChanged(WPARAM wParam, LPARAM lParam);
int  IconsChanged(WPARAM wParam, LPARAM lParam);
int  IcoLibIconsChanged(WPARAM wParam, LPARAM lParam);
int  FontServiceFontsChanged(WPARAM wParam, LPARAM lParam);
int  SmileyAddOptionsChanged(WPARAM wParam, LPARAM lParam);
int  IEViewOptionsChanged(WPARAM wParam, LPARAM lParam);
int  ModPlus_PreShutdown(WPARAM wparam, LPARAM lparam);
int  ModPlus_Init(WPARAM wparam, LPARAM lparam);

void RegisterFontServiceFonts();

LONG_PTR CALLBACK HotkeyHandlerDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * nen / event popup stuff
 */

int   TSAPI NEN_ReadOptions(NEN_OPTIONS *options);
int   TSAPI NEN_WriteOptions(NEN_OPTIONS *options);
int   TSAPI UpdateTrayMenu(const TWindowData *dat, WORD wStatus, const char *szProto, const TCHAR *szStatus, MCONTACT hContact, DWORD fromEvent);
int   TSAPI PopupPreview(NEN_OPTIONS *pluginOptions);
void  TSAPI DeletePopupsForContact(MCONTACT hContact, DWORD dwMask);

/*
 * tray stuff
 */

void  TSAPI CreateSystrayIcon(int create);
void  TSAPI FlashTrayIcon(HICON hIcon);
void  TSAPI UpdateTrayMenuState(TWindowData *dat, BOOL bForced);
void  TSAPI LoadFavoritesAndRecent();
void  TSAPI AddContactToFavorites(MCONTACT hContact, const TCHAR *szNickname, const char *szProto, TCHAR *szStatus,
                                     WORD wStatus, HICON hIcon, BOOL mode, HMENU hMenu);
void  TSAPI CreateTrayMenus(int mode);
void  TSAPI HandleMenuEntryFromhContact(MCONTACT iSelection);

/*
 * gneric msgwindow functions(creation, container management etc.)
 */

BOOL  TSAPI IsUtfSendAvailable(MCONTACT hContact);
HWND  TSAPI CreateNewTabForContact(TContainerData *pContainer, MCONTACT hContact, int isSend,
                                     const char *pszInitialText, BOOL bActivateTAb, BOOL bPopupContainer, BOOL bWantPopup, HANDLE hdbEvent);
int   TSAPI ActivateTabFromHWND(HWND hwndTab, HWND hwnd);
void  TSAPI FlashContainer(TContainerData *pContainer, int iMode, int iNum);
void  TSAPI CreateImageList(BOOL bInitial);

TContainerData* TSAPI FindMatchingContainer(const TCHAR *szName, MCONTACT hContact);
TContainerData* TSAPI CreateContainer(const TCHAR *name, int iTemp, MCONTACT hContactFrom);
TContainerData* TSAPI FindContainerByName(const TCHAR *name);

int   TSAPI GetTabIndexFromHWND(HWND hwndTab, HWND hwnd);
HWND  TSAPI GetHWNDFromTabIndex(HWND hwndTab, int idx);
int   TSAPI GetTabItemFromMouse(HWND hwndTab, POINT *pt);
void  TSAPI CloseOtherTabs(HWND hwndTab, TWindowData &dat);
int   TSAPI ActivateTabFromHWND(HWND hwndTab, HWND hwnd);
void  TSAPI AdjustTabClientRect(TContainerData *pContainer, RECT *rc);
void  TSAPI ReflashContainer(TContainerData *pContainer);

TContainerData* TSAPI AppendToContainerList(TContainerData *pContainer);
TContainerData* TSAPI RemoveContainerFromList(TContainerData *pContainer);

void  TSAPI CloseAllContainers();
void  TSAPI DeleteContainer(int iIndex);
void  TSAPI RenameContainer(int iIndex, const TCHAR *newName);
int   TSAPI GetContainerNameForContact(MCONTACT hContact, TCHAR *szName, int iNameLen);
HMENU TSAPI BuildContainerMenu();
void  TSAPI BuildCodePageList();
void  TSAPI PreTranslateDates();
void  TSAPI ApplyContainerSetting(TContainerData *pContainer, DWORD flags, UINT mode, bool fForceResize);
void  TSAPI BroadCastContainer(const TContainerData *pContainer, UINT message, WPARAM wParam, LPARAM lParam, BYTE iType = 0);
void  TSAPI GetDefaultContainerTitleFormat();
void  TSAPI SetAeroMargins(TContainerData *pContainer);
int   TABSRMM_FireEvent(MCONTACT hContact, HWND hwnd, unsigned int type, unsigned int subType);

INT_PTR MessageWindowOpened(WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK IEViewSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HPPKFSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * skinning engine
 */
void  TSAPI DrawAlpha(HDC hdcwnd, PRECT rc, DWORD basecolor, int alpha, DWORD basecolor2,
                                     BYTE transparent, BYTE FLG_GRADIENT, BYTE FLG_CORNER, DWORD BORDERSTYLE, CImageItem *imageItem);
// the cached message log icons

void  TSAPI CacheMsgLogIcons();
void  TSAPI CacheLogFonts();
void  TSAPI InitAPI();
void  TSAPI LoadIconTheme();
int   TSAPI LoadFromIconLib();
int   TSAPI SetupIconLibConfig();
void  TSAPI RTF_CTableInit();

INT_PTR CALLBACK DlgProcMessage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcContainer(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

int DbEventIsForMsgWindow(DBEVENTINFO *dbei);

int   TSAPI InitOptions(void);
int   TSAPI DbEventIsShown(TWindowData *dat, DBEVENTINFO *dbei);
void  TSAPI StreamInEvents(HWND hwndDlg,HANDLE hDbEventFirst,int count,int fAppend, DBEVENTINFO *dbei_s);
void  TSAPI LoadLogfont(int i,LOGFONTA *lf,COLORREF *colour, char *szModule);

// custom tab control

void  TSAPI ReloadTabConfig();
void  TSAPI FreeTabConfig();
int   TSAPI RegisterTabCtrlClass(void);

// buttons

int   TSAPI UnloadTSButtonModule();

/*
 * debugging support
 */

int   _DebugPopup(MCONTACT hContact, const TCHAR *fmt, ...);
int   _DebugMessage(HWND hwndDlg, TWindowData *dat, const char *fmt, ...);

// themes

const TCHAR* TSAPI GetThemeFileName(int iMode);
void  TSAPI LoadLogfontFromINI(int i, char *szKey, LOGFONTA *lf, COLORREF *colour, const char *szIniFilename);
int   TSAPI CheckThemeVersion(const TCHAR *szIniFilename);
void  TSAPI WriteThemeToINI(const TCHAR *szIniFilename, TWindowData *dat);
void  TSAPI ReadThemeFromINI(const TCHAR *szIniFilename, TContainerData *dat, int noAdvanced, DWORD dwFlags);

// compatibility

// user prefs

int   TSAPI LoadLocalFlags(HWND hwnd, TWindowData *dat);

//TypingNotify
int   TN_ModuleInit();
int   TN_OptionsInitialize(WPARAM wParam, LPARAM lParam);
int   TN_ModuleDeInit();
void  TN_TypingMessage(MCONTACT hContact, int iMode);

// hotkeys

LRESULT ProcessHotkeysByMsgFilter(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR ctrlId);

void TSAPI DrawMenuItem(DRAWITEMSTRUCT *dis, HICON hIcon, DWORD dwIdle);

/*
 * dialog procedures
 */

INT_PTR CALLBACK SelectContainerDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcContainerOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif /* _TABSRMM_FUNCTIONS_H */
