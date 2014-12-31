/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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

#ifndef WIN2K_H__
#define WIN2K_H__ 1

/*
This file was made to define the new constants normally provided by the windows
sdk you can get from http://www.microsoft.com/msdownload/platformsdk/sdkupdate/
To not need to install the whole sdk you can simply comment out the following lines.
To make myself clear, you are supposed to use the sdk, this is just a work around.

All constants are normally declared in winuser.h

File created by Christian Kostner, and tweaked a bit by Richard Hughes*/

#define WinVerMajor()      LOBYTE(LOWORD(GetVersion()))
#define WinVerMinor()      HIBYTE(LOWORD(GetVersion()))

// put stuff that's not apart of any SDKs but is used nonetheless

#if _MSC_VER >= 1300
#define NOWIN2K
#endif

#if WINVER >= 0x501
#define NOWIN2K
#endif

#ifdef _MSC_VER
#define BIGI(x) x##i64
#else
#define BIGI(x) x##LL
#endif

// collapsible groups for Vista
#ifndef LVGS_COLLAPSIBLE
	#define LVGS_COLLAPSIBLE        0x00000008
#endif

#ifndef SM_SERVERR2
	#define SM_SERVERR2              89
	#define VER_SUITE_STORAGE_SERVER 0x00002000
	#define VER_SUITE_COMPUTE_SERVER 0x00004000
	#define VER_SUITE_WH_SERVER      0x00008000
#endif

#if _MSC_VER
	#if !defined(DTBG_CLIPRECT)
		#define DTBG_CLIPRECT           0x00000001  // rcClip has been specified
		#define DTBG_DRAWSOLID          0x00000002  // DEPRECATED: draw transparent/alpha images as solid
		#define DTBG_OMITBORDER         0x00000004  // don't draw border of part
		#define DTBG_OMITCONTENT        0x00000008  // don't draw content area of part
		#define DTBG_COMPUTINGREGION    0x00000010  // TRUE if calling to compute region
		#define DTBG_MIRRORDC           0x00000020  // assume the hdc is mirrorred and
																  // flip images as appropriate (currently
																  // only supported for bgtype = imagefile)
		#define DTBG_NOMIRROR           0x00000040  // don't mirror the output, overrides everything else

		typedef struct _DTBGOPTS
		{
			DWORD dwSize;           // size of the struct
			DWORD dwFlags;          // which options have been specified
			RECT rcClip;            // clipping rectangle
		} DTBGOPTS, *PDTBGOPTS;
	#endif
	#if !defined(DTT_COMPOSITED)
		#define DTT_TEXTCOLOR       (1UL << 0)      // crText has been specified
		#define DTT_BORDERCOLOR     (1UL << 1)      // crBorder has been specified
		#define DTT_SHADOWCOLOR     (1UL << 2)      // crShadow has been specified
		#define DTT_SHADOWTYPE      (1UL << 3)      // iTextShadowType has been specified
		#define DTT_SHADOWOFFSET    (1UL << 4)      // ptShadowOffset has been specified
		#define DTT_BORDERSIZE      (1UL << 5)      // iBorderSize has been specified
		#define DTT_FONTPROP        (1UL << 6)      // iFontPropId has been specified
		#define DTT_COLORPROP       (1UL << 7)      // iColorPropId has been specified
		#define DTT_STATEID         (1UL << 8)      // IStateId has been specified
		#define DTT_CALCRECT        (1UL << 9)      // Use pRect as and in/out parameter
		#define DTT_APPLYOVERLAY    (1UL << 10)     // fApplyOverlay has been specified
		#define DTT_GLOWSIZE        (1UL << 11)     // iGlowSize has been specified
		#define DTT_CALLBACK        (1UL << 12)     // pfnDrawTextCallback has been specified
		#define DTT_COMPOSITED      (1UL << 13)     // Draws text with antialiased alpha (needs a DIB section)

		typedef
		int
		(WINAPI *DTT_CALLBACK_PROC)
		(
			HDC hdc,
			LPWSTR pszText,
			int cchText,
			LPRECT prc,
			UINT dwFlags,
			LPARAM lParam);

		typedef struct _DTTOPTS
		{
			DWORD             dwSize;              // size of the struct
			DWORD             dwFlags;             // which options have been specified
			COLORREF          crText;              // color to use for text fill
			COLORREF          crBorder;            // color to use for text outline
			COLORREF          crShadow;            // color to use for text shadow
			int               iTextShadowType;     // TST_SINGLE or TST_CONTINUOUS
			POINT             ptShadowOffset;      // where shadow is drawn (relative to text)
			int               iBorderSize;         // Border radius around text
			int               iFontPropId;         // Font property to use for the text instead of TMT_FONT
			int               iColorPropId;        // Color property to use for the text instead of TMT_TEXTCOLOR
			int               iStateId;            // Alternate state id
			BOOL              fApplyOverlay;       // Overlay text on top of any text effect?
			int               iGlowSize;           // Glow radious around text
			DTT_CALLBACK_PROC pfnDrawTextCallback; // Callback for DrawText
			LPARAM            lParam;              // Parameter for callback
		} DTTOPTS, *PDTTOPTS;

		#define WTNCA_NODRAWCAPTION       0x00000001    // don't draw the window caption
		#define WTNCA_NODRAWICON          0x00000002    // don't draw the system icon
		#define WTNCA_NOSYSMENU           0x00000004    // don't expose the system menu icon functionality
		#define WTNCA_NOMIRRORHELP        0x00000008    // don't mirror the question mark, even in RTL layout

		enum WINDOWTHEMEATTRIBUTETYPE
		{
			WTA_NONCLIENT = 1
		};

		typedef struct _WTA_OPTIONS
		{
			DWORD dwFlags;          // values for each style option specified in the bitmask
			DWORD dwMask;           // bitmask for flags that are changing
											// valid options are: WTNCA_NODRAWCAPTION, WTNCA_NODRAWICON, WTNCA_NOSYSMENU
		} WTA_OPTIONS, *PWTA_OPTIONS;
	#endif
