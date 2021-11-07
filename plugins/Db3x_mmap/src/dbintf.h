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
|   |-->next contact = nullptr
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

struct DBSignature
{
	char name[15];
	BYTE eof;
};

struct ModuleName
{
	char *name;
	DWORD ofs;
};

#include <pshpack1.h>
struct DBHeader
{
	BYTE signature[16];        // 'Miranda ICQ DB',0,26
	DWORD version;          // as 4 bytes, ie 1.2.3.10 = 0x0102030a
	DWORD ofsFileEnd;       // offset of the end of the database - place to write new structures
	DWORD slackSpace;       // a counter of the number of bytes that have been
										// wasted so far due to deleting structures and/or
										// re-making them at the end. We should compact when
										// this gets above a threshold
	DWORD contactCount;     // number of contacts in the chain,excluding the user
	DWORD ofsFirstContact;  // offset to first DBContact in the chain
	DWORD ofsUser;          // offset to DBContact representing the user
	DWORD ofsModuleNames;   // offset to first struct DBModuleName in the chain
};

#define DBCONTACT_SIGNATURE   0x43DECADEu
struct DBContact
{
	DWORD signature;
	DWORD ofsNext;          // offset to the next contact in the chain. zero if
	// this is the 'user' contact or the last contact in the chain
	DWORD ofsFirstSettings;	// offset to the first DBContactSettings in the chain for this contact.
	DWORD eventCount;       // number of events in the chain for this contact
	DWORD ofsFirstEvent,    // offsets to the first and
	         ofsLastEvent;     // last DBEvent in the chain for this contact
	DWORD ofsFirstUnread;   // offset to the first (chronological) unread event	in the chain, 0 if all are read
	DWORD tsFirstUnread;    // timestamp of the event at ofsFirstUnread
	DWORD dwContactID;
};

#define DBMODULENAME_SIGNATURE  0x4DDECADEu
struct DBModuleName
{
	DWORD signature;
	DWORD ofsNext;          // offset to the next module name in the chain
	BYTE cbName;            // number of characters in this module name
	char name[1];           // name, no nul terminator
};

#define DBCONTACTSETTINGS_SIGNATURE  0x53DECADEu
struct DBContactSettings
{
	DWORD signature;
	DWORD ofsNext;          // offset to the next contactsettings in the chain
	DWORD ofsModuleName;	   // offset to the DBModuleName of the owner of these settings
	DWORD cbBlob;           // size of the blob in bytes. May be larger than the
	// actual size for reducing the number of moves
	// required using granularity in resizing
	BYTE blob[1];           // the blob. a back-to-back sequence of DBSetting
	// structs, the last has cbName = 0
};

#define DBEVENT_SIGNATURE  0x45DECADEu
struct DBEvent_094         // previous event storage format
{
	DWORD signature;
	DWORD ofsPrev, ofsNext;	// offset to the previous and next events in the
	// chain. Chain is sorted chronologically
	DWORD ofsModuleName;	   // offset to a DBModuleName struct of the name of
	// the owner of this event
	DWORD timestamp;        // seconds since 00:00:00 01/01/1970
	DWORD flags;            // see m_database.h, db/event/add
	WORD  wEventType;       // module-defined event type
	DWORD cbBlob;           // number of bytes in the blob
	BYTE  blob[1];          // the blob. module-defined formatting
};

struct DBEvent
{
	DWORD signature;
	MCONTACT contactID;     // a contact this event belongs to
	DWORD ofsPrev, ofsNext;	// offset to the previous and next events in the
	// chain. Chain is sorted chronologically
	DWORD ofsModuleName;	   // offset to a DBModuleName struct of the name of
	// the owner of this event
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

struct DBCachedContact : public DBCachedContactBase
{
	DWORD dwOfsContact;
};

struct CDb3Mmap : public MDatabaseCommon, public MZeroedObject
{
	CDb3Mmap(const wchar_t *tszFileName, int mode);
	~CDb3Mmap();

	int Load(bool bSkipInit);
	int Create(void);
	int CreateDbHeaders(const DBSignature&);
	int CheckDbHeaders(bool bInteractive);

	void DatabaseCorruption(wchar_t *text);
	void WriteSignature(DBSignature&);

