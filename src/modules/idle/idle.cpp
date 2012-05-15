/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2005 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"

#define IDLEMOD "Idle"
#define IDL_USERIDLECHECK "UserIdleCheck"
#define IDL_IDLEMETHOD    "IdleMethod"
#define IDL_IDLETIME1ST   "IdleTime1st"
#define IDL_IDLEONSAVER   "IdleOnSaver" // IDC_SCREENSAVER
#define IDL_IDLEONFULLSCR "IdleOnFullScr" // IDC_FULLSCREEN
#define IDL_IDLEONLOCK    "IdleOnLock" // IDC_LOCKED
#define IDL_IDLEONTSDC    "IdleOnTerminalDisconnect" //
#define IDL_IDLEPRIVATE   "IdlePrivate" // IDC_IDLEPRIVATE
#define IDL_IDLESTATUSLOCK "IdleStatusLock" // IDC_IDLESTATUSLOCK
#define IDL_AAENABLE      "AAEnable"
#define IDL_AASTATUS      "AAStatus"

#define IdleObject_IsIdle(obj) (obj->state&0x1)
#define IdleObject_SetIdle(obj) (obj->state|=0x1)
#define IdleObject_ClearIdle(obj) (obj->state&=~0x1)

// either use meth 0,1 or figure out which one
#define IdleObject_UseMethod0(obj) (obj->state&=~0x2)
#define IdleObject_UseMethod1(obj) (obj->state|=0x2)
#define IdleObject_GetMethod(obj) (obj->state&0x2)

#define IdleObject_IdleCheckSaver(obj) (obj->state&0x4)
#define IdleObject_SetSaverCheck(obj) (obj->state|=0x4)

#define IdleObject_IdleCheckWorkstation(obj) (obj->state&0x8)
#define IdleObject_SetWorkstationCheck(obj) (obj->state|=0x8)

#define IdleObject_IsPrivacy(obj) (obj->state&0x10)
#define IdleObject_SetPrivacy(obj) (obj->state|=0x10)

#define IdleObject_SetStatusLock(obj) (obj->state|=0x20)

#define IdleObject_IdleCheckTerminal(obj) (obj->state&0x40)
#define IdleObject_SetTerminalCheck(obj) (obj->state|=0x40)

#define IdleObject_IdleCheckFullScr(obj) (obj->state&0x80)
#define IdleObject_SetFullScrCheck(obj) (obj->state|=0x80)

//#include <Wtsapi32.h>

#ifndef _INC_WTSAPI

#define WTS_CURRENT_SERVER_HANDLE  ((HANDLE)NULL)
#define WTS_CURRENT_SESSION ((DWORD)-1)

typedef enum _WTS_CONNECTSTATE_CLASS {
    WTSActive,              // User logged on to WinStation
    WTSConnected,           // WinStation connected to client
    WTSConnectQuery,        // In the process of connecting to client
    WTSShadow,              // Shadowing another WinStation
    WTSDisconnected,        // WinStation logged on without client
    WTSIdle,                // Waiting for client to connect
    WTSListen,              // WinStation is listening for connection
    WTSReset,               // WinStation is being reset
    WTSDown,                // WinStation is down due to error
    WTSInit,                // WinStation in initialization
} WTS_CONNECTSTATE_CLASS;


typedef enum _WTS_INFO_CLASS {
    WTSInitialProgram,
    WTSApplicationName,
    WTSWorkingDirectory,
    WTSOEMId,
    WTSSessionId,
    WTSUserName,
    WTSWinStationName,
    WTSDomainName,
    WTSConnectState,
    WTSClientBuildNumber,
    WTSClientName,
    WTSClientDirectory,
    WTSClientProductId,
    WTSClientHardwareId,
    WTSClientAddress,
    WTSClientDisplay,
    WTSClientProtocolType,
} WTS_INFO_CLASS;

#endif

VOID (WINAPI *_WTSFreeMemory)(PVOID);
BOOL (WINAPI *_WTSQuerySessionInformation)(HANDLE, DWORD, WTS_INFO_CLASS, PVOID, DWORD*);

