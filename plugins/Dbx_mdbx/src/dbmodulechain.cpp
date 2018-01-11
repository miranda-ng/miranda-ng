/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org)
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

int CDbxMDBX::InitModules()
{
	txn_ptr_ro trnlck(m_txn);
	cursor_ptr_ro cursor(m_curModules);

	MDBX_val key, data;
	while (mdbx_cursor_get(cursor, &key, &data, MDBX_NEXT) == MDBX_SUCCESS) {
		uint32_t iMod = *(uint32_t*)key.iov_base;
		const char *szMod = (const char*)data.iov_base;
		m_Modules[iMod] = szMod;
	}
	return 0;
}

// will create the offset if it needs to
uint32_t CDbxMDBX::GetModuleID(const char *szName)
{
	uint32_t iHash = mir_hashstr(szName);
	if (m_Modules.find(iHash) == m_Modules.end()) {
		MDBX_val key = { &iHash, sizeof(iHash) }, data = { (void*)szName, strlen(szName) + 1 };
		{
			txn_ptr txn(m_env);
			if (mdbx_put(txn, m_dbModules, &key, &data, 0) != MDBX_SUCCESS)
				return -1;
			if (txn.commit() != MDBX_SUCCESS)
				return -1;
		}

		m_Modules[iHash] = szName;
	}

	return iHash;
}

char* CDbxMDBX::GetModuleName(uint32_t dwId)
{
	auto it = m_Modules.find(dwId);
	return it != m_Modules.end() ? const_cast<char*>(it->second.c_str()) : nullptr;
}

STDMETHODIMP_(BOOL) CDbxMDBX::EnumModuleNames(DBMODULEENUMPROC pFunc, void *pParam)
{
	for (auto it = m_Modules.begin(); it != m_Modules.end(); ++it)
		if (int ret = pFunc(it->second.c_str(), pParam))
			return ret;

	return 0;
}
