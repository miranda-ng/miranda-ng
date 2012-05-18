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
 * $Id: config.h 12709 2010-09-16 12:06:06Z borkra $
 *
 */

#include <stdexcept>

#ifndef __CONFIG_H_
#define __CONFIG_H_

#define	DEFAULT_MODULE "Clist"

#define IS_THEMED (API::sysState.isThemed)
#define IS_AERO (API::sysState.isAero)

typedef  int  (__cdecl *pfnDrawAvatar)(HDC hdcOrig, HDC hdcMem, RECT *rc, struct ClcContact *contact, int y, struct ClcData *dat, int selected, WORD cstatus, int rowHeight);
typedef BOOL 		(WINAPI *pfnSetLayeredWindowAttributes_t)(HWND, COLORREF, BYTE, DWORD);
typedef BOOL 		(WINAPI *pfnUpdateLayeredWindow_t)(HWND, HDC, POINT *, SIZE *, HDC, POINT *, COLORREF, BLENDFUNCTION *, DWORD);
typedef HMONITOR 	(WINAPI *pfnMonitorFromPoint_t)(POINT,DWORD);
typedef HMONITOR 	(WINAPI *pfnMonitorFromWindow_t)(HWND,DWORD);
typedef BOOL     	(WINAPI *pfnGetMonitorInfo_t)(HMONITOR,LPMONITORINFO);
typedef BOOL     	(WINAPI *pfnTrackMouseEvent_t)(LPTRACKMOUSEEVENT);
typedef DWORD 		(WINAPI *pfnSetLayout_t )(HDC, DWORD);
typedef void 		(__cdecl *pfnDrawAlpha_t)(HDC hdcwnd, PRECT rc, DWORD basecolor, BYTE alpha, DWORD basecolor2, BOOL transparent, DWORD FLG_GRADIENT, DWORD FLG_CORNER, DWORD BORDERSTYLE, ImageItem *item);
typedef BOOL 		(WINAPI *pfnAlphaBlend_t)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);

typedef BOOL 		(WINAPI *pfnIsThemeActive_t)();
typedef HANDLE		(WINAPI *pfnOpenThemeData_t)(HWND, LPCWSTR);
typedef UINT		(WINAPI *pfnDrawThemeBackground_t)(HANDLE, HDC, int, int, RECT *, RECT *);
typedef UINT		(WINAPI *pfnCloseThemeData_t)(HANDLE);
typedef UINT		(WINAPI *pfnDrawThemeText_t)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, DWORD, RECT *);
typedef UINT		(WINAPI *pfnDrawThemeTextEx_t)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, RECT *, const DTTOPTS *);
typedef BOOL 		(WINAPI *pfnIsThemeBackgroundPartiallyTransparent_t)(HANDLE, int, int);
typedef HRESULT		(WINAPI *pfnDrawThemeParentBackground_t)(HWND, HDC, RECT *);
typedef HRESULT		(WINAPI *pfnGetThemeBackgroundContentRect_t)(HANDLE, HDC, int, int, const RECT *, const RECT *);
typedef BOOL		(WINAPI *pfnEnableThemeDialogTexture_t)(HANDLE, DWORD);
typedef HRESULT 	(WINAPI *pfnDwmExtendFrameIntoClientArea_t)(HWND hwnd, const MARGINS *margins);
typedef HRESULT 	(WINAPI *pfnDwmIsCompositionEnabled_t)(BOOL *);

class CRTException : public std::runtime_error
{
public:
	CRTException(const char *szMsg, const TCHAR *szParam);
	~CRTException() {}

	void display() const;

private:
	TCHAR	m_szParam[MAX_PATH];
};

class cfg
{
public:
	static void			init							();

	static DWORD 		getDword						(const HANDLE hContact, const char *szModule, const char *szSetting, DWORD uDefault);
	static DWORD  		getDword						(const char *szModule, const char *szSetting, DWORD uDefault);
	static DWORD 		getDword						(const char *szSetting, DWORD uDefault);

	static WORD 		getWord							(const HANDLE hContact, const char *szModule, const char *szSetting, WORD uDefault);
	static WORD  		getWord							(const char *szModule, const char *szSetting, WORD uDefault);
	static WORD 		getWord							(const char *szSetting, WORD uDefault);

