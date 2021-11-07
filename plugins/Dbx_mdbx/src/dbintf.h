/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)
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

#pragma once

#define OWN_CACHED_CONTACT

#include <m_db_int.h>

#pragma warning (disable: 4200)

#define DBMODE_SHARED    0x0001
#define DBMODE_READONLY  0x0002

#define MARKED_READ (DBEF_READ | DBEF_SENT)

#include <pshpack1.h>

#define DBHEADER_VERSION    MAKELONG(1, 4)
#define DBHEADER_SIGNATURE  0x40DECADEu
struct DBHeader
{
	uint32_t dwSignature;
	uint32_t dwVersion;      // database format version
};

struct DBContact
{
	uint32_t dwEventCount;   // number of events in the chain for this contact
	MEVENT   evFirstUnread;
	uint64_t tsFirstUnread;
};

struct DBEvent
{
	MCONTACT dwContactID;    // a contact this event belongs to
	uint32_t iModuleId;	    // offset to a DBModuleName struct of the name of
	uint64_t timestamp;      // seconds since 00:00:00 01/01/1970
	uint32_t flags;          // see m_database.h, db/event/add
	uint16_t wEventType;     // module-defined event type
	uint16_t cbBlob;         // number of bytes in the blob

	bool __forceinline markedRead() const
	{
		return (flags & MARKED_READ) != 0;
	}
};

struct DBEventSortingKey
{
	MCONTACT hContact;
	MEVENT hEvent;
	uint64_t ts;

	static int Compare(const MDBX_val*, const MDBX_val*) MDBX_CXX17_NOEXCEPT;
};

struct DBEventIdKey
{
	uint32_t iModuleId;	    // offset to a DBModuleName struct of the name of
	char     szEventId[256]; // string id

	static int Compare(const MDBX_val*, const MDBX_val*) MDBX_CXX17_NOEXCEPT;
};

struct DBSettingKey
{
	MCONTACT hContact;
	uint32_t dwModuleId;
	char     szSettingName[1];

	static int Compare(const MDBX_val*, const MDBX_val*) MDBX_CXX17_NOEXCEPT;
};

struct DBSettingValue
{
	BYTE type;
	union
	{
		BYTE bVal;
		WORD wVal;
		DWORD dwVal;
		char szVal[];

		struct
		{
			size_t nLength;
			BYTE bVal[];
		} blob;
	};
};

#include <poppack.h>

struct DBCachedContact : public DBCachedContactBase
{
	void Advance(MEVENT id, DBEvent &dbe);
	void Snapshot();
	void Revert();
	
	DBContact dbc, tmp_dbc;
	uint64_t t_tsLast;
	MEVENT t_evLast;
};

struct EventItem
{
	__forceinline EventItem(int _ts, MEVENT _id) :
		ts(_ts), eventId(_id)
	{}

	uint64_t ts;
	MEVENT eventId;
};

class CDbxMDBX : public MDatabaseCommon, public MIDatabaseChecker, public MZeroedObject
{
	friend class CMdbxEventCursor;
	friend class txn_ptr;
	friend class txn_ptr_ro;

	typedef std::map<uint32_t, std::string> TModuleMap;

	struct Impl {
		CDbxMDBX &pro;

		CTimer m_timer;
		void OnTimer(CTimer *pTimer)
		{
			pTimer->Stop();
			pro.DBFlush(true);
		}

		Impl(CDbxMDBX &_p) :
			pro(_p),
			m_timer(Miranda_GetSystemWindow(), UINT_PTR(this))
		{
			m_timer.OnEvent = Callback(this, &Impl::OnTimer);
		}
	} m_impl;

	MDBX_txn* StartTran();

	bool CheckEvent(DBCachedContact *cc, const DBEvent *dbe, DBCachedContact *&cc2);
	bool EditEvent(MCONTACT contactID, MEVENT hDbEvent, const DBEVENTINFO *dbe, bool bNew);
	int  PrepareCheck(void);
	void TouchFile(void);

	////////////////////////////////////////////////////////////////////////////
	// database stuff

	ptrW         m_pwszProfileName;
	bool         m_safetyMode = true, m_bReadOnly;

	MDBX_env    *m_env;
	MDBX_txn    *m_pWriteTran;
	int 			 m_dbError;

	MDBX_dbi     m_dbGlobal;
	DBHeader     m_header;

	DBCachedContact m_ccDummy; // dummy contact to serve a cache item for MCONTACT = 0

	////////////////////////////////////////////////////////////////////////////
	// settings

	MDBX_dbi     m_dbSettings;
	HANDLE       hService[1], hHook;

	void         FillSettings(void);

	////////////////////////////////////////////////////////////////////////////
	// contacts

	MDBX_dbi	    m_dbContacts;
	MCONTACT     m_maxContactId = 0;

	void         FillContacts(void);
	void         GatherContactHistory(MCONTACT hContact, OBJLIST<EventItem> &items);

	////////////////////////////////////////////////////////////////////////////
	// events

	MDBX_dbi	    m_dbEvents, m_dbEventsSort, m_dbEventIds;
	MDBX_cursor *m_curEventsSort;
	MEVENT       m_dwMaxEventId;

	void         FindNextUnread(const txn_ptr &_txn, DBCachedContact *cc, DBEventSortingKey &key2);

	////////////////////////////////////////////////////////////////////////////
	// modules

