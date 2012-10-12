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

int DB::setByte(HANDLE hContact, char *szModule, char *szSetting, int iValue)
{
	return DBWriteContactSettingByte(hContact, szModule, szSetting, iValue);
}

int DB::setByteF(HANDLE hContact, char *szModule, char *szSetting, int id, int iValue)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return setByte(hContact, szModule, formSet, iValue);
}

int DB::setWord(HANDLE hContact, char *szModule, char *szSetting, int iValue)
{
	return DBWriteContactSettingWord(hContact, szModule, szSetting, iValue);
}

int DB::setWordF(HANDLE hContact, char *szModule, char *szSetting, int id, int iValue)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return setWord(hContact, szModule, formSet, iValue);
}

int DB::setDword(HANDLE hContact, char *szModule, char *szSetting, int iValue)
{
	return DBWriteContactSettingDword(hContact, szModule, szSetting, iValue);
}

int DB::setDwordF(HANDLE hContact, char *szModule, char *szSetting, int id, int iValue)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return setDword(hContact, szModule, formSet, iValue);
}

int DB::setAString(HANDLE hContact, char *szModule, char *szSetting, char *szValue)
{
	return DBWriteContactSettingString(hContact, szModule, szSetting, szValue);
}

int DB::setAStringF(HANDLE hContact, char *szModule, char *szSetting, int id, char *szValue)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return setAString(hContact, szModule, formSet, szValue);
}

int DB::setString(HANDLE hContact, char *szModule, char *szSetting, TCHAR *stzValue)
{
	return DBWriteContactSettingTString(hContact, szModule, szSetting, stzValue);
}

int DB::setStringF(HANDLE hContact, char *szModule, char *szSetting, int id, TCHAR *stzValue)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return setString(hContact, szModule, formSet, stzValue);
}

int DB::setCryptedString(HANDLE hContact, char *szModule, char *szSetting, char *szValue)
{
	char buff[256];
	strcpy(buff, szValue);
	CallService(MS_DB_CRYPT_ENCODESTRING, (WPARAM)sizeof(buff), (LPARAM)buff);
	return setAString(hContact, szModule, szSetting, buff);
}

int DB::getByte(HANDLE hContact, char *szModule, char *szSetting, int iErrorValue)
{
	return DBGetContactSettingByte(hContact, szModule, szSetting, iErrorValue);
}

int DB::getByteF(HANDLE hContact, char *szModule, char *szSetting, int id, int iErrorValue)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return getByte(hContact, szModule, formSet, iErrorValue);
}

int DB::getWord(HANDLE hContact, char *szModule, char *szSetting, int iErrorValue)
{
	return DBGetContactSettingWord(hContact, szModule, szSetting, iErrorValue);
}

int DB::getWordF(HANDLE hContact, char *szModule, char *szSetting, int id, int iErrorValue)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return getWord(hContact, szModule, formSet, iErrorValue);
}

int DB::getDword(HANDLE hContact, char *szModule, char *szSetting, int iErrorValue)
{
	return DBGetContactSettingDword(hContact, szModule, szSetting, iErrorValue);
}

int DB::getDwordF(HANDLE hContact, char *szModule, char *szSetting, int id, int iErrorValue)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return getDword(hContact, szModule, formSet, iErrorValue);
}

int DB::getAString(HANDLE hContact, char *szModule, char *szSetting, char *buff)
{
	DBVARIANT dbv;
	if (!DBGetContactSettingString(hContact, szModule, szSetting, &dbv))
	{
		strcpy(buff, dbv.pszVal);
		DBFreeVariant(&dbv);
		return 0;
	}

	buff[0] = 0;
	return 1;
}

int DB::getAStringF(HANDLE hContact, char *szModule, char *szSetting, int id, char *buff)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return getAString(hContact, szModule, formSet, buff);
}

int DB::getString(HANDLE hContact, char *szModule, char *szSetting, TCHAR *buff)
{
	DBVARIANT dbv;
	if (!DBGetContactSettingTString(hContact, szModule, szSetting, &dbv))
	{
		_tcscpy(buff, dbv.ptszVal);
		DBFreeVariant(&dbv);
		return 0;
	}

	buff[0] = 0;
	return 1;
}

int DB::getStringF(HANDLE hContact, char *szModule, char *szSetting, int id, TCHAR *buff)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return getString(hContact, szModule, formSet, buff);
}

int DB::getCryptedString(HANDLE hContact, char *szModule, char *szSetting, char *szValue)
{
	char buff[256];
	if (!getAString(hContact, szModule, szSetting, buff))
	{
		CallService(MS_DB_CRYPT_DECODESTRING, (WPARAM)sizeof(buff), (LPARAM)buff);
		strcpy(szValue, buff);
		return 0;
	}

	szValue[0] = 0;
	return 1;
}

int DB::deleteSetting(HANDLE hContact, char *szModule, char *szSetting)
{
	return DBDeleteContactSetting(hContact, szModule, szSetting);
}

int DB::deleteSettingF(HANDLE hContact, char *szModule, char *szSetting, int id)
{
	char formSet[256];
	mir_snprintf(formSet, sizeof(formSet), szSetting, id);
	return deleteSetting(hContact, szModule, formSet);
}

char *DB::getProto(HANDLE hContact)
{
	char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	return ((INT_PTR)szProto != CALLSERVICE_NOTFOUND) ? szProto : NULL;
}