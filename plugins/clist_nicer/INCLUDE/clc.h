/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2010 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of clist_nicer plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: clc.h 13850 2011-09-10 03:26:59Z borkra $
 *
 */

#define MAXEXTRACOLUMNS     16

#define INTM_XSTATUSCHANGED  (WM_USER+26)
#define INTM_METACHANGEDEVENT (WM_USER+27)
#define INTM_CODEPAGECHANGED (WM_USER+28)
//#define INTM_CLIENTCHANGED   (WM_USER+29)
#define INTM_AVATARCHANGED   (WM_USER+30)
#define INTM_STATUSMSGCHANGED   (WM_USER+31)
#define INTM_SORTCLC            (WM_USER+32)
#define INTM_STATUSCHANGED      (WM_USER+33)
#define INTM_METACHANGED        (WM_USER+34)
#define INTM_INVALIDATECONTACT  (WM_USER+35)
#define INTM_FORCESORT			(WM_USER+36)

#define DEFAULT_TITLEBAR_HEIGHT		18

#define CLS_SKINNEDFRAME	0x0800   //this control will be the main contact list (v. 0.3.4.3+ 2004/11/02)

#define TIMERID_RENAME         10
#define TIMERID_DRAGAUTOSCROLL 11
#define TIMERID_INFOTIP        13
#define TIMERID_SORT           15
#define TIMERID_REFRESH        18
#define TIMERID_PAINT          19
struct ClcGroup;

#define CONTACTF_ONLINE    1
#define CONTACTF_INVISTO   2
#define CONTACTF_VISTO     4
#define CONTACTF_NOTONLIST 8
#define CONTACTF_CHECKED   16
#define CONTACTF_IDLE      32
#define CONTACTF_STICKY    64
#define CONTACTF_PRIORITY  128

#define STATUSMSG_XSTATUSID 1
#define STATUSMSG_XSTATUSNAME 2
#define STATUSMSG_CLIST 4
#define STATUSMSG_YIM 8
#define STATUSMSG_GG 16
#define STATUSMSG_XSTATUS 32

#define STATUSMSG_NOTFOUND 0

#define SMSG_MAXLEN 700

#define EXTRAIMAGECACHESIZE 1000

// extra cache contact flags

#define ECF_RTLNICK 1
#define ECF_RTLSTATUSMSG 2
#define ECF_FORCEAVATAR 4
#define ECF_HIDEAVATAR 8
#define ECF_FORCEOVERLAY 16
#define ECF_HIDEOVERLAY 32
#define ECF_FORCELOCALTIME 64
#define ECF_HIDELOCALTIME 128
#define ECF_FORCEVISIBILITY 256
#define ECF_HIDEVISIBILITY  512
#define ECF_HASREALTIMEZONE 1024

// other contact flags (struct ClCContact;

#define ECF_AVATAR 1
#define ECF_SECONDLINE 2

struct ContactFloater {
	struct ContactFloater *pNextFloater;
	HWND hwnd;
	HDC hdc;
	HBITMAP hbm, hbmOld;
	HANDLE hContact;
};

typedef struct ContactFloater CONTACTFLOATER;

#define DSPF_CENTERSTATUSICON 1
#define DSPF_DIMIDLE 2
#define DSPF_NOFFLINEAVATARS 4
#define DSPF_SHOWLOCALTIME 8
#define DSPF_LOCALTIMESELECTIVE 16
#define DSPF_DONTSEPARATEOFFLINE 32
#define DSPF_CENTERGROUPNAMES 64

#define EXICON_COUNT 11

/* Extra icons settings */
typedef struct _OrderTreeData
{
	BYTE	ID;
	const TCHAR *	Name;
	BYTE	Position;
	BOOL	Visible;
    BOOL    fReserved;
} *PORDERTREEDATA, ORDERTREEDATA;

struct DisplayProfile {
    DWORD   dwFlags;
    DWORD   dwExtraImageMask;
    int     exIconScale;
    BOOL    bCenterStatusIcons;
    BOOL    bDimIdle, bNoOfflineAvatars, bShowLocalTime, bShowLocalTimeSelective, bDontSeparateOffline, bCenterGroupNames;
    BYTE    dualRowMode;
    COLORREF avatarBorder;
    DWORD    avatarRadius;
    int      avatarSize;
    DWORD    clcExStyle;
    DWORD    clcOfflineModes;
    BYTE     sortOrder[3], bUseDCMirroring, bGroupAlign;
    BYTE     avatarPadding;
    BYTE     bLeftMargin, bRightMargin, bRowSpacing, bGroupIndent, bRowHeight, bGroupRowHeight;
    BYTE     exIconOrder[EXICON_COUNT];
};
typedef struct DisplayProfile DISPLAYPROFILE;

