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
 * $Id: config.cpp 12887 2010-10-04 03:58:32Z borkra $
 *
 */

#include <commonheaders.h>

TCluiData		cfg::dat = {0};
ClcData*		cfg::clcdat = 0;
TExtraCache* 	cfg::eCache = 0;
int 			cfg::nextCacheEntry = 0, cfg::maxCacheEntry = 0;

CRITICAL_SECTION cfg::cachecs = {0};

bool			cfg::shutDown = false;

pfnSetLayeredWindowAttributes_t 	API::pfnSetLayeredWindowAttributes = 0;
pfnUpdateLayeredWindow_t			API::pfnUpdateLayeredWindow = 0;
pfnMonitorFromPoint_t  				API::pfnMonitorFromPoint = 0;
pfnMonitorFromWindow_t 				API::pfnMonitorFromWindow = 0;
pfnGetMonitorInfo_t    				API::pfnGetMonitorInfo = 0;
pfnTrackMouseEvent_t   				API::pfnTrackMouseEvent = 0;
//pfnDrawAlpha_t 						API::pfnDrawAlpha = 0;
PGF 								API::pfnGradientFill = 0;
pfnSetLayout_t						API::pfnSetLayout = 0;
pfnAlphaBlend_t						API::pfnAlphaBlend = 0;

TSysConfig							API::sysConfig = {0};
TSysState							API::sysState = {0};

pfnIsThemeActive_t 							API::pfnIsThemeActive = 0;
pfnOpenThemeData_t 							API::pfnOpenThemeData = 0;
pfnDrawThemeBackground_t 					API::pfnDrawThemeBackground = 0;
pfnCloseThemeData_t			 				API::pfnCloseThemeData = 0;
pfnDrawThemeText_t 							API::pfnDrawThemeText = 0;
pfnDrawThemeTextEx_t						API::pfnDrawThemeTextEx = 0;
pfnIsThemeBackgroundPartiallyTransparent_t 	API::pfnIsThemeBackgroundPartiallyTransparent = 0;
pfnDrawThemeParentBackground_t 	 			API::pfnDrawThemeParentBackground = 0;
pfnGetThemeBackgroundContentRect_t 			API::pfnGetThemeBackgroundContentRect = 0;
pfnEnableThemeDialogTexture_t 				API::pfnEnableThemeDialogTexture = 0;

pfnDwmExtendFrameIntoClientArea_t			API::pfnDwmExtendFrameIntoClientArea = 0;
pfnDwmIsCompositionEnabled_t				API::pfnDwmIsCompositionEnabled = 0;

EXCEPTION_RECORD API::exRecord = {0};
CONTEXT		 	 API::exCtx = {0};
LRESULT			 API::exLastResult = 0;
char			 API::exSzFile[MAX_PATH] = "";
TCHAR			 API::exReason[256] = _T("");
int				 API::exLine = 0;
bool			 API::exAllowContinue = false;
HMODULE			 API::hUxTheme = 0, API::hDwm = 0;

void cfg::init()
{
	InitializeCriticalSection(&cachecs);
}

DWORD cfg::getDword(const HANDLE hContact = 0, const char *szModule = 0, const char *szSetting = 0, DWORD uDefault = 0)
{
	return((DWORD)DBGetContactSettingDword(hContact, szModule, szSetting, uDefault));
}

/*
 * read a setting from our default module (Tab_SRMSG)
 */

DWORD cfg::getDword(const char *szSetting = 0, DWORD uDefault = 0)
{
	return((DWORD)DBGetContactSettingDword(0, DEFAULT_MODULE, szSetting, uDefault));
}

/*
 * read a setting from module only
 */

DWORD cfg::getDword(const char *szModule, const char *szSetting, DWORD uDefault)
{
	return((DWORD)DBGetContactSettingDword(0, szModule, szSetting, uDefault));
}


WORD cfg::getWord(const HANDLE hContact = 0, const char *szModule = 0, const char *szSetting = 0, WORD uDefault = 0)
{
	return((WORD)DBGetContactSettingWord(hContact, szModule, szSetting, uDefault));
}

/*
 * read a setting from our default module (Tab_SRMSG)
 */

WORD cfg::getWord(const char *szSetting = 0, WORD uDefault = 0)
{
	return((WORD)DBGetContactSettingWord(0, DEFAULT_MODULE, szSetting, uDefault));
}

