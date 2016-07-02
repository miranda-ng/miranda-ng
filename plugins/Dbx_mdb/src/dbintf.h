/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-16 Miranda NG project (http://miranda-ng.org)
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

#define OWN_CACHED_CONTACT

#include <m_db_int.h>

#pragma warning (disable: 4200)

#define DBMODE_SHARED    0x0001
#define DBMODE_READONLY  0x0002

#define DBVT_ENCRYPTED   250
#define DBVT_UNENCRYPTED 251

#define MARKED_READ (DBEF_READ | DBEF_SENT)

struct ModuleName
{
	DWORD dwId;
	char szName[];
};

#include <pshpack1.h>

#define DBHEADER_VERSION MAKELONG(1, 2)

#define DBHEADER_SIGNATURE  0x40DECADEu
struct DBHeader
{
	DWORD dwSignature;
	DWORD dwVersion;			// database format version
};

struct DBContact
{
	DWORD dwEventCount;       // number of events in the chain for this contact
	DWORD tsFirstUnread;
	DWORD dwFirstUnread;
};

struct DBEvent
{
	MCONTACT contactID;     // a contact this event belongs to
	DWORD ofsModuleName;	// offset to a DBModuleName struct of the name of
	DWORD timestamp;        // seconds since 00:00:00 01/01/1970
	DWORD flags;            // see m_database.h, db/event/add
	WORD  wEventType;       // module-defined event type
	WORD  cbBlob;           // number of bytes in the blob

	bool __forceinline markedRead() const
	{
		return (flags & MARKED_READ) != 0;
	}
};

struct DBEventSortingKey
{
	DWORD dwEventId, ts, dwContactId;
};

struct DBSettingKey
{
	MCONTACT hContact;
	DWORD dwModuleId;
	char  szSettingName[];
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
	void Advance(DWORD id, DBEvent &dbe);
	void Snapshot();
	void Revert();
	DBContact dbc, tmp_dbc;
};

struct EventItem
{
	__forceinline EventItem(int _ts, DWORD _id) :
		ts(_ts), eventId(_id)
	{}

	int ts;
	DWORD eventId;
};

class LMDBEventCursor;

struct CDbxMdb : public MIDatabase, public MIDatabaseChecker, public MZeroedObject
{
	friend class LMDBEventCursor;

	CDbxMdb(const TCHAR *tszFileName, int mode);
	virtual ~CDbxMdb();

	int Load(bool bSkipInit);
	int Create(void);
	int Check(void);

	void DatabaseCorruption(const TCHAR *ptszText);

	void StoreKey(void);
	void SetPassword(const TCHAR *ptszPassword);
	void UpdateMenuItem(void);

	int  PrepareCheck(int*);

	__forceinline LPSTR GetMenuTitle() const { return m_bUsesPassword ? (char*)LPGEN("Change/remove password") : (char*)LPGEN("Set password"); }

	__forceinline bool isEncrypted() const { return m_bEncrypted; }
	__forceinline bool usesPassword() const { return m_bUsesPassword; }
	int      EnableEncryption(bool bEnable);
public:
	STDMETHODIMP_(BOOL)     IsRelational(void) { return TRUE; }
	STDMETHODIMP_(void)     SetCacheSafetyMode(BOOL);

	STDMETHODIMP_(LONG)     GetContactCount(void);
	STDMETHODIMP_(MCONTACT) FindFirstContact(const char *szProto = NULL);
	STDMETHODIMP_(MCONTACT) FindNextContact(MCONTACT contactID, const char *szProto = NULL);
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

	STDMETHODIMP_(BOOL)     GetContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv);
	STDMETHODIMP_(BOOL)     GetContactSettingStr(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv);
	STDMETHODIMP_(BOOL)     GetContactSettingStatic(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv);
	STDMETHODIMP_(BOOL)     FreeVariant(DBVARIANT *dbv);
	STDMETHODIMP_(BOOL)     WriteContactSetting(MCONTACT contactID, DBCONTACTWRITESETTING *dbcws);
	STDMETHODIMP_(BOOL)     DeleteContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting);
	STDMETHODIMP_(BOOL)     EnumContactSettings(MCONTACT contactID, DBCONTACTENUMSETTINGS *dbces);
	STDMETHODIMP_(BOOL)     SetSettingResident(BOOL bIsResident, const char *pszSettingName);
	STDMETHODIMP_(BOOL)     EnumResidentSettings(DBMODULEENUMPROC pFunc, void *pParam);
	STDMETHODIMP_(BOOL)     IsSettingEncrypted(LPCSTR szModule, LPCSTR szSetting);

	STDMETHODIMP_(BOOL)     MetaDetouchSub(DBCachedContact *cc, int nSub);
	STDMETHODIMP_(BOOL)     MetaSetDefault(DBCachedContact *cc);
	STDMETHODIMP_(BOOL)     MetaMergeHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub);
	STDMETHODIMP_(BOOL)     MetaSplitHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub);

