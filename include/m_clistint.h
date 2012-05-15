/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
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

#include "m_genmenu.h"
#include "m_protocols.h"

#define HCONTACT_ISGROUP    0x80000000
#define HCONTACT_ISINFO     0xFFFF0000
#define IsHContactGroup(h)  (((UINT_PTR)(h)^HCONTACT_ISGROUP)<(HCONTACT_ISGROUP^HCONTACT_ISINFO))
#define IsHContactInfo(h)   (((UINT_PTR)(h)&HCONTACT_ISINFO)==HCONTACT_ISINFO)
#define IsHContactContact(h) (((UINT_PTR)(h)&HCONTACT_ISGROUP)==0)
#define MAXEXTRACOLUMNS     16

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

struct ClcContact;
struct ClcData;
struct CListEvent;

/* templates, where are you... */

typedef struct
{
	struct ClcContact** items;
	int count, limit, increment;
	void* sortFunc;
}
	ContactList;

typedef struct
{
	struct CListEvent** items;
	int count, limit, increment;
	void* sortFunc;
}
	EventList;

struct ClcGroup
{
	ContactList cl;
	int expanded,hideOffline,groupId;
	struct ClcGroup *parent;
	int scanIndex;
	int totalMembers;
};

struct ClcFontInfo
{
	HFONT hFont;
	int fontHeight,changed;
	COLORREF colour;
};

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
	char* szProto; //This is DLL-based unique name
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
#define CLCDEFAULT_HOTTEXTCOLOUR (IsWinVer98Plus()?RGB(0,0,255):GetSysColor(COLOR_HOTLIGHT))
#define CLCDEFAULT_QUICKSEARCHCOLOUR RGB(255,255,0)
#define CLCDEFAULT_LEFTMARGIN    0
#define CLCDEFAULT_GAMMACORRECT  1
#define CLCDEFAULT_SHOWIDLE      1
#define CLCDEFAULT_USEWINDOWSCOLOURS 0

#define TRAYICON_ID_BASE    100
#define TIM_CALLBACK   (WM_USER+1857)
#define TIM_CREATE     (WM_USER+1858)

// Miranda 0.4.3.0+
// retrieves the pointer to a CLIST_INTERFACE structure
// NOTE: valid only for the clist clone building, not for the regular use

