/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
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

#ifndef M_CLIST_H__
#include <m_clist.h>
#endif 

#include <m_system.h>
#include <m_protocols.h>
#include <m_clc.h>
#include <m_gui.h>

#define HCONTACT_ISGROUP    0x80000000
#define HCONTACT_ISINFO     0xFFFF0000
#define IsHContactGroup(h)  (((UINT_PTR)(h)^HCONTACT_ISGROUP)<(HCONTACT_ISGROUP^HCONTACT_ISINFO))
#define IsHContactInfo(h)   (((UINT_PTR)(h)&HCONTACT_ISINFO) == HCONTACT_ISINFO)
#define IsHContactContact(h) (((UINT_PTR)(h)&HCONTACT_ISGROUP) == 0)

#ifndef EXTRA_ICON_COUNT
#define EXTRA_ICON_COUNT 10
#endif

#define MAX_TIP_SIZE 2048

#define INTM_NAMECHANGED         (WM_USER+10)
#define INTM_ICONCHANGED         (WM_USER+11)
#define INTM_GROUPCHANGED        (WM_USER+12)
#define INTM_GROUPSCHANGED       (WM_USER+13)
#define INTM_CONTACTADDED        (WM_USER+14)
#define INTM_CONTACTDELETED      (WM_USER+15)
#define INTM_HIDDENCHANGED       (WM_USER+16)
#define INTM_INVALIDATE          (WM_USER+17)
#define INTM_APPARENTMODECHANGED (WM_USER+18)
#define INTM_SETINFOTIPHOVERTIME (WM_USER+19)
#define INTM_NOTONLISTCHANGED    (WM_USER+20)
#define INTM_RELOADOPTIONS       (WM_USER+21)
#define INTM_NAMEORDERCHANGED    (WM_USER+22)
#define INTM_IDLECHANGED         (WM_USER+23)
#define INTM_SCROLLBARCHANGED    (WM_USER+24)
#define INTM_PROTOCHANGED        (WM_USER+25)

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
struct ClcCacheEntry;

struct ClcGroup : public MZeroedObject
{
	__forceinline ClcGroup(int _limit) :
		cl(_limit)
	{}

	LIST<ClcContact> cl;
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
	__forceinline uint8_t getType() const { return (this == nullptr) ? 0 : type; }

	uint8_t type;
	uint8_t flags;
	union {
		struct {
			int iImage;
			MCONTACT hContact;
		};
		struct {
			uint16_t groupId;
			ClcGroup *group;
		};
	};
	uint16_t iExtraImage[EXTRA_ICON_COUNT];
	wchar_t szText[120-EXTRA_ICON_COUNT];
	ClcCacheEntry *pce; // cache is persistent, contacts aren't
};

struct ClcDataBase : public MZeroedObject
{
	__forceinline ClcDataBase() :
		list(50)
	{}

	ClcGroup list;
	int rowHeight;
	int yScroll;
	int selection;
	ClcFontInfo fontInfo[FONTID_MAX + 1];
	int scrollTime;
	HIMAGELIST himlHighlight;
	int groupIndent;
	wchar_t szQuickSearch[128];
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
	uint32_t greyoutFlags;			  //see m_clc.h
	uint32_t offlineModes;
	uint32_t exStyle;
	POINT ptInfoTip;
	int infoTipTimeout;
	uint32_t hInfoTipItem;
	HIMAGELIST himlExtraColumns;
	int extraColumnsCount;
	int extraColumnSpacing;
	int checkboxSize;
	bool bShowSelAlways, bShowIdle, bNoVScrollbar, bUseWindowsColours;
	bool bNeedsResort, bFilterSearch, bLockScrollbar;
};

struct CListEvent : public CLISTEVENT, public MZeroedObject
{
	~CListEvent()
	{
		mir_free((char*)pszService);
		mir_free((char*)szTooltip.a);
	}

