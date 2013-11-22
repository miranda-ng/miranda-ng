/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012-13 Miranda NG project,
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

#define DB_THIS_VERSION          0x00000700u
#define DB_SETTINGS_RESIZE_GRANULARITY  128

#define WSOFS_END   0xFFFFFFFF
#define WS_ERROR    0xFFFFFFFF

struct DBSignature {
  char name[15];
  BYTE eof;
};

struct ModuleName
{
	char *name;
	DWORD ofs;
};

#include <pshpack1.h>
struct DBHeader {
  BYTE signature[16];      // 'Miranda ICQ DB',0,26
  DWORD version;		   //as 4 bytes, ie 1.2.3.10 = 0x0102030a
                           //this version is 0x00000700
  DWORD ofsFileEnd;		   //offset of the end of the database - place to write
                           //new structures
  DWORD slackSpace;		   //a counter of the number of bytes that have been
                           //wasted so far due to deleting structures and/or
						   //re-making them at the end. We should compact when
						   //this gets above a threshold
  DWORD contactCount;	   //number of contacts in the chain,excluding the user
  DWORD ofsFirstContact;   //offset to first DBContact in the chain
  DWORD ofsUser;		   //offset to DBContact representing the user
  DWORD ofsFirstModuleName;	//offset to first struct DBModuleName in the chain
};

#define DBCONTACT_SIGNATURE   0x43DECADEu

struct DBContact
{
  DWORD signature;
  DWORD ofsNext;			 //offset to the next contact in the chain. zero if
                             //this is the 'user' contact or the last contact
							 //in the chain
  DWORD ofsFirstSettings;	 //offset to the first DBContactSettings in the
                             //chain for this contact.
  DWORD eventCount;			 //number of events in the chain for this contact
  DWORD ofsFirstEvent,ofsLastEvent;	 //offsets to the first and last DBEvent in
                                     //the chain for this contact
  DWORD ofsFirstUnreadEvent; //offset to the first (chronological) unread event
							 //in the chain, 0 if all are read
  DWORD timestampFirstUnread; //timestamp of the event at ofsFirstUnreadEvent
};

#define DBMODULENAME_SIGNATURE  0x4DDECADEu
struct DBModuleName
{
  DWORD signature;
  DWORD ofsNext;		//offset to the next module name in the chain
  BYTE cbName;			//number of characters in this module name
  char name[1];			//name, no nul terminator
};

#define DBCONTACTSETTINGS_SIGNATURE  0x53DECADEu
struct DBContactSettings
{
  DWORD signature;
  DWORD ofsNext;		 //offset to the next contactsettings in the chain
  DWORD ofsModuleName;	 //offset to the DBModuleName of the owner of these
                         //settings
  DWORD cbBlob;			 //size of the blob in bytes. May be larger than the
                         //actual size for reducing the number of moves
						 //required using granularity in resizing
  BYTE blob[1];			 //the blob. a back-to-back sequence of DBSetting
                         //structs, the last has cbName = 0
};

#define DBEVENT_SIGNATURE  0x45DECADEu
struct DBEvent
{
	DWORD signature;
	DWORD ofsPrev,ofsNext;	 //offset to the previous and next events in the
	//chain. Chain is sorted chronologically
	DWORD ofsModuleName;		 //offset to a DBModuleName struct of the name of
	//the owner of this event
	DWORD timestamp;			 //seconds since 00:00:00 01/01/1970
	DWORD flags;				 //see m_database.h, db/event/add
	WORD eventType;			 //module-defined event type
	DWORD cbBlob;				 //number of bytes in the blob
	BYTE blob[1];				 //the blob. module-defined formatting
};

#include <poppack.h>

#define MAXCACHEDREADSIZE     65536

struct CDb3Base : public MIDatabase, public MIDatabaseChecker, public MZeroedObject
{
	CDb3Base(const TCHAR* tszFileName);
	~CDb3Base();

	int Load(bool bSkipInit);
	int Create(void);
	int CreateDbHeaders(const DBSignature&);
	int CheckDbHeaders();
	void DatabaseCorruption(TCHAR *text);

	__forceinline HANDLE getFile() const { return m_hDbFile; }

protected:
	STDMETHODIMP_(void)   SetCacheSafetyMode(BOOL);

