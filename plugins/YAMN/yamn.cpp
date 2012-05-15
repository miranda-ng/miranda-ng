/*
 * This code implements miscellaneous usefull functions
 * 
 * (c) majvan 2002-2004
 */
#include "m_yamn.h"
#include "m_protoplugin.h"
#include "m_messages.h"
#include "m_synchro.h"
#include "main.h"
#include "yamn.h"
#ifdef DEBUG_SYNCHRO
	#include <stdio.h>
#endif

//- imported ---------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

extern PYAMN_PROTOPLUGINQUEUE FirstProtoPlugin;
extern YAMN_VARIABLES YAMNVar;

extern char *ProtoName;

extern HANDLE hTTButton;		//TopToolBar button

extern DWORD WriteAccountsToFile();
extern DWORD WINAPI SWMRGWaitToRead(PSWMRG,DWORD);
extern void WINAPI SWMRGDoneReading(PSWMRG);
extern DWORD WINAPI WaitToReadFcn(PSWMRG);
extern void WINAPI ReadDoneFcn(PSWMRG);

//From protoplugin.cpp
extern struct CExportedFunctions ProtoPluginExportedFcn[1];
extern struct CExportedServices ProtoPluginExportedSvc[5];
//From filterplugin.cpp
extern struct CExportedFunctions FilterPluginExportedFcn[1];
extern struct CExportedServices FilterPluginExportedSvc[2];
//From synchro.cpp
extern struct CExportedFunctions SynchroExportedFcn[7];
//From account.cpp
extern struct CExportedFunctions AccountExportedFcn[2];
extern struct CExportedServices AccountExportedSvc[9];
//From mails.cpp (MIME)
extern struct CExportedFunctions MailExportedFcn[8];
extern struct CExportedServices MailExportedSvc[5];

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

//MessageWndCS
//We want to send messages to all windows in the queue
//When we send messages, no other window can register itself to the queue for receiving messages
LPCRITICAL_SECTION MessageWndCS;

//Plugin registration CS
//Used if we add (register) plugin to YAMN plugins and when we browse through registered plugins
LPCRITICAL_SECTION PluginRegCS;

//AccountWriterCS
//We want to store number of writers of Accounts (number of Accounts used for writing)
//If we want to read all accounts (for saving to file) immidiatelly, we have to wait until no account is changing (no thread writing to account)
SCOUNTER *AccountWriterSO;

//NoExitEV
//Event that is signaled when there's a request to exit, so no new pop3 check should be performed
HANDLE ExitEV;

//WriteToFileEV
//If this is signaled, write accounts to file is performed. Set this event if you want to actualize your accounts and messages
HANDLE WriteToFileEV;

//Returns pointer to YAMN exported function
INT_PTR GetFcnPtrSvc(WPARAM wParam,LPARAM lParam);

//Returns pointer to YAMN variables
INT_PTR GetVariablesSvc(WPARAM wParam,LPARAM);

// Thread running only to catch hotkeys
DWORD WINAPI YAMNHotKeyThread(LPVOID Param);

// Function every seconds decrements account counter of seconds and checks if they are 0
// If yes, creates a POP3 thread to check account
void CALLBACK TimerProc(HWND,UINT,UINT,DWORD);

// Function called to check all accounts immidialtelly
// no params
INT_PTR ForceCheckSvc(WPARAM,LPARAM);

//thread is running all the time
//waits for WriteToFileEV and then writes all accounts to file
//DWORD WINAPI FileWritingThread(PVOID);

// Function is called when Miranda notifies plugin that it is about to exit
// Ensures succesfull end of POP3 checking, sets event that no next checking should be performed
// If there's no writer to account (POP3 thread), saves the results to the file
//not used now, perhaps in the future


//int ExitProc(WPARAM wParam,LPARAM lParam);

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

