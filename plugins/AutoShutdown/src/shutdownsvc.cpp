/*

'AutoShutdown'-Plugin for Miranda IM

Copyright 2004-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Shutdown-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

/* Shutdown Dialog */
static HWND hwndShutdownDlg;

/* Events */
static HANDLE hEventOkToShutdown,hEventShutdown;

/************************* Utils **************************************/

wchar_t *desc[] =
{
	LPGENW("Miranda NG is going to be automatically closed in %u second(s)."),
	LPGENW("All Miranda NG protocols are going to be set to offline in %u second(s)."),
	LPGENW("You will be logged off automatically in %u second(s)."),
	LPGENW("The computer will automatically be restarted in %u second(s)."),
	LPGENW("The computer will automatically be set to standby mode in %u second(s)."),
	LPGENW("The computer will automatically be set to hibernate mode in %u second(s)."),
	LPGENW("The workstation will automatically get locked in %u second(s)."),
	LPGENW("All dial-up connections will be closed in %u second(s)."),
	LPGENW("The computer will automatically be shut down in %u second(s).")
};

static BOOL WinNT_SetPrivilege(wchar_t *pszPrivName, BOOL bEnable)
{
	BOOL bReturn = FALSE;
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	/* get a token for this process */
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		tkp.PrivilegeCount = 1; /* one privilege is to set */
		/* get the LUID for the shutdown privilege */
		if (LookupPrivilegeValue(nullptr, pszPrivName, &tkp.Privileges[0].Luid)) {
			tkp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;
			/* get the shutdown privilege for this process */
			bReturn = AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, nullptr, nullptr);
		}
		CloseHandle(hToken);
	}
	return bReturn;
}

static void BroadcastEndSession(DWORD dwRecipients, LPARAM lParam)
{
	BroadcastSystemMessage(BSF_FORCEIFHUNG, &dwRecipients, WM_ENDSESSION, TRUE, lParam);
}

/************************* Workers ************************************/
static BOOL IsShutdownTypeEnabled(BYTE shutdownType)
{
	BOOL bReturn = FALSE;
	switch (shutdownType) {
	case SDSDT_HIBERNATE:
	case SDSDT_STANDBY:
		bReturn = shutdownType == SDSDT_HIBERNATE ? IsPwrHibernateAllowed() : IsPwrSuspendAllowed() != 0;
		/* test privilege */
		if (bReturn) {
			bReturn = WinNT_SetPrivilege(SE_SHUTDOWN_NAME, TRUE);
			if (bReturn) WinNT_SetPrivilege(SE_SHUTDOWN_NAME, FALSE);
		}
		break;
	case SDSDT_LOGOFF:
		{
			HKEY hKey;
			DWORD dwSetting, dwSize;
			/* NoLogOff is BINARY on Win9x/ME and DWORD on Win2000+ */
			bReturn = TRUE;
			if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
				dwSize = sizeof(dwSetting);
				if (RegQueryValueEx(hKey, L"NoLogOff", nullptr, nullptr, (LPBYTE)&dwSetting, &dwSize) == ERROR_SUCCESS)
					if (dwSetting) bReturn = FALSE;
				RegCloseKey(hKey);
			}
		}
		break;
	case SDSDT_LOCKWORKSTATION:
		{
			HKEY hKey;
			DWORD dwSize, dwSetting;
			/* DisableLockWorkstation is DWORD on Win2000+ */
			bReturn = TRUE;
			if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
				dwSize = sizeof(dwSetting);
				if (!RegQueryValueEx(hKey, L"DisableLockWorkstation", nullptr, nullptr, (LPBYTE)&dwSetting, &dwSize))
					if (dwSetting)
						bReturn = FALSE;
				RegCloseKey(hKey);
			}
		}
		break;
	case SDSDT_CLOSERASCONNECTIONS:
		/* check if RAS installed/available */
		bReturn = TRUE;
		break;
	case SDSDT_SETMIRANDAOFFLINE:
	case SDSDT_CLOSEMIRANDA:
		bReturn = TRUE; /* always possible */
		break;
	case SDSDT_REBOOT:
	case SDSDT_SHUTDOWN:
		/* test privileges */
		bReturn = WinNT_SetPrivilege(SE_SHUTDOWN_NAME, TRUE);
		if (bReturn)
			WinNT_SetPrivilege(SE_SHUTDOWN_NAME, FALSE);
		break;
	}
	return bReturn;
}