	int imlIconIndex;
	int flashesDone;

	int menuId;
	int imlIconOverlayIndex;

	long iOrder;
};

struct trayIconInfo_t
{
	int    id;
	char*  szProto;
	HICON  hBaseIcon;
	int    isBase;
	wchar_t* ptszToolTip;
};

struct MenuProto
{
	ptrA     szProto; 
	HGENMENU pMenu;
	HICON    hIcon;
};

/////////////////////////////////////////////////////////////////////////////////////////
// constants

#define DRAGSTAGE_NOTMOVED       0
#define DRAGSTAGE_ACTIVE         1
#define DRAGSTAGEM_STAGE         0x00FF
#define DRAGSTAGEF_MAYBERENAME   0x8000
#define DRAGSTAGEF_OUTSIDE       0x4000

#define CONTACTF_ONLINE          1
#define CONTACTF_INVISTO         2
#define CONTACTF_VISTO           4
#define CONTACTF_NOTONLIST       8
#define CONTACTF_CHECKED         16
#define CONTACTF_IDLE            32

#define DROPTARGET_OUTSIDE       0
#define DROPTARGET_ONSELF        1
#define DROPTARGET_ONNOTHING     2
#define DROPTARGET_ONGROUP       3
#define DROPTARGET_ONCONTACT     4
#define DROPTARGET_INSERTION     5

#define CLCDEFAULT_ROWHEIGHT     16
#define CLCDEFAULT_EXSTYLE       (CLS_EX_EDITLABELS|CLS_EX_TRACKSELECT|CLS_EX_SHOWGROUPCOUNTS|CLS_EX_HIDECOUNTSWHENEMPTY|CLS_EX_TRACKSELECT|CLS_EX_NOTRANSLUCENTSEL)
#define CLCDEFAULT_SCROLLTIME    150
#define CLCDEFAULT_GROUPINDENT   5
#define CLCDEFAULT_BKCOLOUR      GetSysColor(COLOR_3DFACE)
#define CLCDEFAULT_USEBITMAP     0
#define CLCDEFAULT_BKBMPUSE      CLB_STRETCH
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

/////////////////////////////////////////////////////////////////////////////////////////
// functions

