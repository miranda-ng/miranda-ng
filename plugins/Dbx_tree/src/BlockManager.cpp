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
#include "BlockManager.h"
#include "Logger.h"

CBlockManager::CBlockManager(
	CFileAccess & FileAccess,
	CEncryptionManager & EncryptionManager
	)
:	m_BlockSync(),
	m_FileAccess(FileAccess),
	m_EncryptionManager(EncryptionManager),
	m_BlockTable(1024),
	m_FreeBlocks()
{
	m_Optimize.Thread = NULL;
	m_Optimize.Source = 0;
	m_Optimize.Dest = 0;

	m_CacheInfo.Growth = 0;
	m_CacheInfo.Size = 0;
	m_CacheInfo.LastPurge = time(NULL);

	m_PendingHead = NULL;
	m_PendingTail = NULL;
	m_PendingLast = NULL;
	m_LastFlush = time(NULL);
	m_BytesPending = 20;
	m_FirstFreeIndex = 0;

	m_SaveMode = true;
	m_ReadOnly = m_FileAccess.ReadOnly();

	memset(m_Cache, 0, sizeof(m_Cache));
}

CBlockManager::~CBlockManager()
{
	m_BlockSync.BeginWrite();
	if (m_Optimize.Thread)
	{
		m_Optimize.Thread->FreeOnTerminate(false);
		m_Optimize.Thread->Terminate();
		
		m_BlockSync.EndWrite();
		m_Optimize.Thread->WaitFor();

		delete m_Optimize.Thread;
	} else {
		m_BlockSync.EndWrite();
	}

	_PendingFlush(true);
	
	for (uint32_t buddy = 0; buddy < cCacheBuddyCount; buddy++)
	{
		TCacheEntry * i = m_Cache[buddy];
		while (i)
		{
			free(i->Cache);

			TCacheEntry * tmp = i;
			i = i->Next;
			free(tmp);
		}
	}
}

// Optimize File Size
void CBlockManager::ExecuteOptimize()
{ /*
	TBlockHeadFree h = {0,0};
	uint8_t * buf = (uint8_t*)malloc(1 << 18); // 256kb
	uint32_t bufuse = 0;
	uint32_t bufsize = 1 << 18;
	uint32_t lastflush = 0;

	{
		int i = 0;
		while (!m_Optimize.Thread->Terminated() && (i < 600))
		{
			++i;
			Sleep(100); // wait for Miranda to start
		}
	}

	TransactionBeginWrite();

	while (!m_Optimize.Thread->Terminated() && (m_Optimize.Source < m_FileAccess.Size()) && !m_ReadOnly)
	{
		m_FileAccess.Read(&h, m_Optimize.Source, sizeof(h));
		if (h.ID == cFreeBlockID)
		{
			_RemoveFreeBlock(m_Optimize.Source, h.Size);

			m_Optimize.Source += h.Size;
		} else {
			
			if (bufsize < bufuse + h.Size)
			{
				buf = (uint8_t*)realloc(buf, bufuse + h.Size);
				bufsize = bufuse + h.Size;
			}
			m_FileAccess.Read(buf + bufuse, m_Optimize.Source, h.Size);
			
			m_BlockTable[h.ID >> 2].Addr = (m_Optimize.Dest + bufuse) >> 2;

			m_Optimize.Source += h.Size;
			bufuse += h.Size;
		}
		
		if ((m_BlockSync.Waiting() > 0) 
			|| (bufuse + 1024 >= bufsize)
			|| (m_Optimize.Source >= m_FileAccess.Size())) // buffer is nearly full or EOF
		{
			if (m_Optimize.Dest != m_Optimize.Source) // move blocks
			{
				TBlockHeadFree h = {cFreeBlockID, m_Optimize.Source - m_Optimize.Dest};
				TBlockTailFree t = {m_Optimize.Source - m_Optimize.Dest, cFreeBlockID};

				m_FileAccess.Write(buf, m_Optimize.Dest, bufuse);

				m_FileAccess.Write(&h, m_Optimize.Dest + bufuse, sizeof(h));
				m_FileAccess.Invalidate(m_Optimize.Dest + bufuse + sizeof(h), m_Optimize.Source - m_Optimize.Dest - bufuse - sizeof(h) - sizeof(t));
				m_FileAccess.Write(&t, m_Optimize.Dest + bufuse - sizeof(t), sizeof(t));

				if (m_SaveMode)
				{
					m_FileAccess.CloseTransaction();
					m_FileAccess.Flush();
					m_FileAccess.UseJournal(false);
					
					m_FileAccess.Write(buf, m_Optimize.Dest, bufuse);

					m_FileAccess.Write(&h, m_Optimize.Dest + bufuse, sizeof(h));
					m_FileAccess.Invalidate(m_Optimize.Dest + bufuse + sizeof(h), m_Optimize.Source - m_Optimize.Dest - bufuse - sizeof(h) - sizeof(t));
					m_FileAccess.Write(&t, m_Optimize.Dest + bufuse - sizeof(t), sizeof(t));			
					
					m_FileAccess.Flush();
					m_FileAccess.CleanJournal();
					m_FileAccess.UseJournal(true);
				}

				m_Optimize.Dest += bufuse;
				bufuse = 0;
			}

			if (m_BlockSync.Waiting() > 0)
			{
				unsigned int w = m_BlockSync.Waiting();
				m_BlockSync.EndWrite();
				Sleep(w * 64 + 1);
				m_BlockSync.BeginWrite();
				m_FileAccess.UseJournal(m_SaveMode);
			}
		}
	}

	if (m_Optimize.Source >= m_FileAccess.Size())
		m_FileAccess.Size(m_Optimize.Dest);
	
	m_Optimize.Thread = NULL;
	m_Optimize.Source = 0;
	m_Optimize.Dest = 0;
	if (m_SaveMode)
		TransactionEndWrite();
	else
		m_BlockSync.EndWrite();

	free(buf); */

	m_Optimize.Thread = NULL;
}

