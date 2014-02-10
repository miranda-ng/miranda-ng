/*
FTP File YM plugin
Copyright (C) 2007-2010 Jan Holub

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

#include "common.h"

int DB::setByteF(HCONTACT hContact, char *szModule, char *szSetting, int id, int iValue)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return db_set_b(hContact, szModule, formSet, iValue);
}

int DB::setWordF(HCONTACT hContact, char *szModule, char *szSetting, int id, int iValue)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return db_set_w(hContact, szModule, formSet, iValue);
}

int DB::setDwordF(HCONTACT hContact, char *szModule, char *szSetting, int id, int iValue)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return db_set_dw(hContact, szModule, formSet, iValue);
}

int DB::setAStringF(HCONTACT hContact, char *szModule, char *szSetting, int id, char *szValue)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return db_set_s(hContact, szModule, formSet, szValue);
}

int DB::setStringF(HCONTACT hContact, char *szModule, char *szSetting, int id, TCHAR *stzValue)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return db_set_ts(hContact, szModule, formSet, stzValue);
}

int DB::getByteF(HCONTACT hContact, char *szModule, char *szSetting, int id, int iErrorValue)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return db_get_b(hContact, szModule, formSet, iErrorValue);
}

int DB::getWordF(HCONTACT hContact, char *szModule, char *szSetting, int id, int iErrorValue)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return db_get_w(hContact, szModule, formSet, iErrorValue);
}

int DB::getDwordF(HCONTACT hContact, char *szModule, char *szSetting, int id, int iErrorValue)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return db_get_dw(hContact, szModule, formSet, iErrorValue);
}

int DB::getAString(HCONTACT hContact, char *szModule, char *szSetting, char *buff)
{
	DBVARIANT dbv;
	if (!db_get_s(hContact, szModule, szSetting, &dbv))
	{
		strcpy(buff, dbv.pszVal);
		db_free(&dbv);
		return 0;
	}

	buff[0] = 0;
	return 1;
}

int DB::getAStringF(HCONTACT hContact, char *szModule, char *szSetting, int id, char *buff)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return getAString(hContact, szModule, formSet, buff);
}

int DB::getString(HCONTACT hContact, char *szModule, char *szSetting, TCHAR *buff)
{
	DBVARIANT dbv;
	if (!db_get_ts(hContact, szModule, szSetting, &dbv))
	{
		_tcscpy(buff, dbv.ptszVal);
		db_free(&dbv);
		return 0;
	}

	buff[0] = 0;
	return 1;
}

int DB::getStringF(HCONTACT hContact, char *szModule, char *szSetting, int id, TCHAR *buff)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return getString(hContact, szModule, formSet, buff);
}

int DB::deleteSettingF(HCONTACT hContact, char *szModule, char *szSetting, int id)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return db_unset(hContact, szModule, formSet);
}