INT_PTR GetFcnPtrSvc(WPARAM wParam,LPARAM lParam)
{
	register int i;

	for(i=0;i<sizeof(ProtoPluginExportedFcn)/sizeof(ProtoPluginExportedFcn[0]);i++)
		if(0==lstrcmp((char *)wParam,ProtoPluginExportedFcn[i].ID))
			return (INT_PTR)ProtoPluginExportedFcn[i].Ptr;
	for(i=0;i<sizeof(ProtoPluginExportedSvc)/sizeof(ProtoPluginExportedSvc[0]);i++)
		if(0==lstrcmp((char *)wParam,ProtoPluginExportedSvc[i].ID))
			return (INT_PTR)ProtoPluginExportedSvc[i].Ptr;
	for(i=0;i<sizeof(SynchroExportedFcn)/sizeof(SynchroExportedFcn[0]);i++)
		if(0==lstrcmp((char *)wParam,SynchroExportedFcn[i].ID))
			return (INT_PTR)SynchroExportedFcn[i].Ptr;
	for(i=0;i<sizeof(AccountExportedFcn)/sizeof(AccountExportedFcn[0]);i++)
		if(0==lstrcmp((char *)wParam,AccountExportedFcn[i].ID))
			return (INT_PTR)AccountExportedFcn[i].Ptr;
	for(i=0;i<sizeof(AccountExportedSvc)/sizeof(AccountExportedSvc[0]);i++)
		if(0==lstrcmp((char *)wParam,AccountExportedSvc[i].ID))
			return (INT_PTR)AccountExportedSvc[i].Ptr;
	for(i=0;i<sizeof(MailExportedFcn)/sizeof(MailExportedFcn[0]);i++)
		if(0==lstrcmp((char *)wParam,MailExportedFcn[i].ID))
			return (INT_PTR)MailExportedFcn[i].Ptr;
	for(i=0;i<sizeof(MailExportedSvc)/sizeof(MailExportedSvc[0]);i++)
		if(0==lstrcmp((char *)wParam,MailExportedSvc[i].ID))
			return (INT_PTR)MailExportedSvc[i].Ptr;
	for(i=0;i<sizeof(FilterPluginExportedFcn)/sizeof(FilterPluginExportedFcn[0]);i++)
		if(0==lstrcmp((char *)wParam,FilterPluginExportedFcn[i].ID))
			return (INT_PTR)FilterPluginExportedFcn[i].Ptr;
	for(i=0;i<sizeof(FilterPluginExportedSvc)/sizeof(FilterPluginExportedSvc[0]);i++)
		if(0==lstrcmp((char *)wParam,FilterPluginExportedSvc[i].ID))
			return (INT_PTR)FilterPluginExportedSvc[i].Ptr;
	return (INT_PTR)NULL;
}

INT_PTR GetVariablesSvc(WPARAM wParam,LPARAM)
{
	return wParam==YAMN_VARIABLESVERSION ? (INT_PTR)&YAMNVar : (INT_PTR)NULL;
}

DWORD WINAPI YAMNHotKeyThread(LPVOID Param)
{
	MSG WinMessage;
	WORD HotKey = LOWORD(Param);
	int HotKeyID;

//	register hotkey for main YAMN thread first 
	if(!(HotKeyID=RegisterHotKey(NULL,(int)GlobalAddAtom(YAMN_HKCHECKMAIL),HIBYTE(HotKey),LOBYTE(HotKey))))
		return 0;

	while(1)
	{
		GetMessage(&WinMessage,NULL,WM_HOTKEY,WM_YAMN_CHANGEHOTKEY);

//	if we want to close miranda, we get event and do not run pop3 checking anymore
		if(WAIT_OBJECT_0==WaitForSingleObject(ExitEV,0))
			break;

		switch(WinMessage.message)
		{
//	user pressed hotkey
			case WM_HOTKEY:
				ForceCheckSvc((WPARAM)0,(LPARAM)0);
				break;
//	hotkey changed
			case WM_YAMN_CHANGEHOTKEY:
				UnregisterHotKey(NULL,HotKeyID);
				HotKeyID=RegisterHotKey(NULL,(int)GlobalAddAtom(YAMN_HKCHECKMAIL),WinMessage.wParam,WinMessage.lParam);
				break;
		}
	}
	return 1;
}

