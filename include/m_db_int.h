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
aint with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_DB_INT_H__
#define M_DB_INT_H__ 1

#ifndef M_CORE_H__
	#include <m_core.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// basic database checker interface

#define STATUS_MESSAGE    0
#define STATUS_WARNING    1
#define STATUS_ERROR      2
#define STATUS_FATAL      3
#define STATUS_SUCCESS    4

struct DATABASELINK;

struct DBCHeckCallback
{
	uint32_t spaceProcessed, spaceUsed;

	void (*pfnAddLogMessage)(int type, const wchar_t *ptszFormat, ...);
};

interface MIDatabaseChecker
{
	STDMETHOD_(BOOL, Start)(DBCHeckCallback *callback) PURE;
	STDMETHOD_(BOOL, CheckDb)(int phase) PURE;
	STDMETHOD_(void, Destroy)() PURE;
};

/////////////////////////////////////////////////////////////////////////////////////////
// basic database interface

struct DBCachedGlobalValue
{
	char *name;
	DBVARIANT value;
};

struct DBCachedContactValue
{
	char *name;
	DBVARIANT value;
	DBCachedContactValue *next;
};

struct DBCachedContactBase
{
	MCONTACT contactID;
	char *szProto;
	DBCachedContactValue *first, *last;

	// metacontacts
	int       nSubs;    // == -1 -> not a metacontact
	MCONTACT *pSubs;
	MCONTACT  parentID; // == 0 -> not a subcontact
	int       nDefault; // default sub number

	__forceinline bool IsMeta() const { return nSubs != -1; }
	__forceinline bool IsSub() const { return parentID != 0; }
};

#ifndef OWN_CACHED_CONTACT
struct DBCachedContact : public DBCachedContactBase {};
#else
struct DBCachedContact;
#endif

interface MIDatabaseCache : public MZeroedObject
{
	STDMETHOD_(DBCachedContact*, AddContactToCache)(MCONTACT contactID) PURE;
	STDMETHOD_(DBCachedContact*, GetCachedContact)(MCONTACT contactID) PURE;
	STDMETHOD_(DBCachedContact*, GetFirstContact)(void) PURE;
	STDMETHOD_(DBCachedContact*, GetNextContact)(MCONTACT contactID) PURE;
	STDMETHOD_(void, FreeCachedContact)(MCONTACT contactID) PURE;

	STDMETHOD_(char*, InsertCachedSetting)(const char *szName, size_t) PURE;
	STDMETHOD_(char*, GetCachedSetting)(const char *szModuleName, const char *szSettingName, size_t, size_t) PURE;
	STDMETHOD_(void, SetCachedVariant)(DBVARIANT *s, DBVARIANT *d) PURE;
	STDMETHOD_(DBVARIANT*, GetCachedValuePtr)(MCONTACT contactID, char *szSetting, int bAllocate) PURE;
};

interface MIR_APP_EXPORT MIDatabase
{
	STDMETHOD_(BOOL, IsRelational)(void) PURE;
	STDMETHOD_(void, SetCacheSafetyMode)(BOOL) PURE;

	STDMETHOD_(int, GetContactCount)(void) PURE;
	STDMETHOD_(MCONTACT, FindFirstContact)(const char *szProto = nullptr) PURE;
	STDMETHOD_(MCONTACT, FindNextContact)(MCONTACT contactID, const char *szProto = nullptr) PURE;

	STDMETHOD_(int, DeleteContact)(MCONTACT contactID) PURE;
	STDMETHOD_(MCONTACT, AddContact)(void) PURE;
	STDMETHOD_(BOOL, IsDbContact)(MCONTACT contactID) PURE;
	STDMETHOD_(int, GetContactSize)(void) PURE;

