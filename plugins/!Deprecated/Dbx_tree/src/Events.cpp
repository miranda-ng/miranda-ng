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
#include "Events.h"

CEventsTypeManager::CEventsTypeManager(CEntities & Entities, CSettings & Settings)
:	m_Entities(Entities),
	m_Settings(Settings),
	m_Map()
{
	m_Settings._EnsureModuleExists("$EventTypes");
}
CEventsTypeManager::~CEventsTypeManager()
{
	TTypeMap::iterator it = m_Map.begin();

	while (it != m_Map.end())
	{
		delete [] it->second->ModuleName;
		delete it->second;
		++it;
	}
}

uint32_t CEventsTypeManager::MakeGlobalID(char* Module, uint32_t EventType)
{
	uint32_t l = static_cast<uint32_t>(strlen(Module));
	void * buf = malloc(l + sizeof(uint32_t));
	memcpy(buf, Module, l);
	memcpy(((char*)buf) + l, &EventType, sizeof(uint32_t));

	uint32_t h = Hash(buf, l + sizeof(uint32_t));
	free(buf);

	char * m;
	uint32_t t;
	while (GetType(h, m, t) && ((t != EventType) || (strcmp(m, Module) != 0)))
	{
		++h;
	}

	return h;
}
bool CEventsTypeManager::GetType(uint32_t GlobalID, char * & Module, uint32_t & EventType)
{
	TTypeMap::iterator it = m_Map.find(GlobalID);

	if (it == m_Map.end())
	{
		char n[256];

		TDBTSettingDescriptor d = {0,0,0,0,0,0,0,0};
		d.cbSize = sizeof(d);
		d.Entity = m_Entities.getRootEntity();
		d.pszSettingName = n;

		TDBTSetting sid = {0,0,0,0};
		TDBTSetting sname = {0,0,0,0};

		sid.cbSize = sizeof(sid);
		sid.Descriptor = &d;
		sid.Type = DBT_ST_INT;

		sname.cbSize = sizeof(sname);
		sname.Descriptor = &d;
		sname.Type = DBT_ST_ANSI;

		mir_snprintf(n, SIZEOF(n), "$EventTypes/%08x/ModuleID", GlobalID);
		TDBTSettingHandle h = m_Settings.ReadSetting(sid);

		if ((h != DBT_INVALIDPARAM) && (h != 0))
		{
			mir_snprintf(n, SIZEOF(n), "$EventTypes/%08x/ModuleName", GlobalID);
			d.Flags = 0;
			h = m_Settings.ReadSetting(sname);

			if ((h != DBT_INVALIDPARAM) && (h != 0))
			{
				PEventType t = new TEventType;

				t->EventType = sid.Value.Int;

				t->ModuleName = new char[sname.Value.Length];
				strcpy_s(t->ModuleName, sname.Value.Length, sname.Value.pAnsi);

				m_Map.insert(std::make_pair(GlobalID, t));

				mir_free(sname.Value.pAnsi);

				Module = t->ModuleName;
				EventType  = t->EventType;

				return true;
			}
		}
	} else {
		Module = it->second->ModuleName;
		EventType  = it->second->EventType;

		return true;
	}

	return false;
}

uint32_t CEventsTypeManager::EnsureIDExists(char* Module, uint32_t EventType)
{
	uint32_t res = MakeGlobalID(Module, EventType);
	char *   m;
	uint32_t t;
	if (!GetType(res, m, t))
	{
		char n[256];

		TDBTSettingDescriptor d = {0,0,0,0,0,0,0,0};
		d.cbSize = sizeof(d);
		d.pszSettingName = n;
		d.Entity = m_Entities.getRootEntity();

		TDBTSetting s = {0,0,0,0};
		s.cbSize = sizeof(s);
		s.Descriptor = &d;

		mir_snprintf(n, SIZEOF(n), "$EventTypes/%08x/ModuleID", res);
		s.Type = DBT_ST_INT;
		s.Value.Int = EventType;
		m_Settings.WriteSetting(s);

		mir_snprintf(n, SIZEOF(n), "$EventTypes/%08x/ModuleName", res);
		d.Flags = 0;
		s.Type = DBT_ST_ANSI;
		s.Value.Length = static_cast<uint32_t>(strlen(Module) + 1);
		s.Value.pAnsi = Module;
		m_Settings.WriteSetting(s);

		m_Settings._EnsureModuleExists(Module);
	}

	return res;
}


