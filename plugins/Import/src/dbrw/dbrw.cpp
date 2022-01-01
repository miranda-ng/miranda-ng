/*

Import plugin for Miranda NG

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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

#include "../stdafx.h"

static int dbrw_makeDatabase(const wchar_t*)
{
	return 1;
}

static int dbrw_grokHeader(const wchar_t *profile)
{
	HANDLE hFile = CreateFile(profile, GENERIC_READ, 0, NULL, OPEN_ALWAYS, 0, NULL);
	int rc = 1;
	int err = EGROKPRF_CANTREAD;

	if (hFile != INVALID_HANDLE_VALUE) {
		BOOL r;
		char buf[64];
		DWORD dwRead;

		ZeroMemory(buf, sizeof(buf));
		r = ReadFile(hFile, buf, sizeof(buf), &dwRead, NULL);
		CloseHandle(hFile);
		if (r && memcmp(buf, DBRW_HEADER_STR, strlen(DBRW_HEADER_STR)) == 0) {
			sqlite3 *sqlcheck = NULL;
			char *szPath = mir_utf8encodeW(profile);

			rc = sqlite3_open(szPath, &sqlcheck);
			mir_free(szPath);
			if (rc == SQLITE_OK) {
				sqlite3_stmt *stmt;
				err = EGROKPRF_UNKHEADER;

				sqlite3_prepare_v2(sqlcheck, "select * from sqlite_master where type = 'table' and name = 'dbrw_core';", -1, &stmt, NULL);
				if (sqlite3_step(stmt) == SQLITE_ROW) {

					sqlite3_finalize(stmt);
					sqlite3_prepare_v2(sqlcheck, "select val from dbrw_core where setting = 'SchemaVersion';", -1, &stmt, NULL);
					if (sqlite3_step(stmt) == SQLITE_ROW) {
						int sVersion;

						sVersion = sqlite3_column_int(stmt, 0);
						if (sVersion == atoi(DBRW_SCHEMA_VERSION))
							rc = 0;
						else {
							// TODO: Return as valid and upgrade in 
							// dbrw_Load() if schema version is upgradable
						}
					}
				}
				sqlite3_finalize(stmt);
				sqlite3_close(sqlcheck);
			}
		}
		else err = r ? EGROKPRF_UNKHEADER : EGROKPRF_CANTREAD;
	}
	return rc;
}

static MDatabaseCommon* dbrw_Load(const wchar_t *profile, BOOL)
{
	CDbxSQLite *db = new CDbxSQLite();
	db->Open(profile);
	return db;
}

static DATABASELINK dblink =
{
	0,
	"dbrw",
	L"dbx SQLite driver",
	dbrw_makeDatabase,
	dbrw_grokHeader,
	dbrw_Load
};

STDMETHODIMP_(DATABASELINK *) CDbxSQLite::GetDriver()
{
	return &g_patternDbLink;
}

void RegisterDbrw()
{
	RegisterDatabasePlugin(&dblink);
}
