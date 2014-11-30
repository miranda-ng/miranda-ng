/*
 * This code implements miscellaneous usefull functions
 *
 * (c) majvan 2002-2004
 */

#include "yamn.h"

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

//Plugin registration CS
//Used if we add (register) plugin to YAMN plugins and when we browse through registered plugins
CRITICAL_SECTION PluginRegCS;

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
INT_PTR GetFcnPtrSvc(WPARAM wParam, LPARAM lParam);

//Returns pointer to YAMN variables
INT_PTR GetVariablesSvc(WPARAM wParam, LPARAM);

// Function every seconds decrements account counter of seconds and checks if they are 0
// If yes, creates a POP3 thread to check account
void CALLBACK TimerProc(HWND, UINT, UINT, DWORD);

// Function called to check all accounts immidialtelly
// no params
INT_PTR ForceCheckSvc(WPARAM, LPARAM);

//thread is running all the time
//waits for WriteToFileEV and then writes all accounts to file
//DWORD WINAPI FileWritingThread(PVOID);

// Function is called when Miranda notifies plugin that it is about to exit
// Ensures succesfull end of POP3 checking, sets event that no next checking should be performed
// If there's no writer to account (POP3 thread), saves the results to the file
//not used now, perhaps in the future


//int ExitProc(WPARAM wParam, LPARAM lParam);

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

INT_PTR GetFcnPtrSvc(WPARAM wParam, LPARAM lParam)
{
	register int i;

	for (i=0;i<sizeof(ProtoPluginExportedFcn)/sizeof(ProtoPluginExportedFcn[0]);i++)
		if (0==mir_strcmp((char *)wParam, ProtoPluginExportedFcn[i].ID))
			return (INT_PTR)ProtoPluginExportedFcn[i].Ptr;
	for (i=0;i<sizeof(ProtoPluginExportedSvc)/sizeof(ProtoPluginExportedSvc[0]);i++)
		if (0==mir_strcmp((char *)wParam, ProtoPluginExportedSvc[i].ID))
			return (INT_PTR)ProtoPluginExportedSvc[i].Ptr;
	for (i=0;i<sizeof(SynchroExportedFcn)/sizeof(SynchroExportedFcn[0]);i++)
		if (0==mir_strcmp((char *)wParam, SynchroExportedFcn[i].ID))
			return (INT_PTR)SynchroExportedFcn[i].Ptr;
	for (i=0;i<sizeof(AccountExportedFcn)/sizeof(AccountExportedFcn[0]);i++)
		if (0==mir_strcmp((char *)wParam, AccountExportedFcn[i].ID))
			return (INT_PTR)AccountExportedFcn[i].Ptr;
	for (i=0;i<sizeof(AccountExportedSvc)/sizeof(AccountExportedSvc[0]);i++)
		if (0==mir_strcmp((char *)wParam, AccountExportedSvc[i].ID))
			return (INT_PTR)AccountExportedSvc[i].Ptr;
	for (i=0;i<sizeof(MailExportedFcn)/sizeof(MailExportedFcn[0]);i++)
		if (0==mir_strcmp((char *)wParam, MailExportedFcn[i].ID))
			return (INT_PTR)MailExportedFcn[i].Ptr;
	for (i=0;i<sizeof(MailExportedSvc)/sizeof(MailExportedSvc[0]);i++)
		if (0==mir_strcmp((char *)wParam, MailExportedSvc[i].ID))
			return (INT_PTR)MailExportedSvc[i].Ptr;
	for (i=0;i<sizeof(FilterPluginExportedFcn)/sizeof(FilterPluginExportedFcn[0]);i++)
		if (0==mir_strcmp((char *)wParam, FilterPluginExportedFcn[i].ID))
			return (INT_PTR)FilterPluginExportedFcn[i].Ptr;
	for (i=0;i<sizeof(FilterPluginExportedSvc)/sizeof(FilterPluginExportedSvc[0]);i++)
		if (0==mir_strcmp((char *)wParam, FilterPluginExportedSvc[i].ID))
			return (INT_PTR)FilterPluginExportedSvc[i].Ptr;
	return (INT_PTR)NULL;
}

INT_PTR GetVariablesSvc(WPARAM wParam, LPARAM)
{
	return wParam==YAMN_VARIABLESVERSION ? (INT_PTR)&YAMNVar : (INT_PTR)NULL;
}

void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	PYAMN_PROTOPLUGINQUEUE ActualPlugin;
	HACCOUNT ActualAccount;
	HANDLE ThreadRunningEV;
	DWORD Status, tid;

//	we use event to signal, that running thread has all needed stack parameters copied
	if (NULL==(ThreadRunningEV=CreateEvent(NULL, FALSE, FALSE, NULL)))
		return;
