/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)
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

// returns 0 if all the APIs are injected otherwise, 1
#define CONVERT_MSG LPGEN("This database is in old format that isn't supported anymore. Press Yes to convert it to the new format or No to return back")
#define MISSING_DB_MSG LPGEN("To open this database you need to install the dbx_mdbx plugin. Click Yes to download it from Miranda NG's site or No to return back")
#define MISSING_PLUG_MSG LPGEN("To open this database you need to install the Import plugin. Click Yes to download it from Miranda NG's site or No to return back")

static MDatabaseCommon* LoadDatabase(const wchar_t *profile, BOOL bReadOnly)
{
	////////////////////////////////////////////////////////////////////////////////////////
	// if not read only, convert the old profile to libmdbx
	if (!bReadOnly) {
		DATABASELINK *pLink = GetDatabasePlugin("dbx_mdbx");
		if (pLink == nullptr) {
			if (IDYES == MessageBoxW(nullptr, TranslateT(MISSING_DB_MSG), L"Miranda NG", MB_YESNO))
				Utils_OpenUrl("https://miranda-ng.org/p/Dbx_mdbx");
			return nullptr;
		}

		if (!Profile_GetSettingInt(L"Database/SilentUpgrade"))
			if (IDYES != MessageBoxW(nullptr, TranslateT(CONVERT_MSG), L"Miranda NG", MB_YESNO))
				return nullptr;

		int errorCode;
		CMStringW wszBackupName(profile);
		wszBackupName.Append(L".bak");
		if (!MoveFileW(profile, wszBackupName)) {
			DWORD dwError = GetLastError();
			CMStringW wszError(FORMAT, TranslateT("Cannot move old profile '%s' to '%s': error %d"), profile, wszBackupName.c_str(), dwError);
			MessageBoxW(nullptr, wszError, L"Miranda NG", MB_ICONERROR | MB_OK);
			return nullptr;
		}

		if ((errorCode = pLink->makeDatabase(profile)) != 0) {
			MessageBoxW(nullptr, CMStringW(FORMAT, TranslateT("Attempt to create database '%s' failed with error code %d"), profile, errorCode), L"Miranda NG", MB_ICONERROR | MB_OK);
LBL_Error:
			DeleteFileW(profile);
			MoveFileW(wszBackupName, profile);
			return nullptr;
		}

		if (SetServiceModePlugin(L"import", 1) != ERROR_SUCCESS) {
			if (IDYES == MessageBoxW(nullptr, TranslateT(MISSING_PLUG_MSG), L"Miranda NG", MB_YESNO))
				Utils_OpenUrl("https://miranda-ng.org/p/Import");
			goto LBL_Error;
		}

		return pLink->Load(profile, false);
	}

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

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	RegisterDatabasePlugin(&dblink);
	return 0;
}