static DWORD ShutdownNow(BYTE shutdownType)
{
	DWORD dwErrCode = ERROR_SUCCESS;
	switch (shutdownType) {
	case SDSDT_CLOSEMIRANDA:
		if (!Miranda_IsTerminated()) {
			/* waiting for short until ready (but not too long...) */
			DWORD dwLastTickCount = GetTickCount();
			while (!Miranda_OkToExit()) {
				/* infinite loop protection (max 5 sec) */
				if (GetTickCount() - dwLastTickCount >= 5000) { /* wraparound works */
					OutputDebugStringA("Timeout (5 sec)\n"); /* tell others, all ascii */
					break;
				}
				SleepEx(1000, TRUE);
				if (Miranda_IsTerminated()) break; /* someone else did it */
				OutputDebugStringA("Not ready to exit. Waiting...\n"); /* tell others, all ascii */
			}
			/* shutdown service must be called from main thread anyway */
			if (!DestroyWindow(g_clistApi.hwndContactList))
				dwErrCode = GetLastError();
		}
		break;

	case SDSDT_SETMIRANDAOFFLINE:
		/* set global status mode to offline (is remembered by Miranda on exit) */
		Clist_SetStatusMode(ID_STATUS_OFFLINE);
		break;

	case SDSDT_STANDBY:
	case SDSDT_HIBERNATE:
		WinNT_SetPrivilege(SE_SHUTDOWN_NAME, TRUE);
		if (!SetSystemPowerState(shutdownType == SDSDT_STANDBY, TRUE))
			dwErrCode = GetLastError();
		WinNT_SetPrivilege(SE_SHUTDOWN_NAME, FALSE);
		break;

	case SDSDT_LOCKWORKSTATION:
		if (!IsWorkstationLocked())
			dwErrCode = GetLastError();
		break;

	case SDSDT_CLOSERASCONNECTIONS:
		ShutdownNow(SDSDT_SETMIRANDAOFFLINE); /* set Miranda offline */
		/* hang up all ras connections */
		{
			DWORD dwRetries;
			RASCONNSTATUS rcs;
			DWORD dw, dwLastTickCount;

			DWORD dwConnSize = sizeof(RASCONN);
			DWORD dwConnItems = 0;
			RASCONN *paConn = (RASCONN*)mir_alloc(dwConnSize);
			dwErrCode = ERROR_NOT_ENOUGH_MEMORY;
			if (paConn != nullptr) {
				for (dwRetries = 5; dwRetries != 0; dwRetries--) { /* prevent infinite loop (rare) */
					memset(paConn, 0, dwConnSize);
					paConn[0].dwSize = sizeof(RASCONN);
					dwErrCode = RasEnumConnections(paConn, &dwConnSize, &dwConnItems);
					if (dwErrCode != ERROR_BUFFER_TOO_SMALL) break;
					RASCONN *paConnBuf = (RASCONN*)mir_realloc(paConn, dwConnSize);
					if (paConnBuf == nullptr) {
						mir_free(paConn);
						paConn = nullptr;
						dwErrCode = ERROR_NOT_ENOUGH_MEMORY;
						break;
					}
					paConn = paConnBuf;
				}
				if (dwErrCode == ERROR_SUCCESS || dwErrCode == ERROR_BUFFER_TOO_SMALL) {
					for (dw = 0; dw < dwConnItems; ++dw) {
						if (dwErrCode) {
							if (RasHangUp(paConn[dw].hrasconn))
								paConn[dw].hrasconn = nullptr; /* do not wait for on error */
						}
						else {
							dwErrCode = RasHangUp(paConn[dw].hrasconn);
							if (!dwErrCode) paConn[dw].hrasconn = nullptr; /* do not wait for on error */
						}
					}
					/* RAS does not allow to quit directly after HangUp (see docs) */
					dwLastTickCount = GetTickCount();
					memset(&rcs, 0, sizeof(RASCONNSTATUS));
					rcs.dwSize = sizeof(RASCONNSTATUS);
					for (dw = 0; dw < dwConnItems; ++dw) {
						if (paConn[dw].hrasconn != nullptr) {
							while (RasGetConnectStatus(paConn[dw].hrasconn, &rcs) != ERROR_INVALID_HANDLE) {
								Sleep(0); /* give rest of time silce to other threads with equal priority */
								/* infinite loop protection (3000ms defined in docs) */
								dwRetries = GetTickCount();
								if (dwRetries - dwLastTickCount > 3000)
									break; /* wraparound works */
							}
						}
					}
				}
				mir_free(paConn); /* does NULL check */
			}
		}
		/* set Miranda to offline again, to remain offline with reconnection plugins */
		ShutdownNow(SDSDT_SETMIRANDAOFFLINE);
		break;

	case SDSDT_REBOOT:
	case SDSDT_SHUTDOWN:
		if (GetSystemMetrics(SM_SHUTTINGDOWN)) { /* Win2000+, 0 on error */
			dwErrCode = ERROR_SHUTDOWN_IN_PROGRESS;
			break;
		}
		/* WinNT4/2000/XP */
		{
			WinNT_SetPrivilege(SE_SHUTDOWN_NAME, TRUE);

			/* does not send out WM_ENDSESSION messages, so we do it manually to
			* give the applications the chance to save their data */
			WinNT_SetPrivilege(SE_TCB_NAME, TRUE); /* for BSM_ALLDESKTOPS */
			BroadcastEndSession(BSM_APPLICATIONS | BSM_ALLDESKTOPS, ENDSESSION_CLOSEAPP); /* app should close itself */
			WinNT_SetPrivilege(SE_TCB_NAME, FALSE);

			if (!InitiateSystemShutdownEx(nullptr, TranslateT(MODULENAME), 0, TRUE, shutdownType == SDSDT_REBOOT, SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED))
				dwErrCode = GetLastError();

			/* cleanly close Miranda */
			if (!dwErrCode) ShutdownNow(SDSDT_CLOSEMIRANDA);
			break;
		}
		/* fall through for Win9x */
	case SDSDT_LOGOFF:
		{
			UINT flags;
			switch (shutdownType) {
			case SDSDT_LOGOFF: flags = EWX_LOGOFF; break;
			case SDSDT_REBOOT: flags = EWX_REBOOT; break;
			default:           flags = EWX_SHUTDOWN | EWX_POWEROFF;
			}
			if (shutdownType == SDSDT_LOGOFF && !IsWorkstationLocked())
				flags |= EWX_FORCEIFHUNG; /* only considered for WM_ENDSESSION messages */
			else
				flags |= EWX_FORCE; /* must be used when workstation locked */

			if (flags & EWX_FORCE) {
				/* EWX_FORCE does not send out WM_ENDSESSION messages, so we do it
				* manually to give the applications the chance to save their data */
				BroadcastEndSession(BSM_APPLICATIONS, (shutdownType == SDSDT_LOGOFF) ? ENDSESSION_LOGOFF : 0);

				/* Windows Me/98/95 (msdn): Because of the design of the shell,
				* calling ExitWindowsEx with EWX_FORCE fails to completely log off
				* the user (the system terminates the applications and displays the
				* Enter Windows Password dialog box, however, the user's desktop remains.)
				* To log off the user forcibly, terminate the Explorer process before calling
				* ExitWindowsEx with EWX_LOGOFF and EWX_FORCE. */
			}
			if (!ExitWindowsEx(flags, SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED))
				dwErrCode = GetLastError();
			/* cleanly close Miranda */
			if (!dwErrCode)
				ShutdownNow(SDSDT_CLOSEMIRANDA);
		}
		break;
	}
	return dwErrCode;
}

