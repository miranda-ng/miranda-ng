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
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: mim.cpp 12842 2010-09-28 04:32:57Z borkra $
 *
 * wraps some parts of Miranda API
 * Also, OS dependent stuff (visual styles api etc.)
 *
 */


#include "commonheaders.h"
extern PLUGININFOEX pluginInfo;

PITA 	CMimAPI::m_pfnIsThemeActive = 0;
POTD 	CMimAPI::m_pfnOpenThemeData = 0;
PDTB 	CMimAPI::m_pfnDrawThemeBackground = 0;
PCTD 	CMimAPI::m_pfnCloseThemeData = 0;
PDTT 	CMimAPI::m_pfnDrawThemeText = 0;
PDTTE	CMimAPI::m_pfnDrawThemeTextEx = 0;
PITBPT 	CMimAPI::m_pfnIsThemeBackgroundPartiallyTransparent = 0;
PDTPB  	CMimAPI::m_pfnDrawThemeParentBackground = 0;
PGTBCR 	CMimAPI::m_pfnGetThemeBackgroundContentRect = 0;
ETDT 	CMimAPI::m_pfnEnableThemeDialogTexture = 0;
PSLWA 	CMimAPI::m_pSetLayeredWindowAttributes = 0;
PFWEX	CMimAPI::m_MyFlashWindowEx = 0;
PAB		CMimAPI::m_MyAlphaBlend = 0;
PGF		CMimAPI::m_MyGradientFill = 0;
DEFICA	CMimAPI::m_pfnDwmExtendFrameIntoClientArea = 0;
DICE	CMimAPI::m_pfnDwmIsCompositionEnabled = 0;
MMFW	CMimAPI::m_pfnMonitorFromWindow = 0;
GMIA	CMimAPI::m_pfnGetMonitorInfoA = 0;
DRT		CMimAPI::m_pfnDwmRegisterThumbnail = 0;
BPI		CMimAPI::m_pfnBufferedPaintInit = 0;
BPU		CMimAPI::m_pfnBufferedPaintUninit = 0;
BBP		CMimAPI::m_pfnBeginBufferedPaint = 0;
EBP		CMimAPI::m_pfnEndBufferedPaint = 0;
BBW		CMimAPI::m_pfnDwmBlurBehindWindow = 0;
DGC		CMimAPI::m_pfnDwmGetColorizationColor = 0;
BPSA	CMimAPI::m_pfnBufferedPaintSetAlpha = 0;
GLIX	CMimAPI::m_pfnGetLocaleInfoEx = 0;
DWMIIB  CMimAPI::m_pfnDwmInvalidateIconicBitmaps = 0;
DWMSWA	CMimAPI::m_pfnDwmSetWindowAttribute = 0;
DWMUT	CMimAPI::m_pfnDwmUpdateThumbnailProperties = 0;
DURT	CMimAPI::m_pfnDwmUnregisterThumbnail = 0;
DSIT	CMimAPI::m_pfnDwmSetIconicThumbnail = 0;
DSILP	CMimAPI::m_pfnDwmSetIconicLivePreviewBitmap = 0;
bool	CMimAPI::m_shutDown = 0;
TCHAR	CMimAPI::m_userDir[] = _T("\0");

bool	CMimAPI::m_haveBufferedPaint = false;
DWORD	CMimAPI::m_MimVersion = 0;

void CMimAPI::timerMsg(const char *szMsg)
{
	mir_snprintf(m_timerMsg, 256, "%s: %d ticks = %f msec", szMsg, (int)(m_tStop - m_tStart), 1000 * ((double)(m_tStop - m_tStart) * m_dFreq));
	_DebugTraceA(m_timerMsg);
}
/*
 * read a setting for a contact
 */

DWORD CMimAPI::GetDword(const HANDLE hContact = 0, const char *szModule = 0, const char *szSetting = 0, DWORD uDefault = 0) const
{
	return((DWORD)DBGetContactSettingDword(hContact, szModule, szSetting, uDefault));
}

/*
 * read a setting from our default module (Tab_SRMSG)
 */

DWORD CMimAPI::GetDword(const char *szSetting = 0, DWORD uDefault = 0) const
{
	return((DWORD)DBGetContactSettingDword(0, SRMSGMOD_T, szSetting, uDefault));
}

/*
 * read a contact setting with our default module name (Tab_SRMSG)
 */

DWORD CMimAPI::GetDword(const HANDLE hContact = 0, const char *szSetting = 0, DWORD uDefault = 0) const
{
	return((DWORD)DBGetContactSettingDword(hContact, SRMSGMOD_T, szSetting, uDefault));
}

/*
 * read a setting from module only
 */

DWORD CMimAPI::GetDword(const char *szModule, const char *szSetting, DWORD uDefault) const
{
	return((DWORD)DBGetContactSettingDword(0, szModule, szSetting, uDefault));
}

/*
 * same for bytes now
 */
int CMimAPI::GetByte(const HANDLE hContact = 0, const char *szModule = 0, const char *szSetting = 0, int uDefault = 0) const
{
	return(DBGetContactSettingByte(hContact, szModule, szSetting, uDefault));
}

int CMimAPI::GetByte(const char *szSetting = 0, int uDefault = 0) const
{
	return(DBGetContactSettingByte(0, SRMSGMOD_T, szSetting, uDefault));
}