#endif

#if defined (__GNUC__)
	#define SECURITY_ENTRYPOINTA "InitSecurityInterfaceA"
	#define SECURITY_ENTRYPOINT SECURITY_ENTRYPOINTA
	#define FreeCredentialsHandle FreeCredentialsHandle
	#define FAPPCOMMAND_MASK				0xF000
	#define GET_APPCOMMAND_LPARAM(lParam)	((short)(HIWORD(lParam) & ~FAPPCOMMAND_MASK))
	#ifdef __cplusplus
	extern "C" {
	#endif
		WINGDIAPI BOOL WINAPI AlphaBlend(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);
	#ifdef __cplusplus
	}
	#endif
	/* FIXME: MinGW doesn't provide _snscanf, we wrap unsafe sscanf here */
	#define _snscanf(buf, size, fmt, ...) sscanf(buf, fmt, ## __VA_ARGS__)
	#ifndef CDSIZEOF_STRUCT
		#define CDSIZEOF_STRUCT(structname, member) \
			(((int)((LPBYTE)(&((structname*)0)->member) - ((LPBYTE)((structname*)0)))) + sizeof(((structname*)0)->member))
	#endif
	#ifndef OPENFILENAME_SIZE_VERSION_400
		#define OPENFILENAME_SIZE_VERSION_400	CDSIZEOF_STRUCT(OPENFILENAME, lpTemplateName)
	#endif
	#ifndef NOTIFYICONDATAA_V1_SIZE
		#define NOTIFYICONDATAA_V1_SIZE			CDSIZEOF_STRUCT(NOTIFYICONDATAA, szTip[64])
	#endif
	#ifndef NOTIFYICONDATA_V1_SIZE
		#define NOTIFYICONDATA_V1_SIZE			CDSIZEOF_STRUCT(NOTIFYICONDATA, szTip[64])
	#endif
	#ifndef OPENFILENAMEW_SIZE_VERSION_400
		#define OPENFILENAMEW_SIZE_VERSION_400	CDSIZEOF_STRUCT(OPENFILENAMEW, lpTemplateName)
	#endif
	#ifndef NOTIFYICONDATAW_V1_SIZE
		#define NOTIFYICONDATAW_V1_SIZE			CDSIZEOF_STRUCT(NOTIFYICONDATAW, szTip[64])
	#endif
	#ifndef TV_KEYDOWN
	typedef struct tagNMKEY {
		NMHDR hdr;
		UINT nVKey;
		UINT uFlags;
	} NMKEY, *LPNMKEY;
	typedef struct tagTVKEYDOWN
	{
		NMHDR hdr;
		WORD wVKey;
		UINT flags;
	} NMTVKEYDOWN, *LPNMTVKEYDOWN;
	#define TV_KEYDOWN			NMTVKEYDOWN
	#endif
	#define ODS_HOTLIGHT        0x0040
	#define ODS_INACTIVE        0x0080
	#define OLERENDER_FORMAT	2
	#define SES_EXTENDBACKCOLOR	4
	#define SPI_GETFLATMENU		0x1022
	#define COLOR_HOTLIGHT		26
	#define COLOR_MENUBAR		30
	#define COLOR_MENUHILIGHT   29
	#define COLOR_HIGHLIGHT		13
	#define BP_PUSHBUTTON		1  // Push Button Type
	#define PBS_NORMAL			1
	#define PBS_HOT				2
	#define PBS_PRESSED			3
	#define PBS_DISABLED		4
	#define PBS_DEFAULTED		5
	#define BP_CHECKBOX			3  // CheckBox Type
	#define TP_BUTTON           1
	#define TS_NORMAL           1
	#define TS_HOT              2
	#define TS_PRESSED          3
	#define TS_DISABLED         4
	#define TS_CHECKED          5
	#define TS_HOTCHECKED       6
	#ifndef TTM_SETTITLE
	#ifndef UNICODE
		#define TTM_SETTITLE TTM_SETTITLEA
	#else
		#define TTM_SETTITLE TTM_SETTITLEW
	#endif
	#endif
	#define CBS_UNCHECKEDNORMAL 1
	#define CBS_UNCHECKEDHOT    2
	#define CBS_CHECKEDNORMAL   5
	#define CBS_CHECKEDHOT      6
	#define	CFM_WEIGHT			0x00400000
	#define	CFM_UNDERLINETYPE	0x00800000
	#define CFM_BACKCOLOR		0x04000000
	#define CFU_UNDERLINE		1
	#define CFU_UNDERLINEWORD	2
	#define EP_EDITTEXT			1 // Edit
	#define EP_CARET			2
	#define ETS_NORMAL			1
	#define ETS_HOT				2
	#define ETS_SELECTED		3
	#define ETS_DISABLED		4
	#define ETS_FOCUSED			5
	#define ETS_READONLY		6
	#define ETS_ASSIST			7
	#ifndef PBT_APMRESUMESUSPEND
		#define PBT_APMSUSPEND		 0x0004
		#define PBT_APMRESUMESUSPEND 0x0007
	#endif
	#define AW_HOR_POSITIVE		0x00000001
	#define AW_VER_NEGATIVE		0x00000008
	#define AW_HIDE				0x00010000
	#define AW_ACTIVATE			0x00020000
	#define AW_SLIDE			0x00040000
	#define AW_BLEND			0x00080000
	#define WM_UNICHAR			0x0109
	#define LVS_EX_DOUBLEBUFFER	0x00010000
	#define RES_ICON			1
	#ifndef DFCS_HOT
		#define DFCS_HOT			0x1000
	#endif
	#define IP_TTL				7
	#ifndef IP_MULTICAST_IF
		#define IP_MULTICAST_IF		32
	#endif
	#define IMF_AUTOKEYBOARD	0x0001
	#define IMF_AUTOFONTSIZEADJUST 0x0010
	#define GRADIENT_FILL_RECT_H 0x00
	#define GRADIENT_FILL_RECT_V 0x01
	#define LANG_INVARIANT		0x7f
	#define LOCALE_INVARIANT	(MAKELCID(MAKELANGID(LANG_INVARIANT, SUBLANG_NEUTRAL), SORT_DEFAULT))
	#define EN_ALIGN_RTL_EC		0x0701
	#ifndef OBJID_MENU
		#define OBJID_MENU			((LONG)0xFFFFFFFD)
	#endif
	#ifndef OBJID_VSCROLL
		#define OBJID_VSCROLL		((LONG)0xFFFFFFFB)
	#endif
	#define TreeView_SetCheckState(hwndTV, hti, fCheck) \
		TreeView_SetItemState(hwndTV, hti, INDEXTOSTATEIMAGEMASK((fCheck)?2:1), TVIS_STATEIMAGEMASK)
	#define TreeView_GetCheckState(hwndTV, hti) \
		((((UINT)(SNDMSG((hwndTV), TVM_GETITEMSTATE, (WPARAM)(hti), TVIS_STATEIMAGEMASK))) >> 12) -1)
	#define ERROR_INTERNET_SEC_CERT_NO_REV 12056
	#define ERROR_INTERNET_SEC_CERT_REV_FAILED 12057
	#define APPCOMMAND_BROWSER_BACKWARD 1
	#define APPCOMMAND_BROWSER_FORWARD 2
	#define NIN_BALLOONHIDE		(WM_USER +3)
	#define NIN_BALLOONTIMEOUT	(WM_USER + 4)
	#define NIN_BALLOONUSERCLICK (WM_USER + 5)
