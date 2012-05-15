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

#include "Compatibility.h"
#include "Logger.h"
#define DB_NOHELPERFUNCTIONS
	#include "m_database.h"
#undef DB_NOHELPERFUNCTIONS
#ifndef _MSC_VER
#include "savestrings_gcc.h"
#endif

HANDLE gCompServices[31] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
HANDLE gEvents[6] = {0,0,0,0,0,0};

HANDLE hEventDeletedEvent,
       hEventAddedEvent,
			 hEventFilterAddedEvent,
			 hSettingChangeEvent,
			 hContactDeletedEvent,
			 hContactAddedEvent;

INT_PTR CompAddContact(WPARAM wParam, LPARAM lParam)
{
	TDBTEntity entity = {0,0,0,0};
	entity.hParentEntity = DBEntityGetRoot(0, 0);
	entity.hAccountEntity = entity.hParentEntity;

	TDBTEntityHandle res = gDataBase->getEntities().CreateEntity(entity);
	if (res == DBT_INVALIDPARAM)
		return 1;

	NotifyEventHooks(hContactAddedEvent, res, 0);
	return res;
}
INT_PTR CompDeleteContact(WPARAM hContact, LPARAM lParam)
{
	NotifyEventHooks(hContactDeletedEvent, hContact, 0);

	int res = DBEntityDelete(hContact, 0);
	if (res == DBT_INVALIDPARAM)
		return 1;

	return res;
}
INT_PTR CompIsDbContact(WPARAM hContact, LPARAM lParam)
{
	int flags = DBEntityGetFlags(hContact, 0);
	return (flags != DBT_INVALIDPARAM) &&
		     ((flags & DBT_NFM_SpecialEntity) == 0);
}
INT_PTR CompGetContactCount(WPARAM wParam, LPARAM lParam)
{
	TDBTEntityIterFilter f = {0,0,0,0};
	f.cbSize = sizeof(f);
	f.fDontHasFlags = DBT_NF_IsGroup | DBT_NF_IsVirtual | DBT_NF_IsAccount | DBT_NF_IsRoot;
	f.Options = DBT_NIFO_OSC_AC | DBT_NIFO_OC_AC;

	TDBTEntityIterationHandle hiter = DBEntityIterInit((WPARAM)&f, gDataBase->getEntities().getRootEntity());
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
INT_PTR CompFindFirstContact(WPARAM wParam, LPARAM lParam)
{
	return gDataBase->getEntities().compFirstContact();
}
INT_PTR CompFindNextContact(WPARAM hContact, LPARAM lParam)
{
	return gDataBase->getEntities().compNextContact(hContact);
}

INT_PTR CompGetContactSetting(WPARAM hContact, LPARAM pSetting)
{
	DBCONTACTGETSETTING * dbcgs = reinterpret_cast<DBCONTACTGETSETTING *>(pSetting);
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
	desc.Entity = hContact;
	desc.pszSettingName = namebuf;

	set.cbSize = sizeof(set);
	set.Descriptor = &desc;

	if (DBSettingRead(reinterpret_cast<WPARAM>(&set), 0) == DBT_INVALIDPARAM)
		return -1;

	switch (set.Type)
	{
		case DBT_ST_ANSI:
		{
			dbcgs->pValue->type = DBVT_ASCIIZ;
			dbcgs->pValue->pszVal = set.Value.pAnsi;
			dbcgs->pValue->cchVal = set.Value.Length - 1;
		} break;
		case DBT_ST_UTF8:
		{
			dbcgs->pValue->type = DBVT_WCHAR;
			dbcgs->pValue->pwszVal = mir_utf8decodeW(set.Value.pUTF8);
			if (dbcgs->pValue->pwszVal)
				dbcgs->pValue->cchVal = static_cast<uint32_t>(wcslen(dbcgs->pValue->pwszVal));
			else
				dbcgs->pValue->cchVal = 0;
			mir_free(set.Value.pUTF8);
		} break;
		case DBT_ST_WCHAR:
		{
			dbcgs->pValue->type = DBVT_WCHAR;
			dbcgs->pValue->pwszVal = set.Value.pWide;
			dbcgs->pValue->cchVal = set.Value.Length - 1;
		} break;
		case DBT_ST_BLOB:
		{
			dbcgs->pValue->type = DBVT_BLOB;
			dbcgs->pValue->pbVal = set.Value.pBlob;
			dbcgs->pValue->cpbVal = set.Value.Length;
		} break;
		case DBT_ST_BOOL:
		{
			dbcgs->pValue->type = DBVT_BYTE;
			dbcgs->pValue->bVal = (uint8_t)set.Value.Bool;
		} break;
		case DBT_ST_BYTE: case DBT_ST_CHAR:
		{
			dbcgs->pValue->type = DBVT_BYTE;
			dbcgs->pValue->bVal = set.Value.Byte;
		} break;
		case DBT_ST_SHORT: case DBT_ST_WORD:
		{
			dbcgs->pValue->type = DBVT_WORD;
			dbcgs->pValue->wVal = set.Value.Word;
		} break;
		case DBT_ST_INT: case DBT_ST_DWORD:
		{
			dbcgs->pValue->type = DBVT_DWORD;
			dbcgs->pValue->dVal = set.Value.DWord;
		} break;
		case DBT_ST_INT64: case DBT_ST_QWORD:
		case DBT_ST_DOUBLE: case DBT_ST_FLOAT:
		{
			dbcgs->pValue->type = DBVT_BLOB;
			dbcgs->pValue->cpbVal = sizeof(set.Value);
			dbcgs->pValue->pbVal = reinterpret_cast<BYTE*>(mir_alloc(sizeof(set.Value)));
			memcpy(dbcgs->pValue->pbVal, &set.Value, sizeof(set.Value));
		} break;
		default:
		{
			return -1;
		}
	}

	return 0;
}
INT_PTR CompGetContactSettingStr(WPARAM hContact, LPARAM pSetting)
{
	DBCONTACTGETSETTING * dbcgs = reinterpret_cast<DBCONTACTGETSETTING *>(pSetting);

	if ((dbcgs->pValue->type & DBVTF_VARIABLELENGTH) == 0)
	{
		CompFreeVariant(0, reinterpret_cast<LPARAM>(dbcgs->pValue));
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
	desc.Entity = hContact;
	desc.pszSettingName = namebuf;

	set.cbSize = sizeof(set);
	set.Descriptor = &desc;


	switch (dbcgs->pValue->type)
	{
		case DBVT_ASCIIZ: set.Type = DBT_ST_ANSI; break;
		case DBVT_BLOB:   set.Type = DBT_ST_BLOB; break;
		case DBVT_UTF8:   set.Type = DBT_ST_UTF8; break;
		case DBVT_WCHAR:  set.Type = DBT_ST_WCHAR; break;
	}

	if (DBSettingRead(reinterpret_cast<WPARAM>(&set), 0) == DBT_INVALIDPARAM)
		return -1;

	switch (set.Type)
	{
		case DBT_ST_ANSI:
		{
			dbcgs->pValue->type = DBVT_ASCIIZ;
			dbcgs->pValue->pszVal = set.Value.pAnsi;
			dbcgs->pValue->cchVal = set.Value.Length - 1;
		} break;
		case DBT_ST_UTF8:
		{
			dbcgs->pValue->type = DBVT_UTF8;
			dbcgs->pValue->pszVal = set.Value.pUTF8;
			dbcgs->pValue->cchVal = set.Value.Length - 1;
		} break;
		case DBT_ST_WCHAR:
		{
			if (dbcgs->pValue->type == DBVT_WCHAR)
			{
				dbcgs->pValue->pwszVal = set.Value.pWide;
				dbcgs->pValue->cchVal = set.Value.Length - 1;
			} else {
				dbcgs->pValue->type = DBVT_UTF8;
				dbcgs->pValue->pszVal = mir_utf8encodeW(set.Value.pWide);
				dbcgs->pValue->cchVal = static_cast<uint32_t>(strlen(dbcgs->pValue->pszVal));
				mir_free(set.Value.pWide);
			}
		} break;
		case DBT_ST_BLOB:
		{
			dbcgs->pValue->type = DBVT_BLOB;
			dbcgs->pValue->pbVal = set.Value.pBlob;
			dbcgs->pValue->cpbVal = set.Value.Length;
		} break;
		case DBT_ST_BOOL:
		{
			dbcgs->pValue->type = DBVT_BYTE;
			dbcgs->pValue->bVal = (uint8_t)set.Value.Bool;
		} break;
		case DBT_ST_BYTE: case DBT_ST_CHAR:
		{
			dbcgs->pValue->type = DBVT_BYTE;
			dbcgs->pValue->bVal = set.Value.Byte;
		} break;
		case DBT_ST_SHORT: case DBT_ST_WORD:
		{
			dbcgs->pValue->type = DBVT_WORD;
			dbcgs->pValue->wVal = set.Value.Word;
		} break;
		case DBT_ST_INT: case DBT_ST_DWORD:
		{
			dbcgs->pValue->type = DBVT_DWORD;
			dbcgs->pValue->dVal = set.Value.DWord;
		} break;
		case DBT_ST_INT64: case DBT_ST_QWORD:
		case DBT_ST_DOUBLE: case DBT_ST_FLOAT:
		{
			dbcgs->pValue->type = DBVT_BLOB;
			dbcgs->pValue->cpbVal = sizeof(set.Value);
			dbcgs->pValue->pbVal = reinterpret_cast<BYTE*>(mir_alloc(sizeof(set.Value)));
			memcpy(dbcgs->pValue->pbVal, &set.Value, sizeof(set.Value));
		} break;
		default:
		{
			return -1;
		}
	}

	return 0;
}
INT_PTR CompGetContactSettingStatic(WPARAM hContact, LPARAM pSetting)
{
	DBCONTACTGETSETTING * dbcgs = reinterpret_cast<DBCONTACTGETSETTING *>(pSetting);

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
	desc.Entity = hContact;
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

	switch (set.Type)
	{
		case DBT_ST_ANSI:
		{
			if (dbcgs->pValue->cchVal < set.Value.Length)
			{
				memcpy(dbcgs->pValue->pszVal, set.Value.pAnsi, dbcgs->pValue->cchVal);
				dbcgs->pValue->pszVal[dbcgs->pValue->cchVal - 1] = 0;
			} else {
				memcpy(dbcgs->pValue->pszVal, set.Value.pAnsi, set.Value.Length);
			}
			dbcgs->pValue->type = DBVT_ASCIIZ;
			dbcgs->pValue->cchVal = set.Value.Length - 1;

			mir_free(set.Value.pAnsi);
		} break;
		case DBT_ST_UTF8:
		{
			set.Value.pUTF8 = mir_utf8decode(set.Value.pUTF8, NULL);
			set.Value.Length = static_cast<uint32_t>(strlen(set.Value.pUTF8));

			if (dbcgs->pValue->cchVal < set.Value.Length)
			{
				memcpy(dbcgs->pValue->pszVal, set.Value.pUTF8, dbcgs->pValue->cchVal);
				dbcgs->pValue->pszVal[dbcgs->pValue->cchVal - 1] = 0;
			} else {
				memcpy(dbcgs->pValue->pszVal, set.Value.pUTF8, set.Value.Length);
			}
			dbcgs->pValue->type = DBVT_ASCIIZ;
			dbcgs->pValue->cchVal = set.Value.Length - 1;

			mir_free(set.Value.pUTF8);
		} break;
		case DBT_ST_WCHAR:
		{
			char * tmp = mir_u2a(set.Value.pWide);
			WORD l = static_cast<WORD>(strlen(tmp));
			mir_free(set.Value.pWide);

			if (dbcgs->pValue->cchVal < l + 1)
			{
				memcpy(dbcgs->pValue->pszVal, tmp, dbcgs->pValue->cchVal);
				dbcgs->pValue->pszVal[l] = 0;
			} else {
				memcpy(dbcgs->pValue->pszVal, tmp, l + 1);
			}
			dbcgs->pValue->type = DBVT_ASCIIZ;
			dbcgs->pValue->cchVal = l;

			mir_free(tmp);
		} break;
		case DBT_ST_BLOB:
		{
			if (dbcgs->pValue->cchVal < set.Value.Length)
			{
				memcpy(dbcgs->pValue->pbVal, set.Value.pBlob, dbcgs->pValue->cchVal);
			} else {
				memcpy(dbcgs->pValue->pbVal, set.Value.pBlob, set.Value.Length);
			}
			dbcgs->pValue->type = DBVT_BLOB;
			dbcgs->pValue->cchVal = set.Value.Length;

			mir_free(set.Value.pBlob);
		} break;
		case DBT_ST_BOOL:
		{
			dbcgs->pValue->type = DBVT_BYTE;
			dbcgs->pValue->bVal = set.Value.Bool ? TRUE : FALSE;
		} break;
		case DBT_ST_BYTE: case DBT_ST_CHAR:
		{
			dbcgs->pValue->type = DBVT_BYTE;
			dbcgs->pValue->bVal = set.Value.Byte;
		} break;
		case DBT_ST_SHORT: case DBT_ST_WORD:
		{
			dbcgs->pValue->type = DBVT_WORD;
			dbcgs->pValue->wVal = set.Value.Word;
		} break;
		case DBT_ST_INT: case DBT_ST_DWORD:
		{
			dbcgs->pValue->type = DBVT_DWORD;
			dbcgs->pValue->dVal = set.Value.DWord;
		} break;
		default:
		{
			return -1;
		}
	}

	return 0;
}
INT_PTR CompFreeVariant(WPARAM wParam, LPARAM pSetting)
{
	DBVARIANT * dbv = reinterpret_cast<DBVARIANT *>(pSetting);

	if ((dbv->type == DBVT_BLOB) && (dbv->pbVal))
	{
		mir_free(dbv->pbVal);
		dbv->pbVal = 0;
	} else if ((dbv->type & DBVTF_VARIABLELENGTH) && (dbv->pszVal))
	{
		mir_free(dbv->pszVal);
		dbv->pszVal = NULL;
	}
	dbv->type = 0;
	return 0;
}
INT_PTR CompWriteContactSetting(WPARAM hContact, LPARAM pSetting)
{
	DBCONTACTWRITESETTING * dbcws = reinterpret_cast<DBCONTACTWRITESETTING *>(pSetting);

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
	desc.Entity = hContact;
	desc.pszSettingName = namebuf;

	set.cbSize = sizeof(set);
	set.Descriptor = &desc;

	switch (dbcws->value.type)
	{
		case DBVT_ASCIIZ:
		{
			set.Type = DBT_ST_ANSI;
			set.Value.pAnsi = dbcws->value.pszVal;
		} break;
		case DBVT_UTF8:
		{
			wchar_t * tmp = mir_utf8decodeW(dbcws->value.pszVal);
			if (tmp == 0)
			{
				if (IsDebuggerPresent())
				{
					DebugBreak();
#ifdef _DEBUG
				} else {
					LOG(logWARNING, _T("Trying to write malformed UTF8 setting \"%hs\" in module \"%hs\""), dbcws->szSetting, dbcws->szModule);
					CLogger::Instance().ShowMessage();
#endif
				}
				return -1;
			} else {
				mir_free(tmp);
			}

			set.Type = DBT_ST_UTF8;
			set.Value.pUTF8 = dbcws->value.pszVal;
		} break;
		case DBVT_WCHAR:
		{
			set.Type = DBT_ST_WCHAR;
			set.Value.pWide = dbcws->value.pwszVal;
		} break;
		case DBVT_BLOB:
		{
			set.Type = DBT_ST_BLOB;
			set.Value.pBlob = dbcws->value.pbVal;
			set.Value.Length = dbcws->value.cpbVal;
		} break;
		case DBVT_BYTE:
		{
			set.Type = DBT_ST_BYTE;
			set.Value.Byte = dbcws->value.bVal;
		} break;
		case DBVT_WORD:
		{
			set.Type = DBT_ST_WORD;
			set.Value.Word = dbcws->value.wVal;
		} break;
		case DBVT_DWORD:
		{
			set.Type = DBT_ST_DWORD;
			set.Value.DWord = dbcws->value.dVal;
		} break;
		default:
		{
			return -1;
		}
	}

	if (DBSettingWrite(reinterpret_cast<WPARAM>(&set), 0) == DBT_INVALIDPARAM)
		return -1;

	if (dbcws->value.type == DBVT_WCHAR)
	{
		dbcws->value.type = DBVT_UTF8;
		wchar_t * tmp = dbcws->value.pwszVal;
		dbcws->value.pszVal = mir_utf8encodeW(dbcws->value.pwszVal);
		NotifyEventHooks(hSettingChangeEvent, hContact, pSetting);
		mir_free(dbcws->value.pszVal);
		dbcws->value.type = DBVT_WCHAR;
		dbcws->value.pwszVal = tmp;		
	} else {
		NotifyEventHooks(hSettingChangeEvent, hContact, pSetting);
	}

	return 0;
}

INT_PTR CompDeleteContactSetting(WPARAM hContact, LPARAM pSetting)
{
	DBCONTACTGETSETTING * dbcgs = reinterpret_cast<DBCONTACTGETSETTING *>(pSetting);

	char namebuf[512];
	namebuf[0] = 0;
	if (dbcgs->szModule)
		strcpy_s(namebuf, dbcgs->szModule);
	strcat_s(namebuf, "/");
	if (dbcgs->szSetting)
		strcat_s(namebuf, dbcgs->szSetting);

	TDBTSettingDescriptor desc = {0,0,0,0,0,0,0,0};
	desc.cbSize = sizeof(desc);
	desc.Entity = hContact;
	desc.pszSettingName = namebuf;

	if (DBSettingDelete(reinterpret_cast<WPARAM>(&desc), 0) == DBT_INVALIDPARAM)
		return -1;

	{
		DBCONTACTWRITESETTING tmp = {0,0,0,0};
		tmp.szModule = dbcgs->szModule;
		tmp.szSetting = dbcgs->szSetting;
		tmp.value.type = 0;
		NotifyEventHooks(hSettingChangeEvent, hContact, reinterpret_cast<LPARAM>(&tmp));
	}

	return 0;
}
INT_PTR CompEnumContactSettings(WPARAM hContact, LPARAM pEnum)
{
	DBCONTACTENUMSETTINGS * pces = reinterpret_cast<DBCONTACTENUMSETTINGS *>(pEnum);

	TDBTSettingDescriptor desc = {0,0,0,0,0,0,0,0};
	desc.cbSize = sizeof(desc);
	desc.Entity = hContact;

	char namebuf[512];
	namebuf[0] = 0;
	if (pces->szModule)
		strcpy_s(namebuf, pces->szModule);
	strcat_s(namebuf, "/");

	TDBTSettingIterFilter filter = {0,0,0,0,0,0,0,0};
	filter.cbSize = sizeof(filter);
	filter.Descriptor = &desc;
	filter.hEntity = hContact;
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


INT_PTR CompGetEventCount(WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0)
		hContact = gDataBase->getEntities().getRootEntity();

	return DBEventGetCount(hContact, 0);
}
INT_PTR CompAddEvent(WPARAM hContact, LPARAM pEventInfo)
{
	DBEVENTINFO * dbei = reinterpret_cast<DBEVENTINFO*>(pEventInfo);
	if (dbei->cbSize < sizeof(DBEVENTINFO))
		return -1;

	int tmp = NotifyEventHooks(hEventFilterAddedEvent, hContact, pEventInfo);
	if (tmp != 0)
		return tmp;

	if (hContact == 0)
		hContact = gDataBase->getEntities().getRootEntity();


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

	int res = DBEventAdd(hContact, reinterpret_cast<LPARAM>(&ev));
	if (res != DBT_INVALIDPARAM)
	{
		NotifyEventHooks(hEventAddedEvent, hContact, res);
		return res;
	}
	return 0;
}
INT_PTR CompDeleteEvent(WPARAM hContact, LPARAM hEvent)
{
	int res = NotifyEventHooks(hEventDeletedEvent, hContact, hEvent);

	if (hContact == 0)
		hContact = gDataBase->getEntities().getRootEntity();

	if (res == 0)
		return DBEventDelete(hEvent, 0);

	return res;
}
INT_PTR CompGetBlobSize(WPARAM hEvent, LPARAM lParam)
{
	int res = DBEventGetBlobSize(hEvent, 0);
	if (res == DBT_INVALIDPARAM)
		return -1;

	return res;
}
INT_PTR CompGetEvent(WPARAM hEvent, LPARAM pEventInfo)
{
	DBEVENTINFO * dbei = reinterpret_cast<DBEVENTINFO*>(pEventInfo);
	if (dbei->cbSize < sizeof(DBEVENTINFO))
		return -1;

	TDBTEvent ev = {0,0,0,0,0,0,0};
	ev.cbSize = sizeof(ev);
	ev.cbBlob = 0;
	ev.pBlob = NULL;

	int res = DBEventGet(hEvent, reinterpret_cast<LPARAM>(&ev));

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
INT_PTR CompMarkEventRead(WPARAM hContact, LPARAM hEvent)
{
	int res = DBEventMarkRead(hEvent, 0);
	if ((res != DBT_INVALIDPARAM) && (res & DBEF_SENT))
		res = res & ~DBEF_READ;
	return res;
}
INT_PTR CompGetEventContact(WPARAM hEvent, LPARAM lParam)
{
	TDBTEntityHandle res = DBEventGetEntity(hEvent, 0);
	if (res == gDataBase->getEntities().getRootEntity())
		res = 0;

	return res;
}
INT_PTR CompFindFirstEvent(WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0)
		hContact = gDataBase->getEntities().getRootEntity();

	return gDataBase->getEvents().compFirstEvent(hContact);
}
INT_PTR CompFindFirstUnreadEvent(WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0)
		hContact = gDataBase->getEntities().getRootEntity();
	return gDataBase->getEvents().compFirstUnreadEvent(hContact);
}
INT_PTR CompFindLastEvent(WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0)
		hContact = gDataBase->getEntities().getRootEntity();
	return gDataBase->getEvents().compLastEvent(hContact);
}
INT_PTR CompFindNextEvent(WPARAM hEvent, LPARAM lParam)
{
	return gDataBase->getEvents().compNextEvent(hEvent);
}
INT_PTR CompFindPrevEvent(WPARAM hEvent, LPARAM lParam)
{
	return gDataBase->getEvents().compPrevEvent(hEvent);
}

