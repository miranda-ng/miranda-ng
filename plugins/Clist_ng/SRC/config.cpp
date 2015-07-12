
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
 * part of clist_ng plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: config.cpp 137 2010-10-16 21:03:23Z silvercircle $
 *
 * plugin configuration and low level API handling
 *
 */

#include <commonheaders.h>

TCluiData		cfg::dat = {0};
ClcData*		cfg::clcdat = 0;
TExtraCache* 	cfg::eCache = 0;
int 			cfg::nextCacheEntry = 0, cfg::maxCacheEntry = 0;
bool			cfg::isAero = true;
bool			cfg::fBaseSkinValid = true;
FI_INTERFACE*	cfg::fif = 0;
CRITICAL_SECTION cfg::cachecs = {0};

bool			cfg::shutDown = false;
wchar_t			cfg::szProfileDir[MAX_PATH] = L"\0";
int				cfg::maxStatus = ID_STATUS_OFFLINE;
CLUIFrames		*cfg::FrameMgr = nullptr;


pfnAlphaBlend_t						Api::pfnAlphaBlend = 0;
PGF									Api::pfnGradientFill = 0;
pfnGetTickCount64_t					Api::pfnGetTickCount64 = 0;

TSysConfig							Api::sysConfig = {0};
TSysState							Api::sysState = {0};

pfnIsThemeActive_t 							Api::pfnIsThemeActive = 0;
pfnOpenThemeData_t 							Api::pfnOpenThemeData = 0;
pfnDrawThemeBackground_t 					Api::pfnDrawThemeBackground = 0;
pfnCloseThemeData_t			 				Api::pfnCloseThemeData = 0;
pfnDrawThemeText_t 							Api::pfnDrawThemeText = 0;
pfnDrawThemeTextEx_t						Api::pfnDrawThemeTextEx = 0;
pfnIsThemeBackgroundPartiallyTransparent_t 	Api::pfnIsThemeBackgroundPartiallyTransparent = 0;
pfnDrawThemeParentBackground_t 	 			Api::pfnDrawThemeParentBackground = 0;
pfnGetThemeBackgroundContentRect_t 			Api::pfnGetThemeBackgroundContentRect = 0;
pfnEnableThemeDialogTexture_t 				Api::pfnEnableThemeDialogTexture = 0;

pfnDwmExtendFrameIntoClientArea_t			Api::pfnDwmExtendFrameIntoClientArea = 0;
pfnDwmIsCompositionEnabled_t				Api::pfnDwmIsCompositionEnabled = 0;

pfnBufferedPaintInit_t						Api::pfnBufferedPaintInit = 0;
pfnBufferedPaintUninit_t					Api::pfnBufferedPaintUninit = 0;

pfnBeginBufferedPaint_t						Api::pfnBeginBufferedPaint = 0;
pfnEndBufferedPaint_t						Api::pfnEndBufferedPaint = 0;
pfnBufferedPaintSetAlpha_t					Api::pfnBufferedPaintSetAlpha = 0;
pfnBufferedPaintClear_t						Api::pfnBufferedPaintClear = 0;
pfnGetBufferedPaintBits_t					Api::pfnGetBufferedPaintBits = 0;

pfnDwmGetColorizationColor_t				Api::pfnDwmGetColorizationColor = 0;
pfnDwmBlurBehindWindow_t					Api::pfnDwmBlurBehindWindow = 0;

EXCEPTION_RECORD Api::exRecord = {0};
CONTEXT		 	 Api::exCtx = {0};
LRESULT			 Api::exLastResult = 0;
char			 Api::exSzFile[MAX_PATH] = "\0";
wchar_t			 Api::exReason[256] = L"\0";
int				 Api::exLine = 0;
bool			 Api::exAllowContinue = false;
HMODULE			 Api::hUxTheme = 0, Api::hDwm = 0;

void cfg::initCache()
{
	InitializeCriticalSection(&cachecs);
}

DWORD cfg::getDword(const MCONTACT hContact = 0, const char *szModule = 0, const char *szSetting = 0, DWORD uDefault = 0)
{
	return(db_get_dw(hContact, szModule, szSetting, uDefault));
}

