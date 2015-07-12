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
 * part of clist_ng plugin for Miranda.
 *
 * (C) 2005-2015 by silvercircle _at_ gmail _dot_ com and contributors
 */

#ifndef __CLC_H_
#define __CLC_H_

#define NR_DSPOVERRIDES 5

#define DSP_OVR_OFFLINE 0
#define DSP_OVR_ONLINE 1
#define DSP_OVR_SELECTED 2
#define DSP_OVR_HOVERED 3
#define DSP_OVR_PRIORITY 4

struct TDspOverride {
	bool			fActive;
	char			bAvatar;
	char			bSecondLine;
	char			bIcon;
    BYTE     		exIconOrder[EXICON_COUNT];
};

struct TDisplayProfile {
    DWORD   		dwFlags;
    DWORD   		dwExtraImageMask;
    int     		exIconScale;
    BOOL    		bCenterStatusIcons;
    BOOL    		bDimIdle, bNoOfflineAvatars,
    				bShowLocalTime, bShowLocalTimeSelective,
    				bDontSeparateOffline, bCenterGroupNames;
    BYTE    		dualRowMode;
    COLORREF 		avatarBorder;
    int      		avatarSize;
    DWORD    		clcExStyle;
    DWORD    		clcOfflineModes;
    BYTE     		sortOrder[3], bUseDCMirroring, bGroupAlign;
    BYTE     		avatarPadding;
    BYTE     		bLeftMargin, bRightMargin, bRowSpacing,
    				bGroupIndent, bRowHeight, bGroupRowHeight;
    BYTE     		exIconOrder[EXICON_COUNT];
    BYTE	 		bReserved[16];
    TDspOverride 	dspOverride[NR_DSPOVERRIDES];
};

class CLC
{
public:

