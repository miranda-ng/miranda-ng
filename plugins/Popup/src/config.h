/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
© 2004-2007 Victor Pavlychko
© 2010 MPK
© 2010 Merlin_de

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

#ifndef __config_h__
#define __config_h__

struct POPUPOPTIONS
{
	// ==Page General====
	// Timeout
	bool bInfiniteDelay;
	int  Seconds;
	bool bLeaveHovered;

	// Size & Position
	bool bDynamicResize;
	bool bUseMinimumWidth;
	int  MinimumWidth;
	bool bUseMaximumWidth;
	int  MaximumWidth;
	int  Position;
	int  Spreading;

	// position Popup
	int gapTop;
	int gapBottom;
	int gapLeft;
	int gapRight;
	int spacing;
	
	// Miscellaneous
	bool bReorderPopups;
	bool bReorderPopupsWarning;
	
	// Disable when
	bool bDisableWhenFullscreen;
	bool bDisableWhenIdle;

	// others
	uint8_t bDebug;
	
	//==Page Skins====
	LPTSTR SkinPack;
	bool bDisplayTime;
	bool bDropShadow;
	bool bEnableFreeformShadows;
	bool bEnableAeroGlass;
	bool bUseWinColors;
	bool bUseMText;
	
	//==Page Actions====
	uint32_t actions;
	
	// Mouse Override
	int overrideLeft;
	int overrideRight;
	int overrideMiddle;
	
	//==Page Advanced====
	// History
	bool bEnableHistory;
	int  HistorySize;
	bool bUseHppHistoryLog;
	
	// Avatars
	bool bAvatarBorders;
	bool bAvatarPNGBorders;
	int  avatarRadius;
	int  avatarSize;
	bool bEnableAvatarUpdates;
	
	// Monitor
	uint8_t Monitor;
	
	// Transparency
	bool bUseTransparency;
	int  Alpha;
	bool bOpaqueOnHover;
	
	// Effect
	bool bUseAnimations;
	bool bUseEffect;
	LPTSTR Effect;
	uint32_t FadeIn;
	uint32_t FadeOut;
	
	// Others
	int  MaxPopups;
};

//===== User wnd class =====
struct GLOBAL_WND_CLASSES{
	ATOM	cPopupWnd2;
	ATOM	cPopupEditBox;
	ATOM	cPopupMenuHostWnd;
	ATOM	cPopupThreadManagerWnd;
	ATOM	cPopupPreviewBoxWndclass;
	ATOM	cPopupPlusDlgBox;
};
extern GLOBAL_WND_CLASSES g_wndClass;

#define ACT_ENABLE			0x00000001
#define ACT_LARGE			0x00000002
#define ACT_TEXT			0x00000004
#define ACT_RIGHTICONS		0x00000008
#define ACT_LEFTICONS		0x00000010
#define ACT_DEF_NOGLOBAL	0x00000020
#define ACT_DEF_KEEPWND		0x00000040
#define ACT_DEF_IMONLY		0x00000080

#define ACT_DEF_MESSAGE		0x00010000
#define ACT_DEF_DETAILS		0x00020000
#define ACT_DEF_MENU		0x00040000
#define ACT_DEF_ADD			0x00080000
#define ACT_DEF_DISMISS		0x00100000
#define ACT_DEF_PIN			0x00200000
#define ACT_DEF_REPLY		0x00400000
#define ACT_DEF_COPY		0x00800000

void LoadOptions();

//===== General Plugin =====
extern HANDLE hEventNotify;
extern HANDLE hSemaphore;
extern BOOL closing;
extern HANDLE folderId;

extern HANDLE htuText;
extern HANDLE htuTitle;

extern BOOL gbPopupLoaded;
extern BOOL gbHppInstalled;

//===== Brushes, Colours and Fonts =====
extern HBITMAP hbmNoAvatar;

//===== Options =====
extern POPUPOPTIONS PopupOptions;
extern bool OptionLoaded;

//===== Transparency & APIs which are not supported by every OS =====
#ifndef DWLP_MSGRESULT
#define DWLP_MSGRESULT 0
#endif
#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED 0x00080000
#endif
#ifndef LWA_COLORKEY
#define LWA_COLORKEY 0x00000001
#endif
#ifndef LWA_ALPHA
#define LWA_ALPHA 0x00000002
#endif
#ifndef ULW_OPAQUE
#define ULW_OPAQUE 0x00000004
#endif
#ifndef AC_SRC_ALPHA
#define AC_SRC_ALPHA 0x01
#endif
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES (uint32_t (-1))
#endif

#define DWM_BB_ENABLE					0x00000001
#define DWM_BB_BLURREGION				0x00000002
#define DWM_BB_TRANSITIONONMAXIMIZED	0x00000004
struct DWM_BLURBEHIND
{
	uint32_t dwFlags;
	BOOL fEnable;
	HRGN hRgnBlur;
	BOOL fTransitionOnMaximized;
};

extern HRESULT(WINAPI *MyDwmEnableBlurBehindWindow)(HWND hWnd, DWM_BLURBEHIND *pBlurBehind);

typedef struct TestStruct{
	int cbSize;
} TESTSIZE;

// Generic Message Box for Errors
#define MSGERROR(text) MessageBox(NULL, text, MODULNAME_LONG, MB_OK | MB_ICONERROR)
#define MSGINFO	(text) MessageBox(NULL, text, MODULNAME_LONG, MB_OK | MB_ICONINFORMATION)

#endif // __config_h__
