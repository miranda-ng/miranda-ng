/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks
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
 * $Id: config.h 134 2010-10-01 10:23:10Z silvercircle $
 *
 */

#include <stdexcept>

#ifndef __CONFIG_H_
#define __CONFIG_H_

#define	DEFAULT_MODULE "Clist"
#define SKIN_REQUIRED_VERSION 5

#define IS_THEMED (Api::sysState.isThemed)
#define IS_AERO (Api::sysState.isAero)

typedef BOOL (WINAPI *PGF)(HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);

typedef  int	(__cdecl *pfnDrawAvatar)(HDC hdcOrig, HDC hdcMem, RECT *rc, struct ClcContact *contact, int y, struct ClcData *dat, int selected, WORD cstatus, int rowHeight);
typedef BOOL	(WINAPI *pfnSetLayeredWindowAttributes_t)(HWND, COLORREF, BYTE, DWORD);
typedef BOOL	(WINAPI *pfnUpdateLayeredWindow_t)(HWND, HDC, POINT *, SIZE *, HDC, POINT *, COLORREF, BLENDFUNCTION *, DWORD);
typedef HMONITOR(WINAPI *pfnMonitorFromWindow_t)(HWND, DWORD);
typedef BOOL	(WINAPI *pfnGetMonitorInfo_t)(HMONITOR, LPMONITORINFO);
typedef BOOL	(WINAPI *pfnTrackMouseEvent_t)(LPTRACKMOUSEEVENT);
typedef DWORD	(WINAPI *pfnSetLayout_t)(HDC, DWORD);
typedef void	(__cdecl *pfnDrawAlpha_t)(HDC hdcwnd, PRECT rc, DWORD basecolor, BYTE alpha, DWORD basecolor2, BOOL transparent, DWORD FLG_GRADIENT, DWORD FLG_CORNER, DWORD BORDERSTYLE, TImageItem *item);
typedef BOOL	(WINAPI *pfnAlphaBlend_t)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);

typedef BOOL	(WINAPI *pfnIsThemeActive_t)();
typedef HANDLE	(WINAPI *pfnOpenThemeData_t)(HWND, LPCWSTR);
typedef UINT	(WINAPI *pfnDrawThemeBackground_t)(HANDLE, HDC, int, int, RECT *, RECT *);
typedef UINT	(WINAPI *pfnCloseThemeData_t)(HANDLE);
typedef UINT	(WINAPI *pfnDrawThemeText_t)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, DWORD, RECT *);
typedef UINT	(WINAPI *pfnDrawThemeTextEx_t)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, RECT *, const DTTOPTS *);
typedef BOOL	(WINAPI *pfnIsThemeBackgroundPartiallyTransparent_t)(HANDLE, int, int);
typedef HRESULT	(WINAPI *pfnDrawThemeParentBackground_t)(HWND, HDC, RECT *);
typedef HRESULT	(WINAPI *pfnGetThemeBackgroundContentRect_t)(HANDLE, HDC, int, int, const RECT *, const RECT *);
typedef BOOL	(WINAPI *pfnEnableThemeDialogTexture_t)(HANDLE, DWORD);
typedef HRESULT	(WINAPI *pfnDwmExtendFrameIntoClientArea_t)(HWND hwnd, const MARGINS *margins);
typedef HRESULT	(WINAPI *pfnDwmIsCompositionEnabled_t)(BOOL *);

typedef HRESULT	(WINAPI *pfnBufferedPaintInit_t)(void);
typedef HRESULT	(WINAPI *pfnBufferedPaintUninit_t)(void);
typedef HANDLE	(WINAPI *pfnBeginBufferedPaint_t)(HDC, RECT *, BP_BUFFERFORMAT, BP_PAINTPARAMS *, HDC *);
typedef HRESULT	(WINAPI *pfnEndBufferedPaint_t)(HANDLE, BOOL);
typedef HRESULT	(WINAPI *pfnBufferedPaintSetAlpha_t)(HANDLE, const RECT *, BYTE);
typedef HRESULT	(WINAPI *pfnBufferedPaintClear_t)(HANDLE, const RECT *);
typedef HRESULT	(WINAPI *pfnDwmBlurBehindWindow_t)(HWND, DWM_BLURBEHIND *);
typedef HRESULT	(WINAPI *pfnDwmGetColorizationColor_t)(DWORD *, BOOL *);
typedef HRESULT	(WINAPI *pfnGetBufferedPaintBits_t)(HANDLE, RGBQUAD **, int *);
typedef ULONGLONG(WINAPI *pfnGetTickCount64_t)(void);