inline void CBlockManager::_PendingAdd(uint32_t BlockID, uint32_t Addr, uint32_t Size, TCacheEntry * Cache)
{
	TPendingOperation * p = NULL;
	if (BlockID == cFreeBlockID)
	{
		p = (TPendingOperation*)malloc(sizeof(TPendingOperation));

		p->BlockID = cFreeBlockID;
		p->Addr = Addr;
		p->Size = Size;
		p->CacheEntry = NULL;
		p->EncryptionBuffer = NULL;
		
		m_BytesPending += 24 + sizeof(TBlockHeadFree) + sizeof(TBlockTailFree);
		if (Addr & cPendingInvalidate)
			m_BytesPending += 12;

	} else {
		if (Cache->Pending)
		{
			p = Cache->Pending;
			_PendingRemove(Cache->Pending, false);
		} else {
			p = (TPendingOperation*)malloc(sizeof(TPendingOperation));
		}
	
		p->BlockID = BlockID;
		p->Addr = Addr;
		p->Size = Size;
		p->CacheEntry = Cache;
		p->EncryptionBuffer = NULL;

		m_BytesPending += 12 + Size;
	
		Cache->Pending = p;
	}

	p->Next = NULL;
	p->Prev = m_PendingTail;
	if (m_PendingTail)
		m_PendingTail->Next = p;

	m_PendingTail = p;
	if (!m_PendingHead)
		m_PendingHead = p;
}

inline void CBlockManager::_PendingRemove(TPendingOperation * Pending, bool Free)
{
	if (Pending->Prev)
		Pending->Prev->Next = Pending->Next;
	else
		m_PendingHead = Pending->Next;

	if (Pending->Next)
		Pending->Next->Prev = Pending->Prev;
	else
		m_PendingTail = Pending->Prev;

	free(Pending->EncryptionBuffer);

	if (m_PendingLast == Pending)
		m_PendingLast = Pending->Prev;
	
	Pending->CacheEntry->Pending = NULL;
	if (Free)
		free(Pending);
}