void CALLBACK TimerProc(HWND,UINT,UINT,DWORD)
{
	PYAMN_PROTOPLUGINQUEUE ActualPlugin;
	HACCOUNT ActualAccount;
	HANDLE ThreadRunningEV;
	DWORD Status,tid;

//	we use event to signal, that running thread has all needed stack parameters copied
	if(NULL==(ThreadRunningEV=CreateEvent(NULL,FALSE,FALSE,NULL)))
		return;
//	if we want to close miranda, we get event and do not run checking anymore
	if(WAIT_OBJECT_0==WaitForSingleObject(ExitEV,0))
		return;
//	Get actual status of current user in Miranda
		Status=CallService(MS_CLIST_GETSTATUSMODE,0,0);

	EnterCriticalSection(PluginRegCS);
	for(ActualPlugin=FirstProtoPlugin;ActualPlugin!=NULL;ActualPlugin=ActualPlugin->Next)
	{
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"TimerProc:AccountBrowserSO-read wait\n");
#endif
		if(WAIT_OBJECT_0!=SWMRGWaitToRead(ActualPlugin->Plugin->AccountBrowserSO,0))			//we want to access accounts immiadtelly
		{
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"TimerProc:AccountBrowserSO-read enter failed\n");
#endif
			LeaveCriticalSection(PluginRegCS);
			return;
		}
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"TimerProc:AccountBrowserSO-read enter\n");
#endif
		for(ActualAccount=ActualPlugin->Plugin->FirstAccount;ActualAccount!=NULL;ActualAccount=ActualAccount->Next)
		{
			if(ActualAccount->Plugin==NULL || ActualAccount->Plugin->Fcn==NULL)		//account not inited
				continue;
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"TimerProc:ActualAccountSO-read wait\n");
#endif
			if(WAIT_OBJECT_0!=SWMRGWaitToRead(ActualAccount->AccountAccessSO,0))
			{
#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"TimerProc:ActualAccountSO-read wait failed\n");
#endif
				continue;
			}
#ifdef DEBUG_SYNCHRO
			
			switch(Status)
			{
			case ID_STATUS_OFFLINE:
				DebugLog(SynchroFile,"TimerProc:ActualAccountSO-read enter status offline\n");
				break;
			case ID_STATUS_ONLINE:
				DebugLog(SynchroFile,"TimerProc:ActualAccountSO-read enter status online\n");
				break;
			case ID_STATUS_AWAY:
				DebugLog(SynchroFile,"TimerProc:ActualAccountSO-read enter status away\n");
				break;
			case ID_STATUS_DND:
				DebugLog(SynchroFile,"TimerProc:ActualAccountSO-read enter status dnd\n");
				break;
			case ID_STATUS_NA:
				DebugLog(SynchroFile,"TimerProc:ActualAccountSO-read enter status na\n");
				break;
			case ID_STATUS_OCCUPIED:
				DebugLog(SynchroFile,"TimerProc:ActualAccountSO-read enter status occupied\n");
				break;
			case ID_STATUS_FREECHAT:
				DebugLog(SynchroFile,"TimerProc:ActualAccountSO-read enter status freechat\n");
				break;
			case ID_STATUS_INVISIBLE:
				DebugLog(SynchroFile,"TimerProc:ActualAccountSO-read enter status invisible\n");
				break;
			case ID_STATUS_ONTHEPHONE:
				DebugLog(SynchroFile,"TimerProc:ActualAccountSO-read enter status onthephone\n");
				break;
			case ID_STATUS_OUTTOLUNCH:
				DebugLog(SynchroFile,"TimerProc:ActualAccountSO-read enter status outtolunch\n");
				break;
			default:
				DebugLog(SynchroFile,"TimerProc:ActualAccountSO-read enter status unknown\n");
				break;
			}
