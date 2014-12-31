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

long *reconnectOptsPerm=0;
long *reconnectOpts=0;

unsigned int *protoOfflineTimes=0;
int *protoStatus=0; //0:OK; :1:couning; 2:fatalError; 4:reconnectForced


void LoginAck(int protoIndex, ACKDATA * ack)
{
#ifdef AALOG
 	char log[1024];
	sprintf(log,"LoginAck: %s: result: %d; hProcess: %d; lParam: %d;" ,
		accounts[protoIndex]->szModuleName,
		ack->result,ack->hProcess,ack->lParam
//		CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)ack->hProcess, 0),
//		CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)ack->lParam, 0)
	);
	CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
#endif
	if (ack->result == ACKRESULT_FAILED) switch (ack->lParam){
		case LOGINERR_WRONGPASSWORD:
		case LOGINERR_BADUSERID:
		case LOGINERR_WRONGPROTOCOL:
		case LOGINERR_OTHERLOCATION:
			{
#ifdef AALOG
				CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)"LoginAck: FatalError occured");
#endif
				protoStatus[protoIndex]|=2;
				protoOfflineTimes[protoIndex]=0;
				return;
				break;
			}
	}
}


void ProtocolsProc(int protoIndex, ACKDATA * ack)
//ProtoProc: ICQ-AUX: result: 0; hProcess: Online; lParam: Offline;
{
#ifdef AALOG
	char log[1024];
	sprintf(log,"ProtoProc: %s: result: %d; hProcess: %s; lParam: %s; RequestedStatus: %s; AutoAwayStatus: %s" ,
		accounts[protoIndex]->szModuleName,
		ack->result,
		CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)ack->hProcess, 0),
		CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)ack->lParam, 0),
		CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)reqStatus[protoIndex], 0),
		CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)courAwayStatus[protoIndex], 0)
	);
	CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
#endif
	protoOfflineTimes[protoIndex]=0;
	if (ack->lParam==ID_STATUS_OFFLINE)	protoStatus[protoIndex]|=1; //will be counting, keeping eventual "FatalError" Flag
	else protoStatus[protoIndex] = 0; //We are not offline or are connecting -> clear counting and "FatalError"
#ifdef AALOG
	sprintf(log,"ProtoProc: %s: Result: ProtoStatus: %d",
		accounts[protoIndex]->szModuleName,protoStatus[protoIndex]);
	CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
#endif

}

#ifdef AALOG
#define AALOG2
#endif
int IsReconnectForbidden(int protoIndex){ //0 if it permitted
#ifdef AALOG2
	char str[256];
#endif
	if (!(protoStatus[protoIndex]&1)) return 1; //Proto is not counting or is not offline
	protoOfflineTimes[protoIndex] += 2; //the timer ticks every 2 seconds
	if (protoStatus[protoIndex]&4) { // Reconnect is forced
#ifdef AALOG2
		sprintf(str,"ReconnForbiddden: %s reconnection is forced.",accounts[protoIndex]->szModuleName);
		CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
#endif
		return 0;
	}
#ifdef AALOG2
	sprintf(str,"ReconnForbiddden: %s is Offline and counting: %d to %d",accounts[protoIndex]->szModuleName,protoOfflineTimes[protoIndex],HIWORD(reconnectOptsPerm[protoIndex]));
	CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
#endif
	if (protoOfflineTimes[protoIndex] <= HIWORD(reconnectOptsPerm[protoIndex])) return 2; // not yet
	if (!(reconnectOptsPerm[protoIndex]&aa_ReconnectBitsEnabled)) return 3; //reconnect is disabled
#ifdef AALOG2
	sprintf(str,"ReconnForbiddden: %s has ReconnectEnabled",accounts[protoIndex]->szModuleName);
	CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
#endif
	if (protoStatus[protoIndex]&2) { //FatalError?
#ifdef AALOG2
		sprintf(str,"ReconnForbiddden: %s has FatalError; ProtoStatus: %d",accounts[protoIndex]->szModuleName,protoStatus[protoIndex]);
		CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
#endif
		if (reconnectOptsPerm[protoIndex]&aa_ReconnectNotFatalError) {
#ifdef AALOG2
			sprintf(str,"ReconnForbiddden: %s has DontReconnectOnFatalError",accounts[protoIndex]->szModuleName);
			CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
#endif
			return 4; //exit if "dontReconnectOnFatalError" enabled
#ifdef AALOG2
		} else {
			sprintf(str,"ReconnForbiddden: %s has FatalError but DontReconnectOnFatalError disabled",accounts[protoIndex]->szModuleName);
			CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
#endif
		}
	} //if FatalError
	if (IsStatusBitSet(courAwayStatus[protoIndex],LOWORD(reconnectOptsPerm[protoIndex]))){ //is reconnect permited on this lastonline status?
		//reconnecting...
#ifdef AALOG2
		sprintf(str,"ReconnForbiddden: %s Reconnect permitted",accounts[protoIndex]->szModuleName);
		CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
#endif
		return 0; // reconnect is permitted
#ifdef AALOG2
	} else {
		sprintf(str,"ReconnForbiddden: %s Reconnecting to %s(%d) was not permited by %d",
		accounts[protoIndex]->szModuleName,
		CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)courAwayStatus[protoIndex], 0),
		courAwayStatus[protoIndex],
		reconnectOptsPerm[protoIndex]);
		CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
