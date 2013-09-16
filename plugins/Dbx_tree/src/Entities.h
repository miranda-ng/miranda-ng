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
#include <deque>
#include <hash_set>

#pragma pack(push, 1)  // push current alignment to stack, set alignment to 1 byte boundary


/**
	\brief Key Type of the VirtualsBTree

	This BTree don't hold data itself, it's just for organisation
	The virtual Entities are sorted first based on their real Entity.
	That is for enumeration of one Entity's virtual copies, which are all stored in one block in the BTree
**/
typedef struct TVirtualKey {
	TDBTEntityHandle RealEntity;     /// hEntity of the duplicated RealEntity
	TDBTEntityHandle Virtual;       /// hEntity of the virtual duplicate

	bool operator <  (const TVirtualKey & Other) const
	{
		if (RealEntity != Other.RealEntity) return RealEntity < Other.RealEntity;
		if (Virtual != Other.Virtual) return Virtual < Other.Virtual;
		return false;
	}
	//bool operator <= (const TVirtualKey & Other);
	bool operator == (const TVirtualKey & Other) const
	{
		return (RealEntity == Other.RealEntity) && (Virtual == Other.Virtual);
	}
	//bool operator >= (const TVirtualKey & Other);
	bool operator >  (const TVirtualKey & Other) const
	{
		if (RealEntity != Other.RealEntity) return RealEntity > Other.RealEntity;
		if (Virtual != Other.Virtual) return Virtual > Other.Virtual;
		return false;
	}
} TVirtualKey;

/**
	\brief Key Type of the EntityBTree

	The Entities are sorted first based on their level. (root is first node, followed by its children)
	That is for enumeration of one Entity's children, which are all stored in one block in the BTree
**/
typedef struct TEntityKey {
	uint16_t Level;   /// Level where Entity is located or parent-steps to root. Root.Level == 0, root children have level 1 etc.
	TDBTEntityHandle Parent;    /// hEntity of the Parent. Root.Parent == 0
	TDBTEntityHandle Entity;     /// hEntity of the stored Entity itself

	bool operator <  (const TEntityKey & Other) const
	{
		if (Level != Other.Level) return Level < Other.Level;
		if (Parent != Other.Parent) return Parent < Other.Parent;
		if (Entity != Other.Entity) return Entity < Other.Entity;
		return false;
	}
	//bool operator <= (const TEntityKey & Other);
	bool operator == (const TEntityKey & Other) const
	{
		return (Level == Other.Level) && (Parent == Other.Parent) && (Entity == Other.Entity);
	}
	//bool operator >= (const TEntityKey & Other);
	bool operator >  (const TEntityKey & Other) const
	{
		if (Level != Other.Level) return Level > Other.Level;
		if (Parent != Other.Parent) return Parent > Other.Parent;
		if (Entity != Other.Entity) return Entity > Other.Entity;
		return false;
	}
} TEntityKey;

/**
	\brief The data of an Entity
**/
typedef struct TEntity {
	uint16_t Level;       /// Level where Entity is located or parent-steps to root. Root.Level == 0, root children have level 1 etc. !used in the BTreeKey!
	uint16_t ChildCount;    /// Count of the children !invalid for Virtual Entity!
	TDBTEntityHandle ParentEntity; /// hEntity of the Parent. Root.Parent == 0 !used in the BTreeKey!
	union {
		TDBTEntityHandle VParent;     /// if the Entity is Virtual this is the hEntity of the related Realnode
		TDBTEntityHandle Account;     /// if the Entity's account, only for real real normal Entities
	};
	uint32_t Flags;         /// flags, see cEF_*
	/*CSettingsTree::TNodeRef*/
	uint32_t Settings;      /// Offset to the SettingsBTree RootNode of this Entity, NULL if no settings are present
	/*CEventsTree::TNodeRef*/
	uint32_t Events;        /// Offset to the EventsBTree RootNode of this Entity, NULL if no events are present !invalid for Virtal Entity!
	uint32_t EventCount;    /// Count of the stored events !invalid for Virtual Entity!
	uint32_t FirstUnreadEventTimestamp;   /// timestamp of the first unread event
	uint32_t FirstUnreadEventHandle;/// ID of the first unread event
	uint8_t Reserved[4];           /// reserved storage
} TEntity;