INT_PTR CompEnumModules(WPARAM wParam, LPARAM pCallback)
{
	if (!pCallback)
		return -1;
	
	return gDataBase->getSettings().CompEnumModules(reinterpret_cast<DBMODULEENUMPROC>(pCallback), wParam);
}

void Encrypt(char* msg, BOOL up)
{
	int i;
	const int jump = up ? 5 : -5;
	
	for (i=0; msg[i]; i++)
	{
		msg[i] = msg[i] + jump;
	}

}

INT_PTR CompEncodeString(WPARAM wParam, LPARAM lParam)
{
	Encrypt(reinterpret_cast<char*>(lParam),TRUE);
	return 0;
}

INT_PTR CompDecodeString(WPARAM wParam, LPARAM lParam)
{
	Encrypt(reinterpret_cast<char*>(lParam),FALSE);
	return 0;
}

INT_PTR CompGetProfileName(WPARAM cbBytes, LPARAM pszName)
{
	return gDataBase->getProfileName(cbBytes, reinterpret_cast<char*>(pszName));
}

INT_PTR CompGetProfilePath(WPARAM cbBytes, LPARAM pszName)
{
	return gDataBase->getProfilePath(cbBytes, reinterpret_cast<char*>(pszName));
}

bool CompatibilityRegister()
{
	gCompServices[ 0] = CreateServiceFunction(MS_DB_CONTACT_GETCOUNT,         CompGetContactCount);
	gCompServices[ 1] = CreateServiceFunction(MS_DB_CONTACT_FINDFIRST,        CompFindFirstContact);
	gCompServices[ 2] = CreateServiceFunction(MS_DB_CONTACT_FINDNEXT,         CompFindNextContact);
	gCompServices[ 3] = CreateServiceFunction(MS_DB_CONTACT_DELETE,           CompDeleteContact);
	gCompServices[ 4] = CreateServiceFunction(MS_DB_CONTACT_ADD,              CompAddContact);
	gCompServices[ 5] = CreateServiceFunction(MS_DB_CONTACT_IS,	              CompIsDbContact);

	gCompServices[ 6] = CreateServiceFunction(MS_DB_CONTACT_GETSETTING,       CompGetContactSetting);
	gCompServices[ 7] = CreateServiceFunction(MS_DB_CONTACT_GETSETTING_STR,   CompGetContactSettingStr);
	gCompServices[ 8] = CreateServiceFunction(MS_DB_CONTACT_GETSETTINGSTATIC, CompGetContactSettingStatic);
	gCompServices[ 9] = CreateServiceFunction(MS_DB_CONTACT_FREEVARIANT,      CompFreeVariant);
	gCompServices[10] = CreateServiceFunction(MS_DB_CONTACT_WRITESETTING,     CompWriteContactSetting);
	gCompServices[11] = CreateServiceFunction(MS_DB_CONTACT_DELETESETTING,    CompDeleteContactSetting);
	gCompServices[12] = CreateServiceFunction(MS_DB_CONTACT_ENUMSETTINGS,     CompEnumContactSettings);
	//gCompServices[13] = CreateServiceFunction(MS_DB_SETSETTINGRESIDENT,       CompSetSettingResident);

	gCompServices[14] = CreateServiceFunction(MS_DB_EVENT_GETCOUNT,           CompGetEventCount);
	gCompServices[15] = CreateServiceFunction(MS_DB_EVENT_ADD,                CompAddEvent);
	gCompServices[16] = CreateServiceFunction(MS_DB_EVENT_DELETE,             CompDeleteEvent);
	gCompServices[17] = CreateServiceFunction(MS_DB_EVENT_GETBLOBSIZE,        CompGetBlobSize);
	gCompServices[18] = CreateServiceFunction(MS_DB_EVENT_GET,                CompGetEvent);
	gCompServices[19] = CreateServiceFunction(MS_DB_EVENT_MARKREAD,           CompMarkEventRead);
	gCompServices[20] = CreateServiceFunction(MS_DB_EVENT_GETCONTACT,         CompGetEventContact);
	gCompServices[21] = CreateServiceFunction(MS_DB_EVENT_FINDFIRST,          CompFindFirstEvent);
	gCompServices[22] = CreateServiceFunction(MS_DB_EVENT_FINDFIRSTUNREAD,    CompFindFirstUnreadEvent);
	gCompServices[23] = CreateServiceFunction(MS_DB_EVENT_FINDLAST,           CompFindLastEvent);
	gCompServices[24] = CreateServiceFunction(MS_DB_EVENT_FINDNEXT,           CompFindNextEvent);
	gCompServices[25] = CreateServiceFunction(MS_DB_EVENT_FINDPREV,           CompFindPrevEvent);

	gCompServices[26] = CreateServiceFunction(MS_DB_MODULES_ENUM,             CompEnumModules);

	gCompServices[27] = CreateServiceFunction(MS_DB_CRYPT_ENCODESTRING,       CompEncodeString);
	gCompServices[28] = CreateServiceFunction(MS_DB_CRYPT_DECODESTRING,       CompDecodeString);

	gCompServices[29] = CreateServiceFunction(MS_DB_GETPROFILENAME,           CompGetProfileName);
	gCompServices[30] = CreateServiceFunction(MS_DB_GETPROFILEPATH,           CompGetProfilePath);


	hEventDeletedEvent     = CreateHookableEvent(ME_DB_EVENT_DELETED);
	hEventAddedEvent       = CreateHookableEvent(ME_DB_EVENT_ADDED);
	hEventFilterAddedEvent = CreateHookableEvent(ME_DB_EVENT_FILTER_ADD);
	hSettingChangeEvent    = CreateHookableEvent(ME_DB_CONTACT_SETTINGCHANGED);
	hContactDeletedEvent   = CreateHookableEvent(ME_DB_CONTACT_DELETED);
	hContactAddedEvent     = CreateHookableEvent(ME_DB_CONTACT_ADDED);
	return true;
}
bool CompatibilityUnRegister()
{
	int i;
	for (i = 0; i < sizeof(gCompServices) / sizeof(gCompServices[0]); ++i)
	{
		DestroyServiceFunction(gCompServices[i]);
	}
	return true;
}
