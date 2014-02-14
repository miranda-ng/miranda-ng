/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

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

#include <stdint.h>
#include "MREWSync.h"

#include "Events.h"
#include "Settings.h"
#include "Entities.h"

#include "FileAccess.h"
#include "MappedMemory.h"
#include "DirectAccess.h"
#include "Blockmanager.h"

#include "sigslot.h"

#include "EncryptionManager.h"

typedef enum TDBFileType {
	DBFileSetting = 0,
	DBFilePrivate = 1,
	DBFileMax = 2
} TDBFileType;

static const uint8_t cFileSignature[DBFileMax][20] = {"Miranda IM Settings", "Miranda IM DataTree"};
static const uint32_t cDBVersion = 0x00000001;

static const uint32_t cHeaderBlockSignature = 0x7265491E;

#pragma pack(push, 1)  // push current alignment to stack, set alignment to 1 byte boundary

typedef struct TSettingsHeader {
	uint8_t Signature[20];          /// signature must be cSettingsHeader
	uint32_t Version;               /// internal DB version cDataBaseVersion
	uint32_t Obscure;
	TFileEncryption FileEncryption; /// Encryption Method
	uint32_t FileStructureBlock;    /// Offset of CBlockManager master block
	uint32_t FileSize;              /// Offset to the last used byte + 1		
	uint32_t Settings;              /// Offset to the SettingsBTree RootNode	
	uint8_t Reserved[256 - sizeof(TFileEncryption) - 20 - 5*sizeof(uint32_t)]; /// reserved storage
} TSettingsHeader;

typedef struct TPrivateHeader {
	uint8_t Signature[20];          /// signature must be CDataHeader
	uint32_t Version;               /// internal DB version cDataBaseVersion
	uint32_t Obscure;
	TFileEncryption FileEncryption; /// Encryption Method
	uint32_t FileStructureBlock;    /// Offset of CBlockManager master block
	uint32_t FileSize;              /// Offset to the last used byte + 1
	uint32_t RootEntity;            /// Offset to the Root CList Entity
	uint32_t Entities;              /// Offset to the EntityBTree RootNode
	uint32_t Virtuals;              /// Offset to the VirtualsBTree RootNode
	uint8_t Reserved[256 - sizeof(TFileEncryption) - 20 - 7*sizeof(uint32_t)]; /// reserved storage
} TPrivateHeader;


typedef union TGenericFileHeader {
	struct {
		uint8_t Signature[20];          /// signature must be cSettingsHeader
		uint32_t Version;               /// internal DB version cDataBaseVersion
		uint32_t Obscure;
		TFileEncryption FileEncryption; /// Encryption Method
		uint32_t FileStructureBlock;    /// Offset of CBlockManager master block
		uint32_t FileSize;              /// Offset to the last used byte + 1	
		uint8_t Reserved[256 - sizeof(TFileEncryption) - 20 - 4*sizeof(uint32_t)]; /// reserved storage
	} Gen;
	TSettingsHeader Set;
	TPrivateHeader Pri;
} TGenericFileHeader;

#pragma pack(pop)

class CDataBase : public sigslot::has_slots<>, public MIDatabase
{
private:
	TCHAR* m_FileName[DBFileMax];
	bool m_Opened;

	CBlockManager *m_BlockManager[DBFileMax];
	CFileAccess *m_FileAccess[DBFileMax];
	TGenericFileHeader * m_Header[DBFileMax];
	CEncryptionManager *m_EncryptionManager[DBFileMax];

	uint32_t m_HeaderBlock[DBFileMax];

	void onSettingsRootChanged(CSettings* Settings, CSettingsTree::TNodeRef NewRoot);
	void onVirtualsRootChanged(void* Virtuals, CVirtuals::TNodeRef NewRoot);
	void onEntitiesRootChanged(void* Entities, CEntities::TNodeRef NewRoot);
	void onFileSizeChanged(CFileAccess * File, uint32_t Size);

	bool PrivateFileExists();
	bool CreateNewFile(TDBFileType File);

	int CheckFile(TDBFileType Index);
	int LoadFile(TDBFileType Index);
protected:
	CEntities *m_Entities;
	CSettings *m_Settings;
	CEvents   *m_Events;

	void ReWriteHeader(TDBFileType Index);

public:
	CDataBase(const TCHAR* FileName);
	virtual ~CDataBase();

	int CreateDB();
	int CheckDB();
	int OpenDB();

	CEntities & getEntities()
	{
		return *m_Entities;
	}
	CSettings & getSettings()
	{
		return *m_Settings;
	}
	CEvents   & getEvents()
	{
		return *m_Events;
	}

