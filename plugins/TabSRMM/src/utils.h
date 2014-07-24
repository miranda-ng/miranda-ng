/*
 * Miranda NG: the free IM client for Microsoft* Windows*
 *
 * Copyright (c) 2000-09 Miranda ICQ/IM project,
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
 * utility functions for TabSRMM
 *
 */

#ifndef __UTILS_H
#define __UTILS_H

#define RTF_CTABLE_DEFSIZE 8

#define RTF_DEFAULT_HEADER _T("{\\rtf1\\ansi\\deff0\\pard\\li%u\\fi-%u\\ri%u\\tx%u")

#define CNT_KEYNAME "CNTW_Def"
#define CNT_BASEKEYNAME "CNTW_"

struct TRTFColorTable
{
    TCHAR 		szName[10];
    COLORREF 	clr;
    int 		index;
    int 		menuid;
};

static TRTFColorTable _rtf_ctable[] = {
	_T("red"), RGB(255, 0, 0), 0, ID_FONT_RED,
	_T("blue"), RGB(0, 0, 255), 0, ID_FONT_BLUE,
	_T("green"), RGB(0, 255, 0), 0, ID_FONT_GREEN,
	_T("magenta"), RGB(255, 0, 255), 0, ID_FONT_MAGENTA,
	_T("yellow"), RGB(255, 255, 0), 0, ID_FONT_YELLOW,
	_T("cyan"), RGB(0, 255, 255), 0, ID_FONT_CYAN,
	_T("black"), 0, 0, ID_FONT_BLACK,
	_T("white"), RGB(255, 255, 255), 0, ID_FONT_WHITE,
	_T(""), 0, 0, 0
};

class Utils {

public:
	enum {
		CMD_CONTAINER = 1,
		CMD_MSGDIALOG = 2,
		CMD_INFOPANEL = 4,
	};

	static int      FindRTLLocale(TWindowData *dat);
	static TCHAR*   GetPreviewWithEllipsis(TCHAR *szText, size_t iMaxLen);
	static TCHAR*   FilterEventMarkers(TCHAR *wszText);
	static LPCTSTR  FormatRaw(TWindowData *dat, const TCHAR *msg, int flags, BOOL isSent);
	static LPCTSTR  FormatTitleBar(const TWindowData *dat, const TCHAR *szFormat);
	static char*    FilterEventMarkers(char *szText);
	static LPCTSTR  DoubleAmpersands(TCHAR *pszText);
	static void     RTF_CTableInit();
	static void     RTF_ColorAdd(const TCHAR *tszColname, size_t length);
	static void     CreateColorMap(TCHAR *Text);
	static int      RTFColorToIndex(int iCol);
	static int      ReadContainerSettingsFromDB(const MCONTACT hContact, TContainerSettings *cs, const char *szKey = 0);
	static int      WriteContainerSettingsToDB(const MCONTACT hContact, TContainerSettings *cs, const char *szKey = 0);
	static void     SettingsToContainer(TContainerData *pContainer);
	static void     ContainerToSettings(TContainerData *pContainer);
	static void     ReadPrivateContainerSettings(TContainerData *pContainer, bool fForce = false);
	static void     SaveContainerSettings(TContainerData *pContainer, const char *szSetting);
	
	static DWORD CALLBACK StreamOut(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb);
	static LRESULT  CmdDispatcher(UINT uType, HWND hwndDlg, UINT cmd, WPARAM wParam, LPARAM lParam, TWindowData *dat = 0, TContainerData *pContainer = 0);

	static void     addMenuItem(const HMENU& m, MENUITEMINFO& mii, HICON hIcon, const TCHAR *szText, UINT uID, UINT pos);
	static void     enableDlgControl(const HWND hwnd, UINT id, BOOL fEnable = 1);
	static void     showDlgControl(const HWND hwnd, UINT id, int showCmd);
	static int      mustPlaySound(const TWindowData *dat);
	static HICON    iconFromAvatar(const TWindowData *dat);
	static void     setAvatarContact(HWND hWnd, MCONTACT hContact);
	static void     getIconSize(HICON hIcon, int& sizeX, int& sizeY);

	static bool     extractResource(const HMODULE h, const UINT uID, const TCHAR *tszName, const TCHAR *tszPath, const TCHAR *tszFilename, bool fForceOverwrite);
	static void     scaleAvatarHeightLimited(const HBITMAP hBm, double& dNewWidth, double& dNewHeight, const LONG maxHeight);

	static AVATARCACHEENTRY*  loadAvatarFromAVS(const MCONTACT hContact);

	static void     sanitizeFilename(wchar_t *tszFilename);
	static void     ensureTralingBackslash(wchar_t *szPathname);

