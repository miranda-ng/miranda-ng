/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2009 Miranda ICQ/IM project,
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
 * part of tabSRMM messaging plugin for Miranda.
 *
 * (C) 2005-2009 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: mim.h 12272 2010-08-04 08:24:08Z silvercircle $
 *
 * wraps some parts of Miranda API
 * Also, OS dependent stuff (visual styles api etc.)
 *
 */

#ifndef __MIM_H
#define __MIM_H


extern  FI_INTERFACE *FIF;

/*
 * Win32 API definitions of functions dynamically obtained via GetProcAddress()
 * - uxtheme
 * - dwmapi
 * - some GDI functions (AlphaBlend()..)
 */
typedef BOOL 	(WINAPI *SMI)( HMENU hmenu, LPCMENUINFO lpcmi );
typedef HRESULT (WINAPI *DEFICA)(HWND hwnd, const MARGINS *margins);
typedef HRESULT (WINAPI *DICE)(BOOL *);
typedef DWORD 	(WINAPI *PSLWA)(HWND, DWORD, BYTE, DWORD);
typedef BOOL 	(WINAPI *PULW)(HWND, HDC, POINT *, SIZE *, HDC, POINT *, COLORREF, BLENDFUNCTION *, DWORD);
typedef BOOL 	(WINAPI *PFWEX)(FLASHWINFO *);
typedef BOOL 	(WINAPI *PAB)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);
typedef BOOL 	(WINAPI *PGF)(HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);

typedef BOOL 	(WINAPI *PITA)();
typedef HANDLE	(WINAPI *POTD)(HWND, LPCWSTR);
typedef UINT	(WINAPI *PDTB)(HANDLE, HDC, int, int, RECT *, RECT *);
typedef UINT	(WINAPI *PCTD)(HANDLE);
typedef UINT	(WINAPI *PDTT)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, DWORD, RECT *);
typedef UINT	(WINAPI *PDTTE)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, RECT *, const DTTOPTS *);
typedef BOOL 	(WINAPI *PITBPT)(HANDLE, int, int);
typedef HRESULT	(WINAPI *PDTPB)(HWND, HDC, RECT *);
typedef HRESULT	(WINAPI *PGTBCR)(HANDLE, HDC, int, int, const RECT *, const RECT *);

typedef HMONITOR(WINAPI *MMFW)(HWND, DWORD);
typedef BOOL	(WINAPI *GMIA)(HMONITOR, LPMONITORINFO);
typedef HRESULT	(WINAPI *DRT)(HWND, HWND, PHTHUMBNAIL);
typedef BOOL	(WINAPI *ETDT)(HANDLE, DWORD);
typedef HANDLE  (WINAPI *BBP)(HDC, RECT *, BP_BUFFERFORMAT, BP_PAINTPARAMS *, HDC *);
typedef HRESULT (WINAPI *EBP)(HANDLE, BOOL);
typedef HRESULT (WINAPI *BPI)(void);
typedef HRESULT (WINAPI *BPU)(void);
typedef HRESULT (WINAPI *BBW)(HWND, DWM_BLURBEHIND *);
typedef HRESULT (WINAPI *DGC)(DWORD *, BOOL *);
typedef HRESULT (WINAPI *BPSA)(HANDLE, const RECT *, BYTE);
typedef int		(WINAPI *GLIX)(LPCWSTR, LCTYPE, LPCWSTR, int);
typedef HRESULT (WINAPI *DWMSWA)(HWND, DWORD, LPCVOID, DWORD);
typedef HRESULT (WINAPI *DWMIIB)(HWND);
typedef HRESULT (WINAPI *DWMUT)(HTHUMBNAIL, DWM_THUMBNAIL_PROPERTIES *);
typedef HRESULT (WINAPI *DURT)(HTHUMBNAIL);
typedef HRESULT (WINAPI *DSIT)(HWND, HBITMAP, DWORD);
typedef HRESULT (WINAPI *DSILP)(HWND, HBITMAP, POINT *, DWORD);

/*
 * used to encapsulate some parts of the Miranda API
 * constructor does early time initialization - do NOT put anything
 * here, except thngs that deal with the core and database API.
 *
 * it is UNSAFE to assume that any plugin provided services are available
 * when the object is instantiated.
 */

class CMimAPI
{
public:
	CMimAPI()
	{
		GetUTFI();
		InitPaths();
		InitAPI();
		getAeroState();

		LRESULT fi_version = CallService(MS_IMG_GETIFVERSION, 0, 0);
		CallService(MS_IMG_GETINTERFACE, fi_version, (LPARAM)&FIF);

		::QueryPerformanceFrequency((LARGE_INTEGER *)&m_tFreq);
		m_dFreq = (double)(1.0f / m_tFreq);
		m_hChatLogLock = INVALID_HANDLE_VALUE;
	}

