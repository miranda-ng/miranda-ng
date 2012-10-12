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

#pragma once

#include "common.h"

class DB
{
public:
	static int setByte(HANDLE hContact, char *szModule, char *szSetting, int iValue);
	static int setByteF(HANDLE hContact, char *szModule, char *szSetting, int id, int iValue);
	static int setWord(HANDLE hContact, char *szModule, char *szSetting, int iValue);
	static int setWordF(HANDLE hContact, char *szModule, char *szSetting, int id, int iValue);
	static int setDword(HANDLE hContact, char *szModule, char *szSetting, int iValue);
	static int setDwordF(HANDLE hContact, char *szModule, char *szSetting, int id, int iValue);

	static int setAString(HANDLE hContact, char *szModule, char *szSetting, char *szValue);
	static int setAStringF(HANDLE hContact, char *szModule, char *szSetting, int id, char *szValue);
	static int setString(HANDLE hContact, char *szModule, char *szSetting, TCHAR *stzValue);
	static int setStringF(HANDLE hContact, char *szModule, char *szSetting, int id, TCHAR *stzValue);

	static int getByte(HANDLE hContact, char *szModule, char *szSetting, int iErrorValue);
	static int getByteF(HANDLE hContact, char *szModule, char *szSetting, int id, int iErrorValue = -1);
	static int getWord(HANDLE hContact, char *szModule, char *szSetting, int iErrorValue = -1);
	static int getWordF(HANDLE hContact, char *szModule, char *szSetting, int id, int iErrorValue = -1);
	static int getDword(HANDLE hContact, char *szModule, char *szSetting, int iErrorValue = -1);
	static int getDwordF(HANDLE hContact, char *szModule, char *szSetting, int id, int iErrorValue = -1);

	static int getAString(HANDLE hContact, char *szModule, char *szSetting, char *buff);
	static int getAStringF(HANDLE hContact, char *szModule, char *szSetting, int id, char *buff);
	static int getString(HANDLE hContact, char *szModule, char *szSetting, TCHAR *buff);
	static int getStringF(HANDLE hContact, char *szModule, char *szSetting, int id, TCHAR *buff);

	static int setCryptedString(HANDLE hContact, char *szModule, char *szSetting, char *szValue);
	static int getCryptedString(HANDLE hContact, char *szModule, char *szSetting, char *szValue);

	static int deleteSetting(HANDLE hContact, char *szModule, char *szSetting);
	static int deleteSettingF(HANDLE hContact, char *szModule, char *szSetting, int id);

	static char *getProto(HANDLE hContact);
};