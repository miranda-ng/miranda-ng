/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#pragma once

struct ClcContact : public ClcContactBase
{
};

struct ClcData : public ClcDataBase
{
};

struct ClcCacheEntry : public ClcCacheEntryBase
{
};

/* clc.c */
extern int g_IconWidth, g_IconHeight;
extern HIMAGELIST hCListImages;
extern bool g_bReadyToInitClist;
extern OBJLIST<MenuProto> g_menuProtos;

LRESULT CALLBACK fnContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/* clcidents.c */
int fnGetRowsPriorTo(ClcGroup *group, ClcGroup *subgroup, int contactIndex);
int fnGetRowByIndex(ClcData *dat, int testindex, ClcContact **contact, ClcGroup **subgroup);

ClcContact* fnFindItem(uint32_t dwItem, ClcContact *contact);

/* clcitems.c */
ClcGroup* fnAddGroup(HWND hwnd, ClcData *dat, const wchar_t *szName, uint32_t flags, int groupId, int calcTotalMembers);

ClcContact* fnAddInfoItemToGroup(ClcGroup *group, int flags, const wchar_t *pszText);
ClcContact* fnAddItemToGroup(ClcGroup *group, int iAboveItem);
ClcContact* fnAddContactToGroup(ClcData *dat, ClcGroup *group, MCONTACT hContact);

void fnFreeContact(ClcContact *p);

void FreeGroup(ClcGroup *group);

void fnAddContactToTree(HWND hwnd, ClcData *dat, MCONTACT hContact, int updateTotalCount, int checkHideOffline);
void fnRebuildEntireList(HWND hwnd, ClcData *dat);
int  fnGetGroupContentsCount(ClcGroup *group, int visibleOnly);
void fnSortCLC(HWND hwnd, ClcData *dat, int useInsertionSort);
int  fnGetContactHiddenStatus(MCONTACT hContact, char *szProto, ClcData *dat);

/* clcmsgs.c */
LRESULT fnProcessExternalMessages(HWND hwnd, ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam);

/* clcutils.c */
int  fnHitTest(HWND hwnd, ClcData *dat, int testx, int testy, ClcContact **contact, ClcGroup **group, uint32_t * flags);
void fnScrollTo(HWND hwnd, ClcData *dat, int desty, int noSmooth);
void fnRecalcScrollBar(HWND hwnd, ClcData *dat);
void fnSetGroupExpand(HWND hwnd, ClcData *dat, ClcGroup *group, int newState);
int  fnFindRowByText(HWND hwnd, ClcData *dat, const wchar_t *text, int prefixOk);

void fnBeginRenameSelection(HWND hwnd, ClcData *dat);
void fnGetDefaultFontSetting(int i, LOGFONT *lf, COLORREF *colour);
void fnLoadClcOptions(HWND hwnd, ClcData *dat, BOOL bFirst);
void fnSetContactCheckboxes(ClcContact *cc, int checked);

int  fnGetRowBottomY(ClcData *dat, int item);
int  fnGetRowHeight(ClcData *dat, int item);
int  fnGetRowTopY(ClcData *dat, int item);
int  fnGetRowTotalHeight(ClcData *dat);
int  fnRowHitTest(ClcData *dat, int y);

int  GetDropTargetInformation(HWND hwnd, ClcData *dat, POINT pt);

/* clcopts.c */
int ClcOptInit(WPARAM, LPARAM);

namespace Clist
{
	extern CMOption<bool> RemoveTempContacts, DisableIconBlink;
};

/* clistmenus.c */
void RebuildMenuOrder(void);

/* clistsettings.c */
void InvalidateProtoInCache(const char *szModuleName);

void fnGetDefaultFontSetting(int i, LOGFONT *lf, COLORREF * colour);
void fnInvalidateDisplayNameCacheEntry(MCONTACT hContact);

ClcCacheEntry* fnCreateCacheItem(MCONTACT hContact);
void fnCheckCacheItem(ClcCacheEntry *p);
void fnFreeCacheItem(ClcCacheEntry *p);

/* clcfiledrop.c */
void RegisterFileDropping(HWND hwnd);
void UnregisterFileDropping(HWND hwnd);

/* clistevents.c */
struct CListEvent* fnAddEvent(CLISTEVENT *cle);
CLISTEVENT* fnGetEvent(MCONTACT hContact, int idx);

int  fnGetImlIconIndex(HICON hIcon);
int  fnRemoveEvent(MCONTACT hContact, MEVENT dbEvent);

int  EventsProcessContactDoubleClick(MCONTACT hContact);

int  InitCListEvents(void);
void UninitCListEvents(void);

/* clistgroups.c */
int  InitGroupServices(void);
void UninitGroupServices(void);

/* clistmod.c */
int   fnIconFromStatusMode(const char *szProto, int status, MCONTACT hContact);
int   fnShowHide(void);
HICON fnGetIconFromStatusMode(MCONTACT hContact, const char *szProto, int status);
int   fnGetWindowVisibleState(HWND hWnd, int iStepX, int iStepY);

/* clistsettings.cpp */
int ContactAdded(WPARAM, LPARAM);
int ContactDeleted(WPARAM, LPARAM);

/* clisttray.c */
extern mir_cs trayLockCS;

int     fnTrayIconInit(HWND hwnd);
int     fnTrayIconPauseAutoHide(WPARAM, LPARAM);
INT_PTR fnTrayIconProcessMessage(WPARAM, LPARAM);
int     fnTrayCalcChanged(const char *szChangedProto, int averageMode, int netProtoCount);

void    InitTray(void);
void    UninitTray(void);

void    TrayIconSetToBase(char *szPreferredProto);
void    TrayIconTaskbarCreated(HWND hwnd);
int     TrayIconUpdate(HICON hNewIcon, const wchar_t *szNewTip, const char *szPreferredProto, int isBase);
void    TrayIconUpdateWithImageList(int iImage, const wchar_t *szNewTip, const char *szPreferredProto);

/* clui.c */
LRESULT CALLBACK fnContactListWndProc(HWND, UINT, WPARAM, LPARAM);
void fnLoadCluiGlobalOpts(void);
void fnCluiProtocolStatusChanged(int, const char*);

int LoadCLUIModule(void);

/* contact.c */
int fnSetHideOffline(int iValue);

/* docking.c */
int fnDocking_ProcessWindowMessage(WPARAM wParam, LPARAM lParam);