int CMimAPI::GetByte(const HANDLE hContact = 0, const char *szSetting = 0, int uDefault = 0) const
{
	return(DBGetContactSettingByte(hContact, SRMSGMOD_T, szSetting, uDefault));
}

int CMimAPI::GetByte(const char *szModule, const char *szSetting, int uDefault) const
{
	return(DBGetContactSettingByte(0, szModule, szSetting, uDefault));
}

INT_PTR CMimAPI::GetTString(const HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv) const
{
	return(DBGetContactSettingTString(hContact, szModule, szSetting, dbv));
}

INT_PTR CMimAPI::GetString(const HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv) const
{
	return(DBGetContactSettingString(hContact, szModule, szSetting, dbv));
}

/*
 * writer functions
 */

INT_PTR CMimAPI::WriteDword(const HANDLE hContact = 0, const char *szModule = 0, const char *szSetting = 0, DWORD value = 0) const
{
	return(DBWriteContactSettingDword(hContact, szModule, szSetting, value));
}

/*
 * write non-contact setting
*/

INT_PTR CMimAPI::WriteDword(const char *szModule = 0, const char *szSetting = 0, DWORD value = 0) const
{
	return(DBWriteContactSettingDword(0, szModule, szSetting, value));
}

INT_PTR CMimAPI::WriteByte(const HANDLE hContact = 0, const char *szModule = 0, const char *szSetting = 0, BYTE value = 0) const
{
	return(DBWriteContactSettingByte(hContact, szModule, szSetting, value));
}

INT_PTR CMimAPI::WriteByte(const char *szModule = 0, const char *szSetting = 0, BYTE value = 0) const
{
	return(DBWriteContactSettingByte(0, szModule, szSetting, value));
}

INT_PTR CMimAPI::WriteTString(const HANDLE hContact, const char *szModule = 0, const char *szSetting = 0, const TCHAR *str = 0) const
{
	return(DBWriteContactSettingTString(hContact, szModule, szSetting, str));
}

void CMimAPI::GetUTFI()
{
	mir_getUTFI(&m_utfi);
}
char *CMimAPI::utf8_decode(char* str, wchar_t** ucs2) const
{
	return(m_utfi.utf8_decode(str, ucs2));
}
char *CMimAPI::utf8_decodecp(char* str, int codepage, wchar_t** ucs2 ) const
{
	return(m_utfi.utf8_decodecp(str, codepage, ucs2));
}
char *CMimAPI::utf8_encode(const char* src) const
{
	return(m_utfi.utf8_encode(src));
}

char *CMimAPI::utf8_encodecp(const char* src, int codepage) const
{
	return(m_utfi.utf8_encodecp(src, codepage));
}

char *CMimAPI::utf8_encodeW(const wchar_t* src) const
{
	return(m_utfi.utf8_encodeW(src));
}

char *CMimAPI::utf8_encodeT(const TCHAR* src) const
{
	return(m_utfi.utf8_encodeW(src));
}

TCHAR *CMimAPI::utf8_decodeT(const char* src) const
{
	return(m_utfi.utf8_decodeW(src));
}

wchar_t *CMimAPI::utf8_decodeW(const char* str) const
{
	return(m_utfi.utf8_decodeW(str));
}

/**
 * Case insensitive _tcsstr
 *
 * @param szString TCHAR *: String to be searched
 * @param szSearchFor
 *                 TCHAR *: String that should be found in szString
 *
 * @return TCHAR *: found position of szSearchFor in szString. 0 if szSearchFor was not found
 */
const TCHAR* CMimAPI::StriStr(const TCHAR *szString, const TCHAR *szSearchFor)
{
	assert(szString != 0 && szSearchFor != 0);

	if(szString && *szString) {
		if (0 == szSearchFor || 0 == *szSearchFor)
			return(szString);

		for(; *szString; ++szString) {
			if(_totupper(*szString) == _totupper(*szSearchFor)) {
				const TCHAR *h, *n;
				for(h = szString, n = szSearchFor; *h && *n; ++h, ++n) {
					if(_totupper(*h) != _totupper(*n))
						break;
				}
				if(!*n)
					return(szString);
			}
		}
		return(0);
	}
	else
		return(0);
}

int CMimAPI::pathIsAbsolute(const TCHAR *path) const
{
	if (!path || !(lstrlen(path) > 2))
		return 0;
	if ((path[1] == ':' && path[2] == '\\') || (path[0] == '\\' && path[1] == '\\'))
		return 1;
	return 0;
}

size_t CMimAPI::pathToRelative(const TCHAR *pSrc, TCHAR *pOut, const TCHAR *szBase) const
{
	const TCHAR	*tszBase = szBase ? szBase : m_szProfilePath;

	pOut[0] = 0;
	if (!pSrc || !lstrlen(pSrc) || lstrlen(pSrc) > MAX_PATH)
		return 0;
	if (!pathIsAbsolute(pSrc)) {
		mir_sntprintf(pOut, MAX_PATH, _T("%s"), pSrc);
		return lstrlen(pOut);
	} else {
		TCHAR	szTmp[MAX_PATH];

		mir_sntprintf(szTmp, SIZEOF(szTmp), _T("%s"), pSrc);
		if (StriStr(szTmp, tszBase)) {
			if(tszBase[lstrlen(tszBase) - 1] == '\\')
				mir_sntprintf(pOut, MAX_PATH, _T("%s"), pSrc + lstrlen(tszBase));
			else {
				mir_sntprintf(pOut, MAX_PATH, _T("%s"), pSrc + lstrlen(tszBase)  + 1 );
				//pOut[0]='.';
			}
			return(lstrlen(pOut));
		} else {
			mir_sntprintf(pOut, MAX_PATH, _T("%s"), pSrc);
			return(lstrlen(pOut));
		}
	}
}