#pragma pack(pop)		// pop the alignment from stack




/**
	\brief Manages the Virtual Entities in the Database

	A virtual Entity is stored as normal Entity in the database-structure, but doesn't hold own settings/events.
	Such an Entity has the virtual flag set and refers its original duplicate.
	All copies are stored in this BTree sorted to the RealEntity.
	If the RealEntity should be deleted take the first virtual duplicate and make it real. Also change the relation of other copies.
**/
class CVirtuals :	public CFileBTree<TVirtualKey, 4>
{
private:

protected:

public:
	CVirtuals(CBlockManager & BlockManager, TNodeRef Root);
	virtual ~CVirtuals();

	/**
		\brief Changes reference for all copies to the first Virtual in list

		\return New Original (previously first Virtual) to associate data with
	**/
	TDBTEntityHandle _DeleteRealEntity(TDBTEntityHandle hRealEntity);

	bool _InsertVirtual(TDBTEntityHandle hRealEntity, TDBTEntityHandle hVirtual);
	void _DeleteVirtual(TDBTEntityHandle hRealEntity, TDBTEntityHandle hVirtual);

	// services:
	TDBTEntityHandle getParent(TDBTEntityHandle hVirtual);
	TDBTEntityHandle getFirst(TDBTEntityHandle hRealEntity);
	TDBTEntityHandle getNext(TDBTEntityHandle hVirtual);
};


static const uint32_t cEntitySignature = 0x9A6B3C0D;
static const uint16_t cEntityNodeSignature = 0x65A9;
static const uint16_t cVirtualNodeSignature = 0x874E;
/**
	\brief Manages the Entities in the Database

	A hEntity is equivalent to the fileoffset of its related TEntity structure
**/
class CEntities : public CFileBTree<TEntityKey, 6>
{

public:
	CEntities(CBlockManager & BlockManager, TDBTEntityHandle RootEntity, TNodeRef EntityRoot, CVirtuals::TNodeRef VirtualRoot);
	virtual ~CEntities();

	typedef sigslot::signal2<CEntities *, TDBTEntityHandle> TOnEntityDelete;
	typedef sigslot::signal2<CEntities *, TDBTEntityHandle> TOnInternalDeleteSettings;
	typedef sigslot::signal2<CEntities *, TDBTEntityHandle> TOnInternalDeleteEvents;

	typedef sigslot::signal3<CEntities *, TDBTEntityHandle, TDBTEntityHandle> TOnInternalMergeSettings;
	typedef sigslot::signal3<CEntities *, TDBTEntityHandle, TDBTEntityHandle> TOnInternalTransferEvents;

	CVirtuals::TOnRootChanged & sigVirtualRootChanged()
		{
			return m_Virtuals.sigRootChanged();
		};

	TOnEntityDelete & sigEntityDelete()
		{
			return m_sigEntityDelete;
		};
	TOnInternalDeleteEvents & _sigDeleteEvents()
		{
			return m_sigInternalDeleteEvents;
		};
	TOnInternalDeleteSettings & _sigDeleteSettings()
		{
			return m_sigInternalDeleteSettings;
		};
	TOnInternalMergeSettings & _sigMergeSettings()
		{
			return m_sigInternalMergeSettings;
		};
	TOnInternalTransferEvents & _sigTransferEvents()
		{
			return m_sigInternalTransferEvents;
		};

