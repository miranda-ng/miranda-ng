/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
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

/////////////////////////////////////////////////////////////////////////////////////////

struct VarDescr
{
	VarDescr(LPCSTR var, LPCSTR value) :
		szVar(mir_strdup(var)),
		szValue(mir_strdup(value)),
		iLen(0)
	{}

	VarDescr(LPCSTR var, LPSTR value) :
		szVar(mir_strdup(var)),
		szValue(value),
		iLen(0)
	{}

	VarDescr(LPCSTR var, uint8_t *value, int len) :
		szVar(mir_strdup(var)),
		szValue((char*)memcpy(mir_alloc(len), value, len)),
		iLen(len)
	{}

	ptrA szVar, szValue;
	int iLen;
};

int CDb3Mmap::InitCrypt()
{
	if (m_dbHeader.version == DB_OLD_VERSION)
		return 0;

	return MDatabaseCommon::InitCrypt();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Saving encryption key in a database

STDMETHODIMP_(BOOL) CDb3Mmap::ReadCryptoKey(MBinBuffer &buf)
{
	DBVARIANT dbv = {};
	dbv.type = DBVT_BLOB;
	if (GetContactSetting(0, "CryptoEngine", "StoredKey", &dbv))
		return FALSE;

	buf.append(dbv.pbVal, dbv.cpbVal);
	return TRUE;
}

STDMETHODIMP_(BOOL) CDb3Mmap::StoreCryptoKey()
{
	size_t iKeyLength = m_crypto->getKeyLength();
	uint8_t *pKey = (uint8_t*)_alloca(iKeyLength);
	m_crypto->getKey(pKey, iKeyLength);

	DBCONTACTWRITESETTING dbcws = { "CryptoEngine", "StoredKey" };
	dbcws.value.type = DBVT_BLOB;
	dbcws.value.cpbVal = (uint16_t)iKeyLength;
	dbcws.value.pbVal = pKey;
	WriteContactSetting(0, &dbcws);

	SecureZeroMemory(pKey, iKeyLength);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Saving encryption flag

STDMETHODIMP_(BOOL) CDb3Mmap::ReadEncryption()
{
	DBVARIANT dbv = {};
	dbv.type = DBVT_BYTE;
	return (GetContactSetting(0, "CryptoEngine", "DatabaseEncryption", &dbv)) ? false : dbv.bVal != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Saving provider in a database

STDMETHODIMP_(CRYPTO_PROVIDER *) CDb3Mmap::ReadProvider()
{
	DBVARIANT dbv = {};
	dbv.type = DBVT_BLOB;
	if (GetContactSetting(0, "CryptoEngine", "Provider", &dbv))
		return nullptr;

	if (dbv.type != DBVT_BLOB) 
		return nullptr;
	
	auto *pProvider = Crypto_GetProvider(LPCSTR(dbv.pbVal));
	FreeVariant(&dbv);
	return pProvider;
}

STDMETHODIMP_(BOOL) CDb3Mmap::StoreProvider(CRYPTO_PROVIDER *pProvider)
{
	DBCONTACTWRITESETTING dbcws = { "CryptoEngine", "Provider" };
	dbcws.value.type = DBVT_BLOB;
	dbcws.value.pbVal = (uint8_t*)pProvider->pszName;
	dbcws.value.cpbVal = (uint16_t)mir_strlen(pProvider->pszName) + 1;
	WriteContactSetting(0, &dbcws);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP_(BOOL) CDb3Mmap::EnableEncryption(BOOL bEnable)
{
	if (bEnable == m_bEncrypted)
		return TRUE;

	HANDLE hSave1 = g_hevSettingChanged; g_hevSettingChanged = nullptr;
	HANDLE hSave2 = g_hevEventAdded;     g_hevEventAdded = nullptr;
	HANDLE hSave3 = g_hevEventDeleted;   g_hevEventDeleted = nullptr;
	HANDLE hSave4 = g_hevEventFiltered;  g_hevEventFiltered = nullptr;

	mir_cslock lck(m_csDbAccess);
	ToggleSettingsEncryption(0);
	ToggleEventsEncryption(0);

	for (MCONTACT contactID = FindFirstContact(); contactID; contactID = FindNextContact(contactID)) {
		ToggleSettingsEncryption(contactID);
		ToggleEventsEncryption(contactID);
	}

	m_bEncrypted = !m_bEncrypted;

	DBCONTACTWRITESETTING dbcws = { "CryptoEngine", "DatabaseEncryption" };
	dbcws.value.type = DBVT_BYTE;
	dbcws.value.bVal = m_bEncrypted;
	WriteContactSetting(0, &dbcws);

	g_hevSettingChanged = hSave1;
	g_hevEventAdded = hSave2;
	g_hevEventDeleted = hSave3;
	g_hevEventFiltered = hSave4;
	return TRUE;
}

void CDb3Mmap::ToggleSettingsEncryption(MCONTACT contactID)
{
	uint32_t ofsContact = GetContactOffset(contactID);
	if (ofsContact == 0)
		return;

	DBContact *contact = (DBContact*)DBRead(ofsContact, nullptr);
	if (contact->ofsFirstSettings == 0)
		return;

	// fast cycle through all settings
	DBContactSettings *setting = (DBContactSettings*)DBRead(contact->ofsFirstSettings, nullptr);
	uint32_t offset = contact->ofsFirstSettings;
	char *szModule = GetModuleNameByOfs(setting->ofsModuleName);
	if (szModule == nullptr)
		return;

	while (true) {
		OBJLIST<VarDescr> arSettings(10);
		char szSetting[256];
		int bytesRemaining, len;
		uint32_t ofsBlobPtr = offset + offsetof(DBContactSettings, blob), ofsNext = setting->ofsNext;
		uint8_t *pBlob = (uint8_t*)DBRead(ofsBlobPtr, &bytesRemaining);
		while (pBlob[0]) {
			NeedBytes(1);
			len = pBlob[0];
			memcpy(szSetting, pBlob + 1, len); szSetting[len] = 0;
			NeedBytes(1 + pBlob[0]);
			MoveAlong(1 + pBlob[0]);
			NeedBytes(5);

			switch (pBlob[0]) {
			case DBVT_ASCIIZ:
				len = *(PWORD)(pBlob + 1);
				// we need to convert a string into utf8 and encrypt it
				if (!m_bEncrypted) {
					uint8_t bSave = pBlob[len + 3]; pBlob[len + 3] = 0;
					arSettings.insert(new VarDescr(szSetting, mir_utf8encode((LPCSTR)pBlob + 3)));
					pBlob[len + 3] = bSave;
				}
				NeedBytes(3 + len);
				break;

			case DBVT_UTF8:
				len = *(PWORD)(pBlob + 1);
				// we need to encrypt these strings
				if (!m_bEncrypted) {
					uint8_t bSave = pBlob[len + 3]; pBlob[len + 3] = 0;
					arSettings.insert(new VarDescr(szSetting, (LPCSTR)pBlob + 3));
					pBlob[len + 3] = bSave;
				}
				NeedBytes(3 + len);
				break;

			case DBVT_ENCRYPTED:
				len = *(PWORD)(pBlob + 1);
				// we need to decrypt these strings
				if (m_bEncrypted && !IsSettingEncrypted(szModule, szSetting))
					arSettings.insert(new VarDescr(szSetting, pBlob + 3, len));
				NeedBytes(3 + len);
				break;

			case DBVT_BLOB:
				NeedBytes(3 + *(PWORD)(pBlob + 1));
				break;
			}
			NeedBytes(3);
			MoveAlong(1 + GetSettingValueLength(pBlob));
			NeedBytes(1);
		}

		for (auto &p : arSettings) {
			if (!m_bEncrypted) {
				size_t encodedLen;
				uint8_t *pResult = m_crypto->encodeString(p->szValue, &encodedLen);
				if (pResult != nullptr) {
					DBCONTACTWRITESETTING dbcws = { szModule, p->szVar };
					dbcws.value.type = DBVT_ENCRYPTED;
					dbcws.value.pbVal = pResult;
					dbcws.value.cpbVal = (uint16_t)encodedLen;
					WriteContactSetting(contactID, &dbcws);

					mir_free(pResult);
				}
			}
			else {
				size_t realLen;
				ptrA decoded(m_crypto->decodeString((uint8_t*)(char*)p->szValue, p->iLen, &realLen));
				if (decoded != nullptr) {
					DBCONTACTWRITESETTING dbcws = { szModule, p->szVar };
					dbcws.value.type = DBVT_UNENCRYPTED;
					dbcws.value.pszVal = decoded;
					dbcws.value.cchVal = (uint16_t)realLen;
					WriteContactSetting(contactID, &dbcws);
				}
			}
		}

		if (!ofsNext)
			break;

		setting = (DBContactSettings*)DBRead(offset = ofsNext, nullptr);
		if ((szModule = GetModuleNameByOfs(setting->ofsModuleName)) == nullptr)
			break;
	}
}

void CDb3Mmap::ToggleEventsEncryption(MCONTACT contactID)
{
	uint32_t ofsContact = GetContactOffset(contactID);
	if (ofsContact == 0)
		return;

	DBContact contact = *(DBContact*)DBRead(ofsContact, nullptr);
	if (contact.ofsFirstEvent == 0 || contact.signature != DBCONTACT_SIGNATURE)
		return;

	// fast cycle through all events
	for (uint32_t offset = contact.ofsFirstEvent; offset != 0;) {
		DBEvent evt = *(DBEvent*)DBRead(offset, nullptr);
		if (evt.signature != DBEVENT_SIGNATURE)
			return;

		size_t len;
		uint32_t ofsDest;
		mir_ptr<uint8_t> pBlob;
		uint8_t *pSource = DBRead(offset + offsetof(DBEvent, blob), nullptr);
		if (!m_bEncrypted) { // we need more space
			if ((pBlob = m_crypto->encodeBuffer(pSource, evt.cbBlob, &len)) == nullptr)
				return;

			ofsDest = ReallocSpace(offset, offsetof(DBEvent, blob) + evt.cbBlob, offsetof(DBEvent, blob) + (uint32_t)len);

			if (evt.ofsNext) {
				DBEvent *e = (DBEvent*)DBRead(evt.ofsNext, nullptr);
				e->ofsPrev = ofsDest;
				DBWrite(evt.ofsNext, e, sizeof(DBEvent));
			}
			if (evt.ofsPrev) {
				DBEvent *e = (DBEvent*)DBRead(evt.ofsPrev, nullptr);
				e->ofsNext = ofsDest;
				DBWrite(evt.ofsPrev, e, sizeof(DBEvent));
			}
			if (contact.ofsFirstEvent == offset)
				contact.ofsFirstEvent = ofsDest;
			if (contact.ofsLastEvent == offset)
				contact.ofsLastEvent = ofsDest;
			if (contact.ofsFirstUnread == offset)
				contact.ofsFirstUnread = ofsDest;

			evt.flags |= DBEF_ENCRYPTED;
		}
		else {
			if ((pBlob = (uint8_t*)m_crypto->decodeBuffer(pSource, evt.cbBlob, &len)) == nullptr)
				return;

			ofsDest = offset; // reuse the old space
			evt.flags &= ~DBEF_ENCRYPTED;

			if (len < evt.cbBlob)
				DBFill(ofsDest + offsetof(DBEvent, blob) + (uint32_t)len, evt.cbBlob - (uint32_t)len);
		}
		evt.cbBlob = (uint32_t)len;

		DBWrite(ofsDest, &evt, offsetof(DBEvent, blob));
		DBWrite(ofsDest + offsetof(DBEvent, blob), pBlob, (uint32_t)len);

		offset = evt.ofsNext;
	}

	DBWrite(ofsContact, &contact, sizeof(DBContact));
}
