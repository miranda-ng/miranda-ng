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

#include <m_db_int.h>

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

#define DB_OLD_VERSION   0x00000700u
#define DB_094_VERSION   0x00000701u
#define DB_095_VERSION   0x00000800u
#define DB_095_1_VERSION 0x00000801u

#define DB_SETTINGS_RESIZE_GRANULARITY  128

#define WSOFS_END   0xFFFFFFFF
#define WS_ERROR    0xFFFFFFFF

#define DBVT_ENCRYPTED   250
#define DBVT_UNENCRYPTED 251

#define MARKED_READ (DBEF_READ | DBEF_SENT)

#define NeedBytes(n)   if (bytesRemaining<(n)) pBlob = (PBYTE)DBRead(ofsBlobPtr,&bytesRemaining)
#define MoveAlong(n)   {int x = n; pBlob += (x); ofsBlobPtr += (x); bytesRemaining -= (x);}

DWORD __forceinline GetSettingValueLength(PBYTE pSetting)
{
	if (pSetting[0] & DBVTF_VARIABLELENGTH)
		return 2 + *(PWORD)(pSetting + 1);
	return pSetting[0];
}

struct ModuleName
{
	char *name;
	DWORD ofs;
};

#include <pshpack1.h>

struct DBSettingKey
{
	DWORD dwContactID;
	DWORD dwOfsModule;
	char  szSettingName[100];
};

#define DBCONTACT_SIGNATURE   0x43DECADEu
struct DBContact
{
	DWORD signature;
	DWORD eventCount;       // number of events in the chain for this contact
};

#define DBMODULENAME_SIGNATURE  0x4DDECADEu
struct DBModuleName
{
	DWORD signature;
	BYTE cbName;            // number of characters in this module name
	char name[1];           // name, no nul terminator
};

#define DBEVENT_SIGNATURE  0x45DECADEu
struct DBEvent
{
	DWORD signature;
	MCONTACT contactID;     // a contact this event belongs to
	DWORD ofsModuleName;	   // offset to a DBModuleName struct of the name of
	DWORD timestamp;        // seconds since 00:00:00 01/01/1970
	DWORD flags;            // see m_database.h, db/event/add
	WORD  wEventType;       // module-defined event type
	DWORD cbBlob;           // number of bytes in the blob
	BYTE  blob[1];          // the blob. module-defined formatting

	bool __forceinline markedRead() const
	{
		return (flags & MARKED_READ) != 0;
	}
};

#include <poppack.h>

struct CDbxMdb : public MIDatabase, public MIDatabaseChecker, public MZeroedObject
{
	CDbxMdb(const TCHAR *tszFileName, int mode);
	~CDbxMdb();

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
	STDMETHODIMP_(void)   SetCacheSafetyMode(BOOL);

	STDMETHODIMP_(LONG)     GetContactCount(void);
	STDMETHODIMP_(MCONTACT) FindFirstContact(const char *szProto = NULL);
	STDMETHODIMP_(MCONTACT) FindNextContact(MCONTACT contactID, const char *szProto = NULL);
	STDMETHODIMP_(LONG)     DeleteContact(MCONTACT contactID);
	STDMETHODIMP_(MCONTACT) AddContact(void);
	STDMETHODIMP_(BOOL)     IsDbContact(MCONTACT contactID);