struct ContactFloater;

struct TExtraCache {
	BYTE 		iExtraImage[20];
	MCONTACT	hContact;
	HANDLE		hTimeZone;
	DWORD 		iExtraValid;
	BYTE 		valid;
	wchar_t*	statusMsg;
	BYTE 		bStatusMsgValid;
	DWORD 		dwCFlags;
	DWORD 		dwXMask;      // local extra icon mask, calculated from CLN_xmask
	TStatusItem *status_item, *proto_status_item;
	ContactFloater *floater;	// associated floating contact window
	DWORD 		dwLastMsgTime;	// last message received (unix timestamp)
	DWORD 		msgFrequency;	// "message rate" - for sorting by activity
	BOOL  		isChatRoom;		// a chat room
};

class CRTException : public std::runtime_error
{
public:
	CRTException(const char *szMsg, const wchar_t *szParam);
	~CRTException() {}

	void display() const;

private:
	wchar_t	m_szParam[MAX_PATH];
};

struct TSkinDescription {
	ULONG	ulID;				// resource id
	wchar_t	tszName[30];		// file name
};

class cfg
{
public:
	static void			initCache();
	static int 			onInit();			// early stage inite (read base path, extract resources)

	static DWORD 		getDword(const MCONTACT hContact, const char *szModule, const char *szSetting, DWORD uDefault);
	static DWORD  		getDword(const char *szModule, const char *szSetting, DWORD uDefault);
	static DWORD 		getDword(const char *szSetting, DWORD uDefault);

	static WORD 		getWord(const MCONTACT hContact, const char *szModule, const char *szSetting, WORD uDefault);
	static WORD  		getWord(const char *szModule, const char *szSetting, WORD uDefault);
	static WORD 		getWord(const char *szSetting, WORD uDefault);

	static int 			getByte(const MCONTACT hContact, const char *szModule, const char *szSetting, int uDefault);
	static int 	 		getByte(const char *szModule, const char *szSetting, int uDefault);
	static int			getByte(const char *szSetting, int uDefault);

	static INT_PTR 		getTString(const MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv);
	static INT_PTR 		getString(const MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv);

	static INT_PTR		writeDword(const MCONTACT hContact, const char *szModule, const char *szSetting, DWORD value);
	static INT_PTR		writeDword(const char *szModule, const char *szSetting, DWORD value);

	static INT_PTR		writeWord(const MCONTACT hContact, const char *szModule, const char *szSetting, WORD value);
	static INT_PTR		writeWord(const char *szModule, const char *szSetting, WORD value);

	static INT_PTR		writeByte(const MCONTACT hContact, const char *szModule, const char *szSetting, BYTE value);
	static INT_PTR		writeByte(const char *szModule, const char *szSetting, BYTE value);

	static INT_PTR		writeTString(const MCONTACT hContact, const char *szModule, const char *szSetting, const wchar_t *st);
	static INT_PTR		writeString(const MCONTACT hContact, const char *szModule, const char *szSetting, const char *st);
	static int 			getCache(const MCONTACT hContact, const char *szProto);
	static void 		FS_RegisterFonts();
	static CLUIFrames	*FrameMgr;

