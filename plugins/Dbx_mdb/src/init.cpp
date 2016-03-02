/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (�) 2012-16 Miranda NG project (http://miranda-ng.org)
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

int hLangpack;

static PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE | STATIC_PLUGIN,
	// {7C3D0A33-2646-4001-9107-F35EA299D292}
	{ 0x7c3d0a33, 0x2646, 0x4001, { 0x91, 0x7, 0xf3, 0x5e, 0xa2, 0x99, 0xd2, 0x92 } }
};

HINSTANCE g_hInst = NULL;

LIST<CDbxMdb> g_Dbs(1, HandleKeySortT);

/////////////////////////////////////////////////////////////////////////////////////////

// returns 0 if the profile is created, EMKPRF*
static int makeDatabase(const TCHAR *profile)
{
	std::auto_ptr<CDbxMdb> db(new CDbxMdb(profile, 0));
	return db->Create();
}

// returns 0 if the given profile has a valid header
static int grokHeader(const TCHAR *profile)
{
	std::auto_ptr<CDbxMdb> db(new CDbxMdb(profile, DBMODE_SHARED | DBMODE_READONLY));
	return db->Check();
}

// returns 0 if all the APIs are injected otherwise, 1
static MIDatabase* LoadDatabase(const TCHAR *profile, BOOL bReadOnly)
{
	// set the memory, lists & UTF8 manager
	mir_getLP(&pluginInfo);

	std::auto_ptr<CDbxMdb> db(new CDbxMdb(profile, (bReadOnly) ? DBMODE_READONLY : 0));
	if (db->Load(false) != ERROR_SUCCESS)
		return NULL;

	g_Dbs.insert(db.get());
	return db.release();
}

static int UnloadDatabase(MIDatabase *db)
{
	g_Dbs.remove((CDbxMdb*)db);
	delete (CDbxMdb*)db;
	return 0;
}

MIDatabaseChecker* CheckDb(const TCHAR *profile, int *error)
{
	std::auto_ptr<CDbxMdb> db(new CDbxMdb(profile, DBMODE_READONLY));
	if (db->Load(true) != ERROR_SUCCESS) {
		*error = ERROR_ACCESS_DENIED;
		return NULL;
	}

	if (db->PrepareCheck(error))
		return NULL;

	return db.release();
}

static DATABASELINK dblink =
{
	sizeof(DATABASELINK),
	"dbx_lmdb",
	_T("LMDB database driver"),
	makeDatabase,
	grokHeader,
	LoadDatabase,
	UnloadDatabase,
	CheckDb
};

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_DATABASE, MIID_LAST };

extern "C" __declspec(dllexport) int Load(void)
{
	RegisterDatabasePlugin(&dblink);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD, LPVOID)
{
	g_hInst = hInstDLL;
	return TRUE;
}