	STDMETHODIMP_(LONG)     GetEventCount(MCONTACT contactID);
	STDMETHODIMP_(HANDLE)   AddEvent(MCONTACT contactID, DBEVENTINFO *dbe);
	STDMETHODIMP_(BOOL)     DeleteEvent(MCONTACT contactID, HANDLE hDbEvent);
	STDMETHODIMP_(LONG)     GetBlobSize(HANDLE hDbEvent);
	STDMETHODIMP_(BOOL)     GetEvent(HANDLE hDbEvent, DBEVENTINFO *dbe);
	STDMETHODIMP_(BOOL)     MarkEventRead(MCONTACT contactID, HANDLE hDbEvent);
	STDMETHODIMP_(MCONTACT) GetEventContact(HANDLE hDbEvent);
	STDMETHODIMP_(HANDLE)   FindFirstEvent(MCONTACT contactID);
	STDMETHODIMP_(HANDLE)   FindFirstUnreadEvent(MCONTACT contactID);
	STDMETHODIMP_(HANDLE)   FindLastEvent(MCONTACT contactID);
	STDMETHODIMP_(HANDLE)   FindNextEvent(MCONTACT contactID, HANDLE hDbEvent);
	STDMETHODIMP_(HANDLE)   FindPrevEvent(MCONTACT contactID, HANDLE hDbEvent);

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

	void  Map();

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
	MDB_env *m_pMdbEnv;

	HANDLE   hSettingChangeEvent, hContactDeletedEvent, hContactAddedEvent, hEventMarkedRead;

	mir_cs   m_csDbAccess;

	int      CheckProto(DBCachedContact *cc, const char *proto);

	////////////////////////////////////////////////////////////////////////////
	// settings

	MDB_dbi  m_dbSettings;
	int      m_codePage;
	HANDLE   hService, hHook;

	////////////////////////////////////////////////////////////////////////////
	// contacts

	MDB_dbi	m_dbContacts;
	int      m_contactCount, m_dwMaxContactId;

	int      WipeContactHistory(DBContact *dbc);

	////////////////////////////////////////////////////////////////////////////
	// events

	MDB_dbi	m_dbEvents;

	////////////////////////////////////////////////////////////////////////////
	// modules

	MDB_dbi	m_dbModules;
	HANDLE   m_hModHeap;
	LIST<ModuleName> m_lMods, m_lOfs;
	LIST<char> m_lResidentSettings;
	HANDLE   hEventAddedEvent, hEventDeletedEvent, hEventFilterAddedEvent;
	MCONTACT m_hLastCachedContact;
	int      m_maxModuleID;

	void     AddToList(char *name, DWORD ofs);
	DWORD    FindExistingModuleNameOfs(const char *szName);
	int      InitModuleNames(void);
	DWORD    GetModuleNameOfs(const char *szName);
	char*    GetModuleNameByOfs(DWORD ofs);

	////////////////////////////////////////////////////////////////////////////
	// checker

	int      PeekSegment(DWORD ofs, PVOID buf, int cbBytes);
	int      ReadSegment(DWORD ofs, PVOID buf, int cbBytes);
	int      ReadWrittenSegment(DWORD ofs, PVOID buf, int cbBytes);
	int      SignatureValid(DWORD ofs, DWORD signature);
	void     FreeModuleChain();

	DWORD    ConvertModuleNameOfs(DWORD ofsOld);
	void     ConvertOldEvent(DBEvent*& dbei);

	int      GetContactSettingWorker(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, int isStatic);
	int      WorkSettingsChain(DBContact *dbc, int firstTime);
	int      WorkEventChain(DWORD ofsContact, DBContact *dbc, int firstTime);

	DWORD    WriteSegment(DWORD ofs, PVOID buf, int cbBytes);
	DWORD    WriteEvent(DBEvent *dbe);
	DWORD    PeekEvent(DWORD ofs, DWORD dwContactID, DBEvent &dbe);
	void     WriteOfsNextToPrevious(DWORD ofsPrev, DBContact *dbc, DWORD ofsNext);
	void     FinishUp(DWORD ofsLast, DBContact *dbc);

	DBCHeckCallback *cb;
	DWORD    sourceFileSize, ofsAggrCur;

	////////////////////////////////////////////////////////////////////////////
	// encryption

	int      InitCrypt(void);
	void     ToggleEventsEncryption(MCONTACT contactID);
	void     ToggleSettingsEncryption(MCONTACT contactID);

	void     InitDialogs();
	bool     EnterPassword(const BYTE *pKey, const size_t keyLen);
};
