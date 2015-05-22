/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

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

#include "commonheaders.h"

char* mir_strncat_c(char *pszDest, const char cSrc)
{
	size_t size = 2;
	if (pszDest != NULL)
		size += mir_strlen(pszDest); //cSrc = 1 + 1 forNULL temination 

	char *pszRet = (char *)mir_realloc(pszDest, (sizeof(char) * size));
	if (pszRet == NULL)
		return NULL;
	
	pszRet[size - 2] = cSrc;
	pszRet[size - 1] = 0;
	return pszRet;
}

wchar_t* mir_wcsncat_c(wchar_t *pwszDest, const wchar_t wcSrc)
{
	size_t size = 2;
	if (pwszDest != NULL)
		size += wcslen(pwszDest); //cSrc = 1 + 1 forNULL temination 

	wchar_t *pwszRet = (wchar_t *)mir_realloc(pwszDest, (sizeof(wchar_t) * size));
	if (pwszRet == NULL)
		return NULL;
	
	pwszRet[size - 2] = wcSrc;
	pwszRet[size - 1] = 0;
	return pwszRet;
}

char* mir_strnerase(char *pszDest, size_t sizeFrom, size_t sizeTo)
{
	char *pszReturn = NULL;
	size_t sizeNew, sizeLen = mir_strlen(pszDest);
	if (sizeFrom >= 0 && sizeFrom < sizeLen && sizeTo >= 0 && sizeTo <= sizeLen && sizeFrom < sizeTo) {
		sizeNew = sizeLen - (sizeTo - sizeFrom);
		size_t sizeCopy = sizeNew - sizeFrom;
		pszReturn = (char *)mir_alloc(sizeNew + 1);
		memcpy(pszReturn, pszDest, sizeFrom);
		memcpy(pszReturn + sizeFrom, pszDest + sizeTo, sizeCopy);
		pszReturn[sizeNew] = 0;
	}

	pszDest = (char *)mir_realloc(pszDest, sizeNew + 1);
	pszDest = mir_strcpy(pszDest, pszReturn);
	mir_free(pszReturn);
	return pszDest;
}

int mir_IsEmptyA(char *str)
{
	if (str == NULL || str[0] == 0)
		return 1;
	
	int i = 0;
	while (str[i]) {
		if (str[i] != ' ' &&
			str[i] != '\r' &&
			str[i] != '\n')
			return 0;
		i++;
	}
	return 1;
}

int mir_IsEmptyW(wchar_t *str)
{
	if (str == NULL || str[0] == 0)
		return 1;
	
	int i = 0;
	while (str[i]) {
		if (str[i] != ' ' &&
			str[i] != '\r' &&
			str[i] != '\n')
			return 0;
		i++;
	}
	return 1;
}
