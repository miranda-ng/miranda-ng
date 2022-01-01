/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012-22 Miranda NG team,
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

#pragma once

struct MCryptoProvider : public CRYPTO_PROVIDER
{
	MCryptoProvider(const CRYPTO_PROVIDER *pProvider)
	{
		memcpy(this, pProvider, sizeof(*pProvider));
		pszName = mir_strdup(pszName);
		if (dwFlags & CPF_UNICODE)
			szDescr.w = mir_wstrdup(TranslateW_LP(pProvider->szDescr.w, pProvider->pPlugin));
		else
			szDescr.w = mir_a2u(TranslateA_LP(pProvider->szDescr.a, pProvider->pPlugin));
	}

	~MCryptoProvider()
	{
		mir_free(pszName);
		mir_free(szDescr.w);
	}
};

extern OBJLIST<MCryptoProvider> arCryptoProviders;

void InitCryptMenuItem(CMenuItem &mi);