//	if we want to close miranda, we get event and do not run checking anymore
	if (WAIT_OBJECT_0==WaitForSingleObject(ExitEV, 0))
		return;
//	Get actual status of current user in Miranda
		Status=CallService(MS_CLIST_GETSTATUSMODE, 0, 0);

	EnterCriticalSection(&PluginRegCS);
	for (ActualPlugin=FirstProtoPlugin;ActualPlugin != NULL;ActualPlugin=ActualPlugin->Next)
	{
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile, "TimerProc:AccountBrowserSO-read wait\n");
#endif
		if (WAIT_OBJECT_0 != SWMRGWaitToRead(ActualPlugin->Plugin->AccountBrowserSO, 0))			//we want to access accounts immiadtelly
		{
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile, "TimerProc:AccountBrowserSO-read enter failed\n");
#endif
			LeaveCriticalSection(&PluginRegCS);
			return;
		}
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile, "TimerProc:AccountBrowserSO-read enter\n");
#endif
		for (ActualAccount=ActualPlugin->Plugin->FirstAccount;ActualAccount != NULL;ActualAccount=ActualAccount->Next)
		{
			if (ActualAccount->Plugin==NULL || ActualAccount->Plugin->Fcn==NULL)		//account not inited
				continue;
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile, "TimerProc:ActualAccountSO-read wait\n");
#endif
			if (WAIT_OBJECT_0 != SWMRGWaitToRead(ActualAccount->AccountAccessSO, 0))
			{
#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile, "TimerProc:ActualAccountSO-read wait failed\n");
#endif
				continue;
			}
#ifdef DEBUG_SYNCHRO

			switch(Status)
			{
			case ID_STATUS_OFFLINE:
				DebugLog(SynchroFile, "TimerProc:ActualAccountSO-read enter status offline\n");
				break;
			case ID_STATUS_ONLINE:
				DebugLog(SynchroFile, "TimerProc:ActualAccountSO-read enter status online\n");
				break;
			case ID_STATUS_AWAY:
				DebugLog(SynchroFile, "TimerProc:ActualAccountSO-read enter status away\n");
				break;
			case ID_STATUS_DND:
				DebugLog(SynchroFile, "TimerProc:ActualAccountSO-read enter status dnd\n");
				break;
			case ID_STATUS_NA:
				DebugLog(SynchroFile, "TimerProc:ActualAccountSO-read enter status na\n");
				break;
			case ID_STATUS_OCCUPIED:
				DebugLog(SynchroFile, "TimerProc:ActualAccountSO-read enter status occupied\n");
				break;
			case ID_STATUS_FREECHAT:
				DebugLog(SynchroFile, "TimerProc:ActualAccountSO-read enter status freechat\n");
				break;
			case ID_STATUS_INVISIBLE:
				DebugLog(SynchroFile, "TimerProc:ActualAccountSO-read enter status invisible\n");
				break;
			case ID_STATUS_ONTHEPHONE:
				DebugLog(SynchroFile, "TimerProc:ActualAccountSO-read enter status onthephone\n");
				break;
			case ID_STATUS_OUTTOLUNCH:
				DebugLog(SynchroFile, "TimerProc:ActualAccountSO-read enter status outtolunch\n");
				break;
			default:
				DebugLog(SynchroFile, "TimerProc:ActualAccountSO-read enter status unknown\n");
				break;
			}
#endif
			BOOL isAccountCounting = 0;
			if (
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

				if ((!ActualAccount->Interval && !ActualAccount->TimeLeft) || ActualAccount->Plugin->Fcn->TimeoutFcnPtr==NULL)
				{
					goto ChangeIsCountingStatusLabel;
				}
				if (ActualAccount->TimeLeft) {
					ActualAccount->TimeLeft--;
					isAccountCounting = TRUE;
				}
#ifdef DEBUG_SYNCHRO
					DebugLog(SynchroFile, "TimerProc:time left : %i\n", ActualAccount->TimeLeft);
#endif
				WindowList_BroadcastAsync(YAMNVar.MessageWnds, WM_YAMN_CHANGETIME, (WPARAM)ActualAccount, (LPARAM)ActualAccount->TimeLeft);
				if (!ActualAccount->TimeLeft)
				{
					struct CheckParam ParamToPlugin={YAMN_CHECKVERSION, ThreadRunningEV, ActualAccount, YAMN_NORMALCHECK, (void *)0, NULL};
					HANDLE NewThread;

					ActualAccount->TimeLeft=ActualAccount->Interval;
					if (NULL==(NewThread=CreateThread(NULL, 0, (YAMN_STANDARDFCN)ActualAccount->Plugin->Fcn->TimeoutFcnPtr, &ParamToPlugin, 0, &tid)))
					{
#ifdef DEBUG_SYNCHRO
						DebugLog(SynchroFile, "TimerProc:ActualAccountSO-read done\n");
#endif
						ReadDoneFcn(ActualAccount->AccountAccessSO);
						continue;
					}
					else
					{
						WaitForSingleObject(ThreadRunningEV, INFINITE);
						CloseHandle(NewThread);
					}
				}

			}
