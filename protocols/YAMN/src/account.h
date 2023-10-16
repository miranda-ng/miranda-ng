/*
 * This file defines all needed parameters for one account.
 * Other plugin can use this (so YAMN does not check it and another plugin can inform YAMN about new mail e.g.),
 * this can be usefull for plugins like MSN (Hotmail notify)
 *
 * (c) majvan 2002-2004
 */

#ifndef __ACCOUNT_H
#define __ACCOUNT_H

#include <windows.h>
#include <tchar.h>

 //
 //================================== OTHER DEFINITIONS ========================================
 //

enum
{
	// Error codes returned from functions (services) working with account book files
	EACC_SYSTEM = 1,		//use GetLastError() to retrieve detailed information about error
	EACC_ALLOC,		//problem with memory allocation
	EACC_FILECOMPATIBILITY,	//file is corrupted
	EACC_ENDOFFILE,		//unexpected end of file occured
	EACC_FILEVERSION,	//file should be YAMN book format, but newer version that expected
	EACC_FILESIZE,		//file has wrong size
};

enum
{
	// Status of account
	// used in messages WM_YAMN_CHANGESTATUS
	// used also in function GetStatus and SetStatus
	ACC_IDLE = 0,		//account is IDLE (no work is performed with account)
	ACC_FINDING,		//DNS lookup for account
	ACC_CONNECTING,		//connecting in progress
	ACC_LOGGING,		//logging in progress
	ACC_WORKING,		//working
	ACC_DISCONNECTING,	//disconnecting from server
};

#define YAMN_ACC_MSG  0x00000002	// Shows dialog
#define YAMN_ACC_ICO  0x00000004	// Shows system tray icon (1)
#define YAMN_ACC_ICOB 0x00000008	// not used now, enables tray icon flashing (1)
#define YAMN_ACC_APP  0x00000010	// Runs application (1)
#define YAMN_ACC_POP  0x00000020	// Shows popup
#define YAMN_ACC_POPC 0x00000040	// Use custom colors in popup
#define YAMN_ACC_MSGP 0x00000080	// Persistant messgage. This means, when an situation occurs (e.g. new mail) and message is displayed, it is not destroyed when YAMN_ACC_MSG is not set
#define YAMN_ACC_KBN  0x00000100  // Use Keyboard notify
#define YAMN_ACC_CONT 0x00000200  // Use Contact notify
#define YAMN_ACC_CONTNICK 0x00000400  // Use Contact Nick replacement
#define YAMN_ACC_CONTNOEVENT 0x00000800  // Suppress event for this contact

struct YAMN_NOTIFICATION
{
	//(1) - usable only in newmail notification
	DWORD Flags = 0;

	COLORREF PopupB = 0;
	COLORREF PopupT = 0;
	DWORD PopupTime = 0;
	WCHAR *App = nullptr;
	WCHAR *AppParam = nullptr;

	// These parameters are not stored in standard YAMN book file and therefore must be set by plugin
	char *Sound = nullptr;
	HICON TrayIcon1 = nullptr;
	HICON TrayIcon2 = nullptr;
};

struct CServer
{
	char *Name = nullptr;
	DWORD Port = 0;

	char *Login = nullptr;

	// Password encryption definitions
	#define STARTCODEPSW	0x50
	#define	ADDCODEPSW	0x0
	char *Passwd = nullptr;
};

//
//================================== ACCOUNT DEFINITION ==================================
//

#define WAIT_FINISH	WAIT_OBJECT_0+1

// This structure is used to get semaphore-like synchronization:
// Includes incrementing, decrementing DWORD value and if DWORD is zero, sets event
class SCOUNTER
{
	HANDLE Event;
	uint32_t Number = 0;
	CRITICAL_SECTION CounterCS;

public:
	SCOUNTER();
	SCOUNTER(HANDLE InitializedEvent);
	~SCOUNTER();

