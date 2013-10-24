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

#include "Interface.h"
#include "Settings.h"
#include <math.h> // floor function
#include "Hash.h"

TDBTSettingHandle CSettingsTree::_FindSetting(const uint32_t Hash, const char * Name, const uint32_t Length)
{
	TSettingKey key = {0,0};
	key.Hash = Hash;
	iterator i = LowerBound(key);
	uint16_t l;

	TDBTSettingHandle res = 0;

	char * str = NULL;

	while ((res == 0) && (i) && (i->Hash == Hash))
	{
		l = Length;
		if (m_Owner._ReadSettingName(m_BlockManager, i->Setting, l, str) &&
			(strncmp(str, Name, Length) == 0))
		{
			res = i->Setting;
		} else {
			++i;
		}
	}

	free(str);

	return res;
}

bool CSettingsTree::_DeleteSetting(const uint32_t Hash, const TDBTSettingHandle hSetting)
{
	TSettingKey key = {0,0};
	key.Hash = Hash;
	iterator i = LowerBound(key);

	while ((i) && (i->Hash == Hash) && (i->Setting != hSetting))
		++i;

	if ((i) && (i->Hash == Hash))
	{
		Delete(*i);
		return true;
	}

	return false;
}

bool CSettingsTree::_AddSetting(const uint32_t Hash, const TDBTSettingHandle hSetting)
{
	TSettingKey key;
	key.Hash = Hash;
	key.Setting = hSetting;
	Insert(key);
	return true;
}

CSettings::CSettings(
		CBlockManager & BlockManagerSet,
		CBlockManager & BlockManagerPri,
		CSettingsTree::TNodeRef SettingsRoot,
		CEntities & Entities
)
:	m_BlockManagerSet(BlockManagerSet),
	m_BlockManagerPri(BlockManagerPri),
	m_Entities(Entities),
	m_SettingsMap(),
	m_sigRootChanged(),
	m_Modules()
{
	CSettingsTree * settree = new CSettingsTree(*this, m_BlockManagerSet, SettingsRoot, 0);

	settree->sigRootChanged().connect(this, &CSettings::onRootChanged);
	m_SettingsMap.insert(std::make_pair(0, settree));

	m_Entities._sigDeleteSettings().connect(this, &CSettings::onDeleteSettings);
	m_Entities._sigMergeSettings().connect (this, &CSettings::onMergeSettings);

	_LoadModules();
	_EnsureModuleExists("$Modules");
}

CSettings::~CSettings()
{
	TSettingsTreeMap::iterator it = m_SettingsMap.begin();

	while (it != m_SettingsMap.end())
	{
		delete it->second;
		++it;
	}

	TModulesMap::iterator it2 = m_Modules.begin();
	while (it2 != m_Modules.end())
	{
		delete [] it2->second;
		++it2;
	}
}


CSettingsTree * CSettings::getSettingsTree(TDBTEntityHandle hEntity)
{
	TSettingsTreeMap::iterator i = m_SettingsMap.find(hEntity);
	if (i != m_SettingsMap.end())
		return i->second;

	uint32_t root = m_Entities._getSettingsRoot(hEntity);
	if (root == DBT_INVALIDPARAM)
		return NULL;

	CSettingsTree * tree = new CSettingsTree(*this, m_BlockManagerPri, root, hEntity);
	tree->sigRootChanged().connect(this, &CSettings::onRootChanged);
	m_SettingsMap.insert(std::make_pair(hEntity, tree));

	return tree;
}

// TODO check if we need to copy the name or if we can just use the cache
inline bool CSettings::_ReadSettingName(CBlockManager & BlockManager, TDBTSettingHandle Setting, uint16_t & NameLength, char *& NameBuf)
{
	uint32_t sig = cSettingSignature;
	uint32_t size = 0;

	TSetting * setting = BlockManager.ReadBlock<TSetting>(Setting, size, sig);
	if (!setting)
		return false;

	if ((NameLength != 0) && (NameLength != setting->NameLength))
		return false;

	NameLength = setting->NameLength;
	NameBuf = (char*) realloc(NameBuf, NameLength + 1);

	memcpy(NameBuf, setting + 1, NameLength + 1);
	NameBuf[NameLength] = 0;

	return true;
}

void CSettings::_EnsureModuleExists(char * Module)
{
	if ((Module == NULL) || (*Module == 0))
		return;

	char * e = strchr(Module, '/');
	if (e)
		*e = 0;

	TModulesMap::iterator i = m_Modules.find(*((uint16_t*)Module));
	while ((i != m_Modules.end()) && (i->first == *((uint16_t*)Module)) && (strcmp(i->second, Module) != 0))
	{
		++i;
	}

	if ((i == m_Modules.end()) || (i->first != *reinterpret_cast<uint16_t*>(Module)))
	{
		size_t l = strlen(Module);
		char * tmp = new char [l + 1];
		memcpy(tmp, Module, l + 1);
		m_Modules.insert(std::make_pair(*reinterpret_cast<uint16_t*>(tmp), tmp));

		char namebuf[512];
		strcpy_s(namebuf, "$Modules/");
		strcat_s(namebuf, Module);

		TDBTSettingDescriptor desc = {0,0,0,0,0,0,0,0};
		desc.cbSize = sizeof(desc);
		desc.pszSettingName = namebuf;

		TDBTSetting set = {0,0,0,0};
		set.cbSize = sizeof(set);
		set.Descriptor = &desc;
		set.Type = DBT_ST_DWORD;

		WriteSetting(set, cSettingsFileFlag);
	}

	if (e)
		*e = '/';
}

void CSettings::_LoadModules()
{
	TDBTSettingDescriptor desc = {0,0,0,0,0,0,0,0};
	desc.cbSize = sizeof(desc);

	TDBTSettingIterFilter f = {0,0,0,0,0,0,0,0};
	f.cbSize = sizeof(f);
	f.Descriptor = &desc;
	f.NameStart = "$Modules/";

	TDBTSettingIterationHandle hiter = IterationInit(f);

	if ((hiter != 0) && (hiter != DBT_INVALIDPARAM))
	{
		TDBTSettingHandle res = IterationNext(hiter);
		while ((res != 0) && (res != DBT_INVALIDPARAM))
		{
			size_t l = strlen(desc.pszSettingName);
			char * tmp = new char [l - 9 + 1];
			memcpy(tmp, desc.pszSettingName + 9, l - 9 + 1);
			m_Modules.insert(std::make_pair(*reinterpret_cast<uint16_t*>(tmp), tmp));
			res = IterationNext(hiter);
		}

		IterationClose(hiter);
	}
}