	STDMETHOD_(int, GetEventCount)(MCONTACT contactID) PURE;
	STDMETHOD_(MEVENT, AddEvent)(MCONTACT contactID, const DBEVENTINFO *dbe) PURE;
	STDMETHOD_(BOOL, DeleteEvent)(MEVENT hDbEvent) PURE;
	STDMETHOD_(BOOL, EditEvent)(MCONTACT contactID, MEVENT hDbEvent, const DBEVENTINFO *dbe) PURE;
	STDMETHOD_(int, GetBlobSize)(MEVENT hDbEvent) PURE;
	STDMETHOD_(BOOL, GetEvent)(MEVENT hDbEvent, DBEVENTINFO *dbe) PURE;
	STDMETHOD_(BOOL, MarkEventRead)(MCONTACT contactID, MEVENT hDbEvent) PURE;
	STDMETHOD_(MCONTACT, GetEventContact)(MEVENT hDbEvent) PURE;
	STDMETHOD_(MEVENT, FindFirstEvent)(MCONTACT contactID) PURE;
	STDMETHOD_(MEVENT, FindFirstUnreadEvent)(MCONTACT contactID) PURE;
	STDMETHOD_(MEVENT, FindLastEvent)(MCONTACT contactID) PURE;
	STDMETHOD_(MEVENT, FindNextEvent)(MCONTACT contactID, MEVENT hDbEvent) PURE;
	STDMETHOD_(MEVENT, FindPrevEvent)(MCONTACT contactID, MEVENT hDbEvent) PURE;

	STDMETHOD_(BOOL, DeleteModule)(MCONTACT contactID, LPCSTR szModule) PURE;
	STDMETHOD_(BOOL, EnumModuleNames)(DBMODULEENUMPROC pFunc, void *pParam) PURE;

	STDMETHOD_(BOOL, GetContactSetting)(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv) PURE;
	STDMETHOD_(BOOL, GetContactSettingStr)(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv) PURE;
	STDMETHOD_(BOOL, GetContactSettingStatic)(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv) PURE;
	STDMETHOD_(BOOL, FreeVariant)(DBVARIANT *dbv) PURE;
	STDMETHOD_(BOOL, WriteContactSetting)(MCONTACT contactID, DBCONTACTWRITESETTING *dbcws) PURE;
	STDMETHOD_(BOOL, DeleteContactSetting)(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting) PURE;
	STDMETHOD_(BOOL, EnumContactSettings)(MCONTACT contactID, DBSETTINGENUMPROC pfnEnumProc, const char *szModule, void *param) PURE;
	STDMETHOD_(BOOL, SetSettingResident)(BOOL bIsResident, const char *pszSettingName) PURE;
	STDMETHOD_(BOOL, EnumResidentSettings)(DBMODULEENUMPROC pFunc, void *pParam) PURE;
	STDMETHOD_(BOOL, IsSettingEncrypted)(LPCSTR szModule, LPCSTR szSetting) PURE;

	STDMETHOD_(BOOL, MetaDetouchSub)(DBCachedContact*, int nSub) PURE;
	STDMETHOD_(BOOL, MetaSetDefault)(DBCachedContact*) PURE;
	STDMETHOD_(BOOL, MetaMergeHistory)(DBCachedContact *ccMeta, DBCachedContact *ccSub) PURE;
	STDMETHOD_(BOOL, MetaSplitHistory)(DBCachedContact *ccMeta, DBCachedContact *ccSub) PURE;
	STDMETHOD_(BOOL, MetaRemoveSubHistory)(DBCachedContact *ccSub) PURE;

	STDMETHOD_(BOOL, Compact)(void) PURE;
	STDMETHOD_(BOOL, Backup)(LPCWSTR) PURE;
	STDMETHOD_(BOOL, Flush)(void) PURE;

	STDMETHOD_(MIDatabaseChecker*, GetChecker)(void) PURE;
	STDMETHOD_(DATABASELINK*, GetDriver)(void) PURE;

	STDMETHOD_(MEVENT, GetEventById)(LPCSTR szModule, LPCSTR szId) PURE;

	STDMETHOD_(DB::EventCursor*, EventCursor)(MCONTACT hContact, MEVENT hDbEvent) PURE;
	STDMETHOD_(DB::EventCursor*, EventCursorRev)(MCONTACT hContact, MEVENT hDbEvent) PURE;
};