/*
 * read a setting from module only
 */

WORD cfg::getWord(const char *szModule, const char *szSetting, WORD uDefault)
{
	return((WORD)DBGetContactSettingWord(0, szModule, szSetting, uDefault));
}

/*
 * same for bytes now
 */
int cfg::getByte(const HANDLE hContact = 0, const char *szModule = 0, const char *szSetting = 0, int uDefault = 0)
{
	return(DBGetContactSettingByte(hContact, szModule, szSetting, uDefault));
}

int cfg::getByte(const char *szSetting = 0, int uDefault = 0)
{
	return(DBGetContactSettingByte(0, DEFAULT_MODULE, szSetting, uDefault));
}

int cfg::getByte(const char *szModule, const char *szSetting, int uDefault)
{
	return(DBGetContactSettingByte(0, szModule, szSetting, uDefault));
}

INT_PTR cfg::getTString(const HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	return(DBGetContactSettingTString(hContact, szModule, szSetting, dbv));
}

INT_PTR cfg::getString(const HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	return(DBGetContactSettingString(hContact, szModule, szSetting, dbv));
}

/*
 * writer functions
 */

INT_PTR cfg::writeDword(const HANDLE hContact = 0, const char *szModule = 0, const char *szSetting = 0, DWORD value = 0)
{
	return(DBWriteContactSettingDword(hContact, szModule, szSetting, value));
}

INT_PTR cfg::writeDword(const char *szModule = 0, const char *szSetting = 0, DWORD value = 0)
{
	return(DBWriteContactSettingDword(0, szModule, szSetting, value));
}

INT_PTR cfg::writeWord(const HANDLE hContact = 0, const char *szModule = 0, const char *szSetting = 0, WORD value = 0)
{
	return(DBWriteContactSettingWord(hContact, szModule, szSetting, value));
}

INT_PTR cfg::writeWord(const char *szModule = 0, const char *szSetting = 0, WORD value = 0)
{
	return(DBWriteContactSettingWord(0, szModule, szSetting, value));
}

INT_PTR cfg::writeByte(const HANDLE hContact = 0, const char *szModule = 0, const char *szSetting = 0, BYTE value = 0)
{
	return(DBWriteContactSettingByte(hContact, szModule, szSetting, value));
}

INT_PTR cfg::writeByte(const char *szModule = 0, const char *szSetting = 0, BYTE value = 0)
{
	return(DBWriteContactSettingByte(0, szModule, szSetting, value));
}

INT_PTR cfg::writeTString(const HANDLE hContact, const char *szModule = 0, const char *szSetting = 0, const TCHAR *str = 0)
{
	return(DBWriteContactSettingTString(hContact, szModule, szSetting, str));
}

INT_PTR cfg::writeString(const HANDLE hContact, const char *szModule = 0, const char *szSetting = 0, const char *str = 0)
{
	return(DBWriteContactSettingString(hContact, szModule, szSetting, str));
}

int cfg::getCache(const HANDLE hContact, const char *szProto)
{
    int i, iFound = -1;

    for(i = 0; i < nextCacheEntry; i++) {
        if(eCache[i].hContact == hContact) {
            iFound = i;
            break;
        }
    }
    if(iFound == -1) {
		EnterCriticalSection(&cachecs);
        if(nextCacheEntry == maxCacheEntry) {
            maxCacheEntry += 100;
            cfg::eCache = (TExtraCache *)realloc(cfg::eCache, maxCacheEntry * sizeof(TExtraCache));
        }
        memset(&cfg::eCache[nextCacheEntry], 0, sizeof(TExtraCache));
		cfg::eCache[nextCacheEntry].hContact = hContact;
        memset(cfg::eCache[nextCacheEntry].iExtraImage, 0xff, MAXEXTRACOLUMNS);
        cfg::eCache[nextCacheEntry].iExtraValid = 0;
        cfg::eCache[nextCacheEntry].valid = FALSE;
        cfg::eCache[nextCacheEntry].bStatusMsgValid = 0;
        cfg::eCache[nextCacheEntry].statusMsg = NULL;
        cfg::eCache[nextCacheEntry].status_item = NULL;
        LoadSkinItemToCache(&cfg::eCache[nextCacheEntry], szProto);
        cfg::eCache[nextCacheEntry].dwCFlags = 0;
        cfg::eCache[nextCacheEntry].dwDFlags = DBGetContactSettingDword(hContact, "CList", "CLN_Flags", 0);
        cfg::eCache[nextCacheEntry].dwXMask = CalcXMask(hContact);
        GetCachedStatusMsg(nextCacheEntry, const_cast<char *>(szProto));
		cfg::eCache[nextCacheEntry].dwLastMsgTime = INTSORT_GetLastMsgTime(hContact);
        iFound = nextCacheEntry++;
		LeaveCriticalSection(&cachecs);
    }
    return iFound;
}

