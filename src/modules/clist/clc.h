/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

void   fnClcOptionsChanged(void);
void   fnClcBroadcast(int msg, WPARAM wParam, LPARAM lParam);
HMENU  fnBuildGroupPopupMenu(ClcGroup* group);
void   fnInitAutoRebuild(HWND hWnd);

LRESULT CALLBACK fnContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/* clcidents.c */
int       fnGetRowsPriorTo(ClcGroup *group, ClcGroup *subgroup, int contactIndex);
int       fnFindItem(HWND hwnd, struct ClcData *dat, DWORD dwItem, ClcContact **contact, ClcGroup **subgroup, int *isVisible);
int       fnGetRowByIndex(struct ClcData *dat, int testindex, ClcContact **contact, ClcGroup **subgroup);
HANDLE    fnContactToHItem(ClcContact *contact);
HANDLE    fnContactToItemHandle(ClcContact *contact, DWORD *nmFlags);

/* clcitems.c */
ClcGroup* fnAddGroup(HWND hwnd, struct ClcData *dat, const TCHAR *szName, DWORD flags, int groupId, int calcTotalMembers);
ClcGroup* fnRemoveItemFromGroup(HWND hwnd, ClcGroup *group, ClcContact *contact, int updateTotalCount);

void fnFreeContact(ClcContact *p);
void fnFreeGroup(ClcGroup *group);
int  fnAddInfoItemToGroup(ClcGroup *group, int flags, const TCHAR *pszText);
int  fnAddItemToGroup(ClcGroup *group, int iAboveItem);
void fnAddContactToTree(HWND hwnd, struct ClcData *dat, MCONTACT hContact, int updateTotalCount, int checkHideOffline);
int  fnAddContactToGroup(struct ClcData *dat, ClcGroup *group, MCONTACT hContact);
void fnDeleteItemFromTree(HWND hwnd, MCONTACT hItem);
void fnRebuildEntireList(HWND hwnd, struct ClcData *dat);
int  fnGetGroupContentsCount(ClcGroup *group, int visibleOnly);
void fnSortCLC(HWND hwnd, struct ClcData *dat, int useInsertionSort);
void fnSaveStateAndRebuildList(HWND hwnd, struct ClcData *dat);

/* clcmsgs.c */
LRESULT fnProcessExternalMessages(HWND hwnd, struct ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam);

/* clcutils.c */
char* fnGetGroupCountsText(struct ClcData *dat, ClcContact *contact);
int   fnHitTest(HWND hwnd, struct ClcData *dat, int testx, int testy, ClcContact **contact, ClcGroup **group, DWORD * flags);
void  fnScrollTo(HWND hwnd, struct ClcData *dat, int desty, int noSmooth);
void  fnEnsureVisible(HWND hwnd, struct ClcData *dat, int iItem, int partialOk);
void  fnRecalcScrollBar(HWND hwnd, struct ClcData *dat);
void  fnSetGroupExpand(HWND hwnd, struct ClcData *dat, ClcGroup *group, int newState);
void  fnDoSelectionDefaultAction(HWND hwnd, struct ClcData *dat);
int   fnFindRowByText(HWND hwnd, struct ClcData *dat, const TCHAR *text, int prefixOk);
void  fnEndRename(HWND hwnd, struct ClcData *dat, int save);
void  fnDeleteFromContactList(HWND hwnd, struct ClcData *dat);
void  fnBeginRenameSelection(HWND hwnd, struct ClcData *dat);
void  fnCalcEipPosition(struct ClcData *dat, ClcContact *contact, ClcGroup *group, POINT *result);
int   fnGetDropTargetInformation(HWND hwnd, struct ClcData *dat, POINT pt);
int   fnClcStatusToPf2(int status);
int   fnIsHiddenMode(struct ClcData *dat, int status);
void  fnHideInfoTip(HWND hwnd, struct ClcData *dat);
void  fnNotifyNewContact(HWND hwnd, MCONTACT hContact);
DWORD fnGetDefaultExStyle(void);
void  fnGetSetting(int i, LOGFONT* lf, COLORREF* colour);
void  fnGetDefaultFontSetting(int i, LOGFONT* lf, COLORREF* colour);
void  fnGetFontSetting(int i, LOGFONT* lf, COLORREF* colour);
void  fnLoadClcOptions(HWND hwnd, struct ClcData *dat, BOOL bFirst);
void  fnRecalculateGroupCheckboxes(HWND hwnd, struct ClcData *dat);
void  fnSetGroupChildCheckboxes(ClcGroup *group, int checked);
void  fnSetContactCheckboxes(ClcContact *cc, int checked);
void  fnInvalidateItem(HWND hwnd, struct ClcData *dat, int iItem);

int fnGetRowBottomY(struct ClcData *dat, int item);
int fnGetRowHeight(struct ClcData *dat, int item);
int fnGetRowTopY(struct ClcData *dat, int item);
int fnGetRowTotalHeight(struct ClcData *dat);
int fnRowHitTest(struct ClcData *dat, int y);

/* clcopts.c */
int ClcOptInit(WPARAM wParam, LPARAM lParam);
void GetFontSetting(int i, LOGFONTA *lf, COLORREF *colour);

