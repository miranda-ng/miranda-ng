#include "stdafx.h"

static INT_PTR Service_GetCaps(WPARAM wParam, LPARAM)
{
	if (wParam == PFLAGNUM_4)
		return PF4_NOCUSTOMAUTH;
	if (wParam == PFLAG_UNIQUEIDTEXT)
        return (INT_PTR) Translate("Nick");
	if (wParam == PFLAG_MAXLENOFMESSAGE)
        return 400;
	if (wParam == PFLAG_UNIQUEIDSETTING)
        return (INT_PTR) "Id";
	if (wParam == PFLAGNUM_2)
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND;
	if (wParam == PFLAGNUM_5) {
		if (db_get_b(NULL, YAMN_DBMODULE, YAMN_SHOWASPROTO, 1))
			return PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND;
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND;
	}
	return 0;
}

static INT_PTR Service_GetStatus(WPARAM, LPARAM)
{
	return YAMN_STATUS;	
}

static INT_PTR Service_SetStatus(WPARAM wParam, LPARAM)
{	
	int newstatus = (wParam != ID_STATUS_OFFLINE)?ID_STATUS_ONLINE:ID_STATUS_OFFLINE;
	if (newstatus != YAMN_STATUS) {
		int oldstatus = YAMN_STATUS;
		YAMN_STATUS = newstatus;
		ProtoBroadcastAck(YAMN_DBMODULE, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldstatus, newstatus);
	}
	return 0;

}

static INT_PTR Service_GetName(WPARAM wParam, LPARAM lParam)
{
	mir_strncpy((char *) lParam, YAMN_DBMODULE, wParam);
	return 0;
}

static INT_PTR Service_LoadIcon(WPARAM wParam, LPARAM)
{
	if ( LOWORD( wParam ) == PLI_PROTOCOL )
		return (INT_PTR)CopyIcon(g_LoadIconEx(0)); // noone cares about other than PLI_PROTOCOL

	return (INT_PTR)(HICON)NULL;
}
 
INT_PTR ClistContactDoubleclicked(WPARAM, LPARAM lParam)
{
	ContactDoubleclicked(((CLISTEVENT*)lParam)->lParam, lParam);
	return 0;
}

static int Service_ContactDoubleclicked(WPARAM wParam, LPARAM lParam)
{
	ContactDoubleclicked(wParam, lParam);
	return 0;
}

