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
#include "Entities.h"

CVirtuals::CVirtuals(CBlockManager & BlockManager, TNodeRef RootNode)
:   CFileBTree<TVirtualKey, 4>::CFileBTree(BlockManager, RootNode, cVirtualNodeSignature)
{

}

CVirtuals::~CVirtuals()
{

}

TDBTEntityHandle CVirtuals::_DeleteRealEntity(TDBTEntityHandle hRealEntity)
{
	TDBTEntityHandle result;
	TVirtualKey key;
	TEntity * entity;
	bool copies = false;
	uint32_t size = sizeof(TEntity);
	uint32_t sig = cEntitySignature;

	key.RealEntity = hRealEntity;
	key.Virtual = 0;

	iterator i = LowerBound(key);
	result = i->Virtual;
	i.setManaged();
	Delete(*i);

	while ((i) && (i->RealEntity == hRealEntity))
	{
		key = *i;
		Delete(key);

		key.RealEntity = result;
		Insert(key);

		entity = m_BlockManager.ReadBlock<TEntity>(key.Virtual, size, sig);
		if (entity)
		{
			entity->VParent = result;
			m_BlockManager.UpdateBlock(key.Virtual);

			copies = true;
		} // TODO log
	}

	entity = m_BlockManager.ReadBlock<TEntity>(result, size, sig);
	if (entity)
	{
		entity->Flags = entity->Flags & ~(DBT_NF_HasVirtuals | DBT_NF_IsVirtual);
		if (copies)
			entity->Flags |= DBT_NF_HasVirtuals;

		m_BlockManager.UpdateBlock(result);
	} // TODO log
	return result;
}

bool CVirtuals::_InsertVirtual(TDBTEntityHandle hRealEntity, TDBTEntityHandle hVirtual)
{
	TVirtualKey key;
	key.RealEntity = hRealEntity;
	key.Virtual = hVirtual;

	Insert(key);

	return true;
}
void CVirtuals::_DeleteVirtual(TDBTEntityHandle hRealEntity, TDBTEntityHandle hVirtual)
{
	TVirtualKey key;
	key.RealEntity = hRealEntity;
	key.Virtual = hVirtual;

	Delete(key);
}
TDBTEntityHandle CVirtuals::getParent(TDBTEntityHandle hVirtual)
{
	TEntity * entity;
	uint32_t size = sizeof(TEntity);
	uint32_t sig = cEntitySignature;

	CBlockManager::ReadTransaction trans(m_BlockManager);

	entity = m_BlockManager.ReadBlock<TEntity>(hVirtual, size, sig);
	if (!entity || ((entity->Flags & DBT_NF_IsVirtual) == 0))
		return DBT_INVALIDPARAM;

	return entity->VParent;
}
TDBTEntityHandle CVirtuals::getFirst(TDBTEntityHandle hRealEntity)
{
	TEntity * entity;
	uint32_t size = sizeof(TEntity);
	uint32_t sig = cEntitySignature;

	CBlockManager::ReadTransaction trans(m_BlockManager);

	entity = m_BlockManager.ReadBlock<TEntity>(hRealEntity, size, sig);
	if (!entity || ((entity->Flags & DBT_NF_HasVirtuals) == 0))
		return DBT_INVALIDPARAM;

	TVirtualKey key;
	key.RealEntity = hRealEntity;
	key.Virtual = 0;

	iterator i = LowerBound(key);

	if (i && (i->RealEntity == hRealEntity))
		key.Virtual = i->Virtual;
	else
		key.Virtual = 0;
	
	return key.Virtual;
}
TDBTEntityHandle CVirtuals::getNext(TDBTEntityHandle hVirtual)
{
	TEntity * entity;
	uint32_t size = sizeof(TEntity);
	uint32_t sig = cEntitySignature;

	CBlockManager::ReadTransaction trans(m_BlockManager);

	entity = m_BlockManager.ReadBlock<TEntity>(hVirtual, size, sig);
	if (!entity || ((entity->Flags & DBT_NF_IsVirtual) == 0))
		return DBT_INVALIDPARAM;

	TVirtualKey key;
	key.RealEntity = entity->VParent;
	key.Virtual = hVirtual + 1;

	iterator i = LowerBound(key);

	if ((i) && (i->RealEntity == entity->VParent))
		key.Virtual = i->Virtual;
	else
		key.Virtual = 0;
	
	return key.Virtual;
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CEntities::CEntities(CBlockManager & BlockManager, TDBTEntityHandle RootEntity, TNodeRef EntityRoot, CVirtuals::TNodeRef VirtualRoot)
:   CFileBTree<TEntityKey, 6>::CFileBTree(BlockManager, EntityRoot, cEntityNodeSignature),
	m_Virtuals(BlockManager, VirtualRoot),

	m_sigEntityDelete(),
	m_sigInternalDeleteEvents(),
	m_sigInternalDeleteSettings(),
	m_sigInternalMergeSettings(),
	m_sigInternalTransferEvents()
{
	if (RootEntity == 0)
		m_RootEntity = _CreateRootEntity();
	else
		m_RootEntity = RootEntity;

}

CEntities::~CEntities()
{

}

TDBTEntityHandle CEntities::_CreateRootEntity()
{
	TEntity * entity;
	TEntityKey key = {0,0,0};

	CBlockManager::WriteTransaction trans(m_BlockManager);

	entity = m_BlockManager.CreateBlock<TEntity>(key.Entity, cEntitySignature);
	entity->Flags = DBT_NF_IsGroup | DBT_NF_IsRoot;
	m_BlockManager.UpdateBlock(key.Entity);
	Insert(key);
	return key.Entity;
}

void CEntities::_InternalTransferContacts(TDBTEntityHandle OldAccount, TDBTEntityHandle NewAccount)
{
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);

	TEntityKey key = {0,0,0};
	iterator i = LowerBound(key);

	while (i)
	{
		sig = cEntitySignature;
		TEntity * entity = m_BlockManager.ReadBlock<TEntity>(i->Entity, size, sig);
		if (entity && (entity->Account == OldAccount))
		{
			entity->Account = NewAccount;
			m_BlockManager.UpdateBlock(i->Entity);
		}

		++i;
	}
}