/////////////////////////////////////////////////////////////////////////////////////////

#pragma warning(push)
#pragma warning(disable:4275)

struct MICryptoEngine;
struct CRYPTO_PROVIDER;

class MIR_APP_EXPORT MDatabaseCommon : public MIDatabase, public MNonCopyable
{
	HANDLE m_hLock = nullptr;

protected:
	bool m_bEncrypted = false, m_bUsesPassword = false;
	int m_codePage;

	mir_cs m_csDbAccess;
	LIST<char> m_lResidentSettings;
	MIDatabaseCache* m_cache;
	MICryptoEngine *m_crypto = nullptr;

protected:
	int  CheckProto(DBCachedContact *cc, const char *proto);
	void FillContactSettings();
	bool LockName(const wchar_t *pwszProfileName);
	void UnlockName();

	STDMETHOD_(BOOL, GetContactSettingWorker)(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, int isStatic);
	STDMETHOD_(BOOL, WriteContactSettingWorker)(MCONTACT contactID, DBCONTACTWRITESETTING &dbcws) PURE;

public:
	MDatabaseCommon();
	virtual ~MDatabaseCommon();

	__forceinline bool isEncrypted() const { return m_bEncrypted; }
	__forceinline MICryptoEngine* getCrypt() const { return m_crypto; }
	__forceinline MIDatabaseCache* getCache() const { return m_cache; }
	__forceinline bool usesPassword() const { return m_bUsesPassword; }

	void SetPassword(const wchar_t *ptszPassword);

	STDMETHODIMP_(BOOL) DeleteModule(MCONTACT contactID, LPCSTR szModule) override;

	STDMETHODIMP_(MCONTACT) FindFirstContact(const char *szProto = nullptr) override;
	STDMETHODIMP_(MCONTACT) FindNextContact(MCONTACT contactID, const char *szProto = nullptr) override;

	STDMETHODIMP_(BOOL) MetaDetouchSub(DBCachedContact *cc, int nSub) override;
	STDMETHODIMP_(BOOL) MetaSetDefault(DBCachedContact *cc) override;
	STDMETHODIMP_(BOOL) MetaRemoveSubHistory(DBCachedContact *ccSub) override;

	STDMETHODIMP_(BOOL) IsSettingEncrypted(LPCSTR szModule, LPCSTR szSetting) override;
	STDMETHODIMP_(BOOL) GetContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv) override;
	STDMETHODIMP_(BOOL) GetContactSettingStr(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv) override;
	STDMETHODIMP_(BOOL) GetContactSettingStatic(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv) override;
	STDMETHODIMP_(BOOL) FreeVariant(DBVARIANT *dbv);
	STDMETHODIMP_(BOOL) WriteContactSetting(MCONTACT contactID, DBCONTACTWRITESETTING *dbcws) override;

	STDMETHODIMP_(BOOL) EnumResidentSettings(DBMODULEENUMPROC pFunc, void *pParam) override;
	STDMETHODIMP_(BOOL) SetSettingResident(BOOL bIsResident, const char *pszSettingName) override;

	STDMETHODIMP_(BOOL) Compact(void) override;
	STDMETHODIMP_(BOOL) Backup(LPCWSTR) override;
	STDMETHODIMP_(BOOL) Flush(void) override;

	STDMETHODIMP_(MIDatabaseChecker*) GetChecker(void) override;

	STDMETHODIMP_(DB::EventCursor*) EventCursor(MCONTACT hContact, MEVENT hDbEvent) override;
	STDMETHODIMP_(DB::EventCursor*) EventCursorRev(MCONTACT hContact, MEVENT hDbEvent) override;

	////////////////////////////////////////////////////////////////////////////////////////
	// encryption support

	int InitCrypt();

	CRYPTO_PROVIDER* SelectProvider();
	STDMETHOD_(CRYPTO_PROVIDER*, ReadProvider)() PURE;
	STDMETHOD_(BOOL, StoreProvider)(CRYPTO_PROVIDER*) PURE;

	STDMETHOD_(BOOL, ReadCryptoKey)(MBinBuffer&) PURE;
	STDMETHOD_(BOOL, StoreCryptoKey)() PURE;

	STDMETHOD_(BOOL, EnableEncryption)(BOOL) PURE;
	STDMETHOD_(BOOL, ReadEncryption)() PURE;
};

