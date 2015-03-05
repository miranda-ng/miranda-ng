/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-15 Miranda NG project,
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
// utility functions for TabSRMM

#ifndef __UTILS_H
#define __UTILS_H

#define RTF_CTABLE_DEFSIZE 8

#define RTF_DEFAULT_HEADER _T("{\\rtf1\\ansi\\deff0\\pard\\li%u\\fi-%u\\ri%u\\tx%u")

#define CNT_KEYNAME "CNTW_Def"
#define CNT_BASEKEYNAME "CNTW_"

struct TRTFColorTable
{
	TCHAR    szName[10];
	COLORREF clr;
	int      menuid;
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
	static LPTSTR   FormatTitleBar(const TWindowData *dat, const TCHAR *szFormat);
	static char*    FilterEventMarkers(char *szText);
	static LPCTSTR  DoubleAmpersands(TCHAR *pszText);
	static void     RTF_CTableInit();
	static void     RTF_ColorAdd(const TCHAR *tszColname, size_t length);
	static int      ReadContainerSettingsFromDB(const MCONTACT hContact, TContainerSettings *cs, const char *szKey = 0);
	static int      WriteContainerSettingsToDB(const MCONTACT hContact, TContainerSettings *cs, const char *szKey = 0);
	static void     SettingsToContainer(TContainerData *pContainer);
	static void     ContainerToSettings(TContainerData *pContainer);
	static void     ReadPrivateContainerSettings(TContainerData *pContainer, bool fForce = false);
	static void     SaveContainerSettings(TContainerData *pContainer, const char *szSetting);

	static DWORD CALLBACK StreamOut(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb);
	static LRESULT  CmdDispatcher(UINT uType, HWND hwndDlg, UINT cmd, WPARAM wParam, LPARAM lParam, TWindowData *dat = 0, TContainerData *pContainer = 0);

	static void     addMenuItem(const HMENU& m, MENUITEMINFO& mii, HICON hIcon, const TCHAR *szText, UINT uID, UINT pos);
	static void     enableDlgControl(const HWND hwnd, UINT id, bool fEnable = true);
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

	static LRESULT  CALLBACK PopupDlgProcError(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LPTSTR   extractURLFromRichEdit(const ENLINK* _e, const HWND hwndRich);

	static size_t   CopyToClipBoard(const wchar_t *str, const HWND hwndOwner);

	static void     AddToFileList(TCHAR ***pppFiles, int *totalCount, LPCTSTR szFilename);

	//////////////////////////////////////////////////////////////////////////////////////
	// safe strlen function - do not overflow the given buffer length
	// if the buffer does not contain a valid (zero-terminated) string, it
	// will return 0.
	//
	// careful: maxlen must be given in element counts!!

	template<typename T> static size_t safe_strlen(const T* src, const size_t maxlen = 0)
	{
		size_t s = 0;

		while (s < maxlen && *(src++))
			s++;

		return (s >= maxlen && *src != 0) ? 0 : s;
	}

public:
	static	TRTFColorTable*		rtf_ctable;
	static	int					rtf_ctable_size;
};

__forceinline LRESULT _dlgReturn(HWND hWnd, LRESULT result)
{
	SetWindowLongPtr(hWnd, DWLP_MSGRESULT, result);
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
// implements a warning dialog with a "do not show this again" check
// box

class CWarning {

public:
	/*
	 * the warning IDs
	 */
	enum {
		WARN_RELNOTES = 0,
		WARN_ICONPACK_VERSION = 1,
		WARN_EDITUSERNOTES = 2,
		WARN_ICONPACKMISSING = 3,
		WARN_AEROPEEK_SKIN = 4,
		WARN_SENDFILE = 5,
		WARN_HPP_APICHECK = 6,
		WARN_NO_SENDLATER = 7,
		WARN_CLOSEWINDOW = 8,
		WARN_OPTION_CLOSE = 9,
		WARN_THEME_OVERWRITE = 10,
		WARN_LAST = 11
	};

	// the flags(low word is reserved for default windows flags like MB_OK etc.
	enum {
		CWF_UNTRANSLATED = 0x00010000, // do not translate the msg (useful for some error messages)
		CWF_NOALLOWHIDE = 0x00020000  // critical message, hide the "do not show this again" check box
	};

	CWarning(const wchar_t* tszTitle, const wchar_t* tszText, const UINT uId, const DWORD dwFlags);
	~CWarning();

public:
	// static function to construct and show the dialog, returns the user's choice
	static LRESULT show(const int uId, DWORD dwFlags = 0, const wchar_t* tszTxt = 0);
	static void destroyAll();
	LRESULT ShowDialog() const;

private:
	ptrT  m_szTitle, m_szText;
	UINT  m_uId;
	HFONT m_hFontCaption;
	DWORD m_dwFlags;
	HWND  m_hwnd;
	bool  m_fIsModal;

	INT_PTR CALLBACK dlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK	stubDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static __int64 getMask(); // get bit mask for disabled message classes

private:
	static	HANDLE			hWindowList;
};

#endif /* __UTILS_H */
