/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-16 Miranda NG project (http://miranda-ng.org)
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

int CDbxMdb::InitModules()
{
	txn_ptr_ro trnlck(m_txn);
	cursor_ptr_ro cursor(m_curModules);

	MDB_val key, data;
	while (mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == MDB_SUCCESS) 
	{
		uint32_t iMod = *(uint32_t*)key.mv_data;
		const char *szMod = (const char*)data.mv_data;
		m_Modules[iMod] = szMod;
	}
	return 0;
}

// will create the offset if it needs to
uint32_t CDbxMdb::GetModuleID(const char *szName)
{
	uint32_t iHash = mir_hashstr(szName);
	if (m_Modules.find(iHash) == m_Modules.end())
	{
		MDB_val key = { sizeof(iHash), &iHash }, data = { strlen(szName) + 1, (void*)szName };

		for (;; Remap()) {
			txn_ptr txn(m_pMdbEnv);
			MDB_CHECK(mdb_put(txn, m_dbModules, &key, &data, 0), -1);
			if (txn.commit() == MDB_SUCCESS)
				break;
		}
		m_Modules[iHash] = szName;
	}

	return iHash;
}

char* CDbxMdb::GetModuleName(uint32_t dwId)
{
	auto it = m_Modules.find(dwId);
	return it != m_Modules.end() ? const_cast<char*>(it->second.c_str()) : nullptr;
}

STDMETHODIMP_(BOOL) CDbxMdb::EnumModuleNames(DBMODULEENUMPROC pFunc, void *pParam)
{
	for (auto it = m_Modules.begin(); it != m_Modules.end(); ++it)
		if (int ret = pFunc(it->second.c_str(), it->first, (LPARAM)pParam))
			return ret;

	return 0;
}