void API::onInit()
{
	HMODULE hUserDll = 0;

	pfnSetLayout = (DWORD ( WINAPI *)(HDC, DWORD))GetProcAddress( GetModuleHandleA( "GDI32.DLL" ), "SetLayout" );

	hUserDll = GetModuleHandleA("user32.dll");
	if (hUserDll) {
		pfnMonitorFromPoint = ( pfnMonitorFromPoint_t )GetProcAddress(hUserDll, "MonitorFromPoint");
		pfnMonitorFromWindow = ( pfnMonitorFromWindow_t )GetProcAddress(hUserDll, "MonitorFromWindow");
		pfnGetMonitorInfo = ( pfnGetMonitorInfo_t )GetProcAddress(hUserDll, "GetMonitorInfoA");
		pfnSetLayeredWindowAttributes = ( pfnSetLayeredWindowAttributes_t )GetProcAddress(hUserDll, "SetLayeredWindowAttributes");
		pfnUpdateLayeredWindow = ( pfnUpdateLayeredWindow_t )GetProcAddress(hUserDll, "UpdateLayeredWindow");
		pfnTrackMouseEvent = ( pfnTrackMouseEvent_t )GetProcAddress(hUserDll, "TrackMouseEvent");
	}

	pfnAlphaBlend = (pfnAlphaBlend_t) GetProcAddress(GetModuleHandleA("gdi32"), "GdiAlphaBlend");
	pfnGradientFill = (PGF) GetProcAddress(GetModuleHandleA("gdi32"), "GdiGradientFill");
	if (0 == pfnAlphaBlend) {
		HMODULE hMsImgDll = LoadLibraryA("msimg32.dll");
		pfnAlphaBlend = (pfnAlphaBlend_t)GetProcAddress(hMsImgDll, "AlphaBlend");
		pfnGradientFill = (PGF) GetProcAddress(hMsImgDll, "GradientFill");
	}

	sysConfig.isVistaPlus = (IsWinVerVistaPlus() ? true : false);
	sysConfig.isSevenPlus = (IsWinVer7Plus() ? true : false);
	sysConfig.isXPPlus = (IsWinVerXPPlus() ? true : false);
	sysConfig.isWin2KPlus = (IsWinVer2000Plus() ? true : false);

	if(sysConfig.isXPPlus) {
		if ((hUxTheme = Utils::loadSystemLibrary(_T("\\uxtheme.dll")), true) != 0) {
			pfnIsThemeActive = (pfnIsThemeActive_t)GetProcAddress(hUxTheme, "IsThemeActive");
			pfnOpenThemeData = (pfnOpenThemeData_t)GetProcAddress(hUxTheme, "OpenThemeData");
			pfnDrawThemeBackground = (pfnDrawThemeBackground_t)GetProcAddress(hUxTheme, "DrawThemeBackground");
			pfnCloseThemeData = (pfnCloseThemeData_t)GetProcAddress(hUxTheme, "CloseThemeData");
			pfnDrawThemeText = (pfnDrawThemeText_t)GetProcAddress(hUxTheme, "DrawThemeText");
			pfnIsThemeBackgroundPartiallyTransparent = (pfnIsThemeBackgroundPartiallyTransparent_t)GetProcAddress(hUxTheme, "IsThemeBackgroundPartiallyTransparent");
			pfnDrawThemeParentBackground = (pfnDrawThemeParentBackground_t)GetProcAddress(hUxTheme, "DrawThemeParentBackground");
			pfnGetThemeBackgroundContentRect = (pfnGetThemeBackgroundContentRect_t)GetProcAddress(hUxTheme, "GetThemeBackgroundContentRect");
			pfnEnableThemeDialogTexture = (pfnEnableThemeDialogTexture_t)GetProcAddress(hUxTheme, "EnableThemeDialogTexture");

			if (pfnIsThemeActive != 0 && pfnOpenThemeData != 0 && pfnDrawThemeBackground != 0 && pfnCloseThemeData != 0
				&& pfnDrawThemeText != 0 && pfnIsThemeBackgroundPartiallyTransparent != 0 && pfnDrawThemeParentBackground != 0
				&& pfnGetThemeBackgroundContentRect != 0) {
				sysConfig.uxThemeValid = true;
			}
		}
	}
	if(sysConfig.isVistaPlus) {
		if ((hDwm = Utils::loadSystemLibrary(_T("\\dwmapi.dll")), true) != 0) {
			pfnDwmIsCompositionEnabled = (pfnDwmIsCompositionEnabled_t)GetProcAddress(hDwm, "DwmIsCompositionEnabled");
            pfnDwmExtendFrameIntoClientArea = (pfnDwmExtendFrameIntoClientArea_t)GetProcAddress(hDwm,"DwmExtendFrameIntoClientArea");
		}
	}

	updateState();
}