EXTERN_C MIR_APP_DLL(void)      Clist_Broadcast(int msg, WPARAM wParam, LPARAM lParam);
EXTERN_C MIR_APP_DLL(void)      Clist_BroadcastAsync(int msg, WPARAM wParam, LPARAM lParam);
EXTERN_C MIR_APP_DLL(void)      Clist_CalcEipPosition(ClcData *dat, ClcContact *contact, ClcGroup *group, POINT *result);
EXTERN_C MIR_APP_DLL(void)      Clist_ChangeContactIcon(MCONTACT hContact, int iIcon);
EXTERN_C MIR_APP_DLL(void)      Clist_ClcOptionsChanged(void);
EXTERN_C MIR_APP_DLL(int)       Clist_ClcStatusToPf2(int status);
EXTERN_C MIR_APP_DLL(uint32_t)  Clist_ContactToHItem(ClcContact *contact);
EXTERN_C MIR_APP_DLL(HANDLE)    Clist_ContactToItemHandle(ClcContact *contact, uint32_t *nmFlags);
EXTERN_C MIR_APP_DLL(void)      Clist_DeleteFromContactList(HWND hwnd, ClcData *dat);
EXTERN_C MIR_APP_DLL(void)      Clist_DeleteItemFromTree(HWND hwnd, MCONTACT hItem);
EXTERN_C MIR_APP_DLL(void)      Clist_DoSelectionDefaultAction(HWND hwnd, ClcData *dat);
EXTERN_C MIR_APP_DLL(void)      Clist_DrawMenuItem(DRAWITEMSTRUCT *dis, HICON hIcon, HICON eventIcon);
EXTERN_C MIR_APP_DLL(void)      Clist_EndRename(ClcData *dat, int save);
EXTERN_C MIR_APP_DLL(void)      Clist_EnsureVisible(HWND hwnd, ClcData *dat, int iItem, int partialOk);
EXTERN_C MIR_APP_DLL(int)       Clist_EventsProcessTrayDoubleClick(int index);
EXTERN_C MIR_APP_DLL(bool)      Clist_FindItem(HWND hwnd, ClcData *dat, uint32_t dwItem, ClcContact **contact, ClcGroup **subgroup = 0, int *isVisible = 0);
EXTERN_C MIR_APP_DLL(uint32_t)  Clist_GetDefaultExStyle(void);
EXTERN_C MIR_APP_DLL(void)      Clist_GetFontSetting(int i, LOGFONT *lf, COLORREF *colour);
EXTERN_C MIR_APP_DLL(int)       Clist_GetGeneralizedStatus(char **szProto = nullptr);
EXTERN_C MIR_APP_DLL(wchar_t*)  Clist_GetGroupCountsText(ClcData *dat, ClcContact *contact);
EXTERN_C MIR_APP_DLL(int)       Clist_GetRealStatus(ClcContact *cc, int iDefaultValue);
EXTERN_C MIR_APP_DLL(void)      Clist_HideInfoTip(ClcData *dat);
EXTERN_C MIR_APP_DLL(void)      Clist_InitAutoRebuild(HWND hWnd);
EXTERN_C MIR_APP_DLL(void)      Clist_InvalidateItem(HWND hwnd, ClcData *dat, int iItem);
EXTERN_C MIR_APP_DLL(int)       Clist_IsHiddenMode(ClcData *dat, int status);
EXTERN_C MIR_APP_DLL(void)      Clist_LoadContactTree(void);
EXTERN_C MIR_APP_DLL(void)      Clist_NotifyNewContact(HWND hwnd, MCONTACT hContact);
EXTERN_C MIR_APP_DLL(void)      Clist_RecalculateGroupCheckboxes(ClcData *dat);
EXTERN_C MIR_APP_DLL(ClcGroup*) Clist_RemoveItemFromGroup(HWND hwnd, ClcGroup *group, ClcContact *contact, int updateTotalCount);
EXTERN_C MIR_APP_DLL(void)      Clist_SaveStateAndRebuildList(HWND hwnd, ClcData *dat);
EXTERN_C MIR_APP_DLL(void)      Clist_SetGroupChildCheckboxes(ClcGroup *group, int checked);
								        
EXTERN_C MIR_APP_DLL(int)       Clist_TrayIconAdd(HWND hwnd, const char *szProto, const char *szIconProto, int status);
EXTERN_C MIR_APP_DLL(int)       Clist_TrayIconDestroy(HWND hwnd);
EXTERN_C MIR_APP_DLL(void)      Clist_TrayIconIconsChanged(void);
EXTERN_C MIR_APP_DLL(wchar_t*)  Clist_TrayIconMakeTooltip(const wchar_t *szPrefix, const char *szProto);
EXTERN_C MIR_APP_DLL(void)      Clist_TrayIconRemove(HWND hwnd, const char *szProto);
EXTERN_C MIR_APP_DLL(int)       Clist_TrayIconSetBaseInfo(HICON hIcon, const char *szPreferredProto);
EXTERN_C MIR_APP_DLL(void)      Clist_TrayIconUpdateBase(const char *szChangedProto);
EXTERN_C MIR_APP_DLL(void)      Clist_TraySetTimer();

EXTERN_C MIR_APP_DLL(ClcCacheEntry*) Clist_GetCacheEntry(MCONTACT hContact);

// calculates account's index by its position in status bar
EXTERN_C MIR_APP_DLL(int) Clist_GetAccountIndex(int iPos);

/////////////////////////////////////////////////////////////////////////////////////////
// CLIST_INTERFACE structure definition

