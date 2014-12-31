/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-06 Miranda ICQ/IM project,
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
#include "globals.h"
//#include "../../miranda-IM/core/commonheaders.h"


//int idleCheckPerm = 0;
//int idleMethodPerm = 0;
//int idleTimeFirstPerm = 5; // this is in mins
//int idleTimeSecondPerm = 10; //
//int idleTimeFirstOnPerm = 0;
//int idleTimeSecondOnPerm = 0;
//int idleOnSaverPerm = 0;
//int idleOnLockPerm = 0;
//int idleGLIPerm = 1;
//int idlePrivatePerm = 0;
BOOL (WINAPI * MyGetLastInputInfo)(PLASTINPUTINFO);
unsigned long idleOptsPerm;				// C  G  L S  M P  L S --LONG-- --SHORT-
unsigned long idleOpts;					// C  G  L S  M P  L S --LONG-- --SHORT-
static BOOL idleDialogInInit = FALSE;
 BOOL InitDialogIdlePart(HWND hwndDlg)
{
//	TranslateDialogDefault(hwndDlg);
	CheckDlgButton(hwndDlg, IDC_IDLECHECK,		(idleOpts&IdleBitsCheck)	? BST_CHECKED : BST_UNCHECKED);
	// check/uncheck options
	CheckDlgButton(hwndDlg, IDC_IDLEONWINDOWS,	(idleOpts&IdleBitsMethod)	? BST_UNCHECKED : BST_CHECKED);
	CheckDlgButton(hwndDlg, IDC_IDLEONMIRANDA,	(idleOpts&IdleBitsMethod)	? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_IDLEUSEGLI,		(idleOpts&IdleBitsGLI)		? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SCREENSAVER,	(idleOpts&IdleBitsOnSaver)	? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_LOCKED,	        (idleOpts&IdleBitsOnLock)		? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_IDLESHORT,		(idleOpts&IdleBitsFirstOn)	? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_IDLELONG,		(idleOpts&IdleBitsSecondOn)	? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_IDLEPRIVATE,	(idleOpts&IdleBitsPrivate)	? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_DISBABLEAUTOHERE,	(idleOpts&IdleSuppressAutoHere)	? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_DISABLEIDLEMENU,	(idleOpts&IdleSuppressIdleMenu)	? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_HEREONSTATUSCHANGE,	(idleOpts&IdleHereOnStatusChange)? BST_CHECKED : BST_UNCHECKED);
	// set times
	idleDialogInInit = TRUE;
	SetDlgItemInt(hwndDlg, IDC_IDLE1STTIME, GetBits(idleOpts,IdleBitsTimeFirst), FALSE);
	SetDlgItemInt(hwndDlg, IDC_IDLE2NDTIME, GetBits(idleOpts,IdleBitsTimeSecond), FALSE);
	idleDialogInInit = FALSE;
	// enable options
	SendMessage(hwndDlg, WM_USER+4, (WPARAM)((idleOpts&IdleBitsCheck)!=0), 0);
	return TRUE;
}

// delphi code here http://www.swissdelphicenter.ch/torry/printcode.php?id=2048
 /**BOOL IsWorkstationLocked(void)
{
	BOOL rc=0;
	HDESK hDesk = OpenDesktopA("default", 0, FALSE, DESKTOP_SWITCHDESKTOP);
	if ( hDesk != 0 ) {
		rc = SwitchDesktop(hDesk) == FALSE;
		CloseDesktop(hDesk);
	}
	return rc;
}**/

