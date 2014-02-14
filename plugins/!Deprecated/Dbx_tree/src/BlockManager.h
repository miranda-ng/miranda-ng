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

#include <windows.h>
#include <map>
#include <vector>

#include "stdint.h"
#include "FileAccess.h"
#include "EncryptionManager.h"
#include "MREWSync.h"
#include "Thread.h"
#include "intrinsics.h"

class CBlockManager
{
protected:
	static const uint32_t cFreeBlockID = 0xFFFFFFFF;

	static const uint32_t cJournalFlushBytes = (1 << 20) - 2048; // flush before reserved journal-space is exhausted
	static const uint32_t cJournalFlushTimeout = 300; // journal flush every 5 minutes

	static const uint32_t cCacheBuddyBits  = 10;
	static const uint32_t cCacheBuddyCount = 1 << cCacheBuddyBits; // count of static allocated buddy nodes
	static const uint32_t cCacheBuddyCheck = 0xffffffff << cCacheBuddyBits;

	static const uint32_t cCacheMinimumTimeout = 2; // purge less than every n seconds (high priority)
	static const uint32_t cCacheMaximumTimeout = 600; // purge every 10 minutes (high priority)
	static const uint32_t cCachePurgeSize = 1 << 21; // cache up to 2MB
	static const uint32_t cCacheMinimumGrowthForPurge = 1 << 19; // cache only when 512kb were added
	
	#pragma pack(push, 1)  // push current alignment to stack, set alignment to 1 byte boundary

	typedef struct TBlockHeadFree {
		uint32_t ID;
		uint32_t Size;
	} TBlockHeadFree;
	typedef struct TBlockHeadOcc {
		uint32_t ID;
		uint32_t Size;
		uint32_t Signature; /// if occupied block
	}	TBlockHeadOcc;

	typedef struct TBlockTailOcc {
		uint32_t ID;
	} TBlockTailOcc;

	typedef struct TBlockTailFree {
		uint32_t Size; /// if free block
		uint32_t ID;
	} TBlockTailFree;

