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
 * $Id: utils.h 128 2010-09-26 12:12:56Z silvercircle $
 */


#ifndef __UTILS_H_
#define __UTILS_H_

class Utils
{
public:
	static void TSAPI 		enableDlgControl		(const HWND hwnd, UINT id, BOOL fEnable);
	static void TSAPI 		showDlgControl			(const HWND hwnd, UINT id, int showCmd);
	static HMODULE 			loadSystemLibrary		(const wchar_t* szFilename, bool useGetHandle = false);
	static void 			extractResource			(const HMODULE h, const UINT uID, const wchar_t* tszName, const wchar_t* tszPath,
													 const wchar_t* tszFilename, bool fForceOverwrite);
	static void 			ensureTralingBackslash	(wchar_t *szPathname);
	static DWORD __fastcall	hexStringToLong			(const char *szSource);

	static int 				pathIsAbsolute			(const wchar_t* path);
	static size_t 			pathToRelative			(const wchar_t* pSrc, wchar_t* pOut, const wchar_t* szBase = 0);
	static size_t 			pathToAbsolute			(const wchar_t* pSrc, wchar_t* pOut, const wchar_t* szBase = 0);
	static const wchar_t*	striStr					(const wchar_t* szString, const wchar_t* szSearchFor);
	static const wchar_t* 	extractURLFromRichEdit	(const ENLINK* _e, const HWND hwndRich);

	template<class T, class U, class V, class W> 	static HRESULT	writeProfile(T* _section, U* _key, V* _value, W* _file)
	{
		size_t _s = sizeof(T);

		if(_s == 1)
			return(WritePrivateProfileStringA((char *)_section, (char *)_key, (char *)_value, (char *)_file));
		else
			return(WritePrivateProfileStringW((wchar_t *)_section, (wchar_t *)_key, (wchar_t *)_value, (wchar_t *)_file));
	}

	template<class T, class U, class V>				static HRESULT	writeProfile(T* _section, U* _key, DWORD _value, const V* _file)
	{
		T		sValue[1024];

		size_t _s = sizeof(T);
		if(_s == 1) {
			_snprintf((char *)sValue, 1024, "%d", _value);
			sValue[1023] = 0;
			return(WritePrivateProfileStringA((char *)_section, (char *)_key, (char *)sValue, (char *)_file));
		}
		else {
			_snwprintf((wchar_t *)sValue, 1024, L"%d", _value);
			sValue[1023] = 0;
			return(WritePrivateProfileStringW((wchar_t *)_section, (wchar_t *)_key, (wchar_t *)sValue, (wchar_t *)_file));
		}
	}
};

/**
 * implement a warning dialog with a "do not show this again" check
 * box
 */

class WarningDlg {

public:
	/*
	 * the warning IDs
	 */
	enum {
		WARN_RELNOTES						= 0,
		WARN_SKIN_LOADER_ERROR				= 1,
		WARN_SKIN_OVERWRITE					= 2,
		WARN_LAST							= 3,
	};

	/*
	 * the flags (low word is reserved for default windows flags like MB_OK etc.
	 */

	enum {
		CWF_UNTRANSLATED					= 0x00010000,			// do not translate the msg (useful for some error messages)
		CWF_NOALLOWHIDE						= 0x00020000			// critical message, hide the "do not show this again" check box
	};

	WarningDlg(const wchar_t* tszTitle, const wchar_t* tszText, const UINT uId, const DWORD dwFlags);
	~WarningDlg();

public:
	/*
	 * static function to construct and show the dialog, returns the
	 * user's choice
	 */
	static	LRESULT			show				(const int uId, DWORD dwFlags = 0, const wchar_t* tszTxt = 0);
	static	void			destroyAll			();
	LRESULT					ShowDialog			() const;

private:
	UINT							m_uId;
	HFONT							m_hFontCaption;
	DWORD							m_dwFlags;
	HWND							m_hwnd;
	bool							m_fIsModal;
	pSmartWstring					m_pszTitle, m_pszText;

	INT_PTR	CALLBACK		dlgProc				(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK	stubDlgProc			(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static  __int64			getMask				();		// get bit mask for disabled message classes

private:
	static	MWindowList		hWindowList;
	static	wchar_t*		m_warnings[WARN_LAST];
};

#endif /* __UTILS_H_ */
