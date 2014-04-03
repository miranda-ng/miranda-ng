/*
Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,

This file is part of Send Screenshot Plus, a Miranda IM plugin.
Copyright (c) 2010 Ing.U.Horn

Parts of this file based on original sorce code
from UserInfoEx Plugin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "global.h"

wchar_t* mir_wcsncpy(wchar_t* pszDest, const wchar_t* pszSrc, const size_t cchDest)
{
	if (!pszDest || !pszSrc || !cchDest)
		return NULL;
	pszDest = wcsncpy(pszDest, pszSrc, cchDest-1);
	pszDest[cchDest-1] = 0;
	return pszDest;
}

wchar_t* mir_wcsncat(wchar_t* pszDest, const wchar_t* pszSrc, const size_t cchDest)
{
	if (!pszDest || !pszSrc || !cchDest)
		return NULL;
	pszDest = wcsncat(pszDest, pszSrc, cchDest-1);
	pszDest[cchDest-1] = 0;
	return pszDest;
}

//---------------------------------------------------------------------------
void mir_stradd(char* &pszDest, const char* pszSrc)
{
	if(!pszSrc)
		return;

	if(!pszDest)
		pszDest = mir_strdup(pszSrc);
	else {
		size_t lenDest  = strlen(pszDest);
		size_t lenSrc   = strlen(pszSrc);
		size_t lenNew = lenDest + lenSrc + 1;
		pszDest  = (char *) mir_realloc(pszDest, sizeof(char)* lenNew);

		strcpy(pszDest + lenDest, pszSrc);
		pszDest[lenNew-1] = 0;
	}
}

void mir_wcsadd(wchar_t* &pszDest, const wchar_t* pszSrc)
{
	if(!pszSrc)
		return;

	if(!pszDest)
		pszDest = mir_wstrdup(pszSrc);
	else {
		size_t lenDest  = wcslen(pszDest);
		size_t lenSrc   = wcslen(pszSrc);
		size_t lenNew = lenDest + lenSrc + 1;
		pszDest  = (wchar_t *) mir_realloc(pszDest, sizeof(wchar_t)*lenNew);

		wcscpy(pszDest + lenDest, pszSrc);
		pszDest[lenNew-1] = 0;
	}
}