#endif
	}
	return 5; // forbidden because current status is disabled
}
#undef AALOG2
void TimerProc()
{
#ifdef AALOG
	char str[256];
#endif
	int i;
	for (i=0;i<protoCount;i++){
		if (isInterestingProto(i)){
#ifdef AALOG1
			sprintf(str,"TimerProc: %s is a proto",accounts[i]->szModuleName);
			CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
#endif
			if (protoStatus[i]&1) {
#ifdef AALOG1
					sprintf(str,"TimerProc: %s is Counting: AwayStatus: %s; reqStatus: %s;",
						accounts[i]->szModuleName,
						CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)courAwayStatus[i], 0),
						CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)reqStatus[i], 0)
					);
					CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
#endif
				if ((courAwayStatus[i]==ID_STATUS_OFFLINE)||(reqStatus[i]==ID_STATUS_OFFLINE)){
					protoStatus[i]&=2; //keeping fatalerror if any
#ifdef AALOG
					sprintf(str,"TimerProc: %s Counting Cancelled: AwayStatus: %s; reqStatus: %s;",
						accounts[i]->szModuleName,
						CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)courAwayStatus[i], 0),
						CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)reqStatus[i], 0)
					);
					CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
#endif
				}
			}
			switch (IsReconnectForbidden(i)) {
#ifdef AALOG1
			case 1 : sprintf(str,"IsReconnectForbidden: %s is not counting or is not offline",accounts[i]->szModuleName); break;
			case 2 : sprintf(str,"IsReconnectForbidden: %s it is too early to reconnect",accounts[i]->szModuleName); break;
			case 3 : sprintf(str,"IsReconnectForbidden: %s has reconnect disabled",accounts[i]->szModuleName); break;
			case 4 : sprintf(str,"IsReconnectForbidden: %s fatal error and DontReconnect enabled",accounts[i]->szModuleName); break;
			case 5 : sprintf(str,"IsReconnectForbidden: %s current AAStatus should not be reconnected",accounts[i]->szModuleName); break;
#endif
			case 0 :
#ifdef AALOG
				sprintf(str,"IsReconnectForbidden: %s is permited to reconnect",accounts[i]->szModuleName);
#endif
				CallProtoService(accounts[i]->szModuleName, PS_SETSTATUS,  courAwayStatus[i], 0);
				break;
			default :
#ifdef AALOG
				sprintf(str,"IsReconnectForbidden: %s Surprising answer",accounts[i]->szModuleName);
#endif
				break;
			}
#ifdef AALOG1
			CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
#endif
		}//protoType protocol
	}//for
}
void moveScrollBarEvent(HWND hwndDlg){
	TCHAR str[256];
	int slide = SendDlgItemMessage(hwndDlg,IDC_RECONNECTSLIDER,TBM_GETPOS,0,0);
	int delay = GetReconnectDelayFromSlider(slide);
	ShowWindow(GetDlgItem(hwndDlg, IDC_RECONNECTWARNING),GetStringFromDelay(str, delay)?SW_SHOW:SW_HIDE);
	SetDlgItemText(hwndDlg,IDC_ENABLERECONNECT,str);
	reconnectOpts[courProtocolSelection] = SetBits(reconnectOpts[courProtocolSelection],aa_ReconnectBitsDelay,delay);
}