CEvents::CEvents(
	CBlockManager & BlockManager,
	CEncryptionManager & EncryptionManager,
	CEntities & Entities,
	CSettings & Settings
)
:	m_BlockManager(BlockManager),
	m_EncryptionManager(EncryptionManager),
	m_Entities(Entities),
	m_Types(Entities, Settings),
	m_EntityEventsMap()
{
	m_Entities._sigDeleteEvents().connect(this, &CEvents::onDeleteEvents);
	m_Entities._sigTransferEvents().connect(this, &CEvents::onTransferEvents);
}

CEvents::~CEvents()
{
	TEntityEventsMap::iterator i = m_EntityEventsMap.begin();
	while (i != m_EntityEventsMap.end())
	{
		delete i->second->RealTree;
		delete i->second->VirtualTree;
		delete i->second;
		++i;
	}
}

void CEvents::onRootChanged(void* EventsTree, CEventsTree::TNodeRef NewRoot)
{
	m_Entities._setEventsRoot(reinterpret_cast<CEventsTree*>(EventsTree)->Entity(), NewRoot);
}

void CEvents::onDeleteEventCallback(void * Tree, const TEventKey & Key, uint32_t Param)
{
	m_BlockManager.DeleteBlock(Key.Event);
}

void CEvents::onDeleteVirtualEventCallback(void * Tree, const TEventKey & Key, uint32_t Param)
{
	m_BlockManager.DeleteBlock(Key.Event);
}
void CEvents::onDeleteEvents(CEntities * Entities, TDBTEntityHandle hEntity)
{
	PEntityEventsRecord record = getEntityRecord(hEntity);

	if (record == NULL)
		return;

	m_Entities._setEventsRoot(hEntity, 0);

	if (record->VirtualCount)
	{
		CVirtualEventsTree::TDeleteCallback callback;
		callback.connect(this, &CEvents::onDeleteVirtualEventCallback);

		record->VirtualTree->DeleteTree(&callback, hEntity);
	}
	delete record->VirtualTree;

	CEventsTree::TDeleteCallback callback;
	callback.connect(this, &CEvents::onDeleteEventCallback);
	record->RealTree->DeleteTree(&callback, hEntity);
	delete record->RealTree;
	m_EntityEventsMap.erase(hEntity);
}
void CEvents::onTransferEvents(CEntities * Entities, TDBTEntityHandle Source, TDBTEntityHandle Dest)
{
	PEntityEventsRecord record = getEntityRecord(Source);

	if (record == NULL)
		return;

	if (record->VirtualCount)
	{
		TEventKey key = {0,0};

		CVirtualEventsTree::iterator i = record->VirtualTree->LowerBound(key);

		while (i)
		{
			uint32_t sig = cEventSignature;
			uint32_t size = 0;
			TEvent * tmp = m_BlockManager.ReadBlock<TEvent>(i->Event, size, sig);
			if (tmp)
			{
				tmp->Entity = Dest;
				m_BlockManager.UpdateBlock(i->Event);
			}
			++i;
		}
	}

	{
		TEventKey key = {0,0};

		CEventsTree::iterator i = record->RealTree->LowerBound(key);
		while (i)
		{
			uint32_t sig = cEventSignature;
			uint32_t size = 0;
			TEvent * tmp = m_BlockManager.ReadBlock<TEvent>(i->Event, size, sig);
			if (tmp)
			{
				tmp->Entity = Dest;
				m_BlockManager.UpdateBlock(i->Event);
			}		
			++i;
		}

		m_Entities._setEventsRoot(Source, 0);
		m_Entities._setEventsRoot(Dest, record->RealTree->getRoot());
		m_Entities._adjustEventCount(Dest, m_Entities._getEventCount(Source));
		m_Entities._getFirstUnreadEvent(Source, key.Event, key.TimeStamp);
		m_Entities._setFirstUnreadEvent(Dest, key.Event, key.TimeStamp);
	}

	record->VirtualTree->Entity(Dest);
	record->RealTree->Entity(Dest);
	m_EntityEventsMap.erase(Source);
	m_EntityEventsMap.insert(std::make_pair(Dest, record));
}