inline void CBlockManager::_PendingFlush(bool FullFlush)
{
	TPendingOperation * i = NULL;

	if (m_ReadOnly)
		return;

	if (FullFlush)
	{
		if (m_SaveMode)
		{
			_PendingFlush(false); // write to journal
			m_FileAccess.Flush();
			m_FileAccess.UseJournal(false);
			m_FileAccess.Size(m_FileAccess.Size()); // resize real file
		} else {
			m_FileAccess.UseJournal(false);
		}

		i = m_PendingHead;
	} else if (m_PendingLast)
	{
		i = m_PendingLast->Next;
		m_FileAccess.UseJournal(m_SaveMode);
	} else {
		i = m_PendingHead;
		m_FileAccess.UseJournal(m_SaveMode);
	}
		
	while (i)
	{
		if (i->BlockID == cFreeBlockID)
		{
			uint32_t addr = i->Addr & ~cPendingInvalidate;
			if (addr + i->Size <= m_FileAccess.Size())
			{
				TBlockHeadFree h = {cFreeBlockID, i->Size};
				TBlockTailFree t = {i->Size, cFreeBlockID};

				m_FileAccess.Write(&h, addr, sizeof(h));
				if (i->Addr & cPendingInvalidate)
					m_FileAccess.Invalidate(addr + sizeof(h), i->Size - sizeof(h) - sizeof(t));
				m_FileAccess.Write(&t, addr + i->Size - sizeof(t), sizeof(t));
			}
		
		} else {				

			if (i->BlockID && !i->EncryptionBuffer && m_EncryptionManager.IsEncrypted(i->BlockID))
			{
				i->EncryptionBuffer = (TBlockHeadOcc*) malloc(i->Size);
				memcpy(i->EncryptionBuffer, i->CacheEntry->Cache, i->Size);
				m_EncryptionManager.Encrypt(i->EncryptionBuffer + 1, i->Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc), i->BlockID, 0);
			}

			if (i->EncryptionBuffer)
			{
				m_FileAccess.Write(i->EncryptionBuffer, i->Addr, i->Size);
			} else {
				m_FileAccess.Write(i->CacheEntry->Cache, i->Addr, i->Size);
			}
		}
		
		i = i->Next;
	} // while

	if (FullFlush)
	{
		m_FileAccess.Flush();
		if (m_SaveMode)
			m_FileAccess.CleanJournal();

		m_BytesPending = 20;
		m_LastFlush = time(NULL);
		
		i = m_PendingHead;
		while (i)
		{
			free(i->EncryptionBuffer);

			if (i->CacheEntry)
				i->CacheEntry->Pending = NULL;

			TPendingOperation * tmp = i;
			i = i->Next;
			free(tmp);
		}
		m_PendingHead = NULL;
		m_PendingTail = NULL;
		m_PendingLast = NULL;
	}	else {
		m_PendingLast = m_PendingTail;
		m_FileAccess.CloseTransaction();
	}
}

inline CBlockManager::TCacheEntry * CBlockManager::_CacheInsert(uint32_t Idx, TBlockHeadOcc * Cache, bool Virtual)
{
	TCacheEntry * res;
	uint32_t myidx = ROR_32(Idx, cCacheBuddyBits);
	
	res = (TCacheEntry *)malloc(sizeof(TCacheEntry));
	res->Cache = Cache;
	res->Pending = NULL;
	res->Idx = myidx;
	res->Forced = Virtual;
		
	TCacheEntry * volatile * last = &m_Cache[Idx % cCacheBuddyCount];
	TCacheEntry * i;
	do {
		i = *last;

		while (i && (i->Idx < myidx))
		{
			last = &i->Next;
			i = i->Next;
		}
			
		if (i && (i->Idx == myidx))
		{
			free(res);
			free(Cache);

			i->LastUse = time(NULL) >> 2;
			return i;
		}

		res->Next = i;

	} while (i != CMPXCHG_Ptr(*last, res, i));

	res->LastUse = time(NULL) >> 2;

	m_BlockTable[Idx].InCache = true;
	if (!Virtual)
		XADD_32(m_CacheInfo.Growth, res->Cache->Size);

	return res;
}

inline CBlockManager::TCacheEntry * CBlockManager::_CacheFind(uint32_t Idx)
{
	TCacheEntry * i = m_Cache[Idx % cCacheBuddyCount];
	uint32_t myidx = ROR_32(Idx, cCacheBuddyBits);
	while (i && (i->Idx < myidx))
		i = i->Next;

	if (i && (i->Idx == myidx))
		return i;
	else
		return NULL;
}