uint32_t CEntities::_getSettingsRoot(TDBTEntityHandle hEntity)
{
	/*CSettingsTree::TNodeRef*/
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);
	TEntity * entity = m_BlockManager.ReadBlock<TEntity>(hEntity, size, sig);

	if (!entity)
		return DBT_INVALIDPARAM;

	return entity->Settings;
}
bool CEntities::_setSettingsRoot(TDBTEntityHandle hEntity, /*CSettingsTree::TNodeRef*/ uint32_t NewRoot)
{
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);
	TEntity * entity = m_BlockManager.ReadBlock<TEntity>(hEntity, size, sig);

	if (!entity)
		return false;

	entity->Settings = NewRoot;
	m_BlockManager.UpdateBlock(hEntity);

	return true;
}

uint32_t CEntities::_getEventsRoot(TDBTEntityHandle hEntity)
{
	/*CEventsTree::TNodeRef*/
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);
	TEntity * entity = m_BlockManager.ReadBlock<TEntity>(hEntity, size, sig);

	if (!entity)
		return DBT_INVALIDPARAM;

	return entity->Events;
}
bool CEntities::_setEventsRoot(TDBTEntityHandle hEntity, /*CEventsTree::TNodeRef*/ uint32_t NewRoot)
{
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);
	TEntity * entity = m_BlockManager.ReadBlock<TEntity>(hEntity, size, sig);

	if (!entity)
		return false;
	entity->Events = NewRoot;
	m_BlockManager.UpdateBlock(hEntity);

	return true;
}

uint32_t CEntities::_getEventCount(TDBTEntityHandle hEntity)
{
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);
	TEntity * entity = m_BlockManager.ReadBlock<TEntity>(hEntity, size, sig);

	if (!entity)
		return DBT_INVALIDPARAM;

	return entity->EventCount;
}