/**
 * Translate a relativ path to an absolute, using the current profile
 * data directory.
 *
 * @param pSrc   TCHAR *: input path + filename (relative)
 * @param pOut   TCHAR *: the result
 * @param szBase TCHAR *: (OPTIONAL) base path for the translation. Can be 0 in which case
 *               the function will use m_szProfilePath (usually \tabSRMM below %miranda_userdata%
 *
 * @return
 */
size_t CMimAPI::pathToAbsolute(const TCHAR *pSrc, TCHAR *pOut, const TCHAR *szBase) const
{
	const TCHAR	*tszBase = szBase ? szBase : m_szProfilePath;

	pOut[0] = 0;
	if (!pSrc || !lstrlen(pSrc) || lstrlen(pSrc) > MAX_PATH)
		return 0;
	if (pathIsAbsolute(pSrc) && pSrc[0]!='.')
		mir_sntprintf(pOut, MAX_PATH, _T("%s"), pSrc);
	else if (pSrc[0]=='.')
		mir_sntprintf(pOut, MAX_PATH, _T("%s\\%s"), tszBase, pSrc + 1);
	else
		mir_sntprintf(pOut, MAX_PATH, _T("%s\\%s"), tszBase, pSrc);

	return lstrlen(pOut);
}

/*
 * window list functions
 */

void CMimAPI::BroadcastMessage(UINT msg = 0, WPARAM wParam = 0, LPARAM lParam = 0)
{
	WindowList_Broadcast(m_hMessageWindowList, msg, wParam, lParam);
}

void CMimAPI::BroadcastMessageAsync(UINT msg = 0, WPARAM wParam = 0, LPARAM lParam = 0)
{
	WindowList_BroadcastAsync(m_hMessageWindowList, msg, wParam, lParam);
}

HWND CMimAPI::FindWindow(HANDLE h = 0) const
{
	return(WindowList_Find(m_hMessageWindowList, h));
}

INT_PTR CMimAPI::AddWindow(HWND hWnd = 0, HANDLE h = 0)
{
	return(WindowList_Add(m_hMessageWindowList, hWnd, h));
}

INT_PTR CMimAPI::RemoveWindow(HWND hWnd = 0)
{
	return(WindowList_Remove(m_hMessageWindowList, hWnd));
}

int CMimAPI::FoldersPathChanged(WPARAM wParam, LPARAM lParam)
{
	return(M->foldersPathChanged());
}

void CMimAPI::configureCustomFolders()
{
	m_haveFolders = false;
	if (ServiceExists(MS_FOLDERS_REGISTER_PATH)) {
		m_hDataPath = (HANDLE)FoldersRegisterCustomPathT("TabSRMM", "Data path", const_cast<TCHAR *>(getDataPath()));
		m_hSkinsPath = (HANDLE)FoldersRegisterCustomPathT("TabSRMM", "Skins root", const_cast<TCHAR *>(getSkinPath()));
		m_hAvatarsPath = (HANDLE)FoldersRegisterCustomPathT("TabSRMM", "Saved Avatars", const_cast<TCHAR *>(getSavedAvatarPath()));
		m_hChatLogsPath = (HANDLE)FoldersRegisterCustomPathT("TabSRMM", "Group chat logs root", const_cast<TCHAR *>(getChatLogPath()));
		CGlobals::m_event_FoldersChanged = HookEvent(ME_FOLDERS_PATH_CHANGED, CMimAPI::FoldersPathChanged);
		m_haveFolders = true;
	}
	else
		m_hDataPath = m_hSkinsPath = m_hAvatarsPath = m_hChatLogsPath = 0;

	foldersPathChanged();
}

INT_PTR CMimAPI::foldersPathChanged()
{
	TCHAR szTemp[MAX_PATH + 2] = {'\0'};

	if(m_hDataPath) {
		FoldersGetCustomPathT(m_hDataPath, szTemp, MAX_PATH, const_cast<TCHAR *>(getDataPath()));
		mir_sntprintf(m_szProfilePath, MAX_PATH, _T("%s"), szTemp);

		FoldersGetCustomPathT(m_hSkinsPath, szTemp, MAX_PATH, const_cast<TCHAR *>(getSkinPath()));
		mir_sntprintf(m_szSkinsPath, MAX_PATH - 1, _T("%s"), szTemp);

		/*
		 * make sure skins root path always ends with a '\' - this is assumed by the skin
		 * selection code.
		 */

		Utils::ensureTralingBackslash(m_szSkinsPath);

		FoldersGetCustomPathT(m_hAvatarsPath, szTemp, MAX_PATH, const_cast<TCHAR *>(getSavedAvatarPath()));
		mir_sntprintf(m_szSavedAvatarsPath, MAX_PATH, _T("%s"), szTemp);

		FoldersGetCustomPathT(m_hChatLogsPath, szTemp, MAX_PATH, const_cast<TCHAR *>(getChatLogPath()));
		mir_sntprintf(m_szChatLogsPath, MAX_PATH, _T("%s"), szTemp);

		Utils::ensureTralingBackslash(m_szChatLogsPath);
	}
	CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)m_szProfilePath);
	CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)m_szSkinsPath);
	CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)m_szSavedAvatarsPath);
	CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)m_szChatLogsPath);

