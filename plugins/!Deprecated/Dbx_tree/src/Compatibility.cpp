/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

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

#include "Interface.h"
#include "Compatibility.h"
#include "Logger.h"
#define DB_NOHELPERFUNCTIONS
#include "m_database.h"
#include "m_db_int.h"
#undef DB_NOHELPERFUNCTIONS

HANDLE gEvents[6] = {0,0,0,0,0,0};

HANDLE hEventDeletedEvent,
       hEventAddedEvent,
		 hEventFilterAddedEvent,
		 hSettingChangeEvent,
		 hContactDeletedEvent,
		 hContactAddedEvent;

int CDataBase::CheckProto(DBCachedContact *cc, const char *proto)
{
	if (cc->szProto == NULL) {
		char protobuf[MAX_PATH] = {0};
		DBVARIANT dbv;
 		dbv.type = DBVT_ASCIIZ;
		dbv.pszVal = protobuf;
		dbv.cchVal = sizeof(protobuf);
		if (GetContactSettingStatic(cc->contactID, "Protocol", "p", &dbv) != 0 || (dbv.type != DBVT_ASCIIZ))
			return 0;

		cc->szProto = m_cache->GetCachedSetting(NULL, protobuf, 0, (int)strlen(protobuf));
	}

	return !strcmp(cc->szProto, proto);
}

STDMETHODIMP_(HANDLE) CDataBase::AddContact(void)
{
	TDBTEntity entity = {0,0,0,0};
	entity.hParentEntity = DBEntityGetRoot(0, 0);
	entity.hAccountEntity = entity.hParentEntity;

	TDBTEntityHandle res = getEntities().CreateEntity(entity);
	if (res == DBT_INVALIDPARAM)
		return (HANDLE)1;

	m_cache->AddContactToCache(res);

	NotifyEventHooks(hContactAddedEvent, res, 0);
	return (HANDLE)res;
}

STDMETHODIMP_(LONG) CDataBase::DeleteContact(MCONTACT contactID)
{
	NotifyEventHooks(hContactDeletedEvent, contactID, 0);

	int res = DBEntityDelete(contactID, 0);
	if (res == DBT_INVALIDPARAM)
		return 1;

	if (res == 0)
		m_cache->FreeCachedContact(contactID);

	return res;
}

STDMETHODIMP_(BOOL) CDataBase::IsDbContact(MCONTACT contactID)
{
	int flags = DBEntityGetFlags(contactID, 0);
	return (flags != DBT_INVALIDPARAM) &&
		     ((flags & DBT_NFM_SpecialEntity) == 0);
}

STDMETHODIMP_(LONG) CDataBase::GetContactCount(void)
{
	TDBTEntityIterFilter f = {0,0,0,0};
	f.cbSize = sizeof(f);
	f.fDontHasFlags = DBT_NF_IsGroup | DBT_NF_IsVirtual | DBT_NF_IsAccount | DBT_NF_IsRoot;
	f.Options = DBT_NIFO_OSC_AC | DBT_NIFO_OC_AC;

	TDBTEntityIterationHandle hiter = DBEntityIterInit((WPARAM)&f, getEntities().getRootEntity());
	int c = 0;
	if ((hiter != 0) && (hiter != DBT_INVALIDPARAM))
	{
		TDBTEntityHandle con = DBEntityIterNext(hiter, 0);

		while ((con != DBT_INVALIDPARAM) && (con != 0))
		{
			if ((con != 0) && (con != DBT_INVALIDPARAM))
				c++;

			con = DBEntityIterNext(hiter, 0);
		}
		DBEntityIterClose(hiter, 0);
	}
	return c;
}

STDMETHODIMP_(MCONTACT) CDataBase::FindFirstContact(const char* szProto)
{
	DBCachedContact *cc = m_cache->GetFirstContact();
	if (cc == NULL)
		return NULL;

	if (!szProto || CheckProto(cc, szProto))
		return cc->contactID;

	return FindNextContact(cc->contactID, szProto);
}