inline void CBlockManager::_CacheErase(uint32_t Idx)
{
	TCacheEntry * i = m_Cache[Idx % cCacheBuddyCount];
	TCacheEntry * volatile * l = &m_Cache[Idx % cCacheBuddyCount];

	uint32_t myidx = ROR_32(Idx, cCacheBuddyBits);

	while (i->Idx < myidx)
	{
		l = &i->Next;
		i = i->Next;
	}
	*l = i->Next;
		
	free(i->Cache);
	free(i);
}

inline void CBlockManager::_CachePurge()
{
	_PendingFlush(true);
	
	uint32_t ts = time(NULL);
	if (m_CacheInfo.Size + m_CacheInfo.Growth > cCachePurgeSize) {
		ts = ts - (ts - m_CacheInfo.LastPurge) * cCachePurgeSize / (m_CacheInfo.Size + 2 * m_CacheInfo.Growth);
	} else if (m_CacheInfo.Growth > m_CacheInfo.Size)
	{
		ts = ts - (ts - m_CacheInfo.LastPurge) * m_CacheInfo.Size / m_CacheInfo.Growth;
	} else if (m_CacheInfo.Size > m_CacheInfo.Growth)
	{
		ts = ts - (ts - m_CacheInfo.LastPurge) * m_CacheInfo.Growth / m_CacheInfo.Size;
	} else {
		ts = m_CacheInfo.LastPurge;
	}

	m_CacheInfo.Size += m_CacheInfo.Growth;
	m_CacheInfo.Growth = 0;
	m_CacheInfo.LastPurge = time(NULL);

	for (uint32_t buddy = 0; buddy < cCacheBuddyCount; buddy++)
	{
		TCacheEntry * i = m_Cache[buddy];
		TCacheEntry * volatile * l = &m_Cache[buddy];
	
		while (i)
		{
			if (!i->Forced && !i->KeepInCache && i->Idx && ((i->LastUse << 2) < ts))
			{
				uint32_t idx = ROL_32(i->Idx, cCacheBuddyBits);
				m_CacheInfo.Size -= i->Cache->Size;
				m_BlockTable[idx].InCache = false;
				free(i->Cache);

				*l = i->Next;
				TCacheEntry * tmp = i;
				i = i->Next;
				free(tmp);
				
			} else {
				l = &i->Next;
				i = i->Next;
			}
		}
	}
}

inline uint32_t CBlockManager::_GetAvailableIndex()
{
	uint32_t id;
	if (m_FirstFreeIndex)
	{
		id = m_FirstFreeIndex;
		m_FirstFreeIndex = m_BlockTable[id].Addr;
		TBlockTableEntry b = {false, false, 0};
		m_BlockTable[id] = b;
	} else {
		id = static_cast<uint32_t>(m_BlockTable.size());
		if (id > (1 << 12))
			m_BlockTable.resize(id + (1 << 12));
		else
			m_BlockTable.resize(id * 2);

		for (uint32_t i = static_cast<uint32_t>(m_BlockTable.size() - 1); i > id; --i)
		{
			TBlockTableEntry b = {true, true, m_FirstFreeIndex};
			m_BlockTable[i] = b;
			m_FirstFreeIndex = i;
		}
	}
	return id;
}

inline void CBlockManager::_InsertFreeBlock(uint32_t Addr, uint32_t Size, bool InvalidateData, bool Reuse)
{
	if (Addr + Size == m_FileAccess.Size())
	{
		if (Reuse) // in FindFreePosition we would want to use that block
			m_FileAccess.Size(Addr);
	} else {

		if (Reuse)
			m_FreeBlocks.insert(std::make_pair(Size, Addr));

		if (!m_ReadOnly)
			_PendingAdd(cFreeBlockID, InvalidateData ? Addr | cPendingInvalidate : Addr, Size, NULL);
		
	}
}

inline void CBlockManager::_RemoveFreeBlock(uint32_t Addr, uint32_t Size)
{
	TFreeBlockMap::iterator i = m_FreeBlocks.find(Size);
	while ((i != m_FreeBlocks.end()) && (i->first == Size))
	{
		if (i->second == Addr)
		{
			m_FreeBlocks.erase(i);
			i = m_FreeBlocks.end();
		} else {
			++i;
		}
	}
}

