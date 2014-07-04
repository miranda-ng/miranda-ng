/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#ifndef M_CLISTINT_H__
#define M_CLISTINT_H__ 1

#include <CommCtrl.h>

#include "m_genmenu.h"
#include "m_protocols.h"
#include "m_clc.h"

#define HCONTACT_ISGROUP    0x80000000
#define HCONTACT_ISINFO     0xFFFF0000
#define IsHContactGroup(h)  (((DWORD)(h)^HCONTACT_ISGROUP)<(HCONTACT_ISGROUP^HCONTACT_ISINFO))
#define IsHContactInfo(h)   (((DWORD)(h)&HCONTACT_ISINFO) == HCONTACT_ISINFO)
#define IsHContactContact(h) (((DWORD)(h)&HCONTACT_ISGROUP) == 0)

#ifndef EXTRA_ICON_COUNT
#define EXTRA_ICON_COUNT 10
#endif

#define MAX_TIP_SIZE 2048

#define INTM_NAMECHANGED     (WM_USER+10)
#define INTM_ICONCHANGED     (WM_USER+11)
#define INTM_GROUPCHANGED    (WM_USER+12)
#define INTM_GROUPSCHANGED   (WM_USER+13)
#define INTM_CONTACTADDED    (WM_USER+14)
#define INTM_CONTACTDELETED  (WM_USER+15)
#define INTM_HIDDENCHANGED   (WM_USER+16)
#define INTM_INVALIDATE      (WM_USER+17)
#define INTM_APPARENTMODECHANGED (WM_USER+18)
#define INTM_SETINFOTIPHOVERTIME (WM_USER+19)
#define INTM_NOTONLISTCHANGED   (WM_USER+20)
#define INTM_RELOADOPTIONS   (WM_USER+21)
#define INTM_NAMEORDERCHANGED (WM_USER+22)
#define INTM_IDLECHANGED         (WM_USER+23)
#define INTM_SCROLLBARCHANGED (WM_USER+24)
#define INTM_PROTOCHANGED (WM_USER+25)

#define TIMERID_RENAME           10
#define TIMERID_DRAGAUTOSCROLL   11
#define TIMERID_INFOTIP          13
#define TIMERID_REBUILDAFTER     14
#define TIMERID_DELAYEDRESORTCLC 15
#define TIMERID_TRAYHOVER        16
#define TIMERID_TRAYHOVER_2      17

#define GROUP_ALLOCATE_STEP  8

struct ContactList
{
	struct ClcContact** items;
	int count, limit, increment;
	void* sortFunc;
};

struct EventList
{
	struct CListEvent** items;
	int count, limit, increment;
	void* sortFunc;
};

struct ClcGroup
{
	ContactList cl;
	int expanded, hideOffline, groupId;
	ClcGroup *parent;
	int scanIndex;
	int totalMembers;
};

struct ClcFontInfo
{
	HFONT hFont;
	int fontHeight, changed;
	COLORREF colour;
};

struct ClcContactBase
{
	BYTE type;
	BYTE flags;
	union {
		struct {
			int    iImage;
			MCONTACT hContact;
		};
		struct {
			WORD groupId;
			ClcGroup *group;
		};
	};
	WORD  iExtraImage[EXTRA_ICON_COUNT];
	TCHAR szText[120-EXTRA_ICON_COUNT];
	char *proto; // MS_PROTO_GETBASEPROTO
};

struct ClcDataBase
{
	ClcGroup list;
	int rowHeight;
	int yScroll;
	int selection;
	ClcFontInfo fontInfo[FONTID_MAX + 1];
	int scrollTime;
	HIMAGELIST himlHighlight;
	int groupIndent;
	TCHAR szQuickSearch[128];
	int iconXSpace;
	HWND hwndRenameEdit;
	COLORREF bkColour, selBkColour, selTextColour, hotTextColour, quickSearchColour;
	int iDragItem, iInsertionMark;
	int dragStage;
	POINT ptDragStart;
	int dragAutoScrolling;
	int dragAutoScrollHeight;
	int leftMargin;
	int insertionMarkHitHeight;
	HBITMAP hBmpBackground;
	int backgroundBmpUse, bkChanged;
	int iHotTrack;
	int gammaCorrection;
	DWORD greyoutFlags;			  //see m_clc.h
	DWORD offlineModes;
	DWORD exStyle;
	POINT ptInfoTip;
	int infoTipTimeout;
	HANDLE hInfoTipItem;
	HIMAGELIST himlExtraColumns;
	int extraColumnsCount;
	int extraColumnSpacing;
	int checkboxSize;
	int showSelAlways;
	int showIdle;
	int noVScrollbar;
	int useWindowsColours;
	int needsResort;
	int filterSearch;
};