	~CMimAPI() {
		if (m_hUxTheme != 0)
			FreeLibrary(m_hUxTheme);
		if (m_hDwmApi != 0)
			FreeLibrary(m_hDwmApi);
		if	(m_haveBufferedPaint)
			m_pfnBufferedPaintUninit();

		if(m_hChatLogLock != INVALID_HANDLE_VALUE)
			CloseHandle(m_hChatLogLock);
	}

	/*
	 * database functions
	 */

	DWORD FASTCALL 		GetDword						(const HANDLE hContact, const char *szModule, const char *szSetting, DWORD uDefault) const;

	DWORD FASTCALL 		GetDword						(const char *szModule, const char *szSetting, DWORD uDefault) const;
	DWORD FASTCALL 		GetDword						(const char *szSetting, DWORD uDefault) const;
	DWORD FASTCALL 		GetDword						(const HANDLE hContact, const char *szSetting, DWORD uDefault) const;

	int FASTCALL 		GetByte							(const HANDLE hContact, const char *szModule, const char *szSetting, int uDefault) const;
	int FASTCALL 		GetByte							(const char *szModule, const char *szSetting, int uDefault) const;
	int FASTCALL 		GetByte							(const char *szSetting, int uDefault) const;
	int FASTCALL 		GetByte							(const HANDLE hContact, const char *szSetting, int uDefault) const;

	INT_PTR FASTCALL 	GetTString						(const HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv) const;
	INT_PTR FASTCALL 	GetString						(const HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv) const;

	INT_PTR FASTCALL 	WriteDword						(const HANDLE hContact, const char *szModule, const char *szSetting, DWORD value) const;
	INT_PTR FASTCALL 	WriteDword						(const char *szModule, const char *szSetting, DWORD value) const;

	INT_PTR FASTCALL 	WriteByte						(const HANDLE hContact, const char *szModule, const char *szSetting, BYTE value) const;
	INT_PTR FASTCALL 	WriteByte						(const char *szModule, const char *szSetting, BYTE value) const;

	INT_PTR FASTCALL 	WriteTString					(const HANDLE hContact, const char *szModule, const char *szSetting, const TCHAR *st) const;

	/*
	 * utf helpers
	 */

	char* FASTCALL 		utf8_decode						(char* str, wchar_t** ucs2) const;
	char* FASTCALL 		utf8_decodecp					(char* str, int codepage, wchar_t** ucs2) const;
	char* FASTCALL 		utf8_encode						(const char* src) const;
	char* FASTCALL 		utf8_encodecp					(const char* src, int codepage) const;
	char* FASTCALL 		utf8_encodeW					(const wchar_t* src) const;
	char* FASTCALL 		utf8_encodeT					(const TCHAR* src) const;
	TCHAR* FASTCALL 	utf8_decodeT					(const char* src) const;
	wchar_t* FASTCALL 	utf8_decodeW					(const char* str) const;

	/*
	 * path utilities
	 */

	int  				pathIsAbsolute					(const TCHAR *path) const;
	size_t 		 		pathToAbsolute					(const TCHAR *pSrc, TCHAR *pOut, const TCHAR *szBase = 0) const;
	size_t 				pathToRelative					(const TCHAR *pSrc, TCHAR *pOut, const TCHAR *szBase = 0) const;

	const TCHAR* 		getDataPath() const 			{ return(m_szProfilePath); }
	const TCHAR* 		getSkinPath() const 			{ return(m_szSkinsPath); }
	const TCHAR* 		getSavedAvatarPath() const 		{ return(m_szSavedAvatarsPath); }
	const TCHAR* 		getChatLogPath() const 			{ return(m_szChatLogsPath); }
	const bool	 		haveFoldersPlugin() const 		{ return(m_haveFolders); }

	const TCHAR* 		getUserDir();
	void		  		configureCustomFolders();
	INT_PTR		  		foldersPathChanged();

	void				startTimer();
	void				stopTimer						(const char *szMsg = 0);
	void				timerMsg						(const char *szMsg);
	__int64				getTimerStart() const 			{ return(m_tStart); }
	__int64				getTimerStop() const 			{ return(m_tStop); }
	__int64				getTicks() const 				{ return(m_tStop - m_tStart); }
	double				getFreq() const 				{ return(m_dFreq); }
	double				getMsec() const 				{ return(1000 * ((double)(m_tStop - m_tStart) * m_dFreq)); }

	/*
	 * os dependant stuff (aero, visual styles etc.)
	 */

	const bool  		isVSAPIState() const { return m_VsAPI; }
	/**
	 * return status of Vista Aero
	 *
	 * @return bool: true if aero active, false otherwise
	 */
	const bool  		isAero() const 					{ return(m_isAero); }
	const bool  		isDwmActive() const 			{ return(m_DwmActive); }

	/**
	 * Refresh Aero status.
	 * Called on:
	 * * plugin init
	 * * WM_DWMCOMPOSITIONCHANGED message received
	 *
	 * @return
	 */
	bool				getAeroState();
	/**
	 * return status of visual styles theming engine (Windows XP+)
	 *
	 * @return bool: themes are enabled
	 */
	bool				isVSThemed()
	{
		return(m_isVsThemed);
	}
	/*
	 * window lists
	 */