/*
 * a set of 4 (online, offline, selected, hottracked) display profiles
 */

struct DisplayProfileSet {
    UINT    uID;
    TCHAR   tszName[60];
    DISPLAYPROFILE dp[4];
};
typedef struct DisplayProfileSet DISPLAYPROFILESET;

#define DSP_PROFILES_MODULE "CLN_DspProfiles"           // db module for display profiles

struct TExtraCache {
	BYTE iExtraImage[MAXEXTRACOLUMNS];
	HANDLE hContact;
	HANDLE hTimeZone;
	DWORD iExtraValid;
	BYTE valid;
	TCHAR *statusMsg;
	BYTE bStatusMsgValid;
	DWORD dwCFlags;
    DWORD dwDFlags;     // display flags for caching only
    DWORD dwXMask;      // local extra icon mask, calculated from CLN_xmask
	StatusItems_t *status_item, *proto_status_item;
	CONTACTFLOATER *floater;
	DWORD dwLastMsgTime;
    DWORD msgFrequency;
    BOOL  isChatRoom;
};

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
	BYTE iExtraImage[MAXEXTRACOLUMNS];
	TCHAR szText[120 - MAXEXTRACOLUMNS];
	char * proto;    // MS_PROTO_GETBASEPROTO

	// inherited from standard
	BOOL bIsMeta;
	HANDLE hSubContact;
	BYTE xStatus;
	char *metaProto;
	//int clientId;
	DWORD codePage;
	struct avatarCacheEntry *ace;
	WORD wStatus;
	int extraCacheEntry;
	int avatarLeft, extraIconRightBegin;
	int isRtl;
    DWORD cFlags;
    BYTE  bSecondLine;
    //int iRowHeight;   // index into the row height table (for caching)
};

#define DRAGSTAGE_NOTMOVED  0
#define DRAGSTAGE_ACTIVE    1
#define DRAGSTAGEM_STAGE    0x00FF
#define DRAGSTAGEF_MAYBERENAME  0x8000
#define DRAGSTAGEF_OUTSIDE      0x4000

#define FONTID_CONTACTS    0
#define FONTID_INVIS       1
#define FONTID_OFFLINE     2
#define FONTID_NOTONLIST   3
#define FONTID_GROUPS      4
#define FONTID_GROUPCOUNTS 5
#define FONTID_DIVIDERS    6
#define FONTID_OFFINVIS    7
#define FONTID_STATUS      8
#define FONTID_FRAMETITLE  9
#define FONTID_EVENTAREA   10
#define FONTID_TIMESTAMP   11
#define FONTID_LAST        FONTID_TIMESTAMP

struct ClcData {
	struct ClcGroup list;
	int max_row_height;
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
	DWORD greyoutFlags;           //see m_clc.h
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
	BOOL bNeedSort;

	int *row_heights;
	int row_heights_size;
	int row_heights_allocated;

	int row_border;
	int min_row_heigh, group_row_height;

	int currentFontID;
	int rightMargin;
	BYTE SelectMode;
	BYTE isMultiSelect;
	HWND hwndParent;
	DWORD lastSort;
	BOOL bNeedPaint, bisEmbedded, bHideSubcontacts;
	DWORD lastRepaint;
	BOOL forceScroll;
    int  oldSelection;
};

