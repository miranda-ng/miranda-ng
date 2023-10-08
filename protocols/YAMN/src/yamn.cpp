/*
 * This code implements miscellaneous usefull functions
 *
 * (c) majvan 2002-2004
 */

#include "stdafx.h"

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

//Plugin registration CS
//Used if we add (register) plugin to YAMN plugins and when we browse through registered plugins
mir_cs PluginRegCS;

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

// Function is called when Miranda notifies plugin that it is about to exit
// Ensures succesfull end of POP3 checking, sets event that no next checking should be performed
// If there's no writer to account (POP3 thread), saves the results to the file
// not used now, perhaps in the future

void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	//	we use event to signal, that running thread has all needed stack parameters copied
	HANDLE ThreadRunningEV = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (ThreadRunningEV == nullptr)
		return;

	//	if we want to close miranda, we get event and do not run checking anymore
	if (WAIT_OBJECT_0 == WaitForSingleObject(ExitEV, 0))
		return;

	//	Get actual status of current user in Miranda
	DWORD Status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);

	mir_cslock lck(PluginRegCS);
	for (YAMN_PROTOPLUGINQUEUE *ActualPlugin = FirstProtoPlugin; ActualPlugin != nullptr; ActualPlugin = ActualPlugin->Next) {
		SReadGuard srb(ActualPlugin->Plugin->AccountBrowserSO, 0); // we want to access accounts immiadtelly
		if (!srb.Succeeded()) 
			return;

		for (auto *ActualAccount = ActualPlugin->Plugin->FirstAccount; ActualAccount != nullptr; ActualAccount = ActualAccount->Next) {
			if (ActualAccount->Plugin == nullptr || ActualAccount->Plugin->Fcn == nullptr)		//account not inited
				continue;

			SReadGuard sra(ActualAccount->AccountAccessSO, 0);
			if (!sra.Succeeded())
				continue;

			BOOL isAccountCounting = 0;
			if ((ActualAccount->Flags & YAMN_ACC_ENA) &&
				(((ActualAccount->StatusFlags & YAMN_ACC_ST0) && (Status <= ID_STATUS_OFFLINE)) ||
					((ActualAccount->StatusFlags & YAMN_ACC_ST1) && (Status == ID_STATUS_ONLINE)) ||
					((ActualAccount->StatusFlags & YAMN_ACC_ST2) && (Status == ID_STATUS_AWAY)) ||
					((ActualAccount->StatusFlags & YAMN_ACC_ST3) && (Status == ID_STATUS_DND)) ||
					((ActualAccount->StatusFlags & YAMN_ACC_ST4) && (Status == ID_STATUS_NA)) ||
					((ActualAccount->StatusFlags & YAMN_ACC_ST5) && (Status == ID_STATUS_OCCUPIED)) ||
					((ActualAccount->StatusFlags & YAMN_ACC_ST6) && (Status == ID_STATUS_FREECHAT)) ||
					((ActualAccount->StatusFlags & YAMN_ACC_ST7) && (Status == ID_STATUS_INVISIBLE))))
			{
				if ((!ActualAccount->Interval && !ActualAccount->TimeLeft) || ActualAccount->Plugin->Fcn->TimeoutFcnPtr == nullptr)
					goto ChangeIsCountingStatusLabel;

				if (ActualAccount->TimeLeft) {
					ActualAccount->TimeLeft--;
					isAccountCounting = TRUE;
				}

				WindowList_BroadcastAsync(YAMNVar.MessageWnds, WM_YAMN_CHANGETIME, (WPARAM)ActualAccount, (LPARAM)ActualAccount->TimeLeft);
				if (!ActualAccount->TimeLeft) {
					struct CheckParam ParamToPlugin = {YAMN_CHECKVERSION, ThreadRunningEV, ActualAccount, YAMN_NORMALCHECK, (void *)nullptr, nullptr};

					ActualAccount->TimeLeft = ActualAccount->Interval;

					DWORD tid;
					HANDLE NewThread = CreateThread(nullptr, 0, (YAMN_STANDARDFCN)ActualAccount->Plugin->Fcn->TimeoutFcnPtr, &ParamToPlugin, 0, &tid);
					if (NewThread == nullptr)
						continue;

					WaitForSingleObject(ThreadRunningEV, INFINITE);
					CloseHandle(NewThread);
				}
			}

ChangeIsCountingStatusLabel:
			if (((ActualAccount->isCounting) != 0) != isAccountCounting) {
				ActualAccount->isCounting = isAccountCounting;

				uint16_t cStatus = g_plugin.getWord(ActualAccount->hContact, "Status");
				switch (cStatus) {
				case ID_STATUS_ONLINE:
				case ID_STATUS_OFFLINE:
					g_plugin.setWord(ActualAccount->hContact, "Status", isAccountCounting ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE);
				}
			}
		}
	}
	CloseHandle(ThreadRunningEV);
}

INT_PTR ForceCheckSvc(WPARAM, LPARAM)
{
	// we use event to signal, that running thread has all needed stack parameters copied
	HANDLE ThreadRunningEV = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (ThreadRunningEV == nullptr)
		return 0;

	// if we want to close miranda, we get event and do not run pop3 checking anymore
	if (WAIT_OBJECT_0 == WaitForSingleObject(ExitEV, 0))
		return 0;

	{	mir_cslock lck(PluginRegCS);

		for (YAMN_PROTOPLUGINQUEUE *ActualPlugin = FirstProtoPlugin; ActualPlugin != nullptr; ActualPlugin = ActualPlugin->Next) {
			SReadGuard srb(ActualPlugin->Plugin->AccountBrowserSO);
			for (auto *ActualAccount = ActualPlugin->Plugin->FirstAccount; ActualAccount != nullptr; ActualAccount = ActualAccount->Next) {
				if (ActualAccount->Plugin->Fcn == nullptr)		//account not inited
					continue;

				SReadGuard sra(ActualAccount->AccountAccessSO);
				if (!sra.Succeeded())
					continue;

				if ((ActualAccount->Flags & YAMN_ACC_ENA) && (ActualAccount->StatusFlags & YAMN_ACC_FORCE)) { //account cannot be forced to check
					if (ActualAccount->Plugin->Fcn->ForceCheckFcnPtr == nullptr)
						continue;

					DWORD tid;
					CheckParam ParamToPlugin = { YAMN_CHECKVERSION, ThreadRunningEV, ActualAccount, YAMN_FORCECHECK, (void *)nullptr, nullptr };
					if (nullptr == CreateThread(nullptr, 0, (YAMN_STANDARDFCN)ActualAccount->Plugin->Fcn->ForceCheckFcnPtr, &ParamToPlugin, 0, &tid))
						continue;

					WaitForSingleObject(ThreadRunningEV, INFINITE);
				}
			}
		}
	}

	CloseHandle(ThreadRunningEV);
	return 1;
}