#if defined(_FOLDER_LOCKING)
	mir_sntprintf(szTemp, MAX_PATH, L"%sfolder.lck", m_szChatLogsPath);

	if(m_hChatLogLock != INVALID_HANDLE_VALUE)
		CloseHandle(m_hChatLogLock);

	m_hChatLogLock = CreateFile(szTemp, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_HIDDEN, 0);
#endif

	Skin->extractSkinsAndLogo(true);
	Skin->setupAeroSkins();
	return 0;
}

const TCHAR* CMimAPI::getUserDir()
{
	if(m_userDir[0] == 0) {
		wchar_t*	userdata = ::Utils_ReplaceVarsT(L"%miranda_userdata%");
		mir_sntprintf(m_userDir, MAX_PATH, userdata);
		Utils::ensureTralingBackslash(m_userDir);
		mir_free(userdata); 
	}
	return(m_userDir);
}

void CMimAPI::InitPaths()
{
	m_szProfilePath[0] = 0;
	m_szSkinsPath[0] = 0;
	m_szSavedAvatarsPath[0] = 0;

	const TCHAR *szUserdataDir = getUserDir();

	mir_sntprintf(m_szProfilePath, MAX_PATH, _T("%stabSRMM"), szUserdataDir);
	mir_sntprintf(m_szChatLogsPath, MAX_PATH, _T("%sLogs\\"), szUserdataDir);

	mir_sntprintf(m_szSkinsPath, MAX_PATH, _T("%s\\skins\\"), m_szProfilePath);
	mir_sntprintf(m_szSavedAvatarsPath, MAX_PATH, _T("%s\\Saved Contact Pictures"), m_szProfilePath);
}

bool CMimAPI::getAeroState()
{
	BOOL result = FALSE;
	m_isAero = m_DwmActive = false;
	if(IsWinVerVistaPlus()) {
		m_DwmActive = (m_pfnDwmIsCompositionEnabled && (m_pfnDwmIsCompositionEnabled(&result) == S_OK) && result) ? true : false;
		m_isAero = (CSkin::m_skinEnabled == false) && GetByte("useAero", 1) && CSkin::m_fAeroSkinsValid && m_DwmActive;

	}
	m_isVsThemed = (m_VsAPI && m_pfnIsThemeActive && m_pfnIsThemeActive());
	return(m_isAero);
}

/**
 * Initialize various Win32 API functions which are not common to all versions of Windows.
 * We have to work with functions pointers here.
 */