struct ClcContact;
struct ClcData;
struct CListEvent;
struct ClcCacheEntry;

struct trayIconInfo_t
{
	int    id;
	char*  szProto;
	HICON  hBaseIcon;
	int    isBase;
	TCHAR* ptszToolTip;
};

typedef struct _menuProto
{
	char *szProto; //This is DLL-based unique name
	HGENMENU pMenu;
	HICON hIcon;
}
	MenuProto;

/* constants */

#define DRAGSTAGE_NOTMOVED  0
#define DRAGSTAGE_ACTIVE    1
#define DRAGSTAGEM_STAGE    0x00FF
#define DRAGSTAGEF_MAYBERENAME  0x8000
#define DRAGSTAGEF_OUTSIDE      0x4000

#define CONTACTF_ONLINE    1
#define CONTACTF_INVISTO   2
#define CONTACTF_VISTO     4
#define CONTACTF_NOTONLIST 8
#define CONTACTF_CHECKED   16
#define CONTACTF_IDLE      32

#define DROPTARGET_OUTSIDE    0
#define DROPTARGET_ONSELF     1
#define DROPTARGET_ONNOTHING  2
#define DROPTARGET_ONGROUP    3
#define DROPTARGET_ONCONTACT  4
#define DROPTARGET_INSERTION  5

#define CLCDEFAULT_ROWHEIGHT     16
#define CLCDEFAULT_EXSTYLE       (CLS_EX_EDITLABELS|CLS_EX_TRACKSELECT|CLS_EX_SHOWGROUPCOUNTS|CLS_EX_HIDECOUNTSWHENEMPTY|CLS_EX_TRACKSELECT|CLS_EX_NOTRANSLUCENTSEL)  //plus CLS_EX_NOSMOOTHSCROLL is got from the system
#define CLCDEFAULT_SCROLLTIME    150
#define CLCDEFAULT_GROUPINDENT   5
#define CLCDEFAULT_BKCOLOUR      GetSysColor(COLOR_3DFACE)
#define CLCDEFAULT_USEBITMAP     0
#define CLCDEFAULT_BKBMPUSE      CLB_STRETCH
#define CLCDEFAULT_OFFLINEMODES  MODEF_OFFLINE
#define CLCDEFAULT_GREYOUTFLAGS  0
#define CLCDEFAULT_FULLGREYOUTFLAGS  (MODEF_OFFLINE|PF2_INVISIBLE|GREYF_UNFOCUS)
#define CLCDEFAULT_SELBLEND      1
#define CLCDEFAULT_SELBKCOLOUR   GetSysColor(COLOR_HIGHLIGHT)
#define CLCDEFAULT_TEXTCOLOUR    GetSysColor(COLOR_WINDOWTEXT)
#define CLCDEFAULT_SELTEXTCOLOUR GetSysColor(COLOR_HIGHLIGHTTEXT)
#define CLCDEFAULT_HOTTEXTCOLOUR RGB(0, 0, 255)
#define CLCDEFAULT_QUICKSEARCHCOLOUR RGB(255, 255, 0)
#define CLCDEFAULT_LEFTMARGIN    0
#define CLCDEFAULT_RIGHTMARGIN   2
#define CLCDEFAULT_GAMMACORRECT  1
#define CLCDEFAULT_SHOWIDLE      1
#define CLCDEFAULT_USEWINDOWSCOLOURS 0

#define TRAYICON_ID_BASE    100
#define TIM_CALLBACK   (WM_USER+1857)
#define TIM_CREATE     (WM_USER+1858)

/***************************************************************************
 * CLIST_INTERFACE structure definition
 ***************************************************************************/

typedef struct {
	char *szProto;
	DWORD dwStatus;
}
	ClcProtoStatus;

typedef struct
{
	MCONTACT hContact;
	TCHAR* tszName;
	TCHAR* tszGroup;
	int    bIsHidden;
}
	ClcCacheEntryBase;