LRESULT SetDlgItemsReconnect(HWND hwndDlg, int coursel){
		BOOL enabledReconnect = GetBits(reconnectOpts[coursel],aa_ReconnectBitsEnabled);
		BOOL dontReconnectOnFatalError = GetBits(reconnectOpts[coursel],aa_ReconnectNotFatalError);
		int i =0;

		for (i=0;i<numStatuses;i++){
				BOOL bita = IsStatusBitSet(aa_Status[i],protoModes[coursel])==0;
				BOOL bitr = IsStatusBitSet(aa_Status[i],reconnectOpts[coursel])>0;
				EnableWindow(GetDlgItem(hwndDlg, aa_Status[i]+0x20), enabledReconnect);
				StrikeOut(hwndDlg,aa_Status[i]+0x20,bita);
				CheckDlgButton(hwndDlg, aa_Status[i]+0x20, bitr?BST_CHECKED:BST_UNCHECKED );
		}
 		SendDlgItemMessage(hwndDlg,IDC_RECONNECTSLIDER,TBM_SETPOS,TRUE,GetSliderFromReconnectDelay(GetBits(reconnectOpts[coursel],aa_ReconnectBitsDelay)));
		moveScrollBarEvent(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_ENABLERECONNECT, enabledReconnect?BST_CHECKED:BST_UNCHECKED );
		CheckDlgButton(hwndDlg, IDC_NOTFATALERROR, dontReconnectOnFatalError?BST_CHECKED:BST_UNCHECKED );
		EnableWindow(GetDlgItem(hwndDlg, IDC_RECONNECTSLIDER), enabledReconnect);
		EnableWindow(GetDlgItem(hwndDlg, IDC_NOTFATALERROR), enabledReconnect);
		CheckDlgButton(hwndDlg, IDC_KEEPMAINSTATUSOFFLINE,	(idleOpts&KeepMainStatusOffline)? BST_CHECKED : BST_UNCHECKED);
	return TRUE;
}


INT_PTR CALLBACK DlgProcOptsReconnect(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
        case WM_INITDIALOG:
            TranslateDialogDefault(hwndDlg);
			SetDlgItemsReconnect(hwndDlg, courProtocolSelection);
            return TRUE;
		case WM_USER+5:
		{
			int thebit = numStatuses-StatusToProtoIndex(LOWORD(lParam),allprotomodes);
			int value = IsDlgButtonChecked(hwndDlg, LOWORD(lParam)+32)==BST_CHECKED;
			thebit = 1<<thebit;
			reconnectOpts[courProtocolSelection] =
				SetBits(reconnectOpts[courProtocolSelection],
					thebit,
					value);
			break;
		}
		case WM_USER+6:
		{
			reconnectOpts[courProtocolSelection] = SetBits(reconnectOpts[courProtocolSelection],aa_ReconnectBitsEnabled,wParam);
			SetDlgItemsReconnect(hwndDlg,courProtocolSelection);
			break;
		}
		case WM_USER+7:
		{
			reconnectOpts[courProtocolSelection] = SetBits(reconnectOpts[courProtocolSelection],aa_ReconnectNotFatalError,wParam);
			SetDlgItemsReconnect(hwndDlg,courProtocolSelection);
			break;
		}
		case WM_USER+8:
		{
			idleOpts = SetBits(idleOpts,KeepMainStatusOffline,wParam);
			break;
		}
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
				case 	ID_STATUS_AWAY+32:
				case 	ID_STATUS_DND+32:
				case 	ID_STATUS_NA+32:
				case 	ID_STATUS_OCCUPIED+32:
				case 	ID_STATUS_INVISIBLE+32:
				case 	ID_STATUS_ONTHEPHONE+32:
				case 	ID_STATUS_OUTTOLUNCH+32:
				case 	ID_STATUS_ONLINE+32:
				case 	ID_STATUS_FREECHAT+32:
				case 	ID_STATUS_OFFLINE+32:
						SendMessage(hwndDlg, WM_USER+5,0,(wParam-32)|0x00100000);
						break;
 				case IDC_ENABLERECONNECT:
					SendMessage(hwndDlg, WM_USER+6, (WPARAM)IsDlgButtonChecked(hwndDlg, IDC_ENABLERECONNECT) == BST_CHECKED, 0);
					break;
				case IDC_NOTFATALERROR:
					SendMessage(hwndDlg, WM_USER+7, (WPARAM)IsDlgButtonChecked(hwndDlg, IDC_NOTFATALERROR) == BST_CHECKED, 0);
					break;
				case IDC_KEEPMAINSTATUSOFFLINE:
					SendMessage(hwndDlg, WM_USER+8, (WPARAM)IsDlgButtonChecked(hwndDlg, IDC_KEEPMAINSTATUSOFFLINE) == BST_CHECKED, 0);
					break;
           }
		if (HIWORD(wParam)==BN_CLICKED)
		{
           SendMessage(GetParent(hwndDlg), PSM_CHANGED, (unsigned int)hwndDlg, 0);
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
		case WM_HSCROLL:
			if((HWND)lParam == GetDlgItem(hwndDlg, IDC_RECONNECTSLIDER)) {
				moveScrollBarEvent(hwndDlg);
				if (LOWORD(wParam) == SB_ENDSCROLL) SendMessage(GetParent(hwndDlg), PSM_CHANGED, (unsigned int)hwndDlg, 0);
			}
			break;
        case WM_DESTROY:
            break;
    }
    return FALSE;
}