inline uint32_t CBlockManager::_FindFreePosition(uint32_t Size)
{
	// try to find free block
	TFreeBlockMap::iterator f;
	TFreeBlockMap::iterator e = m_FreeBlocks.end();

	if (m_FreeBlocks.size())
	{
		f = m_FreeBlocks.find(Size);
		if (f == e)
		{
			if (m_FreeBlocks.rbegin()->first > Size * 2)
			{
				f = m_FreeBlocks.end();
				--f;
			}			
		}
	} else {
		f = e;
	}

	uint32_t addr = 0;

	if (f == e) // no block found - expand file
	{
		addr = m_FileAccess.Size();
		m_FileAccess.Size(addr + Size);
		
	} else {
		addr = f->second;

		if (f->first != Size)
		{
			_InsertFreeBlock(addr + Size, f->first - Size, false, true);
		}
		_InsertFreeBlock(addr, Size, false, false);
		
		m_FreeBlocks.erase(f);
	}

	return addr;
}

inline bool CBlockManager::_InitOperation(uint32_t BlockID, uint32_t & Addr, TCacheEntry * & Cache)
{
	if (!BlockID || (BlockID & 3) || ((BlockID >> 2) >= m_BlockTable.size()))
		return false;

	uint32_t idx = BlockID >> 2;
	TBlockTableEntry dat = m_BlockTable[idx];

	if (dat.Deleted) // deleted or FreeIDList item
		return false;

	Addr = dat.Addr << 2;
	if (dat.InCache)
	{
		Cache = _CacheFind(idx);
	} else if (Addr)
	{
		TBlockHeadOcc h;
		
		m_FileAccess.Read(&h, Addr, sizeof(h));

		TBlockHeadOcc * block = (TBlockHeadOcc *) malloc(h.Size);
		m_FileAccess.Read(block, Addr, h.Size);
		
		m_EncryptionManager.Decrypt(block + 1, h.Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc), BlockID, 0);

		Cache = _CacheInsert(idx, block, false);
	} else {
		return false;
	}

	return Cache != NULL;
}

inline void CBlockManager::_UpdateBlock(uint32_t BlockID, TCacheEntry * CacheEntry, uint32_t Addr) 
{
	CacheEntry->KeepInCache = m_ReadOnly;

	if (!CacheEntry->Forced)
	{
		if (!m_ReadOnly)
			_PendingAdd(BlockID, Addr, CacheEntry->Cache->Size, CacheEntry);
	}
}


uint32_t CBlockManager::ScanFile(uint32_t FirstBlockStart, uint32_t HeaderSignature, uint32_t FileSize)
{
	TBlockHeadOcc h, lasth = {0, 0, 0};
	uint32_t p;
	uint32_t res = 0;
	bool invalidateblock = false;

	p = FirstBlockStart;
	m_FirstBlockStart = FirstBlockStart;
	m_Optimize.Source = 0;
	m_Optimize.Dest = 0;

	{ // insert header cache element
		void * header = malloc(FirstBlockStart);
		m_FileAccess.Read(header, 0, FirstBlockStart);
		_CacheInsert(0, (TBlockHeadOcc*)header, false);
	}

	TransactionBeginWrite();

	while (p < FileSize)
	{
		m_FileAccess.Read(&h, p, sizeof(h));
		if (CLogger::Instance().Level() >= CLogger::logERROR || !h.Size)
		{
			LOG(logCRITICAL, _T("Block-structure of file is corrupt!"));
			return 0;
		}

		if (h.ID == cFreeBlockID)
		{
			if (m_Optimize.Dest == 0)
				m_Optimize.Dest = p;

			if (lasth.ID == cFreeBlockID)
			{				
				lasth.Size += h.Size;
				invalidateblock = true;
			} else {				
				lasth = h;
			}

		} else {

			if (lasth.ID == cFreeBlockID)
			{
				if (m_Optimize.Source == 0)
					m_Optimize.Source = p;

				_InsertFreeBlock(p - lasth.Size, lasth.Size, invalidateblock, true);
			}
			lasth = h;
			invalidateblock = false;

			while ((h.ID >> 2) >= m_BlockTable.size())
				m_BlockTable.resize(m_BlockTable.size() << 1);

			m_BlockTable[h.ID >> 2].Addr = p >> 2;

			if (h.Signature == HeaderSignature)
				res = h.ID;
		}

		p = p + h.Size;
	}

	m_FirstFreeIndex = 0;
	for (uint32_t i = static_cast<uint32_t>(m_BlockTable.size() - 1); i > 0; --i)
	{
		if (m_BlockTable[i].Addr == 0)
		{
			TBlockTableEntry b = {true, true, m_FirstFreeIndex};
			m_BlockTable[i] = b;
			m_FirstFreeIndex = i;
		}
	}

	TransactionEndWrite();

	if (m_Optimize.Source && !m_FileAccess.ReadOnly())
	{
		m_Optimize.Thread = new COptimizeThread(*this);
		m_Optimize.Thread->Priority(CThread::tpLowest);
		m_Optimize.Thread->FreeOnTerminate(true);
		m_Optimize.Thread->Resume();
	}

	return res;
}