/* clistmenus.c */
HGENMENU fnGetProtocolMenu(const char*);
int      fnGetProtocolVisibility(const char* accName);
int      fnConvertMenu(CLISTMENUITEM*, TMO_MenuItem*);
int      fnGetAverageMode(int *pNetProtoCount);

int      fnGetAccountIndexByPos(int Pos);
int      fnGetProtoIndexByPos(PROTOCOLDESCRIPTOR **proto, int protoCnt, int Pos);
void     RebuildMenuOrder(void);

INT_PTR  MenuProcessCommand(WPARAM wParam, LPARAM lParam);

/* clistsettings.c */
TCHAR* fnGetContactDisplayName(MCONTACT hContact, int mode);
void   fnGetDefaultFontSetting(int i, LOGFONT* lf, COLORREF * colour);
void   fnInvalidateDisplayNameCacheEntry(MCONTACT hContact);

ClcCacheEntry* fnGetCacheEntry(MCONTACT hContact);
ClcCacheEntry* fnCreateCacheItem (MCONTACT hContact);
void fnCheckCacheItem(ClcCacheEntry *p);
void fnFreeCacheItem(ClcCacheEntry *p);

/* clcfiledrop.c */
void InitFileDropping(void);

void   fnRegisterFileDropping (HWND hwnd);
void   fnUnregisterFileDropping (HWND hwnd);

/* clistevents.c */
struct CListEvent* fnAddEvent(CLISTEVENT *cle);
CLISTEVENT* fnGetEvent(MCONTACT hContact, int idx);

struct CListEvent* fnCreateEvent(void);
void fnFreeEvent(struct CListEvent* p);

int   fnEventsProcessContactDoubleClick(MCONTACT hContact);
int   fnEventsProcessTrayDoubleClick(int);
int   fnGetImlIconIndex(HICON hIcon);
int   fnRemoveEvent(MCONTACT hContact, HANDLE dbEvent);

/* clistmod.c */
int    fnGetContactIcon(MCONTACT hContact);
int    fnIconFromStatusMode(const char *szProto, int status, MCONTACT hContact);
int    fnShowHide(WPARAM wParam, LPARAM lParam);
HICON  fnGetIconFromStatusMode(MCONTACT hContact, const char *szProto, int status);
TCHAR* fnGetStatusModeDescription(int wParam, int lParam);
int    fnGetWindowVisibleState(HWND hWnd, int iStepX, int iStepY);

/* clisttray.c */
extern mir_cs trayLockCS;

void   fnInitTray(void);
void   fnUninitTray(void);
int    fnCListTrayNotify(MIRANDASYSTRAYNOTIFY *msn);
int    fnTrayIconAdd(HWND hwnd, const char *szProto, const char *szIconProto, int status);
int    fnTrayIconDestroy(HWND hwnd);
void   fnTrayIconIconsChanged (void);
int    fnTrayIconInit(HWND hwnd);
TCHAR* fnTrayIconMakeTooltip(const TCHAR *szPrefix, const char *szProto);
int    fnTrayIconPauseAutoHide (WPARAM wParam, LPARAM lParam);
INT_PTR    fnTrayIconProcessMessage (WPARAM wParam, LPARAM lParam);
void   fnTrayIconRemove(HWND hwnd, const char *szProto);
int    fnTrayIconSetBaseInfo(HICON hIcon, const char *szPreferredProto);
void   fnTrayIconSetToBase (char *szPreferredProto);
void   fnTrayIconTaskbarCreated(HWND hwnd);
int    fnTrayIconUpdate(HICON hNewIcon, const TCHAR *szNewTip, const char *szPreferredProto, int isBase);
void   fnTrayIconUpdateBase (const char *szChangedProto);
int    fnTrayCalcChanged(const char *szChangedProto, int averageMode, int netProtoCount);
void   fnTrayIconUpdateWithImageList (int iImage, const TCHAR *szNewTip, char *szPreferredProto);

VOID CALLBACK fnTrayCycleTimerProc(HWND hwnd, UINT message, UINT_PTR idEvent, DWORD dwTime);

/* clui.c */
LRESULT CALLBACK fnContactListWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void fnLoadCluiGlobalOpts(void);
void fnCluiProtocolStatusChanged(int, const char*);
void fnDrawMenuItem(DRAWITEMSTRUCT *dis, HICON hIcon, HICON eventIcon);

/* contact.c */
void fnChangeContactIcon (MCONTACT hContact, int iIcon, int add);
void fnLoadContactTree (void);
int  fnCompareContacts (const ClcContact *contact1, const ClcContact *contact2);
void fnSortContacts (void);
int  fnSetHideOffline (WPARAM wParam, LPARAM lParam);

/* docking.c */
int fnDocking_ProcessWindowMessage (WPARAM wParam, LPARAM lParam);

/* group.c */
TCHAR* fnGetGroupName (int idx, DWORD* pdwFlags);
int    fnRenameGroup (int groupID, TCHAR* newName);

/* keyboard.c */
int  fnHotKeysRegister (HWND hwnd);
void fnHotKeysUnregister (HWND hwnd);
int  fnHotKeysProcess (HWND hwnd, WPARAM wParam, LPARAM lParam);
int  fnHotkeysProcessMessage (WPARAM wParam, LPARAM lParam);
