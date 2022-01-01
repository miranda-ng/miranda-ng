/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team,
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
#include "database.h"

static DBVARIANT temp;

static int stringCompare(const char *p1, const char *p2)
{
	return mir_strcmp(p1, p2);
}

static int compareGlobals(const DBCachedGlobalValue *p1, const DBCachedGlobalValue *p2)
{
	return mir_strcmp(p1->name, p2->name);
}

MDatabaseCache::MDatabaseCache(MIDatabase *_db) :
	m_db(_db),
	m_lSettings(100, stringCompare),
	m_lContacts(50, NumericKeySortT),
	m_lGlobalSettings(50, compareGlobals),
	m_lastSetting(nullptr),
	m_lastVL(nullptr)
{
}

MDatabaseCache::~MDatabaseCache()
{
	for (auto &it : m_lContacts)
		mir_free(it->pSubs);
}

/////////////////////////////////////////////////////////////////////////////////////////

DBCachedContact* MDatabaseCache::AddContactToCache(MCONTACT contactID)
{
	mir_cslock lck(m_csContact);

	int index = m_lContacts.getIndex((DBCachedContact*)&contactID);
	if (index != -1)
		return m_lContacts[index];

	DBCachedContact *cc = (DBCachedContact*)mir_calloc(m_db->GetContactSize());
	cc->contactID = contactID;
	cc->nSubs = -1;
	m_lContacts.insert(cc);
	return cc;
}

DBCachedContact* MDatabaseCache::GetCachedContact(MCONTACT contactID)
{
	mir_cslock lck(m_csContact);

	int index = m_lContacts.getIndex((DBCachedContact*)&contactID);
	return (index == -1) ? nullptr : m_lContacts[index];
}

DBCachedContact* MDatabaseCache::GetFirstContact()
{
	mir_cslock lck(m_csContact);
	return m_lContacts[0];
}

DBCachedContact* MDatabaseCache::GetNextContact(MCONTACT contactID)
{
	mir_cslock lck(m_csContact);

	int index = m_lContacts.getIndex((DBCachedContact*)&contactID);
	return (index == -1) ? nullptr : m_lContacts[index+1];
}

void MDatabaseCache::FreeCachedContact(MCONTACT contactID)
{
	mir_cslock lck(m_csContact);

	int index = m_lContacts.getIndex((DBCachedContact*)&contactID);
	if (index == -1)
		return;

	DBCachedContact *cc = m_lContacts[index];
	DBCachedContactValue* V = cc->first;
	while (V != nullptr) {
		DBCachedContactValue* V1 = V->next;
		FreeCachedVariant(&V->value);
		mir_free(V);
		V = V1;
	}

	mir_free(cc->pSubs);
	mir_free(cc);

	m_lContacts.remove(index);
}

/////////////////////////////////////////////////////////////////////////////////////////

char* MDatabaseCache::InsertCachedSetting(const char* szName, size_t cbLen)
{
	char* newValue = (char*)mir_alloc(cbLen);
	*newValue++ = 0;
	mir_strcpy(newValue, szName);
	{
		mir_cslock lck(m_csVal);
		m_lSettings.insert(newValue);
	}
	return newValue;
}

char* MDatabaseCache::GetCachedSetting(const char *szModuleName, const char *szSettingName, size_t moduleNameLen, size_t settingNameLen)
{
	char szFullName[512];
	const char *szKey;
	if (szModuleName != nullptr) {
		mir_strcpy(szFullName, szModuleName);
		szFullName[moduleNameLen] = '/';
		mir_strcpy(szFullName + moduleNameLen + 1, szSettingName);
		szKey = szFullName;
	}
	else szKey = szSettingName;

	if (m_lastSetting && !mir_strcmp(szKey, m_lastSetting))
		return m_lastSetting;

	mir_cslock lck(m_csVal);
	int index = m_lSettings.getIndex((char*)szKey);
	if (index != -1)
		m_lastSetting = m_lSettings[index];
	else
		m_lastSetting = InsertCachedSetting(szKey, settingNameLen + moduleNameLen + 3);

	return m_lastSetting;
}