#define CLUI_FRAME_SHOWTOPBUTTONS 1
#define CLUI_FRAME_SHOWBOTTOMBUTTONS 2
#define CLUI_SHOWCLIENTICONS 4
#define CLUI_SHOWVISI 8
#define CLUI_FRAME_CLISTSUNKEN 16
#define CLUI_SHOWXSTATUS 32
#define CLUI_FRAME_BUTTONSFLAT 64
#define CLUI_FRAME_BUTTONSCLASSIC 128
#define CLUI_USEMETAICONS 256
#define CLUI_FRAME_AUTOHIDENOTIFY 512
#define CLUI_FRAME_USEXSTATUSASSTATUS 1024
#define CLUI_STICKYEVENTS 2048
#define CLUI_FRAME_SBARSHOW 4096
#define CLUI_STATUSASTEXT 8192
#define CLUI_FULLROWSELECT 16384
#define CLUI_FRAME_EVENTAREASUNKEN 32768
#define CLUI_FRAME_BUTTONBARSUNKEN 65536
#define CLUI_FRAME_AVATARS         0x20000
#define CLUI_FRAME_AVATARSLEFT     0x40000
#define CLUI_FRAME_GDIPLUS         0x80000
#define CLUI_FRAME_AVATARBORDER    0x100000
#define CLUI_FRAME_STATUSICONS     0x200000
#define CLUI_FRAME_AVATARSRIGHTWITHNICK    0x400000
#define CLUI_FRAME_TRANSPARENTAVATAR 0x800000
#define CLUI_FRAME_ROUNDAVATAR      0x1000000
#define CLUI_FRAME_ALWAYSALIGNNICK  0x2000000
#define CLUI_FRAME_AVATARSRIGHT     0x4000000
#define CLUI_FRAME_SHOWSTATUSMSG    0x8000000
#define CLUI_FRAME_OVERLAYICONS    0x10000000
#define CLUI_FRAME_SELECTIVEICONS  0x20000000
#define CLUI_FRAME_ROUNDEDFRAME    0x40000000
#define CLUI_FRAME_NOGROUPICON     0x80000000

#define MULTIROW_NEVER 0
#define MULTIROW_ALWAYS 1
#define MULTIROW_IFSPACE 2
#define MULTIROW_IFNEEDED 3

#define CLUI_USE_FLOATER 1
#define CLUI_FLOATER_AUTOHIDE 2
#define CLUI_FLOATER_EVENTS 4

#define CLC_GROUPALIGN_LEFT 0
#define CLC_GROUPALIGN_RIGHT 1
#define CLC_GROUPALIGN_AUTO 2

struct TCluiData {
	DWORD dwFlags;
	DWORD topOffset, bottomOffset;
	int statusBarHeight;
	int soundsOff;
	BYTE tabSRMM_Avail;
	BYTE IcoLib_Avail;
	BYTE bMetaAvail;
    BYTE bFontServiceAvail;
	BYTE bAvatarServiceAvail;
	HICON hIconVisible, hIconInvisible, hIconChatactive, hIconConnecting;
	DWORD dwButtonHeight, dwButtonWidth;
	DWORD toolbarVisibility;
	DWORD winFlags;
	DWORD winFlagsEx;
	HMENU hMenuButtons;
	int notifyActive;
	int hIconNotify;
	HMENU hMenuNotify;
	int iLastEventAdded;
	int wNextMenuID;
	HANDLE hUpdateContact;
	DWORD sortTimer;
	TCHAR *szNoEvents;
	BOOL forceResize;
	BOOL neeedSnap;
	COLORREF avatarBorder;
	HBRUSH hBrushAvatarBorder, hBrushColorKey;
	HBRUSH hBrushCLCBk;
	DWORD avatarRadius;
	int avatarSize;
	BOOL bForceRefetchOnPaint;
	BYTE dualRowMode;
	BYTE avatarPadding;
	BYTE cornerRadius;
	BYTE isTransparent;
	BYTE alpha, autoalpha;
	BYTE fadeinout;
	BYTE autosize;
	BYTE gapBetweenFrames;
	BYTE titleBarHeight;
	DWORD dwExtraImageMask;
	BYTE bClipBorder, bRowSpacing;
	HBITMAP bmpBackground, hbmBgOld, hbmBg;
	HDC hdcBg;
	HDC hdcPic;
	HBITMAP hbmPicOld;
	BITMAP bminfoBg;
	SIZE dcSize;
	POINT ptW;
	BOOL bWallpaperMode;
	BOOL bNoOfflineAvatars;
	BOOL bEventAreaEnabled;
	BOOL bFullTransparent;
	BOOL bDblClkAvatars;
	BOOL bApplyIndentToBg;
	BOOL bEqualSections;
	DWORD bFilterEffective;
	BOOL bCenterStatusIcons;
	BOOL bSkinnedToolbar;
	BOOL bSkinnedStatusBar;
	BOOL bUsePerProto;
	BOOL bOverridePerStatusColors;
	BOOL bDontSeparateOffline;
	TCHAR groupFilter[2048];
	char protoFilter[2048];
	char varFilter[2048];
	DWORD lastMsgFilter;
	char current_viewmode[256], old_viewmode[256];
	BYTE boldHideOffline;
	DWORD statusMaskFilter;
	DWORD stickyMaskFilter;
	DWORD filterFlags;
	COLORREF colorkey;
	char szMetaName[256];
	BOOL bMetaEnabled;
	BOOL bSecIMAvail;
	BOOL bNoTrayTips;
	int exIconScale;
	BOOL bShowLocalTime;
	BOOL bShowLocalTimeSelective;
	BOOL bShowXStatusOnSbar;
	BOOL bLayeredHack;
	HPEN hPen3DBright, hPen3DDark;
	BYTE bSkinnedButtonMode;
	BYTE bFirstRun;
	BYTE bUseDCMirroring;
	BYTE bCLeft, bCRight, bCTop, bCBottom;
	BYTE bUseFloater;
	BYTE fullyInited;
	BYTE bAutoExpandGroups;
	SIZE szOldCTreeSize;
	BYTE bWantFastGradients, bUseFastGradients;
	BYTE sortOrder[3];
	BYTE bGroupAlign;
    BYTE bSkinnedScrollbar;
    DWORD langPackCP;
    BOOL fOnDesktop;
    int  group_padding;
    DWORD t_now;
    BYTE exIconOrder[EXICON_COUNT];
    BOOL realTimeSaving;
	TCHAR tszProfilePath[MAX_PATH];
	FILETIME ft;
	SYSTEMTIME st;
};