/*
 * read a setting from our default module (Tab_SRMSG)
 */

DWORD cfg::getDword(const char *szSetting = 0, DWORD uDefault = 0)
{
	return(db_get_dw(0, DEFAULT_MODULE, szSetting, uDefault));
}

/*
 * read a setting from module only
 */

DWORD cfg::getDword(const char *szModule, const char *szSetting, DWORD uDefault)
{
	return(db_get_dw(0, szModule, szSetting, uDefault));
}


WORD cfg::getWord(const MCONTACT hContact = 0, const char *szModule = 0, const char *szSetting = 0, WORD uDefault = 0)
{
	return(db_get_w(hContact, szModule, szSetting, uDefault));
}

/*
 * read a setting from our default module (Tab_SRMSG)
 */

WORD cfg::getWord(const char *szSetting = 0, WORD uDefault = 0)
{
	return(db_get_w(0, DEFAULT_MODULE, szSetting, uDefault));
}

/*
 * read a setting from module only
 */

WORD cfg::getWord(const char *szModule, const char *szSetting, WORD uDefault)
{
	return(db_get_w(0, szModule, szSetting, uDefault));
}

/*
 * same for bytes now
 */
int cfg::getByte(const MCONTACT hContact = 0, const char *szModule = 0, const char *szSetting = 0, int uDefault = 0)
{
	return(db_get_b(hContact, szModule, szSetting, uDefault));
}

int cfg::getByte(const char *szSetting = 0, int uDefault = 0)
{
	return(db_get_b(0, DEFAULT_MODULE, szSetting, uDefault));
}

int cfg::getByte(const char *szModule, const char *szSetting, int uDefault)
{
	return(db_get_b(0, szModule, szSetting, uDefault));
}

INT_PTR cfg::getTString(const MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	return(db_get_ws(hContact, szModule, szSetting, dbv));
}

INT_PTR cfg::getString(const MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	return(db_get_s(hContact, szModule, szSetting, dbv));
}

/*
 * writer functions
 */

INT_PTR cfg::writeDword(const MCONTACT hContact = 0, const char *szModule = 0, const char *szSetting = 0, DWORD value = 0)
{
	return(db_set_dw(hContact, szModule, szSetting, value));
}

INT_PTR cfg::writeDword(const char *szModule = 0, const char *szSetting = 0, DWORD value = 0)
{
	return(db_set_dw(0, szModule, szSetting, value));
}

INT_PTR cfg::writeWord(const MCONTACT hContact = 0, const char *szModule = 0, const char *szSetting = 0, WORD value = 0)
{
	return(db_set_w(hContact, szModule, szSetting, value));
}

INT_PTR cfg::writeWord(const char *szModule = 0, const char *szSetting = 0, WORD value = 0)
{
	return(db_set_w(0, szModule, szSetting, value));
}

INT_PTR cfg::writeByte(const MCONTACT hContact = 0, const char *szModule = 0, const char *szSetting = 0, BYTE value = 0)
{
	return(db_set_b(hContact, szModule, szSetting, value));
}

INT_PTR cfg::writeByte(const char *szModule = 0, const char *szSetting = 0, BYTE value = 0)
{
	return(db_set_b(0, szModule, szSetting, value));
}

INT_PTR cfg::writeTString(const MCONTACT hContact, const char *szModule = 0, const char *szSetting = 0, const wchar_t *str = 0)
{
	return(db_set_ws(hContact, szModule, szSetting, str));
}

INT_PTR cfg::writeString(const MCONTACT hContact, const char *szModule = 0, const char *szSetting = 0, const char *str = 0)
{
	return(db_set_s(hContact, szModule, szSetting, str));
}

int cfg::getCache(const MCONTACT hContact, const char *szProto)
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
		cfg::eCache[nextCacheEntry].dwCFlags = 0;
		cfg::eCache[nextCacheEntry].dwXMask = CalcXMask(hContact);
		GetCachedStatusMsg(nextCacheEntry, const_cast<char *>(szProto));
		cfg::eCache[nextCacheEntry].dwLastMsgTime = INTSORT_GetLastMsgTime(hContact);
		iFound = nextCacheEntry++;
		LeaveCriticalSection(&cachecs);
	}
	return iFound;
}

