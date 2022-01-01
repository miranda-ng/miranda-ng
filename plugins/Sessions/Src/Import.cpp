/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

bool LoadContactsFromMask(MCONTACT hContact, int mode, int count)
{
	CMStringA szValue;
	if (mode == 0)
		szValue = g_plugin.getMStringA(hContact, "LastSessionsMarks");
	else if (mode == 1)
		szValue = g_plugin.getMStringA(hContact, "UserSessionsMarks");

	if (szValue.IsEmpty())
		return false;

	return szValue[count] == '1';
}

int GetInSessionOrder(MCONTACT hContact, int mode, int count)
{
	char szTemp[3] = { 0, 0, 0 };
	count *= 2;

	if (mode == 0) {
		CMStringA szValue(g_plugin.getMStringA(hContact, "LastSessionsOrder"));
		if (!szValue.IsEmpty() && count < szValue.GetLength()) {
			memcpy(szTemp, szValue.c_str() + count, 2);
			return atoi(szTemp);
		}
	}
	else if (mode == 1) {
		CMStringA szValue(g_plugin.getMStringA(hContact, "UserSessionsOrder"));
		if (!szValue.IsEmpty() && count < szValue.GetLength()) {
			memcpy(szTemp, szValue.c_str() + count, 2);
			return atoi(szTemp);
		}
	}
	return 0;
}

void CMPlugin::CheckImport()
{
	if (db_get_b(0, "Compatibility", MODULENAME) > 0)
		return;

	MCONTACT tmp[255];

	for (int i = 0;; i++) {
		char szSetting[100];
		mir_snprintf(szSetting, "SessionDate_%d", i);
		CMStringW wszName(getMStringW(szSetting));
		if (wszName.IsEmpty())
			break;

		delSetting(szSetting);

		memset(tmp, 0, sizeof(tmp));
		for (auto &hContact : Contacts()) {
			if (LoadContactsFromMask(hContact, 0, i)) {
				int idx = GetInSessionOrder(hContact, 0, i);
				if (idx < _countof(tmp))
					tmp[idx] = hContact;
			}
		}
		if (tmp[0] == 0)
			continue;

		CSession *pNew = new CSession();
		pNew->id = 255 - i;
		pNew->wszName = wszName;
		for (int k = 0; tmp[k]; k++)
			pNew->contacts.push_back(tmp[k]);

		pNew->save();
	}

	for (int i = 0;; i++) {
		char szSetting[100];
		mir_snprintf(szSetting, "UserSessionDsc_%d", i);
		CMStringW wszName(getMStringW(szSetting));
		if (wszName.IsEmpty())
			break;

		delSetting(szSetting);

		memset(tmp, 0, sizeof(tmp));
		for (auto &hContact : Contacts()) {
			if (LoadContactsFromMask(hContact, 1, i)) {
				int idx = GetInSessionOrder(hContact, 1, i);
				if (idx < _countof(tmp))
					tmp[idx] = hContact;
			}
		}
		if (tmp[0] == 0)
			continue;

		CSession *pNew = new CSession();
		pNew->id = 255 - i;
		pNew->bIsUser = true;
		pNew->wszName = wszName;

		mir_snprintf(szSetting, "FavUserSession_%d", i);
		pNew->bIsFavorite = getBool(szSetting);
		delSetting(szSetting);

		for (int k = 0; tmp[k]; k++)
			pNew->contacts.push_back(tmp[k]);

		pNew->save();
	}

	delSetting("UserSessionsCount");

	for (auto &hContact : Contacts())
		db_delete_module(hContact, MODULENAME);

	g_lastDateId = g_lastUserId = 256;
	db_set_b(0, "Compatibility", MODULENAME, 1);
}