struct ClcCacheEntryBase
{
	MCONTACT hContact;
	int      bIsHidden;
	wchar_t* tszName;
	wchar_t* tszGroup;
	char*    szProto;
};

struct CLIST_INTERFACE
{
	HWND hwndContactList, hwndContactTree, hwndStatus;
	HMENU hMenuMain;
	HINSTANCE hInst;

	// clc.h
	LRESULT        (CALLBACK *pfnContactListControlWndProc)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// clcidents.c
	ClcContact*    (*pfnFindItem)(uint32_t dwItem, ClcContact *contact);
				      
	int            (*pfnGetRowsPriorTo)(ClcGroup *group, ClcGroup *subgroup, int contactIndex);
	int            (*pfnGetRowByIndex)(ClcData *dat, int testindex, ClcContact **contact, ClcGroup **subgroup);
	int            (*pfnGetContactHiddenStatus)(MCONTACT hContact, char *szProto, ClcData *dat);
	int            (*pfnIsVisibleContact)(ClcCacheEntry *pce, ClcGroup *group);

	/* clcitems.c */
	ClcContact*    (*pfnCreateClcContact)(void);
	ClcGroup*      (*pfnAddGroup)(HWND hwnd, ClcData *dat, const wchar_t *szName, uint32_t flags, int groupId, int calcTotalMembers);
	
	void           (*pfnFreeContact)(ClcContact *contact);
				      
	ClcContact*    (*pfnAddInfoItemToGroup)(ClcGroup *group, int flags, const wchar_t *pszText);
	ClcContact*    (*pfnAddItemToGroup)(ClcGroup *group, int iAboveItem);
	ClcContact*    (*pfnAddContactToGroup)(ClcData *dat, ClcGroup *group, MCONTACT hContact);
				      
	void           (*pfnAddContactToTree)(HWND hwnd, ClcData *dat, MCONTACT hContact, int updateTotalCount, int checkHideOffline);
	void           (*pfnRebuildEntireList)(HWND hwnd, ClcData *dat);
	int            (*pfnGetGroupContentsCount)(ClcGroup *group, int visibleOnly);
	void           (*pfnSortCLC)(HWND hwnd, ClcData *dat, int useInsertionSort);

	/* clcmsgs.c */
	LRESULT        (*pfnProcessExternalMessages)(HWND hwnd, ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam);

	/* clcpaint.c */
	void           (*pfnPaintClc)(HWND hwnd, ClcData *dat, HDC hdc, RECT * rcPaint);

	/* clcutils.c */
	int            (*pfnHitTest)(HWND hwnd, ClcData *dat, int testx, int testy, ClcContact **contact, ClcGroup **group, uint32_t * flags);
	void           (*pfnScrollTo)(HWND hwnd, ClcData *dat, int desty, int noSmooth);
	void           (*pfnRecalcScrollBar)(HWND hwnd, ClcData *dat);
	void           (*pfnSetGroupExpand)(HWND hwnd, ClcData *dat, ClcGroup *group, int newState);
	int            (*pfnFindRowByText)(HWND hwnd, ClcData *dat, const wchar_t *text, int prefixOk);
	void           (*pfnBeginRenameSelection)(HWND hwnd, ClcData *dat);
	void           (*pfnGetDefaultFontSetting)(int i, LOGFONT *lf, COLORREF *colour);
	void           (*pfnLoadClcOptions)(HWND hwnd, ClcData *dat, BOOL bFirst);
				      
	int            (*pfnGetRowBottomY)(ClcData *dat, int item);
	int            (*pfnGetRowHeight)(ClcData *dat, int item);
	int            (*pfnGetRowTopY)(ClcData *dat, int item);
	int            (*pfnGetRowTotalHeight)(ClcData *dat);
	int            (*pfnRowHitTest)(ClcData *dat, int y);
		  