	static void     sendContactMessage(MCONTACT hContact, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static HMODULE  loadSystemLibrary(const wchar_t* szFilename);

	static INT_PTR CALLBACK PopupDlgProcError(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LPCTSTR extractURLFromRichEdit(const ENLINK* _e, const HWND hwndRich);

	template<typename T> static size_t  CopyToClipBoard(T* _t, const HWND hwndOwner)
	{
		if (!OpenClipboard(hwndOwner) || _t == 0)
			return 0;

		std::basic_string<T> *s = new std::basic_string<T>(_t);
		size_t _s = sizeof(T);
		size_t i = _s * (s->length() + 1);

		EmptyClipboard();
		HGLOBAL hData = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, i);

		CopyMemory((void*)GlobalLock(hData), (void*)_t, i);
		GlobalUnlock(hData);
		SetClipboardData(_s == sizeof(char) ? CF_TEXT : CF_UNICODETEXT, hData);
		CloseClipboard();
		delete s;
		return(i);
	}

	template<typename T> static void AddToFileList(T ***pppFiles, int *totalCount, LPCTSTR szFilename)
	{
		size_t _s = sizeof(T);

		*pppFiles = (T**)mir_realloc(*pppFiles, (++*totalCount + 1) * sizeof(T*));
		(*pppFiles)[*totalCount] = NULL;

		if (_s == 1)
			(*pppFiles)[*totalCount-1] = reinterpret_cast<T *>(mir_t2a(szFilename));
		else
			(*pppFiles)[*totalCount-1] = reinterpret_cast<T *>(mir_tstrdup(szFilename));

		if (GetFileAttributes(szFilename) & FILE_ATTRIBUTE_DIRECTORY) {
			WIN32_FIND_DATA fd;
			HANDLE hFind;
			TCHAR szPath[MAX_PATH];

			lstrcpy(szPath, szFilename);
			lstrcat(szPath, _T("\\*"));
			if ((hFind = FindFirstFile(szPath, &fd)) != INVALID_HANDLE_VALUE) {
				do {
					if (!lstrcmp(fd.cFileName, _T(".")) || !lstrcmp(fd.cFileName, _T("..")))
						continue;
					lstrcpy(szPath, szFilename);
					lstrcat(szPath, _T("\\"));
					lstrcat(szPath, fd.cFileName);
					AddToFileList(pppFiles, totalCount, szPath);
				} while (FindNextFile(hFind, &fd));
				FindClose(hFind);
			}
		}
	}

	/**
	 * safe strlen function - do not overflow the given buffer length
	 * if the buffer does not contain a valid (zero-terminated) string, it
	 * will return 0.
	 *
	 * careful: maxlen must be given in element counts!!
	 */
	template<typename T> static size_t safe_strlen(const T* src, const size_t maxlen = 0)
	{
		size_t s = 0;

		while(s < maxlen && *(src++))
			s++;

		if (s >= maxlen && *src != 0)
			return 0;
		else
			return(s);
	}

public:
	static	TRTFColorTable*		rtf_ctable;
	static	int					rtf_ctable_size;
};

LRESULT 		_dlgReturn(HWND hWnd, LRESULT result);



/**
 * implement a warning dialog with a "do not show this again" check
 * box
 */

class CWarning {

public:
	/*
	 * the warning IDs
	 */
	enum {
		WARN_RELNOTES						= 0,
		WARN_ICONPACK_VERSION				= 1,
		WARN_EDITUSERNOTES					= 2,
		WARN_ICONPACKMISSING				= 3,
		WARN_AEROPEEK_SKIN					= 4,
		WARN_SENDFILE						= 5,
		WARN_HPP_APICHECK					= 6,
		WARN_NO_SENDLATER					= 7,
		WARN_CLOSEWINDOW					= 8,
		WARN_OPTION_CLOSE					= 9,
		WARN_THEME_OVERWRITE				= 10,
		WARN_LAST							= 11
	};

	/*
	 * the flags (low word is reserved for default windows flags like MB_OK etc.
	 */

	enum {
		CWF_UNTRANSLATED					= 0x00010000,			// do not translate the msg (useful for some error messages)
		CWF_NOALLOWHIDE						= 0x00020000			// critical message, hide the "do not show this again" check box
	};

	CWarning(const wchar_t* tszTitle, const wchar_t* tszText, const UINT uId, const DWORD dwFlags);
	~CWarning();

public:
	/*
	 * static function to construct and show the dialog, returns the
	 * user's choice
	 */
	static	LRESULT			show				(const int uId, DWORD dwFlags = 0, const wchar_t* tszTxt = 0);
	static	void			destroyAll			();
	LRESULT					ShowDialog			() const;

private:
	ptrT                    m_szTitle, m_szText;
	UINT							m_uId;
	HFONT							m_hFontCaption;
	DWORD							m_dwFlags;
	HWND							m_hwnd;
	bool							m_fIsModal;

	INT_PTR	CALLBACK		dlgProc				(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	//void					resize				() const;
	static INT_PTR CALLBACK	stubDlgProc			(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static  __int64			getMask				();		// get bit mask for disabled message classes

private:
	static	HANDLE			hWindowList;
};

#endif /* __UTILS_H */