ChangeIsCountingStatusLabel:
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile, "TimerProc:ActualAccountSO-read done\n");
#endif
			if (((ActualAccount->isCounting) != 0) != isAccountCounting) {
				ActualAccount->isCounting=isAccountCounting;
				WORD cStatus = db_get_w(ActualAccount->hContact, YAMN_DBMODULE, "Status", 0);
				switch (cStatus) {
					case ID_STATUS_ONLINE:
					case ID_STATUS_OFFLINE:
						db_set_w(ActualAccount->hContact, YAMN_DBMODULE, "Status", isAccountCounting?ID_STATUS_ONLINE:ID_STATUS_OFFLINE);
					default: break;
				}
			}
			ReadDoneFcn(ActualAccount->AccountAccessSO);
		}
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile, "TimerProc:AccountBrowserSO-read done\n");
#endif
		SWMRGDoneReading(ActualPlugin->Plugin->AccountBrowserSO);
	}
	LeaveCriticalSection(&PluginRegCS);
	CloseHandle(ThreadRunningEV);
	return;
}

INT_PTR ForceCheckSvc(WPARAM, LPARAM)
{
	PYAMN_PROTOPLUGINQUEUE ActualPlugin;
	HACCOUNT ActualAccount;
	HANDLE ThreadRunningEV;
	DWORD tid;

	//we use event to signal, that running thread has all needed stack parameters copied
	if (NULL==(ThreadRunningEV=CreateEvent(NULL, FALSE, FALSE, NULL)))
		return 0;
	//if we want to close miranda, we get event and do not run pop3 checking anymore
	if (WAIT_OBJECT_0==WaitForSingleObject(ExitEV, 0))
		return 0;
	EnterCriticalSection(&PluginRegCS);
	for (ActualPlugin=FirstProtoPlugin;ActualPlugin != NULL;ActualPlugin=ActualPlugin->Next)
	{
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile, "ForceCheck:AccountBrowserSO-read wait\n");
		#endif
		SWMRGWaitToRead(ActualPlugin->Plugin->AccountBrowserSO, INFINITE);
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile, "ForceCheck:AccountBrowserSO-read enter\n");
		#endif
		for (ActualAccount=ActualPlugin->Plugin->FirstAccount;ActualAccount != NULL;ActualAccount=ActualAccount->Next)
		{
			if (ActualAccount->Plugin->Fcn==NULL)		//account not inited
				continue;
			#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile, "ForceCheck:ActualAccountSO-read wait\n");
			#endif
			if (WAIT_OBJECT_0 != WaitToReadFcn(ActualAccount->AccountAccessSO))
			{
				#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile, "ForceCheck:ActualAccountSO-read wait failed\n");
				#endif
				continue;
			}
			#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile, "ForceCheck:ActualAccountSO-read enter\n");
			#endif
			if ((ActualAccount->Flags & YAMN_ACC_ENA) && (ActualAccount->StatusFlags & YAMN_ACC_FORCE))			//account cannot be forced to check
			{
				if (ActualAccount->Plugin->Fcn->ForceCheckFcnPtr==NULL)
				{
					ReadDoneFcn(ActualAccount->AccountAccessSO);
					continue;
				}
				struct CheckParam ParamToPlugin={YAMN_CHECKVERSION, ThreadRunningEV, ActualAccount, YAMN_FORCECHECK, (void *)0, NULL};

				if (NULL==CreateThread(NULL, 0, (YAMN_STANDARDFCN)ActualAccount->Plugin->Fcn->ForceCheckFcnPtr, &ParamToPlugin, 0, &tid))
				{
					ReadDoneFcn(ActualAccount->AccountAccessSO);
					continue;
				}
				else
					WaitForSingleObject(ThreadRunningEV, INFINITE);
			}
			ReadDoneFcn(ActualAccount->AccountAccessSO);
		}
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile, "ForceCheck:AccountBrowserSO-read done\n");
#endif
		SWMRGDoneReading(ActualPlugin->Plugin->AccountBrowserSO);
	}
	LeaveCriticalSection(&PluginRegCS);
	CloseHandle(ThreadRunningEV);

	if (hTTButton) CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTTButton, 0);
	return 1;
}
