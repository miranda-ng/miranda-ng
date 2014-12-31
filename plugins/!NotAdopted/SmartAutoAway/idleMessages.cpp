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


long *idleMessOptsPerm;
long *idleMessOpts;
                                                // xxxx xxxL xxxS xxxH xxxE xxx- RSTHERE-
int *messCaps=0;

 TCHAR **mesgHere;
 TCHAR **mesgShort;
 TCHAR **mesgLong;
 TCHAR **mesgHerePerm;
 TCHAR **mesgShortPerm;
 TCHAR **mesgLongPerm;

BOOL InitDialogIdleMessagePart(HWND hwndDlg)
{
	return TRUE;
}

void GetStringFromDelayMsg(char * str, int delay)
{
	int min,sec;
	min = (int)delay/60;
	sec = delay-min*60;
	sprintf(str,"Set the default status message after ");
	if (min) sprintf(str,"%s%d min ",str,min);
	if (sec) sprintf(str,"%s%d sec",str,sec);
//	if (!min) sprintf(str,"%s (Too fast!)",str);
}


void moveScrollBarRestoreMsgEvent(HWND hwndDlg){
	char str[256];
	int slide = SendDlgItemMessage(hwndDlg,IDC_RESTOREMSGSLIDER,TBM_GETPOS,0,0);
	int delay = GetReconnectDelayFromSlider(slide)/6;
	GetStringFromDelayMsg(str,delay);
	SetDlgItemTextA(hwndDlg,IDC_ENABLERESTOREMSG,str);
	idleMessOpts[courProtocolSelection] = SetBits(idleMessOpts[courProtocolSelection],IdleMessOptsMaskRstHereDelay,delay);
}


static BOOL idleDialogInInit = FALSE;

LRESULT SetDlgItemsIdleMessages(HWND hwndDlg, int coursel){
		int i,t;
		TCHAR str[64];
		BOOL  statusAvail[3];
		for (i=0;i<3;i++) {
			switch (i) {
				case 0 : t = reqStatus[coursel]; break;
				case 1 : t = aa_Status[GetBits(awayStatuses[coursel],maskShortAwayStatus)-1]; break;
				case 2 : t = aa_Status[GetBits(awayStatuses[coursel],maskLongAwayStatus)-1]; break;
				default: t = 0;
			}
			statusAvail[i] = messCaps[coursel] & Proto_Status2Flag(t);
			_sntprintf(str,63,_T("%s"),CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, t, GSMDF_TCHAR));
			switch (i) {
				case 0 : _sntprintf(str,63,_T("%s: %s"),TranslateT("I am here"),CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, t, GSMDF_TCHAR));t = IDC_IDLEGROUP_HERE; break;
				case 1 : _sntprintf(str,63,_T("%s: %s"),TranslateT("Short Idle"),CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, t, GSMDF_TCHAR));t = IDC_IDLEGROUP_SHORT; break;
				case 2 : _sntprintf(str,63,_T("%s: %s"),TranslateT("Long Idle"),CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, t, GSMDF_TCHAR));t = IDC_IDLEGROUP_LONG; break;
				default: t = 0;
			}
			SetDlgItemText(hwndDlg,t,str);
		}

		EnableWindow(GetDlgItem(hwndDlg, IDC_IDLEGROUP_HERE), messCaps[coursel]?1:0);
		EnableWindow(GetDlgItem(hwndDlg, IDC_IDLEMSGHERE_THIS), statusAvail[0]?1:0);
		EnableWindow(GetDlgItem(hwndDlg, IDC_IDLEMSGHERE), (statusAvail[0] && GetBits(idleMessOpts[coursel],IdleMessOptsMaskUseHere))?1:0);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLERESTOREMSG), (statusAvail[0] && GetBits(idleMessOpts[coursel],IdleMessOptsMaskUseHere))?1:0);
		EnableWindow(GetDlgItem(hwndDlg, IDC_RESTOREMSGSLIDER), (statusAvail[0] && GetBits(idleMessOpts[coursel],IdleMessOptsMaskUseHere) && GetBits(idleMessOpts[coursel],IdleMessOptsMaskRstHere))?1:0);
		EnableWindow(GetDlgItem(hwndDlg, IDC_IDLEGROUP_SHORT), messCaps[coursel]?1:0);
		EnableWindow(GetDlgItem(hwndDlg, IDC_IDLEMSGSHORT_THIS), statusAvail[1]?1:0);
		EnableWindow(GetDlgItem(hwndDlg, IDC_IDLEMSGSHORT), (statusAvail[1] && GetBits(idleMessOpts[coursel],IdleMessOptsMaskUseShort))?1:0);
		EnableWindow(GetDlgItem(hwndDlg, IDC_IDLEGROUP_LONG), messCaps[coursel]?1:0);
		EnableWindow(GetDlgItem(hwndDlg, IDC_IDLEMSGLONG_THIS), statusAvail[2]?1:0);
		EnableWindow(GetDlgItem(hwndDlg, IDC_IDLEMSGLONG), (statusAvail[2] && GetBits(idleMessOpts[coursel],IdleMessOptsMaskUseLong))?1:0);

		CheckDlgButton(hwndDlg, IDC_IDLEMSGHERE_THIS, (statusAvail[0] && GetBits(idleMessOpts[coursel],IdleMessOptsMaskUseHere))?BST_CHECKED:BST_UNCHECKED );
		CheckDlgButton(hwndDlg, IDC_ENABLERESTOREMSG, (
			statusAvail[0] &&
			GetBits(idleMessOpts[coursel],IdleMessOptsMaskUseHere) &&
			GetBits(idleMessOpts[coursel],IdleMessOptsMaskRstHere))?BST_CHECKED:BST_UNCHECKED );
		CheckDlgButton(hwndDlg, IDC_IDLEMSGSHORT_THIS, (statusAvail[1] && GetBits(idleMessOpts[coursel],IdleMessOptsMaskUseShort))?BST_CHECKED:BST_UNCHECKED );
		CheckDlgButton(hwndDlg, IDC_IDLEMSGLONG_THIS, (statusAvail[2] && GetBits(idleMessOpts[coursel],IdleMessOptsMaskUseLong))?BST_CHECKED:BST_UNCHECKED );

	SendDlgItemMessage(hwndDlg,IDC_RESTOREMSGSLIDER,TBM_SETPOS,TRUE,GetSliderFromReconnectDelay(GetBits(idleMessOpts[coursel],IdleMessOptsMaskRstHereDelay)*6));
	moveScrollBarRestoreMsgEvent(hwndDlg);
	idleDialogInInit = TRUE;
	SetDlgItemText(hwndDlg,IDC_IDLEMSGHERE,mesgHere[coursel]);
	SetDlgItemText(hwndDlg,IDC_IDLEMSGSHORT,mesgShort[coursel]);
	SetDlgItemText(hwndDlg,IDC_IDLEMSGLONG,mesgLong[coursel]);
	if (messCaps[coursel]) 	{
		SetDlgItemTextA(hwndDlg,IDC_PROTODONOTSUPPORT,"");
	} else {
		_sntprintf(str,63,_T("%S cannot set status messages"),accounts[coursel]->szModuleName);
		SetDlgItemText(hwndDlg,IDC_PROTODONOTSUPPORT,str);
	}
	idleDialogInInit = FALSE;
	EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLES), (messCaps[coursel]));
