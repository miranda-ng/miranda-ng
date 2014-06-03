/*
Copyright (C) 2006-2007 Scott Ellis
Copyright (C) 2007-2011 Jan Holub

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#ifndef _OPTIONS_INC
#define _OPTIONS_INC

#define WMU_ENABLE_LIST_BUTTONS			(WM_USER + 0x030)
#define WMU_ENABLE_MODULE_ENTRY			(WM_USER + 0x031)

#define LABEL_LEN			1024
#define VALUE_LEN			8192
#define MODULE_NAME_LEN		512
#define SETTING_NAME_LEN	512

#define IDPRESETITEM		1000

#define MS_TOOLTIP_SHOWTIP	"mToolTip/ShowTip"

typedef struct {
	UINT id, uintCoreIconId, uintResIconId;
	TCHAR *swzTooltip;
} OPTBUTTON;

typedef enum { DIT_ALL = 0, DIT_CONTACTS = 1, DIT_CHATS = 2 } DisplayItemType;
typedef struct {
	TCHAR swzLabel[LABEL_LEN];
	TCHAR swzValue[VALUE_LEN];
	DisplayItemType type;
	bool bLineAbove, bValueNewline;
	bool bIsVisible;
	bool bParseTipperVarsFirst;
} DISPLAYITEM;

// display item types
static struct {
	DisplayItemType type;
	TCHAR *title;
} displayItemTypes[] = {
	{ DIT_ALL, LPGENT("Show for all contact types") },
	{ DIT_CONTACTS, LPGENT("Show only for contacts") },
	{ DIT_CHATS, LPGENT("Show only for chatrooms") }
};

typedef enum { DVT_DB = 0, DVT_PROTODB = 1 } DisplaySubstType;
typedef struct {
	TCHAR swzName[LABEL_LEN];
	DisplaySubstType type;
	char szModuleName[MODULE_NAME_LEN];
	char szSettingName[SETTING_NAME_LEN];
	int iTranslateFuncId;
} DISPLAYSUBST;

struct DSListNode {
	DISPLAYSUBST ds;
	DSListNode *next;
};

struct DIListNode {
	DISPLAYITEM di;
	DIListNode *next;
};

typedef struct {
	BYTE top;
	BYTE right;
	BYTE bottom;
	BYTE left;
} MARGINS;

// tray tooltip items
static TCHAR *trayTipItems[TRAYTIP_ITEMS_COUNT] = {
	LPGENT("Number of contacts"),
	LPGENT("Protocol lock status"),
	LPGENT("Logon time"),
	LPGENT("Unread emails"),
	LPGENT("Status"),
	LPGENT("Status message"),
	LPGENT("Extra status"),
	LPGENT("Listening to"),
	LPGENT("Favorite contacts"),
	LPGENT("Miranda uptime"),
	LPGENT("Contact list event")
};

// extra icons
static TCHAR *extraIconName[6] = {
	LPGENT("Status"),
	LPGENT("Extra status"),
	LPGENT("Jabber activity"),
	LPGENT("Gender"),
	LPGENT("Country flag"),
	LPGENT("Client")
};

typedef struct {
	bool bDragging;
	HTREEITEM hDragItem;
} EXTRAICONDATA;

typedef struct {
	BYTE order;
	BYTE vis;
} ICONSTATE;

typedef enum { PAV_NONE = 0, PAV_LEFT = 1, PAV_RIGHT = 2 } PopupAvLayout;
typedef enum { PTL_NOICON = 0, PTL_LEFTICON = 1, PTL_RIGHTICON = 2 } PopupIconTitleLayout;
typedef enum { PP_BOTTOMRIGHT = 0, PP_BOTTOMLEFT = 1, PP_TOPRIGHT = 2, PP_TOPLEFT = 3 } PopupPosition;
typedef enum { PSE_NONE = 0, PSE_ANIMATE = 1, PSE_FADE = 2 } PopupShowEffect;

typedef struct {
	int iWinWidth, iWinMaxHeight, iAvatarSize; //tweety
	PopupIconTitleLayout titleIconLayout;
	bool bShowTitle;
	PopupAvLayout avatarLayout;
	int iTextIndent, iTitleIndent, iValueIndent;
	bool bShowNoFocus;
	DSListNode *dsList;
	int iDsCount;
	DIListNode *diList;
	int iDiCount;
	int iTimeIn;
	int iPadding, iOuterAvatarPadding, iInnerAvatarPadding, iTextPadding;
	PopupPosition pos;
	int iMinWidth, iMinHeight; // no UI for these
	int iMouseTollerance;
	bool bStatusBarTips;
	int iSidebarWidth;
	COLORREF colBg, colBorder, colAvatarBorder, colDivider, colBar, colTitle, colLabel, colValue, colTrayTitle, colSidebar;
	int iLabelValign, iLabelHalign, iValueValign, iValueHalign;
	bool bWaitForStatusMsg, bWaitForAvatar;
	
	// tooltip skin
	SkinMode skinMode;
	TCHAR szSkinName[256];
	TCHAR szPreviewFile[1024];
	TCHAR *szImgFile[SKIN_ITEMS_COUNT];
	MARGINS margins[SKIN_ITEMS_COUNT];
	TransformationMode transfMode[SKIN_ITEMS_COUNT];
	PopupShowEffect showEffect;
	bool bLoadFonts;
	bool bLoadProportions;
	int iEnableColoring;
	int iAnimateSpeed;
	int iOpacity;
	int iAvatarOpacity;
	bool bBorder;
	bool bRound, bAvatarRound;
	bool bDropShadow;
	bool bAeroGlass;

	// tray tooltip
	bool bTraytip;
	bool bHandleByTipper;
	bool bExpandTraytip;
	bool bHideOffline;
	int iExpandTime;
	int iFirstItems, iSecondItems;
	int iFavoriteContFlags;

	// extra setting
	bool bOriginalAvatarSize;
	bool bAvatarBorder;
	bool bWaitForContent;
	bool bGetNewStatusMsg;
	bool bDisableIfInvisible;
	bool bRetrieveXstatus;
	bool bLimitMsg;
	int iLimitCharCount;
	int iSmileyAddFlags;
	BYTE exIconsOrder[EXICONS_COUNT];
	BYTE exIconsVis[EXICONS_COUNT];
} OPTIONS;


extern OPTIONS opt;

void InitOptions();
void LoadOptions();
void SaveOptions();
void DeinitOptions();

#endif
