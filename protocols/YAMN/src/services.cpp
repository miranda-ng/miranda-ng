#include "stdafx.h"

static INT_PTR Service_GetCaps(WPARAM wParam, LPARAM)
{
	if (wParam == PFLAGNUM_4)
		return PF4_NOCUSTOMAUTH;
	if (wParam == PFLAG_UNIQUEIDTEXT)
		return (INT_PTR)TranslateT("Nick");
	if (wParam == PFLAG_MAXLENOFMESSAGE)
		return 400;
	if (wParam == PFLAGNUM_2)
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND;
	if (wParam == PFLAGNUM_5) {
		if (g_plugin.getByte(YAMN_SHOWASPROTO, 1))
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
	int newstatus = (wParam != ID_STATUS_OFFLINE) ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
	if (newstatus != YAMN_STATUS) {
		int oldstatus = YAMN_STATUS;
		YAMN_STATUS = newstatus;
		ProtoBroadcastAck(YAMN_DBMODULE, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldstatus, newstatus);
	}
	return 0;

}

static INT_PTR Service_GetName(WPARAM wParam, LPARAM lParam)
{
	mir_strncpy((char *)lParam, YAMN_DBMODULE, wParam);
	return 0;
}

static INT_PTR Service_LoadIcon(WPARAM wParam, LPARAM)
{
	if (LOWORD(wParam) == PLI_PROTOCOL)
		return (INT_PTR)CopyIcon(g_plugin.getIcon(IDI_CHECKMAIL)); // noone cares about other than PLI_PROTOCOL

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
	char *szProto = Proto_GetBaseAccountName(wParam);
	if (mir_strcmp(szProto, YAMN_DBMODULE))
		return 0;

	DBVARIANT dbv;
	if (g_plugin.getString(wParam, "Id", &dbv))
		return 0;

	CAccount *ActualAccount = (CAccount *)CallService(MS_YAMN_FINDACCOUNTBYNAME, (WPARAM)POP3Plugin, (LPARAM)dbv.pszVal);
	if (ActualAccount != nullptr) {
		STARTUPINFOW si = { 0 };
		si.cb = sizeof(si);

#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile, "ContactApplication:ActualAccountSO-read wait\n");
#endif
		if (WAIT_OBJECT_0 == WaitToReadFcn(ActualAccount->AccountAccessSO)) {
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile, "ContactApplication:ualAccountSO-read enter\n");
#endif
			if (ActualAccount->NewMailN.App != nullptr) {
				wchar_t *Command;
				if (ActualAccount->NewMailN.AppParam != nullptr)
					Command = new wchar_t[mir_wstrlen(ActualAccount->NewMailN.App) + mir_wstrlen(ActualAccount->NewMailN.AppParam) + 6];
				else
					Command = new wchar_t[mir_wstrlen(ActualAccount->NewMailN.App) + 6];

				if (Command != nullptr) {
					mir_wstrcpy(Command, L"\"");
					mir_wstrcat(Command, ActualAccount->NewMailN.App);
					mir_wstrcat(Command, L"\" ");
					if (ActualAccount->NewMailN.AppParam != nullptr)
						mir_wstrcat(Command, ActualAccount->NewMailN.AppParam);

					PROCESS_INFORMATION pi;
					CreateProcessW(nullptr, Command, nullptr, nullptr, FALSE, NORMAL_PRIORITY_CLASS, nullptr, nullptr, &si, &pi);
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

uint32_t WINAPI SWMRGWaitToRead(PSWMRG pSWMRG, uint32_t dwTimeout);
static INT_PTR AccountMailCheck(WPARAM wParam, LPARAM lParam)
{
	//This service will check/sincronize the account pointed by wParam
	CAccount *ActualAccount = (CAccount *)wParam;
	// copy/paste make mistakes
	if (ActualAccount != nullptr) {
		//we use event to signal, that running thread has all needed stack parameters copied
		HANDLE ThreadRunningEV = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (ThreadRunningEV == nullptr)
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
				CheckParam ParamToPlugin = { YAMN_CHECKVERSION, ThreadRunningEV, ActualAccount, lParam ? YAMN_FORCECHECK : YAMN_NORMALCHECK, nullptr, nullptr };

				ActualAccount->TimeLeft = ActualAccount->Interval;
				DWORD tid;
				HANDLE NewThread = CreateThread(nullptr, 0, (YAMN_STANDARDFCN)ActualAccount->Plugin->Fcn->SynchroFcnPtr, &ParamToPlugin, 0, &tid);
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
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (mir_strcmp(szProto, YAMN_DBMODULE))
		return 0;

	DBVARIANT dbv;
	if (g_plugin.getString(hContact, "Id", &dbv))
		return 0;

	CAccount *ActualAccount = (CAccount *)CallService(MS_YAMN_FINDACCOUNTBYNAME, (WPARAM)POP3Plugin, (LPARAM)dbv.pszVal);
	if (ActualAccount != nullptr) {
		//we use event to signal, that running thread has all needed stack parameters copied
		HANDLE ThreadRunningEV;
		if (nullptr == (ThreadRunningEV = CreateEvent(nullptr, FALSE, FALSE, nullptr)))
			return 0;
		//if we want to close miranda, we get event and do not run pop3 checking anymore
		if (WAIT_OBJECT_0 == WaitForSingleObject(ExitEV, 0))
			return 0;
		mir_cslock lck(PluginRegCS);
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile, "ForceCheck:ActualAccountSO-read wait\n");
#endif
		if (WAIT_OBJECT_0 != WaitToReadFcn(ActualAccount->AccountAccessSO)) {
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile, "ForceCheck:ActualAccountSO-read wait failed\n");
#endif
		}
		else {
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile, "ForceCheck:ActualAccountSO-read enter\n");
#endif
			if ((ActualAccount->Flags & YAMN_ACC_ENA) && (ActualAccount->StatusFlags & YAMN_ACC_FORCE))			//account cannot be forced to check
			{
				if (ActualAccount->Plugin->Fcn->ForceCheckFcnPtr == nullptr)
					ReadDoneFcn(ActualAccount->AccountAccessSO);

				DWORD tid;
				struct CheckParam ParamToPlugin = { YAMN_CHECKVERSION, ThreadRunningEV, ActualAccount, YAMN_FORCECHECK, (void *)nullptr, nullptr };
				if (nullptr == CreateThread(nullptr, 0, (YAMN_STANDARDFCN)ActualAccount->Plugin->Fcn->ForceCheckFcnPtr, &ParamToPlugin, 0, &tid))
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
	char *szProto = Proto_GetBaseAccountName(wParam);
	if (mir_strcmp(szProto, YAMN_DBMODULE))
		return;

	DBVARIANT dbv;
	if (g_plugin.getString(wParam, "Id", &dbv))
		return;

	CAccount *ActualAccount = (CAccount *)CallService(MS_YAMN_FINDACCOUNTBYNAME, (WPARAM)POP3Plugin, (LPARAM)dbv.pszVal);
	if (ActualAccount != nullptr) {
#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile, "Service_ContactDoubleclicked:ActualAccountSO-read wait\n");
#endif
		if (WAIT_OBJECT_0 == WaitToReadFcn(ActualAccount->AccountAccessSO)) {
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile, "Service_ContactDoubleclicked:ActualAccountSO-read enter\n");
#endif
			YAMN_MAILBROWSERPARAM Param = { nullptr, ActualAccount, ActualAccount->NewMailN.Flags, ActualAccount->NoNewMailN.Flags, nullptr };

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

	BITMAPINFOHEADER bih = { 0 };
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

	HDC hdc = GetDC(nullptr);
	HBITMAP hBmp = CreateCompatibleBitmap(hdc, bih.biWidth, bih.biHeight);
	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
	FillRect(hdcMem, &rc, hBkgBrush);
	DrawIconEx(hdcMem, 0, 0, hIcon, bih.biWidth, bih.biHeight, 0, nullptr, DI_NORMAL);
	SelectObject(hdcMem, hoBmp);
	return hBmp;
}

int AddTopToolbarIcon(WPARAM, LPARAM)
{
	if (g_plugin.getByte(YAMN_TTBFCHECK, 1)) {
		if (ServiceExists(MS_TTB_REMOVEBUTTON) && hTTButton == nullptr) {
			TTBButton btn = {};
			btn.pszService = MS_YAMN_FORCECHECK;
			btn.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
			btn.hIconHandleUp = btn.hIconHandleDn = g_plugin.getIconHandle(IDI_CHECKMAIL);
			btn.name = btn.pszTooltipUp = LPGEN("Check mail");
			hTTButton = g_plugin.addTTB(&btn);
		}
	}
	else {
		if (hTTButton != nullptr) {
			CallService(MS_TTB_REMOVEBUTTON, (WPARAM)hTTButton, 0);
			hTTButton = nullptr;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int Shutdown(WPARAM, LPARAM)
{
	CallService(MS_TTB_REMOVEBUTTON, (WPARAM)hTTButton, 0);

	g_plugin.setDword(YAMN_DBMSGPOSX, HeadPosX);
	g_plugin.setDword(YAMN_DBMSGPOSY, HeadPosY);
	g_plugin.setDword(YAMN_DBMSGSIZEX, HeadSizeX);
	g_plugin.setDword(YAMN_DBMSGSIZEY, HeadSizeY);
	g_plugin.setWord(YAMN_DBMSGPOSSPLIT, HeadSplitPos);
	YAMNVar.Shutdown = TRUE;
	KillTimer(nullptr, SecTimer);

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

// Function to put all enabled contact to the Online status
void RefreshContact(void)
{
	CAccount *Finder;
	for (Finder = POP3Plugin->FirstAccount; Finder != nullptr; Finder = Finder->Next) {
		if (Finder->hContact != NULL) {
			Contact::Hide(Finder->hContact, !(Finder->Flags & YAMN_ACC_ENA) && (Finder->NewMailN.Flags & YAMN_ACC_CONT));
		}
		else if ((Finder->Flags & YAMN_ACC_ENA) && (Finder->NewMailN.Flags & YAMN_ACC_CONT)) {
			Finder->hContact = db_add_contact();
			Proto_AddToContact(Finder->hContact, YAMN_DBMODULE);
			g_plugin.setString(Finder->hContact, "Id", Finder->Name);
			g_plugin.setString(Finder->hContact, "Nick", Finder->Name);
			g_plugin.setWord(Finder->hContact, "Status", ID_STATUS_ONLINE);
			db_set_s(Finder->hContact, "CList", "StatusMsg", Translate("No new mail message"));
		}
	}
}
