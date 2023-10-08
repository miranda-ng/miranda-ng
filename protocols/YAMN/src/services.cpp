#include "stdafx.h"

static INT_PTR Service_GetCaps(WPARAM wParam, LPARAM)
{
	switch (wParam) {
	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT("Nick");
	case PFLAG_MAXLENOFMESSAGE:
		return 400;
	case PFLAGNUM_2:
	case PFLAGNUM_5:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND;
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
	ContactDoubleclicked(((CLISTEVENT *)lParam)->lParam, lParam);
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

	CAccount *ActualAccount = FindAccountByName(POP3Plugin, dbv.pszVal);
	if (ActualAccount != nullptr) {
		STARTUPINFOW si = { 0 };
		si.cb = sizeof(si);

		SReadGuard sra(ActualAccount->AccountAccessSO);
		if (sra.Succeeded()) {
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
		}
	}
	db_free(&dbv);
	return 0;
}

static INT_PTR AccountMailCheck(WPARAM wParam, LPARAM lParam)
{
	// This service will check/sincronize the account pointed by wParam
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
		SReadGuard sra(ActualAccount->AccountAccessSO, 0);
		if (sra.Succeeded()) {
			if ((ActualAccount->Flags & YAMN_ACC_ENA) && ActualAccount->Plugin->Fcn->SynchroFcnPtr) {
				CheckParam ParamToPlugin = { YAMN_CHECKVERSION, ThreadRunningEV, ActualAccount, lParam != 0 ? YAMN_FORCECHECK : YAMN_NORMALCHECK, nullptr, nullptr };

				ActualAccount->TimeLeft = ActualAccount->Interval;
				DWORD tid;
				HANDLE NewThread = CreateThread(nullptr, 0, (YAMN_STANDARDFCN)ActualAccount->Plugin->Fcn->SynchroFcnPtr, &ParamToPlugin, 0, &tid);
				if (NewThread) {
					WaitForSingleObject(ThreadRunningEV, INFINITE);
					CloseHandle(NewThread);
				}
			}
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

	if (CAccount *ActualAccount = FindAccountByName(POP3Plugin, dbv.pszVal)) {
		// we use event to signal, that running thread has all needed stack parameters copied
		HANDLE ThreadRunningEV;
		if (nullptr == (ThreadRunningEV = CreateEvent(nullptr, FALSE, FALSE, nullptr)))
			return 0;

		// if we want to close miranda, we get event and do not run pop3 checking anymore
		if (WAIT_OBJECT_0 == WaitForSingleObject(ExitEV, 0))
			return 0;

		mir_cslock lck(PluginRegCS);
		SReadGuard sra(ActualAccount->AccountAccessSO);
		if (sra.Succeeded()) {
			// account cannot be forced to check
			if ((ActualAccount->Flags & YAMN_ACC_ENA) && (ActualAccount->StatusFlags & YAMN_ACC_FORCE)) { 
				DWORD tid;
				CheckParam ParamToPlugin = { YAMN_CHECKVERSION, ThreadRunningEV, ActualAccount, YAMN_FORCECHECK, (void *)nullptr, nullptr };
				if (CreateThread(nullptr, 0, (YAMN_STANDARDFCN)ActualAccount->Plugin->Fcn->ForceCheckFcnPtr, &ParamToPlugin, 0, &tid))
					WaitForSingleObject(ThreadRunningEV, INFINITE);
			}
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

	if (CAccount *ActualAccount = FindAccountByName(POP3Plugin, dbv.pszVal)) {
		SReadGuard sra(ActualAccount->AccountAccessSO);
		if (sra.Succeeded()) {
			YAMN_MAILBROWSERPARAM Param = { ActualAccount, ActualAccount->NewMailN.Flags, ActualAccount->NoNewMailN.Flags, nullptr };

			Param.nnflags = Param.nnflags | YAMN_ACC_MSG;			//show mails in account even no new mail in account
			Param.nnflags = Param.nnflags & ~YAMN_ACC_POP;

			Param.nflags = Param.nflags | YAMN_ACC_MSG;			//show mails in account even no new mail in account
			Param.nflags = Param.nflags & ~YAMN_ACC_POP;

			RunMailBrowserSvc((WPARAM)&Param, YAMN_MAILBROWSERVERSION);
		}
	}
	db_free(&dbv);
}

/////////////////////////////////////////////////////////////////////////////////////////

HBITMAP LoadBmpFromIcon(HICON hIcon)
{
	int IconSizeX = 16;
	int IconSizeY = 16;

	HBRUSH hBkgBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));

	BITMAPINFOHEADER bih = {};
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

static int AddTopToolbarIcon(WPARAM, LPARAM)
{
	TTBButton btn = {};
	btn.pszService = MS_YAMN_FORCECHECK;
	btn.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	btn.hIconHandleUp = btn.hIconHandleDn = g_plugin.getIconHandle(IDI_CHECKMAIL);
	btn.name = btn.pszTooltipUp = LPGEN("Check mail");
	g_plugin.addTTB(&btn);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int Shutdown(WPARAM, LPARAM)
{
	g_plugin.setDword(YAMN_DBMSGPOSX, HeadPosX);
	g_plugin.setDword(YAMN_DBMSGPOSY, HeadPosY);
	g_plugin.setDword(YAMN_DBMSGSIZEX, HeadSizeX);
	g_plugin.setDword(YAMN_DBMSGSIZEY, HeadSizeY);
	g_plugin.setWord(YAMN_DBMSGPOSSPLIT, HeadSplitPos);
	YAMNVar.Shutdown = TRUE;
	KillTimer(nullptr, SecTimer);

	UnregisterProtoPlugins();
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
	CreateServiceFunction(YAMN_DBMODULE PS_GETNAME, Service_GetName);
	CreateServiceFunction(YAMN_DBMODULE PS_LOADICON, Service_LoadIcon);

	// Checks mail
	CreateServiceFunction(MS_YAMN_FORCECHECK, ForceCheckSvc);

	// Runs YAMN's mail browser
	CreateServiceFunction(MS_YAMN_MAILBROWSER, RunMailBrowserSvc);

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