#endif
			BOOL isAccountCounting = 0;
			if(
				(ActualAccount->Flags & YAMN_ACC_ENA) &&
				(((ActualAccount->StatusFlags & YAMN_ACC_ST0) && (Status<=ID_STATUS_OFFLINE)) ||
				((ActualAccount->StatusFlags & YAMN_ACC_ST1) && (Status==ID_STATUS_ONLINE)) ||
				((ActualAccount->StatusFlags & YAMN_ACC_ST2) && (Status==ID_STATUS_AWAY)) ||
				((ActualAccount->StatusFlags & YAMN_ACC_ST3) && (Status==ID_STATUS_DND)) ||
				((ActualAccount->StatusFlags & YAMN_ACC_ST4) && (Status==ID_STATUS_NA)) ||
				((ActualAccount->StatusFlags & YAMN_ACC_ST5) && (Status==ID_STATUS_OCCUPIED)) ||
				((ActualAccount->StatusFlags & YAMN_ACC_ST6) && (Status==ID_STATUS_FREECHAT)) ||
				((ActualAccount->StatusFlags & YAMN_ACC_ST7) && (Status==ID_STATUS_INVISIBLE)) ||
				((ActualAccount->StatusFlags & YAMN_ACC_ST8) && (Status==ID_STATUS_ONTHEPHONE)) ||
				((ActualAccount->StatusFlags & YAMN_ACC_ST9) && (Status==ID_STATUS_OUTTOLUNCH))))
			{

				if((!ActualAccount->Interval && !ActualAccount->TimeLeft) || ActualAccount->Plugin->Fcn->TimeoutFcnPtr==NULL)
				{
					goto ChangeIsCountingStatusLabel;
				}
				if(ActualAccount->TimeLeft){
					ActualAccount->TimeLeft--;
					isAccountCounting = TRUE;
				}
#ifdef DEBUG_SYNCHRO
					DebugLog(SynchroFile,"TimerProc:time left : %i\n",ActualAccount->TimeLeft);
#endif
				WindowList_BroadcastAsync(YAMNVar.MessageWnds,WM_YAMN_CHANGETIME,(WPARAM)ActualAccount,(LPARAM)ActualAccount->TimeLeft);
				if(!ActualAccount->TimeLeft)
				{
					struct CheckParam ParamToPlugin={YAMN_CHECKVERSION,ThreadRunningEV,ActualAccount,YAMN_NORMALCHECK,(void *)0,NULL};
					HANDLE NewThread;
		
					ActualAccount->TimeLeft=ActualAccount->Interval;
					if(NULL==(NewThread=CreateThread(NULL,0,(YAMN_STANDARDFCN)ActualAccount->Plugin->Fcn->TimeoutFcnPtr,&ParamToPlugin,0,&tid)))
					{
#ifdef DEBUG_SYNCHRO
						DebugLog(SynchroFile,"TimerProc:ActualAccountSO-read done\n");
#endif
						ReadDoneFcn(ActualAccount->AccountAccessSO);
						continue;
					}
					else
					{
						WaitForSingleObject(ThreadRunningEV,INFINITE);
						CloseHandle(NewThread);
					}
				}

			}
ChangeIsCountingStatusLabel:
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"TimerProc:ActualAccountSO-read done\n");
#endif
			if (((ActualAccount->isCounting)!=0)!=isAccountCounting){
				ActualAccount->isCounting=isAccountCounting;
				WORD cStatus = DBGetContactSettingWord(ActualAccount->hContact,ProtoName,"Status",0);
				switch (cStatus){
					case ID_STATUS_ONLINE:
					case ID_STATUS_OFFLINE:
						DBWriteContactSettingWord(ActualAccount->hContact, ProtoName, "Status", isAccountCounting?ID_STATUS_ONLINE:ID_STATUS_OFFLINE);
					default: break;
				}
			}
			ReadDoneFcn(ActualAccount->AccountAccessSO);
		}
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"TimerProc:AccountBrowserSO-read done\n");
#endif
		SWMRGDoneReading(ActualPlugin->Plugin->AccountBrowserSO);
	}
	LeaveCriticalSection(PluginRegCS);
	CloseHandle(ThreadRunningEV);
	return;
}

