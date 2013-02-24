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
#include "MREWSync.h"
#include <assert.h>
#include "intrinsics.h"

#if defined(MREW_DO_DEBUG_LOGGING) && (defined(DEBUG) || defined(_DEBUG))
	#include <stdio.h>
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
// +------------+---------------+---------------+------------+---------------+
// | ReaderBusy | ReaderWaiting | WriterWaiting | WriterBusy | UseOddReader? |
// +------------+---------------+---------------+------------+---------------+
//     63..44         43..24          23..4            3             0
///////////////////////////////////////////////////////////////////////////////////////////////////
#define isWriterBusy(Sentinel)    ((Sentinel) & 0x0000000000000008ui64)
#define isWriterWaiting(Sentinel) ((Sentinel) & 0x0000000000fffff0ui64)
#define isReaderWaiting(Sentinel) ((Sentinel) & 0x00000fffff000000ui64)
#define isReaderBusy(Sentinel)    ((Sentinel) & 0xfffff00000000000ui64)

#define countWriterWaiting(Sentinel) (((Sentinel) & 0x0000000000fffff0ui64) >>  4)
#define countReaderWaiting(Sentinel) (((Sentinel) & 0x00000fffff000000ui64) >> 24)
#define countReaderBusy(Sentinel)    (((Sentinel) & 0xfffff00000000000ui64) >> 44)

#define WriterBusy    (1ui64 <<  3)
#define WriterWaiting (1ui64 <<  4)
#define ReaderWaiting (1ui64 << 24)
#define ReaderBusy    (1ui64 << 44)

#define isUseOddReader(Sentinel) ((Sentinel) & 1)
#define useOddReader             (1ui64)

CMultiReadExclusiveWriteSynchronizer::CMultiReadExclusiveWriteSynchronizer(void)
: tls(),
	m_Sentinel(0)
{
	m_ReadSignal[0] = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_ReadSignal[1] = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_WriteSignal   = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_WriterID = 0;
	m_WriteRecursion = 0;
	m_Revision = 0;
}

CMultiReadExclusiveWriteSynchronizer::~CMultiReadExclusiveWriteSynchronizer(void)
{
	BeginWrite();
	CloseHandle(m_WriteSignal);
	CloseHandle(m_ReadSignal[0]);
	CloseHandle(m_ReadSignal[1]);
}

void CMultiReadExclusiveWriteSynchronizer::BeginRead()
{
	unsigned long id = GetCurrentThreadId();
	unsigned & reccount(tls.Open(this, 0));
	
	reccount++;
	if ((m_WriterID != id) && (reccount == 1))
	{
		int64_t old;
		int64_t newvalue;

		do {
			old = m_Sentinel;
			if (isWriterBusy(old))
				newvalue = old + ReaderBusy; // writer has lock -> we are going to enter after he leaves -> we are busy but have to wait
			else if (isWriterWaiting(old))
				newvalue = old + ReaderWaiting; // writer is waiting for lock -> don't set myself busy as he waits for all readers to leave lock
			else 
				newvalue = old + ReaderBusy; // no writer in sight, just take lock
		
		} while (CMPXCHG_64(m_Sentinel, newvalue, old) != old);


		if (isWriterBusy(old) || isWriterWaiting(old))
		{
			if (isUseOddReader(old))
				WaitForSingleObject(m_ReadSignal[1], INFINITE);
			else
				WaitForSingleObject(m_ReadSignal[0], INFINITE);
		}
	}
}
void CMultiReadExclusiveWriteSynchronizer::EndRead()
{
	unsigned long id = GetCurrentThreadId();
	unsigned & reccount(tls.Open(this, 1));
	reccount--;

	if ((reccount == 0) && (m_WriterID != id))
	{
		int64_t old;
		int64_t newvalue;

		do {
			old = m_Sentinel;
			if ((countReaderBusy(old) == 1) && isWriterWaiting(old))
			{ // give control to the writer... move waiting readers to busy (but blocked)
				newvalue = old - WriterWaiting + WriterBusy - ReaderBusy + countReaderWaiting(old) * (ReaderBusy - ReaderWaiting);
			} else {
				newvalue = old - ReaderBusy;
			}
		} while (CMPXCHG_64(m_Sentinel, newvalue, old) != old);

		if ((countReaderBusy(old) == 1) && isWriterWaiting(old))
		{
			SetEvent(m_WriteSignal);
		}
	}

	if (reccount == 0)
		tls.Remove(this);
}
bool CMultiReadExclusiveWriteSynchronizer::BeginWrite()
{
	unsigned long id = GetCurrentThreadId();
	unsigned * reccount = tls.Find(this);
	bool res = true;

	if (m_WriterID != id)
	{
		int64_t old;
		int64_t newvalue;
		unsigned int oldrevision = m_Revision;

		if (reccount) // upgrade our readlock
		{
			do {
				old = m_Sentinel;
				// isWriterBusy cannot happen because we have a readlock, so we ignore it
				if (countReaderBusy(old) > 1) // there is another reader.. we have to wait for him. set arriving readers to waiting state
				{
					newvalue = old + WriterWaiting - ReaderBusy;
				} else if (isWriterWaiting(old)) // there is another writer waiting, who arrived earlier. we will sign him in and wait. we are the last reader, so we have to update the sentinel
				{
					newvalue = old + WriterBusy - ReaderBusy + countReaderWaiting(old) * (ReaderBusy - ReaderWaiting);
				} else { // nobody is busy, we want the lock
					newvalue = old + WriterBusy - ReaderBusy;
				}
			} while (CMPXCHG_64(m_Sentinel, newvalue, old) != old);

			if (countReaderBusy(old) > 1)
			{
				WaitForSingleObject(m_WriteSignal, INFINITE); // someone woke me up... he had to take care of all state changes of the sentinel
			} else if (isWriterWaiting(old)) // we will wait for the other writer, as we are the last reader, we have to wake him up
			{
				SetEvent(m_WriteSignal);
				Sleep(0); // yield thread trying to keep FIFO order
				WaitForSingleObject(m_WriteSignal, INFINITE); // someone woke me up... he had to take care of all state changes of the sentinel
			}
		} else { // gain write lock
			do {
				old = m_Sentinel;
				if (isWriterBusy(old)) // there is a writer.. we have to wait for him
				{
					newvalue = old + WriterWaiting;
				} else if (isReaderBusy(old)) // there is a reader.. we have to wait for him. set arriving readers to waiting state
				{
					newvalue = old + WriterWaiting;
				} else if (isWriterWaiting(old)) // there is another writer waiting, who arrived earlier. we will wait
				{
					newvalue = old + WriterWaiting;
				} else { // nobody is busy, we want the lock
					newvalue = old + WriterBusy;
				}
			} while (CMPXCHG_64(m_Sentinel, newvalue, old) != old);

			if (isWriterBusy(old) || isReaderBusy(old) || isWriterWaiting(old))
			{
				WaitForSingleObject(m_WriteSignal, INFINITE); // someone woke me up... he had to take care of all state changes of the sentinel
			}
		}
		res = (oldrevision == (INC_32(m_Revision) - 1));

		m_WriterID = id;
	}
	m_WriteRecursion++;

	return res;
}