	__forceinline HANDLE GetEvent() const { return Event; }

	uint32_t GetNumber();
	uint32_t Inc();
	uint32_t Dec();
};

struct SCGuard
{
	SCOUNTER &pSC;

	__forceinline SCGuard(SCOUNTER &sc) :
		pSC(sc)
	{
		sc.Inc();
	}

	__forceinline ~SCGuard()
	{
		pSC.Dec();
	}
};

// The single-writer/multiple-reader guard 
// compound synchronization object (SO)
// Notices: Copyright (c) 1995-1997 Jeffrey Richter
// Changes: majvan, only one process implementation,
//          hFinishEV event added- signals when we do not want to use this SO anymore

class SWMRG
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

public:
	SWMRG(wchar_t *Name = nullptr);
	~SWMRG();

	uint32_t WaitToWrite(uint32_t dwTimeout = INFINITE);
	void DoneWriting();

	uint32_t WaitToRead(uint32_t dwTimeout = INFINITE);
	void DoneReading();

	void Stop()
	{
		::SetEvent(hFinishEV);
	}
};

struct SReadGuard
{
	SWMRG &pSO;
	uint32_t dwError;

	SReadGuard(SWMRG &so, uint32_t timeout = INFINITE) :
		pSO(so)
	{
		dwError = so.WaitToRead(timeout);
	}

	~SReadGuard()
	{
		Uninit();
	}

	bool Succeeded() const
	{
		return dwError == WAIT_OBJECT_0;
	}

	void Uninit()
	{
		if (dwError == WAIT_OBJECT_0) {
			pSO.DoneReading();
			dwError = WAIT_FINISH;
		}
	}

	operator uint32_t() const { return dwError; }
};

struct SWriteGuard
{
	SWMRG &pSO;
	uint32_t dwError;

	SWriteGuard(SWMRG &so, uint32_t timeout = INFINITE) :
		pSO(so)
	{
		dwError = so.WaitToWrite(timeout);
	}

	~SWriteGuard()
	{
		Uninit();
	}

	bool Succeeded() const
	{
		return dwError == WAIT_OBJECT_0;
	}

	void Uninit()
	{
		if (dwError == WAIT_OBJECT_0) {
			pSO.DoneWriting();
			dwError = WAIT_FINISH;
		}
	}

	operator uint32_t() const { return dwError; }
};

/////////////////////////////////////////////////////////////////////////////////////////
// CAccount - basic email account class

struct CAccount : public MZeroedObject
{
	#define YAMN_ACCOUNTFILEVERSION	2	//version of standard file format (YAMN book file format)
	
	// If changes are made in this structure, version is changed.
	// So then YAMN does not initialzie your structure, if version does not matches.

	BOOL AbleToWork;			// This is set to TRUE by default. When it is needed to stop working on this account, YAMN sets this to zero.

	struct YAMN_PROTOPLUGIN *Plugin;	// free access, because this member should not be changed. The same as YAMN_PLUGIN structure

	char *Name;				// access only through AccountAccessSO

	CServer *Server;				//access only through AccountAccessSO

	WORD Interval;				//access only through AccountAccessSO

	//	YAMN account flags (set by user)
	#define YAMN_ACC_ENA	0x00000001		//Enables account. If account is disabled, no countdown is performed
	#define YAMN_ACC_POPN	0x00000002		//Shows one popup per one new mail or for N mails
	#define YAMN_ACC_APOP	0x00000004		//Use APOP authentication
	#define YAMN_ACC_SSL23	0x00000008		//Use SSLv2,3
	#define YAMN_ACC_NOTLS	0x00000010		//Don't try StartTLS (STLS) even available
	#define YAMN_ACC_BODY	0x00000020		//Always retrieve body of the message
	DWORD Flags;				//access only through AccountAccessSO