uint32_t CEntities::_adjustEventCount(TDBTEntityHandle hEntity, int32_t Adjust)
{
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);
	TEntity * entity = m_BlockManager.ReadBlock<TEntity>(hEntity, size, sig);

	if (!entity)
		return DBT_INVALIDPARAM;

	if (((Adjust < 0) && ((uint32_t)(-Adjust) <= entity->EventCount)) ||
			((Adjust > 0) && ((0xffffffff - entity->EventCount) > (uint32_t)Adjust)))
	{
		entity->EventCount += Adjust;
		m_BlockManager.UpdateBlock(hEntity);
	}
	
	return entity->EventCount;
}

bool CEntities::_getFirstUnreadEvent(TDBTEntityHandle hEntity, uint32_t & hEvent, uint32_t & Timestamp)
{
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);
	TEntity * entity = m_BlockManager.ReadBlock<TEntity>(hEntity, size, sig);
	
	if (!entity)
		return false;
	
	Timestamp = entity->FirstUnreadEventTimestamp;
	hEvent = entity->FirstUnreadEventHandle;
	return true;
}
bool CEntities::_setFirstUnreadEvent(TDBTEntityHandle hEntity, uint32_t hEvent, uint32_t Timestamp)
{
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);
	TEntity * entity = m_BlockManager.ReadBlock<TEntity>(hEntity, size, sig);

	if (!entity)
		return false;
	entity->FirstUnreadEventTimestamp = Timestamp;
	entity->FirstUnreadEventHandle = hEvent;
	m_BlockManager.UpdateBlock(hEntity);

	return true;
}

TDBTEntityHandle CEntities::getParent(TDBTEntityHandle hEntity)
{
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);

	CBlockManager::ReadTransaction trans(m_BlockManager);

	TEntity * entity = m_BlockManager.ReadBlock<TEntity>(hEntity, size, sig);
	if (!entity)
		return DBT_INVALIDPARAM;
	
	return entity->ParentEntity;
}
TDBTEntityHandle CEntities::setParent(TDBTEntityHandle hEntity, TDBTEntityHandle hParent)
{
	TEntity *entity, *newparent, *oldparent;
	uint32_t size = sizeof(TEntity);
	uint32_t sig = cEntitySignature;

	CBlockManager::WriteTransaction trans(m_BlockManager);

	entity = m_BlockManager.ReadBlock<TEntity>(hEntity, size, sig);
	newparent = m_BlockManager.ReadBlock<TEntity>(hParent, size, sig);
	if (!entity || !newparent)
		return DBT_INVALIDPARAM;

	oldparent = m_BlockManager.ReadBlock<TEntity>(entity->ParentEntity, size, sig);
	if (!oldparent)
		return DBT_INVALIDPARAM;

	// update parents
	if (--oldparent->ChildCount == 0)
		oldparent->Flags &= ~DBT_NF_HasChildren;
	
	if (++newparent->ChildCount == 1)
		newparent->Flags |= DBT_NF_HasChildren;


	m_BlockManager.UpdateBlock(entity->ParentEntity);
	m_BlockManager.UpdateBlock(hParent);

	// update rest

	TEntityKey key;
	int dif = newparent->Level - entity->Level + 1;

	if (dif == 0) // no level difference, update only moved Entity
	{
		key.Entity = hEntity;
		key.Level = entity->Level;
		key.Parent = entity->ParentEntity;
		Delete(key);
		key.Parent = hParent;
		Insert(key);

		entity->ParentEntity = hParent;
		m_BlockManager.UpdateBlock(hEntity);

	} else {
		TDBTEntityIterFilter filter = {0,0,0,0};
		filter.cbSize = sizeof(filter);
		filter.Options = DBT_NIFO_OSC_AC | DBT_NIFO_OC_AC;

		TDBTEntityIterationHandle iter = IterationInit(filter, hEntity);

		key.Entity = IterationNext(iter);

		while ((key.Entity != 0) && (key.Entity != DBT_INVALIDPARAM))
		{
			size = sizeof(TEntity);
			sig = cEntitySignature;
			TEntity * child = m_BlockManager.ReadBlock<TEntity>(key.Entity, size, sig);

			if (child)
			{
				key.Level = child->Level;
				key.Parent = child->ParentEntity;
				Delete(key);
				
				if (key.Entity == hEntity)
				{
					key.Parent = hParent;
					entity->ParentEntity = hParent;
				}
				
				child->Level += dif;
				key.Level += dif;
				m_BlockManager.UpdateBlock(key.Entity);

				Insert(key);
			} // TODO log
			key.Entity = IterationNext(iter);
		}

		IterationClose(iter);
	}

	/// TODO raise event

	return entity->ParentEntity;
}