BOOL bIsWTSApiPresent = FALSE;

static BOOL bModuleInitialized = FALSE;

BOOL InitWTSAPI()
{
	HMODULE hDll = LoadLibraryA("wtsapi32.dll");
	if (hDll) {
		_WTSFreeMemory = (VOID (WINAPI *)(PVOID))GetProcAddress(hDll, "WTSFreeMemory");
		#ifdef UNICODE
			_WTSQuerySessionInformation = (BOOL (WINAPI *)(HANDLE, DWORD, WTS_INFO_CLASS, PVOID, DWORD*))GetProcAddress(hDll, "WTSQuerySessionInformationW");
		#else
			_WTSQuerySessionInformation = (BOOL (WINAPI *)(HANDLE, DWORD, WTS_INFO_CLASS, PVOID, DWORD*))GetProcAddress(hDll, "WTSQuerySessionInformationA");
		#endif

		if (_WTSFreeMemory && _WTSQuerySessionInformation) return 1;
	}
	return 0;
}

BOOL IsTerminalDisconnected()
{
	PVOID pBuffer = NULL;
	DWORD pBytesReturned = 0;
	BOOL result = FALSE;

	if ( !bIsWTSApiPresent )
		return FALSE;

	if ( _WTSQuerySessionInformation( WTS_CURRENT_SERVER_HANDLE, WTS_CURRENT_SESSION, WTSConnectState, &pBuffer, &pBytesReturned)) {
		if ( *( PDWORD )pBuffer == WTSDisconnected)
			result = TRUE;
	}
	else bIsWTSApiPresent = FALSE;

	if ( pBuffer )
		_WTSFreeMemory( pBuffer );
	return result;
}

typedef struct {
	UINT_PTR hTimer;
	unsigned int useridlecheck;
	unsigned int state;
	unsigned int minutes;	// user setting, number of minutes of inactivity to wait for
	POINT mousepos;
	unsigned int mouseidle;
	int aastatus;
	int idleType;
}
	IdleObject;

static const WORD aa_Status[] = {ID_STATUS_AWAY, ID_STATUS_NA, ID_STATUS_OCCUPIED, ID_STATUS_DND, ID_STATUS_ONTHEPHONE, ID_STATUS_OUTTOLUNCH};

static IdleObject gIdleObject;
static HANDLE hIdleEvent;
static BOOL (WINAPI * MyGetLastInputInfo)(PLASTINPUTINFO);

void CALLBACK IdleTimer(HWND hwnd, UINT umsg, UINT_PTR idEvent, DWORD dwTime);

static void IdleObject_ReadSettings(IdleObject * obj)
{
	obj->useridlecheck = DBGetContactSettingByte(NULL, IDLEMOD, IDL_USERIDLECHECK, 0);
	obj->minutes = DBGetContactSettingByte(NULL, IDLEMOD, IDL_IDLETIME1ST, 10);
	obj->aastatus = !DBGetContactSettingByte(NULL, IDLEMOD, IDL_AAENABLE, 0) ? 0 : DBGetContactSettingWord(NULL, IDLEMOD, IDL_AASTATUS, 0);
	if ( DBGetContactSettingByte(NULL, IDLEMOD, IDL_IDLEMETHOD, 0) ) IdleObject_UseMethod1(obj);
	else IdleObject_UseMethod0(obj);
	if ( DBGetContactSettingByte(NULL, IDLEMOD, IDL_IDLEONSAVER, 0) ) IdleObject_SetSaverCheck(obj);
	if ( DBGetContactSettingByte(NULL, IDLEMOD, IDL_IDLEONFULLSCR, 0) ) IdleObject_SetFullScrCheck(obj);
	if ( DBGetContactSettingByte(NULL, IDLEMOD, IDL_IDLEONLOCK, 0 ) ) IdleObject_SetWorkstationCheck(obj);
	if ( DBGetContactSettingByte(NULL, IDLEMOD, IDL_IDLEPRIVATE, 0) ) IdleObject_SetPrivacy(obj);
	if ( DBGetContactSettingByte(NULL, IDLEMOD, IDL_IDLESTATUSLOCK, 0) ) IdleObject_SetStatusLock(obj);
	if ( DBGetContactSettingByte(NULL, IDLEMOD, IDL_IDLEONTSDC, 0) ) IdleObject_SetTerminalCheck(obj);
}

