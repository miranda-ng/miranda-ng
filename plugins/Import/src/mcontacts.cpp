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

#define HEADER_STR "BHBF"

static int mc_makeDatabase(const wchar_t*)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CDbxMcontacts database driver, read-only

#pragma pack(push, 1)

struct MC_FileHeader
{
	unsigned char signature[4];
	unsigned char version;
	unsigned char extraFlags;
	unsigned short int reserved;
	unsigned int codepage;
	unsigned short int dataStart;
};

struct MC_MsgHeader
{
	DWORD timestamp;
	WORD eventType;
	WORD flags;
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
		MC_MsgHeader hdr;

		while (true) {
			DWORD dwPos = SetFilePointer(m_hFile, 0, 0, FILE_CURRENT), dwRead;
			BOOL r = ReadFile(m_hFile, &hdr, sizeof(hdr), &dwRead, 0);
			if (!r || dwPos < sizeof(hdr))
				return;

			readString();
			m_events.push_back(dwPos);
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

		return memcmp(&m_hdr.signature, HEADER_STR, 4) ? EGROKPRF_UNKHEADER : EGROKPRF_NOERROR;
	}

	STDMETHODIMP_(BOOL) IsRelational(void) override { return FALSE; }
	STDMETHODIMP_(void) SetCacheSafetyMode(BOOL) override {};

	STDMETHODIMP_(LONG) GetContactCount(void) override
	{
		return 1;
	}

	STDMETHODIMP_(LONG) DeleteContact(MCONTACT) override { return 1; }
	STDMETHODIMP_(MCONTACT) AddContact(void) override { return 0; }
	STDMETHODIMP_(BOOL) IsDbContact(MCONTACT contactID) override { return contactID == 1; }
	STDMETHODIMP_(LONG) GetContactSize(void) override { return sizeof(DBCachedContact); }

	STDMETHODIMP_(LONG) GetEventCount(MCONTACT) override { return (LONG)m_events.size(); }
	STDMETHODIMP_(MEVENT) AddEvent(MCONTACT, DBEVENTINFO*) override { return 0; }
	STDMETHODIMP_(BOOL) DeleteEvent(MCONTACT, MEVENT) override { return 1; }
	STDMETHODIMP_(LONG) GetBlobSize(MEVENT) override { return 0; }
	STDMETHODIMP_(BOOL) GetEvent(MEVENT, DBEVENTINFO*) override { return 0; }
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