uint32_t CEntities::getChildCount(TDBTEntityHandle hEntity)
{
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);

	CBlockManager::ReadTransaction trans(m_BlockManager);

	TEntity * entity = m_BlockManager.ReadBlock<TEntity>(hEntity, size, sig);
	if (!entity)
		return DBT_INVALIDPARAM;

	return entity->ChildCount;
}

uint32_t CEntities::getFlags(TDBTEntityHandle hEntity)
{
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);

	CBlockManager::ReadTransaction trans(m_BlockManager);

	TEntity * entity = m_BlockManager.ReadBlock<TEntity>(hEntity, size, sig);
	if (!entity)
		return DBT_INVALIDPARAM;

	return entity->Flags;
}

uint32_t CEntities::getAccount(TDBTEntityHandle hEntity)
{
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);

	CBlockManager::ReadTransaction trans(m_BlockManager);

	TEntity * entity = m_BlockManager.ReadBlock<TEntity>(hEntity, size, sig);
	if (!entity)
		return DBT_INVALIDPARAM;

	if (entity->Flags & DBT_NF_IsVirtual)
		return getAccount(entity->VParent);
	else if (entity->Flags & (DBT_NF_IsAccount | DBT_NF_IsGroup | DBT_NF_IsRoot))
		return 0;

	return entity->Flags;
}

TDBTEntityHandle CEntities::CreateEntity(const TDBTEntity & Entity)
{
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);
	TDBTEntityHandle haccount = 0;

	CBlockManager::WriteTransaction trans(m_BlockManager);

	TEntity * parent = m_BlockManager.ReadBlock<TEntity>(Entity.hParentEntity, size, sig);
	
	if (!parent)
		return DBT_INVALIDPARAM;

	// check account specification
	if ((Entity.fFlags == 0) && (Entity.hAccountEntity != m_RootEntity)) // TODO disable root account thing, after conversion
	{
		TEntity * account = m_BlockManager.ReadBlock<TEntity>(Entity.hAccountEntity, size, sig);
		if (!account || !(account->Flags & DBT_NF_IsAccount))
			return DBT_INVALIDPARAM;
		
		if (account->Flags & DBT_NF_IsVirtual)
		{
			haccount = VirtualGetParent(Entity.hAccountEntity);
		} else {
			haccount = Entity.hAccountEntity;
		}
	}

	TDBTEntityHandle hentity;
	TEntity * entityblock = m_BlockManager.CreateBlock<TEntity>(hentity, cEntitySignature);
	if (!entityblock)
		return DBT_INVALIDPARAM;

	TEntityKey key;

	entityblock->Level = parent->Level + 1;
	entityblock->ParentEntity = Entity.hParentEntity;
	entityblock->Flags = Entity.fFlags;
	entityblock->Account = haccount;

	m_BlockManager.UpdateBlock(hentity);

	key.Level = entityblock->Level;
	key.Parent = entityblock->ParentEntity;
	key.Entity = hentity;

	Insert(key);

	if (parent->ChildCount == 0)
		parent->Flags = parent->Flags | DBT_NF_HasChildren;
	
	++parent->ChildCount;
	m_BlockManager.UpdateBlock(Entity.hParentEntity);

	return hentity;
}

