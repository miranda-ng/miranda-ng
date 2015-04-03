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

void CDbxKyoto::AddToList(char *name, DWORD ofs)
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

int CDbxKyoto::InitModuleNames(void)
{
	m_maxModuleID = 0;

	cursor_ptr cursor(m_dbModules);
	for (cursor->jump(); cursor->step();) {
		size_t size;
		const char *pRec, *pKey = cursor->get(&size, &pRec, &size);
		DBModuleName *pmod = (DBModuleName*)pRec;
		if (pmod->dwSignature != DBMODULENAME_SIGNATURE)
			DatabaseCorruption(NULL);

		char *pVal = (char*)HeapAlloc(m_hModHeap, 0, pmod->cbName+1);
		memcpy(pVal, pmod->name, pmod->cbName);
		pVal[pmod->cbName] = 0;

		int moduleId = *(int*)pKey;
		AddToList(pVal, moduleId);

		if (moduleId > m_maxModuleID)
			m_maxModuleID = moduleId;
		delete[] pKey;
	}

	return 0;
}

DWORD CDbxKyoto::FindExistingModuleNameOfs(const char *szName)
{
	ModuleName mn = { (char*)szName, 0 };

	int index = m_lMods.getIndex(&mn);
	if (index != -1)		
		return m_lMods[index]->ofs;

	return 0;
}

// will create the offset if it needs to
DWORD CDbxKyoto::GetModuleNameOfs(const char *szName)
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
	pmod->dwSignature = DBMODULENAME_SIGNATURE;
	pmod->cbName = (char)nameLen;
	strcpy(pmod->name, szName);
	m_dbModules.set((LPCSTR)&newIdx, sizeof(int), (LPCSTR)pmod, sizeof(DBModuleName) + nameLen);

	// add to cache
	char *mod = (char*)HeapAlloc(m_hModHeap, 0, nameLen + 1);
	strcpy(mod, szName);
	AddToList(mod, newIdx);

	// quit
	return -1;
}

char* CDbxKyoto::GetModuleNameByOfs(DWORD ofs)
{
	ModuleName mn = { NULL, ofs };
	int index = m_lOfs.getIndex(&mn);
	if (index != -1)
		return m_lOfs[index]->name;

	return NULL;
}

STDMETHODIMP_(BOOL) CDbxKyoto::EnumModuleNames(DBMODULEENUMPROC pFunc, void *pParam)
{
	for (int i = 0; i < m_lMods.getCount(); i++) {
		ModuleName *pmn = m_lMods[i];
		int ret = pFunc(pmn->name, pmn->ofs, (LPARAM)pParam);
		if (ret)
			return ret;
	}
	return 0;
}
