/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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
	CRTException(const char *szMsg, const TCHAR *szParam);
	~CRTException() {}

	void display() const;

private:
	TCHAR	m_szParam[MAX_PATH];
};

class cfg
{
public:
	static void    init();

   static DWORD   getDword(const MCONTACT hContact, const char *szModule, const char *szSetting, DWORD uDefault);
   static DWORD   getDword(const char *szModule, const char *szSetting, DWORD uDefault);
   static DWORD   getDword(const char *szSetting, DWORD uDefault);

   static WORD    getWord(const MCONTACT hContact, const char *szModule, const char *szSetting, WORD uDefault);
   static WORD    getWord(const char *szModule, const char *szSetting, WORD uDefault);
   static WORD    getWord(const char *szSetting, WORD uDefault);

   static int     getByte(const MCONTACT hContact, const char *szModule, const char *szSetting, int uDefault);
   static int     getByte(const char *szModule, const char *szSetting, int uDefault);
   static int     getByte(const char *szSetting, int uDefault);

   static INT_PTR getTString(const MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv);
   static INT_PTR getString(const MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv);

   static INT_PTR writeDword(const MCONTACT hContact, const char *szModule, const char *szSetting, DWORD value);
   static INT_PTR writeDword(const char *szModule, const char *szSetting, DWORD value);

   static INT_PTR writeWord(const MCONTACT hContact, const char *szModule, const char *szSetting, WORD value);
   static INT_PTR writeWord(const char *szModule, const char *szSetting, WORD value);

   static INT_PTR writeByte(const MCONTACT hContact, const char *szModule, const char *szSetting, BYTE value);
   static INT_PTR writeByte(const char *szModule, const char *szSetting, BYTE value);

   static INT_PTR writeTString(const MCONTACT hContact, const char *szModule, const char *szSetting, const TCHAR *st);
   static INT_PTR writeString(const MCONTACT hContact, const char *szModule, const char *szSetting, const char *st);

	static TExtraCache* getCache(const MCONTACT hContact, const char *szProto);

public:
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
	static int 				Ex_ShowDialog				(EXCEPTION_POINTERS *ep, const char *szFile, int line, TCHAR* szReason, bool fAllowContinue);

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

	static HMODULE			hDwm;
};


class Utils
{
public:
	static void TSAPI 		enableDlgControl(const HWND hwnd, UINT id, BOOL fEnable);
	static void TSAPI 		showDlgControl(const HWND hwnd, UINT id, int showCmd);
	static HMODULE 			loadSystemLibrary(const TCHAR* szFilename, bool useGetHandle = false);

};

#endif /* __CONFIG_H_*/
