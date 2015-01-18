/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-15 Miranda NG project,
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

#include "..\..\core\commonheaders.h"
#include "database.h"

static int stringCompare(const char *p1, const char *p2)
{
	return strcmp(p1, p2);
}

static int compareGlobals(const DBCachedGlobalValue *p1, const DBCachedGlobalValue *p2)
{
	return strcmp(p1->name, p2->name);
}

MDatabaseCache::MDatabaseCache(size_t _size) :
	m_contactSize(_size),
	m_lSettings(100, stringCompare),
	m_lContacts(50, NumericKeySortT),
	m_lGlobalSettings(50, compareGlobals)
{
	m_hCacheHeap = HeapCreate(0, 0, 0);
}

MDatabaseCache::~MDatabaseCache()
{
	for (int i = 0; i < m_lContacts.getCount(); i++)
		mir_free(m_lContacts[i]->pSubs);

	HeapDestroy(m_hCacheHeap);
}

/////////////////////////////////////////////////////////////////////////////////////////

DBCachedContact* MDatabaseCache::AddContactToCache(MCONTACT contactID)
{
	mir_cslock lck(m_cs);

	int index = m_lContacts.getIndex((DBCachedContact*)&contactID);
	if (index != -1)
		return m_lContacts[index];

	DBCachedContact *cc = (DBCachedContact*)HeapAlloc(m_hCacheHeap, HEAP_ZERO_MEMORY, m_contactSize);
	cc->contactID = contactID;
	cc->nSubs = -1;
	m_lContacts.insert(cc);
	return cc;
}

DBCachedContact* MDatabaseCache::GetCachedContact(MCONTACT contactID)
{
	mir_cslock lck(m_cs);

	int index = m_lContacts.getIndex((DBCachedContact*)&contactID);
	return (index == -1) ? NULL : m_lContacts[index];
}

DBCachedContact* MDatabaseCache::GetFirstContact()
{
	mir_cslock lck(m_cs);
	return m_lContacts[0];
}

DBCachedContact* MDatabaseCache::GetNextContact(MCONTACT contactID)
{
	mir_cslock lck(m_cs);

	int index = m_lContacts.getIndex((DBCachedContact*)&contactID);
	return (index == -1) ? NULL : m_lContacts[index+1];
}

void MDatabaseCache::FreeCachedContact(MCONTACT contactID)
{
	mir_cslock lck(m_cs);

	int index = m_lContacts.getIndex((DBCachedContact*)&contactID);
	if (index == -1)
		return;

	DBCachedContact *cc = m_lContacts[index];
	DBCachedContactValue* V = cc->first;
	while (V != NULL) {
		DBCachedContactValue* V1 = V->next;
		FreeCachedVariant(&V->value);
		HeapFree(m_hCacheHeap, 0, V);
		V = V1;
	}

	mir_free(cc->pSubs);
	HeapFree(m_hCacheHeap, 0, cc);

	m_lContacts.remove(index);
}

/////////////////////////////////////////////////////////////////////////////////////////

char* MDatabaseCache::InsertCachedSetting(const char* szName, int cbLen)
{
	char* newValue = (char*)HeapAlloc(m_hCacheHeap, 0, cbLen);
	*newValue++ = 0;
	strcpy(newValue, szName);
	m_lSettings.insert(newValue);
	return newValue;
}

char* MDatabaseCache::GetCachedSetting(const char *szModuleName, const char *szSettingName, int moduleNameLen, int settingNameLen)
{
	char szFullName[512];
	const char *szKey;
	if (szModuleName != NULL) {
		strcpy(szFullName, szModuleName);
		szFullName[moduleNameLen] = '/';
		strcpy(szFullName + moduleNameLen + 1, szSettingName);
		szKey = szFullName;
	}
	else szKey = szSettingName;

	if (m_lastSetting && !strcmp(szKey, m_lastSetting))
		return m_lastSetting;

	int index = m_lSettings.getIndex((char*)szKey);
	if (index != -1)
		m_lastSetting = m_lSettings[index];
	else
		m_lastSetting = InsertCachedSetting(szKey, settingNameLen + moduleNameLen + 3);

	return m_lastSetting;
}

void MDatabaseCache::SetCachedVariant(DBVARIANT* s /* new */, DBVARIANT* d /* cached */)
{
	char* szSave = (d->type == DBVT_UTF8 || d->type == DBVT_ASCIIZ) ? d->pszVal : NULL;

	memcpy(d, s, sizeof(DBVARIANT));
	if ((s->type == DBVT_UTF8 || s->type == DBVT_ASCIIZ) && s->pszVal != NULL) {
		if (szSave != NULL)
			d->pszVal = (char*)HeapReAlloc(m_hCacheHeap, 0, szSave, strlen(s->pszVal) + 1);
		else
			d->pszVal = (char*)HeapAlloc(m_hCacheHeap, 0, strlen(s->pszVal) + 1);
		strcpy(d->pszVal, s->pszVal);
	}
	else if (szSave != NULL)
		HeapFree(m_hCacheHeap, 0, szSave);
}

void MDatabaseCache::FreeCachedVariant(DBVARIANT* V)
{
	if ((V->type == DBVT_ASCIIZ || V->type == DBVT_UTF8) && V->pszVal != NULL)
		HeapFree(m_hCacheHeap, 0, V->pszVal);
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
				HeapFree(m_hCacheHeap, 0, V);
				return NULL;
			}
		}
		else {
			if (bAllocate != 1)
				return NULL;

			V = (DBCachedGlobalValue*)HeapAlloc(m_hCacheHeap, HEAP_ZERO_MEMORY, sizeof(DBCachedGlobalValue));
			V->name = szSetting;
			m_lGlobalSettings.insert(V);
		}

		return &V->value;
	}

	// a contact setting
	DBCachedContactValue *V, *V1;
	DBCachedContact ccTemp, *cc;

	ccTemp.contactID = contactID;

	int index = m_lContacts.getIndex(&ccTemp);
	if (index == -1)
		return NULL;

	m_lastVL = cc = m_lContacts[index];

	for (V = cc->first; V != NULL; V = V->next)
		if (V->name == szSetting)
			break;

	if (V == NULL) {
		if (bAllocate != 1)
			return NULL;

		V = (DBCachedContactValue *)HeapAlloc(m_hCacheHeap, HEAP_ZERO_MEMORY, sizeof(DBCachedContactValue));
		if (cc->last)
			cc->last->next = V;
		else
			cc->first = V;
		cc->last = V;
		V->name = szSetting;
	}
	else if (bAllocate == -1) {
		m_lastVL = NULL;
		FreeCachedVariant(&V->value);
		if (cc->first == V) {
			cc->first = V->next;
			if (cc->last == V)
				cc->last = V->next; // NULL
		}
		else
			for (V1 = cc->first; V1 != NULL; V1 = V1->next)
				if (V1->next == V) {
					V1->next = V->next;
					if (cc->last == V)
						cc->last = V1;
					break;
				}
		HeapFree(m_hCacheHeap, 0, V);
		return NULL;
	}

	return &V->value;
}
