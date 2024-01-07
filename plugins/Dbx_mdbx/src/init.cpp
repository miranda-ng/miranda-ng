/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)
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

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE | STATIC_PLUGIN,
	// {7C3D0A33-2646-4001-9107-F35EA299D292}
	{ 0x7c3d0a33, 0x2646, 0x4001, { 0x91, 0x7, 0xf3, 0x5e, 0xa2, 0x99, 0xd2, 0x92 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(nullptr, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_DATABASE, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

static void logger(MDBX_log_level_t type, const char *function, int line, const char *msg, va_list args) MDBX_CXX17_NOEXCEPT
{
	char tmp[4096];
	_vsnprintf_s(tmp, _countof(tmp), msg, args);
	Netlib_Logf(nullptr, "MDBX[%d] (%s, %d): %s", type, function, line, tmp);
}

/////////////////////////////////////////////////////////////////////////////////////////

// returns 0 if the profile is created, EMKPRF*
static int makeDatabase(const wchar_t *profile)
{
	std::unique_ptr<CDbxMDBX> db(new CDbxMDBX(profile, 0));
	return db->Map();
}

// returns 0 if the given profile has a valid header
static int grokHeader(const wchar_t *profile)
{
	std::unique_ptr<CDbxMDBX> db(new CDbxMDBX(profile, DBMODE_SHARED | DBMODE_READONLY));
	return db->Check();
}

// returns a pointer to a database instance if all the APIs are injected, otherwise NULL
static MDatabaseCommon* loadDatabase(const wchar_t *profile, BOOL bReadOnly)
{
	// if not read only, convert the old profile to SQLITE
	if (!bReadOnly)
		return DB::Upgrade(profile);

	std::unique_ptr<CDbxMDBX> db(new CDbxMDBX(profile, (bReadOnly) ? DBMODE_READONLY : 0));
	if (db->Map() != ERROR_SUCCESS)
		return nullptr;

	if (db->Load() != ERROR_SUCCESS)
		return nullptr;

	return db.release();
}

static DATABASELINK dblink =
{
	MDB_CAPS_COMPACT | MDB_CAPS_CHECK,
	"dbx_mdbx",
	L"MDBX database driver",
	makeDatabase,
	grokHeader,
	loadDatabase
};

STDMETHODIMP_(DATABASELINK *) CDbxMDBX::GetDriver()
{
	return &dblink;
}

int CMPlugin::Load()
{
	mdbx_setup_debug(MDBX_LOG_WARN, MDBX_DBG_ASSERT, &logger);
	RegisterDatabasePlugin(&dblink);
	return 0;
}