bool CMultiReadExclusiveWriteSynchronizer::EndWrite()
{
	unsigned long id = GetCurrentThreadId();
	unsigned * reccount = tls.Find(this);

	m_WriteRecursion--;

	if (m_WriteRecursion == 0)
	{
		int64_t old;
		int64_t newvalue;

		m_WriterID = 0;

		if (isUseOddReader(m_Sentinel)) // reset upcoming signal
			ResetEvent(m_ReadSignal[0]);
		else
			ResetEvent(m_ReadSignal[1]);

		if (reccount) // downgrade to reader lock
		{
			do {
				old = m_Sentinel;
				newvalue = (old ^ useOddReader) - WriterBusy + ReaderBusy; // single case... we are a waiting reader and we will keep control of the lock
			} while (CMPXCHG_64(m_Sentinel, newvalue, old) != old);

			if (isUseOddReader(old)) // allow additional readers to pass
				SetEvent(m_ReadSignal[1]);
			else
				SetEvent(m_ReadSignal[0]);

		} else {

			do {
				old = m_Sentinel;
				if (isReaderBusy(old)) // give control to waiting readers
				{
					newvalue = (old ^ useOddReader) - WriterBusy;
				} else if (isWriterWaiting(old)) // no reader arrived while i was working... give control to next writer 
				{
					newvalue = (old ^ useOddReader) - WriterWaiting;
				} else { // nobody else is there... just close lock
					newvalue = (old ^ useOddReader) - WriterBusy;
				}
			} while (CMPXCHG_64(m_Sentinel, newvalue, old) != old);

			if (isReaderBusy(old)) // release waiting readers
			{
				if (isUseOddReader(old))
					SetEvent(m_ReadSignal[1]);
				else
					SetEvent(m_ReadSignal[0]);
			} else if (isWriterWaiting(old))
			{
				SetEvent(m_WriteSignal);
			}
		}
		return true;
	}

	return false;
}

bool CMultiReadExclusiveWriteSynchronizer::TryBeginWrite()
{
	unsigned long id = GetCurrentThreadId();
	unsigned * reccount = tls.Find(this); 

	if (m_WriterID != id)
	{
		int64_t old;
		int64_t newvalue;
		unsigned int oldrevision = m_Revision;

		if (reccount) // upgrade our readlock
		{
			do {
				old = m_Sentinel;
				// isWriterBusy cannot happen because we have a readlock, so we ignore it
				if (countReaderBusy(old) > 1) // there is another reader.. we have to wait for him. set arriving readers to waiting state
				{
					return false;
				} else if (isWriterWaiting(old)) // there is another writer waiting, who arrived earlier. we will sign him in and wait. we are the last reader, so we have to update the sentinel
				{
					return false;
				} else { // nobody is busy, we want the lock
					newvalue = old + WriterBusy - ReaderBusy;
				}
			} while (CMPXCHG_64(m_Sentinel, newvalue, old) != old);

		} else { // gain write lock
			do {
				old = m_Sentinel;
				if (isWriterBusy(old)) // there is a writer.. we have to wait for him
				{
					return false;
				} else if (isReaderBusy(old)) // there is a reader.. we have to wait for him. set arriving readers to waiting state
				{
					return false;
				} else if (isWriterWaiting(old)) // there is another writer waiting, who arrived earlier. we will wait
				{
					return false;
				} else { // nobody is busy, we want the lock
					newvalue = old + WriterBusy;
				}
			} while (CMPXCHG_64(m_Sentinel, newvalue, old) != old);
		}
		INC_32(m_Revision);

		m_WriterID = id;
	}
	m_WriteRecursion++;

	return true;
}

unsigned int CMultiReadExclusiveWriteSynchronizer::Waiting()
{
	int64_t old = m_Sentinel;
	if (isWriterBusy(old))
	{ // cast is safe, we don't loose data because these fields are max 20 bits
		return static_cast<unsigned int>(countReaderBusy(old) + countReaderWaiting(old) + countWriterWaiting(old));
	} else {
		return static_cast<unsigned int>(countReaderWaiting(old) + countWriterWaiting(old));
	}
};