static INT_PTR ContactApplication(WPARAM wParam, LPARAM)
{
	char *szProto = GetContactProto(wParam);
	if ( mir_strcmp(szProto, YAMN_DBMODULE))
		return 0;

	DBVARIANT dbv;
	if ( db_get(wParam, YAMN_DBMODULE, "Id", &dbv))
		return 0;

	HACCOUNT ActualAccount = (HACCOUNT) CallService(MS_YAMN_FINDACCOUNTBYNAME, (WPARAM)POP3Plugin, (LPARAM)dbv.pszVal);
	if (ActualAccount != NULL) {
		STARTUPINFOW si = { 0 };
		si.cb = sizeof(si);
				
		#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile, "ContactApplication:ActualAccountSO-read wait\n");
		#endif
		if (WAIT_OBJECT_0 == WaitToReadFcn(ActualAccount->AccountAccessSO)) {
			#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile, "ContactApplication:ualAccountSO-read enter\n");
			#endif
			if (ActualAccount->NewMailN.App != NULL) {
				WCHAR *Command;
				if (ActualAccount->NewMailN.AppParam != NULL)
					Command = new WCHAR[mir_wstrlen(ActualAccount->NewMailN.App)+mir_wstrlen(ActualAccount->NewMailN.AppParam)+6];
				else
					Command = new WCHAR[mir_wstrlen(ActualAccount->NewMailN.App)+6];
					
				if (Command != NULL) {
					mir_wstrcpy(Command, L"\"");
					mir_wstrcat(Command, ActualAccount->NewMailN.App);
					mir_wstrcat(Command, L"\" ");
					if (ActualAccount->NewMailN.AppParam != NULL)
						mir_wstrcat(Command, ActualAccount->NewMailN.AppParam);

					PROCESS_INFORMATION pi;
					CreateProcessW(NULL, Command, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
					delete[] Command;
				}
			}

			#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile, "ContactApplication:ActualAccountSO-read done\n");
			#endif
			ReadDoneFcn(ActualAccount->AccountAccessSO);
		}
		#ifdef DEBUG_SYNCHRO
		else
			DebugLog(SynchroFile, "ContactApplication:ActualAccountSO-read enter failed\n");
		#endif
	}
	db_free(&dbv);
	return 0;
}

DWORD WINAPI SWMRGWaitToRead(PSWMRG pSWMRG, DWORD dwTimeout);
static INT_PTR AccountMailCheck(WPARAM wParam, LPARAM lParam)
{
	//This service will check/sincronize the account pointed by wParam
	HACCOUNT ActualAccount = (HACCOUNT)wParam;
	// copy/paste make mistakes
	if (ActualAccount != NULL) {
		//we use event to signal, that running thread has all needed stack parameters copied
		HANDLE ThreadRunningEV = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (ThreadRunningEV == NULL)
			return 0;
		//if we want to close miranda, we get event and do not run pop3 checking anymore
		if (WAIT_OBJECT_0 == WaitForSingleObject(ExitEV, 0))
			return 0;

		mir_cslock lck(PluginRegCS);
		#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile, "AccountCheck:ActualAccountSO-read wait\n");
		#endif
		if (WAIT_OBJECT_0 != SWMRGWaitToRead(ActualAccount->AccountAccessSO, 0)) {
			#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile, "ForceCheck:ActualAccountSO-read wait failed\n");
			#endif
		}
		else {
			#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile, "ForceCheck:ActualAccountSO-read enter\n");
			#endif
			if ((ActualAccount->Flags & YAMN_ACC_ENA) && ActualAccount->Plugin->Fcn->SynchroFcnPtr) {
				struct CheckParam ParamToPlugin = {YAMN_CHECKVERSION, ThreadRunningEV, ActualAccount, lParam?YAMN_FORCECHECK:YAMN_NORMALCHECK, 0, NULL};

				ActualAccount->TimeLeft = ActualAccount->Interval;
				DWORD tid;
				HANDLE NewThread = CreateThread(NULL, 0, (YAMN_STANDARDFCN)ActualAccount->Plugin->Fcn->SynchroFcnPtr, &ParamToPlugin, 0, &tid);
				if (NewThread) {
					WaitForSingleObject(ThreadRunningEV, INFINITE);
					CloseHandle(NewThread);
				}
			}
			ReadDoneFcn(ActualAccount->AccountAccessSO);
		}
		CloseHandle(ThreadRunningEV);
	}
	return 0;
}

static INT_PTR ContactMailCheck(WPARAM hContact, LPARAM)
{
	char *szProto = GetContactProto(hContact);
	if ( mir_strcmp(szProto, YAMN_DBMODULE))
		return 0;

	DBVARIANT dbv;
	if ( db_get(hContact, YAMN_DBMODULE, "Id", &dbv))
		return 0;

	HACCOUNT ActualAccount = (HACCOUNT) CallService(MS_YAMN_FINDACCOUNTBYNAME, (WPARAM)POP3Plugin, (LPARAM)dbv.pszVal);
	if (ActualAccount != NULL) {
		//we use event to signal, that running thread has all needed stack parameters copied
		HANDLE ThreadRunningEV;
		if (NULL == (ThreadRunningEV = CreateEvent(NULL, FALSE, FALSE, NULL)))
			return 0;
		//if we want to close miranda, we get event and do not run pop3 checking anymore
		if (WAIT_OBJECT_0 == WaitForSingleObject(ExitEV, 0))
			return 0;
		mir_cslock lck(PluginRegCS);
		#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile, "ForceCheck:ActualAccountSO-read wait\n");
		#endif
		if (WAIT_OBJECT_0 != WaitToReadFcn(ActualAccount->AccountAccessSO))
		{
			#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile, "ForceCheck:ActualAccountSO-read wait failed\n");
			#endif
		}
		else
		{
			#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile, "ForceCheck:ActualAccountSO-read enter\n");
			#endif
			if ((ActualAccount->Flags & YAMN_ACC_ENA) && (ActualAccount->StatusFlags & YAMN_ACC_FORCE))			//account cannot be forced to check
			{
				if (ActualAccount->Plugin->Fcn->ForceCheckFcnPtr == NULL)
					ReadDoneFcn(ActualAccount->AccountAccessSO);

				DWORD tid;
				struct CheckParam ParamToPlugin = {YAMN_CHECKVERSION, ThreadRunningEV, ActualAccount, YAMN_FORCECHECK, (void *)0, NULL};
				if (NULL == CreateThread(NULL, 0, (YAMN_STANDARDFCN)ActualAccount->Plugin->Fcn->ForceCheckFcnPtr, &ParamToPlugin, 0, &tid))
					ReadDoneFcn(ActualAccount->AccountAccessSO);
				else
					WaitForSingleObject(ThreadRunningEV, INFINITE);
			}
			ReadDoneFcn(ActualAccount->AccountAccessSO);
		}
		CloseHandle(ThreadRunningEV);
	}
	db_free(&dbv);
	return 0;
}

/*static*/ void ContactDoubleclicked(WPARAM wParam, LPARAM)
{
	char *szProto = GetContactProto(wParam);
	if ( mir_strcmp(szProto, YAMN_DBMODULE))
		return;

	DBVARIANT dbv;
	if ( db_get(wParam, YAMN_DBMODULE, "Id", &dbv))
		return;

	HACCOUNT ActualAccount = (HACCOUNT)CallService(MS_YAMN_FINDACCOUNTBYNAME, (WPARAM)POP3Plugin, (LPARAM)dbv.pszVal);
	if (ActualAccount != NULL) {
		#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile, "Service_ContactDoubleclicked:ActualAccountSO-read wait\n");
		#endif
		if (WAIT_OBJECT_0 == WaitToReadFcn(ActualAccount->AccountAccessSO)) {
			#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile, "Service_ContactDoubleclicked:ActualAccountSO-read enter\n");
			#endif
			YAMN_MAILBROWSERPARAM Param = { 0, ActualAccount, ActualAccount->NewMailN.Flags, ActualAccount->NoNewMailN.Flags, 0 };

			Param.nnflags = Param.nnflags | YAMN_ACC_MSG;			//show mails in account even no new mail in account
			Param.nnflags = Param.nnflags & ~YAMN_ACC_POP;

			Param.nflags = Param.nflags | YAMN_ACC_MSG;			//show mails in account even no new mail in account
			Param.nflags = Param.nflags & ~YAMN_ACC_POP;

			RunMailBrowserSvc((WPARAM)&Param, YAMN_MAILBROWSERVERSION);
					
			#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile, "Service_ContactDoubleclicked:ActualAccountSO-read done\n");
			#endif
			ReadDoneFcn(ActualAccount->AccountAccessSO);
		}
		#ifdef DEBUG_SYNCHRO
		else
			DebugLog(SynchroFile, "Service_ContactDoubleclicked:ActualAccountSO-read enter failed\n");
		#endif
				
	}
	db_free(&dbv);
}

/////////////////////////////////////////////////////////////////////////////////////////

HBITMAP LoadBmpFromIcon(HICON hIcon)
{
	int IconSizeX = 16;
	int IconSizeY = 16;

	HBRUSH hBkgBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));

	BITMAPINFOHEADER bih = {0};
	bih.biSize = sizeof(bih);
	bih.biBitCount = 24;
	bih.biPlanes = 1;
	bih.biCompression = BI_RGB;
	bih.biHeight = IconSizeY;
	bih.biWidth = IconSizeX; 
	
	RECT rc;
	rc.top = rc.left = 0;
	rc.right = bih.biWidth;
	rc.bottom = bih.biHeight;

	HDC hdc = GetDC(NULL);
	HBITMAP hBmp = CreateCompatibleBitmap(hdc, bih.biWidth, bih.biHeight);
	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
	FillRect(hdcMem, &rc, hBkgBrush);
	DrawIconEx(hdcMem, 0, 0, hIcon, bih.biWidth, bih.biHeight, 0, NULL, DI_NORMAL);
	SelectObject(hdcMem, hoBmp);
	return hBmp;
}

