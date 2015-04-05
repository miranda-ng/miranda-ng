/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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

#include <kchashdb.h>
using namespace kyotocabinet;

/* tree diagram

DBHeader
|-->end of file (plain offset)
|-->first contact (DBContact)
|   |-->next contact (DBContact)
|   |   \--> ...
|   |-->first settings (DBContactSettings)
|   |	 |-->next settings (DBContactSettings)
|   |   |   \--> ...
|   |   \-->module name (DBModuleName)
|   \-->first/last/firstunread event
|-->user contact (DBContact)
|   |-->next contact = NULL
|   |-->first settings	as above
|   \-->first/last/firstunread event as above
\-->first module name (DBModuleName)
\-->next module name (DBModuleName)
\--> ...
*/

#define DBMODE_SHARED    0x0001
#define DBMODE_READONLY  0x0002

#define DBVT_ENCRYPTED   250
#define DBVT_UNENCRYPTED 251

#define MARKED_READ (DBEF_READ | DBEF_SENT)

struct ModuleName
{
	char *name;
	DWORD ofs;
};

#include <pshpack1.h>

#define DBCONTACT_SIGNATURE 0x43DECADEu
struct DBContact
{
	DWORD dwSignature;
	DWORD dwEventCount;       // number of events in the chain for this contact
	DWORD tsFirstUnread;
	DWORD dwFirstUnread;
};

#define DBMODULENAME_SIGNATURE 0x4DDECADEu
struct DBModuleName
{
	DWORD dwSignature;
	BYTE cbName;            // number of characters in this module name
	char name[1];           // name, no nul terminator
};

#define DBEVENT_SIGNATURE  0x45DECADEu
struct DBEvent
{
	DWORD dwSignature;
	MCONTACT contactID;     // a contact this event belongs to
	DWORD ofsModuleName;	   // offset to a DBModuleName struct of the name of
	DWORD timestamp;        // seconds since 00:00:00 01/01/1970
	DWORD flags;            // see m_database.h, db/event/add
	WORD  wEventType;       // module-defined event type
	WORD  cbBlob;           // number of bytes in the blob

	bool __forceinline markedRead() const
	{
		return (flags & MARKED_READ) != 0;
	}
};

#include <poppack.h>

struct DBEventSortingKey
{
	uint32_t dwContactId, ts, dwEventId;
};

struct DBSettingSortingKey
{
	uint32_t dwContactID, dwOfsModule;
	char     szSettingName[100];
};

struct DBCachedContact : public DBCachedContactBase
{
	void Advance(DWORD id, DBEvent &dbe);

	DBContact dbc;
};

struct CTable : public TreeDB
{
	CTable()
	{
		tune_options(TLINEAR | TCOMPRESS);
		tune_alignment(3);
		tune_map(1024 * 1024);
	}
};

struct CDbxKyoto : public MIDatabase, public MIDatabaseChecker, public MZeroedObject
{
	CDbxKyoto(const TCHAR *tszFileName, int mode);
	~CDbxKyoto();

	int Load(bool bSkipInit);
	int Create(void);
	int Check(void);

	void DatabaseCorruption(const TCHAR *ptszText);

	void ToggleEncryption(void);
	void StoreKey(void);
	void SetPassword(const TCHAR *ptszPassword);
	void UpdateMenuItem(void);

	int  PrepareCheck(int*);

	__forceinline LPSTR GetMenuTitle() const { return m_bUsesPassword ? LPGEN("Change/remove password") : LPGEN("Set password"); }

	__forceinline bool isEncrypted() const { return m_bEncrypted; }
	__forceinline bool usesPassword() const { return m_bUsesPassword; }

public:
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
	void  InvalidateSettingsGroupOfsCacheEntry(DWORD) {}
	int   WorkInitialCheckHeaders(void);