void * CBlockManager::_CreateBlock(uint32_t & BlockID, const uint32_t Signature, uint32_t Size)
{
	uint32_t idx = _GetAvailableIndex();
	BlockID = idx << 2;

	Size = m_EncryptionManager.AlignSize(BlockID, (Size + 3) & 0xfffffffc); // align on cipher after we aligned on 4 bytes

	TBlockHeadOcc h = {BlockID, Size + sizeof(TBlockHeadOcc) + sizeof(TBlockTailOcc), Signature};
	TBlockTailOcc t = {BlockID};

	TBlockHeadOcc * block = (TBlockHeadOcc*) malloc(Size + sizeof(h) + sizeof(t));
	*block = h;
	memset(block + 1, 0, Size);
	*(TBlockTailOcc*)(((uint8_t*)(block + 1)) + Size) = t;

	TCacheEntry * ce = _CacheInsert(idx, block, false);

	if (m_ReadOnly)
	{
		TBlockTableEntry b = {false, true, 0};
		m_BlockTable[idx] = b;
	} else {
		uint32_t addr = _FindFreePosition(Size + sizeof(h) + sizeof(t));
		TBlockTableEntry b = {false, true, addr >> 2 };
		m_BlockTable[idx] = b;

		_UpdateBlock(BlockID, ce, addr);
	}

	return ce->Cache + 1;
}

void * CBlockManager::_CreateBlockVirtual(uint32_t & BlockID, const uint32_t Signature, uint32_t Size)
{
	uint32_t idx = _GetAvailableIndex();
	BlockID = idx << 2;

	Size = m_EncryptionManager.AlignSize(BlockID, (Size + 3) & 0xfffffffc); // align on cipher after we aligned on 4 bytes

	TBlockHeadOcc h = {BlockID, Size + sizeof(TBlockHeadOcc) + sizeof(TBlockTailOcc), Signature};
	TBlockTailOcc t = {BlockID};

	TBlockHeadOcc * block = (TBlockHeadOcc*) malloc(Size + sizeof(h) + sizeof(t));
	*block = h;
	memset(block + 1, 0, Size);
	*(TBlockTailOcc*)(((uint8_t*)(block + 1)) + Size) = t;

	return _CacheInsert(idx, block, true)->Cache + 1;
}

bool CBlockManager::DeleteBlock(uint32_t BlockID)
{
	uint32_t idx = BlockID >> 2;
	uint32_t addr;
	uint32_t size;
	TCacheEntry * ce;
	if (!_InitOperation(BlockID, addr, ce))
		return false;

	if (!ce->Forced)
		XADD_32(m_CacheInfo.Size, 0 - ce->Cache->Size);

	if (ce->Pending)
		_PendingRemove(ce->Pending, true);

	size = ce->Cache->Size;
	_CacheErase(idx);

	if (addr == 0) // Block in memory only
	{
		TBlockTableEntry b = {false, false, 0};
		m_BlockTable[idx] = b;
		
	} else if (m_ReadOnly) 
	{
		m_BlockTable[idx].Deleted = true;
		m_BlockTable[idx].InCache = false;
	} else {
		_InsertFreeBlock(addr, size, true, true);
		
		TBlockTableEntry b = {false, false, 0};
		m_BlockTable[idx] = b;
	}

	return true;
}