CEvents::PEntityEventsRecord CEvents::getEntityRecord(TDBTEntityHandle hEntity)
{
	TEntityEventsMap::iterator i = m_EntityEventsMap.find(hEntity);
	if (i != m_EntityEventsMap.end())
		return i->second;

	uint32_t root = m_Entities._getEventsRoot(hEntity);
	if (root == DBT_INVALIDPARAM)
		return NULL;

	PEntityEventsRecord res = new TEntityEventsRecord;
	res->RealTree = new CEventsTree(m_BlockManager, root, hEntity);
	res->RealTree->sigRootChanged().connect(this, &CEvents::onRootChanged);
	res->VirtualTree = new CVirtualEventsTree(hEntity);
	res->VirtualCount = 0;
	res->FirstVirtualUnread.TimeStamp = 0;
	res->FirstVirtualUnread.Event = 0;
	m_EntityEventsMap.insert(std::make_pair(hEntity, res));

	return res;
}

inline uint32_t CEvents::adjustVirtualEventCount(PEntityEventsRecord Record, int32_t Adjust)
{
	if (((Adjust < 0) && ((uint32_t)(-Adjust) <= Record->VirtualCount)) ||
		  ((Adjust > 0) && ((0xffffffff - Record->VirtualCount) > (uint32_t)Adjust)))
	{
		Record->VirtualCount += Adjust;
	}

	return Record->VirtualCount;
}

inline bool CEvents::MarkEventsTree(TEventBase::iterator Iterator, TDBTEventHandle FirstUnread)
{
	uint32_t sig, size;
	bool b = true;
	bool res = false;
	while (Iterator && b)
	{
		sig = cEventSignature;
		size = 0;
		TEvent * event = m_BlockManager.ReadBlock<TEvent>(Iterator->Event, size, sig);
		if (event)
		{
			if (Iterator->Event == FirstUnread) 
				res = true;

			if ((event->Flags & DBT_EF_READ) == 0)
			{
				event->Flags |= DBT_EF_READ;
				m_BlockManager.UpdateBlock(Iterator->Event);
				--Iterator;
			} else {
				b = false;
			}
		} else {
			--Iterator;
		}
	}
	return res;
}
inline void CEvents::FindNextUnreadEvent(TEventBase::iterator & Iterator)
{
	uint32_t sig, size;
	while (Iterator)
	{
		sig = cEventSignature;
		size = 0;
		TEvent * event = m_BlockManager.ReadBlock<TEvent>(Iterator->Event, size, sig);
		if (event)
		{
			if (event->Flags & DBT_EF_READ)
				++Iterator;
			else
				return;
		} else {
			++Iterator;
		}
	}
}

unsigned int CEvents::GetBlobSize(TDBTEventHandle hEvent)
{
	uint32_t sig = cEventSignature;
	uint32_t size = 0;

	CBlockManager::ReadTransaction trans(m_BlockManager);
	TEvent * event = m_BlockManager.ReadBlock<TEvent>(hEvent, size, sig);
	if (!event)
		return DBT_INVALIDPARAM;

	return event->DataLength;
}

unsigned int CEvents::Get(TDBTEventHandle hEvent, TDBTEvent & Event)
{
	uint32_t sig = cEventSignature;
	uint32_t size = 0;
	
	CBlockManager::ReadTransaction trans(m_BlockManager);
	
	TEvent * event = m_BlockManager.ReadBlock<TEvent>(hEvent, size, sig);
	if (!event)
		return DBT_INVALIDPARAM;

	uint8_t * blob = reinterpret_cast<uint8_t *>(event + 1);

	if (!m_Types.GetType(event->Type, Event.ModuleName, Event.EventType))
	{
		Event.EventType = event->Type;
		Event.ModuleName = "???";
	}

	Event.Flags = event->Flags;
	if (m_BlockManager.IsForcedVirtual(hEvent))
		Event.Flags |= DBT_EF_VIRTUAL;

	Event.Timestamp = event->TimeStamp;

	if (Event.cbBlob < event->DataLength)
		Event.pBlob = (uint8_t*) mir_realloc(Event.pBlob, event->DataLength);

	memcpy(Event.pBlob, blob, event->DataLength);
	Event.cbBlob = event->DataLength;

	return 0;
}

