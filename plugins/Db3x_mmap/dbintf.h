/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012 Miranda NG project,
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

struct ModuleName
{
	char *name;
	DWORD ofs;
};

struct CDdxMmap : public MIDatabase, public MZeroedObject
{
	CDdxMmap(const TCHAR* tszFileName);
	~CDdxMmap();

	int Load(bool bSkipInit);
	int Create(void);
	int CreateDbHeaders();
	int CheckDbHeaders();
	void DatabaseCorruption(TCHAR *text);

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

private:
	TCHAR*   m_tszProfileName;
	HANDLE   m_hDbFile;
	DBHeader m_dbHeader;
	DWORD    m_ChunkSize;
	BOOL     m_safetyMode;
	
	////////////////////////////////////////////////////////////////////////////
	// database stuff
public:	
	UINT_PTR m_flushBuffersTimerId;
	DWORD    m_flushFailTick;
	PBYTE    m_pDbCache;

private:
	PBYTE    m_pNull;
	HANDLE   m_hMap;
	DWORD    m_dwFileSize;

	CRITICAL_SECTION m_csDbAccess;

	int   CheckProto(HANDLE hContact, const char *proto);
	DWORD CreateNewSpace(int bytes);
	void  DeleteSpace(DWORD ofs, int bytes);
	DWORD ReallocSpace(DWORD ofs, int oldSize, int newSize);

	void  Map();
	void  ReMap(DWORD needed);
	void  DBMoveChunk(DWORD ofsDest, DWORD ofsSource, int bytes);
	PBYTE DBRead(DWORD ofs, int bytesRequired, int *bytesAvail);
	void  DBWrite(DWORD ofs, PVOID pData, int bytes);
	void  DBFill(DWORD ofs, int bytes);
	void  DBFlush(int setting);
	int   InitCache(void);

	__forceinline PBYTE DBRead(HANDLE hContact, int bytesRequired, int *bytesAvail)
	{	return DBRead((DWORD)hContact, bytesRequired, bytesAvail);
	}

	////////////////////////////////////////////////////////////////////////////
	// settings 

	int m_codePage;

	HANDLE m_hCacheHeap;
	HANDLE m_hLastCachedContact;
	char* m_lastSetting;
	DBCachedContactValueList *m_lastVL;

	LIST<DBCachedContactValueList> m_lContacts;
	LIST<DBCachedGlobalValue> m_lGlobalSettings;
	LIST<char> m_lSettings, m_lResidentSettings;
	HANDLE hSettingChangeEvent, hContactDeletedEvent, hContactAddedEvent;

	DWORD GetSettingsGroupOfsByModuleNameOfs(DBContact *dbc,DWORD ofsModuleName);
	char* InsertCachedSetting(const char* szName, size_t cbNameLen);
	char* GetCachedSetting(const char *szModuleName,const char *szSettingName, int moduleNameLen, int settingNameLen);
	void SetCachedVariant(DBVARIANT* s, DBVARIANT* d);
	void FreeCachedVariant(DBVARIANT* V);
	DBVARIANT* GetCachedValuePtr(HANDLE hContact, char* szSetting, int bAllocate);
	int GetContactSettingWorker(HANDLE hContact,DBCONTACTGETSETTING *dbcgs,int isStatic);

	////////////////////////////////////////////////////////////////////////////
	// contacts

	DBCachedContactValueList* AddToCachedContactList(HANDLE hContact, int index);

	////////////////////////////////////////////////////////////////////////////
	// modules

	HANDLE m_hModHeap;
	LIST<ModuleName> m_lMods, m_lOfs;
	HANDLE hEventAddedEvent, hEventDeletedEvent, hEventFilterAddedEvent;
	ModuleName *m_lastmn;

	void  AddToList(char *name, DWORD len, DWORD ofs);
	DWORD FindExistingModuleNameOfs(const char *szName);
	int   InitModuleNames(void);
	DWORD GetModuleNameOfs(const char *szName);
	char *GetModuleNameByOfs(DWORD ofs);
};

extern LIST<CDdxMmap> g_Dbs;
