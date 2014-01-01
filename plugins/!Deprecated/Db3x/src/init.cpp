/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012-14 Miranda NG project,
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
	//{1394A3AB-2585-4196-8F72-0EAEC2450E11}
    {0x1394a3ab, 0x2585, 0x4196, {0x8f, 0x72, 0xe, 0xae, 0xc2, 0x45, 0xe, 0x11}}
};

HINSTANCE g_hInst = NULL;

LIST<CDb3x> g_Dbs(1, HandleKeySortT);

/////////////////////////////////////////////////////////////////////////////////////////

// returns 0 if the profile is created, EMKPRF*
static int makeDatabase(const TCHAR *profile)
{
	std::auto_ptr<CDb3x> db(new CDb3x(profile));
	if (db->Create() != ERROR_SUCCESS)
		return EMKPRF_CREATEFAILED;

	return db->CreateDbHeaders(dbSignature);
}

// returns 0 if the given profile has a valid header
static int grokHeader(const TCHAR *profile)
{
	std::auto_ptr<CDb3x> db(new CDb3x(profile));
	if (db->Load(true) != ERROR_SUCCESS)
		return EGROKPRF_CANTREAD;

	return db->CheckDbHeaders();
}

// returns 0 if all the APIs are injected otherwise, 1
static MIDatabase* LoadDatabase(const TCHAR *profile)
{
	// set the memory, lists & UTF8 manager
	mir_getLP( &pluginInfo );

	std::auto_ptr<CDb3x> db(new CDb3x(profile));
	if (db->Load(false) != ERROR_SUCCESS)
		return NULL;

	g_Dbs.insert(db.get());
	return db.release();
}

static int UnloadDatabase(MIDatabase* db)
{
	g_Dbs.remove((CDb3x*)db);
	delete (CDb3x*)db;
	return 0;
}

MIDatabaseChecker* CheckDb(const TCHAR* profile, int *error)
{
	std::auto_ptr<CDb3x> db(new CDb3x(profile));
	if (db->Load(true) != ERROR_SUCCESS) {
		if (error != NULL) *error = EGROKPRF_CANTREAD;
		return NULL;
	}

	int chk = db->CheckDbHeaders();
	if (chk != ERROR_SUCCESS) {
		*error = chk;
		return NULL;
	}

	*error = 0;
	return db.release();
}

static DATABASELINK dblink =
{
	sizeof(DATABASELINK),
	"db3x",
	_T("db3x driver"),
	makeDatabase,
	grokHeader,
	LoadDatabase,
	UnloadDatabase,
	CheckDb
};

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_DATABASE, MIID_LAST};

extern "C" __declspec(dllexport) int Load(void)
{
	RegisterDatabasePlugin(&dblink);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	g_Dbs.destroy();
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
	g_hInst = hInstDLL;
	return TRUE;
}