unsigned int CEvents::GetCount(TDBTEntityHandle hEntity)
{
	CBlockManager::ReadTransaction trans(m_BlockManager);

	uint32_t res = m_Entities._getEventCount(hEntity);
	PEntityEventsRecord record = getEntityRecord(hEntity);

	if ((res == DBT_INVALIDPARAM) || !record)
		return DBT_INVALIDPARAM;
	
	res = res + record->VirtualCount; // access to Virtual Count need sync, too

	return res;
}

unsigned int CEvents::Delete(TDBTEventHandle hEvent)
{
	uint32_t sig = cEventSignature;
	uint32_t size = 0;
	TEventKey key = {0, hEvent};

	CBlockManager::WriteTransaction trans(m_BlockManager);

	TEvent * event = m_BlockManager.ReadBlock<TEvent>(hEvent, size, sig);
	if (!event)
		return DBT_INVALIDPARAM;

	key.TimeStamp = event->TimeStamp;

	PEntityEventsRecord record = getEntityRecord(event->Entity);
	if (!record)
		return DBT_INVALIDPARAM;	
	
	if (m_BlockManager.IsForcedVirtual(hEvent))
	{
		if (record->VirtualTree->Delete(key))
		{
			adjustVirtualEventCount(record, -1);
			
			if (record->FirstVirtualUnread.Event == hEvent)
			{
				CVirtualEventsTree::iterator vi = record->VirtualTree->LowerBound(key);
				FindNextUnreadEvent(vi);
				if (vi)
				{
					record->FirstVirtualUnread = *vi;
				} else {
					record->FirstVirtualUnread.TimeStamp = 0;
					record->FirstVirtualUnread.Event = 0;
				}
			}
		}
	} else { // real event
		if (record->RealTree->Delete(key))
		{
			m_Entities._adjustEventCount(event->Entity, -1);
			TEventKey unreadkey;
			m_Entities._getFirstUnreadEvent(event->Entity, unreadkey.Event, unreadkey.TimeStamp);
			if (unreadkey.Event == hEvent)
			{
				CEventsTree::iterator it = record->VirtualTree->LowerBound(key);
				FindNextUnreadEvent(it);
				if (it)
				{
					m_Entities._setFirstUnreadEvent(event->Entity, it->Event, it->TimeStamp);
				} else {
					m_Entities._setFirstUnreadEvent(event->Entity, 0, 0);
				}
			}
		}
	}
	m_BlockManager.DeleteBlock(hEvent);

	return 0;
}

