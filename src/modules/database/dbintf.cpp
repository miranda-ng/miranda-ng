/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-15 Miranda NG project,
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

#include "..\..\core\commonheaders.h"
#include "database.h"

MIDatabase *currDb = NULL;
DATABASELINK *currDblink = NULL;

MIR_CORE_DLL(void) db_setCurrent(MIDatabase*);

static INT_PTR srvSetSafetyMode(WPARAM wParam, LPARAM)
{
	if (!currDb) return 1;

	currDb->SetCacheSafetyMode(wParam != 0);
	return 0;
}

static INT_PTR srvGetContactCount(WPARAM, LPARAM)
{
	return (currDb) ? currDb->GetContactCount() : 0;
}

///////////////////////////////////////////////////////////////////////////////
// Contacts

static INT_PTR srvDeleteContact(WPARAM wParam, LPARAM)
{
	DBVARIANT dbv = {0};
	if (!db_get_ts(wParam, "ContactPhoto", "File", &dbv)) {
		DeleteFile(dbv.ptszVal);
		db_free(&dbv);
	}
	return (currDb) ? currDb->DeleteContact(wParam) : 0;
}

static INT_PTR srvAddContact(WPARAM wParam, LPARAM)
{
	MCONTACT hNew = (currDb) ? currDb->AddContact() : 0;
	Netlib_Logf(NULL, "New contact created: %d", hNew);
	return hNew;
}

static INT_PTR srvIsDbContact(WPARAM wParam, LPARAM)
{
	return (currDb) ? currDb->IsDbContact(wParam) : 0;
}

///////////////////////////////////////////////////////////////////////////////
// Module chain

static INT_PTR srvEnumModuleNames(WPARAM wParam,LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->EnumModuleNames((DBMODULEENUMPROC)lParam, (void*)wParam) : 0;
}

///////////////////////////////////////////////////////////////////////////////
// Settings

static INT_PTR srvEnumContactSettings(WPARAM wParam,LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->EnumContactSettings(wParam, (DBCONTACTENUMSETTINGS*)lParam) : 0;
}

static INT_PTR srvEnumResidentSettings(WPARAM wParam,LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->EnumResidentSettings((DBMODULEENUMPROC)wParam, (void*)lParam) : 0;
}

///////////////////////////////////////////////////////////////////////////////
// Database list

LIST<DATABASELINK> arDbPlugins(5);

static INT_PTR srvRegisterPlugin(WPARAM wParam,LPARAM lParam)
{
	DATABASELINK* pPlug = (DATABASELINK*)lParam;
	if (pPlug == NULL)
		return 1;

	arDbPlugins.insert(pPlug);
	return 0;
}

static INT_PTR srvFindPlugin(WPARAM wParam,LPARAM lParam)
{
	for (int i=arDbPlugins.getCount()-1; i >= 0; i--) {
		int error = arDbPlugins[i]->grokHeader((TCHAR*)lParam);
		if (error == ERROR_SUCCESS || error == EGROKPRF_OBSOLETE)
			return (INT_PTR)arDbPlugins[i];
	}

	return NULL;
}

static INT_PTR srvGetCurrentDb(WPARAM wParam,LPARAM lParam)
{
	return (INT_PTR)currDb;
}

static INT_PTR srvInitInstance(WPARAM wParam,LPARAM lParam)
{
	MIDatabase* pDb = (MIDatabase*)lParam;
	if (pDb != NULL)
		pDb->m_cache = new MDatabaseCache();
	return 0;
}

static INT_PTR srvDestroyInstance(WPARAM wParam,LPARAM lParam)
{
	MIDatabase* pDb = (MIDatabase*)lParam;
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
	CreateServiceFunction(MS_DB_CONTACT_GETCOUNT, srvGetContactCount);
	CreateServiceFunction(MS_DB_CONTACT_DELETE, srvDeleteContact);
	CreateServiceFunction(MS_DB_CONTACT_ADD, srvAddContact);
	CreateServiceFunction(MS_DB_CONTACT_IS, srvIsDbContact);

	CreateServiceFunction(MS_DB_MODULES_ENUM, srvEnumModuleNames);

	CreateServiceFunction(MS_DB_CONTACT_ENUMSETTINGS, srvEnumContactSettings);
	CreateServiceFunction("DB/ResidentSettings/Enum", srvEnumResidentSettings);

	CreateServiceFunction(MS_DB_REGISTER_PLUGIN, srvRegisterPlugin);
	CreateServiceFunction(MS_DB_FIND_PLUGIN, srvFindPlugin);
	CreateServiceFunction(MS_DB_GET_CURRENT, srvGetCurrentDb);

	CreateServiceFunction(MS_DB_INIT_INSTANCE, srvInitInstance);
	CreateServiceFunction(MS_DB_DESTROY_INSTANCE, srvDestroyInstance);
	return 0;
}

void LoadDatabaseServices()
{
	CreateServiceFunction(MS_DB_SETSAFETYMODE, srvSetSafetyMode);
}
