/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

#define INTM_XSTATUSCHANGED		(WM_USER+26)
#define INTM_METACHANGEDEVENT	(WM_USER+27)
#define INTM_CODEPAGECHANGED	(WM_USER+28)
#define INTM_AVATARCHANGED		(WM_USER+30)
#define INTM_STATUSMSGCHANGED	(WM_USER+31)
#define INTM_SORTCLC			(WM_USER+32)
#define INTM_STATUSCHANGED		(WM_USER+33)
#define INTM_METACHANGED		(WM_USER+34)
#define INTM_FORCESORT			(WM_USER+36)

#define DEFAULT_TITLEBAR_HEIGHT	18

#define CLS_SKINNEDFRAME		0x0800 //this control will be the main contact list (v. 0.3.4.3+ 2004/11/02)

#define TIMERID_RENAME			10
#define TIMERID_DRAGAUTOSCROLL	11
#define TIMERID_INFOTIP			13
#define TIMERID_SORT			15
#define TIMERID_REFRESH			18
#define TIMERID_PAINT			19

#define CONTACTF_ONLINE			1
//#define CONTACTF_INVISTO		2
//#define CONTACTF_VISTO		4
#define CONTACTF_NOTONLIST		8
#define CONTACTF_CHECKED		16
#define CONTACTF_IDLE			32
#define CONTACTF_STICKY			64
#define CONTACTF_PRIORITY		128

#define STATUSMSG_XSTATUSID		1
#define STATUSMSG_XSTATUSNAME	2
#define STATUSMSG_CLIST			4
#define STATUSMSG_YIM			8
#define STATUSMSG_GG			16
#define STATUSMSG_XSTATUS		32

#define STATUSMSG_NOTFOUND		0

#define SMSG_MAXLEN				700

#define EXTRAIMAGECACHESIZE		1000

// extra cache contact flags

#define ECF_RTLNICK 1
#define ECF_RTLSTATUSMSG 2
#define ECF_FORCEAVATAR 4
#define ECF_HIDEAVATAR 8
#define ECF_FORCEOVERLAY 16
#define ECF_HIDEOVERLAY 32
#define ECF_FORCELOCALTIME 64
#define ECF_HIDELOCALTIME 128
#define ECF_HASREALTIMEZONE 1024

// other contact flags (struct ClCContact;

#define ECF_AVATAR 1
#define ECF_SECONDLINE 2

#define DSPF_CENTERSTATUSICON 1
#define DSPF_DIMIDLE 2
#define DSPF_NOFFLINEAVATARS 4
#define DSPF_SHOWLOCALTIME 8
#define DSPF_LOCALTIMESELECTIVE 16
#define DSPF_DONTSEPARATEOFFLINE 32
#define DSPF_CENTERGROUPNAMES 64

/* Extra icons settings */
typedef struct _OrderTreeData
{
	uint8_t			ID;
	const wchar_t *	Name;
	uint8_t			Position;
	BOOL			Visible;
	BOOL			fReserved;
} *PORDERTREEDATA, ORDERTREEDATA;

struct TExtraCache
{
	MCONTACT	hContact;
	HANDLE		hTimeZone;
	uint8_t		valid;
	wchar_t		*statusMsg;
	uint8_t		bStatusMsgValid;
	uint32_t		dwCFlags;
	uint32_t		dwDFlags; // display flags for caching only
	StatusItems_t	*status_item, *proto_status_item;
	uint32_t		dwLastMsgTime;
	uint32_t		msgFrequency;
	BOOL		isChatRoom;
};

struct ClcCacheEntry : public ClcCacheEntryBase
{};

struct ClcContact : public ClcContactBase
{
	BOOL     bIsMeta;
	uint8_t     xStatus;
	int      xStatusIcon;
	MCONTACT	hSubContact;
	char    *metaProto;
	uint32_t    codePage;
	uint16_t     wStatus;
	int      avatarLeft, extraIconRightBegin;
	int      isRtl;
	uint32_t    cFlags;
	uint8_t     bSecondLine;

	AVATARCACHEENTRY *ace;
	TExtraCache *pExtra;
};

#define DRAGSTAGE_NOTMOVED		0
#define DRAGSTAGE_ACTIVE		1
#define DRAGSTAGEM_STAGE		0x00FF
#define DRAGSTAGEF_MAYBERENAME	0x8000
#define DRAGSTAGEF_OUTSIDE		0x4000

#define FONTID_CONTACTS		0
#define FONTID_INVIS		1
#define FONTID_OFFLINE		2
#define FONTID_NOTONLIST	3
#define FONTID_GROUPS		4
#define FONTID_GROUPCOUNTS	5
#define FONTID_DIVIDERS		6
#define FONTID_OFFINVIS		7
#define FONTID_STATUS		8
#define FONTID_FRAMETITLE	9
#define FONTID_EVENTAREA	10
#define FONTID_TIMESTAMP	11
#define FONTID_LAST			FONTID_TIMESTAMP