static void IdleObject_Create(IdleObject * obj)
{
	ZeroMemory(obj, sizeof(IdleObject));
	obj->hTimer=SetTimer(NULL, 0, 2000, IdleTimer);
	IdleObject_ReadSettings(obj);
}

static void IdleObject_Destroy(IdleObject * obj)
{
	if (IdleObject_IsIdle(obj))
		NotifyEventHooks(hIdleEvent, 0, 0);
	IdleObject_ClearIdle(obj);
	KillTimer(NULL, obj->hTimer);
}

static int IdleObject_IsUserIdle(IdleObject * obj)
{
	DWORD dwTick;
	if ( IdleObject_GetMethod(obj) ) {
		CallService(MS_SYSTEM_GETIDLE, 0, (LPARAM)&dwTick);
		return GetTickCount() - dwTick > (obj->minutes * 60 * 1000);
	}
	
	if ( MyGetLastInputInfo != NULL ) {
		LASTINPUTINFO ii;
		ZeroMemory(&ii,sizeof(ii));
		ii.cbSize=sizeof(ii);
		if ( MyGetLastInputInfo(&ii) ) 
			return GetTickCount() - ii.dwTime > (obj->minutes * 60 * 1000);
	}
	else {
		POINT pt;
		GetCursorPos(&pt);
		if ( pt.x != obj->mousepos.x || pt.y != obj->mousepos.y ) {
			obj->mousepos=pt;
			obj->mouseidle=0;
		}
		else obj->mouseidle += 2;

		if ( obj->mouseidle )
			return obj->mouseidle * 1000 >= (obj->minutes * 60 * 1000);
	}
	return FALSE;
}

static bool IsWorkstationLocked (void)
{
	bool rc = false;

	if (openInputDesktop != NULL) {
		HDESK hDesk = openInputDesktop(0, FALSE, DESKTOP_SWITCHDESKTOP);
		if (hDesk == NULL)
			rc = true;
		else if (closeDesktop != NULL)
			closeDesktop(hDesk);
	}
	return rc;
}

static bool IsScreenSaverRunning(void)
{
	BOOL rc = FALSE;
	SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &rc, FALSE);
	return rc != 0;
}

