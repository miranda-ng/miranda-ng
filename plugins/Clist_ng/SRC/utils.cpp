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
 * $Id: utils.cpp 137 2010-10-16 21:03:23Z silvercircle $
 *
 * utility functions for clist_ng.
 */

#include <commonheaders.h>

#define RTF_DEFAULT_HEADER _T("{\\rtf1\\ansi\\deff0\\pard\\li%u\\fi-%u\\ri%u\\tx%u")

wchar_t* WarningDlg::m_warnings[WarningDlg::WARN_LAST] = {
		LPGENT("Notes|release notes"),
		LPGENT("Skin loading error|The skin cannot be loaded"),
		LPGENT("Overwrite skin file|You are about to save your customized settings to the \\b1 original skin file.\\b0  This could cause problems and may require to reinstall the skin.\nYou should use \\b1 Save as user modification\\b0  to keep the original skin untouched.\n\nContinue?")
};

MWindowList	WarningDlg::hWindowList = 0;

/**
 * ensure that a path name ends on a trailing backslash
 * @param szPathname - pathname to check
 */
void Utils::ensureTralingBackslash(wchar_t *szPathname)
{
	if(szPathname[lstrlenW(szPathname) - 1] != '\\')
		wcscat(szPathname, L"\\");
}

void Utils::extractResource(const HMODULE h, const UINT uID, const wchar_t* tszName, const wchar_t* tszPath,
							const wchar_t* tszFilename, bool fForceOverwrite)
{
	HRSRC 	hRes;
	HGLOBAL	hResource;
	wchar_t	szFilename[MAX_PATH];

	hRes = FindResourceW(h, MAKEINTRESOURCE(uID), tszName);

	if(hRes) {
		hResource = LoadResource(h, hRes);
		if(hResource) {
			HANDLE  hFile;
			char 	*pData = (char *)LockResource(hResource);
			DWORD	dwSize = SizeofResource(g_hInst, hRes), written = 0;
			mir_sntprintf(szFilename, MAX_PATH, _T("%s%s"), tszPath, tszFilename);
			if(!fForceOverwrite) {
				if(PathFileExistsW(szFilename))
					return;
			}
			if((hFile = CreateFileW(szFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)) != INVALID_HANDLE_VALUE) {
				WriteFile(hFile, (void *)pData, dwSize, &written, NULL);
				CloseHandle(hFile);
			}
			else {
				wchar_t	wszTemp[512];
				mir_sntprintf(wszTemp, 512, L"%s - (WIN32 Error: %d)", szFilename, GetLastError());
				throw(CRTException("Error while extracting base skin.", wszTemp));
			}
		}
	}
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
HMODULE Utils::loadSystemLibrary(const wchar_t* szFilename, bool useGetHandle)
{
	wchar_t		sysPathName[MAX_PATH + 2];
	HMODULE		_h = 0;

	try {
		if(0 == ::GetSystemDirectoryW(sysPathName, MAX_PATH))
			throw(CRTException("Error while loading system library", szFilename));

		sysPathName[MAX_PATH - 1] = 0;
		if(wcslen(sysPathName) + wcslen(szFilename) >= MAX_PATH)
			throw(CRTException("Error while loading system library", szFilename));

		lstrcatW(sysPathName, szFilename);
		if(useGetHandle)
			_h = ::GetModuleHandle(sysPathName);
		else
			_h = LoadLibraryW(sysPathName);
		if(0 == _h)
			throw(CRTException("Error while loading system library", szFilename));
	}
	catch(CRTException& ex) {
		ex.display();
		return(0);
	}
	return(_h);
}

DWORD __fastcall Utils::hexStringToLong(const char *szSource)
{
    char *stopped;
    COLORREF clr = strtol(szSource, &stopped, 16);
    if(clr == -1)
        return clr;
    return(RGB(GetBValue(clr), GetGValue(clr), GetRValue(clr)));
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
const wchar_t* Utils::striStr(const wchar_t* szString, const wchar_t* szSearchFor)
{
	if(szString && *szString) {
		if (0 == szSearchFor || 0 == *szSearchFor)
			return(szString);

		for(; *szString; ++szString) {
			if(towupper(*szString) == towupper(*szSearchFor)) {
				const wchar_t* h, *n;
				for(h = szString, n = szSearchFor; *h && *n; ++h, ++n) {
					if(towupper(*h) != towupper(*n))
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

int Utils::pathIsAbsolute(const wchar_t* path)
{
	if (!path || !(lstrlenW(path) > 2))
		return 0;
	if ((path[1] == ':' && path[2] == '\\') || (path[0] == '\\' && path[1] == '\\'))
		return 1;
	return 0;
}

size_t Utils::pathToRelative(const wchar_t* pSrc, wchar_t* pOut, const wchar_t* szBase)
{
	const wchar_t*	tszBase = szBase ? szBase : cfg::szProfileDir;

	pOut[0] = 0;
	if (!pSrc || !lstrlenW(pSrc) || lstrlenW(pSrc) > MAX_PATH)
		return 0;
	if (!pathIsAbsolute(pSrc)) {
		mir_sntprintf(pOut, MAX_PATH, L"%s", pSrc);
		return lstrlenW(pOut);
	} else {
		wchar_t	szTmp[MAX_PATH];

		mir_sntprintf(szTmp, _countof(szTmp), L"%s", pSrc);
		if (striStr(szTmp, tszBase)) {
			if(tszBase[lstrlenW(tszBase) - 1] == '\\')
				mir_sntprintf(pOut, MAX_PATH, L"%s", pSrc + lstrlenW(tszBase));
			else {
				mir_sntprintf(pOut, MAX_PATH, L"%s", pSrc + lstrlenW(tszBase)  + 1 );
				//pOut[0]='.';
			}
			return(lstrlenW(pOut));
		} else {
			mir_sntprintf(pOut, MAX_PATH, L"%s", pSrc);
			return(lstrlenW(pOut));
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
size_t Utils::pathToAbsolute(const wchar_t* pSrc, wchar_t* pOut, const wchar_t* szBase)
{
	const wchar_t*	tszBase = szBase ? szBase : cfg::szProfileDir;
	const wchar_t*  wszFmt = (tszBase[lstrlenW(tszBase) - 1] == '\\' ? L"%s%s" : L"%s\\%s");

	pOut[0] = 0;
	if (!pSrc || !lstrlenW(pSrc) || lstrlenW(pSrc) > MAX_PATH)
		return 0;
	if (pathIsAbsolute(pSrc) && pSrc[0]!='.')
		mir_sntprintf(pOut, MAX_PATH, L"%s", pSrc);
	else if (pSrc[0]=='.')
		mir_sntprintf(pOut, MAX_PATH, wszFmt, tszBase, pSrc + 1);
	else
		mir_sntprintf(pOut, MAX_PATH, wszFmt, tszBase, pSrc);

	return lstrlenW(pOut);
}

/**
 * extract the clicked URL from a rich edit control. Return the URL as TCHAR*
 * caller MUST mir_free() the returned string
 * @param 	hwndRich -  rich edit window handle
 * @return	wchar_t*	extracted URL
 */
const wchar_t* Utils::extractURLFromRichEdit(const ENLINK* _e, const HWND hwndRich)
{
	TEXTRANGEW 	tr = {0};
	CHARRANGE 	sel = {0};

	::SendMessageW(hwndRich, EM_EXGETSEL, 0, (LPARAM) & sel);
	if (sel.cpMin != sel.cpMax)
		return(0);

	tr.chrg = _e->chrg;
	tr.lpstrText = (wchar_t *)mir_alloc(2 * (tr.chrg.cpMax - tr.chrg.cpMin + 8));
	::SendMessageW(hwndRich, EM_GETTEXTRANGE, 0, (LPARAM) & tr);
	if (wcschr(tr.lpstrText, '@') != NULL && wcschr(tr.lpstrText, ':') == NULL && wcschr(tr.lpstrText, '/') == NULL) {
		::MoveMemory(tr.lpstrText + 7, tr.lpstrText, sizeof(TCHAR) * (tr.chrg.cpMax - tr.chrg.cpMin + 1));
		::CopyMemory(tr.lpstrText, L"mailto:", 7);
	}
	return(tr.lpstrText);
}

/**
 * implementation of the CWarning class
 */
WarningDlg::WarningDlg(const wchar_t *tszTitle, const wchar_t *tszText, const UINT uId, const DWORD dwFlags)
{
	m_pszTitle = pSmartWstring(new std::basic_string<wchar_t>(tszTitle));
	m_pszText = pSmartWstring(new std::basic_string<wchar_t>(tszText));

	m_uId = uId;
	m_hFontCaption = 0;
	m_dwFlags = dwFlags;

	m_fIsModal = ((m_dwFlags & MB_YESNO || m_dwFlags & MB_YESNOCANCEL) ? true : false);
}

WarningDlg::~WarningDlg()
{
	if(m_hFontCaption)
		::DeleteObject(m_hFontCaption);

#if defined(__LOGDEBUG_)
	_DebugTraceW(L"destroy object");
#endif
}

LRESULT WarningDlg::ShowDialog() const
{
	if(!m_fIsModal) {
		::CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_WARNING), 0, stubDlgProc, reinterpret_cast<LPARAM>(this));
		return(0);
	}
	else {
		LRESULT res = ::DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_WARNING), 0, stubDlgProc, reinterpret_cast<LPARAM>(this));
		return(res);
	}
}

__int64 WarningDlg::getMask()
{
	__int64 mask = 0;

	DWORD	dwLow = cfg::getDword("CList", "cWarningsL", 0);
	DWORD	dwHigh = cfg::getDword("CList", "cWarningsH", 0);

	mask = ((((__int64)dwHigh) << 32) & 0xffffffff00000000) | dwLow;

	return(mask);
}

/**
 * send cancel message to all open warning dialogs so they are destroyed
 * before plugin is unloaded.
 *
 * called by the OkToExit handler in globals.cpp
 */
void WarningDlg::destroyAll()
{
	if(hWindowList)
		WindowList_Broadcast(hWindowList, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
}
/**
 * show a CWarning dialog using the id value. Check whether the user has chosen to
 * not show this message again. This has room for 64 different warning dialogs, which
 * should be enough in the first place. Extending it should not be too hard though.
 */
LRESULT WarningDlg::show(const int uId, DWORD dwFlags, const wchar_t* tszTxt)
{
	wchar_t*	separator_pos = 0;
	__int64 	mask = 0, val = 0;
	LRESULT 	result = 0;
	wchar_t*	_s = 0;

	if (0 == hWindowList)
		hWindowList = WindowList_Create();

	/*
	 * don't open new warnings when shutdown was initiated (modal ones will otherwise
	 * block the shutdown)
	 */
	if(cfg::shutDown)
		return(-1);

	if(uId >= 0) {
		mask = getMask();
		val = ((__int64)1L) << uId;
	}
	else
		mask = val = 0;

	if(!(0 == (mask & val) || dwFlags & CWF_NOALLOWHIDE))
		return(-1);

	if(tszTxt)
		_s = const_cast<wchar_t *>(tszTxt);
	else {
		if(uId != -1) {
			if(dwFlags & CWF_UNTRANSLATED)
				_s = const_cast<wchar_t *>(m_warnings[uId]);
			else {
				/*
				* revert to untranslated warning when the translated message
				* is not well-formatted.
				*/
				_s = const_cast<wchar_t *>(TranslateW(m_warnings[uId]));

				if(wcslen(_s) < 3 || 0 == wcschr(_s, '|'))
					_s = const_cast<wchar_t *>(m_warnings[uId]);
			}
		}
		else if(-1 == uId && tszTxt) {
			dwFlags |= CWF_NOALLOWHIDE;
			_s = (dwFlags & CWF_UNTRANSLATED ? const_cast<wchar_t *>(tszTxt) : TranslateW(tszTxt));
		}
		else
			return(-1);
	}

	if((wcslen(_s) > 3) && ((separator_pos = wcschr(_s, '|')) != 0)) {

		wchar_t *s = reinterpret_cast<wchar_t *>(mir_alloc((wcslen(_s) + 1) * 2));
		wcscpy(s, _s);
		separator_pos = wcschr(s, '|');

		if(separator_pos) {
			separator_pos[0] = 0;

			WarningDlg *w = new WarningDlg(s, &separator_pos[1], uId, dwFlags);
			if(!(dwFlags & MB_YESNO || dwFlags & MB_YESNOCANCEL)) {
				w->ShowDialog();
				mir_free(s);
			}
			else {
				result = w->ShowDialog();
				mir_free(s);
				return(result);
			}
		}
		else
			mir_free(s);
	}
	return(-1);
}

/**
 * stub dlg procedure. Just register the object pointer in WM_INITDIALOG
 */
INT_PTR CALLBACK WarningDlg::stubDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WarningDlg	*w = reinterpret_cast<WarningDlg *>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if(w)
		return(w->dlgProc(hwnd, msg, wParam, lParam));

	switch(msg) {
		case WM_INITDIALOG: {
			w = reinterpret_cast<WarningDlg *>(lParam);
			if(w) {
				::SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
				return(w->dlgProc(hwnd, msg, wParam, lParam));
			}
			break;
		}

#if defined(__LOGDEBUG_)
		case WM_NCDESTROY:
			_DebugTraceW(L"window destroyed");
			break;
#endif

		default:
			break;
	}
	return(FALSE);
}

/**
 * dialog procedure for the warning dialog box
 */
INT_PTR CALLBACK WarningDlg::dlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG: {
			HICON		hIcon = 0;
			UINT		uResId = 0;
			TCHAR		temp[1024];
			SETTEXTEX	stx = {ST_SELECTION, CP_UTF8};
			size_t		pos = 0;

			m_hwnd = hwnd;

			::SetWindowTextW(hwnd, L"Clist NG warning");
			::SendMessage(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(Skin_LoadIcon(SKINICON_OTHER_MIRANDA, true)));
			::SendMessage(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(Skin_LoadIcon(SKINICON_OTHER_MIRANDA)));
			::SendDlgItemMessage(hwnd, IDC_WARNTEXT, EM_AUTOURLDETECT, (WPARAM) TRUE, 0);
			::SendDlgItemMessage(hwnd, IDC_WARNTEXT, EM_SETEVENTMASK, 0, ENM_LINK);

			mir_sntprintf(temp, 1024, RTF_DEFAULT_HEADER, 0, 0, 0, 30*15);
			std::basic_string<wchar_t> *str = new std::basic_string<wchar_t>(temp);

			str->append((*m_pszText).c_str());
			str->append(L"}");

			TranslateDialogDefault(hwnd);

			/*
			 * convert normal line breaks to rtf
			 */
			while((pos = str->find(L"\n")) != str->npos) {
				str->erase(pos, 1);
				str->insert(pos, L"\\line ");
			}

			char *utf8 = Utf8EncodeW(str->c_str());
			::SendDlgItemMessage(hwnd, IDC_WARNTEXT, EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)utf8);
			mir_free(utf8);
			delete str;

			::SetDlgItemTextW(hwnd, IDC_CAPTION, (*m_pszTitle).c_str());

			if(m_dwFlags & CWF_NOALLOWHIDE)
				Utils::showDlgControl(hwnd, IDC_DONTSHOWAGAIN, SW_HIDE);
			if(m_dwFlags & MB_YESNO || m_dwFlags & MB_YESNOCANCEL) {
				Utils::showDlgControl(hwnd, IDOK, SW_HIDE);
				::SetFocus(::GetDlgItem(hwnd, IDCANCEL));
			}
			else {
				Utils::showDlgControl(hwnd, IDCANCEL, SW_HIDE);
				Utils::showDlgControl(hwnd, IDYES, SW_HIDE);
				Utils::showDlgControl(hwnd, IDNO, SW_HIDE);
				::SetFocus(::GetDlgItem(hwnd, IDOK));
			}
			if(m_dwFlags & MB_ICONERROR || m_dwFlags & MB_ICONHAND)
				uResId = 32513;
			else if(m_dwFlags & MB_ICONEXCLAMATION || m_dwFlags & MB_ICONWARNING)
				uResId = 32515;
			else if(m_dwFlags & MB_ICONASTERISK || m_dwFlags & MB_ICONINFORMATION)
				uResId = 32516;
			else if(m_dwFlags & MB_ICONQUESTION)
				uResId = 32514;

			if(uResId)
				hIcon = reinterpret_cast<HICON>(::LoadImage(0, MAKEINTRESOURCE(uResId), IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE));
			else
				hIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE, true);

			::SendDlgItemMessageW(hwnd, IDC_WARNICON, STM_SETICON, reinterpret_cast<WPARAM>(hIcon), 0);
			if(!(m_dwFlags & MB_YESNO || m_dwFlags & MB_YESNOCANCEL))
				::ShowWindow(hwnd, SW_SHOWNORMAL);

			WindowList_Add(hWindowList, hwnd, (MCONTACT)hwnd);
			return(TRUE);
		}

		case WM_CTLCOLORSTATIC: {
			HWND hwndChild = reinterpret_cast<HWND>(lParam);
			UINT id = ::GetDlgCtrlID(hwndChild);
			if(0 == m_hFontCaption) {
				HFONT hFont = reinterpret_cast<HFONT>(::SendDlgItemMessage(hwnd, IDC_CAPTION, WM_GETFONT, 0, 0));
				LOGFONT lf = {0};

				::GetObject(hFont, sizeof(lf), &lf);
				lf.lfHeight = (int)((double)lf.lfHeight * 1.7f);
				m_hFontCaption = ::CreateFontIndirect(&lf);
				::SendDlgItemMessage(hwnd, IDC_CAPTION, WM_SETFONT, (WPARAM)m_hFontCaption, FALSE);
			}

			if(IDC_CAPTION == id) {
				::SetTextColor(reinterpret_cast<HDC>(wParam), ::GetSysColor(COLOR_HIGHLIGHT));
				::SendMessage(hwndChild, WM_SETFONT, (WPARAM)m_hFontCaption, FALSE);
			}

			if(IDC_WARNGROUP != id && IDC_DONTSHOWAGAIN != id) {
				::SetBkColor((HDC)wParam, ::GetSysColor(COLOR_WINDOW));
				return reinterpret_cast<INT_PTR>(::GetSysColorBrush(COLOR_WINDOW));
			}
			break;
		}

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
				case IDCANCEL:
				case IDYES:
				case IDNO:
					if(!m_fIsModal && (IDOK == LOWORD(wParam) || IDCANCEL == LOWORD(wParam))) {		// modeless dialogs can receive a IDCANCEL from destroyAll()
						::SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
						delete this;
						WindowList_Remove(hWindowList, hwnd);
						::DestroyWindow(hwnd);
					}
					else {
						::SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
						delete this;
						WindowList_Remove(hWindowList, hwnd);
						::EndDialog(hwnd, LOWORD(wParam));
					}
					break;

				case IDC_DONTSHOWAGAIN: {
					__int64 mask = getMask(), val64 = ((__int64)1L << m_uId), newVal = 0;

					newVal = mask | val64;

					if(::IsDlgButtonChecked(hwnd, IDC_DONTSHOWAGAIN)) {
						DWORD val = (DWORD)(newVal & 0x00000000ffffffff);
						cfg::writeDword("CList", "cWarningsL", val);
						val = (DWORD)((newVal >> 32) & 0x00000000ffffffff);
						cfg::writeDword("CList", "cWarningsH", val);
					}
					break;
				}
				default:
					break;
			}
			break;

		case WM_NOTIFY: {
			switch (((NMHDR *) lParam)->code) {
				case EN_LINK:
					switch (((ENLINK *) lParam)->msg) {
						case WM_LBUTTONUP: {
							ENLINK* 		e = reinterpret_cast<ENLINK *>(lParam);

							const wchar_t*	wszUrl = Utils::extractURLFromRichEdit(e, ::GetDlgItem(hwnd, IDC_WARNTEXT));
							if(wszUrl) {
								char* szUrl = mir_t2a(wszUrl);

								Utils_OpenUrl(szUrl);
								mir_free(szUrl);
								mir_free(const_cast<TCHAR *>(wszUrl));
							}
							break;
						}
					}
					break;
				default:
					break;
			}
			break;
		}
		default:
			break;
	}
	return(FALSE);
}
