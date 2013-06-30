/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-12 Miranda IM, 2012-13 Miranda NG project,
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

static MIDatabase* currDb = NULL;

/////////////////////////////////////////////////////////////////////////////////////////
// getting data

MIR_CORE_DLL(int) db_get_b(HANDLE hContact, const char *szModule, const char *szSetting, int errorValue)
{
	if (currDb != NULL) {
		DBVARIANT dbv;
		DBCONTACTGETSETTING cgs;
		cgs.szModule = szModule;
		cgs.szSetting = szSetting;
		cgs.pValue = &dbv;
		if ( !currDb->GetContactSetting(hContact, &cgs))
			switch(dbv.type) {
				case DBVT_BYTE:	return dbv.bVal;
				case DBVT_WORD:   return BYTE(dbv.wVal);
				case DBVT_DWORD:	return BYTE(dbv.dVal);
			}
	}
	return errorValue;
}

MIR_CORE_DLL(int) db_get_w(HANDLE hContact, const char *szModule, const char *szSetting, int errorValue)
{
	if (currDb != NULL) {
		DBVARIANT dbv;
		DBCONTACTGETSETTING cgs;
		cgs.szModule = szModule;
		cgs.szSetting = szSetting;
		cgs.pValue = &dbv;
		if ( !currDb->GetContactSetting(hContact, &cgs))
			switch(dbv.type) {
				case DBVT_BYTE:	return dbv.bVal;
				case DBVT_WORD:   return dbv.wVal;
				case DBVT_DWORD:	return WORD(dbv.dVal);
			}
	}
	return errorValue;
}

MIR_CORE_DLL(DWORD) db_get_dw(HANDLE hContact, const char *szModule, const char *szSetting, DWORD errorValue)
{
	if (currDb != NULL) {
		DBVARIANT dbv;
		DBCONTACTGETSETTING cgs;
		cgs.szModule = szModule;
		cgs.szSetting = szSetting;
		cgs.pValue = &dbv;
		if ( !currDb->GetContactSetting(hContact, &cgs))
			switch(dbv.type) {
				case DBVT_BYTE:	return dbv.bVal;
				case DBVT_WORD:   return dbv.wVal;
				case DBVT_DWORD:	return dbv.dVal;
			}
	}

	return errorValue;
}

MIR_CORE_DLL(INT_PTR) db_get(HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	if (currDb == NULL) return 1;

	DBCONTACTGETSETTING cgs;
	cgs.szModule = szModule;
	cgs.szSetting = szSetting;
	cgs.pValue = dbv;
	return currDb->GetContactSetting(hContact, &cgs);
}

MIR_CORE_DLL(INT_PTR) db_get_s(HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv, const int nType)
{
	if (currDb == NULL) return 1;

	DBCONTACTGETSETTING cgs;
	cgs.szModule = szModule;
	cgs.szSetting = szSetting;
	cgs.pValue = dbv;
	dbv->type = (BYTE)nType;
	return currDb->GetContactSettingStr(hContact, &cgs);
}

MIR_CORE_DLL(char*) db_get_sa(HANDLE hContact, const char *szModule, const char *szSetting)
{
	char *str = NULL;
	DBVARIANT dbv = {0};
	db_get_s(hContact, szModule, szSetting, &dbv, DBVT_ASCIIZ);
	if (dbv.type == DBVT_ASCIIZ)
		str = mir_strdup(dbv.pszVal);
	db_free(&dbv);
	return str;
}

MIR_CORE_DLL(wchar_t*) db_get_wsa(HANDLE hContact, const char *szModule, const char *szSetting)
{
	wchar_t *str = NULL;
	DBVARIANT dbv={0};
	db_get_s(hContact, szModule, szSetting, &dbv, DBVT_WCHAR);
	if (dbv.type == DBVT_WCHAR)
		str = mir_wstrdup(dbv.pwszVal);
	db_free(&dbv);
	return str;
}

/////////////////////////////////////////////////////////////////////////////////////////
// setting data

MIR_CORE_DLL(INT_PTR) db_set(HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	if (currDb == NULL) return 1;

	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value = *dbv;
	return currDb->WriteContactSetting(hContact, &cws);
}

MIR_CORE_DLL(INT_PTR) db_set_b(HANDLE hContact, const char *szModule, const char *szSetting, BYTE val)
{
	if (currDb == NULL) return 1;

	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_BYTE;
	cws.value.bVal = val;
	return currDb->WriteContactSetting(hContact, &cws);
}

MIR_CORE_DLL(INT_PTR) db_set_w(HANDLE hContact, const char *szModule, const char *szSetting, WORD val)
{
	if (currDb == NULL) return 1;

	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_WORD;
	cws.value.wVal = val;
	return currDb->WriteContactSetting(hContact, &cws);
}

MIR_CORE_DLL(INT_PTR) db_set_dw(HANDLE hContact, const char *szModule, const char *szSetting, DWORD val)
{
	if (currDb == NULL) return 1;

	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_DWORD;
	cws.value.dVal = val;
	return currDb->WriteContactSetting(hContact, &cws);
}

MIR_CORE_DLL(INT_PTR) db_set_s(HANDLE hContact, const char *szModule, const char *szSetting, const char *val)
{
	if (currDb == NULL) return 1;

	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_ASCIIZ;
	cws.value.pszVal = (char*)val;
	return currDb->WriteContactSetting(hContact, &cws);
}

