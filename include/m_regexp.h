/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef MIM_REGEXP_H
#define MIM_REGEXP_H

#include <pcre.h>

class MRegexp16
{
	pcre16 *m_pattern = nullptr;
	pcre16_extra *m_extra = nullptr;
	bool m_bIsValid = false;
	const wchar_t *m_prevText;
	int m_nMatches = 0, m_start = 0;
	int m_offsets[100];

	MRegexp16(const MRegexp16&); // never applied

public:
	MRegexp16()
	{}

	MRegexp16(const wchar_t *pwszPattern)
	{
		compile(pwszPattern);
	}

	void compile(const wchar_t *pwszPattern)
	{
		m_bIsValid = false;

		int erroffset;
		const char *err;
		m_pattern = ::pcre16_compile(pwszPattern, 0, &err, &erroffset, nullptr);
		if (m_pattern == nullptr)
			return;

		m_extra = ::pcre16_study(m_pattern, 0, &err);
		if (m_extra == nullptr)
			return;

		m_bIsValid = true;
	}

	~MRegexp16()
	{
		if (m_pattern) ::pcre16_free(m_pattern);
		if (m_extra) ::pcre16_free(m_extra);
	}

	__forceinline bool isValid() const { return m_bIsValid; }
	__forceinline int  numMatches() const { return m_nMatches; }

	__forceinline int getPos() const { return m_offsets[0]; }
	__forceinline int getLength() const { return m_offsets[1] - m_offsets[0]; }

	CMStringW getMatch()
	{
		return CMStringW(m_prevText + getPos(), getLength());
	}

	CMStringW getGroup(int i)
	{
		if (i >= m_nMatches)
			return L"";

		return CMStringW(m_prevText + m_offsets[i*2], m_offsets[i*2 + 1] - m_offsets[i*2]);
	}

	int match(const wchar_t *pwszText)
	{
		return m_nMatches = ::pcre16_exec(m_pattern, m_extra, m_prevText = pwszText, (int)mir_wstrlen(pwszText), 0, 0, m_offsets, _countof(m_offsets));
	}

	int nextMatch(const wchar_t *pwszText)
	{
		m_nMatches = ::pcre16_exec(m_pattern, m_extra, m_prevText = pwszText, (int)mir_wstrlen(pwszText), m_start, 0, m_offsets, _countof(m_offsets));
		m_start = (m_nMatches >= 0) ? m_offsets[1] : 0;

		return m_nMatches;
	}
};

#endif // MIM_REGEXP_H
