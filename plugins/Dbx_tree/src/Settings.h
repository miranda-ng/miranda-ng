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

#include "Interface.h"
#include "FileBTree.h"
#include "MREWSync.h"
#include "sigslot.h"
#include "IterationHeap.h"
#include <queue>
#include "lockfree_hashmap.h"

class CSettings;
class CSettingsTree;

#include "Entities.h"

#pragma pack(push, 1)  // push current alignment to stack, set alignment to 1 byte boundary

/**
	\brief Key Type of the SettingsBTree

	The setting names (ASCII) are hashed, so that they can easily be accessed.
**/

typedef struct TSettingKey {
	uint32_t          Hash;    /// 32 bit hash of the Setting name
	TDBTSettingHandle Setting;

	bool operator <  (const TSettingKey & Other) const
	{
		if (Hash != Other.Hash) return Hash < Other.Hash;
		if (Setting != Other.Setting) return Setting < Other.Setting;
		return false;
	}
	//bool operator <= (const TSettingKey & Other);
	bool operator == (const TSettingKey & Other) const
	{
		return (Hash == Other.Hash) && (Setting == Other.Setting);
	}
	//bool operator >= (const TSettingKey & Other);
	bool operator >  (const TSettingKey & Other) const
	{
		if (Hash != Other.Hash) return Hash > Other.Hash;
		if (Setting != Other.Setting) return Setting > Other.Setting;
		return false;
	}
} TSettingKey;

static const uint32_t cSettingSignature = 0xF5B87A3D;
static const uint16_t cSettingNodeSignature = 0xBA12;

/**
	\brief The data of a setting

	A setting's data is variable length. The data is a TSetting-structure followed by variable length data.
	- fixed data
	- SettingName (ASCII)
	- maybe blob data
**/
typedef struct TSetting {
	TDBTEntityHandle Entity;   /// Settings' Entity
	uint32_t   Flags;          /// flags
	uint16_t   Type;           /// setting type
	uint16_t   NameLength;     /// settingname length
	union {
		union {
			bool Bool;
			int8_t  Char;  uint8_t  Byte;
			int16_t Short; uint16_t Word;
			uint32_t Int;   uint32_t DWord;
			int64_t Int64; uint64_t QWord;
			float Float;
			double Double;
		} Value;  /// if type is fixed length, the data is stored right here

		struct {
			uint32_t BlobLength;   /// if type is variable length this describes the length of the data in bytes
			uint32_t AllocSize;    /// this is the allocated space for the blob ALWAYS in byte! this prevents us to realloc it too often
		};
	};
	uint8_t    Reserved[8];
	// settingname with terminating NULL
  // blob
} TSetting;

#pragma pack(pop)

/**
	\brief Manages the Settings in the Database
**/
class CSettingsTree : public CFileBTree<TSettingKey, 8>
{
protected:
	TDBTEntityHandle m_Entity;
	CSettings & m_Owner;
public:
	CSettingsTree(CSettings & Owner, CBlockManager & BlockManager, TNodeRef RootNode, TDBTEntityHandle Entity)
		:	CFileBTree<TSettingKey, 8>(BlockManager, RootNode, cSettingNodeSignature),
			m_Owner(Owner),
			m_Entity(Entity)
		{

		};
	~CSettingsTree()
		{

		};

	TDBTEntityHandle Entity()
		{
			return m_Entity;
		};
	void Entity(TDBTEntityHandle NewEntity)
		{
			m_Entity = NewEntity;
		};

	TDBTSettingHandle _FindSetting(const uint32_t Hash, const char * Name, const uint32_t Length);
	bool _DeleteSetting(const uint32_t Hash, const TDBTSettingHandle hSetting);
	bool _AddSetting(const uint32_t Hash, const TDBTSettingHandle hSetting);
};


/**
	\brief Manages all Settings and provides access to them
**/
class CSettings : public sigslot::has_slots<>
{
public:
	typedef sigslot::signal2<CSettings*, CSettingsTree::TNodeRef> TOnRootChanged;

	static const uint32_t cSettingsFileFlag = 0x00000001;

	CSettings(
		CBlockManager & BlockManagerSet,
		CBlockManager & BlockManagerPri,
		CSettingsTree::TNodeRef SettingsRoot,
		CEntities & Entities
		);
	virtual ~CSettings();


	TOnRootChanged & sigRootChanged()
		{
			return m_sigRootChanged;
		};

	bool _ReadSettingName(CBlockManager & BlockManager, TDBTSettingHandle Setting, uint16_t & NameLength, char *& NameBuf);
	void _EnsureModuleExists(char * Module);

	// compatibility:
	typedef int (*DBMODULEENUMPROC)(const char *szModuleName,DWORD ofsModuleName,LPARAM lParam);

	int CompEnumModules(DBMODULEENUMPROC CallBack, LPARAM lParam);
	// services:
	TDBTSettingHandle FindSetting(TDBTSettingDescriptor & Descriptor);
	unsigned int DeleteSetting(TDBTSettingDescriptor & Descriptor);
	unsigned int DeleteSetting(TDBTSettingHandle hSetting);
	TDBTSettingHandle WriteSetting(TDBTSetting & Setting);
	TDBTSettingHandle WriteSetting(TDBTSetting & Setting, TDBTSettingHandle hSetting);
	unsigned int ReadSetting(TDBTSetting & Setting);
	unsigned int ReadSetting(TDBTSetting & Setting, TDBTSettingHandle hSetting);


	TDBTSettingIterationHandle IterationInit(TDBTSettingIterFilter & Filter);
	TDBTSettingHandle IterationNext(TDBTSettingIterationHandle Iteration);
	unsigned int IterationClose(TDBTSettingIterationHandle Iteration);


private:

	typedef lockfree::hash_map<TDBTEntityHandle, CSettingsTree*> TSettingsTreeMap;
	
	typedef CIterationHeap<CSettingsTree::iterator> TSettingsHeap;

	CBlockManager & m_BlockManagerSet;
	CBlockManager & m_BlockManagerPri;

	CEntities & m_Entities;

	TSettingsTreeMap m_SettingsMap;

	typedef struct TSettingIterationResult {
		TDBTSettingHandle Handle;
		TDBTEntityHandle Entity;
		char * Name;
		uint16_t NameLen;
	} TSettingIterationResult;

	typedef struct TSettingIteration {
		TDBTSettingIterFilter Filter;
		uint16_t FilterNameStartLength;
		TSettingsHeap * Heap;
		std::queue<TSettingIterationResult> * Frame;
		bool LockSetting;
		bool LockPrivate;
	} TSettingIteration, *PSettingIteration;

	TOnRootChanged m_sigRootChanged;
	void onRootChanged(void* SettingsTree, CSettingsTree::TNodeRef NewRoot);

	void onDeleteSettingCallback(void * Tree, const TSettingKey & Key, uint32_t Param);
	void onDeleteSettings(CEntities * Entities, TDBTEntityHandle hEntity);
	void onMergeSettingCallback(void * Tree, const TSettingKey & Key, uint32_t Param);
	void onMergeSettings(CEntities * Entities, TDBTEntityHandle Source, TDBTEntityHandle Dest);

	CSettingsTree * getSettingsTree(TDBTEntityHandle hEntity);
	typedef lockfree::hash_multimap<uint16_t, char *> TModulesMap;
  
	TModulesMap m_Modules;

	void _LoadModules();

};