	MDBX_dbi	    m_dbModules;
	TModuleMap   m_Modules;

	int          InitModules();

	uint32_t     GetModuleID(const char *szName);
	char*        GetModuleName(uint32_t dwId);

	////////////////////////////////////////////////////////////////////////////
	// encryption

	MDBX_dbi m_dbCrypto;

public:
	CDbxMDBX(const wchar_t *tszFileName, int mode);
	virtual ~CDbxMDBX();

	int  Check(void);
	void DBFlush(bool bForce = false);
	int  Load();
	int  Map();

	int  CheckEvents1(void);
	int  CheckEvents2(void);
	int  CheckEvents3(void);

public:
	STDMETHODIMP_(BOOL)     IsRelational(void) override { return TRUE; }
	STDMETHODIMP_(void)     SetCacheSafetyMode(BOOL) override;

	STDMETHODIMP_(int)      GetContactCount(void) override;
	STDMETHODIMP_(int)      DeleteContact(MCONTACT contactID) override;
	STDMETHODIMP_(MCONTACT) AddContact(void) override;
	STDMETHODIMP_(BOOL)     IsDbContact(MCONTACT contactID) override;
	STDMETHODIMP_(int)      GetContactSize(void) override;

	STDMETHODIMP_(int)      GetEventCount(MCONTACT contactID) override;
	STDMETHODIMP_(MEVENT)   AddEvent(MCONTACT contactID, const DBEVENTINFO *dbe) override;
	STDMETHODIMP_(BOOL)     DeleteEvent(MEVENT hDbEvent) override;
	STDMETHODIMP_(BOOL)     EditEvent(MCONTACT contactID, MEVENT hDbEvent, const DBEVENTINFO *dbe) override;
	STDMETHODIMP_(int)      GetBlobSize(MEVENT hDbEvent) override;
	STDMETHODIMP_(BOOL)     GetEvent(MEVENT hDbEvent, DBEVENTINFO *dbe) override;
	STDMETHODIMP_(BOOL)     MarkEventRead(MCONTACT contactID, MEVENT hDbEvent) override;
	STDMETHODIMP_(MCONTACT) GetEventContact(MEVENT hDbEvent) override;
	STDMETHODIMP_(MEVENT)   FindFirstEvent(MCONTACT contactID) override;
	STDMETHODIMP_(MEVENT)   FindFirstUnreadEvent(MCONTACT contactID) override;
	STDMETHODIMP_(MEVENT)   FindLastEvent(MCONTACT contactID) override;
	STDMETHODIMP_(MEVENT)   FindNextEvent(MCONTACT contactID, MEVENT hDbEvent) override;
	STDMETHODIMP_(MEVENT)   FindPrevEvent(MCONTACT contactID, MEVENT hDbEvent) override;

	STDMETHODIMP_(BOOL)     EnumModuleNames(DBMODULEENUMPROC pFunc, void *pParam) override;

	STDMETHODIMP_(BOOL)     WriteContactSettingWorker(MCONTACT contactID, DBCONTACTWRITESETTING &dbcws) override;
	STDMETHODIMP_(BOOL)     DeleteContactSetting(MCONTACT contactID, const char *szModule, const char *szSetting) override;
	STDMETHODIMP_(BOOL)     EnumContactSettings(MCONTACT hContact, DBSETTINGENUMPROC pfnEnumProc, const char *szModule, void *param) override;

	STDMETHODIMP_(BOOL)     MetaMergeHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub) override;
	STDMETHODIMP_(BOOL)     MetaSplitHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub) override;
	STDMETHODIMP_(BOOL)     MetaRemoveSubHistory(DBCachedContact *ccSub) override;

	STDMETHODIMP_(CRYPTO_PROVIDER*) ReadProvider(void) override;
	STDMETHODIMP_(BOOL)     StoreProvider(CRYPTO_PROVIDER*) override;

	STDMETHODIMP_(BOOL)     EnableEncryption(BOOL) override;
	STDMETHODIMP_(BOOL)     ReadEncryption(void) override;

	STDMETHODIMP_(BOOL)     ReadCryptoKey(MBinBuffer&) override;
	STDMETHODIMP_(BOOL)     StoreCryptoKey(void) override;

	STDMETHODIMP_(BOOL)     Compact() override;
	STDMETHODIMP_(BOOL)     Backup(const wchar_t*) override;
	STDMETHODIMP_(BOOL)     Flush() override;

	STDMETHODIMP_(MEVENT)   GetEventById(const char *szModule, const char *szId) override;

	STDMETHODIMP_(DATABASELINK*) GetDriver() override;

	STDMETHODIMP_(DB::EventCursor *) EventCursor(MCONTACT hContact, MEVENT hDbEvent) override;
	STDMETHODIMP_(DB::EventCursor *) EventCursorRev(MCONTACT hContact, MEVENT hDbEvent) override;

protected:
	STDMETHODIMP_(MIDatabaseChecker *) GetChecker() override
	{
		return this;
	}

	STDMETHODIMP_(BOOL) Start(DBCHeckCallback *callback)
	{
		cb = callback;
		return ERROR_SUCCESS;
	}

	STDMETHODIMP_(VOID) Destroy()
	{}

	STDMETHODIMP_(BOOL) CheckDb(int phase);

	DBCHeckCallback *cb;
};