void CSettings::onRootChanged(void* SettingsTree, CSettingsTree::TNodeRef NewRoot)
{
	if (((CSettingsTree*)SettingsTree)->Entity() == 0)
		m_sigRootChanged.emit(this, NewRoot);
	else
		m_Entities._setSettingsRoot(((CSettingsTree*)SettingsTree)->Entity(), NewRoot);
}

void CSettings::onDeleteSettingCallback(void * Tree, const TSettingKey & Key, uint32_t Param)
{
	if (Param == 0)
	{
		m_BlockManagerSet.DeleteBlock(Key.Setting);
	} else {
		m_BlockManagerPri.DeleteBlock(Key.Setting);
	}
}
void CSettings::onDeleteSettings(CEntities * Entities, TDBTEntityHandle hEntity)
{
	CSettingsTree * tree = getSettingsTree(hEntity);

	m_Entities._setSettingsRoot(hEntity, 0);

	if (tree)
	{
		CSettingsTree::TDeleteCallback callback;
		callback.connect(this, &CSettings::onDeleteSettingCallback);

		tree->DeleteTree(&callback, hEntity);

		TSettingsTreeMap::iterator i = m_SettingsMap.find(hEntity);
		delete i->second; // tree
		m_SettingsMap.erase(i);
	}
}


typedef struct TSettingMergeHelper
{
	TDBTEntityHandle Source;
	TDBTEntityHandle Dest;
	CSettingsTree * SourceTree;

} TSettingMergeHelper, *PSettingMergeHelper;


void CSettings::onMergeSettingCallback(void * Tree, const TSettingKey & Key,uint32_t Param)
{
	PSettingMergeHelper hlp = (PSettingMergeHelper)Param;

	uint16_t dnl = 0;
	char * dnb = NULL;

	_ReadSettingName(m_BlockManagerPri, Key.Setting, dnl, dnb);

	TSettingKey k = {0,0};
	k.Hash = Key.Hash;

	CSettingsTree::iterator i = hlp->SourceTree->LowerBound(k);
	TDBTSettingHandle res = 0;
	while ((res == 0) && i && (i->Hash == Key.Hash))
	{
		uint16_t snl = dnl;
		char * snb = NULL;

		if (_ReadSettingName(m_BlockManagerPri, i->Setting, snl, snb)
			&& (strcmp(dnb, snb) == 0)) // found it
		{
			res = i->Setting;
		}
	}

	if (res == 0)
	{
		hlp->SourceTree->Insert(Key);
	} else {
		hlp->SourceTree->Delete(*i);
		hlp->SourceTree->Insert(Key);
		m_BlockManagerPri.DeleteBlock(res);
	}
}

void CSettings::onMergeSettings(CEntities * Entities, TDBTEntityHandle Source, TDBTEntityHandle Dest)
{
	
	if ((Source != 0) && (Dest != 0))
	{
		LOG(logERROR, _T("Cannot Merge with global settings!\nSource %d Dest %d"), Source, Dest);
		return;
	}

	CSettingsTree * stree = getSettingsTree(Source);
	CSettingsTree * dtree = getSettingsTree(Dest);

	if (stree && dtree)
	{
		m_Entities._setSettingsRoot(Source, 0);

		stree->Entity(Dest);
		m_Entities._setSettingsRoot(Dest, stree->getRoot());

		TSettingKey key = {0,0};
		CSettingsTree::iterator it = stree->LowerBound(key);

		while (it) // transfer all source settings to new Entity
		{
			uint32_t sig = cSettingSignature;
			uint32_t size = 0;
			TSetting * tmp = m_BlockManagerPri.ReadBlock<TSetting>(it->Setting, size, sig);
			if (tmp)
			{
				tmp->Entity = Dest;
				m_BlockManagerPri.UpdateBlock(it->Setting);
			}
			++it;
		}

		// merge the dest tree into the source tree. override existing items
		// do it this way, because source tree should be much larger
		TSettingMergeHelper hlp;
		hlp.Source = Source;
		hlp.Dest = Dest;
		hlp.SourceTree = stree;

		CSettingsTree::TDeleteCallback callback;
		callback.connect(this, &CSettings::onMergeSettingCallback);
		dtree->DeleteTree(&callback, (uint32_t)&hlp);

		TSettingsTreeMap::iterator i = m_SettingsMap.find(Dest);
		delete i->second; // dtree
		i->second = stree;
		m_SettingsMap.erase(Source);

	}
}




TDBTSettingHandle CSettings::FindSetting(TDBTSettingDescriptor & Descriptor)
{
	if (Descriptor.Flags & DBT_SDF_FoundValid)
		return Descriptor.FoundHandle;

	uint32_t namelength = static_cast<uint32_t>( strlen(Descriptor.pszSettingName));
	uint32_t namehash;

	if (Descriptor.Flags & DBT_SDF_HashValid)
	{
		namehash = Descriptor.Hash;
	} else {
		namehash = Hash(Descriptor.pszSettingName, namelength);
		Descriptor.Hash = namehash;
		Descriptor.Flags = Descriptor.Flags | DBT_SDF_HashValid;
	}

	Descriptor.Flags = Descriptor.Flags & ~DBT_SDF_FoundValid;

	CSettingsTree * tree;
	TDBTSettingHandle res = 0;
	CBlockManager * file = &m_BlockManagerPri;
	if (Descriptor.Entity == 0)
		file = &m_BlockManagerSet;

	CBlockManager::ReadTransaction trans(*file);

	if ((Descriptor.Entity == 0) || (Descriptor.Options == 0))
	{
		tree = getSettingsTree(Descriptor.Entity);
		if (tree == NULL)
			return DBT_INVALIDPARAM;

		res = tree->_FindSetting(namehash, Descriptor.pszSettingName, namelength);

		if (res)
		{
			Descriptor.FoundInEntity = Descriptor.Entity;
			Descriptor.FoundHandle = res;
			Descriptor.Flags = Descriptor.Flags | DBT_SDF_FoundValid;
		}

		if (Descriptor.Entity == 0)
			res = res | cSettingsFileFlag;

		return res;
	}

	uint32_t cf = m_Entities.getFlags(Descriptor.Entity);
	if (cf == DBT_INVALIDPARAM)
		return DBT_INVALIDPARAM;

	// search the setting
	res = 0;

	TDBTEntityIterFilter f;
	f.cbSize = sizeof(f);
	if (cf & DBT_NF_IsGroup)
	{
		f.fDontHasFlags = 0;
		f.fHasFlags = DBT_NF_IsGroup;
	} else {
		f.fDontHasFlags = DBT_NF_IsGroup;
		f.fHasFlags = 0;
	}
	f.Options = Descriptor.Options;

	TDBTEntityIterationHandle i = m_Entities.IterationInit(f, Descriptor.Entity);
	if ((i == DBT_INVALIDPARAM) || (i == 0))
		return DBT_INVALIDPARAM;

	TDBTEntityHandle e = m_Entities.IterationNext(i);
	TDBTEntityHandle found = 0;
	while ((res == 0) && (e != 0))
	{
		tree = getSettingsTree(e);
		if (tree)
		{
			res = tree->_FindSetting(namehash, Descriptor.pszSettingName, namelength);
			found = e;
		}

		e = m_Entities.IterationNext(i);
	}

	m_Entities.IterationClose(i);

	if (res)
	{
		Descriptor.FoundInEntity = found;
		Descriptor.FoundHandle = res;
		Descriptor.Flags = Descriptor.Flags | DBT_SDF_FoundValid;
	}

	return res;
}

