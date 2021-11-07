/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

MIR_CORE_EXPORT MDatabaseCommon* g_pCurrDb = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////
// database functions

MIR_CORE_DLL(void) db_set_safety_mode(BOOL bNewMode)
{
	if (g_pCurrDb)
		g_pCurrDb->SetCacheSafetyMode(bNewMode != 0);
}

MIR_CORE_DLL(int) db_get_contact_count(void)
{
	return (g_pCurrDb) ? g_pCurrDb->GetContactCount() : 0;
}

MIR_CORE_DLL(MDatabaseCommon*) db_get_current()
{
	return g_pCurrDb;
}

MIR_CORE_DLL(int) db_delete_module(MCONTACT hContact, const char *szModuleName)
{
	return (g_pCurrDb) ? g_pCurrDb->DeleteModule(hContact, szModuleName) : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// contact functions

MIR_CORE_DLL(MCONTACT) db_add_contact(void)
{
	MCONTACT hNew = (g_pCurrDb) ? g_pCurrDb->AddContact() : 0;
	Netlib_Logf(nullptr, "New contact created: %d", hNew);
	return hNew;
}

MIR_CORE_DLL(int) db_delete_contact(MCONTACT hContact)
{
	ptrW wszPhoto(db_get_wsa(hContact, "ContactPhoto", "File"));
	if (wszPhoto != nullptr) {
      #ifdef _MSC_VER
         DeleteFileW(wszPhoto);
      #else
         remove(T2Utf(wszPhoto));
      #endif
   }

	Netlib_Logf(nullptr, "Contact deleted: %d", hContact);
	return (g_pCurrDb) ? g_pCurrDb->DeleteContact(hContact) : 0;
}

MIR_CORE_DLL(int) db_is_contact(MCONTACT hContact)
{
	return (g_pCurrDb) ? g_pCurrDb->IsDbContact(hContact) : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// enumerators

MIR_CORE_DLL(int) db_enum_modules(DBMODULEENUMPROC pFunc, void *param)
{
	return (g_pCurrDb) ? g_pCurrDb->EnumModuleNames(pFunc, param) : 0;
}

MIR_CORE_DLL(int) db_enum_residents(DBMODULEENUMPROC pFunc, void *param)
{
	return (g_pCurrDb) ? g_pCurrDb->EnumResidentSettings(pFunc, param) : 0;
}

EXTERN_C MIR_CORE_DLL(int) db_enum_settings(MCONTACT hContact, DBSETTINGENUMPROC pFunc, const char *szModule, void *param)
{
	return (g_pCurrDb) ? g_pCurrDb->EnumContactSettings(hContact, pFunc, szModule, param) : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// getting data

MIR_CORE_DLL(int) db_get_b(MCONTACT hContact, const char *szModule, const char *szSetting, int errorValue)
{
	if (g_pCurrDb != nullptr) {
		DBVARIANT dbv;
		if (!g_pCurrDb->GetContactSetting(hContact, szModule, szSetting, &dbv))
		{
			switch(dbv.type) {
				case DBVT_BYTE:	return dbv.bVal;
				case DBVT_WORD:   return BYTE(dbv.wVal);
				case DBVT_DWORD:	return BYTE(dbv.dVal);
			}
			g_pCurrDb->FreeVariant(&dbv);
		}
	}
	return errorValue;
}

MIR_CORE_DLL(int) db_get_w(MCONTACT hContact, const char *szModule, const char *szSetting, int errorValue)
{
	if (g_pCurrDb != nullptr) {
		DBVARIANT dbv;
		if (!g_pCurrDb->GetContactSetting(hContact, szModule, szSetting, &dbv)) {
			switch(dbv.type) {
				case DBVT_BYTE:	return dbv.bVal;
				case DBVT_WORD:   return dbv.wVal;
				case DBVT_DWORD:	return WORD(dbv.dVal);
			}
			g_pCurrDb->FreeVariant(&dbv);
		}
	}
	return errorValue;
}

MIR_CORE_DLL(DWORD) db_get_dw(MCONTACT hContact, const char *szModule, const char *szSetting, DWORD errorValue)
{
	if (g_pCurrDb != nullptr) {
		DBVARIANT dbv;
		if (!g_pCurrDb->GetContactSetting(hContact, szModule, szSetting, &dbv)) {
			switch(dbv.type) {
				case DBVT_BYTE:	return dbv.bVal;
				case DBVT_WORD:   return dbv.wVal;
				case DBVT_DWORD:	return dbv.dVal;
			}
			g_pCurrDb->FreeVariant(&dbv);
		}
	}

	return errorValue;
}

MIR_CORE_DLL(INT_PTR) db_get(MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	if (g_pCurrDb == nullptr)
		return 1;

	return g_pCurrDb->GetContactSetting(hContact, szModule, szSetting, dbv);
}

MIR_CORE_DLL(INT_PTR) db_get_s(MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv, const int nType)
{
	if (g_pCurrDb == nullptr)
		return 1;

	dbv->type = (BYTE)nType;
	return g_pCurrDb->GetContactSettingStr(hContact, szModule, szSetting, dbv);
}

MIR_CORE_DLL(char*) db_get_sa(MCONTACT hContact, const char *szModule, const char *szSetting, const char *szValue)
{
	if (g_pCurrDb) {
		DBVARIANT dbv = { DBVT_ASCIIZ };
		if (!g_pCurrDb->GetContactSettingStr(hContact, szModule, szSetting, &dbv))
			return dbv.pszVal;
	}

	return (szValue == nullptr) ? nullptr : mir_strdup(szValue);
}

MIR_CORE_DLL(char*) db_get_utfa(MCONTACT hContact, const char *szModule, const char *szSetting, const char *szValue)
{
	if (g_pCurrDb) {
		DBVARIANT dbv = { DBVT_UTF8 };
		if (!g_pCurrDb->GetContactSettingStr(hContact, szModule, szSetting, &dbv))
			return dbv.pszVal;
	}

	return (szValue == nullptr) ? nullptr : mir_strdup(szValue);
}

MIR_CORE_DLL(wchar_t*) db_get_wsa(MCONTACT hContact, const char *szModule, const char *szSetting, const wchar_t *szValue)
{
	if (g_pCurrDb) {
		DBVARIANT dbv = { DBVT_WCHAR };
		if (!g_pCurrDb->GetContactSettingStr(hContact, szModule, szSetting, &dbv))
			return dbv.pwszVal;
	}

	return (szValue == nullptr) ? nullptr : mir_wstrdup(szValue);
}

MIR_CORE_DLL(CMStringA) db_get_sm(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, const char *szValue)
{
	if (g_pCurrDb == nullptr)
		return (szValue == nullptr) ? CMStringA() : CMStringA(szValue);

	DBVARIANT dbv = { DBVT_ASCIIZ };
	if (g_pCurrDb->GetContactSettingStr(hContact, szModule, szSetting, &dbv))
		return (szValue == nullptr) ? CMStringA() : CMStringA(szValue);

	return CMStringA(ptrA(dbv.pszVal).get());
}

MIR_CORE_DLL(CMStringW) db_get_wsm(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, const wchar_t *szValue)
{
	if (g_pCurrDb == nullptr)
		return (szValue == nullptr) ? CMStringW() : CMStringW(szValue);

	DBVARIANT dbv = { DBVT_WCHAR };
	if (g_pCurrDb->GetContactSettingStr(hContact, szModule, szSetting, &dbv))
		return (szValue == nullptr) ? CMStringW() : CMStringW(szValue);

	return CMStringW(ptrW(dbv.pwszVal).get());
}

/////////////////////////////////////////////////////////////////////////////////////////
// getting static data

MIR_CORE_DLL(int) db_get_static(MCONTACT hContact, const char *szModule, const char *szSetting, char *pDest, int cbDest)
{
	if (g_pCurrDb == nullptr)
		return 1;

	DBVARIANT dbv;
	dbv.type = DBVT_ASCIIZ;
	dbv.pszVal = pDest;
	dbv.cchVal = cbDest;
	return g_pCurrDb->GetContactSettingStatic(hContact, szModule, szSetting, &dbv);
}

MIR_CORE_DLL(int) db_get_static_utf(MCONTACT hContact, const char *szModule, const char *szSetting, char *pDest, int cbDest)
{
	if (g_pCurrDb == nullptr)
		return 1;

	DBVARIANT dbv;
	dbv.type = DBVT_UTF8;
	dbv.pszVal = pDest;
	dbv.cchVal = cbDest;
	return g_pCurrDb->GetContactSettingStatic(hContact, szModule, szSetting, &dbv);
}

MIR_CORE_DLL(int) db_get_wstatic(MCONTACT hContact, const char *szModule, const char *szSetting, wchar_t *pDest, int cbDest)
{
	if (g_pCurrDb == nullptr)
		return 1;

	DBVARIANT dbv;
	dbv.type = DBVT_WCHAR;
	dbv.pwszVal = pDest;
	dbv.cchVal = cbDest;
	return g_pCurrDb->GetContactSettingStatic(hContact, szModule, szSetting, &dbv);
}

/////////////////////////////////////////////////////////////////////////////////////////
// setting data

MIR_CORE_DLL(INT_PTR) db_set(MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	if (g_pCurrDb == nullptr) return 1;

	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value = *dbv;
	return g_pCurrDb->WriteContactSetting(hContact, &cws);
}

MIR_CORE_DLL(INT_PTR) db_set_b(MCONTACT hContact, const char *szModule, const char *szSetting, BYTE val)
{
	if (g_pCurrDb == nullptr) return 1;

	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_BYTE;
	cws.value.bVal = val;
	return g_pCurrDb->WriteContactSetting(hContact, &cws);
}

MIR_CORE_DLL(INT_PTR) db_set_w(MCONTACT hContact, const char *szModule, const char *szSetting, WORD val)
{
	if (g_pCurrDb == nullptr) return 1;

	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_WORD;
	cws.value.wVal = val;
	return g_pCurrDb->WriteContactSetting(hContact, &cws);
}

MIR_CORE_DLL(INT_PTR) db_set_dw(MCONTACT hContact, const char *szModule, const char *szSetting, DWORD val)
{
	if (g_pCurrDb == nullptr) return 1;

	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_DWORD;
	cws.value.dVal = val;
	return g_pCurrDb->WriteContactSetting(hContact, &cws);
}

MIR_CORE_DLL(INT_PTR) db_set_s(MCONTACT hContact, const char *szModule, const char *szSetting, const char *val)
{
	if (g_pCurrDb == nullptr) return 1;

	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_ASCIIZ;
	cws.value.pszVal = (char*)(val == nullptr ? "" : val);
	return g_pCurrDb->WriteContactSetting(hContact, &cws);
}

MIR_CORE_DLL(INT_PTR) db_set_ws(MCONTACT hContact, const char *szModule, const char *szSetting, const wchar_t *val)
{
	if (g_pCurrDb == nullptr) return 1;

	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_WCHAR;
	cws.value.pwszVal = (wchar_t*)(val == nullptr ? L"" : val);
	return g_pCurrDb->WriteContactSetting(hContact, &cws);
}

MIR_CORE_DLL(INT_PTR) db_set_utf(MCONTACT hContact, const char *szModule, const char *szSetting, const char *val)
{
	if (g_pCurrDb == nullptr) return 1;

	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_UTF8;
	cws.value.pszVal = (char*)(val == nullptr ? "" : val);
	return g_pCurrDb->WriteContactSetting(hContact, &cws);
}

MIR_CORE_DLL(INT_PTR) db_set_blob(MCONTACT hContact, const char *szModule, const char *szSetting, void *val, unsigned len)
{
	if (g_pCurrDb == nullptr) return 1;

	DBCONTACTWRITESETTING cws;
	cws.szModule = szModule;
	cws.szSetting = szSetting;
	cws.value.type = DBVT_BLOB;
	cws.value.cpbVal = (WORD)len;
	cws.value.pbVal = (unsigned char*)val;
	return g_pCurrDb->WriteContactSetting(hContact, &cws);
}

/////////////////////////////////////////////////////////////////////////////////////////
// events

MIR_CORE_DLL(MEVENT) db_event_add(MCONTACT hContact, const DBEVENTINFO *dbei)
{
	return (g_pCurrDb == nullptr) ? 0 : g_pCurrDb->AddEvent(hContact, dbei);
}

MIR_CORE_DLL(int) db_event_count(MCONTACT hContact)
{
	return (g_pCurrDb == nullptr) ? 0 : g_pCurrDb->GetEventCount(hContact);
}

MIR_CORE_DLL(int) db_event_delete(MEVENT hDbEvent)
{
	return (g_pCurrDb == nullptr) ? 0 : g_pCurrDb->DeleteEvent(hDbEvent);
}

MIR_CORE_DLL(int) db_event_edit(MCONTACT hContact, MEVENT hDbEvent, const DBEVENTINFO *dbei)
{
	return (g_pCurrDb == nullptr) ? 0 : g_pCurrDb->EditEvent(hContact, hDbEvent, dbei);
}

MIR_CORE_DLL(MEVENT) db_event_first(MCONTACT hContact)
{
	return (g_pCurrDb == nullptr) ? 0 : g_pCurrDb->FindFirstEvent(hContact);
}

MIR_CORE_DLL(MEVENT) db_event_firstUnread(MCONTACT hContact)
{
	return (g_pCurrDb == nullptr) ? 0 : g_pCurrDb->FindFirstUnreadEvent(hContact);
}

MIR_CORE_DLL(int) db_event_get(MEVENT hDbEvent, DBEVENTINFO *dbei)
{
	return (g_pCurrDb == nullptr) ? 1 : g_pCurrDb->GetEvent(hDbEvent, dbei);
}

MIR_CORE_DLL(int) db_event_getBlobSize(MEVENT hDbEvent)
{
	return (g_pCurrDb == nullptr) ? 0 : g_pCurrDb->GetBlobSize(hDbEvent);
}

MIR_CORE_DLL(MCONTACT) db_event_getContact(MEVENT hDbEvent)
{
	return (g_pCurrDb == nullptr) ? 0 : g_pCurrDb->GetEventContact(hDbEvent);
}

MIR_CORE_DLL(MEVENT) db_event_last(MCONTACT hContact)
{
	return (g_pCurrDb == nullptr) ? 0 : g_pCurrDb->FindLastEvent(hContact);
}

MIR_CORE_DLL(int) db_event_markRead(MCONTACT hContact, MEVENT hDbEvent)
{
	return (g_pCurrDb == nullptr) ? 0 : g_pCurrDb->MarkEventRead(hContact, hDbEvent);
}

MIR_CORE_DLL(MEVENT) db_event_next(MCONTACT hContact, MEVENT hDbEvent)
{
	return (g_pCurrDb == nullptr) ? 0 : g_pCurrDb->FindNextEvent(hContact, hDbEvent);
}

MIR_CORE_DLL(MEVENT) db_event_prev(MCONTACT hContact, MEVENT hDbEvent)
{
	return (g_pCurrDb == nullptr) ? 0 : g_pCurrDb->FindPrevEvent(hContact, hDbEvent);
}

MIR_CORE_DLL(MEVENT) db_event_getById(const char *szModule, const char *szId)
{
	return (g_pCurrDb == nullptr) ? 0 : g_pCurrDb->GetEventById(szModule, szId);
}

/////////////////////////////////////////////////////////////////////////////////////////
// event cursors

DB::EventCursor::~EventCursor()
{
}

MIR_CORE_DLL(DB::EventCursor*) DB::Events(MCONTACT hContact, MEVENT iStartEvent)
{
	return (g_pCurrDb == nullptr) ? 0 : g_pCurrDb->EventCursor(hContact, iStartEvent);
}

MIR_CORE_DLL(DB::EventCursor*) DB::EventsRev(MCONTACT hContact, MEVENT iStartEvent)
{
	return (g_pCurrDb == nullptr) ? 0 : g_pCurrDb->EventCursorRev(hContact, iStartEvent);
}

DB::ECPTR::ECPTR(EventCursor *_pCursor) :
	m_cursor(_pCursor),
	m_prevFetched(-1),
	m_currEvent(0)
{
}

DB::ECPTR::~ECPTR()
{
	delete m_cursor;
}

void DB::ECPTR::DeleteEvent()
{
	m_prevFetched = m_cursor->FetchNext();
	db_event_delete(m_currEvent);
}

MEVENT DB::ECPTR::FetchNext()
{
	if (m_prevFetched != -1) {
		m_currEvent = m_prevFetched;
		m_prevFetched = -1;
	}
	else m_currEvent = m_cursor->FetchNext();

	return m_currEvent;
}

/////////////////////////////////////////////////////////////////////////////////////////
// misc functions

MIR_CORE_DLL(INT_PTR) db_free(DBVARIANT *dbv)
{
	return (g_pCurrDb == nullptr) ? 1 : g_pCurrDb->FreeVariant(dbv);
}

MIR_CORE_DLL(INT_PTR) db_unset(MCONTACT hContact, const char *szModule, const char *szSetting)
{
	if (g_pCurrDb == nullptr)
		return 1;

	return g_pCurrDb->DeleteContactSetting(hContact, szModule, szSetting);
}

MIR_CORE_DLL(DBCachedContact*) db_get_contact(MCONTACT hContact)
{
	return (g_pCurrDb == nullptr) ? nullptr : g_pCurrDb->getCache()->GetCachedContact(hContact);
}

MIR_CORE_DLL(MCONTACT) db_find_first(const char *szProto)
{
	return (g_pCurrDb == nullptr) ? 0 : g_pCurrDb->FindFirstContact(szProto);
}

MIR_CORE_DLL(MCONTACT) db_find_next(MCONTACT hContact, const char *szProto)
{
	return (g_pCurrDb == nullptr) ? 0 : g_pCurrDb->FindNextContact(hContact, szProto);
}

MIR_CORE_DLL(void) db_setCurrent(MDatabaseCommon *_db)
{
	g_pCurrDb = _db;
	if (g_pCurrDb == nullptr)
		return;

	// try to get the langpack's name from a profile
	ptrW langpack(db_get_wsa(0, "Langpack", "Current"));
	if (langpack && langpack[0] != '\0')
		LoadLangPack(langpack);
	else
		GetDefaultLang();
}

MIR_CORE_DLL(BOOL) db_set_resident(const char *szModule, const char *szService, BOOL bEnable)
{
	if (g_pCurrDb == nullptr || szModule == nullptr || szService == nullptr)
		return FALSE;

	char str[MAXMODULELABELLENGTH * 2];
	mir_snprintf(str, "%s/%s", szModule, szService);
	return g_pCurrDb->SetSettingResident(bEnable, str);
}