void CMimAPI::InitAPI()
{
	m_hUxTheme = 0;
	m_VsAPI = false;

	HMODULE hDLL = GetModuleHandleA("user32");
	m_pSetLayeredWindowAttributes = (PSLWA) GetProcAddress(hDLL, "SetLayeredWindowAttributes");
	m_MyFlashWindowEx = (PFWEX) GetProcAddress(hDLL, "FlashWindowEx");

	m_MyAlphaBlend = (PAB) GetProcAddress(GetModuleHandleA("gdi32"), "GdiAlphaBlend");
	if (m_MyAlphaBlend == 0)
		m_MyAlphaBlend = (PAB) GetProcAddress(LoadLibraryA("msimg32.dll"), "AlphaBlend");

	m_MyGradientFill = (PGF) GetProcAddress(GetModuleHandleA("gdi32"), "GdiGradientFill");
	if (m_MyGradientFill == 0)
		m_MyGradientFill = (PGF) GetProcAddress(GetModuleHandleA("msimg32"), "GradientFill");

	m_pfnMonitorFromWindow = (MMFW)GetProcAddress(GetModuleHandleA("USER32"), "MonitorFromWindow");
	m_pfnGetMonitorInfoA = (GMIA)GetProcAddress(GetModuleHandleA("USER32"), "GetMonitorInfoA");

	if (IsWinVerXPPlus()) {
		if ((m_hUxTheme = Utils::loadSystemLibrary(L"\\uxtheme.dll")) != 0) {
			m_pfnIsThemeActive = (PITA)GetProcAddress(m_hUxTheme, "IsThemeActive");
			m_pfnOpenThemeData = (POTD)GetProcAddress(m_hUxTheme, "OpenThemeData");
			m_pfnDrawThemeBackground = (PDTB)GetProcAddress(m_hUxTheme, "DrawThemeBackground");
			m_pfnCloseThemeData = (PCTD)GetProcAddress(m_hUxTheme, "CloseThemeData");
			m_pfnDrawThemeText = (PDTT)GetProcAddress(m_hUxTheme, "DrawThemeText");
			m_pfnIsThemeBackgroundPartiallyTransparent = (PITBPT)GetProcAddress(m_hUxTheme, "IsThemeBackgroundPartiallyTransparent");
			m_pfnDrawThemeParentBackground = (PDTPB)GetProcAddress(m_hUxTheme, "DrawThemeParentBackground");
			m_pfnGetThemeBackgroundContentRect = (PGTBCR)GetProcAddress(m_hUxTheme, "GetThemeBackgroundContentRect");
			m_pfnEnableThemeDialogTexture = (ETDT)GetProcAddress(m_hUxTheme, "EnableThemeDialogTexture");

			if (m_pfnIsThemeActive != 0 && m_pfnOpenThemeData != 0 && m_pfnDrawThemeBackground != 0 && m_pfnCloseThemeData != 0
				&& m_pfnDrawThemeText != 0 && m_pfnIsThemeBackgroundPartiallyTransparent != 0 && m_pfnDrawThemeParentBackground != 0
				&& m_pfnGetThemeBackgroundContentRect != 0) {
				m_VsAPI = true;
			}
		}
	}

	/*
	 * vista+ DWM API
	 */

	m_hDwmApi = 0;
	if (IsWinVerVistaPlus())  {
	    m_hDwmApi = Utils::loadSystemLibrary(L"\\dwmapi.dll");
	    if (m_hDwmApi)  {
            m_pfnDwmExtendFrameIntoClientArea = (DEFICA)GetProcAddress(m_hDwmApi,"DwmExtendFrameIntoClientArea");
            m_pfnDwmIsCompositionEnabled = (DICE)GetProcAddress(m_hDwmApi,"DwmIsCompositionEnabled");
			m_pfnDwmRegisterThumbnail = (DRT)GetProcAddress(m_hDwmApi, "DwmRegisterThumbnail");
			m_pfnDwmBlurBehindWindow = (BBW)GetProcAddress(m_hDwmApi, "DwmEnableBlurBehindWindow");
			m_pfnDwmGetColorizationColor = (DGC)GetProcAddress(m_hDwmApi, "DwmGetColorizationColor");
			m_pfnDwmInvalidateIconicBitmaps = (DWMIIB)GetProcAddress(m_hDwmApi, "DwmInvalidateIconicBitmaps");
			m_pfnDwmSetWindowAttribute = (DWMSWA)GetProcAddress(m_hDwmApi, "DwmSetWindowAttribute");
			m_pfnDwmUpdateThumbnailProperties = (DWMUT)GetProcAddress(m_hDwmApi, "DwmUpdateThumbnailProperties");
			m_pfnDwmUnregisterThumbnail = (DURT)GetProcAddress(m_hDwmApi, "DwmUnregisterThumbnail");
			m_pfnDwmSetIconicThumbnail = (DSIT)GetProcAddress(m_hDwmApi, "DwmSetIconicThumbnail");
			m_pfnDwmSetIconicLivePreviewBitmap = (DSILP)GetProcAddress(m_hDwmApi, "DwmSetIconicLivePreviewBitmap");
	    }
		/*
		 * additional uxtheme APIs (Vista+)
		 */
		if(m_hUxTheme) {
			m_pfnDrawThemeTextEx = (PDTTE)GetProcAddress(m_hUxTheme, "DrawThemeTextEx");
			m_pfnBeginBufferedPaint = (BBP)GetProcAddress(m_hUxTheme, "BeginBufferedPaint");
			m_pfnEndBufferedPaint = (EBP)GetProcAddress(m_hUxTheme, "EndBufferedPaint");
			m_pfnBufferedPaintInit = (BPI)GetProcAddress(m_hUxTheme, "BufferedPaintInit");
			m_pfnBufferedPaintUninit = (BPU)GetProcAddress(m_hUxTheme, "BufferedPaintUnInit");
			m_pfnBufferedPaintSetAlpha = (BPSA)GetProcAddress(m_hUxTheme, "BufferedPaintSetAlpha");
			m_haveBufferedPaint = (m_pfnBeginBufferedPaint != 0 && m_pfnEndBufferedPaint != 0) ? true : false;
			if(m_haveBufferedPaint)
				m_pfnBufferedPaintInit();
		}
		m_pfnGetLocaleInfoEx = (GLIX)GetProcAddress(GetModuleHandleA("kernel32"), "GetLocaleInfoEx");
    }
	else
		m_haveBufferedPaint = false;
}

/**
 * hook subscriber function for incoming message typing events
 */