protected:
	STDMETHODIMP_(BOOL)     Start(DBCHeckCallback *callback);
	STDMETHODIMP_(BOOL)     CheckDb(int phase, int firstTime);
	STDMETHODIMP_(VOID)     Destroy();

protected:

	void  FillContacts(void);

	int   Map();
	bool  Remap();

protected:
	TCHAR*   m_tszProfileName;
	bool     m_safetyMode, m_bReadOnly, m_bShared, m_bEncrypted, m_bUsesPassword;

	////////////////////////////////////////////////////////////////////////////
	// database stuff
public:
	MICryptoEngine *m_crypto;

protected:
	MDB_env *m_pMdbEnv;
	MDB_txn *m_txn;
	DWORD    m_dwFileSize;
	MDB_dbi  m_dbGlobal;
	DBHeader m_header;

	HANDLE   hSettingChangeEvent, hContactDeletedEvent, hContactAddedEvent, hEventMarkedRead;

	mir_cs   m_csDbAccess;

	int      CheckProto(DBCachedContact *cc, const char *proto);

	////////////////////////////////////////////////////////////////////////////
	// settings

	MDB_dbi  m_dbSettings;
	MDB_cursor *m_curSettings;

	int      m_codePage;
	HANDLE   hService, hHook;

	////////////////////////////////////////////////////////////////////////////
	// contacts

	MDB_dbi	m_dbContacts;
	MDB_cursor *m_curContacts;

	DWORD    m_contactCount;
	MCONTACT m_maxContactId;

	void     GatherContactHistory(MCONTACT hContact, LIST<EventItem> &items);

	////////////////////////////////////////////////////////////////////////////
	// events

	MDB_dbi	m_dbEvents, m_dbEventsSort;
	MDB_cursor *m_curEvents, *m_curEventsSort;
	DWORD    m_dwMaxEventId;

	void     FindNextUnread(const txn_ptr &_txn, DBCachedContact *cc, DBEventSortingKey &key2);

	////////////////////////////////////////////////////////////////////////////
	// modules

	MDB_dbi	m_dbModules;
	MDB_cursor *m_curModules;

	HANDLE   m_hModHeap;
	LIST<ModuleName> m_lMods, m_lOfs;
	LIST<char> m_lResidentSettings;
	HANDLE   hEventAddedEvent, hEventDeletedEvent, hEventFilterAddedEvent;
	MCONTACT m_hLastCachedContact;
	DWORD      m_maxModuleID;

	void     AddToList(const char *name, DWORD ofs);
	DWORD    FindExistingModuleNameOfs(const char *szName);
	int      InitModuleNames(void);
	DWORD    GetModuleNameOfs(const char *szName);
	char*    GetModuleNameByOfs(DWORD ofs);


	int GetContactSettingWorker(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, int isStatic);


	DBCHeckCallback *cb;

	////////////////////////////////////////////////////////////////////////////
	// encryption

	MDB_dbi  m_dbCrypto;

	int      InitCrypt(void);
	CRYPTO_PROVIDER* SelectProvider();
	void     GenerateNewKey();

	void     InitDialogs();
};