unsigned int CSettings::DeleteSetting(TDBTSettingDescriptor & Descriptor)
{
	TDBTSettingHandle hset = FindSetting(Descriptor);
	if ((hset == 0) || (hset == DBT_INVALIDPARAM))
	{
		return DBT_INVALIDPARAM;
	}

	unsigned int res = 0;
	if ((Descriptor.Flags & DBT_SDF_FoundValid) && (Descriptor.Flags & DBT_SDF_HashValid))
	{
		CBlockManager * file = &m_BlockManagerPri;

		if (Descriptor.FoundInEntity == 0)
		{
			file = &m_BlockManagerSet;
			hset = hset & ~cSettingsFileFlag;
		}

		CBlockManager::WriteTransaction trans(*file);

		uint32_t sig = cSettingSignature;
		uint32_t size = 0;
		TSetting * setting = file->ReadBlock<TSetting>(hset, size, sig);
		if (setting && (setting->Entity == Descriptor.FoundInEntity))
		{
			CSettingsTree * tree = getSettingsTree(setting->Entity);
			if (tree)
			{
				tree->_DeleteSetting(Descriptor.Hash, hset);
				file->DeleteBlock(hset);
			}
		}

	} else {
		res = DeleteSetting(hset);
	}

	return res;
}
unsigned int CSettings::DeleteSetting(TDBTSettingHandle hSetting)
{
	CBlockManager * file = &m_BlockManagerPri;

	if (hSetting & cSettingsFileFlag)
	{
		file = &m_BlockManagerSet;
		hSetting = hSetting & ~cSettingsFileFlag;
	}

	CBlockManager::WriteTransaction trans(*file);

	uint32_t sig = cSettingSignature;
	uint32_t size = 0;
	TSetting * setting = file->ReadBlock<TSetting>(hSetting, size, sig);

	if (!setting)
		return DBT_INVALIDPARAM;
	
	CSettingsTree * tree = getSettingsTree(setting->Entity);
	if (tree == NULL)
		return DBT_INVALIDPARAM;

	char * str = reinterpret_cast<char*>(setting + 1);
	tree->_DeleteSetting(Hash(str, setting->NameLength), hSetting);

	file->DeleteBlock(hSetting);

	return 0;
}
TDBTSettingHandle CSettings::WriteSetting(TDBTSetting & Setting)
{
	CBlockManager * file = &m_BlockManagerPri;
	if (Setting.Descriptor->Entity == 0)
		file = &m_BlockManagerSet;
	
	CBlockManager::WriteTransaction trans(*file);

	TDBTSettingHandle hset = FindSetting(*Setting.Descriptor);
	if (hset == DBT_INVALIDPARAM)
		return hset;

	hset = WriteSetting(Setting, hset);

	return hset;
}

TDBTSettingHandle CSettings::WriteSetting(TDBTSetting & Setting, TDBTSettingHandle hSetting)
{
	uint32_t sig = cSettingSignature;
	uint32_t size = 0;
	TSetting * setting = NULL;

	if (!hSetting && !(Setting.Descriptor && Setting.Descriptor->Entity))
		return DBT_INVALIDPARAM;

	CBlockManager * file = &m_BlockManagerPri;
	bool fileflag = false;

	if (hSetting & cSettingsFileFlag)
	{
		file = &m_BlockManagerSet;
		hSetting = hSetting & ~cSettingsFileFlag;
		fileflag = true;
	}

	CSettingsTree * tree = NULL;

	if (hSetting == 0)
	{
		if (Setting.Descriptor->Entity == 0)
		{
			file = &m_BlockManagerSet;
			fileflag = true;
		}

		CBlockManager::WriteTransaction trans(*file);
		
		if ((Setting.Descriptor) && (Setting.Descriptor->pszSettingName)) // setting needs a name
		{
			tree = getSettingsTree(Setting.Descriptor->Entity);
			_EnsureModuleExists(Setting.Descriptor->pszSettingName);
		}

	} else {
		CBlockManager::WriteTransaction trans(*file);

		setting = file->ReadBlock<TSetting>(hSetting, size, sig);

		if (setting) // check if hSetting is valid
			tree = getSettingsTree(setting->Entity);
	}

	if (tree == NULL)
		return DBT_INVALIDPARAM;

	uint32_t blobsize = 0;

	if (Setting.Type & DBT_STF_VariableLength)
	{
		switch (Setting.Type)
		{
			case DBT_ST_ANSI: case DBT_ST_UTF8:
				{
					if (Setting.Value.Length == 0)
						blobsize = static_cast<uint32_t>(strlen(Setting.Value.pAnsi) + 1);
					else
						blobsize = Setting.Value.Length;
				} break;
			case DBT_ST_WCHAR:
				{
					if (Setting.Value.Length == 0)
						blobsize = sizeof(wchar_t) * static_cast<uint32_t>(wcslen(Setting.Value.pWide) + 1);
					else
						blobsize = sizeof(wchar_t) * (Setting.Value.Length);
				} break;
			default:
				blobsize = Setting.Value.Length;
				break;
		}
	}

	size = sizeof(TSetting) + static_cast<uint32_t>(strlen(Setting.Descriptor->pszSettingName)) + 1 + blobsize;

	if (hSetting == 0) // create new setting
	{
		setting = file->CreateBlock<TSetting>(hSetting, cSettingSignature, size);

		setting->Entity = Setting.Descriptor->Entity;
		setting->Flags = 0;
		setting->AllocSize = blobsize;

		if (Setting.Descriptor && (Setting.Descriptor->Flags & DBT_SDF_HashValid))
		{
			tree->_AddSetting(Setting.Descriptor->Hash, hSetting);
		} else  {
			tree->_AddSetting(Hash(Setting.Descriptor->pszSettingName, static_cast<uint32_t>(strlen(Setting.Descriptor->pszSettingName))), hSetting);
		}

	} else {
		uint32_t tmp = 0;
		setting = file->ReadBlock<TSetting>(hSetting, tmp, sig);

		if (((Setting.Type & DBT_STF_VariableLength) == 0) && (setting->Type & DBT_STF_VariableLength))
		{ // shrink setting (variable size->fixed size)
			file->ResizeBlock(hSetting, setting, size);
		}

		if ((Setting.Type & DBT_STF_VariableLength) && ((setting->Type & DBT_STF_VariableLength) == 0))
		{ // trick it
			setting->AllocSize = 0;
		}
	}

	setting->Type = Setting.Type;
	setting->NameLength = static_cast<uint32_t>(strlen(Setting.Descriptor->pszSettingName));
	memcpy(setting + 1, Setting.Descriptor->pszSettingName, setting->NameLength + 1);

	if (Setting.Type & DBT_STF_VariableLength)
	{
		setting->AllocSize = file->ResizeBlock(hSetting, setting, size) -
				                (sizeof(TSetting) + setting->NameLength + 1);

		setting->BlobLength = blobsize;

		memcpy(reinterpret_cast<uint8_t*>(setting + 1) + setting->NameLength + 1, Setting.Value.pBlob, blobsize);
	} else {
		memset(&(setting->Value), 0, sizeof(setting->Value));
		switch (Setting.Type)
		{
			case DBT_ST_BOOL:
				setting->Value.Bool = Setting.Value.Bool; break;
			case DBT_ST_BYTE: case DBT_ST_CHAR:
				setting->Value.Byte = Setting.Value.Byte; break;
			case DBT_ST_SHORT: case DBT_ST_WORD:
				setting->Value.Short = Setting.Value.Short; break;
			case DBT_ST_INT: case DBT_ST_DWORD:
				setting->Value.Int = Setting.Value.Int; break;
			default:
				setting->Value.QWord = Setting.Value.QWord; break;
		}
	}
	
	file->UpdateBlock(hSetting);

	if (fileflag)
		hSetting = hSetting | cSettingsFileFlag;

	return hSetting;
}