#define SORTBY_NAME 1
#define SORTBY_PROTO 2
#define SORTBY_STATUS 3
#define SORTBY_LASTMSG 4
#define SORTBY_FREQUENCY 5
#define SORTBY_PRIOCONTACTS 6

struct IconDesc {
	char *szName;
	char *szDesc;
	int uId;           // icon ID
};

struct NotifyMenuItemExData {
	HANDLE hContact;
	int iIcon;              // icon index in the image list
	HICON hIcon;            // corresponding icon handle
	HANDLE hDbEvent;
};

#define BUTTON_HEIGHT_D 21
#define BUTTON_WIDTH_D 21
// #define NOTIFY_HEIGHT 24

#define TOPBUTTON_PUSH 1
#define TOPBUTTON_SENDONDOWN 2

struct CluiTopButton {
	HWND hwnd;
	HICON hIcon, hAltIcon;
	UINT id, idIcon, idAltIcon;
	char *szIcoLibIcon, *szIcoLibAltIcon;
	DWORD flags;
	DWORD visibilityOrder;
	TCHAR *szTooltip;
};

struct TrayIconInfo {
	union {
		HICON hIcon;
		int iIcon;
	};
};

typedef struct {
	char protoName[50];
	UINT menuID;
	BOOL added;
	HICON hIcon;
} protoMenu;

//clcidents.c
int FindItem(HWND hwnd, struct ClcData *dat, HANDLE hItem, struct ClcContact **contact, struct ClcGroup **subgroup, int *isVisible);
HANDLE ContactToItemHandle(struct ClcContact *contact, DWORD *nmFlags);

//clcitems.c
void RebuildEntireList(HWND hwnd, struct ClcData *dat);
void SaveStateAndRebuildList(HWND hwnd, struct ClcData *dat);
DWORD INTSORT_GetLastMsgTime(HANDLE hContact);

//clcmsgs.c
LRESULT ProcessExternalMessages(HWND hwnd, struct ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam);

//clcutils.c
void 	SetGroupExpand(HWND hwnd, struct ClcData *dat, struct ClcGroup *group, int newState);
void 	DoSelectionDefaultAction(HWND hwnd, struct ClcData *dat);
int 	FindRowByText(HWND hwnd, struct ClcData *dat, const TCHAR *text, int prefixOk);
void 	BeginRenameSelection(HWND hwnd, struct ClcData *dat);
int 	HitTest(HWND hwnd, struct ClcData *dat, int testx, int testy, struct ClcContact **contact, struct ClcGroup **group, DWORD *flags);
void 	ScrollTo(HWND hwnd, struct ClcData *dat, int desty, int noSmooth);
void 	RecalcScrollBar(HWND hwnd, struct ClcData *dat);
size_t 	MY_pathToRelative(const TCHAR *pSrc, TCHAR *pOut);
size_t 	MY_pathToAbsolute(const TCHAR *pSrc, TCHAR *pOut);

