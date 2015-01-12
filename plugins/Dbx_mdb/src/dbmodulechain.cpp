/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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

#include "commonheaders.h"

void CDbxMdb::AddToList(char *name, DWORD ofs)
{
	ModuleName *mn = (ModuleName*)HeapAlloc(m_hModHeap, 0, sizeof(ModuleName));
	mn->name = name;
	mn->ofs = ofs;

	if (m_lMods.getIndex(mn) != -1)
		DatabaseCorruption(_T("%s (Module Name not unique)"));
	m_lMods.insert(mn);

	if (m_lOfs.getIndex(mn) != -1)
		DatabaseCorruption(_T("%s (Module Offset not unique)"));
	m_lOfs.insert(mn);
}

int CDbxMdb::InitModuleNames(void)
{
	m_maxModuleID = 0;

	txn_lock trnlck(m_pMdbEnv);
	mdb_open(trnlck, "modules", MDB_INTEGERKEY, &m_dbModules);

	MDB_cursor *cursor;
	if (mdb_cursor_open(trnlck, m_dbModules, &cursor) != MDB_SUCCESS)
		return 1;

	MDB_val key, data;
	
	while (mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == 0) {
		DBModuleName *pmod = (DBModuleName*)data.mv_data;
		if (pmod->signature != DBMODULENAME_SIGNATURE)
			DatabaseCorruption(NULL);

		char *pVal = (char*)HeapAlloc(m_hModHeap, 0, pmod->cbName+1);
		memcpy(pVal, pmod->name, pmod->cbName);
		pVal[pmod->cbName] = 0;

		int moduleId = *(int*)key.mv_data;
		AddToList(pVal, moduleId);

		if (moduleId > m_maxModuleID)
			m_maxModuleID = moduleId;
	}

	mdb_cursor_close(cursor);
	return 0;
}

DWORD CDbxMdb::FindExistingModuleNameOfs(const char *szName)
{
	ModuleName mn = { (char*)szName, 0 };

	int index = m_lMods.getIndex(&mn);
	if (index != -1)		
		return m_lMods[index]->ofs;

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

	int nameLen = (int)strlen(szName);

	// need to create the module name
	int newIdx = ++m_maxModuleID;
	DBModuleName *pmod = (DBModuleName*)_alloca(sizeof(DBModuleName) + nameLen);
	pmod->signature = DBMODULENAME_SIGNATURE;
	pmod->cbName = (char)nameLen;
	strcpy(pmod->name, szName);
	
	MDB_val key = { sizeof(int), &newIdx }, data = { sizeof(DBModuleName) + nameLen, pmod };

	txn_lock trnlck(m_pMdbEnv);
	mdb_open(trnlck, "modules", MDB_INTEGERKEY, &m_dbModules);
	mdb_put(trnlck, m_dbModules, &key, &data, 0);
	trnlck.commit();

	// add to cache
	char *mod = (char*)HeapAlloc(m_hModHeap, 0, nameLen + 1);
	strcpy(mod, szName);
	AddToList(mod, newIdx);

	// quit
	return -1;
}

char* CDbxMdb::GetModuleNameByOfs(DWORD ofs)
{
	ModuleName mn = { NULL, ofs };
	int index = m_lOfs.getIndex(&mn);
	if (index != -1)
		return m_lOfs[index]->name;

	return NULL;
}

STDMETHODIMP_(BOOL) CDbxMdb::EnumModuleNames(DBMODULEENUMPROC pFunc, void *pParam)
{
	for (int i = 0; i < m_lMods.getCount(); i++) {
		ModuleName *pmn = m_lMods[i];
		int ret = pFunc(pmn->name, pmn->ofs, (LPARAM)pParam);
		if (ret)
			return ret;
	}
	return 0;
}
