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

void CDb3Mmap::AddToList(char *name, DWORD ofs)
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

int CDb3Mmap::InitModuleNames(void)
{
	DWORD ofsThis = m_dbHeader.ofsModuleNames;
	DBModuleName *dbmn = (struct DBModuleName*)DBRead(ofsThis, NULL);
	while (ofsThis) {
		if (dbmn->signature != DBMODULENAME_SIGNATURE)
			DatabaseCorruption(NULL);

		int nameLen = dbmn->cbName;

		char *mod = (char*)HeapAlloc(m_hModHeap, 0, nameLen + 1);
		memcpy(mod, DBRead(ofsThis + offsetof(struct DBModuleName, name), NULL), nameLen);
		mod[nameLen] = 0;

		AddToList(mod, ofsThis);

		ofsThis = dbmn->ofsNext;
		dbmn = (struct DBModuleName*)DBRead(ofsThis, NULL);
	}
	return 0;
}

DWORD CDb3Mmap::FindExistingModuleNameOfs(const char *szName)
{
	ModuleName mn = { (char*)szName, 0 };
	if (m_lastmn && !mir_strcmp(mn.name, m_lastmn->name))
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
DWORD CDb3Mmap::GetModuleNameOfs(const char *szName)
{
	DWORD ofsExisting = FindExistingModuleNameOfs(szName);
	if (ofsExisting)
		return ofsExisting;

	if (m_bReadOnly)
		return 0;

	int nameLen = (int)mir_strlen(szName);

	// need to create the module name
	DWORD ofsNew = CreateNewSpace(nameLen + offsetof(struct DBModuleName, name));

	DBModuleName dbmn;
	dbmn.signature = DBMODULENAME_SIGNATURE;
	dbmn.cbName = nameLen;
	dbmn.ofsNext = m_dbHeader.ofsModuleNames;
	m_dbHeader.ofsModuleNames = ofsNew;
	DBWrite(0, &m_dbHeader, sizeof(m_dbHeader));
	DBWrite(ofsNew, &dbmn, offsetof(struct DBModuleName, name));
	DBWrite(ofsNew + offsetof(struct DBModuleName, name), (PVOID)szName, nameLen);
	DBFlush(0);

	// add to cache
	char *mod = (char*)HeapAlloc(m_hModHeap, 0, nameLen + 1);
	mir_strcpy(mod, szName);
	AddToList(mod, ofsNew);

	// quit
	return ofsNew;
}

char* CDb3Mmap::GetModuleNameByOfs(DWORD ofs)
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

	DatabaseCorruption(NULL);
	return NULL;
}

STDMETHODIMP_(BOOL) CDb3Mmap::EnumModuleNames(DBMODULEENUMPROC pFunc, void *pParam)
{
	for (int i = 0; i < m_lMods.getCount(); i++) {
		ModuleName *pmn = m_lMods[i];
		int ret = pFunc(pmn->name, pmn->ofs, (LPARAM)pParam);
		if (ret)
			return ret;
	}
	return 0;
}
