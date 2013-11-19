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

int CDataBase::CheckProto(HANDLE hContact, const char *proto)
{
	DBCachedContact *cc = m_cache->GetCachedContact(hContact);
	if (cc == NULL)
		cc = m_cache->AddContactToCache(hContact);

	if (cc->szProto == NULL) {
		char protobuf[MAX_PATH] = {0};
		DBVARIANT dbv;
		DBCONTACTGETSETTING sVal = { "Protocol", "p", &dbv };

 		dbv.type = DBVT_ASCIIZ;
		dbv.pszVal = protobuf;
		dbv.cchVal = sizeof(protobuf);
		if ( GetContactSettingStatic(hContact, &sVal) != 0 || (dbv.type != DBVT_ASCIIZ))
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

	NotifyEventHooks(hContactAddedEvent, res, 0);
	return (HANDLE)res;
}

STDMETHODIMP_(LONG) CDataBase::DeleteContact(HANDLE hContact)
{
	NotifyEventHooks(hContactDeletedEvent, (WPARAM)hContact, 0);

	int res = DBEntityDelete((WPARAM)hContact, 0);
	if (res == DBT_INVALIDPARAM)
		return 1;

	if (res == 0)
		m_cache->FreeCachedContact(hContact);

	return res;
}

STDMETHODIMP_(BOOL) CDataBase::IsDbContact(HANDLE hContact)
{
	int flags = DBEntityGetFlags((WPARAM)hContact, 0);
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

STDMETHODIMP_(HANDLE) CDataBase::FindFirstContact(const char* szProto)
{
	HANDLE hContact = (HANDLE)getEntities().compFirstContact();
	if (!szProto || CheckProto(hContact, szProto))
		return hContact;

	return FindNextContact(hContact, szProto);
}

STDMETHODIMP_(HANDLE) CDataBase::FindNextContact(HANDLE hContact, const char* szProto)
{
	while (hContact) {
		DBCachedContact *VL = m_cache->GetCachedContact(hContact);
		if (VL == NULL)
			VL = m_cache->AddContactToCache(hContact);

		if (VL->hNext != NULL) {
			if (!szProto || CheckProto(VL->hNext, szProto))
				return VL->hNext;

			hContact = VL->hNext;
			continue;
		}

		VL->hNext = (HANDLE)getEntities().compNextContact((WPARAM)hContact);
		if (VL->hNext != NULL && (!szProto || CheckProto(VL->hNext, szProto)))
			return VL->hNext;

		hContact = VL->hNext;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool isEncrypted(LPCSTR szModule, LPCSTR szSetting)
{
	if (!_strnicmp(szSetting, "password", 8))       return true;
	if (!strcmp(szSetting, "NLProxyAuthPassword")) return true;
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

STDMETHODIMP_(BOOL) CDataBase::GetContactSetting(HANDLE hContact, DBCONTACTGETSETTING *dbcgs)
{
	dbcgs->pValue->type = 0;

	char namebuf[512];
	namebuf[0] = 0;

	if (!(dbcgs->szModule || dbcgs->szSetting))
		return -1;

	if (dbcgs->szModule)
		strcpy_s(namebuf, dbcgs->szModule);
	strcat_s(namebuf, "/");
	if (dbcgs->szSetting)
		strcat_s(namebuf, dbcgs->szSetting);

	TDBTSettingDescriptor desc = {0,0,0,0,0,0,0,0};
	TDBTSetting set = {0,0,0,0};
	desc.cbSize = sizeof(desc);
	desc.Entity = (WPARAM)hContact;
	desc.pszSettingName = namebuf;

	set.cbSize = sizeof(set);
	set.Descriptor = &desc;

	if (DBSettingRead(reinterpret_cast<WPARAM>(&set), 0) == DBT_INVALIDPARAM)
		return -1;

	switch (set.Type) {
	case DBT_ST_ANSI:
		dbcgs->pValue->type = DBVT_ASCIIZ;
		dbcgs->pValue->pszVal = set.Value.pAnsi;
		dbcgs->pValue->cchVal = set.Value.Length - 1;
		if (isEncrypted(dbcgs->szModule, dbcgs->szSetting))
			DecodeString(dbcgs->pValue->pszVal);
		break;
	case DBT_ST_UTF8:
		if (isEncrypted(dbcgs->szModule, dbcgs->szSetting))
			DecodeString(set.Value.pUTF8);
		dbcgs->pValue->type = DBVT_WCHAR;
		dbcgs->pValue->pwszVal = mir_utf8decodeW(set.Value.pUTF8);
		if (dbcgs->pValue->pwszVal)
			dbcgs->pValue->cchVal = static_cast<uint32_t>(wcslen(dbcgs->pValue->pwszVal));
		else
			dbcgs->pValue->cchVal = 0;
		mir_free(set.Value.pUTF8);
		break;
	case DBT_ST_WCHAR:
		dbcgs->pValue->type = DBVT_WCHAR;
		dbcgs->pValue->pwszVal = set.Value.pWide;
		dbcgs->pValue->cchVal = set.Value.Length - 1;
		break;
	case DBT_ST_BLOB:
		dbcgs->pValue->type = DBVT_BLOB;
		dbcgs->pValue->pbVal = set.Value.pBlob;
		dbcgs->pValue->cpbVal = set.Value.Length;
		break;
	case DBT_ST_BOOL:
		dbcgs->pValue->type = DBVT_BYTE;
		dbcgs->pValue->bVal = (uint8_t)set.Value.Bool;
		break;
	case DBT_ST_BYTE: case DBT_ST_CHAR:
		dbcgs->pValue->type = DBVT_BYTE;
		dbcgs->pValue->bVal = set.Value.Byte;
		break;
	case DBT_ST_SHORT: case DBT_ST_WORD:
		dbcgs->pValue->type = DBVT_WORD;
		dbcgs->pValue->wVal = set.Value.Word;
		break;
	case DBT_ST_INT: case DBT_ST_DWORD:
		dbcgs->pValue->type = DBVT_DWORD;
		dbcgs->pValue->dVal = set.Value.DWord;
		break;
	case DBT_ST_INT64: case DBT_ST_QWORD:
	case DBT_ST_DOUBLE: case DBT_ST_FLOAT:
		dbcgs->pValue->type = DBVT_BLOB;
		dbcgs->pValue->cpbVal = sizeof(set.Value);
		dbcgs->pValue->pbVal = reinterpret_cast<BYTE*>(mir_alloc(sizeof(set.Value)));
		memcpy(dbcgs->pValue->pbVal, &set.Value, sizeof(set.Value));
		break;

	default:
		return -1;
	}

	return 0;
}

STDMETHODIMP_(BOOL) CDataBase::GetContactSettingStr(HANDLE hContact, DBCONTACTGETSETTING *dbcgs)
{
	if ((dbcgs->pValue->type & DBVTF_VARIABLELENGTH) == 0)
	{
		FreeVariant(dbcgs->pValue);
		dbcgs->pValue->type = 0;
	}

	char namebuf[512];
	namebuf[0] = 0;
	if (dbcgs->szModule)
		strcpy_s(namebuf, dbcgs->szModule);
	strcat_s(namebuf, "/");
	if (dbcgs->szSetting)
		strcat_s(namebuf, dbcgs->szSetting);

	TDBTSettingDescriptor desc = {0,0,0,0,0,0,0,0};
	TDBTSetting set = {0,0,0,0};
	desc.cbSize = sizeof(desc);
	desc.Entity = (WPARAM)hContact;
	desc.pszSettingName = namebuf;

	set.cbSize = sizeof(set);
	set.Descriptor = &desc;

	switch (dbcgs->pValue->type) {
		case DBVT_ASCIIZ: set.Type = DBT_ST_ANSI; break;
		case DBVT_BLOB:   set.Type = DBT_ST_BLOB; break;
		case DBVT_UTF8:   set.Type = DBT_ST_UTF8; break;
		case DBVT_WCHAR:  set.Type = DBT_ST_WCHAR; break;
	}

	if (DBSettingRead(reinterpret_cast<WPARAM>(&set), 0) == DBT_INVALIDPARAM)
		return -1;

	switch (set.Type) {
	case DBT_ST_ANSI:
		dbcgs->pValue->type = DBVT_ASCIIZ;
		dbcgs->pValue->pszVal = set.Value.pAnsi;
		dbcgs->pValue->cchVal = set.Value.Length - 1;
		if (isEncrypted(dbcgs->szModule, dbcgs->szSetting))
			DecodeString(dbcgs->pValue->pszVal);
		break;
	case DBT_ST_UTF8:
		dbcgs->pValue->type = DBVT_UTF8;
		dbcgs->pValue->pszVal = set.Value.pUTF8;
		dbcgs->pValue->cchVal = set.Value.Length - 1;
		if (isEncrypted(dbcgs->szModule, dbcgs->szSetting))
			DecodeString(dbcgs->pValue->pszVal);
		break;
	case DBT_ST_WCHAR:
		if (dbcgs->pValue->type == DBVT_WCHAR) {
			dbcgs->pValue->pwszVal = set.Value.pWide;
			dbcgs->pValue->cchVal = set.Value.Length - 1;
		}
		else {
			dbcgs->pValue->type = DBVT_UTF8;
			dbcgs->pValue->pszVal = mir_utf8encodeW(set.Value.pWide);
			dbcgs->pValue->cchVal = static_cast<uint32_t>(strlen(dbcgs->pValue->pszVal));
			if (isEncrypted(dbcgs->szModule, dbcgs->szSetting))
				DecodeString(dbcgs->pValue->pszVal);
			mir_free(set.Value.pWide);
		}
		break;
	case DBT_ST_BLOB:
		dbcgs->pValue->type = DBVT_BLOB;
		dbcgs->pValue->pbVal = set.Value.pBlob;
		dbcgs->pValue->cpbVal = set.Value.Length;
		break;
	case DBT_ST_BOOL:
		dbcgs->pValue->type = DBVT_BYTE;
		dbcgs->pValue->bVal = (uint8_t)set.Value.Bool;
		break;
	case DBT_ST_BYTE: case DBT_ST_CHAR:
		dbcgs->pValue->type = DBVT_BYTE;
		dbcgs->pValue->bVal = set.Value.Byte;
		break;
	case DBT_ST_SHORT: case DBT_ST_WORD:
		dbcgs->pValue->type = DBVT_WORD;
		dbcgs->pValue->wVal = set.Value.Word;
		break;
	case DBT_ST_INT: case DBT_ST_DWORD:
		dbcgs->pValue->type = DBVT_DWORD;
		dbcgs->pValue->dVal = set.Value.DWord;
		break;
	case DBT_ST_INT64: case DBT_ST_QWORD:
	case DBT_ST_DOUBLE: case DBT_ST_FLOAT:
		dbcgs->pValue->type = DBVT_BLOB;
		dbcgs->pValue->cpbVal = sizeof(set.Value);
		dbcgs->pValue->pbVal = reinterpret_cast<BYTE*>(mir_alloc(sizeof(set.Value)));
		memcpy(dbcgs->pValue->pbVal, &set.Value, sizeof(set.Value));
		break;
	default:
		return -1;
	}

	return 0;
}

STDMETHODIMP_(BOOL) CDataBase::GetContactSettingStatic(HANDLE hContact, DBCONTACTGETSETTING *dbcgs)
{
	char namebuf[512];
	namebuf[0] = 0;
	if (dbcgs->szModule)
		strcpy_s(namebuf, dbcgs->szModule);
	strcat_s(namebuf, "/");
	if (dbcgs->szSetting)
		strcat_s(namebuf, dbcgs->szSetting);

	TDBTSettingDescriptor desc = {0,0,0,0,0,0,0,0};
	TDBTSetting set = {0,0,0,0};
	desc.cbSize = sizeof(desc);
	desc.Entity = (WPARAM)hContact;
	desc.pszSettingName = namebuf;

	set.cbSize = sizeof(set);
	set.Descriptor = &desc;

	if (DBSettingRead(reinterpret_cast<WPARAM>(&set), 0) == DBT_INVALIDPARAM)
		return -1;

	if ((set.Type & DBT_STF_VariableLength) ^ (dbcgs->pValue->type & DBVTF_VARIABLELENGTH))
	{
		if (set.Type & DBT_STF_VariableLength)
			mir_free(set.Value.pBlob);
		return -1;
	}

	switch (set.Type) {
	case DBT_ST_ANSI:
		if (dbcgs->pValue->cchVal < set.Value.Length) {
			memcpy(dbcgs->pValue->pszVal, set.Value.pAnsi, dbcgs->pValue->cchVal);
			dbcgs->pValue->pszVal[dbcgs->pValue->cchVal - 1] = 0;
		}
		else memcpy(dbcgs->pValue->pszVal, set.Value.pAnsi, set.Value.Length);

		dbcgs->pValue->type = DBVT_ASCIIZ;
		dbcgs->pValue->cchVal = set.Value.Length - 1;
		if (isEncrypted(dbcgs->szModule, dbcgs->szSetting))
			DecodeString(dbcgs->pValue->pszVal);

		mir_free(set.Value.pAnsi);
		break;
	case DBT_ST_UTF8:
		set.Value.pUTF8 = mir_utf8decode(set.Value.pUTF8, NULL);
		set.Value.Length = static_cast<uint32_t>(strlen(set.Value.pUTF8));

		if (dbcgs->pValue->cchVal < set.Value.Length) {
			memcpy(dbcgs->pValue->pszVal, set.Value.pUTF8, dbcgs->pValue->cchVal);
			dbcgs->pValue->pszVal[dbcgs->pValue->cchVal - 1] = 0;
		}
		else memcpy(dbcgs->pValue->pszVal, set.Value.pUTF8, set.Value.Length);

		dbcgs->pValue->type = DBVT_ASCIIZ;
		dbcgs->pValue->cchVal = set.Value.Length - 1;
		if (isEncrypted(dbcgs->szModule, dbcgs->szSetting))
			DecodeString(dbcgs->pValue->pszVal);

		mir_free(set.Value.pUTF8);
		break;
	case DBT_ST_WCHAR:
		{
			char *tmp = mir_u2a(set.Value.pWide);
			WORD l = static_cast<WORD>(strlen(tmp));
			mir_free(set.Value.pWide);

			if (dbcgs->pValue->cchVal < l + 1) {
				memcpy(dbcgs->pValue->pszVal, tmp, dbcgs->pValue->cchVal);
				dbcgs->pValue->pszVal[l] = 0;
			}
			else memcpy(dbcgs->pValue->pszVal, tmp, l + 1);

			dbcgs->pValue->type = DBVT_ASCIIZ;
			dbcgs->pValue->cchVal = l;
			if (isEncrypted(dbcgs->szModule, dbcgs->szSetting))
				DecodeString(dbcgs->pValue->pszVal);

			mir_free(tmp);
		}
		break;
	case DBT_ST_BLOB:
		if (dbcgs->pValue->cchVal < set.Value.Length)
			memcpy(dbcgs->pValue->pbVal, set.Value.pBlob, dbcgs->pValue->cchVal);
		else
			memcpy(dbcgs->pValue->pbVal, set.Value.pBlob, set.Value.Length);

		dbcgs->pValue->type = DBVT_BLOB;
		dbcgs->pValue->cchVal = set.Value.Length;
		mir_free(set.Value.pBlob);
		break;
	case DBT_ST_BOOL:
		dbcgs->pValue->type = DBVT_BYTE;
		dbcgs->pValue->bVal = set.Value.Bool ? TRUE : FALSE;
		break;
	case DBT_ST_BYTE: case DBT_ST_CHAR:
		dbcgs->pValue->type = DBVT_BYTE;
		dbcgs->pValue->bVal = set.Value.Byte;
		break;
	case DBT_ST_SHORT: case DBT_ST_WORD:
		dbcgs->pValue->type = DBVT_WORD;
		dbcgs->pValue->wVal = set.Value.Word;
		break;
	case DBT_ST_INT: case DBT_ST_DWORD:
		dbcgs->pValue->type = DBVT_DWORD;
		dbcgs->pValue->dVal = set.Value.DWord;
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

STDMETHODIMP_(BOOL) CDataBase::WriteContactSetting(HANDLE hContact, DBCONTACTWRITESETTING *dbcws)
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
	desc.Entity = (WPARAM)hContact;
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
		NotifyEventHooks(hSettingChangeEvent, (WPARAM)hContact, (LPARAM)dbcws);
		mir_free(dbcws->value.pszVal);
		dbcws->value.type = DBVT_WCHAR;
		dbcws->value.pwszVal = tmp;		
	}
	else NotifyEventHooks(hSettingChangeEvent, (WPARAM)hContact, (LPARAM)dbcws);

	return 0;
}

STDMETHODIMP_(BOOL) CDataBase::DeleteContactSetting(HANDLE hContact, DBCONTACTGETSETTING *dbcgs)
{
	char namebuf[512];
	namebuf[0] = 0;
	if (dbcgs->szModule)
		strcpy_s(namebuf, dbcgs->szModule);
	strcat_s(namebuf, "/");
	if (dbcgs->szSetting)
		strcat_s(namebuf, dbcgs->szSetting);

	TDBTSettingDescriptor desc = {0,0,0,0,0,0,0,0};
	desc.cbSize = sizeof(desc);
	desc.Entity = (WPARAM)hContact;
	desc.pszSettingName = namebuf;

	if (DBSettingDelete(reinterpret_cast<WPARAM>(&desc), 0) == DBT_INVALIDPARAM)
		return -1;

	{
		DBCONTACTWRITESETTING tmp = {0,0,0,0};
		tmp.szModule = dbcgs->szModule;
		tmp.szSetting = dbcgs->szSetting;
		tmp.value.type = 0;
		NotifyEventHooks(hSettingChangeEvent, (WPARAM)hContact, (LPARAM)&tmp);
	}

	return 0;
}

STDMETHODIMP_(BOOL) CDataBase::EnumContactSettings(HANDLE hContact, DBCONTACTENUMSETTINGS* pces)
{
	TDBTSettingDescriptor desc = {0,0,0,0,0,0,0,0};
	desc.cbSize = sizeof(desc);
	desc.Entity = (WPARAM)hContact;

	char namebuf[512];
	namebuf[0] = 0;
	if (pces->szModule)
		strcpy_s(namebuf, pces->szModule);
	strcat_s(namebuf, "/");

	TDBTSettingIterFilter filter = {0,0,0,0,0,0,0,0};
	filter.cbSize = sizeof(filter);
	filter.Descriptor = &desc;
	filter.hEntity = (WPARAM)(WPARAM)hContact;
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

STDMETHODIMP_(LONG) CDataBase::GetEventCount(HANDLE hContact)
{
	if (hContact == 0)
		hContact = (HANDLE)getEntities().getRootEntity();

	return DBEventGetCount((WPARAM)hContact, 0);
}

STDMETHODIMP_(HANDLE) CDataBase::AddEvent(HANDLE hContact, DBEVENTINFO *dbei)
{
	if (dbei->cbSize < sizeof(DBEVENTINFO))
		return (HANDLE)-1;

	int tmp = NotifyEventHooks(hEventFilterAddedEvent, (WPARAM)hContact, (LPARAM)dbei);
	if (tmp != 0)
		return (HANDLE)tmp;

	if (hContact == 0)
		hContact = (HANDLE)getEntities().getRootEntity();


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

	int res = DBEventAdd((WPARAM)hContact, reinterpret_cast<LPARAM>(&ev));
	if (res != DBT_INVALIDPARAM)
	{
		NotifyEventHooks(hEventAddedEvent, (WPARAM)hContact, res);
		return (HANDLE)res;
	}
	return NULL;
}

STDMETHODIMP_(BOOL) CDataBase::DeleteEvent(HANDLE hContact, HANDLE hDbEvent)
{
	int res = NotifyEventHooks(hEventDeletedEvent, (WPARAM)hContact, (WPARAM)hDbEvent);

	if (hContact == 0)
		hContact = (HANDLE)getEntities().getRootEntity();

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

STDMETHODIMP_(BOOL) CDataBase::MarkEventRead(HANDLE hContact, HANDLE hDbEvent)
{
	int res = DBEventMarkRead((WPARAM)hDbEvent, 0);
	if ((res != DBT_INVALIDPARAM) && (res & DBEF_SENT))
		res = res & ~DBEF_READ;
	return res;
}

STDMETHODIMP_(HANDLE) CDataBase::GetEventContact(HANDLE hDbEvent)
{
	TDBTEntityHandle res = DBEventGetEntity((WPARAM)hDbEvent, 0);
	if (res == getEntities().getRootEntity())
		res = 0;

	return (HANDLE)res;
}

STDMETHODIMP_(HANDLE) CDataBase::FindFirstEvent(HANDLE hContact)
{
	if (hContact == 0)
		hContact = (HANDLE)getEntities().getRootEntity();

	return (HANDLE)getEvents().compFirstEvent((WPARAM)hContact);
}

STDMETHODIMP_(HANDLE) CDataBase::FindFirstUnreadEvent(HANDLE hContact)
{
	if (hContact == 0)
		hContact = (HANDLE)getEntities().getRootEntity();
	return (HANDLE)getEvents().compFirstUnreadEvent((WPARAM)hContact);
}

STDMETHODIMP_(HANDLE) CDataBase::FindLastEvent(HANDLE hContact)
{
	if (hContact == 0)
		hContact = (HANDLE)getEntities().getRootEntity();
	return (HANDLE)getEvents().compLastEvent((WPARAM)hContact);
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
