/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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
#include "encrypt.h"

/////////////////////////////////////////////////////////////////////////////////////////

static int CompareFunc(const MCryptoProvider *p1, const MCryptoProvider *p2)
{
	return mir_strcmp(p1->pszName, p2->pszName);
}

OBJLIST<MCryptoProvider> arCryptoProviders(5, CompareFunc);

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) Crypto_RegisterEngine(const CRYPTO_PROVIDER *pProvider)
{
	if (pProvider && pProvider->dwSize == sizeof(CRYPTO_PROVIDER))
		arCryptoProviders.insert(new MCryptoProvider(pProvider));
}

MIR_APP_DLL(void) Crypto_ListProviders(int *pCount, CRYPTO_PROVIDER ***pList)
{
	if (pCount)
		*pCount = arCryptoProviders.getCount();

	if (pList)
		*pList = (CRYPTO_PROVIDER**)arCryptoProviders.getArray();
}

MIR_APP_DLL(CRYPTO_PROVIDER*) Crypto_GetProvider(const char *pszName)
{
	if (pszName == 0)
		return nullptr;

	auto *tmp = (MCryptoProvider *)_alloca(sizeof(MCryptoProvider));
	tmp->pszName = (LPSTR)pszName;
	return arCryptoProviders.find(tmp);
}