// ticks every 2 seconds
 int FirstTick;
 unsigned long secondsAfterLastActivity;
 unsigned long lastIdleEventTime = 0;
 int isCurrentlyIdle=0; //0 notidle //1 short //2long
 VOID CALLBACK IdleTimer(HWND hwnd, UINT message, UINT_PTR idEvent, DWORD dwTime)
{
	DWORD dwTick = GetTickCount();
	int isIdle[3] = {0};
	static int isEventFired[3] = {0};
	int j;

	if (FirstTick) {
		FirstTick--;
		if (FirstTick==0){
			int flags = 0;
			starting = 1;
			switch (isCurrentlyIdle)
			{
				default : {flags = 0;/*MessageBox(theDialog,"BeHere","Button Pressed",0);*/ break;}
				case 1: {flags = IDF_SHORT|IDF_ISIDLE; /*MessageBox(theDialog,"BeAway","Button Pressed",0);*/ break;}
				case 2: {flags = IDF_LONG|IDF_ISIDLE; /*MessageBox(theDialog,"BeNA","Button Pressed",0);*/ break;}
			}
			if (isCurrentlyIdle) {
				lastIdleEventTime = db_get_dw(NULL,AA_MODULE,AA_LASTIDLETIME,(DWORD)time(NULL));
				secondsAfterLastActivity = time(NULL) - lastIdleEventTime;

			} else secondsAfterLastActivity = 0;
			flags |= idleOpts&IdleBitsPrivate?IDF_PRIVACY:0;
			NotifyEventHooks( hIdleEvent, 0, flags ); //Tell everyone we are here
			return;
		}
	}

	for (j = 0 ; j < protoCount ; j++) {
		if ( !isInterestingProto(j) ) continue;
		if (isWaitingForRestoreStatusMessage[j]) {
			isWaitingForRestoreStatusMessage[j]--;
			if (!(isWaitingForRestoreStatusMessage[j])) {
#ifdef UNICODE
				TCHAR *awayMsg=0;
				if (HasAwayMessageW){
					awayMsg = (TCHAR *) CallService(MS_AWAYMSG_GETSTATUSMSGW, (WPARAM) courAwayStatus[j], 0);
				} else {
					char *temp=(char *)CallService(MS_AWAYMSG_GETSTATUSMSG, (WPARAM) courAwayStatus[j], 0);
					if ((int)temp == CALLSERVICE_NOTFOUND) awayMsg = 0;
					else{
						awayMsg = (TCHAR *)mir_alloc((maxMessageLength+1)*sizeof(TCHAR));
						my_a2u(temp,awayMsg,maxMessageLength);
						mir_free(temp);
					}
				}
#else
				char * awayMsg = (char *) CallService(MS_AWAYMSG_GETSTATUSMSG, (WPARAM) courAwayStatus[j], 0);
				if ((int)awayMsg==CALLSERVICE_NOTFOUND) awayMsg = NULL;
#endif

#ifdef AALOG
				{
				char str[1000];
				sprintf(str,"%s: Restoring the default \"StatusMessage\": %s",accounts[j]->szModuleName,awayMsg ? awayMsg : "none");
				CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
				}
#endif
#ifdef UNICODE
					if (protoHasAwayMessageW[j]) CallProtoService(accounts[j]->szModuleName, PS_SETAWAYMSGW, courAwayStatus[j], (LPARAM) awayMsg);
					else {
						char temp[maxMessageLength+1];
						my_u2a(awayMsg,temp,maxMessageLength);
						CallProtoService(accounts[j]->szModuleName, PS_SETAWAYMSG, courAwayStatus[j], (LPARAM) temp);
					}
#else
					CallProtoService(accounts[j]->szModuleName, PS_SETAWAYMSG, courAwayStatus[j], (LPARAM) awayMsg);
#endif
					if (awayMsg)  {
#ifdef AALOG
						char str[1000];
						sprintf(str,"%s: Cleaning after MS_AWAYMSG_GETSTATUSMSG: %s",accounts[j]->szModuleName,awayMsg);
						CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
#endif
						mir_free(awayMsg);
					}

			}
		}
	}

	TimerProc(); //for the reconnect
	secondsAfterLastActivity = 0;
	isCurrentlyIdle = 0;
	if ( (idleOptsPerm&IdleBitsCheck) == 0 ) return;

	if ( (idleOptsPerm&IdleBitsMethod) == 0 ) {
		// use windows idle time
		if ( GetBits(idleOptsPerm,IdleBitsGLI) && (MyGetLastInputInfo != 0) ) {
			LASTINPUTINFO ii;
			memset(&ii,0,sizeof(ii));
			ii.cbSize=sizeof(ii);
			if ( MyGetLastInputInfo(&ii) ) 	dwTick = ii.dwTime;
		} else {
			// mouse check
			static int mouseIdle = 0;
			static POINT lastMousePos = {0};
			POINT pt;
			GetCursorPos(&pt);
			if ( pt.x != lastMousePos.x || pt.y != lastMousePos.y )
			{
				mouseIdle=0;
				lastMousePos=pt;
			}
			else mouseIdle += 2; // interval of timer
			if ( mouseIdle ) dwTick = GetTickCount() - (mouseIdle * 1000);
		}
	} else {
		// use miranda idle time
		CallService(MS_SYSTEM_GETIDLE, 0, (LPARAM)&dwTick);
	}
	secondsAfterLastActivity = (GetTickCount() - dwTick)/1000;
#ifdef AALOG
	{	char log[1024];
		sprintf(log,"%d seconds after last activity",secondsAfterLastActivity);
		CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);}
