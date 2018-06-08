/*

Import plugin for Miranda NG

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org)

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

#include <memory>
#include <vector>

#define HEADER_STR "HB"

static int mc_makeDatabase(const wchar_t*)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CDbxMcontacts database driver, read-only

#pragma pack(push, 1)

struct MC_FileHeader
{
	uint8_t  signature[2];
	uint32_t version;
	uint32_t dataSize;
};

struct MC_MsgHeader
{
	uint32_t cbSize;    // size of the structure in bytes
	uint32_t szModule;  // pointer to name of the module that 'owns' this
							  // event, ie the one that is in control of the data format
	uint32_t timestamp; // seconds since 00:00, 01/01/1970. Gives us times until
							  // 2106 unless you use the standard C library which is
							  // signed and can only do until 2038. In GMT.
	uint32_t flags;	  // the omnipresent flags
	uint32_t eventType; // module-defined event type field
	uint32_t cbBlob;    // size of pBlob in bytes
	uint32_t pBlob;     // pointer to buffer containing module-defined event data
};

#pragma pack(pop)

class CDbxMc : public MDatabaseCommon, public MZeroedObject
{
	HANDLE m_hFile = INVALID_HANDLE_VALUE;

	MC_FileHeader m_hdr;
	
	std::vector<DWORD> m_events;
	std::vector<DWORD>::iterator m_curr;

	CMStringA readString()
	{
		CMStringA res;
		char c;
		DWORD dwRead;
		while (ReadFile(m_hFile, &c, 1, &dwRead, 0)) {
			if (c == 0)
				break;
			res.AppendChar(c);
		}
		return res;
	}

public:
	CDbxMc()
	{}

	~CDbxMc()
	{
		if (m_hFile != INVALID_HANDLE_VALUE)
			::CloseHandle(m_hFile);
	}

	void Load()
	{
		// mcontacts operates with the only contact with pseudo id=1
		m_cache->AddContactToCache(1);

		MC_MsgHeader hdr;
		for (uint32_t pos = 0; pos < m_hdr.dataSize; pos += sizeof(hdr)) {
			DWORD dwPos = SetFilePointer(m_hFile, 0, 0, FILE_CURRENT), dwRead;
			BOOL r = ReadFile(m_hFile, &hdr, sizeof(hdr), &dwRead, 0);
			if (!r || dwRead < sizeof(hdr))
				return;
			if (hdr.cbSize < sizeof(hdr))
				return;

			m_events.push_back(dwPos);
			SetFilePointer(m_hFile, hdr.cbSize - sizeof(hdr) + hdr.cbBlob, 0, FILE_CURRENT);
		}
	}

	int Open(const wchar_t *profile)
	{
		m_hFile = CreateFile(profile, GENERIC_READ, 0, 0, OPEN_ALWAYS, 0, 0);
		if (m_hFile == INVALID_HANDLE_VALUE)
			return EGROKPRF_CANTREAD;

		DWORD dwRead;
		BOOL r = ReadFile(m_hFile, &m_hdr, sizeof(m_hdr), &dwRead, nullptr);
		if (!r)
			return EGROKPRF_CANTREAD;

		return memcmp(&m_hdr.signature, HEADER_STR, 2) ? EGROKPRF_UNKHEADER : EGROKPRF_NOERROR;
	}

	STDMETHODIMP_(BOOL) IsRelational(void) override { return FALSE; }

	// mcontacts format always store history for one contact only
	STDMETHODIMP_(LONG) GetContactCount(void) override
	{
		return 1;
	}

	STDMETHODIMP_(LONG) GetEventCount(MCONTACT) override
	{
		return (LONG)m_events.size();
	}

	STDMETHODIMP_(BOOL) GetEvent(MEVENT dwOffset, DBEVENTINFO *dbei) override
	{
		if (INVALID_SET_FILE_POINTER == SetFilePointer(m_hFile, dwOffset, 0, FILE_BEGIN))
			return 0;

		DWORD dwRead;
		MC_MsgHeader hdr;
		BOOL r = ReadFile(m_hFile, &hdr, sizeof(hdr), &dwRead, nullptr);
		if (!r || dwRead < sizeof(hdr) || hdr.cbSize < sizeof(hdr))
			return 0;

		dbei->eventType = hdr.eventType;
		dbei->cbBlob = hdr.cbBlob;
		dbei->flags = hdr.flags;
		dbei->timestamp = hdr.timestamp;

		if (hdr.cbBlob) {
			dbei->pBlob = (PBYTE)mir_alloc(hdr.cbBlob + 1);
			if (!ReadFile(m_hFile, dbei->pBlob, hdr.cbBlob, &dwRead, 0) || dwRead != hdr.cbBlob) {
				mir_free(dbei->pBlob);
				dbei->pBlob = 0;
				return 0;
			}

			dbei->pBlob[hdr.cbBlob] = 0;
		}

		return 0;
	}

	STDMETHODIMP_(void) SetCacheSafetyMode(BOOL) override {};

	STDMETHODIMP_(LONG) DeleteContact(MCONTACT) override { return 1; }
	STDMETHODIMP_(MCONTACT) AddContact(void) override { return 0; }
	STDMETHODIMP_(BOOL) IsDbContact(MCONTACT contactID) override { return contactID == 1; }
	STDMETHODIMP_(LONG) GetContactSize(void) override { return sizeof(DBCachedContact); }

	STDMETHODIMP_(MEVENT) AddEvent(MCONTACT, DBEVENTINFO*) override { return 0; }
	STDMETHODIMP_(BOOL) DeleteEvent(MCONTACT, MEVENT) override { return 1; }
	STDMETHODIMP_(LONG) GetBlobSize(MEVENT) override { return 0; }
	STDMETHODIMP_(BOOL) MarkEventRead(MCONTACT, MEVENT) override { return 1; }
	STDMETHODIMP_(MCONTACT) GetEventContact(MEVENT) override { return 0; }
	
	STDMETHODIMP_(MEVENT) FindFirstEvent(MCONTACT) override
	{
		m_curr = m_events.begin();
		return *m_curr;
	}

	STDMETHODIMP_(MEVENT) FindNextEvent(MCONTACT, MEVENT) override
	{
		if (m_curr == m_events.end())
			return 0;

		++m_curr;
		return *m_curr;
	}

	STDMETHODIMP_(MEVENT) FindLastEvent(MCONTACT) override
	{
		m_curr = m_events.end();
		return *m_curr;
	}

	STDMETHODIMP_(MEVENT) FindPrevEvent(MCONTACT, MEVENT) override
	{
		if (m_curr == m_events.begin())
			return 0;

		--m_curr;
		return *m_curr;
	}

	STDMETHODIMP_(MEVENT) FindFirstUnreadEvent(MCONTACT) override { return 0; }

	STDMETHODIMP_(BOOL) EnumModuleNames(DBMODULEENUMPROC, void *) override { return 0; }

	STDMETHODIMP_(BOOL) GetContactSettingWorker(MCONTACT, LPCSTR, LPCSTR, DBVARIANT*, int) override { return 1; }
	STDMETHODIMP_(BOOL) WriteContactSetting(MCONTACT, DBCONTACTWRITESETTING*) override { return 1; }
	STDMETHODIMP_(BOOL) DeleteContactSetting(MCONTACT, LPCSTR, LPCSTR) override { return 1; }
	STDMETHODIMP_(BOOL) EnumContactSettings(MCONTACT, DBSETTINGENUMPROC, const char*, void*) override { return 1; }

	STDMETHODIMP_(BOOL) MetaMergeHistory(DBCachedContact*, DBCachedContact*) override { return 1; }
	STDMETHODIMP_(BOOL) MetaSplitHistory(DBCachedContact*, DBCachedContact*) override { return 1; }
};

static int mc_grokHeader(const wchar_t *profile)
{
	return CDbxMc().Open(profile);
}

static MDatabaseCommon* mc_load(const wchar_t *profile, BOOL)
{
	std::auto_ptr<CDbxMc> db(new CDbxMc());
	if (db->Open(profile))
		return nullptr;

	db->Load();
	return db.release();
}

static DATABASELINK dblink =
{
	0,
	"mcontacts",
	L"mContacts file driver",
	mc_makeDatabase,
	mc_grokHeader,
	mc_load
};

void RegisterMContacts()
{
	RegisterDatabasePlugin(&dblink);
}
