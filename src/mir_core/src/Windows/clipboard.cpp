/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "../stdafx.h"

static UINT g_iRtf = 0;

MClipAnsi::MClipAnsi(const char *pszString) :
	m_szString(pszString)
{}

void MClipAnsi::Copy() const
{
	size_t cbLen = mir_strlen(m_szString);
	if (!cbLen)
		return;

	HGLOBAL hData = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, cbLen + 1);
	if (hData) {
		mir_strcpy((char *)GlobalLock(hData), m_szString);
		GlobalUnlock(hData);
		SetClipboardData(CF_TEXT, hData);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

MClipRtf::MClipRtf(const char *pszString) :
	m_szString(pszString)
{}

void MClipRtf::Copy() const
{
	size_t cbLen = mir_strlen(m_szString);
	if (!cbLen)
		return;

	if (g_iRtf == 0)
		g_iRtf = RegisterClipboardFormatW(CF_RTF);

	HGLOBAL hData = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, cbLen + 1);
	if (hData) {
		mir_strcpy((char *)GlobalLock(hData), m_szString);
		GlobalUnlock(hData);
		SetClipboardData(g_iRtf, hData);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

MClipUnicode::MClipUnicode(const wchar_t *pwszString) :
	m_wszString(pwszString)
{}

void MClipUnicode::Copy() const
{
	size_t cbLen = mir_wstrlen(m_wszString);
	if (!cbLen)
		return;

	HGLOBAL hData = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (cbLen + 1) * sizeof(wchar_t));
	if (hData) {
		mir_wstrcpy((wchar_t *)GlobalLock(hData), m_wszString);
		GlobalUnlock(hData);
		SetClipboardData(CF_UNICODETEXT, hData);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

MClipData& MClipData::operator+(const MClipData &p2)
{
	m_pNext = &p2;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(void) Utils_ClipboardCopy(const MClipData &pData)
{
	if (!OpenClipboard(nullptr))
		return;

	EmptyClipboard();

	for (const MClipData *p = &pData; p; p = p->m_pNext)
		p->Copy();

	CloseClipboard();
}