/************************* Shutdown Dialog ****************************/

#define M_START_SHUTDOWN    (WM_APP+111)
#define M_UPDATE_COUNTDOWN  (WM_APP+112)
static INT_PTR CALLBACK ShutdownDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BYTE shutdownType = (BYTE)GetWindowLongPtr(hwndDlg, DWLP_USER);
	WORD countdown = (WORD)GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TEXT_HEADER), GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		hwndShutdownDlg = hwndDlg;
		SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)lParam);
		TranslateDialogDefault(hwndDlg);

		if (lParam == SDSDT_SHUTDOWN || lParam == SDSDT_REBOOT || lParam == SDSDT_LOGOFF)
			ShowWindow(GetDlgItem(hwndDlg, IDC_TEXT_UNSAVEDWARNING), SW_SHOW);
		SendDlgItemMessage(hwndDlg, IDC_ICON_HEADER, STM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIcon("AutoShutdown_Header"));
		{
			HFONT hBoldFont;
			LOGFONT lf;
			if (GetObject((HFONT)SendDlgItemMessage(hwndDlg, IDC_TEXT_HEADER, WM_GETFONT, 0, 0), sizeof(lf), &lf)) {
				lf.lfWeight = FW_BOLD;
				hBoldFont = CreateFontIndirect(&lf);
			}
			else hBoldFont = nullptr;
			SendDlgItemMessage(hwndDlg, IDC_TEXT_HEADER, WM_SETFONT, (WPARAM)hBoldFont, FALSE);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TEXT_HEADER), GWLP_USERDATA, (LONG_PTR)hBoldFont);
		}
		{
			countdown = g_plugin.getWord("ConfirmDlgCountdown", SETTING_CONFIRMDLGCOUNTDOWN_DEFAULT);
			if (countdown < 3)
				countdown = SETTING_CONFIRMDLGCOUNTDOWN_DEFAULT;
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TEXT_HEADER), GWLP_USERDATA, countdown);
			SendMessage(hwndDlg, M_UPDATE_COUNTDOWN, 0, countdown);
		}
		Skin_PlaySound("AutoShutdown_Countdown");
		if (!SetTimer(hwndDlg, 1, 1000, nullptr))
			PostMessage(hwndDlg, M_START_SHUTDOWN, 0, 0);

		Utils_RestoreWindowPositionNoSize(hwndDlg, NULL, MODULENAME, "ConfirmDlg_");

		/* disallow foreground window changes (WinMe/2000+) */
		SetForegroundWindow(hwndDlg);
		LockSetForegroundWindow(LSFW_LOCK);

		SendMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, IDCANCEL), TRUE);
		return FALSE; /* focus set on cancel */

	case WM_DESTROY:
		{
			hwndShutdownDlg = nullptr;
			ShowWindow(hwndDlg, SW_HIDE);
			/* reallow foreground window changes (WinMe/2000+) */
			LockSetForegroundWindow(LSFW_UNLOCK);
			Utils_SaveWindowPosition(hwndDlg, NULL, MODULENAME, "ConfirmDlg_");
			HFONT hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_TEXT_HEADER, WM_GETFONT, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_TEXT_HEADER, WM_SETFONT, 0, FALSE); /* no return value */
			if (hFont != nullptr) DeleteObject(hFont);
		}
		return TRUE;

	case M_START_SHUTDOWN:
		if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BUTTON_SHUTDOWNNOW))) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_SHUTDOWNNOW), FALSE);
			ShowWindow(hwndDlg, SW_HIDE);  /* get rid of the dialog immediately */
			DWORD dwErrCode = ShutdownNow(shutdownType);
			if (dwErrCode != ERROR_SUCCESS) {
				char *pszErr = GetWinErrorDescription(dwErrCode);
				ShowInfoMessage(NIIF_ERROR, Translate("Automatic shutdown error"), Translate("The shutdown process failed!\nReason: %s"), (pszErr != nullptr) ? pszErr : Translate("Unknown"));
				if (pszErr != nullptr) LocalFree(pszErr);
			}
			DestroyWindow(hwndDlg);
		}
		return TRUE;

	case WM_TIMER:
		if (countdown) {
			--countdown;
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TEXT_HEADER), GWLP_USERDATA, countdown);
			if (countdown == 27 || countdown == 24 || countdown == 21 || countdown == 19 ||
				countdown == 17 || countdown == 15 || countdown == 13 || countdown == 11 ||
				countdown <= 10)
				Skin_PlaySound("AutoShutdown_Countdown");
		}
		else KillTimer(hwndDlg, wParam);  /* countdown finished */
		PostMessage(hwndDlg, M_UPDATE_COUNTDOWN, 0, countdown);
		return TRUE;

	case M_UPDATE_COUNTDOWN:  /* lParam=(WORD)countdown */
		{
			wchar_t szText[256];
			mir_snwprintf(szText, TranslateW(desc[shutdownType - 1]), lParam);
			SetDlgItemText(hwndDlg, IDC_TEXT_HEADER, szText);
			/* countdown finished */
			if (!lParam)
				PostMessage(hwndDlg, M_START_SHUTDOWN, 0, 0);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_SHUTDOWNNOW:
			KillTimer(hwndDlg, 1);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TEXT_HEADER), GWLP_USERDATA, 0);
			SendMessage(hwndDlg, M_UPDATE_COUNTDOWN, 0, (LONG)0);
			PostMessage(hwndDlg, M_START_SHUTDOWN, 0, 0);
			return TRUE;
		case IDCANCEL: /* WM_CLOSE */
			if (countdown) {
				KillTimer(hwndDlg, 1);
				DestroyWindow(hwndDlg);
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/************************* Services ***********************************/

INT_PTR ServiceShutdown(WPARAM wParam, LPARAM lParam)
{
	/* passing 0 as wParam is only to be used internally, undocumented */
	if (!wParam) wParam = g_plugin.getByte("ShutdownType", SETTING_SHUTDOWNTYPE_DEFAULT);
	if (!IsShutdownTypeEnabled((BYTE)wParam)) return 1; /* does shutdownType range check */
	if ((BOOL)lParam && hwndShutdownDlg != nullptr) return 2;

	/* ask others if allowed */
	if (NotifyEventHooks(hEventOkToShutdown, wParam, lParam)) {
		OutputDebugStringA("automatic shutdown denied by event hook\n"); /* all ascii */
		return 3;
	}
	/* tell others */
	NotifyEventHooks(hEventShutdown, wParam, lParam);
	/* show dialog */
	if (lParam && g_plugin.getByte("ShowConfirmDlg", SETTING_SHOWCONFIRMDLG_DEFAULT))
		if (CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SHUTDOWNNOW), nullptr, ShutdownDlgProc, (BYTE)wParam) != nullptr)
			return 0;
	/* show error */

	DWORD dwErrCode = ShutdownNow((BYTE)wParam);
	if (dwErrCode != ERROR_SUCCESS) {
		char *pszErr = GetWinErrorDescription(dwErrCode);
		ShowInfoMessage(NIIF_ERROR, Translate("Automatic shutdown error"), Translate("Initiating the shutdown process failed!\nReason: %s"), (pszErr != nullptr) ? pszErr : Translate("Unknown"));
		if (pszErr != nullptr)
			LocalFree(pszErr);
		return 4;
	}

	return 0;
}