	/* clistmod.c */
	int            (*pfnIconFromStatusMode)(const char *szProto, int status, MCONTACT hContact);
	int            (*pfnShowHide)(void);
	
	/* clistsettings.c */
	ClcCacheEntry* (*pfnCreateCacheItem)(MCONTACT hContact);
	void           (*pfnCheckCacheItem)(ClcCacheEntry*);
	void           (*pfnFreeCacheItem)(ClcCacheEntry*);
				     
	void           (*pfnInvalidateDisplayNameCacheEntry)(MCONTACT hContact);

	/* clisttray.c */
	int            (*pfnTrayIconPauseAutoHide)(WPARAM wParam, LPARAM lParam);
	INT_PTR        (*pfnTrayIconProcessMessage)(WPARAM wParam, LPARAM lParam);

	/* clui.c */
	LRESULT        (CALLBACK *pfnContactListWndProc)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
				      
	void           (*pfnCluiProtocolStatusChanged)(int status, const char *szProto);
	void           (*pfnLoadCluiGlobalOpts)(void);
	BOOL           (*pfnInvalidateRect)(HWND hwnd, const RECT* lpRect, BOOL bErase);
	void           (*pfnOnCreateClc)(void);

	/* contact.c */
	int            (*pfnCompareContacts)(const ClcContact *contact1, const ClcContact *contact2);
	int            (*pfnSetHideOffline)(int newValue); // TRUE, FALSE or -1 to revert the current setting

	/* docking.c */
	int            (*pfnDocking_ProcessWindowMessage)(WPARAM wParam, LPARAM lParam);

	/*************************************************************************************
	 * version 2 - events processing
	 *************************************************************************************/

	OBJLIST<CListEvent> *events;

	CListEvent* (*pfnAddEvent)(CLISTEVENT*);
	CLISTEVENT* (*pfnGetEvent)(MCONTACT hContact, int idx);

	int      (*pfnRemoveEvent)(MCONTACT hContact, MEVENT hDbEvent);
	int      (*pfnGetImlIconIndex)(HICON hIcon);

	/*************************************************************************************
	 * version 3 additions
	 *************************************************************************************/

	int      (*pfnGetWindowVisibleState)(HWND hWnd, int iStepX, int iStepY);

	/*************************************************************************************
	 * version 4 additions - genmenu
	 *************************************************************************************/

	OBJLIST<MenuProto> *menuProtos;

	int      currentDesiredStatusMode;
	bool     bAutoRebuild, bOwnerDrawMenu;

	/*************************************************************************************
	 * version 5 additions - tray icons
	 *************************************************************************************/

	trayIconInfo_t* trayIcon;
	int      trayIconCount;
	int      cycleStep;
	wchar_t* szTip;
	BOOL     bTrayMenuOnScreen;

	HICON    (*pfnGetIconFromStatusMode)(MCONTACT hContact, const char *szProto, int status);

	int      (*pfnTrayCalcChanged)(const char *szChangedProto, int averageMode, int iProtoCount);
	int      (*pfnTrayIconInit)(HWND hwnd);

	/*************************************************************************************
	 * Miranda NG additions
	 *************************************************************************************/
	void     (*pfnSetContactCheckboxes)(ClcContact *cc, int checked);
};

// retrieves the pointer to a CLIST_INTERFACE structure
// NOTE: valid only for the clist clone building, not for the regular use

EXTERN_C MIR_APP_DLL(CLIST_INTERFACE*) Clist_GetInterface(void);

extern MIR_APP_EXPORT CLIST_INTERFACE g_clistApi;

namespace Clist
{
	extern MIR_APP_EXPORT CMOption<bool>
		HideOffline,
		UseGroups,
		FilterSearch,
		HideEmptyGroups,
		ConfirmDelete,
		Tray1Click,
		TrayAlwaysStatus;

	extern MIR_APP_EXPORT CMOption<uint32_t>
		OfflineModes;
};

#endif // M_CLISTINT_H__
