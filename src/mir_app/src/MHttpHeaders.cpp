/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-23 Miranda NG team,
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

#include "stdafx.h"

static int CompareHeaders(const MHttpHeader *p1, const MHttpHeader *p2)
{
	return strcmp(p1->szName, p2->szName);
}

MHttpHeaders::MHttpHeaders() :
	OBJLIST<MHttpHeader>(10, CompareHeaders)
{
}

MHttpHeaders::~MHttpHeaders()
{
}

void MHttpHeaders::AddHeader(const char *pszName, const char *pszValue)
{
	MHttpHeader tmp(pszName, 0);
	if (auto *p = find(&tmp))
		p->szValue = mir_strdup(pszValue);
	else
		insert(new MHttpHeader(pszName, pszValue));
}

void MHttpHeaders::DeleteHeader(const char *pszName)
{
	MHttpHeader tmp(pszName, 0);
	int idx = getIndex(&tmp);
	if (idx != -1)
		remove(idx);
}

char* MHttpHeaders::FindHeader(const char *pszName) const
{
	MHttpHeader tmp(pszName, 0);
	if (auto *p = find(&tmp))
		return p->szValue;
	return nullptr;
}

CMStringA MHttpHeaders::GetCookies() const
{
	CMStringA ret;

	for (auto &it : *this) {
		if (mir_strcmpi(it->szName, "Set-Cookie"))
			continue;

		CMStringA szCookie;
		if (const char *p = strchr(it->szValue, ';'))
			szCookie.Append(it->szValue, p - it->szValue.get());
		else
			szCookie = it->szValue;
		
		szCookie.TrimRight();

		if (!ret.IsEmpty())
			ret.Append("; ");
		ret += szCookie;
	}
	return ret;
}
