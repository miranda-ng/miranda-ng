/*
    Helper functions for Miranda-IM (www.miranda-im.org)
    Copyright 2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "db_helpers.h"

struct RemoveSettings{
	char *szPrefix;
	int count;
	char **szSettings;
};

static int DBRemoveEnumProc(const char *szSetting, LPARAM lParam) {

	struct RemoveSettings *rs;

	rs = (struct RemoveSettings *)lParam;
	if (!strncmp(szSetting, rs->szPrefix, strlen(rs->szPrefix))) {
		rs->szSettings = ( char** )realloc(rs->szSettings, (rs->count+1)*sizeof(char *));
		rs->szSettings[rs->count] = _strdup(szSetting);
		rs->count += 1;
	}
	
	return 0;
}

int Hlp_RemoveDatabaseSettings(HANDLE hContact, char *szModule, char *szPrefix) {

	DBCONTACTENUMSETTINGS dbces;
	struct RemoveSettings rs;
	int i, count;

	ZeroMemory(&rs, sizeof(struct RemoveSettings));
	rs.szPrefix = szPrefix;
	ZeroMemory(&dbces, sizeof(DBCONTACTENUMSETTINGS));
	dbces.pfnEnumProc = DBRemoveEnumProc;
	dbces.lParam = (LPARAM)&rs;
	dbces.szModule = szModule;
	if (CallService(MS_DB_CONTACT_ENUMSETTINGS, (WPARAM)(HANDLE)hContact, (LPARAM)&dbces) == -1) {
		
		return -1;
	}
	count = 0;
	if (rs.szSettings != NULL) {
		for (i=0;i<rs.count;i++) {
			if (rs.szSettings[i] != NULL) {
				if (!DBDeleteContactSetting(hContact, szModule, rs.szSettings[i])) {
					count += 1;
				}
				free(rs.szSettings[i]);
			}
		}
		free(rs.szSettings);
	}

	return count;
}
