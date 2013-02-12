/*
 * This code implements synchronization objects code between threads. If you want, you can include it to your
 * code. This file is not dependent on any other external code (functions)
 *
 * (c) majvan 2002-2004
 */

#include "yamn.h"

// Initializes a SWMRG structure. This structure must be 
// initialized before any writer or reader threads attempt
// to wait on it.
// The structure must be allocated by the application and 
// the structure's address is passed as the first parameter.
// The lpszName parameter is the name of the object. Pass
// NULL if you do not want to share the object.
BOOL WINAPI SWMRGInitialize(PSWMRG pSWMRG,TCHAR *Name);

// Deletes the system resources associated with a SWMRG 
// structure. The structure must be deleted only when
// no writer or reader threads in the calling process
// will wait on it.
void WINAPI SWMRGDelete(PSWMRG pSWMRG);

// A writer thread calls this function to know when 
// it can successfully write to the shared data.
// returns WAIT_FINISH when we are in write-access or WAIT_FAILED
// when event about quick finishing is set (or when system returns fail when waiting for synchro object)
DWORD WINAPI SWMRGWaitToWrite(PSWMRG pSWMRG,DWORD dwTimeout);

// A writer thread calls this function to let other threads
// know that it no longer needs to write to the shared data.
void WINAPI SWMRGDoneWriting(PSWMRG pSWMRG);

// A reader thread calls this function to know when 
// it can successfully read the shared data.
// returns WAIT_FINISH when we are in read-access or WAIT_FAILED
// when event about quick finishing is set (or when system returns fail when waiting for synchro object)
DWORD WINAPI SWMRGWaitToRead(PSWMRG pSWMRG, DWORD dwTimeout);

// A reader thread calls this function to let other threads
// know when it no longer needs to read the shared data.
void WINAPI SWMRGDoneReading(PSWMRG pSWMRG);

// WaitToReadFcn
// is used to wait for read access with SWMRG SO, but it also increments counter if successfull
// returns WAIT_FAILED or WAIT_FINISH
// when WAIT_FAILED, we should not begin to access datas, we are not in read-access mode
DWORD WINAPI WaitToReadFcn(PSWMRG SObject);

// WriteDoneFcn
// is used to release read access with SWMRG SO, but it also decrements counter if successfull
void WINAPI ReadDoneFcn(PSWMRG SObject);

// This functions is for export purposes
// Plugin can call this function to manage SCOUNTER synchronization object

// Gets number value stored in SCOUNTER SO
// Note you must not read the number from memory directly, because
// CPU can stop reading thread when it has read HI-Word, then another thread
// can change the value and then OS starts the previous thread, that reads the
// LO-WORD of DWORD. And the return value HI+LO-WORD is corrupted
DWORD WINAPI SCGetNumberFcn(PSCOUNTER SCounter);

// Increments SCOUNTER and unsets event
// Returns Number after incrementing
DWORD WINAPI SCIncFcn(PSCOUNTER SCounter);

// Decrements SCOUNTER and sets event if zero
// Returns Number after decrementing
DWORD WINAPI SCDecFcn(PSCOUNTER SCounter);

struct CExportedFunctions SynchroExportedFcn[]=
{
	{YAMN_WAITTOWRITEID,(void *)WaitToWriteFcn},
	{YAMN_WRITEDONEID,(void *)WriteDoneFcn},
	{YAMN_WAITTOREADID,(void *)WaitToReadFcn},
	{YAMN_READDONEID,(void *)ReadDoneFcn},
	{YAMN_SCGETNUMBERID,(void *)SCGetNumberFcn},
	{YAMN_SCINCID,(void *)SCIncFcn},
	{YAMN_SCDECID,(void *)SCDecFcn},
};

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void WINAPI SWMRGDelete(PSWMRG pSWMRG)
{
// Destroys any synchronization objects that were 
// successfully created.
	if (NULL != pSWMRG->hEventNoWriter)
		CloseHandle(pSWMRG->hEventNoWriter);
	if (NULL != pSWMRG->hEventNoReaders)
		CloseHandle(pSWMRG->hEventNoReaders);
	if (NULL != pSWMRG->hSemNumReaders)
		CloseHandle(pSWMRG->hSemNumReaders);
	if (NULL != pSWMRG->hFinishEV)
		CloseHandle(pSWMRG->hFinishEV);
}

