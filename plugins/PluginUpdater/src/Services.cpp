/*
Copyright (C) 2010 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "common.h"

static INT_PTR srvParseHashes(WPARAM wParam, LPARAM lParam)
{
	LPCTSTR ptszUrl = (LPCTSTR)wParam;
	LPTSTR  ptszBaseUrl = (LPTSTR)lParam;
	if (ptszUrl == NULL || ptszBaseUrl == NULL)
		return NULL;

	SERVLIST *pList = new SERVLIST(50, CompareHashes);
	ptrT baseUrl;
	if ( ParseHashes(ptszUrl, baseUrl, *pList)) {
		_tcsncpy(ptszBaseUrl, baseUrl, MAX_PATH);
		return (INT_PTR)pList;
	}

	delete pList;
	*ptszBaseUrl = 0;
	return NULL;
}

static INT_PTR srvFreeHashes(WPARAM, LPARAM lParam)
{
	SERVLIST *pList = (SERVLIST*)lParam;
	delete pList;
	return 0;
}

static INT_PTR srvGetHashCount(WPARAM, LPARAM lParam)
{
	SERVLIST *pList = (SERVLIST*)lParam;
	return (pList == NULL) ? 0 : pList->getCount();
}

static INT_PTR srvGetNthHash(WPARAM wParam, LPARAM lParam)
{
	SERVLIST *pList = (SERVLIST*)lParam;
	return (pList == NULL) ? 0 : INT_PTR(&(*pList)[wParam]);
}

void InitServices()
{
	CreateServiceFunction(MS_PU_PARSEHASHES,  srvParseHashes);
	CreateServiceFunction(MS_PU_FREEHASHES,   srvFreeHashes);
	CreateServiceFunction(MS_PU_GETHASHCOUNT, srvGetHashCount);
	CreateServiceFunction(MS_PU_GETNTHHASH,   srvGetNthHash);
}