TDBTEventHandle CEvents::Add(TDBTEntityHandle hEntity, TDBTEvent & Event)
{
	TDBTEventHandle res = 0;
	TEvent * event;

	CBlockManager::WriteTransaction trans(m_BlockManager);

	uint32_t eflags = m_Entities.getFlags(hEntity);
	PEntityEventsRecord record = getEntityRecord(hEntity);

	if ((eflags == DBT_INVALIDPARAM) ||
		 ((eflags & (DBT_NF_IsGroup | DBT_NF_IsRoot)) == DBT_NF_IsGroup) ||  // forbid events in groups. but allow root to have system history
		 !record)
	{
		return DBT_INVALIDPARAM;
	}

	if (eflags & DBT_NF_IsVirtual)
		hEntity = m_Entities.VirtualGetParent(hEntity);
	
	uint8_t *blobdata = Event.pBlob;
	bool bloballocated = false;

	if (Event.Flags & DBT_EF_VIRTUAL)
	{
		event = m_BlockManager.CreateBlockVirtual<TEvent>(res, cEventSignature, sizeof(TEvent) + Event.cbBlob);
	} else {
		event = m_BlockManager.CreateBlock<TEvent>(res, cEventSignature, sizeof(TEvent) + Event.cbBlob);
	}

	if (!event)
		return DBT_INVALIDPARAM;

	TEventKey key = {0,0};

	event->TimeStamp = Event.Timestamp;
	event->Flags = Event.Flags & ~DBT_EF_VIRTUAL;
	event->Type = m_Types.EnsureIDExists(Event.ModuleName, Event.EventType);
	event->DataLength = Event.cbBlob;
	event->Entity = hEntity;

	key.TimeStamp = event->TimeStamp;
	key.Event = res;
	memcpy(event + 1, Event.pBlob, Event.cbBlob);

	m_BlockManager.UpdateBlock(res);

	if (Event.Flags & DBT_EF_VIRTUAL)
	{
		record->VirtualTree->Insert(key);
		adjustVirtualEventCount(record, +1);
		if (!(Event.Flags & DBT_EF_READ) && ((record->FirstVirtualUnread.Event == 0) || (key < record->FirstVirtualUnread)))
		{
			record->FirstVirtualUnread = key;
		}
	} else {
		record->RealTree->Insert(key);
		m_Entities._adjustEventCount(hEntity, +1);

		if (!(Event.Flags & DBT_EF_READ))
		{
			TEventKey unreadkey;
			if (m_Entities._getFirstUnreadEvent(hEntity, unreadkey.Event, unreadkey.TimeStamp) &&
				  ((unreadkey.Event == 0) || (key < unreadkey)))
			{
				m_Entities._setFirstUnreadEvent(hEntity, key.Event, key.TimeStamp);
			}
		}
	}
	
	return res;
}
unsigned int CEvents::MarkRead(TDBTEventHandle hEvent)
{
	uint32_t sig = cEventSignature;
	uint32_t size = 0;
	TEventKey key = {0, hEvent};
	
	CBlockManager::WriteTransaction trans(m_BlockManager);

	TEvent * event = m_BlockManager.ReadBlock<TEvent>(hEvent, size, sig);

	if (!event)
		return DBT_INVALIDPARAM;

	key.TimeStamp = event->TimeStamp;

	if (event->Flags & DBT_EF_READ)
		return event->Flags;
	
	PEntityEventsRecord record = getEntityRecord(event->Entity);
	if (!record)
		return DBT_INVALIDPARAM;
	
	CEventsTree::iterator it = record->RealTree->UpperBound(key);
	CVirtualEventsTree::iterator vi = record->VirtualTree->UpperBound(key);

	m_Entities._getFirstUnreadEvent(event->Entity, key.Event, key.TimeStamp);
	if (MarkEventsTree(it, key.Event))
	{
		FindNextUnreadEvent(++it);
		if (it)
		{
			m_Entities._setFirstUnreadEvent(event->Entity, it->Event, it->TimeStamp);
		} else {
			m_Entities._setFirstUnreadEvent(event->Entity, 0, 0);
		}
	}
	if (MarkEventsTree(vi, record->FirstVirtualUnread.Event))
	{
		FindNextUnreadEvent(++vi);
		if (vi)
		{
			record->FirstVirtualUnread = *it;
		} else {
			record->FirstVirtualUnread.TimeStamp = 0;
			record->FirstVirtualUnread.Event = 0;
		}
	}

	return event->Flags | DBT_EF_READ;
}
unsigned int CEvents::WriteToDisk(TDBTEventHandle hEvent)
{
	uint32_t sig = cEventSignature;
	uint32_t size = 0;
	TEventKey key;
	key.Event = hEvent;

	CBlockManager::WriteTransaction trans(m_BlockManager);

	if (!m_BlockManager.IsForcedVirtual(hEvent))
		return DBT_INVALIDPARAM;
	
	TEvent * event = m_BlockManager.ReadBlock<TEvent>(hEvent, size, sig);
	if (!event || (size < sizeof(TEvent)))
		return DBT_INVALIDPARAM;

	PEntityEventsRecord record = getEntityRecord(event->Entity);
	if (!record)
		return DBT_INVALIDPARAM;			

	key.TimeStamp = event->TimeStamp;
	key.Event = hEvent;

	if (record->VirtualTree->Delete(key))
		adjustVirtualEventCount(record, -1);

	if (record->RealTree->Insert(key))
		m_Entities._adjustEventCount(event->Entity, +1);
	m_BlockManager.WriteBlockToDisk(hEvent);
	
	return 0;
}

TDBTEntityHandle CEvents::getEntity(TDBTEventHandle hEvent)
{
	uint32_t sig = cEventSignature;
	uint32_t size = 0;

	CBlockManager::ReadTransaction trans(m_BlockManager);
	TEvent * event = m_BlockManager.ReadBlock<TEvent>(hEvent, size, sig);
	if (!event)
		return DBT_INVALIDPARAM;

	return event->Entity;
}