unsigned int CSettings::ReadSetting(TDBTSetting & Setting)
{
	CBlockManager * file = &m_BlockManagerPri;
	if (Setting.Descriptor->Entity == 0)
		file = &m_BlockManagerSet;
	
	CBlockManager::ReadTransaction trans(*file);

	TDBTSettingHandle hset = FindSetting(*Setting.Descriptor);
	if ((hset == 0) || (hset == DBT_INVALIDPARAM))
		return DBT_INVALIDPARAM;
	
	PDBTSettingDescriptor back = Setting.Descriptor;
	Setting.Descriptor = NULL;

	if (ReadSetting(Setting, hset) == DBT_INVALIDPARAM)
		hset = DBT_INVALIDPARAM;

	Setting.Descriptor = back;

	return hset;
}

unsigned int CSettings::ReadSetting(TDBTSetting & Setting, TDBTSettingHandle hSetting)
{
	CBlockManager * file = &m_BlockManagerPri;

	if (hSetting & cSettingsFileFlag)
	{
		file = &m_BlockManagerSet;
		hSetting = hSetting & ~cSettingsFileFlag;
	}

	uint32_t sig = cSettingSignature;
	uint32_t size = 0;

	if (hSetting == 0)
		return DBT_INVALIDPARAM;

	CBlockManager::ReadTransaction trans(*file);

	TSetting * setting = file->ReadBlock<TSetting>(hSetting, size, sig);

	if (!setting)
		return DBT_INVALIDPARAM;
	
	uint8_t* str = reinterpret_cast<uint8_t*>(setting + 1) + setting->NameLength + 1;

	if (Setting.Type == 0)
	{
		Setting.Type = setting->Type;
		if (setting->Type & DBT_STF_VariableLength)
		{
			Setting.Value.Length = setting->BlobLength;
			switch (setting->Type)
			{
				case DBT_ST_WCHAR:
				{
					Setting.Value.Length = setting->BlobLength / sizeof(wchar_t);
					Setting.Value.pWide = (wchar_t*) mir_realloc(Setting.Value.pWide, sizeof(wchar_t) * Setting.Value.Length);
					memcpy(Setting.Value.pWide, str, setting->BlobLength);
					Setting.Value.pWide[Setting.Value.Length - 1] = 0;

				} break;
				case DBT_ST_ANSI: case DBT_ST_UTF8:
				{
					Setting.Value.Length = setting->BlobLength;
					Setting.Value.pAnsi = (char *) mir_realloc(Setting.Value.pAnsi, setting->BlobLength);
					memcpy(Setting.Value.pAnsi, str, setting->BlobLength);
					Setting.Value.pAnsi[Setting.Value.Length - 1] = 0;

				} break;
				default:
				{
					Setting.Value.Length = setting->BlobLength;
					Setting.Value.pBlob = (uint8_t *) mir_realloc(Setting.Value.pBlob, setting->BlobLength);
					memcpy(Setting.Value.pBlob, str, setting->BlobLength);
				} break;
			}
		} else {
			Setting.Value.QWord = setting->Value.QWord;
		}
	} else {
		switch (setting->Type)
		{
			case DBT_ST_BYTE: case DBT_ST_WORD: case DBT_ST_DWORD: case DBT_ST_QWORD:
				{
					switch (Setting.Type)
					{
						case DBT_ST_BYTE:  Setting.Value.Byte  = (uint8_t)   setting->Value.QWord; break;
						case DBT_ST_WORD:  Setting.Value.Word  = (uint16_t)  setting->Value.QWord; break;
						case DBT_ST_DWORD: Setting.Value.DWord = (uint32_t)  setting->Value.QWord; break;
						case DBT_ST_QWORD: Setting.Value.QWord = (uint64_t)  setting->Value.QWord; break;
						case DBT_ST_CHAR:  Setting.Value.Char  = ( int8_t)   setting->Value.QWord; break;
						case DBT_ST_SHORT: Setting.Value.Short = ( int16_t)  setting->Value.QWord; break;
						case DBT_ST_INT:   Setting.Value.Int   = ( int32_t)  setting->Value.QWord; break;
						case DBT_ST_INT64: Setting.Value.Int64 = ( int64_t)  setting->Value.QWord; break;
						case DBT_ST_BOOL:  Setting.Value.Bool  = setting->Value.QWord != 0; break;

						case DBT_ST_ANSI: case DBT_ST_UTF8:
							{
								char buffer[24];
								buffer[0] = 0;
								Setting.Value.Length = 1 + mir_snprintf(buffer, SIZEOF(buffer), "%llu", setting->Value.QWord);
								Setting.Value.pAnsi = (char *) mir_realloc(Setting.Value.pAnsi, Setting.Value.Length);
								memcpy(Setting.Value.pAnsi, buffer, Setting.Value.Length);

							} break;
						case DBT_ST_WCHAR:
							{
								wchar_t buffer[24];
								buffer[0] = 0;
								Setting.Value.Length = 1 + mir_snwprintf(buffer, SIZEOF(buffer), L"%llu", setting->Value.QWord);
								Setting.Value.pWide = (wchar_t *) mir_realloc(Setting.Value.pWide, Setting.Value.Length * sizeof(wchar_t));
								memcpy(Setting.Value.pWide, buffer, Setting.Value.Length * sizeof(wchar_t));

							} break;
						case DBT_ST_BLOB:
							{
								Setting.Value.Length = 0;
								switch (setting->Type)
								{
									case DBT_ST_BYTE:  Setting.Value.Length = 1; break;
									case DBT_ST_WORD:  Setting.Value.Length = 2; break;
									case DBT_ST_DWORD: Setting.Value.Length = 4; break;
									case DBT_ST_QWORD: Setting.Value.Length = 8; break;
								}

								Setting.Value.pBlob = (uint8_t *) mir_realloc(Setting.Value.pBlob, Setting.Value.Length);
								memcpy(Setting.Value.pBlob, &setting->Value, Setting.Value.Length);


							} break;
					}

				} break;
			case DBT_ST_CHAR: case DBT_ST_SHORT: case DBT_ST_INT: case DBT_ST_INT64:
				{
					int64_t val = 0;
					switch (setting->Type)
					{
						case DBT_ST_CHAR:  val = setting->Value.Char;  break;
						case DBT_ST_SHORT: val = setting->Value.Short; break;
						case DBT_ST_INT:   val = setting->Value.Int;   break;
						case DBT_ST_INT64: val = setting->Value.Int64; break;
					}
					switch (Setting.Type)
					{
						case DBT_ST_BYTE:  Setting.Value.Byte  = (uint8_t)   val; break;
						case DBT_ST_WORD:  Setting.Value.Word  = (uint16_t)  val; break;
						case DBT_ST_DWORD: Setting.Value.DWord = (uint32_t)  val; break;
						case DBT_ST_QWORD: Setting.Value.QWord = (uint64_t)  val; break;
						case DBT_ST_CHAR:  Setting.Value.Char  = ( int8_t)   val; break;
						case DBT_ST_SHORT: Setting.Value.Short = ( int16_t)  val; break;
						case DBT_ST_INT:   Setting.Value.Int   = ( int32_t)  val; break;
						case DBT_ST_INT64: Setting.Value.Int64 = ( int64_t)  val; break;
						case DBT_ST_BOOL:  Setting.Value.Bool  = val != 0; break;

						case DBT_ST_ANSI: case DBT_ST_UTF8:
							{
								char buffer[24];
								buffer[0] = 0;
								Setting.Value.Length = 1 + mir_snprintf(buffer, SIZEOF(buffer), "%lli", val);
								Setting.Value.pAnsi = (char *) mir_realloc(Setting.Value.pAnsi, Setting.Value.Length);
								memcpy(Setting.Value.pAnsi, buffer, Setting.Value.Length);

							} break;
						case DBT_ST_WCHAR:
							{
								wchar_t buffer[24];
								buffer[0] = 0;
								Setting.Value.Length = 1 + mir_snwprintf(buffer, SIZEOF(buffer), L"%lli", val);
								Setting.Value.pWide = (wchar_t *) mir_realloc(Setting.Value.pWide, Setting.Value.Length * sizeof(wchar_t));
								memcpy(Setting.Value.pWide, buffer, Setting.Value.Length * sizeof(wchar_t));

							} break;
						case DBT_ST_BLOB:
							{
								Setting.Value.Length = 0;
								switch (setting->Type)
								{
									case DBT_ST_CHAR:  Setting.Value.Length = 1; break;
									case DBT_ST_SHORT: Setting.Value.Length = 2; break;
									case DBT_ST_INT:   Setting.Value.Length = 4; break;
									case DBT_ST_INT64: Setting.Value.Length = 8; break;
								}

								Setting.Value.pBlob = (unsigned char *) mir_realloc(Setting.Value.pBlob, Setting.Value.Length);
								memcpy(Setting.Value.pBlob, &setting->Value, Setting.Value.Length);

							} break;
					}

				} break;
			case DBT_ST_FLOAT: case DBT_ST_DOUBLE:
				{
					double val = 0;
					if (setting->Type == DBT_ST_DOUBLE)
						val = setting->Value.Double;
					else
						val = setting->Value.Float;

					switch (Setting.Type)
					{
						case DBT_ST_BYTE:  Setting.Value.Byte  = (uint8_t)   floor(val); break;
						case DBT_ST_WORD:  Setting.Value.Word  = (uint16_t)  floor(val); break;
						case DBT_ST_DWORD: Setting.Value.DWord = (uint32_t)  floor(val); break;
						case DBT_ST_QWORD: Setting.Value.QWord = (uint64_t)  floor(val); break;
						case DBT_ST_CHAR:  Setting.Value.Char  = ( int8_t)   floor(val); break;
						case DBT_ST_SHORT: Setting.Value.Short = ( int16_t)  floor(val); break;
						case DBT_ST_INT:   Setting.Value.Int   = ( int32_t)  floor(val); break;
						case DBT_ST_INT64: Setting.Value.Int64 = ( int64_t)  floor(val); break;
						case DBT_ST_BOOL:  Setting.Value.Bool  = val != 0; break;

						case DBT_ST_ANSI: case DBT_ST_UTF8:
							{
								char buffer[128];
								buffer[0] = 0;
								Setting.Value.Length = 1 + mir_snprintf(buffer, SIZEOF(buffer), "%lf", setting->Value.QWord);
								Setting.Value.pAnsi = (char *) mir_realloc(Setting.Value.pAnsi, Setting.Value.Length);
								memcpy(Setting.Value.pAnsi, buffer, Setting.Value.Length);
							} break;
						case DBT_ST_WCHAR:
							{
								wchar_t buffer[128];
								buffer[0] = 0;
								Setting.Value.Length = 1 + mir_snwprintf(buffer, SIZEOF(buffer), L"%lf", setting->Value.QWord);
								Setting.Value.pWide = (wchar_t *) mir_realloc(Setting.Value.pWide, Setting.Value.Length * sizeof(wchar_t));
								memcpy(Setting.Value.pWide, buffer, Setting.Value.Length * sizeof(wchar_t));
							} break;
						case DBT_ST_BLOB:
							{
								Setting.Value.Length = 4;
								if (setting->Type == DBT_ST_DOUBLE)
									Setting.Value.Length = 8;

								Setting.Value.pBlob = (uint8_t*) mir_realloc(Setting.Value.pBlob, Setting.Value.Length);
								memcpy(Setting.Value.pBlob, &setting->Value, Setting.Value.Length);

							} break;
					}

				} break;
			case DBT_ST_BOOL:
				{
					switch (Setting.Type)
					{
						case DBT_ST_BYTE: case DBT_ST_WORD: case DBT_ST_DWORD: case DBT_ST_QWORD:
						case DBT_ST_CHAR: case DBT_ST_SHORT: case DBT_ST_INT: case DBT_ST_INT64:
							{
								if (setting->Value.Bool)
									Setting.Value.QWord = 1;
								else
									Setting.Value.QWord = 0;
							} break;
						case DBT_ST_FLOAT:
							{
								if (setting->Value.Bool)
									Setting.Value.Float = 1;
								else
									Setting.Value.Float = 0;
							} break;
						case DBT_ST_DOUBLE:
							{
								if (setting->Value.Bool)
									Setting.Value.Double = 1;
								else
									Setting.Value.Double = 0;
							} break;
						case DBT_ST_ANSI: case DBT_ST_UTF8:
							{
								char * buffer = "false";
								Setting.Value.Length = 5;
								if (setting->Value.Bool)
								{
									buffer = "true";
									Setting.Value.Length = 4;
								}

								Setting.Value.pAnsi = (char *) mir_realloc(Setting.Value.pAnsi, Setting.Value.Length);
								memcpy(Setting.Value.pAnsi, buffer, Setting.Value.Length);
							} break;
						case DBT_ST_WCHAR:
							{
								wchar_t * buffer = L"false";
								Setting.Value.Length = 5;
								if (setting->Value.Bool)
								{
									buffer = L"true";
									Setting.Value.Length = 4;
								}

								Setting.Value.pWide = (wchar_t *) mir_realloc(Setting.Value.pWide, Setting.Value.Length * sizeof(wchar_t));
								memcpy(Setting.Value.pWide, buffer, Setting.Value.Length * sizeof(wchar_t));
							} break;
						case DBT_ST_BLOB:
							{
								Setting.Value.pBlob = (uint8_t*) mir_realloc(Setting.Value.pBlob, sizeof(bool));
								(*((bool*)Setting.Value.pBlob)) = setting->Value.Bool;
								Setting.Value.Length = sizeof(bool);
							} break;
					}
				} break;
			case DBT_ST_ANSI:
				{
					str[setting->BlobLength - 1] = 0;

					switch (Setting.Type)
					{
						case DBT_ST_BYTE: case DBT_ST_WORD: case DBT_ST_DWORD: case DBT_ST_QWORD: case DBT_ST_BOOL:
						case DBT_ST_CHAR: case DBT_ST_SHORT: case DBT_ST_INT: case DBT_ST_INT64:
							{
								Setting.Value.QWord = 0;
							} break;
						case DBT_ST_ANSI:
							{
								Setting.Value.Length = setting->BlobLength;
								Setting.Value.pAnsi = (char *) mir_realloc(Setting.Value.pAnsi, setting->BlobLength);
								memcpy(Setting.Value.pAnsi, str, setting->BlobLength);
							} break;
						case DBT_ST_UTF8:
							{
								Setting.Value.pUTF8 = mir_utf8encode((char*)str);
								Setting.Value.Length = static_cast<uint32_t>(strlen(Setting.Value.pUTF8) + 1);
							} break;
						case DBT_ST_WCHAR:
							{
								Setting.Value.pWide = mir_a2u((char*)str);
								Setting.Value.Length = static_cast<uint32_t>(wcslen(Setting.Value.pWide) + 1);
							} break;
						case DBT_ST_BLOB:
							{
								Setting.Value.Length = setting->BlobLength;
								Setting.Value.pBlob = (uint8_t *) mir_realloc(Setting.Value.pBlob, setting->BlobLength);
								memcpy(Setting.Value.pBlob, str, setting->BlobLength);
							} break;
					}
				} break;
			case DBT_ST_UTF8:
				{
					str[setting->BlobLength - 1] = 0;

					switch (Setting.Type)
					{
						case DBT_ST_BYTE: case DBT_ST_WORD: case DBT_ST_DWORD: case DBT_ST_QWORD: case DBT_ST_BOOL:
						case DBT_ST_CHAR: case DBT_ST_SHORT: case DBT_ST_INT: case DBT_ST_INT64:
							{
								Setting.Value.QWord = 0;
							} break;
						case DBT_ST_ANSI:
							{
								mir_utf8decode((char*)str, NULL);
								Setting.Value.Length = static_cast<uint32_t>(strlen((char*)str) + 1);
								Setting.Value.pAnsi = (char *) mir_realloc(Setting.Value.pAnsi, Setting.Value.Length);
								memcpy(Setting.Value.pAnsi, str, Setting.Value.Length);
							} break;
						case DBT_ST_UTF8:
							{
								Setting.Value.Length = setting->BlobLength;
								Setting.Value.pUTF8 = (char *) mir_realloc(Setting.Value.pUTF8, setting->BlobLength);
								memcpy(Setting.Value.pUTF8, str, setting->BlobLength);
							} break;
						case DBT_ST_WCHAR:
							{
								Setting.Value.pWide = mir_utf8decodeW((char*)str);
								if (Setting.Value.pWide)
								{
									Setting.Value.Length = static_cast<uint32_t>(wcslen(Setting.Value.pWide) + 1);
								} else {
									Setting.Value.Length = 0;
									Setting.Type = 0;
								}
							} break;
						case DBT_ST_BLOB:
							{
								Setting.Value.pBlob = (uint8_t *) mir_realloc(Setting.Value.pBlob, setting->BlobLength);
								memcpy(Setting.Value.pBlob, str, setting->BlobLength);
								Setting.Value.Length = setting->BlobLength;
							} break;
					}
				} break;
			case DBT_ST_WCHAR:
				{
					((wchar_t*)str)[setting->BlobLength / sizeof(wchar_t) - 1] = 0;

					switch (Setting.Type)
					{
						case DBT_ST_BYTE: case DBT_ST_WORD: case DBT_ST_DWORD: case DBT_ST_QWORD: case DBT_ST_BOOL:
						case DBT_ST_CHAR: case DBT_ST_SHORT: case DBT_ST_INT: case DBT_ST_INT64:
							{
								Setting.Value.QWord = 0;
							} break;
						case DBT_ST_ANSI:
							{
								Setting.Value.pAnsi = mir_u2a((wchar_t*)str);
								Setting.Value.Length = static_cast<uint32_t>(strlen(Setting.Value.pAnsi) + 1);
							} break;
						case DBT_ST_UTF8:
							{
								Setting.Value.pUTF8 = mir_utf8encodeW((wchar_t*)str);
								Setting.Value.Length = static_cast<uint32_t>(strlen(Setting.Value.pUTF8) + 1);
							} break;
						case DBT_ST_WCHAR:
							{
								Setting.Value.Length = setting->BlobLength / sizeof(wchar_t);
								Setting.Value.pWide = (wchar_t*) mir_realloc(Setting.Value.pWide, Setting.Value.Length * sizeof(wchar_t));
								memcpy(Setting.Value.pWide, str, Setting.Value.Length * sizeof(wchar_t));
							} break;
						case DBT_ST_BLOB:
							{
								Setting.Value.pBlob = (uint8_t *) mir_realloc(Setting.Value.pBlob, setting->BlobLength);
								memcpy(Setting.Value.pBlob, str, setting->BlobLength);
								Setting.Value.Length = setting->BlobLength;
							} break;
					}
				} break;
			case DBT_ST_BLOB:
				{
					switch (Setting.Type)
					{
						case DBT_ST_BYTE: case DBT_ST_WORD: case DBT_ST_DWORD: case DBT_ST_QWORD: case DBT_ST_BOOL:
						case DBT_ST_CHAR: case DBT_ST_SHORT: case DBT_ST_INT: case DBT_ST_INT64:
							{
								Setting.Value.QWord = 0;
							} break;
						case DBT_ST_ANSI: case DBT_ST_WCHAR: case DBT_ST_UTF8:
							{
								Setting.Value.Length = 0;
								if (Setting.Value.pBlob)
									mir_free(Setting.Value.pBlob);

								Setting.Value.pBlob = NULL;
							} break;
						case DBT_ST_BLOB:
							{
								Setting.Value.pBlob = (uint8_t *) mir_realloc(Setting.Value.pBlob, setting->BlobLength);
								memcpy(Setting.Value.pBlob, str, setting->BlobLength);
								Setting.Value.Length = setting->BlobLength;
							} break;
					}
				} break;

		}
	}


	if (Setting.Descriptor)
	{
		Setting.Descriptor->Entity = setting->Entity;
		Setting.Descriptor->FoundInEntity = setting->Entity;

		Setting.Descriptor->pszSettingName = (char *) mir_realloc(Setting.Descriptor->pszSettingName, setting->NameLength + 1);
		memcpy(Setting.Descriptor->pszSettingName, setting + 1, setting->NameLength + 1);
		Setting.Descriptor->pszSettingName[setting->NameLength] = 0;
	}

	return setting->Type;
}