	/*
	 * option dialog procedures
	 */
	static INT_PTR CALLBACK 	DlgProcClcMainOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK 	DlgProcViewModesSetup(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK 	DlgProcFloatingContacts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK 	SkinOptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK 	DlgProcCluiOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK 	DlgProcSBarOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK 	DlgProcGenOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK 	TabOptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK 	DlgProcDspGroups(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK 	DlgProcDspItems(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK 	DlgProcDspAdvanced(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK 	DlgProcDspClasses(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK 	DlgProcXIcons(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK 	DlgProcDspProfiles(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK 	DlgProcSkinOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


	static void 				ReloadExtraIcons();
	static void 				extractBaseSkin(bool fForceOverwrite);

public:
	static TCluiData	dat;
	static ClcData*		clcdat;
	static TExtraCache* eCache;
	static int			nextCacheEntry, maxCacheEntry;
	static int			maxStatus;

	static CRITICAL_SECTION	cachecs;

	static bool			shutDown;
	static bool			isAero;
	static wchar_t		szProfileDir[MAX_PATH];
	static bool			fBaseSkinValid;
	static FI_INTERFACE* fif;

	static TSkinDescription 	cfg::my_default_skin[];
};

struct TSysConfig {
	bool			isVistaPlus;
	bool			isSevenPlus;
};

struct TSysState {
	bool			isThemed;
	bool			isAero;
	bool			isDwmActive;
};

class Api
{
public:
	static int			onInit();
	static void			onUnload();
	static void			updateState();

	static void 			Ex_CopyEditToClipboard(HWND hWnd);
	static INT_PTR CALLBACK Ex_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void 			Ex_Handler();
	static int 				Ex_ShowDialog(EXCEPTION_POINTERS *ep, const char *szFile, int line, wchar_t* szReason, bool fAllowContinue);

	/*
	 * helpers
	 */

	static PGF											pfnGradientFill;
	static pfnAlphaBlend_t								pfnAlphaBlend;
	static pfnGetTickCount64_t							pfnGetTickCount64;
	/*
	 * uxtheme
	 */

	static pfnIsThemeActive_t 							pfnIsThemeActive;
	static pfnOpenThemeData_t 							pfnOpenThemeData;
	static pfnDrawThemeBackground_t 					pfnDrawThemeBackground;
	static pfnCloseThemeData_t			 				pfnCloseThemeData;
	static pfnDrawThemeText_t 							pfnDrawThemeText;
	static pfnDrawThemeTextEx_t							pfnDrawThemeTextEx;
	static pfnIsThemeBackgroundPartiallyTransparent_t 	pfnIsThemeBackgroundPartiallyTransparent;
	static pfnDrawThemeParentBackground_t 	 			pfnDrawThemeParentBackground;
	static pfnGetThemeBackgroundContentRect_t 			pfnGetThemeBackgroundContentRect;
	static pfnEnableThemeDialogTexture_t 				pfnEnableThemeDialogTexture;

	/*
	 * UxTheme buffered paint API (Vista+)
	 */
	static pfnBufferedPaintInit_t						pfnBufferedPaintInit;
	static pfnBufferedPaintUninit_t						pfnBufferedPaintUninit;
	static pfnBeginBufferedPaint_t						pfnBeginBufferedPaint;
	static pfnEndBufferedPaint_t						pfnEndBufferedPaint;
	static pfnBufferedPaintSetAlpha_t					pfnBufferedPaintSetAlpha;
	static pfnBufferedPaintClear_t						pfnBufferedPaintClear;
	static pfnGetBufferedPaintBits_t					pfnGetBufferedPaintBits;

	/*
	 * DWM
	 */
	static pfnDwmBlurBehindWindow_t						pfnDwmBlurBehindWindow;
	static pfnDwmGetColorizationColor_t					pfnDwmGetColorizationColor;;
	static pfnDwmExtendFrameIntoClientArea_t			pfnDwmExtendFrameIntoClientArea;
	static pfnDwmIsCompositionEnabled_t					pfnDwmIsCompositionEnabled;

	static TSysConfig						sysConfig;
	static TSysState						sysState;

	static EXCEPTION_RECORD exRecord;
	static CONTEXT			exCtx;
	static LRESULT			exLastResult;
	static char				exSzFile[MAX_PATH];
	static wchar_t			exReason[256];
	static int				exLine;
	static bool				exAllowContinue;

	static HMODULE			hUxTheme, hDwm;
};


#endif /* __CONFIG_H_*/
