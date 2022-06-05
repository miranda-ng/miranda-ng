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
// highlighter class for multi user chats

#pragma once

class CMUCHighlight {

public:

	enum {
		MATCH_TEXT = 1,
		MATCH_NICKNAME = 2,
		MATCH_UIN = 4,
	};

	CMUCHighlight()
	{
		init();
	}

	~CMUCHighlight()
	{
		cleanup();
	}

	void init();
	void cleanup();
	bool match(const GCEVENT *pgce, const SESSION_INFO *psi, uint32_t dwFlags = MATCH_NICKNAME);

private:
	void tokenize(wchar_t *tszString, wchar_t** &patterns, UINT &nr);

	uint32_t  m_iMode = 0; // combination of MATCH_* masks
	UINT      m_iNickPatterns = 0;
	UINT      m_iTextPatterns = 0;
	bool      m_fInitialized = false;
	bool      m_Valid = true;
	bool      m_fHighlightMe;
	wchar_t **m_NickPatterns = nullptr;
	wchar_t **m_TextPatterns = nullptr;
	wchar_t  *m_NickPatternString = nullptr;
	wchar_t  *m_TextPatternString = nullptr;
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