	#pragma pack(pop)

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Block table entry. </summary>
	///
	/// Addr   Deleted   InCache   Meaning
	///    0         0         0   successfully deleted block
	///    0         0         1   virtual only block (either forced virtual or created on a read-only file)
	///    0         1         0   invalid
	///    0         1         1   FreeID list (last entry)
	///  set         0         0   Normal in-file block
	///  set         0         1   in file and cache (normal cache which could differ on a read-only file or forced virtual out of a read-only file - check TCacheEntry)
	///  set         1         0   deleted block or a read-only file
	///  set         1         1   FreeID list entry
	///   
	/// <remarks>	Michael "Protogenes" Kunz, 07.09.2010. </remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef struct TBlockTableEntry {
		uint32_t Deleted : 1;	///< Flag is set if the block was deleted but can't be removed, because the file is read-only
		uint32_t InCache : 1;	///< Flag is set if block is in the cache (either forced virtual or from the file)
		uint32_t Addr : 30;	///< The Offset in the file div 4, so we can address files up to 4GB
	} TBlockTableEntry;
	std::vector<TBlockTableEntry> m_BlockTable;
	
	struct TPendingOperation;
	typedef struct TCacheEntry {
		TCacheEntry * volatile Next;
		TBlockHeadOcc * volatile Cache;
		TPendingOperation * Pending;

		uint32_t Idx;
		uint32_t Forced : 1;
		uint32_t KeepInCache : 1;
		uint32_t LastUse : 30;
	} TCacheEntry;
	
	TCacheEntry * m_Cache[cCacheBuddyCount];

	struct {
		uint32_t volatile Size;
		uint32_t volatile Growth;
		uint32_t volatile LastPurge;
	} m_CacheInfo;

	CFileAccess & m_FileAccess;
	CEncryptionManager & m_EncryptionManager;
	CMultiReadExclusiveWriteSynchronizer m_BlockSync;

	uint32_t m_FirstBlockStart;
	bool m_SaveMode;
	bool m_ReadOnly;

	typedef std::multimap<uint32_t, uint32_t> TFreeBlockMap;
	TFreeBlockMap m_FreeBlocks;
	uint32_t m_FirstFreeIndex;

	static const uint32_t cPendingInvalidate = 0x00000001;
	typedef struct TPendingOperation {
		TPendingOperation * Next;	///< The next
		TPendingOperation * Prev;	///< The previous
		uint32_t BlockID;	///< Identifier for the block
		uint32_t Addr;	///< The address in the file
		uint32_t Size;	///< The size of the block
		TCacheEntry * CacheEntry;	///< The cache entry
		TBlockHeadOcc * EncryptionBuffer;	///< Buffer for encrypted block
	} TPendingOperation;
	
	TPendingOperation * m_PendingHead;	///< The double linked list head
	TPendingOperation * m_PendingTail;	///< The double linked list tail
	TPendingOperation * m_PendingLast;	///< The last processed item

	uint32_t m_LastFlush;	///< The last flush timestamp
	uint32_t m_BytesPending;	///< The bytes pending for write

	class COptimizeThread : public CThread
	{
		protected:
			CBlockManager & m_Owner;
			void Execute() { m_Owner.ExecuteOptimize(); };
		public:
			COptimizeThread(CBlockManager & Owner) : CThread(true), m_Owner(Owner) {};
			~COptimizeThread() {};
	};

	struct {
		uint32_t Source;
		uint32_t Dest;
		COptimizeThread * Thread;
	} m_Optimize;
	void ExecuteOptimize();
		
	uint32_t _GetAvailableIndex();
	void _InsertFreeBlock(uint32_t Addr, uint32_t Size, bool InvalidateData, bool Reuse);
	void _RemoveFreeBlock(uint32_t Addr, uint32_t Size);
	uint32_t _FindFreePosition(uint32_t Size);

	bool _InitOperation(uint32_t BlockID, uint32_t & Addr, TCacheEntry * & Cache);
	void _UpdateBlock(uint32_t BlockID, TCacheEntry * CacheEntry, uint32_t Addr);

	void * _ReadBlock(uint32_t BlockID, uint32_t & Size, uint32_t & Signature);
	void * _CreateBlock(uint32_t & BlockID, const uint32_t Signature, uint32_t Size);
	void * _CreateBlockVirtual(uint32_t & BlockID, const uint32_t Signature, uint32_t Size);
	uint32_t _ResizeBlock(uint32_t BlockID, void * & Buffer, uint32_t Size);

	TCacheEntry * _CacheInsert(uint32_t Idx, TBlockHeadOcc * Cache, bool Virtual);
	TCacheEntry * _CacheFind(uint32_t Idx);
	void _CacheErase(uint32_t Idx);

	void _CachePurge();
	void _PendingAdd(uint32_t BlockID, uint32_t Addr, uint32_t Size, TCacheEntry * Cache);
	void _PendingRemove(TPendingOperation * Pending, bool Free);
	void _PendingFlush(bool FullFlush);


	
	void TransactionBeginRead()
		{
			m_BlockSync.BeginRead();
		};

	void TransactionEndRead()
		{
			m_BlockSync.EndRead();
		};

	void TransactionBeginWrite()
		{
			m_BlockSync.BeginWrite();
			m_FileAccess.UseJournal(m_SaveMode);
		};

	void TransactionEndWrite()
		{
			if (m_BlockSync.WriteRecursionCount() == 1)
			{
				m_FileAccess.CompleteTransaction();
				m_BytesPending += 12;

				if ((m_CacheInfo.LastPurge + cCacheMaximumTimeout < time(NULL))
					|| ((m_CacheInfo.Size + m_CacheInfo.Growth > cCachePurgeSize) 
					&& (m_CacheInfo.Growth > cCacheMinimumGrowthForPurge)
					&& (m_CacheInfo.LastPurge + cCacheMinimumTimeout < time(NULL))))
				{
					_CachePurge();
				} else if ((m_BytesPending >= cJournalFlushBytes) || (time(NULL) > m_LastFlush + cJournalFlushTimeout))
				{
					_PendingFlush(true);
				} else {
					_PendingFlush(false);
				}
			}

			m_BlockSync.EndWrite();
		};
