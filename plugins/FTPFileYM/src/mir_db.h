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

#include "stdafx.h"

namespace DB
{
	int setByteF(MCONTACT hContact, char *szModule, char *szSetting, int id, int iValue);
	int setWordF(MCONTACT hContact, char *szModule, char *szSetting, int id, int iValue);
	int setDwordF(MCONTACT hContact, char *szModule, char *szSetting, int id, int iValue);

	int setAStringF(MCONTACT hContact, char *szModule, char *szSetting, int id, char *szValue);
	int setStringF(MCONTACT hContact, char *szModule, char *szSetting, int id, wchar_t *stzValue);

	int getByteF(MCONTACT hContact, char *szModule, char *szSetting, int id, int iErrorValue = -1);
	int getWordF(MCONTACT hContact, char *szModule, char *szSetting, int id, int iErrorValue = -1);
	int getDwordF(MCONTACT hContact, char *szModule, char *szSetting, int id, int iErrorValue = -1);

	int deleteSettingF(MCONTACT hContact, char *szModule, char *szSetting, int id);
};