struct ClcData : public ClcDataBase
{
	int *row_heights;
	int row_heights_size;
	int row_heights_allocated;

	int row_border;
	int min_row_heigh, group_row_height;

	int currentFontID;
	int rightMargin;
	uint8_t SelectMode;
	uint8_t isMultiSelect;
	HWND hwndParent;
	uint32_t lastSort;
	uint32_t lastRepaint;
	int oldSelection;

	bool bNeedPaint, bisEmbedded, bHideSubcontacts, bForceScroll;
};

//#define CLUI_FRAME_SHOWTOPBUTTONS 1
#define CLUI_FRAME_SHOWBOTTOMBUTTONS 2
#define CLUI_SHOWCLIENTICONS 4
//#define CLUI_SHOWVISI 8
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
//#define CLUI_FRAME_BUTTONBARSUNKEN 65536
#define CLUI_FRAME_AVATARS				0x20000
#define CLUI_FRAME_AVATARSLEFT			0x40000
#define CLUI_FRAME_GDIPLUS				0x80000
#define CLUI_FRAME_AVATARBORDER			0x100000
#define CLUI_FRAME_STATUSICONS			0x200000
#define CLUI_FRAME_AVATARSRIGHTWITHNICK	0x400000
#define CLUI_FRAME_TRANSPARENTAVATAR	0x800000
#define CLUI_FRAME_ROUNDAVATAR			0x1000000
#define CLUI_FRAME_ALWAYSALIGNNICK		0x2000000
#define CLUI_FRAME_AVATARSRIGHT			0x4000000
#define CLUI_FRAME_SHOWSTATUSMSG		0x8000000
#define CLUI_FRAME_OVERLAYICONS			0x10000000
#define CLUI_FRAME_SELECTIVEICONS		0x20000000
#define CLUI_FRAME_ROUNDEDFRAME			0x40000000
#define CLUI_FRAME_NOGROUPICON			0x80000000

#define MULTIROW_NEVER 0
#define MULTIROW_ALWAYS 1
#define MULTIROW_IFSPACE 2
#define MULTIROW_IFNEEDED 3

#define CLC_GROUPALIGN_LEFT 0
#define CLC_GROUPALIGN_RIGHT 1
#define CLC_GROUPALIGN_AUTO 2

struct TCluiData {
	uint32_t dwFlags;
	uint32_t topOffset, bottomOffset;
	int statusBarHeight;
	int soundsOff;
	uint8_t bAvatarServiceAvail;
	HICON hIconConnecting;
	uint32_t winFlags;
	uint32_t winFlagsEx;
	int notifyActive;
	int hIconNotify;
	HMENU hMenuNotify;
	int iLastEventAdded;
	int wNextMenuID;
	MCONTACT hUpdateContact;
	uint32_t sortTimer;
	BOOL forceResize;
	BOOL neeedSnap;
	COLORREF avatarBorder;
	HBRUSH hBrushAvatarBorder, hBrushColorKey;
	HBRUSH hBrushCLCBk;
	uint32_t avatarRadius;
	int avatarSize;
	BOOL bForceRefetchOnPaint;
	uint8_t dualRowMode;
	uint8_t avatarPadding;
	uint8_t cornerRadius;
	uint8_t isTransparent;
	uint8_t alpha, autoalpha;
	uint8_t fadeinout;
	uint8_t autosize;
	uint8_t gapBetweenFrames;
	uint8_t titleBarHeight;
	uint8_t bClipBorder, bRowSpacing;
	HBITMAP bmpBackground, hbmBgOld, hbmBg;
	HBITMAP hbmToolbar, hbmToolbarOld;
	HDC hdcBg;
	HDC hdcPic;
	HDC hdcToolbar;
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
	uint32_t bFilterEffective;
	BOOL bCenterStatusIcons;
	BOOL bSkinnedToolbar;
	BOOL bSkinnedStatusBar;
	BOOL bUsePerProto;
	BOOL bOverridePerStatusColors;
	BOOL bDontSeparateOffline;
	wchar_t groupFilter[2048];
	char protoFilter[2048];
	char varFilter[2048];
	uint32_t lastMsgFilter;
	char current_viewmode[256], old_viewmode[256];
	uint8_t boldHideOffline;
	uint32_t statusMaskFilter;
	uint32_t stickyMaskFilter;
	uint32_t filterFlags;
	COLORREF colorkey;
	BOOL bMetaEnabled;
	BOOL bSecIMAvail;
	BOOL bNoTrayTips;
	BOOL bShowLocalTime;
	BOOL bShowLocalTimeSelective;
	BOOL bShowXStatusOnSbar;
	BOOL bLayeredHack;
	HPEN hPen3DBright, hPen3DDark;
	uint8_t bSkinnedButtonMode;
	uint8_t bFirstRun;
	uint8_t bUseDCMirroring;
	uint8_t bCLeft, bCRight, bCTop, bCBottom;
	uint8_t fullyInited;
	uint8_t bAutoExpandGroups;
	SIZE szOldCTreeSize;
	uint8_t bWantFastGradients, bUseFastGradients;
	uint8_t sortOrder[3];
	uint8_t bGroupAlign;
	uint8_t bSkinnedScrollbar;
	uint32_t langPackCP;
	BOOL fOnDesktop;
	int group_padding;
	uint32_t t_now;
	BOOL realTimeSaving;
	wchar_t tszProfilePath[MAX_PATH];
	FILETIME ft;
	SYSTEMTIME st;
};

