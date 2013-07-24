/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project, 

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
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "global.h"

char	*mir_strncpy(char *pszDest, const char *pszSrc, const size_t cchDest)
{
	if (!pszDest || !pszSrc || !cchDest)
		return NULL;
	pszDest = strncpy(pszDest, pszSrc, cchDest-1);
	pszDest[cchDest-1] = 0;
	return pszDest;
}

wchar_t	*mir_wcsncpy(wchar_t *pszDest, const wchar_t *pszSrc, const size_t cchDest)
{
	if (!pszDest || !pszSrc || !cchDest)
		return NULL;
	pszDest = wcsncpy(pszDest, pszSrc, cchDest-1);
	pszDest[cchDest-1] = 0;
	return pszDest;
}

char	*mir_strncat(char *pszDest, const char *pszSrc, const size_t cchDest)
{
	if (!pszDest || !pszSrc || !cchDest)
		return NULL;
	strncat(pszDest, pszSrc, cchDest-1);
	pszDest[cchDest-1] = 0;
	return pszDest;
}

wchar_t	*mir_wcsncat(wchar_t *pszDest, const wchar_t *pszSrc, const size_t cchDest)
{
	if (!pszDest || !pszSrc || !cchDest)
		return NULL;
	pszDest = wcsncat(pszDest, pszSrc, cchDest-1);
	pszDest[cchDest-1] = 0;
	return pszDest;
}

char	*mir_strncat_c(char *pszDest, const char cSrc) {
	size_t lenNew = strlen(pszDest) + 2;
	if (!pszDest)
		pszDest = (char *) mir_alloc(sizeof(char) * lenNew);
	else
		pszDest = (char *) mir_realloc(pszDest, sizeof(char) * lenNew);
	pszDest[lenNew-2] = cSrc;
	pszDest[lenNew-1] = 0;
	return pszDest;
}

wchar_t	*mir_wcsncat_c(wchar_t *pwszDest, const wchar_t wcSrc) {
	size_t lenNew = wcslen(pwszDest) + 2;
	if (!pwszDest)
		pwszDest = (wchar_t *) mir_alloc(sizeof(wchar_t) * lenNew);
	else
		pwszDest = (wchar_t *) mir_realloc(pwszDest, sizeof(wchar_t) * lenNew);
	pwszDest[lenNew-2] = wcSrc;
	pwszDest[lenNew-1] = 0;
	return pwszDest;
}

char	*mir_strnerase(char *pszDest, size_t sizeFrom, size_t sizeTo) {
	char *pszReturn = NULL;
	size_t sizeNew, sizeLen = strlen(pszDest);
	if (sizeFrom >= 0 && sizeFrom < sizeLen && sizeTo >= 0 && sizeTo <= sizeLen && sizeFrom < sizeTo) {
		sizeNew = sizeLen - (sizeTo - sizeFrom);
		size_t sizeCopy = sizeNew - sizeFrom;
		pszReturn = (char *) mir_alloc(sizeNew + 1);
		memcpy(pszReturn, pszDest, sizeFrom);
		memcpy(pszReturn + sizeFrom, pszDest + sizeTo, sizeCopy);
		pszReturn[sizeNew] = 0;
	}

	pszDest = (char *) mir_realloc(pszDest, sizeNew + 1);
	pszDest = mir_strcpy(pszDest, pszReturn);
	mir_free(pszReturn);
	return pszDest;
}

//---------------------------------------------------------------------------
void	mir_stradd(char*	&pszDest, const char	*pszSrc)
{
	if(!pszSrc) {
		return;
	}
	else if(!pszDest) {
		pszDest = mir_strdup(pszSrc);
	}
	else {
		size_t lenDest  = strlen(pszDest);
		size_t lenSrc   = strlen(pszSrc);
		size_t lenNew = lenDest + lenSrc + 1;
		pszDest  = (char *) mir_realloc(pszDest, sizeof(char)* lenNew);
		
		strcpy(pszDest + lenDest, pszSrc);
		pszDest[lenNew-1] = 0;
	}
}

void	mir_wcsadd(wchar_t*	&pszDest, const wchar_t	*pszSrc)
{
	if(!pszSrc) {
		return;
	}
	else if(!pszDest) {
		pszDest = mir_wstrdup(pszSrc);
	}
	else {
		size_t lenDest  = wcslen(pszDest);
		size_t lenSrc   = wcslen(pszSrc);
		size_t lenNew = lenDest + lenSrc + 1;
		pszDest  = (wchar_t *) mir_realloc(pszDest, sizeof(wchar_t)*lenNew);
		
		wcscpy(pszDest + lenDest, pszSrc);
		pszDest[lenNew-1] = 0;
	}
}