TDBTEventIterationHandle CEvents::IterationInit(TDBTEventIterFilter & Filter)
{
	CBlockManager::ReadTransaction trans(m_BlockManager);

	PEntityEventsRecord record = getEntityRecord(Filter.hEntity);

	if (!record)
		return DBT_INVALIDPARAM;

	std::queue<TEventBase * > q;
	q.push(record->RealTree);
	q.push(record->VirtualTree);

	TDBTEntityIterFilter f = {0,0,0,0};
	f.cbSize = sizeof(f);
	f.Options = Filter.Options;

	TDBTEntityIterationHandle citer = m_Entities.IterationInit(f, Filter.hEntity);
	if (citer != DBT_INVALIDPARAM)
	{
		m_Entities.IterationNext(citer);
		TDBTEntityHandle c = m_Entities.IterationNext(citer);
		while (c != 0)
		{
			record = getEntityRecord(c);
			if (record)
			{
				q.push(record->RealTree);
				q.push(record->VirtualTree);
			}

			c = m_Entities.IterationNext(citer);
		}

		m_Entities.IterationClose(citer);
	}

	for (unsigned j = 0; j < Filter.ExtraCount; ++j)
	{
		record = getEntityRecord(Filter.ExtraEntities[j]);
		if (record)
		{
			q.push(record->RealTree);
			q.push(record->VirtualTree);
		}
	}

	PEventIteration iter = new TEventIteration;
	iter->Filter = Filter;
	iter->LastEvent = 0;
	iter->Heap = NULL;

	TEventKey key;
	key.TimeStamp = Filter.tSince;
	key.Event = 0;

	while (!q.empty())
	{
		TEventBase * b = q.front();
		q.pop();

		TEventBase::iterator it = b->LowerBound(key);
		if (it)
		{
			TEventBase::iterator * it2 = new TEventBase::iterator(it);
			it2->setManaged();
			if (iter->Heap)
			{
				iter->Heap->Insert(*it2);
			} else {
				iter->Heap = new TEventsHeap(*it2, TEventsHeap::ITForward, true);
			}
		}
	}

	if (iter->Heap == NULL)
	{
		delete iter;
		iter = (PEventIteration)DBT_INVALIDPARAM;
	}
	
	return reinterpret_cast<TDBTEventIterationHandle>(iter);
}

TDBTEventHandle CEvents::IterationNext(TDBTEventIterationHandle Iteration)
{
	PEventIteration iter = reinterpret_cast<PEventIteration>(Iteration);

	CBlockManager::ReadTransaction trans(m_BlockManager);

	TDBTEventHandle res = 0;
	TEventBase::iterator it = iter->Heap->Top();

	while ((it) && (it.wasDeleted() || ((it->TimeStamp <= iter->Filter.tTill) && (it->Event == iter->LastEvent))))
	{
		iter->Heap->Pop();
		it = iter->Heap->Top();
	}

	if ((it) && !it.wasDeleted() && (it->TimeStamp <= iter->Filter.tTill))
	{
		res = it->Event;
		iter->Heap->Pop();
	}

	if (res)
	{
		iter->LastEvent = res;
		if (iter->Filter.Event)
		{
			iter->Filter.Event->EventType = 0;
			Get(res, *iter->Filter.Event);
		}
	}

	return res;
}

unsigned int CEvents::IterationClose(TDBTEventIterationHandle Iteration)
{
	PEventIteration iter = reinterpret_cast<PEventIteration>(Iteration);

	CBlockManager::ReadTransaction trans(m_BlockManager);
	delete iter->Heap;
	delete iter;
	return 0;
}