	//	YAMN account flags (set by plugin)
	#define YAMN_ACC_BROWSE	0x00000001		//Can browse mails. On this account we can run mailbrowser window
	#define YAMN_ACC_POPUP	0x00000002		//Popups of new mail belonging to this account can be showed 
	DWORD AbilityFlags;

	//	YAMN account status flags
	#define YAMN_ACC_ST0    0x00000001		// Check (countdown) when Offline
	#define YAMN_ACC_ST1    0x00000002		// Check (countdown) when Online
	#define YAMN_ACC_ST2    0x00000004		// Check (countdown) when Away
	#define YAMN_ACC_ST3    0x00000008		// Check (countdown) when Not available
	#define YAMN_ACC_ST4    0x00000010		// Check (countdown) when Occupied
	#define YAMN_ACC_ST5    0x00000020		// Check (countdown) when DND
	#define YAMN_ACC_ST6    0x00000040		// Check (countdown) when Free for chat
	#define YAMN_ACC_ST7    0x00000080		// Check (countdown) when Invisible

	#define YAMN_ACC_STARTA 0x00010000		// Check on start anyway
	#define YAMN_ACC_STARTS 0x00020000		// Check on start regarding to status setting 
	#define YAMN_ACC_FORCE  0x00040000		// Check when "check new mail" item pressed (it is called forced checking)
	DWORD StatusFlags; // access only through AccountAccessSO

	//	Plugin flags. Use this DWORD if you want YAMN to store it to YAMN book file. You can set here any value
	DWORD PluginFlags;

	YAMN_NOTIFICATION NewMailN;			//access only through AccountAccessSO
	YAMN_NOTIFICATION NoNewMailN;		//access only through AccountAccessSO
	YAMN_NOTIFICATION BadConnectN;		//access only through AccountAccessSO

	SYSTEMTIME LastChecked;			//last check, access only through AccountAccessSO
	SYSTEMTIME LastSChecked;		//last check (successfull), access only through AccountAccessSO
	SYSTEMTIME LastSynchronised;	//last synchronisation (successfull), access only through AccountAccessSO
	SYSTEMTIME LastMail;			//last check when new mail detected, access only through AccountAccessSO

	TCHAR Status[255];			//access only through GetStatusFcn() and SetStatusFcn() functions

	DWORD TimeLeft;				//access only through AccountAccessSO

	HANDLE Mails;				//access only through MessagesAccessSO

	// Account members are mostly the same, but there can be protocol (POP3,IMAP...) special features.
	// To use them, only inherit this class and add your own features.
	// First idea was to add pointer to void, where plugin can store its own values.
	// But this solution is better in my opinion.

	// This is event with counter. Event is signaled when no threads are using account (and will not be using)
	// Very usefull for account delete operation
	SCOUNTER UsingThreads;

	// We have to achieve, that only one thread can write to account and more threads can read.
	// Writing to account means that we change account parameters
	// Reading from account meands we read account parameters
	// Use WaitToRead(), ReadDone(), WaitToWrite(), WriteDone() synchronization functions
	// For plugins, this is a pointer to void. It does not matter for plugin what is this variable for,
	// because plugin works only with synchronization routines. And why is this void * ? It is because
	// plugin does not need to include headers for SWMRG structures...
	SWMRG AccountAccessSO;

	// We have to achieve, that only one thread can write to account mails and more threads can read.
	// While some thread writes mails, other thread can write to account. This can be small problem, but it never appears in YAMN.
	// But you should think about this note if you want to add some features in the future
	// Writing to messages means any changes to message queue or message data
	// Reading from messages means reading message queue (browsing through all messages) or reading message data
	// Use MsgsWaitToRead(),MsgsReadDone(),MsgsWaitToWrite(),MsgsWriteDone() synchronization functions
	SWMRG MessagesAccessSO;

	//For clist contact notification
	MCONTACT hContact;
	BOOL isCounting;

	CAccount *Next;

	void CheckMail();
	void RefreshContact();
};

#endif