#define SORTBY_NAME 1
#define SORTBY_PROTO 2
#define SORTBY_STATUS 3
#define SORTBY_LASTMSG 4
#define SORTBY_FREQUENCY 5
#define SORTBY_PRIOCONTACTS 6

struct IconDesc
{
	char *szName;
	char *szDesc;
	int uId;           // icon ID
};

struct NotifyMenuItemExData
{
	MCONTACT hContact;
	int iIcon;              // icon index in the image list
	HICON hIcon;            // corresponding icon handle
	MEVENT hDbEvent;
};

// #define NOTIFY_HEIGHT 24

struct CluiTopButton
{
	int ctrlid;
	char *pszButtonID, *pszButtonDn, *pszButtonName;
	int isPush, isVis, isAction;
	HANDLE hButton;
	HWND hwndButton;
};

struct TrayIconInfo
{
	union
	{
		HICON hIcon;
		int iIcon;
	};
};

struct protoMenu
{
	char protoName[50];
	UINT menuID;
	BOOL added;
	HICON hIcon;
};

// clcitems.c
void RebuildEntireList(HWND hwnd, struct ClcData *dat);
uint32_t INTSORT_GetLastMsgTime(MCONTACT hContact);

// clcmsgs.c
LRESULT ProcessExternalMessages(HWND hwnd, struct ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam);

// clcutils.c
void   SetGroupExpand(HWND hwnd, struct ClcData *dat, ClcGroup *group, int newState);
void   BeginRenameSelection(HWND hwnd, struct ClcData *dat);
int    HitTest(HWND hwnd, struct ClcData *dat, int testx, int testy, ClcContact **contact, ClcGroup **group, uint32_t *flags);
void   ScrollTo(HWND hwnd, struct ClcData *dat, int desty, int noSmooth);
void   RecalcScrollBar(HWND hwnd, struct ClcData *dat);
size_t MY_pathToRelative(const wchar_t *pSrc, wchar_t *pOut);
size_t MY_pathToAbsolute(const wchar_t *pSrc, wchar_t *pOut);

int    GetDropTargetInformation(HWND hwnd, struct ClcData *dat, POINT pt);
void   LoadClcOptions(HWND hwnd, struct ClcData *dat, BOOL bFirst);
uint8_t   GetCachedStatusMsg(TExtraCache* p, char *szProto);
void   GetExtendedInfo(ClcContact *contact, struct ClcData *dat);
LRESULT CALLBACK NewStatusBarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void   HideShowNotifyFrame();
uint32_t  GetCLUIWindowStyle(uint8_t style);
void   ApplyCLUIBorderStyle();

int FrameNCCalcSize(HWND hwnd, WNDPROC oldWndProc, WPARAM wParam, LPARAM lParam, BOOL hasTitleBar);
int FrameNCPaint(HWND hwnd, WNDPROC oldWndProc, WPARAM wParam, LPARAM lParam, BOOL hasTitleBar);

void FreeProtocolData( void );

void SetButtonStates();
void ConfigureCLUIGeometry(int mode);
void IcoLibReloadIcons();
int  CompareContacts(const ClcContact* p1, const ClcContact* p2);
void PaintNotifyArea(HDC hDC, RECT *rc);
int  AvatarChanged(WPARAM wParam, LPARAM lParam);
void ConfigureFrame();
void ConfigureEventArea();
void ClearIcons(int mode);
void SkinDrawBg(HWND hwnd, HDC hdc);
int  GetBasicFontID(ClcContact *contact);
int  CLVM_GetContactHiddenStatus(MCONTACT hContact, char *szStatus, struct ClcData *dat);
void CreateViewModeFrame();
void ReloadExtraInfo(MCONTACT hContact);
void LoadAvatarForContact(ClcContact *p);
void ApplyViewMode(const char *name);

