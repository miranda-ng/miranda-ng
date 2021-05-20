/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-21 Miranda NG team,
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

#include "stdafx.h"

void CMUCHighlight::cleanup()
{
	mir_free(m_NickPatternString);
	mir_free(m_TextPatternString);
	m_TextPatternString = m_NickPatternString = nullptr;

	mir_free(m_NickPatterns);
	mir_free(m_TextPatterns);
	m_iNickPatterns = m_iTextPatterns = 0;
	m_NickPatterns = m_TextPatterns = nullptr;
}

void CMUCHighlight::init()
{
	DBVARIANT dbv = { 0 };

	if (m_fInitialized)
		cleanup();							// clean up first, if we were already initialized

	m_fInitialized = true;

	if (0 == db_get_ws(0, CHAT_MODULE, "HighlightWords", &dbv)) {
		m_TextPatternString = dbv.pwszVal;
		wcslwr(m_TextPatternString);
	}

	if (0 == db_get_ws(0, CHAT_MODULE, "HighlightNames", &dbv))
		m_NickPatternString = dbv.pwszVal;

	m_iMode = db_get_b(0, CHAT_MODULE, "HighlightEnabled", MATCH_TEXT);
	m_fHighlightMe = (db_get_b(0, CHAT_MODULE, "HighlightMe", 1) ? true : false);

	tokenize(m_TextPatternString, m_TextPatterns, m_iTextPatterns);
	tokenize(m_NickPatternString, m_NickPatterns, m_iNickPatterns);
}

void CMUCHighlight::tokenize(wchar_t *tszString, wchar_t**& patterns, UINT& nr)
{
	if (tszString == nullptr)
		return;

	wchar_t	*p = tszString;

	if (*p == 0)
		return;

	nr = 0;

	if (*p != ' ')
		nr++;

	while (*p) {
		if (*p == ' ') {
			p++;
			while (*p && iswspace(*p))
				p++;
			if (*p)
				nr++;
		}
		p++;
	}
	patterns = (wchar_t **)mir_alloc(nr * sizeof(wchar_t*));

	p = tszString;
	nr = 0;

	if (*p != ' ')
		patterns[nr++] = p;

	while (*p) {
		if (*p == ' ') {
			*p++ = 0;
			while (*p && iswspace(*p))
				p++;
			if (*p)
				patterns[nr++] = p;
		}
		p++;
	}
}

bool CMUCHighlight::match(const GCEVENT *pgce, const SESSION_INFO *psi, DWORD dwFlags)
{
	int result = 0, nResult = 0;

	if (pgce == nullptr || m_Valid == false)
		return false;

	if ((m_iMode & MATCH_TEXT) && (dwFlags & MATCH_TEXT) && (m_fHighlightMe || m_iTextPatterns > 0) && psi != nullptr) {
		wchar_t *p = g_chatApi.RemoveFormatting(pgce->pszText.w);
		p = NEWWSTR_ALLOCA(p);
		if (p == nullptr)
			return false;
		CharLower(p);

		wchar_t *tszMe = (psi && psi->getMe()) ? NEWWSTR_ALLOCA(psi->getMe()->pszNick) : nullptr;
		if (tszMe)
			CharLower(tszMe);

		if (m_fHighlightMe && tszMe) {
			result = wcsstr(p, tszMe) ? MATCH_TEXT : 0;
			if (0 == m_iTextPatterns)
				goto skip_textpatterns;
		}

		while (p && !result) {
			while (*p && (*p == ' ' || *p == ',' || *p == '.' || *p == ':' || *p == ';' || *p == '?' || *p == '!'))
				p++;

			if (*p == 0)
				break;

			wchar_t *p1 = p;
			while (*p1 && *p1 != ' ' && *p1 != ',' && *p1 != '.' && *p1 != ':' && *p1 != ';' && *p1 != '?' && *p1 != '!')
				p1++;

			if (*p1)
				*p1 = 0;
			else
				p1 = nullptr;

			for (UINT i = 0; i < m_iTextPatterns && !result; i++)
				result = wildcmpw(p, m_TextPatterns[i]) ? MATCH_TEXT : 0;

			if (p1) {
				*p1 = ' ';
				p = p1 + 1;
			}
			else p = nullptr;
		}
	}

skip_textpatterns:

	// optionally, match the nickname against the list of nicks to highlight
	if ((m_iMode & MATCH_NICKNAME) && (dwFlags & MATCH_NICKNAME) && pgce->pszNick.w && m_iNickPatterns > 0) {
		for (UINT i = 0; i < m_iNickPatterns && !nResult; i++) {
			if (pgce->pszNick.w)
				nResult = wildcmpw(pgce->pszNick.w, m_NickPatterns[i]) ? MATCH_NICKNAME : 0;
			if ((m_iMode & MATCH_UIN) && pgce->pszUserInfo.w)
				nResult = wildcmpw(pgce->pszUserInfo.w, m_NickPatterns[i]) ? MATCH_NICKNAME : 0;
		}
	}

	return result || nResult;
}