#define DROPTARGET_OUTSIDE    0
#define DROPTARGET_ONSELF     1
#define DROPTARGET_ONNOTHING  2
#define DROPTARGET_ONGROUP    3
#define DROPTARGET_ONCONTACT  4
#define DROPTARGET_INSERTION  5
int GetDropTargetInformation(HWND hwnd, struct ClcData *dat, POINT pt);
void LoadClcOptions(HWND hwnd, struct ClcData *dat);
void RecalculateGroupCheckboxes(HWND hwnd, struct ClcData *dat);
void SetGroupChildCheckboxes(struct ClcGroup *group, int checked);
BYTE GetCachedStatusMsg(int iExtraCacheEntry, char *szProto);
int __fastcall GetStatusOnlineness(int status);
void GetExtendedInfo(struct ClcContact *contact, struct ClcData *dat);
extern LRESULT CALLBACK NewStatusBarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void HideShowNotifyFrame();
DWORD GetCLUIWindowStyle(BYTE style);
void ApplyCLUIBorderStyle(HWND hwnd);

int FrameNCCalcSize(HWND hwnd, WNDPROC oldWndProc, WPARAM wParam, LPARAM lParam, BOOL hasTitleBar);
int FrameNCPaint(HWND hwnd, WNDPROC oldWndProc, WPARAM wParam, LPARAM lParam, BOOL hasTitleBar);

void FreeProtocolData( void );

void GetClientID(struct ClcContact *contact, char *client);
int LoadCLCButtonModule(void);
void SetButtonStates(HWND hwnd);
void ConfigureCLUIGeometry(int mode);
void IcoLibReloadIcons();
int CompareContacts(const struct ClcContact* p1, const struct ClcContact* p2);
void PaintNotifyArea(HDC hDC, RECT *rc);
int AvatarChanged(WPARAM wParam, LPARAM lParam);
void ConfigureFrame();
void ConfigureEventArea(HWND hwnd);
void ClearIcons(int mode);
void SkinDrawBg(HWND hwnd, HDC hdc);
int GetBasicFontID(struct ClcContact * contact);
extern int __fastcall CLVM_GetContactHiddenStatus(HANDLE hContact, char *szStatus, struct ClcData *dat);
void Utf8Decode( char* str, wchar_t** ucs2 );
char* Utf8Encode( const char* src );
//int _DebugPopup(HANDLE hContact, const char *fmt, ...);
void CreateViewModeFrame();
int GetExtraCache(HANDLE hContact, char *szProto);
void ReloadExtraInfo(HANDLE hContact);
void LoadAvatarForContact(struct ClcContact *p);
void ApplyViewMode(const char *name);
DWORD CalcXMask(HANDLE hContact);

//clcpaint.c
void PaintClc(HWND hwnd, struct ClcData *dat, HDC hdc, RECT *rcPaint);
void __inline PaintItem(HDC hdcMem, struct ClcGroup *group, struct ClcContact *contact, int indent, int y, struct ClcData *dat, int index, HWND hwnd, DWORD style, RECT *clRect, BOOL *bFirstNGdrawn, int groupCountsFontTopShift, int rowHeight);
void Reload3dBevelColors();
void ReloadThemedOptions();
void CreateButtonBar(HWND hWnd);
void SetButtonToSkinned();
void RTL_DetectAndSet(struct ClcContact *contact, HANDLE hContact);
void RTL_DetectGroupName(struct ClcContact *group);
void CLN_LoadAllIcons(BOOL mode);
void ReloadSkinItemsToCache();
void SFL_RegisterWindowClass(), SFL_UnregisterWindowClass();
void SFL_Create();
void SFL_Destroy();
void SFL_SetState(int iMode);
void SFL_SetSize();
void SFL_PaintNotifyArea();
void SFL_Update(HICON hIcon, int iIcon, HIMAGELIST hIml, const TCHAR *szText, BOOL refresh);

void FLT_Update(struct ClcData *dat, struct ClcContact *contact);
int FLT_CheckAvail();
void FLT_Create(int iEntry);
void FLT_SetSize(struct TExtraCache *centry, LONG width, LONG height);
void FLT_SyncWithClist();
void FLT_ReadOptions(), FLT_WriteOptions(), FLT_RefreshAll();

//clcopts.c
int ClcOptInit(WPARAM wParam, LPARAM lParam);
DWORD GetDefaultExStyle(void);
void GetFontSetting(int i, LOGFONTA *lf, COLORREF *colour);
void CluiProtocolStatusChanged( int parStatus, const char* szProto );

