/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-13 Miranda NG project,
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

static INT_PTR srvFindFirstContact(WPARAM, LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->FindFirstContact((LPCSTR)lParam) : 0;
}

static INT_PTR srvFindNextContact(WPARAM wParam, LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->FindNextContact((HANDLE)wParam, (LPCSTR)lParam) : 0;
}

static INT_PTR srvDeleteContact(WPARAM wParam, LPARAM)
{
	return (currDb) ? currDb->DeleteContact((HANDLE)wParam) : 0;
}

static INT_PTR srvAddContact(WPARAM wParam, LPARAM)
{
	return (currDb) ? (INT_PTR)currDb->AddContact() : 0;
}

static INT_PTR srvIsDbContact(WPARAM wParam, LPARAM)
{
	return (currDb) ? currDb->IsDbContact((HANDLE)wParam) : 0;
}

///////////////////////////////////////////////////////////////////////////////
// Events

static INT_PTR srvGetEventCount(WPARAM wParam, LPARAM lParam)
{
	return (currDb) ? currDb->GetEventCount((HANDLE)wParam) : 0;
}

static INT_PTR srvAddEvent(WPARAM wParam, LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->AddEvent((HANDLE)wParam, (DBEVENTINFO*)lParam) : 0;
}

static INT_PTR srvDeleteEvent(WPARAM wParam, LPARAM lParam)
{
	return (currDb) ? currDb->DeleteEvent((HANDLE)wParam, (HANDLE)lParam) : 0;
}

static INT_PTR srvGetBlobSize(WPARAM wParam, LPARAM lParam)
{
	return (currDb) ? currDb->GetBlobSize((HANDLE)wParam) : 0;
}

static INT_PTR srvGetEvent(WPARAM wParam, LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->GetEvent((HANDLE)wParam, (DBEVENTINFO*)lParam) : 0;
}

static INT_PTR srvMarkEventRead(WPARAM wParam, LPARAM lParam)
{
	return (currDb) ? currDb->MarkEventRead((HANDLE)wParam, (HANDLE)lParam) : 0;
}

static INT_PTR srvGetEventContact(WPARAM wParam, LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->GetEventContact((HANDLE)wParam) : 0;
}

static INT_PTR srvFindFirstEvent(WPARAM wParam, LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->FindFirstEvent((HANDLE)wParam) : 0;
}

static INT_PTR srvFindFirstUnreadEvent(WPARAM wParam, LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->FindFirstUnreadEvent((HANDLE)wParam) : 0;
}

static INT_PTR srvFindLastEvent(WPARAM wParam, LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->FindLastEvent((HANDLE)wParam) : 0;
}

static INT_PTR srvFindNextEvent(WPARAM wParam, LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->FindNextEvent((HANDLE)wParam) : 0;
}

static INT_PTR srvFindPrevEvent(WPARAM wParam, LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->FindPrevEvent((HANDLE)wParam) : 0;
}

///////////////////////////////////////////////////////////////////////////////
// Module chain

static INT_PTR srvEnumModuleNames(WPARAM wParam,LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->EnumModuleNames((DBMODULEENUMPROC)lParam, (void*)wParam) : 0;
}

///////////////////////////////////////////////////////////////////////////////
// Settings

static INT_PTR srvGetContactSetting(WPARAM wParam,LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->GetContactSetting((HANDLE)wParam, (DBCONTACTGETSETTING*)lParam) : 0;
}

static INT_PTR srvGetContactSettingStr(WPARAM wParam,LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->GetContactSettingStr((HANDLE)wParam, (DBCONTACTGETSETTING*)lParam) : 0;
}

static INT_PTR srvGetContactSettingStatic(WPARAM wParam,LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->GetContactSettingStatic((HANDLE)wParam, (DBCONTACTGETSETTING*)lParam) : 0;
}

static INT_PTR srvFreeVariant(WPARAM wParam,LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->FreeVariant((DBVARIANT*)lParam) : 0;
}

static INT_PTR srvWriteContactSetting(WPARAM wParam,LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->WriteContactSetting((HANDLE)wParam, (DBCONTACTWRITESETTING*)lParam) : 0;
}