INT_PTR ServiceIsTypeEnabled(WPARAM wParam, LPARAM)
{
	return IsShutdownTypeEnabled((BYTE)wParam); /* does shutdownType range check */
}

const wchar_t *apszShort[] = {
	LPGENW("Close Miranda NG"),LPGENW("Set Miranda NG offline"),LPGENW("Log off user"),
	LPGENW("Restart computer"),LPGENW("Shutdown computer"),LPGENW("Standby mode"),LPGENW("Hibernate mode"),
	LPGENW("Lock workstation"),LPGENW("Hang up dial-up connections"),LPGENW("Close Miranda NG"),
	LPGENW("Set Miranda NG offline"),LPGENW("Log off user"),LPGENW("Restart computer"),LPGENW("Shutdown computer"),
	LPGENW("Standby mode"),LPGENW("Hibernate mode"),LPGENW("Lock workstation"),LPGENW("Hang up dial-up connections")
};

const wchar_t *apszLong[] = {
	LPGENW("Sets all Miranda NG protocols to offline and closes Miranda NG."),
	LPGENW("Sets all Miranda NG protocols to offline."),
	LPGENW("Logs the current Windows user off so that another user can log in."),
	LPGENW("Shuts down Windows and then restarts Windows."),
	LPGENW("Closes all running programs and shuts down Windows to a point at which it is safe to turn off the power."),
	LPGENW("Saves the current Windows session in memory and sets the system to suspend mode."),
	LPGENW("Saves the current Windows session on hard drive, so that the power can be turned off."),
	LPGENW("Locks the computer. To unlock the computer, you must log in."),
	LPGENW("Sets all protocols to offline and closes all RAS connections.")
};