	int getProfileName(int BufferSize, char * Buffer);
	int getProfilePath(int BufferSize, char * Buffer);

public:     // services 
	INT_PTR __cdecl  DBEntityGetRoot(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl  DBEntityChildCount(WPARAM hEntity, LPARAM lParam);
	INT_PTR __cdecl  DBEntityGetParent(WPARAM hEntity, LPARAM lParam);
	INT_PTR __cdecl  DBEntityMove(WPARAM hEntity, LPARAM hParent);
	INT_PTR __cdecl  DBEntityGetFlags(WPARAM hEntity, LPARAM lParam);
	INT_PTR __cdecl  DBEntityIterInit(WPARAM pFilter, LPARAM hParent);
	INT_PTR __cdecl  DBEntityIterNext(WPARAM hIteration, LPARAM lParam);
	INT_PTR __cdecl  DBEntityIterClose(WPARAM hIteration, LPARAM lParam);
	INT_PTR __cdecl  DBEntityDelete(WPARAM hEntity, LPARAM lParam);
	INT_PTR __cdecl  DBEntityCreate(WPARAM pEntity, LPARAM lParam);
	INT_PTR __cdecl  DBEntityGetAccount(WPARAM hEntity, LPARAM lParam);

	INT_PTR __cdecl  DBVirtualEntityCreate(WPARAM hEntity, LPARAM hParent);
	INT_PTR __cdecl  DBVirtualEntityGetParent(WPARAM hVirtualEntity, LPARAM lParam);
	INT_PTR __cdecl  DBVirtualEntityGetFirst(WPARAM hEntity, LPARAM lParam);
	INT_PTR __cdecl  DBVirtualEntityGetNext(WPARAM hVirtualEntity, LPARAM lParam);

	INT_PTR __cdecl  DBSettingFind(WPARAM pSettingDescriptor, LPARAM lParam);
	INT_PTR __cdecl  DBSettingDelete(WPARAM pSettingDescriptor, LPARAM lParam);
	INT_PTR __cdecl  DBSettingDeleteHandle(WPARAM hSetting, LPARAM lParam);
	INT_PTR __cdecl  DBSettingWrite(WPARAM pSetting, LPARAM lParam);
	INT_PTR __cdecl  DBSettingWriteHandle(WPARAM pSetting, LPARAM hSetting);
	INT_PTR __cdecl  DBSettingRead(WPARAM pSetting, LPARAM lParam);
	INT_PTR __cdecl  DBSettingReadHandle(WPARAM pSetting, LPARAM hSetting);
	INT_PTR __cdecl  DBSettingIterInit(WPARAM pFilter, LPARAM lParam);
	INT_PTR __cdecl  DBSettingIterNext(WPARAM hIteration, LPARAM lParam);
	INT_PTR __cdecl  DBSettingIterClose(WPARAM hIteration, LPARAM lParam);

	INT_PTR __cdecl  DBEventGetBlobSize(WPARAM hEvent, LPARAM lParam);
	INT_PTR __cdecl  DBEventGet(WPARAM hEvent, LPARAM pEvent);
	INT_PTR __cdecl  DBEventGetCount(WPARAM hEntity, LPARAM lParam);
	INT_PTR __cdecl  DBEventDelete(WPARAM hEvent, LPARAM lParam);
	INT_PTR __cdecl  DBEventAdd(WPARAM hEntity, LPARAM pEvent);
	INT_PTR __cdecl  DBEventMarkRead(WPARAM hEvent, LPARAM lParam);
	INT_PTR __cdecl  DBEventWriteToDisk(WPARAM hEvent, LPARAM lParam);
	INT_PTR __cdecl  DBEventGetEntity(WPARAM hEvent, LPARAM lParam);
	INT_PTR __cdecl  DBEventIterInit(WPARAM pFilter, LPARAM lParam);
	INT_PTR __cdecl  DBEventIterNext(WPARAM hIteration, LPARAM lParam);
	INT_PTR __cdecl  DBEventIterClose(WPARAM hIteration, LPARAM lParam);

	bool RegisterServices();

	typedef INT_PTR ( __cdecl CDataBase::*DbServiceFunc )( WPARAM, LPARAM );
	void CreateDbService(const char *szServiceName, DbServiceFunc pFunc);

protected:  // to be compatible with the standard Miranda databases
	STDMETHODIMP_(void)     SetCacheSafetyMode(BOOL);

	STDMETHODIMP_(LONG)     GetContactCount(void);
	STDMETHODIMP_(MCONTACT) FindFirstContact(const char* szProto = NULL);
	STDMETHODIMP_(MCONTACT) FindNextContact(MCONTACT contactID, const char* szProto = NULL);
	STDMETHODIMP_(LONG)     DeleteContact(MCONTACT contactID);
	STDMETHODIMP_(HANDLE)   AddContact(void);
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
	STDMETHODIMP_(HANDLE)   FindNextEvent(HANDLE hDbEvent);
	STDMETHODIMP_(HANDLE)   FindPrevEvent(HANDLE hDbEvent);
								   
	STDMETHODIMP_(BOOL)     EnumModuleNames(DBMODULEENUMPROC pFunc, void *pParam);
								   
	STDMETHODIMP_(BOOL)     GetContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv);
	STDMETHODIMP_(BOOL)     GetContactSettingStr(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv);
	STDMETHODIMP_(BOOL)     GetContactSettingStatic(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv);
	STDMETHODIMP_(BOOL)     FreeVariant(DBVARIANT *dbv);
	STDMETHODIMP_(BOOL)     WriteContactSetting(MCONTACT contactID, DBCONTACTWRITESETTING *dbcws);
	STDMETHODIMP_(BOOL)     DeleteContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting);
	STDMETHODIMP_(BOOL)     EnumContactSettings(MCONTACT contactID, DBCONTACTENUMSETTINGS* dbces);
	STDMETHODIMP_(BOOL)     SetSettingResident(BOOL bIsResident, const char *pszSettingName);
	STDMETHODIMP_(BOOL)     EnumResidentSettings(DBMODULEENUMPROC pFunc, void *pParam);
	STDMETHODIMP_(BOOL)     IsSettingEncrypted(LPCSTR szModule, LPCSTR szSetting);

	int CheckProto(DBCachedContact *cc, const char *proto);
};