#endif
	// has the first idle elapsed? If not permited FirstIdle time:ignore
	isIdle[0] = (idleOptsPerm&IdleBitsFirstOn)? (secondsAfterLastActivity >= (DWORD)( GetBits(idleOptsPerm,IdleBitsTimeFirst) * 60 )):0;
	// and the second? if not permited the SecondIdleTime: idnore
	isIdle[1] = (idleOptsPerm&IdleBitsSecondOn)?(secondsAfterLastActivity >= (DWORD)( GetBits(idleOptsPerm,IdleBitsTimeSecond) * 60 )):0;

	isIdle[2] = FALSE;
	if ( idleOptsPerm&IdleBitsOnSaver ) { // check saver
		BOOL isScreenSaverRunning = FALSE;
		SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &isScreenSaverRunning, FALSE);
		isIdle[2] |= isScreenSaverRunning;
	}

	// check workstation?
	if ( idleOptsPerm&IdleBitsOnLock ) { // check station locked?
		isIdle[2] |= IsWorkstationLocked();
	}
	{	//check if the new idle is more idle than the previous idle is idle ;)
		int temp = isIdle[1]?2:((isIdle[0] || isIdle[2])?1:0);
		if (temp>=isCurrentlyIdle) isCurrentlyIdle = temp;
		else return; //if we are alredy idle then exit
	}
	for ( j = 0; j<3; j++ )
	{
		int flags = ( idleOptsPerm&IdleBitsPrivate ? IDF_PRIVACY:0 );
		switch (j) {
			case 0: flags |= IDF_SHORT; break;
			case 1: flags |= IDF_LONG; break;
			case 2: flags |= IDF_ONFORCE; break;
		}
		if ( isIdle[j]==1  && isEventFired[j] == 0 ) { // idle and no one knows
			isEventFired[j]=1;
			NotifyEventHooks( hIdleEvent, 0, IDF_ISIDLE | flags );
		}
		if ( isIdle[j]==0 && isEventFired[j] == 1 ) { // not idle, no one knows
			isEventFired[j]=0;
			if ( !(idleOptsPerm&IdleSuppressAutoHere) ) NotifyEventHooks( hIdleEvent, 0, flags );
		}
	}//for
