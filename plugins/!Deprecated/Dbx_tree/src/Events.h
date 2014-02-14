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
#include "BTree.h"
#include "FileBTree.h"
#include "BlockManager.h"
#include "IterationHeap.h"
#include "Entities.h"
#include "Settings.h"
#include "Hash.h"
#include "EncryptionManager.h"
#include "sigslot.h"

#include <hash_map>
#include <hash_set>
#include <queue>
#include <time.h>
#include <windows.h>

#pragma pack(push, 1)  // push current alignment to stack, set alignment to 1 byte boundary

/**
	\brief Key Type of the EventsBTree

	The Key consists of a timestamp, seconds elapsed since 1.1.1970
	and an Index, which makes it possible to store multiple events with the same timestamp
**/
typedef struct TEventKey {
	uint32_t        TimeStamp; /// timestamp at which the event occoured
	TDBTEventHandle Event;

	bool operator <  (const TEventKey & Other) const
	{
		if (TimeStamp != Other.TimeStamp) return TimeStamp < Other.TimeStamp;
		if (Event != Other.Event) return Event < Other.Event;
		return false;
	}
	//bool operator <= (const TEventKey & Other);
	bool operator == (const TEventKey & Other) const
	{
		return (TimeStamp == Other.TimeStamp) && (Event == Other.Event);
	}

	//bool operator >= (const TEventKey & Other);
	bool operator >  (const TEventKey & Other) const
	{
		if (TimeStamp != Other.TimeStamp) return TimeStamp > Other.TimeStamp;
		if (Event != Other.Event) return Event > Other.Event;
		return false;
	}

} TEventKey;

/**
	\brief The data of an Event

	A event's data is variable length. The data is a TDBTEvent-structure followed by varaible length data.
	- fixed data
	- blob data (mostly UTF8 message body)
**/
typedef struct TEvent {
	uint32_t Flags;				       /// Flags
	uint32_t TimeStamp;          /// Timestamp of the event (seconds elapsed since 1.1.1970) used as key element
	uint32_t Type;               /// Eventtype
	TDBTEntityHandle Entity;     /// hEntity which owns this event
	uint32_t DataLength;         /// Length of the stored data in bytes

	uint8_t Reserved[8];         /// reserved storage
} TEvent;

#pragma pack(pop)


static const uint32_t cEventSignature = 0x365A7E92;
static const uint16_t cEventNodeSignature = 0x195C;

/**
	\brief Manages the Events Index in the Database
**/
class CEventsTree : public CFileBTree<TEventKey, 16>
{
private:
	TDBTEntityHandle m_Entity;

public:
	CEventsTree(CBlockManager & BlockManager, TNodeRef RootNode, TDBTEntityHandle Entity)
		:	CFileBTree<TEventKey, 16>::CFileBTree(BlockManager, RootNode, cEventNodeSignature),
			m_Entity(Entity)
		{	};
	~CEventsTree()
		{	};

	TDBTEntityHandle Entity()
		{
			return m_Entity;
		};
	void Entity(TDBTEntityHandle NewEntity)
		{
			m_Entity = NewEntity;
		};
};

/**
	\brief Manages the Virtual Events Index
	Sorry for duplicating code...
**/
class CVirtualEventsTree : public CBTree<TEventKey, 16>
{
private:
	TDBTEntityHandle m_Entity;

public:
	CVirtualEventsTree(TDBTEntityHandle Entity)
		:	CBTree<TEventKey, 16>::CBTree(0),
			m_Entity(Entity)
		{	};

	~CVirtualEventsTree()
		{	};

	TDBTEntityHandle Entity()
		{
			return m_Entity;
		};
	void Entity(TDBTEntityHandle NewEntity)
		{
			m_Entity = NewEntity;
		};
};


class CEventsTypeManager
{
public:
	CEventsTypeManager(CEntities & Entities, CSettings & Settings);
	~CEventsTypeManager();