static struct {
	UINT 	id;
	TCHAR*	name;
} _tagFSINFO[] = {
	FONTID_CONTACTS, LPGENT("Standard contacts"),
	FONTID_INVIS, LPGENT("Online contacts to whom you have a different visibility"),
	FONTID_OFFLINE, LPGENT("Offline contacts"),
	FONTID_OFFINVIS, LPGENT("Offline contacts to whom you have a different visibility"),
	FONTID_NOTONLIST, LPGENT("Contacts which are 'not on list'"),
	FONTID_GROUPS, LPGENT("Groups"),
	FONTID_GROUPCOUNTS, LPGENT("Group member counts"),
	FONTID_DIVIDERS, LPGENT("Dividers"),
	FONTID_STATUS, LPGENT("Status mode"),
	FONTID_FRAMETITLE, LPGENT("Frame titles"),
	FONTID_EVENTAREA, LPGENT("Event area"),
	FONTID_TIMESTAMP, LPGENT("Contact list local time"),
	0, NULL
};

struct ColorOptionsList {
	int			order;
	TCHAR*		tszName;
	char*		szSetting;
	COLORREF 	def;
};

/*
 * note: bits 24-31 in default color indicates that color is a system color index
 * (GetSysColor(default_color & 0x00ffffff)), not a rgb value.
 */
static ColorOptionsList _clrs[] = {
	0, L"List background", "BkColour", COLOR_WINDOW | 0xff000000,
	1, L"Group header background", "BkColourGroups", COLOR_3DFACE | 0xff000000,
	2, L"Selected text", "SelTextColour", COLOR_HIGHLIGHTTEXT | 0xff000000,
	3, L"Hottrack text", "HotTextColour", COLOR_HOTLIGHT | 0xff000000,
	4, L"Quicksearch text", "QuickSearchColour", CLCDEFAULT_QUICKSEARCHCOLOUR,
	5, L"Frame title background", "BkFrameTitles", COLOR_3DFACE | 0xff000000,
	6, L"Event area background", "BkEventAera", COLOR_WINDOW | 0xff000000
};