unsigned int CEntities::DeleteEntity(TDBTEntityHandle hEntity)
{
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);
	TDBTEntityHandle haccount = 0;

	CBlockManager::WriteTransaction trans(m_BlockManager);

	TEntity * entity = m_BlockManager.ReadBlock<TEntity>(hEntity, size, sig);

	if (!entity)
		return DBT_INVALIDPARAM;

	TEntity * parent = m_BlockManager.ReadBlock<TEntity>(entity->ParentEntity, size, sig);
	if (!parent)
		return DBT_INVALIDPARAM;

	m_sigEntityDelete.emit(this, hEntity);

	if (entity->Flags & DBT_NF_HasVirtuals)
	{
		// move virtuals and make one of them real
		TDBTEntityHandle newreal = m_Virtuals._DeleteRealEntity(hEntity);

		TEntity * realblock = m_BlockManager.ReadBlock<TEntity>(newreal, size, sig);
		if (realblock)
		{
			realblock->EventCount = entity->EventCount;
			realblock->Events = entity->Events;

			m_BlockManager.UpdateBlock(newreal);

			m_sigInternalTransferEvents.emit(this, hEntity, newreal);
			m_sigInternalMergeSettings.emit(this, hEntity, newreal);

			if (entity->Flags & DBT_NF_IsAccount)
				_InternalTransferContacts(hEntity, newreal);
		} // TODO log
	} else {
		m_sigInternalDeleteEvents.emit(this, hEntity);
		m_sigInternalDeleteSettings.emit(this, hEntity);

		if ((entity->Flags & DBT_NF_IsAccount) && !(entity->Flags & DBT_NF_IsVirtual))
			_InternalTransferContacts(hEntity, m_RootEntity);
		
	}

	TEntityKey key;
	key.Level = entity->Level;
	key.Parent = entity->ParentEntity;
	key.Entity = hEntity;
	Delete(key);

	if (entity->Flags & DBT_NF_HasChildren) // keep the children
	{
		parent->Flags |= DBT_NF_HasChildren;
		parent->ChildCount += entity->ChildCount;

		TDBTEntityIterFilter filter = {0,0,0,0};
		filter.cbSize = sizeof(filter);
		filter.Options = DBT_NIFO_OSC_AC | DBT_NIFO_OC_AC;

		TDBTEntityIterationHandle iter = IterationInit(filter, hEntity);
		if (iter != DBT_INVALIDPARAM)
		{
			IterationNext(iter);
			key.Entity = IterationNext(iter);

			while ((key.Entity != 0) && (key.Entity != DBT_INVALIDPARAM))
			{
				size = sizeof(TEntity);
				sig = cEntitySignature;
				TEntity * child = m_BlockManager.ReadBlock<TEntity>(key.Entity, size, sig);
				if (child)
				{
					key.Parent = child->ParentEntity;
					key.Level = child->Level;
					Delete(key);

					if (key.Parent == hEntity)
					{
						key.Parent = entity->ParentEntity;
						child->ParentEntity = entity->ParentEntity;
					}
					
					key.Level--;
					m_BlockManager.UpdateBlock(key.Entity);

					Insert(key);

				}
				key.Entity = IterationNext(iter);
			}

			IterationClose(iter);
		}
	}

	if (--parent->ChildCount == 0)
		parent->Flags = parent->Flags & (~DBT_NF_HasChildren);

	m_BlockManager.UpdateBlock(entity->ParentEntity);

	m_BlockManager.DeleteBlock(hEntity); // we needed this block, delete it now

	return 0;
}