	//internal helpers:
	/*CSettingsTree::TNodeRef*/
	uint32_t _getSettingsRoot(TDBTEntityHandle hEntity);
	bool _setSettingsRoot(TDBTEntityHandle hEntity, /*CSettingsTree::TNodeRef*/ uint32_t NewRoot);
	uint32_t _getEventsRoot(TDBTEntityHandle hEntity);
	bool _setEventsRoot(TDBTEntityHandle hEntity, /*CSettingsTree::TNodeRef*/ uint32_t NewRoot);
	uint32_t _getEventCount(TDBTEntityHandle hEntity);
	uint32_t _adjustEventCount(TDBTEntityHandle hEntity, int32_t Adjust);
	bool _getFirstUnreadEvent(TDBTEntityHandle hEntity, uint32_t & hEvent, uint32_t & Timestamp);
	bool _setFirstUnreadEvent(TDBTEntityHandle hEntity, uint32_t hEvent, uint32_t Timestamp);

	CVirtuals & _getVirtuals()
		{
			return m_Virtuals;
		};

	//compatibility:
	TDBTEntityHandle compFirstContact();
	TDBTEntityHandle compNextContact(TDBTEntityHandle hEntity);
	//Services:
	TDBTEntityHandle CEntities::getRootEntity()
		{
			return m_RootEntity;
		};

	TDBTEntityHandle getParent(TDBTEntityHandle hEntity);
	TDBTEntityHandle setParent(TDBTEntityHandle hEntity, TDBTEntityHandle hParent);
	uint32_t getChildCount(TDBTEntityHandle hEntity);
	uint32_t getFlags(TDBTEntityHandle hEntity);
	uint32_t getAccount(TDBTEntityHandle hEntity);

	TDBTEntityHandle CreateEntity(const TDBTEntity & Entity);
	unsigned int DeleteEntity(TDBTEntityHandle hEntity);

	TDBTEntityIterationHandle IterationInit(const TDBTEntityIterFilter & Filter, TDBTEntityHandle hParent);
	TDBTEntityHandle IterationNext(TDBTEntityIterationHandle Iteration);
	unsigned int IterationClose(TDBTEntityIterationHandle Iteration);

	TDBTEntityHandle VirtualCreate(TDBTEntityHandle hRealEntity, TDBTEntityHandle hParent);
	TDBTEntityHandle VirtualGetParent(TDBTEntityHandle hVirtual)
		{
			return m_Virtuals.getParent(hVirtual);
		};
	TDBTEntityHandle VirtualGetFirst(TDBTEntityHandle hRealEntity)
		{
			return m_Virtuals.getFirst(hRealEntity);
		};
	TDBTEntityHandle VirtualGetNext(TDBTEntityHandle hVirtual)
		{
			return m_Virtuals.getNext(hVirtual);
		};
private:

protected:

	typedef struct TEntityIterationItem {
		uint8_t Options;
		uint8_t LookupDepth;
		uint16_t Level;
		TDBTEntityHandle Handle;
		uint32_t Flags;
	} TEntityIterationItem;

	typedef struct TEntityIteration {
		TDBTEntityIterFilter filter;
		std::deque<TEntityIterationItem> * q;
		std::deque<TEntityIterationItem> * parents;
		std::deque<TEntityIterationItem> * accounts;
		stdext::hash_set<TDBTEntityHandle> * returned;
	} TEntityIteration, *PEntityIteration;

	TDBTEntityHandle m_RootEntity;
	CVirtuals m_Virtuals;

	TDBTEntityHandle _CreateRootEntity();
	void _InternalTransferContacts(TDBTEntityHandle OldAccount, TDBTEntityHandle NewAccount);

	TOnEntityDelete            m_sigEntityDelete;
	TOnInternalDeleteEvents    m_sigInternalDeleteEvents;
	TOnInternalDeleteSettings  m_sigInternalDeleteSettings;
	TOnInternalMergeSettings   m_sigInternalMergeSettings;
	TOnInternalTransferEvents  m_sigInternalTransferEvents;

};