int CMimAPI::TypingMessage(WPARAM wParam, LPARAM lParam)
{
	HWND			hwnd = 0;
	int				issplit = 1, foundWin = 0, preTyping = 0;
	struct			TContainerData *pContainer = NULL;
	BOOL			fShowOnClist = TRUE;

	if(wParam) {

		if ((hwnd = M->FindWindow((HANDLE) wParam)) && M->GetByte(SRMSGMOD, SRMSGSET_SHOWTYPING, SRMSGDEFSET_SHOWTYPING))
			preTyping = SendMessage(hwnd, DM_TYPING, 0, lParam);

		if (hwnd && IsWindowVisible(hwnd))
			foundWin = MessageWindowOpened(0, (LPARAM)hwnd);
		else
			foundWin = 0;


		if(hwnd) {
			SendMessage(hwnd, DM_QUERYCONTAINER, 0, (LPARAM)&pContainer);
			if(pContainer == NULL)
				return 0;					// should never happen
		}

		if(M->GetByte(SRMSGMOD, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST)) {
			if(!hwnd && !M->GetByte(SRMSGMOD, SRMSGSET_SHOWTYPINGNOWINOPEN, 1))
				fShowOnClist = FALSE;
			if(hwnd && !M->GetByte(SRMSGMOD, SRMSGSET_SHOWTYPINGWINOPEN, 1))
				fShowOnClist = FALSE;
		}
		else
			fShowOnClist = FALSE;

		if((!foundWin || !(pContainer->dwFlags&CNT_NOSOUND)) && preTyping != (lParam != 0)){
			if (lParam)
				SkinPlaySound("TNStart");
			else
				SkinPlaySound("TNStop");
		}

		if(M->GetByte(SRMSGMOD, "ShowTypingPopup", 0)) {
			BOOL	fShow = FALSE;
			int		iMode = M->GetByte("MTN_PopupMode", 0);

			switch(iMode) {
				case 0:
					fShow = TRUE;
					break;
				case 1:
					if(!foundWin || !(pContainer && pContainer->hwndActive == hwnd && GetForegroundWindow() == pContainer->hwnd))
						fShow = TRUE;
					break;
				case 2:
					if(hwnd == 0)
						fShow = TRUE;
					else {
						if(PluginConfig.m_HideOnClose) {
							struct	TContainerData *pContainer = 0;
							SendMessage(hwnd, DM_QUERYCONTAINER, 0, (LPARAM)&pContainer);
							if(pContainer && pContainer->fHidden)
								fShow = TRUE;
						}
					}
					break;
			}
			if(fShow)
				TN_TypingMessage(wParam, lParam);
		}

		if ((int) lParam) {
			TCHAR szTip[256];

			_sntprintf(szTip, SIZEOF(szTip), CTranslator::get(CTranslator::GEN_MTN_STARTWITHNICK), (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, wParam, GCDNF_TCHAR));
			if (fShowOnClist && ServiceExists(MS_CLIST_SYSTRAY_NOTIFY) && M->GetByte(SRMSGMOD, "ShowTypingBalloon", 0)) {
				MIRANDASYSTRAYNOTIFY tn;
				tn.szProto = NULL;
				tn.cbSize = sizeof(tn);
				tn.tszInfoTitle = const_cast<TCHAR *>(CTranslator::get(CTranslator::GEN_MTN_TTITLE));
				tn.tszInfo = szTip;
				tn.dwInfoFlags = NIIF_INFO | NIIF_INTERN_UNICODE;
				tn.uTimeout = 1000 * 4;
				CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM) & tn);
			}
			if(fShowOnClist) {
				CLISTEVENT cle;

				ZeroMemory(&cle, sizeof(cle));
				cle.cbSize = sizeof(cle);
				cle.hContact = (HANDLE) wParam;
				cle.hDbEvent = (HANDLE) 1;
				cle.flags = CLEF_ONLYAFEW | CLEF_TCHAR;
				cle.hIcon = PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING];
				cle.pszService = "SRMsg/TypingMessage";
				cle.ptszTooltip = szTip;
				CallServiceSync(MS_CLIST_REMOVEEVENT, wParam, (LPARAM) 1);
				CallServiceSync(MS_CLIST_ADDEVENT, wParam, (LPARAM) & cle);
			}
		}
	}
	return 0;
}

/**
 * this is the global ack dispatcher. It handles both ACKTYPE_MESSAGE and ACKTYPE_AVATAR events
 * for ACKTYPE_MESSAGE it searches the corresponding send job in the queue and, if found, dispatches
 * it to the owners window
 *
 * ACKTYPE_AVATAR no longer handled here, because we have avs services now.
 */

int CMimAPI::ProtoAck(WPARAM wParam, LPARAM lParam)
{
	ACKDATA *pAck = (ACKDATA *) lParam;
	HWND hwndDlg = 0;
	int i = 0, j, iFound = SendQueue::NR_SENDJOBS;

	if (lParam == 0)
		return 0;

	SendJob *jobs = sendQueue->getJobByIndex(0);

	if (pAck->type == ACKTYPE_MESSAGE) {
		for (j = 0; j < SendQueue::NR_SENDJOBS; j++) {
			if (pAck->hProcess == jobs[j].hSendId && pAck->hContact == jobs[j].hOwner) {
				TWindowData *dat = jobs[j].hwndOwner ? (TWindowData *)GetWindowLongPtr(jobs[j].hwndOwner, GWLP_USERDATA) : NULL;
				if (dat) {
					if (dat->hContact == jobs[j].hOwner) {
						iFound = j;
						break;
					}
				} else {      // ack message w/o an open window...
					sendQueue->ackMessage(NULL, (WPARAM)MAKELONG(j, i), lParam);
					return 0;
				}
			}
			if (iFound == SendQueue::NR_SENDJOBS)          // no mathing entry found in this queue entry.. continue
				continue;
			else
				break;
		}
		if (iFound == SendQueue::NR_SENDJOBS) {             // no matching send info found in the queue
			sendLater->processAck(pAck);											   //
			return 0;									   // try to find the process handle in the list of open send later jobs
		} else {                                  // the job was found
			SendMessage(jobs[iFound].hwndOwner, HM_EVENTSENT, (WPARAM)MAKELONG(iFound, i), lParam);
			return 0;
		}
	}
	return 0;
}