BOOL WINAPI SWMRGInitialize(PSWMRG pSWMRG,TCHAR *Name)
{
	pSWMRG->hEventNoWriter=NULL;
	pSWMRG->hEventNoReaders=NULL;
	pSWMRG->hSemNumReaders=NULL;
	pSWMRG->hFinishEV=NULL;

// Creates the automatic-reset event that is signalled when 
// no writer threads are writing.
// Initially no reader threads are reading.
	if (Name != NULL)
		Name[0]=(TCHAR)'W';
	pSWMRG->hEventNoWriter=CreateEvent(NULL,FALSE,TRUE,Name);

// Creates the manual-reset event that is signalled when 
// no reader threads are reading.
// Initially no reader threads are reading.
	if (Name != NULL)
		Name[0]=(TCHAR)'R';
	pSWMRG->hEventNoReaders=CreateEvent(NULL,TRUE,TRUE,Name);

// Initializes the variable that indicates the number of 
// reader threads that are reading.
// Initially no reader threads are reading.
	if (Name != NULL)
		Name[0]=(TCHAR)'C';
	pSWMRG->hSemNumReaders=CreateSemaphore(NULL,0,0x7FFFFFFF,Name);

	if (Name != NULL)
		Name[0]=(TCHAR)'F';
	pSWMRG->hFinishEV=CreateEvent(NULL,TRUE,FALSE,Name);

// If a synchronization object could not be created,
// destroys any created objects and return failure.
	if ((NULL==pSWMRG->hEventNoWriter) || (NULL==pSWMRG->hEventNoReaders) || (NULL==pSWMRG->hSemNumReaders) || (NULL==pSWMRG->hFinishEV))
	{
		SWMRGDelete(pSWMRG);
		return FALSE;
	}
	return TRUE;
}

DWORD WINAPI SWMRGWaitToWrite(PSWMRG pSWMRG,DWORD dwTimeout)
{
	DWORD dw; 
	HANDLE aHandles[2];

// We can write if the following are true:
// 1. No other threads are writing.
// 2. No threads are reading.
// But first we have to know if SWMRG structure is not about to delete
	aHandles[0]=pSWMRG->hEventNoWriter;
	aHandles[1]=pSWMRG->hEventNoReaders;
	if (WAIT_OBJECT_0==(dw=WaitForSingleObject(pSWMRG->hFinishEV,0)))
		return WAIT_FINISH;
	if (WAIT_FAILED==dw)
		return dw;
	dw=WaitForMultipleObjects(2,aHandles,TRUE,dwTimeout);
// if a request to delete became later, we should not catch it. Try once more to ask if account is not about to delete
	if ((dw != WAIT_FAILED) && (WAIT_OBJECT_0==(WaitForSingleObject(pSWMRG->hFinishEV,0))))
	{
		SetEvent(pSWMRG->hEventNoWriter);
		return WAIT_FINISH;
	}

// This thread can write to the shared data.
// Automatic event for NoWriter sets hEventNoWriter to nonsignaled after WaitForMultipleObject

// Because a writer thread is writing, the Event 
// should not be reset. This stops other 
// writers and readers.
	return dw;
}

void WINAPI SWMRGDoneWriting(PSWMRG pSWMRG)
// Presumably, a writer thread calling this function has
// successfully called WaitToWrite. This means that we
// do not have to wait on any synchronization objects 
// here because the writer already owns the Event.
{
// Allow other writer/reader threads to use
// the SWMRG synchronization object.
	SetEvent(pSWMRG->hEventNoWriter);
}

DWORD WINAPI SWMRGWaitToRead(PSWMRG pSWMRG, DWORD dwTimeout)
{
	DWORD dw; 
	LONG lPreviousCount;

// We can read if no threads are writing.
// And there's not request to delete structure
	if (WAIT_OBJECT_0==(dw=WaitForSingleObject(pSWMRG->hFinishEV,0)))
		return WAIT_FINISH;
	if (WAIT_FAILED==dw)
		return dw;
	dw=WaitForSingleObject(pSWMRG->hEventNoWriter, dwTimeout);
// if a request to delete became later, we should not catch it. Try once more to ask if account is not about to delete
	if ((dw != WAIT_FAILED) && (WAIT_OBJECT_0==(WaitForSingleObject(pSWMRG->hFinishEV,0))))
	{
		SetEvent(pSWMRG->hEventNoWriter);
		return WAIT_FINISH;
	}

	if (dw==WAIT_OBJECT_0)
	{
	// This thread can read from the shared data.
	// Increment the number of reader threads.
	// But there can't be more than one thread incrementing readers,
	// so this is why we use semaphore.
		ReleaseSemaphore(pSWMRG->hSemNumReaders,1,&lPreviousCount);
		if (lPreviousCount==0)
			// If this is the first reader thread, 
			// set event to reflect this. Other reader threads can read, no writer thread can write.
			ResetEvent(pSWMRG->hEventNoReaders);

		// Allow other writer/reader threads to use
		// the SWMRG synchronization object. hEventNoWrite is still non-signaled
		// (it looks like writer is processing thread, but it is not true)
		SetEvent(pSWMRG->hEventNoWriter);
	}

	return(dw);
}