bool IsFullScreen(void) 
{
	RECT rcScreen = {0};

	rcScreen.right = GetSystemMetrics(SM_CXSCREEN);
	rcScreen.bottom = GetSystemMetrics(SM_CYSCREEN);

	if (MyMonitorFromWindow)
	{
		HMONITOR hMon = MyMonitorFromWindow(cli.hwndContactList, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		if (MyGetMonitorInfo(hMon, &mi))
			rcScreen = mi.rcMonitor;
	}

	HWND hWndDesktop = GetDesktopWindow();
	HWND hWndShell = GetShellWindow();

	// check foregroundwindow
	HWND hWnd = GetForegroundWindow();
	if (hWnd && hWnd != hWndDesktop && hWnd != hWndShell) 
	{
		TCHAR tszClassName[128] = _T("");
		GetClassName(hWnd, tszClassName, SIZEOF(tszClassName));
		if (_tcscmp(tszClassName, _T("WorkerW")))
		{
			RECT rect, rectw, recti;
			GetWindowRect(hWnd, &rectw);

			GetClientRect(hWnd, &rect);
			ClientToScreen(hWnd, (LPPOINT)&rect);
			ClientToScreen(hWnd, (LPPOINT)&rect.right);
			
			if (EqualRect(&rect, &rectw) && IntersectRect(&recti, &rect, &rcScreen) && 
				EqualRect(&recti, &rcScreen))
				return true;
		}
	}

	return false;
}

static void IdleObject_Tick(IdleObject * obj)
{
	bool idle = false;
	int  idleType = 0, flags = 0;

	if ( obj->useridlecheck && IdleObject_IsUserIdle(obj)) {
		idleType = 1; idle = true; 
	}
	else if ( IdleObject_IdleCheckSaver(obj) && IsScreenSaverRunning()) {
		idleType = 2; idle = true; 
	}
	else if ( IdleObject_IdleCheckFullScr(obj) && IsFullScreen()) {
		idleType = 5; idle = true; 
	}
	else if ( IdleObject_IdleCheckWorkstation(obj) && IsWorkstationLocked()) {
		idleType = 3; idle = true; 
	}
	else if ( IdleObject_IdleCheckTerminal(obj) && IsTerminalDisconnected()) {
		idleType = 4; idle = true; 
	}

	if ( IdleObject_IsPrivacy(obj))
		flags |= IDF_PRIVACY;

	if ( !IdleObject_IsIdle(obj) && idle ) {
		IdleObject_SetIdle(obj);
		obj->idleType = idleType;
		NotifyEventHooks(hIdleEvent, 0, IDF_ISIDLE | flags);
	}
	if ( IdleObject_IsIdle(obj) && !idle ) {
		IdleObject_ClearIdle(obj);
		obj->idleType = 0;
		NotifyEventHooks(hIdleEvent, 0, flags);
}	}

void CALLBACK IdleTimer(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	if ( gIdleObject.hTimer == idEvent )
		IdleObject_Tick( &gIdleObject );
}

int IdleGetStatusIndex(WORD status)
{
    int j;
    for (j = 0; j < SIZEOF(aa_Status); j++ )
        if ( aa_Status[j] == status )
			  return j;

    return 0;
}

static INT_PTR CALLBACK IdleOptsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg ) {
	case WM_INITDIALOG:
	{
		int j;
		int method = DBGetContactSettingByte(NULL,IDLEMOD,IDL_IDLEMETHOD, 0);
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_IDLESHORT, DBGetContactSettingByte(NULL,IDLEMOD,IDL_USERIDLECHECK,0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_IDLEONWINDOWS, method == 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_IDLEONMIRANDA, method ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SCREENSAVER, DBGetContactSettingByte(NULL,IDLEMOD,IDL_IDLEONSAVER,0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_FULLSCREEN, DBGetContactSettingByte(NULL,IDLEMOD,IDL_IDLEONFULLSCR,0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOCKED, DBGetContactSettingByte(NULL,IDLEMOD,IDL_IDLEONLOCK,0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_IDLEPRIVATE, DBGetContactSettingByte(NULL,IDLEMOD,IDL_IDLEPRIVATE,0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_IDLESTATUSLOCK, DBGetContactSettingByte(NULL,IDLEMOD,IDL_IDLESTATUSLOCK,0) ? BST_CHECKED : BST_UNCHECKED);
		if ( !bIsWTSApiPresent )
			EnableWindow( GetDlgItem( hwndDlg, IDC_IDLETERMINAL ), FALSE );
		else
			CheckDlgButton(hwndDlg, IDC_IDLETERMINAL, DBGetContactSettingByte(NULL,IDLEMOD,IDL_IDLEONTSDC,0) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_IDLESPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_IDLE1STTIME), 0);
		SendDlgItemMessage(hwndDlg, IDC_IDLESPIN, UDM_SETRANGE32, 1, 60);
		SendDlgItemMessage(hwndDlg, IDC_IDLESPIN, UDM_SETPOS, 0, MAKELONG((short) DBGetContactSettingByte(NULL,IDLEMOD,IDL_IDLETIME1ST, 10), 0));
		SendDlgItemMessage(hwndDlg, IDC_IDLE1STTIME, EM_LIMITTEXT, (WPARAM)2, 0);

      CheckDlgButton(hwndDlg, IDC_AASHORTIDLE, DBGetContactSettingByte(NULL, IDLEMOD, IDL_AAENABLE, 0) ? BST_CHECKED:BST_UNCHECKED);
		for ( j = 0; j < SIZEOF(aa_Status); j++ )
			SendDlgItemMessage(hwndDlg, IDC_AASTATUS, CB_ADDSTRING, 0, ( LPARAM )cli.pfnGetStatusModeDescription( aa_Status[j], 0 ));

		j = IdleGetStatusIndex((WORD)(DBGetContactSettingWord(NULL, IDLEMOD, IDL_AASTATUS, 0)));
		SendDlgItemMessage(hwndDlg, IDC_AASTATUS, CB_SETCURSEL, j, 0);
		SendMessage(hwndDlg, WM_USER+2, 0, 0);
		return TRUE;
	}
	case WM_USER+2:
	{
		BOOL checked = IsDlgButtonChecked(hwndDlg, IDC_IDLESHORT) == BST_CHECKED;
		EnableWindow(GetDlgItem(hwndDlg, IDC_IDLEONWINDOWS), checked);
		EnableWindow(GetDlgItem(hwndDlg, IDC_IDLEONMIRANDA), checked);
		EnableWindow(GetDlgItem(hwndDlg, IDC_IDLE1STTIME), checked);
		EnableWindow(GetDlgItem(hwndDlg, IDC_AASTATUS), IsDlgButtonChecked(hwndDlg, IDC_AASHORTIDLE)==BST_CHECKED?1:0);
		EnableWindow(GetDlgItem(hwndDlg, IDC_IDLESTATUSLOCK), IsDlgButtonChecked(hwndDlg, IDC_AASHORTIDLE)==BST_CHECKED?1:0);
		break;
	}
	case WM_NOTIFY:
	{
		NMHDR * hdr = (NMHDR *)lParam;
		if ( hdr && hdr->code == PSN_APPLY ) {
			int method = IsDlgButtonChecked(hwndDlg, IDC_IDLEONWINDOWS) == BST_CHECKED;
			int mins = SendDlgItemMessage(hwndDlg, IDC_IDLESPIN, UDM_GETPOS, 0, 0);
			DBWriteContactSettingByte(NULL, IDLEMOD, IDL_IDLETIME1ST, (BYTE)(HIWORD(mins) == 0 ? LOWORD(mins) : 10));
			DBWriteContactSettingByte(NULL, IDLEMOD, IDL_USERIDLECHECK, (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_IDLESHORT) == BST_CHECKED));
			DBWriteContactSettingByte(NULL, IDLEMOD, IDL_IDLEMETHOD, (BYTE)(method ? 0 : 1));
			DBWriteContactSettingByte(NULL, IDLEMOD, IDL_IDLEONSAVER, (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_SCREENSAVER) == BST_CHECKED));
			DBWriteContactSettingByte(NULL, IDLEMOD, IDL_IDLEONFULLSCR, (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_FULLSCREEN) == BST_CHECKED));
			DBWriteContactSettingByte(NULL, IDLEMOD, IDL_IDLEONLOCK, (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_LOCKED) == BST_CHECKED));
			DBWriteContactSettingByte(NULL, IDLEMOD, IDL_IDLEONTSDC, (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_IDLETERMINAL) == BST_CHECKED));
			DBWriteContactSettingByte(NULL, IDLEMOD, IDL_IDLEPRIVATE, (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_IDLEPRIVATE) == BST_CHECKED));
			DBWriteContactSettingByte(NULL, IDLEMOD, IDL_AAENABLE, (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_AASHORTIDLE)==BST_CHECKED?1:0));
			DBWriteContactSettingByte(NULL, IDLEMOD, IDL_IDLESTATUSLOCK, (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_IDLESTATUSLOCK)==BST_CHECKED?1:0));
			{
				int curSel = SendDlgItemMessage(hwndDlg, IDC_AASTATUS, CB_GETCURSEL, 0, 0);
				if ( curSel != CB_ERR ) {
					DBWriteContactSettingWord(NULL, IDLEMOD, IDL_AASTATUS, (WORD)(aa_Status[curSel]));
				}
			}
			// destroy any current idle and reset settings.
			IdleObject_Destroy(&gIdleObject);
			IdleObject_Create(&gIdleObject);
		}
		break;
	}
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDC_IDLE1STTIME:
		{
			int min;
			if ( (HWND)lParam != GetFocus() || HIWORD(wParam) != EN_CHANGE ) return FALSE;
			min=GetDlgItemInt(hwndDlg, IDC_IDLE1STTIME, NULL, FALSE);
			if ( min == 0 && GetWindowTextLength(GetDlgItem(hwndDlg, IDC_IDLE1STTIME)) )
				SendDlgItemMessage(hwndDlg, IDC_IDLESPIN, UDM_SETPOS, 0, MAKELONG((short) 1, 0));
			break;
		}
		case IDC_IDLESHORT:
		case IDC_AASHORTIDLE:
			SendMessage(hwndDlg, WM_USER+2, 0, 0);
			break;

		case IDC_AASTATUS:
			if ( HIWORD(wParam) != CBN_SELCHANGE )
				return TRUE;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	}
	return FALSE;
}

