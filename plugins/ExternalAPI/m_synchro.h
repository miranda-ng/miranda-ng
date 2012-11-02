#ifndef	__SYNCHRO_H
#define	__SYNCHRO_H

#include <windows.h>

//
//================================== OTHER DEFINITIONS ========================================
//

#define WAIT_FINISH	WAIT_OBJECT_0+1

// This structure is used to get semaphore-like synchronization:
// Includes incrementing, decrementing DWORD value and if DWORD is zero, sets event
typedef struct SynchronisedCounter
{
// Stores number value
	HANDLE Event;
	DWORD Number;

// These methods are deleted due to external plugins. Use SCGetNumber,SCInc and SCDec instead
//	DWORD GetNumber();		
//	DWORD Inc();
//	DWORD Dec();

// Yes, some code is defined here. But it is not so problematic, because it uses only Win32 API calls and Win32 structures,
	SynchronisedCounter(): Number(0)
	{
		InitializeCriticalSection(&CounterCS);
		Event=CreateEvent(NULL,FALSE,TRUE,NULL);
		SetEvent(Event);
	}

	SynchronisedCounter(HANDLE InitializedEvent): Number(0)
	{
		InitializeCriticalSection(&CounterCS);
		Event=InitializedEvent;
		SetEvent(Event);
	}

	~SynchronisedCounter()
	{
		DeleteCriticalSection(&CounterCS);
		CloseHandle(Event);
	}

//private:				//it is not private as other functions (not methods) use these members
	CRITICAL_SECTION CounterCS;
} SCOUNTER, *PSCOUNTER;

// The single-writer/multiple-reader guard 
// compound synchronization object (SO)
// Notices: Copyright (c) 1995-1997 Jeffrey Richter
// Changes: majvan, only one process implementation,
//          hFinishEV event added- signals when we do not want to use this SO anymore
typedef struct SingleWriterMultiReaderGuard
{
// This event guards access to the other objects
// managed by this data structure and also indicates 
// whether any writer threads are writing.
	HANDLE hEventNoWriter;

// This manual-reset event is signaled when
// no reader threads are reading.
	HANDLE hEventNoReaders;

// This value is used simply as a counter.
// (the count is the number of reader threads)
	HANDLE hSemNumReaders;

// The request is for not to enter critical section
// for writing or reading due to going to delete guard
	HANDLE hFinishEV;
} SWMRG, *PSWMRG;

//
//================================== FUNCTIONS DEFINITIONS ========================================
//

typedef DWORD (WINAPI *YAMN_WAITTOWRITEFCN)(PSWMRG,PSCOUNTER);
typedef void  (WINAPI *YAMN_WRITEDONEFCN)(PSWMRG,PSCOUNTER);
typedef DWORD (WINAPI *YAMN_WAITTOREADFCN)(PSWMRG);
typedef void  (WINAPI *YAMN_READDONEFCN)(PSWMRG);
typedef DWORD (WINAPI *YAMN_SCMANAGEFCN)(PSCOUNTER);

//
//================================== QUICK FUNCTION CALL DEFINITIONS ========================================
//

//These are defininitions for YAMN exported functions. Your plugin can use them.
//pYAMNFcn is global variable, it is pointer to your structure containing YAMN functions.
//It is something similar like pluginLink variable in Miranda plugin. If you use
//this name of variable, you have already defined these functions and you can use them.
//It's similar to Miranda's CreateService function.
//These functions are used to synchronize accounts. YAMN could create service for these
//functions and you could call them then e.g. CallService(MS_YAMNWAITTOWRITE,WPARAM,LPARAM),
//but I think this solution is better, because these functions are much used. It is more
//"normal" if you call function for example like:
//WaitToWrite(ActualAccount) than CallService(MS_YAMNWAITTOWRITE,ActualAccount,NULL))