void WINAPI SWMRGDoneReading(PSWMRG pSWMRG)
{
	HANDLE aHandles[2];
	LONG lNumReaders;

// We can stop reading if the events are available,
// but when we stop reading we must also decrement the
// number of reader threads.
	aHandles[0]=pSWMRG->hEventNoWriter;
	aHandles[1]=pSWMRG->hSemNumReaders;
	WaitForMultipleObjects(2,aHandles,TRUE,INFINITE);

// Get the remaining number of readers by releasing the
// semaphore and then restoring the count by immediately
// performing a wait.
	ReleaseSemaphore(pSWMRG->hSemNumReaders,1,&lNumReaders);
	WaitForSingleObject(pSWMRG->hSemNumReaders,INFINITE);

// If there are no remaining readers, 
// set the event to relect this.
	if (lNumReaders==0)
		// If there are no reader threads, 
		// set our event to reflect this.
		SetEvent(pSWMRG->hEventNoReaders);

// Allow other writer/reader threads to use
// the SWMRG synchronization object.
// (it looks like writer is processing thread, but it is not true)
	SetEvent(pSWMRG->hEventNoWriter);
}

DWORD WINAPI WaitToWriteFcn(PSWMRG SObject,PSCOUNTER SCounter)
{
	DWORD EnterCode;
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tSO WaitToWrite: %x\n",SObject);
#endif
	if (WAIT_OBJECT_0==(EnterCode=SWMRGWaitToWrite(SObject,INFINITE)))
		if (SCounter != NULL)
			SCIncFcn(SCounter);
	return EnterCode;
}

void WINAPI WriteDoneFcn(PSWMRG SObject,PSCOUNTER SCounter)
{
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tSO WriteDone: %x\n",SObject);
#endif
	SWMRGDoneWriting(SObject);
	if (SCounter != NULL)
		SCDecFcn(SCounter);
}

DWORD WINAPI WaitToReadFcn(PSWMRG SObject)
{
	DWORD EnterCode;
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tSO WaitToRead: %x\n",SObject);
#endif
	EnterCode=SWMRGWaitToRead(SObject,INFINITE);
	return EnterCode;
}

void WINAPI ReadDoneFcn(PSWMRG SObject)
{
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tSO ReadDone: %x\n",SObject);
#endif
	SWMRGDoneReading(SObject);
}

DWORD WINAPI SCGetNumberFcn(PSCOUNTER SCounter)
{
	DWORD Temp;
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tGetNumber-cs wait\n");
#endif
	EnterCriticalSection(&SCounter->CounterCS);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tGetNumber-cs enter\n");
#endif
	Temp=SCounter->Number;
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tValue: %d\n",Temp);
	DebugLog(SynchroFile,"\tGetNumber-cs done\n");
#endif
	LeaveCriticalSection(&SCounter->CounterCS);
	return Temp;
}

DWORD WINAPI SCIncFcn(PSCOUNTER SCounter)
{
	DWORD Temp;
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tIncrementValue-cs wait\n");
#endif
	EnterCriticalSection(&SCounter->CounterCS);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tIncrementValue-cs enter\n");
#endif
	Temp=++SCounter->Number;
	ResetEvent(SCounter->Event);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tValue: %d\n",Temp);
	DebugLog(SynchroFile,"\tIncrementValue-cs done\n");
#endif
	LeaveCriticalSection(&SCounter->CounterCS);
	return Temp;
}

DWORD WINAPI SCDecFcn(PSCOUNTER SCounter)
{
	DWORD Temp;
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tDecrementValue-cs wait\n");
#endif
	EnterCriticalSection(&SCounter->CounterCS);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tDecrementValue-cs enter\n");
#endif
	if (!(Temp=--SCounter->Number))
	{
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"\tDecrementValue-zero ev set\n");
#endif
		SetEvent(SCounter->Event);
	}
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"\tValue: %d\n",Temp);
	DebugLog(SynchroFile,"\tDecrementValue-cs done\n");
#endif
	LeaveCriticalSection(&SCounter->CounterCS);
	return Temp;
}
