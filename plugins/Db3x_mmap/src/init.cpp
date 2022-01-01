/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
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

CMPlugin g_plugin;

LIST<CDb3Mmap> g_Dbs(1, HandleKeySortT);

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_DATABASE, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE | STATIC_PLUGIN,
	//{F7A6B27C-9D9C-4A42-BE86-A448AE109161}
	{0xf7a6b27c, 0x9d9c, 0x4a42, {0xbe, 0x86, 0xa4, 0x48, 0xae, 0x10, 0x91, 0x61}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(nullptr, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

// returns 0 if the profile is created, EMKPRF*
static int makeDatabase(const wchar_t*)
{
	return EMKPRF_CREATEFAILED;
}

// returns 0 if the given profile has a valid header
static int grokHeader(const wchar_t *profile)
{
	std::unique_ptr<CDb3Mmap> db(new CDb3Mmap(profile, DBMODE_SHARED | DBMODE_READONLY));
	if (db->Load(true) != ERROR_SUCCESS)
		return EGROKPRF_CANTREAD;

	return db->CheckDbHeaders(false);
}

// returns a pointer to a database instance if all the APIs are injected, otherwise NULL
static MDatabaseCommon* LoadDatabase(const wchar_t *profile, BOOL bReadOnly)
{
	// if not read only, convert the old profile to libmdbx
	if (!bReadOnly)
		return DB::Upgrade(profile);

	std::unique_ptr<CDb3Mmap> db(new CDb3Mmap(profile, DBMODE_READONLY));
	if (db->Load(false) != ERROR_SUCCESS)
		return nullptr;

	g_Dbs.insert(db.get());
	return db.release();
}

static DATABASELINK dblink =
{
	0,
	"dbx_mmap",
	L"dbx mmap driver",
	makeDatabase,
	grokHeader,
	LoadDatabase
};

STDMETHODIMP_(DATABASELINK *) CDb3Mmap::GetDriver()
{
	return &dblink;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	RegisterDatabasePlugin(&dblink);
	return 0;
}