//How to use YAMN functions:
//Create a structure containing pointer to functions you want to use in your plugin
//This structure can look something like this:
//
//	struct
//	{
//		YAMN_WAITTOWRITEFCN	WaitToWriteFcn;
//		YAMN_WRITEDONEFCN	WriteDoneFcn;
//	} *pYAMNFcn;
//
//then you have to fill this structure with pointers...
//you have to use YAMN service to get pointer, like this (I wrote here all functions you may need,
//you can copy to your sources only those you need):
//
//	pYAMNFcn->WaitToWriteFcn=(YAMN_WAITTOWRITEFCN)CallService(MS_YAMN_GETFCNPTR,(WPARAM)YAMN_WAITTOWRITEID,0);
//	pYAMNFcn->WriteDoneFcn=(YAMN_WRITEDONEFCN)CallService(MS_YAMN_GETFCNPTR,(WPARAM)YAMN_WRITEDONEID,0);
//	pYAMNFcn->WaitToReadFcn=(YAMN_WAITTOREADFCN)CallService(MS_YAMN_GETFCNPTR,(WPARAM)YAMN_WAITTOREADID,0);
//	pYAMNFcn->ReadDoneFcn=(YAMN_READDONEFCN)CallService(MS_YAMN_GETFCNPTR,(WPARAM)YAMN_READDONEID,0);
//	pYAMNFcn->SCGetNumberFcn=(YAMN_SCMANAGEFCN)CallService(MS_YAMN_GETFCNPTR,(WPARAM)YAMN_SCGETNUMBERID,0);
//	pYAMNFcn->SCIncFcn=(YAMN_SCMANAGEFCN)CallService(MS_YAMN_GETFCNPTR,(WPARAM)YAMN_SCINCID,0);
//	pYAMNFcn->SCDecFcn=(YAMN_SCMANAGEFCN)CallService(MS_YAMN_GETFCNPTR,(WPARAM)YAMN_SCDECID,0);
//
//and in your plugin just simply use e.g.:
//
//	MsgsWriteDone(ActualAccount);		//this command leaves write access to account mails
//

#define	YAMN_WAITTOWRITEID	"YAMN/WaitToWrite"
#define	YAMN_WRITEDONEID	"YAMN/WriteDone"
#define	YAMN_WAITTOREADID	"YAMN/WaitToRead"
#define	YAMN_READDONEID		"YAMN/ReadDone"
#define	YAMN_SCGETNUMBERID	"YAMN/SCGetNumber"
#define	YAMN_SCINCID		"YAMN/SCInc"
#define	YAMN_SCDECID		"YAMN/SCDec"

#define	WaitToWrite(x)			pYAMNFcn->WaitToWriteFcn(x->AccountAccessSO,0)
#define	WaitToWriteEx(x,y)		pYAMNFcn->WaitToWriteFcn(x->AccountAccessSO,y)
#define	WriteDone(x)			pYAMNFcn->WriteDoneFcn(x->AccountAccessSO,0)
#define	WaitToRead(x)			pYAMNFcn->WaitToReadFcn(x->AccountAccessSO)
#define	WaitToReadEx(x,y)		pYAMNFcn->WaitToReadFcn(x->AccountAccessSO,y)
#define	ReadDone(x)			pYAMNFcn->ReadDoneFcn(x->AccountAccessSO)

#define	MsgsWaitToWrite(x)		pYAMNFcn->WaitToWriteFcn(x->MessagesAccessSO,0)
#define	MsgsWaitToWriteEx(x,y)		pYAMNFcn->WaitToWriteFcn(x->MessagesAccessSO,y)
#define	MsgsWriteDone(x)		pYAMNFcn->WriteDoneFcn(x->MessagesAccessSO,0)
#define	MsgsWaitToRead(x)		pYAMNFcn->WaitToReadFcn(x->MessagesAccessSO)
#define	MsgsWaitToReadEx(x)		pYAMNFcn->WaitToReadFcn(x->MessagesAccessSO,y)
#define	MsgsReadDone(x)			pYAMNFcn->ReadDoneFcn(x->MessagesAccessSO)

#define	WaitToWriteSO(x)		pYAMNFcn->WaitToWriteFcn(x,0)
#define	WaitToWriteSOEx(x,y)		pYAMNFcn->WaitToWriteFcn(x,y)
#define	WriteDoneSO(x)			pYAMNFcn->WriteDoneFcn(x,0)
#define	WaitToReadSO(x)			pYAMNFcn->WaitToReadFcn(x)
#define	WaitToReadSOEx(x,y)		pYAMNFcn->WaitToReadFcn(x,y)
#define	ReadDoneSO(x)			pYAMNFcn->ReadDoneFcn(x)

#define	SCGetNumber(x)			pYAMNFcn->SCGetNumberFcn(x)
#define	SCInc(x)			pYAMNFcn->SCIncFcn(x)
#define	SCDec(x)			pYAMNFcn->SCDecFcn(x)

#endif