void API::onUnload()
{
	if(hUxTheme)
		FreeLibrary(hUxTheme);
}

/**
 * update system's state (theme status, aero status, DWM check...
 *
 * called when windows broadcasts things like WM_THEMECHANGED or
 * WM_DWMCOMPOSITIONCHANGED
 */
void API::updateState()
{
	BOOL result = FALSE;

	::ZeroMemory(&sysState, sizeof(TSysState));

	if(sysConfig.uxThemeValid)
		sysState.isThemed = pfnIsThemeActive() ? true : false;

	if(sysConfig.isVistaPlus) {
		sysState.isDwmActive = (pfnDwmIsCompositionEnabled && (pfnDwmIsCompositionEnabled(&result) == S_OK) && result) ? true : false;
		sysState.isAero = /* (CSkin::m_skinEnabled == false) && */ cfg::getByte("CLUI", "useAero", 1) /* && CSkin::m_fAeroSkinsValid */ && sysState.isDwmActive;
	}
}

BOOL API::SetLayeredWindowAttributes(HWND hWnd, COLORREF clr, BYTE alpha, DWORD dwFlags)
{
	if(sysConfig.isWin2KPlus)
		return(pfnSetLayeredWindowAttributes(hWnd, clr, alpha, dwFlags));

	return(FALSE);
}

/**
 * exception handling for SEH exceptions
 */

/**
 * exception handling - copy error message to clip board
 * @param hWnd: 	window handle of the edit control containing the error message
 */
void API::Ex_CopyEditToClipboard(HWND hWnd)
{
	SendMessage(hWnd, EM_SETSEL, 0, 65535L);
	SendMessage(hWnd, WM_COPY, 0 , 0);
	SendMessage(hWnd, EM_SETSEL, 0, 0);
}

INT_PTR CALLBACK API::Ex_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode, wID;

	switch(uMsg) {
		case WM_INITDIALOG: {
				char szBuffer[2048];
#ifdef _WIN64
				sprintf(szBuffer,
						"Exception %16.16X at address %16.16X occured in %s at line %d.\r\n\r\nEAX=%16.16X EBX=%16.16X ECX=%16.16X\r\nEDX=%16.16X ESI=%16.16X EDI=%16.16X\r\nEBP=%16.16X ESP=%16.16X EIP=%16.16X",
						exRecord.ExceptionCode, exRecord.ExceptionAddress, exSzFile, exLine,
						exCtx.Rax,exCtx.Rbx, exCtx.Rcx, exCtx.Rdx,
						exCtx.Rsi, exCtx.Rdi, exCtx.Rbp, exCtx.Rsp, exCtx.Rip);
#else
				sprintf(szBuffer,
						"Exception %8.8X at address %8.8X occured in %s at line %d.\r\n\r\nEAX=%8.8X EBX=%8.8X ECX=%8.8X\r\nEDX=%8.8X ESI=%8.8X EDI=%8.8X\r\nEBP=%8.8X ESP=%8.8X EIP=%8.8X",
						exRecord.ExceptionCode, exRecord.ExceptionAddress, exSzFile, exLine,
						exCtx.Eax,exCtx.Ebx, exCtx.Ecx, exCtx.Edx,
						exCtx.Esi, exCtx.Edi, exCtx.Ebp, exCtx.Esp, exCtx.Eip);
#endif
				SetDlgItemTextA(hwndDlg, IDC_EXCEPTION_DETAILS, szBuffer);
				SetFocus(GetDlgItem(hwndDlg, IDC_EXCEPTION_DETAILS));
				SendDlgItemMessage(hwndDlg, IDC_EXCEPTION_DETAILS, WM_SETFONT, (WPARAM)GetStockObject(OEM_FIXED_FONT), 0);
				SetDlgItemText(hwndDlg, IDC_EX_REASON, exReason);
				Utils::enableDlgControl(hwndDlg, IDOK, exAllowContinue ? TRUE : FALSE);
			}
			break;

		case WM_COMMAND:
			wNotifyCode = HIWORD(wParam);
			wID = LOWORD(wParam);
			if (wNotifyCode == BN_CLICKED)
			{
				if (wID == IDOK || wID == IDCANCEL)
					EndDialog(hwndDlg, wID);

				if (wID == IDC_COPY_EXCEPTION)
					Ex_CopyEditToClipboard(GetDlgItem(hwndDlg, IDC_EXCEPTION_DETAILS));
			}

			break;
	}
	return FALSE;
}