//	{
//		int flags = ( idleOptsPerm&IdleBitsPrivate ? IDF_PRIVACY:0 );
//		if (isIdle[0]||isIdle[1]||isIdle[2]) flags |= IDF_ISIDLE;
//		if (isIdle[0]) flags |= IDF_SHORT;
//		if (isIdle[1]) flags |= IDF_LONG;
//		if (isIdle[2]) flags |= IDF_ONFORCE;
//		NotifyEventHooks( hIdleEvent, 0, flags );
//	}
}

 INT_PTR IdleGetInfo(WPARAM wParam, LPARAM lParam)
{
	MIRANDA_IDLE_INFO * mii = (MIRANDA_IDLE_INFO *) lParam;
	if (mii) {
		int minutesAfterLastActivity = (time(NULL)-lastIdleEventTime)/60;
#ifdef AALOG
		char log[1024];
		sprintf(log,"\"%s\": %d came.",
			MS_IDLE_GETIDLEINFO,
			mii->cbSize/sizeof(mii->cbSize));
		CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
#endif
		if (minutesAfterLastActivity<=0) minutesAfterLastActivity = 1;
		//this is the only field with similar meaning in "old" and "new miranda";
		if (mii->cbSize/sizeof(mii->cbSize)>=2) mii->idleTime = isCurrentlyIdle?minutesAfterLastActivity:0;
		if (mii->cbSize/sizeof(mii->cbSize)>=3) mii->privacy = (idleOptsPerm&IdleBitsPrivate) !=0;
		if (mii->cbSize/sizeof(mii->cbSize)>=4) mii->aaStatus = isCurrentlyIdle==1?ID_STATUS_AWAY:(isCurrentlyIdle>1?ID_STATUS_NA:ID_STATUS_ONLINE); //dont know what to with that
		if (mii->cbSize/sizeof(mii->cbSize)>=5) mii->aaLock = (idleOptsPerm&IdleSuppressAutoHere) !=0;
		if (mii->cbSize/sizeof(mii->cbSize)>=6) mii->idleType = isCurrentlyIdle?1:0;
		return 0;
	} else return 1;
}