static int IdleOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof(odp);
	odp.position = 100000000;
	odp.hInstance = hMirandaInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_IDLE);
	odp.pszGroup = LPGEN("Status");
	odp.pszTitle = LPGEN("Idle");
	odp.pfnDlgProc = IdleOptsDlgProc;
	odp.flags = ODPF_BOLDGROUPS;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );
	return 0;
}

static INT_PTR IdleGetInfo(WPARAM, LPARAM lParam)
{
	MIRANDA_IDLE_INFO *mii = ( MIRANDA_IDLE_INFO* )lParam;
	if ( !mii || ( mii->cbSize != sizeof(MIRANDA_IDLE_INFO) && mii->cbSize != MIRANDA_IDLE_INFO_SIZE_1 ))
		return 1;

	mii->idleTime = gIdleObject.minutes;
	mii->privacy = gIdleObject.state&0x10;
	mii->aaStatus = gIdleObject.aastatus;
	mii->aaLock = gIdleObject.state&0x20;

	if ( mii->cbSize == sizeof(MIRANDA_IDLE_INFO))
		mii->idleType = gIdleObject.idleType;
	return 0;
}

static int IdleModernOptInit(WPARAM wParam, LPARAM)
{
	static const int iBoldControls[] =
	{
		IDC_TXT_TITLE1, IDC_TXT_TITLE2, IDC_TXT_TITLE3,
		MODERNOPT_CTRL_LAST
	};

	MODERNOPTOBJECT obj = {0};
	obj.cbSize = sizeof(obj);
	obj.hInstance = hMirandaInst;
	obj.dwFlags = MODEROPT_FLG_TCHAR | MODEROPT_FLG_NORESIZE;
	obj.iSection = MODERNOPT_PAGE_STATUS;
	obj.iType = MODERNOPT_TYPE_SECTIONPAGE;
	obj.iBoldControls = (int*)iBoldControls;
	obj.lpzTemplate = MAKEINTRESOURCEA(IDD_MODERNOPT_IDLE);
	obj.pfnDlgProc = IdleOptsDlgProc;
//	obj.lpzClassicGroup = "Status";
//	obj.lpzClassicPage = "Messages";
	obj.lpzHelpUrl = "http://wiki.miranda-im.org/";
	CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
	return 0;
}

int LoadIdleModule(void)
{
	bModuleInitialized = TRUE;

	bIsWTSApiPresent = InitWTSAPI();
	MyGetLastInputInfo=(BOOL (WINAPI *)(LASTINPUTINFO*))GetProcAddress(GetModuleHandleA("user32"), "GetLastInputInfo");
	hIdleEvent=CreateHookableEvent(ME_IDLE_CHANGED);
	IdleObject_Create(&gIdleObject);
	CreateServiceFunction(MS_IDLE_GETIDLEINFO, IdleGetInfo);
	HookEvent(ME_OPT_INITIALISE, IdleOptInit);
	HookEvent(ME_MODERNOPT_INITIALIZE, IdleModernOptInit);
	return 0;
}

void UnloadIdleModule()
{
	if ( !bModuleInitialized ) return;

	IdleObject_Destroy(&gIdleObject);
	DestroyHookableEvent(hIdleEvent);
	hIdleEvent=NULL;
}
