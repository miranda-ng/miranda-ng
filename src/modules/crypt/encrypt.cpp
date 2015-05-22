/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"

/////////////////////////////////////////////////////////////////////////////////////////

static int CompareFunc(const CRYPTO_PROVIDER *p1, const CRYPTO_PROVIDER *p2)
{
	return mir_strcmp(p1->pszName, p2->pszName);
}

static LIST<CRYPTO_PROVIDER> arProviders(5, CompareFunc);

static INT_PTR srvRegister(WPARAM wParam, LPARAM lParam)
{
	CRYPTO_PROVIDER *p = (CRYPTO_PROVIDER*)lParam;
	if (p == NULL || p->dwSize != sizeof(CRYPTO_PROVIDER))
		return 1;

	CRYPTO_PROVIDER *pNew = new CRYPTO_PROVIDER(*p);
	pNew->pszName = mir_strdup(p->pszName);
	if (pNew->dwFlags & CPF_UNICODE)
		pNew->ptszDescr = mir_u2t(TranslateW_LP(p->pwszDescr, wParam));
	else
		pNew->ptszDescr = mir_a2t(TranslateA_LP(p->pszDescr, wParam));
	arProviders.insert(pNew);
	return 0;
}

static INT_PTR srvEnumProviders(WPARAM wParam, LPARAM lParam)
{
	if (wParam && lParam) {
		*(int*)wParam = arProviders.getCount();
		*(CRYPTO_PROVIDER***)lParam = arProviders.getArray();
	}
	return 0;
}

static INT_PTR srvGetProvider(WPARAM wParam, LPARAM lParam)
{
	if (lParam == 0)
		return 0;

	CRYPTO_PROVIDER tmp;
	tmp.pszName = (LPSTR)lParam;
	return (INT_PTR)arProviders.find(&tmp);
}

/////////////////////////////////////////////////////////////////////////////////////////

int InitCrypt(void)
{
	CreateServiceFunction(MS_CRYPTO_REGISTER_ENGINE, srvRegister);
	CreateServiceFunction(MS_CRYPTO_ENUM_PROVIDERS,  srvEnumProviders);
	CreateServiceFunction(MS_CRYPTO_GET_PROVIDER,    srvGetProvider);
	return 0;
}

void UninitCrypt(void)
{
	for (int i = 0; i < arProviders.getCount(); i++) {
		CRYPTO_PROVIDER *p = arProviders[i];
		mir_free(p->pszName);
		mir_free(p->pszDescr);
		delete p;
	}
}