TDBTEntityIterationHandle CEntities::IterationInit(const TDBTEntityIterFilter & Filter, TDBTEntityHandle hParent)
{
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);
	TDBTEntityHandle haccount = 0;

	CBlockManager::ReadTransaction trans(m_BlockManager);

	TEntity * parent = m_BlockManager.ReadBlock<TEntity>(hParent, size, sig);

	if (!parent)
		return DBT_INVALIDPARAM;

	PEntityIteration iter = new TEntityIteration;
	iter->filter = Filter;
	iter->q = new std::deque<TEntityIterationItem>;
	iter->parents = new std::deque<TEntityIterationItem>;
	iter->accounts = new std::deque<TEntityIterationItem>;
	iter->returned = new stdext::hash_set<TDBTEntityHandle>;
	iter->returned->insert(hParent);

	TEntityIterationItem it;
	it.Flags = parent->Flags;
	it.Handle = hParent;
	it.Level = parent->Level;
	it.Options = Filter.Options & 0x000000ff;
	it.LookupDepth = 0;

	iter->q->push_back(it);

	return (TDBTEntityIterationHandle)iter;
}
TDBTEntityHandle CEntities::IterationNext(TDBTEntityIterationHandle Iteration)
{
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);

	CBlockManager::ReadTransaction trans(m_BlockManager);

	PEntityIteration iter = reinterpret_cast<PEntityIteration>(Iteration);
	TEntityIterationItem item;
	TDBTEntityHandle result = 0;

	if (iter->q->empty())
	{
		std::deque <TEntityIterationItem> * tmp = iter->q;
		iter->q = iter->parents;
		iter->parents = tmp;
	}

	if (iter->q->empty())
	{
		std::deque <TEntityIterationItem> * tmp = iter->q;
		iter->q = iter->accounts;
		iter->accounts = tmp;
	}

	if (iter->q->empty() &&
		(iter->filter.Options & DBT_NIFO_GF_USEROOT) &&
		(iter->returned->find(m_RootEntity) == iter->returned->end()))
	{
		item.Handle = m_RootEntity;
		item.Level = 0;
		item.Options = 0;
		item.Flags = 0;
		item.LookupDepth = 255;

		iter->filter.Options = iter->filter.Options & ~DBT_NIFO_GF_USEROOT;

		iter->q->push_back(item);
	}

	if (iter->q->empty())
		return 0;
	
	do {
		item = iter->q->front();
		iter->q->pop_front();

		TEntityIterationItem newitem;

		// children
		if ((item.Flags & DBT_NF_HasChildren) &&
			(item.Options & DBT_NIFO_OSC_AC))
		{
			TEntityKey key;
			key.Parent = item.Handle;
			key.Level = item.Level + 1;

			newitem.Level = item.Level + 1;
			newitem.LookupDepth = item.LookupDepth;
			newitem.Options = (iter->filter.Options / DBT_NIFO_OC_AC * DBT_NIFO_OSC_AC) & (DBT_NIFO_OSC_AC | DBT_NIFO_OSC_AO | DBT_NIFO_OSC_AOC | DBT_NIFO_OSC_AOP);

			if (iter->filter.Options & DBT_NIFO_GF_DEPTHFIRST)
			{
				key.Entity = 0xffffffff;

				CEntities::iterator c = UpperBound(key);
				while ((c) && (c->Parent == item.Handle))
				{
					newitem.Handle = c->Entity;

					if (iter->returned->find(newitem.Handle) == iter->returned->end())
					{
						TEntity * tmp = m_BlockManager.ReadBlock<TEntity>(newitem.Handle, size, sig);
						if (tmp)
						{
							newitem.Flags = tmp->Flags;
						  if (((newitem.Flags & DBT_NF_IsGroup) == 0) || ((DBT_NF_IsGroup & iter->filter.fHasFlags) == 0)) // if we want only groups, we don't need to trace down Entities...
							{
								iter->q->push_front(newitem);
								iter->returned->insert(newitem.Handle);
							}
						}
					}

					--c;
				}
			} else {
				key.Entity = 0;

				CEntities::iterator c = LowerBound(key);
				while ((c) && (c->Parent == item.Handle))
				{
					newitem.Handle = c->Entity;

					if (iter->returned->find(newitem.Handle) == iter->returned->end())
					{
						TEntity * tmp = m_BlockManager.ReadBlock<TEntity>(newitem.Handle, size, sig);
						if (tmp)
						{
							newitem.Flags = tmp->Flags;
							if (((newitem.Flags & DBT_NF_IsGroup) == 0) || ((DBT_NF_IsGroup & iter->filter.fHasFlags) == 0)) // if we want only groups, we don't need to trace down Entities...
							{
								iter->q->push_back(newitem);
								iter->returned->insert(newitem.Handle);
							}
						}
					}

					++c;
				}

			}
		}

		// parent...
		if ((item.Options & DBT_NIFO_OSC_AP) && (item.Handle != m_RootEntity))
		{
			newitem.Handle = getParent(item.Handle);
			if ((iter->returned->find(newitem.Handle) == iter->returned->end()) &&
				(newitem.Handle != DBT_INVALIDPARAM))
			{
				TEntity * tmp = m_BlockManager.ReadBlock<TEntity>(newitem.Handle, size, sig);
				if (tmp)
				{
					newitem.Level = item.Level - 1;
					newitem.LookupDepth = item.LookupDepth;
					newitem.Options = (iter->filter.Options / DBT_NIFO_OP_AC * DBT_NIFO_OSC_AC) & (DBT_NIFO_OSC_AC | DBT_NIFO_OSC_AP | DBT_NIFO_OSC_AO | DBT_NIFO_OSC_AOC | DBT_NIFO_OSC_AOP);
					newitem.Flags = tmp->Flags;

					if ((newitem.Flags & iter->filter.fDontHasFlags & DBT_NF_IsGroup) == 0) // if we don't want groups, stop it
					{
						iter->parents->push_back(newitem);
						iter->returned->insert(newitem.Handle);
					}
				}
			}
		}

		// virtual lookup, original Entity is the next one
		if ((item.Flags & DBT_NF_IsVirtual) &&
			(item.Options & DBT_NIFO_OSC_AO) &&
			(((iter->filter.Options >> 28) >= item.LookupDepth) || ((iter->filter.Options >> 28) == 0)))
		{
			newitem.Handle = VirtualGetParent(item.Handle);
			
			if ((iter->returned->find(newitem.Handle) == iter->returned->end()) &&
				  (newitem.Handle != DBT_INVALIDPARAM))
			{
				TEntity * tmp = m_BlockManager.ReadBlock<TEntity>(newitem.Handle, size, sig);
				if (tmp)				   
				{
					newitem.Level = tmp->Level;
					newitem.Options = 0;
					newitem.Flags = tmp->Flags;

					if ((item.Options & DBT_NIFO_OSC_AOC) == DBT_NIFO_OSC_AOC)
						newitem.Options |= DBT_NIFO_OSC_AC;
					if ((item.Options & DBT_NIFO_OSC_AOP) == DBT_NIFO_OSC_AOP)
						newitem.Options |= DBT_NIFO_OSC_AP;

					newitem.LookupDepth = item.LookupDepth + 1;

					iter->q->push_front(newitem);
					iter->returned->insert(newitem.Handle);
				}
			}
		}

		if (((iter->filter.fHasFlags & item.Flags) == iter->filter.fHasFlags) &&
			((iter->filter.fDontHasFlags & item.Flags) == 0))
		{
			result = item.Handle;

			// account lookup
			if (((item.Flags & (DBT_NF_IsAccount | DBT_NF_IsGroup | DBT_NF_IsRoot)) == 0) &&
			    ((item.Options & DBT_NIFO_OC_USEACCOUNT) == DBT_NIFO_OC_USEACCOUNT))
			{
				TDBTEntityHandle acc = item.Handle;
				if (item.Flags & DBT_NF_IsVirtual)
					acc = VirtualGetParent(item.Handle);

				acc = getAccount(acc);

				std::deque<TEntityIterationItem>::iterator acci = iter->accounts->begin();

				while ((acci != iter->accounts->end()) && (acc != 0))
				{
					if (acci->Handle == acc)
						acc = 0;
					++acci;
				}
				if (acc != 0)
				{
					TEntity * tmp = m_BlockManager.ReadBlock<TEntity>(acc, size, sig);
					if (tmp)
					{
						newitem.Options = 0;
						newitem.LookupDepth = 0;
						newitem.Handle = acc;
						newitem.Flags = tmp->Flags;
						newitem.Level = tmp->Level;
						iter->accounts->push_back(newitem);
					}
				}
			}
		}

	} while ((result == 0) && !iter->q->empty());

	if (result == 0)
		result = IterationNext(Iteration);
	
	return result;
}
unsigned int CEntities::IterationClose(TDBTEntityIterationHandle Iteration)
{
	PEntityIteration iter = reinterpret_cast<PEntityIteration>(Iteration);

	delete iter->q;
	delete iter->parents;
	delete iter->accounts;
	delete iter->returned;
	delete iter;

	return 0;
}