TDBTSettingIterationHandle CSettings::IterationInit(TDBTSettingIterFilter & Filter)
{
	CBlockManager::ReadTransaction transset(m_BlockManagerSet);
	CBlockManager::ReadTransaction transpri(m_BlockManagerPri);
	
	std::queue<TDBTEntityHandle> Entities;
	Entities.push(Filter.hEntity);

	CSettingsTree * tree = getSettingsTree(Filter.hEntity);

	if (tree == NULL)
		return DBT_INVALIDPARAM;
	
	if (Filter.hEntity != 0)
	{
		uint32_t cf = m_Entities.getFlags(Filter.hEntity);

		if (cf == DBT_INVALIDPARAM)
			return DBT_INVALIDPARAM;
		
		TDBTEntityIterFilter f = {0,0,0,0};
		f.cbSize = sizeof(f);
		if (cf & DBT_NF_IsGroup)
		{
			f.fHasFlags = DBT_NF_IsGroup;
		} else {
			f.fDontHasFlags = DBT_NF_IsGroup;
		}
		f.Options = Filter.Options;

		TDBTEntityIterationHandle citer = m_Entities.IterationInit(f, Filter.hEntity);
		if (citer != DBT_INVALIDPARAM)
		{
			m_Entities.IterationNext(citer); // the initial Entity was already added
			TDBTEntityHandle e = m_Entities.IterationNext(citer);
			while (e != 0)
			{
				Entities.push(e);
				e = m_Entities.IterationNext(citer);
			}

			m_Entities.IterationClose(citer);
		}
	}

	for (unsigned int j = 0; j < Filter.ExtraCount; ++j)
		Entities.push(Filter.ExtraEntities[j]);
	

	PSettingIteration iter = new TSettingIteration;
	iter->Filter = Filter;
	iter->FilterNameStartLength = 0;
	iter->LockSetting = (Filter.hEntity == 0);
	iter->LockPrivate = (Filter.hEntity != 0);
	if (Filter.NameStart)
	{
		uint16_t l = static_cast<uint32_t>(strlen(Filter.NameStart));
		iter->Filter.NameStart = new char[l + 1];
		memcpy(iter->Filter.NameStart, Filter.NameStart, l + 1);
		iter->FilterNameStartLength = l;
	}

	TSettingKey key = {0, 0};

	// pop first Entity. we have always one and always its tree
	Entities.pop();

	CSettingsTree::iterator * tmp = new CSettingsTree::iterator(tree->LowerBound(key));
	tmp->setManaged();
	iter->Heap = new TSettingsHeap(*tmp, TSettingsHeap::ITForward, true);

	while (!Entities.empty())
	{
		TDBTEntityHandle e = Entities.front();
		Entities.pop();

		tree = getSettingsTree(e);
		if (tree != NULL)
		{
			tmp = new CSettingsTree::iterator(tree->LowerBound(key));
			tmp->setManaged();
			iter->Heap->Insert(*tmp);

			iter->LockSetting = iter->LockSetting || (e == 0);
			iter->LockPrivate = iter->LockPrivate || (e != 0);
		}
	}

	iter->Frame = new std::queue<TSettingIterationResult>;

	return reinterpret_cast<TDBTSettingIterationHandle>(iter);
}