	__forceinline HANDLE getFile() const { return m_hDbFile; }

public:
	STDMETHODIMP_(BOOL)     IsRelational(void) override { return FALSE; }
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

	STDMETHODIMP_(BOOL)     ReadCryptoKey(MBinBuffer&) override;
	STDMETHODIMP_(BOOL)     StoreCryptoKey(void) override;

	STDMETHODIMP_(BOOL)     EnableEncryption(BOOL) override;
	STDMETHODIMP_(BOOL)     ReadEncryption(void) override;

	STDMETHODIMP_(CRYPTO_PROVIDER*) ReadProvider() override;
	STDMETHODIMP_(BOOL)     StoreProvider(CRYPTO_PROVIDER*) override;

	STDMETHODIMP_(BOOL)     GetContactSettingWorker(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, int isStatic) override;
	STDMETHODIMP_(BOOL)     WriteContactSettingWorker(MCONTACT contactID, DBCONTACTWRITESETTING &dbcws) override;
	STDMETHODIMP_(BOOL)     DeleteContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting) override;
	STDMETHODIMP_(BOOL)     EnumContactSettings(MCONTACT hContact, DBSETTINGENUMPROC pfnEnumProc, const char *szModule, void *param) override;

	STDMETHODIMP_(BOOL)     MetaMergeHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub) override;
	STDMETHODIMP_(BOOL)     MetaSplitHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub) override;

	STDMETHODIMP_(MEVENT)   GetEventById(LPCSTR szModule, LPCSTR szId) override;

	STDMETHODIMP_(DATABASELINK*) GetDriver() override;

protected:
	DWORD GetSettingsGroupOfsByModuleNameOfs(DBContact *dbc, DWORD ofsModuleName);
	void  InvalidateSettingsGroupOfsCacheEntry(DWORD) {}

	void  DBMoveChunk(DWORD ofsDest, DWORD ofsSource, int bytes);
	PBYTE DBRead(DWORD ofs, int *bytesAvail);
	void  DBWrite(DWORD ofs, PVOID pData, int bytes);
	void  DBFill(DWORD ofs, int bytes);
	void  DBFlush(int setting);
	int   InitMap(void);
	void  FillContacts(void);

	PBYTE m_pNull;

	void  Map();
	void  ReMap(DWORD needed);

protected:
	wchar_t*   m_tszProfileName;
	HANDLE   m_hDbFile;
	DBHeader m_dbHeader;
	DWORD    m_ChunkSize;
	bool     m_safetyMode, m_bReadOnly, m_bShared;

	////////////////////////////////////////////////////////////////////////////
	// database stuff
public:
	UINT_PTR m_flushBuffersTimerId;
	DWORD    m_flushFailTick;
	PBYTE    m_pDbCache;
	HANDLE   m_hMap;

protected:
	DWORD    m_dwFileSize, m_dwMaxContactId;

	DWORD    CreateNewSpace(int bytes);
	void     DeleteSpace(DWORD ofs, int bytes);
	DWORD    ReallocSpace(DWORD ofs, int oldSize, int newSize);

	////////////////////////////////////////////////////////////////////////////
	// settings

	HANDLE   hService, hHook;

	////////////////////////////////////////////////////////////////////////////
	// contacts

	int      WipeContactHistory(DBContact *dbc);
	DWORD    GetContactOffset(MCONTACT contactID, DBCachedContact **cc = nullptr);

	////////////////////////////////////////////////////////////////////////////
	// events

	DBEvent  m_tmpEvent;

	DBEvent* AdaptEvent(DWORD offset, DWORD hContact);

	////////////////////////////////////////////////////////////////////////////
	// modules

	HANDLE   m_hModHeap;
	LIST<ModuleName> m_lMods, m_lOfs;
	MCONTACT m_hLastCachedContact;
	ModuleName *m_lastmn;

	void     AddToList(char *name, DWORD ofs);
	DWORD    FindExistingModuleNameOfs(const char *szName);
	int      InitModuleNames(void);
	DWORD    GetModuleNameOfs(const char *szName);
	char*    GetModuleNameByOfs(DWORD ofs);

	////////////////////////////////////////////////////////////////////////////
	// encryption

	int      InitCrypt(void);
	void     ToggleEventsEncryption(MCONTACT contactID);
	void     ToggleSettingsEncryption(MCONTACT contactID);
};