	static LRESULT CALLBACK 		wndProc							(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static void 					Paint							(HWND hwnd, ClcData *dat, HDC hdc, RECT *rcPaint);
	static void 					PaintItem						(HDC hdcMem, ClcGroup *group, struct ClcContact *contact, int indent,
			int y, struct ClcData *dat, int index, HWND hwnd, DWORD style,
			RECT *clRect, BOOL *bFirstNGdrawn, int groupCountsFontTopShift, int rowHeight);
	static int 						AddContactToGroup				(ClcData *dat, ClcGroup *group, MCONTACT hContact);
	static void 					RebuildEntireList				(HWND hwnd, ClcData *dat);
	static ClcGroup* 				RemoveItemFromGroup				(HWND hwnd, ClcGroup *group, ClcContact *contact, int updateTotalCount);
	static CListEvent* 				AddEvent						(CLISTEVENT *cle);
	static int 						RemoveEvent						(MCONTACT hContact, MEVENT hDbEvent);
	static ClcGroup* 				AddGroup						(HWND hwnd, ClcData *dat, const wchar_t *szName, DWORD flags, int groupId, int calcTotalMembers);
	static int 						AddInfoItemToGroup				(ClcGroup *group, int flags, const wchar_t *pszText);
	static int 						fnIconFromStatusMode			( const char* szProto, int status, MCONTACT hContact );
	static int 						IconFromStatusMode				(const char *szProto, int status, MCONTACT hContact, HICON *phIcon);
	static void 					LoadClcOptions					(HWND hwnd, ClcData *dat, BOOL first);
	static void 					LoadContactTree					();
	static LRESULT 					ProcessExternalMessages			(HWND hwnd, ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam);
	static void 					RecalcScrollBar					(HWND hwnd, ClcData *dat);
	static INT_PTR 					TrayIconProcessMessage			(WPARAM wParam, LPARAM lParam);

	static int 						SettingChanged					(WPARAM wParam, LPARAM lParam);
	static void 					ScrollTo						(HWND hwnd, ClcData *dat, int desty, int noSmooth);
	static ClcContact* 				CreateClcContact				();
	static CListEvent* 				fnCreateEvent					();
	static int 						HitTest							(HWND hwnd, ClcData *dat, int testx, int testy, ClcContact **contact,
																	 ClcGroup **group, DWORD *flags);
	static int 						RTL_HitTest						(HWND hwnd, ClcData *dat, int testx, int testy, ClcContact *hitcontact,
																	 DWORD *flags, int indent, int hit);

	static int 						loadModule						(void);
	static int 						preshutdown						(WPARAM wParam, LPARAM lParam);
	static int 						shutDown						(WPARAM wParam, LPARAM lParam);
	static int 						findItem						(HWND hwnd, ClcData *dat, HANDLE hItem, ClcContact **contact, ClcGroup **subgroup, int *isVisible);
	static int 						SetHideOffline					(WPARAM wParam, LPARAM lParam);
	static int 						CompareContacts					(const ClcContact* c1, const ClcContact* c2);
	static void 					BeginRenameSelection			(HWND hwnd, ClcData *dat);
	static void 					countAvatars					(ClcData *dat);


	static inline int getStatusOnlineness(int status, bool ignoreConnectingState)
	{
		if(!ignoreConnectingState) {
			if(status >= ID_STATUS_CONNECTING && status < ID_STATUS_OFFLINE)
				return 120;
		}

		if(status >= ID_STATUS_ONLINE && status <= ID_STATUS_OUTTOLUNCH)
			return(_status2onlineness[status - ID_STATUS_OFFLINE]);
		return 0;
	}

	static inline int				getGeneralisedStatus			()
	{
		int status, thisStatus, statusOnlineness, thisOnlineness;

		status = ID_STATUS_OFFLINE;
		statusOnlineness = 0;

			thisStatus = cfg::maxStatus;
			if (thisStatus == ID_STATUS_INVISIBLE)
				return ID_STATUS_INVISIBLE;
			thisOnlineness = getStatusOnlineness(thisStatus, true);
			if (thisOnlineness > statusOnlineness) {
				status = thisStatus;
				statusOnlineness = thisOnlineness;
			}
		return status;
	}

	static LRESULT	(CALLBACK*		saveContactListControlWndProc)	(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	/*
	static int 					(*	saveAddContactToGroup )			(ClcData *dat, ClcGroup *group, MCONTACT hContact);
	static ClcGroup* 			(*	saveRemoveItemFromGroup )		(HWND hwnd, ClcGroup *group, ClcContact *contact, int updateTotalCount);
	static CListEvent* 			(*	saveAddEvent )					(CLISTEVENT *cle);
	static int 					(*	saveRemoveEvent )				(MCONTACT hContact, HANDLE hDbEvent);
	static ClcGroup* 			(*	saveAddGroup )					(HWND hwnd, ClcData *dat, const wchar_t *szName, DWORD flags, int groupId, int calcTotalMembers);
	static int 					(*	saveAddInfoItemToGroup )		(ClcGroup *group, int flags, const wchar_t *pszText);
	static int 					(*	saveIconFromStatusMode )		(const char *szProto, int status, MCONTACT hContact );
	static void 				(*	saveLoadClcOptions )			(HWND hwnd, ClcData *dat, BOOL first);
	static LRESULT 				(*	saveProcessExternalMessages )	(HWND hwnd, ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam);
	static void 				(*	saveRecalcScrollBar )			(HWND hwnd, ClcData *dat);
	static INT_PTR 				(*	saveTrayIconProcessMessage )	(WPARAM wParam, LPARAM lParam);
	*/
	static bool						fHottrackDone, fInPaint;
	static int						iHottrackItem;
	static HANDLE 					hSettingsChanged, hDBEvent;
	static HIMAGELIST				hClistImages;
	static HANDLE					hTheme;
	static unsigned int				uNrAvatars;
	static TDisplayProfile			dsp_default;
	static int 						_status2onlineness[];

};

#define MAXEXTRACOLUMNS     16

#define INTM_XSTATUSCHANGED  (WM_USER+26)
#define INTM_METACHANGEDEVENT (WM_USER+27)
#define INTM_CODEPAGECHANGED (WM_USER+28)
#define INTM_CLIENTCHANGED   (WM_USER+29)
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
	ContactFloater*	pNextFloater;
	HWND 			hwnd;
	HDC 			hdc;
	HBITMAP 		hbm, hbmOld;
	MCONTACT		hContact;
};

typedef struct ContactFloater CONTACTFLOATER;

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
	BYTE	ID;
	wchar_t *	Name;
	BYTE	Position;
	BOOL	Visible;
    BOOL    fReserved;
} *PORDERTREEDATA, ORDERTREEDATA;

