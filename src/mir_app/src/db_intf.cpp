/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-16 Miranda NG project,
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

static INT_PTR srvRegisterPlugin(WPARAM, LPARAM lParam)
{
	DATABASELINK* pPlug = (DATABASELINK*)lParam;
	if (pPlug == NULL)
		return 1;

	arDbPlugins.insert(pPlug);
	return 0;
}

static INT_PTR srvFindPlugin(WPARAM, LPARAM lParam)
{
	for (int i = arDbPlugins.getCount() - 1; i >= 0; i--) {
		int error = arDbPlugins[i]->grokHeader((wchar_t*)lParam);
		if (error == ERROR_SUCCESS || error == EGROKPRF_OBSOLETE)
			return (INT_PTR)arDbPlugins[i];
	}

	return NULL;
}

static INT_PTR srvInitInstance(WPARAM, LPARAM lParam)
{
	MIDatabase *pDb = (MIDatabase*)lParam;
	if (pDb != NULL)
		pDb->m_cache = new MDatabaseCache(pDb->GetContactSize());
	return 0;
}

static INT_PTR srvDestroyInstance(WPARAM, LPARAM lParam)
{
	MIDatabase *pDb = (MIDatabase*)lParam;
	if (pDb != NULL) {
		MDatabaseCache *pCache = (MDatabaseCache*)pDb->m_cache;
		pDb->m_cache = NULL;
		delete pCache;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

int LoadDbintfModule()
{
	CreateServiceFunction(MS_DB_REGISTER_PLUGIN, srvRegisterPlugin);
	CreateServiceFunction(MS_DB_FIND_PLUGIN, srvFindPlugin);

	CreateServiceFunction(MS_DB_INIT_INSTANCE, srvInitInstance);
	CreateServiceFunction(MS_DB_DESTROY_INSTANCE, srvDestroyInstance);

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