	STDMETHODIMP_(LONG)   GetContactCount(void);
	STDMETHODIMP_(HANDLE) FindFirstContact(const char* szProto = NULL);
	STDMETHODIMP_(HANDLE) FindNextContact(HANDLE hContact, const char* szProto = NULL);
	STDMETHODIMP_(LONG)   DeleteContact(HANDLE hContact);
	STDMETHODIMP_(HANDLE) AddContact(void);
	STDMETHODIMP_(BOOL)   IsDbContact(HANDLE hContact);

	STDMETHODIMP_(LONG)   GetEventCount(HANDLE hContact);
	STDMETHODIMP_(HANDLE) AddEvent(HANDLE hContact, DBEVENTINFO *dbe);
	STDMETHODIMP_(BOOL)   DeleteEvent(HANDLE hContact, HANDLE hDbEvent);
	STDMETHODIMP_(LONG)   GetBlobSize(HANDLE hDbEvent);
	STDMETHODIMP_(BOOL)   GetEvent(HANDLE hDbEvent, DBEVENTINFO *dbe);
	STDMETHODIMP_(BOOL)   MarkEventRead(HANDLE hContact, HANDLE hDbEvent);
	STDMETHODIMP_(HANDLE) GetEventContact(HANDLE hDbEvent);
	STDMETHODIMP_(HANDLE) FindFirstEvent(HANDLE hContact);
	STDMETHODIMP_(HANDLE) FindFirstUnreadEvent(HANDLE hContact);
	STDMETHODIMP_(HANDLE) FindLastEvent(HANDLE hContact);
	STDMETHODIMP_(HANDLE) FindNextEvent(HANDLE hDbEvent);
	STDMETHODIMP_(HANDLE) FindPrevEvent(HANDLE hDbEvent);

	STDMETHODIMP_(BOOL)   EnumModuleNames(DBMODULEENUMPROC pFunc, void *pParam);

	STDMETHODIMP_(BOOL)   GetContactSetting(HANDLE hContact, DBCONTACTGETSETTING *dbcgs);
	STDMETHODIMP_(BOOL)   GetContactSettingStr(HANDLE hContact, DBCONTACTGETSETTING *dbcgs);
	STDMETHODIMP_(BOOL)   GetContactSettingStatic(HANDLE hContact, DBCONTACTGETSETTING *dbcgs);
	STDMETHODIMP_(BOOL)   FreeVariant(DBVARIANT *dbv);
	STDMETHODIMP_(BOOL)   WriteContactSetting(HANDLE hContact, DBCONTACTWRITESETTING *dbcws);
	STDMETHODIMP_(BOOL)   DeleteContactSetting(HANDLE hContact, DBCONTACTGETSETTING *dbcgs);
	STDMETHODIMP_(BOOL)   EnumContactSettings(HANDLE hContact, DBCONTACTENUMSETTINGS* dbces);
	STDMETHODIMP_(BOOL)   SetSettingResident(BOOL bIsResident, const char *pszSettingName);
	STDMETHODIMP_(BOOL)   EnumResidentSettings(DBMODULEENUMPROC pFunc, void *pParam);

protected:
	STDMETHODIMP_(BOOL)   Start(DBCHeckCallback *callback);
	STDMETHODIMP_(BOOL)   CheckDb(int phase, int firstTime);
	STDMETHODIMP_(VOID)   Destroy();

protected:
	virtual	DWORD GetSettingsGroupOfsByModuleNameOfs(DBContact *dbc,DWORD ofsContact,DWORD ofsModuleName) = 0;
	virtual	void  InvalidateSettingsGroupOfsCacheEntry(DWORD ofsSettingsGroup) {}
	virtual	int   WorkInitialCheckHeaders(void);

	virtual	void  DBMoveChunk(DWORD ofsDest, DWORD ofsSource, int bytes) = 0;
	virtual	PBYTE DBRead(DWORD ofs, int bytesRequired, int *bytesAvail) = 0;
	virtual	void  DBWrite(DWORD ofs, PVOID pData, int bytes) = 0;
	virtual	void  DBFill(DWORD ofs, int bytes) = 0;
	virtual	void  DBFlush(int setting) = 0;
	virtual	int   InitCache(void) = 0;

protected:
	int InitCrypt(void);

public:  // Check functions
	int WorkInitialChecks(int);
	int WorkModuleChain(int);
	int WorkUser(int);
	int WorkContactChain(int);
	int WorkAggressive(int);
	int WorkFinalTasks(int);

protected:
	TCHAR*   m_tszProfileName;
	HANDLE   m_hDbFile;
	DBHeader m_dbHeader;
	DWORD    m_ChunkSize;
	bool     m_safetyMode, m_bReadOnly;