	uint32_t MakeGlobalID(char* Module, uint32_t EventType);
	bool GetType(uint32_t GlobalID, char * & Module, uint32_t & EventType);
	uint32_t EnsureIDExists(char* Module, uint32_t EventType);

private:
	typedef struct TEventType {
		char *   ModuleName;
		uint32_t EventType;
	} TEventType, *PEventType;
	typedef stdext::hash_map<uint32_t, PEventType> TTypeMap;

	CEntities & m_Entities;
	CSettings & m_Settings;
	TTypeMap m_Map;

};


class CEvents : public sigslot::has_slots<>
{
public:

	CEvents(
		CBlockManager & BlockManager,
		CEncryptionManager & EncryptionManager,
		CEntities & Entities,
		CSettings & Settings
		);
	~CEvents();

	//compatibility
	TDBTEventHandle compFirstEvent(TDBTEntityHandle hEntity);
	TDBTEventHandle compFirstUnreadEvent(TDBTEntityHandle hEntity);
	TDBTEventHandle compLastEvent(TDBTEntityHandle hEntity);
	TDBTEventHandle compNextEvent(TDBTEventHandle hEvent);
	TDBTEventHandle compPrevEvent(TDBTEventHandle hEvent);

	//services
	unsigned int GetBlobSize(TDBTEventHandle hEvent);
	unsigned int Get(TDBTEventHandle hEvent, TDBTEvent & Event);
	unsigned int GetCount(TDBTEntityHandle hEntity);
	unsigned int Delete(TDBTEventHandle hEvent);
	TDBTEventHandle Add(TDBTEntityHandle hEntity, TDBTEvent & Event);
	unsigned int MarkRead(TDBTEventHandle hEvent);
	unsigned int WriteToDisk(TDBTEventHandle hEvent);

	TDBTEntityHandle getEntity(TDBTEventHandle hEvent);

	TDBTEventIterationHandle IterationInit(TDBTEventIterFilter & Filter);
	TDBTEventHandle IterationNext(TDBTEventIterationHandle Iteration);
	unsigned int IterationClose(TDBTEventIterationHandle Iteration);


private:
	typedef CBTree<TEventKey, 16> TEventBase;
	typedef struct  
	{
		CEventsTree * RealTree;
		CVirtualEventsTree * VirtualTree;
		uint32_t VirtualCount;
		TEventKey FirstVirtualUnread;
	} TEntityEventsRecord, *PEntityEventsRecord;
	typedef stdext::hash_map<TDBTEntityHandle, TEntityEventsRecord*> TEntityEventsMap;
	typedef CIterationHeap<TEventBase::iterator> TEventsHeap;

	CBlockManager & m_BlockManager;
	CEncryptionManager & m_EncryptionManager;

	CEntities & m_Entities;
	CEventsTypeManager m_Types;

	TEntityEventsMap m_EntityEventsMap;

	typedef struct TEventIteration {
		TDBTEventIterFilter Filter;
		TEventsHeap * Heap;
		TDBTEventHandle LastEvent;
	} TEventIteration, *PEventIteration;

	void onRootChanged(void* EventsTree, CEventsTree::TNodeRef NewRoot);

	void onDeleteEventCallback(void * Tree, const TEventKey & Key, uint32_t Param);
	void onDeleteVirtualEventCallback(void * Tree, const TEventKey & Key, uint32_t Param);
	void onDeleteEvents(CEntities * Entities, TDBTEntityHandle hEntity);
	void onTransferEvents(CEntities * Entities, TDBTEntityHandle Source, TDBTEntityHandle Dest);

	PEntityEventsRecord getEntityRecord(TDBTEntityHandle hEntity);
	uint32_t adjustVirtualEventCount(PEntityEventsRecord Record, int32_t Adjust);
	bool MarkEventsTree(TEventBase::iterator Iterator, TDBTEventHandle FirstUnread);
	void FindNextUnreadEvent(TEventBase::iterator & Iterator);
};