void ClcSetButtonState(int ctrlId, int status);
HWND ClcGetButtonWindow(int ctrlid);

// clcpaint.c
void PaintClc(HWND hwnd, struct ClcData *dat, HDC hdc, RECT *rcPaint);
void __inline PaintItem(HDC hdcMem, ClcGroup *group, ClcContact *contact, int indent, int y, struct ClcData *dat, int index, HWND hwnd, uint32_t style, RECT *clRect, BOOL *bFirstNGdrawn, int groupCountsFontTopShift, int rowHeight);
void Reload3dBevelColors();
void ReloadThemedOptions();
void SetButtonToSkinned();
void RTL_DetectAndSet(ClcContact *contact, MCONTACT hContact);
void RTL_DetectGroupName(ClcContact *group);
void CLN_LoadAllIcons(BOOL mode);
void ReloadSkinItemsToCache();

// clcopts.c
int  ClcOptInit(WPARAM wParam, LPARAM lParam);
void CluiProtocolStatusChanged(int, const char*);

void LoadSkinItemToCache(struct TExtraCache *cEntry);

// Docking.c

int Docking_IsDocked(WPARAM wParam, LPARAM lParam);

// Buttons

#define BUTTONSETIMLICON		(WM_USER+20)
#define BUTTONSETSKINNED		(WM_USER+21)
#define BUTTONSETBTNITEM		(WM_USER+22)
#define BUTTONSETTTBUTTON		(WM_USER+23)

// Menus

void IMG_DeleteItems();
int CoolSB_SetupScrollBar();

#define NIIF_INTERN_UNICODE			0x00000100

#define SETTING_WINDOWSTYLE_DEFAULT	0

#define SETTING_TRAYICON_SINGLE		0
#define SETTING_TRAYICON_CYCLE		1
#define SETTING_TRAYICON_MULTI		2

#define SETTING_STATE_HIDDEN		0
#define SETTING_STATE_MINIMIZED		1
#define SETTING_STATE_NORMAL		2

#define SETTING_BRINGTOFRONT_DEFAULT	0

#define SETTING_WINDOWSTYLE_TOOLWINDOW	1
#define SETTING_WINDOWSTYLE_THINBORDER	2
#define SETTING_WINDOWSTYLE_NOBORDER	3

#define CLCHT_ONAVATAR			0x2000
#define CLCHT_ONITEMSPACE		0x4000

#define CLM_SETEXTRAIMAGEINT		(CLM_FIRST+101)
#define CLM_SETSTICKY				(CLM_FIRST+100)
#define CLM_ISMULTISELECT			(CLM_FIRST+102)
#define CLM_SETEXTRAIMAGEINTMETA	(CLM_FIRST+104)
#define CLM_GETSTATUSMSG			(CLM_FIRST+105)
#define CLM_SETHIDESUBCONTACTS		(CLM_FIRST+106)
#define CLM_TOGGLEPRIORITYCONTACT	(CLM_FIRST+107)
#define CLM_QUERYPRIORITYCONTACT	(CLM_FIRST+108)

#define IDC_RESETMODES		110
#define IDC_SELECTMODE		108
#define IDC_CONFIGUREMODES	109

#define NR_CLIENTS 40

typedef BOOL (WINAPI *PGF)(HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);

#define IDC_STBHIDEOFFLINE			IDC_TBHIDEOFFLINE - 20
#define IDC_STBHIDEGROUPS			IDC_TBHIDEGROUPS - 20
#define IDC_STBSOUND				IDC_TBSOUND - 20
#define IDC_STBFINDANDADD			IDC_TBFINDANDADD - 20
#define IDC_STBOPTIONS				IDC_TBOPTIONS - 20
#define IDC_STBMINIMIZE				IDC_TBMINIMIZE - 20
#define IDC_STABSRMMSLIST			IDC_TABSRMMSLIST - 20
#define IDC_STABSRMMMENU			IDC_TABSRMMMENU - 20
#define IDC_STBSELECTVIEWMODE		IDC_TBSELECTVIEWMODE - 20
#define IDC_STBCLEARVIEWMODE		IDC_TBCLEARVIEWMODE - 20
#define IDC_STBCONFIGUREVIEWMODE	IDC_TBCONFIGUREVIEWMODE - 20
#define IDC_STBTOPMENU				IDC_TBTOPMENU - 20
#define IDC_STBTOPSTATUS			IDC_TBTOPSTATUS - 20
#define IDC_STBFOLDER				IDC_TBFOLDER - 20
#define IDC_STBPOPUP				IDC_TBPOPUP - 20
#define IDC_STBACCOUNTS				IDC_TBACCOUNTS - 20