	MICryptoEngine *m_crypto;

	////////////////////////////////////////////////////////////////////////////
	// database stuff
public:	
	UINT_PTR m_flushBuffersTimerId;
	DWORD    m_flushFailTick;
	PBYTE    m_pDbCache;
	HANDLE   m_hMap;

protected:
	DWORD    m_dwFileSize;
	HANDLE   hSettingChangeEvent, hContactDeletedEvent, hContactAddedEvent;

	CRITICAL_SECTION m_csDbAccess;

	int   CheckProto(HANDLE hContact, const char *proto);
	DWORD CreateNewSpace(int bytes);
	void  DeleteSpace(DWORD ofs, int bytes);
	DWORD ReallocSpace(DWORD ofs, int oldSize, int newSize);

	__forceinline PBYTE DBRead(HANDLE hContact, int bytesRequired, int *bytesAvail)
	{	return DBRead((DWORD)hContact, bytesRequired, bytesAvail);
	}

	////////////////////////////////////////////////////////////////////////////
	// settings 

	int m_codePage;

	////////////////////////////////////////////////////////////////////////////
	// modules

	HANDLE m_hModHeap;
	LIST<ModuleName> m_lMods, m_lOfs;
	LIST<char> m_lResidentSettings;
	HANDLE hEventAddedEvent, hEventDeletedEvent, hEventFilterAddedEvent;
	HANDLE m_hLastCachedContact;
	ModuleName *m_lastmn;

	void  AddToList(char *name, DWORD len, DWORD ofs);
	DWORD FindExistingModuleNameOfs(const char *szName);
	int   InitModuleNames(void);
	DWORD GetModuleNameOfs(const char *szName);
	char *GetModuleNameByOfs(DWORD ofs);

	////////////////////////////////////////////////////////////////////////////
	// checker

	int PeekSegment(DWORD ofs, PVOID buf, int cbBytes);
	int ReadSegment(DWORD ofs, PVOID buf, int cbBytes);
	int ReadWrittenSegment(DWORD ofs, PVOID buf, int cbBytes);
	int SignatureValid(DWORD ofs, DWORD signature);
	void FreeModuleChain();

	DWORD ConvertModuleNameOfs(DWORD ofsOld);
	void ConvertOldEvent(DBEvent*& dbei);

	int GetContactSettingWorker(HANDLE hContact,DBCONTACTGETSETTING *dbcgs,int isStatic);
	int WorkSettingsChain(DWORD ofsContact, DBContact *dbc, int firstTime);
	int WorkEventChain(DWORD ofsContact, DBContact *dbc, int firstTime);

	DWORD WriteSegment(DWORD ofs, PVOID buf, int cbBytes);
	DWORD WriteEvent(DBEvent *dbe);
	void WriteOfsNextToPrevious(DWORD ofsPrev,DBContact *dbc,DWORD ofsNext);
	void FinishUp(DWORD ofsLast,DBContact *dbc);

	DBCHeckCallback *cb;
	DWORD sourceFileSize, ofsAggrCur;
};

struct CDb3Mmap : public CDb3Base
{
	CDb3Mmap(const TCHAR* ptszFileName);
	~CDb3Mmap();

protected:
	virtual	DWORD GetSettingsGroupOfsByModuleNameOfs(DBContact *dbc,DWORD ofsContact,DWORD ofsModuleName);
	virtual	void  DBMoveChunk(DWORD ofsDest, DWORD ofsSource, int bytes);
	virtual	PBYTE DBRead(DWORD ofs, int bytesRequired, int *bytesAvail);
	virtual	void  DBWrite(DWORD ofs, PVOID pData, int bytes);
	virtual	void  DBFill(DWORD ofs, int bytes);
	virtual	void  DBFlush(int setting);
	virtual	int   InitCache(void);

protected:
	PBYTE m_pNull;

	void  Map();
	void  ReMap(DWORD needed);
};

typedef int (CDb3Base::*CheckWorker)(int);