void cfg::FS_RegisterFonts()
{
	ColourIDT 	colourid;
	FontIDT    	fid = {0};
	char 		szTemp[50];
	DBVARIANT 	dbv;
	int 		j = 0;

	fid.cbSize = sizeof(fid);
	wcsncpy(fid.group, L"Contact List", _countof(fid.group));
	strncpy(fid.dbSettingsGroup, "CLC", 5);
	fid.flags = FIDF_DEFAULTVALID | FIDF_ALLOWEFFECTS | FIDF_APPENDNAME | FIDF_SAVEPOINTSIZE;
	wcsncpy(fid.backgroundGroup, L"Contact List", _countof(fid.backgroundGroup));
	while(_tagFSINFO[j].name != 0) {
		if(FONTID_EVENTAREA == _tagFSINFO[j].id)
			wcsncpy(fid.backgroundName, L"Event area background", _countof(fid.backgroundName));
		else if(FONTID_FRAMETITLE == _tagFSINFO[j].id)
			wcsncpy(fid.backgroundName, L"Frame title background", _countof(fid.backgroundName));
		else if(FONTID_GROUPCOUNTS == _tagFSINFO[j].id || FONTID_GROUPS == _tagFSINFO[j].id)
			_tcsncpy(fid.backgroundName, L"Group header background", _countof(fid.backgroundName));
		else
			wcsncpy(fid.backgroundName, L"List background", _countof(fid.backgroundName));

		mir_snprintf(szTemp, sizeof(szTemp), "Font%d", _tagFSINFO[j].id);
		strncpy(fid.prefix, szTemp, sizeof(fid.prefix));
		fid.order = _tagFSINFO[j].id;
		wcsncpy(fid.name, _tagFSINFO[j].name, 60);
		_snprintf(szTemp, sizeof(szTemp), "Font%dCol", _tagFSINFO[j].id);
		fid.deffontsettings.colour = (COLORREF)cfg::getDword("CLC", szTemp, GetSysColor(COLOR_WINDOWTEXT));

		_snprintf(szTemp, sizeof(szTemp), "Font%dSize", _tagFSINFO[j].id);
		fid.deffontsettings.size = (BYTE)cfg::getByte("CLC", szTemp, 8);

		_snprintf(szTemp, sizeof(szTemp), "Font%dSty", _tagFSINFO[j].id);
		fid.deffontsettings.style = cfg::getByte("CLC", szTemp, 0);
		_snprintf(szTemp, sizeof(szTemp), "Font%dSet", _tagFSINFO[j].id);
		fid.deffontsettings.charset = cfg::getByte("CLC", szTemp, DEFAULT_CHARSET);
		_snprintf(szTemp, sizeof(szTemp), "Font%dName", _tagFSINFO[j].id);
		if(cfg::getString(NULL, "CLC", szTemp, &dbv))
			lstrcpyn(fid.deffontsettings.szFace, L"Tahoma", LF_FACESIZE);
		else {
			lstrcpyn(fid.deffontsettings.szFace, dbv.ptszVal, LF_FACESIZE);
			mir_free(dbv.ptszVal);
		}
		FontRegisterT(&fid);
		j++;
	}

	colourid.order = 0;
	strncpy(colourid.dbSettingsGroup, "CLC", sizeof(colourid.dbSettingsGroup));
	colourid.cbSize = sizeof(ColourIDT);
	mir_sntprintf(colourid.group, _countof(colourid.group), L"%s", L"Contact List");
	for (int i = 0; i < _countof(_clrs); i++) {
		colourid.order = _clrs[i].order;
		mir_snprintf(colourid.setting, sizeof(colourid.setting), "%s", _clrs[i].szSetting);
		mir_sntprintf(colourid.name, _countof(colourid.name), L"%s", _clrs[i].tszName);
		colourid.defcolour = (_clrs[i].def & 0xff000000 ? GetSysColor(_clrs[i].def & 0x00ffffff) : _clrs[i].def);
		ColourRegisterT(&colourid);
	}
}

TSkinDescription cfg::my_default_skin[] = {
	IDR_SKIN_BASE, _T("base.cng"),
	IDR_SKIN_BACK, _T("back.png"),
	IDR_SKIN_BACKAERO, _T("AeroBack.png"),
	IDR_SKIN_GLYPHS, _T("glyphs.png"),
};

/**
 * first stage config init. Just read profile base path and try to
 * extract the skin from the DLL
 */
int cfg::onInit()
{
	wchar_t*	userdata = ::Utils_ReplaceVarsT(L"%miranda_profilesdir%");
	wchar_t		szBaseSkin[MAX_PATH];

	LRESULT fi_version = CallService(MS_IMG_GETIFVERSION, 0, 0);
	CallService(MS_IMG_GETINTERFACE, fi_version, (LPARAM)&fif);

	if(0 == fif)
		return(-S_FALSE);

	mir_sntprintf(szProfileDir, MAX_PATH, L"%s", userdata);
	mir_free(userdata);

	Utils::ensureTralingBackslash(szProfileDir);
	mir_sntprintf(szBaseSkin, MAX_PATH, L"%s%s", szProfileDir, L"skin\\clng\\base");
	CreateDirectoryTreeW(szBaseSkin);
	extractBaseSkin(false);

	return(fBaseSkinValid ? S_OK : -S_FALSE);
}
/**
 * extract the aero skin images from the DLL and store them in
 * the private data folder.
 * runs at every startup
 *
 * only overwrites the files when version number does not match or
 * one of the files is missing.
 */
