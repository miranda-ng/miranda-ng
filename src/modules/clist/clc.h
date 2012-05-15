/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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

struct ClcContact {
	BYTE type;
	BYTE flags;
	union {
		struct {
			WORD iImage;
			HANDLE hContact;
		};
		struct {
			WORD groupId;
			struct ClcGroup *group;
		};
	};
	BYTE  iExtraImage[MAXEXTRACOLUMNS];
	TCHAR szText[120-MAXEXTRACOLUMNS];
	char * proto;	// MS_PROTO_GETBASEPROTO
};

struct ClcData {
	struct ClcGroup list;
	int rowHeight;
	int yScroll;
	int selection;
	struct ClcFontInfo fontInfo[FONTID_MAX + 1];
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
};

/* clc.c */
extern int g_IconWidth, g_IconHeight;

void   fnClcOptionsChanged( void );
void   fnClcBroadcast( int msg, WPARAM wParam, LPARAM lParam );
HMENU  fnBuildGroupPopupMenu( struct ClcGroup* group );

LRESULT CALLBACK fnContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/* clcidents.c */
int    fnGetRowsPriorTo( struct ClcGroup *group, struct ClcGroup *subgroup, int contactIndex );
int    fnFindItem( HWND hwnd, struct ClcData *dat, HANDLE hItem, struct ClcContact **contact, struct ClcGroup **subgroup, int *isVisible );
int    fnGetRowByIndex( struct ClcData *dat, int testindex, struct ClcContact **contact, struct ClcGroup **subgroup );
HANDLE fnContactToHItem( struct ClcContact* contact );
HANDLE fnContactToItemHandle( struct ClcContact * contact, DWORD * nmFlags );

/* clcitems.c */
struct ClcGroup* fnAddGroup( HWND hwnd, struct ClcData *dat, const TCHAR *szName, DWORD flags, int groupId, int calcTotalMembers );
struct ClcGroup* fnRemoveItemFromGroup(HWND hwnd, struct ClcGroup *group, struct ClcContact *contact, int updateTotalCount);

void fnFreeContact( struct ClcContact *p );
void fnFreeGroup( struct ClcGroup *group );
int  fnAddInfoItemToGroup(struct ClcGroup *group, int flags, const TCHAR *pszText);
int  fnAddItemToGroup( struct ClcGroup *group,int iAboveItem );
void fnAddContactToTree( HWND hwnd, struct ClcData *dat, HANDLE hContact, int updateTotalCount, int checkHideOffline);
int  fnAddContactToGroup( struct ClcData *dat, struct ClcGroup *group, HANDLE hContact);
void fnDeleteItemFromTree( HWND hwnd, HANDLE hItem );
void fnRebuildEntireList( HWND hwnd, struct ClcData *dat );
int  fnGetGroupContentsCount( struct ClcGroup *group, int visibleOnly );
void fnSortCLC( HWND hwnd, struct ClcData *dat, int useInsertionSort );
void fnSaveStateAndRebuildList(HWND hwnd, struct ClcData *dat);

/* clcmsgs.c */
LRESULT fnProcessExternalMessages(HWND hwnd, struct ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam );

/* clcutils.c */
char* fnGetGroupCountsText(struct ClcData *dat, struct ClcContact *contact );
int   fnHitTest( HWND hwnd, struct ClcData *dat, int testx, int testy, struct ClcContact **contact, struct ClcGroup **group, DWORD * flags );
void  fnScrollTo( HWND hwnd, struct ClcData *dat, int desty, int noSmooth );
void  fnEnsureVisible(HWND hwnd, struct ClcData *dat, int iItem, int partialOk );
void  fnRecalcScrollBar( HWND hwnd, struct ClcData *dat );
void  fnSetGroupExpand( HWND hwnd, struct ClcData *dat, struct ClcGroup *group, int newState );
void  fnDoSelectionDefaultAction( HWND hwnd, struct ClcData *dat );
int   fnFindRowByText(HWND hwnd, struct ClcData *dat, const TCHAR *text, int prefixOk );
void  fnEndRename(HWND hwnd, struct ClcData *dat, int save );
void  fnDeleteFromContactList( HWND hwnd, struct ClcData *dat );
void  fnBeginRenameSelection( HWND hwnd, struct ClcData *dat );
void  fnCalcEipPosition( struct ClcData *dat, struct ClcContact *contact, struct ClcGroup *group, POINT *result);
int   fnGetDropTargetInformation( HWND hwnd, struct ClcData *dat, POINT pt );
int   fnClcStatusToPf2( int status );
int   fnIsHiddenMode( struct ClcData *dat, int status );
void  fnHideInfoTip( HWND hwnd, struct ClcData *dat );
void  fnNotifyNewContact( HWND hwnd, HANDLE hContact );
DWORD fnGetDefaultExStyle( void );
void  fnGetSetting( int i, LOGFONT* lf, COLORREF* colour );
void  fnGetDefaultFontSetting(int i, LOGFONT* lf, COLORREF* colour);
void  fnGetFontSetting( int i, LOGFONT* lf, COLORREF* colour );
void  fnLoadClcOptions( HWND hwnd, struct ClcData *dat );
void  fnRecalculateGroupCheckboxes( HWND hwnd, struct ClcData *dat );
void  fnSetGroupChildCheckboxes( struct ClcGroup *group, int checked );
void  fnInvalidateItem( HWND hwnd, struct ClcData *dat, int iItem );