typedef struct TSettingIterationHelper {
		TDBTSettingHandle Handle;
		CSettingsTree * Tree;
		uint16_t NameLen;
		char * Name;
	} TSettingIterationHelper;

TDBTSettingHandle CSettings::IterationNext(TDBTSettingIterationHandle Iteration)
{
	PSettingIteration iter = reinterpret_cast<PSettingIteration>(Iteration);
	CBlockManager::ReadTransaction transset;
	CBlockManager::ReadTransaction transpri;

	if (iter->LockSetting)
		transset = CBlockManager::ReadTransaction(m_BlockManagerSet);
	if (iter->LockPrivate)
		transpri = CBlockManager::ReadTransaction(m_BlockManagerPri);

	while (iter->Frame->empty() && iter->Heap->Top())
	{
		while (iter->Heap->Top() && iter->Heap->Top().wasDeleted())
			iter->Heap->Pop();

		if (iter->Heap->Top())
		{
			uint32_t h = iter->Heap->Top()->Hash;
			std::queue<TSettingIterationHelper> q;
			TSettingIterationHelper help;
			help.NameLen = 0;
			help.Name = NULL;

			help.Handle = iter->Heap->Top()->Setting;
			help.Tree = (CSettingsTree *) iter->Heap->Top().Tree();
			if (help.Tree)
				q.push(help);

			iter->Heap->Pop();

			// add all candidates
			while (iter->Heap->Top() && (iter->Heap->Top()->Hash == h))
			{
				if (!iter->Heap->Top().wasDeleted())
				{
					help.Handle = iter->Heap->Top()->Setting;
					help.Tree = (CSettingsTree *) iter->Heap->Top().Tree();
					q.push(help);
				}
				iter->Heap->Pop();
			}

			while (!q.empty())
			{
				help = q.front();
				q.pop();

				if (help.Name == NULL)
				{
					if (help.Tree->Entity() == 0)
						_ReadSettingName(m_BlockManagerSet, help.Handle, help.NameLen, help.Name);
					else
						_ReadSettingName(m_BlockManagerPri, help.Handle, help.NameLen, help.Name);
				}


				q.push(help);
				while (q.front().Handle != help.Handle)  // remove all queued settings with same name
				{
					bool namereadres = false;

					TSettingIterationHelper tmp;
					tmp = q.front();
					q.pop();

					if (tmp.Name == NULL)
					{
						if (tmp.Tree->Entity() == 0)
							namereadres = _ReadSettingName(m_BlockManagerSet, tmp.Handle, tmp.NameLen, tmp.Name);
						else
							namereadres = _ReadSettingName(m_BlockManagerPri, tmp.Handle, tmp.NameLen, tmp.Name);
					}

					if (!namereadres)
					{
						q.push(tmp);
					} else {
						if (strcmp(tmp.Name, help.Name) != 0)
						{
							q.push(tmp);
						} else {
							free(tmp.Name);
						}
					}
				}

				// namefilter
				if ((iter->Filter.NameStart == NULL) || ((iter->FilterNameStartLength <= help.NameLen) && (memcmp(iter->Filter.NameStart, help.Name, iter->FilterNameStartLength) == 0)))
				{
					TSettingIterationResult tmp;
					if (help.Tree->Entity() == 0)
						help.Handle |= cSettingsFileFlag;

					tmp.Handle = help.Handle;
					tmp.Entity = help.Tree->Entity();
					tmp.Name = help.Name;
					tmp.NameLen = help.NameLen;
					iter->Frame->push(tmp);
				} else {
					free(help.Name);
				}

				q.pop();
			}
		}
	}


	TSettingIterationResult res = {0,0,0,0};
	if (!iter->Frame->empty())
	{
		res = iter->Frame->front();
		iter->Frame->pop();

		if ((iter->Filter.Descriptor) && ((iter->Filter.Setting == NULL) || (iter->Filter.Setting->Descriptor != iter->Filter.Descriptor)))
		{
			iter->Filter.Descriptor->Entity = res.Entity;
			iter->Filter.Descriptor->pszSettingName = (char *) mir_realloc(iter->Filter.Descriptor->pszSettingName, res.NameLen + 1);
			memcpy(iter->Filter.Descriptor->pszSettingName, res.Name, res.NameLen + 1);
			iter->Filter.Descriptor->FoundInEntity = res.Entity;
		}
		if (iter->Filter.Setting)
		{
			if ((iter->Filter.Setting->Type & DBT_STF_VariableLength) && (iter->Filter.Setting->Value.pBlob))
			{
				mir_free(iter->Filter.Setting->Value.pBlob);
				iter->Filter.Setting->Value.pBlob = NULL;
			}
			iter->Filter.Setting->Type = 0;

			ReadSetting(*iter->Filter.Setting, res.Handle);
		}

		free(res.Name);
	}
	
	return res.Handle;
}
unsigned int CSettings::IterationClose(TDBTSettingIterationHandle Iteration)
{
	PSettingIteration iter = reinterpret_cast<PSettingIteration>(Iteration);
	{
		CBlockManager::ReadTransaction transset;
		CBlockManager::ReadTransaction transpri;

		if (iter->LockSetting)
			transset = CBlockManager::ReadTransaction(m_BlockManagerSet);
		if (iter->LockPrivate)
			transpri = CBlockManager::ReadTransaction(m_BlockManagerPri);

		delete iter->Heap; // only this needs synchronization
	}

	delete [] iter->Filter.NameStart;

	if (iter->Filter.Descriptor && iter->Filter.Descriptor->pszSettingName)
	{
		mir_free(iter->Filter.Descriptor->pszSettingName);
		iter->Filter.Descriptor->pszSettingName = NULL;
	}
	if (iter->Filter.Setting)
	{
		if (iter->Filter.Setting->Descriptor)
		{
			mir_free(iter->Filter.Setting->Descriptor->pszSettingName);
			iter->Filter.Setting->Descriptor->pszSettingName = NULL;
		}

		if (iter->Filter.Setting->Type & DBT_STF_VariableLength)
		{
			mir_free(iter->Filter.Setting->Value.pBlob);
			iter->Filter.Setting->Value.pBlob = NULL;
		}
	}

	while (!iter->Frame->empty())
	{
		free(iter->Frame->front().Name);

		iter->Frame->pop();
	}
	delete iter->Frame;
	delete iter;

	return 0;
}


int CSettings::CompEnumModules(DBMODULEENUMPROC CallBack, LPARAM lParam)
{
	CBlockManager::ReadTransaction trans(m_BlockManagerSet);

	TModulesMap::iterator i = m_Modules.begin();
	int res = 0;
	while ((i != m_Modules.end()) && (res == 0))
	{
		char * tmp = i->second;
		trans.Close();

		res = CallBack(tmp, 0, lParam);

		trans = CBlockManager::ReadTransaction(m_BlockManagerSet);
		++i;
	}

	return res;
}