int AddTopToolbarIcon(WPARAM,LPARAM)
{
	if ( db_get_b(NULL, YAMN_DBMODULE, YAMN_TTBFCHECK, 1)) {
		if ( ServiceExists(MS_TTB_REMOVEBUTTON) && hTTButton == NULL) {
			TTBButton btn = { 0 };
			btn.pszService = MS_YAMN_FORCECHECK;
			btn.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
			btn.hIconHandleUp = btn.hIconHandleDn = g_GetIconHandle(0);
			btn.name = btn.pszTooltipUp = LPGEN("Check mail");
			hTTButton = TopToolbar_AddButton(&btn);
		}
	}
	else {
		if (hTTButton != NULL) {
			CallService(MS_TTB_REMOVEBUTTON, (WPARAM)hTTButton, 0);
			hTTButton = NULL;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int Shutdown(WPARAM, LPARAM)
{
	CallService(MS_TTB_REMOVEBUTTON, (WPARAM)hTTButton, 0);

	db_set_dw(NULL, YAMN_DBMODULE, YAMN_DBMSGPOSX, HeadPosX);
	db_set_dw(NULL, YAMN_DBMODULE, YAMN_DBMSGPOSY, HeadPosY);
	db_set_dw(NULL, YAMN_DBMODULE, YAMN_DBMSGSIZEX, HeadSizeX);
	db_set_dw(NULL, YAMN_DBMODULE, YAMN_DBMSGSIZEY, HeadSizeY);
	db_set_w(NULL, YAMN_DBMODULE, YAMN_DBMSGPOSSPLIT, HeadSplitPos);
	YAMNVar.Shutdown = TRUE;
	KillTimer(NULL, SecTimer);

	UnregisterProtoPlugins();
	UnregisterFilterPlugins();
	return 0;
}

int SystemModulesLoaded(WPARAM, LPARAM); //in main.cpp

void HookEvents(void)
{
	HookEvent(ME_SYSTEM_MODULESLOADED, SystemModulesLoaded);
	HookEvent(ME_TTB_MODULELOADED, AddTopToolbarIcon);
	HookEvent(ME_OPT_INITIALISE, YAMNOptInitSvc);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, Shutdown);
	HookEvent(ME_CLIST_DOUBLECLICKED, Service_ContactDoubleclicked);
}

void CreateServiceFunctions(void)
{
	// Standard 'protocol' services
	CreateServiceFunction(YAMN_DBMODULE PS_GETCAPS, Service_GetCaps);
	CreateServiceFunction(YAMN_DBMODULE PS_GETSTATUS, Service_GetStatus);
	CreateServiceFunction(YAMN_DBMODULE PS_SETSTATUS, Service_SetStatus);
	CreateServiceFunction(YAMN_DBMODULE PS_GETNAME, Service_GetName);
	CreateServiceFunction(YAMN_DBMODULE PS_LOADICON, Service_LoadIcon);

	// Function with which protocol plugin can register
	CreateServiceFunction(MS_YAMN_GETFCNPTR, GetFcnPtrSvc);

	// Function returns pointer to YAMN variables
	CreateServiceFunction(MS_YAMN_GETVARIABLES, GetVariablesSvc);

	// Function with which protocol plugin can register
	CreateServiceFunction(MS_YAMN_REGISTERPROTOPLUGIN, RegisterProtocolPluginSvc);

	// Function with which protocol plugin can unregister
	CreateServiceFunction(MS_YAMN_UNREGISTERPROTOPLUGIN, UnregisterProtocolPluginSvc);

	// Function creates an account for plugin
	CreateServiceFunction(MS_YAMN_CREATEPLUGINACCOUNT, CreatePluginAccountSvc);

	// Function deletes plugin account 
	CreateServiceFunction(MS_YAMN_DELETEPLUGINACCOUNT, DeletePluginAccountSvc);

	// Finds account for plugin by name
	CreateServiceFunction(MS_YAMN_FINDACCOUNTBYNAME, FindAccountByNameSvc);

	// Creates next account for plugin
	CreateServiceFunction(MS_YAMN_GETNEXTFREEACCOUNT, GetNextFreeAccountSvc);

	// Function removes account from YAMN queue. Does not delete it from memory
	CreateServiceFunction(MS_YAMN_DELETEACCOUNT, DeleteAccountSvc);

	// Function finds accounts for specified plugin
	CreateServiceFunction(MS_YAMN_READACCOUNTS, AddAccountsFromFileSvc);

	// Function that stores all plugin mails to one file 
	CreateServiceFunction(MS_YAMN_WRITEACCOUNTS, WriteAccountsToFileSvc);

	// Function that returns user's filename
	CreateServiceFunction(MS_YAMN_GETFILENAME, GetFileNameSvc);

	// Releases unicode string from memory
	CreateServiceFunction(MS_YAMN_DELETEFILENAME, DeleteFileNameSvc);

	// Checks mail
	CreateServiceFunction(MS_YAMN_FORCECHECK, ForceCheckSvc);

	// Runs YAMN's mail browser
	CreateServiceFunction(MS_YAMN_MAILBROWSER, RunMailBrowserSvc);

	// Runs YAMN's bad conenction window
	CreateServiceFunction(MS_YAMN_BADCONNECTION, RunBadConnectionSvc);

	// Function creates new mail for plugin
	CreateServiceFunction(MS_YAMN_CREATEACCOUNTMAIL, CreateAccountMailSvc);

	// Function deletes plugin account 
	CreateServiceFunction(MS_YAMN_DELETEACCOUNTMAIL, DeleteAccountMailSvc);

	// Function with which filter plugin can register
	CreateServiceFunction(MS_YAMN_REGISTERFILTERPLUGIN, RegisterFilterPluginSvc);

	// Function with which filter plugin can unregister
	CreateServiceFunction(MS_YAMN_UNREGISTERFILTERPLUGIN, UnregisterFilterPluginSvc);

	// Function filters mail
	CreateServiceFunction(MS_YAMN_FILTERMAIL, FilterMailSvc);

	// Function contact list double click
	CreateServiceFunction(MS_YAMN_CLISTDBLCLICK, ClistContactDoubleclicked);

	// Function to check individual account
	CreateServiceFunction(MS_YAMN_ACCOUNTCHECK, AccountMailCheck);

	// Function contact list context menu click
	CreateServiceFunction(MS_YAMN_CLISTCONTEXT, ContactMailCheck);

	// Function contact list context menu click
	CreateServiceFunction(MS_YAMN_CLISTCONTEXTAPP, ContactApplication);
}

//Function to put all enabled contact to the Online status
void RefreshContact(void)
{
	HACCOUNT Finder;
	for (Finder = POP3Plugin->FirstAccount;Finder != NULL;Finder = Finder->Next) {
		if (Finder->hContact != NULL) {
			if ((Finder->Flags & YAMN_ACC_ENA) && (Finder->NewMailN.Flags & YAMN_ACC_CONT))
				db_unset(Finder->hContact, "CList", "Hidden");
			else
				db_set_b(Finder->hContact, "CList", "Hidden", 1);
		}
		else if ((Finder->Flags & YAMN_ACC_ENA) && (Finder->NewMailN.Flags & YAMN_ACC_CONT)) {
			Finder->hContact = db_add_contact();
			Proto_AddToContact(Finder->hContact, YAMN_DBMODULE);
			db_set_s(Finder->hContact, YAMN_DBMODULE, "Id", Finder->Name);
			db_set_s(Finder->hContact, YAMN_DBMODULE, "Nick", Finder->Name);
			db_set_s(Finder->hContact, "Protocol", "p", YAMN_DBMODULE);
			db_set_w(Finder->hContact, YAMN_DBMODULE, "Status", ID_STATUS_ONLINE);
			db_set_s(Finder->hContact, "CList", "StatusMsg", Translate("No new mail message"));
}	}	}