INT_PTR ForceCheckSvc(WPARAM,LPARAM)
{
	PYAMN_PROTOPLUGINQUEUE ActualPlugin;
	HACCOUNT ActualAccount;
	HANDLE ThreadRunningEV;
	DWORD tid;

	//we use event to signal, that running thread has all needed stack parameters copied
	if(NULL==(ThreadRunningEV=CreateEvent(NULL,FALSE,FALSE,NULL)))
		return 0;
	//if we want to close miranda, we get event and do not run pop3 checking anymore
	if(WAIT_OBJECT_0==WaitForSingleObject(ExitEV,0))
		return 0;
	EnterCriticalSection(PluginRegCS);
	for(ActualPlugin=FirstProtoPlugin;ActualPlugin!=NULL;ActualPlugin=ActualPlugin->Next)
	{
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"ForceCheck:AccountBrowserSO-read wait\n");
		#endif                                                                        
		SWMRGWaitToRead(ActualPlugin->Plugin->AccountBrowserSO,INFINITE);
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"ForceCheck:AccountBrowserSO-read enter\n");
		#endif
		for(ActualAccount=ActualPlugin->Plugin->FirstAccount;ActualAccount!=NULL;ActualAccount=ActualAccount->Next)
		{
			if(ActualAccount->Plugin->Fcn==NULL)		//account not inited
				continue;
			#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"ForceCheck:ActualAccountSO-read wait\n");
			#endif
			if(WAIT_OBJECT_0!=WaitToReadFcn(ActualAccount->AccountAccessSO))
			{
				#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"ForceCheck:ActualAccountSO-read wait failed\n");
				#endif
				continue;
			}
			#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"ForceCheck:ActualAccountSO-read enter\n");
			#endif
			if((ActualAccount->Flags & YAMN_ACC_ENA) && (ActualAccount->StatusFlags & YAMN_ACC_FORCE))			//account cannot be forced to check
			{
				if(ActualAccount->Plugin->Fcn->ForceCheckFcnPtr==NULL)
				{
					ReadDoneFcn(ActualAccount->AccountAccessSO);
					continue;
				}
				struct CheckParam ParamToPlugin={YAMN_CHECKVERSION,ThreadRunningEV,ActualAccount,YAMN_FORCECHECK,(void *)0,NULL};

				if(NULL==CreateThread(NULL,0,(YAMN_STANDARDFCN)ActualAccount->Plugin->Fcn->ForceCheckFcnPtr,&ParamToPlugin,0,&tid))
				{
					ReadDoneFcn(ActualAccount->AccountAccessSO);
					continue;
				}
				else
					WaitForSingleObject(ThreadRunningEV,INFINITE);
			}
			ReadDoneFcn(ActualAccount->AccountAccessSO);
		}
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"ForceCheck:AccountBrowserSO-read done\n");
#endif
		SWMRGDoneReading(ActualPlugin->Plugin->AccountBrowserSO);
	}
	LeaveCriticalSection(PluginRegCS);
	CloseHandle(ThreadRunningEV);
	CallService(MS_TTB_SETBUTTONSTATE,(WPARAM)hTTButton,(LPARAM)TTBST_RELEASED);
	return 1;
}
/*
int ExitProc(WPARAM wParam,LPARAM lParam)
{
	THIS WILL BE IMPLEMENTED LATER
//	First, no thread must add or delete accounts. This is achieved by entering browsing through accounts
//	If any thread want to delete or add, it waits for write-access to browse accounts (so it waits infinite time)
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"ExitProc:AccountBrowserSO-wait to obtain read access\n"));
#endif
	if(WAIT_TIMEOUT==SWMRGWaitToRead(AccountBrowserSO,0))
	{
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"ExitProc:AccountBrowserSO-read access obtain failed, I'll try later\n"));
#endif
		return 1;
	}
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"ExitProc:AccountBrowserSO-read access obtained\n"));
#endif
#ifdef DEBUG_SYNCHRO
	TCHAR Debug[100];

	_stprintf(Debug,_T("ExitProc:Writers: %d\n"),AccountWriterSO->GetNumber());
	DEBUG_SYNCHRO2F(Debug);
	DebugLog(SynchroFile,"ExitProc:NoWriterEV-test\n"));
#endif
//	next, threads must not write to any account. This works like hFinishEV event in AccountAccessSO and MessagesAccessSO.
//	When hFinishEV is set, any beginning with reading and writing to account (messages) is failed.
//	This is similar, but the difference is, that we can finish the whole work (we can decide: if ExitEV is set, should we
//	end immidialtelly or should we continue (to end operation successfully)?
//	E.g. I decided that once we started checking account, we get all new mails and then we can end.
//	The second and more significant difference is, that ExitEV is signal to all accounts and messages, not only to one account.

	SetEvent(ExitEV);
	if(WAIT_TIMEOUT==WaitForSingleObject(AccountWriterSO->Event,0))
	{
//	There exists a thread writing to account, so we ca try later to write accounts to file, if no thread is writting
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"ExitProc:NoWriterEV-writer(s) exists, I'll try later\n"));
#endif
		SWMRGDoneReading(AccountBrowserSO);
		return 1;
	}

#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"ExitProc:NoWriterEV-no writer, going to save!\n"));
#endif
//	Save to file
	WriteAccountsToFile();
	SWMRGDoneReading(AccountBrowserSO);
//	Now, all is saved, we can safe exit from Miranda
	return 0;	
}
*/
/*
DWORD WINAPI FileWritingThread(PVOID)
{
	HACCOUNT ActualAccount=FirstAccount;

	while(1)
	{
		WaitForSingleObject(WriteToFileEV,INFINITE);
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"FileWriting:WriteToFileEV-signaled\n"));
#endif
//	now, write accounts and messages if it is possible. If it is not possible e.g. to read messages from one account,
//	function will wait until messages are not used and then writes messages
		WriteAccountsToFile();
	}
	return 0;
}
*/
