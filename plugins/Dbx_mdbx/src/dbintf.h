/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org)
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

#define DBVT_ENCRYPTED   250
#define DBVT_UNENCRYPTED 251

#define MARKED_READ (DBEF_READ | DBEF_SENT)

#include <pshpack1.h>

#define DBHEADER_VERSION MAKELONG(1, 4)

#define DBHEADER_SIGNATURE  0x40DECADEu
struct DBHeader
{
	uint32_t dwSignature;
	uint32_t dwVersion;			// database format version
};

struct DBContact
{
	uint32_t dwEventCount;       // number of events in the chain for this contact
	MEVENT   evFirstUnread;
	uint64_t tsFirstUnread;
};

struct DBEvent
{
	MCONTACT contactID;     // a contact this event belongs to
	uint32_t iModuleId;	    // offset to a DBModuleName struct of the name of
	uint64_t timestamp;        // seconds since 00:00:00 01/01/1970
	uint32_t flags;            // see m_database.h, db/event/add
	uint16_t wEventType;       // module-defined event type
	uint16_t cbBlob;           // number of bytes in the blob

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

	static int Compare(const MDBX_val* a, const MDBX_val* b);
};

struct DBSettingKey
{
	MCONTACT hContact;
	uint32_t dwModuleId;
	char     szSettingName[1];

	static int Compare(const MDBX_val*, const MDBX_val*);
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

class CDbxMDBX : public MDatabaseCommon, public MZeroedObject
{
	friend class MDBXEventCursor;

	__forceinline MDBX_txn* StartTran() const
	{
		MDBX_txn *res = 0;
		int rc = mdbx_txn_begin(m_env, nullptr, (m_bReadOnly) ? MDBX_RDONLY : 0, &res);
		/* FIXME: throw an exception */
		_ASSERT(rc == MDBX_SUCCESS);
		UNREFERENCED_PARAMETER(rc);
		return res;
	}

	void FillContacts(void);
	int  PrepareCheck(void);
	void TouchFile(void);
	void UpdateMenuItem(void);

	////////////////////////////////////////////////////////////////////////////
	// database stuff

	TCHAR*   m_tszProfileName;
	bool     m_safetyMode, m_bReadOnly, m_bShared, m_bEncrypted, m_bUsesPassword;

	MDBX_env *m_env;
	CMDBX_txn_ro m_txn_ro;

	MDBX_dbi m_dbGlobal;
	DBHeader m_header;
	HANDLE   hSettingChangeEvent, hContactDeletedEvent, hContactAddedEvent, hEventMarkedRead;

	HWND     m_hwndTimer; // for flushing database

	DBCachedContact m_ccDummy; // dummy contact to serve a cache item for MCONTACT = 0

	////////////////////////////////////////////////////////////////////////////
	// settings

	MDBX_dbi  m_dbSettings;
	MDBX_cursor *m_curSettings;

	HANDLE   hService, hHook;

	////////////////////////////////////////////////////////////////////////////
	// contacts

	MDBX_dbi	    m_dbContacts;
	MDBX_cursor *m_curContacts;

	MCONTACT m_maxContactId;

	void     GatherContactHistory(MCONTACT hContact, LIST<EventItem> &items);

	////////////////////////////////////////////////////////////////////////////
	// events

	MDBX_dbi	    m_dbEvents, m_dbEventsSort;
	MDBX_cursor *m_curEvents, *m_curEventsSort;
	MEVENT       m_dwMaxEventId;

	HANDLE   hEventAddedEvent, hEventDeletedEvent, hEventFilterAddedEvent;

	void     FindNextUnread(const txn_ptr &_txn, DBCachedContact *cc, DBEventSortingKey &key2);

	////////////////////////////////////////////////////////////////////////////
	// modules

	MDBX_dbi	m_dbModules;
	MDBX_cursor *m_curModules;

	std::map<uint32_t, std::string> m_Modules;

	int      InitModules();

	uint32_t GetModuleID(const char *szName);
	char*    GetModuleName(uint32_t dwId);

	////////////////////////////////////////////////////////////////////////////
	// encryption

	MDBX_dbi  m_dbCrypto;

	int      InitCrypt(void);
	CRYPTO_PROVIDER* SelectProvider();

	void     InitDialogs();

public:
	CDbxMDBX(const TCHAR *tszFileName, int mode);
	virtual ~CDbxMDBX();

	int  Check(void);
	void DBFlush(bool bForce = false);
	int  EnableEncryption(bool bEnable);
	int  Load();
	int  Map();
	void StoreKey(void);
	void SetPassword(const wchar_t *ptszPassword);

	__forceinline LPSTR GetMenuTitle() const { return m_bUsesPassword ? (char*)LPGEN("Change/remove password") : (char*)LPGEN("Set password"); }

	__forceinline bool isEncrypted() const { return m_bEncrypted; }
	__forceinline bool usesPassword() const { return m_bUsesPassword; }

public:
	STDMETHODIMP_(BOOL)     IsRelational(void) { return TRUE; }
	STDMETHODIMP_(void)     SetCacheSafetyMode(BOOL);

	STDMETHODIMP_(LONG)     GetContactCount(void);
	STDMETHODIMP_(LONG)     DeleteContact(MCONTACT contactID);
	STDMETHODIMP_(MCONTACT) AddContact(void);
	STDMETHODIMP_(BOOL)     IsDbContact(MCONTACT contactID);
	STDMETHODIMP_(LONG)     GetContactSize(void);

	STDMETHODIMP_(LONG)     GetEventCount(MCONTACT contactID);
	STDMETHODIMP_(MEVENT)   AddEvent(MCONTACT contactID, DBEVENTINFO *dbe);
	STDMETHODIMP_(BOOL)     DeleteEvent(MCONTACT contactID, MEVENT hDbEvent);
	STDMETHODIMP_(LONG)     GetBlobSize(MEVENT hDbEvent);
	STDMETHODIMP_(BOOL)     GetEvent(MEVENT hDbEvent, DBEVENTINFO *dbe);
	STDMETHODIMP_(BOOL)     MarkEventRead(MCONTACT contactID, MEVENT hDbEvent);
	STDMETHODIMP_(MCONTACT) GetEventContact(MEVENT hDbEvent);
	STDMETHODIMP_(MEVENT)   FindFirstEvent(MCONTACT contactID);
	STDMETHODIMP_(MEVENT)   FindFirstUnreadEvent(MCONTACT contactID);
	STDMETHODIMP_(MEVENT)   FindLastEvent(MCONTACT contactID);
	STDMETHODIMP_(MEVENT)   FindNextEvent(MCONTACT contactID, MEVENT hDbEvent);
	STDMETHODIMP_(MEVENT)   FindPrevEvent(MCONTACT contactID, MEVENT hDbEvent);

	STDMETHODIMP_(BOOL)     EnumModuleNames(DBMODULEENUMPROC pFunc, void *pParam);

	STDMETHODIMP_(BOOL)     GetContactSettingWorker(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, int isStatic);
	STDMETHODIMP_(BOOL)     WriteContactSetting(MCONTACT contactID, DBCONTACTWRITESETTING *dbcws);
	STDMETHODIMP_(BOOL)     DeleteContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting);
	STDMETHODIMP_(BOOL)     EnumContactSettings(MCONTACT hContact, DBSETTINGENUMPROC pfnEnumProc, const char *szModule, void *param);

	STDMETHODIMP_(BOOL)     MetaMergeHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub);
	STDMETHODIMP_(BOOL)     MetaSplitHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub);

public:
	MICryptoEngine *m_crypto;
};