#define DSP_PROFILES_MODULE "CLG_DspProfiles"           // db module for display profile overrides

struct ClcContact : public ClcContactBase
{
	BOOL		bIsMeta;
	BYTE		xStatus;
	int			xStatusIcon;
	MCONTACT	hSubContact;
	char		*metaProto;
	DWORD		codePage;
	WORD		wStatus;
	int			avatarLeft, extraIconRightBegin;
	int			isRtl;
	DWORD		cFlags;
	BYTE		bSecondLine;
	BYTE		bSecondLineLocal;
	DWORD		dwDFlags;
	avatarCacheEntry	*ace;
	int 				extraCacheEntry;
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

class CLCPaintHelper;

struct ClcData : public ClcDataBase
{
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
	int oldSelection;
	CLCPaintHelper *ph;
};


#define CLUI_FRAME_SHOWBOTTOMBUTTONS 2
#define CLUI_SHOWCLIENTICONS 4
#define CLUI_SHOWVISI 8
#define CLUI_FRAME_CLISTSUNKEN 16
#define CLUI_SHOWXSTATUS 32
#define CLUI_FRAME_BUTTONSFLAT 64
#define CLUI_FRAME_BUTTONSCLASSIC 128
#define CLUI_USEMETAICONS 256
#define CLUI_FRAME_AUTOHIDENOTIFY 512
#define CLUI_STICKYEVENTS 2048
#define CLUI_FRAME_SBARSHOW 4096
#define CLUI_STATUSASTEXT 8192
#define CLUI_FULLROWSELECT 16384
#define CLUI_FRAME_BUTTONBARSUNKEN 65536
#define CLUI_FRAME_AVATARS         0x20000
#define CLUI_FRAME_AVATARSLEFT     0x40000
#define CLUI_FRAME_GDIPLUS         0x80000
#define CLUI_FRAME_AVATARBORDER    0x100000
#define CLUI_FRAME_STATUSICONS     0x200000
#define CLUI_FRAME_AVATARSRIGHTWITHNICK    0x400000
#define CLUI_FRAME_TRANSPARENTAVATAR 0x800000
//#define CLUI_FRAME_ROUNDAVATAR      0x1000000
#define CLUI_FRAME_ALWAYSALIGNNICK  0x2000000
#define CLUI_FRAME_AVATARSRIGHT     0x4000000
#define CLUI_FRAME_SHOWSTATUSMSG    0x8000000
#define CLUI_FRAME_OVERLAYICONS    0x10000000
#define CLUI_FRAME_SELECTIVEICONS  0x20000000
// #define CLUI_FRAME_ROUNDEDFRAME    0x40000000
#define CLUI_FRAME_NOGROUPICON     0x80000000

#define MULTIROW_NEVER 0
#define MULTIROW_ALWAYS 1
#define MULTIROW_IFNEEDED 2

#define CLUI_USE_FLOATER 1
#define CLUI_FLOATER_AUTOHIDE 2
#define CLUI_FLOATER_EVENTS 4

#define CLC_GROUPALIGN_LEFT 0
#define CLC_GROUPALIGN_RIGHT 1
#define CLC_GROUPALIGN_AUTO 2

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
	MCONTACT hContact;
	int iIcon;              // icon index in the image list
	HICON hIcon;            // corresponding icon handle
	MEVENT hDbEvent;
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
	wchar_t *szTooltip;
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

DWORD 			INTSORT_GetLastMsgTime(MCONTACT hContact);
int 			InitCustomMenus(void);

void 	DoSelectionDefaultAction(HWND hwnd, struct ClcData *dat);

#define DROPTARGET_OUTSIDE    0
#define DROPTARGET_ONSELF     1
#define DROPTARGET_ONNOTHING  2
#define DROPTARGET_ONGROUP    3
#define DROPTARGET_ONCONTACT  4
#define DROPTARGET_INSERTION  5

BYTE GetCachedStatusMsg(int iExtraCacheEntry, char *szProto);
void GetExtendedInfo(struct ClcContact *contact, struct ClcData *dat);
extern LRESULT CALLBACK NewStatusBarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int FrameNCCalcSize(HWND hwnd, WNDPROC oldWndProc, WPARAM wParam, LPARAM lParam, BOOL hasTitleBar);
int FrameNCPaint(HWND hwnd, WNDPROC oldWndProc, WPARAM wParam, LPARAM lParam, BOOL hasTitleBar);

void FreeProtocolData( void );
void DSP_LoadFromDefaults(TDisplayProfile *);

void GetClientID(struct ClcContact *contact, char *client);
int LoadCLCButtonModule(void);
void SetButtonStates(HWND hwnd);
void IcoLibReloadIcons();
int AvatarChanged(WPARAM wParam, LPARAM lParam);
void ClearIcons(int mode);
int GetBasicFontID(struct ClcContact * contact);
extern int __fastcall CLVM_GetContactHiddenStatus(MCONTACT hContact, char *szStatus, struct ClcData *dat);
void CreateViewModeFrame();
void ReloadExtraInfo(MCONTACT hContact);
void LoadAvatarForContact(struct ClcContact *p);
void ApplyViewMode(const char *name);
DWORD CalcXMask(MCONTACT hContact);

void Reload3dBevelColors();
void ReloadThemedOptions();
void CreateButtonBar(HWND hWnd);
void SetButtonToSkinned();
void RTL_DetectAndSet(struct ClcContact *contact, MCONTACT hContact);
void RTL_DetectGroupName(struct ClcContact *group);
void CLN_LoadAllIcons(BOOL mode);
void ReloadSkinItemsToCache();
void SFL_RegisterWindowClass();
void SFL_UnregisterWindowClass();
void SFL_Create();
void SFL_Destroy();
void SFL_SetState(int iMode);
void SFL_SetSize();
void SFL_PaintNotifyArea();
void SFL_Update(HICON hIcon, int iIcon, HIMAGELIST hIml, const wchar_t *szText, BOOL refresh);

void FLT_Update(struct ClcData *dat, struct ClcContact *contact);
int FLT_CheckAvail();
void FLT_Create(int iEntry);
void FLT_SetSize(struct TExtraCache *centry, LONG width, LONG height);
void FLT_SyncWithClist();
void FLT_ReadOptions();
void FLT_WriteOptions();
void FLT_RefreshAll();

//clcopts.c
int ClcOptInit(WPARAM wParam, LPARAM lParam);
void CluiProtocolStatusChanged( int parStatus, const char* szProto );

void LoadSkinItemToCache(struct TExtraCache *cEntry, const char *szProto);

int Docking_IsDocked(WPARAM wParam, LPARAM lParam);

// Menus

int ClcSoundHook(WPARAM wParam, LPARAM lParam);

int CoolSB_SetupScrollBar(HWND hwnd);

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

/*
 * status and contact floater flags
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
#define FLT_ONTOP			2048

typedef struct _floatopts {
	DWORD dwFlags;
	BYTE  pad_left, pad_right, pad_top, pad_bottom;
	DWORD width;
	COLORREF border_colour;
	BYTE trans, act_trans;
	BYTE enabled;
    BYTE def_hover_time;
    WORD hover_time;
} FLOATINGOPTIONS;

extern FLOATINGOPTIONS g_floatoptions;

#endif /* __CLC_H_*/