#pragma warning(pop)

/////////////////////////////////////////////////////////////////////////////////////////
// Read-only database, that cannot add/modify information in a profile

class MIR_APP_EXPORT MDatabaseReadonly : public MDatabaseCommon
{
public:
	MDatabaseReadonly();

	STDMETHODIMP_(BOOL) IsRelational(void) override;

	STDMETHODIMP_(void) SetCacheSafetyMode(BOOL) override;

	STDMETHODIMP_(BOOL) EnumModuleNames(DBMODULEENUMPROC, void*) override;

	STDMETHODIMP_(CRYPTO_PROVIDER*) ReadProvider() override;
	STDMETHODIMP_(BOOL) StoreProvider(CRYPTO_PROVIDER*) override;
	STDMETHODIMP_(BOOL) ReadCryptoKey(MBinBuffer&) override;
	STDMETHODIMP_(BOOL) StoreCryptoKey() override;
	STDMETHODIMP_(BOOL) EnableEncryption(BOOL) override;
	STDMETHODIMP_(BOOL) ReadEncryption() override;

	////////////////////////////////////////////////////////////////////////////////////////
	STDMETHODIMP_(MCONTACT) AddContact(void) override;
	STDMETHODIMP_(int) DeleteContact(MCONTACT) override;
	STDMETHODIMP_(BOOL) IsDbContact(MCONTACT contactID) override;
	STDMETHODIMP_(int) GetContactSize(void) override;

	////////////////////////////////////////////////////////////////////////////////////////
	STDMETHODIMP_(MEVENT) AddEvent(MCONTACT, const DBEVENTINFO*) override;
	STDMETHODIMP_(BOOL) DeleteEvent(MEVENT) override;
	STDMETHODIMP_(BOOL) EditEvent(MCONTACT contactID, MEVENT hDbEvent, const DBEVENTINFO *dbe) override;
	STDMETHODIMP_(int) GetBlobSize(MEVENT) override;
	STDMETHODIMP_(BOOL) MarkEventRead(MCONTACT, MEVENT) override;
	STDMETHODIMP_(MCONTACT) GetEventContact(MEVENT) override;
	STDMETHODIMP_(MEVENT) FindFirstUnreadEvent(MCONTACT) override;

	////////////////////////////////////////////////////////////////////////////////////////
	STDMETHODIMP_(BOOL) GetContactSettingWorker(MCONTACT, LPCSTR, LPCSTR, DBVARIANT*, int) override;
	STDMETHODIMP_(BOOL) WriteContactSettingWorker(MCONTACT, DBCONTACTWRITESETTING&) override;
	STDMETHODIMP_(BOOL) DeleteContactSetting(MCONTACT, LPCSTR, LPCSTR) override;
	STDMETHODIMP_(BOOL) EnumContactSettings(MCONTACT, DBSETTINGENUMPROC, const char*, void*) override;

	////////////////////////////////////////////////////////////////////////////////////////
	STDMETHODIMP_(BOOL) MetaMergeHistory(DBCachedContact*, DBCachedContact*) override;
	STDMETHODIMP_(BOOL) MetaSplitHistory(DBCachedContact*, DBCachedContact*) override;
	STDMETHODIMP_(BOOL) MetaRemoveSubHistory(DBCachedContact*) override;