int CMimAPI::PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if ( hContact ) {
		char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);

		CLISTMENUITEM clmi = {0};
		clmi.cbSize = sizeof( CLISTMENUITEM );
		clmi.flags = CMIM_FLAGS | CMIF_DEFAULT | CMIF_HIDDEN;

		if ( szProto ) {
			// leave this menu item hidden for chats
			if ( !M->GetByte(hContact, szProto, "ChatRoom", 0 ))
				if ( CallProtoService( szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND )
					clmi.flags &= ~CMIF_HIDDEN;
		}

		CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )PluginConfig.m_hMenuItem, ( LPARAM )&clmi );
	}
	return 0;
}

/**
 * this handler is called first in the message window chain - it will handle events for which a message window
 * is already open. if not, it will do nothing and the 2nd handler (MessageEventAdded) will perform all
 * the needed actions.
 *
 * this handler POSTs the event to the message window procedure - so it is fast and can exit quickly which will
 * improve the overall responsiveness when receiving messages.
 */

int CMimAPI::DispatchNewEvent(WPARAM wParam, LPARAM lParam)
{
	if (wParam) {
		HWND h = M->FindWindow((HANDLE)wParam);
		if (h)
			PostMessage(h, HM_DBEVENTADDED, wParam, lParam);            // was SENDMESSAGE !!! XXX
	}
	return 0;
}

/**
 * Message event added is called when a new message is added to the database
 * if no session is open for the contact, this function will determine if and how a new message
 * session (tab) must be created.
 *
 * if a session is already created, it just does nothing and DispatchNewEvent() will take care.
 */