uint32_t CBlockManager::_ResizeBlock(uint32_t BlockID, void * & Buffer, uint32_t Size)
{
	uint32_t idx = BlockID >> 2;
	uint32_t addr;
	TCacheEntry * ce;

	if (Size == 0)
		return 0;

	if (!_InitOperation(BlockID, addr, ce))
		return 0;

	Size = m_EncryptionManager.AlignSize(BlockID, (Size + 3) & 0xfffffffc); // align on cipher after we aligned on 4 bytes

	uint32_t os = ce->Cache->Size;
	uint32_t ns = Size + sizeof(TBlockHeadOcc) + sizeof(TBlockTailOcc);
	if (ns == ce->Cache->Size)
	{
		Buffer = ce->Cache + 1;
		return Size;
	}

	ce->Cache = (TBlockHeadOcc*) realloc(ce->Cache, ns);
	ce->Cache->Size = ns;
	TBlockTailOcc t = {BlockID};
	*(TBlockTailOcc*)(((uint8_t*)(ce->Cache + 1)) + Size) = t;
	XADD_32(m_CacheInfo.Size, ns - os);

	Buffer = ce->Cache + 1;
	ce->KeepInCache = m_ReadOnly;

	if (!m_ReadOnly && addr)
	{
		_InsertFreeBlock(addr, os, true, true);
		addr = _FindFreePosition(ns);
		m_BlockTable[idx].Addr = addr >> 2;

		_UpdateBlock(BlockID, ce, addr); // write down
	}

	return Size;
}

bool CBlockManager::IsForcedVirtual(uint32_t BlockID)
{
	TCacheEntry * ce = _CacheFind(BlockID >> 2);
	return ce && ce->Forced;
}

bool CBlockManager::WriteBlockToDisk(uint32_t BlockID)
{
	uint32_t addr;
	TCacheEntry * ce;

	if (!_InitOperation(BlockID, addr, ce))
		return false;

	if (!ce->Forced)
		return true;
	
	ce->Forced = false;
	XADD_32(m_CacheInfo.Size, ce->Cache->Size);

	if (!m_ReadOnly)
	{
		addr = _FindFreePosition(ce->Cache->Size);
		m_BlockTable[BlockID >> 2].Addr = addr >> 2;
		_UpdateBlock(BlockID, ce, addr);
	}
	return true;
}

bool CBlockManager::MakeBlockVirtual(uint32_t BlockID)
{
	uint32_t addr;
	TCacheEntry * ce;
		
	if (!_InitOperation(BlockID, addr, ce))
		return false;

	if (ce->Forced)
		return true;

	if (ce->Pending) // don't write down, we kill it anyway
		_PendingRemove(ce->Pending, true);

	ce->Forced = true;
	XADD_32(m_CacheInfo.Size, 0 - ce->Cache->Size);

	if (!m_ReadOnly)
	{
		_InsertFreeBlock(addr, ce->Cache->Size, true, true);
		m_BlockTable[BlockID >> 2].Addr = 0;
	}
	return true;
}

void * CBlockManager::_ReadBlock(uint32_t BlockID, uint32_t & Size, uint32_t & Signature)
{
	uint32_t addr;
	TCacheEntry * ce;

	if ((BlockID == 0) && (Signature == -1))
	{
		Size = m_FirstBlockStart;
		return m_Cache[0]->Cache;
	}
	
	if (!_InitOperation(BlockID, addr, ce))
		return NULL;

	if ((Signature != 0) && (Signature != ce->Cache->Signature))
	{
		Signature = ce->Cache->Signature;
		return NULL;
	}
	Signature = ce->Cache->Signature;

	if ((Size != 0) && (Size != ce->Cache->Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc)))
	{
		Size = ce->Cache->Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc);
		return NULL;
	}
	Size = ce->Cache->Size - sizeof(TBlockHeadOcc) - sizeof(TBlockTailOcc);
	
	return ce->Cache + 1;
}

bool CBlockManager::UpdateBlock(uint32_t BlockID, uint32_t Signature)
{
	uint32_t addr;
	TCacheEntry * ce;

	if ((BlockID == 0) && (Signature == -1)) 
	{
		if (!m_ReadOnly)
			_PendingAdd(0, 0, m_FirstBlockStart, m_Cache[0]);
		
		return true;
	}
	
	if (!_InitOperation(BlockID, addr, ce))
		return false;
	
	if (Signature)
		ce->Cache->Signature = Signature;

	_UpdateBlock(BlockID, ce, addr);

	return true;
}

// make file writeable:
// write all cached blocks to file and check for the old addresses
// remove virtual only from file
// update m_FreeBlocks along the way

