/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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

#include "commonheaders.h"

MIR_CORE_DLL(int) db_get_b(HANDLE hContact, const char *szModule, const char *szSetting, int errorValue)
{
	DBVARIANT dbv;
	DBCONTACTGETSETTING cgs;
	cgs.szModule  =  szModule;
	cgs.szSetting  =  szSetting;
	cgs.pValue  =  &dbv;
	if (CallService(MS_DB_CONTACT_GETSETTING, (WPARAM)hContact, (LPARAM)&cgs))
		return errorValue;
	return dbv.bVal;
}

MIR_CORE_DLL(int) db_get_w(HANDLE hContact, const char *szModule, const char *szSetting, int errorValue)
{
	DBVARIANT dbv;
	DBCONTACTGETSETTING cgs;
	cgs.szModule = szModule;
	cgs.szSetting = szSetting;
	cgs.pValue = &dbv;
	if (CallService(MS_DB_CONTACT_GETSETTING, (WPARAM)hContact, (LPARAM)&cgs))
		return errorValue;
	return dbv.wVal;
}

MIR_CORE_DLL(DWORD) db_get_dw(HANDLE hContact, const char *szModule, const char *szSetting, DWORD errorValue)
{
	DBVARIANT dbv;
	DBCONTACTGETSETTING cgs;
	cgs.szModule = szModule;
	cgs.szSetting = szSetting;
	cgs.pValue = &dbv;
	if (CallService(MS_DB_CONTACT_GETSETTING, (WPARAM)hContact, (LPARAM)&cgs))
		return errorValue;
	return dbv.dVal;
}

MIR_CORE_DLL(INT_PTR) db_get(HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	DBCONTACTGETSETTING cgs;
	cgs.szModule = szModule;
	cgs.szSetting = szSetting;
	cgs.pValue = dbv;

	return CallService(MS_DB_CONTACT_GETSETTING, (WPARAM)hContact, (LPARAM)&cgs);
}

MIR_CORE_DLL(INT_PTR) db_get_s(HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv, const int nType)
{
	DBCONTACTGETSETTING cgs;
	cgs.szModule = szModule;
	cgs.szSetting = szSetting;
	cgs.pValue = dbv;
	dbv->type = (BYTE)nType;
	return CallService(MS_DB_CONTACT_GETSETTING_STR, (WPARAM)hContact, (LPARAM)&cgs);
}

MIR_CORE_DLL(char*) db_get_sa(HANDLE hContact, const char *szModule, const char *szSetting)
{
	char *str = NULL;
	DBVARIANT dbv = {0};
	db_get_s(hContact, szModule, szSetting, &dbv, DBVT_ASCIIZ);
	if (dbv.type == DBVT_ASCIIZ)
		str = mir_strdup(dbv.pszVal);
	DBFreeVariant(&dbv);
	return str;
}

MIR_CORE_DLL(wchar_t*) db_get_wsa(HANDLE hContact, const char *szModule, const char *szSetting)
{
	wchar_t *str = NULL;
	DBVARIANT dbv={0};
	db_get_s(hContact, szModule, szSetting, &dbv, DBVT_WCHAR);
	if (dbv.type == DBVT_WCHAR)
		str = mir_wstrdup(dbv.pwszVal);
	DBFreeVariant(&dbv);
	return str;
}

MIR_CORE_DLL(INT_PTR) db_free(DBVARIANT *dbv)
{
	return CallService(MS_DB_CONTACT_FREEVARIANT, 0, (LPARAM)dbv);
}

MIR_CORE_DLL(INT_PTR) db_unset(HANDLE hContact, const char *szModule, const char *szSetting)
{
	DBCONTACTGETSETTING cgs;
	cgs.szModule = szModule;
	cgs.szSetting = szSetting;
	return CallService(MS_DB_CONTACT_DELETESETTING, (WPARAM)hContact, (LPARAM)&cgs);
}

MIR_CORE_DLL(INT_PTR) db_set_b(HANDLE hContact, const char *szModule, const char *szSetting, BYTE val)
{
	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_BYTE;
	cws.value.bVal = val;
	return CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws);
}

MIR_CORE_DLL(INT_PTR) db_set_w(HANDLE hContact, const char *szModule, const char *szSetting, WORD val)
{
	DBCONTACTWRITESETTING cws;

	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_WORD;
	cws.value.wVal = val;
	return CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws);
}

MIR_CORE_DLL(INT_PTR) db_set_dw(HANDLE hContact, const char *szModule, const char *szSetting, DWORD val)
{
	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_DWORD;
	cws.value.dVal = val;
	return CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws);
}

MIR_CORE_DLL(INT_PTR) db_set_s(HANDLE hContact, const char *szModule, const char *szSetting, const char *val)
{
	DBCONTACTWRITESETTING cws;

	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_ASCIIZ;
	cws.value.pszVal = (char*)val;
	return CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws);
}

MIR_CORE_DLL(INT_PTR) db_set_ws(HANDLE hContact, const char *szModule, const char *szSetting, const WCHAR *val)
{
	DBCONTACTWRITESETTING cws;

	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_WCHAR;
	cws.value.pwszVal = (WCHAR*)val;
	return CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws);
}

MIR_CORE_DLL(INT_PTR) db_set_utf(HANDLE hContact, const char *szModule, const char *szSetting, const char *val)
{
	DBCONTACTWRITESETTING cws;

	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_UTF8;
	cws.value.pszVal = (char*)val;
	return CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws);
}

MIR_CORE_DLL(INT_PTR) db_set_blob(HANDLE hContact, const char *szModule, const char *szSetting, void *val, unsigned len)
{
	DBCONTACTWRITESETTING cws;

	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_BLOB;
    cws.value.cpbVal  =  (WORD)len;
	cws.value.pbVal = (unsigned char*)val;
	return CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws);
}