MIR_CORE_DLL(INT_PTR) db_set_ws(HANDLE hContact, const char *szModule, const char *szSetting, const WCHAR *val)
{
	if (currDb == NULL) return 1;

	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_WCHAR;
	cws.value.pwszVal = (WCHAR*)val;
	return currDb->WriteContactSetting(hContact, &cws);
}

MIR_CORE_DLL(INT_PTR) db_set_utf(HANDLE hContact, const char *szModule, const char *szSetting, const char *val)
{
	if (currDb == NULL) return 1;

	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_UTF8;
	cws.value.pszVal = (char*)val;
	return currDb->WriteContactSetting(hContact, &cws);
}

MIR_CORE_DLL(INT_PTR) db_set_blob(HANDLE hContact, const char *szModule, const char *szSetting, void *val, unsigned len)
{
	if (currDb == NULL) return 1;

	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_BLOB;
	cws.value.cpbVal = (WORD)len;
	cws.value.pbVal = (unsigned char*)val;
	return currDb->WriteContactSetting(hContact, &cws);
}

/////////////////////////////////////////////////////////////////////////////////////////
// events

MIR_CORE_DLL(HANDLE) db_event_add(HANDLE hContact, DBEVENTINFO *dbei)
{
	return (currDb == NULL) ? 0 : currDb->AddEvent(hContact, dbei);
}

MIR_CORE_DLL(int) db_event_count(HANDLE hContact)
{
	return (currDb == NULL) ? 0 : currDb->GetEventCount(hContact);
}

MIR_CORE_DLL(int) db_event_delete(HANDLE hContact, HANDLE hDbEvent)
{
	return (currDb == NULL) ? 0 : currDb->DeleteEvent(hContact, hDbEvent);
}

MIR_CORE_DLL(HANDLE) db_event_first(HANDLE hContact)
{
	return (currDb == NULL) ? 0 : currDb->FindFirstEvent(hContact);
}

MIR_CORE_DLL(HANDLE) db_event_firstUnread(HANDLE hContact)
{
	return (currDb == NULL) ? 0 : currDb->FindFirstUnreadEvent(hContact);
}

MIR_CORE_DLL(int) db_event_get(HANDLE hDbEvent, DBEVENTINFO *dbei)
{
	return (currDb == NULL) ? 1 : currDb->GetEvent(hDbEvent, dbei);
}

MIR_CORE_DLL(int) db_event_getBlobSize(HANDLE hDbEvent)
{
	return (currDb == NULL) ? 0 : currDb->GetBlobSize(hDbEvent);
}

MIR_CORE_DLL(HANDLE) db_event_getContact(HANDLE hDbEvent)
{
	return (currDb == NULL) ? 0 : currDb->GetEventContact(hDbEvent);
}

MIR_CORE_DLL(HANDLE) db_event_last(HANDLE hDbEvent)
{
	return (currDb == NULL) ? 0 : currDb->FindLastEvent(hDbEvent);
}

MIR_CORE_DLL(int) db_event_markRead(HANDLE hContact, HANDLE hDbEvent)
{
	return (currDb == NULL) ? 0 : currDb->MarkEventRead(hContact, hDbEvent);
}

MIR_CORE_DLL(HANDLE) db_event_next(HANDLE hDbEvent)
{
	return (currDb == NULL) ? 0 : currDb->FindNextEvent(hDbEvent);
}

MIR_CORE_DLL(HANDLE) db_event_prev(HANDLE hDbEvent)
{
	return (currDb == NULL) ? 0 : currDb->FindPrevEvent(hDbEvent);
}

/////////////////////////////////////////////////////////////////////////////////////////
// misc functions

MIR_CORE_DLL(INT_PTR) db_free(DBVARIANT *dbv)
{
	return (currDb == NULL) ? 1 : currDb->FreeVariant(dbv);
}

MIR_CORE_DLL(INT_PTR) db_unset(HANDLE hContact, const char *szModule, const char *szSetting)
{
	if (currDb == NULL) return 1;

	DBCONTACTGETSETTING cgs;
	cgs.szModule = szModule;
	cgs.szSetting = szSetting;
	return currDb->DeleteContactSetting(hContact, &cgs);
}

MIR_CORE_DLL(HANDLE) db_find_first(const char *szProto)
{
	return (currDb == NULL) ? NULL : currDb->FindFirstContact(szProto);
}

MIR_CORE_DLL(HANDLE) db_find_next(HANDLE hContact, const char *szProto)
{
	return (currDb == NULL) ? NULL : currDb->FindNextContact(hContact, szProto);
}

extern "C" MIR_CORE_DLL(void) db_setCurrent(MIDatabase* _db)
{
	currDb = _db;
}

MIR_CORE_DLL(BOOL) db_set_resident(const char *szModule, const char *szService, BOOL bEnable)
{
	if (currDb == NULL || szModule == NULL || szService == NULL)
		return FALSE;

	char str[MAXMODULELABELLENGTH * 2];
	mir_snprintf(str, SIZEOF(str), "%s/%s", szModule, szService);
	return currDb->SetSettingResident(bEnable, str);
}