TDBTEntityHandle CEntities::VirtualCreate(TDBTEntityHandle hRealEntity, TDBTEntityHandle hParent)
{
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);
	TDBTEntityHandle haccount = 0;

	CBlockManager::WriteTransaction trans(m_BlockManager);

	TEntity * realentity = m_BlockManager.ReadBlock<TEntity>(hRealEntity, size, sig);

	if (!realentity || (realentity->Flags & (DBT_NF_IsGroup | DBT_NF_IsRoot)))
		return DBT_INVALIDPARAM;
	
	TDBTEntity entity = {0,0,0,0};
	entity.hParentEntity = hParent;
	entity.fFlags = DBT_NF_IsVirtual | (realentity->Flags & DBT_NF_IsAccount);
	entity.hAccountEntity = 0;

	TDBTEntityHandle result = CreateEntity(entity);
	if (result == DBT_INVALIDPARAM)
		return DBT_INVALIDPARAM;

	TEntity * entityblock = m_BlockManager.ReadBlock<TEntity>(result, size, sig);
	if (!entityblock)
		return DBT_INVALIDPARAM;

	if (realentity->Flags & DBT_NF_IsVirtual)
	{
		hRealEntity = realentity->VParent;
		realentity = m_BlockManager.ReadBlock<TEntity>(hRealEntity, size, sig);

		if (!realentity)
			return DBT_INVALIDPARAM;
	}

	entityblock->VParent = hRealEntity;
	m_BlockManager.UpdateBlock(result);

	if ((realentity->Flags & DBT_NF_HasVirtuals) == 0)
	{
		realentity->Flags |= DBT_NF_HasVirtuals;
		m_BlockManager.UpdateBlock(hRealEntity);
	}

	m_Virtuals._InsertVirtual(hRealEntity, result);
	return result;
}