// SDK isn't present or some older VC compiler was used, include missing things.
#elif !defined(NOWIN2K) && (!defined WS_EX_LAYERED || !defined IDC_HAND)

	#pragma message("win2k.h")

	#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)

	#define PSDK_WORKAROUND

	#define MONITOR_DEFAULTTONEAREST 2

	#ifndef EM_SETTEXTEX
		#define EM_SETTEXTEX	(WM_USER + 97)
		#define ST_DEFAULT		0
		#define ST_KEEPUNDO		1
		#define ST_SELECTION	2
		#define ST_NEWCHARS		4
		typedef struct _settextex
		{
			DWORD	flags;
			UINT	codepage;
		} SETTEXTEX;
	#endif

	#if(_WIN32_WINNT >= 0x0500)
		#define WS_EX_LAYERED		0x00080000
		#define MIIM_STRING			0x00000040
		#define MIIM_BITMAP			0x00000080
		#define MIIM_FTYPE			0x00000100
		#define HBMMENU_CALLBACK            ((HBITMAP) -1)
		#define ODS_HOTLIGHT        0x0040
		#define ODS_INACTIVE        0x0080
		#define IDC_HAND            MAKEINTRESOURCE(32649)
		#define COLOR_HOTLIGHT		26
		#define COLOR_MENUBAR		30
		#define COLOR_MENUHILIGHT   29
		#define COLOR_HIGHLIGHT		13
		#define SPI_GETFLATMENU		0x1022
		#define TVS_NOHSCROLL       0x8000
		#define SPI_GETLISTBOXSMOOTHSCROLLING       0x1006
		#define SPI_GETHOTTRACKING                  0x100E
		#define BIF_NEWDIALOGSTYLE	0x0040
		#define LVS_EX_LABELTIP     0x00004000
		#define LVS_EX_DOUBLEBUFFER	0x00010000
		#define DFCS_HOT 0x1000
		#define FLASHW_TRAY 0x00000002;
		typedef struct {
			UINT cbSize;
			HWND hwnd;
			DWORD dwFlags;
			UINT uCount;
			DWORD dwTimeout;
		} FLASHWINFO;
		/* for the help plugin without the SDK */
		#define SM_XVIRTUALSCREEN 76
		#define SM_YVIRTUALSCREEN 77
		#define SM_CXVIRTUALSCREEN 78
		#define SM_CYVIRTUALSCREEN 79
		#define COLOR_HOTLIGHT 26
		#define VK_OEM_PLUS		0xBB
		#define VK_OEM_MINUS	0xBD

		/* the structure only needs to be defined for VC5 or lower */
		#if _MSC_VER < 1200
			typedef struct tagLASTINPUTINFO {
			UINT cbSize;
			DWORD dwTime;
			} LASTINPUTINFO, *PLASTINPUTINFO;
		#endif /* #if _MSC_VER < 1200 */

		#ifndef OPENFILENAME_SIZE_VERSION_400
			#define OPENFILENAME_SIZE_VERSION_400 sizeof(OPENFILENAME)
		#endif

		#ifndef CCM_SETVERSION
			#define CCM_SETVERSION          (CCM_FIRST + 0x7)
		#endif

		#define SYSRGN 4
		WINGDIAPI int WINAPI GetRandomRgn(IN HDC, IN HRGN, IN INT);

	#endif /* _WIN32_WINNT >= 0x0500 */

	#define LWA_ALPHA               0x00000002
	#define AW_HIDE                 0x00010000
	#define AW_BLEND                0x00080000
	#define SPI_GETSCREENSAVERRUNNING 114
	#define SM_CMONITORS            80

	#ifndef AW_ACTIVATE
		#define AW_ACTIVATE 0x00020000
		#define AW_SLIDE 0x00040000
		#define AW_VER_NEGATIVE 0x00000008
		#define AW_HOR_POSITIVE 0x00000001
	#endif

	#ifndef DWORD_PTR
		typedef DWORD DWORD_PTR;
	#endif

	#ifndef HMONITOR
		DECLARE_HANDLE(HMONITOR);
		typedef struct tagMONITORINFO
		{
			DWORD   cbSize;
			RECT    rcMonitor;
			RECT    rcWork;
			DWORD   dwFlags;
		} MONITORINFO, *LPMONITORINFO;
	#endif


	#ifndef IDropTargetHelper
	DEFINE_GUID(CLSID_DragDropHelper,   0x4657278a, 0x411b, 0x11d2, 0x83, 0x9a, 0x0, 0xc0, 0x4f, 0xd9, 0x18, 0xd0);
	#define INTERFACE IDropTargetHelper
		DECLARE_INTERFACE_(IDropTargetHelper, IUnknown)
		{
			// IUnknown methods
			STDMETHOD (QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
			STDMETHOD_(ULONG, AddRef) (THIS) PURE;
			STDMETHOD_(ULONG, Release) (THIS) PURE;

		    // IDropTargetHelper
			STDMETHOD (DragEnter)(THIS_ HWND hwndTarget, IDataObject* pDataObject, POINT* ppt, DWORD dwEffect) PURE;
			STDMETHOD (DragLeave)(THIS) PURE;
			STDMETHOD (DragOver)(THIS_ POINT* ppt, DWORD dwEffect) PURE;
			STDMETHOD (Drop)(THIS_ IDataObject* pDataObject, POINT* ppt, DWORD dwEffect) PURE;
			STDMETHOD (Show)(THIS_ BOOL fShow) PURE;

		};
	#endif /* IDropTargetHelper */

	#define WM_MENURBUTTONUP                0x0122

// tabsrmm uses these

#define SES_EXTENDBACKCOLOR	4
#define EM_SETEDITSTYLE (WM_USER + 204)
#define EM_SETSCROLLPOS (WM_USER + 222)
#define SF_USECODEPAGE 0x00000020

#define TreeView_SetItemState(hwndTV, hti, data, _mask) \
{ TVITEM _TVi; \
  _TVi.mask = TVIF_STATE; \
  _TVi.hItem = hti; \
  _TVi.stateMask = _mask; \
  _TVi.state = data; \
  SendMessage((hwndTV), TVM_SETITEM, 0, (LPARAM)(TV_ITEM *)&_TVi); \
}

#endif /* SDK check */

#ifndef BPPF_ERASE
	typedef enum _BP_BUFFERFORMAT
	{
		BPBF_COMPATIBLEBITMAP,    // Compatible bitmap
		BPBF_DIB,                 // Device-independent bitmap
		BPBF_TOPDOWNDIB,          // Top-down device-independent bitmap
		BPBF_TOPDOWNMONODIB       // Top-down monochrome device-independent bitmap
	} BP_BUFFERFORMAT;


	typedef struct _BP_PAINTPARAMS
	{
		DWORD                       cbSize;
		DWORD                       dwFlags; // BPPF_ flags
		const RECT *                prcExclude;
		const BLENDFUNCTION *       pBlendFunction;
	} BP_PAINTPARAMS, *PBP_PAINTPARAMS;

	#define BPPF_ERASE               1
	#define BPPF_NOCLIP              2
	#define BPPF_NONCLIENT           4
#endif


		/* windows seven taskbar interface comes with sdk v7.0
		   if not existent define on our own */
#ifdef _SHLOBJ_H_
	#ifndef __ITaskbarList3_INTERFACE_DEFINED__
	#define __ITaskbarList3_INTERFACE_DEFINED__
		/* interface ITaskbarList3 */

		typedef
			enum TBPFLAG
		{
			TBPF_NOPROGRESS	 = 0,
			TBPF_INDETERMINATE	 = 0x1,
			TBPF_NORMAL	 = 0x2,
			TBPF_ERROR	 = 0x4,
			TBPF_PAUSED	 = 0x8
		} TBPFLAG;

		typedef struct THUMBBUTTON *LPTHUMBBUTTON;

		static const GUID IID_ITaskbarList3 = { 0xea1afb91, 0x9e28, 0x4b86, { 0x90, 0xE9, 0x9e, 0x9f, 0x8a, 0x5e, 0xef, 0xaf } };

	#ifdef INTERFACE
	#undef INTERFACE
	#endif
	#define INTERFACE ITaskbarList3
		DECLARE_INTERFACE_(ITaskbarList3, ITaskbarList2)
		{
			// IUnknown methods
			STDMETHOD (QueryInterface)       (THIS_ REFIID riid, void **ppv) PURE;
			STDMETHOD_(ULONG, AddRef)        (THIS) PURE;
			STDMETHOD_(ULONG, Release)       (THIS) PURE;

			// overloaded ITaskbarList2 members
			STDMETHOD (HrInit)               (THIS) PURE;
			STDMETHOD (AddTab)               (THIS_ HWND hwnd) PURE;
			STDMETHOD (DeleteTab)            (THIS_ HWND hwnd) PURE;
			STDMETHOD (ActivateTab)          (THIS_ HWND hwnd) PURE;
			STDMETHOD (SetActiveAlt)         (THIS_ HWND hwnd) PURE;
			STDMETHOD (MarkFullscreenWindow) (THIS_ HWND hwnd, int fFullscreen) PURE;

			// ITaskbarList3 members
			STDMETHOD (SetProgressValue)     (THIS_ HWND hwnd, ULONGLONG ullCompleted, ULONGLONG ullTotal) PURE;
			STDMETHOD (SetProgressState)     (THIS_ HWND hwnd, TBPFLAG tbpFlags) PURE;
			STDMETHOD (RegisterTab)          (THIS_ HWND hwndTab, HWND hwndMDI) PURE;
			STDMETHOD (UnregisterTab)        (THIS_ HWND hwndTab) PURE;
			STDMETHOD (SetTabOrder)          (THIS_ HWND hwndTab, HWND hwndInsertBefore) PURE;
			STDMETHOD (SetTabActive)         (THIS_ HWND hwndTab, HWND hwndMDI, DWORD dwReserved) PURE;
			STDMETHOD (ThumbBarAddButtons)   (THIS_ HWND hwnd, UINT cButtons, LPTHUMBBUTTON pButton) PURE;
			STDMETHOD (ThumbBarUpdateButtons)(THIS_ HWND hwnd, UINT cButtons, LPTHUMBBUTTON pButton) PURE;
			STDMETHOD (ThumbBarSetImageList) (THIS_ HWND hwnd, HIMAGELIST himl) PURE;
			STDMETHOD (SetOverlayIcon)       (THIS_ HWND hwnd, HICON hIcon, LPCWSTR pszDescription) PURE;
			STDMETHOD (SetThumbnailTooltip)  (THIS_ HWND hwnd, LPCWSTR pszTip) PURE;
			STDMETHOD (SetThumbnailClip)     (THIS_ HWND hwnd, RECT *prcClip) PURE;
		};

	#endif 	/* __ITaskbarList3_INTERFACE_DEFINED__ */
#endif  /* _SHLOBJ_H_ */

#ifndef FILETYPEATTRIBUTEFLAGS
	#define FTA_Exclude               0x00000001
	#define FTA_Show                  0x00000002
	#define FTA_HasExtension          0x00000004
	#define FTA_NoEdit                0x00000008
	#define FTA_NoRemove              0x00000010
	#define FTA_NoNewVerb             0x00000020
	#define FTA_NoEditVerb            0x00000040
	#define FTA_NoRemoveVerb          0x00000080
	#define FTA_NoEditDesc            0x00000100
	#define FTA_NoEditIcon            0x00000200
	#define FTA_NoEditDflt            0x00000400
	#define FTA_NoEditVerbCmd         0x00000800
	#define FTA_NoEditVerbExe         0x00001000
	#define FTA_NoDDE                 0x00002000
	#define FTA_NoEditMIME            0x00008000
	#define FTA_OpenIsSafe            0x00010000
	#define FTA_AlwaysUnsafe          0x00020000
	#define FTA_AlwaysShowExt         0x00040000
	#define FTA_NoRecentDocs          0x00100000
	#define FTA_SafeForElevation      0x00200000
	#define FTA_AlwaysUseDirectInvoke 0x00400000
#endif  /* FTA_Exclude */

#endif // WIN2K_H__