int CMimAPI::MessageEventAdded(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd;
	CLISTEVENT cle;
	DBEVENTINFO dbei;
	BYTE bAutoPopup = FALSE, bAutoCreate = FALSE, bAutoContainer = FALSE, bAllowAutoCreate = 0;
	struct TContainerData *pContainer = 0;
	TCHAR szName[CONTAINER_NAMELEN + 1];
	DWORD dwStatusMask = 0;
	struct TWindowData *mwdat=NULL;

	ZeroMemory(&dbei, sizeof(dbei));
	dbei.cbSize = sizeof(dbei);
	dbei.cbBlob = 0;
	CallService(MS_DB_EVENT_GET, lParam, (LPARAM) & dbei);

	hwnd = M->FindWindow((HANDLE) wParam);

	if (dbei.flags & DBEF_SENT || !(dbei.eventType == EVENTTYPE_MESSAGE || dbei.eventType == EVENTTYPE_FILE) || dbei.flags & DBEF_READ)
		return 0;

	CallServiceSync(MS_CLIST_REMOVEEVENT, wParam, (LPARAM) 1);
		//MaD: hide on close mod, simulating standard behavior for hidden container
	if (hwnd) {
		struct TContainerData *pTargetContainer = 0;
		WINDOWPLACEMENT wp={0};
		wp.length = sizeof(wp);
		SendMessage(hwnd, DM_QUERYCONTAINER, 0, (LPARAM)&pTargetContainer);

		if(pTargetContainer && PluginConfig.m_HideOnClose && !IsWindowVisible(pTargetContainer->hwnd))	{
			GetWindowPlacement(pTargetContainer->hwnd, &wp);
			GetContainerNameForContact((HANDLE) wParam, szName, CONTAINER_NAMELEN);

			bAutoPopup = M->GetByte(SRMSGSET_AUTOPOPUP, SRMSGDEFSET_AUTOPOPUP);
			bAutoCreate = M->GetByte("autotabs", 1);
			bAutoContainer = M->GetByte("autocontainer", 1);
			dwStatusMask = M->GetDword("autopopupmask", -1);

			bAllowAutoCreate = FALSE;

			if (bAutoPopup || bAutoCreate) {
				BOOL bActivate = TRUE, bPopup = TRUE;
				if(bAutoPopup) {
					if(wp.showCmd == SW_SHOWMAXIMIZED)
						ShowWindow(pTargetContainer->hwnd, SW_SHOWMAXIMIZED);
					else
						ShowWindow(pTargetContainer->hwnd, SW_SHOWNOACTIVATE);
					return 0;
				}
				else {
					bActivate = FALSE;
					bPopup = (BOOL) M->GetByte("cpopup", 0);
					pContainer = FindContainerByName(szName);
					if (pContainer != NULL) {
						if(bAutoContainer) {
							ShowWindow(pTargetContainer->hwnd, SW_SHOWMINNOACTIVE);
							return 0;
						}
						else goto nowindowcreate;
					}
					else {
						if(bAutoContainer) {
							ShowWindow(pTargetContainer->hwnd, SW_SHOWMINNOACTIVE);
							return 0;
						}
					}
				}
			}
		}
		else
			return 0;
	} else {
		if(dbei.eventType == EVENTTYPE_FILE) {
			tabSRMM_ShowPopup(wParam, lParam, dbei.eventType, 0, 0, 0, dbei.szModule, 0);
			return(0);
		}
	}

	/*
	 * if no window is open, we are not interested in anything else but unread message events
	 */

	/* new message */
	if (!nen_options.iNoSounds)
		SkinPlaySound("AlertMsg");

	if (nen_options.iNoAutoPopup)
		goto nowindowcreate;

	GetContainerNameForContact((HANDLE) wParam, szName, CONTAINER_NAMELEN);

	bAutoPopup = M->GetByte(SRMSGSET_AUTOPOPUP, SRMSGDEFSET_AUTOPOPUP);
	bAutoCreate = M->GetByte("autotabs", 1);
	bAutoContainer = M->GetByte("autocontainer", 1);
	dwStatusMask = M->GetDword("autopopupmask", -1);

	bAllowAutoCreate = FALSE;

	if (dwStatusMask == -1)
		bAllowAutoCreate = TRUE;
	else {
		char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)wParam, 0);
		DWORD dwStatus = 0;

		if (PluginConfig.g_MetaContactsAvail && szProto && !strcmp(szProto, (char *)CallService(MS_MC_GETPROTOCOLNAME, 0, 0))) {
			HANDLE hSubconttact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, wParam, 0);

			szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hSubconttact, 0);
		}
		if (szProto) {
			dwStatus = (DWORD)CallProtoService(szProto, PS_GETSTATUS, 0, 0);
			if (dwStatus == 0 || dwStatus <= ID_STATUS_OFFLINE || ((1 << (dwStatus - ID_STATUS_ONLINE)) & dwStatusMask))           // should never happen, but...
				bAllowAutoCreate = TRUE;
		}
	}
	if (bAllowAutoCreate && (bAutoPopup || bAutoCreate)) {
		BOOL bActivate = TRUE, bPopup = TRUE;
		if (bAutoPopup) {
			bActivate = bPopup = TRUE;
			if ((pContainer = FindContainerByName(szName)) == NULL)
				pContainer = CreateContainer(szName, FALSE, (HANDLE)wParam);
			CreateNewTabForContact(pContainer, (HANDLE) wParam, 0, NULL, bActivate, bPopup, FALSE, 0);
			return 0;
		} else {
			bActivate = FALSE;
			bPopup = (BOOL) M->GetByte("cpopup", 0);
			pContainer = FindContainerByName(szName);
			if (pContainer != NULL) {
				//if ((IsIconic(pContainer->hwnd)) && PluginConfig.haveAutoSwitch())
				//	pContainer->dwFlags |= CNT_DEFERREDTABSELECT;
				if (M->GetByte("limittabs", 0) &&  !wcsncmp(pContainer->szName, L"default", 6)) {
					if ((pContainer = FindMatchingContainer(L"default", (HANDLE)wParam)) != NULL) {
						CreateNewTabForContact(pContainer, (HANDLE) wParam, 0, NULL, bActivate, bPopup, TRUE, (HANDLE)lParam);
						return 0;
					} else if (bAutoContainer) {
						pContainer = CreateContainer(szName, CNT_CREATEFLAG_MINIMIZED, (HANDLE)wParam);         // 2 means create minimized, don't popup...
						CreateNewTabForContact(pContainer, (HANDLE) wParam,  0, NULL, bActivate, bPopup, TRUE, (HANDLE)lParam);
						SendMessageW(pContainer->hwnd, WM_SIZE, 0, 0);
						return 0;
					}
				} else {
					CreateNewTabForContact(pContainer, (HANDLE) wParam, 0, NULL, bActivate, bPopup, TRUE, (HANDLE)lParam);
					return 0;
				}

			} else {
				if (bAutoContainer) {
					pContainer = CreateContainer(szName, CNT_CREATEFLAG_MINIMIZED, (HANDLE)wParam);         // 2 means create minimized, don't popup...
					CreateNewTabForContact(pContainer, (HANDLE) wParam,  0, NULL, bActivate, bPopup, TRUE, (HANDLE)lParam);
					SendMessageW(pContainer->hwnd, WM_SIZE, 0, 0);
					return 0;
				}
			}
		}
	}

	/*
	 * for tray support, we add the event to the tray menu. otherwise we send it back to
	 * the contact list for flashing
	 */
nowindowcreate:
	if (!(dbei.flags & DBEF_READ)) {
		UpdateTrayMenu(0, 0, dbei.szModule, NULL, (HANDLE)wParam, 1);
		if (!nen_options.bTraySupport) {
			TCHAR toolTip[256], *contactName;
			ZeroMemory(&cle, sizeof(cle));
			cle.cbSize = sizeof(cle);
			cle.hContact = (HANDLE) wParam;
			cle.hDbEvent = (HANDLE) lParam;
			cle.flags = CLEF_TCHAR;
			cle.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
			cle.pszService = "SRMsg/ReadMessage";
			contactName = (TCHAR*) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, wParam, GCDNF_TCHAR);
			mir_sntprintf(toolTip, SIZEOF(toolTip), CTranslator::get(CTranslator::GEN_MSG_TTITLE), contactName);
			cle.ptszTooltip = toolTip;
			CallService(MS_CLIST_ADDEVENT, 0, (LPARAM) & cle);
		}
		tabSRMM_ShowPopup(wParam, lParam, dbei.eventType, 0, 0, 0, dbei.szModule, 0);
	}
	return 0;
}

CMimAPI *M = 0;
FI_INTERFACE *FIF = 0;