void API::Ex_Handler()
{
	if (exLastResult == IDCANCEL)
		ExitProcess(1);
}

int API::Ex_ShowDialog(EXCEPTION_POINTERS *ep, const char *szFile, int line, TCHAR* szReason, bool fAllowContinue)
{
	char	szDrive[MAX_PATH], szDir[MAX_PATH], szName[MAX_PATH], szExt[MAX_PATH];

	_splitpath(szFile, szDrive, szDir, szName, szExt);
	memcpy(&exRecord, ep->ExceptionRecord, sizeof(EXCEPTION_RECORD));
	memcpy(&exCtx, ep->ContextRecord, sizeof(CONTEXT));

	_snprintf(exSzFile, MAX_PATH, "%s%s", szName, szExt);
	mir_sntprintf(exReason, 256, _T("An application error has occured: %s"), szReason);
	exLine = line;
	exLastResult = DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_EXCEPTION), 0, Ex_DlgProc, 0);
	exAllowContinue = fAllowContinue;
	if(IDCANCEL == exLastResult)
		ExitProcess(1);
	return 1;
}


/**
 * enable or disable a dialog control
 */
void TSAPI Utils::enableDlgControl(const HWND hwnd, UINT id, BOOL fEnable)
{
	::EnableWindow(::GetDlgItem(hwnd, id), fEnable);
}

/**
 * show or hide a dialog control
 */
void TSAPI Utils::showDlgControl(const HWND hwnd, UINT id, int showCmd)
{
	::ShowWindow(::GetDlgItem(hwnd, id), showCmd);
}

/**
 * load a system library from the Windows system path and return its module
 * handle.
 *
 * return 0 and throw an exception if something goes wrong.
 */
HMODULE Utils::loadSystemLibrary(const TCHAR* szFilename, bool useGetHandle)
{
	TCHAR		sysPathName[MAX_PATH + 2];
	HMODULE		_h = 0;

	try {
		if(0 == ::GetSystemDirectory(sysPathName, MAX_PATH))
			throw(CRTException("Error while loading system library", szFilename));

		sysPathName[MAX_PATH - 1] = 0;
		if(_tcslen(sysPathName) + _tcslen(szFilename) >= MAX_PATH)
			throw(CRTException("Error while loading system library", szFilename));

		lstrcat(sysPathName, szFilename);
		if(useGetHandle)
			_h = ::GetModuleHandle(sysPathName);
		else
			_h = LoadLibrary(sysPathName);
		if(0 == _h)
			throw(CRTException("Error while loading system library", szFilename));
	}
	catch(CRTException& ex) {
		ex.display();
		return(0);
	}
	return(_h);
}

CRTException::CRTException(const char *szMsg, const TCHAR *szParam) : std::runtime_error(std::string(szMsg))
{
	mir_sntprintf(m_szParam, MAX_PATH, szParam);
}

void CRTException::display() const
{
	TCHAR*	tszMsg = mir_a2t(what());
	TCHAR  	tszBoxMsg[500];

	mir_sntprintf(tszBoxMsg, 500, _T("%s\n\n(%s)"), tszMsg, m_szParam);
	::MessageBox(0, tszBoxMsg, _T("TabSRMM runtime error"), MB_OK | MB_ICONERROR);
	mir_free(tszMsg);
}
