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

typedef struct tagPOPUPOPTIONS {
	//==Page General====
	//Timeout
	BYTE InfiniteDelay;
	int Seconds;
	BYTE LeaveHovered;
	//Size&Position
	BYTE DynamicResize;
	BYTE UseMinimumWidth;
	WORD MinimumWidth;
	BYTE UseMaximumWidth;
	WORD MaximumWidth;
	int Position;
	int Spreading;
	//position Popup
	int gapTop;
	int gapBottom;
	int gapLeft;
	int gapRight;
	int spacing;
	//Miscellaneous
	BYTE ReorderPopups;
	BYTE ReorderPopupsWarning;
	//Disable when
	BOOL ModuleIsEnabled;
	BYTE DisableWhenFullscreen;
	//others
	BYTE debug;
	//==Page Skins====
	LPTSTR SkinPack;
	BYTE DisplayTime;
	BYTE DropShadow;
	BYTE EnableFreeformShadows;
	BYTE EnableAeroGlass;
	BYTE UseWinColors;
	BYTE UseMText;
	//==Page Actions====
	DWORD actions;
	//Mouse Override
	int overrideLeft;
	int overrideRight;
	int overrideMiddle;
	//==Page Advanced====
	//History
	BYTE EnableHistory;
	WORD HistorySize;
	BYTE UseHppHistoryLog;
	//Avatars
	BYTE avatarBorders;
	BYTE avatarPNGBorders;
	BYTE avatarRadius;
	WORD avatarSize;
	BYTE EnableAvatarUpdates;
	//Monitor
	BYTE Monitor;
	//Transparency
	BYTE UseTransparency;
	BYTE Alpha;
	BYTE OpaqueOnHover;
	//Effect
	BYTE UseAnimations;
	BYTE UseEffect;
	LPTSTR Effect;
	DWORD FadeIn;
	DWORD FadeOut;
	//Others
	WORD MaxPopups;
} POPUPOPTIONS;

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
extern HINSTANCE hInst;
extern HANDLE hMainThread;
extern HANDLE hEventNotify;
extern HANDLE hSemaphore;
extern BOOL closing;
extern HANDLE folderId;

extern MTEXT_INTERFACE MText;
extern HANDLE htuText;
extern HANDLE htuTitle;

extern BOOL gbPopupLoaded;
extern BOOL gbHppInstalled;

//===== Brushes, Colours and Fonts =====
extern HBITMAP hbmNoAvatar;

//===== Options =====
extern POPUPOPTIONS PopupOptions;
extern bool OptionLoaded;
//extern SKINELEMENT *skin;
//extern SKINELEMENT *w_skin;
//extern SKINELEMENT *n_skin;

//===== Plugin informations struct =====
extern PLUGININFOEX pluginInfoEx;

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
#define INVALID_FILE_ATTRIBUTES (DWORD (-1))
#endif

#define DWM_BB_ENABLE					0x00000001
#define DWM_BB_BLURREGION				0x00000002
#define DWM_BB_TRANSITIONONMAXIMIZED	0x00000004
struct DWM_BLURBEHIND
{
	DWORD dwFlags;
	BOOL fEnable;
	HRGN hRgnBlur;
	BOOL fTransitionOnMaximized;
};

extern HRESULT(WINAPI *MyDwmEnableBlurBehindWindow)(HWND hWnd, DWM_BLURBEHIND *pBlurBehind);

typedef struct TestStruct{
	int cbSize;
} TESTSIZE;

// Generic Message Box for Errors
#define MSGERROR(text) MessageBox(NULL, text, _T(MODULNAME_LONG), MB_OK | MB_ICONERROR)
#define MSGINFO	(text) MessageBox(NULL, text, _T(MODULNAME_LONG), MB_OK | MB_ICONINFORMATION)

#endif // __config_h__
