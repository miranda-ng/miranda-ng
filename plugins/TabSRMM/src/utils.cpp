////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// generic utility functions

#include "stdafx.h"

OBJLIST<TRTFColorTable> Utils::rtf_clrs(10);

/////////////////////////////////////////////////////////////////////////////////////////

char* Utils::FilterEventMarkers(char *szText)
{
	for (char *p = strstr(szText, "~-+"); p != nullptr; p = strstr(p, "~-+")) {
		char *pEnd = strstr(p + 3, "+-~");
		if (pEnd == nullptr)
			break;

		strdel(p, (pEnd - p) + 3);
	}

	return szText;
}

wchar_t* Utils::FilterEventMarkers(wchar_t *wszText)
{
	for (wchar_t *p = wcsstr(wszText, L"~-+"); p != nullptr; p = wcsstr(p, L"~-+")) {
		wchar_t *pEnd = wcsstr(p + 3, L"+-~");
		if (pEnd == nullptr)
			break;

		strdelw(p, (pEnd - p) + 3);
	}

	return wszText;
}

/////////////////////////////////////////////////////////////////////////////////////////

void Utils::DoubleAmpersands(wchar_t *pszText, size_t len)
{
	CMStringW text(pszText);
	text.Replace(L"&", L"&&");
	mir_wstrncpy(pszText, text.c_str(), len);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Get a preview of the text with an ellipsis appended(...)
//
// @param szText	source text
// @param iMaxLen	max length of the preview
// @return wchar_t*   result (caller must mir_free() it)

wchar_t* Utils::GetPreviewWithEllipsis(wchar_t *szText, size_t iMaxLen)
{
	size_t uRequired;
	wchar_t *p = nullptr, cSaved;
	bool	 fEllipsis = false;

	if (mir_wstrlen(szText) <= iMaxLen) {
		uRequired = mir_wstrlen(szText) + 4;
		cSaved = 0;
	}
	else {
		p = &szText[iMaxLen - 1];
		fEllipsis = true;

		while (p >= szText && *p != ' ')
			p--;
		if (p == szText)
			p = szText + iMaxLen - 1;

		cSaved = *p;
		*p = 0;
		uRequired = (p - szText) + 6;
	}
	wchar_t *szResult = reinterpret_cast<wchar_t *>(mir_alloc((uRequired + 1) * sizeof(wchar_t)));
	mir_snwprintf(szResult, (uRequired + 1), fEllipsis ? L"%s..." : L"%s", szText);

	if (p)
		*p = cSaved;

	return szResult;
}

/////////////////////////////////////////////////////////////////////////////////////////
// init default color table. the table may grow when using custom colors via bbcodes

static const wchar_t *sttColorNames[] = {
	L"black",
	L"",  L"", L"", L"", L"", L"", L"", L"",
	L"blue", L"cyan", L"magenta", L"green", L"yellow", L"red", L"white"
};

void Utils::RTF_CTableInit()
{
	int iTableSize;
	COLORREF *pTable = Srmm_GetColorTable(&iTableSize);
	for (int i = 0; i < iTableSize; i++)
		rtf_clrs.insert(new TRTFColorTable(sttColorNames[i], pTable[i]));
}

/////////////////////////////////////////////////////////////////////////////////////////
// add a color to the global rtf color table

void Utils::RTF_ColorAdd(const wchar_t *tszColname)
{
	wchar_t *stopped;
	COLORREF clr = wcstol(tszColname, &stopped, 16);
	rtf_clrs.insert(new TRTFColorTable(tszColname, RGB(GetBValue(clr), GetGValue(clr), GetRValue(clr))));
}

/////////////////////////////////////////////////////////////////////////////////////////
// generic error popup dialog procedure

LRESULT CALLBACK Utils::PopupDlgProcError(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = PUGetContact(hWnd);

	switch (message) {
	case WM_COMMAND:
	case WM_CONTEXTMENU:
		PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_HANDLECLISTEVENT, hContact, 0);
		PUDeletePopup(hWnd);
		break;
	case WM_MOUSEWHEEL:
		break;
	case WM_SETCURSOR:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// read a blob from db into the container settings structure
// @param hContact:	contact handle (0 = read global)
// @param cs		TContainerSettings* target structure
// @return			0 on success, 1 failure (blob does not exist OR is not a valid private setting structure

struct TOldContainerSettings
{
	BOOL    fPrivate;
	uint32_t   dwFlags;
	uint32_t   dwFlagsEx;
	uint32_t   dwTransparency;
	uint32_t   panelheight;
	int     iSplitterY;
	wchar_t szTitleFormat[32];
	uint16_t    avatarMode;
	uint16_t    ownAvatarMode;
	uint16_t    autoCloseSeconds;
	uint8_t    reserved[10];
};

int Utils::ReadContainerSettingsFromDB(const MCONTACT hContact, TContainerSettings *cs, const char *szKey)
{
	CMStringA szSetting(szKey ? szKey : CNT_KEYNAME);
	int iSplitterX = db_get_dw(0, SRMSGMOD_T, szSetting + "_SplitterX", -1);
	if (iSplitterX == -1) { // nothing? try the old format
		DBVARIANT dbv = { 0 };
		if (0 == db_get(hContact, SRMSGMOD_T, szSetting + "_Blob", &dbv)) {
			TOldContainerSettings oldBin = {};
			if (dbv.type == DBVT_BLOB && dbv.cpbVal > 0 && dbv.cpbVal <= sizeof(oldBin)) {
				::memcpy(&oldBin, (void*)dbv.pbVal, dbv.cpbVal);
				cs->flags.dw = oldBin.dwFlags;
				cs->flagsEx.dw = oldBin.dwFlagsEx;
				cs->dwTransparency = oldBin.dwTransparency;
				cs->panelheight = oldBin.panelheight;
				cs->iSplitterY = oldBin.iSplitterY;
				cs->iSplitterX = 35;
				wcsncpy_s(cs->szTitleFormat, oldBin.szTitleFormat, _TRUNCATE);
				cs->avatarMode = oldBin.avatarMode;
				cs->ownAvatarMode = oldBin.ownAvatarMode;
				cs->autoCloseSeconds = oldBin.autoCloseSeconds;
				cs->fPrivate = oldBin.fPrivate != 0;
				Utils::WriteContainerSettingsToDB(hContact, cs, szKey);
				db_unset(hContact, SRMSGMOD_T, szSetting);
				::db_free(&dbv);
				return 0;
			}
		}
		cs->fPrivate = false;
		db_free(&dbv);
		return 1;
	}

	cs->flags.dw = db_get_dw(hContact, SRMSGMOD_T, szSetting + "_Flags", 0);
	cs->flagsEx.dw = db_get_dw(hContact, SRMSGMOD_T, szSetting + "_FlagsEx", 0);
	cs->dwTransparency = db_get_dw(hContact, SRMSGMOD_T, szSetting + "_Transparency", 0);
	cs->panelheight = db_get_dw(hContact, SRMSGMOD_T, szSetting + "_PanelY", 0);
	cs->iSplitterX = iSplitterX;
	cs->iSplitterY = db_get_dw(hContact, SRMSGMOD_T, szSetting + "_SplitterY", 0);
	cs->avatarMode = db_get_w(hContact, SRMSGMOD_T, szSetting + "_AvatarMode", 0);
	cs->ownAvatarMode = db_get_w(hContact, SRMSGMOD_T, szSetting + "_OwnAvatarMode", 0);
	cs->autoCloseSeconds = db_get_w(hContact, SRMSGMOD_T, szSetting + "_AutoCloseSecs", 0);
	cs->fPrivate = db_get_b(hContact, SRMSGMOD_T, szSetting + "_Private", 0) != 0;
	db_get_wstatic(hContact, SRMSGMOD_T, szSetting + "_Format", cs->szTitleFormat, _countof(cs->szTitleFormat));
	return 0;
}

int Utils::WriteContainerSettingsToDB(const MCONTACT hContact, TContainerSettings *cs, const char *szKey)
{
	CMStringA szSetting(szKey ? szKey : CNT_KEYNAME);
	db_set_dw(hContact, SRMSGMOD_T, szSetting + "_Flags", cs->flags.dw);
	db_set_dw(hContact, SRMSGMOD_T, szSetting + "_FlagsEx", cs->flagsEx.dw);
	db_set_dw(hContact, SRMSGMOD_T, szSetting + "_Transparency", cs->dwTransparency);
	db_set_dw(hContact, SRMSGMOD_T, szSetting + "_PanelY", cs->panelheight);
	db_set_dw(hContact, SRMSGMOD_T, szSetting + "_SplitterX", cs->iSplitterX);
	db_set_dw(hContact, SRMSGMOD_T, szSetting + "_SplitterY", cs->iSplitterY);
	db_set_ws(hContact, SRMSGMOD_T, szSetting + "_Format", cs->szTitleFormat);
	db_set_w(hContact, SRMSGMOD_T, szSetting + "_AvatarMode", cs->avatarMode);
	db_set_w(hContact, SRMSGMOD_T, szSetting + "_OwnAvatarMode", cs->ownAvatarMode);
	db_set_w(hContact, SRMSGMOD_T, szSetting + "_AutoCloseSecs", cs->autoCloseSeconds);
	db_set_b(hContact, SRMSGMOD_T, szSetting + "_Private", cs->fPrivate);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// calculate new width and height values for a user picture (avatar)
//
// @param: maxHeight -	determines maximum height for the picture, width will
// 					be scaled accordingly.

void Utils::scaleAvatarHeightLimited(const HBITMAP hBm, double& dNewWidth, double& dNewHeight, LONG maxHeight)
{
	BITMAP	bm;
	double	dAspect;

	GetObject(hBm, sizeof(bm), &bm);

	if (bm.bmHeight > bm.bmWidth) {
		if (bm.bmHeight > 0)
			dAspect = (double)(maxHeight) / (double)bm.bmHeight;
		else
			dAspect = 1.0;
		dNewWidth = (double)bm.bmWidth * dAspect;
		dNewHeight = (double)maxHeight;
	}
	else {
		if (bm.bmWidth > 0)
			dAspect = (double)(maxHeight) / (double)bm.bmWidth;
		else
			dAspect = 1.0;
		dNewHeight = (double)bm.bmHeight * dAspect;
		dNewWidth = (double)maxHeight;
	}
}

AVATARCACHEENTRY* Utils::loadAvatarFromAVS(const MCONTACT hContact)
{
	if (!ServiceExists(MS_AV_GETAVATARBITMAP))
		return nullptr;

	return (AVATARCACHEENTRY*)CallService(MS_AV_GETAVATARBITMAP, hContact, 0);
}

void Utils::sendContactMessage(MCONTACT hContact, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND h = Srmm_FindWindow(hContact);
	if (h != nullptr)
		PostMessage(h, uMsg, wParam, lParam);
}

void Utils::getIconSize(HICON hIcon, int& sizeX, int& sizeY)
{
	ICONINFO ii;
	BITMAP bm;
	::GetIconInfo(hIcon, &ii);
	::GetObject(ii.hbmColor, sizeof(bm), &bm);
	sizeX = bm.bmWidth;
	sizeY = bm.bmHeight;
	::DeleteObject(ii.hbmMask);
	::DeleteObject(ii.hbmColor);
}

/////////////////////////////////////////////////////////////////////////////////////////
// add a menu item to a ownerdrawn menu. mii must be pre-initialized
//
// @param m			menu handle
// @param mii		menu item info structure
// @param hIcon		the icon (0 allowed -> no icon)
// @param szText	menu item text (must NOT be 0)
// @param uID		the item command id
// @param pos		zero-based position index

void Utils::addMenuItem(const HMENU& m, MENUITEMINFO &mii, HICON hIcon, const wchar_t *szText, UINT uID, UINT pos)
{
	mii.wID = uID;
	mii.dwItemData = (ULONG_PTR)hIcon;
	mii.dwTypeData = const_cast<wchar_t *>(szText);
	mii.cch = (int)mir_wstrlen(mii.dwTypeData) + 1;

	::InsertMenuItem(m, pos, TRUE, &mii);
}

/////////////////////////////////////////////////////////////////////////////////////////
// enable or disable a dialog control

void Utils::enableDlgControl(const HWND hwnd, UINT id, bool fEnable)
{
	::EnableWindow(::GetDlgItem(hwnd, id), fEnable);
}

/////////////////////////////////////////////////////////////////////////////////////////
// show or hide a dialog control

void Utils::showDlgControl(const HWND hwnd, UINT id, int showCmd)
{
	::ShowWindow(::GetDlgItem(hwnd, id), showCmd);
}

/////////////////////////////////////////////////////////////////////////////////////////
// stream function to write the contents of the message log to an rtf file
uint32_t CALLBACK Utils::StreamOut(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	wchar_t *szFilename = (wchar_t*)dwCookie;
	HANDLE hFile = CreateFile(szFilename, GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE) {
		SetFilePointer(hFile, 0, nullptr, FILE_END);
		FilterEventMarkers((char*)pbBuff);
		WriteFile(hFile, pbBuff, cb, (DWORD *)pcb, nullptr);
		*pcb = cb;
		CloseHandle(hFile);
		return 0;
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// extract a resource from the given module
// tszPath must end with \ 

bool Utils::extractResource(const HMODULE h, const UINT uID, const wchar_t *tszName, const wchar_t *tszPath,
	const wchar_t *tszFilename, bool fForceOverwrite)
{
	HRSRC hRes = FindResource(h, MAKEINTRESOURCE(uID), tszName);
	if (hRes) {
		HGLOBAL hResource = LoadResource(h, hRes);
		if (hResource) {
			char 	*pData = (char *)LockResource(hResource);
			DWORD dwSize = SizeofResource(g_plugin.getInst(), hRes), written = 0;

			wchar_t	szFilename[MAX_PATH];
			mir_snwprintf(szFilename, L"%s%s", tszPath, tszFilename);
			if (!fForceOverwrite)
				if (PathFileExists(szFilename))
					return true;

			HANDLE hFile = CreateFile(szFilename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (hFile == INVALID_HANDLE_VALUE)
				return false;

			WriteFile(hFile, (void*)pData, dwSize, &written, nullptr);
			CloseHandle(hFile);
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// extract the clicked URL from a rich edit control. Return the URL as wchar_t*
// caller MUST mir_free() the returned string
// @param 	hwndRich -  rich edit window handle
// @return	wchar_t*	extracted URL

wchar_t* Utils::extractURLFromRichEdit(const ENLINK* _e, const HWND hwndRich)
{
	CHARRANGE sel = { 0 };
	::SendMessage(hwndRich, EM_EXGETSEL, 0, (LPARAM)&sel);
	if (sel.cpMin != sel.cpMax)
		return nullptr;

	TEXTRANGE tr;
	tr.chrg = _e->chrg;
	tr.lpstrText = (wchar_t*)mir_alloc(sizeof(wchar_t) * (tr.chrg.cpMax - tr.chrg.cpMin + 8));
	::SendMessage(hwndRich, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
	if (wcschr(tr.lpstrText, '@') != nullptr && wcschr(tr.lpstrText, ':') == nullptr && wcschr(tr.lpstrText, '/') == nullptr) {
		mir_wstrncpy(tr.lpstrText, L"mailto:", 7);
		mir_wstrncpy(tr.lpstrText + 7, tr.lpstrText, tr.chrg.cpMax - tr.chrg.cpMin + 1);
	}
	return tr.lpstrText;
}

/////////////////////////////////////////////////////////////////////////////////////////
// filters out invalid characters from a string used as part of a file
// or folder name. All invalid characters will be replaced by spaces.
//
// @param tszFilename - string to filter.

void Utils::sanitizeFilename(wchar_t* tszFilename)
{
	static wchar_t *forbiddenCharacters = L"%/\\':|\"<>?";
	static size_t forbiddenCharactersLen = mir_wstrlen(forbiddenCharacters);

	for (size_t i = 0; i < forbiddenCharactersLen; i++) {
		wchar_t*	szFound = nullptr;

		while ((szFound = wcschr(tszFilename, (int)forbiddenCharacters[i])) != nullptr)
			*szFound = ' ';
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// ensure that a path name ends on a trailing backslash
// @param szPathname - pathname to check

void Utils::ensureTralingBackslash(wchar_t *szPathname)
{
	if (szPathname[mir_wstrlen(szPathname) - 1] != '\\')
		mir_wstrcat(szPathname, L"\\");
}

/////////////////////////////////////////////////////////////////////////////////////////
// load a system library from the Windows system path and return its module
// handle.
//
// return 0 and throw an exception if something goes wrong.

HMODULE Utils::loadSystemLibrary(const wchar_t* szFilename)
{
	wchar_t sysPathName[MAX_PATH + 2];
	if (0 == ::GetSystemDirectoryW(sysPathName, MAX_PATH))
		return nullptr;

	sysPathName[MAX_PATH - 1] = 0;
	if (mir_wstrlen(sysPathName) + mir_wstrlen(szFilename) >= MAX_PATH)
		return nullptr;

	mir_wstrcat(sysPathName, szFilename);
	HMODULE _h = LoadLibraryW(sysPathName);
	if (nullptr == _h)
		return nullptr;

	return _h;
}

/////////////////////////////////////////////////////////////////////////////////////////
// setting avatar's contact

void Utils::setAvatarContact(HWND hWnd, MCONTACT hContact)
{
	MCONTACT hSub = db_mc_getSrmmSub(hContact);
	SendMessage(hWnd, AVATAR_SETCONTACT, 0, (hSub) ? hSub : hContact);
}

/////////////////////////////////////////////////////////////////////////////////////////

HWND TSAPI GetTabWindow(HWND hwndTab, int i)
{
	if (i < 0)
		return nullptr;

	TCITEM tci;
	tci.mask = TCIF_PARAM;
	return (TabCtrl_GetItem(hwndTab, i, &tci)) ? (HWND)tci.lParam : nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// file list handler

int _DebugTraceW(const wchar_t *fmt, ...)
{
	wchar_t 	debug[2048];
	int     	ibsize = 2047;
	SYSTEMTIME	st;
	va_list 	va;
	char		tszTime[50];
	va_start(va, fmt);

	GetLocalTime(&st);

	mir_snprintf(tszTime, "%02d.%02d.%04d - %02d:%02d:%02d.%04d: ", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);


	mir_vsnwprintf(debug, ibsize - 10, fmt, va);
	//#ifdef _DEBUG
	OutputDebugStringW(debug);
	//#else
	{
		char szLogFileName[MAX_PATH], szDataPath[MAX_PATH];
		FILE *f;

		Profile_GetPathA(MAX_PATH, szDataPath);
		mir_snprintf(szLogFileName, "%s\\%s", szDataPath, "tabsrmm_debug.log");
		f = fopen(szLogFileName, "a+");
		if (f) {
			fputs(tszTime, f);
			fputs(T2Utf(debug), f);
			fputs("\n", f);
			fclose(f);
		}
	}
	//#endif
	return 0;
}

/*
* output a notification message.
* may accept a hContact to include the contacts nickname in the notification message...
* the actual message is using printf() rules for formatting and passing the arguments...
*
* can display the message either as systray notification (baloon popup) or using the
* popup plugin.
*/
int _DebugPopup(MCONTACT hContact, const wchar_t *fmt, ...)
{
	va_list	va;
	wchar_t		debug[1024];
	int			ibsize = 1023;

	va_start(va, fmt);
	mir_vsnwprintf(debug, ibsize, fmt, va);

	wchar_t	szTitle[128];
	mir_snwprintf(szTitle, TranslateT("TabSRMM message (%s)"),
		(hContact != 0) ? Clist_GetContactDisplayName(hContact) : TranslateT("Global"));

	Clist_TrayNotifyW(nullptr, szTitle, debug, NIIF_INFO, 1000 * 4);
	return 0;
}
