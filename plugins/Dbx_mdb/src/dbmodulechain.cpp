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

void CDbxMdb::AddToList(const char *szName, DWORD dwId)
{
	size_t iNameLength = strlen(szName) + 1;
	ModuleName *mn = (ModuleName*)HeapAlloc(m_hModHeap, 0, sizeof(ModuleName) + iNameLength);
	mn->dwId = dwId;
	memcpy(&mn->szName, szName, iNameLength);

	if (m_lMods.getIndex(mn) != -1)
		DatabaseCorruption(_T("%s (Module Name not unique)"));
	m_lMods.insert(mn);

	if (m_lOfs.getIndex(mn) != -1)
		DatabaseCorruption(_T("%s (Module Offset not unique)"));
	m_lOfs.insert(mn);
}

int CDbxMdb::InitModuleNames(void)
{
	txn_ptr_ro trnlck(m_txn);
	cursor_ptr_ro cursor(m_curModules);

	MDB_val key, data;
	while (mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == MDB_SUCCESS) 
	{
		DWORD iMod = *(DWORD*)key.mv_data;
		const char *szMod = (const char*)data.mv_data;
		AddToList(szMod, iMod);
	}
	return 0;
}

thread_local ModuleName *t_lastmn = nullptr;

DWORD CDbxMdb::FindExistingModuleNameOfs(const char *szName)
{
	if (t_lastmn && !strcmp(szName, t_lastmn->szName))
		return t_lastmn->dwId;

	ModuleName *pmn = m_lMods.find((ModuleName*)(szName - sizeof(DWORD))); // crazy hack
	if (pmn != nullptr)
	{
		t_lastmn = pmn;
		return pmn->dwId;
	}
	return 0;
}

// will create the offset if it needs to
DWORD CDbxMdb::GetModuleNameOfs(const char *szName)
{
	DWORD ofsExisting = FindExistingModuleNameOfs(szName);
	if (ofsExisting)
		return ofsExisting;

	if (m_bReadOnly)
		return 0;

	size_t nameLen = strlen(szName);

//	mir_cslock lck(m_csDbAccess);

	// need to create the module name
	DWORD newIdx = InterlockedIncrement(&m_maxModuleID);
	
	MDB_val key = { sizeof(DWORD), &newIdx }, data = { nameLen + 1, (void*)szName };

	for (;; Remap()) {
		txn_ptr trnlck(m_pMdbEnv);
		MDB_CHECK(mdb_put(trnlck, m_dbModules, &key, &data, 0), -1);
		if (trnlck.commit() == MDB_SUCCESS)
			break;
	}

	AddToList(szName, newIdx);

	// quit
	return newIdx;
}

char* CDbxMdb::GetModuleNameByOfs(DWORD ofs)
{
	ModuleName *mn = m_lOfs.find((ModuleName*)&ofs);
	return mn ? mn->szName : nullptr;
}

STDMETHODIMP_(BOOL) CDbxMdb::EnumModuleNames(DBMODULEENUMPROC pFunc, void *pParam)
{
	for (int i = 0; i < m_lMods.getCount(); i++) {
		ModuleName *pmn = m_lMods[i];
		if (int ret = pFunc(pmn->szName, pmn->dwId, (LPARAM)pParam))
			return ret;
	}
	return 0;
}