void MDatabaseCache::SetCachedVariant(DBVARIANT* s /* new */, DBVARIANT* d /* cached */)
{
	void *szSave = nullptr;
	if (d->type & DBVTF_VARIABLELENGTH)
		szSave = (d->type == DBVT_UTF8 || d->type == DBVT_ASCIIZ) ? (void*)d->pszVal : d->pbVal;

	memcpy(d, s, sizeof(DBVARIANT));

	if (s->type & DBVTF_VARIABLELENGTH) {
		// string variable length value
		if (s->type == DBVT_UTF8 || s->type == DBVT_ASCIIZ) {
			if (s->pszVal == nullptr) {
				mir_free(szSave);
				return;
			}

			if (szSave != nullptr)
				d->pszVal = (char*)mir_realloc(szSave, mir_strlen(s->pszVal) + 1);
			else
				d->pszVal = (char*)mir_alloc(mir_strlen(s->pszVal) + 1);
			strcpy(d->pszVal, s->pszVal);
		}
		// binary variable length value
		else {
			if (s->pbVal == nullptr) {
				mir_free(szSave);
				return;
			}

			if (szSave != nullptr)
				d->pbVal = (uint8_t*)mir_realloc(szSave, s->cpbVal);
			else
				d->pbVal = (uint8_t*)mir_alloc(s->cpbVal);
			memcpy(d->pbVal, s->pbVal, s->cpbVal);
		}
	}
	else if (szSave != nullptr)
		mir_free(szSave);
}

void MDatabaseCache::FreeCachedVariant(DBVARIANT* V)
{
	if ((V->type == DBVT_ASCIIZ || V->type == DBVT_UTF8) && V->pszVal != nullptr)
		mir_free(V->pszVal);
}

STDMETHODIMP_(DBVARIANT*) MDatabaseCache::GetCachedValuePtr(MCONTACT contactID, char *szSetting, int bAllocate)
{
	// a global setting
	if (contactID == 0) {
		DBCachedGlobalValue Vtemp, *V;
		Vtemp.name = szSetting;
		int index = m_lGlobalSettings.getIndex(&Vtemp);
		if (index != -1) {
			V = m_lGlobalSettings[index];
			if (bAllocate == -1) {
				FreeCachedVariant(&V->value);
				m_lGlobalSettings.remove(index);
				mir_free(V);
				return &temp; // not null - smth were deleted
			}
		}
		else {
			if (bAllocate != 1)
				return nullptr;

			V = (DBCachedGlobalValue*)mir_calloc(sizeof(DBCachedGlobalValue));
			V->name = szSetting;
			m_lGlobalSettings.insert(V);
		}

		return &V->value;
	}

	// a contact setting
	DBCachedContactValue *V, *V1;
	DBCachedContact ccTemp, *cc;
	{
		mir_cslock lck(m_csContact);
		ccTemp.contactID = contactID;

		int index = m_lContacts.getIndex(&ccTemp);
		if (index == -1)
			return nullptr;

		m_lastVL = cc = m_lContacts[index];
	}

	for (V = cc->first; V != nullptr; V = V->next)
		if (V->name == szSetting)
			break;

	if (V == nullptr) {
		if (bAllocate != 1)
			return nullptr;

		V = (DBCachedContactValue *)mir_calloc(sizeof(DBCachedContactValue));
		if (cc->last)
			cc->last->next = V;
		else
			cc->first = V;
		cc->last = V;
		V->name = szSetting;
	}
	else if (bAllocate == -1) {
		m_lastVL = nullptr;
		FreeCachedVariant(&V->value);
		if (cc->first == V) {
			cc->first = V->next;
			if (cc->last == V)
				cc->last = V->next; // nullptr
		}
		else
			for (V1 = cc->first; V1 != nullptr; V1 = V1->next)
				if (V1->next == V) {
					V1->next = V->next;
					if (cc->last == V)
						cc->last = V1;
					break;
				}
		mir_free(V);
		return &temp; // not null - smth were deleted
	}

	return &V->value;
}