	void  FillContacts(void);

public:  // Check functions
	int WorkInitialChecks(int);
	int WorkModuleChain(int);
	int WorkUser(int);
	int WorkContactChain(int);
	int WorkAggressive(int);
	int WorkFinalTasks(int);

protected:
	TCHAR*   m_tszProfileName;
	bool     m_safetyMode, m_bReadOnly, m_bShared, m_bEncrypted, m_bUsesPassword;

	////////////////////////////////////////////////////////////////////////////
	// database stuff
public:
	MICryptoEngine *m_crypto;

protected:
	HANDLE   hSettingChangeEvent, hContactDeletedEvent, hContactAddedEvent, hEventMarkedRead;
	DWORD    m_dwVersion;
	mir_cs   m_csDbAccess;

	int      CheckProto(DBCachedContact *cc, const char *proto);

	////////////////////////////////////////////////////////////////////////////
	// settings

	CTable    m_dbSettings;
	int       m_codePage;
	HANDLE    hService, hHook;

	////////////////////////////////////////////////////////////////////////////
	// contacts

	CTable    m_dbContacts;
	int       m_contactCount, m_dwMaxContactId;

	int       WipeContactHistory(DBContact *dbc);

	////////////////////////////////////////////////////////////////////////////
	// events

	CTable    m_dbEvents, m_dbEventsSort;
	DWORD     m_dwMaxEventId, m_tsLast;
	MEVENT    m_evLast;
	
	TreeDB::Cursor *m_evCursor;

	void      FindNextUnread(DBCachedContact *cc, DBEventSortingKey &key2);

	////////////////////////////////////////////////////////////////////////////
	// modules

	CTable    m_dbModules;
	HANDLE    m_hModHeap;
	LIST<ModuleName> m_lMods, m_lOfs;
	LIST<char> m_lResidentSettings;
	HANDLE    hEventAddedEvent, hEventDeletedEvent, hEventFilterAddedEvent;
	MCONTACT  m_hLastCachedContact;
	int       m_maxModuleID;
	ModuleName *m_lastmn;

	void      AddToList(char *name, DWORD ofs);
	DWORD     FindExistingModuleNameOfs(const char *szName);
	int       InitModuleNames(void);
	DWORD     GetModuleNameOfs(const char *szName);
	char*     GetModuleNameByOfs(DWORD ofs);

	////////////////////////////////////////////////////////////////////////////
	// checker

	int       PeekSegment(DWORD ofs, PVOID buf, int cbBytes);
	int       ReadSegment(DWORD ofs, PVOID buf, int cbBytes);
	int       ReadWrittenSegment(DWORD ofs, PVOID buf, int cbBytes);
	int       SignatureValid(DWORD ofs, DWORD dwSignature);
	void      FreeModuleChain();

	DWORD     ConvertModuleNameOfs(DWORD ofsOld);
	void      ConvertOldEvent(DBEvent*& dbei);

	int       GetContactSettingWorker(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, int isStatic);
	int       WorkSettingsChain(DBContact *dbc, int firstTime);
	int       WorkEventChain(DWORD ofsContact, DBContact *dbc, int firstTime);

	DWORD     WriteSegment(DWORD ofs, PVOID buf, int cbBytes);
	DWORD     WriteEvent(DBEvent *dbe);
	DWORD     PeekEvent(DWORD ofs, DWORD dwContactID, DBEvent &dbe);
	void      WriteOfsNextToPrevious(DWORD ofsPrev, DBContact *dbc, DWORD ofsNext);
	void      FinishUp(DWORD ofsLast, DBContact *dbc);

	DBCHeckCallback *cb;
	DWORD     sourceFileSize, ofsAggrCur;

	////////////////////////////////////////////////////////////////////////////
	// encryption

	int       InitCrypt(void);
	void      ToggleEventsEncryption(MCONTACT contactID);
	void      ToggleSettingsEncryption(MCONTACT contactID);

	void      InitDialogs();
	bool      EnterPassword(const BYTE *pKey, const size_t keyLen);
};