TDBTEventHandle CEvents::compFirstEvent(TDBTEntityHandle hEntity)
{
	CBlockManager::ReadTransaction trans(m_BlockManager);

	TDBTEventHandle res = 0;

	PEntityEventsRecord record = getEntityRecord(hEntity);
	if (!record)
		return 0;
	
	TEventKey key = {0,0};
	CEventsTree::iterator i = record->RealTree->LowerBound(key);
	CVirtualEventsTree::iterator vi = record->VirtualTree->LowerBound(key);

	if (i && vi)
	{
		if (*i < *vi)
		{
			res = i->Event;
		} else {
			res = vi->Event;
		}
	} else if (i)
	{
		res = i->Event;
	} else if (vi)
	{
		res = vi->Event;
	}
	
	return res;
}
TDBTEventHandle CEvents::compFirstUnreadEvent(TDBTEntityHandle hEntity)
{
	CBlockManager::ReadTransaction trans(m_BlockManager);

	TDBTEventHandle res = 0;
	
	PEntityEventsRecord record = getEntityRecord(hEntity);
	if (!record)
		return 0;

	TEventKey key;
	m_Entities._getFirstUnreadEvent(hEntity, key.Event, key.TimeStamp);
	if (key.Event)
	{
		if (record->FirstVirtualUnread.Event && (record->FirstVirtualUnread < key))
		{
			res = record->FirstVirtualUnread.Event;
		} else {
			res = key.Event;
		}
	} else if (record->FirstVirtualUnread.Event)
	{
		res = record->FirstVirtualUnread.Event;
	}
	
	return res;
}
TDBTEventHandle CEvents::compLastEvent(TDBTEntityHandle hEntity)
{
	CBlockManager::ReadTransaction trans(m_BlockManager);
	TDBTEventHandle res = 0;

	PEntityEventsRecord record = getEntityRecord(hEntity);
	if (!record)
		return 0;

	TEventKey key = {0xffffffff, 0xffffffff};

	CEventsTree::iterator i = record->RealTree->UpperBound(key);
	CVirtualEventsTree::iterator vi = record->VirtualTree->UpperBound(key);

	if (i && vi)
	{
		if (*i > *vi)
		{
			res = i->Event;
		} else {
			res = vi->Event;
		}
	} else if (i)
	{
		res = i->Event;
	} else if (vi)
	{
		res = vi->Event;
	}
	
	return res;
}
TDBTEventHandle CEvents::compNextEvent(TDBTEventHandle hEvent)
{
	uint32_t sig = cEventSignature;
	uint32_t size = 0;

	CBlockManager::ReadTransaction trans(m_BlockManager);

	TEvent * event = m_BlockManager.ReadBlock<TEvent>(hEvent, size, sig);

	if (!event)
		return 0;

	TEventKey key = {event->TimeStamp, hEvent};

	PEntityEventsRecord record = getEntityRecord(event->Entity);

	if (!record)
		return 0;
	
	if (key.Event == 0xffffffff)
	{
		if (key.TimeStamp == 0xffffffff)
		{
			return 0;
		} else {
			++key.TimeStamp;
		}
	} else {
		++key.Event;
	}

	CEventsTree::iterator i = record->RealTree->LowerBound(key);
	CVirtualEventsTree::iterator vi = record->VirtualTree->LowerBound(key);

	TDBTEventHandle res = 0;
	if (i && vi)
	{
		if (*i < *vi)
		{
			res = i->Event;
		} else {
			res = vi->Event;
		}
	} else if (i)
	{
		res = i->Event;
	} else if (vi)
	{
		res = vi->Event;
	}

	return res;
}
TDBTEventHandle CEvents::compPrevEvent(TDBTEventHandle hEvent)
{
	uint32_t sig = cEventSignature;
	uint32_t size = 0;

	CBlockManager::ReadTransaction trans(m_BlockManager);

	TEvent * event = m_BlockManager.ReadBlock<TEvent>(hEvent, size, sig);

	if (!event)
		return 0;

	TEventKey key = {event->TimeStamp, hEvent};

	PEntityEventsRecord record = getEntityRecord(event->Entity);
	if (!record)
		return 0;

	if (key.Event == 0)
	{
		if (key.TimeStamp == 0)
		{
			return 0;
		} else {
			--key.TimeStamp;
		}
	} else {
		--key.Event;
	}

	CEventsTree::iterator i = record->RealTree->UpperBound(key);
	CVirtualEventsTree::iterator vi = record->VirtualTree->UpperBound(key);

	TDBTEventHandle res = 0;
	if (i && vi)
	{
		if (*i > *vi)
		{
			res = i->Event;
		} else {
			res = vi->Event;
		}
	} else if (i)
	{
		res = i->Event;
	} else if (vi)
	{
		res = vi->Event;
	}
	
	return res;
}