#define MS_CLIST_RETRIEVE_INTERFACE "CList/RetrieveInterface"

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
	HANDLE hContact;
	TCHAR* name;
	#if defined( _UNICODE )
		char* szName;
	#endif
	TCHAR* group;
	int    isHidden;
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
	void  ( *pfnClcOptionsChanged )( void );
	void  ( *pfnClcBroadcast )( int, WPARAM, LPARAM );
	HMENU ( *pfnBuildGroupPopupMenu )( struct ClcGroup* );

	LRESULT ( CALLBACK *pfnContactListControlWndProc )( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

	/* clcfiledrop.c */
	void ( *pfnRegisterFileDropping )( HWND hwnd );
	void ( *pfnUnregisterFileDropping )( HWND hwnd );

	/* clcidents.c */
	int    ( *pfnGetRowsPriorTo )( struct ClcGroup *group, struct ClcGroup *subgroup, int contactIndex );
	int    ( *pfnFindItem )( HWND hwnd, struct ClcData *dat, HANDLE hItem, struct ClcContact **contact, struct ClcGroup **subgroup, int *isVisible );
	int    ( *pfnGetRowByIndex )( struct ClcData *dat, int testindex, struct ClcContact **contact, struct ClcGroup **subgroup );
   HANDLE ( *pfnContactToHItem )( struct ClcContact* contact );
	HANDLE ( *pfnContactToItemHandle )( struct ClcContact * contact, DWORD * nmFlags );

	/* clcitems.c */
	struct ClcContact* ( *pfnCreateClcContact )( void );
	struct ClcGroup* ( *pfnAddGroup )( HWND hwnd, struct ClcData *dat, const TCHAR *szName, DWORD flags, int groupId, int calcTotalMembers );
	struct ClcGroup* ( *pfnRemoveItemFromGroup )(HWND hwnd, struct ClcGroup *group, struct ClcContact *contact, int updateTotalCount);

	void ( *pfnFreeContact )( struct ClcContact *contact );
	void ( *pfnFreeGroup )( struct ClcGroup *group );

	int  ( *pfnAddInfoItemToGroup )(struct ClcGroup *group, int flags, const TCHAR *pszText);
	int  ( *pfnAddItemToGroup )( struct ClcGroup *group,int iAboveItem );
	int  ( *pfnAddContactToGroup )( struct ClcData *dat, struct ClcGroup *group, HANDLE hContact );
	void ( *pfnAddContactToTree )( HWND hwnd, struct ClcData *dat, HANDLE hContact, int updateTotalCount, int checkHideOffline);
	void ( *pfnDeleteItemFromTree )( HWND hwnd, HANDLE hItem );
	void ( *pfnRebuildEntireList )( HWND hwnd, struct ClcData *dat );
	int  ( *pfnGetGroupContentsCount )( struct ClcGroup *group, int visibleOnly );
	void ( *pfnSortCLC )( HWND hwnd, struct ClcData *dat, int useInsertionSort );
	void ( *pfnSaveStateAndRebuildList )(HWND hwnd, struct ClcData *dat);

	/* clcmsgs.c */
	LRESULT ( *pfnProcessExternalMessages )(HWND hwnd, struct ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam );

	/* clcpaint.c */
	void  ( *pfnPaintClc )( HWND hwnd, struct ClcData *dat, HDC hdc, RECT * rcPaint );

	/* clcutils.c */
	char* ( *pfnGetGroupCountsText )(struct ClcData *dat, struct ClcContact *contact );
	int   ( *pfnHitTest )( HWND hwnd, struct ClcData *dat, int testx, int testy, struct ClcContact **contact, struct ClcGroup **group, DWORD * flags );
	void  ( *pfnScrollTo )( HWND hwnd, struct ClcData *dat, int desty, int noSmooth );
	void  ( *pfnEnsureVisible )(HWND hwnd, struct ClcData *dat, int iItem, int partialOk );
	void  ( *pfnRecalcScrollBar )( HWND hwnd, struct ClcData *dat );
	void  ( *pfnSetGroupExpand )( HWND hwnd, struct ClcData *dat, struct ClcGroup *group, int newState );
	void  ( *pfnDoSelectionDefaultAction )( HWND hwnd, struct ClcData *dat );
	int   ( *pfnFindRowByText )(HWND hwnd, struct ClcData *dat, const TCHAR *text, int prefixOk );
	void  ( *pfnEndRename )(HWND hwnd, struct ClcData *dat, int save );
	void  ( *pfnDeleteFromContactList )( HWND hwnd, struct ClcData *dat );
	void  ( *pfnBeginRenameSelection )( HWND hwnd, struct ClcData *dat );
	void  ( *pfnCalcEipPosition )( struct ClcData *dat, struct ClcContact *contact, struct ClcGroup *group, POINT *result);
	int   ( *pfnGetDropTargetInformation )( HWND hwnd, struct ClcData *dat, POINT pt );
	int   ( *pfnClcStatusToPf2 )( int status );
   int   ( *pfnIsHiddenMode )( struct ClcData *dat, int status );
	void  ( *pfnHideInfoTip )( HWND hwnd, struct ClcData *dat );
	void  ( *pfnNotifyNewContact )( HWND hwnd, HANDLE hContact );
	DWORD ( *pfnGetDefaultExStyle )( void );
	void  ( *pfnGetDefaultFontSetting )( int i, LOGFONT* lf, COLORREF* colour );
	void  ( *pfnGetFontSetting )( int i, LOGFONT* lf, COLORREF* colour );
	void  ( *pfnLoadClcOptions )( HWND hwnd, struct ClcData *dat );
	void  ( *pfnRecalculateGroupCheckboxes )( HWND hwnd, struct ClcData *dat );
	void  ( *pfnSetGroupChildCheckboxes )( struct ClcGroup *group, int checked );
	void  ( *pfnInvalidateItem )( HWND hwnd, struct ClcData *dat, int iItem );

	int   ( *pfnGetRowBottomY )(struct ClcData *dat, int item);
	int   ( *pfnGetRowHeight )(struct ClcData *dat, int item);
	int   ( *pfnGetRowTopY )(struct ClcData *dat, int item);
	int   ( *pfnGetRowTotalHeight )(struct ClcData *dat);
	int   ( *pfnRowHitTest )(struct ClcData *dat, int y);

	/* clistevents.c */
	int   ( *pfnEventsProcessContactDoubleClick )( HANDLE hContact );
	int   ( *pfnEventsProcessTrayDoubleClick )( int );

	/* clistmod.c */
	int    ( *pfnIconFromStatusMode )( const char *szProto, int status, HANDLE hContact );
	int    ( *pfnShowHide )( WPARAM, LPARAM );
	TCHAR* ( *pfnGetStatusModeDescription )( int mode, int flags );

	/* clistsettings.c */
	ClcCacheEntryBase* ( *pfnGetCacheEntry )( HANDLE hContact );
	ClcCacheEntryBase* ( *pfnCreateCacheItem )( HANDLE hContact );
	void               ( *pfnCheckCacheItem )( ClcCacheEntryBase* );
	void               ( *pfnFreeCacheItem )( ClcCacheEntryBase* );

	TCHAR* ( *pfnGetContactDisplayName )( HANDLE hContact, int mode );
	void   ( *pfnInvalidateDisplayNameCacheEntry )( HANDLE hContact );

	/* clisttray.c */
	void ( *pfnTrayIconUpdateWithImageList )( int iImage, const TCHAR *szNewTip, char *szPreferredProto );
	void ( *pfnTrayIconUpdateBase )( const char *szChangedProto );
	void ( *pfnTrayIconSetToBase )( char *szPreferredProto );
	void ( *pfnTrayIconIconsChanged )( void );
	int  ( *pfnTrayIconPauseAutoHide )( WPARAM wParam, LPARAM lParam );
	INT_PTR ( *pfnTrayIconProcessMessage )( WPARAM wParam, LPARAM lParam );
	int  ( *pfnCListTrayNotify )( MIRANDASYSTRAYNOTIFY* );

	/* clui.c */
	LRESULT ( CALLBACK *pfnContactListWndProc )( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

	void ( *pfnCluiProtocolStatusChanged )( int status, const char* szProto );
	void ( *pfnDrawMenuItem )( LPDRAWITEMSTRUCT, HICON, HICON );
	void ( *pfnLoadCluiGlobalOpts )( void );
	BOOL ( *pfnInvalidateRect )( HWND hwnd, CONST RECT* lpRect,BOOL bErase );
	void ( *pfnOnCreateClc )( void );

	/* contact.c */
	void ( *pfnChangeContactIcon )( HANDLE hContact, int iIcon, int add );
	void ( *pfnLoadContactTree )( void );
	int  ( *pfnCompareContacts )( const struct ClcContact *contact1, const struct ClcContact *contact2 );
	void ( *pfnSortContacts )( void );
	int  ( *pfnSetHideOffline )( WPARAM wParam, LPARAM lParam );

	/* docking.c */
	int ( *pfnDocking_ProcessWindowMessage )( WPARAM wParam, LPARAM lParam );

	/* group.c */
	TCHAR* ( *pfnGetGroupName )( int idx, DWORD* pdwFlags );
   int    ( *pfnRenameGroup )( int groupID, TCHAR* newName );

	/* keyboard.c */
	int   ( *pfnHotKeysRegister )( HWND hwnd );
	void  ( *pfnHotKeysUnregister )( HWND hwnd );
	int   ( *pfnHotKeysProcess )( HWND hwnd, WPARAM wParam, LPARAM lParam );
	int   ( *pfnHotkeysProcessMessage )( WPARAM wParam, LPARAM lParam );

	/*************************************************************************************
	 * version 2 - events processing
	 *************************************************************************************/

	EventList events;

	struct CListEvent* ( *pfnCreateEvent )( void );
	void  ( *pfnFreeEvent )( struct CListEvent* );

	struct CListEvent* ( *pfnAddEvent )( CLISTEVENT* );
	CLISTEVENT* ( *pfnGetEvent )( HANDLE hContact, int idx );

	int   ( *pfnRemoveEvent )( HANDLE hContact, HANDLE hDbEvent );
	int   ( *pfnGetImlIconIndex )( HICON hIcon );

	/*************************************************************************************
	 * version 3 additions
	 *************************************************************************************/

	int   ( *pfnGetWindowVisibleState )( HWND hWnd, int iStepX, int iStepY );

	/*************************************************************************************
	 * version 4 additions (0.7.0.x) - genmenu
	 *************************************************************************************/

	MenuProto* menuProtos;
	int        menuProtoCount;

	HANDLE hPreBuildStatusMenuEvent;
	int    currentStatusMenuItem, currentDesiredStatusMode;
	BOOL   bDisplayLocked;

	HGENMENU ( *pfnGetProtocolMenu )( const char* );
	int      ( *pfnStub2 )( int );

	int    ( *pfnGetProtocolVisibility )( const char* );
	int    ( *pfnGetProtoIndexByPos )( PROTOCOLDESCRIPTOR** proto, int protoCnt, int Pos);
	void   ( *pfnReloadProtoMenus )( void );

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

	HICON  ( *pfnGetIconFromStatusMode )( HANDLE hContact, const char *szProto, int status );

	void   ( *pfnInitTray )( void );
	int    ( *pfnTrayIconAdd )( HWND hwnd, const char *szProto, const char *szIconProto, int status );
	int    ( *pfnTrayIconDestroy )( HWND hwnd );
	int    ( *pfnTrayIconInit )( HWND hwnd );
	TCHAR* ( *pfnTrayIconMakeTooltip )( const TCHAR *szPrefix, const char *szProto );
	void   ( *pfnTrayIconRemove )( HWND hwnd, const char *szProto );
	int    ( *pfnTrayIconSetBaseInfo )( HICON hIcon, const char *szPreferredProto );
	void   ( *pfnTrayIconTaskbarCreated )( HWND hwnd );
	int    ( *pfnTrayIconUpdate )( HICON hNewIcon, const TCHAR *szNewTip, const char *szPreferredProto, int isBase );

	void   ( *pfnUninitTray )( void );
	void   ( *pfnLockTray )( void );
	void   ( *pfnUnlockTray )( void );

	VOID   ( CALLBACK *pfnTrayCycleTimerProc )( HWND hwnd, UINT message, UINT_PTR idEvent, DWORD dwTime );

	/*************************************************************************************
	 * version 6 additions (0.8.0.x) - accounts
	 *************************************************************************************/
	int    ( *pfnGetAccountIndexByPos )( int pos );
}
	CLIST_INTERFACE;

extern CLIST_INTERFACE cli, *pcli;

#endif // M_CLISTINT_H__
