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

#ifndef _MSC_VER
#include <stdint.h>
#else
#include "stdint.h"
#endif
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


class CDataBase : public sigslot::has_slots<>
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
	CDataBase(const char* FileName);
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

};


extern CDataBase *gDataBase;