INT_PTR ServiceGetTypeDescription(WPARAM wParam, LPARAM lParam)
{
	/* shutdownType range check */
	if (!wParam || (BYTE)wParam > SDSDT_MAX) return 0;
	/* select description */
	wchar_t *pszDesc = (wchar_t*)((lParam & GSTDF_LONGDESC) ? apszLong : apszShort)[wParam - 1];
	if (!(lParam & GSTDF_UNTRANSLATED))
		pszDesc = TranslateW(pszDesc);

	/* convert as needed */
	if (lParam & GSTDF_UNICODE)
		return (INT_PTR)pszDesc;
		
	static char szConvBuf[128];
	mir_strncpy(szConvBuf, _T2A(pszDesc), sizeof(szConvBuf));
	return (INT_PTR)szConvBuf;
}

/************************* Misc ***************************************/

void InitShutdownSvc(void)
{
	/* Shutdown Dialog */
	hwndShutdownDlg = nullptr;
	g_plugin.addSound("AutoShutdown_Countdown", LPGENW("Alerts"), LPGENW("Automatic shutdown countdown"));

	/* Events */
	hEventOkToShutdown = CreateHookableEvent(ME_AUTOSHUTDOWN_OKTOSHUTDOWN);
	hEventShutdown = CreateHookableEvent(ME_AUTOSHUTDOWN_SHUTDOWN);

	/* Services */
	CreateServiceFunction(MS_AUTOSHUTDOWN_SHUTDOWN, ServiceShutdown);
	CreateServiceFunction(MS_AUTOSHUTDOWN_ISTYPEENABLED, ServiceIsTypeEnabled);
	CreateServiceFunction(MS_AUTOSHUTDOWN_GETTYPEDESCRIPTION, ServiceGetTypeDescription);
}

void UninitShutdownSvc(void)
{
	/* Shutdown Dialog */
	if (hwndShutdownDlg != nullptr)
		DestroyWindow(hwndShutdownDlg);

	/* Services */
	DestroyHookableEvent(hEventOkToShutdown);
	DestroyHookableEvent(hEventShutdown);
}