STDMETHODIMP_(MCONTACT) CDataBase::FindNextContact(MCONTACT contactID, const char* szProto)
{
	while (contactID) {
		DBCachedContact *cc = m_cache->GetNextContact(contactID);
		if (cc == NULL)
			break;

		if (!szProto || CheckProto(cc, szProto))
			return cc->contactID;

		contactID = cc->contactID;
		continue;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool isEncrypted(LPCSTR szModule, LPCSTR szSetting)
{
	if (!_strnicmp(szSetting, "password", 8))      return true;
	if (!strcmp(szSetting, "NLProxyAuthPassword")) return true;
	if (!strcmp(szSetting, "LNPassword"))          return true;
	if (!strcmp(szSetting, "FileProxyPassword"))   return true;
	if (!strcmp(szSetting, "TokenSecret"))         return true;

	if (!strcmp(szModule, "SecureIM")) {
		if (!strcmp(szSetting, "pgp"))              return true;
		if (!strcmp(szSetting, "pgpPrivKey"))       return true;
	}
	return false;
}

//VERY VERY VERY BASIC ENCRYPTION FUNCTION

static void Encrypt(char *msg, BOOL up)
{
	int jump = (up) ? 5 : -5;
	for (int i = 0; msg[i]; i++)
		msg[i] = msg[i] + jump;
}

__forceinline void EncodeString(LPSTR buf)
{
	Encrypt(buf, TRUE);
}

__forceinline void DecodeString(LPSTR buf)
{
	Encrypt(buf, FALSE);
}

/////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP_(BOOL) CDataBase::GetContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv)
{
	dbv->type = 0;

	char namebuf[512];
	namebuf[0] = 0;

	if (!(szModule || szSetting))
		return -1;

	if (szModule)
		strcpy_s(namebuf, szModule);
	strcat_s(namebuf, "/");
	if (szSetting)
		strcat_s(namebuf, szSetting);

	TDBTSettingDescriptor desc = {0,0,0,0,0,0,0,0};
	TDBTSetting set = {0,0,0,0};
	desc.cbSize = sizeof(desc);
	desc.Entity = contactID;
	desc.pszSettingName = namebuf;

	set.cbSize = sizeof(set);
	set.Descriptor = &desc;

	if (DBSettingRead(reinterpret_cast<WPARAM>(&set), 0) == DBT_INVALIDPARAM)
		return -1;

	switch (set.Type) {
	case DBT_ST_ANSI:
		dbv->type = DBVT_ASCIIZ;
		dbv->pszVal = set.Value.pAnsi;
		dbv->cchVal = set.Value.Length - 1;
		if (isEncrypted(szModule, szSetting))
			DecodeString(dbv->pszVal);
		break;
	case DBT_ST_UTF8:
		if (isEncrypted(szModule, szSetting))
			DecodeString(set.Value.pUTF8);
		dbv->type = DBVT_WCHAR;
		dbv->pwszVal = mir_utf8decodeW(set.Value.pUTF8);
		if (dbv->pwszVal)
			dbv->cchVal = static_cast<uint32_t>(wcslen(dbv->pwszVal));
		else
			dbv->cchVal = 0;
		mir_free(set.Value.pUTF8);
		break;
	case DBT_ST_WCHAR:
		dbv->type = DBVT_WCHAR;
		dbv->pwszVal = set.Value.pWide;
		dbv->cchVal = set.Value.Length - 1;
		break;
	case DBT_ST_BLOB:
		dbv->type = DBVT_BLOB;
		dbv->pbVal = set.Value.pBlob;
		dbv->cpbVal = set.Value.Length;
		break;
	case DBT_ST_BOOL:
		dbv->type = DBVT_BYTE;
		dbv->bVal = (uint8_t)set.Value.Bool;
		break;
	case DBT_ST_BYTE: case DBT_ST_CHAR:
		dbv->type = DBVT_BYTE;
		dbv->bVal = set.Value.Byte;
		break;
	case DBT_ST_SHORT: case DBT_ST_WORD:
		dbv->type = DBVT_WORD;
		dbv->wVal = set.Value.Word;
		break;
	case DBT_ST_INT: case DBT_ST_DWORD:
		dbv->type = DBVT_DWORD;
		dbv->dVal = set.Value.DWord;
		break;
	case DBT_ST_INT64: case DBT_ST_QWORD:
	case DBT_ST_DOUBLE: case DBT_ST_FLOAT:
		dbv->type = DBVT_BLOB;
		dbv->cpbVal = sizeof(set.Value);
		dbv->pbVal = reinterpret_cast<BYTE*>(mir_alloc(sizeof(set.Value)));
		memcpy(dbv->pbVal, &set.Value, sizeof(set.Value));
		break;

	default:
		return -1;
	}

	return 0;
}

STDMETHODIMP_(BOOL) CDataBase::GetContactSettingStr(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv)
{
	if ((dbv->type & DBVTF_VARIABLELENGTH) == 0)
	{
		FreeVariant(dbv);
		dbv->type = 0;
	}

	char namebuf[512];
	namebuf[0] = 0;
	if (szModule)
		strcpy_s(namebuf, szModule);
	strcat_s(namebuf, "/");
	if (szSetting)
		strcat_s(namebuf, szSetting);

	TDBTSettingDescriptor desc = {0,0,0,0,0,0,0,0};
	TDBTSetting set = {0,0,0,0};
	desc.cbSize = sizeof(desc);
	desc.Entity = contactID;
	desc.pszSettingName = namebuf;

	set.cbSize = sizeof(set);
	set.Descriptor = &desc;

	switch (dbv->type) {
		case DBVT_ASCIIZ: set.Type = DBT_ST_ANSI; break;
		case DBVT_BLOB:   set.Type = DBT_ST_BLOB; break;
		case DBVT_UTF8:   set.Type = DBT_ST_UTF8; break;
		case DBVT_WCHAR:  set.Type = DBT_ST_WCHAR; break;
	}

	if (DBSettingRead(reinterpret_cast<WPARAM>(&set), 0) == DBT_INVALIDPARAM)
		return -1;

	switch (set.Type) {
	case DBT_ST_ANSI:
		dbv->type = DBVT_ASCIIZ;
		dbv->pszVal = set.Value.pAnsi;
		dbv->cchVal = set.Value.Length - 1;
		if (isEncrypted(szModule, szSetting))
			DecodeString(dbv->pszVal);
		break;
	case DBT_ST_UTF8:
		dbv->type = DBVT_UTF8;
		dbv->pszVal = set.Value.pUTF8;
		dbv->cchVal = set.Value.Length - 1;
		if (isEncrypted(szModule, szSetting))
			DecodeString(dbv->pszVal);
		break;
	case DBT_ST_WCHAR:
		if (dbv->type == DBVT_WCHAR) {
			dbv->pwszVal = set.Value.pWide;
			dbv->cchVal = set.Value.Length - 1;
		}
		else {
			dbv->type = DBVT_UTF8;
			dbv->pszVal = mir_utf8encodeW(set.Value.pWide);
			dbv->cchVal = static_cast<uint32_t>(strlen(dbv->pszVal));
			if (isEncrypted(szModule, szSetting))
				DecodeString(dbv->pszVal);
			mir_free(set.Value.pWide);
		}
		break;
	case DBT_ST_BLOB:
		dbv->type = DBVT_BLOB;
		dbv->pbVal = set.Value.pBlob;
		dbv->cpbVal = set.Value.Length;
		break;
	case DBT_ST_BOOL:
		dbv->type = DBVT_BYTE;
		dbv->bVal = (uint8_t)set.Value.Bool;
		break;
	case DBT_ST_BYTE: case DBT_ST_CHAR:
		dbv->type = DBVT_BYTE;
		dbv->bVal = set.Value.Byte;
		break;
	case DBT_ST_SHORT: case DBT_ST_WORD:
		dbv->type = DBVT_WORD;
		dbv->wVal = set.Value.Word;
		break;
	case DBT_ST_INT: case DBT_ST_DWORD:
		dbv->type = DBVT_DWORD;
		dbv->dVal = set.Value.DWord;
		break;
	case DBT_ST_INT64: case DBT_ST_QWORD:
	case DBT_ST_DOUBLE: case DBT_ST_FLOAT:
		dbv->type = DBVT_BLOB;
		dbv->cpbVal = sizeof(set.Value);
		dbv->pbVal = reinterpret_cast<BYTE*>(mir_alloc(sizeof(set.Value)));
		memcpy(dbv->pbVal, &set.Value, sizeof(set.Value));
		break;
	default:
		return -1;
	}

	return 0;
}

STDMETHODIMP_(BOOL) CDataBase::GetContactSettingStatic(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv)
{
	char namebuf[512];
	namebuf[0] = 0;
	if (szModule)
		strcpy_s(namebuf, szModule);
	strcat_s(namebuf, "/");
	if (szSetting)
		strcat_s(namebuf, szSetting);

	TDBTSettingDescriptor desc = {0,0,0,0,0,0,0,0};
	TDBTSetting set = {0,0,0,0};
	desc.cbSize = sizeof(desc);
	desc.Entity = contactID;
	desc.pszSettingName = namebuf;

	set.cbSize = sizeof(set);
	set.Descriptor = &desc;

	if (DBSettingRead(reinterpret_cast<WPARAM>(&set), 0) == DBT_INVALIDPARAM)
		return -1;

	if ((set.Type & DBT_STF_VariableLength) ^ (dbv->type & DBVTF_VARIABLELENGTH))
	{
		if (set.Type & DBT_STF_VariableLength)
			mir_free(set.Value.pBlob);
		return -1;
	}

	switch (set.Type) {
	case DBT_ST_ANSI:
		if (dbv->cchVal < set.Value.Length) {
			memcpy(dbv->pszVal, set.Value.pAnsi, dbv->cchVal);
			dbv->pszVal[dbv->cchVal - 1] = 0;
		}
		else memcpy(dbv->pszVal, set.Value.pAnsi, set.Value.Length);

		dbv->type = DBVT_ASCIIZ;
		dbv->cchVal = set.Value.Length - 1;
		if (isEncrypted(szModule, szSetting))
			DecodeString(dbv->pszVal);

		mir_free(set.Value.pAnsi);
		break;
	case DBT_ST_UTF8:
		set.Value.pUTF8 = mir_utf8decode(set.Value.pUTF8, NULL);
		set.Value.Length = static_cast<uint32_t>(strlen(set.Value.pUTF8));

		if (dbv->cchVal < set.Value.Length) {
			memcpy(dbv->pszVal, set.Value.pUTF8, dbv->cchVal);
			dbv->pszVal[dbv->cchVal - 1] = 0;
		}
		else memcpy(dbv->pszVal, set.Value.pUTF8, set.Value.Length);

		dbv->type = DBVT_ASCIIZ;
		dbv->cchVal = set.Value.Length - 1;
		if (isEncrypted(szModule, szSetting))
			DecodeString(dbv->pszVal);

		mir_free(set.Value.pUTF8);
		break;
	case DBT_ST_WCHAR:
		{
			char *tmp = mir_u2a(set.Value.pWide);
			WORD l = static_cast<WORD>(strlen(tmp));
			mir_free(set.Value.pWide);

			if (dbv->cchVal < l + 1) {
				memcpy(dbv->pszVal, tmp, dbv->cchVal);
				dbv->pszVal[l] = 0;
			}
			else memcpy(dbv->pszVal, tmp, l + 1);

			dbv->type = DBVT_ASCIIZ;
			dbv->cchVal = l;
			if (isEncrypted(szModule, szSetting))
				DecodeString(dbv->pszVal);

			mir_free(tmp);
		}
		break;
	case DBT_ST_BLOB:
		if (dbv->cchVal < set.Value.Length)
			memcpy(dbv->pbVal, set.Value.pBlob, dbv->cchVal);
		else
			memcpy(dbv->pbVal, set.Value.pBlob, set.Value.Length);

		dbv->type = DBVT_BLOB;
		dbv->cchVal = set.Value.Length;
		mir_free(set.Value.pBlob);
		break;
	case DBT_ST_BOOL:
		dbv->type = DBVT_BYTE;
		dbv->bVal = set.Value.Bool ? TRUE : FALSE;
		break;
	case DBT_ST_BYTE: case DBT_ST_CHAR:
		dbv->type = DBVT_BYTE;
		dbv->bVal = set.Value.Byte;
		break;
	case DBT_ST_SHORT: case DBT_ST_WORD:
		dbv->type = DBVT_WORD;
		dbv->wVal = set.Value.Word;
		break;
	case DBT_ST_INT: case DBT_ST_DWORD:
		dbv->type = DBVT_DWORD;
		dbv->dVal = set.Value.DWord;
		break;
	default:
		return -1;
	}

	return 0;
}

STDMETHODIMP_(BOOL) CDataBase::FreeVariant(DBVARIANT *dbv)
{
	if (dbv->type == DBVT_BLOB && dbv->pbVal) {
		mir_free(dbv->pbVal);
		dbv->pbVal = 0;
	}
	else if ((dbv->type & DBVTF_VARIABLELENGTH) && (dbv->pszVal)) {
		mir_free(dbv->pszVal);
		dbv->pszVal = NULL;
	}
	dbv->type = 0;
	return 0;
}

STDMETHODIMP_(BOOL) CDataBase::WriteContactSetting(MCONTACT contactID, DBCONTACTWRITESETTING *dbcws)
{
	char namebuf[512];
	namebuf[0] = 0;
	if (dbcws->szModule)
		strcpy_s(namebuf, dbcws->szModule);
	strcat_s(namebuf, "/");
	if (dbcws->szSetting)
		strcat_s(namebuf, dbcws->szSetting);

	TDBTSettingDescriptor desc = {0,0,0,0,0,0,0,0};
	TDBTSetting set = {0,0,0,0};
	desc.cbSize = sizeof(desc);
	desc.Entity = contactID;
	desc.pszSettingName = namebuf;

	set.cbSize = sizeof(set);
	set.Descriptor = &desc;

	switch (dbcws->value.type) {
	case DBVT_ASCIIZ:
		set.Type = DBT_ST_ANSI;
		set.Value.pAnsi = dbcws->value.pszVal;
		if (isEncrypted(dbcws->szModule, dbcws->szSetting))
			EncodeString(dbcws->value.pszVal);
		break;
	case DBVT_UTF8:
		if (isEncrypted(dbcws->szModule, dbcws->szSetting))
			EncodeString(dbcws->value.pszVal);
		{
			wchar_t * tmp = mir_utf8decodeW(dbcws->value.pszVal);
			if (tmp == 0) {
				if (IsDebuggerPresent())
				{
					DebugBreak();
#ifdef _DEBUG
				}
				else {
					LOG(logWARNING, _T("Trying to write malformed UTF8 setting \"%hs\" in module \"%hs\""), dbcws->szSetting, dbcws->szModule);
					CLogger::Instance().ShowMessage();
#endif
				}
				return -1;
			}
			else mir_free(tmp);
		}

		set.Type = DBT_ST_UTF8;
		set.Value.pUTF8 = dbcws->value.pszVal;
		break;
	case DBVT_WCHAR:
		set.Type = DBT_ST_WCHAR;
		set.Value.pWide = dbcws->value.pwszVal;
		break;
	case DBVT_BLOB:
		set.Type = DBT_ST_BLOB;
		set.Value.pBlob = dbcws->value.pbVal;
		set.Value.Length = dbcws->value.cpbVal;
		break;
	case DBVT_BYTE:
		set.Type = DBT_ST_BYTE;
		set.Value.Byte = dbcws->value.bVal;
		break;
	case DBVT_WORD:
		set.Type = DBT_ST_WORD;
		set.Value.Word = dbcws->value.wVal;
		break;
	case DBVT_DWORD:
		set.Type = DBT_ST_DWORD;
		set.Value.DWord = dbcws->value.dVal;
		break;
	default:
		return -1;
	}

	if (DBSettingWrite(reinterpret_cast<WPARAM>(&set), 0) == DBT_INVALIDPARAM)
		return -1;

	if (dbcws->value.type == DBVT_WCHAR) {
		dbcws->value.type = DBVT_UTF8;
		wchar_t * tmp = dbcws->value.pwszVal;
		dbcws->value.pszVal = mir_utf8encodeW(dbcws->value.pwszVal);
		NotifyEventHooks(hSettingChangeEvent, contactID, (LPARAM)dbcws);
		mir_free(dbcws->value.pszVal);
		dbcws->value.type = DBVT_WCHAR;
		dbcws->value.pwszVal = tmp;		
	}
	else NotifyEventHooks(hSettingChangeEvent, contactID, (LPARAM)dbcws);

	return 0;
}

STDMETHODIMP_(BOOL) CDataBase::DeleteContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting)
{
	char namebuf[512];
	namebuf[0] = 0;
	if (szModule)
		strcpy_s(namebuf, szModule);
	strcat_s(namebuf, "/");
	if (szSetting)
		strcat_s(namebuf, szSetting);

	TDBTSettingDescriptor desc = {0,0,0,0,0,0,0,0};
	desc.cbSize = sizeof(desc);
	desc.Entity = contactID;
	desc.pszSettingName = namebuf;

	if (DBSettingDelete(reinterpret_cast<WPARAM>(&desc), 0) == DBT_INVALIDPARAM)
		return -1;

	{
		DBCONTACTWRITESETTING tmp = {0,0,0,0};
		tmp.szModule = szModule;
		tmp.szSetting = szSetting;
		tmp.value.type = 0;
		NotifyEventHooks(hSettingChangeEvent, contactID, (LPARAM)&tmp);
	}

	return 0;
}

STDMETHODIMP_(BOOL) CDataBase::EnumContactSettings(MCONTACT contactID, DBCONTACTENUMSETTINGS* pces)
{
	TDBTSettingDescriptor desc = {0,0,0,0,0,0,0,0};
	desc.cbSize = sizeof(desc);
	desc.Entity = contactID;

	char namebuf[512];
	namebuf[0] = 0;
	if (pces->szModule)
		strcpy_s(namebuf, pces->szModule);
	strcat_s(namebuf, "/");

	TDBTSettingIterFilter filter = {0,0,0,0,0,0,0,0};
	filter.cbSize = sizeof(filter);
	filter.Descriptor = &desc;
	filter.hEntity = (WPARAM)contactID;
	filter.NameStart = namebuf;

	TDBTSettingIterationHandle hiter = DBSettingIterInit(reinterpret_cast<WPARAM>(&filter), 0);
	if ((hiter == 0) || (hiter == DBT_INVALIDPARAM))
		return -1;

	int res = 0;
	TDBTSettingHandle hset = DBSettingIterNext(hiter, 0);
	while (hset != 0)
	{
		char * p = strchr(desc.pszSettingName, '/');
		if (p) {
			++p;
		} else {
			p = desc.pszSettingName;
		}

		res = pces->pfnEnumProc(p, pces->lParam);
		if (res == 0)
		{
			hset = DBSettingIterNext(hiter, 0);
		} else {
			hset = 0;
		}
	}

	DBSettingIterClose(hiter, 0);

	if (desc.pszSettingName)
		mir_free(desc.pszSettingName);

	return res;
}

STDMETHODIMP_(LONG) CDataBase::GetEventCount(MCONTACT contactID)
{
	if (contactID == 0)
		contactID = getEntities().getRootEntity();

	return DBEventGetCount(contactID, 0);
}

STDMETHODIMP_(HANDLE) CDataBase::AddEvent(MCONTACT contactID, DBEVENTINFO *dbei)
{
	if (dbei->cbSize < sizeof(DBEVENTINFO))
		return (HANDLE)-1;

	int tmp = NotifyEventHooks(hEventFilterAddedEvent, contactID, (LPARAM)dbei);
	if (tmp != 0)
		return (HANDLE)tmp;

	if (contactID == 0)
		contactID = getEntities().getRootEntity();

	TDBTEvent ev = {0,0,0,0,0,0,0};
	ev.cbSize = sizeof(ev);
	ev.ModuleName = dbei->szModule;
	ev.Timestamp = dbei->timestamp;
	ev.Flags = dbei->flags;
	if (ev.Flags & DBEF_SENT)
		ev.Flags = ev.Flags | DBEF_READ;
	ev.EventType = dbei->eventType;
	ev.cbBlob = dbei->cbBlob;
	ev.pBlob = dbei->pBlob;

	int res = DBEventAdd(contactID, reinterpret_cast<LPARAM>(&ev));
	if (res != DBT_INVALIDPARAM)
	{
		NotifyEventHooks(hEventAddedEvent, contactID, res);
		return (HANDLE)res;
	}
	return NULL;
}

STDMETHODIMP_(BOOL) CDataBase::DeleteEvent(MCONTACT contactID, HANDLE hDbEvent)
{
	int res = NotifyEventHooks(hEventDeletedEvent, contactID, (WPARAM)hDbEvent);

	if (contactID == 0)
		contactID = getEntities().getRootEntity();

	if (res == 0)
		return DBEventDelete((WPARAM)hDbEvent, 0);

	return res;
}

STDMETHODIMP_(LONG) CDataBase::GetBlobSize(HANDLE hDbEvent)
{
	int res = DBEventGetBlobSize((WPARAM)hDbEvent, 0);
	if (res == DBT_INVALIDPARAM)
		return -1;

	return res;
}

STDMETHODIMP_(BOOL) CDataBase::GetEvent(HANDLE hDbEvent, DBEVENTINFO *dbei)
{
	if (dbei->cbSize < sizeof(DBEVENTINFO))
		return -1;

	TDBTEvent ev = {0,0,0,0,0,0,0};
	ev.cbSize = sizeof(ev);
	ev.cbBlob = 0;
	ev.pBlob = NULL;

	int res = DBEventGet((WPARAM)hDbEvent, reinterpret_cast<LPARAM>(&ev));

	dbei->szModule = ev.ModuleName;
	dbei->timestamp = ev.Timestamp;
	dbei->flags = ev.Flags;
	if (dbei->flags & DBEF_SENT)
		dbei->flags = dbei->flags & ~DBEF_READ;
	dbei->eventType = ev.EventType;

	if (dbei->cbBlob && dbei->pBlob)
	{
		if (dbei->cbBlob >= ev.cbBlob)
			memcpy(dbei->pBlob, ev.pBlob, ev.cbBlob);
		else
			memcpy(dbei->pBlob, ev.pBlob, dbei->cbBlob);
	}
	mir_free(ev.pBlob);
	dbei->cbBlob = ev.cbBlob;

	if (res == DBT_INVALIDPARAM)
		return 1;

	return res;
}

STDMETHODIMP_(BOOL) CDataBase::MarkEventRead(MCONTACT contactID, HANDLE hDbEvent)
{
	int res = DBEventMarkRead((WPARAM)hDbEvent, 0);
	if ((res != DBT_INVALIDPARAM) && (res & DBEF_SENT))
		res = res & ~DBEF_READ;
	return res;
}

STDMETHODIMP_(MCONTACT) CDataBase::GetEventContact(HANDLE hDbEvent)
{
	TDBTEntityHandle res = DBEventGetEntity((WPARAM)hDbEvent, 0);
	if (res == getEntities().getRootEntity())
		res = 0;

	return res;
}

STDMETHODIMP_(HANDLE) CDataBase::FindFirstEvent(MCONTACT contactID)
{
	if (contactID == 0)
		contactID = getEntities().getRootEntity();

	return (HANDLE)getEvents().compFirstEvent(contactID);
}

STDMETHODIMP_(HANDLE) CDataBase::FindFirstUnreadEvent(MCONTACT contactID)
{
	if (contactID == 0)
		contactID = getEntities().getRootEntity();
	return (HANDLE)getEvents().compFirstUnreadEvent(contactID);
}

STDMETHODIMP_(HANDLE) CDataBase::FindLastEvent(MCONTACT contactID)
{
	if (contactID == 0)
		contactID = getEntities().getRootEntity();
	return (HANDLE)getEvents().compLastEvent(contactID);
}

STDMETHODIMP_(HANDLE) CDataBase::FindNextEvent(HANDLE hDbEvent)
{
	return (HANDLE)getEvents().compNextEvent((WPARAM)hDbEvent);
}

STDMETHODIMP_(HANDLE) CDataBase::FindPrevEvent(HANDLE hDbEvent)
{
	return (HANDLE)getEvents().compPrevEvent((WPARAM)hDbEvent);
}

STDMETHODIMP_(BOOL) CDataBase::EnumModuleNames(DBMODULEENUMPROC pCallback, void *pParam)
{
	if (!pCallback)
		return -1;
	
	return getSettings().CompEnumModules(pCallback, (WPARAM)pParam);
}

STDMETHODIMP_(BOOL) CDataBase::SetSettingResident(BOOL bIsResident, const char *pszSettingName)
{
	return FALSE;
}

STDMETHODIMP_(BOOL) CDataBase::EnumResidentSettings(DBMODULEENUMPROC pFunc, void *pParam)
{
	return FALSE;
}

STDMETHODIMP_(void) CDataBase::SetCacheSafetyMode(BOOL) {}


STDMETHODIMP_(BOOL) CDataBase::IsSettingEncrypted(LPCSTR szModule, LPCSTR szSetting)
{
	return FALSE;
}

bool CompatibilityRegister()
{
	hEventDeletedEvent     = CreateHookableEvent(ME_DB_EVENT_DELETED);
	hEventAddedEvent       = CreateHookableEvent(ME_DB_EVENT_ADDED);
	hEventFilterAddedEvent = CreateHookableEvent(ME_DB_EVENT_FILTER_ADD);
	hSettingChangeEvent    = CreateHookableEvent(ME_DB_CONTACT_SETTINGCHANGED);
	hContactDeletedEvent   = CreateHookableEvent(ME_DB_CONTACT_DELETED);
	hContactAddedEvent     = CreateHookableEvent(ME_DB_CONTACT_ADDED);
	return true;
}