void LoadSkinItemToCache(struct TExtraCache *cEntry, const char *szProto);

// debugging support

void _DebugTraceW(const wchar_t *fmt, ...);
#ifdef _CLN_GDIP
extern "C" void _DebugTraceA(const char *fmt, ...);
#else
void _DebugTraceA(const char *fmt, ...);
#endif
// Docking.c

int Docking_IsDocked(WPARAM wParam, LPARAM lParam);

// Menus

int ClcSoundHook(WPARAM wParam, LPARAM lParam);

void IMG_DeleteItems();
int CoolSB_SetupScrollBar();

#define NIIF_INTERN_UNICODE 0x00000100

#define SETTING_WINDOWSTYLE_DEFAULT 0

#define SETTING_TRAYICON_SINGLE   0
#define SETTING_TRAYICON_CYCLE    1
#define SETTING_TRAYICON_MULTI    2

#define SETTING_STATE_HIDDEN      0
#define SETTING_STATE_MINIMIZED   1
#define SETTING_STATE_NORMAL      2

#define SETTING_BRINGTOFRONT_DEFAULT 0

#define SETTING_WINDOWSTYLE_TOOLWINDOW 1
#define SETTING_WINDOWSTYLE_THINBORDER 2
#define SETTING_WINDOWSTYLE_NOBORDER 3

#define BM_SETPRIVATEICON (WM_USER + 6)
#define BM_SETIMLICON (WM_USER + 7)
#define BM_SETSKINNED (WM_USER + 8)
#define BM_SETASMENUACTION (WM_USER + 9)
#define BM_SETBTNITEM (WM_USER+10)

#define EIMG_SHOW_RES0 8
#define EIMG_SHOW_EMAIL 1
#define EIMG_SHOW_WEB 2
#define EIMG_SHOW_SMS 4
#define EIMG_SHOW_ADV1 16
#define EIMG_SHOW_ADV2 32
#define EIMG_SHOW_ADV3 512
#define EIMG_SHOW_CLIENT 64
#define EIMG_SHOW_ADV4 1024
#define EIMG_SHOW_RES1 128
#define EIMG_SHOW_RES2 256

#define CLCHT_ONITEMEXTRAEX  0x1000  //on an extra icon, HIBYTE(HIWORD()) says which
#define CLCHT_ONAVATAR       0x2000
#define CLCHT_ONITEMSPACE     0x4000

#define CLM_SETEXTRAIMAGEINT   (CLM_FIRST+101)
#define CLM_SETSTICKY (CLM_FIRST+100)
#define CLM_ISMULTISELECT (CLM_FIRST+102)
#define CLM_SETEXTRAIMAGEINTMETA   (CLM_FIRST+104)
#define CLM_GETSTATUSMSG (CLM_FIRST+105)
#define CLM_SETHIDESUBCONTACTS (CLM_FIRST+106)
#define CLM_TOGGLEPRIORITYCONTACT (CLM_FIRST+107)
#define CLM_QUERYPRIORITYCONTACT (CLM_FIRST+108)
#define CLM_TOGGLEFLOATINGCONTACT (CLM_FIRST+109)
#define CLM_QUERYFLOATINGCONTACT (CLM_FIRST+110)

#define IDC_RESETMODES 110
#define IDC_SELECTMODE 108
#define IDC_CONFIGUREMODES 109

#define NR_CLIENTS 40

typedef BOOL (WINAPI *PGF)(HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);

/*
 * floating stuff
 */

#define FLT_SIMPLE			1
#define FLT_AVATARS			2
#define FLT_DUALROW			4
#define FLT_EXTRAICONS		8
#define FLT_SYNCWITHCLIST	16
#define FLT_AUTOHIDE		32
#define FLT_SNAP			64
#define FLT_BORDER			128
#define FLT_ROUNDED			256
#define FLT_FILLSTDCOLOR    512
#define FLT_SHOWTOOLTIPS	1024

typedef struct _floatopts {
	DWORD dwFlags;
	BYTE  pad_left, pad_right, pad_top, pad_bottom;
	DWORD width;
	COLORREF border_colour;
	BYTE trans, act_trans;
	BYTE radius;
	BYTE enabled;
    BYTE def_hover_time;
    WORD hover_time;
} FLOATINGOPTIONS;

extern FLOATINGOPTIONS g_floatoptions;


