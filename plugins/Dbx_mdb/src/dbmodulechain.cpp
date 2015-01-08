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

	m_lMods.insert(mn);
	m_lOfs.insert(mn);
}

int CDbxMdb::InitModuleNames(void)
{
	MDB_cursor *cursor;
	mdb_cursor_open(m_txn, m_dbModules, &cursor);

	int rc, moduleId;
	char moduleName[100];
	MDB_val key = { sizeof(int), &moduleId }, data = { sizeof(moduleName), moduleName };
	while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT)) == 0)
		AddToList(moduleName, moduleId);

	mdb_cursor_close(cursor);
	return 0;
}

DWORD CDbxMdb::FindExistingModuleNameOfs(const char *szName)
{
	ModuleName mn = { (char*)szName, 0 };
	if (m_lastmn && !strcmp(mn.name, m_lastmn->name))
		return m_lastmn->ofs;

	int index = m_lMods.getIndex(&mn);
	if (index != -1) {
		ModuleName *pmn = m_lMods[index];
		m_lastmn = pmn;
		return pmn->ofs;
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

	int nameLen = (int)strlen(szName);

	// need to create the module name

	// add to cache
	char *mod = (char*)HeapAlloc(m_hModHeap, 0, nameLen + 1);
	strcpy(mod, szName);
	AddToList(mod, -1);

	// quit
	return -1;
}

char* CDbxMdb::GetModuleNameByOfs(DWORD ofs)
{
	if (m_lastmn && m_lastmn->ofs == ofs)
		return m_lastmn->name;

	ModuleName mn = { NULL, ofs };
	int index = m_lOfs.getIndex(&mn);
	if (index != -1) {
		ModuleName *pmn = m_lOfs[index];
		m_lastmn = pmn;
		return pmn->name;
	}

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
