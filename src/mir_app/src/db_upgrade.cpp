/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team,
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

#define CONVERT_MSG LPGEN("This database is in the old format that isn't supported anymore. Press Yes to convert it to the new format or No to return")
#define MISSING_DB_MSG LPGEN("To open this database you need to install the Dbx_sqlite plugin. Click Yes to download it from Miranda NG's site or No to return")
#define MISSING_PLUG_MSG LPGEN("To open this database you need to install the Import plugin. Click Yes to download it from Miranda NG's site or No to return")

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(MDatabaseCommon*) DB::Upgrade(const wchar_t *profile)
{
	DATABASELINK *pLink = GetDatabasePlugin("dbx_sqlite");
	if (pLink == nullptr) {
		if (IDYES == MessageBoxW(nullptr, TranslateT(MISSING_DB_MSG), L"Miranda NG", MB_YESNO))
			Utils_OpenUrl("https://miranda-ng.org/p/Dbx_sqlite");
		return nullptr;
	}

	if (!Profile_GetSettingInt(L"Database/SilentUpgrade"))
		if (IDYES != MessageBoxW(nullptr, TranslateT(CONVERT_MSG), L"Miranda NG", MB_YESNO))
			return nullptr;

	int errorCode;
	CMStringW wszBackupName(profile);
	wszBackupName.Append(L".bak");
	DeleteFileW(wszBackupName);
	if (!MoveFileW(profile, wszBackupName)) {
		uint32_t dwError = GetLastError();
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

	if (SetServiceModePlugin("import", 1) != ERROR_SUCCESS) {
		if (IDYES == MessageBoxW(nullptr, TranslateT(MISSING_PLUG_MSG), L"Miranda NG", MB_YESNO))
			Utils_OpenUrl("https://miranda-ng.org/p/Import");
		goto LBL_Error;
	}

	return pLink->Load(profile, false);
}
