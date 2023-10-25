/*
 * This code implements synchronization objects code between threads. If you want, you can include it to your
 * code. This file is not dependent on any other external code (functions)
 *
 * (c) majvan 2002-2004
 */

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Initializes a SWMRG structure. This structure must be 
// initialized before any writer or reader threads attempt
// to wait on it.
// The structure must be allocated by the application and 
// the structure's address is passed as the first parameter.
// The lpszName parameter is the name of the object. Pass
// NULL if you do not want to share the object.

SWMRG::SWMRG(wchar_t *Name)
{
	// Creates the automatic-reset event that is signalled when 
	// no writer threads are writing.
	// Initially no reader threads are reading.
	if (Name != nullptr)
		Name[0] = (wchar_t)'W';
	hEventNoWriter = ::CreateEvent(nullptr, FALSE, TRUE, Name);

	// Creates the manual-reset event that is signalled when 
	// no reader threads are reading.
	// Initially no reader threads are reading.
	if (Name != nullptr)
		Name[0] = (wchar_t)'R';
	hEventNoReaders = ::CreateEvent(nullptr, TRUE, TRUE, Name);

	// Initializes the variable that indicates the number of 
	// reader threads that are reading.
	// Initially no reader threads are reading.
	if (Name != nullptr)
		Name[0] = (wchar_t)'C';
	hSemNumReaders = ::CreateSemaphore(nullptr, 0, 0x7FFFFFFF, Name);

	if (Name != nullptr)
		Name[0] = (wchar_t)'F';
	hFinishEV = ::CreateEvent(nullptr, TRUE, FALSE, Name);
}

// Destroys any synchronization objects that were 
// successfully created.

SWMRG::~SWMRG()
{
	if (nullptr != hEventNoWriter)
		CloseHandle(hEventNoWriter);
	if (nullptr != hEventNoReaders)
		CloseHandle(hEventNoReaders);
	if (nullptr != hSemNumReaders)
		CloseHandle(hSemNumReaders);
	if (nullptr != hFinishEV)
		CloseHandle(hFinishEV);
}

/////////////////////////////////////////////////////////////////////////////////////////
// SWMRGWaitToWrite
// A writer thread calls this function to know when 
// it can successfully write to the shared data.
// returns WAIT_FINISH when we are in write-access or WAIT_FAILED
// when event about quick finishing is set (or when system returns fail when waiting for synchro object)

uint32_t SWMRG::WaitToWrite(uint32_t dwTimeout)
{
	uint32_t dw;
	HANDLE aHandles[2];
	// Netlib_Logf(hNetlibUser, "SWMRGWaitToWrite %p", this);

	// We can write if the following are true:
	// 1. No other threads are writing.
	// 2. No threads are reading.
	// But first we have to know if SWMRG structure is not about to delete
	aHandles[0] = hEventNoWriter;
	aHandles[1] = hEventNoReaders;
	if (WAIT_OBJECT_0 == (dw = WaitForSingleObject(hFinishEV, 0)))
		return WAIT_FINISH;
	if (WAIT_FAILED == dw)
		return dw;
	dw = WaitForMultipleObjects(2, aHandles, TRUE, dwTimeout);
	// if a request to delete became later, we should not catch it. Try once more to ask if account is not about to delete
	if ((dw != WAIT_FAILED) && (WAIT_OBJECT_0 == (WaitForSingleObject(hFinishEV, 0)))) {
		SetEvent(hEventNoWriter);
		return WAIT_FINISH;
	}

	// This thread can write to the shared data.
	// Automatic event for NoWriter sets hEventNoWriter to nonsignaled after WaitForMultipleObject

	// Because a writer thread is writing, the Event 
	// should not be reset. This stops other 
	// writers and readers.
	return dw;
}

/////////////////////////////////////////////////////////////////////////////////////////
// SWMRGDoneWriting
// A writer thread calls this function to let other threads
// know that it no longer needs to write to the shared data.

void SWMRG::DoneWriting()
{
	// Netlib_Logf(hNetlibUser, "SWMRGDoneWriting %p", this);

	// Allow other writer/reader threads to use
	// the SWMRG synchronization object.
	SetEvent(hEventNoWriter);
}

/////////////////////////////////////////////////////////////////////////////////////////
// SWMRGWaitToRead
// is used to wait for read access with SWMRG SO, but it also increments counter if successfull
// returns WAIT_FAILED or WAIT_FINISH
// when WAIT_FAILED, we should not begin to access datas, we are not in read-access mode