//	CheckDlgButton(hwndDlg, IDC_ENABLERECONNECT, enabledReconnect?BST_CHECKED:BST_UNCHECKED );
//	CheckDlgButton(hwndDlg, IDC_NOTFATALERROR, dontReconnectOnFatalError?BST_CHECKED:BST_UNCHECKED );
//	EnableWindow(GetDlgItem(hwndDlg, IDC_RECONNECTSLIDER), enabledReconnect);
//	EnableWindow(GetDlgItem(hwndDlg, IDC_NOTFATALERROR), enabledReconnect);
	return TRUE;
}



INT_PTR CALLBACK DlgProcOptsIdleMessages(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
        case WM_INITDIALOG:
            TranslateDialogDefault(hwndDlg);
			SetDlgItemsIdleMessages(hwndDlg, courProtocolSelection);
            return TRUE;
		case WM_USER+1:
		{
			EnableWindow(GetDlgItem(hwndDlg, IDC_RESTOREMSGSLIDER), wParam);
			idleMessOpts[courProtocolSelection] = SetBits(idleMessOpts[courProtocolSelection],IdleMessOptsMaskRstHere,wParam);
			break;
		}
		case WM_USER+2:
		{
			EnableWindow(GetDlgItem(hwndDlg, IDC_IDLEMSGHERE), wParam);
			EnableWindow(GetDlgItem(hwndDlg, IDC_RESTOREMSGSLIDER), wParam);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLERESTOREMSG), wParam);
			idleMessOpts[courProtocolSelection] = SetBits(idleMessOpts[courProtocolSelection],IdleMessOptsMaskUseHere,wParam);
			break;
		}
		case WM_USER+3:
		{
			EnableWindow(GetDlgItem(hwndDlg, IDC_IDLEMSGSHORT), wParam);
			idleMessOpts[courProtocolSelection] = SetBits(idleMessOpts[courProtocolSelection],IdleMessOptsMaskUseShort,wParam);
			break;
		}
		case WM_USER+4:
		{
			EnableWindow(GetDlgItem(hwndDlg, IDC_IDLEMSGLONG), wParam);
			idleMessOpts[courProtocolSelection] = SetBits(idleMessOpts[courProtocolSelection],IdleMessOptsMaskUseLong,wParam);
			break;
		}
		case WM_USER+8:
		{
			TCHAR temp[maxMessageLength+1];
			ParseString((TCHAR *)lParam,temp,maxMessageLength);
#ifdef UNICODE
			if(!protoHasAwayMessageW[courProtocolSelection]) {
				char tempa[maxMessageLength+1];
				my_u2a(temp, tempa, maxMessageLength);
				my_a2u(tempa,temp,maxMessageLength);
			}
#endif
			SetDlgItemText(hwndDlg,IDC_PROTODONOTSUPPORT,temp);
			break;
		}
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
 				case IDC_ENABLERESTOREMSG:
					{
						SendMessage(hwndDlg, WM_USER+1, (WPARAM)IsDlgButtonChecked(hwndDlg, IDC_ENABLERESTOREMSG) == BST_CHECKED, 0);
						break;
					}
				case IDC_IDLEMSGHERE_THIS:
					{
						SendMessage(hwndDlg,WM_USER+2, (WPARAM)IsDlgButtonChecked(hwndDlg, IDC_IDLEMSGHERE_THIS) == BST_CHECKED, 0);
						break;
					}
				case IDC_IDLEMSGSHORT_THIS:
					{
						SendMessage(hwndDlg,WM_USER+3, (WPARAM)IsDlgButtonChecked(hwndDlg, IDC_IDLEMSGSHORT_THIS) == BST_CHECKED, 0);
						break;
					}
				case IDC_IDLEMSGLONG_THIS:
					{
						SendMessage(hwndDlg,WM_USER+4, (WPARAM)IsDlgButtonChecked(hwndDlg, IDC_IDLEMSGLONG_THIS) == BST_CHECKED, 0);
						break;
					}
				case IDC_IDLEMSGHERE:
					{
						if (HIWORD(wParam)==EN_CHANGE || HIWORD(wParam)==EN_SETFOCUS){
							TCHAR str[maxMessageLength+1];
							GetDlgItemText(hwndDlg,IDC_IDLEMSGHERE,str,maxMessageLength+1);
							SendMessage(hwndDlg,WM_USER+8, 0, (LPARAM)str);
						} else if (HIWORD(wParam)==EN_KILLFOCUS) {
							GetDlgItemText(hwndDlg,IDC_IDLEMSGHERE,mesgHere[courProtocolSelection],maxMessageLength+1);
							SendMessage(hwndDlg,WM_USER+8, 0, (LPARAM)"");
						}
						break;
					}
				case IDC_IDLEMSGSHORT:
					{
						if (HIWORD(wParam)==EN_CHANGE || HIWORD(wParam)==EN_SETFOCUS){
							TCHAR str[maxMessageLength+1];
							GetDlgItemText(hwndDlg,IDC_IDLEMSGSHORT,str,maxMessageLength+1);
							SendMessage(hwndDlg,WM_USER+8, 0, (LPARAM)str);
						} else if (HIWORD(wParam)==EN_KILLFOCUS) {
							GetDlgItemText(hwndDlg,IDC_IDLEMSGSHORT,mesgShort[courProtocolSelection],maxMessageLength+1);
							SendMessage(hwndDlg,WM_USER+8, 0, (LPARAM)"");
						}
						break;
					}
				case IDC_IDLEMSGLONG:
					{
						if (HIWORD(wParam)==EN_CHANGE || HIWORD(wParam)==EN_SETFOCUS){
							TCHAR str[maxMessageLength+1];
							GetDlgItemText(hwndDlg,IDC_IDLEMSGLONG,str,maxMessageLength+1);
							SendMessage(hwndDlg,WM_USER+8, 0, (LPARAM)str);
						} else if (HIWORD(wParam)==EN_KILLFOCUS) {
							GetDlgItemText(hwndDlg,IDC_IDLEMSGLONG,mesgLong[courProtocolSelection],maxMessageLength+1);
							SendMessage(hwndDlg,WM_USER+8, 0, (LPARAM)"");
						}
						break;
					}
				case IDC_VARIABLES:
					{
						TCHAR szout[2048]=_T("");
						UINT pos=0;
						for (int i=0;i<25;i++){
							if (VariableList[i][0]){
								pos+=_sntprintf(&szout[pos],2048-pos,_T("%%%s \t %s\n"),VariableList[i][0],TranslateTS(VariableList[i][1]));
							} else {
								pos+=_sntprintf(&szout[pos],2048-pos,_T("-- %s --\n"),TranslateTS(VariableList[i][1]));
							}
						}
						//sprintf(szout,VARIABLE_LIST);
						MessageBox(NULL,szout,TranslateT("Variables to use"),MB_OK|MB_TOPMOST);
						break;
					}

				break; //case WM_COMMAND
            }
            if (!idleDialogInInit) if ((HIWORD(wParam)==BN_CLICKED)||(HIWORD(wParam)==EN_CHANGE))
			{
				if (LOWORD(wParam)!=IDC_VARIABLES)
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
		case WM_HSCROLL:
			if((HWND)lParam == GetDlgItem(hwndDlg, IDC_RESTOREMSGSLIDER)) {
				moveScrollBarRestoreMsgEvent(hwndDlg);
				if (LOWORD(wParam) == SB_ENDSCROLL) SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;
        case WM_DESTROY:
            break;
    }
    return FALSE;
}
