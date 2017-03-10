/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-17 Miranda NG project,
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
#include "database.h"

///////////////////////////////////////////////////////////////////////////////
// Database list

LIST<DATABASELINK> arDbPlugins(5);

MIR_APP_DLL(void) RegisterDatabasePlugin(DATABASELINK *pDescr)
{
	if (pDescr != nullptr)
		arDbPlugins.insert(pDescr);
}

MIR_APP_DLL(DATABASELINK*) FindDatabasePlugin(const wchar_t *ptszFileName)
{
	for (int i = arDbPlugins.getCount() - 1; i >= 0; i--) {
		int error = arDbPlugins[i]->grokHeader(ptszFileName);
		if (error == ERROR_SUCCESS || error == EGROKPRF_OBSOLETE)
			return arDbPlugins[i];
	}

	return nullptr;
}

MIR_APP_DLL(void) InitDbInstance(MIDatabase *pDatabase)
{
	if (pDatabase != nullptr)
		pDatabase->m_cache = new MDatabaseCache(pDatabase->GetContactSize());
}

MIR_APP_DLL(void) DestroyDbInstance(MIDatabase *pDatabase)
{
	if (pDatabase != nullptr) {
		MDatabaseCache *pCache = (MDatabaseCache*)pDatabase->m_cache;
		pDatabase->m_cache = nullptr;
		delete pCache;
	}
}

///////////////////////////////////////////////////////////////////////////////

int LoadDbintfModule()
{
	// create events once, they will be inherited by all database plugins
	CreateHookableEvent(ME_DB_CONTACT_DELETED);
	CreateHookableEvent(ME_DB_CONTACT_ADDED);
	CreateHookableEvent(ME_DB_CONTACT_SETTINGCHANGED);
	CreateHookableEvent(ME_DB_EVENT_MARKED_READ);

	CreateHookableEvent(ME_DB_EVENT_ADDED);
	CreateHookableEvent(ME_DB_EVENT_DELETED);
	CreateHookableEvent(ME_DB_EVENT_FILTER_ADD);
	return 0;
}
