/////////////////////////////////////////////////////////////////////////////////////////
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
// utility functions for TabSRMM

#ifndef __UTILS_H
#define __UTILS_H

#define RTF_CTABLE_DEFSIZE 8

#define RTF_DEFAULT_HEADER L"{\\rtf1\\ansi\\deff0\\pard\\li%u\\fi-%u\\ri%u\\tx%u"

#define CNT_KEYNAME "CNTW_Def"
#define CNT_BASEKEYNAME "CNTW_"

struct TRTFColorTable
{
	__forceinline TRTFColorTable(const wchar_t *wszName, COLORREF _clr) :
		clr(_clr)
	{
		mir_wstrncpy(szName, wszName, _countof(szName));
	}

	wchar_t  szName[10];
	COLORREF clr;
};

namespace Utils
{
	wchar_t* GetPreviewWithEllipsis(wchar_t *szText, size_t iMaxLen);
	wchar_t* FilterEventMarkers(wchar_t *wszText);
	char*    FilterEventMarkers(char *szText);
	void     DoubleAmpersands(wchar_t *pszText, size_t len);
	void     RTF_CTableInit();
	void     RTF_ColorAdd(const wchar_t *tszColname);
	int      ReadContainerSettingsFromDB(const MCONTACT hContact, TContainerSettings *cs, const char *szKey = nullptr);
	int      WriteContainerSettingsToDB(const MCONTACT hContact, TContainerSettings *cs, const char *szKey = nullptr);

	uint32_t CALLBACK StreamOut(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb);

	void     addMenuItem(const HMENU& m, MENUITEMINFO& mii, HICON hIcon, const wchar_t *szText, UINT uID, UINT pos);
	void     enableDlgControl(const HWND hwnd, UINT id, bool fEnable = true);
	void     showDlgControl(const HWND hwnd, UINT id, int showCmd);
	void     setAvatarContact(HWND hWnd, MCONTACT hContact);
	void     getIconSize(HICON hIcon, int& sizeX, int& sizeY);

	bool     extractResource(const HMODULE h, const UINT uID, const wchar_t *tszName, const wchar_t *tszPath, const wchar_t *tszFilename, bool fForceOverwrite);
	void     scaleAvatarHeightLimited(const HBITMAP hBm, double& dNewWidth, double& dNewHeight, const LONG maxHeight);

	AVATARCACHEENTRY*  loadAvatarFromAVS(const MCONTACT hContact);

	void     sanitizeFilename(wchar_t *tszFilename);
	void     ensureTralingBackslash(wchar_t *szPathname);

	void     sendContactMessage(MCONTACT hContact, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HMODULE  loadSystemLibrary(const wchar_t* szFilename);

	LRESULT  CALLBACK PopupDlgProcError(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LPTSTR   extractURLFromRichEdit(const ENLINK* _e, const HWND hwndRich);

	//////////////////////////////////////////////////////////////////////////////////////
	// safe mir_strlen function - do not overflow the given buffer length
	// if the buffer does not contain a valid (zero-terminated) string, it
	// will return 0.
	//
	// careful: maxlen must be given in element counts!!

	template<typename T> size_t safe_strlen(const T* src, const size_t maxlen = 0)
	{
		size_t s = 0;

		while (s < maxlen && *(src++))
			s++;

		return (s >= maxlen && *src != 0) ? 0 : s;
	}

	extern OBJLIST<TRTFColorTable> rtf_clrs;
};

__forceinline LRESULT _dlgReturn(HWND hWnd, LRESULT result)
{
	SetWindowLongPtr(hWnd, DWLP_MSGRESULT, result);
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
// implements a warning dialog with a "do not show this again" check
// box

namespace CWarning
{
	// warning IDs
	enum
	{
		WARN_SAVEFILE = 1,
		WARN_EDITUSERNOTES,
		WARN_ICONPACKMISSING,
		WARN_AEROPEEK_SKIN,
		WARN_SENDFILE,
		WARN_HPP_APICHECK,
		WARN_NO_SENDLATER,
		WARN_CLOSEWINDOW,
		WARN_OPTION_CLOSE,
		WARN_THEME_OVERWRITE,
		WARN_LAST
	};

	// the flags(low word is reserved for default windows flags like MB_OK etc.
	enum
	{
		CWF_UNTRANSLATED = 0x00010000, // do not translate the msg (useful for some error messages)
		CWF_NOALLOWHIDE = 0x00020000  // critical message, hide the "do not show this again" check box
	};

	LRESULT show(const int uId, uint32_t dwFlags = 0, const wchar_t *tszTxt = nullptr);
	void destroyAll();
}

#endif /* __UTILS_H */