TDBTEntityHandle CEntities::compFirstContact()
{
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);

	CBlockManager::ReadTransaction trans(m_BlockManager);
	
	TEntityKey key = {0,0,0};
	iterator i = LowerBound(key);
	TDBTEntityHandle res = 0;

	while (i && (res == 0))
	{
		TEntity * tmp = m_BlockManager.ReadBlock<TEntity>(i->Entity, size, sig);
		if (tmp)
		{
			if ((tmp->Flags & DBT_NFM_SpecialEntity) == 0)
				res = i->Entity;
		}
		if (res == 0)
			++i;
	}

	return res;
}
TDBTEntityHandle CEntities::compNextContact(TDBTEntityHandle hEntity)
{
	uint32_t sig = cEntitySignature;
	uint32_t size = sizeof(TEntity);

	CBlockManager::ReadTransaction trans(m_BlockManager);

	TEntityKey key;
	key.Entity = hEntity;
	TDBTEntityHandle res = 0;

	TEntity * entity = m_BlockManager.ReadBlock<TEntity>(hEntity, size, sig);

	if (entity)
	{
		key.Level = entity->Level;
		key.Parent = entity->ParentEntity;
		key.Entity++;
		iterator i = LowerBound(key);

		while (i && (res == 0))
		{
			entity = m_BlockManager.ReadBlock<TEntity>(i->Entity, size, sig);
			if (entity)
			{
				if ((entity->Flags & DBT_NFM_SpecialEntity) == 0)
					res = i->Entity;
			}
			if (res == 0)
				++i;
		}
	}

	return res;
}