int fnGetRowBottomY(struct ClcData *dat, int item);
int fnGetRowHeight(struct ClcData *dat, int item);
int fnGetRowTopY(struct ClcData *dat, int item);
int fnGetRowTotalHeight(struct ClcData *dat);
int fnRowHitTest(struct ClcData *dat, int y);

/* clcopts.c */
int ClcOptInit(WPARAM wParam,LPARAM lParam);
DWORD GetDefaultExStyle(void);
void GetFontSetting(int i,LOGFONTA *lf,COLORREF *colour);

/* clistmenus.c */
HGENMENU fnGetProtocolMenu( const char* );
int      fnGetProtocolVisibility( const char* accName );

int      fnGetAccountIndexByPos(int Pos);
int      fnGetProtoIndexByPos(PROTOCOLDESCRIPTOR ** proto, int protoCnt, int Pos);
void     RebuildMenuOrder( void );

INT_PTR  MenuProcessCommand(WPARAM wParam, LPARAM lParam);

/* clistsettings.c */
TCHAR* fnGetContactDisplayName( HANDLE hContact, int mode );
void   fnGetDefaultFontSetting( int i, LOGFONT* lf, COLORREF * colour);
void   fnInvalidateDisplayNameCacheEntry( HANDLE hContact );

ClcCacheEntryBase* fnGetCacheEntry( HANDLE hContact );
ClcCacheEntryBase* fnCreateCacheItem ( HANDLE hContact );
void fnCheckCacheItem( ClcCacheEntryBase* p );
void fnFreeCacheItem( ClcCacheEntryBase* p );

/* clcfiledrop.c */
void InitFileDropping(void);

void   fnRegisterFileDropping ( HWND hwnd );
void   fnUnregisterFileDropping ( HWND hwnd );

/* clistevents.c */
struct CListEvent* fnAddEvent( CLISTEVENT *cle );
CLISTEVENT* fnGetEvent( HANDLE hContact, int idx );

struct CListEvent* fnCreateEvent( void );
void fnFreeEvent( struct CListEvent* p );

int   fnEventsProcessContactDoubleClick( HANDLE hContact );
int   fnEventsProcessTrayDoubleClick( int );
int   fnGetImlIconIndex(HICON hIcon);
int   fnRemoveEvent( HANDLE hContact, HANDLE dbEvent );

/* clistmod.c */
int    fnIconFromStatusMode(const char *szProto, int status, HANDLE hContact);
int    fnShowHide( WPARAM wParam, LPARAM lParam );
HICON  fnGetIconFromStatusMode( HANDLE hContact, const char *szProto, int status );
TCHAR* fnGetStatusModeDescription( int wParam, int lParam);
int    fnGetWindowVisibleState(HWND hWnd, int iStepX, int iStepY);

/* clisttray.c */
void   fnInitTray( void );
void   fnUninitTray( void );
void   fnLockTray( void );
void   fnUnlockTray( void );
int    fnCListTrayNotify(MIRANDASYSTRAYNOTIFY *msn);
int    fnTrayIconAdd(HWND hwnd, const char *szProto, const char *szIconProto, int status);
int    fnTrayIconDestroy( HWND hwnd );
void   fnTrayIconIconsChanged ( void );
int    fnTrayIconInit( HWND hwnd );
TCHAR* fnTrayIconMakeTooltip( const TCHAR *szPrefix, const char *szProto );
int    fnTrayIconPauseAutoHide ( WPARAM wParam, LPARAM lParam );
INT_PTR    fnTrayIconProcessMessage ( WPARAM wParam, LPARAM lParam );
void   fnTrayIconRemove(HWND hwnd, const char *szProto);
int    fnTrayIconSetBaseInfo(HICON hIcon, const char *szPreferredProto);
void   fnTrayIconSetToBase ( char *szPreferredProto );
void   fnTrayIconTaskbarCreated( HWND hwnd );
int    fnTrayIconUpdate( HICON hNewIcon, const TCHAR *szNewTip, const char *szPreferredProto, int isBase );
void   fnTrayIconUpdateBase ( const char *szChangedProto );
void   fnTrayIconUpdateWithImageList ( int iImage, const TCHAR *szNewTip, char *szPreferredProto );

VOID CALLBACK fnTrayCycleTimerProc(HWND hwnd, UINT message, UINT_PTR idEvent, DWORD dwTime);

/* clui.c */
LRESULT CALLBACK fnContactListWndProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
void fnLoadCluiGlobalOpts( void );
void fnCluiProtocolStatusChanged(int,const char*);
void fnDrawMenuItem(DRAWITEMSTRUCT *dis, HICON hIcon, HICON eventIcon);

/* contact.c */
void fnChangeContactIcon ( HANDLE hContact, int iIcon, int add );
void fnLoadContactTree ( void );
int  fnCompareContacts ( const struct ClcContact *contact1, const struct ClcContact *contact2);
void fnSortContacts ( void );
int  fnSetHideOffline ( WPARAM wParam, LPARAM lParam );

/* docking.c */
int fnDocking_ProcessWindowMessage ( WPARAM wParam, LPARAM lParam );

/* group.c */
TCHAR* fnGetGroupName ( int idx, DWORD* pdwFlags );
int    fnRenameGroup ( int groupID, TCHAR* newName );

/* keyboard.c */
int  fnHotKeysRegister ( HWND hwnd );
void fnHotKeysUnregister ( HWND hwnd );
int  fnHotKeysProcess ( HWND hwnd, WPARAM wParam, LPARAM lParam );
int  fnHotkeysProcessMessage ( WPARAM wParam, LPARAM lParam );