	////////////////////////////////////////////////////////////////////////////////////////
	STDMETHODIMP_(MEVENT) GetEventById(LPCSTR szModule, LPCSTR szId) override;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Each database plugin should register itself using this structure

// Codes for DATABASELINK functions

// grokHeader() error codes
#define EGROKPRF_NOERROR   0
#define EGROKPRF_CANTREAD  1  // can't open the profile for reading
#define EGROKPRF_UNKHEADER 2  // header not supported, not a supported profile
#define EGROKPRF_VERNEWER  3  // header correct, version in profile newer than reader/writer
#define EGROKPRF_DAMAGED   4  // header/version fine, other internal data missing, damaged.
#define EGROKPRF_OBSOLETE  5  // obsolete database version detected, requiring conversion

// makeDatabase() error codes
#define EMKPRF_CREATEFAILED 1   // for some reason CreateFile() didnt like something

#define MDB_CAPS_CREATE  0x0001 // new database can be created
#define MDB_CAPS_COMPACT 0x0002 // database can be compacted
#define MDB_CAPS_CHECK   0x0004 // database can be checked


struct DATABASELINK
{
	int capabilities;
	char* szShortName;  // uniqie short database name
	wchar_t* szFullName;  // in English, auto-translated by the core

	/*
		profile: pointer to a string which contains full path + name
		Affect: The database plugin should create the profile, the filepath will not exist at
			the time of this call, profile will be C:\..\<name>.dat
		Returns: 0 on success, non zero on failure - error contains extended error information, see EMKPRF_*
	*/
	int (*makeDatabase)(const wchar_t *profile);

	/*
		profile: [in] a null terminated string to file path of selected profile
		error: [in/out] pointer to an int to set with error if any
		Affect: Ask the database plugin if it supports the given profile, if it does it will
			return 0, if it doesnt return 1, with the error set in error -- EGROKPRF_* can be valid error
			condition, most common error would be [EGROKPRF_UNKHEADER]
		Note: Just because 1 is returned, doesnt mean the profile is not supported, the profile might be damaged
			etc.
		Returns: 0 on success, non zero on failure
	*/
	int (*grokHeader)(const wchar_t *profile);

	/*
	Affect: Tell the database to create all services/hooks that a 3.xx legacy database might support into link,
		which is a PLUGINLINK structure
	Returns: 0 on success, nonzero on failure
	*/
	MDatabaseCommon* (*Load)(const wchar_t *profile, BOOL bReadOnly);
};

/////////////////////////////////////////////////////////////////////////////////////////
// global database event handles

EXTERN_C MIR_APP_EXPORT HANDLE
	g_hevContactDeleted,   // ME_DB_CONTACT_DELETED
	g_hevContactAdded, 	  // ME_DB_CONTACT_ADDED
	g_hevSettingChanged,   // ME_DB_CONTACT_SETTINGCHANGED
	g_hevMarkedRead,       // ME_DB_EVENT_MARKED_READ
	g_hevEventAdded,		  // ME_DB_EVENT_ADDED
	g_hevEventEdited, 	  // ME_DB_EVENT_EDITED
	g_hevEventDeleted,     // ME_DB_EVENT_DELETED
	g_hevEventFiltered;    // ME_DB_EVENT_FILTER_ADD

/////////////////////////////////////////////////////////////////////////////////////////
// cache access function

EXTERN_C MIR_CORE_DLL(DBCachedContact*) db_get_contact(MCONTACT);

/////////////////////////////////////////////////////////////////////////////////////////
// database list's functions

EXTERN_C MIR_CORE_DLL(MDatabaseCommon*) db_get_current(void);
EXTERN_C MIR_CORE_DLL(void) db_setCurrent(MDatabaseCommon *_db);

// registers a database plugin
EXTERN_C MIR_APP_DLL(void) RegisterDatabasePlugin(DATABASELINK *pDescr);

// looks for a database plugin by its short name
// returns DATABASELINK* of the required plugin or nullptr on error
EXTERN_C MIR_APP_DLL(DATABASELINK*) GetDatabasePlugin(const char *pszDriverName);

// looks for a database plugin suitable to open this file
// returns DATABASELINK* of the required plugin or nullptr on error
EXTERN_C MIR_APP_DLL(DATABASELINK*) FindDatabasePlugin(const wchar_t *ptszFileName);

/////////////////////////////////////////////////////////////////////////////////////////
// database upgrader

namespace DB
{
	MIR_APP_DLL(MDatabaseCommon *) Upgrade(const wchar_t *profile);
}

#endif // M_DB_INT_H__