	void			 	BroadcastMessage				(UINT msg, WPARAM wParam, LPARAM lParam);
	void			 	BroadcastMessageAsync			(UINT msg, WPARAM wParam, LPARAM lParam);
	INT_PTR			 	AddWindow						(HWND hWnd, HANDLE h);
	INT_PTR		 		RemoveWindow					(HWND hWnd);
	HWND		 		FindWindow						(HANDLE h) const;

	static	int 										FoldersPathChanged(WPARAM wParam, LPARAM lParam);		// hook subscriber for folders plugin
	static 	const TCHAR* TSAPI 							StriStr(const TCHAR *szString, const TCHAR *szSearchFor);
	static	int											TypingMessage(WPARAM wParam, LPARAM lParam);
	static	int											ProtoAck(WPARAM wParam, LPARAM lParam);
	static	int											PrebuildContactMenu(WPARAM wParam, LPARAM lParam);
	static 	int											DispatchNewEvent(WPARAM wParam, LPARAM lParam);
	static	int											MessageEventAdded(WPARAM wParam, LPARAM lParam);
public:
	HANDLE 		m_hMessageWindowList;
	/*
	 various function pointers
	*/
	static PITA 	m_pfnIsThemeActive;
	static POTD 	m_pfnOpenThemeData;
	static PDTB 	m_pfnDrawThemeBackground;
	static PCTD 	m_pfnCloseThemeData;
	static PDTT 	m_pfnDrawThemeText;
	static PDTTE	m_pfnDrawThemeTextEx;
	static PITBPT 	m_pfnIsThemeBackgroundPartiallyTransparent;
	static PDTPB  	m_pfnDrawThemeParentBackground;
	static PGTBCR 	m_pfnGetThemeBackgroundContentRect;
	static ETDT 	m_pfnEnableThemeDialogTexture;
	static PSLWA 	m_pSetLayeredWindowAttributes;
	static PFWEX	m_MyFlashWindowEx;
	static PAB		m_MyAlphaBlend;
	static PGF		m_MyGradientFill;
	static DEFICA	m_pfnDwmExtendFrameIntoClientArea;
	static DICE		m_pfnDwmIsCompositionEnabled;
	static MMFW		m_pfnMonitorFromWindow;
	static GMIA		m_pfnGetMonitorInfoA;
	static DRT		m_pfnDwmRegisterThumbnail;
	static BPI		m_pfnBufferedPaintInit;
	static BPU		m_pfnBufferedPaintUninit;
	static BBP		m_pfnBeginBufferedPaint;
	static EBP		m_pfnEndBufferedPaint;
	static BBW 		m_pfnDwmBlurBehindWindow;
	static DGC		m_pfnDwmGetColorizationColor;
	static BPSA		m_pfnBufferedPaintSetAlpha;
	static GLIX		m_pfnGetLocaleInfoEx;
	static DWMSWA   m_pfnDwmSetWindowAttribute;
	static DWMIIB	m_pfnDwmInvalidateIconicBitmaps;
	static DWMUT	m_pfnDwmUpdateThumbnailProperties;
	static DURT		m_pfnDwmUnregisterThumbnail;
	static DSIT		m_pfnDwmSetIconicThumbnail;
	static DSILP	m_pfnDwmSetIconicLivePreviewBitmap;
	static bool		m_shutDown, m_haveBufferedPaint;

	static DWORD	m_MimVersion;

private:
	UTF8_INTERFACE 	m_utfi;
	TCHAR 		m_szProfilePath[MAX_PATH + 2], m_szSkinsPath[MAX_PATH + 2], m_szSavedAvatarsPath[MAX_PATH + 2], m_szChatLogsPath[MAX_PATH + 2];
	HMODULE		m_hUxTheme, m_hDwmApi;
	bool		m_VsAPI;
	bool		m_isAero;
	bool		m_DwmActive;
	bool		m_isVsThemed;
	HANDLE		m_hDataPath, m_hSkinsPath, m_hAvatarsPath, m_hChatLogsPath;
	__int64		m_tStart, m_tStop, m_tFreq;
	double		m_dFreq;
	char		m_timerMsg[256];
	bool		m_haveFolders;
	HANDLE		m_hChatLogLock;

	void	InitAPI();
	void	GetUTFI();
	void 	InitPaths();

private:
	static TCHAR	m_userDir[MAX_PATH + 2];
};

inline void CMimAPI::startTimer()
{
	::QueryPerformanceCounter((LARGE_INTEGER *)&m_tStart);
}

inline void CMimAPI::stopTimer(const char *szMsg)
{
	::QueryPerformanceCounter((LARGE_INTEGER *)&m_tStop);

	if(szMsg)
		timerMsg(szMsg);
}

extern  CMimAPI		*M;

#endif /* __MIM_H */