	static int 			getByte							(const HANDLE hContact, const char *szModule, const char *szSetting, int uDefault);
	static int 	 		getByte							(const char *szModule, const char *szSetting, int uDefault);
	static int			getByte							(const char *szSetting, int uDefault);

	static INT_PTR 		getTString						(const HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv);
	static INT_PTR 		getString						(const HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv);

	static INT_PTR		writeDword						(const HANDLE hContact, const char *szModule, const char *szSetting, DWORD value);
	static INT_PTR		writeDword						(const char *szModule, const char *szSetting, DWORD value);

	static INT_PTR		writeWord						(const HANDLE hContact, const char *szModule, const char *szSetting, WORD value);
	static INT_PTR		writeWord						(const char *szModule, const char *szSetting, WORD value);

	static INT_PTR		writeByte						(const HANDLE hContact, const char *szModule, const char *szSetting, BYTE value);
	static INT_PTR		writeByte						(const char *szModule, const char *szSetting, BYTE value);

	static INT_PTR		writeTString					(const HANDLE hContact, const char *szModule, const char *szSetting, const TCHAR *st);
	static INT_PTR		writeString						(const HANDLE hContact, const char *szModule, const char *szSetting, const char *st);
	static int 			getCache						(const HANDLE hContact, const char *szProto);

public:
	static TCluiData	dat;
	static ClcData*		clcdat;
	static TExtraCache* eCache;
	static int			nextCacheEntry, maxCacheEntry;

	static CRITICAL_SECTION	cachecs;

	static bool			shutDown;
};

struct TSysConfig {
	bool			isVistaPlus;
	bool			isSevenPlus;
	bool			isXPPlus;
	bool			uxThemeValid;
	bool			isWin2KPlus;
};

struct TSysState {
	bool			isThemed;
	bool			isAero;
	bool			isDwmActive;
};

class API
{
public:
	static void			onInit						();
	static void			onUnload					();
	static void			updateState					();

	static void 			Ex_CopyEditToClipboard		(HWND hWnd);
	static INT_PTR CALLBACK Ex_DlgProc					(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void 			Ex_Handler					();
	static int 				Ex_ShowDialog				(EXCEPTION_POINTERS *ep, const char *szFile, int line, TCHAR* szReason, bool fAllowContinue);


	/*
	 * helpers
	 */

	static BOOL				SetLayeredWindowAttributes(HWND hWnd, COLORREF clr, BYTE alpha, DWORD dwFlags);

	static pfnSetLayeredWindowAttributes_t 	pfnSetLayeredWindowAttributes;
	static pfnUpdateLayeredWindow_t			pfnUpdateLayeredWindow;
	static pfnMonitorFromPoint_t  			pfnMonitorFromPoint;
	static pfnMonitorFromWindow_t 			pfnMonitorFromWindow;
	static pfnGetMonitorInfo_t    			pfnGetMonitorInfo;
	static pfnTrackMouseEvent_t   			pfnTrackMouseEvent;
	// static pfnDrawAlpha_t 					pfnDrawAlpha;
	static pfnSetLayout_t					pfnSetLayout;
	static PGF 								pfnGradientFill;
	static pfnAlphaBlend_t					pfnAlphaBlend;

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

	static pfnDwmExtendFrameIntoClientArea_t			pfnDwmExtendFrameIntoClientArea;
	static pfnDwmIsCompositionEnabled_t					pfnDwmIsCompositionEnabled;

	static TSysConfig						sysConfig;
	static TSysState						sysState;

	static EXCEPTION_RECORD exRecord;
	static CONTEXT			exCtx;
	static LRESULT			exLastResult;
	static char				exSzFile[MAX_PATH];
	static TCHAR			exReason[256];
	static int				exLine;
	static bool				exAllowContinue;

	static HMODULE			hUxTheme, hDwm;
};


class Utils
{
public:
	static void TSAPI 		enableDlgControl(const HWND hwnd, UINT id, BOOL fEnable);
	static void TSAPI 		showDlgControl(const HWND hwnd, UINT id, int showCmd);
	static HMODULE 			loadSystemLibrary(const TCHAR* szFilename, bool useGetHandle = false);

};

#endif /* __CONFIG_H_*/