typedef struct
{
	int version;

	HWND hwndContactList, hwndContactTree, hwndStatus;
	HMENU hMenuMain;
	HMODULE hInst;

	int hClcProtoCount;
	ClcProtoStatus *clcProto;

	/* clc.h */
	void  (*pfnClcOptionsChanged)(void);
	void  (*pfnClcBroadcast)(int, WPARAM, LPARAM);
	HMENU (*pfnBuildGroupPopupMenu)(struct ClcGroup*);

	LRESULT (CALLBACK *pfnContactListControlWndProc)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	/* clcfiledrop.c */
	void (*pfnRegisterFileDropping)(HWND hwnd);
	void (*pfnUnregisterFileDropping)(HWND hwnd);

	/* clcidents.c */
	int    (*pfnGetRowsPriorTo)(ClcGroup *group, ClcGroup *subgroup, int contactIndex);
	int    (*pfnFindItem)(HWND hwnd, struct ClcData *dat, DWORD dwItem, ClcContact **contact, ClcGroup **subgroup, int *isVisible);
	int    (*pfnGetRowByIndex)(struct ClcData *dat, int testindex, ClcContact **contact, ClcGroup **subgroup);
	HANDLE (*pfnContactToHItem)(ClcContact *contact);
	HANDLE (*pfnContactToItemHandle)(ClcContact *contact, DWORD *nmFlags);

	/* clcitems.c */
	ClcContact* (*pfnCreateClcContact)(void);
	struct ClcGroup* (*pfnAddGroup)(HWND hwnd, struct ClcData *dat, const TCHAR *szName, DWORD flags, int groupId, int calcTotalMembers);
	struct ClcGroup* (*pfnRemoveItemFromGroup)(HWND hwnd, ClcGroup *group, ClcContact *contact, int updateTotalCount);

	void (*pfnFreeContact)(ClcContact *contact);
	void (*pfnFreeGroup)(ClcGroup *group);

	int  (*pfnAddInfoItemToGroup)(ClcGroup *group, int flags, const TCHAR *pszText);
	int  (*pfnAddItemToGroup)(ClcGroup *group, int iAboveItem);
	int  (*pfnAddContactToGroup)(struct ClcData *dat, ClcGroup *group, MCONTACT hContact);
	void (*pfnAddContactToTree)(HWND hwnd, struct ClcData *dat, MCONTACT hContact, int updateTotalCount, int checkHideOffline);
	void (*pfnDeleteItemFromTree)(HWND hwnd, MCONTACT hItem);
	void (*pfnRebuildEntireList)(HWND hwnd, struct ClcData *dat);
	int  (*pfnGetGroupContentsCount)(ClcGroup *group, int visibleOnly);
	void (*pfnSortCLC)(HWND hwnd, struct ClcData *dat, int useInsertionSort);
	void (*pfnSaveStateAndRebuildList)(HWND hwnd, struct ClcData *dat);

	/* clcmsgs.c */
	LRESULT (*pfnProcessExternalMessages)(HWND hwnd, struct ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam);

	/* clcpaint.c */
	void  (*pfnPaintClc)(HWND hwnd, struct ClcData *dat, HDC hdc, RECT * rcPaint);

	/* clcutils.c */
	char* (*pfnGetGroupCountsText)(struct ClcData *dat, ClcContact *contact);
	int   (*pfnHitTest)(HWND hwnd, struct ClcData *dat, int testx, int testy, ClcContact **contact, ClcGroup **group, DWORD * flags);
	void  (*pfnScrollTo)(HWND hwnd, struct ClcData *dat, int desty, int noSmooth);
	void  (*pfnEnsureVisible)(HWND hwnd, struct ClcData *dat, int iItem, int partialOk);
	void  (*pfnRecalcScrollBar)(HWND hwnd, struct ClcData *dat);
	void  (*pfnSetGroupExpand)(HWND hwnd, struct ClcData *dat, ClcGroup *group, int newState);
	void  (*pfnDoSelectionDefaultAction)(HWND hwnd, struct ClcData *dat);
	int   (*pfnFindRowByText)(HWND hwnd, struct ClcData *dat, const TCHAR *text, int prefixOk);
	void  (*pfnEndRename)(HWND hwnd, struct ClcData *dat, int save);
	void  (*pfnDeleteFromContactList)(HWND hwnd, struct ClcData *dat);
	void  (*pfnBeginRenameSelection)(HWND hwnd, struct ClcData *dat);
	void  (*pfnCalcEipPosition)(struct ClcData *dat, ClcContact *contact, ClcGroup *group, POINT *result);
	int   (*pfnGetDropTargetInformation)(HWND hwnd, struct ClcData *dat, POINT pt);
	int   (*pfnClcStatusToPf2)(int status);
	int   (*pfnIsHiddenMode)(struct ClcData *dat, int status);
	void  (*pfnHideInfoTip)(HWND hwnd, struct ClcData *dat);
	void  (*pfnNotifyNewContact)(HWND hwnd, MCONTACT hContact);
	DWORD (*pfnGetDefaultExStyle)(void);
	void  (*pfnGetDefaultFontSetting)(int i, LOGFONT* lf, COLORREF* colour);
	void  (*pfnGetFontSetting)(int i, LOGFONT* lf, COLORREF* colour);
	void  (*pfnLoadClcOptions)(HWND hwnd, struct ClcData *dat);
	void  (*pfnRecalculateGroupCheckboxes)(HWND hwnd, struct ClcData *dat);
	void  (*pfnSetGroupChildCheckboxes)(ClcGroup *group, int checked);
	void  (*pfnInvalidateItem)(HWND hwnd, struct ClcData *dat, int iItem);

	int   (*pfnGetRowBottomY)(struct ClcData *dat, int item);
	int   (*pfnGetRowHeight)(struct ClcData *dat, int item);
	int   (*pfnGetRowTopY)(struct ClcData *dat, int item);
	int   (*pfnGetRowTotalHeight)(struct ClcData *dat);
	int   (*pfnRowHitTest)(struct ClcData *dat, int y);

	/* clistevents.c */
	int   (*pfnEventsProcessContactDoubleClick)(MCONTACT hContact);
	int   (*pfnEventsProcessTrayDoubleClick)(int);

	/* clistmod.c */
	int    (*pfnIconFromStatusMode)(const char *szProto, int status, MCONTACT hContact);
	int    (*pfnShowHide)(WPARAM, LPARAM);
	TCHAR* (*pfnGetStatusModeDescription)(int mode, int flags);

	/* clistsettings.c */
	ClcCacheEntry* (*pfnGetCacheEntry)(MCONTACT hContact);
	ClcCacheEntry* (*pfnCreateCacheItem)(MCONTACT hContact);
	void           (*pfnCheckCacheItem)(ClcCacheEntry*);
	void           (*pfnFreeCacheItem)(ClcCacheEntry*);

	TCHAR* (*pfnGetContactDisplayName)(MCONTACT hContact, int mode);
	void   (*pfnInvalidateDisplayNameCacheEntry)(MCONTACT hContact);

	/* clisttray.c */
	void (*pfnTrayIconUpdateWithImageList)(int iImage, const TCHAR *szNewTip, char *szPreferredProto);
	void (*pfnTrayIconUpdateBase)(const char *szChangedProto);
	void (*pfnTrayIconSetToBase)(char *szPreferredProto);
	void (*pfnTrayIconIconsChanged)(void);
	int  (*pfnTrayIconPauseAutoHide)(WPARAM wParam, LPARAM lParam);
	INT_PTR (*pfnTrayIconProcessMessage)(WPARAM wParam, LPARAM lParam);
	int  (*pfnCListTrayNotify)(MIRANDASYSTRAYNOTIFY*);

	/* clui.c */
	LRESULT (CALLBACK *pfnContactListWndProc)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void (*pfnCluiProtocolStatusChanged)(int status, const char *szProto);
	void (*pfnDrawMenuItem)(LPDRAWITEMSTRUCT, HICON, HICON);
	void (*pfnLoadCluiGlobalOpts)(void);
	BOOL (*pfnInvalidateRect)(HWND hwnd, CONST RECT* lpRect, BOOL bErase);
	void (*pfnOnCreateClc)(void);

	/* contact.c */
	void (*pfnChangeContactIcon)(MCONTACT hContact, int iIcon, int add);
	void (*pfnLoadContactTree)(void);
	int  (*pfnCompareContacts)(const ClcContact *contact1, const ClcContact *contact2);
	void (*pfnSortContacts)(void);
	int  (*pfnSetHideOffline)(WPARAM wParam, LPARAM lParam);

	/* docking.c */
	int (*pfnDocking_ProcessWindowMessage)(WPARAM wParam, LPARAM lParam);

	/* group.c */
	TCHAR* (*pfnGetGroupName)(int idx, DWORD* pdwFlags);
	int    (*pfnRenameGroup)(int groupID, TCHAR* newName);

	/* keyboard.c */
	int   (*pfnHotKeysRegister)(HWND hwnd);
	void  (*pfnHotKeysUnregister)(HWND hwnd);
	int   (*pfnHotKeysProcess)(HWND hwnd, WPARAM wParam, LPARAM lParam);
	int   (*pfnHotkeysProcessMessage)(WPARAM wParam, LPARAM lParam);

	/*************************************************************************************
	 * version 2 - events processing
	 *************************************************************************************/

	EventList events;

	struct CListEvent* (*pfnCreateEvent)(void);
	void  (*pfnFreeEvent)(struct CListEvent*);

	struct CListEvent* (*pfnAddEvent)(CLISTEVENT*);
	CLISTEVENT* (*pfnGetEvent)(MCONTACT hContact, int idx);

	int   (*pfnRemoveEvent)(MCONTACT hContact, HANDLE hDbEvent);
	int   (*pfnGetImlIconIndex)(HICON hIcon);

	/*************************************************************************************
	 * version 3 additions
	 *************************************************************************************/

	int   (*pfnGetWindowVisibleState)(HWND hWnd, int iStepX, int iStepY);

	/*************************************************************************************
	 * version 4 additions (0.7.0.x) - genmenu
	 *************************************************************************************/

	MenuProto* menuProtos;
	int        menuProtoCount;

	HANDLE hPreBuildStatusMenuEvent;
	int    currentStatusMenuItem, currentDesiredStatusMode;
	BOOL   bDisplayLocked, bAutoRebuild;

	HGENMENU (*pfnGetProtocolMenu)(const char*);
	int      (*pfnStub2)(int);

	int    (*pfnGetProtocolVisibility)(const char*);
	int    (*pfnGetProtoIndexByPos)(PROTOCOLDESCRIPTOR** proto, int protoCnt, int Pos);
	void   (*pfnReloadProtoMenus)(void);

	/*************************************************************************************
	 * version 5 additions (0.7.0.x) - tray icons
	 *************************************************************************************/

	struct   trayIconInfo_t* trayIcon;
	int      trayIconCount;
	int      shellVersion;
	UINT_PTR cycleTimerId;
	int      cycleStep;
	TCHAR*   szTip;
	BOOL     bTrayMenuOnScreen;

	HICON  (*pfnGetIconFromStatusMode)(MCONTACT hContact, const char *szProto, int status);

	void   (*pfnInitTray)(void);
	int    (*pfnTrayIconAdd)(HWND hwnd, const char *szProto, const char *szIconProto, int status);
	int    (*pfnTrayIconDestroy)(HWND hwnd);
	int    (*pfnTrayIconInit)(HWND hwnd);
	TCHAR* (*pfnTrayIconMakeTooltip)(const TCHAR *szPrefix, const char *szProto);
	void   (*pfnTrayIconRemove)(HWND hwnd, const char *szProto);
	int    (*pfnTrayIconSetBaseInfo)(HICON hIcon, const char *szPreferredProto);
	void   (*pfnTrayIconTaskbarCreated)(HWND hwnd);
	int    (*pfnTrayIconUpdate)(HICON hNewIcon, const TCHAR *szNewTip, const char *szPreferredProto, int isBase);

	void   (*pfnUninitTray)(void);
	void   (*pfnLockTray)(void);
	void   (*pfnUnlockTray)(void);

	VOID   (CALLBACK *pfnTrayCycleTimerProc)(HWND hwnd, UINT message, UINT_PTR idEvent, DWORD dwTime);

	/*************************************************************************************
	 * version 6 additions (0.8.0.x) - accounts
	 *************************************************************************************/
	int    (*pfnGetAccountIndexByPos)(int pos);
	int    (*pfnConvertMenu)(CLISTMENUITEM*, TMO_MenuItem*);

	/*************************************************************************************
	 * version 7 additions (0.11.0.x) - extra images
	 *************************************************************************************/
	void   (*pfnReloadExtraIcons)(void);
	void   (*pfnSetAllExtraIcons)(MCONTACT hContact);

	/*************************************************************************************
	 * Miranda NG additions
	 *************************************************************************************/
	int    (*pfnGetContactIcon)(MCONTACT hContact);
	int    (*pfnTrayCalcChanged)(const char *szChangedProto, int averageMode, int iProtoCount);
	int    (*pfnGetAverageMode)(int *pNetProtoCount);
	void   (*pfnInitAutoRebuild)(HWND hwnd);
	void   (*pfnSetContactCheckboxes)(ClcContact *cc, int checked);
}
	CLIST_INTERFACE;

extern CLIST_INTERFACE cli, *pcli;

// Miranda 0.4.3.0+
// retrieves the pointer to a CLIST_INTERFACE structure
// NOTE: valid only for the clist clone building, not for the regular use

#define MS_CLIST_RETRIEVE_INTERFACE "CList/RetrieveInterface"

__forceinline void mir_getCLI()
{	pcli = (CLIST_INTERFACE*)CallService(MS_CLIST_RETRIEVE_INTERFACE, 0, 0);
}
#endif // M_CLISTINT_H__