static INT_PTR srvDeleteContactSetting(WPARAM wParam,LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->DeleteContactSetting((HANDLE)wParam, (DBCONTACTGETSETTING*)lParam) : 0;
}

static INT_PTR srvEnumContactSettings(WPARAM wParam,LPARAM lParam)
{
	return (currDb) ? (INT_PTR)currDb->EnumContactSettings((HANDLE)wParam, (DBCONTACTENUMSETTINGS*)lParam) : 0;
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
		if (error == ERROR_SUCCESS)
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
	CreateServiceFunction(MS_DB_SETSAFETYMODE, srvSetSafetyMode);

	CreateServiceFunction(MS_DB_CONTACT_GETCOUNT, srvGetContactCount);
	CreateServiceFunction(MS_DB_CONTACT_FINDFIRST, srvFindFirstContact);
	CreateServiceFunction(MS_DB_CONTACT_FINDNEXT, srvFindNextContact);
	CreateServiceFunction(MS_DB_CONTACT_DELETE, srvDeleteContact);
	CreateServiceFunction(MS_DB_CONTACT_ADD, srvAddContact);
	CreateServiceFunction(MS_DB_CONTACT_IS, srvIsDbContact);

	CreateServiceFunction(MS_DB_EVENT_GETCOUNT, srvGetEventCount);
	CreateServiceFunction(MS_DB_EVENT_ADD, srvAddEvent);
	CreateServiceFunction(MS_DB_EVENT_DELETE, srvDeleteEvent);
	CreateServiceFunction(MS_DB_EVENT_GETBLOBSIZE, srvGetBlobSize);
	CreateServiceFunction(MS_DB_EVENT_GET, srvGetEvent);
	CreateServiceFunction(MS_DB_EVENT_MARKREAD, srvMarkEventRead);
	CreateServiceFunction(MS_DB_EVENT_GETCONTACT, srvGetEventContact);
	CreateServiceFunction(MS_DB_EVENT_FINDFIRST, srvFindFirstEvent);
	CreateServiceFunction(MS_DB_EVENT_FINDFIRSTUNREAD, srvFindFirstUnreadEvent);
	CreateServiceFunction(MS_DB_EVENT_FINDLAST, srvFindLastEvent);
	CreateServiceFunction(MS_DB_EVENT_FINDNEXT, srvFindNextEvent);
	CreateServiceFunction(MS_DB_EVENT_FINDPREV, srvFindPrevEvent);

	CreateServiceFunction(MS_DB_MODULES_ENUM, srvEnumModuleNames);

	CreateServiceFunction(MS_DB_CONTACT_GETSETTING, srvGetContactSetting);
	CreateServiceFunction(MS_DB_CONTACT_GETSETTING_STR, srvGetContactSettingStr);
	CreateServiceFunction(MS_DB_CONTACT_GETSETTINGSTATIC, srvGetContactSettingStatic);
	CreateServiceFunction(MS_DB_CONTACT_FREEVARIANT, srvFreeVariant);
	CreateServiceFunction(MS_DB_CONTACT_WRITESETTING, srvWriteContactSetting);
	CreateServiceFunction(MS_DB_CONTACT_DELETESETTING, srvDeleteContactSetting);
	CreateServiceFunction(MS_DB_CONTACT_ENUMSETTINGS, srvEnumContactSettings);
	CreateServiceFunction("DB/ResidentSettings/Enum", srvEnumResidentSettings);

	CreateServiceFunction(MS_DB_REGISTER_PLUGIN, srvRegisterPlugin);
	CreateServiceFunction(MS_DB_FIND_PLUGIN, srvFindPlugin);
	CreateServiceFunction(MS_DB_GET_CURRENT, srvGetCurrentDb);

	CreateServiceFunction(MS_DB_INIT_INSTANCE, srvInitInstance);
	CreateServiceFunction(MS_DB_DESTROY_INSTANCE, srvDestroyInstance);
	return 0;
}

void UnloadDbintfModule()
{
	arDbPlugins.destroy();
}