public:
	CBlockManager(CFileAccess & FileAccess, CEncryptionManager & EncryptionManager);
	~CBlockManager();


	class ReadTransaction
	{
		private:
			CBlockManager * m_Owner;
			uint32_t volatile * m_RefCount;
			bool m_Closed;
		public:
			ReadTransaction()
				:	m_Owner(NULL),
					 m_RefCount(NULL),
					 m_Closed(true)
				{

				};
			ReadTransaction(CBlockManager & BlockManager)
				:	m_Owner(&BlockManager),
					m_RefCount(new uint32_t(1)),
					m_Closed(false)
			{
				m_Owner->TransactionBeginRead();
			};
			ReadTransaction(const ReadTransaction & Other)
				: m_Owner(Other.m_Owner),
					m_RefCount(Other.m_RefCount),
					m_Closed(Other.m_Closed)
			{
				if (!m_Closed)
					INC_32(*m_RefCount);
			};
			~ReadTransaction()
			{
				if (!m_Closed && (DEC_32(*m_RefCount) == 0))
				{
					delete m_RefCount;
					m_Owner->TransactionEndRead();
				}
			};

			ReadTransaction & operator =(const ReadTransaction & Other)
			{
				if (!m_Closed && (DEC_32(*m_RefCount) == 0))
				{
					delete m_RefCount;
					m_Owner->TransactionEndRead();
				}

				m_Owner = Other.m_Owner;
				m_RefCount = Other.m_RefCount;
				m_Closed = Other.m_Closed;
				if (!m_Closed)
					INC_32(*m_RefCount);

				return *this;
			}

			void Close()
			{
				if (!m_Closed && (DEC_32(*m_RefCount) == 0))
				{
					delete m_RefCount;
					m_Owner->TransactionEndRead();
				}
				m_Closed = true;
			}

	};
	class WriteTransaction
	{
	private:
		CBlockManager * m_Owner;
		uint32_t volatile * m_RefCount;
		bool m_Closed;
	public:
		WriteTransaction()
			:	m_Owner(NULL),
				m_RefCount(NULL),
				m_Closed(true)
		{

		};
		WriteTransaction(CBlockManager & BlockManager)
			:	m_Owner(&BlockManager),
				m_RefCount(new uint32_t(1)),
				m_Closed(false)
		{
			m_Owner->TransactionBeginWrite();
		};
		WriteTransaction(const WriteTransaction & Other)
			: m_Owner(Other.m_Owner),
				m_RefCount(Other.m_RefCount),
				m_Closed(Other.m_Closed)
		{
			if (!m_Closed)
				INC_32(*m_RefCount);
		};
		~WriteTransaction()
		{
			if (!m_Closed && (DEC_32(*m_RefCount) == 0))
			{
				delete m_RefCount;
				m_Owner->TransactionEndWrite();
			}
		};

		WriteTransaction & operator =(const WriteTransaction & Other)
		{
			if (!m_Closed && (DEC_32(*m_RefCount) == 0))
			{
				delete m_RefCount;
				m_Owner->TransactionEndWrite();
			}

			m_Owner = Other.m_Owner;
			m_RefCount = Other.m_RefCount;
			m_Closed = Other.m_Closed;
			if (!m_Closed)
				INC_32(*m_RefCount);

			return *this;
		}

		void Close()
		{
			if (!m_Closed && (DEC_32(*m_RefCount) == 0))
			{
				delete m_RefCount;
				m_Owner->TransactionEndWrite();
			}
			m_Closed = true;
		}

	};

	uint32_t ScanFile(uint32_t FirstBlockStart, uint32_t HeaderSignature, uint32_t FileSize);

	template <typename BlockType>
	BlockType * ReadBlock(uint32_t BlockID, uint32_t & Size, uint32_t & Signature)
		{
			return reinterpret_cast<BlockType*>(_ReadBlock(BlockID, Size, Signature));
		};

	template <typename BlockType>
	BlockType * CreateBlock(uint32_t & BlockID, const uint32_t Signature, uint32_t Size = sizeof(BlockType))
		{
			return reinterpret_cast<BlockType*>(_CreateBlock(BlockID, Signature, Size));
		};

	template <typename BlockType>
	BlockType * CreateBlockVirtual(uint32_t & BlockID, const uint32_t Signature, uint32_t Size = sizeof(BlockType))
		{
			return reinterpret_cast<BlockType*>(_CreateBlockVirtual(BlockID, Signature, Size));
		};

	template <typename BlockType>
	uint32_t ResizeBlock(uint32_t BlockID, BlockType * & Buffer, uint32_t Size)
		{
			void * tmp = Buffer;
			uint32_t res = _ResizeBlock(BlockID, tmp, Size);
			Buffer = reinterpret_cast<BlockType*>(tmp);
			return res;
		};

	bool UpdateBlock(uint32_t BlockID, uint32_t Signature = 0);
	bool DeleteBlock(uint32_t BlockID);

	bool IsForcedVirtual(uint32_t BlockID);
	bool WriteBlockToDisk(uint32_t BlockID);
	bool MakeBlockVirtual(uint32_t BlockID);
};
