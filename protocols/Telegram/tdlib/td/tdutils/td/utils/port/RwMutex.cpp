//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2022
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <assert.h>

#include "td/utils/port/RwMutex.h"

typedef void(WINAPI *srwlock_function)(winlock_t*);
static srwlock_function srwlock_Init, srwlock_AcquireShared, srwlock_ReleaseShared, srwlock_AcquireExclusive, srwlock_ReleaseExclusive;

static bool bInited = false;

namespace td {
	//----------------------------------------------------------------------------
	// Stub for slim read-write lock
	// Copyright (C) 1995-2002 Brad Wilson

	static void WINAPI stub_srwlock_Init(winlock_t *srwl) {
		srwl->readerCount = srwl->writerCount = 0;
	}

	static void WINAPI stub_srwlock_AcquireShared(winlock_t *srwl) {
		while (true) {
			assert(srwl->writerCount >= 0 && srwl->readerCount >= 0);

			//  If there's a writer already, spin without unnecessarily
			//  interlocking the CPUs
			if (srwl->writerCount != 0) {
				YieldProcessor();
				continue;
			}

			//  Add to the readers list
			_InterlockedIncrement(&srwl->readerCount);

			// Check for writers again (we may have been preempted). If
			// there are no writers writing or waiting, then we're done.
			if (srwl->writerCount == 0)
				break;

			// Remove from the readers list, spin, try again
			_InterlockedDecrement(&srwl->readerCount);
			YieldProcessor();
		}
	}

	static void WINAPI stub_srwlock_ReleaseShared(winlock_t *srwl) {
		assert(srwl->readerCount > 0);
		_InterlockedDecrement(&srwl->readerCount);
	}

	static void WINAPI stub_srwlock_AcquireExclusive(winlock_t *srwl) {
		while (true) {
			assert(srwl->writerCount >= 0 && srwl->readerCount >= 0);

			//  If there's a writer already, spin without unnecessarily
			//  interlocking the CPUs
			if (srwl->writerCount != 0) {
				YieldProcessor();
				continue;
			}

			// See if we can become the writer (expensive, because it inter-
			// locks the CPUs, so writing should be an infrequent process)
			if (_InterlockedExchange(&srwl->writerCount, 1) == 0)
				break;
		}

		// Now we're the writer, but there may be outstanding readers.
		// Spin until there aren't any more; new readers will wait now
		// that we're the writer.
		while (srwl->readerCount != 0) {
			assert(srwl->writerCount >= 0 && srwl->readerCount >= 0);
			YieldProcessor();
		}
	}

	static void WINAPI stub_srwlock_ReleaseExclusive(winlock_t *srwl) {
		assert(srwl->writerCount == 1 && srwl->readerCount >= 0);
		srwl->writerCount = 0;
	}

	void InitializeLock(winlock_t &lock)
	{
		if (!bInited) {
			HINSTANCE hKernel32dll = GetModuleHandleA("kernel32.dll");

			const srwlock_function init = (srwlock_function)GetProcAddress(hKernel32dll, "InitializeSRWLock");
			if (init != NULL) {
				srwlock_Init = init;
				srwlock_AcquireShared = (srwlock_function)GetProcAddress(hKernel32dll, "AcquireSRWLockShared");
				srwlock_ReleaseShared = (srwlock_function)GetProcAddress(hKernel32dll, "ReleaseSRWLockShared");
				srwlock_AcquireExclusive = (srwlock_function)GetProcAddress(hKernel32dll, "AcquireSRWLockExclusive");
				srwlock_ReleaseExclusive = (srwlock_function)GetProcAddress(hKernel32dll, "ReleaseSRWLockExclusive");
			} else {
				srwlock_Init = stub_srwlock_Init;
				srwlock_AcquireShared = stub_srwlock_AcquireShared;
				srwlock_ReleaseShared = stub_srwlock_ReleaseShared;
				srwlock_AcquireExclusive = stub_srwlock_AcquireExclusive;
				srwlock_ReleaseExclusive = stub_srwlock_ReleaseExclusive;
			}

			bInited = true;
		}

		srwlock_Init(&lock);
	}

	void AcquireLockShared(winlock_t &lock)
	{
		srwlock_AcquireShared(&lock);
	}
	
	void AcquireLockExclusive(winlock_t &lock)
	{
		srwlock_AcquireExclusive(&lock);
	}

	void ReleaseLockShared(winlock_t &lock)
	{
		srwlock_ReleaseShared(&lock);
	}
	
	void ReleaseLockExclusive(winlock_t &lock)
	{
		srwlock_ReleaseExclusive(&lock);
	}
}  // namespace td