uint32_t SWMRG::WaitToRead(uint32_t dwTimeout)
{
	uint32_t dw;
	LONG lPreviousCount;
	// Netlib_Logf(hNetlibUser, "SWMRGWaitToRead %p", this);

	// We can read if no threads are writing.
	// And there's not request to delete structure
	if (WAIT_OBJECT_0 == (dw = WaitForSingleObject(hFinishEV, 0)))
		return WAIT_FINISH;
	if (WAIT_FAILED == dw)
		return dw;
	dw = WaitForSingleObject(hEventNoWriter, dwTimeout);
	// if a request to delete became later, we should not catch it. Try once more to ask if account is not about to delete
	if ((dw != WAIT_FAILED) && (WAIT_OBJECT_0 == (WaitForSingleObject(hFinishEV, 0)))) {
		SetEvent(hEventNoWriter);
		return WAIT_FINISH;
	}

	if (dw == WAIT_OBJECT_0) {
		// This thread can read from the shared data.
		// Increment the number of reader threads.
		// But there can't be more than one thread incrementing readers,
		// so this is why we use semaphore.
		ReleaseSemaphore(hSemNumReaders, 1, &lPreviousCount);
		if (lPreviousCount == 0)
			// If this is the first reader thread, 
			// set event to reflect this. Other reader threads can read, no writer thread can write.
			ResetEvent(hEventNoReaders);

		// Allow other writer/reader threads to use
		// the SWMRG synchronization object. hEventNoWrite is still non-signaled
		// (it looks like writer is processing thread, but it is not true)
		SetEvent(hEventNoWriter);
	}

	return dw;
}

/////////////////////////////////////////////////////////////////////////////////////////
// SWMRGDoneReading
// A reader thread calls this function to let other threads
// know when it no longer needs to read the shared data.

void SWMRG::DoneReading()
{
	HANDLE aHandles[2];
	LONG lNumReaders;
	// Netlib_Logf(hNetlibUser, "SWMRGDoneReading %p", this);

	// We can stop reading if the events are available,
	// but when we stop reading we must also decrement the
	// number of reader threads.
	aHandles[0] = hEventNoWriter;
	aHandles[1] = hSemNumReaders;
	WaitForMultipleObjects(2, aHandles, TRUE, INFINITE);

	// Get the remaining number of readers by releasing the
	// semaphore and then restoring the count by immediately
	// performing a wait.
	ReleaseSemaphore(hSemNumReaders, 1, &lNumReaders);
	WaitForSingleObject(hSemNumReaders, INFINITE);

	// If there are no remaining readers, 
	// set the event to relect this.
	if (lNumReaders == 0)
		// If there are no reader threads, 
		// set our event to reflect this.
		SetEvent(hEventNoReaders);

	// Allow other writer/reader threads to use
	// the SWMRG synchronization object.
	// (it looks like writer is processing thread, but it is not true)
	SetEvent(hEventNoWriter);
}

/////////////////////////////////////////////////////////////////////////////////////////
// SCOUNTER

SCOUNTER::SCOUNTER()
{
	InitializeCriticalSection(&CounterCS);
	Event = CreateEvent(nullptr, FALSE, TRUE, nullptr);
	SetEvent(Event);
}

SCOUNTER::SCOUNTER(HANDLE InitializedEvent)
{
	InitializeCriticalSection(&CounterCS);
	Event = InitializedEvent;
	SetEvent(Event);
}

SCOUNTER::~SCOUNTER()
{
	DeleteCriticalSection(&CounterCS);
	CloseHandle(Event);
}

// Gets number value stored in SCOUNTER SO
// Note you must not read the number from memory directly, because
// CPU can stop reading thread when it has read HI-Word, then another thread
// can change the value and then OS starts the previous thread, that reads the
// LO-uint16_t of uint32_t. And the return value HI+LO-uint16_t is corrupted

uint32_t SCOUNTER::GetNumber()
{
	EnterCriticalSection(&CounterCS);

	uint32_t Temp = Number;

	LeaveCriticalSection(&CounterCS);
	return Temp;
}

// Increments SCOUNTER and unsets event
// Returns Number after incrementing

uint32_t SCOUNTER::Inc()
{
	EnterCriticalSection(&CounterCS);

	uint32_t Temp = ++Number;
	ResetEvent(Event);

	LeaveCriticalSection(&CounterCS);
	return Temp;
}

// Decrements SCOUNTER and sets event if zero
// Returns Number after decrementing

uint32_t SCOUNTER::Dec()
{
	uint32_t Temp;
	EnterCriticalSection(&CounterCS);

	if (!(Temp = --Number)) {
		SetEvent(Event);
	}

	LeaveCriticalSection(&CounterCS);
	return Temp;
}
