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

#include <stdexcept>

#ifndef __CONFIG_H_
#define __CONFIG_H_

#define	DEFAULT_MODULE "Clist"

#define IS_THEMED (API::sysState.isThemed)
#define IS_AERO (API::sysState.isAero)

typedef HRESULT 	(WINAPI *pfnDwmExtendFrameIntoClientArea_t)(HWND hwnd, const MARGINS *margins);
typedef HRESULT 	(WINAPI *pfnDwmIsCompositionEnabled_t)(BOOL *);

class CRTException : public std::runtime_error
{
public:
	CRTException(const char *szMsg, const wchar_t *szParam);
	~CRTException() {}

	void display() const;

private:
	wchar_t	m_szParam[MAX_PATH];
};

class cfg
{
public:
	static TExtraCache* getCache(const MCONTACT hContact, const char *szProto);

   static TCluiData dat;
   static ClcData*  clcdat;

	static LIST<TExtraCache> arCache;

   static bool    shutDown;
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

class API
{
public:
	static void			onInit						();
	static void			updateState					();

	static void 			Ex_CopyEditToClipboard		(HWND hWnd);
	static INT_PTR CALLBACK Ex_DlgProc					(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void 			Ex_Handler					();
	static int 				Ex_ShowDialog				(EXCEPTION_POINTERS *ep, const char *szFile, int line, wchar_t* szReason, bool fAllowContinue);

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

	static HMODULE			hDwm;
};


class Utils
{
public:
	static void TSAPI 		enableDlgControl(const HWND hwnd, UINT id, BOOL fEnable);
	static void TSAPI 		showDlgControl(const HWND hwnd, UINT id, int showCmd);
	static HMODULE 			loadSystemLibrary(const wchar_t* szFilename, bool useGetHandle = false);

};

#endif /* __CONFIG_H_*/