INT_PTR CALLBACK DlgProcOptsIdle(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
        case WM_INITDIALOG:
            TranslateDialogDefault(hwndDlg);
			InitDialogIdlePart(hwndDlg);
            return TRUE;
		case WM_USER+4:
		{
			DWORD nID[] = { IDC_IDLEONWINDOWS,
							IDC_IDLEUSEGLI,
							IDC_IDLEONMIRANDA,
							IDC_SCREENSAVER,
							IDC_LOCKED,
							IDC_IDLESHORT,
							IDC_IDLE1STTIME,
							IDC_IDLELONG,
							IDC_IDLE2NDTIME//,IDC_IDLEPRIVATE
							};
			int j;
			// enable/disable all sub controls,
			for (j = 0; j < sizeof(nID) / sizeof(nID[0]); j++) {
				int nEnable = wParam;
				switch ( nID[j] ) {
					case IDC_IDLEUSEGLI: nEnable &= IsDlgButtonChecked(hwndDlg, IDC_IDLEONWINDOWS) == BST_CHECKED ? 1 : 0; break;
					case IDC_IDLE1STTIME: nEnable &= IsDlgButtonChecked(hwndDlg, IDC_IDLESHORT) == BST_CHECKED ? 1 : 0; break;
					case IDC_IDLE2NDTIME: nEnable &= IsDlgButtonChecked(hwndDlg, IDC_IDLELONG) == BST_CHECKED ? 1 : 0; break;
				}
				EnableWindow(GetDlgItem(hwndDlg, nID[j]), nEnable);
			}

			idleOpts=SetBits(idleOpts,IdleBitsCheck,  IsDlgButtonChecked(hwndDlg, IDC_IDLECHECK) == BST_CHECKED);
			idleOpts=SetBits(idleOpts,IdleBitsMethod, IsDlgButtonChecked(hwndDlg, IDC_IDLEONMIRANDA) == BST_CHECKED);
			idleOpts=SetBits(idleOpts,IdleBitsFirstOn, IsDlgButtonChecked(hwndDlg, IDC_IDLESHORT) == BST_CHECKED);
			idleOpts=SetBits(idleOpts,IdleBitsSecondOn, IsDlgButtonChecked(hwndDlg, IDC_IDLELONG) == BST_CHECKED);
			idleOpts=SetBits(idleOpts,IdleBitsGLI, IsDlgButtonChecked(hwndDlg, IDC_IDLEUSEGLI) == BST_CHECKED);
			idleOpts=SetBits(idleOpts,IdleBitsOnSaver, IsDlgButtonChecked(hwndDlg, IDC_SCREENSAVER) == BST_CHECKED);
			idleOpts=SetBits(idleOpts,IdleBitsOnLock, IsDlgButtonChecked(hwndDlg, IDC_LOCKED) == BST_CHECKED);
			idleOpts=SetBits(idleOpts,IdleBitsPrivate, IsDlgButtonChecked(hwndDlg, IDC_IDLEPRIVATE) == BST_CHECKED);
			idleOpts=SetBits(idleOpts,IdleSuppressAutoHere, IsDlgButtonChecked(hwndDlg, IDC_DISBABLEAUTOHERE) == BST_CHECKED);
			idleOpts=SetBits(idleOpts,IdleSuppressIdleMenu, IsDlgButtonChecked(hwndDlg, IDC_DISABLEIDLEMENU) == BST_CHECKED);
			idleOpts=SetBits(idleOpts,IdleHereOnStatusChange, IsDlgButtonChecked(hwndDlg, IDC_HEREONSTATUSCHANGE) == BST_CHECKED);
			break;
		}
		case WM_USER+8:
		{
			if (wParam == IDC_IDLE1STTIME) idleOpts = SetBits(idleOpts,IdleBitsTimeFirst,GetDlgItemInt(hwndDlg, wParam, NULL, FALSE));
			if (wParam == IDC_IDLE2NDTIME) idleOpts = SetBits(idleOpts,IdleBitsTimeSecond,GetDlgItemInt(hwndDlg, wParam, NULL, FALSE));
			break;
		}
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
				case IDC_IDLEBUTTON:
				case IDC_IDLEBUTTON+1:
				case IDC_IDLEBUTTON+2:SimulateIdle(LOWORD(wParam)-IDC_IDLEBUTTON); break;
				case IDC_IDLECHECK:
				case IDC_IDLEONWINDOWS:
				case IDC_IDLEONMIRANDA:
				case IDC_IDLESHORT:
				case IDC_IDLELONG:
				case IDC_IDLEUSEGLI:
				case IDC_SCREENSAVER:
				case IDC_LOCKED:
				case IDC_IDLEPRIVATE:
				case IDC_DISBABLEAUTOHERE:
				case IDC_HEREONSTATUSCHANGE:
				case IDC_DISABLEIDLEMENU:
				{
					SendMessage(hwndDlg, WM_USER+4, (WPARAM)IsDlgButtonChecked(hwndDlg, IDC_IDLECHECK) == BST_CHECKED, 0);
					break;
				}
				case IDC_IDLE1STTIME:
				case IDC_IDLE2NDTIME:
				{
					if ( HIWORD(wParam) != EN_CHANGE ) return TRUE;
					SendMessage(hwndDlg, WM_USER+8, LOWORD(wParam), 0);
					break;
				}
            }
            if (!idleDialogInInit) if ((HIWORD(wParam)==BN_CLICKED)||(HIWORD(wParam)==EN_CHANGE))
			{
				if ((LOWORD(wParam)!=IDC_IDLEBUTTON)&&(LOWORD(wParam)!=IDC_IDLEBUTTON+1)&&(LOWORD(wParam)!=IDC_IDLEBUTTON+2))
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
            break;
        case WM_NOTIFY:
            switch (((LPNMHDR) lParam)->idFrom) {
                case 0:
                    switch (((LPNMHDR) lParam)->code) {
                        case PSN_APPLY: {
                            return TRUE;
                        }
                    }
                    break;
            }
            break;
        case WM_DESTROY:
            break;
    }
    return FALSE;
}
