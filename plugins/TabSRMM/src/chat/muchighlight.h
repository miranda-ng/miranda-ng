/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (c) 2012-14 Miranda NG project,
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
// highlighter class for multi user chats

class CMUCHighlight {

public:

	enum {
		MATCH_TEXT = 1,
		MATCH_NICKNAME = 2,
		MATCH_UIN = 4,
	};

	CMUCHighlight()
	{
		m_fInitialized = false;
		m_TextPatternString = m_NickPatternString = 0;
		m_NickPatterns = m_TextPatterns = 0;
		m_iNickPatterns = m_iTextPatterns = 0;
		m_dwFlags = 0;
		m_Valid = true;
		init();
	}

	~CMUCHighlight()
	{
		cleanup();
	}

	void init();
	void cleanup();
	int  match(const GCEVENT *pgce, const SESSION_INFO *psi, DWORD dwFlags = MATCH_NICKNAME);

	static INT_PTR CALLBACK dlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);		   // option page dlg proc
	static INT_PTR CALLBACK dlgProcAdd(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);		   // for the "add to" dialog

private:
	void    tokenize(TCHAR *tszString, TCHAR**& patterns, UINT& nr);

	DWORD   m_dwFlags;
	bool    m_fInitialized;
	TCHAR** m_NickPatterns;
	TCHAR** m_TextPatterns;
	UINT    m_iNickPatterns;
	UINT    m_iTextPatterns;
	TCHAR  *m_NickPatternString;
	TCHAR  *m_TextPatternString;
	bool    m_Valid;
	bool    m_fHighlightMe;
};

struct THighLightEdit
{
	enum {
		CMD_ADD = 1,
		CMD_EDIT = 2
	};

	UINT uCmd;
	SESSION_INFO *si;
	USERINFO *ui;
};