void cfg::extractBaseSkin(bool fForceOverwrite)
{
	wchar_t 	wszBasePath[MAX_PATH], wszTest[MAX_PATH];
	bool		fChecksPassed = true;
	HANDLE		hFile;

	mir_sntprintf(wszBasePath, MAX_PATH, L"%s%s", szProfileDir, L"skin\\clng\\base\\");
	mir_sntprintf(wszTest, MAX_PATH, L"%s%s", wszBasePath, L"base.cng");

	/*
	 * version check, also fails when the file is simply missing
	 */
	int uVersion = GetPrivateProfileInt(L"SkinInfo", L"Version", 0, wszTest);
	if(uVersion < SKIN_REQUIRED_VERSION)
		fChecksPassed = false;

	/*
	 * version check passed, verify files are present
	 */
	if(fChecksPassed) {
		for(int i = 0; i < safe_sizeof(my_default_skin); i++) {
			mir_sntprintf(wszTest, MAX_PATH, L"%s%s", wszBasePath, my_default_skin[i].tszName);
			if((hFile = CreateFile(wszTest, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE) {
				fChecksPassed  = false;
				break;
			}
			CloseHandle(hFile);
		}
	}

	// files are not present in skin dir, extract the default skin
	if(!fChecksPassed) {
		try {
			for(int i = 0; i < safe_sizeof(my_default_skin); i++)
				Utils::extractResource(g_hInst, my_default_skin[i].ulID, L"SKIN_GLYPH", wszBasePath, my_default_skin[i].tszName, true);

			fBaseSkinValid = true;
		} catch(CRTException& ex) {
			ex.display();
			fBaseSkinValid = false;
		}
	}
}

int Api::onInit()
{
	HMODULE hUserDll = 0;

	hUserDll = GetModuleHandleA("user32.dll");

	pfnGradientFill = (PGF) GetProcAddress(GetModuleHandleA("gdi32"), "GdiGradientFill");
	if(0 == pfnGradientFill)
		pfnGradientFill = (PGF) GetProcAddress(GetModuleHandleA("msimg32"), "GradientFill");

	pfnAlphaBlend = (pfnAlphaBlend_t) GetProcAddress(GetModuleHandleA("gdi32"), "GdiAlphaBlend");
	if(0 == pfnAlphaBlend)
		pfnAlphaBlend = (pfnAlphaBlend_t)GetProcAddress(GetModuleHandleA("msimg32"), "AlphaBlend");

	pfnGetTickCount64 = (pfnGetTickCount64_t)GetProcAddress(GetModuleHandleA("kernel32"), "GetTickCount64");

	sysConfig.isVistaPlus = (IsWinVerVistaPlus() ? true : false);
	sysConfig.isSevenPlus = (IsWinVer7Plus() ? true : false);

	if(!sysConfig.isVistaPlus)
		return(-S_FALSE);

	if(sysConfig.isVistaPlus) {
		if((hUxTheme = Utils::loadSystemLibrary(L"\\uxtheme.dll"), true) != 0) {
			pfnIsThemeActive = (pfnIsThemeActive_t)GetProcAddress(hUxTheme, "IsThemeActive");
			pfnOpenThemeData = (pfnOpenThemeData_t)GetProcAddress(hUxTheme, "OpenThemeData");
			pfnDrawThemeBackground = (pfnDrawThemeBackground_t)GetProcAddress(hUxTheme, "DrawThemeBackground");
			pfnCloseThemeData = (pfnCloseThemeData_t)GetProcAddress(hUxTheme, "CloseThemeData");
			pfnDrawThemeText = (pfnDrawThemeText_t)GetProcAddress(hUxTheme, "DrawThemeText");
			pfnIsThemeBackgroundPartiallyTransparent = (pfnIsThemeBackgroundPartiallyTransparent_t)GetProcAddress(hUxTheme, "IsThemeBackgroundPartiallyTransparent");
			pfnDrawThemeParentBackground = (pfnDrawThemeParentBackground_t)GetProcAddress(hUxTheme, "DrawThemeParentBackground");
			pfnGetThemeBackgroundContentRect = (pfnGetThemeBackgroundContentRect_t)GetProcAddress(hUxTheme, "GetThemeBackgroundContentRect");
			pfnEnableThemeDialogTexture = (pfnEnableThemeDialogTexture_t)GetProcAddress(hUxTheme, "EnableThemeDialogTexture");

			if(pfnIsThemeActive != 0 && pfnOpenThemeData != 0 && pfnDrawThemeBackground != 0 && pfnCloseThemeData != 0
					&& pfnDrawThemeText != 0 && pfnIsThemeBackgroundPartiallyTransparent != 0 && pfnDrawThemeParentBackground != 0
					&& pfnGetThemeBackgroundContentRect != 0) {
			}
			pfnBeginBufferedPaint = (pfnBeginBufferedPaint_t)GetProcAddress(hUxTheme, "BeginBufferedPaint");
			pfnEndBufferedPaint = (pfnEndBufferedPaint_t)GetProcAddress(hUxTheme, "EndBufferedPaint");
			pfnBufferedPaintInit = (pfnBufferedPaintInit_t)GetProcAddress(hUxTheme, "BufferedPaintInit");
			pfnBufferedPaintUninit = (pfnBufferedPaintUninit_t)GetProcAddress(hUxTheme, "BufferedPaintUnInit");
			pfnBufferedPaintSetAlpha = (pfnBufferedPaintSetAlpha_t)GetProcAddress(hUxTheme, "BufferedPaintSetAlpha");
			pfnBufferedPaintClear = (pfnBufferedPaintClear_t)GetProcAddress(hUxTheme, "BufferedPaintClear");
			pfnGetBufferedPaintBits = (pfnGetBufferedPaintBits_t)GetProcAddress(hUxTheme, "GetBufferedPaintBits");
			pfnDrawThemeTextEx = (pfnDrawThemeTextEx_t)GetProcAddress(hUxTheme, "DrawThemeTextEx");

			if((hDwm = Utils::loadSystemLibrary(L"\\dwmapi.dll"), true) != 0) {
				pfnDwmIsCompositionEnabled = (pfnDwmIsCompositionEnabled_t)GetProcAddress(hDwm, "DwmIsCompositionEnabled");
				pfnDwmExtendFrameIntoClientArea = (pfnDwmExtendFrameIntoClientArea_t)GetProcAddress(hDwm, "DwmExtendFrameIntoClientArea");

				pfnDwmBlurBehindWindow = (pfnDwmBlurBehindWindow_t)GetProcAddress(hDwm, "DwmEnableBlurBehindWindow");
				pfnDwmGetColorizationColor = (pfnDwmGetColorizationColor_t)GetProcAddress(hDwm, "DwmGetColorizationColor");
			}
		}
	}
	pfnBufferedPaintInit();
	updateState();
	return(S_OK);
}

void Api::onUnload()
{
	if(hUxTheme)
		FreeLibrary(hUxTheme);

	pfnBufferedPaintUninit();
}

/**
 * update system's state (theme status, aero status, DWM check...
 *
 * called when windows broadcasts things like WM_THEMECHANGED or
 * WM_DWMCOMPOSITIONCHANGED
 */
void Api::updateState()
{
	BOOL result = FALSE;

	::ZeroMemory(&sysState, sizeof(TSysState));

	sysState.isThemed = pfnIsThemeActive() ? true : false;

	if(sysConfig.isVistaPlus) {
		sysState.isDwmActive = (pfnDwmIsCompositionEnabled && (pfnDwmIsCompositionEnabled(&result) == S_OK) && result) ? true : false;
		sysState.isAero = cfg::getByte(SKIN_DB_MODULE, "fUseAero", 1) && sysState.isDwmActive;
	}
	else
		sysState.isAero = sysState.isDwmActive = false;
}

/**
 * exception handling for SEH exceptions
 */

/**
 * exception handling - copy error message to clip board
 * @param hWnd: 	window handle of the edit control containing the error message
 */
void Api::Ex_CopyEditToClipboard(HWND hWnd)
{
	SendMessage(hWnd, EM_SETSEL, 0, 65535L);
	SendMessage(hWnd, WM_COPY, 0 , 0);
	SendMessage(hWnd, EM_SETSEL, 0, 0);
}

INT_PTR CALLBACK Api::Ex_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode, wID;

	switch(uMsg) {
		case WM_INITDIALOG: {
			char szBuffer[2048];
#ifdef _WIN64
			sprintf(szBuffer,
					"Exception %16.16X at address %16.16X occured in %s at line %d.\r\n\r\nEAX=%16.16X EBX=%16.16X ECX=%16.16X\r\nEDX=%16.16X ESI=%16.16X EDI=%16.16X\r\nEBP=%16.16X ESP=%16.16X EIP=%16.16X",
					exRecord.ExceptionCode, exRecord.ExceptionAddress, exSzFile, exLine,
					exCtx.Rax, exCtx.Rbx, exCtx.Rcx, exCtx.Rdx,
					exCtx.Rsi, exCtx.Rdi, exCtx.Rbp, exCtx.Rsp, exCtx.Rip);
#else
			sprintf(szBuffer,
					"Exception %8.8X at address %8.8X occured in %s at line %d.\r\n\r\nEAX=%8.8X EBX=%8.8X ECX=%8.8X\r\nEDX=%8.8X ESI=%8.8X EDI=%8.8X\r\nEBP=%8.8X ESP=%8.8X EIP=%8.8X",
					exRecord.ExceptionCode, exRecord.ExceptionAddress, exSzFile, exLine,
					exCtx.Eax, exCtx.Ebx, exCtx.Ecx, exCtx.Edx,
					exCtx.Esi, exCtx.Edi, exCtx.Ebp, exCtx.Esp, exCtx.Eip);
#endif
			SetDlgItemTextA(hwndDlg, IDC_EXCEPTION_DETAILS, szBuffer);
			SetFocus(GetDlgItem(hwndDlg, IDC_EXCEPTION_DETAILS));
			SendDlgItemMessage(hwndDlg, IDC_EXCEPTION_DETAILS, WM_SETFONT, (WPARAM)GetStockObject(OEM_FIXED_FONT), 0);
			SetDlgItemTextW(hwndDlg, IDC_EX_REASON, exReason);
			Utils::enableDlgControl(hwndDlg, IDOK, exAllowContinue ? TRUE : FALSE);
		}
		break;

		case WM_COMMAND:
			wNotifyCode = HIWORD(wParam);
			wID = LOWORD(wParam);
			if(wNotifyCode == BN_CLICKED) {
				if(wID == IDOK || wID == IDCANCEL)
					EndDialog(hwndDlg, wID);

				if(wID == IDC_COPY_EXCEPTION)
					Ex_CopyEditToClipboard(GetDlgItem(hwndDlg, IDC_EXCEPTION_DETAILS));
			}

			break;
	}
	return FALSE;
}

void Api::Ex_Handler()
{
	if(exLastResult == IDCANCEL)
		ExitProcess(1);
}

int Api::Ex_ShowDialog(EXCEPTION_POINTERS *ep, const char *szFile, int line, wchar_t* szReason, bool fAllowContinue)
{
	char	szDrive[MAX_PATH], szDir[MAX_PATH], szName[MAX_PATH], szExt[MAX_PATH];

	_splitpath(szFile, szDrive, szDir, szName, szExt);
	memcpy(&exRecord, ep->ExceptionRecord, sizeof(EXCEPTION_RECORD));
	memcpy(&exCtx, ep->ContextRecord, sizeof(CONTEXT));

	_snprintf(exSzFile, MAX_PATH, "%s%s", szName, szExt);
	mir_sntprintf(exReason, 256, L"An application error has occured: %s", szReason);
	exLine = line;
	exLastResult = DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_EXCEPTION), 0, Ex_DlgProc, 0);
	exAllowContinue = fAllowContinue;
	if(IDCANCEL == exLastResult)
		ExitProcess(1);
	return 1;
}

CRTException::CRTException(const char *szMsg, const wchar_t *szParam) : std::runtime_error(std::string(szMsg))
{
	mir_sntprintf(m_szParam, MAX_PATH, szParam);
}

void CRTException::display() const
{
	wchar_t*	tszMsg = mir_a2t(what());
	wchar_t  	tszBoxMsg[500];

	mir_sntprintf(tszBoxMsg, 500, _T("%s\n\n(%s)"), tszMsg, m_szParam);
	::MessageBox(0, tszBoxMsg, _T("ClistNG runtime error"), MB_OK | MB_ICONERROR);
	mir_free(tszMsg);
}

